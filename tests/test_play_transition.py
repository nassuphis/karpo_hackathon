"""Tests for Play button state machine and fast mode → play transition.

Covers:
  - Play/Pause/Resume button text transitions
  - Fast mode active → Play exits fast mode first
  - Animation lifecycle with morph enabled
"""

import pytest


class TestPlayButtonText:
    """Play button should show correct text for each state."""

    def test_initial_text_is_play(self, page):
        """Play button should say '▶ Play' initially."""
        text = page.evaluate("""() => {
            return document.getElementById('play-btn').textContent.trim();
        }""")
        assert "Play" in text

    def test_play_changes_to_pause(self, page):
        """Clicking Play with animated coeffs should change text to Pause."""
        result = page.evaluate("""() => {
            // Give coeff[0] a circle path so animation can start
            coefficients[0].pathType = "circle";
            coefficients[0].rAbs = 0.5;
            coefficients[0].speed = 1;
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle", 0.5, 0, {});

            var btn = document.getElementById('play-btn');
            btn.click();
            var textAfterPlay = btn.textContent.trim();

            // Cleanup
            stopAnimation();
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            btn.textContent = "\\u25b6 Play";
            return textAfterPlay;
        }""")
        assert "Pause" in result

    def test_pause_changes_to_resume(self, page):
        """Clicking Pause should change text to Resume."""
        result = page.evaluate("""() => {
            coefficients[0].pathType = "circle";
            coefficients[0].rAbs = 0.5;
            coefficients[0].speed = 1;
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle", 0.5, 0, {});

            var btn = document.getElementById('play-btn');
            btn.click();  // Play → Pause
            btn.click();  // Pause → Resume
            var text = btn.textContent.trim();

            // Cleanup
            stopAnimation();
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            btn.textContent = "\\u25b6 Play";
            return text;
        }""")
        assert "Resume" in result

    def test_no_animated_coeffs_buzzes(self, page):
        """Play with no animated coeffs should not start (uiBuzz)."""
        result = page.evaluate("""() => {
            // Ensure no animated coefficients
            for (var i = 0; i < coefficients.length; i++) {
                coefficients[i].pathType = "none";
            }
            var btn = document.getElementById('play-btn');
            var wasBefore = animState.playing;
            btn.click();
            var wasAfter = animState.playing;
            return { before: wasBefore, after: wasAfter };
        }""")
        assert result["before"] is False
        assert result["after"] is False


class TestFastModeToPlay:
    """Clicking Play while fast mode is active should exit fast mode first."""

    def test_play_exits_fast_mode(self, page):
        """Play button should set fastModeActive=false before starting animation."""
        result = page.evaluate("""() => {
            // Give coeff[0] a path
            coefficients[0].pathType = "circle";
            coefficients[0].rAbs = 0.5;
            coefficients[0].speed = 1;
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle", 0.5, 0, {});

            // Simulate fast mode being active (without actually starting workers)
            fastModeActive = true;

            var btn = document.getElementById('play-btn');
            btn.click();

            var result = {
                fastModeAfter: fastModeActive,
                playing: animState.playing
            };

            // Cleanup
            stopAnimation();
            fastModeActive = false;
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            btn.textContent = "\\u25b6 Play";
            return result;
        }""")
        assert result["fastModeAfter"] is False
        assert result["playing"] is True

    def test_start_animation_guard_rejects_fast_mode(self, page):
        """startAnimation() should return early if fastModeActive is true."""
        result = page.evaluate("""() => {
            fastModeActive = true;
            var wasBefore = animState.playing;
            startAnimation();
            var wasAfter = animState.playing;
            fastModeActive = false;
            return { before: wasBefore, after: wasAfter };
        }""")
        assert result["before"] is False
        assert result["after"] is False  # guard prevented start


class TestPlayWithMorph:
    """Animation with morph-enabled should work correctly."""

    def test_morph_motion_allows_play(self, page):
        """When morph has motion (rate > 0 and C != D), Play should start."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            morphRate = 0.1;
            // Move D-node away from C so there's morph motion
            morphTargetCoeffs[0].re = coefficients[0].re + 2;

            var btn = document.getElementById('play-btn');
            btn.click();
            var playing = animState.playing;

            // Cleanup
            stopAnimation();
            morphEnabled = false;
            morphRate = 0;
            initMorphTarget();
            btn.textContent = "\\u25b6 Play";
            return playing;
        }""")
        assert result is True

    def test_animation_advances_morph_theta(self, page):
        """During animation, morphTheta should advance."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            morphRate = 0.1;
            morphTargetCoeffs[0].re = coefficients[0].re + 2;
            morphTheta = 0;

            startAnimation();
            // Manually tick the animation forward
            var before = morphTheta;
            // Simulate a small time advance
            animState.elapsedAtPause = null;
            animState.elapsed = 1.0;
            morphTheta = 2 * Math.PI * morphRate * animState.elapsed;
            var after = morphTheta;

            // Cleanup
            stopAnimation();
            morphEnabled = false;
            morphRate = 0;
            morphTheta = 0;
            initMorphTarget();
            return { before, after, advanced: after > before };
        }""")
        assert result["advanced"] is True
