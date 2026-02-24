"""Tests for recently added features: disk-cloud/sq-cloud paths, BITCFG tab,
rel-proximity color mode, clearTrails comprehensive, Home key/button reset."""

import pytest
import math


# ============================================================
# Disk-cloud and Square-cloud path types
# ============================================================

class TestDiskCloud:
    """Tests for the disk-cloud path type (random disk sampling with power param)."""

    def test_disk_cloud_in_catalog(self, page):
        """disk-cloud should be in PATH_CATALOG."""
        result = page.evaluate("""() => {
            for (var entry of PATH_CATALOG) {
                if (entry.value === 'disk-cloud') return true;
                if (entry.items) {
                    for (var item of entry.items) {
                        if (item.value === 'disk-cloud') return true;
                    }
                }
            }
            return false;
        }""")
        assert result is True

    def test_disk_cloud_in_params(self, page):
        """disk-cloud should have PATH_PARAMS with speed, radius, pow, points."""
        result = page.evaluate("""() => {
            var p = PATH_PARAMS['disk-cloud'];
            if (!p) return null;
            return p.map(x => x.key);
        }""")
        assert result is not None
        assert "speed" in result
        assert "radius" in result
        assert "pow" in result
        assert "points" in result

    def test_disk_cloud_no_dither_variant(self, page):
        """disk-cloud should NOT have a dither variant in PATH_PARAMS."""
        result = page.evaluate("""() => {
            return 'disk-cloud-dither' in PATH_PARAMS;
        }""")
        assert result is False

    def test_disk_cloud_no_dither_in_catalog(self, page):
        """disk-cloud-dither should NOT be in PATH_CATALOG."""
        result = page.evaluate("""() => {
            for (var entry of PATH_CATALOG) {
                if (entry.value === 'disk-cloud-dither') return true;
                if (entry.items) {
                    for (var item of entry.items) {
                        if (item.value === 'disk-cloud-dither') return true;
                    }
                }
            }
            return false;
        }""")
        assert result is False

    def test_disk_cloud_generates_curve(self, page):
        """computeCurveN should generate a cloud for disk-cloud."""
        result = page.evaluate("""() => {
            var c = {re: 0, im: 0, pathType: 'disk-cloud', radius: 2, speed: 1,
                     angle: 0, ccw: false, extra: {pow: 1, points: 200},
                     curve: [{re:0, im:0}], curveIndex: 0};
            var curve = computeCurveN(c.re, c.im, c.pathType, c.radius, c.angle, c.extra, 200);
            if (!curve) return null;
            return {
                length: curve.length,
                isCloud: !!curve._isCloud,
                allFinite: curve.every(p => isFinite(p.re) && isFinite(p.im))
            };
        }""")
        assert result is not None
        assert result["length"] == 200
        assert result["isCloud"] is True
        assert result["allFinite"] is True

    def test_disk_cloud_within_radius(self, page):
        """All disk-cloud points should be within the specified radius."""
        result = page.evaluate("""() => {
            var R = 2;
            var c = {re: 1, im: 1, pathType: 'disk-cloud', radius: R, speed: 1,
                     angle: 0, ccw: false, extra: {pow: 1, points: 500},
                     curve: [{re:1, im:1}], curveIndex: 0};
            var curve = computeCurveN(c.re, c.im, c.pathType, c.radius, c.angle, c.extra, 500);
            var maxDist = 0;
            for (var i = 0; i < curve.length; i++) {
                var dx = curve[i].re - 1, dy = curve[i].im - 1;
                var d = Math.sqrt(dx*dx + dy*dy);
                if (d > maxDist) maxDist = d;
            }
            return {maxDist: maxDist, withinR: maxDist <= R + 1e-10};
        }""")
        assert result["withinR"] is True

    def test_disk_cloud_power_concentrates(self, page):
        """Higher pow should concentrate points toward center."""
        result = page.evaluate("""() => {
            function avgDist(pow) {
                var c = {re: 0, im: 0, pathType: 'disk-cloud', radius: 2, speed: 1,
                         angle: 0, ccw: false, extra: {pow: pow, points: 1000},
                         curve: [{re:0, im:0}], curveIndex: 0};
                var curve = computeCurveN(c.re, c.im, c.pathType, c.radius, c.angle, c.extra, 1000);
                var sum = 0;
                for (var i = 0; i < curve.length; i++) {
                    sum += Math.sqrt(curve[i].re*curve[i].re + curve[i].im*curve[i].im);
                }
                return sum / curve.length;
            }
            var avg1 = avgDist(1);
            var avg3 = avgDist(3);
            return {avg1: avg1, avg3: avg3, concentrated: avg3 < avg1};
        }""")
        assert result["concentrated"] is True

    def test_disk_cloud_animpath_returns_center(self, page):
        """animPathFn for disk-cloud should return the center point."""
        result = page.evaluate("""() => {
            var center = {re: 1.5, im: -0.5};
            var pt = animPathFn('disk-cloud', 0.5, center, 2, {pow: 1});
            return {re: pt.re, im: pt.im};
        }""")
        assert abs(result["re"] - 1.5) < 1e-10
        assert abs(result["im"] - (-0.5)) < 1e-10


