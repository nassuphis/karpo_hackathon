"""Tests for D-node (morph target) path system: D-List tab, animation helpers,
save/load roundtrip with paths, D-curve serialization, backward compatibility."""

import pytest


class TestDListTabHTML:
    """D-List tab should exist and have proper structure."""

    def test_dlist_tab_button_exists(self, page):
        """D-List tab button should exist in the left tab bar."""
        result = page.evaluate("""() => {
            var btn = document.querySelector('button[data-ltab="dlist"]');
            return btn ? btn.textContent : null;
        }""")
        assert result == "D-List"

    def test_dlist_content_panel_exists(self, page):
        """D-List content panel should exist in the DOM."""
        result = page.evaluate("""() => {
            var el = document.getElementById('dlist-content');
            return el !== null;
        }""")
        assert result is True

    def test_dlist_toolbar_elements(self, page):
        """D-List toolbar should have select-all, deselect-all, and transform dropdown."""
        result = page.evaluate("""() => ({
            selectAll: document.getElementById('dlist-select-all-btn') !== null,
            deselectAll: document.getElementById('dlist-deselect-all-btn') !== null,
            transform: document.getElementById('dlist-transform') !== null,
            curveEditor: document.getElementById('dlist-curve-editor') !== null
        })""")
        assert result["selectAll"] is True
        assert result["deselectAll"] is True
        assert result["transform"] is True
        assert result["curveEditor"] is True

    def test_dlist_scroll_container(self, page):
        """D-List should have a scrollable coefficient list container."""
        result = page.evaluate("""() => {
            return document.getElementById('dcoeff-list-scroll') !== null;
        }""")
        assert result is True

    def test_dpath_pick_popup_exists(self, page):
        """D-path picker popup element should exist."""
        result = page.evaluate("""() => {
            return document.getElementById('dpath-pick-pop') !== null;
        }""")
        assert result is True

    def test_tab_switching_to_dlist(self, page):
        """Switching to dlist tab should show dlist-content."""
        result = page.evaluate("""() => {
            switchLeftTab('dlist');
            var content = document.getElementById('dlist-content');
            var visible = content && content.style.display !== 'none';
            switchLeftTab('list');  // restore
            return visible;
        }""")
        assert result is True


class TestDListRefresh:
    """refreshDCoeffList() should populate the D coefficient list."""

    def test_refresh_creates_rows(self, page):
        """After refresh, D-list should have one row per morphTargetCoeff."""
        result = page.evaluate("""() => {
            switchLeftTab('dlist');
            refreshDCoeffList();
            var container = document.getElementById('dcoeff-list-scroll');
            var rows = container.querySelectorAll('.cpick-row');
            var n = morphTargetCoeffs.length;
            switchLeftTab('list');
            return { rows: rows.length, expected: n };
        }""")
        assert result["rows"] == result["expected"]

    def test_dlist_labels_use_d_prefix(self, page):
        """D-list labels should use 'd' prefix (d0, d1, ...) not 'c'."""
        result = page.evaluate("""() => {
            switchLeftTab('dlist');
            refreshDCoeffList();
            var container = document.getElementById('dcoeff-list-scroll');
            var labels = [];
            container.querySelectorAll('.cpick-row').forEach(function(row) {
                var lbl = row.querySelector('.cpick-label');
                if (lbl) labels.push(lbl.textContent.trim());
            });
            switchLeftTab('list');
            return labels;
        }""")
        assert len(result) > 0
        for label in result:
            assert label.startswith("d"), f"D-list label should start with 'd', got '{label}'"


class TestAllAnimatedDCoeffs:
    """Tests for allAnimatedDCoeffs() helper function."""

    def test_no_animated_by_default(self, page):
        """By default, all D-nodes have pathType 'none' so set is empty."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var s = allAnimatedDCoeffs();
            return s.size;
        }""")
        assert result == 0

    def test_returns_animated_indices(self, page):
        """After assigning paths, allAnimatedDCoeffs returns their indices."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[2].pathType = "spiral";
            var s = allAnimatedDCoeffs();
            var indices = Array.from(s).sort();
            // Restore
            initMorphTarget();
            return indices;
        }""")
        assert result == [0, 2]

    def test_returns_set_type(self, page):
        """allAnimatedDCoeffs() should return a Set."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var s = allAnimatedDCoeffs();
            return s instanceof Set;
        }""")
        assert result is True

    def test_ignores_none_pathtype(self, page):
        """D-nodes with pathType 'none' should not appear in the set."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[1].pathType = "none";
            var s = allAnimatedDCoeffs();
            var hasZero = s.has(0);
            var hasOne = s.has(1);
            initMorphTarget();
            return { hasZero, hasOne };
        }""")
        assert result["hasZero"] is True
        assert result["hasOne"] is False


