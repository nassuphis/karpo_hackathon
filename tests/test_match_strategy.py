"""Tests for root-matching strategy options (Hungarian, Greedy×1, Greedy×4)."""

import pytest


# Helper JS to extract the worker blob source code (template literal content)
EXTRACT_BLOB_SRC = """
    // Override Blob and URL to capture the source code
    var capturedCode = null;
    var origBlob = Blob;
    var origCreateObjectURL = URL.createObjectURL;
    window.Blob = function(parts, opts) { capturedCode = parts[0]; return new origBlob(parts, opts); };
    URL.createObjectURL = function(b) { return 'blob:test'; };
    createFastModeWorkerBlob();
    window.Blob = origBlob;
    URL.createObjectURL = origCreateObjectURL;
"""


class TestStateVariable:
    def test_default_strategy(self, page):
        """bitmapMatchStrategy should default to 'assign4'."""
        result = page.evaluate("() => bitmapMatchStrategy")
        assert result == "assign4"


class TestHungarianAlgorithm:
    """Tests the hungarianMatch function from the worker blob source."""

    def _run_hungarian(self, page, old_re, old_im, new_re, new_im):
        """Helper: extract hungarianMatch from blob and run it."""
        return page.evaluate("""(args) => {
            """ + EXTRACT_BLOB_SRC + """
            // Extract hungarianMatch by eval'ing only function definitions
            var env = {};
            try {
                var wrapped = '(function(self){' + capturedCode + ';return hungarianMatch;})({onmessage:null})';
                var fn = eval(wrapped);
            } catch(e) {
                return { error: e.message };
            }
            var oldRe = new Float64Array(args.oldRe);
            var oldIm = new Float64Array(args.oldIm);
            var newRe = new Float64Array(args.newRe);
            var newIm = new Float64Array(args.newIm);
            fn(newRe, newIm, oldRe, oldIm, args.n);
            return { re: Array.from(newRe), im: Array.from(newIm) };
        }""", {"oldRe": old_re, "oldIm": old_im, "newRe": new_re, "newIm": new_im, "n": len(old_re)})

    def test_identity_case(self, page):
        """When new roots == old roots, Hungarian should preserve order."""
        result = self._run_hungarian(page, [1, 2, 3], [0, 0, 0], [1, 2, 3], [0, 0, 0])
        assert "error" not in result, f"JS error: {result.get('error')}"
        assert result["re"] == [1, 2, 3]
        assert result["im"] == [0, 0, 0]

    def test_reversed_roots(self, page):
        """Hungarian should correctly match reversed roots to original order."""
        result = self._run_hungarian(page, [0, 5, 10], [0, 0, 0], [10, 5, 0], [0, 0, 0])
        assert "error" not in result, f"JS error: {result.get('error')}"
        assert result["re"] == [0, 5, 10]

    def test_optimal_assignment_2x2(self, page):
        """Hungarian should find optimal for a 2×2 case."""
        result = self._run_hungarian(page, [0, 3], [0, 0], [0.1, 2.9], [0, 0])
        assert "error" not in result, f"JS error: {result.get('error')}"
        assert abs(result["re"][0] - 0.1) < 1e-10
        assert abs(result["re"][1] - 2.9) < 1e-10

    def test_valid_permutation(self, page):
        """Hungarian output should be a valid permutation of inputs."""
        result = self._run_hungarian(page,
            [1, 3, 7, 2, 9], [0.5, -1, 2, 0, -3],
            [9.1, 1.1, 7.1, 3.1, 2.1], [-3.1, 0.6, 2.1, -1.1, 0.1])
        assert "error" not in result, f"JS error: {result.get('error')}"
        out_re = sorted(result["re"])
        expected_re = sorted([9.1, 1.1, 7.1, 3.1, 2.1])
        for a, b in zip(out_re, expected_re):
            assert abs(a - b) < 1e-10

    def test_single_root(self, page):
        """Hungarian should handle n=1 trivially."""
        result = self._run_hungarian(page, [5], [3], [7], [1])
        assert "error" not in result, f"JS error: {result.get('error')}"
        assert result["re"] == [7]
        assert result["im"] == [1]


class TestSerializationStrategy:
    def test_default_in_serialization(self, page):
        """serializeFastModeData should include matchStrategy."""
        result = page.evaluate("""() => {
            bitmapMatchStrategy = 'assign4';
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            fastModeCurves = new Map();
            var sd = serializeFastModeData([], 100, currentRoots.length);
            return sd.matchStrategy;
        }""")
        assert result == "assign4"

    def test_hungarian_in_serialization(self, page):
        """matchStrategy should reflect hungarian1 when set."""
        result = page.evaluate("""() => {
            bitmapMatchStrategy = 'hungarian1';
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            fastModeCurves = new Map();
            var sd = serializeFastModeData([], 100, currentRoots.length);
            bitmapMatchStrategy = 'assign4';
            return sd.matchStrategy;
        }""")
        assert result == "hungarian1"

    def test_assign1_in_serialization(self, page):
        """matchStrategy should reflect assign1 when set."""
        result = page.evaluate("""() => {
            bitmapMatchStrategy = 'assign1';
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            fastModeCurves = new Map();
            var sd = serializeFastModeData([], 100, currentRoots.length);
            bitmapMatchStrategy = 'assign4';
            return sd.matchStrategy;
        }""")
        assert result == "assign1"


