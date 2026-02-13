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
| wasm-ld (lld) | **NOT installed** — `brew install lld` needed |
| Node.js v23 | Installed — used for base64 encoding |
| Emscripten | Not installed (fallback option) |
| Rust | Not installed (fallback option) |
| WABT | Not installed (fallback option) |

## Architecture

**Scope**: Only the worker solver gets WASM. The main thread solver stays in JS (called once per frame for interactive mode — marshalling overhead not worth it).

**Zero-copy integration**: Worker typed arrays (`coeffsRe`, `coeffsIm`, `rootsRe`, `rootsIm`) become views into WASM linear memory. Coefficient interpolation writes directly into WASM memory, solver reads/writes there, pixel painting reads from there. No copying in the hot path.

```
Worker step loop (per step):
  1. Interpolate coefficients → typed array views into WASM memory
  2. wasmInstance.exports.solveEA(offsets...) → operates on same memory
  3. Read root positions from same views → paint pixels
```

## Files

### New: `solver.c` (project root)

Pure C solver, no stdlib dependencies. ~80 lines.

```c
#define MAX_ITER 64

// Exported to WASM — called from worker JS
__attribute__((export_name("solveEA")))
void solveEA(double *cRe, double *cIm, int nCoeffs,
             double *warmRe, double *warmIm, int nRoots,
             int trackIter, unsigned char *iterCounts) {
    // Strip leading zeros
    int start = 0;
    while (start < nCoeffs - 1 && cRe[start]*cRe[start] + cIm[start]*cIm[start] < 1e-30) start++;
    int degree = nCoeffs - 1 - start;
    if (degree <= 0) return;

    // Degree 1: direct
    if (degree == 1) { /* -b/a */ return; }

    // Copy stripped coefficients to local arrays
    double cr[256], ci[256];
    double rRe[255], rIm[255];
    unsigned char conv[255] = {0};
    // ... copy from inputs ...

    // Main iteration loop (identical algorithm to JS)
    for (int iter = 0; iter < MAX_ITER; iter++) {
        double maxCorr2 = 0;
        for (int i = 0; i < degree; i++) {
            // Horner: p(z) and p'(z)
            // Newton: w = p/p'
            // Aberth: S = Σ 1/(z_i - z_j)
            // Correction: z -= w/(1 - w*S)
        }
        if (maxCorr2 < 1e-16) break;
    }

    // Write back to warmRe/warmIm (finite roots only)
    // NaN rescue: skip in WASM, let JS handle after call
}
```

Key: no `math.h`, no `malloc`, no libc. Pure `+`, `-`, `*`, `/` on `double`. NaN check via `x != x` (standard IEEE 754). NaN rescue (cos/sin for unit-circle seeding) stays in JS — it's a cold path.

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

**Change 1** — Add base64 constant (near top of script block, ~line 825):
```javascript
const WASM_SOLVER_B64 = "AGFzbQ...";  // ~2-4KB base64 string
```

**Change 2** — Modify `createFastModeWorkerBlob()` (~line 7352):

Replace the JS `solveEA` function with WASM-backed version:

```javascript
// Inside worker blob string:

const WASM_B64 = "${WASM_SOLVER_B64}";  // Injected from outer scope
var wasmExports = null;
var wasmMemory = null;

// Memory layout (byte offsets into WASM linear memory):
// 0x0000: data region (coeffs, roots, iterCounts)
// 0x8000: C shadow stack (grows downward from 0xFFFF)
var MEM_COEFFS_RE, MEM_COEFFS_IM, MEM_WARM_RE, MEM_WARM_IM, MEM_ITER;

function initWasm(nCoeffs, nRoots) {
    var bytes = Uint8Array.from(atob(WASM_B64), function(c) { return c.charCodeAt(0); });
    wasmMemory = new WebAssembly.Memory({ initial: 1 });  // 64KB
    var mod = new WebAssembly.Module(bytes.buffer);
    var inst = new WebAssembly.Instance(mod, { env: { memory: wasmMemory } });
    wasmExports = inst.exports;

    // Compute fixed offsets (all 8-byte aligned)
    MEM_COEFFS_RE = 0;
    MEM_COEFFS_IM = nCoeffs * 8;
    MEM_WARM_RE = nCoeffs * 2 * 8;
    MEM_WARM_IM = (nCoeffs * 2 + nRoots) * 8;
    MEM_ITER = (nCoeffs * 2 + nRoots * 2) * 8;
}
```

**Change 3** — Worker `init` handler: call `initWasm(nCoeffs, nRoots)` after receiving init data.

**Change 4** — Replace worker typed arrays with WASM memory views:

```javascript
// In "init" handler, AFTER initWasm():
S_coeffsRe = new Float64Array(wasmMemory.buffer, MEM_COEFFS_RE, S_nCoeffs);
S_coeffsIm = new Float64Array(wasmMemory.buffer, MEM_COEFFS_IM, S_nCoeffs);
// Copy initial coefficient values into WASM memory views
for (var i = 0; i < S_nCoeffs; i++) {
    S_coeffsRe[i] = initCoeffsRe[i];
    S_coeffsIm[i] = initCoeffsIm[i];
}
```

**Change 5** — In the step loop, replace `solveEA(...)` call with WASM call:

```javascript
// Old:
// solveEA(coeffsRe, coeffsIm, nCoeffs, tmpRe, tmpIm, nRoots, iterCounts);

// New — rootsRe/rootsIm are already views into WASM memory:
tmpRe.set(rootsRe); tmpIm.set(rootsIm);  // tmpRe/tmpIm are views at MEM_WARM_RE/IM
wasmExports.solveEA(
    MEM_COEFFS_RE, MEM_COEFFS_IM, nCoeffs,
    MEM_WARM_RE, MEM_WARM_IM, nRoots,
    iterColor ? 1 : 0, MEM_ITER
);
// Results are already in tmpRe/tmpIm (same memory)
```

**Change 6** — NaN rescue after WASM call (cold path, JS):

```javascript
// After wasmExports.solveEA():
for (var i = 0; i < nRoots; i++) {
    if (tmpRe[i] !== tmpRe[i] || tmpIm[i] !== tmpIm[i]) {  // isNaN
        var angle = (2 * Math.PI * i) / nRoots + 0.37;
        tmpRe[i] = Math.cos(angle);
        tmpIm[i] = Math.sin(angle);
    }
}
```

## WASM Memory Layout (64KB = 1 page)

```
0x0000 ┌─────────────────────────┐
       │ coeffsRe[256]  (2 KB)   │  Float64Array view
0x0800 │ coeffsIm[256]  (2 KB)   │  Float64Array view
0x1000 │ warmRe[255]    (2 KB)   │  Float64Array view (in/out)
0x1800 │ warmIm[255]    (2 KB)   │  Float64Array view (in/out)
0x2000 │ iterCounts[255](0.25KB) │  Uint8Array view
0x2100 │ (padding)               │
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
5. Test iteration coloring mode (uses `iterCounts`)
6. Verify NaN rescue works: load a config that produces NaN roots
7. Compare bitmap output pixel-for-pixel between JS and WASM versions (should be identical or near-identical due to floating point)

## Fallback

If clang + lld has issues, alternatives in order of preference:
1. **Emscripten**: `brew install emscripten`, use `emcc -sSIDE_MODULE -sSTANDALONE_WASM`
2. **AssemblyScript**: `npm install assemblyscript`, TypeScript-like syntax
3. **Keep JS**: The current JS solver is already fast; WASM is an optimization, not a necessity
