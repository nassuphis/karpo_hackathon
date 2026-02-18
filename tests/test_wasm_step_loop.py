"""Tests for the WASM step loop — verifies init, memory layout, solver correctness,
pixel output, and comparison with the JS step loop path."""

import pytest
from pathlib import Path

@pytest.fixture(scope="session")
def wasm_step_loop_b64():
    """Load WASM step loop base64 string (or None if not built)."""
    path = Path(__file__).parent.parent / "step_loop.wasm.b64"
    if path.exists():
        return path.read_text().strip()
    return None


# JS helpers inlined into evaluate function bodies.
# computeWasmLayout is inside the worker blob and not accessible from page scope.

_CWL = """
    function computeWasmLayout(nc, nr, maxP, nE, nDE, nFC, nSI, tCP, tDP, heapBase) {
        var o = heapBase || 65536;
        function a8(x) { return (x + 7) & ~7; }
        var L = {};
        L.cfgI = o; o = a8(o + 65 * 4);
        L.cfgD = o; o = a8(o + 3 * 8);
        L.cRe = o; o = a8(o + nc * 8);
        L.cIm = o; o = a8(o + nc * 8);
        L.clR = o; o = a8(o + nr);
        L.clG = o; o = a8(o + nr);
        L.clB = o; o = a8(o + nr);
        L.jRe = o; o = a8(o + nc * 8);
        L.jIm = o; o = a8(o + nc * 8);
        L.mTR = o; o = a8(o + nc * 8);
        L.mTI = o; o = a8(o + nc * 8);
        L.ppR = o; o = a8(o + 16);
        L.ppG = o; o = a8(o + 16);
        L.ppB = o; o = a8(o + 16);
        L.dpR = o; o = a8(o + 16);
        L.dpG = o; o = a8(o + 16);
        L.dpB = o; o = a8(o + 16);
        L.sI = o; o = a8(o + Math.max(nSI, 1) * 4);
        L.fCI = o; o = a8(o + Math.max(nFC, 1) * 4);
        L.eIdx = o; o = a8(o + Math.max(nE, 1) * 4);
        L.eSpd = o; o = a8(o + Math.max(nE, 1) * 8);
        L.eCcw = o; o = a8(o + Math.max(nE, 1) * 4);
        L.eDth = o; o = a8(o + Math.max(nE, 1) * 8);
        L.cOff = o; o = a8(o + Math.max(nE, 1) * 4);
        L.cLen = o; o = a8(o + Math.max(nE, 1) * 4);
        L.cCld = o; o = a8(o + Math.max(nE, 1) * 4);
        L.dIdx = o; o = a8(o + Math.max(nDE, 1) * 4);
        L.dSpd = o; o = a8(o + Math.max(nDE, 1) * 8);
        L.dCcw = o; o = a8(o + Math.max(nDE, 1) * 4);
        L.dDth = o; o = a8(o + Math.max(nDE, 1) * 8);
        L.dOff = o; o = a8(o + Math.max(nDE, 1) * 4);
        L.dLen = o; o = a8(o + Math.max(nDE, 1) * 4);
        L.dCld = o; o = a8(o + Math.max(nDE, 1) * 4);
        L.cvF = o; o = a8(o + Math.max(tCP, 1) * 2 * 8);
        L.dcF = o; o = a8(o + Math.max(tDP, 1) * 2 * 8);
        L.wCR = o; o = a8(o + nc * 8);
        L.wCI = o; o = a8(o + nc * 8);
        L.tRe = o; o = a8(o + nr * 8);
        L.tIm = o; o = a8(o + nr * 8);
        L.mWR = o; o = a8(o + nc * 8);
        L.mWI = o; o = a8(o + nc * 8);
        L.pRR = o; o = a8(o + nr * 8);
        L.pRI = o; o = a8(o + nr * 8);
        L.piO = o; o = a8(o + maxP * 4);
        L.prO = o; o = a8(o + maxP);
        L.pgO = o; o = a8(o + maxP);
        L.pbO = o; o = a8(o + maxP);
        L.pages = Math.ceil(o / 65536);
        return L;
    }
"""

_INST = """
    function instantiateWasm(b64, progressFn) {
        var raw = atob(b64);
        var bytes = new Uint8Array(raw.length);
        for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
        var mod = new WebAssembly.Module(bytes.buffer);
        var mem = new WebAssembly.Memory({initial: 2});
        var inst = new WebAssembly.Instance(mod, {
            env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log,
                   reportProgress: progressFn || function(){} }
        });
        var hb = inst.exports.__heap_base ? inst.exports.__heap_base.value : 65536;
        return {inst: inst, mem: mem, hb: hb};
    }
"""

