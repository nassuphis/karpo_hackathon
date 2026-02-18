"""Benchmark: JS vs WASM Ehrlich-Aberth solver at various degrees,
plus full step loop (solver + pixel output) comparison."""

import pytest
from pathlib import Path

# Import WASM step loop helpers from test_wasm_step_loop
from test_wasm_step_loop import _CWL, _INST, _WRITE_CFG, _SETUP, wasm_step_loop_b64  # noqa: F401

_HELPERS = _CWL + _INST + _WRITE_CFG + _SETUP

# The JS worker solver source — copied verbatim from index.html line 7366
# This is the flat-array optimized version used in Web Workers
JS_SOLVER_SOURCE = """
function solveEA_JS(cRe, cIm, nCoeffs, warmRe, warmIm, nRoots, iterCounts) {
    var start = 0;
    while (start < nCoeffs - 1 && cRe[start]*cRe[start] + cIm[start]*cIm[start] < 1e-30) start++;
    var degree = nCoeffs - 1 - start;
    if (degree <= 0) return;
    if (degree === 1) {
        var aR = cRe[start], aI = cIm[start], bR = cRe[start+1], bI = cIm[start+1];
        var d = aR*aR + aI*aI;
        if (d < 1e-30) return;
        warmRe[0] = -(bR*aR + bI*aI) / d;
        warmIm[0] = -(bI*aR - bR*aI) / d;
        if (iterCounts) iterCounts[0] = 1;
        return;
    }
    var n = nCoeffs - start;
    var cr = new Float64Array(n), ci = new Float64Array(n);
    for (var k = 0; k < n; k++) { cr[k] = cRe[start+k]; ci[k] = cIm[start+k]; }
    var rRe = new Float64Array(degree), rIm = new Float64Array(degree);
    for (var i = 0; i < degree; i++) { rRe[i] = warmRe[i]; rIm[i] = warmIm[i]; }
    var trackIter = !!iterCounts;
    var conv = trackIter ? new Uint8Array(degree) : null;
    for (var iter = 0; iter < 64; iter++) {
        var maxCorr2 = 0;
        for (var i = 0; i < degree; i++) {
            if (trackIter && conv[i]) continue;
            var zR = rRe[i], zI = rIm[i];
            var pR = cr[0], pI = ci[0], dpR = 0, dpI = 0;
            for (var k = 1; k < n; k++) {
                var ndR = dpR*zR - dpI*zI + pR, ndI = dpR*zI + dpI*zR + pI;
                dpR = ndR; dpI = ndI;
                var npR = pR*zR - pI*zI + cr[k], npI = pR*zI + pI*zR + ci[k];
                pR = npR; pI = npI;
            }
            var dpM = dpR*dpR + dpI*dpI;
            if (dpM < 1e-60) continue;
            var wR = (pR*dpR + pI*dpI)/dpM, wI = (pI*dpR - pR*dpI)/dpM;
            var sR = 0, sI = 0;
            for (var j = 0; j < degree; j++) {
                if (j === i) continue;
                var dR = zR - rRe[j], dI = zI - rIm[j];
                var dM = dR*dR + dI*dI;
                if (dM < 1e-60) continue;
                sR += dR/dM; sI += -dI/dM;
            }
            var wsR = wR*sR - wI*sI, wsI = wR*sI + wI*sR;
            var dnR = 1 - wsR, dnI = -wsI;
            var dnM = dnR*dnR + dnI*dnI;
            if (dnM < 1e-60) continue;
            var crrR = (wR*dnR + wI*dnI)/dnM, crrI = (wI*dnR - wR*dnI)/dnM;
            rRe[i] -= crrR; rIm[i] -= crrI;
            var h2 = crrR*crrR + crrI*crrI;
            if (h2 > maxCorr2) maxCorr2 = h2;
            if (trackIter && h2 < 1e-16) { conv[i] = 1; iterCounts[i] = iter + 1; }
        }
        if (maxCorr2 < 1e-16) {
            if (trackIter) { for (var i = 0; i < degree; i++) if (!conv[i]) { conv[i] = 1; iterCounts[i] = iter + 1; } }
            break;
        }
    }
    if (trackIter) { for (var i = 0; i < degree; i++) if (!conv[i]) iterCounts[i] = 64; }
    for (var i = 0; i < degree; i++) {
        if (isFinite(rRe[i]) && isFinite(rIm[i])) {
            warmRe[i] = rRe[i]; warmIm[i] = rIm[i];
        } else {
            var angle = (2 * Math.PI * i) / degree + 0.37;
            warmRe[i] = Math.cos(angle);
            warmIm[i] = Math.sin(angle);
        }
    }
}
"""

