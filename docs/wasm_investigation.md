# WASM Acceleration for Fast Mode Workers

> **Status: INTEGRATED (v26+)**. One WASM module is embedded in `index.html`: the full step-loop module (`WASM_STEP_LOOP_B64`, ~15 KB). Users switch between JS and WASM via the **cfg** button in the bitmap toolbar. When WASM is selected, the step-loop module is tried; if it fails, pure JS runs. The selection persists in save/load snapshots. The legacy solver-only module (`WASM_SOLVER_B64`, `solver.c` -> `solver.wasm`, ~2 KB) has been removed.

## Context

The fast mode bottleneck is the Ehrlich-Aberth solver running in Web Workers (~3 us at degree 29, scaling O(n^2 * iters)). The JS solver is already well-optimized (flat Float64Arrays, no Math.hypot, squared tolerance), but WASM can squeeze out another 1.3-2x by eliminating JIT warmup, GC pauses, and leveraging tighter register allocation for pure f64 arithmetic. The payoff grows with degree -- at degree 100+, the solver dominates pass time.

The full step-loop WASM (`step_loop.c`) moves the **entire per-pass computation** into compiled code, eliminating all JS overhead during the hot loop. JS only copies roots in before `runStepLoop()` and reads sparse pixel output + final roots after.

**Constraint**: Everything must stay in a single `index.html` file. The `.wasm` binaries get base64-encoded and embedded as JS string constants.

## What to Install

```bash
brew install lld    # Provides wasm-ld linker (~9 deps, most already installed)
```

That's it. Homebrew LLVM (`/opt/homebrew/opt/llvm/bin/clang`) is already installed and can compile C -> wasm objects. The missing piece is `lld` which provides `wasm-ld` for linking into standalone `.wasm` modules.

### System Status (checked 2026-02-13)

| Tool | Status |
|------|--------|
| Apple Clang 17 | Installed -- does NOT support wasm target |
| Homebrew LLVM 20 | Installed at `/opt/homebrew/opt/llvm/bin/clang` -- supports `--target=wasm32` |
| wasm-ld (lld) | Installed at `/opt/homebrew/bin/wasm-ld` |
| Node.js v23 | Installed -- used for base64 encoding |
| Emscripten | Not installed (fallback option) |
| Rust | Not installed (fallback option) |
| WABT | Not installed (fallback option) |

## Architecture

### Two-tier execution design

**Tier 1 -- WASM step loop (`step_loop.c` -> `step_loop.wasm`, ~15 KB)**: The entire worker pass runs in WASM. The C code contains the EA solver, curve interpolation (C-curves and D-curves), root matching (greedy and Hungarian), morph blending, jiggle offsets, all four color modes (uniform, index-rainbow, proximity, derivative), derivative sensitivity, and sparse pixel output. JS only copies roots in before `runStepLoop()` and reads sparse pixel output + final roots after. No JS-WASM boundary crossing during the step loop. This is the preferred path when WASM is enabled.

**Tier 2 -- Pure JS**: If the WASM module fails, the worker runs the pure JS step loop (identical algorithm, no WASM).

**Scope**: Only workers get WASM. The main thread solver stays in JS (called once per frame for interactive mode -- marshalling overhead not worth it).

**Unsupported modes**: The WASM step loop does not implement `idxProxColor` or `ratioColor` bitmap color modes. If either is active, the worker forces a fallback to the JS step loop even when WASM is available (~line 10410).

### Two-tier initialization cascade (worker `init` handler, ~line 10398)

```
if (useWasm) {
    try step_loop WASM  -> S_useWasmLoop = true
    if failed:
        pure JS fallback
}
// Force JS step loop for unsupported color modes (idxProxColor, ratioColor)
if (S_useWasmLoop && (S_idxProxColor || S_ratioColor)) S_useWasmLoop = false;
```

### Memory model: imported memory