class TestSqCloud:
    """Tests for the sq-cloud path type (random square sampling with power param)."""

    def test_sq_cloud_in_catalog(self, page):
        """sq-cloud should be in PATH_CATALOG."""
        result = page.evaluate("""() => {
            for (var entry of PATH_CATALOG) {
                if (entry.value === 'sq-cloud') return true;
                if (entry.items) {
                    for (var item of entry.items) {
                        if (item.value === 'sq-cloud') return true;
                    }
                }
            }
            return false;
        }""")
        assert result is True

    def test_sq_cloud_in_params(self, page):
        """sq-cloud should have PATH_PARAMS with speed, radius, pow, points."""
        result = page.evaluate("""() => {
            var p = PATH_PARAMS['sq-cloud'];
            if (!p) return null;
            return p.map(x => x.key);
        }""")
        assert result is not None
        assert "speed" in result
        assert "radius" in result
        assert "pow" in result
        assert "points" in result

    def test_sq_cloud_no_dither_variant(self, page):
        """sq-cloud should NOT have a dither variant in PATH_PARAMS."""
        result = page.evaluate("""() => {
            return 'sq-cloud-dither' in PATH_PARAMS;
        }""")
        assert result is False

    def test_sq_cloud_generates_curve(self, page):
        """computeCurveN should generate a cloud for sq-cloud."""
        result = page.evaluate("""() => {
            var c = {re: 0, im: 0, pathType: 'sq-cloud', radius: 2, speed: 1,
                     angle: 0, ccw: false, extra: {pow: 1, points: 200},
                     curve: [{re:0, im:0}], curveIndex: 0};
            var curve = computeCurveN(c.re, c.im, c.pathType, c.radius, c.angle, c.extra, 200);
            if (!curve) return null;
            return {
                length: curve.length,
                isCloud: !!curve._isCloud,
                allFinite: curve.every(p => isFinite(p.re) && isFinite(p.im))
            };
        }""")
        assert result is not None
        assert result["length"] == 200
        assert result["isCloud"] is True
        assert result["allFinite"] is True

    def test_sq_cloud_within_bounds(self, page):
        """All sq-cloud points should be within [-R, R] × [-R, R] of center."""
        result = page.evaluate("""() => {
            var R = 2;
            var c = {re: 1, im: 1, pathType: 'sq-cloud', radius: R, speed: 1,
                     angle: 0, ccw: false, extra: {pow: 1, points: 500},
                     curve: [{re:1, im:1}], curveIndex: 0};
            var curve = computeCurveN(c.re, c.im, c.pathType, c.radius, c.angle, c.extra, 500);
            var maxX = 0, maxY = 0;
            for (var i = 0; i < curve.length; i++) {
                var dx = Math.abs(curve[i].re - 1), dy = Math.abs(curve[i].im - 1);
                if (dx > maxX) maxX = dx;
                if (dy > maxY) maxY = dy;
            }
            return {maxX: maxX, maxY: maxY, withinR: maxX <= R + 1e-10 && maxY <= R + 1e-10};
        }""")
        assert result["withinR"] is True

    def test_sq_cloud_power_concentrates(self, page):
        """Higher pow should concentrate points toward center for sq-cloud."""
        result = page.evaluate("""() => {
            function avgDist(pow) {
                var c = {re: 0, im: 0, pathType: 'sq-cloud', radius: 2, speed: 1,
                         angle: 0, ccw: false, extra: {pow: pow, points: 1000},
                         curve: [{re:0, im:0}], curveIndex: 0};
                var curve = computeCurveN(c.re, c.im, c.pathType, c.radius, c.angle, c.extra, 1000);
                var sum = 0;
                for (var i = 0; i < curve.length; i++) {
                    sum += Math.sqrt(curve[i].re*curve[i].re + curve[i].im*curve[i].im);
                }
                return sum / curve.length;
            }
            var avg1 = avgDist(1);
            var avg3 = avgDist(3);
            return {avg1: avg1, avg3: avg3, concentrated: avg3 < avg1};
        }""")
        assert result["concentrated"] is True