# WASM data lives ABOVE the stack (stack = 0x0000-0x7FFF, data = 0x8000+)
WASM_SETUP = """
function setupWasm(wasmB64) {
    var bytes = Uint8Array.from(atob(wasmB64), function(c) { return c.charCodeAt(0); });
    var mod = new WebAssembly.Module(bytes.buffer);
    var inst = new WebAssembly.Instance(mod, {});
    var wasmMemory = inst.exports.memory;
    var wasmSolve = inst.exports.solveEA;

    // Data offsets — placed after the 32KB stack (stack occupies 0x0000-0x7FFF)
    var BASE = 32768;  // 0x8000
    var OFF_CRE  = BASE;                  // coeffsRe[256] = 2048 bytes
    var OFF_CIM  = BASE + 2048;           // coeffsIm[256] = 2048 bytes
    var OFF_WRE  = BASE + 4096;           // warmRe[255]   = 2040 bytes
    var OFF_WIM  = BASE + 6136;           // warmIm[255]   = 2040 bytes
    var OFF_ITER = BASE + 8176;           // iterCounts[255] = 255 bytes

    return {
        memory: wasmMemory,
        solve: wasmSolve,
        OFF_CRE: OFF_CRE,
        OFF_CIM: OFF_CIM,
        OFF_WRE: OFF_WRE,
        OFF_WIM: OFF_WIM,
        OFF_ITER: OFF_ITER
    };
}
"""


def test_benchmark_js(page):
    """Benchmark the JS Ehrlich-Aberth solver at various polynomial degrees."""
    results = page.evaluate("""() => {
        """ + JS_SOLVER_SOURCE + """

        var results = [];
        var degrees = [5, 10, 20, 50, 100];

        var seed = 42;
        function rng() { seed = (seed * 1664525 + 1013904223) & 0x7fffffff; return seed / 0x7fffffff; }

        for (var di = 0; di < degrees.length; di++) {
            var deg = degrees[di];
            var nCoeffs = deg + 1;

            var cRe = new Float64Array(nCoeffs);
            var cIm = new Float64Array(nCoeffs);
            for (var i = 0; i < nCoeffs; i++) {
                cRe[i] = rng() * 2 - 1;
                cIm[i] = rng() * 2 - 1;
            }

            var N = deg <= 20 ? 10000 : (deg <= 50 ? 2000 : 500);

            var warmRe = new Float64Array(deg);
            var warmIm = new Float64Array(deg);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                warmRe[i] = Math.cos(a);
                warmIm[i] = Math.sin(a);
            }

            // JIT warmup
            for (var r = 0; r < 100; r++) {
                solveEA_JS(cRe, cIm, nCoeffs, warmRe, warmIm, deg, null);
            }

            // Reset warm start
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                warmRe[i] = Math.cos(a);
                warmIm[i] = Math.sin(a);
            }

            var t0 = performance.now();
            for (var r = 0; r < N; r++) {
                solveEA_JS(cRe, cIm, nCoeffs, warmRe, warmIm, deg, null);
            }
            var elapsed = performance.now() - t0;

            results.push({
                degree: deg,
                iterations: N,
                totalMs: elapsed,
                usPerCall: (elapsed / N) * 1000
            });
        }
        return results;
    }""")

    print("\n=== JS Ehrlich-Aberth Solver Benchmark ===")
    print(f"{'Degree':>8} {'Calls':>8} {'Total ms':>10} {'us/call':>10}")
    print("-" * 40)
    for r in results:
        print(f"{r['degree']:>8} {r['iterations']:>8} {r['totalMs']:>10.1f} {r['usPerCall']:>10.2f}")

    for r in results:
        assert r["totalMs"] > 0
        assert r["usPerCall"] > 0