The WASM step loop (`step_loop.wasm`) uses **imported memory** (`-Wl,--import-memory`). JS creates a `WebAssembly.Memory` object, passes it to the WASM module during instantiation, and controls growth. This is necessary because the step loop's memory requirements depend on runtime parameters (degree, step count, number of curves) that aren't known at compile time.

### Step-loop initialization flow (`initWasmStepLoop()`, ~line 10200)

1. Decode base64 -> `Uint8Array` -> `WebAssembly.Module`
2. Create a small initial `WebAssembly.Memory({initial: 2})` (2 pages = 128 KB)
3. Instantiate the module with imported memory + env functions (cos, sin, log, reportProgress)
4. Read `__heap_base` from WASM exports -- this is where C's BSS/globals end and free memory begins
5. Call `computeWasmLayout()` to compute byte offsets for all data sections, starting at `__heap_base`
6. Grow memory to `ceil(totalBytes / 65536)` pages
7. Copy all config, coefficients, colors, curves, palettes, etc. into WASM memory at computed offsets
8. Call `wasmLoopExports.init(cfgIntOffset, cfgDblOffset)` -- C code reads config and sets up all internal pointers

### Step-loop `run` flow (~line 10432)

```
Worker run handler:
  if S_useWasmLoop:
    1. Copy roots into WASM memory (passRootsRe/Im at layout offsets L.pRR/L.pRI)
    2. pc = wasmLoopExports.runStepLoop(stepStart, stepEnd, elapsedOffset)
    3. Read sparse pixel output: paintIdx[pc], paintR/G/B[pc] (sliced copies)
    4. Read final roots: rootsRe/Im (sliced copies from passRootsRe/Im)
    5. Post result back to main thread via transferable buffers
  else:
    JS step loop (pure JS)
```

If `runStepLoop()` throws, the worker sets `S_useWasmLoop = false` and posts an error (~line 10449). Subsequent passes fall back to the JS step loop.

## Files

### `solver.c` (project root) -- historical reference, no longer embedded

Pure C solver, no stdlib dependencies. 166 lines. The solver-only WASM module (`solver.wasm`, ~2 KB) has been removed from `index.html`. The `solver.c` file remains in the repo for reference but is no longer compiled or embedded.