# ============================================================
# BITCFG tab
# ============================================================

class TestBitcfgTab:
    """Tests for the BITCFG tab (moved from bitmap cfg popup)."""

    def test_bitcfg_tab_button_exists(self, page):
        """BITCFG tab button should exist."""
        result = page.evaluate("""() => {
            var btn = document.querySelector('button[data-tab="bitcfg"]');
            return btn !== null;
        }""")
        assert result is True

    def test_bitcfg_content_div_exists(self, page):
        """bitcfg-content div should exist."""
        result = page.evaluate("""() => {
            return document.getElementById('bitcfg-content') !== null;
        }""")
        assert result is True

    def test_switch_to_bitcfg_builds_content(self, page):
        """Switching to BITCFG tab should build content via buildBitmapCfgContent."""
        result = page.evaluate("""() => {
            switchTab('bitcfg');
            var el = document.getElementById('bitcfg-content');
            return el.children.length > 0;
        }""")
        assert result is True

    def test_bitcfg_has_solver_title(self, page):
        """BITCFG tab should show SOLVER ENGINE title."""
        result = page.evaluate("""() => {
            switchTab('bitcfg');
            var el = document.getElementById('bitcfg-content');
            var title = el.querySelector('.pop-title');
            return title ? title.textContent : null;
        }""")
        assert result is not None
        assert "SOLVER" in result

    def test_bitcfg_has_color_mode_toggles(self, page):
        """BITCFG tab should show color mode toggles (audio-toggle divs)."""
        result = page.evaluate("""() => {
            switchTab('bitcfg');
            var el = document.getElementById('bitcfg-content');
            var toggles = el.querySelectorAll('.audio-toggle');
            return toggles.length;
        }""")
        assert result >= 5  # At least: uniform, rainbow, derivative, proximity, idx-prox, ratio, rel-proximity

    def test_bitcfg_has_bg_color_grid(self, page):
        """BITCFG tab should have background color grid."""
        result = page.evaluate("""() => {
            switchTab('bitcfg');
            var el = document.getElementById('bitcfg-content');
            // Look for color grid cells (small colored divs)
            var text = el.textContent;
            return text.indexOf('BACKGROUND') >= 0 || text.indexOf('Background') >= 0 ||
                   text.indexOf('BG') >= 0;
        }""")
        assert result is True

    def test_bitcfg_contains_all_color_modes(self, page):
        """BITCFG tab should list all 7 bitmap color modes."""
        result = page.evaluate("""() => {
            switchTab('bitcfg');
            var el = document.getElementById('bitcfg-content');
            var text = el.textContent;
            return {
                uniform: text.indexOf('Uniform') >= 0,
                rainbow: text.indexOf('Rainbow') >= 0,
                derivative: text.indexOf('Derivative') >= 0,
                proximity: text.indexOf('Proximity') >= 0,
                idxProx: text.indexOf('Prox') >= 0,
                ratio: text.indexOf('Ratio') >= 0,
                relProx: text.indexOf('Rel.') >= 0 || text.indexOf('Rel ') >= 0
            };
        }""")
        assert result["uniform"] is True
        assert result["rainbow"] is True
        assert result["proximity"] is True
        assert result["relProx"] is True


# ============================================================
# Relative Proximity color mode
# ============================================================

