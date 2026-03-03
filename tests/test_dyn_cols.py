"""Tests for dynamic path-parameter columns in C-list and D-list tables."""

import pytest


class TestDynColRegistry:
    """_DYN_COL_DEFS, _DYN_COL_ORDER, _DYN_HDR_LABELS built at startup."""

    def test_registry_populated(self, page):
        """_DYN_COL_DEFS and _DYN_COL_ORDER are non-empty."""
        result = page.evaluate("""() => ({
            nDefs: Object.keys(_DYN_COL_DEFS).length,
            nOrder: _DYN_COL_ORDER.length,
        })""")
        assert result["nDefs"] > 0
        assert result["nOrder"] > 0
        assert result["nDefs"] == result["nOrder"]

    def test_fixed_keys_excluded(self, page):
        """rAbs, speed, points are NOT in _DYN_COL_ORDER."""
        result = page.evaluate("""() => ({
            hasRabs: _DYN_COL_ORDER.includes("rAbs"),
            hasSpeed: _DYN_COL_ORDER.includes("speed"),
            hasPoints: _DYN_COL_ORDER.includes("points"),
        })""")
        assert result["hasRabs"] is False
        assert result["hasSpeed"] is False
        assert result["hasPoints"] is False

    def test_angle_ccw_are_dynamic(self, page):
        """angle and ccw should be in the dynamic column registry."""
        result = page.evaluate("""() => ({
            hasAngle: _DYN_COL_ORDER.includes("angle"),
            hasCcw: _DYN_COL_ORDER.includes("ccw"),
            angleDef: !!_DYN_COL_DEFS["angle"],
            ccwDef: !!_DYN_COL_DEFS["ccw"],
        })""")
        assert result["hasAngle"] is True
        assert result["hasCcw"] is True
        assert result["angleDef"] is True
        assert result["ccwDef"] is True

    def test_known_keys_present(self, page):
        """Known dynamic param keys appear in the registry."""
        result = page.evaluate("""() => {
            var keys = ["mult", "turns", "width", "freqA", "freqB",
                        "pow", "rings", "sigma", "ditherDist"];
            var present = {};
            for (var k of keys) present[k] = !!_DYN_COL_DEFS[k];
            return present;
        }""")
        for k, v in result.items():
            assert v is True, f"{k} missing from _DYN_COL_DEFS"

    def test_hdr_labels_cover_all_order(self, page):
        """Every key in _DYN_COL_ORDER has a label in _DYN_HDR_LABELS."""
        result = page.evaluate("""() => {
            var missing = [];
            for (var k of _DYN_COL_ORDER) {
                if (!_DYN_HDR_LABELS[k] && !(_DYN_COL_DEFS[k] && _DYN_COL_DEFS[k].label))
                    missing.push(k);
            }
            return missing;
        }""")
        assert result == [], f"Keys without header labels: {result}"

    def test_order_matches_defs_keys(self, page):
        """_DYN_COL_ORDER and Object.keys(_DYN_COL_DEFS) have the same set."""
        result = page.evaluate("""() => {
            var orderSet = new Set(_DYN_COL_ORDER);
            var defsSet = new Set(Object.keys(_DYN_COL_DEFS));
            var onlyOrder = [...orderSet].filter(k => !defsSet.has(k));
            var onlyDefs = [...defsSet].filter(k => !orderSet.has(k));
            return { onlyOrder, onlyDefs };
        }""")
        assert result["onlyOrder"] == []
        assert result["onlyDefs"] == []


