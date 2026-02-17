# WASM Ehrlich-Aberth Solver for Fast Mode Workers

> **Status: INTEGRATED (v26)**. The WASM solver is fully integrated into `index.html`. Users can switch between JS and WASM solvers via the **cfg** button in the bitmap toolbar. The selection persists in save/load snapshots.

## Context

The fast mode bottleneck is now the Ehrlich-Aberth solver running in Web Workers (~3ms at degree 29, scaling O(n^2 * iters)). The JS solver is already well-optimized (flat Float64Arrays, no Math.hypot, squared tolerance), but WASM can potentially squeeze out another 1.3-2x by eliminating JIT warmup, GC pauses, and leveraging tighter register allocation for pure f64 arithmetic. The payoff grows with degree — at degree 100+, the solver dominates pass time.

**Constraint**: Everything must stay in a single `index.html` file. The `.wasm` binary gets base64-encoded and embedded as a JS string constant.

## What to Install

```bash
brew install lld    # Provides wasm-ld linker (~9 deps, most already installed)
```

That's it. Homebrew LLVM (`/opt/homebrew/opt/llvm/bin/clang`) is already installed and can compile C → wasm objects. The missing piece is `lld` which provides `wasm-ld` for linking into standalone `.wasm` modules.

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

**Scope**: Only the worker solver gets WASM. The main thread solver stays in JS (called once per frame for interactive mode — marshalling overhead not worth it).

**Copy-in/copy-out integration**: Worker typed arrays (`coeffsRe`, `coeffsIm`, `rootsRe`, `rootsIm`) remain standard JS `Float64Array`s. The `solveEA_wasm()` wrapper copies inputs into WASM linear memory, calls the solver, and copies results back (with NaN rescue). This avoids the complexity of making all worker arrays views into WASM memory while keeping the solver hot path in compiled code.

```
Worker step loop (per step):
  1. Interpolate coefficients → standard JS typed arrays
  2. solveEA_wasm() copies coeffs/roots into WASM memory, calls solver, copies results back
  3. Read root positions from JS typed arrays → paint pixels
```

## Files

### New: `solver.c` (project root)

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
            // Aberth: S = Σ 1/(z_i - z_j)
            // Correction: z -= w/(1 - w*S)
        }
        if (maxCorr2 < TOL2) break;
    }

    // Write back to warmRe/warmIm (only finite values)
    // NaN check via x == x (IEEE 754): skip NaN roots, leave warm-start unchanged
}
```

Key: no `math.h`, no `malloc`, no libc. Pure `+`, `-`, `*`, `/` on `double`. The `trackIter`/`iterCounts` parameters remain in the ABI for forward compatibility but are called with `0, 0` from JS (iteration count mode was removed from the UI). NaN check is in the C code (`x == x`); NaN rescue (cos/sin for unit-circle seeding) happens in the JS `solveEA_wasm()` wrapper during the copy-back step.

### New: `build-wasm.sh` (project root)

```bash
#!/bin/bash
/opt/homebrew/opt/llvm/bin/clang --target=wasm32-unknown-unknown -O3 -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=solveEA \
  -Wl,--initial-memory=65536 \
  -Wl,-z,stack-size=32768 \
  -o solver.wasm solver.c

# Base64 encode for embedding
node -e "console.log(require('fs').readFileSync('solver.wasm').toString('base64'))" > solver.wasm.b64

echo "WASM size: $(wc -c < solver.wasm) bytes"
echo "Base64 size: $(wc -c < solver.wasm.b64) bytes"
echo "Base64 string saved to solver.wasm.b64 — paste into index.html"
```

### Modified: `index.html`

**Change 1** — Add base64 constant (~line 1124):
```javascript
const WASM_SOLVER_B64 = "AGFzbQ...";  // ~2-4KB base64 string
```

**Change 2** — Modify `createFastModeWorkerBlob()` (~line 8954):

Add a WASM solver alongside the existing JS `solveEA`:

```javascript
// Inside worker blob string (~line 9133):

// --- WASM solver support ---
var S_useWasm = false;
var wasmExports = null;
var wasmMemBuf = null;
var W_coeffsRe, W_coeffsIm, W_warmRe, W_warmIm;
var W_OFF_CR, W_OFF_CI, W_OFF_WR, W_OFF_WI;