_WRITE_CFG = """
    function writeCfg(buf, L, nc, nr, opts) {
        opts = opts || {};
        var cfgI32 = new Int32Array(buf, L.cfgI, 65);
        cfgI32[0] = nc; cfgI32[1] = nr;
        cfgI32[2] = opts.canvasW || 200; cfgI32[3] = opts.canvasH || 200;
        cfgI32[4] = opts.totalSteps || 10;
        cfgI32[5] = opts.colorMode || 0;
        cfgI32[6] = opts.matchStrategy || 0;
        cfgI32[12] = 0;
        cfgI32[13] = opts.uR !== undefined ? opts.uR : 255;
        cfgI32[14] = opts.uG !== undefined ? opts.uG : 255;
        cfgI32[15] = opts.uB !== undefined ? opts.uB : 255;
        cfgI32[20] = L.cRe; cfgI32[21] = L.cIm;
        cfgI32[22] = L.clR; cfgI32[23] = L.clG; cfgI32[24] = L.clB;
        cfgI32[25] = L.jRe; cfgI32[26] = L.jIm;
        cfgI32[27] = L.mTR; cfgI32[28] = L.mTI;
        cfgI32[29] = L.ppR; cfgI32[30] = L.ppG; cfgI32[31] = L.ppB;
        cfgI32[32] = L.dpR; cfgI32[33] = L.dpG; cfgI32[34] = L.dpB;
        cfgI32[35] = L.sI; cfgI32[36] = L.fCI;
        cfgI32[37] = L.eIdx; cfgI32[38] = L.eSpd; cfgI32[39] = L.eCcw; cfgI32[40] = L.eDth;
        cfgI32[41] = L.cOff; cfgI32[42] = L.cLen; cfgI32[43] = L.cCld;
        cfgI32[44] = L.dIdx; cfgI32[45] = L.dSpd; cfgI32[46] = L.dCcw; cfgI32[47] = L.dDth;
        cfgI32[48] = L.dOff; cfgI32[49] = L.dLen; cfgI32[50] = L.dCld;
        cfgI32[51] = L.cvF; cfgI32[52] = L.dcF;
        cfgI32[53] = L.wCR; cfgI32[54] = L.wCI;
        cfgI32[55] = L.tRe; cfgI32[56] = L.tIm;
        cfgI32[57] = L.mWR; cfgI32[58] = L.mWI;
        cfgI32[59] = L.pRR; cfgI32[60] = L.pRI;
        cfgI32[61] = L.piO; cfgI32[62] = L.prO; cfgI32[63] = L.pgO; cfgI32[64] = L.pbO;
        var cfgF64 = new Float64Array(buf, L.cfgD, 3);
        cfgF64[0] = opts.range || 2.0;
        cfgF64[1] = opts.fps || 1.0;
        cfgF64[2] = opts.morphRate || 0.0;
    }
"""

_SETUP = """
    function setupAndRun(w, nc, nr, maxP, coeffsRe, coeffsIm, opts) {
        opts = opts || {};
        var L = computeWasmLayout(nc, nr, maxP, 0, 0, 0, 0, 0, 0, w.hb);
        var curPages = w.mem.buffer.byteLength / 65536;
        if (L.pages > curPages) w.mem.grow(L.pages - curPages);
        var buf = w.mem.buffer;
        writeCfg(buf, L, nc, nr, opts);
        new Float64Array(buf, L.cRe, nc).set(coeffsRe);
        new Float64Array(buf, L.cIm, nc).set(coeffsIm);
        new Float64Array(buf, L.wCR, nc).set(coeffsRe);
        new Float64Array(buf, L.wCI, nc).set(coeffsIm);
        if (opts.rootColorsR) {
            new Uint8Array(buf, L.clR, nr).set(opts.rootColorsR);
            new Uint8Array(buf, L.clG, nr).set(opts.rootColorsG);
            new Uint8Array(buf, L.clB, nr).set(opts.rootColorsB);
        }
        var pRR = new Float64Array(buf, L.pRR, nr);
        var pRI = new Float64Array(buf, L.pRI, nr);
        if (opts.initRootsRe) {
            pRR.set(opts.initRootsRe);
            pRI.set(opts.initRootsIm);
        } else {
            for (var i = 0; i < nr; i++) {
                var a = (2 * Math.PI * i) / nr + 0.37;
                pRR[i] = Math.cos(a); pRI[i] = Math.sin(a);
            }
        }
        w.inst.exports.init(L.cfgI, L.cfgD);
        var stepStart = opts.stepStart || 0;
        var stepEnd = opts.stepEnd !== undefined ? opts.stepEnd : (opts.totalSteps || 10);
        var pc = w.inst.exports.runStepLoop(stepStart, stepEnd, opts.elapsed || 0.0);
        return {
            paintCount: pc, L: L, buf: buf, pRR: pRR, pRI: pRI,
            paintIdx: new Int32Array(buf, L.piO, pc),
            paintR: new Uint8Array(buf, L.prO, pc),
            paintG: new Uint8Array(buf, L.pgO, pc),
            paintB: new Uint8Array(buf, L.pbO, pc)
        };
    }
"""

