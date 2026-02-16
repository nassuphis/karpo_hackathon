"""Tests for D-node right-click context menu — path editing on the morph panel."""

import pytest


class TestDNodeCtxPopupExists:
    """Basic DOM existence checks for the D-node context menu."""

    def test_dnode_ctx_div_exists(self, page):
        """dnode-ctx popup div should exist in the DOM."""
        result = page.evaluate("() => document.getElementById('dnode-ctx') !== null")
        assert result is True

    def test_dnode_ctx_initially_hidden(self, page):
        """dnode-ctx should not have the 'open' class initially."""
        result = page.evaluate("""() => {
            return !document.getElementById('dnode-ctx').classList.contains('open');
        }""")
        assert result is True


class TestOpenDNodeCtx:
    """openDNodeCtx() should open a context menu for D-nodes."""

    def test_open_sets_ctxDNodeIdx(self, page):
        """Opening D-node ctx should set ctxDNodeIdx to the target index."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var idx = ctxDNodeIdx;
            closeDNodeCtx(true);
            return idx;
        }""")
        assert result == 0

    def test_open_shows_popup(self, page):
        """Opening D-node ctx should add 'open' class to dnode-ctx."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var isOpen = document.getElementById('dnode-ctx').classList.contains('open');
            closeDNodeCtx(true);
            return isOpen;
        }""")
        assert result is True

    def test_open_creates_path_select(self, page):
        """Context menu should contain a path type <select>."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var sel = document.querySelector('#dnode-ctx select');
            var hasOptions = sel && sel.options.length > 1;
            closeDNodeCtx(true);
            return hasOptions;
        }""")
        assert result is True

    def test_title_shows_d_prefix(self, page):
        """Context menu title should use 'd' prefix, not 'c'."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var n = morphTargetCoeffs.length;
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var title = document.querySelector('#dnode-ctx .ctx-title span');
            var text = title ? title.textContent : '';
            closeDNodeCtx(true);
            return text;
        }""")
        assert result.startswith("d"), f"Expected title starting with 'd', got '{result}'"
        assert "trajectory" in result

    def test_snapshots_current_state(self, page):
        """Opening ctx should snapshot the D-node's path state."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[0].radius = 42;
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var snap = ctxDNodeSnapshot;
            var ok = snap && snap.pathType === "circle" && snap.radius === 42;
            closeDNodeCtx(true);
            initMorphTarget();
            return ok;
        }""")
        assert result is True


class TestCloseDNodeCtx:
    """closeDNodeCtx should revert or keep changes based on flag."""

    def test_close_revert_restores_path(self, page):
        """closeDNodeCtx(true) should revert path changes."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "none";
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            // Change path type in the menu
            morphTargetCoeffs[0].pathType = "circle";
            morphTargetCoeffs[0].radius = 50;
            closeDNodeCtx(true);  // revert
            var after = morphTargetCoeffs[0].pathType;
            initMorphTarget();
            return after;
        }""")
        assert result == "none"

    def test_close_no_revert_keeps_changes(self, page):
        """closeDNodeCtx(false) should keep the new path."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "none";
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            morphTargetCoeffs[0].pathType = "circle";
            ctxDNodeSnapshot = null;  // discard snapshot like Accept does
            closeDNodeCtx(false);
            var after = morphTargetCoeffs[0].pathType;
            initMorphTarget();
            return after;
        }""")
        assert result == "circle"

    def test_close_resets_idx(self, page):
        """After closing, ctxDNodeIdx should be -1."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            closeDNodeCtx(false);
            return ctxDNodeIdx;
        }""")
        assert result == -1


class TestDNodeCtxAccept:
    """The Accept button should commit path changes."""

    def test_accept_button_exists(self, page):
        """Context menu should have an Accept button."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var btns = document.querySelectorAll('#dnode-ctx .ctx-accept');
            var texts = [];
            btns.forEach(function(b) { texts.push(b.textContent); });
            closeDNodeCtx(true);
            return texts;
        }""")
        assert "Accept" in result

    def test_no_delete_button(self, page):
        """D-node context menu should NOT have a Delete button."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var btns = document.querySelectorAll('#dnode-ctx .ctx-accept');
            var texts = [];
            btns.forEach(function(b) { texts.push(b.textContent); });
            closeDNodeCtx(true);
            return texts;
        }""")
        assert "Delete" not in result

    def test_no_ps_button(self, page):
        """D-node context menu should NOT have a PS button (prime speed is on D-List tab)."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var btns = document.querySelectorAll('#dnode-ctx .ctx-accept');
            var texts = [];
            btns.forEach(function(b) { texts.push(b.textContent); });
            closeDNodeCtx(true);
            return texts;
        }""")
        assert "PS" not in result


class TestDNodeCtxPathChange:
    """Changing path type via context menu should update the D-node."""

    def test_change_path_type_via_select(self, page):
        """Selecting a new path in the ctx should update the D-node's pathType."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "none";
            morphTargetCoeffs[0].curve = [{re: morphTargetCoeffs[0].re, im: morphTargetCoeffs[0].im}];
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            // Change the select
            var sel = document.querySelector('#dnode-ctx select');
            sel.value = "circle";
            sel.dispatchEvent(new Event('change'));
            // Check preview applied
            var pathType = morphTargetCoeffs[0].pathType;
            var hasCurve = morphTargetCoeffs[0].curve.length > 1;
            // Accept
            ctxDNodeSnapshot = null;
            closeDNodeCtx(false);
            var result = { pathType, hasCurve };
            initMorphTarget();
            return result;
        }""")
        assert result["pathType"] == "circle"
        assert result["hasCurve"] is True

    def test_revert_undoes_path_change(self, page):
        """Closing with revert after path change should undo it."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = "none";
            morphTargetCoeffs[0].curve = [{re: morphTargetCoeffs[0].re, im: morphTargetCoeffs[0].im}];
            var evt = new MouseEvent('contextmenu', { bubbles: true, cancelable: true });
            openDNodeCtx(evt, 0);
            var sel = document.querySelector('#dnode-ctx select');
            sel.value = "circle";
            sel.dispatchEvent(new Event('change'));
            // Revert
            closeDNodeCtx(true);
            var pathType = morphTargetCoeffs[0].pathType;
            initMorphTarget();
            return pathType;
        }""")
        assert result == "none"


class TestDNodeCtxOnMorphPanel:
    """Right-clicking a D-node dot on the morph panel should open the ctx."""

    def test_contextmenu_on_morph_dot(self, page):
        """Right-clicking a morph-coeff circle should open dnode-ctx."""
        result = page.evaluate("""() => {
            initMorphTarget();
            switchLeftTab('morph');
            renderMorphPanel();
            var dot = document.querySelector('#morph-panel circle.morph-coeff');
            if (!dot) return { error: 'no dot found' };
            dot.dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true }));
            var isOpen = document.getElementById('dnode-ctx').classList.contains('open');
            var idx = ctxDNodeIdx;
            closeDNodeCtx(true);
            switchLeftTab('coeffs');
            return { isOpen, idx };
        }""")
        assert result.get("isOpen") is True
        assert result.get("idx") == 0


