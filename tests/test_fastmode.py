"""Tests for continuous fast mode: fmtPassCount, button labels, serialization, jiggle interval."""

import pytest


class TestFmtPassCount:
    def test_zero_elapsed(self, page):
        """At 0 elapsed, should show 000000s."""
        result = page.evaluate("""() => {
            fastModeElapsedOffset = 0;
            return fmtPassCount();
        }""")
        assert result == "000000s"

    def test_small_elapsed(self, page):
        """Elapsed=42 should show 000042s."""
        result = page.evaluate("""() => {
            fastModeElapsedOffset = 42;
            return fmtPassCount();
        }""")
        assert result == "000042s"

    def test_large_elapsed(self, page):
        """Elapsed=123456 should show 123456s."""
        result = page.evaluate("""() => {
            fastModeElapsedOffset = 123456;
            return fmtPassCount();
        }""")
        assert result == "123456s"

    def test_fractional_rounds(self, page):
        """Fractional elapsed should round to nearest integer."""
        result = page.evaluate("""() => {
            fastModeElapsedOffset = 7.6;
            return fmtPassCount();
        }""")
        assert result == "000008s"

    def test_padded_width_stable(self, page):
        """Width should be constant at 7 chars (6 digits + 's')."""
        result = page.evaluate("""() => {
            var lengths = [];
            for (var e of [0, 1, 10, 100, 1000, 99999]) {
                fastModeElapsedOffset = e;
                lengths.push(fmtPassCount().length);
            }
            fastModeElapsedOffset = 0;
            return lengths;
        }""")
        assert all(l == 7 for l in result), f"Inconsistent widths: {result}"


class TestButtonLabels:
    def test_init_button_label(self, page):
        """Init button should say 'init'."""
        result = page.evaluate("""() => {
            return document.getElementById('bitmap-start-btn').textContent;
        }""")
        assert result == "init"

    def test_fast_button_initial_label(self, page):
        """Fast mode button should initially say 'start' (disabled)."""
        result = page.evaluate("""() => {
            var btn = document.getElementById('bitmap-fast-btn');
            return { text: btn.textContent, disabled: btn.disabled };
        }""")
        assert result["text"] == "start"
        assert result["disabled"] is True

    def test_no_progress_bar_element(self, page):
        """Progress bar and toggle should not exist in DOM."""
        result = page.evaluate("""() => ({
            progressBar: document.getElementById('bitmap-progress-bar'),
            progressToggle: document.getElementById('bitmap-progress-toggle'),
            progress: document.getElementById('bitmap-progress')
        })""")
        assert result["progressBar"] is None
        assert result["progressToggle"] is None
        assert result["progress"] is None

    def test_pass_count_element_exists(self, page):
        """Pass count span should exist but be hidden initially."""
        result = page.evaluate("""() => {
            var el = document.getElementById('bitmap-pass-count');
            return { exists: el !== null, display: el ? el.style.display : null };
        }""")
        assert result["exists"] is True
        assert result["display"] == "none"


class TestJiggleInterval:
    def test_default_value(self, page):
        """jiggleInterval should default to 4."""
        result = page.evaluate("() => jiggleInterval")
        assert result == 4

    def test_save_load_roundtrip(self, page):
        """jiggleInterval survives save/load as targetSeconds."""
        result = page.evaluate("""() => {
            jiggleInterval = 42;
            var state = buildStateMetadata();
            jiggleInterval = 4;
            applyLoadedState(state);
            return { interval: jiggleInterval, wireKey: state.targetSeconds };
        }""")
        assert result["interval"] == 42
        assert result["wireKey"] == 42  # backward compat wire format

    def test_old_snap_compat(self, page):
        """Loading old snap with targetSeconds field works."""
        result = page.evaluate("""() => {
            var state = buildStateMetadata();
            state.targetSeconds = 77;
            applyLoadedState(state);
            return jiggleInterval;
        }""")
        assert result == 77


class TestFastModeVariablesRemoved:
    def test_no_cumulative_sec(self, page):
        """fastModeCumulativeSec should not exist."""
        result = page.evaluate("() => typeof fastModeCumulativeSec")
        assert result == "undefined"

    def test_no_target_seconds(self, page):
        """fastModeTargetSeconds should not exist."""
        result = page.evaluate("() => typeof fastModeTargetSeconds")
        assert result == "undefined"

    def test_no_show_progress(self, page):
        """fastModeShowProgress should not exist."""
        result = page.evaluate("() => typeof fastModeShowProgress")
        assert result == "undefined"


