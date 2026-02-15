"""Tests for animation entry points: startAnimation, home button, scrub slider,
play guard with D-nodes, and animation state management."""

import pytest


class TestAnimationState:
    """Basic animation state management."""

    def test_initial_state_not_playing(self, page):
        """Animation should not be playing on page load."""
        result = page.evaluate("() => animState.playing")
        assert result is False

    def test_play_button_exists(self, page):
        """Play button should exist in DOM."""
        result = page.evaluate("""() => {
            var btn = document.getElementById('play-btn');
            return btn !== null;
        }""")
        assert result is True

    def test_home_button_exists(self, page):
        """Home button should exist in DOM."""
        result = page.evaluate("""() => {
            var btn = document.getElementById('home-btn');
            return btn !== null;
        }""")
        assert result is True

    def test_scrub_slider_exists(self, page):
        """Scrub slider should exist in DOM."""
        result = page.evaluate("""() => {
            var slider = document.getElementById('scrub-slider');
            return { exists: slider !== null, min: slider ? slider.min : null, max: slider ? slider.max : null };
        }""")
        assert result["exists"] is True
        assert result["min"] == "0"
        assert result["max"] == "5000"


class TestStartAnimationGuard:
    """startAnimation() should check for animated coefficients before starting."""

    def test_no_animated_no_start(self, page):
        """If no C or D nodes are animated, startAnimation should not start playing."""
        result = page.evaluate("""() => {
            // Ensure all paths are "none"
            for (var c of coefficients) c.pathType = "none";
            initMorphTarget();
            animState.playing = false;
            startAnimation();
            var playing = animState.playing;
            stopAnimation();
            return playing;
        }""")
        # With no animated coefficients, startAnimation should not start
        assert result is False

    def test_animated_c_allows_start(self, page):
        """If C nodes have paths, startAnimation should allow start."""
        result = page.evaluate("""() => {
            coefficients[0].pathType = "circle";
            coefficients[0].radius = 50;
            coefficients[0].speed = 1;
            coefficients[0].angle = 0;
            coefficients[0].ccw = false;
            coefficients[0].extra = {};
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle",
                coefficients[0].radius / 100 * coeffExtent(), 0, {});

            animState.playing = false;
            startAnimation();
            var playing = animState.playing;
            stopAnimation();

            // Restore
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            return playing;
        }""")
        assert result is True

    def test_animated_d_allows_start(self, page):
        """If D nodes have paths (but C nodes don't), startAnimation should still allow start."""
        result = page.evaluate("""() => {
            // Ensure all C are "none"
            for (var c of coefficients) {
                c.pathType = "none";
                c.curve = [{re: c.re, im: c.im}];
            }
            // But D has a path
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[0].radius = 50;
            morphTargetCoeffs[0].speed = 1;
            morphTargetCoeffs[0].angle = 0;
            morphTargetCoeffs[0].ccw = false;
            morphTargetCoeffs[0].extra = {};
            morphTargetCoeffs[0].curve = computeCurve(
                morphTargetCoeffs[0].re, morphTargetCoeffs[0].im, "circle",
                morphTargetCoeffs[0].radius / 100 * coeffExtent(), 0, {});

            animState.playing = false;
            startAnimation();
            var playing = animState.playing;
            stopAnimation();
            initMorphTarget();
            return playing;
        }""")
        assert result is True


class TestHomeButton:
    """Home button should reset C and D nodes to curve[0]."""

    def test_home_resets_c_to_curve_start(self, page):
        """Home button resets C coefficients to curve[0]."""
        result = page.evaluate("""() => {
            var c = coefficients[0];
            c.pathType = "circle";
            c.radius = 50;
            c.speed = 1;
            c.angle = 0;
            c.ccw = false;
            c.extra = {};
            c.curve = computeCurve(c.re, c.im, "circle", c.radius / 100 * coeffExtent(), 0, {});
            var homeRe = c.curve[0].re;
            var homeIm = c.curve[0].im;

            // Move it away
            c.re = 999;
            c.im = 888;
            c.curveIndex = 50;

            // Trigger home
            document.getElementById('home-btn').click();

            var afterRe = c.re;
            var afterIm = c.im;

            // Restore
            c.pathType = "none";
            c.curve = [{re: c.re, im: c.im}];
            return { homeRe, homeIm, afterRe, afterIm };
        }""")
        assert abs(result["afterRe"] - result["homeRe"]) < 1e-10
        assert abs(result["afterIm"] - result["homeIm"]) < 1e-10

    def test_home_resets_d_to_curve_start(self, page):
        """Home button resets D-nodes to curve[0]."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var d = morphTargetCoeffs[0];
            d.pathType = "circle";
            d.radius = 50;
            d.speed = 1;
            d.angle = 0;
            d.ccw = false;
            d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), 0, {});
            var homeRe = d.curve[0].re;
            var homeIm = d.curve[0].im;

            // Move it away
            d.re = 999;
            d.im = 888;
            d.curveIndex = 50;

            // Trigger home
            document.getElementById('home-btn').click();

            var afterRe = d.re;
            var afterIm = d.im;

            initMorphTarget();
            return { homeRe, homeIm, afterRe, afterIm };
        }""")
        assert abs(result["afterRe"] - result["homeRe"]) < 1e-10
        assert abs(result["afterIm"] - result["homeIm"]) < 1e-10

    def test_home_resets_curve_index(self, page):
        """Home button should reset curveIndex to 0 for animated D-nodes."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var d = morphTargetCoeffs[0];
            d.pathType = "circle";
            d.radius = 50;
            d.speed = 1;
            d.angle = 0;
            d.ccw = false;
            d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), 0, {});
            d.curveIndex = 50;

            document.getElementById('home-btn').click();

            var idx = d.curveIndex;
            initMorphTarget();
            return idx;
        }""")
        assert result == 0