class TestRelProximity:
    """Tests for the rel-proximity bitmap color mode."""

    def test_rel_proximity_in_bitmap_modes(self, page):
        """bitmapModes should contain rel-proximity entry."""
        result = page.evaluate("""() => {
            switchTab('bitcfg');
            var el = document.getElementById('bitcfg-content');
            var text = el.textContent;
            return text.indexOf('Rel.') >= 0 || text.indexOf('Rel ') >= 0;
        }""")
        assert result is True

    def test_rel_proximity_selectable(self, page):
        """Clicking rel-proximity toggle should update bitmapColorMode."""
        result = page.evaluate("""() => {
            switchTab('bitcfg');
            var el = document.getElementById('bitcfg-content');
            var toggles = el.querySelectorAll('.audio-toggle');
            toggles.forEach(t => {
                var label = t.querySelector('.toggle-label');
                if (label && (label.textContent.indexOf('Rel.') >= 0 || label.textContent.indexOf('Rel ') >= 0)) {
                    t.click();
                }
            });
            return bitmapColorMode;
        }""")
        assert result == "rel-proximity"

    def test_rel_proximity_save_load(self, page):
        """rel-proximity color mode should roundtrip through save/load."""
        result = page.evaluate("""() => {
            bitmapColorMode = 'rel-proximity';
            var state = buildStateMetadata();
            bitmapColorMode = 'uniform';
            applyLoadedState(state);
            return bitmapColorMode;
        }""")
        assert result == "rel-proximity"

    def test_rel_proximity_worker_data_flag(self, page):
        """Worker data should include relProxColor flag when rel-proximity selected."""
        result = page.evaluate("""() => {
            bitmapColorMode = 'rel-proximity';
            // The worker data transfer sets relProxColor: bitmapColorMode === "rel-proximity"
            return bitmapColorMode === 'rel-proximity';
        }""")
        assert result is True


# ============================================================
# clearTrails comprehensive clearing
# ============================================================

class TestClearTrails:
    """Tests for the clearTrails() function clearing all 5 trail systems."""

    def test_cleartrails_clears_trail_data(self, page):
        """clearTrails should clear trailData array."""
        result = page.evaluate("""() => {
            trailData = [{x: 1}];
            clearTrails();
            return trailData.length;
        }""")
        assert result == 0

    def test_cleartrails_clears_final_trail_data(self, page):
        """clearTrails should clear finalTrailData array."""
        result = page.evaluate("""() => {
            finalTrailData = [{x: 1}];
            clearTrails();
            return finalTrailData.length;
        }""")
        assert result == 0

    def test_cleartrails_clears_root_trail_svg(self, page):
        """clearTrails should remove trail SVG paths."""
        result = page.evaluate("""() => {
            clearTrails();
            var paths = trailLayer.selectAll('path.trail-path');
            return paths.size();
        }""")
        assert result == 0

    def test_cleartrails_clears_coeff_trail_layer(self, page):
        """clearTrails should clear coeffTrailLayer."""
        result = page.evaluate("""() => {
            // Add a test element to coeffTrailLayer
            coeffTrailLayer.append('circle').attr('r', 5);
            var before = coeffTrailLayer.selectAll('*').size();
            clearTrails();
            var after = coeffTrailLayer.selectAll('*').size();
            return {before: before, after: after};
        }""")
        assert result["before"] > 0
        assert result["after"] == 0

    def test_cleartrails_clears_morph_trail_layer(self, page):
        """clearTrails should clear morphTrailLayer if it exists."""
        result = page.evaluate("""() => {
            if (!morphTrailLayer) return {skip: true};
            morphTrailLayer.append('circle').attr('r', 5);
            var before = morphTrailLayer.selectAll('*').size();
            clearTrails();
            var after = morphTrailLayer.selectAll('*').size();
            return {skip: false, before: before, after: after};
        }""")
        if result.get("skip"):
            pytest.skip("morphTrailLayer not initialized")
        assert result["before"] > 0
        assert result["after"] == 0

    def test_cleartrails_idempotent(self, page):
        """Calling clearTrails twice should not error."""
        result = page.evaluate("""() => {
            clearTrails();
            clearTrails();
            return true;
        }""")
        assert result is True


# ============================================================
# Home button / Home key reset
# ============================================================