```c
#define MAX_ITER 64
#define TOL2 1e-16

// Exported to WASM -- called from worker JS
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

Full worker step loop in C. 816 lines. Contains:

- **EA solver** (ported from `solver.c`, same algorithm, no `trackIter`/`iterCounts`)
- **Curve interpolation** for both C-curves and D-curves (cloud and smooth modes)
- **Dither** via xorshift128 PRNG + Box-Muller Gaussian (using `__builtin_sqrt` + imported `js_log`/`js_cos`)
- **Follow-C** D-nodes that mirror C-node positions
- **Morph blend** using cosine interpolation (`0.5 - 0.5 * cos(2*PI*morphRate*elapsed)`)
- **Jiggle offsets** applied post-interpolation
- **Root matching**: greedy O(n^2) and Hungarian O(n^3) (capped at `HUNGARIAN_MAX=32` for stack safety -- 32x32x8 = 8 KB on stack)
- **Derivative sensitivity** via `computeSens()` + `rankNorm()` with insertion sort
- **Proximity coloring** with running-max normalization
- **All four color modes**: uniform (mode 0), index-rainbow (mode 1), proximity (mode 2), derivative (mode 3)
- **Pixel output** to sparse buffers (paintIdx + paintR/G/B)
- **Progress reporting** via imported JS function (every `PROGRESS_INTERVAL=2000` steps)

Constants:
```c
#define MAX_DEG    255
#define MAX_COEFFS 256
#define HUNGARIAN_MAX 32
#define SOLVER_MAX_ITER 64
#define SOLVER_TOL2     1e-16
#define PROGRESS_INTERVAL 2000
```

Imports from JS environment (4 functions):
```c
__attribute__((import_module("env"), import_name("cos")))   extern double js_cos(double);
__attribute__((import_module("env"), import_name("sin")))   extern double js_sin(double);
__attribute__((import_module("env"), import_name("log")))   extern double js_log(double);
__attribute__((import_module("env"), import_name("reportProgress"))) extern void js_reportProgress(int step);
```

Exports: `init(cfgIntOffset, cfgDblOffset)`, `runStepLoop(stepStart, stepEnd, elapsedOffset) -> paintCount`, `__heap_base`.

Uses imported memory (`-Wl,--import-memory`) rather than exported memory, so JS controls growth.

#### Config layout

The C code reads configuration from two flat arrays placed in WASM memory at offsets passed to `init()`:

- **65 int32 values** (`cfgI[0..64]`): nCoeffs, nRoots, canvasW/H, totalSteps, colorMode, matchStrategy, morphEnabled, nEntries, nDEntries, nFollowC, nSelIndices, hasJiggle, uniformR/G/B, rngSeed[4], and 45 data section byte offsets (CI_OFF_COEFFS_RE through CI_OFF_PAINT_B)
- **3 float64 values** (`cfgD[0..2]`): range, FPS, morphRate

The C code reads these via `cfgI[CI_*]` and `cfgD[CD_*]` index macros. Data pointers are reconstructed from the int32 byte offsets using `PTR(type, idx)` macro.

#### Step loop algorithm (`runStepLoop`)

For each step in `[stepStart, stepEnd)`:

1. **Reset coefficients** to base values (when jiggle is active)
2. **Interpolate C-curves**: advance each animated coefficient along its curve (cloud = nearest-point, smooth = linear interpolation between adjacent points)
3. **Apply dither**: Gaussian noise scaled by `ditherSigma` per C-curve entry
4. **Interpolate D-curves**: same as C-curves but into morph work arrays
5. **Follow-C**: copy C-node positions to D-nodes flagged as follow-C
6. **Morph blend**: cosine interpolation between C and D coefficients
7. **Apply jiggle offsets**: add pre-computed offsets to all coefficients
8. **Solve** (EA): copy old roots as warm start, run solver, NaN rescue (unit-circle seeding)
9. **Color-mode processing + pixel output**: match roots (mode-dependent), compute colors, emit sparse pixels
10. **Progress report**: every 2000 steps, call `js_reportProgress(step)`

### `build-wasm.sh` (project root)

Builds the WASM step-loop module:

```bash
#!/bin/bash
set -e
CLANG=/opt/homebrew/opt/llvm/bin/clang

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

Key compiler flags:

| Flag | Step loop |
|------|-----------|
| `--import-memory` | Yes (JS provides and grows memory) |
| `--stack-first` | Yes (stack at bottom of address space, before BSS) |
| Stack size | 64 KB (larger due to Hungarian cost matrix on stack) |
| `--export=__heap_base` | Yes (JS needs to know where BSS ends to place data) |
| Exports | `init`, `runStepLoop`, `__heap_base` |

### Modified: `index.html`

**Base64 constant** (~line 1135):
```javascript
const WASM_STEP_LOOP_B64 = "AGFzbQ...";  // ~20KB base64 string (~15KB .wasm)
```

**Worker blob -- WASM step loop support** (~line 10131):
```javascript
// --- WASM step loop support ---
var S_useWasmLoop = false;
var wasmLoopExports = null;
var wasmLoopMemory = null;
var wasmLoopLayout = null;
var wasmLoopNRoots = 0;
var wasmLoopNCoeffs = 0;
var wasmLoopWorkerId = 0;
var wasmLoopTotalRunSteps = 0;

function wasmReportProgress(step) { ... }
function computeWasmLayout(nc, nr, maxP, nE, nDE, nFC, nSI, tCP, tDP, heapBase) { ... }
function initWasmStepLoop(d) { ... }
```