class TestComputeActiveDynCols:
    """computeActiveDynCols(coeffArray) returns correct active columns."""

    def test_no_paths_returns_empty(self, page):
        """All 'none' paths → no dynamic columns."""
        result = page.evaluate("""() => {
            var arr = [
                { pathType: "none", re: 1, im: 0 },
                { pathType: "none", re: 0, im: 1 },
            ];
            return computeActiveDynCols(arr);
        }""")
        assert result == []

    def test_circle_returns_angle_ccw(self, page):
        """Circle path has angle and ccw params."""
        result = page.evaluate("""() => {
            var arr = [
                { pathType: "circle", re: 1, im: 0, angle: 0, ccw: false, extra: {} },
            ];
            return computeActiveDynCols(arr);
        }""")
        assert "angle" in result
        assert "ccw" in result

    def test_o_spiral_returns_turns(self, page):
        """o-spiral path includes 'turns' param."""
        result = page.evaluate("""() => {
            var arr = [
                { pathType: "o-spiral", re: 1, im: 0, angle: 0, ccw: false, extra: { turns: 3 } },
            ];
            return computeActiveDynCols(arr);
        }""")
        assert "turns" in result

    def test_lissajous_returns_freq_params(self, page):
        """Lissajous path includes freqA and freqB."""
        result = page.evaluate("""() => {
            var arr = [
                { pathType: "lissajous", re: 0, im: 0, angle: 0, ccw: false,
                  extra: { freqA: 3, freqB: 2 } },
            ];
            return computeActiveDynCols(arr);
        }""")
        assert "freqA" in result
        assert "freqB" in result

    def test_mixed_paths_union(self, page):
        """Multiple path types → union of their params."""
        result = page.evaluate("""() => {
            var arr = [
                { pathType: "circle", re: 1, im: 0, angle: 0, ccw: false, extra: {} },
                { pathType: "o-spiral", re: 0, im: 0, angle: 0, ccw: false, extra: { turns: 2 } },
            ];
            return computeActiveDynCols(arr);
        }""")
        assert "angle" in result
        assert "ccw" in result
        assert "turns" in result

    def test_follow_c_excluded(self, page):
        """'follow-c' paths are excluded from active column computation."""
        result = page.evaluate("""() => {
            var arr = [
                { pathType: "follow-c", re: 1, im: 0, extra: {} },
            ];
            return computeActiveDynCols(arr);
        }""")
        assert result == []

    def test_order_matches_dyn_col_order(self, page):
        """Returned keys follow the global _DYN_COL_ORDER ordering."""
        result = page.evaluate("""() => {
            var arr = [
                { pathType: "lissajous", re: 0, im: 0, angle: 0, ccw: false,
                  extra: { freqA: 3, freqB: 2 } },
                { pathType: "spiral", re: 1, im: 0, angle: 0, ccw: false, extra: { turns: 2 } },
            ];
            var active = computeActiveDynCols(arr);
            // Verify each key's index matches _DYN_COL_ORDER ordering
            for (var i = 1; i < active.length; i++) {
                var prevIdx = _DYN_COL_ORDER.indexOf(active[i-1]);
                var currIdx = _DYN_COL_ORDER.indexOf(active[i]);
                if (currIdx <= prevIdx) return { ok: false, prev: active[i-1], curr: active[i] };
            }
            return { ok: true };
        }""")
        assert result["ok"] is True


class TestDynColCellText:
    """dynColCellText(c, key) formats cell values correctly."""

    def test_none_path_returns_dash(self, page):
        """'none' pathType returns em-dash."""
        result = page.evaluate("""() => {
            return dynColCellText({ pathType: "none" }, "angle");
        }""")
        assert result == "\u2014"

    def test_follow_c_returns_dash(self, page):
        """'follow-c' pathType returns em-dash."""
        result = page.evaluate("""() => {
            return dynColCellText({ pathType: "follow-c" }, "angle");
        }""")
        assert result == "\u2014"

    def test_missing_param_returns_dash(self, page):
        """Param not in path's PATH_PARAMS returns em-dash."""
        result = page.evaluate("""() => {
            // circle doesn't have 'turns'
            return dynColCellText({ pathType: "circle", extra: {} }, "turns");
        }""")
        assert result == "\u2014"

    def test_angle_reads_from_coeff(self, page):
        """angle (a _STD_KEY) reads from c.angle, not c.extra."""
        result = page.evaluate("""() => {
            return dynColCellText({
                pathType: "circle", angle: 0.25, ccw: false, extra: {}
            }, "angle");
        }""")
        # angle fmt is v.toFixed(2), so 0.25 → "0.25"
        assert result == "0.25"

    def test_ccw_toggle_display(self, page):
        """ccw (toggle type) shows CW/CCW labels."""
        result = page.evaluate("""() => {
            var cw = dynColCellText({ pathType: "circle", angle: 0, ccw: false, extra: {} }, "ccw");
            var ccw = dynColCellText({ pathType: "circle", angle: 0, ccw: true, extra: {} }, "ccw");
            return { cw, ccw };
        }""")
        # Toggle labels: false shows labels[0], true shows labels[1]
        assert result["cw"] != result["ccw"]

    def test_extra_param_reads_from_extra(self, page):
        """Dynamic params like 'turns' read from c.extra."""
        result = page.evaluate("""() => {
            return dynColCellText({
                pathType: "o-spiral", angle: 0, ccw: false, extra: { turns: 5 }
            }, "turns");
        }""")
        # turns fmt is v.toFixed(1), so 5 → "5.0"
        assert result == "5.0"

    def test_default_used_when_extra_missing(self, page):
        """If c.extra doesn't have the key, uses pdef.default."""
        result = page.evaluate("""() => {
            var pdef = PATH_PARAMS["o-spiral"].find(p => p.key === "turns");
            var text = dynColCellText({
                pathType: "o-spiral", angle: 0, ccw: false, extra: {}
            }, "turns");
            return { text, defaultVal: pdef.default };
        }""")
        # Should show formatted default (2 → "2.0")
        assert result["text"] is not None
        assert len(result["text"]) > 0


