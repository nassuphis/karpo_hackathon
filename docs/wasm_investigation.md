# WASM Ehrlich-Aberth Solver for Fast Mode Workers

> **Status: INTEGRATED (v26+)**. Two WASM modules are embedded in `index.html`: the solver-only module (`WASM_SOLVER_B64`) and the full step-loop module (`WASM_STEP_LOOP_B64`). Users switch between JS and WASM via the **cfg** button in the bitmap toolbar. When WASM is selected, the step-loop module is tried first; if it fails, the solver-only module is used; if that also fails, pure JS runs. The selection persists in save/load snapshots.

## Context

The fast mode bottleneck is now the Ehrlich-Aberth solver running in Web Workers (~3ms at degree 29, scaling O(n^2 * iters)). The JS solver is already well-optimized (flat Float64Arrays, no Math.hypot, squared tolerance), but WASM can potentially squeeze out another 1.3-2x by eliminating JIT warmup, GC pauses, and leveraging tighter register allocation for pure f64 arithmetic. The payoff grows with degree — at degree 100+, the solver dominates pass time.

The solver-only WASM eliminated per-step JS-WASM boundary overhead for the solver, but the step loop itself (curve interpolation, root matching, morph blending, pixel output) still ran in JS. The full step-loop WASM (`step_loop.c`) moves the entire per-pass computation into compiled code, eliminating all JS overhead during the hot loop.

**Constraint**: Everything must stay in a single `index.html` file. The `.wasm` binaries get base64-encoded and embedded as JS string constants.

## What to Install

```bash
brew install lld    # Provides wasm-ld linker (~9 deps, most already installed)
```

That's it. Homebrew LLVM (`/opt/homebrew/opt/llvm/bin/clang`) is already installed and can compile C -> wasm objects. The missing piece is `lld` which provides `wasm-ld` for linking into standalone `.wasm` modules.

### System Status (checked 2026-02-13)

| Tool | Status |
|------|--------|
| Apple Clang 17 | Installed — does NOT support wasm target |
| Homebrew LLVM 20 | Installed at `/opt/homebrew/opt/llvm/bin/clang` — supports `--target=wasm32` |
| wasm-ld (lld) | Installed at `/opt/homebrew/bin/wasm-ld` |
| Node.js v23 | Installed — used for base64 encoding |
| Emscripten | Not installed (fallback option) |
| Rust | Not installed (fallback option) |
| WABT | Not installed (fallback option) |

## Architecture

### Two-tier WASM design

**Tier 1 — Full step loop (`step_loop.c`)**: The entire worker pass runs in WASM. JS only copies roots in before `runStepLoop()` and reads sparse pixel output + final roots after. No JS-WASM boundary crossing during the step loop. This is the preferred path when WASM is enabled.

**Tier 2 — Solver only (`solver.c`)**: Only the EA solver is WASM. The step loop (curve interpolation, matching, pixels) remains in JS. Each step crosses the JS-WASM boundary via `solveEA_wasm()`. This is the fallback if the step-loop module fails to instantiate.

**JS fallback**: If both WASM modules fail, the worker runs the pure JS step loop (identical algorithm, no WASM).

**Scope**: Only workers get WASM. The main thread solver stays in JS (called once per frame for interactive mode — marshalling overhead not worth it).

### Three-tier initialization cascade (worker `init` handler, ~line 9570)

```
if (useWasm) {
    try step_loop WASM  → S_useWasmLoop = true
    if failed:
        try solver-only WASM → S_useWasm = true
    if both failed:
        pure JS fallback
}
```

### Solver-only copy-in/copy-out integration

Worker typed arrays (`coeffsRe`, `coeffsIm`, `rootsRe`, `rootsIm`) remain standard JS `Float64Array`s. The `solveEA_wasm()` wrapper copies inputs into WASM linear memory, calls the solver, and copies results back (with NaN rescue). This avoids the complexity of making all worker arrays views into WASM memory while keeping the solver hot path in compiled code.

### Step-loop integration

The step-loop WASM uses imported memory (`WebAssembly.Memory`), not exported. JS computes a memory layout (`computeWasmLayout()`, ~line 9322), grows memory to fit, copies all config/data into WASM memory, then calls `init()` and `runStepLoop()`. The C code reads config from flat int32/float64 arrays at known offsets.

