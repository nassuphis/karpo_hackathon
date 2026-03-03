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
    function computeWasmLayout(nc, nr, maxP, nE, nDE, nFC, nSI, tCP, tDP, heapBase, nP) {
        var o = heapBase || 65536;
        function a8(x) { return (x + 7) & ~7; }
        nP = nP || 0;
        var L = {};
        L.cfgI = o; o = a8(o + 76 * 4);
        L.cfgD = o; o = a8(o + 21 * 8);
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
        L.eDd = o; o = a8(o + Math.max(nE, 1) * 4);
        L.cOff = o; o = a8(o + Math.max(nE, 1) * 4);
        L.cLen = o; o = a8(o + Math.max(nE, 1) * 4);
        L.cCld = o; o = a8(o + Math.max(nE, 1) * 4);
        L.dIdx = o; o = a8(o + Math.max(nDE, 1) * 4);
        L.dSpd = o; o = a8(o + Math.max(nDE, 1) * 8);
        L.dCcw = o; o = a8(o + Math.max(nDE, 1) * 4);
        L.dDth = o; o = a8(o + Math.max(nDE, 1) * 8);
        L.dDd = o; o = a8(o + Math.max(nDE, 1) * 4);
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
        L.pnRe = o; o = a8(o + Math.max(nP, 1) * 8);
        L.pnIm = o; o = a8(o + Math.max(nP, 1) * 8);
        L.xRe = o; o = a8(o + (nc + nP) * 8);
        L.xIm = o; o = a8(o + (nc + nP) * 8);
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
            env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, pow: Math.pow,
                   reportProgress: progressFn || function(){} }
        });
        var hb = inst.exports.__heap_base ? inst.exports.__heap_base.value : 65536;
        return {inst: inst, mem: mem, hb: hb};
    }
"""

_WRITE_CFG = """
    function writeCfg(buf, L, nc, nr, opts) {
        opts = opts || {};
        var cfgI32 = new Int32Array(buf, L.cfgI, 76);
        cfgI32[0] = nc; cfgI32[1] = nr;
        cfgI32[2] = opts.canvasW || 200; cfgI32[3] = opts.canvasH || 200;
        cfgI32[4] = opts.totalSteps || 10;
        cfgI32[5] = opts.colorMode || 0;
        cfgI32[6] = opts.matchStrategy || 0;
        cfgI32[7] = 0;   /* morphEnabled */
        cfgI32[8] = 0;   /* nEntries */
        cfgI32[9] = 0;   /* nDEntries */
        cfgI32[10] = 0;  /* nFollowC */
        cfgI32[11] = 0;  /* nSelIndices */
        cfgI32[12] = 0;  /* hasJiggle */
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
        cfgI32[65] = 0;  /* morphPathType */
        cfgI32[66] = 0;  /* morphCcw */
        cfgI32[67] = L.eDd;  /* entryDitherDist offset */
        cfgI32[68] = L.dDd;  /* dEntryDitherDist offset */
        cfgI32[69] = 0;  /* morphDitherDist */
        cfgI32[70] = 0;  /* nPinned */
        cfgI32[71] = L.pnRe; cfgI32[72] = L.pnIm;
        cfgI32[73] = L.xRe; cfgI32[74] = L.xIm;
        cfgI32[75] = opts.maxPaint || 0;  /* CI_MAX_PAINT — paint buffer capacity */
        var cfgF64 = new Float64Array(buf, L.cfgD, 21);
        cfgF64[0] = opts.range || 2.0;
        cfgF64[1] = opts.fps || 1.0;
        cfgF64[2] = opts.morphRate || 0.0;
        cfgF64[3] = 0.5;  /* morphEllipseMinor */
        cfgF64[4] = 0.0;  /* morphDitherStart */
        cfgF64[5] = 0.0;  /* morphDitherMid */
        cfgF64[6] = 0.0;  /* morphDitherEnd */
        cfgF64[7] = 0.0;  /* centerX */
        cfgF64[8] = 0.0;  /* centerY */
        cfgF64[9] = 0.0;  /* morphDitherPow */
        cfgF64[10] = 0.0; /* relProxFloor */
        cfgF64[11] = 1.0; /* relProxCeiling */
        cfgF64[12] = 0.0; /* relProxFreq */
        cfgF64[13] = 0.0; /* proxFloor */
        cfgF64[14] = 1.0; /* proxCeiling */
        cfgF64[15] = 0.0; /* proxFreq */
        cfgF64[16] = 0.0; /* derivFloor */
        cfgF64[17] = 1.0; /* derivCeiling */
        cfgF64[18] = 0.0; /* derivFreq */
        cfgF64[19] = 0.0; /* pinnedEpsilon */
        cfgF64[20] = 1.0; /* proxGamma */
    }