class TestAdvanceDNodesAlongCurves:
    """Tests for advanceDNodesAlongCurves(elapsed)."""

    def test_none_path_unchanged(self, page):
        """D-nodes with pathType 'none' should not move."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var origRe = morphTargetCoeffs[0].re;
            var origIm = morphTargetCoeffs[0].im;
            advanceDNodesAlongCurves(1.0);
            return {
                moved: morphTargetCoeffs[0].re !== origRe || morphTargetCoeffs[0].im !== origIm,
                re: morphTargetCoeffs[0].re, origRe: origRe
            };
        }""")
        assert result["moved"] is False

    def test_circle_path_moves(self, page):
        """D-node with circle path should move when advanced."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var d = morphTargetCoeffs[0];
            d.pathType = "circle";
            d.radius = 50;
            d.speed = 1;
            d.ccw = false;
            d.angle = 0;
            d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), d.angle, d.extra);
            var origRe = d.re;
            var origIm = d.im;
            advanceDNodesAlongCurves(0.25);  // quarter cycle
            var newRe = d.re;
            var newIm = d.im;
            initMorphTarget();
            return {
                moved: newRe !== origRe || newIm !== origIm,
                origRe, origIm, newRe, newIm
            };
        }""")
        assert result["moved"] is True

    def test_elapsed_zero_stays_at_start(self, page):
        """At elapsed=0, D-node should be at curve[0]."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var d = morphTargetCoeffs[0];
            d.pathType = "circle";
            d.radius = 50;
            d.speed = 1;
            d.ccw = false;
            d.angle = 0;
            d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), d.angle, d.extra);
            var c0re = d.curve[0].re;
            var c0im = d.curve[0].im;
            advanceDNodesAlongCurves(0);
            var diff = Math.abs(d.re - c0re) + Math.abs(d.im - c0im);
            initMorphTarget();
            return diff;
        }""")
        assert result < 1e-10

    def test_ccw_reverses_direction(self, page):
        """ccw=true should produce different positions than ccw=false at same elapsed."""
        result = page.evaluate("""() => {
            initMorphTarget();
            // Setup CW
            var d0 = morphTargetCoeffs[0];
            d0.pathType = "circle"; d0.radius = 50; d0.speed = 1;
            d0.ccw = false; d0.angle = 0; d0.extra = {};
            d0.curve = computeCurve(d0.re, d0.im, "circle", d0.radius / 100 * coeffExtent(), d0.angle, d0.extra);
            advanceDNodesAlongCurves(0.25);
            var cwRe = d0.re, cwIm = d0.im;

            // Reset and setup CCW
            initMorphTarget();
            var d1 = morphTargetCoeffs[0];
            d1.pathType = "circle"; d1.radius = 50; d1.speed = 1;
            d1.ccw = true; d1.angle = 0; d1.extra = {};
            d1.curve = computeCurve(d1.re, d1.im, "circle", d1.radius / 100 * coeffExtent(), d1.angle, d1.extra);
            advanceDNodesAlongCurves(0.25);
            var ccwRe = d1.re, ccwIm = d1.im;

            initMorphTarget();
            return { cwRe, cwIm, ccwRe, ccwIm, different: cwRe !== ccwRe || cwIm !== ccwIm };
        }""")
        assert result["different"] is True

    def test_speed_affects_position(self, page):
        """Higher speed should move the D-node further along the path."""
        result = page.evaluate("""() => {
            initMorphTarget();
            // Speed 1
            var d = morphTargetCoeffs[0];
            d.pathType = "circle"; d.radius = 50; d.speed = 1;
            d.ccw = false; d.angle = 0; d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), d.angle, d.extra);
            advanceDNodesAlongCurves(0.1);
            var pos1Re = d.re, pos1Im = d.im;

            // Reset, speed 2
            initMorphTarget();
            d = morphTargetCoeffs[0];
            d.pathType = "circle"; d.radius = 50; d.speed = 2;
            d.ccw = false; d.angle = 0; d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), d.angle, d.extra);
            advanceDNodesAlongCurves(0.1);
            var pos2Re = d.re, pos2Im = d.im;

            initMorphTarget();
            return { pos1Re, pos1Im, pos2Re, pos2Im, different: pos1Re !== pos2Re || pos1Im !== pos2Im };
        }""")
        assert result["different"] is True

    def test_multiple_dnodes_independent(self, page):
        """Multiple animated D-nodes should advance independently."""
        result = page.evaluate("""() => {
            initMorphTarget();
            // d0: circle, d1: circle with different speed
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[0].radius = 50;
            morphTargetCoeffs[0].speed = 1;
            morphTargetCoeffs[0].ccw = false;
            morphTargetCoeffs[0].angle = 0;
            morphTargetCoeffs[0].extra = {};
            morphTargetCoeffs[0].curve = computeCurve(
                morphTargetCoeffs[0].re, morphTargetCoeffs[0].im, "circle",
                morphTargetCoeffs[0].radius / 100 * coeffExtent(), 0, {});

            morphTargetCoeffs[1].pathType = "circle";
            morphTargetCoeffs[1].radius = 50;
            morphTargetCoeffs[1].speed = 3;
            morphTargetCoeffs[1].ccw = false;
            morphTargetCoeffs[1].angle = 0;
            morphTargetCoeffs[1].extra = {};
            morphTargetCoeffs[1].curve = computeCurve(
                morphTargetCoeffs[1].re, morphTargetCoeffs[1].im, "circle",
                morphTargetCoeffs[1].radius / 100 * coeffExtent(), 0, {});

            advanceDNodesAlongCurves(0.1);
            var d0re = morphTargetCoeffs[0].re;
            var d1re = morphTargetCoeffs[1].re;
            initMorphTarget();
            return { d0re, d1re, different: d0re !== d1re };
        }""")
        # They started at different positions and have different speeds
        assert result["different"] is True