def test_benchmark_wasm(page, wasm_b64):
    """Benchmark the WASM Ehrlich-Aberth solver at various polynomial degrees."""
    if wasm_b64 is None:
        pytest.skip("WASM not built — run ./build-wasm.sh first")

    results = page.evaluate("""(wasmB64) => {
        """ + WASM_SETUP + """
        var wasm = setupWasm(wasmB64);

        var results = [];
        var degrees = [5, 10, 20, 50, 100];

        var seed = 42;
        function rng() { seed = (seed * 1664525 + 1013904223) & 0x7fffffff; return seed / 0x7fffffff; }

        for (var di = 0; di < degrees.length; di++) {
            var deg = degrees[di];
            var nCoeffs = deg + 1;

            var cReArr = new Float64Array(nCoeffs);
            var cImArr = new Float64Array(nCoeffs);
            for (var i = 0; i < nCoeffs; i++) {
                cReArr[i] = rng() * 2 - 1;
                cImArr[i] = rng() * 2 - 1;
            }

            // Write into WASM module's own memory
            var memCRe = new Float64Array(wasm.memory.buffer, wasm.OFF_CRE, nCoeffs);
            var memCIm = new Float64Array(wasm.memory.buffer, wasm.OFF_CIM, nCoeffs);
            var memWRe = new Float64Array(wasm.memory.buffer, wasm.OFF_WRE, deg);
            var memWIm = new Float64Array(wasm.memory.buffer, wasm.OFF_WIM, deg);

            memCRe.set(cReArr);
            memCIm.set(cImArr);

            var N = deg <= 20 ? 10000 : (deg <= 50 ? 2000 : 500);

            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                memWRe[i] = Math.cos(a);
                memWIm[i] = Math.sin(a);
            }

            // Warmup
            for (var r = 0; r < 100; r++) {
                wasm.solve(wasm.OFF_CRE, wasm.OFF_CIM, nCoeffs, wasm.OFF_WRE, wasm.OFF_WIM, deg, 0, wasm.OFF_ITER);
            }

            // Reset warm start
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                memWRe[i] = Math.cos(a);
                memWIm[i] = Math.sin(a);
            }

            var t0 = performance.now();
            for (var r = 0; r < N; r++) {
                wasm.solve(wasm.OFF_CRE, wasm.OFF_CIM, nCoeffs, wasm.OFF_WRE, wasm.OFF_WIM, deg, 0, wasm.OFF_ITER);
            }
            var elapsed = performance.now() - t0;

            results.push({
                degree: deg,
                iterations: N,
                totalMs: elapsed,
                usPerCall: (elapsed / N) * 1000
            });
        }
        return results;
    }""", wasm_b64)

    print("\n=== WASM Ehrlich-Aberth Solver Benchmark ===")
    print(f"{'Degree':>8} {'Calls':>8} {'Total ms':>10} {'us/call':>10}")
    print("-" * 40)
    for r in results:
        print(f"{r['degree']:>8} {r['iterations']:>8} {r['totalMs']:>10.1f} {r['usPerCall']:>10.2f}")

    for r in results:
        assert r["totalMs"] > 0
        assert r["usPerCall"] > 0


