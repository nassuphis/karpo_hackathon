# Root Finding: Ehrlich-Aberth Method

The core computational engine implements the [Ehrlich-Aberth method](https://en.wikipedia.org/wiki/Aberth_method) — a simultaneous iterative root-finding algorithm with cubic convergence. Two implementations exist: a JavaScript version for interactive use (main thread) and a WASM version compiled from C for fast-mode workers.

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

## Implementation Details

The solver (`solveRootsEA` in `index.html`) uses:

- **Flat arrays** `[re, im]` for root storage during iteration (avoids object allocation overhead in the hot loop)
- **`Float64Array`** for coefficient storage (cache-friendly, typed-array fast paths in V8/SpiderMonkey)
- **Simultaneous Horner evaluation** of p(z) and p'(z) — the derivative is accumulated alongside the polynomial value with zero extra cost:
  ```
  dp = dp · z + p      // derivative accumulator
  p  = p · z + cₖ      // polynomial accumulator
  ```
- **Guard clauses** for degenerate cases: near-zero derivative (skip update), near-coincident roots (skip Aberth term), leading zero coefficients (strip before solving), degree 1 (direct formula)
- **Radius heuristic** for cold-start initialization: initial guesses are spread on a circle of radius (|c₀|/|cₙ|)^(1/n), derived from the constant-to-leading coefficient ratio, with an angular offset of 0.37 radians to break symmetry

## Bidirectional Editing

Dragging roots works in the opposite direction: the polynomial is reconstructed from its roots by expanding the product (z − r₀)(z − r₁)···(z − rₙ₋₁) using sequential polynomial multiplication — O(n²) complex multiplications. The resulting coefficients are rendered on the left panel and the domain coloring updates accordingly.

## Domain Coloring

When enabled (off by default, toggle via the ◐ toolbar button on the roots panel), the roots panel background is painted using [domain coloring](https://en.wikipedia.org/wiki/Domain_coloring). For each pixel at position z in the complex plane, the polynomial p(z) is evaluated and mapped to a color:

- **Hue** = arg(p(z)) — the phase of the polynomial's value. Roots appear as points where all colors converge (all phases meet at a zero).
- **Lightness** = 0.5 + 0.4·cos(2π·frac(log₂|p(z)|)) — contour lines at powers-of-2 modulus. Zeros appear as dark points.
- **Saturation** = 0.8 fixed.

The polynomial is evaluated via Horner's method. The canvas renders at half resolution with `devicePixelRatio` support and is CSS-scaled to full size, keeping it smooth at 60fps even at degree 30.

## WASM Solver

Fast-mode workers can optionally use a WASM implementation of the same Ehrlich-Aberth algorithm, compiled from C (`solver.c` in the project root). The WASM binary (~2KB) is base64-encoded and embedded in `index.html` as the `WASM_SOLVER_B64` constant.

### Why WASM

The JS solver is already well-optimized (flat Float64Arrays, no `Math.hypot`, squared tolerance), but WASM provides:

- **No JIT warmup**: Compiled ahead of time, consistent performance from the first call
- **No GC pauses**: Pure stack allocation, no heap objects to collect
- **Tighter codegen**: Direct f64 register operations without JS engine overhead

The payoff scales with polynomial degree — at degree 100+, the O(n² × iters) solver dominates pass time.

### Architecture

Only workers use WASM. The main-thread solver stays in JS (called once per frame during interactive mode — marshalling overhead isn't worth it for a single call).

The WASM solver (`solver.c`) is pure C with no stdlib, no malloc, no `math.h` — just `+`, `-`, `*`, `/` on `double`. NaN detection uses `x != x` (IEEE 754). NaN rescue (cos/sin for unit-circle re-seeding) stays in JS as a cold path after the WASM call returns.

**Step partitioning**: The main thread distributes bitmap steps across workers using floor-division with remainder: each worker gets `floor(totalSteps / nWorkers)` steps, with the first `totalSteps % nWorkers` workers receiving one extra step. Worker count is clamped to `min(numWorkers, totalSteps)` so no worker runs with zero steps. Each worker's sparse paint buffer is pre-sized to `ceil(totalSteps / numWorkers) * nRoots` entries — the per-worker allocation avoids the old approach of sizing every buffer for the full step count.

### Memory Layout

Workers allocate WASM linear memory (64KB = 1 page):

```
0x0000  coeffsRe[256]    2KB    Float64Array view (input)
0x0800  coeffsIm[256]    2KB    Float64Array view (input)
0x1000  warmRe[255]      2KB    Float64Array view (in/out)
0x1800  warmIm[255]      2KB    Float64Array view (in/out)
0x2000  (unused)
0x8000  C shadow stack   32KB   Solver local arrays (grows down)
```

Data is copied into WASM memory before each call and results are copied back — the copy overhead is negligible relative to the O(n²) solver cost.

### Build Workflow

```bash
./build-wasm.sh    # Requires Homebrew LLVM + lld
```

This compiles `solver.c` → `solver.wasm` → `solver.wasm.b64`. The base64 string is then pasted into the `WASM_SOLVER_B64` constant in `index.html`. Only needed when the solver algorithm changes.

### Selecting the Solver

Click the **cfg** button in the bitmap toolbar to open the solver config popup. Choose **JS** or **WASM**. The selection takes effect on the next fast-mode start (workers are initialized with the chosen solver type). The setting is persisted in save/load snapshots.

| Parameter | JS Worker | WASM |
|-----------|-----------|------|
| Max iterations | 64 | 64 |
| Convergence threshold | 1e-16 (squared) | 1e-16 (squared) |
| Leading-zero test | magnitude² < 1e-30 | magnitude² < 1e-30 |
| NaN rescue | In solver (isFinite check) | Post-call JS (x !== x check) |
| Iteration tracking | None (removed) | None (removed) |
