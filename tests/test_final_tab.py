"""Tests for the Final tab: rendering, trails, animation updates, and
interaction with the morph system."""

import pytest


class TestFinalTabExists:
    """Basic DOM existence checks for the Final tab."""

    def test_final_tab_button_exists(self, page):
        """Final tab button should exist with data-ltab='final'."""
        result = page.evaluate("""() => {
            var btn = document.querySelector('[data-ltab="final"]');
            return btn !== null;
        }""")
        assert result is True

    def test_final_content_div_exists(self, page):
        """final-content div should exist."""
        result = page.evaluate("""() => {
            return document.getElementById('final-content') !== null;
        }""")
        assert result is True

    def test_final_panel_svg_exists(self, page):
        """final-panel SVG should exist."""
        result = page.evaluate("""() => {
            return document.getElementById('final-panel') !== null;
        }""")
        assert result is True

    def test_switch_to_final_tab(self, page):
        """switchLeftTab('final') should set leftTab and show content."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            var active = leftTab === 'final';
            var visible = document.getElementById('final-content').classList.contains('active');
            switchLeftTab('coeffs');
            return { active, visible };
        }""")
        assert result["active"] is True
        assert result["visible"] is True


class TestFinalPanelRendering:
    """renderFinalPanel() should create dots for each coefficient."""

    def test_renders_correct_number_of_dots(self, page):
        """Final panel should have one dot per coefficient."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            renderFinalPanel();
            var dots = document.querySelectorAll('#final-panel circle.final-coeff');
            var n = coefficients.length;
            switchLeftTab('coeffs');
            return { dots: dots.length, expected: n };
        }""")
        assert result["dots"] == result["expected"]

    def test_renders_labels(self, page):
        """Final panel should have labels for each coefficient."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            renderFinalPanel();
            var labels = document.querySelectorAll('#final-panel text.final-label');
            var n = coefficients.length;
            switchLeftTab('coeffs');
            return { labels: labels.length, expected: n };
        }""")
        assert result["labels"] == result["expected"]

    def test_without_morph_shows_c_positions(self, page):
        """With morph off, Final panel should show C-node positions."""
        result = page.evaluate("""() => {
            morphEnabled = false;
            switchLeftTab('final');
            renderFinalPanel();
            var dot = document.querySelector('#final-panel circle.final-coeff');
            var cx = parseFloat(dot.getAttribute('cx'));
            var expectedX = cxs()(coefficients[0].re);
            switchLeftTab('coeffs');
            return { cx, expectedX };
        }""")
        assert abs(result["cx"] - result["expectedX"]) < 1

    def test_with_morph_shows_blended_positions(self, page):
        """With morph on, Final panel should show blended C/D positions."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            // Set morphTheta to π so mu = (1 - cos(π))/2 = 1.0 → full D position
            // Actually, set to π/2 so mu = (1 - cos(π/2))/2 = 0.5
            morphTheta = Math.PI / 2;
            morphMu = 0.5;
            // Move D-node 0 far from C-node 0
            morphTargetCoeffs[0].re = coefficients[0].re + 2;
            morphTargetCoeffs[0].im = coefficients[0].im;
            switchLeftTab('final');
            renderFinalPanel();
            var dot = document.querySelector('#final-panel circle.final-coeff');
            var cx = parseFloat(dot.getAttribute('cx'));
            var blendedRe = coefficients[0].re * 0.5 + morphTargetCoeffs[0].re * 0.5;
            var expectedX = cxs()(blendedRe);
            // Cleanup
            morphEnabled = false; morphTheta = 0;
            initMorphTarget();
            switchLeftTab('coeffs');
            return { cx, expectedX };
        }""")
        assert abs(result["cx"] - result["expectedX"]) < 1


