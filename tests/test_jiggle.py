"""Tests for the jiggle system: 10 modes, offset generation, centroid, targeting."""

import math
import pytest


class TestJiggleHelpers:
    def test_gauss_rand_distribution(self, page):
        """gaussRand() should produce roughly normal distribution with mean≈0."""
        result = page.evaluate("""() => {
            var samples = [];
            for (var i = 0; i < 1000; i++) samples.push(gaussRand());
            var mean = samples.reduce((a, b) => a + b, 0) / samples.length;
            var variance = samples.reduce((a, b) => a + (b - mean) ** 2, 0) / samples.length;
            return { mean, variance, count: samples.length };
        }""")
        assert abs(result["mean"]) < 0.15, f"Mean too far from 0: {result['mean']}"
        assert 0.3 < result["variance"] < 3.0, f"Variance out of range: {result['variance']}"

    def test_jiggle_target_coeffs(self, page):
        """jiggleTargetCoeffs() returns selected coefficient indices."""
        result = page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            var targets = [...jiggleTargetCoeffs()];
            return { targets, nCoeffs: coefficients.length };
        }""")
        assert len(result["targets"]) == result["nCoeffs"]

    def test_compute_jiggle_centroid(self, page):
        """computeJiggleCentroid() returns center of mass of home positions."""
        result = page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            var targets = jiggleTargetCoeffs();
            var centroid = computeJiggleCentroid(targets);
            // Manual computation
            var arr = [...targets];
            var sumRe = 0, sumIm = 0;
            for (var idx of arr) {
                sumRe += coefficients[idx].curve[0].re;
                sumIm += coefficients[idx].curve[0].im;
            }
            return {
                centroid,
                expected: { re: sumRe / arr.length, im: sumIm / arr.length }
            };
        }""")
        c = result["centroid"]
        e = result["expected"]
        assert abs(c["re"] - e["re"]) < 1e-10
        assert abs(c["im"] - e["im"]) < 1e-10


class TestJiggleModes:
    """Test each of the 10 jiggle modes produces offsets."""

    MODES = ["random", "rotate", "walk", "scale", "circle",
             "spiral", "breathe", "wobble", "lissajous"]

    @pytest.mark.parametrize("mode", MODES)
    def test_mode_produces_offsets(self, page, mode):
        """Each jiggle mode should produce a non-empty offset map."""
        result = page.evaluate("""(mode) => {
            jiggleMode = mode;
            jiggleSigma = 10;
            jiggleTheta = 0.1;
            jiggleScaleStep = 0.05;
            jigglePeriod = 4;
            jiggleAmplitude = 10;
            jiggleLissFreqX = 3;
            jiggleLissFreqY = 2;
            jiggleCumulativeAngle = 0;
            jiggleCumulativeScale = 1.0;
            jiggleTriggerCount = 0;
            generateJiggleOffsets();
            if (!jiggleOffsets) return { size: 0 };
            var offsets = [];
            jiggleOffsets.forEach((v, k) => offsets.push({idx: k, re: v.re, im: v.im}));
            jiggleMode = "none";
            jiggleOffsets = null;
            return { size: offsets.length, offsets };
        }""", mode)
        assert result["size"] > 0, f"Mode '{mode}' produced no offsets"

    @pytest.mark.parametrize("mode", MODES)
    def test_mode_offsets_are_finite(self, page, mode):
        """All offsets should be finite numbers."""
        result = page.evaluate("""(mode) => {
            jiggleMode = mode;
            jiggleSigma = 10;
            jiggleTheta = 0.1;
            jiggleScaleStep = 0.05;
            jigglePeriod = 4;
            jiggleAmplitude = 10;
            jiggleLissFreqX = 3;
            jiggleLissFreqY = 2;
            jiggleCumulativeAngle = 0;
            jiggleCumulativeScale = 1.0;
            jiggleTriggerCount = 0;
            generateJiggleOffsets();
            var allFinite = true;
            if (jiggleOffsets) {
                jiggleOffsets.forEach((v, k) => {
                    if (!isFinite(v.re) || !isFinite(v.im)) allFinite = false;
                });
            }
            var size = jiggleOffsets ? jiggleOffsets.size : 0;
            jiggleMode = "none";
            jiggleOffsets = null;
            return { allFinite, size };
        }""", mode)
        assert result["allFinite"] is True, f"Mode '{mode}' produced non-finite offsets"

    def test_none_mode_no_offsets(self, page):
        """jiggleMode='none' should not generate offsets."""
        result = page.evaluate("""() => {
            jiggleMode = "none";
            generateJiggleOffsets();
            return { offsets: jiggleOffsets };
        }""")
        assert result["offsets"] is None


class TestJiggleCumulative:
    def test_rotate_accumulates_angle(self, page):
        """Rotate mode should increase jiggleCumulativeAngle by theta*2π each call."""
        result = page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            jiggleMode = "rotate";
            jiggleTheta = 0.1;
            jiggleCumulativeAngle = 0;
            generateJiggleOffsets();
            var angle1 = jiggleCumulativeAngle;
            generateJiggleOffsets();
            var angle2 = jiggleCumulativeAngle;
            jiggleMode = "none";
            jiggleOffsets = null;
            return { angle1, angle2, step: 0.1 * 2 * Math.PI };
        }""")
        step = result["step"]  # 0.1 * 2π
        assert abs(result["angle1"] - step) < 1e-10
        assert abs(result["angle2"] - 2 * step) < 1e-10

    def test_scale_accumulates(self, page):
        """Scale mode should multiply jiggleCumulativeScale by (1 + step/100)."""
        result = page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            jiggleMode = "scale";
            jiggleScaleStep = 0.05;
            jiggleCumulativeScale = 1.0;
            generateJiggleOffsets();
            var scale1 = jiggleCumulativeScale;
            jiggleMode = "none";
            jiggleOffsets = null;
            return { scale1 };
        }""")
        # Formula: scale *= (1 + 0.05/100) = 1.0005
        assert abs(result["scale1"] - 1.0005) < 1e-10

    def test_trigger_count_increments(self, page):
        """jiggleTriggerCount should increment on each call."""
        result = page.evaluate("""() => {
            jiggleMode = "breathe";
            jigglePeriod = 4;
            jiggleAmplitude = 10;
            jiggleTriggerCount = 0;
            generateJiggleOffsets();
            var count1 = jiggleTriggerCount;
            generateJiggleOffsets();
            var count2 = jiggleTriggerCount;
            jiggleMode = "none";
            jiggleOffsets = null;
            return { count1, count2 };
        }""")
        assert result["count1"] == 1
        assert result["count2"] == 2


class TestJiggleSaveLoad:
    def test_jiggle_mode_roundtrip(self, page):
        """Jiggle mode and parameters survive save/load."""
        result = page.evaluate("""() => {
            jiggleMode = "lissajous";
            jiggleSigma = 20;
            jiggleLissFreqX = 5;
            jiggleLissFreqY = 3;
            var state = buildStateMetadata();
            jiggleMode = "none";
            jiggleSigma = 10;
            applyLoadedState(state);
            return {
                mode: jiggleMode,
                sigma: jiggleSigma,
                freqX: jiggleLissFreqX,
                freqY: jiggleLissFreqY
            };
        }""")
        assert result["mode"] == "lissajous"
        assert result["sigma"] == 20
        assert result["freqX"] == 5
        assert result["freqY"] == 3