class TestScrubSlider:
    """Scrub slider should advance C and D nodes along their paths."""

    def test_scrub_advances_c(self, page):
        """Scrubbing should move animated C coefficients."""
        result = page.evaluate("""() => {
            var c = coefficients[0];
            c.pathType = "circle";
            c.radius = 50;
            c.speed = 1;
            c.angle = 0;
            c.ccw = false;
            c.extra = {};
            c.curve = computeCurve(c.re, c.im, "circle", c.radius / 100 * coeffExtent(), 0, {});
            var homeRe = c.curve[0].re;

            // Scrub to 1.25 seconds (avoid integer which wraps back to start)
            var slider = document.getElementById('scrub-slider');
            slider.value = 1250;
            slider.dispatchEvent(new Event('input'));

            var afterRe = c.re;

            // Restore
            slider.value = 0;
            slider.dispatchEvent(new Event('input'));
            c.pathType = "none";
            c.curve = [{re: c.re, im: c.im}];
            return { homeRe, afterRe, moved: homeRe !== afterRe };
        }""")
        assert result["moved"] is True

    def test_scrub_advances_d(self, page):
        """Scrubbing should move animated D-nodes."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var d = morphTargetCoeffs[0];
            d.pathType = "circle";
            d.radius = 50;
            d.speed = 1;
            d.angle = 0;
            d.ccw = false;
            d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), 0, {});
            var homeRe = d.curve[0].re;

            // Scrub to 1.25 seconds (avoid integer which wraps back to start)
            var slider = document.getElementById('scrub-slider');
            slider.value = 1250;
            slider.dispatchEvent(new Event('input'));

            var afterRe = d.re;

            // Restore
            slider.value = 0;
            slider.dispatchEvent(new Event('input'));
            initMorphTarget();
            return { homeRe, afterRe, moved: homeRe !== afterRe };
        }""")
        assert result["moved"] is True

    def test_scrub_zero_returns_to_start(self, page):
        """Scrubbing to 0 should put nodes at curve[0]."""
        result = page.evaluate("""() => {
            var c = coefficients[0];
            c.pathType = "circle";
            c.radius = 50;
            c.speed = 1;
            c.angle = 0;
            c.ccw = false;
            c.extra = {};
            c.curve = computeCurve(c.re, c.im, "circle", c.radius / 100 * coeffExtent(), 0, {});
            var homeRe = c.curve[0].re;
            var homeIm = c.curve[0].im;

            // Scrub to 0
            var slider = document.getElementById('scrub-slider');
            slider.value = 0;
            slider.dispatchEvent(new Event('input'));

            var atZeroRe = c.re;
            var atZeroIm = c.im;

            // Restore
            c.pathType = "none";
            c.curve = [{re: c.re, im: c.im}];
            return { homeRe, homeIm, atZeroRe, atZeroIm };
        }""")
        assert abs(result["atZeroRe"] - result["homeRe"]) < 1e-6
        assert abs(result["atZeroIm"] - result["homeIm"]) < 1e-6

    def test_scrub_value_label_updates(self, page):
        """Scrub value label should update when slider changes."""
        result = page.evaluate("""() => {
            var slider = document.getElementById('scrub-slider');
            slider.value = 2500;
            slider.dispatchEvent(new Event('input'));
            var label = document.getElementById('scrub-val').textContent;
            slider.value = 0;
            slider.dispatchEvent(new Event('input'));
            return label;
        }""")
        # 2500 / 1000 = 2.5s
        assert "2.50" in result


class TestStopAnimation:
    def test_stop_sets_not_playing(self, page):
        """stopAnimation() should set playing to false."""
        result = page.evaluate("""() => {
            animState.playing = true;
            stopAnimation();
            return animState.playing;
        }""")
        assert result is False


class TestAllAnimatedCoeffs:
    """allAnimatedCoeffs() for C coefficients."""

    def test_returns_empty_when_no_paths(self, page):
        """When all C are 'none', allAnimatedCoeffs returns empty set."""
        result = page.evaluate("""() => {
            for (var c of coefficients) c.pathType = "none";
            var s = allAnimatedCoeffs();
            return s.size;
        }""")
        assert result == 0

    def test_returns_animated_indices(self, page):
        """allAnimatedCoeffs returns Set of animated C coefficient indices."""
        result = page.evaluate("""() => {
            coefficients[0].pathType = "circle";
            var s = allAnimatedCoeffs();
            var hasZero = s.has(0);
            coefficients[0].pathType = "none";
            return hasZero;
        }""")
        assert result is True