class TestApplyColVal:
    """applyColVal(c, colKey, val) applies bulk-edit values correctly."""

    def test_speed_applies_and_no_recompute(self, page):
        """spd → c.speed, returns false (no curve recompute)."""
        result = page.evaluate("""() => {
            var c = { speed: 0.5, re: 0, im: 0, extra: {} };
            var needRecompute = applyColVal(c, "spd", 0.25);
            return { speed: c.speed, recompute: needRecompute };
        }""")
        assert abs(result["speed"] - 0.25) < 1e-10
        assert result["recompute"] is False

    def test_radius_applies_and_recompute(self, page):
        """rad → c.rAbs, returns true (curve recompute needed)."""
        result = page.evaluate("""() => {
            var c = { rAbs: 1.0, re: 0, im: 0, extra: {} };
            var needRecompute = applyColVal(c, "rad", 2.5);
            return { rAbs: c.rAbs, recompute: needRecompute };
        }""")
        assert abs(result["rAbs"] - 2.5) < 1e-10
        assert result["recompute"] is True

    def test_points_applies_to_extra(self, page):
        """pts → c.extra.points, returns true."""
        result = page.evaluate("""() => {
            var c = { re: 0, im: 0, extra: {} };
            var needRecompute = applyColVal(c, "pts", 500);
            return { points: c.extra.points, recompute: needRecompute };
        }""")
        assert result["points"] == 500
        assert result["recompute"] is True

    def test_angle_std_key_applies_to_coeff(self, page):
        """angle (_STD_KEY) → c.angle directly, returns true."""
        result = page.evaluate("""() => {
            var c = { angle: 0, re: 0, im: 0, extra: {} };
            var needRecompute = applyColVal(c, "angle", 90);
            return { angle: c.angle, inExtra: c.extra.angle, recompute: needRecompute };
        }""")
        assert result["angle"] == 90
        assert result["inExtra"] is None  # NOT in extra
        assert result["recompute"] is True

    def test_ccw_std_key_applies_to_coeff(self, page):
        """ccw (_STD_KEY) → c.ccw directly, returns true."""
        result = page.evaluate("""() => {
            var c = { ccw: false, re: 0, im: 0, extra: {} };
            var needRecompute = applyColVal(c, "ccw", true);
            return { ccw: c.ccw, inExtra: c.extra.ccw, recompute: needRecompute };
        }""")
        assert result["ccw"] is True
        assert result["inExtra"] is None
        assert result["recompute"] is True

    def test_extra_param_applies_to_extra(self, page):
        """Dynamic param like 'turns' → c.extra.turns, returns true."""
        result = page.evaluate("""() => {
            var c = { re: 0, im: 0, extra: {} };
            var needRecompute = applyColVal(c, "turns", 7);
            return { turns: c.extra.turns, recompute: needRecompute };
        }""")
        assert result["turns"] == 7
        assert result["recompute"] is True

    def test_creates_extra_if_missing(self, page):
        """applyColVal creates c.extra if it doesn't exist."""
        result = page.evaluate("""() => {
            var c = { re: 0, im: 0 };
            applyColVal(c, "turns", 3);
            return { hasExtra: !!c.extra, turns: c.extra.turns };
        }""")
        assert result["hasExtra"] is True
        assert result["turns"] == 3


