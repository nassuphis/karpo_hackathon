"""Tests for CTrail integration with the Final tab.

The CTrail button (coeffDragTrailsEnabled) should also trigger final coefficient
trail accumulation and rendering on the Final tab, not just coefficient drag
trails on the coeff panel.
"""

import pytest


class TestCTrailToggle:
    """CTrail button toggles coeffDragTrailsEnabled."""

    def test_ctrail_starts_disabled(self, page):
        result = page.evaluate("() => coeffDragTrailsEnabled")
        assert result is False

    def test_ctrail_toggle_on(self, page):
        result = page.evaluate("""() => {
            var btn = document.getElementById('coeff-drag-trail-btn');
            btn.click();
            var val = coeffDragTrailsEnabled;
            btn.click();  // toggle back off
            return val;
        }""")
        assert result is True

    def test_ctrail_toggle_clears_final_trail_data(self, page):
        """Toggling CTrail should clear finalTrailData."""
        result = page.evaluate("""() => {
            // Populate some fake final trail data
            finalTrailData = [[{re: 1, im: 2}], [{re: 3, im: 4}]];
            var btn = document.getElementById('coeff-drag-trail-btn');
            btn.click();  // toggle ON — should clear
            var lenAfterOn = finalTrailData.length;
            btn.click();  // toggle OFF
            return lenAfterOn;
        }""")
        assert result == 0


class TestFinalTrailAccumulationWithCTrail:
    """finalTrailData should accumulate during animation when CTrail is on."""

    def test_ctrail_enables_final_trail_accumulation(self, page):
        """With CTrail on (not Trails), finalTrailData should accumulate in solveRoots."""
        result = page.evaluate("""() => {
            // Setup: animated coeff
            coefficients[0].pathType = "circle";
            coefficients[0].rAbs = 0.5;
            coefficients[0].speed = 1;
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle", 0.5, 0, {});

            // CTrail on, Trails off
            coeffDragTrailsEnabled = true;
            trailsEnabled = false;
            animState.playing = true;
            finalTrailData = [];

            // Solve a few times (simulating animation frames)
            for (var i = 0; i < 5; i++) {
                solveRoots();
            }

            var accumulated = finalTrailData.length > 0;
            var pointCount = finalTrailData[0] ? finalTrailData[0].length : 0;

            // Cleanup
            coeffDragTrailsEnabled = false;
            animState.playing = false;
            finalTrailData = [];
            trailData = [];
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            return { accumulated, pointCount };
        }""")
        assert result["accumulated"] is True
        assert result["pointCount"] == 5

    def test_trails_off_ctrail_off_no_accumulation(self, page):
        """With both Trails and CTrail off, no finalTrailData should accumulate."""
        result = page.evaluate("""() => {
            coefficients[0].pathType = "circle";
            coefficients[0].rAbs = 0.5;
            coefficients[0].speed = 1;
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle", 0.5, 0, {});

            coeffDragTrailsEnabled = false;
            trailsEnabled = false;
            animState.playing = true;
            finalTrailData = [];

            for (var i = 0; i < 5; i++) solveRoots();

            var result = finalTrailData.length;

            animState.playing = false;
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            return result;
        }""")
        assert result == 0

    def test_root_trails_not_affected_by_ctrail(self, page):
        """CTrail should NOT cause root trailData to accumulate (only Trails does)."""
        result = page.evaluate("""() => {
            coefficients[0].pathType = "circle";
            coefficients[0].rAbs = 0.5;
            coefficients[0].speed = 1;
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle", 0.5, 0, {});

            coeffDragTrailsEnabled = true;
            trailsEnabled = false;
            animState.playing = true;
            trailData = [];

            for (var i = 0; i < 5; i++) solveRoots();

            var rootTrailLen = trailData.length > 0 ? trailData[0].length : 0;

            // Cleanup
            coeffDragTrailsEnabled = false;
            animState.playing = false;
            trailData = [];
            finalTrailData = [];
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            return rootTrailLen;
        }""")
        assert result == 0  # root trails should NOT accumulate from CTrail alone


class TestFinalPanelRenderingWithCTrail:
    """Final panel should render trails when CTrail is on."""

    def test_final_panel_shows_trails_with_ctrail(self, page):
        """renderFinalPanel should render trails when coeffDragTrailsEnabled."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            renderFinalPanel();

            // Populate trail data
            var n = coefficients.length;
            finalTrailData = [];
            for (var i = 0; i < n; i++) {
                finalTrailData.push([
                    {re: coefficients[i].re, im: coefficients[i].im},
                    {re: coefficients[i].re + 0.1, im: coefficients[i].im + 0.1}
                ]);
            }

            // CTrail on, Trails off
            coeffDragTrailsEnabled = true;
            trailsEnabled = false;
            renderFinalPanel();

            var paths = document.querySelectorAll('#final-panel .final-trail-layer path.trail-path');
            var count = paths.length;

            // Cleanup
            coeffDragTrailsEnabled = false;
            finalTrailData = [];
            switchLeftTab('coeffs');
            return count;
        }""")
        assert result > 0

    def test_final_panel_hides_trails_when_both_off(self, page):
        """No trails rendered when both trailsEnabled and coeffDragTrailsEnabled are off."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            renderFinalPanel();

            var n = coefficients.length;
            finalTrailData = [];
            for (var i = 0; i < n; i++) {
                finalTrailData.push([
                    {re: coefficients[i].re, im: coefficients[i].im},
                    {re: coefficients[i].re + 0.1, im: coefficients[i].im + 0.1}
                ]);
            }

            coeffDragTrailsEnabled = false;
            trailsEnabled = false;
            renderFinalPanel();

            // renderFinalTrails should NOT be called, so trail SVGs from
            // a previous render should not update. But since both are off,
            // the condition skips rendering entirely.
            // Check that the condition works by verifying no NEW paths created.
            var layer = document.querySelector('#final-panel .final-trail-layer');
            // Clear any old paths manually for this test
            if (layer) layer.innerHTML = '';
            renderFinalPanel();
            var paths = layer ? layer.querySelectorAll('path.trail-path') : [];

            // Cleanup
            finalTrailData = [];
            switchLeftTab('coeffs');
            return paths.length;
        }""")
        assert result == 0


class TestCTrailWithMorph:
    """CTrail + morph should show blended coefficient trails on Final tab."""

    def test_morph_blended_trails_recorded(self, page):
        """With morph enabled, finalTrailData should contain blended positions."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            morphPathType = "line";
            morphTheta = Math.PI / 2;  // mu = 0.5

            // Move D-node 0 away from C-node 0
            var origDRe = morphTargetCoeffs[0].re;
            morphTargetCoeffs[0].re = coefficients[0].re + 2;

            coeffDragTrailsEnabled = true;
            trailsEnabled = false;
            animState.playing = true;
            finalTrailData = [];

            solveRoots();

            // The final trail position should be the blend of C and D
            var trailPt = finalTrailData.length > 0 && finalTrailData[0].length > 0
                ? finalTrailData[0][0] : null;
            var expectedRe = coefficients[0].re * 0.5 + morphTargetCoeffs[0].re * 0.5;
            var close = trailPt ? Math.abs(trailPt.re - expectedRe) < 0.1 : false;

            // Cleanup
            morphEnabled = false;
            morphTheta = 0;
            morphPathType = "c-node";
            morphTargetCoeffs[0].re = origDRe;
            initMorphTarget();
            coeffDragTrailsEnabled = false;
            animState.playing = false;
            finalTrailData = [];
            return { hasTrail: trailPt !== null, close };
        }""")
        assert result["hasTrail"] is True
        assert result["close"] is True