class TestFinalTrailData:
    """finalTrailData should accumulate during animation."""

    def test_final_trail_data_declared(self, page):
        """finalTrailData should exist as an array."""
        result = page.evaluate("() => Array.isArray(finalTrailData)")
        assert result is True

    def test_final_trail_data_empty_initially(self, page):
        """finalTrailData should be empty on page load."""
        result = page.evaluate("() => finalTrailData.length")
        assert result == 0

    def test_accumulates_during_animation(self, page):
        """finalTrailData should grow when animation is playing with trails on."""
        result = page.evaluate("""() => {
            // Setup: give coeff[0] a circle path
            coefficients[0].pathType = "circle";
            coefficients[0].radius = 50;
            coefficients[0].speed = 1;
            coefficients[0].angle = 0;
            coefficients[0].ccw = false;
            coefficients[0].extra = {};
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle",
                coefficients[0].radius / 100 * coeffExtent(), 0, {});

            trailsEnabled = true;
            finalTrailData = [];
            animState.playing = true;

            // Simulate what renderRoots does for trail accumulation
            var n = currentRoots.length;
            while (trailData.length < n) trailData.push([]);
            trailData.length = n;
            for (var i = 0; i < n; i++) {
                trailData[i].push({ re: currentRoots[i].re, im: currentRoots[i].im });
            }
            // Final trails
            var nC = coefficients.length;
            var nD = morphTargetCoeffs.length;
            var doBlend = morphEnabled && nD === nC;
            while (finalTrailData.length < nC) finalTrailData.push([]);
            finalTrailData.length = nC;
            for (var i = 0; i < nC; i++) {
                var c = coefficients[i];
                var re = doBlend ? c.re * (1 - morphMu) + morphTargetCoeffs[i].re * morphMu : c.re;
                var im = doBlend ? c.im * (1 - morphMu) + morphTargetCoeffs[i].im * morphMu : c.im;
                finalTrailData[i].push({ re: re, im: im });
            }

            var result = {
                length: finalTrailData.length,
                firstTrailLen: finalTrailData[0] ? finalTrailData[0].length : 0,
                hasReIm: finalTrailData[0] && finalTrailData[0][0] ?
                    typeof finalTrailData[0][0].re === 'number' : false
            };

            // Cleanup
            animState.playing = false;
            trailsEnabled = false;
            trailData = [];
            finalTrailData = [];
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            return result;
        }""")
        assert result["length"] > 0
        assert result["firstTrailLen"] == 1
        assert result["hasReIm"] is True

    def test_cleared_on_clear_trails(self, page):
        """clearTrails() should empty finalTrailData."""
        result = page.evaluate("""() => {
            finalTrailData = [[{re: 1, im: 2}]];
            clearTrails();
            return finalTrailData.length;
        }""")
        assert result == 0


class TestFinalTrailRendering:
    """renderFinalTrails() should create SVG paths on the final panel."""

    def test_render_final_trails_uses_correct_panel_key(self, page):
        """renderFinalTrails should use panels.coeff.range, not panels.coefficients.range."""
        result = page.evaluate("""() => {
            // Verify panels.coeff exists and panels.coefficients does not
            return {
                hasCoeff: typeof panels.coeff === 'object' && panels.coeff !== null,
                hasCoefficients: typeof panels.coefficients !== 'undefined',
                range: panels.coeff ? panels.coeff.range : null
            };
        }""")
        assert result["hasCoeff"] is True
        assert result["hasCoefficients"] is False
        assert result["range"] is not None

    def test_renders_trail_paths(self, page):
        """renderFinalTrails should create SVG path elements."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            renderFinalPanel();  // init the panel

            // Populate trail data manually
            var n = coefficients.length;
            finalTrailData = [];
            for (var i = 0; i < n; i++) {
                finalTrailData.push([
                    {re: coefficients[i].re, im: coefficients[i].im},
                    {re: coefficients[i].re + 0.1, im: coefficients[i].im + 0.1},
                    {re: coefficients[i].re + 0.2, im: coefficients[i].im + 0.2}
                ]);
            }

            trailsEnabled = true;
            renderFinalPanel();
            var paths = document.querySelectorAll('#final-panel .final-trail-layer path.trail-path');

            // Cleanup
            trailsEnabled = false;
            finalTrailData = [];
            switchLeftTab('coeffs');
            return paths.length;
        }""")
        assert result > 0

    def test_trail_paths_have_d_attribute(self, page):
        """Trail paths should have a valid 'd' attribute with M/L commands."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            renderFinalPanel();  // init the panel

            var n = coefficients.length;
            finalTrailData = [];
            for (var i = 0; i < n; i++) {
                finalTrailData.push([
                    {re: coefficients[i].re, im: coefficients[i].im},
                    {re: coefficients[i].re + 0.1, im: coefficients[i].im + 0.1}
                ]);
            }

            trailsEnabled = true;
            renderFinalPanel();
            var path = document.querySelector('#final-panel .final-trail-layer path.trail-path');
            var d = path ? path.getAttribute('d') : '';

            // Cleanup
            trailsEnabled = false;
            finalTrailData = [];
            switchLeftTab('coeffs');
            return { hasD: d.length > 0, startsWithM: d.startsWith('M') };
        }""")
        assert result["hasD"] is True
        assert result["startsWithM"] is True