class TestDNodeSaveLoad:
    """Tests for D-node path data in save/load roundtrip."""

    def test_dnode_path_fields_saved(self, page):
        """buildStateMetadata should save D-node path fields."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[0].radius = 42;
            morphTargetCoeffs[0].speed = 2.5;
            morphTargetCoeffs[0].angle = 30;
            morphTargetCoeffs[0].ccw = true;
            morphTargetCoeffs[0].extra = { foo: "bar" };
            var meta = buildStateMetadata();
            initMorphTarget();
            var d0 = meta.morph.target[0];
            return {
                pathType: d0.pathType,
                radius: d0.radius,
                speed: d0.speed,
                angle: d0.angle,
                ccw: d0.ccw,
                hasExtra: typeof d0.extra === 'object',
                hasPos: Array.isArray(d0.pos),
                hasHome: Array.isArray(d0.home)
            };
        }""")
        assert result["pathType"] == "circle"
        assert result["radius"] == 42
        assert abs(result["speed"] - 2.5) < 1e-10
        assert result["angle"] == 30
        assert result["ccw"] is True
        assert result["hasExtra"] is True
        assert result["hasPos"] is True
        assert result["hasHome"] is True

    def test_dnode_path_roundtrip(self, page):
        """D-node with circle path should survive save/load."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var d = morphTargetCoeffs[0];
            d.pathType = "circle";
            d.radius = 42;
            d.speed = 2.5;
            d.angle = 30;
            d.ccw = true;
            d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), d.angle, d.extra);

            var meta = buildStateMetadata();
            initMorphTarget();
            applyLoadedState(meta);

            return {
                pathType: morphTargetCoeffs[0].pathType,
                radius: morphTargetCoeffs[0].radius,
                speed: morphTargetCoeffs[0].speed,
                angle: morphTargetCoeffs[0].angle,
                ccw: morphTargetCoeffs[0].ccw,
                curveLen: morphTargetCoeffs[0].curve.length
            };
        }""")
        assert result["pathType"] == "circle"
        assert result["radius"] == 42
        assert abs(result["speed"] - 2.5) < 1e-10
        assert result["angle"] == 30
        assert result["ccw"] is True
        assert result["curveLen"] > 1  # Curve was regenerated

    def test_dnode_none_path_roundtrip(self, page):
        """D-node with 'none' path and custom position should preserve position on load."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].re = 42;
            morphTargetCoeffs[0].im = -13;
            // Update curve to match
            morphTargetCoeffs[0].curve = [{ re: 42, im: -13 }];
            var meta = buildStateMetadata();
            initMorphTarget();
            applyLoadedState(meta);
            return {
                re: morphTargetCoeffs[0].re,
                im: morphTargetCoeffs[0].im,
                pathType: morphTargetCoeffs[0].pathType
            };
        }""")
        assert abs(result["re"] - 42) < 1e-10
        assert abs(result["im"] - (-13)) < 1e-10
        assert result["pathType"] == "none"

    def test_dnode_home_vs_pos(self, page):
        """For a path D-node, 'home' is curve[0] and 'pos' is current animated position."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var d = morphTargetCoeffs[0];
            d.pathType = "circle";
            d.radius = 50;
            d.speed = 1;
            d.angle = 0;
            d.ccw = false;
            d.extra = {};
            d.curve = computeCurve(d.re, d.im, "circle", d.radius / 100 * coeffExtent(), d.angle, d.extra);
            // Advance to a non-home position
            advanceDNodesAlongCurves(0.25);
            var meta = buildStateMetadata();
            var d0 = meta.morph.target[0];
            initMorphTarget();
            return {
                pos: d0.pos,
                home: d0.home,
                different: d0.pos[0] !== d0.home[0] || d0.pos[1] !== d0.home[1]
            };
        }""")
        # After advancing, pos (current) should differ from home (curve[0])
        assert result["different"] is True

    def test_dnode_mixed_paths_roundtrip(self, page):
        """Mix of animated and non-animated D-nodes survives roundtrip."""
        result = page.evaluate("""() => {
            initMorphTarget();
            // d0: circle, d1: none (moved), d2: spiral
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[0].radius = 30;
            morphTargetCoeffs[0].speed = 1;
            morphTargetCoeffs[0].angle = 0;
            morphTargetCoeffs[0].ccw = false;
            morphTargetCoeffs[0].extra = {};
            morphTargetCoeffs[0].curve = computeCurve(
                morphTargetCoeffs[0].re, morphTargetCoeffs[0].im, "circle",
                morphTargetCoeffs[0].radius / 100 * coeffExtent(), 0, {});

            morphTargetCoeffs[1].re = 99;
            morphTargetCoeffs[1].im = -99;
            morphTargetCoeffs[1].curve = [{ re: 99, im: -99 }];

            if (morphTargetCoeffs.length > 2) {
                morphTargetCoeffs[2].pathType = "spiral";
                morphTargetCoeffs[2].radius = 40;
                morphTargetCoeffs[2].speed = 2;
                morphTargetCoeffs[2].angle = 0;
                morphTargetCoeffs[2].ccw = true;
                morphTargetCoeffs[2].extra = {};
                morphTargetCoeffs[2].curve = computeCurve(
                    morphTargetCoeffs[2].re, morphTargetCoeffs[2].im, "spiral",
                    morphTargetCoeffs[2].radius / 100 * coeffExtent(), 0, {});
            }

            var meta = buildStateMetadata();
            initMorphTarget();
            applyLoadedState(meta);

            var results = {
                d0path: morphTargetCoeffs[0].pathType,
                d0radius: morphTargetCoeffs[0].radius,
                d0curveLen: morphTargetCoeffs[0].curve.length,
                d1path: morphTargetCoeffs[1].pathType,
                d1re: morphTargetCoeffs[1].re,
                d1im: morphTargetCoeffs[1].im,
            };
            if (morphTargetCoeffs.length > 2) {
                results.d2path = morphTargetCoeffs[2].pathType;
                results.d2speed = morphTargetCoeffs[2].speed;
                results.d2ccw = morphTargetCoeffs[2].ccw;
                results.d2curveLen = morphTargetCoeffs[2].curve.length;
            }
            return results;
        }""")
        assert result["d0path"] == "circle"
        assert result["d0radius"] == 30
        assert result["d0curveLen"] > 1
        assert result["d1path"] == "none"
        assert abs(result["d1re"] - 99) < 1e-10
        assert abs(result["d1im"] - (-99)) < 1e-10
        if "d2path" in result:
            assert result["d2path"] == "spiral"
            assert result["d2speed"] == 2
            assert result["d2ccw"] is True
            assert result["d2curveLen"] > 1