# All helpers combined — to be placed inside a function body
_HELPERS = _CWL + _INST + _WRITE_CFG + _SETUP


# ============================================================
# Basic WASM module loading and __heap_base
# ============================================================

class TestWasmModuleLoading:
    def test_wasm_module_loads(self, page, wasm_step_loop_b64):
        """WASM step loop module can be instantiated."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            try {
                var raw = atob(b64);
                var bytes = new Uint8Array(raw.length);
                for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
                var mod = new WebAssembly.Module(bytes.buffer);
                var mem = new WebAssembly.Memory({initial: 4});
                var inst = new WebAssembly.Instance(mod, {
                    env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, reportProgress: function(){} }
                });
                return {ok: true, exports: Object.keys(inst.exports)};
            } catch(e) {
                return {ok: false, error: String(e)};
            }
        }""", wasm_step_loop_b64)
        assert result["ok"], f"Failed to load WASM: {result.get('error')}"
        assert "init" in result["exports"]
        assert "runStepLoop" in result["exports"]

    def test_heap_base_exported(self, page, wasm_step_loop_b64):
        """WASM exports __heap_base for memory layout calculation."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            var raw = atob(b64);
            var bytes = new Uint8Array(raw.length);
            for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
            var mod = new WebAssembly.Module(bytes.buffer);
            var mem = new WebAssembly.Memory({initial: 4});
            var inst = new WebAssembly.Instance(mod, {
                env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, reportProgress: function(){} }
            });
            return {
                hasHeapBase: '__heap_base' in inst.exports,
                value: inst.exports.__heap_base ? inst.exports.__heap_base.value : null
            };
        }""", wasm_step_loop_b64)
        assert result["hasHeapBase"], "__heap_base not exported — BSS/config overlap possible"
        assert result["value"] is not None
        assert result["value"] > 65536, "heap_base should be above the 64KB stack"

    def test_heap_base_above_stack(self, page, wasm_step_loop_b64):
        """__heap_base must be above 64KB stack to avoid BSS/config overlap."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            var raw = atob(b64);
            var bytes = new Uint8Array(raw.length);
            for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
            var mod = new WebAssembly.Module(bytes.buffer);
            var mem = new WebAssembly.Memory({initial: 4});
            var inst = new WebAssembly.Instance(mod, {
                env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, reportProgress: function(){} }
            });
            return inst.exports.__heap_base.value;
        }""", wasm_step_loop_b64)
        assert result >= 65536 + 200, f"heap_base too low ({result}), BSS would overlap with config"


# ============================================================
# Memory layout (computeWasmLayout) tests
# ============================================================

