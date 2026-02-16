"""Tests for the C-Nodes trajectory editor (anim-bar): preview/revert pattern,
PS button removal, and button naming."""

import pytest


class TestAnimBarButtons:
    """Verify button text and absence of PS in the anim-bar."""

    def test_update_button_text(self, page):
        """Update button should read 'Update Whole Selection'."""
        result = page.evaluate("""() => {
            return document.getElementById('sel2path-btn').textContent;
        }""")
        assert result == "Update Whole Selection"

    def test_no_ps_button_in_bar(self, page):
        """Anim-bar should not contain a PS button after controls are built."""
        result = page.evaluate("""() => {
            // Select a coefficient to enable bar, pick circle (has speed slider)
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            animPathSel.value = "circle";
            rebuildBarControls("circle");
            barCurrentPath = "";  // force rebuild
            rebuildBarControls("circle");
            var psBtn = document.getElementById('bar-ps-btn');
            // Cleanup
            selectedCoeffs.clear();
            updateAnimBar();
            return psBtn === null;
        }""")
        assert result is True


class TestAnimBarPreview:
    """Slider interactions should preview without committing."""

    def test_preview_snapshots_on_first_interaction(self, page):
        """Moving a slider should create barSnapshots."""
        result = page.evaluate("""() => {
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            var before = barSnapshots;
            // Simulate a preview
            animPathSel.value = "circle";
            previewBarToSelection();
            var after = barSnapshots !== null;
            // Cleanup
            revertBarPreview();
            selectedCoeffs.clear();
            updateAnimBar();
            return { before: before, after: after };
        }""")
        assert result["before"] is None
        assert result["after"] is True

    def test_preview_changes_coefficient_path(self, page):
        """Preview should visually change the coefficient's path type."""
        result = page.evaluate("""() => {
            var origType = coefficients[0].pathType;
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            animPathSel.value = "circle";
            previewBarToSelection();
            var previewType = coefficients[0].pathType;
            // Cleanup
            revertBarPreview();
            selectedCoeffs.clear();
            updateAnimBar();
            return { origType, previewType };
        }""")
        assert result["previewType"] == "circle"

    def test_preview_preserves_curve_in_snapshot(self, page):
        """Snapshot should contain the original curve data."""
        result = page.evaluate("""() => {
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            var origCurveLen = coefficients[0].curve.length;
            animPathSel.value = "circle";
            previewBarToSelection();
            var snapCurveLen = barSnapshots[0] ? barSnapshots[0].curve.length : -1;
            // Cleanup
            revertBarPreview();
            selectedCoeffs.clear();
            updateAnimBar();
            return { origCurveLen, snapCurveLen };
        }""")
        assert result["origCurveLen"] == result["snapCurveLen"]


class TestAnimBarRevert:
    """Reverting should restore original coefficient state."""

    def test_revert_restores_path_type(self, page):
        """revertBarPreview() should restore original pathType."""
        result = page.evaluate("""() => {
            var origType = coefficients[0].pathType;
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            animPathSel.value = "circle";
            previewBarToSelection();
            revertBarPreview();
            var afterType = coefficients[0].pathType;
            selectedCoeffs.clear();
            updateAnimBar();
            return { origType, afterType };
        }""")
        assert result["origType"] == result["afterType"]

    def test_revert_clears_snapshots(self, page):
        """After revert, barSnapshots should be null."""
        result = page.evaluate("""() => {
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            animPathSel.value = "circle";
            previewBarToSelection();
            revertBarPreview();
            var cleared = barSnapshots === null;
            selectedCoeffs.clear();
            updateAnimBar();
            return cleared;
        }""")
        assert result is True

    def test_selection_change_reverts_preview(self, page):
        """Changing selection while preview active should revert."""
        result = page.evaluate("""() => {
            var origType = coefficients[0].pathType;
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            animPathSel.value = "circle";
            previewBarToSelection();
            // Change selection — triggers updateAnimBar which reverts
            selectedCoeffs.clear();
            updateAnimBar();
            var afterType = coefficients[0].pathType;
            return { origType, afterType };
        }""")
        assert result["origType"] == result["afterType"]

    def test_revert_noop_when_no_preview(self, page):
        """revertBarPreview() with no active preview should be a no-op."""
        result = page.evaluate("""() => {
            barSnapshots = null;
            revertBarPreview();
            return barSnapshots === null;
        }""")
        assert result is True