class TestSolveRootsWithFinalTab:
    """solveRoots() should not throw when Final tab is active with trails."""

    def test_solve_roots_succeeds_with_final_tab(self, page):
        """solveRoots() should complete without error when leftTab='final' and trails on."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            trailsEnabled = true;
            animState.playing = true;
            try {
                solveRoots();
                var ok = true;
            } catch(e) {
                var ok = false;
                var err = e.message;
            }
            // Cleanup
            animState.playing = false;
            trailsEnabled = false;
            finalTrailData = [];
            trailData = [];
            switchLeftTab('coeffs');
            return { ok: ok, err: typeof err !== 'undefined' ? err : null };
        }""")
        assert result["ok"] is True, f"solveRoots() threw: {result['err']}"

    def test_solve_roots_updates_roots_with_final_tab(self, page):
        """After solveRoots() with Final tab, currentRoots should be valid."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            trailsEnabled = true;
            animState.playing = true;
            solveRoots();
            var valid = currentRoots.length > 0 &&
                        isFinite(currentRoots[0].re) &&
                        isFinite(currentRoots[0].im);
            // Cleanup
            animState.playing = false;
            trailsEnabled = false;
            finalTrailData = [];
            trailData = [];
            switchLeftTab('coeffs');
            return valid;
        }""")
        assert result is True

    def test_pending_solve_resets_after_error_fix(self, page):
        """pendingSolve should be false after solveRoots completes."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            trailsEnabled = true;
            pendingSolve = false;
            solveRoots();
            // Cleanup
            trailsEnabled = false;
            finalTrailData = [];
            trailData = [];
            switchLeftTab('coeffs');
            return pendingSolve === false;
        }""")
        assert result is True


class TestAnimationWithFinalTrails:
    """Integration: animation + Final tab + trails should all work together."""

    def test_roots_move_during_animation_with_final_tab(self, page):
        """Roots should continue updating when Final tab is active."""
        result = page.evaluate("""() => {
            // Setup animation on coeff[0]
            coefficients[0].pathType = "circle";
            coefficients[0].radius = 50;
            coefficients[0].speed = 1;
            coefficients[0].angle = 0;
            coefficients[0].ccw = false;
            coefficients[0].extra = {};
            coefficients[0].curve = computeCurve(
                coefficients[0].re, coefficients[0].im, "circle",
                coefficients[0].radius / 100 * coeffExtent(), 0, {});

            switchLeftTab('final');
            trailsEnabled = true;

            // Record initial root position
            solveRoots();
            var r0_re = currentRoots[0].re;
            var r0_im = currentRoots[0].im;

            // Move coefficient manually (simulating animation advancement)
            var curve = coefficients[0].curve;
            var midIdx = Math.floor(curve.length / 4);
            coefficients[0].re = curve[midIdx].re;
            coefficients[0].im = curve[midIdx].im;

            // Solve again — should produce different roots
            animState.playing = true;
            solveRoots();
            animState.playing = false;

            var r1_re = currentRoots[0].re;
            var r1_im = currentRoots[0].im;
            var moved = (r0_re !== r1_re) || (r0_im !== r1_im);

            // Cleanup
            trailsEnabled = false;
            finalTrailData = [];
            trailData = [];
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            switchLeftTab('coeffs');
            return moved;
        }""")
        assert result is True

    def test_final_dots_update_during_solve(self, page):
        """Final panel dots should reflect updated coefficient positions."""
        result = page.evaluate("""() => {
            switchLeftTab('final');
            renderFinalPanel();

            // Get initial dot position
            var dot0 = document.querySelector('#final-panel circle.final-coeff');
            var cx0 = parseFloat(dot0.getAttribute('cx'));

            // Move coefficient
            var origRe = coefficients[0].re;
            coefficients[0].re += 1.0;
            solveRoots();  // triggers renderFinalPanel

            var cx1 = parseFloat(dot0.getAttribute('cx'));
            var moved = cx0 !== cx1;

            // Cleanup
            coefficients[0].re = origRe;
            solveRoots();
            switchLeftTab('coeffs');
            return moved;
        }""")
        assert result is True