def test_benchmark_comparison(page, wasm_b64):
    """Side-by-side JS vs WASM benchmark with ratio comparison."""
    if wasm_b64 is None:
        pytest.skip("WASM not built — run ./build-wasm.sh first")

    results = page.evaluate("""(wasmB64) => {
        """ + JS_SOLVER_SOURCE + """
        """ + WASM_SETUP + """
        var wasm = setupWasm(wasmB64);

        var results = [];
        var degrees = [5, 10, 20, 50, 100];

        var seed = 42;
        function rng() { seed = (seed * 1664525 + 1013904223) & 0x7fffffff; return seed / 0x7fffffff; }

        for (var di = 0; di < degrees.length; di++) {
            var deg = degrees[di];
            var nCoeffs = deg + 1;

            seed = 42 + di * 1000;
            var cRe = new Float64Array(nCoeffs);
            var cIm = new Float64Array(nCoeffs);
            for (var i = 0; i < nCoeffs; i++) {
                cRe[i] = rng() * 2 - 1;
                cIm[i] = rng() * 2 - 1;
            }

            var N = deg <= 20 ? 10000 : (deg <= 50 ? 2000 : 500);

            // === JS benchmark ===
            var jsWarmRe = new Float64Array(deg);
            var jsWarmIm = new Float64Array(deg);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                jsWarmRe[i] = Math.cos(a); jsWarmIm[i] = Math.sin(a);
            }
            for (var r = 0; r < 200; r++) solveEA_JS(cRe, cIm, nCoeffs, jsWarmRe, jsWarmIm, deg, null);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                jsWarmRe[i] = Math.cos(a); jsWarmIm[i] = Math.sin(a);
            }
            var t0 = performance.now();
            for (var r = 0; r < N; r++) solveEA_JS(cRe, cIm, nCoeffs, jsWarmRe, jsWarmIm, deg, null);
            var jsMs = performance.now() - t0;

            // === WASM benchmark ===
            var memCRe = new Float64Array(wasm.memory.buffer, wasm.OFF_CRE, nCoeffs);
            var memCIm = new Float64Array(wasm.memory.buffer, wasm.OFF_CIM, nCoeffs);
            var memWRe = new Float64Array(wasm.memory.buffer, wasm.OFF_WRE, deg);
            var memWIm = new Float64Array(wasm.memory.buffer, wasm.OFF_WIM, deg);
            memCRe.set(cRe);
            memCIm.set(cIm);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                memWRe[i] = Math.cos(a); memWIm[i] = Math.sin(a);
            }
            for (var r = 0; r < 200; r++) wasm.solve(wasm.OFF_CRE, wasm.OFF_CIM, nCoeffs, wasm.OFF_WRE, wasm.OFF_WIM, deg, 0, wasm.OFF_ITER);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                memWRe[i] = Math.cos(a); memWIm[i] = Math.sin(a);
            }
            var t0w = performance.now();
            for (var r = 0; r < N; r++) wasm.solve(wasm.OFF_CRE, wasm.OFF_CIM, nCoeffs, wasm.OFF_WRE, wasm.OFF_WIM, deg, 0, wasm.OFF_ITER);
            var wasmMs = performance.now() - t0w;

            results.push({
                degree: deg,
                iterations: N,
                jsUs: (jsMs / N) * 1000,
                wasmUs: (wasmMs / N) * 1000,
                ratio: wasmMs > 0 ? jsMs / wasmMs : Infinity
            });
        }
        return results;
    }""", wasm_b64)

    print("\n" + "=" * 62)
    print("  Ehrlich-Aberth Solver Benchmark: JS vs WASM")
    print("=" * 62)
    print(f"{'Degree':>8} {'Calls':>8} {'JS us/call':>12} {'WASM us/call':>13} {'JS/WASM':>9}")
    print("-" * 62)
    for r in results:
        ratio_str = f"{r['ratio']:.2f}x" if r['ratio'] != float('inf') else "inf"
        print(f"{r['degree']:>8} {r['iterations']:>8} {r['jsUs']:>12.2f} {r['wasmUs']:>13.2f} {ratio_str:>9}")
    print("-" * 62)

    for r in results:
        assert r["jsUs"] > 0
        assert r["wasmUs"] > 0


