"""End-to-end integration tests — loading snaps, fast mode, determinism."""

import json
import pytest
from pathlib import Path


SNAP_DIR = Path(__file__).parent.parent / "snaps"


class TestLoadSnap:
    def test_load_bug1(self, page):
        snap_path = SNAP_DIR / "bug1.json"
        if not snap_path.exists():
            pytest.skip("bug1.json not found")

        snap_data = json.loads(snap_path.read_text())
        degree = page.evaluate("""(snap) => {
            applyLoadedState(snap);
            return coefficients.length - 1;
        }""", snap_data)
        expected_degree = snap_data.get("degree", len(snap_data.get("coefficients", [])) - 1)
        assert degree == expected_degree, f"Loaded degree {degree}, expected {expected_degree}"


class TestSolverDeterminism:
    def test_repeated_solve_identical(self, page):
        """Solve same polynomial 10 times — results must be bit-identical."""
        results = page.evaluate("""() => {
            var coeffs = [
                {re: 1, im: 0}, {re: 0, im: 0}, {re: 0, im: 0},
                {re: 0, im: 0}, {re: 0, im: 0}, {re: -1, im: 0}
            ];  // z^5 - 1
            var all = [];
            for (var t = 0; t < 10; t++) {
                var roots = solveRootsEA(coeffs, null, null);
                all.push(roots.map(r => [r.re, r.im]));
            }
            // Compare all to first
            for (var t = 1; t < 10; t++) {
                for (var i = 0; i < all[0].length; i++) {
                    if (all[t][i][0] !== all[0][i][0] || all[t][i][1] !== all[0][i][1]) {
                        return {identical: false, trial: t, root: i};
                    }
                }
            }
            return {identical: true};
        }""")
        assert results["identical"], f"Mismatch at trial {results.get('trial')}, root {results.get('root')}"


class TestFastModePixels:
    def test_fast_mode_produces_pixels(self, page):
        """Start fast mode briefly, verify some pixels were painted."""
        has_pixels = page.evaluate("""() => {
            return new Promise(resolve => {
                // Set up a simple polynomial (degree 3)
                if (typeof enterFastMode !== 'function') {
                    resolve({error: 'enterFastMode not found'});
                    return;
                }

                // Use current state, enter fast mode
                var origSteps = typeof stepsPerWorkerPass !== 'undefined' ? stepsPerWorkerPass : 1000;

                // Check if bitmap canvas exists
                var bc = document.getElementById('bitmapCanvas');
                if (!bc) {
                    resolve({error: 'no bitmapCanvas'});
                    return;
                }

                enterFastMode();

                // Wait for some passes to complete
                setTimeout(() => {
                    if (typeof exitFastMode === 'function') exitFastMode();

                    // Check if persistent buffer has non-background pixels
                    var ctx = bc.getContext('2d');
                    var w = bc.width, h = bc.height;
                    var sample = ctx.getImageData(0, 0, Math.min(w, 100), Math.min(h, 100));
                    var nonZero = 0;
                    for (var i = 0; i < sample.data.length; i += 4) {
                        if (sample.data[i] !== 0 || sample.data[i+1] !== 0 || sample.data[i+2] !== 0) {
                            nonZero++;
                        }
                    }
                    resolve({nonZeroPixels: nonZero, sampleSize: sample.data.length / 4});
                }, 2000);
            });
        }""")
        if "error" in has_pixels:
            pytest.skip(has_pixels["error"])
        assert has_pixels["nonZeroPixels"] > 0, "No pixels painted in fast mode"
