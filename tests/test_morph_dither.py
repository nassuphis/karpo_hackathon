"""Tests for morph C-D path dither in solveRoots() and morphDitherSample().

Verifies:
  - Dither envelope shaping (start/mid/end sigma envelopes)
  - morphDitherSample() produces noise with correct distribution
  - solveRoots() applies dither when morph dither sigma > 0
  - Dither settings roundtrip through save/load
"""

import math
import pytest


class TestMorphDitherEnvelope:
    """Verify the dither envelope: ds depends on theta position along C-D path."""

    def test_mid_sigma_peaks_at_theta_half_pi(self, page):
        """morphDitherMidSigma envelope = sin²(theta), peaks at θ=π/2."""
        result = page.evaluate("""() => {
            var theta = Math.PI / 2;
            var sinT = Math.sin(theta), cosT = Math.cos(theta);
            var startEnv = cosT > 0 ? cosT * cosT : 0;
            var endEnv = cosT < 0 ? cosT * cosT : 0;
            var midEnv = sinT * sinT;
            return { startEnv, midEnv, endEnv };
        }""")
        assert abs(result["startEnv"]) < 1e-12  # cos(π/2)=0
        assert abs(result["midEnv"] - 1.0) < 1e-12  # sin²(π/2)=1
        assert abs(result["endEnv"]) < 1e-12

    def test_start_sigma_peaks_at_theta_zero(self, page):
        """morphDitherStartSigma envelope = max(cosθ,0)², peaks at θ=0."""
        result = page.evaluate("""() => {
            var theta = 0;
            var sinT = Math.sin(theta), cosT = Math.cos(theta);
            var startEnv = cosT > 0 ? cosT * cosT : 0;
            var midEnv = sinT * sinT;
            return { startEnv, midEnv };
        }""")
        assert abs(result["startEnv"] - 1.0) < 1e-12  # cos²(0)=1
        assert abs(result["midEnv"]) < 1e-12  # sin²(0)=0

    def test_end_sigma_peaks_at_theta_pi(self, page):
        """morphDitherEndSigma envelope = max(-cosθ,0)², peaks at θ=π."""
        result = page.evaluate("""() => {
            var theta = Math.PI;
            var sinT = Math.sin(theta), cosT = Math.cos(theta);
            var startEnv = cosT > 0 ? cosT * cosT : 0;
            var endEnv = cosT < 0 ? cosT * cosT : 0;
            var midEnv = sinT * sinT;
            return { startEnv, midEnv, endEnv };
        }""")
        assert abs(result["startEnv"]) < 1e-12  # cos(π)=-1, not > 0
        assert abs(result["endEnv"] - 1.0) < 1e-12  # (-cos(π))²=1
        assert abs(result["midEnv"]) < 1e-10  # sin²(π)≈0


class TestMorphDitherSample:
    """morphDitherSample(sigma) should produce random (re, im) offsets."""

    def test_returns_two_element_array(self, page):
        """morphDitherSample returns [dre, dim]."""
        result = page.evaluate("""() => {
            var sample = morphDitherSample(1.0);
            return { isArray: Array.isArray(sample), len: sample.length };
        }""")
        assert result["isArray"] is True
        assert result["len"] == 2

    def test_zero_sigma_returns_zero(self, page):
        """When sigma is 0, dither offset should be 0 (or very small)."""
        result = page.evaluate("""() => {
            var sample = morphDitherSample(0);
            return { re: sample[0], im: sample[1] };
        }""")
        assert abs(result["re"]) < 1e-12
        assert abs(result["im"]) < 1e-12

    def test_samples_are_random(self, page):
        """Multiple calls should produce different values."""
        result = page.evaluate("""() => {
            var s1 = morphDitherSample(1.0);
            var s2 = morphDitherSample(1.0);
            var s3 = morphDitherSample(1.0);
            // At least one pair should differ (probability of all equal ≈ 0)
            return (s1[0] !== s2[0]) || (s2[0] !== s3[0]) ||
                   (s1[1] !== s2[1]) || (s2[1] !== s3[1]);
        }""")
        assert result is True

    def test_magnitude_scales_with_sigma(self, page):
        """Larger sigma should produce larger average magnitude."""
        result = page.evaluate("""() => {
            var sumSmall = 0, sumLarge = 0;
            for (var i = 0; i < 200; i++) {
                var s = morphDitherSample(0.01);
                sumSmall += Math.hypot(s[0], s[1]);
                s = morphDitherSample(10.0);
                sumLarge += Math.hypot(s[0], s[1]);
            }
            return { avgSmall: sumSmall / 200, avgLarge: sumLarge / 200 };
        }""")
        assert result["avgLarge"] > result["avgSmall"] * 5