def test_wasm_correctness(page, wasm_b64):
    """Verify WASM solver produces same roots as JS solver."""
    if wasm_b64 is None:
        pytest.skip("WASM not built — run ./build-wasm.sh first")

    result = page.evaluate("""(wasmB64) => {
        """ + JS_SOLVER_SOURCE + """
        """ + WASM_SETUP + """
        var wasm = setupWasm(wasmB64);

        // Test: z^5 - 1 = 0 (5th roots of unity)
        var nCoeffs = 6, deg = 5;
        var cRe = new Float64Array([1, 0, 0, 0, 0, -1]);
        var cIm = new Float64Array([0, 0, 0, 0, 0, 0]);

        // JS solve
        var jsWRe = new Float64Array(deg), jsWIm = new Float64Array(deg);
        for (var i = 0; i < deg; i++) {
            var a = (2 * Math.PI * i) / deg + 0.37;
            jsWRe[i] = Math.cos(a); jsWIm[i] = Math.sin(a);
        }
        solveEA_JS(cRe, cIm, nCoeffs, jsWRe, jsWIm, deg, null);

        // WASM solve — same initial conditions
        var memCRe = new Float64Array(wasm.memory.buffer, wasm.OFF_CRE, nCoeffs);
        var memCIm = new Float64Array(wasm.memory.buffer, wasm.OFF_CIM, nCoeffs);
        var memWRe = new Float64Array(wasm.memory.buffer, wasm.OFF_WRE, deg);
        var memWIm = new Float64Array(wasm.memory.buffer, wasm.OFF_WIM, deg);
        memCRe.set(cRe); memCIm.set(cIm);
        for (var i = 0; i < deg; i++) {
            var a = (2 * Math.PI * i) / deg + 0.37;
            memWRe[i] = Math.cos(a); memWIm[i] = Math.sin(a);
        }
        wasm.solve(wasm.OFF_CRE, wasm.OFF_CIM, nCoeffs, wasm.OFF_WRE, wasm.OFF_WIM, deg, 0, wasm.OFF_ITER);

        var jsRoots = [], wasmRoots = [];
        for (var i = 0; i < deg; i++) {
            jsRoots.push([jsWRe[i], jsWIm[i]]);
            wasmRoots.push([memWRe[i], memWIm[i]]);
        }

        var errors = [];
        for (var i = 0; i < deg; i++) {
            var jsMag = Math.sqrt(jsRoots[i][0]*jsRoots[i][0] + jsRoots[i][1]*jsRoots[i][1]);
            var wasmMag = Math.sqrt(wasmRoots[i][0]*wasmRoots[i][0] + wasmRoots[i][1]*wasmRoots[i][1]);
            if (Math.abs(jsMag - 1) > 1e-6) errors.push('JS root ' + i + ' magnitude ' + jsMag);
            if (Math.abs(wasmMag - 1) > 1e-6) errors.push('WASM root ' + i + ' magnitude ' + wasmMag);
        }

        // Check JS and WASM roots match (unordered — find closest pairs)
        for (var i = 0; i < deg; i++) {
            var bestDist = Infinity;
            for (var j = 0; j < deg; j++) {
                var d = Math.sqrt(
                    (jsRoots[i][0] - wasmRoots[j][0]) * (jsRoots[i][0] - wasmRoots[j][0]) +
                    (jsRoots[i][1] - wasmRoots[j][1]) * (jsRoots[i][1] - wasmRoots[j][1])
                );
                if (d < bestDist) bestDist = d;
            }
            if (bestDist > 1e-6) {
                errors.push('JS root ' + i + ' has no WASM match (closest dist: ' + bestDist + ')');
            }
        }

        return {errors: errors, jsRoots: jsRoots, wasmRoots: wasmRoots};
    }""", wasm_b64)

    assert len(result["errors"]) == 0, f"WASM/JS mismatch: {result['errors']}"


# ============================================================
# Full Step Loop Benchmark: JS vs WASM
# ============================================================
# Measures the entire per-pass pipeline: solver + pixel output
# (no curve interpolation — static polynomial, uniform color)

