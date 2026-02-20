# Root Finding: Ehrlich-Aberth Method

The core computational engine implements the [Ehrlich-Aberth method](https://en.wikipedia.org/wiki/Aberth_method) — a simultaneous iterative root-finding algorithm with cubic convergence. Three implementations exist: a JavaScript version for interactive use (main thread), a JavaScript version optimized for workers (flat arrays, inside the worker blob), and a WASM version compiled from C (`step_loop.c`) that runs the entire step loop (solver + curve interpolation + root matching + pixel output) in WASM.

## How It Works

Given a degree-*n* polynomial p(z) = cₙzⁿ + ··· + c₁z + c₀ (subscript = power of z), the algorithm maintains *n* root approximations z₁, ..., zₙ and refines them all simultaneously:

1. **Evaluate** p(zᵢ) and p'(zᵢ) at each current root estimate using [Horner's method](https://en.wikipedia.org/wiki/Horner%27s_method) — this computes both the polynomial value and its derivative in a single O(n) pass per root.

2. **Newton step:** Compute the Newton correction wᵢ = p(zᵢ) / p'(zᵢ).

3. **Aberth correction:** Instead of applying the Newton step directly (which would be plain Newton's method and only converge quadratically), apply the Aberth deflation:

   ```
   zᵢ ← zᵢ − wᵢ / (1 − wᵢ · Σⱼ≠ᵢ 1/(zᵢ − zⱼ))
   ```

   The sum term accounts for the other roots, effectively deflating the polynomial so each root estimate repels from the others. This is what gives the method its cubic convergence and prevents multiple estimates from collapsing onto the same root.

4. **Converge** when the maximum correction magnitude drops below a tolerance. Main thread: max |correction| < 10⁻¹² (magnitude via `Math.hypot`). Workers/WASM: max |correction|² < 10⁻¹⁶ (squared magnitude, avoiding `Math.hypot`).

## Why It's Fast for Interactive Use

The UI supports degree 2–30 (minimum degree 2 = quadratic). The key insight is **warm-starting**: when the user drags a coefficient slightly, the roots barely move. The previous frame's root positions are an excellent initial guess, so the solver converges in **1–3 iterations** during interactive drag (versus 15–30 iterations from a cold start). At degree 30, each iteration is O(n²) for the Aberth sums, making the total cost negligible compared to the domain coloring render.

## Three Solver Implementations

### 1. Main-Thread JS: `solveRootsEA` (~line 5151 in index.html)

Used for interactive mode (drag, animation preview). Called once per frame on the main thread.

- **Input**: `coeffs` array of `{re, im}` objects (descending degree), optional `warmStart` array of `{re, im}` initial guesses
- **Output**: Array of `{re, im}` root objects
- **Parameters**: MAX_ITER=100, TOL=1e-12 (magnitude via `Math.hypot`)
- Uses `[re, im]` pairs internally during iteration, then converts back to `{re, im}` objects for return
- `Float64Array` for coefficient storage (cache-friendly, typed-array fast paths in V8/SpiderMonkey)
- Simultaneous Horner evaluation of p(z) and p'(z)
- Guard clauses for degenerate cases: near-zero derivative (skip update), near-coincident roots (skip Aberth term), leading zero coefficients (strip before solving), degree 1 (direct formula)
- Radius heuristic for cold-start initialization: initial guesses spread on a circle of radius (|c₀|/|cₙ|)^(1/n), with angular offset of 0.37 radians to break symmetry
- NaN rescue: non-finite results fall back to warm-start values, then to unit circle positions

### 2. Worker Blob JS: `solveEA` (~line 9911 in index.html)

Used in the JavaScript step loop inside fast-mode Web Workers. Called once per step per worker.

- **Input**: Flat `Float64Array` views — `cRe`, `cIm` (coefficients), `warmRe`, `warmIm` (roots, in/out), plus `nCoeffs` and `nRoots` counts
- **Output**: Mutates `warmRe`/`warmIm` in place
- **Parameters**: MAX_ITER=64, TOL=1e-16 (squared magnitude, no `Math.hypot`)
- Pure flat-array arithmetic — no object allocation in the hot loop
- Leading-zero test uses squared magnitude < 1e-30 (avoids `Math.hypot`)
- NaN rescue in-solver: non-finite roots are re-seeded on unit circle at angle `(2πi/degree + 0.37)` using `Math.cos`/`Math.sin`
- The JS step loop is the fallback when the WASM step loop is unavailable

### 3. WASM Step Loop: `step_loop.c` → `step_loop.wasm`

The preferred WASM path. Rather than calling WASM for the solver alone (which would cross the JS-WASM boundary once per step), `step_loop.c` runs the **entire step loop** in WASM: curve interpolation, morph blending, jiggle offsets, EA solver, root matching, and pixel output. JS only calls into WASM twice per pass — `init()` then `runStepLoop()`.

- **Source**: `step_loop.c` in the project root (~817 lines)
- **Binary**: `step_loop.wasm` (~15KB), base64-encoded as `WASM_STEP_LOOP_B64` in `index.html`
- **Exported functions**: `init(cfgIntOffset, cfgDblOffset)` and `runStepLoop(stepStart, stepEnd, elapsedOffset)` → returns pixel count
- **Imported functions**: `cos`, `sin`, `log` (JS `Math.*`), `reportProgress` (for progress updates)
- **Solver parameters**: SOLVER_MAX_ITER=64, SOLVER_TOL2=1e-16 (squared magnitude)
- Pure C with no stdlib, no malloc — stack arrays for local solver state (up to MAX_DEG=255, MAX_COEFFS=256)
- Includes full implementations of: `solveEA`, `matchRootsGreedy`, `hungarianMatch` (capped at HUNGARIAN_MAX=32 for stack safety), `computeSens`, `rankNorm`
- Handles all four color modes (uniform, index-rainbow, proximity, derivative)
- PRNG: xorshift128 for jiggle/dither, seeded from JS
- NaN check uses `x != x` (IEEE 754); NaN rescue uses imported `cos`/`sin`
- Progress reporting every 2000 steps via imported `reportProgress`

#### Historical: Solver-Only WASM (`solver.c` → `solver.wasm`) -- removed

The old solver-only WASM (~2KB) has been removed. It previously exported a single `solveEA` function and served as a fallback between the step-loop WASM and pure JS. The `solver.c` file remains in the repo for reference. The system now uses a 2-tier strategy: WASM step loop -> pure JS.

## WASM Initialization & Fallback Chain

When a worker receives an `init` message with `useWasm: true`, it follows this priority:

1. **Try `step_loop.wasm`**: If `wasmStepLoopB64` is provided, attempt `initWasmStepLoop()`. Sets `S_useWasmLoop = true` on success.
2. **Pure JS**: If the WASM step loop fails, the worker uses the JS `solveEA` function in a JS step loop.

Additionally, `S_useWasmLoop` is forced to `false` for color modes not yet supported by `step_loop.c` (currently: index-proximity and ratio modes), in which case the worker falls back to the pure JS step loop.

## WASM Memory Layout

### step_loop.wasm (full step loop)

Uses imported memory with dynamic page allocation. The first 64KB is reserved for the C shadow stack (grows downward). Above `__heap_base`, JS computes a layout (`computeWasmLayout`) that packs:

- Config arrays: 65 int32 config values + 3 float64 config values
- Input data: coefficient arrays, color arrays, jiggle offsets, morph targets, palette arrays, selection indices
- C-curve and D-curve entry arrays (parallel arrays for index, speed, ccw, dither, offsets, lengths, isCloud)
- Curve point data (flat re/im pairs)
- Working arrays: workCoeffsRe/Im, tmpRe/Im, morphWorkRe/Im, passRootsRe/Im
- Output buffers: paintIdx (Int32), paintR/G/B (Uint8)

Memory is grown to fit the computed layout. Total pages depend on polynomial degree, number of curve entries, and max paints per worker.

## Build Workflow

```bash
./build-wasm.sh    # Requires Homebrew LLVM + lld (clang at /opt/homebrew/opt/llvm/bin/clang)
```

This compiles:
- `step_loop.c` → `step_loop.wasm` → `step_loop.wasm.b64` (full step loop, ~15KB)

The base64 string is pasted into `WASM_STEP_LOOP_B64` in `index.html`. Compiler flags:
- `--import-memory`, `--stack-first`, `--stack-size=65536`, exports `init` + `runStepLoop` + `__heap_base`

## Bidirectional Editing

Dragging roots works in the opposite direction: the polynomial is reconstructed from its roots by expanding the product (z − r₀)(z − r₁)···(z − rₙ₋₁) using sequential polynomial multiplication — O(n²) complex multiplications. The resulting coefficients are rendered on the left panel and the domain coloring updates accordingly.

## Domain Coloring

When enabled (off by default, toggle via the toolbar button on the roots panel), the roots panel background is painted using [domain coloring](https://en.wikipedia.org/wiki/Domain_coloring). For each pixel at position z in the complex plane, the polynomial p(z) is evaluated and mapped to a color:

- **Hue** = arg(p(z)) — the phase of the polynomial's value. Roots appear as points where all colors converge (all phases meet at a zero).
- **Lightness** = 0.5 + 0.4·cos(2π·frac(log₂|p(z)|)) — contour lines at powers-of-2 modulus. Zeros appear as dark points.
- **Saturation** = 0.8 fixed.

The polynomial is evaluated via Horner's method. The canvas renders at half resolution with `devicePixelRatio` support and is CSS-scaled to full size, keeping it smooth at 60fps even at degree 30.

## Selecting the Solver

Click the **cfg** button in the bitmap toolbar to open the solver config popup. Choose **JS** or **WASM**. The selection takes effect on the next fast-mode start (workers are initialized with the chosen solver type). The setting is persisted in save/load snapshots.

When WASM is selected, workers use the full step-loop WASM (`step_loop.wasm`) and fall back to pure JS if it fails.

## Parameter Comparison

| Parameter | Main-Thread JS | Worker JS | WASM (both) |
|-----------|---------------|-----------|-------------|
| Max iterations | 100 | 64 | 64 |
| Convergence threshold | 1e-12 (magnitude) | 1e-16 (squared) | 1e-16 (squared) |
| Convergence check | `Math.hypot` | manual `re²+im²` | manual `re²+im²` |
| Leading-zero test | `Math.hypot` < 1e-15 | magnitude² < 1e-30 | magnitude² < 1e-30 |
| NaN rescue | `isFinite` → warm-start → unit circle | `isFinite` → unit circle reseed | `x != x` → unit circle (via imported cos/sin) |
| Iteration tracking | None (removed) | None (removed) | None (removed) |
| Stack arrays | MAX_COEFFS=256, MAX_DEG=255 | Dynamic (via `new Float64Array`) | MAX_COEFFS=256, MAX_DEG=255 |

## Step Partitioning

The main thread distributes bitmap steps across workers using floor-division with remainder: each worker gets `floor(totalSteps / nWorkers)` steps, with the first `totalSteps % nWorkers` workers receiving one extra step. Worker count is clamped to `min(numWorkers, totalSteps)` so no worker runs with zero steps. Each worker's sparse paint buffer is pre-sized to `ceil(totalSteps / numWorkers) * nRoots` entries.