class TestMemoryLayout:
    def test_layout_starts_at_heap_base(self, page):
        """computeWasmLayout uses heapBase parameter, not hardcoded 65536."""
        result = page.evaluate("() => {" + _CWL + """
            var L = computeWasmLayout(4, 3, 100, 1, 0, 0, 0, 5, 0, 66000);
            return {cfgI: L.cfgI};
        }""")
        assert result["cfgI"] == 66000, "Layout should start at heapBase"

    def test_layout_no_overlap_with_bss(self, page, wasm_step_loop_b64):
        """Config data starts at __heap_base, not at 65536 (where BSS lives)."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _CWL + _INST + """
            var w = instantiateWasm(b64);
            var L = computeWasmLayout(4, 3, 100, 1, 0, 0, 0, 5, 0, w.hb);
            return {heapBase: w.hb, cfgI: L.cfgI, noOverlap: L.cfgI >= w.hb};
        }""", wasm_step_loop_b64)
        assert result["noOverlap"], f"cfgI ({result['cfgI']}) overlaps BSS (heapBase={result['heapBase']})"

    def test_layout_8byte_alignment(self, page):
        """All layout offsets are 8-byte aligned."""
        result = page.evaluate("() => {" + _CWL + """
            var L = computeWasmLayout(5, 4, 200, 2, 1, 1, 1, 10, 5, 65808);
            var offsets = [L.cfgI, L.cfgD, L.cRe, L.cIm, L.clR, L.clG, L.clB,
                           L.jRe, L.jIm, L.mTR, L.mTI, L.ppR, L.ppG, L.ppB,
                           L.dpR, L.dpG, L.dpB, L.sI, L.fCI, L.eIdx, L.eSpd,
                           L.eCcw, L.eDth, L.cOff, L.cLen, L.cCld,
                           L.wCR, L.wCI, L.tRe, L.tIm, L.mWR, L.mWI,
                           L.pRR, L.pRI, L.piO, L.prO, L.pgO, L.pbO];
            var misaligned = offsets.filter(function(o) { return o % 8 !== 0; });
            return {misaligned: misaligned, count: misaligned.length};
        }""")
        assert result["count"] == 0, f"Misaligned offsets: {result['misaligned']}"

    def test_layout_no_overlap_between_sections(self, page):
        """No two sections overlap in the layout."""
        result = page.evaluate("() => {" + _CWL + """
            var nc = 6, nr = 5;
            var L = computeWasmLayout(nc, nr, 500, 3, 2, 1, 2, 20, 10, 65808);
            var sections = [
                {name: 'cfgI', start: L.cfgI, size: 65*4},
                {name: 'cfgD', start: L.cfgD, size: 3*8},
                {name: 'cRe', start: L.cRe, size: nc*8},
                {name: 'cIm', start: L.cIm, size: nc*8},
                {name: 'wCR', start: L.wCR, size: nc*8},
                {name: 'wCI', start: L.wCI, size: nc*8},
                {name: 'tRe', start: L.tRe, size: nr*8},
                {name: 'tIm', start: L.tIm, size: nr*8},
                {name: 'pRR', start: L.pRR, size: nr*8},
                {name: 'pRI', start: L.pRI, size: nr*8},
                {name: 'piO', start: L.piO, size: 500*4},
                {name: 'prO', start: L.prO, size: 500},
                {name: 'pgO', start: L.pgO, size: 500},
                {name: 'pbO', start: L.pbO, size: 500}
            ];
            sections.sort(function(a,b) { return a.start - b.start; });
            var overlaps = [];
            for (var i = 0; i < sections.length - 1; i++) {
                var end = sections[i].start + sections[i].size;
                if (end > sections[i+1].start) {
                    overlaps.push(sections[i].name + ' overlaps ' + sections[i+1].name);
                }
            }
            return overlaps;
        }""")
        assert len(result) == 0, f"Section overlaps: {result}"


# ============================================================
# WASM step loop init and run (unit-level, in-page)
# ============================================================

class TestWasmStepLoopInit:
    def test_init_reads_config_correctly(self, page, wasm_step_loop_b64):
        """After init, WASM produces correct pixel count and uniform color."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 10, colorMode: 0, uR: 255, uG: 0, uB: 0});
            var rootMags = [];
            for (var i = 0; i < 3; i++) {
                rootMags.push(Math.sqrt(r.pRR[i]*r.pRR[i] + r.pRI[i]*r.pRI[i]));
            }
            return {
                paintCount: r.paintCount,
                firstR: r.paintCount > 0 ? r.paintR[0] : -1,
                firstG: r.paintCount > 0 ? r.paintG[0] : -1,
                rootMags: rootMags
            };
        }""", wasm_step_loop_b64)

        assert result["paintCount"] > 0, "WASM should produce pixels"
        assert result["paintCount"] == 30, "10 steps * 3 roots = 30 pixels"
        assert result["firstR"] == 255
        assert result["firstG"] == 0
        for i, mag in enumerate(result["rootMags"]):
            assert abs(mag - 1.0) < 0.01, f"Root {i} magnitude {mag} != 1"


class TestWasmSolverCorrectness:
    def test_roots_of_unity_degree3(self, page, wasm_step_loop_b64):
        """WASM step loop solver finds correct roots of z^3 - 1."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 1, stepEnd: 1});
            var roots = [];
            for (var i = 0; i < 3; i++) {
                roots.push({re: r.pRR[i], im: r.pRI[i],
                    mag: Math.sqrt(r.pRR[i]*r.pRR[i] + r.pRI[i]*r.pRI[i])});
            }
            return roots;
        }""", wasm_step_loop_b64)

        for i, root in enumerate(result):
            assert abs(root["mag"] - 1.0) < 1e-6, f"Root {i} magnitude {root['mag']} far from 1"

    def test_roots_of_unity_degree5(self, page, wasm_step_loop_b64):
        """WASM step loop solver finds correct roots of z^5 - 1."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 6, 5, 100, [1, 0, 0, 0, 0, -1], [0, 0, 0, 0, 0, 0],
                {totalSteps: 1, stepEnd: 1});
            var mags = [];
            for (var i = 0; i < 5; i++) {
                mags.push(Math.sqrt(r.pRR[i]*r.pRR[i] + r.pRI[i]*r.pRI[i]));
            }
            return {paintCount: r.paintCount, mags: mags};
        }""", wasm_step_loop_b64)

        assert result["paintCount"] == 5, "1 step * 5 roots = 5 pixels"
        for i, mag in enumerate(result["mags"]):
            assert abs(mag - 1.0) < 1e-6, f"Root {i} magnitude {mag} far from 1"

    def test_roots_match_known_polynomial(self, page, wasm_step_loop_b64):
        """WASM solver finds roots of z^2 + 1 = 0 (should be +i, -i)."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 3, 2, 100, [1, 0, 1], [0, 0, 0],
                {totalSteps: 1, stepEnd: 1});
            var roots = [];
            for (var i = 0; i < 2; i++) {
                roots.push({re: r.pRR[i], im: r.pRI[i]});
            }
            roots.sort(function(a, b) { return a.im - b.im; });
            return roots;
        }""", wasm_step_loop_b64)

        assert abs(result[0]["re"]) < 1e-6, "First root real part should be ~0"
        assert abs(result[0]["im"] - (-1.0)) < 1e-6, "First root should be -i"
        assert abs(result[1]["re"]) < 1e-6, "Second root real part should be ~0"
        assert abs(result[1]["im"] - 1.0) < 1e-6, "Second root should be +i"


# ============================================================
# End-to-end fast mode: WASM and JS produce pixels in the app
# ============================================================

class TestWasmFastModeEndToEnd:
    def _run_fast_mode_and_count_pixels(self, page, solver_type):
        """Helper: run fast mode with given solver, return pixel count."""
        page.evaluate(f"solverType = '{solver_type}'")
        page.evaluate("""() => {
            if (coefficients.length >= 3 && coefficients[1].pathType === 'none') {
                coefficients[1].pathType = 'circle';
                coefficients[1].radius = 0.3;
                coefficients[1].speed = 1;
            }
        }""")
        page.evaluate("enterFastMode()")
        page.wait_for_timeout(3000)
        page.evaluate("exitFastMode()")
        page.wait_for_timeout(500)

        result = page.evaluate("""() => {
            var c = document.getElementById('bitmap-canvas');
            if (!c) return {pixels: 0, hasCanvas: false};
            var ctx = c.getContext('2d');
            var w = c.width, h = c.height;
            var imgData = ctx.getImageData(0, 0, w, h);
            var nonZero = 0;
            for (var i = 0; i < imgData.data.length; i += 4) {
                if (imgData.data[i] !== 0 || imgData.data[i+1] !== 0 || imgData.data[i+2] !== 0) nonZero++;
            }
            return {pixels: nonZero, hasCanvas: true, width: w, height: h};
        }""")
        return result

    def test_wasm_fast_mode_produces_pixels(self, page):
        """Run fast mode with WASM solver, verify pixels are painted."""
        result = self._run_fast_mode_and_count_pixels(page, "wasm")
        assert result["hasCanvas"], "Bitmap canvas not found"
        assert result["pixels"] > 0, "WASM fast mode produced zero pixels"

    def test_js_fast_mode_produces_pixels(self, page):
        """Run fast mode with JS solver, verify pixels are painted (control test)."""
        result = self._run_fast_mode_and_count_pixels(page, "js")
        assert result["hasCanvas"], "Bitmap canvas not found"
        assert result["pixels"] > 0, "JS fast mode produced zero pixels"


# ============================================================
# WASM vs JS pixel output comparison
# ============================================================

class TestWasmVsJsComparison:
    def test_wasm_and_js_produce_similar_pixel_counts(self, page):
        """WASM and JS should produce similar numbers of pixels for the same polynomial."""
        results = {}
        for solver in ["js", "wasm"]:
            page.evaluate(f"solverType = '{solver}'")
            page.evaluate("""() => {
                if (coefficients.length >= 3 && coefficients[1].pathType === 'none') {
                    coefficients[1].pathType = 'circle';
                    coefficients[1].radius = 0.3;
                    coefficients[1].speed = 1;
                }
            }""")
            page.evaluate("enterFastMode()")
            page.wait_for_timeout(2000)
            page.evaluate("exitFastMode()")
            page.wait_for_timeout(500)

            result = page.evaluate("""() => {
                var c = document.getElementById('bitmap-canvas');
                if (!c) return {pixels: 0};
                var ctx = c.getContext('2d');
                var imgData = ctx.getImageData(0, 0, c.width, c.height);
                var nonZero = 0;
                for (var i = 0; i < imgData.data.length; i += 4) {
                    if (imgData.data[i] !== 0 || imgData.data[i+1] !== 0 || imgData.data[i+2] !== 0) nonZero++;
                }
                return {pixels: nonZero};
            }""")
            results[solver] = result["pixels"]

            page.evaluate("""() => {
                var c = document.getElementById('bitmap-canvas');
                if (c) { var ctx = c.getContext('2d'); ctx.clearRect(0, 0, c.width, c.height); }
                if (typeof bitmapPersistentBuffer !== 'undefined' && bitmapPersistentBuffer) {
                    bitmapPersistentBuffer.data.fill(0);
                }
            }""")

        assert results["js"] > 0, "JS produced no pixels"
        assert results["wasm"] > 0, "WASM produced no pixels"
        ratio = max(results["js"], results["wasm"]) / max(min(results["js"], results["wasm"]), 1)
        assert ratio < 5, f"JS ({results['js']}) and WASM ({results['wasm']}) pixel counts differ by {ratio:.1f}x"


# ============================================================
# Color mode tests
# ============================================================

class TestWasmColorModes:
    def test_uniform_color_mode(self, page, wasm_step_loop_b64):
        """WASM uniform color mode outputs the configured color."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 5, stepEnd: 5, colorMode: 0, uR: 128, uG: 64, uB: 32});
            var allR = true, allG = true, allB = true;
            for (var i = 0; i < r.paintCount; i++) {
                if (r.paintR[i] !== 128) allR = false;
                if (r.paintG[i] !== 64) allG = false;
                if (r.paintB[i] !== 32) allB = false;
            }
            return {pc: r.paintCount, allR: allR, allG: allG, allB: allB};
        }""", wasm_step_loop_b64)

        assert result["pc"] == 15, "5 steps * 3 roots = 15"
        assert result["allR"], "Not all red channel = 128"
        assert result["allG"], "Not all green channel = 64"
        assert result["allB"], "Not all blue channel = 32"

    def test_index_rainbow_color_mode(self, page, wasm_step_loop_b64):
        """WASM index-rainbow mode uses per-root colors."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 5, stepEnd: 5, colorMode: 1, matchStrategy: 1,
                 rootColorsR: [255, 0, 0], rootColorsG: [0, 255, 0], rootColorsB: [0, 0, 255]});
            var colors = {};
            for (var i = 0; i < r.paintCount; i++) {
                var key = r.paintR[i] + ',' + r.paintG[i] + ',' + r.paintB[i];
                colors[key] = (colors[key] || 0) + 1;
            }
            return {pc: r.paintCount, distinctColors: Object.keys(colors).length};
        }""", wasm_step_loop_b64)

        assert result["pc"] == 15
        assert result["distinctColors"] >= 2, "Index-rainbow should produce multiple distinct colors"

    def test_uniform_vs_rainbow_differ(self, page, wasm_step_loop_b64):
        """Uniform and index-rainbow modes produce different color distributions."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r1 = setupAndRun(w, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 5, stepEnd: 5, colorMode: 0, uR: 255, uG: 255, uB: 255});
            var uniformColors = {};
            for (var i = 0; i < r1.paintCount; i++) {
                var key = r1.paintR[i] + ',' + r1.paintG[i] + ',' + r1.paintB[i];
                uniformColors[key] = true;
            }
            var w2 = instantiateWasm(b64);
            var r2 = setupAndRun(w2, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 5, stepEnd: 5, colorMode: 1, matchStrategy: 1,
                 rootColorsR: [255, 0, 0], rootColorsG: [0, 255, 0], rootColorsB: [0, 0, 255]});
            var rainbowColors = {};
            for (var i = 0; i < r2.paintCount; i++) {
                var key = r2.paintR[i] + ',' + r2.paintG[i] + ',' + r2.paintB[i];
                rainbowColors[key] = true;
            }
            return {
                uniformDistinct: Object.keys(uniformColors).length,
                rainbowDistinct: Object.keys(rainbowColors).length
            };
        }""", wasm_step_loop_b64)

        assert result["uniformDistinct"] == 1, "Uniform should have exactly 1 color"
        assert result["rainbowDistinct"] >= 2, "Rainbow should have multiple colors"


# ============================================================
# Worker integration — WASM init message and fallback
# ============================================================

class TestWorkerWasmIntegration:
    def test_wasm_step_loop_b64_present(self, page):
        """WASM_STEP_LOOP_B64 constant is present and non-trivial."""
        result = page.evaluate("""() => {
            return typeof WASM_STEP_LOOP_B64 === 'string' && WASM_STEP_LOOP_B64.length > 1000;
        }""")
        assert result, "WASM_STEP_LOOP_B64 constant not found or too short"

    def test_serialized_data_has_curve_counts(self, page):
        """serializeFastModeData source includes totalCPts and totalDPts fields."""
        result = page.evaluate("""() => {
            var src = serializeFastModeData.toString();
            return {
                hasTotalCPts: src.indexOf('totalCPts') !== -1,
                hasTotalDPts: src.indexOf('totalDPts') !== -1
            };
        }""")
        assert result["hasTotalCPts"], "totalCPts missing from serializeFastModeData"
        assert result["hasTotalDPts"], "totalDPts missing from serializeFastModeData"

    def test_error_message_handled(self, page):
        """handleFastModeWorkerMessage handles 'error' type messages."""
        result = page.evaluate("""() => {
            var src = handleFastModeWorkerMessage.toString();
            return src.indexOf('error') !== -1;
        }""")
        assert result, "handleFastModeWorkerMessage doesn't handle 'error' message type"


# ============================================================
# Progress reporting
# ============================================================

class TestWasmProgressReporting:
    def test_progress_callback_fires(self, page, wasm_step_loop_b64):
        """WASM reports progress via imported reportProgress function."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _CWL + """
            var raw = atob(b64);
            var bytes = new Uint8Array(raw.length);
            for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
            var mod = new WebAssembly.Module(bytes.buffer);
            var mem = new WebAssembly.Memory({initial: 2});
            var progressCalls = [];
            var inst = new WebAssembly.Instance(mod, {
                env: {
                    memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log,
                    reportProgress: function(step) { progressCalls.push(step); }
                }
            });
            var hb = inst.exports.__heap_base ? inst.exports.__heap_base.value : 65536;
            var nc = 4, nr = 3;
            var L = computeWasmLayout(nc, nr, 30000, 0, 0, 0, 0, 0, 0, hb);
            var curPages = mem.buffer.byteLength / 65536;
            if (L.pages > curPages) mem.grow(L.pages - curPages);
            var buf = mem.buffer;

            var cfgI32 = new Int32Array(buf, L.cfgI, 65);
            cfgI32[0] = nc; cfgI32[1] = nr;
            cfgI32[2] = 200; cfgI32[3] = 200;
            cfgI32[4] = 5000; cfgI32[5] = 0; cfgI32[12] = 0;
            cfgI32[13] = 255; cfgI32[14] = 255; cfgI32[15] = 255;
            cfgI32[20] = L.cRe; cfgI32[21] = L.cIm;
            cfgI32[22] = L.clR; cfgI32[23] = L.clG; cfgI32[24] = L.clB;
            cfgI32[25] = L.jRe; cfgI32[26] = L.jIm;
            cfgI32[27] = L.mTR; cfgI32[28] = L.mTI;
            cfgI32[29] = L.ppR; cfgI32[30] = L.ppG; cfgI32[31] = L.ppB;
            cfgI32[32] = L.dpR; cfgI32[33] = L.dpG; cfgI32[34] = L.dpB;
            cfgI32[35] = L.sI; cfgI32[36] = L.fCI;
            cfgI32[37] = L.eIdx; cfgI32[38] = L.eSpd; cfgI32[39] = L.eCcw; cfgI32[40] = L.eDth;
            cfgI32[41] = L.cOff; cfgI32[42] = L.cLen; cfgI32[43] = L.cCld;
            cfgI32[44] = L.dIdx; cfgI32[45] = L.dSpd; cfgI32[46] = L.dCcw; cfgI32[47] = L.dDth;
            cfgI32[48] = L.dOff; cfgI32[49] = L.dLen; cfgI32[50] = L.dCld;
            cfgI32[51] = L.cvF; cfgI32[52] = L.dcF;
            cfgI32[53] = L.wCR; cfgI32[54] = L.wCI;
            cfgI32[55] = L.tRe; cfgI32[56] = L.tIm;
            cfgI32[57] = L.mWR; cfgI32[58] = L.mWI;
            cfgI32[59] = L.pRR; cfgI32[60] = L.pRI;
            cfgI32[61] = L.piO; cfgI32[62] = L.prO; cfgI32[63] = L.pgO; cfgI32[64] = L.pbO;

            var cfgF64 = new Float64Array(buf, L.cfgD, 3);
            cfgF64[0] = 2.0; cfgF64[1] = 1.0; cfgF64[2] = 0.0;

            new Float64Array(buf, L.cRe, nc).set([1, 0, 0, -1]);
            new Float64Array(buf, L.cIm, nc).set([0, 0, 0, 0]);
            new Float64Array(buf, L.wCR, nc).set([1, 0, 0, -1]);
            new Float64Array(buf, L.wCI, nc).set([0, 0, 0, 0]);

            var pRR = new Float64Array(buf, L.pRR, nr);
            var pRI = new Float64Array(buf, L.pRI, nr);
            for (var i = 0; i < nr; i++) {
                var a = (2 * Math.PI * i) / nr + 0.37;
                pRR[i] = Math.cos(a); pRI[i] = Math.sin(a);
            }

            inst.exports.init(L.cfgI, L.cfgD);
            inst.exports.runStepLoop(0, 5000, 0.0);

            return {callCount: progressCalls.length};
        }""", wasm_step_loop_b64)

        assert result["callCount"] >= 2, f"Expected >=2 progress calls, got {result['callCount']}"


# ============================================================
# Edge cases
# ============================================================

class TestWasmEdgeCases:
    def test_degree2_polynomial(self, page, wasm_step_loop_b64):
        """WASM handles degree-2 polynomial (z^2 - 1)."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 3, 2, 100, [1, 0, -1], [0, 0, 0],
                {totalSteps: 10, stepEnd: 10});
            var rootMags = [];
            for (var i = 0; i < 2; i++) {
                rootMags.push(Math.sqrt(r.pRR[i]*r.pRR[i] + r.pRI[i]*r.pRI[i]));
            }
            return {pc: r.paintCount, rootMags: rootMags};
        }""", wasm_step_loop_b64)

        assert result["pc"] == 20, "10 steps * 2 roots = 20"
        for i, mag in enumerate(result["rootMags"]):
            assert abs(mag - 1.0) < 0.01, f"Root {i} magnitude {mag} far from 1"

    def test_zero_steps(self, page, wasm_step_loop_b64):
        """WASM handles zero steps gracefully."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 1, stepStart: 5, stepEnd: 5});
            return r.paintCount;
        }""", wasm_step_loop_b64)

        assert result == 0, "Zero steps should produce zero pixels"

    def test_large_step_count(self, page, wasm_step_loop_b64):
        """WASM handles a large number of steps without crashing."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 4, 3, 50000, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 10000, stepEnd: 10000, canvasW: 500, canvasH: 500});
            return {pc: r.paintCount, ok: r.paintCount > 0};
        }""", wasm_step_loop_b64)

        assert result["ok"], "Large step count should produce pixels"
        assert result["pc"] == 30000, "10000 steps * 3 roots = 30000 pixels"

    def test_pixel_indices_within_canvas(self, page, wasm_step_loop_b64):
        """All pixel indices should be within canvas bounds."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            var canvasW = 100, canvasH = 100;
            var r = setupAndRun(w, 4, 3, 500, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 50, stepEnd: 50, canvasW: canvasW, canvasH: canvasH});
            var maxIdx = canvasW * canvasH;
            var outOfBounds = 0, negative = 0;
            for (var i = 0; i < r.paintCount; i++) {
                if (r.paintIdx[i] < 0) negative++;
                if (r.paintIdx[i] >= maxIdx) outOfBounds++;
            }
            return {pc: r.paintCount, outOfBounds: outOfBounds, negative: negative};
        }""", wasm_step_loop_b64)

        assert result["pc"] > 0, "Should produce some pixels"
        assert result["negative"] == 0, f"{result['negative']} negative pixel indices"
        assert result["outOfBounds"] == 0, f"{result['outOfBounds']} out-of-bounds pixel indices"


# ============================================================
# WASM vs JS solver output comparison (direct, not through workers)
# ============================================================

class TestWasmVsJsSolverDirect:
    def test_same_roots_for_z3_minus_1(self, page, wasm_step_loop_b64):
        """WASM and JS (worker-blob) solvers find the same roots for z^3 - 1."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            // JS solver (main-thread version takes {re,im} objects)
            var coeffs = [{re:1,im:0},{re:0,im:0},{re:0,im:0},{re:-1,im:0}];
            var warm = [{re:0.5,im:0.5},{re:-0.5,im:0.5},{re:0.9,im:-0.1}];
            var jsRoots = solveRootsEA(coeffs, warm);
            jsRoots.sort(function(a, b) { return a.re - b.re; });

            // WASM solver (same initial guesses)
            var w = instantiateWasm(b64);
            var r = setupAndRun(w, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 1, stepEnd: 1,
                 initRootsRe: [0.5, -0.5, 0.9],
                 initRootsIm: [0.5, 0.5, -0.1]});

            var wasmRe = Array.from(new Float64Array(r.buf, r.L.pRR, 3));
            var wasmIm = Array.from(new Float64Array(r.buf, r.L.pRI, 3));
            var wasmRoots = wasmRe.map(function(re, i) { return {re: re, im: wasmIm[i]}; });
            wasmRoots.sort(function(a, b) { return a.re - b.re; });

            var maxDiff = 0;
            for (var i = 0; i < 3; i++) {
                var dRe = Math.abs(jsRoots[i].re - wasmRoots[i].re);
                var dIm = Math.abs(jsRoots[i].im - wasmRoots[i].im);
                maxDiff = Math.max(maxDiff, dRe, dIm);
            }
            return {maxDiff: maxDiff, jsRoots: jsRoots, wasmRoots: wasmRoots};
        }""", wasm_step_loop_b64)

        assert result["maxDiff"] < 1e-6, \
            f"JS and WASM roots differ by {result['maxDiff']}: JS={result['jsRoots']}, WASM={result['wasmRoots']}"