# JS step loop that replicates what the worker blob does per step:
# solveEA → compute pixel coords → write to sparse output arrays
JS_STEP_LOOP = JS_SOLVER_SOURCE + """
function jsStepLoop(cRe, cIm, nCoeffs, rootsRe, rootsIm, nRoots,
                    canvasW, canvasH, range, stepStart, stepEnd,
                    paintIdx, paintR, paintG, paintB) {
    var pc = 0;
    var half = canvasW / 2, halfH = canvasH / 2;
    var scale = half / range;
    for (var step = stepStart; step < stepEnd; step++) {
        solveEA_JS(cRe, cIm, nCoeffs, rootsRe, rootsIm, nRoots, null);
        for (var r = 0; r < nRoots; r++) {
            var px = Math.round(half + rootsRe[r] * scale);
            var py = Math.round(halfH - rootsIm[r] * scale);
            if (px >= 0 && px < canvasW && py >= 0 && py < canvasH) {
                paintIdx[pc] = py * canvasW + px;
                paintR[pc] = 255; paintG[pc] = 255; paintB[pc] = 255;
                pc++;
            }
        }
    }
    return pc;
}
"""


def test_step_loop_benchmark_js(page):
    """Benchmark the JS step loop (solver + pixel gen) at various degrees and step counts."""
    results = page.evaluate("""() => {
        """ + JS_STEP_LOOP + """

        var seed = 42;
        function rng() { seed = (seed * 1664525 + 1013904223) & 0x7fffffff; return seed / 0x7fffffff; }

        var configs = [
            {deg: 5,  steps: 10000},
            {deg: 5,  steps: 50000},
            {deg: 10, steps: 10000},
            {deg: 10, steps: 50000},
            {deg: 20, steps: 5000},
            {deg: 20, steps: 20000}
        ];
        var results = [];
        for (var ci = 0; ci < configs.length; ci++) {
            var deg = configs[ci].deg, N = configs[ci].steps;
            var nc = deg + 1;
            seed = 42 + ci * 1000;
            var cRe = new Float64Array(nc), cIm = new Float64Array(nc);
            for (var i = 0; i < nc; i++) { cRe[i] = rng() * 2 - 1; cIm[i] = rng() * 2 - 1; }

            var maxP = N * deg;
            var rRe = new Float64Array(deg), rIm = new Float64Array(deg);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                rRe[i] = Math.cos(a); rIm[i] = Math.sin(a);
            }
            var pI = new Int32Array(maxP), pR = new Uint8Array(maxP);
            var pG = new Uint8Array(maxP), pB = new Uint8Array(maxP);

            // Warmup (100 steps)
            jsStepLoop(cRe, cIm, nc, rRe, rIm, deg, 500, 500, 2.0, 0, 100, pI, pR, pG, pB);
            // Reset roots
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                rRe[i] = Math.cos(a); rIm[i] = Math.sin(a);
            }

            var t0 = performance.now();
            var pc = jsStepLoop(cRe, cIm, nc, rRe, rIm, deg, 500, 500, 2.0, 0, N, pI, pR, pG, pB);
            var elapsed = performance.now() - t0;

            results.push({
                degree: deg, steps: N, totalMs: elapsed, pixels: pc,
                stepsPerSec: Math.round(N / (elapsed / 1000)),
                usPerStep: (elapsed / N) * 1000
            });
        }
        return results;
    }""")

    print("\n=== JS Step Loop Benchmark (solver + pixel gen) ===")
    print(f"{'Degree':>8} {'Steps':>8} {'Total ms':>10} {'Steps/s':>12} {'us/step':>10} {'Pixels':>10}")
    print("-" * 62)
    for r in results:
        print(f"{r['degree']:>8} {r['steps']:>8} {r['totalMs']:>10.1f} "
              f"{r['stepsPerSec']:>12,} {r['usPerStep']:>10.2f} {r['pixels']:>10,}")

    for r in results:
        assert r["stepsPerSec"] > 0
        assert r["pixels"] > 0