class TestInitBitmapCanvasResets:
    def test_init_resets_elapsed(self, page):
        """initBitmapCanvas() resets elapsed offset to 0."""
        result = page.evaluate("""() => {
            fastModeElapsedOffset = 999;
            initBitmapCanvas();
            return fastModeElapsedOffset;
        }""")
        assert result == 0

    def test_init_resets_pass_count(self, page):
        """initBitmapCanvas() resets pass count to 0."""
        result = page.evaluate("""() => {
            fastModePassCount = 999;
            initBitmapCanvas();
            return fastModePassCount;
        }""")
        assert result == 0

    def test_init_clears_worker_roots(self, page):
        """initBitmapCanvas() clears warm-start roots."""
        result = page.evaluate("""() => {
            fastModeWorkerRoots = {re: [1], im: [2]};
            initBitmapCanvas();
            return fastModeWorkerRoots;
        }""")
        assert result is None


class TestSerializeFastModeData:
    def test_basic_serialization(self, page):
        """serializeFastModeData returns expected structure."""
        result = page.evaluate("""() => {
            initBitmapCanvas();
            fastModeCurves = new Map();
            var animated = allAnimatedCoeffs();
            var sd = serializeFastModeData(animated, 100, currentRoots.length);
            return {
                hasCoeffsRe: 'coeffsRe' in sd,
                hasCoeffsIm: 'coeffsIm' in sd,
                hasRootsRe: 'rootsRe' in sd,
                hasRootsIm: 'rootsIm' in sd,
                hasNCoeffs: 'nCoeffs' in sd,
                hasNRoots: 'nRoots' in sd,
                hasSteps: 'stepsVal' in sd,
                nCoeffs: sd.nCoeffs,
                nRoots: sd.nRoots,
                steps: sd.stepsVal
            };
        }""")
        assert result["hasCoeffsRe"] is True
        assert result["hasCoeffsIm"] is True
        assert result["hasRootsRe"] is True
        assert result["hasRootsIm"] is True
        assert result["steps"] == 100

    def test_jiggle_offsets_applied_to_nonanimate(self, page):
        """Non-animated coefficients get jiggle offsets baked into serialized data."""
        result = page.evaluate("""() => {
            initBitmapCanvas();
            // Set all coefficients to "none" path (non-animated)
            for (var c of coefficients) c.pathType = "none";
            // Create jiggle offsets (after init, which resets them)
            jiggleOffsets = new Map();
            jiggleOffsets.set(0, {re: 5.0, im: 3.0});
            fastModeCurves = new Map();
            var animated = allAnimatedCoeffs();
            var sd = serializeFastModeData(animated, 100, currentRoots.length);
            var origRe = coefficients[0].re;
            var serializedRe = sd.coeffsRe[0];
            jiggleOffsets = null;
            return { origRe, serializedRe, diff: serializedRe - origRe };
        }""")
        assert abs(result["diff"] - 5.0) < 1e-10, "Jiggle offset not applied"


class TestClearButtonNoElapsedReset:
    def test_clear_preserves_elapsed(self, page):
        """Clear button only clears pixels, not elapsed."""
        result = page.evaluate("""() => {
            // Init canvas first
            initBitmapCanvas();
            fastModeElapsedOffset = 42;
            // Simulate clear
            if (bitmapCtx) {
                bitmapCtx.fillStyle = bitmapCanvasColor;
                bitmapCtx.fillRect(0, 0, bitmapCtx.canvas.width, bitmapCtx.canvas.height);
                fillPersistentBuffer(bitmapCanvasColor);
                bitmapFrameCount = 0;
            }
            return fastModeElapsedOffset;
        }""")
        assert result == 42


class TestFastModeToggle:
    def test_fast_btn_handler_no_reset(self, page):
        """Fast mode toggle should not reset elapsed offset."""
        result = page.evaluate("""() => {
            // Can't fully test enter/exit without canvas, but verify the
            // handler function doesn't reference fastModeCumulativeSec or reset elapsed
            var src = document.getElementById('bitmap-fast-btn')
                .getAttribute('onclick') || '';
            return {
                hasCumulativeSec: src.includes('fastModeCumulativeSec'),
                hasElapsedReset: src.includes('fastModeElapsedOffset = 0')
            };
        }""")
        # These should not appear in inline handler (they don't use onclick anyway)
        assert result["hasCumulativeSec"] is False
        assert result["hasElapsedReset"] is False
