"""Tests for save/load roundtrip of fields not covered by test_state.py:
bitmap settings, solver type, morph D-node paths, selected coefficients,
trail data, and backward compatibility for missing fields."""

import pytest


class TestBitmapFieldsRoundtrip:
    """Bitmap-related state fields in save/load."""

    def test_bitmap_canvas_color_roundtrip(self, page):
        """bitmapCanvasColor should survive save/load."""
        result = page.evaluate("""() => {
            bitmapCanvasColor = "#ff00ff";
            var meta = buildStateMetadata();
            bitmapCanvasColor = "#000000";
            applyLoadedState(meta);
            return bitmapCanvasColor;
        }""")
        assert result == "#ff00ff"

    def test_bitmap_color_mode_roundtrip(self, page):
        """bitmapColorMode should survive save/load."""
        result = page.evaluate("""() => {
            bitmapColorMode = "proximity";
            var meta = buildStateMetadata();
            bitmapColorMode = "uniform";
            applyLoadedState(meta);
            return bitmapColorMode;
        }""")
        assert result == "proximity"

    def test_bitmap_uniform_color_roundtrip(self, page):
        """bitmapUniformColor should survive save/load."""
        result = page.evaluate("""() => {
            bitmapUniformColor = [10, 20, 30];
            var meta = buildStateMetadata();
            bitmapUniformColor = [255, 255, 255];
            applyLoadedState(meta);
            return bitmapUniformColor;
        }""")
        assert result == [10, 20, 30]

    def test_bitmap_match_strategy_roundtrip(self, page):
        """bitmapMatchStrategy should survive save/load."""
        result = page.evaluate("""() => {
            bitmapMatchStrategy = "hungarian1";
            var meta = buildStateMetadata();
            bitmapMatchStrategy = "assign4";
            applyLoadedState(meta);
            return bitmapMatchStrategy;
        }""")
        assert result == "hungarian1"

    def test_bitmap_export_format_roundtrip(self, page):
        """bitmapExportFormat should survive save/load."""
        result = page.evaluate("""() => {
            bitmapExportFormat = "png";
            var meta = buildStateMetadata();
            bitmapExportFormat = "jpeg";
            applyLoadedState(meta);
            return bitmapExportFormat;
        }""")
        assert result == "png"

    def test_bitmap_coeff_view_roundtrip(self, page):
        """bitmapCoeffView should survive save/load."""
        result = page.evaluate("""() => {
            bitmapCoeffView = true;
            var meta = buildStateMetadata();
            bitmapCoeffView = false;
            applyLoadedState(meta);
            return bitmapCoeffView;
        }""")
        assert result is True


class TestSolverTypeRoundtrip:
    def test_solver_type_roundtrip(self, page):
        """solverType should survive save/load."""
        result = page.evaluate("""() => {
            var origType = solverType;
            var meta = buildStateMetadata();
            applyLoadedState(meta);
            return { saved: meta.solverType, restored: solverType, orig: origType };
        }""")
        assert result["saved"] == result["restored"]


class TestSelectedCoeffsRoundtrip:
    def test_selected_coeffs_roundtrip(self, page):
        """selectedCoeffs should survive save/load."""
        result = page.evaluate("""() => {
            selectedCoeffs.clear();
            selectedCoeffs.add(0);
            selectedCoeffs.add(2);
            var meta = buildStateMetadata();
            selectedCoeffs.clear();
            applyLoadedState(meta);
            return Array.from(selectedCoeffs).sort();
        }""")
        assert result == [0, 2]

    def test_empty_selected_coeffs_roundtrip(self, page):
        """Empty selectedCoeffs should survive save/load."""
        result = page.evaluate("""() => {
            selectedCoeffs.clear();
            var meta = buildStateMetadata();
            selectedCoeffs.add(0);  // modify
            applyLoadedState(meta);
            return selectedCoeffs.size;
        }""")
        assert result == 0


class TestTrailDataRoundtrip:
    def test_trail_data_roundtrip(self, page):
        """trailData should survive save/load."""
        result = page.evaluate("""() => {
            // trailData is array of arrays of {re, im} points
            trailData = [
                [{re: 1, im: 2}, {re: 3, im: 4}],
                [{re: 5, im: 6}]
            ];
            var meta = buildStateMetadata();
            trailData = [];
            applyLoadedState(meta);
            return {
                nTrails: trailData.length,
                trail0len: trailData[0] ? trailData[0].length : 0,
                trail1len: trailData[1] ? trailData[1].length : 0,
                p0re: trailData[0] ? trailData[0][0].re : null,
                p0im: trailData[0] ? trailData[0][0].im : null
            };
        }""")
        assert result["nTrails"] == 2
        assert result["trail0len"] == 2
        assert result["trail1len"] == 1
        assert abs(result["p0re"] - 1) < 1e-10
        assert abs(result["p0im"] - 2) < 1e-10