def test_step_loop_benchmark_wasm(page, wasm_step_loop_b64):
    """Benchmark the WASM step loop at various degrees and step counts."""
    if wasm_step_loop_b64 is None:
        pytest.skip("step_loop.wasm not built")

    results = page.evaluate("(b64) => {" + _HELPERS + """

        var seed = 42;
        function rng() { seed = (seed * 1664525 + 1013904223) & 0x7fffffff; return seed / 0x7fffffff; }

        var configs = [
            {deg: 5,  steps: 10000},
            {deg: 5,  steps: 50000},
            {deg: 10, steps: 10000},
            {deg: 10, steps: 50000},
            {deg: 20, steps: 5000},
            {deg: 20, steps: 20000}
        ];
        var results = [];
        for (var ci = 0; ci < configs.length; ci++) {
            var deg = configs[ci].deg, N = configs[ci].steps;
            var nc = deg + 1;
            seed = 42 + ci * 1000;
            var cRe = [], cIm = [];
            for (var i = 0; i < nc; i++) { cRe.push(rng() * 2 - 1); cIm.push(rng() * 2 - 1); }

            var maxP = N * deg;
            var w = instantiateWasm(b64);
            var L = computeWasmLayout(nc, deg, maxP, 0, 0, 0, 0, 0, 0, w.hb);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;
            writeCfg(buf, L, nc, deg, {
                canvasW: 500, canvasH: 500, totalSteps: N, colorMode: 0,
                uR: 255, uG: 255, uB: 255, range: 2.0
            });
            new Float64Array(buf, L.cRe, nc).set(cRe);
            new Float64Array(buf, L.cIm, nc).set(cIm);
            new Float64Array(buf, L.wCR, nc).set(cRe);
            new Float64Array(buf, L.wCI, nc).set(cIm);

            var pRR = new Float64Array(buf, L.pRR, deg);
            var pRI = new Float64Array(buf, L.pRI, deg);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                pRR[i] = Math.cos(a); pRI[i] = Math.sin(a);
            }
            w.inst.exports.init(L.cfgI, L.cfgD);

            // Warmup (100 steps)
            w.inst.exports.runStepLoop(0, 100, 0.0);
            // Reset roots
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                pRR[i] = Math.cos(a); pRI[i] = Math.sin(a);
            }

            var t0 = performance.now();
            var pc = w.inst.exports.runStepLoop(0, N, 0.0);
            var elapsed = performance.now() - t0;

            results.push({
                degree: deg, steps: N, totalMs: elapsed, pixels: pc,
                stepsPerSec: Math.round(N / (elapsed / 1000)),
                usPerStep: (elapsed / N) * 1000
            });
        }
        return results;
    }""", wasm_step_loop_b64)

    print("\n=== WASM Step Loop Benchmark (full loop in WASM) ===")
    print(f"{'Degree':>8} {'Steps':>8} {'Total ms':>10} {'Steps/s':>12} {'us/step':>10} {'Pixels':>10}")
    print("-" * 62)
    for r in results:
        print(f"{r['degree']:>8} {r['steps']:>8} {r['totalMs']:>10.1f} "
              f"{r['stepsPerSec']:>12,} {r['usPerStep']:>10.2f} {r['pixels']:>10,}")

    for r in results:
        assert r["stepsPerSec"] > 0
        assert r["pixels"] > 0