function initWasm(b64, nCoeffs, nRoots) {
    var raw = atob(b64);
    var bytes = new Uint8Array(raw.length);
    for (var i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
    var mod = new WebAssembly.Module(bytes.buffer);
    var inst = new WebAssembly.Instance(mod);  // WASM exports its own memory
    wasmExports = inst.exports;
    wasmMemBuf = wasmExports.memory.buffer;
    // Compute fixed offsets (all 8-byte aligned)
    W_OFF_CR = 0;
    W_OFF_CI = nCoeffs * 8;
    W_OFF_WR = nCoeffs * 2 * 8;
    W_OFF_WI = (nCoeffs * 2 + nRoots) * 8;
    W_coeffsRe = new Float64Array(wasmMemBuf, W_OFF_CR, nCoeffs);
    W_coeffsIm = new Float64Array(wasmMemBuf, W_OFF_CI, nCoeffs);
    W_warmRe = new Float64Array(wasmMemBuf, W_OFF_WR, nRoots);
    W_warmIm = new Float64Array(wasmMemBuf, W_OFF_WI, nRoots);
}

function solveEA_wasm(cRe, cIm, nCoeffs, warmRe, warmIm, nRoots) {
    // Copy inputs into WASM memory
    W_coeffsRe.set(cRe);
    W_coeffsIm.set(cIm);
    W_warmRe.set(warmRe);
    W_warmIm.set(warmIm);
    wasmExports.solveEA(W_OFF_CR, W_OFF_CI, nCoeffs, W_OFF_WR, W_OFF_WI, nRoots, 0, 0);
    // Copy results back with NaN rescue
    for (var i = 0; i < nRoots; i++) {
        var rr = W_warmRe[i], ri = W_warmIm[i];
        if (rr === rr && ri === ri) {  // NaN check
            warmRe[i] = rr; warmIm[i] = ri;
        } else {
            var angle = (2 * Math.PI * i) / nRoots + 0.37;
            warmRe[i] = Math.cos(angle); warmIm[i] = Math.sin(angle);
        }
    }
}
```

**Change 3** — Worker `init` handler (~line 9218): call `initWasm(b64, nCoeffs, nRoots)` when `useWasm` flag is set. Falls back to JS solver on error.

**Change 4** — In the step loop (~line 9351), select solver based on `S_useWasm`:

```javascript
tmpRe.set(rootsRe); tmpIm.set(rootsIm);
if (S_useWasm && wasmExports) {
    solveEA_wasm(coeffsRe, coeffsIm, nCoeffs, tmpRe, tmpIm, nRoots);
} else {
    solveEA(coeffsRe, coeffsIm, nCoeffs, tmpRe, tmpIm, nRoots);
}
```

**Change 5** — Main thread sends WASM flag and base64 data to workers (~line 9862):

```javascript
useWasm: solverType === "wasm",
wasmB64: solverType === "wasm" ? WASM_SOLVER_B64 : null,
```

## WASM Memory Layout (64KB = 1 page)

```
0x0000 ┌─────────────────────────┐
       │ coeffsRe[256]  (2 KB)   │  Float64Array view
0x0800 │ coeffsIm[256]  (2 KB)   │  Float64Array view
0x1000 │ warmRe[255]    (2 KB)   │  Float64Array view (in/out)
0x1800 │ warmIm[255]    (2 KB)   │  Float64Array view (in/out)
0x2000 │ (unused)                │
       ├─────────────────────────┤
0x8000 │ C shadow stack (32 KB)  │  Grows downward from 0xFFFF
       │ (solver local arrays)   │
0xFFFF └─────────────────────────┘
```

Total data region: ~8.25 KB. Shadow stack: 32 KB (holds cr[256], ci[256], rRe[255], rIm[255], conv[255] = ~8 KB).

## Expected Performance

| Metric | Current (JS) | Expected (WASM) | Notes |
|--------|-------------|-----------------|-------|
| Solver per step (deg 29) | ~3 μs | ~1.5-2 μs | 1.5-2x from no JIT warmup, no GC, tighter codegen |
| Worker per pass (1K steps) | ~3 ms | ~1.5-2 ms | Solver-bound |
| Total pass (16 workers) | ~8 ms | ~6-7 ms | Composite unchanged |
| Degree 100+, 10K steps | Dominates | ~2x faster | Bigger payoff at higher degree |

**Honest assessment**: For degree 29, the improvement is modest (~1-1.5ms per pass). The real payoff is at higher degrees where O(n^2 * iters) dominates, and in eliminating GC-induced jitter for consistent frame times.

## Build & Embed Workflow

```
1. Edit solver.c
2. Run ./build-wasm.sh
3. Copy contents of solver.wasm.b64
4. Paste into WASM_SOLVER_B64 constant in index.html
5. Test in browser
```

This is a manual step (not automated), but only needed when the solver algorithm changes (rare). The `.c` and `.wasm` files live in the repo alongside `index.html`.

## Verification

1. Load `snaps/bug1.json` (the config that previously broke fast mode)
2. Run fast mode at 2K, 10K steps — bitmap should fill densely (same as JS version)
3. Compare timing popup: worker time should be ~1.5-2x faster
4. Test with degree 5 (simple) and degree 50+ (stress test)
5. Test proximity and derivative coloring modes
6. Verify NaN rescue works: load a config that produces NaN roots
7. Compare bitmap output pixel-for-pixel between JS and WASM versions (should be identical or near-identical due to floating point)

## Fallback

If clang + lld has issues, alternatives in order of preference:
1. **Emscripten**: `brew install emscripten`, use `emcc -sSIDE_MODULE -sSTANDALONE_WASM`
2. **AssemblyScript**: `npm install assemblyscript`, TypeScript-like syntax
3. **Keep JS**: The current JS solver is already fast; WASM is an optimization, not a necessity