class TestDynColsInDOM:
    """Dynamic columns appear/disappear in the C-list DOM."""

    def test_no_dyn_headers_when_all_none(self, page):
        """With all coefficients on 'none' path, no dynamic headers appear."""
        result = page.evaluate("""() => {
            // Ensure all coefficients are 'none' path
            for (var c of coefficients) c.pathType = "none";
            refreshCoeffList();
            var hdr = document.querySelector("#coeff-list-hdr");
            var dynHdrs = hdr ? hdr.querySelectorAll(".dyn-col-hdr") : [];
            return dynHdrs.length;
        }""")
        assert result == 0

    def test_circle_adds_angle_ccw_headers(self, page):
        """Setting a coefficient to 'circle' adds ang/dir headers."""
        result = page.evaluate("""() => {
            coefficients[0].pathType = "circle";
            coefficients[0].angle = 0;
            coefficients[0].ccw = false;
            if (!coefficients[0].extra) coefficients[0].extra = {};
            refreshCoeffList();
            var hdr = document.querySelector("#coeff-list-hdr");
            var dynHdrs = hdr ? Array.from(hdr.querySelectorAll(".dyn-col-hdr")) : [];
            var labels = dynHdrs.map(el => el.textContent);
            // Restore
            coefficients[0].pathType = "none";
            refreshCoeffList();
            return labels;
        }""")
        assert "ang" in result
        assert "dir" in result

    def test_dyn_cells_match_header_count(self, page):
        """Each row has the same number of .cpick-dyn cells as there are dynamic headers."""
        result = page.evaluate("""() => {
            coefficients[0].pathType = "circle";
            coefficients[0].angle = 0;
            coefficients[0].ccw = false;
            if (!coefficients[0].extra) coefficients[0].extra = {};
            refreshCoeffList();
            var hdr = document.querySelector("#coeff-list-hdr");
            var nHeaders = hdr ? hdr.querySelectorAll(".dyn-col-hdr").length : 0;
            var rows = document.querySelectorAll("#coeff-list-scroll .cpick-row");
            var cellCounts = [];
            rows.forEach(r => cellCounts.push(r.querySelectorAll(".cpick-dyn").length));
            // Restore
            coefficients[0].pathType = "none";
            refreshCoeffList();
            return { nHeaders, cellCounts };
        }""")
        n = result["nHeaders"]
        assert n > 0
        for count in result["cellCounts"]:
            assert count == n

    def test_o_spiral_adds_turns_column(self, page):
        """Setting a coefficient to 'o-spiral' adds a 'trn' header."""
        result = page.evaluate("""() => {
            coefficients[0].pathType = "o-spiral";
            coefficients[0].ccw = false;
            if (!coefficients[0].extra) coefficients[0].extra = {};
            coefficients[0].extra.turns = 3;
            coefficients[0].extra.mult = 1.5;
            refreshCoeffList();
            var hdr = document.querySelector("#coeff-list-hdr");
            var dynHdrs = hdr ? Array.from(hdr.querySelectorAll(".dyn-col-hdr")) : [];
            var labels = dynHdrs.map(el => el.textContent);
            coefficients[0].pathType = "none";
            refreshCoeffList();
            return labels;
        }""")
        assert "trn" in result

    def test_columns_removed_when_path_cleared(self, page):
        """Changing all paths back to 'none' removes dynamic columns."""
        result = page.evaluate("""() => {
            // Set ALL coefficients to circle first
            for (var c of coefficients) {
                c.pathType = "circle";
                c.angle = 0;
                c.ccw = false;
                if (!c.extra) c.extra = {};
            }
            refreshCoeffList();
            var before = document.querySelector("#coeff-list-hdr").querySelectorAll(".dyn-col-hdr").length;
            // Now set ALL back to none
            for (var c of coefficients) c.pathType = "none";
            refreshCoeffList();
            var after = document.querySelector("#coeff-list-hdr").querySelectorAll(".dyn-col-hdr").length;
            return { before, after };
        }""")
        assert result["before"] > 0
        assert result["after"] == 0


class TestUpdateListPathCols:
    """updateListPathCols() updates dynamic cell text without full refresh."""

    def test_cell_text_updates_on_param_change(self, page):
        """Changing a param value is reflected after updateListPathCols()."""
        result = page.evaluate("""() => {
            // Need leftTab = "list" for updateListPathCols to run
            var origTab = leftTab;
            leftTab = "list";
            coefficients[0].pathType = "circle";
            coefficients[0].angle = 0.30;
            coefficients[0].ccw = false;
            if (!coefficients[0].extra) coefficients[0].extra = {};
            refreshCoeffList();
            // Change angle
            coefficients[0].angle = 0.75;
            updateListPathCols();
            // Read first row's dyn cells
            var rows = document.querySelectorAll("#coeff-list-scroll .cpick-row");
            var cells = rows[0] ? Array.from(rows[0].querySelectorAll(".cpick-dyn")) : [];
            var texts = cells.map(el => el.textContent);
            // Restore
            coefficients[0].pathType = "none";
            leftTab = origTab;
            refreshCoeffList();
            return texts;
        }""")
        assert "0.75" in result

    def test_column_set_change_triggers_full_rebuild(self, page):
        """If active columns change, updateListPathCols triggers full rebuild."""
        result = page.evaluate("""() => {
            // Start with circle (angle, ccw)
            coefficients[0].pathType = "circle";
            coefficients[0].angle = 0;
            coefficients[0].ccw = false;
            if (!coefficients[0].extra) coefficients[0].extra = {};
            refreshCoeffList();
            var before = document.querySelector("#coeff-list-hdr").querySelectorAll(".dyn-col-hdr").length;
            // Switch to o-spiral (adds mult, turns, drops angle)
            coefficients[0].pathType = "o-spiral";
            coefficients[0].extra.turns = 3;
            coefficients[0].extra.mult = 1.5;
            updateListPathCols();
            var after = document.querySelector("#coeff-list-hdr").querySelectorAll(".dyn-col-hdr").length;
            // Restore
            coefficients[0].pathType = "none";
            refreshCoeffList();
            return { before, after };
        }""")
        # o-spiral has mult, turns, ccw — circle has angle, ccw
        # After switch the column set changes so it triggers a full rebuild
        assert result["after"] >= 2