```
Worker run handler:
  if S_useWasmLoop:
    1. Copy roots into WASM memory (passRootsRe/Im)
    2. Call wasmLoopExports.runStepLoop(stepStart, stepEnd, elapsedOffset)
    3. Read sparse pixel output (paintIdx/R/G/B) + final roots from WASM memory
    4. Post result back to main thread
  else:
    JS step loop (solver-only WASM or pure JS)
```

## Files

### `solver.c` (project root)

Pure C solver, no stdlib dependencies. ~167 lines.

```c
#define MAX_ITER 64
#define TOL2 1e-16

// Exported to WASM — called from worker JS
__attribute__((export_name("solveEA")))
void solveEA(double *cRe, double *cIm, int nCoeffs,
             double *warmRe, double *warmIm, int nRoots,
             int trackIter, unsigned char *iterCounts) {
    // Strip leading zeros, degree-1 direct solve
    // Copy stripped coefficients to stack arrays: cr[256], ci[256]
    // Copy warm-start roots to local arrays: rRe[255], rIm[255]

    // Convergence tracking (only when trackIter != 0):
    //   conv[255] array tracks per-root convergence
    //   iterCounts[i] gets the iteration where root i converged

    // Main iteration loop (identical algorithm to JS)
    for (int iter = 0; iter < MAX_ITER; iter++) {
        double maxCorr2 = 0;
        for (int i = 0; i < degree; i++) {
            // Horner: p(z) and p'(z)
            // Newton: w = p/p'
            // Aberth: S = sum 1/(z_i - z_j)
            // Correction: z -= w/(1 - w*S)
        }
        if (maxCorr2 < TOL2) break;
    }

    // Write back to warmRe/warmIm (only finite values)
    // NaN check via x == x (IEEE 754): skip NaN roots, leave warm-start unchanged
}
```

Key: no `math.h`, no `malloc`, no libc. Pure `+`, `-`, `*`, `/` on `double`. The `trackIter`/`iterCounts` parameters remain in the ABI for forward compatibility but are called with `0, 0` from JS (iteration count mode was removed from the UI). NaN check is in the C code (`x == x`); NaN rescue (cos/sin for unit-circle seeding) happens in the JS `solveEA_wasm()` wrapper during the copy-back step.

### `step_loop.c` (project root)

Full worker step loop in C. ~816 lines. Contains:

- **EA solver** (ported from `solver.c`, same algorithm, no `trackIter`/`iterCounts`)
- **Curve interpolation** for both C-curves and D-curves (cloud and smooth modes)
- **Dither** via xorshift128 PRNG + Box-Muller Gaussian
- **Follow-C** D-nodes that mirror C-node positions
- **Morph blend** using cosine interpolation
- **Jiggle offsets** applied post-interpolation
- **Root matching**: greedy O(n^2) and Hungarian O(n^3) (capped at degree 32 for stack safety)
- **Derivative sensitivity** via `computeSens()` + `rankNorm()` with insertion sort
- **Proximity coloring** with running-max normalization
- **All four color modes**: uniform, index-rainbow, proximity, derivative
- **Pixel output** to sparse buffers (paintIdx + paintR/G/B)
- **Progress reporting** via imported JS function (every 2000 steps)

Imports from JS environment (4 functions):
```c
__attribute__((import_module("env"), import_name("cos")))   extern double js_cos(double);
__attribute__((import_module("env"), import_name("sin")))   extern double js_sin(double);
__attribute__((import_module("env"), import_name("log")))   extern double js_log(double);
__attribute__((import_module("env"), import_name("reportProgress"))) extern void js_reportProgress(int step);
```

Exports: `init(cfgIntOffset, cfgDblOffset)`, `runStepLoop(stepStart, stepEnd, elapsedOffset) -> paintCount`, `__heap_base`.

Uses imported memory (`-Wl,--import-memory`) rather than exported memory, so JS controls growth.

### `build-wasm.sh` (project root)

Builds both WASM modules:

```bash
#!/bin/bash
set -e
CLANG=/opt/homebrew/opt/llvm/bin/clang

# --- Build solver-only WASM ---
$CLANG --target=wasm32-unknown-unknown -O3 -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=solveEA \
  -Wl,--initial-memory=65536 \
  -Wl,-z,stack-size=32768 \
  -o solver.wasm solver.c

node -e "console.log(require('fs').readFileSync('solver.wasm').toString('base64'))" > solver.wasm.b64

# --- Build full step-loop WASM ---
$CLANG --target=wasm32-unknown-unknown -O3 -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=init \
  -Wl,--export=runStepLoop \
  -Wl,--export=__heap_base \
  -Wl,--import-memory \
  -Wl,--stack-first \
  -Wl,-z,stack-size=65536 \
  -o step_loop.wasm step_loop.c

node -e "console.log(require('fs').readFileSync('step_loop.wasm').toString('base64'))" > step_loop.wasm.b64
```

Note the differences: step-loop uses `--import-memory` (JS provides and grows memory), `--stack-first` (stack at bottom of address space), 64 KB stack (larger than solver's 32 KB due to Hungarian cost matrix on stack).

### Modified: `index.html`

**Base64 constants** (~line 1125-1126):
```javascript
const WASM_STEP_LOOP_B64 = "AGFzbQ...";  // ~20KB base64 string (~15KB .wasm)
const WASM_SOLVER_B64 = "AGFzbQ...";     // ~3KB base64 string (~2KB .wasm)
```

**Worker blob** — WASM solver support (~line 9263):
```javascript
// --- WASM solver support ---
var S_useWasm = false;
var wasmExports = null;
var wasmMemBuf = null;
var W_coeffsRe, W_coeffsIm, W_warmRe, W_warmIm;
var W_OFF_CR, W_OFF_CI, W_OFF_WR, W_OFF_WI;

function initWasm(b64, nCoeffs, nRoots) { ... }
function solveEA_wasm(cRe, cIm, nCoeffs, warmRe, warmIm, nRoots) { ... }
```

**Worker blob** — WASM step loop support (~line 9308):
```javascript
// --- WASM step loop support ---
var S_useWasmLoop = false;
var wasmLoopExports = null;
var wasmLoopMemory = null;
var wasmLoopLayout = null;

function wasmReportProgress(step) { ... }
function computeWasmLayout(nc, nr, maxP, nE, nDE, nFC, nSI, tCP, tDP, heapBase) { ... }
function initWasmStepLoop(d) { ... }
```

**Worker `init` handler** (~line 9570): Three-tier cascade — try step loop WASM, fall back to solver-only WASM, then pure JS.

**Worker `run` handler** (~line 9602): If `S_useWasmLoop`, runs the WASM fast path (copy roots in, call `runStepLoop()`, read pixels out). Otherwise falls back to JS step loop with optional WASM solver (~line 9775).

**Main thread** sends both base64 strings to workers (~line 10288):
```javascript
useWasm: solverType === "wasm",
wasmB64: solverType === "wasm" ? WASM_SOLVER_B64 : null,
wasmStepLoopB64: solverType === "wasm" ? WASM_STEP_LOOP_B64 : null,
```

## WASM Memory Layouts

### Solver-only (64KB = 1 page, exported memory)

```
0x0000 +-------------------------+
       | coeffsRe[256]  (2 KB)   |  Float64Array view
0x0800 | coeffsIm[256]  (2 KB)   |  Float64Array view
0x1000 | warmRe[255]    (2 KB)   |  Float64Array view (in/out)
0x1800 | warmIm[255]    (2 KB)   |  Float64Array view (in/out)
0x2000 | (unused)                |
       +-------------------------+
0x8000 | C shadow stack (32 KB)  |  Grows downward from 0xFFFF
       | (solver local arrays)   |
0xFFFF +-------------------------+
```

Total data region: ~8.25 KB. Shadow stack: 32 KB (holds cr[256], ci[256], rRe[255], rIm[255], conv[255] = ~8 KB).

### Step loop (multi-page, imported memory, growable)

```
0x00000 +-------------------------+
        | C shadow stack (64 KB)  |  --stack-first, grows downward
0x10000 +-------------------------+
        | C BSS (global vars)     |  Static pointers, PRNG state
        | __heap_base             |  End of BSS, start of dynamic layout
        +-------------------------+
        | Config int32[65]        |  nCoeffs, nRoots, canvasW/H, colorMode, offsets...
        | Config float64[3]       |  range, FPS, morphRate
        +-------------------------+
        | coeffsRe[nc]            |
        | coeffsIm[nc]            |
        | colorsR/G/B[nr]         |
        | jiggleRe/Im[nc]         |
        | morphTargetRe/Im[nc]    |
        | proxPal R/G/B[16]       |
        | derivPal R/G/B[16]      |
        | selIndices[nSel]        |
        | followCIdx[nFC]         |
        | C-curve entries (idx, speed, ccw, dither, offsets, lengths, isCloud) |
        | D-curve entries (same layout as C-curves)                            |
        | curvesFlat[tCP*2]       |
        | dCurvesFlat[tDP*2]      |
        | workCoeffsRe/Im[nc]     |  Scratch for interpolated coefficients
        | tmpRe/Im[nr]            |  Solver scratch
        | morphWorkRe/Im[nc]      |  D-node morph scratch
        | passRootsRe/Im[nr]      |  Root positions (in/out)
        | paintIdx[maxP]          |  Output: pixel indices (int32)
        | paintR/G/B[maxP]        |  Output: pixel colors (uint8)
        +-------------------------+
```

Memory is computed by `computeWasmLayout()` (~line 9322) which returns byte offsets for each section. All offsets are 8-byte aligned. JS grows memory to `ceil(totalBytes / 65536)` pages before calling `init()`.

The config uses 65 int32 values (data section byte offsets stored as int32) plus 3 float64 values (range, FPS, morphRate). The C code reads these via `cfgI[CI_*]` and `cfgD[CD_*]` index macros.

## Expected Performance

| Metric | Current (JS) | Expected (WASM) | Notes |
|--------|-------------|-----------------|-------|
| Solver per step (deg 29) | ~3 us | ~1.5-2 us | 1.5-2x from no JIT warmup, no GC, tighter codegen |
| Worker per pass (1K steps) | ~3 ms | ~1.5-2 ms | Solver-bound |
| Total pass (16 workers) | ~8 ms | ~6-7 ms | Composite unchanged |
| Degree 100+, 10K steps | Dominates | ~2x faster | Bigger payoff at higher degree |
| Step loop overhead | JS interp | Zero | Full step loop eliminates all JS-WASM boundary crossing |

**Honest assessment**: For degree 29, the improvement is modest (~1-1.5ms per pass). The real payoff is at higher degrees where O(n^2 * iters) dominates, and in eliminating GC-induced jitter for consistent frame times. The full step-loop WASM additionally eliminates JS overhead for curve interpolation, root matching, and pixel output — meaningful at high step counts.

## Build & Embed Workflow

```
1. Edit solver.c and/or step_loop.c
2. Run ./build-wasm.sh
3. Copy contents of solver.wasm.b64 → paste into WASM_SOLVER_B64 in index.html
4. Copy contents of step_loop.wasm.b64 → paste into WASM_STEP_LOOP_B64 in index.html
5. Test in browser
```

This is a manual step (not automated), but only needed when the solver or step-loop algorithm changes (rare). The `.c` and `.wasm` files live in the repo alongside `index.html`.

## Verification

1. Load `snaps/bug1.json` (the config that previously broke fast mode)
2. Run fast mode at 2K, 10K steps — bitmap should fill densely (same as JS version)
3. Compare timing popup: worker time should be ~1.5-2x faster
4. Test with degree 5 (simple) and degree 50+ (stress test)
5. Test all four color modes: uniform, index-rainbow, proximity, derivative
6. Test morph blending and D-curve animation under WASM
7. Test jiggle offsets under WASM
8. Verify NaN rescue works: load a config that produces NaN roots
9. Compare bitmap output pixel-for-pixel between JS and WASM versions (should be identical or near-identical due to floating point)
10. Verify fallback cascade: if step-loop WASM fails, solver-only should activate; if both fail, JS runs

## Fallback

Three-tier cascade within each worker:
1. **WASM step loop** (`step_loop.wasm`): Full pass in compiled code, zero JS-WASM boundary crossings during step loop
2. **WASM solver only** (`solver.wasm`): Solver in WASM, step loop in JS — falls back here if step-loop module fails to instantiate
3. **Pure JS**: Both solver and step loop in JS — falls back here if all WASM modules fail

If clang + lld has issues building, alternatives in order of preference:
1. **Emscripten**: `brew install emscripten`, use `emcc -sSIDE_MODULE -sSTANDALONE_WASM`
2. **AssemblyScript**: `npm install assemblyscript`, TypeScript-like syntax
3. **Keep JS**: The current JS solver is already fast; WASM is an optimization, not a necessity