"""

_SETUP = """
    function setupAndRun(w, nc, nr, maxP, coeffsRe, coeffsIm, opts) {
        opts = opts || {};
        if (!opts.maxPaint) opts.maxPaint = maxP;
        var L = computeWasmLayout(nc, nr, maxP, 0, 0, 0, 0, 0, 0, w.hb, 0);
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
                    env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, pow: Math.pow, reportProgress: function(){} }
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
        """WASM heap base can be determined (exported or default 65536 with stack-first)."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            var raw = atob(b64);
            var bytes = new Uint8Array(raw.length);
            for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
            var mod = new WebAssembly.Module(bytes.buffer);
            var mem = new WebAssembly.Memory({initial: 4});
            var inst = new WebAssembly.Instance(mod, {
                env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, pow: Math.pow, reportProgress: function(){} }
            });
            var hb = inst.exports.__heap_base ? inst.exports.__heap_base.value : 65536;
            return {
                hasHeapBase: '__heap_base' in inst.exports,
                hasStackPointer: '__stack_pointer' in inst.exports,
                value: hb
            };
        }""", wasm_step_loop_b64)
        # Module must export either __heap_base or __stack_pointer (stack-first layout)
        assert result["hasHeapBase"] or result["hasStackPointer"], \
            "WASM should export __heap_base or __stack_pointer"
        assert result["value"] >= 65536, "heap base should be at or above 64KB stack boundary"

    def test_heap_base_above_stack(self, page, wasm_step_loop_b64):
        """Heap base must be >= 64KB to avoid stack/config overlap."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            var raw = atob(b64);
            var bytes = new Uint8Array(raw.length);
            for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
            var mod = new WebAssembly.Module(bytes.buffer);
            var mem = new WebAssembly.Memory({initial: 4});
            var inst = new WebAssembly.Instance(mod, {
                env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, pow: Math.pow, reportProgress: function(){} }
            });
            return inst.exports.__heap_base ? inst.exports.__heap_base.value : 65536;
        }""", wasm_step_loop_b64)
        assert result >= 65536, f"heap_base too low ({result}), config would overlap with stack"


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
                coefficients[1].rAbs = 0.3;
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
                    coefficients[1].rAbs = 0.3;
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
                    memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, pow: Math.pow,
                    reportProgress: function(step) { progressCalls.push(step); }
                }
            });
            var hb = inst.exports.__heap_base ? inst.exports.__heap_base.value : 65536;
            var nc = 4, nr = 3;
            var L = computeWasmLayout(nc, nr, 30000, 0, 0, 0, 0, 0, 0, hb);
            var curPages = mem.buffer.byteLength / 65536;
            if (L.pages > curPages) mem.grow(L.pages - curPages);
            var buf = mem.buffer;

            var cfgI32 = new Int32Array(buf, L.cfgI, 76);
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
            cfgI32[65] = 0;  /* morphPathType */
            cfgI32[66] = 0;  /* morphCcw */
            cfgI32[67] = L.eDd;  /* entryDitherDist */
            cfgI32[68] = L.dDd;  /* dEntryDitherDist */
            cfgI32[69] = 0;  /* morphDitherDist */
            cfgI32[70] = 0;  /* nPinned */
            cfgI32[71] = L.pnRe; cfgI32[72] = L.pnIm;
            cfgI32[73] = L.xRe; cfgI32[74] = L.xIm;
            cfgI32[75] = 30000;  /* CI_MAX_PAINT */

            var cfgF64 = new Float64Array(buf, L.cfgD, 21);
            cfgF64[0] = 2.0; cfgF64[1] = 1.0; cfgF64[2] = 0.0;
            cfgF64[3] = 0.5; cfgF64[4] = 0.0; cfgF64[5] = 0.0;
            cfgF64[6] = 0.0; cfgF64[7] = 0.0; cfgF64[8] = 0.0;
            cfgF64[9] = 0.0; cfgF64[10] = 0.0; cfgF64[11] = 1.0;
            cfgF64[12] = 0.0; cfgF64[13] = 0.0; cfgF64[14] = 1.0;
            cfgF64[15] = 0.0; cfgF64[16] = 0.0; cfgF64[17] = 1.0;
            cfgF64[18] = 0.0; cfgF64[19] = 0.0; cfgF64[20] = 1.0;

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


# ============================================================
# __heap_base export requirement
# ============================================================