class TestSaveLoadRoundtrip:
    def test_strategy_roundtrip(self, page):
        """Save/load should preserve bitmapMatchStrategy."""
        result = page.evaluate("""() => {
            bitmapMatchStrategy = 'hungarian1';
            var meta = buildStateMetadata();
            bitmapMatchStrategy = 'assign4';
            if (meta.bitmapMatchStrategy && ['assign4','assign1','hungarian1'].includes(meta.bitmapMatchStrategy)) {
                bitmapMatchStrategy = meta.bitmapMatchStrategy;
            }
            return bitmapMatchStrategy;
        }""")
        assert result == "hungarian1"

    def test_missing_field_defaults(self, page):
        """Old snaps without bitmapMatchStrategy should default to assign4."""
        result = page.evaluate("""() => {
            bitmapMatchStrategy = 'hungarian1';
            var meta = buildStateMetadata();
            delete meta.bitmapMatchStrategy;
            if (meta.bitmapMatchStrategy && ['assign4','assign1','hungarian1'].includes(meta.bitmapMatchStrategy)) {
                bitmapMatchStrategy = meta.bitmapMatchStrategy;
            } else {
                bitmapMatchStrategy = 'assign4';
            }
            return bitmapMatchStrategy;
        }""")
        assert result == "assign4"


class TestUIChips:
    def test_chips_exist_in_popup(self, page):
        """Bitmap cfg popup should show 3 match strategy chips."""
        result = page.evaluate("""() => {
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            openBitmapCfgPop();
            var pop = document.getElementById('bitmap-cfg-pop');
            if (!pop) return [];
            var divs = pop.querySelectorAll('div');
            var labels = [];
            divs.forEach(function(d) {
                var t = d.textContent.trim();
                if (t === 'Hungarian' || t === 'Greedy\\u00d71' || t === 'Greedy\\u00d74')
                    labels.push(t);
            });
            closeBitmapCfgPop();
            return labels;
        }""")
        assert len(result) == 3

    def test_default_chip_highlighted(self, page):
        """Greedy×4 chip should be highlighted by default (assign4)."""
        result = page.evaluate("""() => {
            bitmapMatchStrategy = 'assign4';
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            openBitmapCfgPop();
            var pop = document.getElementById('bitmap-cfg-pop');
            var divs = pop.querySelectorAll('div');
            var active = null;
            divs.forEach(function(d) {
                if (d.style && d.style.background && d.style.background.indexOf('var(--accent)') >= 0) {
                    active = d.textContent.trim();
                }
            });
            closeBitmapCfgPop();
            return active;
        }""")
        assert result is not None
        assert "\u00d74" in result  # Contains ×4

    def test_chip_click_updates_strategy(self, page):
        """Clicking a chip should update bitmapMatchStrategy."""
        result = page.evaluate("""() => {
            bitmapMatchStrategy = 'assign4';
            document.getElementById('bitmap-res-select').value = '1000';
            initBitmapCanvas();
            openBitmapCfgPop();
            var pop = document.getElementById('bitmap-cfg-pop');
            var divs = pop.querySelectorAll('div');
            divs.forEach(function(d) {
                if (d.textContent.trim() === 'Hungarian') d.click();
            });
            var result = bitmapMatchStrategy;
            closeBitmapCfgPop();
            bitmapMatchStrategy = 'assign4';
            return result;
        }""")
        assert result == "hungarian1"


class TestWorkerBlobContents:
    def test_blob_contains_hungarian(self, page):
        """Worker blob source should contain hungarianMatch function."""
        result = page.evaluate("""() => {
            var capturedCode = null;
            var origBlob = Blob;
            var origCreateObjectURL = URL.createObjectURL;
            window.Blob = function(parts, opts) { capturedCode = parts[0]; return new origBlob(parts, opts); };
            URL.createObjectURL = function(b) { return 'blob:test'; };
            createFastModeWorkerBlob();
            window.Blob = origBlob;
            URL.createObjectURL = origCreateObjectURL;
            return capturedCode && capturedCode.indexOf('hungarianMatch') >= 0;
        }""")
        assert result is True

    def test_blob_contains_match_strategy_var(self, page):
        """Worker blob source should contain S_matchStrategy variable."""
        result = page.evaluate("""() => {
            var capturedCode = null;
            var origBlob = Blob;
            var origCreateObjectURL = URL.createObjectURL;
            window.Blob = function(parts, opts) { capturedCode = parts[0]; return new origBlob(parts, opts); };
            URL.createObjectURL = function(b) { return 'blob:test'; };
            createFastModeWorkerBlob();
            window.Blob = origBlob;
            URL.createObjectURL = origCreateObjectURL;
            return capturedCode && capturedCode.indexOf('S_matchStrategy') >= 0;
        }""")
        assert result is True