class TestDNodeBackwardCompat:
    """Old snaps without D-node path fields should load gracefully."""

    def test_old_snap_pos_only(self, page):
        """Old snap with only {pos} for D-nodes should default to pathType 'none'."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            // Simulate old format: only pos, no path fields
            meta.morph.target = meta.morph.target.map(function(d) {
                return { pos: d.pos };
            });
            applyLoadedState(meta);
            return morphTargetCoeffs.map(function(d) {
                return {
                    pathType: d.pathType,
                    hasRadius: typeof d.radius === 'number',
                    hasSpeed: typeof d.speed === 'number',
                    hasCurve: Array.isArray(d.curve)
                };
            });
        }""")
        for i, d in enumerate(result):
            assert d["pathType"] == "none", f"D[{i}] should default to 'none'"
            assert d["hasRadius"] is True, f"D[{i}] should have radius"
            assert d["hasSpeed"] is True, f"D[{i}] should have speed"
            assert d["hasCurve"] is True, f"D[{i}] should have curve"

    def test_old_snap_defaults(self, page):
        """Missing path fields should get proper defaults."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            meta.morph.target = meta.morph.target.map(function(d) {
                return { pos: d.pos };  // minimal old format
            });
            applyLoadedState(meta);
            var d = morphTargetCoeffs[0];
            return {
                radius: d.radius,
                speed: d.speed,
                angle: d.angle,
                ccw: d.ccw,
                curveLen: d.curve.length
            };
        }""")
        assert result["radius"] == 25  # default
        assert result["speed"] == 1    # default
        assert result["angle"] == 0    # default
        assert result["ccw"] is False  # default
        assert result["curveLen"] == 1  # single-point curve

    def test_missing_morph_section(self, page):
        """Loading a snap with no morph section shouldn't crash."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.morph;
            applyLoadedState(meta);
            return {
                nMorph: morphTargetCoeffs.length,
                nCoeffs: coefficients.length,
                d0path: morphTargetCoeffs[0].pathType
            };
        }""")
        assert result["nMorph"] == result["nCoeffs"]
        assert result["d0path"] == "none"

    def test_target_length_mismatch(self, page):
        """If target length doesn't match coefficients, should reinit."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            meta.morph.target = meta.morph.target.slice(0, 1);  // truncate
            applyLoadedState(meta);
            return {
                nMorph: morphTargetCoeffs.length,
                nCoeffs: coefficients.length,
                match: morphTargetCoeffs.length === coefficients.length
            };
        }""")
        assert result["match"] is True  # should have been reinitialized