**Worker `init` handler** (~line 10371): Two-tier cascade -- try step loop WASM, fall back to pure JS. Also forces JS step loop for unsupported color modes (idxProxColor, ratioColor).

**Worker `run` handler** (~line 10430): If `S_useWasmLoop`, runs the WASM fast path (~line 10432: copy roots in, call `runStepLoop()`, read pixels out). Otherwise falls back to pure JS step loop (~line 10472).

**Main thread** sends the base64 string to workers (~line 11190):
```javascript
useWasm: solverType === "wasm",
wasmStepLoopB64: solverType === "wasm" ? WASM_STEP_LOOP_B64 : null,
```

## WASM Memory Layout

### Step loop (multi-page, imported memory, growable)

```
0x00000 +-------------------------+
        | C shadow stack (64 KB)  |  --stack-first, grows downward from 0x10000
0x10000 +-------------------------+
        | C BSS (global vars)     |  Static pointers (cfgI, cfgD, data ptrs),
        |                         |  PRNG state (rngS[4]), scalar config copies
        | __heap_base             |  End of BSS, start of JS-controlled layout
        +-------------------------+
        |                         |  <-- All sections below computed by
        |                         |      computeWasmLayout(), 8-byte aligned
        +-------------------------+
        | Config int32[65]        |  nCoeffs, nRoots, canvasW/H, colorMode,
        |                         |  matchStrategy, 45 data section byte offsets...
        | Config float64[3]       |  range, FPS, morphRate
        +-------------------------+
        | coeffsRe[nc]            |  Base coefficient values (restored each step)
        | coeffsIm[nc]            |
        | colorsR/G/B[nr]         |  Per-root colors for index-rainbow mode
        | jiggleRe/Im[nc]         |  Jiggle offsets (optional)
        | morphTargetRe/Im[nc]    |  D-node morph targets
        | proxPal R/G/B[16]       |  Proximity palette (16 entries)
        | derivPal R/G/B[16]      |  Derivative palette (16 entries)
        | selIndices[nSel]        |  Selected coefficient indices (for derivative)
        | followCIdx[nFC]         |  D-node follow-C index list
        +-------------------------+
        | C-curve entries:        |
        |   entryIdx[nE]          |  Which coefficient each curve animates
        |   entrySpeed[nE]        |  Speed multipliers (float64)
        |   entryCcw[nE]          |  Direction flags (int32)
        |   entryDither[nE]       |  Dither sigma values (float64)
        |   curveOffsets[nE]      |  Offset into curvesFlat (in points)
        |   curveLengths[nE]      |  Number of points per curve
        |   curveIsCloud[nE]      |  Cloud vs smooth flag
        +-------------------------+
        | D-curve entries:        |  (same layout as C-curves)
        |   dEntryIdx[nDE]        |
        |   ... (7 arrays)        |
        +-------------------------+
        | curvesFlat[tCP*2]       |  All C-curve points, interleaved re/im
        | dCurvesFlat[tDP*2]      |  All D-curve points, interleaved re/im
        +-------------------------+
        | workCoeffsRe/Im[nc]     |  Scratch for interpolated coefficients
        | tmpRe/Im[nr]            |  Solver scratch (warm-start copy)
        | morphWorkRe/Im[nc]      |  D-node morph scratch
        | passRootsRe/Im[nr]      |  Root positions (in/out between JS and WASM)
        +-------------------------+
        | paintIdx[maxP]          |  Output: pixel indices (int32)
        | paintR[maxP]            |  Output: pixel red (uint8)
        | paintG[maxP]            |  Output: pixel green (uint8)
        | paintB[maxP]            |  Output: pixel blue (uint8)
        +-------------------------+
```

Memory is computed by `computeWasmLayout()` (~line 10145) which returns byte offsets for each section. All offsets are 8-byte aligned (via `a8(x) = (x + 7) & ~7`). JS grows memory to `ceil(totalBytes / 65536)` pages before calling `init()`.