class TestAnimBarCommit:
    """Committing should keep changes and clear snapshots."""

    def test_commit_keeps_new_path(self, page):
        """commitBarPreview() should keep the previewed path type."""
        result = page.evaluate("""() => {
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            animPathSel.value = "circle";
            previewBarToSelection();
            commitBarPreview();
            var pathType = coefficients[0].pathType;
            var snapsNull = barSnapshots === null;
            // Cleanup — restore none
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            selectedCoeffs.clear();
            updateAnimBar();
            return { pathType, snapsNull };
        }""")
        assert result["pathType"] == "circle"
        assert result["snapsNull"] is True

    def test_update_button_commits_preview(self, page):
        """Clicking 'Update Whole Selection' with active preview should commit."""
        result = page.evaluate("""() => {
            selectedCoeffs.add(0);
            lastSelectedCoeff = 0;
            updateAnimBar();
            animPathSel.value = "circle";
            previewBarToSelection();
            // Click the update button
            document.getElementById('sel2path-btn').click();
            var pathType = coefficients[0].pathType;
            var snapsNull = barSnapshots === null;
            // Cleanup
            coefficients[0].pathType = "none";
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            selectedCoeffs.clear();
            updateAnimBar();
            return { pathType, snapsNull };
        }""")
        assert result["pathType"] == "circle"
        assert result["snapsNull"] is True


class TestCListEditorButtons:
    """C-List curve editor should have no PS, 'Update This', or node cycler."""

    def test_no_ps_button(self, page):
        """C-List curve editor should not have a PS button."""
        result = page.evaluate("""() => {
            return document.getElementById('lce-ps-btn');
        }""")
        assert result is None

    def test_no_update_one_button(self, page):
        """C-List curve editor should not have an 'Update This' button."""
        result = page.evaluate("""() => {
            return document.getElementById('lce-update-one');
        }""")
        assert result is None

    def test_no_prev_button(self, page):
        """C-List curve editor should not have a prev cycler button."""
        result = page.evaluate("""() => {
            return document.getElementById('lce-prev');
        }""")
        assert result is None

    def test_no_next_button(self, page):
        """C-List curve editor should not have a next cycler button."""
        result = page.evaluate("""() => {
            return document.getElementById('lce-next');
        }""")
        assert result is None

    def test_no_coeff_name_label(self, page):
        """C-List curve editor should not have a coefficient name label."""
        result = page.evaluate("""() => {
            return document.getElementById('lce-coeff-name');
        }""")
        assert result is None

    def test_update_sel_button_exists(self, page):
        """C-List curve editor should have 'Update Whole Selection' button."""
        result = page.evaluate("""() => {
            var btn = document.getElementById('lce-update-sel');
            return btn ? btn.textContent : null;
        }""")
        assert result == "Update Whole Selection"

    def test_path_dropdown_exists(self, page):
        """C-List curve editor should still have the path type dropdown."""
        result = page.evaluate("""() => {
            var sel = document.getElementById('lce-path-sel');
            return sel !== null && sel.tagName === 'SELECT';
        }""")
        assert result is True


class TestDListEditorButtons:
    """D-List curve editor should have no PS, 'Update This', or node cycler."""

    def test_no_ps_button(self, page):
        """D-List curve editor should not have a PS button."""
        result = page.evaluate("""() => {
            return document.getElementById('dle-ps-btn');
        }""")
        assert result is None

    def test_no_update_one_button(self, page):
        """D-List curve editor should not have an 'Update This' button."""
        result = page.evaluate("""() => {
            return document.getElementById('dle-update-one');
        }""")
        assert result is None

    def test_no_prev_button(self, page):
        """D-List curve editor should not have a prev cycler button."""
        result = page.evaluate("""() => {
            return document.getElementById('dle-prev');
        }""")
        assert result is None

    def test_no_next_button(self, page):
        """D-List curve editor should not have a next cycler button."""
        result = page.evaluate("""() => {
            return document.getElementById('dle-next');
        }""")
        assert result is None

    def test_no_coeff_name_label(self, page):
        """D-List curve editor should not have a coefficient name label."""
        result = page.evaluate("""() => {
            return document.getElementById('dle-coeff-name');
        }""")
        assert result is None

    def test_update_sel_button_exists(self, page):
        """D-List curve editor should have 'Update Whole Selection' button."""
        result = page.evaluate("""() => {
            var btn = document.getElementById('dle-update-sel');
            return btn ? btn.textContent : null;
        }""")
        assert result == "Update Whole Selection"

    def test_path_dropdown_exists(self, page):
        """D-List curve editor should still have the path type dropdown."""
        result = page.evaluate("""() => {
            var sel = document.getElementById('dle-path-sel');
            return sel !== null && sel.tagName === 'SELECT';
        }""")
        assert result is True