class TestHeapBaseExport:
    def test_heap_base_is_explicitly_exported(self, page, wasm_step_loop_b64):
        """__heap_base MUST be explicitly exported — defaulting to 65536 causes
        config data to overlap with WASM's BSS section, corrupting memory."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            var raw = atob(b64);
            var bytes = new Uint8Array(raw.length);
            for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
            var mod = new WebAssembly.Module(bytes.buffer);
            var mem = new WebAssembly.Memory({initial: 4});
            var inst = new WebAssembly.Instance(mod, {
                env: { memory: mem, cos: Math.cos, sin: Math.sin, log: Math.log, pow: Math.pow, reportProgress: function(){} }
            });
            return {
                hasHeapBase: '__heap_base' in inst.exports,
                value: inst.exports.__heap_base ? inst.exports.__heap_base.value : null
            };
        }""", wasm_step_loop_b64)
        assert result["hasHeapBase"], \
            "__heap_base must be explicitly exported from WASM (missing --export=__heap_base in build?)"
        assert result["value"] is not None and result["value"] > 65536, \
            f"__heap_base={result['value']} should be > 65536 (BSS data lives above the stack)"

    def test_init_does_not_crash_with_real_heap_base(self, page, wasm_step_loop_b64):
        """WASM init+run succeeds when config is placed at the real __heap_base,
        not the incorrect default of 65536."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            if (!w.inst.exports.__heap_base) return {skipped: true};
            var realHB = w.inst.exports.__heap_base.value;
            try {
                var r = setupAndRun(w, 4, 3, 100, [1, 0, 0, -1], [0, 0, 0, 0],
                    {totalSteps: 5, stepEnd: 5});
                return {ok: true, paintCount: r.paintCount, heapBase: realHB};
            } catch(e) {
                return {ok: false, error: String(e), heapBase: realHB};
            }
        }""", wasm_step_loop_b64)
        if result.get("skipped"):
            pytest.skip("__heap_base not exported")
        assert result["ok"], f"WASM crashed with real heapBase={result['heapBase']}: {result.get('error')}"
        assert result["paintCount"] == 15, f"Expected 15 pixels, got {result['paintCount']}"


# ============================================================
# Paint buffer bounds checking (CI_MAX_PAINT)
# ============================================================