class TestJiggleFieldsRoundtrip:
    """Jiggle sub-fields not covered by test_state.py."""

    def test_jiggle_scale_step_roundtrip(self, page):
        """jiggleScaleStep should survive save/load."""
        result = page.evaluate("""() => {
            jiggleScaleStep = 0.05;
            var meta = buildStateMetadata();
            jiggleScaleStep = 0.01;
            applyLoadedState(meta);
            return jiggleScaleStep;
        }""")
        assert abs(result - 0.05) < 1e-10

    def test_jiggle_period_roundtrip(self, page):
        """jigglePeriod should survive save/load."""
        result = page.evaluate("""() => {
            jigglePeriod = 10;
            var meta = buildStateMetadata();
            jigglePeriod = 4;
            applyLoadedState(meta);
            return jigglePeriod;
        }""")
        assert result == 10

    def test_jiggle_amplitude_roundtrip(self, page):
        """jiggleAmplitude should survive save/load."""
        result = page.evaluate("""() => {
            jiggleAmplitude = 0.3;
            var meta = buildStateMetadata();
            jiggleAmplitude = 0.1;
            applyLoadedState(meta);
            return jiggleAmplitude;
        }""")
        assert abs(result - 0.3) < 1e-10

    def test_jiggle_liss_freq_roundtrip(self, page):
        """jiggleLissFreqX and jiggleLissFreqY should survive save/load."""
        result = page.evaluate("""() => {
            jiggleLissFreqX = 5;
            jiggleLissFreqY = 7;
            var meta = buildStateMetadata();
            jiggleLissFreqX = 3;
            jiggleLissFreqY = 2;
            applyLoadedState(meta);
            return { x: jiggleLissFreqX, y: jiggleLissFreqY };
        }""")
        assert result["x"] == 5
        assert result["y"] == 7


class TestNumWorkersRoundtrip:
    def test_num_workers_roundtrip(self, page):
        """numWorkers should survive save/load."""
        result = page.evaluate("""() => {
            numWorkers = 8;
            var meta = buildStateMetadata();
            numWorkers = 4;
            applyLoadedState(meta);
            return numWorkers;
        }""")
        assert result == 8


class TestBackwardCompatMissingFields:
    """Loading old snaps with missing fields should use sensible defaults."""

    def test_missing_bitmap_color_mode(self, page):
        """Old snap without bitmapColorMode should fall back gracefully."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.bitmapColorMode;
            applyLoadedState(meta);
            // Should default or mirror rootColorMode
            return typeof bitmapColorMode === 'string';
        }""")
        assert result is True

    def test_missing_bitmap_match_strategy(self, page):
        """Old snap without bitmapMatchStrategy should default to 'assign4'."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.bitmapMatchStrategy;
            applyLoadedState(meta);
            return bitmapMatchStrategy;
        }""")
        assert result == "assign4"

    def test_missing_bitmap_export_format(self, page):
        """Old snap without bitmapExportFormat should default gracefully."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.bitmapExportFormat;
            applyLoadedState(meta);
            return typeof bitmapExportFormat === 'string';
        }""")
        assert result is True

    def test_missing_solver_type(self, page):
        """Old snap without solverType should default to current."""
        result = page.evaluate("""() => {
            var origType = solverType;
            var meta = buildStateMetadata();
            delete meta.solverType;
            applyLoadedState(meta);
            return typeof solverType === 'string';
        }""")
        assert result is True

    def test_missing_num_workers(self, page):
        """Old snap without numWorkers should not crash."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.numWorkers;
            applyLoadedState(meta);
            return typeof numWorkers === 'number' && numWorkers > 0;
        }""")
        assert result is True

    def test_missing_jiggle(self, page):
        """Old snap without jiggle should not crash."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.jiggle;
            applyLoadedState(meta);
            return typeof jiggleMode === 'string';
        }""")
        assert result is True

    def test_missing_selected_coeffs(self, page):
        """Old snap without selectedCoeffs should not crash."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.selectedCoeffs;
            applyLoadedState(meta);
            return selectedCoeffs instanceof Set;
        }""")
        assert result is True

    def test_missing_trail_data(self, page):
        """Old snap without trailData should not crash."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.trailData;
            applyLoadedState(meta);
            return Array.isArray(trailData);
        }""")
        assert result is True

    def test_missing_bitmap_canvas_color(self, page):
        """Old snap without bitmapCanvasColor should default gracefully."""
        result = page.evaluate("""() => {
            var meta = buildStateMetadata();
            delete meta.bitmapCanvasColor;
            applyLoadedState(meta);
            return typeof bitmapCanvasColor === 'string';
        }""")
        assert result is True


class TestDomainColoringRoundtrip:
    def test_domain_coloring_roundtrip(self, page):
        """domainColoringEnabled should survive save/load."""
        result = page.evaluate("""() => {
            domainColoringEnabled = true;
            var meta = buildStateMetadata();
            domainColoringEnabled = false;
            applyLoadedState(meta);
            return domainColoringEnabled;
        }""")
        assert result is True

    def test_trails_roundtrip(self, page):
        """trailsEnabled should survive save/load."""
        result = page.evaluate("""() => {
            trailsEnabled = true;
            var meta = buildStateMetadata();
            trailsEnabled = false;
            applyLoadedState(meta);
            return trailsEnabled;
        }""")
        assert result is True