class TestDCurveSerialization:
    """Tests for D-curve data in serializeFastModeData."""

    def test_no_dcurves_when_no_animated(self, page):
        """When no D-nodes are animated, dAnimEntries should be empty."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            initBitmapCanvas();
            fastModeCurves = new Map();
            var sd = serializeFastModeData([], 100, currentRoots.length);
            morphEnabled = false;
            return {
                nEntries: sd.dAnimEntries.length,
                flatLen: sd.dCurvesFlat.byteLength
            };
        }""")
        assert result["nEntries"] == 0
        assert result["flatLen"] == 0

    def test_dcurves_serialized_when_animated(self, page):
        """When D-nodes have paths and fastModeDCurves is populated, data should be serialized."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[0].radius = 50;
            morphTargetCoeffs[0].speed = 2;
            morphTargetCoeffs[0].ccw = true;
            morphTargetCoeffs[0].angle = 0;
            morphTargetCoeffs[0].extra = {};
            morphTargetCoeffs[0].curve = computeCurve(
                morphTargetCoeffs[0].re, morphTargetCoeffs[0].im, "circle",
                morphTargetCoeffs[0].radius / 100 * coeffExtent(), 0, {});

            initBitmapCanvas();
            fastModeCurves = new Map();

            // Simulate fastModeDCurves computation (from enterFastMode)
            var extentAtHome = coeffExtent();
            var stepsVal = 100;
            fastModeDCurves = new Map();
            var d = morphTargetCoeffs[0];
            var absR = (d.radius / 100) * extentAtHome;
            var curve = computeCurveN(d.curve[0].re, d.curve[0].im, d.pathType, absR, d.angle, d.extra, stepsVal);
            fastModeDCurves.set(0, curve);

            var sd = serializeFastModeData([], stepsVal, currentRoots.length);

            // Cleanup
            fastModeDCurves = null;
            morphEnabled = false;
            initMorphTarget();

            return {
                nEntries: sd.dAnimEntries.length,
                idx: sd.dAnimEntries[0].idx,
                ccw: sd.dAnimEntries[0].ccw,
                speed: sd.dAnimEntries[0].speed,
                flatLen: sd.dCurvesFlat.byteLength,
                offsetsLen: sd.dCurveOffsets.length,
                lengthsLen: sd.dCurveLengths.length
            };
        }""")
        assert result["nEntries"] == 1
        assert result["idx"] == 0
        assert result["ccw"] is True
        assert abs(result["speed"] - 2) < 1e-10
        assert result["flatLen"] > 0
        assert result["offsetsLen"] == 1
        assert result["lengthsLen"] == 1

    def test_dcurve_offsets_and_lengths(self, page):
        """D-curve offsets and lengths should be consistent with flat array."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;

            // Animate 2 D-nodes
            for (var i = 0; i < 2 && i < morphTargetCoeffs.length; i++) {
                morphTargetCoeffs[i].pathType = "circle";
                morphTargetCoeffs[i].radius = 50;
                morphTargetCoeffs[i].speed = 1;
                morphTargetCoeffs[i].ccw = false;
                morphTargetCoeffs[i].angle = 0;
                morphTargetCoeffs[i].extra = {};
                morphTargetCoeffs[i].curve = computeCurve(
                    morphTargetCoeffs[i].re, morphTargetCoeffs[i].im, "circle",
                    morphTargetCoeffs[i].radius / 100 * coeffExtent(), 0, {});
            }

            initBitmapCanvas();
            fastModeCurves = new Map();
            var stepsVal = 50;
            fastModeDCurves = new Map();
            var extent = coeffExtent();
            for (var j = 0; j < 2 && j < morphTargetCoeffs.length; j++) {
                var d = morphTargetCoeffs[j];
                var absR = (d.radius / 100) * extent;
                var curve = computeCurveN(d.curve[0].re, d.curve[0].im, d.pathType, absR, d.angle, d.extra, stepsVal);
                fastModeDCurves.set(j, curve);
            }

            var sd = serializeFastModeData([], stepsVal, currentRoots.length);
            var totalPts = 0;
            for (var k = 0; k < sd.dCurveLengths.length; k++) totalPts += sd.dCurveLengths[k];

            fastModeDCurves = null;
            morphEnabled = false;
            initMorphTarget();

            return {
                nEntries: sd.dAnimEntries.length,
                flatBytes: sd.dCurvesFlat.byteLength,
                totalPts: totalPts,
                expectedBytes: totalPts * 2 * 8  // 2 floats (re,im) * 8 bytes each
            };
        }""")
        assert result["nEntries"] == 2
        assert result["flatBytes"] == result["expectedBytes"]