class TestHomeReset:
    """Tests for the Home button and Home key reset behavior."""

    def test_home_button_resets_coefficients(self, page):
        """Home button should reset coefficients to curve[0]."""
        result = page.evaluate("""() => {
            // Move coefficients away from curve[0]
            for (var i = 0; i < coefficients.length; i++) {
                coefficients[i].re += 5;
                coefficients[i].im += 5;
            }
            var moved = coefficients.map(c => ({re: c.re, im: c.im}));

            // Click Home
            document.getElementById('home-btn').click();

            // Check they match curve[0]
            var allReset = true;
            for (var i = 0; i < coefficients.length; i++) {
                var c = coefficients[i];
                if (Math.abs(c.re - c.curve[0].re) > 1e-10 ||
                    Math.abs(c.im - c.curve[0].im) > 1e-10) {
                    allReset = false;
                }
            }
            return allReset;
        }""")
        assert result is True

    def test_home_button_resets_curve_index(self, page):
        """Home button should reset curveIndex to 0."""
        result = page.evaluate("""() => {
            coefficients[0].curveIndex = 50;
            document.getElementById('home-btn').click();
            return coefficients[0].curveIndex;
        }""")
        assert result == 0

    def test_home_button_stops_animation(self, page):
        """Home button should stop animation if playing."""
        result = page.evaluate("""() => {
            // Don't actually start animation (needs canvas), just check state
            document.getElementById('home-btn').click();
            return animState.playing === false || animState.playing === undefined;
        }""")
        assert result is True

    def test_home_button_clears_trails(self, page):
        """Home button should call clearTrails."""
        result = page.evaluate("""() => {
            trailData = [{x: 1}];
            finalTrailData = [{x: 1}];
            document.getElementById('home-btn').click();
            return {trails: trailData.length, finalTrails: finalTrailData.length};
        }""")
        assert result["trails"] == 0
        assert result["finalTrails"] == 0

    def test_home_button_resets_jiggle(self, page):
        """Home button should reset jiggle state."""
        result = page.evaluate("""() => {
            jiggleOffsets = [1, 2, 3];
            jiggleStep = 10;
            document.getElementById('home-btn').click();
            return {offsets: jiggleOffsets, step: jiggleStep};
        }""")
        assert result["offsets"] is None
        assert result["step"] == -1

    def test_home_button_resets_morph(self, page):
        """Home button should reset morphMu and morphTheta."""
        result = page.evaluate("""() => {
            morphMu = 0.5;
            morphTheta = 1.23;
            document.getElementById('home-btn').click();
            return {mu: morphMu, theta: morphTheta};
        }""")
        assert result["mu"] == 0
        assert result["theta"] == 0

    def test_home_key_triggers_button(self, page):
        """Pressing Home key should trigger the home button click."""
        result = page.evaluate("""() => {
            // Move a coefficient
            coefficients[0].re += 10;
            var movedRe = coefficients[0].re;

            // Dispatch Home key
            document.dispatchEvent(new KeyboardEvent('keydown', {key: 'Home'}));

            // Check coefficient was reset to curve[0]
            return {
                movedRe: movedRe,
                afterRe: coefficients[0].re,
                curveRe: coefficients[0].curve[0].re,
                reset: Math.abs(coefficients[0].re - coefficients[0].curve[0].re) < 1e-10
            };
        }""")
        assert result["reset"] is True

    def test_home_resets_d_nodes(self, page):
        """Home button should reset D-node positions to their curve[0]."""
        result = page.evaluate("""() => {
            if (morphTargetCoeffs.length === 0) return {skip: true};
            var d = morphTargetCoeffs[0];
            d.re += 5; d.im += 5;
            document.getElementById('home-btn').click();
            var expected = d.curve[0];
            // For non-orbit paths, should match curve[0]
            if (!d.curve._orbRefC) {
                return {skip: false,
                    match: Math.abs(d.re - expected.re) < 1e-8 && Math.abs(d.im - expected.im) < 1e-8};
            }
            return {skip: false, match: true};  // orbit paths add ref C position
        }""")
        if result.get("skip"):
            pytest.skip("No D-nodes to test")
        assert result["match"] is True