def test_step_loop_benchmark_comparison(page, wasm_b64, wasm_step_loop_b64):
    """Side-by-side JS step loop vs WASM step loop with speedup ratio."""
    if wasm_step_loop_b64 is None:
        pytest.skip("step_loop.wasm not built")

    results = page.evaluate("(args) => {" + JS_STEP_LOOP + _HELPERS + """
        var b64 = args.b64;

        var seed = 42;
        function rng() { seed = (seed * 1664525 + 1013904223) & 0x7fffffff; return seed / 0x7fffffff; }

        var configs = [
            {deg: 5,  steps: 10000},
            {deg: 5,  steps: 50000},
            {deg: 10, steps: 10000},
            {deg: 10, steps: 50000},
            {deg: 20, steps: 5000},
            {deg: 20, steps: 20000}
        ];
        var results = [];
        for (var ci = 0; ci < configs.length; ci++) {
            var deg = configs[ci].deg, N = configs[ci].steps;
            var nc = deg + 1;
            seed = 42 + ci * 1000;
            var cRe = new Float64Array(nc), cIm = new Float64Array(nc);
            for (var i = 0; i < nc; i++) { cRe[i] = rng() * 2 - 1; cIm[i] = rng() * 2 - 1; }

            // === JS step loop ===
            var maxP = N * deg;
            var rRe = new Float64Array(deg), rIm = new Float64Array(deg);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                rRe[i] = Math.cos(a); rIm[i] = Math.sin(a);
            }
            var pI = new Int32Array(maxP), pR = new Uint8Array(maxP);
            var pG = new Uint8Array(maxP), pB = new Uint8Array(maxP);
            // Warmup
            jsStepLoop(cRe, cIm, nc, rRe, rIm, deg, 500, 500, 2.0, 0, 200, pI, pR, pG, pB);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                rRe[i] = Math.cos(a); rIm[i] = Math.sin(a);
            }
            var t0 = performance.now();
            var jsPc = jsStepLoop(cRe, cIm, nc, rRe, rIm, deg, 500, 500, 2.0, 0, N, pI, pR, pG, pB);
            var jsMs = performance.now() - t0;

            // === WASM step loop ===
            var cReArr = Array.from(cRe), cImArr = Array.from(cIm);
            var w = instantiateWasm(b64);
            var L = computeWasmLayout(nc, deg, maxP, 0, 0, 0, 0, 0, 0, w.hb);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;
            writeCfg(buf, L, nc, deg, {
                canvasW: 500, canvasH: 500, totalSteps: N, colorMode: 0,
                uR: 255, uG: 255, uB: 255, range: 2.0
            });
            new Float64Array(buf, L.cRe, nc).set(cReArr);
            new Float64Array(buf, L.cIm, nc).set(cImArr);
            new Float64Array(buf, L.wCR, nc).set(cReArr);
            new Float64Array(buf, L.wCI, nc).set(cImArr);

            var wpRR = new Float64Array(buf, L.pRR, deg);
            var wpRI = new Float64Array(buf, L.pRI, deg);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                wpRR[i] = Math.cos(a); wpRI[i] = Math.sin(a);
            }
            w.inst.exports.init(L.cfgI, L.cfgD);
            // Warmup
            w.inst.exports.runStepLoop(0, 200, 0.0);
            for (var i = 0; i < deg; i++) {
                var a = (2 * Math.PI * i) / deg + 0.37;
                wpRR[i] = Math.cos(a); wpRI[i] = Math.sin(a);
            }
            var t0w = performance.now();
            var wasmPc = w.inst.exports.runStepLoop(0, N, 0.0);
            var wasmMs = performance.now() - t0w;

            results.push({
                degree: deg, steps: N,
                jsMs: jsMs, wasmMs: wasmMs,
                jsPixels: jsPc, wasmPixels: wasmPc,
                jsStepsPerSec: Math.round(N / (jsMs / 1000)),
                wasmStepsPerSec: Math.round(N / (wasmMs / 1000)),
                speedup: wasmMs > 0 ? jsMs / wasmMs : Infinity
            });
        }
        return results;
    }""", {"b64": wasm_step_loop_b64})

    print("\n" + "=" * 80)
    print("  Full Step Loop Benchmark: JS vs WASM (solver + pixel output)")
    print("=" * 80)
    print(f"{'Degree':>8} {'Steps':>8} {'JS ms':>10} {'WASM ms':>10} "
          f"{'JS steps/s':>13} {'WASM steps/s':>13} {'Speedup':>9}")
    print("-" * 80)
    for r in results:
        sp = f"{r['speedup']:.2f}x" if r['speedup'] != float('inf') else "inf"
        print(f"{r['degree']:>8} {r['steps']:>8} {r['jsMs']:>10.1f} {r['wasmMs']:>10.1f} "
              f"{r['jsStepsPerSec']:>13,} {r['wasmStepsPerSec']:>13,} {sp:>9}")
    print("-" * 80)

    for r in results:
        assert r["jsStepsPerSec"] > 0
        assert r["wasmStepsPerSec"] > 0
        assert r["jsPixels"] > 0
        assert r["wasmPixels"] > 0