class TestCListEditorFunction:
    """C-List curve editor should apply settings to entire selection."""

    def test_update_sel_applies_to_all_selected(self, page):
        """Update Whole Selection should apply path to all selected coefficients."""
        result = page.evaluate("""() => {
            // Select first two coefficients
            selectedCoeffs.add(0);
            selectedCoeffs.add(1);
            switchLeftTab('list');
            refreshListCurveEditor();

            // Set path to circle and click update
            var sel = document.getElementById('lce-path-sel');
            sel.value = 'circle';
            sel.dispatchEvent(new Event('change'));
            document.getElementById('lce-update-sel').click();

            var p0 = coefficients[0].pathType;
            var p1 = coefficients[1].pathType;

            // Cleanup
            coefficients[0].pathType = 'none';
            coefficients[0].curve = [{re: coefficients[0].re, im: coefficients[0].im}];
            coefficients[1].pathType = 'none';
            coefficients[1].curve = [{re: coefficients[1].re, im: coefficients[1].im}];
            selectedCoeffs.clear();
            switchLeftTab('coeffs');
            return { p0, p1 };
        }""")
        assert result["p0"] == "circle"
        assert result["p1"] == "circle"

    def test_editor_shows_first_selected_path(self, page):
        """Editor dropdown should reflect first selected coefficient's path type."""
        result = page.evaluate("""() => {
            // Set coeff 1 to circle
            coefficients[1].pathType = 'circle';
            coefficients[1].radius = 50;
            coefficients[1].speed = 1;
            coefficients[1].curve = computeCurve(
                coefficients[1].re, coefficients[1].im, 'circle',
                coefficients[1].radius / 100 * coeffExtent(), 0, {});

            // Select only coeff 1
            selectedCoeffs.add(1);
            switchLeftTab('list');
            refreshListCurveEditor();

            var shown = document.getElementById('lce-path-sel').value;

            // Cleanup
            coefficients[1].pathType = 'none';
            coefficients[1].curve = [{re: coefficients[1].re, im: coefficients[1].im}];
            selectedCoeffs.clear();
            switchLeftTab('coeffs');
            return shown;
        }""")
        assert result == "circle"


class TestDListEditorFunction:
    """D-List curve editor should apply settings to entire selection."""

    def test_update_sel_applies_to_all_selected(self, page):
        """Update Whole Selection should apply path to all selected D-nodes."""
        result = page.evaluate("""() => {
            initMorphTarget();
            selectedMorphCoeffs.add(0);
            selectedMorphCoeffs.add(1);
            switchLeftTab('dlist');
            refreshDListCurveEditor();

            var sel = document.getElementById('dle-path-sel');
            sel.value = 'circle';
            sel.dispatchEvent(new Event('change'));
            document.getElementById('dle-update-sel').click();

            var p0 = morphTargetCoeffs[0].pathType;
            var p1 = morphTargetCoeffs[1].pathType;

            // Cleanup
            selectedMorphCoeffs.clear();
            initMorphTarget();
            switchLeftTab('coeffs');
            return { p0, p1 };
        }""")
        assert result["p0"] == "circle"
        assert result["p1"] == "circle"

    def test_editor_shows_first_selected_path(self, page):
        """Editor dropdown should reflect first selected D-node's path type."""
        result = page.evaluate("""() => {
            initMorphTarget();
            morphTargetCoeffs[0].pathType = 'spiral';
            morphTargetCoeffs[0].radius = 50;
            morphTargetCoeffs[0].speed = 1;
            morphTargetCoeffs[0].curve = computeCurve(
                morphTargetCoeffs[0].re, morphTargetCoeffs[0].im, 'spiral',
                morphTargetCoeffs[0].radius / 100 * coeffExtent(), 0, {});

            selectedMorphCoeffs.add(0);
            switchLeftTab('dlist');
            refreshDListCurveEditor();

            var shown = document.getElementById('dle-path-sel').value;

            // Cleanup
            selectedMorphCoeffs.clear();
            initMorphTarget();
            switchLeftTab('coeffs');
            return shown;
        }""")
        assert result == "spiral"