### `__heap_base` and JS-controlled memory

The `__heap_base` export is a `WebAssembly.Global` that marks where the C compiler's static data (BSS) ends. This address is the boundary between:

- **Compiler-managed**: stack (at bottom, 0-64KB due to `--stack-first`) and BSS (global variables)
- **JS-managed**: everything above `__heap_base`, laid out by `computeWasmLayout()`

On instantiation, JS reads `__heap_base` (~line 10228-10230):
```javascript
var heapBase = 65536;
if (wasmLoopExports.__heap_base) heapBase = wasmLoopExports.__heap_base.value;
```

Then passes `heapBase` to `computeWasmLayout()` which uses it as the starting offset for the config and data sections. This ensures the JS-written data never overlaps with the C compiler's globals.

## Expected Performance

| Metric | Current (JS) | Expected (WASM) | Notes |
|--------|-------------|-----------------|-------|
| Solver per step (deg 29) | ~3 us | ~1.5-2 us | 1.5-2x from no JIT warmup, no GC, tighter codegen |
| Worker per pass (1K steps) | ~3 ms | ~1.5-2 ms | Solver-bound |
| Total pass (16 workers) | ~8 ms | ~6-7 ms | Composite unchanged |
| Degree 100+, 10K steps | Dominates | ~2x faster | Bigger payoff at higher degree |
| Step loop overhead | JS interp | Zero | Full step loop eliminates all JS-WASM boundary crossing |

**Honest assessment**: For degree 29, the improvement is modest (~1-1.5ms per pass). The real payoff is at higher degrees where O(n^2 * iters) dominates, and in eliminating GC-induced jitter for consistent frame times. The full step-loop WASM additionally eliminates JS overhead for curve interpolation, root matching, and pixel output -- meaningful at high step counts.

## Build & Embed Workflow

```
1. Edit step_loop.c
2. Run ./build-wasm.sh
3. Copy contents of step_loop.wasm.b64 -> paste into WASM_STEP_LOOP_B64 in index.html (~line 1135)
4. Test in browser
```

This is a manual step (not automated), but only needed when the step-loop algorithm changes (rare). The `.c` and `.wasm` files live in the repo alongside `index.html`.

Current binary size:
- `step_loop.wasm`: 15,242 bytes (20,324 chars base64)

## Verification

1. Load `snaps/bug1.json` (the config that previously broke fast mode)
2. Run fast mode at 2K, 10K steps -- bitmap should fill densely (same as JS version)
3. Compare timing popup: worker time should be ~1.5-2x faster
4. Test with degree 5 (simple) and degree 50+ (stress test)
5. Test all four color modes: uniform, index-rainbow, proximity, derivative
6. Test morph blending and D-curve animation under WASM
7. Test jiggle offsets under WASM
8. Verify NaN rescue works: load a config that produces NaN roots
9. Compare bitmap output pixel-for-pixel between JS and WASM versions (should be identical or near-identical due to floating point)
10. Verify fallback: if step-loop WASM fails, JS runs
11. Verify idxProxColor and ratioColor modes fall back to JS step loop gracefully

## Fallback

Two-tier cascade within each worker:
1. **WASM step loop** (`step_loop.wasm`): Full pass in compiled code, zero JS-WASM boundary crossings during step loop
2. **Pure JS**: Both solver and step loop in JS -- falls back here if the WASM module fails to instantiate

Additionally, the WASM step loop is disabled at runtime for color modes it does not implement (idxProxColor, ratioColor), falling back to the pure JS step loop.

If clang + lld has issues building, alternatives in order of preference:
1. **Emscripten**: `brew install emscripten`, use `emcc -sSIDE_MODULE -sSTANDALONE_WASM`
2. **AssemblyScript**: `npm install assemblyscript`, TypeScript-like syntax
3. **Keep JS**: The current JS solver is already fast; WASM is an optimization, not a necessity