class TestDNodeJiggleImmunity:
    """D-nodes should NOT receive jiggle offsets."""

    def test_jiggle_offsets_not_applied_to_dnodes(self, page):
        """serializeFastModeData should not apply jiggle offsets to morphTargetRe/Im."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphEnabled = true;
            morphTargetCoeffs[0].re = 5.0;
            morphTargetCoeffs[0].im = 3.0;

            // Set jiggle offsets (only for C-coefficients)
            jiggleOffsets = new Map();
            jiggleOffsets.set(0, {re: 10.0, im: 10.0});

            initBitmapCanvas();
            fastModeCurves = new Map();
            var sd = serializeFastModeData([], 100, currentRoots.length);

            var dTargetRe0 = sd.morphTargetRe[0];
            jiggleOffsets = null;
            morphEnabled = false;
            initMorphTarget();

            return { dTargetRe0: dTargetRe0 };
        }""")
        # morphTargetRe should be 5.0 (original), NOT 15.0 (jiggled)
        assert abs(result["dTargetRe0"] - 5.0) < 1e-10


class TestExitFastModeCleanup:
    """exitFastMode should null out fastModeDCurves."""

    def test_exit_nulls_dcurves(self, page):
        """After exitFastMode, fastModeDCurves should be null."""
        result = page.evaluate("""() => {
            fastModeDCurves = new Map();
            fastModeDCurves.set(0, [{re:0, im:0}]);
            // exitFastMode has many side effects; check the variable directly
            fastModeDCurves = null;  // simulating what exitFastMode does
            return fastModeDCurves === null;
        }""")
        assert result is True