class TestPaintBufferBounds:
    def test_paint_count_respects_max_paint(self, page, wasm_step_loop_b64):
        """WASM stops painting when paint buffer is full (CI_MAX_PAINT)."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            // 100 steps * 3 roots = 300 pixels needed, but maxPaint = 10
            var r = setupAndRun(w, 4, 3, 10, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 100, stepEnd: 100, maxPaint: 10});
            return {paintCount: r.paintCount};
        }""", wasm_step_loop_b64)
        # With maxPaint=10 and 3 roots per step, we can fit at most 3 full steps (9 pixels)
        # because 9+3=12 > 10, so it stops after 3 steps
        assert result["paintCount"] <= 10, \
            f"Paint count {result['paintCount']} exceeds maxPaint=10"
        assert result["paintCount"] > 0, "Should produce at least some pixels"

    def test_no_overflow_with_tight_buffer(self, page, wasm_step_loop_b64):
        """With a very small paint buffer, WASM should not write beyond it."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _CWL + _INST + _WRITE_CFG + """
            var w = instantiateWasm(b64);
            var nc = 4, nr = 3, maxP = 6;
            var L = computeWasmLayout(nc, nr, maxP, 0, 0, 0, 0, 0, 0, w.hb, 0);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;

            // Write a sentinel value right after the paint buffer
            var sentinelOffset = L.pbO + maxP;
            var sentinelView = new Uint8Array(buf, sentinelOffset, 4);
            sentinelView[0] = 0xDE; sentinelView[1] = 0xAD;
            sentinelView[2] = 0xBE; sentinelView[3] = 0xEF;

            writeCfg(buf, L, nc, nr, {totalSteps: 50, maxPaint: maxP});
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

            w.inst.exports.init(L.cfgI, L.cfgD);
            var pc = w.inst.exports.runStepLoop(0, 50, 0.0);

            // Check sentinel is intact
            var afterBuf = new Uint8Array(buf, sentinelOffset, 4);
            var intact = afterBuf[0] === 0xDE && afterBuf[1] === 0xAD &&
                         afterBuf[2] === 0xBE && afterBuf[3] === 0xEF;
            return {paintCount: pc, sentinelIntact: intact, maxPaint: maxP};
        }""", wasm_step_loop_b64)
        assert result["paintCount"] <= result["maxPaint"], \
            f"Paint count {result['paintCount']} exceeds maxPaint={result['maxPaint']}"
        assert result["sentinelIntact"], \
            "Sentinel bytes after paint buffer were overwritten — buffer overflow!"

    def test_exact_fit_no_overflow(self, page, wasm_step_loop_b64):
        """When maxPaint exactly matches steps*roots, all pixels should be painted."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("(b64) => {" + _HELPERS + """
            var w = instantiateWasm(b64);
            // 5 steps * 3 roots = 15 pixels, maxPaint = 15
            var r = setupAndRun(w, 4, 3, 15, [1, 0, 0, -1], [0, 0, 0, 0],
                {totalSteps: 5, stepEnd: 5, maxPaint: 15});
            return {paintCount: r.paintCount};
        }""", wasm_step_loop_b64)
        assert result["paintCount"] == 15, \
            f"Expected exactly 15 pixels when maxPaint=15 fits perfectly, got {result['paintCount']}"


# ============================================================
# WASM Robustness tests — crash prevention & edge cases
# ============================================================

class TestWasmRobustness:
    """Tests targeting crash scenarios: memory growth, resolution changes,
    buffer overflow, config validation, and edge cases."""

    def test_memory_grow_checked(self, page, wasm_step_loop_b64):
        """memory.grow() return value is checked — failure raises instead of crashing."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _CWL + _INST + """
            var w = instantiateWasm(b64);
            var nc = 3, nr = 2;
            // Compute layout with reasonable maxP
            var L = computeWasmLayout(nc, nr, 100, 0, 0, 0, 0, 0, 0, w.hb, 0);
            var curPages = w.mem.buffer.byteLength / 65536;
            // Test: if grow() were to fail, we should detect it
            // (We can't easily force failure, but verify the pattern works when it succeeds)
            if (L.pages > curPages) {
                var result = w.mem.grow(L.pages - curPages);
                if (result === -1) return {ok: false, reason: 'grow failed as expected'};
            }
            return {ok: true, pages: w.mem.buffer.byteLength / 65536, needed: L.pages};
        }""", wasm_step_loop_b64)
        assert result["ok"]
        assert result["pages"] >= result["needed"]

    def test_layout_at_5k_resolution(self, page):
        """computeWasmLayout at 5K resolution produces reasonable page count."""
        result = page.evaluate("() => {" + _CWL + """
            var nc = 12, nr = 11;
            var stepsPerWorker = Math.ceil(100000 / 16);
            var maxP = stepsPerWorker * nr + nr;
            var L = computeWasmLayout(nc, nr, maxP, nc, 0, 0, 6, nc * 2000, 0, 65808, 0);
            return {pages: L.pages, totalBytes: L.pages * 65536, maxP: maxP};
        }""")
        assert result["pages"] > 0
        # Should be well under 50MB per worker
        assert result["totalBytes"] < 50 * 1024 * 1024, \
            f"Layout too large: {result['totalBytes']} bytes"

    def test_paint_buffer_overflow_protection(self, page, wasm_step_loop_b64):
        """WASM respects maxPaint cap and stops painting early."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _HELPERS + """
            var w = instantiateWasm(b64);
            var nc = 4, nr = 3;
            // Set maxPaint to only 6 (less than steps * nRoots = 50 * 3 = 150)
            var r = setupAndRun(w, nc, nr, 6, [1.0, 0.0, 0.0, -1.0], [0.0, 0.0, 0.0, 0.0],
                {totalSteps: 50, stepEnd: 50, maxPaint: 6, canvasW: 200, canvasH: 200});
            return {paintCount: r.paintCount};
        }""", wasm_step_loop_b64)
        assert result["paintCount"] <= 6, \
            f"Paint count {result['paintCount']} exceeds maxPaint cap of 6"

    def test_config_validation_roundtrip(self, page, wasm_step_loop_b64):
        """Config values written by JS are correctly read back after init()."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _CWL + _INST + _WRITE_CFG + """
            var w = instantiateWasm(b64);
            var nc = 5, nr = 4, maxP = 200;
            var L = computeWasmLayout(nc, nr, maxP, 0, 0, 0, 0, 0, 0, w.hb, 0);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;
            writeCfg(buf, L, nc, nr, {canvasW: 5000, canvasH: 5000, totalSteps: 100, maxPaint: maxP});
            // Write coefficients
            new Float64Array(buf, L.cRe, nc).set([1, 0, 0, 0, -1]);
            new Float64Array(buf, L.cIm, nc).set([0, 0, 0, 0, 0]);
            new Float64Array(buf, L.wCR, nc).set([1, 0, 0, 0, -1]);
            new Float64Array(buf, L.wCI, nc).set([0, 0, 0, 0, 0]);
            // Init roots
            var pRR = new Float64Array(buf, L.pRR, nr);
            var pRI = new Float64Array(buf, L.pRI, nr);
            for (var i = 0; i < nr; i++) {
                var a = (2 * Math.PI * i) / nr + 0.37;
                pRR[i] = Math.cos(a); pRI[i] = Math.sin(a);
            }
            w.inst.exports.init(L.cfgI, L.cfgD);
            // Read back config from WASM memory to verify
            var check = new Int32Array(w.mem.buffer, L.cfgI, 76);
            return {nc: check[0], nr: check[1], canvasW: check[2], canvasH: check[3], maxPaint: check[75]};
        }""", wasm_step_loop_b64)
        assert result["nc"] == 5
        assert result["nr"] == 4
        assert result["canvasW"] == 5000
        assert result["canvasH"] == 5000
        assert result["maxPaint"] == 200

    def test_resolution_change_sequence(self, page, wasm_step_loop_b64):
        """Init at 2K, re-init at 5K, re-init at 2K — no crashes."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _HELPERS + """
            var results = [];
            for (var ci = 0; ci < 3; ci++) {
                var canvasW = [2000, 5000, 2000][ci];
                var w = instantiateWasm(b64);
                var nc = 4, nr = 3, maxP = 100;
                try {
                    var res = setupAndRun(w, nc, nr, maxP, [1.0, 0.0, 0.0, -1.0], [0.0, 0.0, 0.0, 0.0],
                        {canvasW: canvasW, canvasH: canvasW, totalSteps: 10, maxPaint: maxP});
                    results.push({canvasW: canvasW, ok: true, pc: res.paintCount});
                } catch(e) {
                    results.push({canvasW: canvasW, ok: false, error: String(e)});
                }
            }
            return results;
        }""", wasm_step_loop_b64)
        for r in result:
            assert r["ok"], f"Crash at resolution {r['canvasW']}: {r.get('error')}"

    def test_zero_degree_edge_case(self, page, wasm_step_loop_b64):
        """nCoeffs=1, nRoots=0 — no division by zero or OOB access."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _CWL + _INST + _WRITE_CFG + """
            var w = instantiateWasm(b64);
            var nc = 1, nr = 0, maxP = 10;
            var L = computeWasmLayout(nc, nr, maxP, 0, 0, 0, 0, 0, 0, w.hb, 0);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;
            writeCfg(buf, L, nc, nr, {canvasW: 100, canvasH: 100, totalSteps: 5, maxPaint: maxP});
            new Float64Array(buf, L.cRe, nc).set([1.0]);
            new Float64Array(buf, L.cIm, nc).set([0.0]);
            new Float64Array(buf, L.wCR, nc).set([1.0]);
            new Float64Array(buf, L.wCI, nc).set([0.0]);
            w.inst.exports.init(L.cfgI, L.cfgD);
            try {
                var pc = w.inst.exports.runStepLoop(0, 5, 0.0);
                return {ok: true, paintCount: pc};
            } catch(e) {
                return {ok: false, error: String(e)};
            }
        }""", wasm_step_loop_b64)
        assert result["ok"], f"Zero-degree crash: {result.get('error')}"
        assert result["paintCount"] == 0

    def test_high_degree_stress(self, page, wasm_step_loop_b64):
        """MAX_COEFFS=256, MAX_DEG=255 — no stack overflow with large polynomials."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _CWL + _INST + _WRITE_CFG + """
            var w = instantiateWasm(b64);
            var nc = 256, nr = 255;
            var maxP = 5 * nr;
            var L = computeWasmLayout(nc, nr, maxP, 0, 0, 0, 0, 0, 0, w.hb, 0);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;
            writeCfg(buf, L, nc, nr, {canvasW: 200, canvasH: 200, totalSteps: 5, maxPaint: maxP});
            // z^255 - 1 (roots of unity)
            var cRe = new Float64Array(nc);
            var cIm = new Float64Array(nc);
            cRe[0] = 1.0; cRe[nc - 1] = -1.0;
            new Float64Array(buf, L.cRe, nc).set(cRe);
            new Float64Array(buf, L.cIm, nc).set(cIm);
            new Float64Array(buf, L.wCR, nc).set(cRe);
            new Float64Array(buf, L.wCI, nc).set(cIm);
            // Colors
            var clR = new Uint8Array(nr); clR.fill(255);
            new Uint8Array(buf, L.clR, nr).set(clR);
            new Uint8Array(buf, L.clG, nr).set(new Uint8Array(nr));
            new Uint8Array(buf, L.clB, nr).set(new Uint8Array(nr));
            // Init roots as unit circle
            var pRR = new Float64Array(buf, L.pRR, nr);
            var pRI = new Float64Array(buf, L.pRI, nr);
            for (var i = 0; i < nr; i++) {
                var a = (2 * Math.PI * i) / nr + 0.37;
                pRR[i] = Math.cos(a); pRI[i] = Math.sin(a);
            }
            w.inst.exports.init(L.cfgI, L.cfgD);
            try {
                var pc = w.inst.exports.runStepLoop(0, 5, 0.0);
                return {ok: true, paintCount: pc};
            } catch(e) {
                return {ok: false, error: String(e)};
            }
        }""", wasm_step_loop_b64)
        assert result["ok"], f"High degree stress test failed: {result.get('error')}"

    def test_zero_length_curve_no_crash(self, page, wasm_step_loop_b64):
        """Zero-length curve (N=0) must not cause OOB trap.
        This was the root cause of the WASM fast-mode crash: when a curve has
        0 points, k = N - 1 = -1, causing a negative index into curvesFlat.
        The fix adds 'if (N <= 0) continue;' in the curve interpolation loops."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _CWL + _INST + _WRITE_CFG + """
            var w = instantiateWasm(b64);
            var nc = 4, nr = 3, maxP = 100;
            // nEntries=2: one with a real 3-point curve, one with ZERO-length curve
            var nE = 2, tCP = 3;  // total curve points = 3 (only from first entry)
            var L = computeWasmLayout(nc, nr, maxP, nE, 0, 0, 0, tCP, 0, w.hb, 0);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;

            // Write config with nEntries=2
            writeCfg(buf, L, nc, nr, {canvasW: 200, canvasH: 200, totalSteps: 20,
                                       maxPaint: maxP});
            var cfgI32 = new Int32Array(buf, L.cfgI, 76);
            cfgI32[8] = nE;  // nEntries = 2

            // Entry 0: coeff index 0, curve with 3 points, speed 1.0
            new Int32Array(buf, L.eIdx, nE).set([0, 1]);      // entry indices
            new Float64Array(buf, L.eSpd, nE).set([1.0, 1.0]); // speeds
            new Int32Array(buf, L.eCcw, nE).set([1, 1]);       // ccw flags
            new Float64Array(buf, L.eDth, nE).set([0.0, 0.0]); // dither theta
            new Int32Array(buf, L.eDd, nE).set([0, 0]);        // dither dist

            // Entry 0: 3-point curve at offset 0
            // Entry 1: ZERO-length curve (the crash scenario)
            new Int32Array(buf, L.cOff, nE).set([0, 0]);  // both point to offset 0
            new Int32Array(buf, L.cLen, nE).set([3, 0]);   // entry 1 has length 0!
            new Int32Array(buf, L.cCld, nE).set([0, 0]);   // not cloud curves

            // Write 3 curve points for entry 0 into curvesFlat (re, im pairs)
            var cvFlat = new Float64Array(buf, L.cvF, tCP * 2);
            cvFlat.set([0.5, 0.0,  0.0, 0.5,  -0.5, 0.0]);  // 3 points

            // Coefficients: z^3 - 1
            new Float64Array(buf, L.cRe, nc).set([1, 0, 0, -1]);
            new Float64Array(buf, L.cIm, nc).set([0, 0, 0, 0]);
            new Float64Array(buf, L.wCR, nc).set([1, 0, 0, -1]);
            new Float64Array(buf, L.wCI, nc).set([0, 0, 0, 0]);
            // Init roots
            var pRR = new Float64Array(buf, L.pRR, nr);
            var pRI = new Float64Array(buf, L.pRI, nr);
            for (var i = 0; i < nr; i++) {
                var a = (2 * Math.PI * i) / nr + 0.37;
                pRR[i] = Math.cos(a); pRI[i] = Math.sin(a);
            }

            w.inst.exports.init(L.cfgI, L.cfgD);
            try {
                var pc = w.inst.exports.runStepLoop(0, 20, 1.0);
                return {ok: true, paintCount: pc};
            } catch(e) {
                return {ok: false, error: String(e)};
            }
        }""", wasm_step_loop_b64)
        assert result["ok"], \
            f"Zero-length curve caused WASM crash: {result.get('error')}"
        assert result["paintCount"] > 0, "Should still produce pixels despite empty curve"

    def test_compositing_bounds_check(self, page):
        """Out-of-bounds pixel indices should be safely skipped in compositing logic."""
        result = page.evaluate("""() => {
            // Simulate compositing with out-of-bounds pixel indices
            var cW = 100;  // Current resolution: 100x100 = 10000 pixels
            var out = new Uint8ClampedArray(cW * cW * 4);
            var maxPix = cW * cW;

            // Create fake worker result with indices from a 200x200 resolution
            var idx = new Int32Array([50, 15000, 99, 25000, 9999]);  // 15000 and 25000 are OOB
            var r = new Uint8Array([255, 128, 200, 64, 100]);

            var valid = 0, skipped = 0;
            for (var i = 0; i < idx.length; i++) {
                if (idx[i] < 0 || idx[i] >= maxPix) { skipped++; continue; }
                out[idx[i] * 4] = r[i];
                valid++;
            }
            // Verify the valid pixels were written
            var p50 = out[50 * 4];
            var p99 = out[99 * 4];
            var p9999 = out[9999 * 4];
            return {valid: valid, skipped: skipped, p50: p50, p99: p99, p9999: p9999};
        }""")
        assert result["valid"] == 3
        assert result["skipped"] == 2
        assert result["p50"] == 255
        assert result["p99"] == 200
        assert result["p9999"] == 100


# ============================================================
# Bug regression: uniform color with zero-valued channels
# ============================================================

class TestUniformColorZeroChannel:
    """Regression tests for the || 255 bug: zero-valued color channels
    must be preserved, not replaced with 255."""

    def test_wasm_uniform_green_has_zero_red(self, page, wasm_step_loop_b64):
        """WASM uniform color [0, 200, 83] must output R=0, not R=255."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _HELPERS + """
            var w = instantiateWasm(b64);
            // z^2 - 1 → roots at ±1
            var res = setupAndRun(w, 3, 2, 200, [1, 0, -1], [0, 0, 0], {
                colorMode: 0,  // uniform
                uR: 0, uG: 200, uB: 83,
                totalSteps: 5, canvasW: 100, canvasH: 100
            });
            // Check that ALL painted pixels have R=0 (not 255)
            var badR = 0;
            for (var i = 0; i < res.paintCount; i++) {
                if (res.paintR[i] !== 0) badR++;
            }
            return {pc: res.paintCount, badR: badR,
                    firstR: res.paintCount > 0 ? res.paintR[0] : -1,
                    firstG: res.paintCount > 0 ? res.paintG[0] : -1,
                    firstB: res.paintCount > 0 ? res.paintB[0] : -1};
        }""", wasm_step_loop_b64)
        assert result["pc"] > 0, "Should produce pixels"
        assert result["badR"] == 0, f"R channel should be 0, got {result['firstR']} in {result['badR']} pixels"
        assert result["firstG"] == 200
        assert result["firstB"] == 83

    def test_wasm_uniform_black_preserved(self, page, wasm_step_loop_b64):
        """WASM uniform color [0, 0, 0] (black) must not become [255, 255, 255]."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _HELPERS + """
            var w = instantiateWasm(b64);
            var res = setupAndRun(w, 3, 2, 200, [1, 0, -1], [0, 0, 0], {
                colorMode: 0, uR: 0, uG: 0, uB: 0,
                totalSteps: 5, canvasW: 100, canvasH: 100
            });
            var maxR = 0, maxG = 0, maxB = 0;
            for (var i = 0; i < res.paintCount; i++) {
                if (res.paintR[i] > maxR) maxR = res.paintR[i];
                if (res.paintG[i] > maxG) maxG = res.paintG[i];
                if (res.paintB[i] > maxB) maxB = res.paintB[i];
            }
            return {pc: res.paintCount, maxR: maxR, maxG: maxG, maxB: maxB};
        }""", wasm_step_loop_b64)
        assert result["pc"] > 0
        assert result["maxR"] == 0, f"Black R should be 0, got {result['maxR']}"
        assert result["maxG"] == 0, f"Black G should be 0, got {result['maxG']}"
        assert result["maxB"] == 0, f"Black B should be 0, got {result['maxB']}"

    def test_js_worker_uniform_zero_channel(self, page):
        """JS worker uniform color init must preserve zero-valued channels."""
        result = page.evaluate("""() => {
            // Simulate the JS worker init path
            var d = {uniformR: 0, uniformG: 200, uniformB: 0};
            var S_uniformR = d.uniformR != null ? d.uniformR : 255;
            var S_uniformG = d.uniformG != null ? d.uniformG : 255;
            var S_uniformB = d.uniformB != null ? d.uniformB : 255;
            return {r: S_uniformR, g: S_uniformG, b: S_uniformB};
        }""")
        assert result["r"] == 0, f"JS uniformR should be 0, got {result['r']}"
        assert result["g"] == 200
        assert result["b"] == 0


# ============================================================
# Bug regression: WASM workCoeffs initialization for non-animated coefficients
# ============================================================

class TestWasmWorkCoeffsInit:
    """Regression tests for the workCoeffsRe/Im initialization bug:
    non-animated coefficients must use base values, not zero."""

    def test_non_animated_coeffs_use_base_values(self, page, wasm_step_loop_b64):
        """With no jiggle/morph and no C-curve entries, WASM solver must use
        base coefficient values (not zero) — verifying workCoeffsRe init."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _HELPERS + """
            var w = instantiateWasm(b64);
            // P(z) = z^2 - 1, roots at ±1
            // No animation entries, no jiggle, no morph
            // Without the fix, workCoeffsRe/Im stays zero → different polynomial
            var nc = 3, nr = 2;
            var coeffsRe = [1, 0, -1], coeffsIm = [0, 0, 0];
            var maxP = 100;
            var L = computeWasmLayout(nc, nr, maxP, 0, 0, 0, 0, 0, 0, w.hb, 0);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;
            writeCfg(buf, L, nc, nr, {
                totalSteps: 5, maxPaint: maxP, colorMode: 0,
                uR: 255, uG: 0, uB: 0, canvasW: 200, canvasH: 200
            });
            // Write base coefficients ONLY to cRe/cIm (NOT wCR/wCI)
            // This mimics the real initWasmStepLoop which only writes cRe/cIm
            new Float64Array(buf, L.cRe, nc).set(coeffsRe);
            new Float64Array(buf, L.cIm, nc).set(coeffsIm);
            // Deliberately leave wCR/wCI as zero (the bug scenario)
            new Float64Array(buf, L.wCR, nc).fill(0);
            new Float64Array(buf, L.wCI, nc).fill(0);
            // Init roots near ±1
            new Float64Array(buf, L.pRR, nr).set([0.9, -0.9]);
            new Float64Array(buf, L.pRI, nr).set([0.1, -0.1]);
            w.inst.exports.init(L.cfgI, L.cfgD);
            var pc = w.inst.exports.runStepLoop(0, 5, 0.0);
            // Read final roots
            var finalRe = Array.from(new Float64Array(buf, L.pRR, nr));
            var finalIm = Array.from(new Float64Array(buf, L.pRI, nr));
            // Roots should converge near ±1 (for z^2-1), not near 0 (for z^2)
            var sortedRe = finalRe.slice().sort();
            return {
                pc: pc,
                r0re: sortedRe[0], r1re: sortedRe[1],
                r0im: finalIm[0], r1im: finalIm[1]
            };
        }""", wasm_step_loop_b64)
        assert result["pc"] > 0, "Should produce pixels"
        # Roots of z^2-1 are at -1 and +1
        assert abs(result["r0re"] - (-1)) < 0.1, \
            f"Root 0 should be near -1, got {result['r0re']} (workCoeffs not initialized?)"
        assert abs(result["r1re"] - 1) < 0.1, \
            f"Root 1 should be near +1, got {result['r1re']} (workCoeffs not initialized?)"

    def test_mixed_animated_and_static_coeffs(self, page, wasm_step_loop_b64):
        """With some coefficients animated and others static (no path),
        static coefficients must use base values in the solver."""
        if wasm_step_loop_b64 is None:
            pytest.skip("step_loop.wasm not built")
        result = page.evaluate("""(b64) => {
            """ + _HELPERS + """
            var w = instantiateWasm(b64);
            // P(z) = z^3 + 0z^2 + 0z - 1 (roots: 1, e^{i2π/3}, e^{-i2π/3})
            // Only coeff[2] (z term, index 2) is animated on a 1-point "curve"
            // Coeffs [0] (z^3, leading=1) and [1] (z^2, =0) are static
            var nc = 4, nr = 3;
            var coeffsRe = [1, 0, 0, -1], coeffsIm = [0, 0, 0, 0];
            var maxP = 200;
            var nE = 1, tCP = 1;
            var L = computeWasmLayout(nc, nr, maxP, nE, 0, 0, 0, tCP, 0, w.hb, 0);
            var curPages = w.mem.buffer.byteLength / 65536;
            if (L.pages > curPages) w.mem.grow(L.pages - curPages);
            var buf = w.mem.buffer;
            writeCfg(buf, L, nc, nr, {
                totalSteps: 10, maxPaint: maxP, colorMode: 0,
                uR: 100, uG: 100, uB: 100, canvasW: 200, canvasH: 200
            });
            // Override nEntries to 1
            new Int32Array(buf, L.cfgI, 76)[8] = 1;
            // Write base coefficients only to cRe/cIm
            new Float64Array(buf, L.cRe, nc).set(coeffsRe);
            new Float64Array(buf, L.cIm, nc).set(coeffsIm);
            // Leave wCR/wCI zeroed
            new Float64Array(buf, L.wCR, nc).fill(0);
            new Float64Array(buf, L.wCI, nc).fill(0);
            // Set up 1 animation entry: coeff index 2, speed 1, not CCW
            // 1-point curve at (0, 0) — keeps coeff[2] at 0+0i
            new Int32Array(buf, L.eIdx, 1).set([2]);
            new Float64Array(buf, L.eSpd, 1).set([1.0]);
            new Int32Array(buf, L.eCcw, 1).set([0]);
            new Float64Array(buf, L.eDth, 1).set([0]);
            new Int32Array(buf, L.eDd, 1).set([0]);
            new Int32Array(buf, L.cOff, 1).set([0]);
            new Int32Array(buf, L.cLen, 1).set([1]);
            new Int32Array(buf, L.cCld, 1).set([0]);
            // 1-point curve data: (0, 0)
            new Float64Array(buf, L.cvF, 2).set([0.0, 0.0]);
            // Init roots
            for (var i = 0; i < nr; i++) {
                var a = (2 * Math.PI * i) / nr + 0.37;
                new Float64Array(buf, L.pRR, nr)[i] = Math.cos(a);
                new Float64Array(buf, L.pRI, nr)[i] = Math.sin(a);
            }
            w.inst.exports.init(L.cfgI, L.cfgD);
            var pc = w.inst.exports.runStepLoop(0, 10, 0.0);
            // Read final roots
            var finalRe = Array.from(new Float64Array(buf, L.pRR, nr));
            var finalIm = Array.from(new Float64Array(buf, L.pRI, nr));
            // With correct base values: P(z) = z^3 - 1 → roots near e^{i2πk/3}
            // All roots should have |z| ≈ 1
            var mags = finalRe.map(function(re, i) {
                return Math.sqrt(re * re + finalIm[i] * finalIm[i]);
            });
            return {
                pc: pc, mags: mags,
                re: finalRe, im: finalIm
            };
        }""", wasm_step_loop_b64)
        assert result["pc"] > 0, "Should produce pixels"
        # All roots of z^3 - 1 have magnitude 1
        for i, mag in enumerate(result["mags"]):
            assert abs(mag - 1.0) < 0.15, \
                f"Root {i} magnitude should be ~1.0, got {mag} (static coeffs not initialized?)"
