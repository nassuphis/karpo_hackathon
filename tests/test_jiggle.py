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
            jiggleSigma = 5.5;
            jiggleLissFreqX = 5;
            jiggleLissFreqY = 3;
            var state = buildStateMetadata();
            jiggleMode = "none";
            jiggleSigma = 1;
            applyLoadedState(state);
            return {
                mode: jiggleMode,
                sigma: jiggleSigma,
                freqX: jiggleLissFreqX,
                freqY: jiggleLissFreqY
            };
        }""")
        assert result["mode"] == "lissajous"
        assert result["sigma"] == 5.5
        assert result["freqX"] == 5
        assert result["freqY"] == 3


class TestNearestPrime:
    """Tests for the nearestPrime() utility function."""

    def test_prime_returns_itself(self, page):
        """A prime number should return itself."""
        result = page.evaluate("() => [2,3,5,7,11,13,97,101,4999].map(n => nearestPrime(n))")
        assert result == [2, 3, 5, 7, 11, 13, 97, 101, 4999]

    def test_composite_returns_nearest(self, page):
        """A composite should return the nearest prime."""
        result = page.evaluate("""() => {
            return {
                n4: nearestPrime(4),
                n6: nearestPrime(6),
                n9: nearestPrime(9),
                n10: nearestPrime(10),
                n100: nearestPrime(100),
                n1000: nearestPrime(1000)
            };
        }""")
        assert result["n4"] == 3 or result["n4"] == 5  # equidistant, either ok
        assert result["n6"] == 5 or result["n6"] == 7
        assert result["n9"] == 7 or result["n9"] == 11
        assert result["n10"] == 11
        assert result["n100"] == 101
        assert result["n1000"] == 997

    def test_small_values(self, page):
        """Values <= 2 should return 2."""
        result = page.evaluate("() => [nearestPrime(0), nearestPrime(1), nearestPrime(2)]")
        assert result == [2, 2, 2]

    def test_result_is_always_prime(self, page):
        """Result of nearestPrime should always be prime."""
        result = page.evaluate("""() => {
            function isPrime(v) {
                if (v < 2) return false;
                if (v === 2) return true;
                if (v % 2 === 0) return false;
                for (var d = 3; d * d <= v; d += 2) if (v % d === 0) return false;
                return true;
            }
            var failures = [];
            for (var n = 2; n <= 200; n++) {
                var p = nearestPrime(n);
                if (!isPrime(p)) failures.push({n, p});
            }
            return failures;
        }""")
        assert result == [], f"Non-prime results: {result}"

    def test_distance_is_minimal(self, page):
        """nearestPrime(n) should be at most as far as any other prime."""
        result = page.evaluate("""() => {
            function isPrime(v) {
                if (v < 2) return false;
                if (v === 2) return true;
                if (v % 2 === 0) return false;
                for (var d = 3; d * d <= v; d += 2) if (v % d === 0) return false;
                return true;
            }
            var failures = [];
            for (var n = 2; n <= 500; n++) {
                var p = nearestPrime(n);
                var dist = Math.abs(p - n);
                // Check no prime is closer
                for (var k = n - dist; k <= n + dist; k++) {
                    if (k >= 2 && isPrime(k) && Math.abs(k - n) < dist) {
                        failures.push({n, p, closer: k});
                        break;
                    }
                }
            }
            return failures;
        }""")
        assert result == [], f"Found closer primes: {result}"


class TestStepButtons:
    """Tests for -1/+1/P buttons on jiggle step/period controls."""

    MODES_WITH_STEPS = ["rotate", "circle", "spiral-centroid", "wobble", "lissajous"]

    MODES_WITH_PERIOD = ["breathe", "wobble"]

    @pytest.mark.parametrize("mode", MODES_WITH_STEPS + MODES_WITH_PERIOD)
    def test_buttons_exist(self, page, mode):
        """Each step/period control should have -1, +1, P buttons."""
        result = page.evaluate("""(mode) => {
            jiggleMode = mode;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var labels = [];
            buttons.forEach(b => labels.push(b.textContent));
            return labels;
        }""", mode)
        assert "\u22121" in result, f"Missing -1 button for {mode}"
        assert "+1" in result, f"Missing +1 button for {mode}"
        assert "P" in result, f"Missing P button for {mode}"

    def test_rotate_plus_one(self, page):
        """Clicking +1 on rotate should increment jiggleAngleSteps."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            jiggleAngleSteps = 100;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var plusBtn = null;
            buttons.forEach(b => { if (b.textContent === "+1") plusBtn = b; });
            plusBtn.click();
            return jiggleAngleSteps;
        }""")
        assert result == 101

    def test_rotate_minus_one(self, page):
        """Clicking -1 on rotate should decrement jiggleAngleSteps."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            jiggleAngleSteps = 100;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var minusBtn = null;
            buttons.forEach(b => { if (b.textContent === "\\u22121") minusBtn = b; });
            minusBtn.click();
            return jiggleAngleSteps;
        }""")
        assert result == 99

    def test_rotate_prime(self, page):
        """Clicking P on rotate should set jiggleAngleSteps to nearest prime."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            jiggleAngleSteps = 100;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var primeBtn = null;
            buttons.forEach(b => { if (b.textContent === "P") primeBtn = b; });
            primeBtn.click();
            return jiggleAngleSteps;
        }""")
        assert result == 101

    def test_rotate_prime_noop_on_prime(self, page):
        """P button on a prime value should keep it unchanged."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            jiggleAngleSteps = 97;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var primeBtn = null;
            buttons.forEach(b => { if (b.textContent === "P") primeBtn = b; });
            primeBtn.click();
            return jiggleAngleSteps;
        }""")
        assert result == 97

    def test_circle_plus_one(self, page):
        """Clicking +1 on circle should increment jiggleCircleSteps."""
        result = page.evaluate("""() => {
            jiggleMode = "circle";
            jiggleCircleSteps = 60;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var plusBtn = null;
            buttons.forEach(b => { if (b.textContent === "+1") plusBtn = b; });
            plusBtn.click();
            return jiggleCircleSteps;
        }""")
        assert result == 61

    def test_circle_prime(self, page):
        """Clicking P on circle should set jiggleCircleSteps to nearest prime."""
        result = page.evaluate("""() => {
            jiggleMode = "circle";
            jiggleCircleSteps = 60;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var primeBtn = null;
            buttons.forEach(b => { if (b.textContent === "P") primeBtn = b; });
            primeBtn.click();
            return jiggleCircleSteps;
        }""")
        assert result == 59 or result == 61

    def test_spiral_plus_one(self, page):
        """Clicking +1 on spiral-centroid should increment jiggleAngleSteps."""
        result = page.evaluate("""() => {
            jiggleMode = "spiral-centroid";
            jiggleAngleSteps = 200;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var plusBtn = null;
            buttons.forEach(b => { if (b.textContent === "+1") plusBtn = b; });
            plusBtn.click();
            return jiggleAngleSteps;
        }""")
        assert result == 201

    def test_lissajous_plus_one(self, page):
        """Clicking +1 on lissajous should increment jigglePeriod."""
        result = page.evaluate("""() => {
            jiggleMode = "lissajous";
            jigglePeriod = 50;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var plusBtn = null;
            buttons.forEach(b => { if (b.textContent === "+1") plusBtn = b; });
            plusBtn.click();
            return jigglePeriod;
        }""")
        assert result == 51

    def test_lissajous_prime(self, page):
        """Clicking P on lissajous should set jigglePeriod to nearest prime."""
        result = page.evaluate("""() => {
            jiggleMode = "lissajous";
            jigglePeriod = 50;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var primeBtn = null;
            buttons.forEach(b => { if (b.textContent === "P") primeBtn = b; });
            primeBtn.click();
            return jigglePeriod;
        }""")
        assert result == 47 or result == 53

    def test_minus_respects_minimum(self, page):
        """Clicking -1 should not go below the slider minimum."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            jiggleAngleSteps = 10;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var minusBtn = null;
            buttons.forEach(b => { if (b.textContent === "\\u22121") minusBtn = b; });
            minusBtn.click();
            return jiggleAngleSteps;
        }""")
        assert result == 10  # clamped at min=10

    def test_plus_respects_maximum(self, page):
        """Clicking +1 should not go above the slider maximum."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            jiggleAngleSteps = 5000;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var plusBtn = null;
            buttons.forEach(b => { if (b.textContent === "+1") plusBtn = b; });
            plusBtn.click();
            return jiggleAngleSteps;
        }""")
        assert result == 5000  # clamped at max=5000

    def test_breathe_period_plus_one(self, page):
        """Breathe period +1 increments jigglePeriod."""
        result = page.evaluate("""() => {
            jiggleMode = "breathe";
            jigglePeriod = 50;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var plusBtn = null;
            buttons.forEach(b => { if (b.textContent === "+1") plusBtn = b; });
            plusBtn.click();
            return jigglePeriod;
        }""")
        assert result == 51

    def test_breathe_period_prime(self, page):
        """Breathe period P sets jigglePeriod to nearest prime."""
        result = page.evaluate("""() => {
            jiggleMode = "breathe";
            jigglePeriod = 50;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var primeBtn = null;
            buttons.forEach(b => { if (b.textContent === "P") primeBtn = b; });
            primeBtn.click();
            return jigglePeriod;
        }""")
        assert result == 47 or result == 53

    def test_breathe_period_min_clamp(self, page):
        """Breathe period -1 should not go below 2."""
        result = page.evaluate("""() => {
            jiggleMode = "breathe";
            jigglePeriod = 2;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var minusBtn = null;
            buttons.forEach(b => { if (b.textContent === "\\u22121") minusBtn = b; });
            minusBtn.click();
            return jigglePeriod;
        }""")
        assert result == 2

    def test_slider_value_syncs_with_buttons(self, page):
        """After clicking a button, the slider/input value should match."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            jiggleAngleSteps = 100;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var slider = container.querySelector('input[type="range"]');
            var buttons = container.querySelectorAll("button");
            var plusBtn = null;
            buttons.forEach(b => { if (b.textContent === "+1") plusBtn = b; });
            plusBtn.click();
            return { variable: jiggleAngleSteps, sliderValue: parseInt(slider.value, 10) };
        }""")
        assert result["variable"] == 101
        assert result["sliderValue"] == 101

    def test_wobble_has_both_step_button_rows(self, page):
        """Wobble mode should have buttons for both steps and period."""
        result = page.evaluate("""() => {
            jiggleMode = "wobble";
            jiggleAngleSteps = 100;
            jigglePeriod = 20;
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var count = { minus: 0, plus: 0, prime: 0 };
            buttons.forEach(b => {
                if (b.textContent === "\\u22121") count.minus++;
                else if (b.textContent === "+1") count.plus++;
                else if (b.textContent === "P") count.prime++;
            });
            return count;
        }""")
        assert result["minus"] == 2, "Should have 2 minus buttons (steps + period)"
        assert result["plus"] == 2, "Should have 2 plus buttons (steps + period)"
        assert result["prime"] == 2, "Should have 2 prime buttons (steps + period)"

    def test_prime_button_title(self, page):
        """P buttons should have title='nearest prime'."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            buildJigglePanel();
            var container = document.getElementById("jiggle-controls");
            var buttons = container.querySelectorAll("button");
            var titles = [];
            buttons.forEach(b => { if (b.textContent === "P") titles.push(b.title); });
            return titles;
        }""")
        assert result == ["nearest prime"]
