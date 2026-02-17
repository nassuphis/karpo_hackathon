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
    """Test each of the 11 jiggle modes produces offsets."""

    MODES = ["random", "rotate", "walk", "scale-center", "scale-centroid", "circle",
             "spiral-centroid", "spiral-center", "breathe", "wobble", "lissajous"]

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
            jiggleStep = -1;
            jiggleWalkCache = null;
            computeJiggleForStep(1);
            if (!jiggleOffsets) return { size: 0 };
            var offsets = [];
            jiggleOffsets.forEach((v, k) => offsets.push({idx: k, re: v.re, im: v.im}));
            jiggleMode = "none";
            jiggleOffsets = null;
            jiggleStep = -1;
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
            jiggleStep = -1;
            jiggleWalkCache = null;
            computeJiggleForStep(1);
            var allFinite = true;
            if (jiggleOffsets) {
                jiggleOffsets.forEach((v, k) => {
                    if (!isFinite(v.re) || !isFinite(v.im)) allFinite = false;
                });
            }
            var size = jiggleOffsets ? jiggleOffsets.size : 0;
            jiggleMode = "none";
            jiggleOffsets = null;
            jiggleStep = -1;
            return { allFinite, size };
        }""", mode)
        assert result["allFinite"] is True, f"Mode '{mode}' produced non-finite offsets"

    def test_none_mode_no_offsets(self, page):
        """jiggleMode='none' should not generate offsets."""
        result = page.evaluate("""() => {
            jiggleMode = "none";
            computeJiggleForStep(1);
            return { offsets: jiggleOffsets };
        }""")
        assert result["offsets"] is None


class TestJigglePureFunction:
    def test_rotate_pure_function_of_step(self, page):
        """Rotate mode: offset at step N = rotation by theta*2π*N (no accumulation)."""
        result = page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            jiggleMode = "rotate";
            jiggleTheta = 0.1;

            // Compute at step 1
            jiggleStep = -1;
            computeJiggleForStep(1);
            var off1 = [];
            jiggleOffsets.forEach((v, k) => off1.push({re: v.re, im: v.im}));

            // Compute at step 2
            jiggleStep = -1;
            computeJiggleForStep(2);
            var off2 = [];
            jiggleOffsets.forEach((v, k) => off2.push({re: v.re, im: v.im}));

            // Recompute step 1 again — should get same result (pure function)
            jiggleStep = -1;
            computeJiggleForStep(1);
            var off1b = [];
            jiggleOffsets.forEach((v, k) => off1b.push({re: v.re, im: v.im}));

            jiggleMode = "none";
            jiggleOffsets = null;
            jiggleStep = -1;
            return { off1, off2, off1b };
        }""")
        # step 1 called twice should give same result
        for a, b in zip(result["off1"], result["off1b"]):
            assert abs(a["re"] - b["re"]) < 1e-10
            assert abs(a["im"] - b["im"]) < 1e-10
        # step 1 and step 2 should differ
        any_diff = any(abs(a["re"] - b["re"]) > 1e-10 or abs(a["im"] - b["im"]) > 1e-10
                       for a, b in zip(result["off1"], result["off2"]))
        assert any_diff, "Steps 1 and 2 should produce different offsets"

    def test_scale_pure_function_of_step(self, page):
        """Scale mode: offset is a pure function of step (no accumulation)."""
        result = page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            jiggleMode = "scale-center";
            jiggleScaleStep = 0.5;

            jiggleStep = -1;
            computeJiggleForStep(3);
            var off3 = [];
            jiggleOffsets.forEach((v, k) => off3.push({re: v.re, im: v.im}));

            // Recompute step 3 — same result
            jiggleStep = -1;
            computeJiggleForStep(3);
            var off3b = [];
            jiggleOffsets.forEach((v, k) => off3b.push({re: v.re, im: v.im}));

            jiggleMode = "none";
            jiggleOffsets = null;
            jiggleStep = -1;
            return { off3, off3b };
        }""")
        for a, b in zip(result["off3"], result["off3b"]):
            assert abs(a["re"] - b["re"]) < 1e-10
            assert abs(a["im"] - b["im"]) < 1e-10

    def test_breathe_pure_function_of_step(self, page):
        """Breathe mode: offset is sin(2π·step/period), pure function of step."""
        result = page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            jiggleMode = "breathe";
            jigglePeriod = 8;
            jiggleAmplitude = 10;

            jiggleStep = -1;
            computeJiggleForStep(2);
            var off2 = [];
            jiggleOffsets.forEach((v, k) => off2.push({re: v.re, im: v.im}));

            // Same step again
            jiggleStep = -1;
            computeJiggleForStep(2);
            var off2b = [];
            jiggleOffsets.forEach((v, k) => off2b.push({re: v.re, im: v.im}));

            jiggleMode = "none";
            jiggleOffsets = null;
            jiggleStep = -1;
            return { off2, off2b };
        }""")
        for a, b in zip(result["off2"], result["off2b"]):
            assert abs(a["re"] - b["re"]) < 1e-10
            assert abs(a["im"] - b["im"]) < 1e-10

    def test_same_step_is_cached(self, page):
        """Calling computeJiggleForStep with same step should be a no-op (cached)."""
        result = page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            jiggleMode = "random";
            jiggleSigma = 10;
            jiggleStep = -1;

            computeJiggleForStep(5);
            var off1 = [];
            jiggleOffsets.forEach((v, k) => off1.push({re: v.re, im: v.im}));

            // Same step — cached, no recomputation
            computeJiggleForStep(5);
            var off2 = [];
            jiggleOffsets.forEach((v, k) => off2.push({re: v.re, im: v.im}));

            jiggleMode = "none";
            jiggleOffsets = null;
            jiggleStep = -1;
            return { off1, off2 };
        }""")
        # Same step should return identical random offsets (cached)
        for a, b in zip(result["off1"], result["off2"]):
            assert abs(a["re"] - b["re"]) < 1e-10
            assert abs(a["im"] - b["im"]) < 1e-10

    def test_jiggle_step_from_elapsed(self, page):
        """jiggleStepFromElapsed should return floor(elapsed / interval)."""
        result = page.evaluate("""() => {
            jiggleMode = "breathe";
            jiggleInterval = 4;
            return {
                s0: jiggleStepFromElapsed(0),
                s1: jiggleStepFromElapsed(3.9),
                s2: jiggleStepFromElapsed(4.0),
                s3: jiggleStepFromElapsed(8.5),
                s4: jiggleStepFromElapsed(12.0)
            };
        }""")
        assert result["s0"] == 0
        assert result["s1"] == 0
        assert result["s2"] == 1
        assert result["s3"] == 2
        assert result["s4"] == 3


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