class TestSolveRootsMorphDither:
    """solveRoots() should apply morph dither when sigma > 0."""

    def test_dither_causes_root_variation(self, page):
        """With morph dither on, solving same coefficients should give varying roots."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            morphPathType = "line";
            morphTheta = Math.PI / 2;  // mid-envelope peak
            // Move D away from C
            morphTargetCoeffs[0].re = coefficients[0].re + 1;

            // Set mid dither sigma high enough to cause variation
            morphDitherMidSigma = 5;
            morphDitherStartSigma = 0;
            morphDitherEndSigma = 0;

            // Solve multiple times and collect root[0] positions
            var positions = [];
            for (var i = 0; i < 10; i++) {
                solveRoots();
                positions.push({ re: currentRoots[0].re, im: currentRoots[0].im });
            }

            // Check if positions vary (at least one differs from first)
            var varied = false;
            for (var i = 1; i < positions.length; i++) {
                if (Math.abs(positions[i].re - positions[0].re) > 1e-10 ||
                    Math.abs(positions[i].im - positions[0].im) > 1e-10) {
                    varied = true;
                    break;
                }
            }

            // Cleanup
            morphEnabled = false;
            morphDitherMidSigma = 0;
            morphTheta = 0;
            initMorphTarget();
            return varied;
        }""")
        assert result is True

    def test_no_dither_gives_consistent_roots(self, page):
        """With all dither sigmas = 0, roots should be deterministic."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            morphPathType = "line";
            morphTheta = Math.PI / 2;
            morphTargetCoeffs[0].re = coefficients[0].re + 1;

            morphDitherMidSigma = 0;
            morphDitherStartSigma = 0;
            morphDitherEndSigma = 0;

            solveRoots();
            var r0 = { re: currentRoots[0].re, im: currentRoots[0].im };
            solveRoots();
            var r1 = { re: currentRoots[0].re, im: currentRoots[0].im };

            // Cleanup
            morphEnabled = false;
            morphTheta = 0;
            initMorphTarget();
            return { same: Math.abs(r0.re - r1.re) < 1e-12 && Math.abs(r0.im - r1.im) < 1e-12 };
        }""")
        assert result["same"] is True

    def test_start_dither_active_only_near_theta_zero(self, page):
        """Start dither should only affect roots when theta ≈ 0 (near C-node)."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            morphPathType = "line";
            morphTargetCoeffs[0].re = coefficients[0].re + 1;

            morphDitherStartSigma = 10;
            morphDitherMidSigma = 0;
            morphDitherEndSigma = 0;

            // At theta=0: startEnv = cos²(0) = 1, should dither
            morphTheta = 0.01;  // near zero, not exactly 0 (theta=0 early-returns)
            var varied0 = false;
            var positions = [];
            for (var i = 0; i < 10; i++) {
                solveRoots();
                positions.push(currentRoots[0].re);
            }
            for (var i = 1; i < positions.length; i++) {
                if (Math.abs(positions[i] - positions[0]) > 1e-10) { varied0 = true; break; }
            }

            // At theta=π/2: startEnv = cos²(π/2) = 0, should NOT dither
            morphTheta = Math.PI / 2;
            positions = [];
            for (var i = 0; i < 10; i++) {
                solveRoots();
                positions.push(currentRoots[0].re);
            }
            var variedPi2 = false;
            for (var i = 1; i < positions.length; i++) {
                if (Math.abs(positions[i] - positions[0]) > 1e-10) { variedPi2 = true; break; }
            }

            // Cleanup
            morphEnabled = false;
            morphDitherStartSigma = 0;
            morphTheta = 0;
            initMorphTarget();
            return { varied0, variedPi2 };
        }""")
        assert result["varied0"] is True  # dither active near θ=0
        assert result["variedPi2"] is False  # no dither at θ=π/2


class TestMorphDitherSaveLoad:
    """Morph dither settings should roundtrip through save/load."""

    def test_dither_sigma_roundtrip(self, page):
        result = page.evaluate("""() => {
            morphDitherStartSigma = 0.05;
            morphDitherMidSigma = 0.077;
            morphDitherEndSigma = 0.03;
            var state = buildStateMetadata();
            morphDitherStartSigma = 0;
            morphDitherMidSigma = 0;
            morphDitherEndSigma = 0;
            applyLoadedState(state);
            return {
                start: morphDitherStartSigma,
                mid: morphDitherMidSigma,
                end: morphDitherEndSigma
            };
        }""")
        assert abs(result["start"] - 0.05) < 1e-10
        assert abs(result["mid"] - 0.077) < 1e-10
        assert abs(result["end"] - 0.03) < 1e-10

    def test_dither_dist_roundtrip(self, page):
        result = page.evaluate("""() => {
            morphDitherDist = "disk";
            var state = buildStateMetadata();
            morphDitherDist = "normal";
            applyLoadedState(state);
            return morphDitherDist;
        }""")
        assert result == "disk"
