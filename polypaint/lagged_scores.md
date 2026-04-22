# Lagged Scores Plan

Status: proposed feature plan for adding lag-1 metric sources to solve-score programs so a program can reference the previous solve's roots/coefficients/params alongside the current solve's. Scoped to the fused raster pipeline and its solve-score clip prepass.

## Goal

Let a solve-score program reference the previous solve's data as a new source, so metrics like "proximity of the current solve vs. the one just before it in serpentine order" become expressible in the chain DSL.

New source tags:

- `slv-1` — previous solve's roots
- `cf-1` — previous solve's coefficients (only when the program also uses `cf`)
- `pm-1` — previous solve's params (only when the program also uses `pm`)

Lag magnitude is fixed at 1 for v1. No arbitrary lag-N.

### Semantic definition (pinned)

`proximity(q=0.5, slv-1)` means **evaluate the same single-solve proximity metric against the previous solve's roots**. The lag source is an **index shift**, not a pairwise/change metric. At pixel `i`, the score is the proximity of solve `i-1`'s root cloud.

In serpentine order, that makes the score of pixel `i` reflect a physically-adjacent neighborhood (same row, previous column). Row flips and pass boundaries produce index-based — but physically discontinuous — neighbors; this is explicitly accepted behavior (see Non-goals). The design does not try to pick "spatial previous" over "index previous."

### Grammar (pinned)

Accepted lag tokens are exactly the three lowercase literals, matched by `^(slv|cf|pm)-1$`:

- `slv-1`
- `cf-1`
- `pm-1`

Rejected (parser raises a clear error): any whitespace (`slv -1`, ` slv-1`), case variants (`SLV-1`, `Slv-1`), other lag values (`slv-2`, `slv-01`, `slv-10`), trailing characters (`slv-1x`), missing digit (`slv-`), trailing digit without dash (`slv1`).

The same grammar applies to the chain DSL parser (Python + JS), the `--score_sources=` CSV parser, and the clip-artifact `metrics[].source` field round-trip.

## Short Answer

Feasible and cheap if done at two levels. The fused raster is `multispan_sectioned`-only now, and each native worker thread inside `roots2pix_mt` downloads its own byte range from the manifest. That means "overlap by one row" has to be honored both at the Lambda logical-section manifest boundary and at every native thread's per-download byte range. The multispan reader can't represent bytes before logical offset 0, so the prelude must be modeled as extra **logical** rows prepended to the manifest, with the native side told how many leading rows are warmup.

Done this way: zero synchronization, zero cross-thread reads, zero seams except the genuine global-first solve which uses the sentinel `prev_of_solve_0 = solve_0`.

## Open Conjecture: Lores Clip Ranges May Generalize Without Lag Awareness

**Status: conjecture, not a definitive answer. Must be validated empirically before relying on it in the implementation.**

### The apparent problem

The clip prepass runs on a downsampled `lores.bin` (`handler_compute_plan.py:538` `_compute_lores_n` produces an N that's ~200 for typical params, vs. 4096 for the full render — roughly 20× coarser per axis, ~400× fewer solves total). The clip prepass computes `(clip_lo, clip_hi)` from quantiles over the lores sample, and the raster uses those ranges at full-render density.

At first glance this looks like a showstopper for lag-1: "previous solve" on lores is a spatially-distant neighbor in parameter space (20× further than on the full render), so any metric that depends on the *relationship* between adjacent solves would have a different distribution on lores than on the full render. Clip ranges calibrated on the lores would then be miscalibrated for the render.

### The conjecture

For the **v1 semantic** pinned earlier (`slv-1` = "evaluate the same single-solve metric against the previous solve's roots" = an **index shift**, not a pairwise/change metric), this concern is probably moot:

> **Conjecture**: For any metric `f` that is a function of a single solve's roots, and any uniformly-sampled parameter-grid `S`, the multiset `{f(s_i) : i ∈ S}` and the multiset `{f(s_{i-1}) : i ∈ S}` (with sentinel `f(s_{-1}) = f(s_0)`) have statistically equivalent distributions. Therefore the lag-1 quantile clip range on lores equals the lag-0 quantile clip range on lores to within `1/|S|`. And since lag-0 quantiles on lores are already the accepted approximation of full-render lag-0 quantiles, lag-1 quantiles on lores should be an equally valid approximation of full-render lag-1 quantiles.

The reasoning:

1. Lag-1 is just the lag-0 metric evaluated with a shifted root input. The metric `f` itself is identical.
2. For a set of N uniformly-sampled solves, the multiset of lag-0 scores is `{f(s_0), f(s_1), ..., f(s_{N-1})}` and the multiset of lag-1 scores is `{f(sentinel = s_0), f(s_0), f(s_1), ..., f(s_{N-2})}`. The two multisets differ by one doubled element and one missing element out of N. For any quantile reasonably away from the extremes and `N > ~1000`, the quantile values are identical to within rounding.
3. This holds at **any** sampling density, including lores. Lag-1 lores quantiles ≈ lag-0 lores quantiles. Lag-0 lores quantiles already approximate full-render lag-0 quantiles. Transitively, lag-1 lores quantiles should approximate full-render lag-1 quantiles.

### What this would let us do

If the conjecture holds: the clip prepass needs **no lag awareness in the binary**. A three-line Python change in `handler_solve_proximity.py:_clip_metric_slot` that substitutes `slv-1` → `slv` in the CSV argv (preserving the original source tag in the written clip artifact) is sufficient. The clip binary stays untouched. `solve_proximity_stats.c` worker-loop changes, prev-pointer plumbing in the clip binary, and binary-side prelude validation for clip are all removed from scope.

The fused raster at full-render density still needs the full lag treatment (prev pointer, prepare_step on prev, per-thread byte overlap, section-level prelude). The conjecture only affects the clip stage.

### Where the conjecture could be wrong

- **Future pairwise / cross-solve metrics**. A metric that is a function of `(current, prev)` together — e.g., "root displacement from previous solve" — has lag-1 semantics fundamentally different from lag-0, and the distribution would depend on step size. **Out of scope for v1** per the pinned semantic, but a future feature.
- **Pass / row-flip boundary effects at small N**. The set `{f(s_{i-1})}` includes boundary-straddling pairs (row-flip: last column of row k-1 as prev for first column of row k; pass boundary: last solve of pass p-1 as prev for first solve of pass p). At lores density, these boundary cases are a larger fraction of samples than at full-render density (lores is 200×200 = 200 row boundaries; full render is 4096×4096 = 4096 row boundaries, but fraction in both cases is 1/lores_n vs 1/render_n — so ~20× more frequent as a fraction at lores). If the boundary-case score distribution is systematically different from the within-row distribution, lores quantiles would over-weight boundary cases relative to the full render.
- **Root-transform interaction**. If a non-trivial `root_xforms` chain amplifies small parameter-space differences into large root-space differences, the distribution of `f(s_{i-1})` might diverge from `f(s_i)` in a way that's different at lores vs full-render density. Unlikely for reasonable transforms but worth empirically checking.
- **Degenerate metrics**. Some metrics may have clustered values where `f(s_0)` duplicated into the sample shifts a quantile meaningfully. Edge case.

### Validation plan (must run before relying on this)

Before coding the simplified clip stage:

1. Synthetic test: generate a parameter grid, solve at lores density, solve at full-render density. Compute `proximity` (and each other metric in scope) for every solve at both densities. Produce four distributions: `{lores lag-0}`, `{lores lag-1}`, `{full lag-0}`, `{full lag-1}`. Compute q=0.001 and q=0.999 quantiles for each. Assert all four quantiles agree to within some epsilon (say 1%) across all metrics.
2. If any metric fails, mark it as "not eligible for the simplified clip" and fall back to a full lag-aware clip prepass for that metric specifically. The chain compiler would then emit a "requires full clip" flag per slot, and the handler would route those slots through a different path.
3. If most metrics pass: the plan's clip-prepass scope collapses to the Python-only argv downgrade.

### Why this is a conjecture, not a fact

I haven't actually run the validation test. The reasoning above is structurally sound for the iid-uniform-sample case, but real parameter grids are not iid — they're systematic sweeps. Statistical distributions over systematic sweeps can have fine structure that a shift-by-one disturbs. I believe the effect is small but I haven't measured it, and the plan **must** include a validation gate before the simplified clip stage ships. Until that gate passes, the conservative fallback is the full lag-aware clip prepass described in earlier sections.

### Recommendation

Stage the implementation:

1. **First**: write the synthetic validation test. Run it against every metric currently in scope.
2. **If it passes**: simplify the clip-prepass scope to the Python-only downgrade. Document the invariant loudly at the call site.
3. **If any metric fails**: implement the full lag-aware clip prepass (as originally written in earlier sections) for that metric only. The rest can still use the downgrade path.

Pick this up before implementing the feature; the clip-prepass scope decision gates the rest of the implementation.

## Vision — Beyond V1

The conjecture above handles lag-1 metrics **in isolation**. The moment programs start composing them — which is the whole point — the per-slot analysis stops being sufficient. The motivating example:

```text
abs_diff(proximity(slv), proximity(slv-1))
```

This measures **the absolute change in proximity between a solve and its predecessor**, mapped per-pixel. The individual metric slots both have the same distribution (by the conjecture). But the *program output* — the absolute difference of two clipped metric values — depends on the **joint distribution** of adjacent proximity values, which is entirely a function of parameter-space smoothness.

At full-render density, neighbors are physically close, proximity values are highly correlated, abs_diff values cluster near 0 → output image is mostly black with rare spikes. At lores density, neighbors are parameter-space distant, proximity values are weakly correlated, abs_diff values are spread out → a normalization calibrated from lores would over-compress the rendered image.

This is not a pathological edge case. It is the main use case operators will write the moment `slv-1` exists. The plan must make a design decision about where the distribution of the **program output**, not just the per-slot inputs, is calibrated.

This section collects directions that address this and related structural questions. None are locked in for v1; all are provocations worth discussing before implementation freezes.

### Vision 1: Kill the clip prepass, use streaming quantiles during render

The clip prepass is a feed-forward hack from a compute-scarce era. The raster worker already touches every pixel. Have it maintain a **per-slot t-digest (or GK summary) of scores as it renders**, merge the digests at the end of pass 1, and compute clip ranges from the actual rendered distribution. Final image comes out of pass 2 with the correct normalization applied.

For the `abs_diff` case specifically: also maintain a t-digest of the **program output** (not just the per-slot inputs). The output normalization is computed from the actual render, which is definitionally correct regardless of metric, lag, or composition.

Cost: ~2× raster time. Removes from scope: `lores.bin` and family, `solve_proximity_stats`, the clip artifact, the clip handler, per-slot clip range plumbing, fingerprint validation. Massive simplification. The lag feature this plan is about collapses to "teach the raster worker about lag prev pointers" — half a day of work if the clip stage is gone.

Caveat: the interactive preview (viewport lores) still wants the fast path. Keep it for preview. Production render uses streaming quantiles.

### Vision 2: Lag-0 and lag-1 slots are provably the same statistical object

Not a conjecture for individual slots — a theorem by construction. `proximity(q=0.1%, slv)` and `proximity(q=0.1%, slv-1)` are the same quantile of the same function over the same sample, modulo one boundary element. Dedupe them at the planner: when the compiler sees both, it emits a single clip-slot computation and two metric-slot references that share the result. When it sees only the lag-1 form, the clip stage runs in lag-0 mode and the result is installed under the lag-1 tag.

This is the conjecture reframed as a structural invariant. It makes the lag-1 clip question trivial for **individual slots only**. It does NOT address the `abs_diff` composition question — that still needs Vision 1 (or 4 or 5) for a rigorous answer, or the status quo (lores-based clip of the program output via fingerprint-level composition) as an approximation.

### Vision 3: Lag as a first-class compute primitive — generalize to lag-k

Don't hard-code k=1 in the grammar. Accept `slv-k` for any k up to a sane max (say 256). The worker maintains a ring buffer of transformed steps of size `max_k + 1`. Per-thread byte-range overlap extends by `max_k` rows instead of 1. Manifest prelude = `max_k`. The evaluator takes a vector of prev pointers indexed by k.

Suddenly the DSL becomes a **signal-processing language over the serpentine score sequence**:

- `avg(proximity(slv), proximity(slv-1), proximity(slv-2), proximity(slv-3))` — 4-tap low-pass filter over score space. Smoothing.
- `abs_diff(proximity(slv), proximity(slv-8))` — 8-step edge detector in score space.
- `weighted_sum(0.5, proximity(slv), 0.5, proximity(slv-1)) - weighted_sum(0.5, proximity(slv-2), 0.5, proximity(slv-3))` — difference of two smoothed bands.
- `max(proximity(slv-k) for k in 0..31)` — running-max envelope.

The feature effort is almost identical to lag-1 only, once the evaluator is vectorized over k. V1 can implement lag-1 and leave the grammar/parser extensible; v2 adds arbitrary k.

### Vision 4: Score-space feedback drives adaptive rendering

The render pipeline is feed-forward today: solve → score → color. What if scores could feed back?

- **Variance of lag-1 abs_diff across a tile = "turbulence" signal.** High-turbulence tiles get refined to higher resolution automatically; low-turbulence tiles render at lower res and interpolate. Self-adapting LOD driven by the score sequence itself.
- **Autocorrelation of the score sequence across a row = "texture type" classifier.** Render smooth regions with a fast kernel, textured regions with a slow high-quality kernel.
- **Running running-mean of scores = "focus region" detector.** Use it to drive an automatic "interesting region" viewer or an auto-cropping deep-zoom selector.

Lag-k metrics are the building blocks for all of this. The DSL becomes the interface for **describing what "interesting" looks like**, and the pipeline adapts rendering to it.

### Vision 5: Self-normalizing combinators via explicit DSL normalization

Add a DSL combinator `normalize(expr, q)` that explicitly computes its argument's q-th quantile over a streaming sketch during render and normalizes its argument to [0,1] using that quantile as `clip_hi`. Applied at the program output:

```text
normalize(abs_diff(proximity(slv), proximity(slv-1)), 0.999)
```

Now the operator is EXPLICITLY asking for the difference to be spread. The render pipeline collects a sketch of the inner expression's values, computes the 0.999 quantile, and normalizes. No lores clip stage needed for that slot. No `abs_diff` calibration guess.

More general than Vision 2's slot-level dedup because it handles arbitrary program outputs. More concrete than Vision 1 because it's opt-in per-expression, not a pipeline rewrite.

### Vision 6: Multi-scale lores + scaling law for derived distributions

Run the lores stage at **two or three densities** (e.g., N=100, N=200, N=400). For any program output whose distribution depends on neighbor spacing (abs_diff, weighted_sum with opposing signs, etc.), fit a scaling model of the form `quantile(N) = a + b/N^c` and extrapolate to render density.

Cheap: 2-3× lores cost, still <1% of render time. No pipeline rewrite. Works for any metric, including future pairwise ones. Requires a scaling-law fit per program output, but that's a small regression.

### What the visions add up to

The **architectural bet** worth making is:

- **Vision 2** gives you lag-1 slots for free in v1. Do it now.
- **Vision 1 or 6** gives you correct program-output distributions for compositions like `abs_diff`. Pick one and put it on the roadmap — the feature only becomes genuinely useful once operators can write abs_diff chains that actually render something visible.
- **Vision 3** makes the feature future-proof; plan the grammar for lag-k even if v1 only ships lag-1.
- **Vision 4 and 5** are the real product direction — once the score sequence is a first-class signal, the whole app pivots from "render a polynomial function" to "explore the parameter space with score-driven tooling."

### Impact on this plan

For v1 itself, the concrete recommendation is:

1. Ship the raster-side lag-1 machinery (per-thread byte overlap, sentinel, prev pointer, transformed-prev) as the plan describes.
2. For clip ranges: apply **Vision 2** — at plan time, dedupe lag-0 and lag-1 slots of the same (metric, quantile, omega) tuple. When only lag-1 is present, compute clip ranges in lag-0 mode and install under the lag-1 tag.
3. **Document that compositions like `abs_diff(proximity(slv), proximity(slv-1))` will currently render with a lores-biased normalization that under-represents the smooth-adjacency signal.** Explicitly acknowledge this is a known limitation that Vision 1 or 6 will address in a follow-up.
4. Leave lag-k (Vision 3) as a grammar extension point: reject `slv-k` for k≠1 today, but don't bake k=1 into the evaluator signature.

This keeps v1 shippable while honestly marking the horizon.

## Current State

### Fused render has two scoring stages

1. **Clip prepass** — `lambda/handler_solve_proximity.py:180, 194` dispatches `solve_proximity_stats --mode=clip` to compute per-metric clip ranges (lo/hi) over the full solve set. Each metric slot carries its own source tag via a CSV passed as `--score_sources=` (see `handler_solve_proximity.py:306`).
2. **Fused raster** — `lambda/handler_raster_mt.py:187` dispatches `roots2pix_mt --input_mode=multispan_sectioned`. Per-solve scoring uses the clip ranges computed in stage 1 to normalize each metric into [0, 1] before running the RPN stack machine.

If lag-1 metrics exist but are only implemented in stage 2, the clip ranges in stage 1 are computed from lag-0 data and the raster produces wrong normalization. **Lag support must land in both stages or the render is wrong.**

### Source tags live in a CSV, not in `program_spec`

- `lambda/solve_score.h:787-805` `parse_solve_score_source_csv` parses the CSV into `metricSources[]`. Accepted tokens today: `slv`, `cf`, `pm`.
- `lambda/solve_score_chain.py:324, 430` and `handler_solve_proximity.py:325` emit source tags separately from `program_spec`. `program_spec` is the RPN opcode string; source tags are passed as `--score_sources=<csv>`.
- **Lag tags belong in the CSV parser**, not in `parse_solve_score_program_spec`.

### Gating allowlists in both compilers

Before any token parser sees `slv-1`, four allowlists reject it:

- JS: `index.html:3843-3847` `_solveScoreMetricAllowedSources` (returns `['slv','cf']` or `['slv','cf','pm']`)
- Python: `solve_score_chain.py:52` `VALID_SOLVE_SCORE_SOURCES = {"slv","cf","pm"}`
- Python: `solve_score_chain.py:71-78` `_METRIC_ALLOWED_SOURCES`
- Python: `solve_score_chain.py:156-160` `_validate_metric_source` raises on unknown tokens

All four must be extended before any `slv-1` test can pass.

### Shared-binary evaluator hazard

`solve_score_eval_program_with_sources` is used by **multiple** C binaries:

- `roots2pix_mt` (fused raster — lag-aware in this plan)
- `solve_proximity_stats` (clip prepass — lag-aware in this plan)
- `solve_palette_chunk` and `solve_palette_chunk_mt` (**not** lag-aware; `solve_palette_chunk.c:345, 350` and `solve_palette_chunk_mt.c:236, 306, 313`)

All four binaries link `solve_score.h` and parse `--score_sources=` via the same shared parser. If the evaluator **silently substitutes the current-source pointer** whenever a lag source's prev pointer is `NULL`, palette-chunk binaries will accept a `slv-1` CSV and produce lag-0 output — silently wrong.

The mitigation is that the evaluator must **hard-fail** when a `*_LAG1` source is tagged but the caller did not supply a prev pointer. Sentinel behavior for the global-first solve (`p == 0`) must live in the **callers** (raster worker, clip worker) by setting `prevRoots = roots` explicitly at `s == 0`. See Contract Changes.

### `step_scores.raw` reuse is transparent

`handler_extract_palette_from_step_scores.py:277-290` shuffles pre-bucketed scores via `step_scores_to_palette_raw` (no chain re-evaluation). `handler_color_repalette.py` and `color_recolor_raw.py` re-render RGB from raw greyscale with a different palette (no rescoring). Cached `step_scores.raw` from a lag-1 render therefore encodes the correct lag-1 values; repalette and recolor paths inherit lag semantics automatically. No work needed in those paths.

### Other pipelines that touch solve-score — one-line verdict each

- `lambda/handler_compute_preview.py` — no `solve_score`, `score_sources`, or `score_program` references. Preview doesn't evaluate chains. Not in scope. No rejection needed.
- `lambda/raw_sidecar.py:52-72` `_normalize_clip_slots` — `source` is stored as a free string (`str(row.get("source") or "slv").strip()`). Lag tokens round-trip through sidecar serialization with zero code change.
- `lambda/solve_proximity_stats.c` `--mode=hist` — shares `score_worker_main` at `:366-395` with `--mode=clip`. The prev-pointer change in that loop applies to both modes automatically. For fused color, hist is dead (prior audit), but the behavior is consistent if anything else calls hist mode.
- `lambda/handler_palette_finalize.py`, `lambda/handler_extract_palette_from_step_scores.py` — no chain evaluation; these consume pre-bucketed scores. Not in scope.
- Bilevel rasterizers (`bilevel_raster.c`, `bilevel_section_raster.c`, `coeffs_bilevel_raster.c`) — no `solve_score`/`score_sources`. Not in scope.
- `handler_compute_chunk_fused.py`, `compute_fused.py` — solver only, no scoring. Not in scope.

### Fused raster per-native-thread download topology

`lambda/roots2pix_mt.c:289-320` — each worker thread calls `multispan_reader_read_exact(..., byteStart, sectionBytes, sectionBuf, ...)` **individually**, into its own private `sectionBuf`. Native threads do not share a section buffer; every thread's buffer holds only its own assigned byte range. Cross-thread reads are impossible without a second download or a redesign.

`lambda/roots2pix_mt.c:909-914` partitions the logical section into native-thread ranges `[start, end)` with width `base ± 1`. Each thread's `byteStart` is computed from its `start` and the section's row stride.

### Multispan reader bounds

`lambda/multispan_reader.c:529, 581, 589` — manifest `logical_size` is positive; every span must fit in `[0, logical_size)`. The reader cannot fetch bytes before logical offset 0. **"Prelude" must be prepended inside the manifest, not before it.**

### Root transforms

`lambda/roots2pix_mt.c:356` `prepare_step(rawStep, degree, rtChain, nRt, stepBuf, wkRe, wkIm)` applies rotation and the user's `root_xforms` chain to the raw roots before scoring. A lag-1 `slv-1` metric must use the previous solve's roots after the same transform chain, not the raw previous roots. Current evaluator receives only the already-transformed `step` pointer; there is no pre-existing transformed-prev.

### Threading invariant (still holds)

Each native thread reads from its own `sectionBuf` only and writes to per-thread pixel/fragment output. Lag-1 does not change this. The cross-boundary availability problem is resolved by shifting each thread's download window left by one row, not by cross-thread reads.

## Chosen Approach

Two-level overlap, both modeled as extra logical bytes inside the multispan manifest.

1. **Logical-section level** (per Lambda section): when `solve_start > 0` AND any lag source is in use for this family, the manifest's logical layout gets one extra leading row prepended. The section's `logical_size` grows by `row_bytes`. Spans are rewritten so their logical offsets start 1 row later. The first row (logical bytes `[0, row_bytes)`) is the warmup row — the section's `solve_start - 1` from the upstream solve source.
2. **Native-thread level** (inside `roots2pix_mt`): every native thread's `byteStart` covers one row before its first scored solve, sourced from the already-grown section buffer. No cross-thread reads.

### Per-thread `byteStart` formula

Let:
- `S` = section's global `solve_start`.
- `P` = section's `prelude_rows` (0 or 1).
- `thread_start` = native thread's global first scored solve index.
- `thread_width` = scored-solve count for this native thread.
- `row_bytes` = degree × 2 × sizeof(float).

The thread's download window is:

```
prev_logical_offset = (thread_start - 1 - S + P) * row_bytes
```

Three cases:

| Section | Thread | `prev_logical_offset` | Action |
|---|---|---|---|
| `S == 0, P == 0` (global-first section) | Thread 0 | `-row_bytes` (invalid) | **Sentinel**: `byteStart = 0`; in the worker loop, set `prevStep = step` when processing `thread_start`. |
| `S == 0, P == 0` | Threads 1..T-1 | positive | `byteStart = prev_logical_offset`; prev slot = thread's own first byte. |
| `S > 0, P == 1` (any non-first section, any thread) | Any thread | `>= 0` | `byteStart = prev_logical_offset`; for thread 0, prev is the section's prelude row; for threads 1+, prev is the preceding thread's last scored row, which sits inside the manifest. |

`sectionBytes = (thread_width + 1) * row_bytes` in all cases that read a prev row; `sectionBytes = thread_width * row_bytes` for the sentinel case.

**Invariant**: at the start of every iteration of the worker loop, `prevStep` is a valid pointer to either a real prev-row in the section buffer, or (only for the first scored solve of the global-first section) a copy of `step` itself.

First solve of the whole render (`p == 0`) has no upstream `solve_start - 1`. The manifest builder omits the section-level prelude for that first section; the worker loop enters the sentinel path for thread 0's first scored solve only.

Per-source families follow the same rule: if the program uses `cf-1`, the coeff manifest gets a prelude row the same way. If the program uses `pm-1`, the param manifest does. If the program uses only `slv-1`, only the solve manifest is extended.

Root transforms: the worker loop computes the transformed-previous step into a second scratch buffer alongside the transformed-current step, and passes both into the evaluator.

### Where the sentinel lives

Sentinel `prev_of_solve_0 = solve_0` is implemented **in the caller**, not the evaluator:

- Raster worker (`roots2pix_mt.c`): when processing global solve index `0`, set `prevStep = step` explicitly before the `eval` call.
- Clip worker (`solve_proximity_stats.c`): at `s == 0`, set `prevRoots = roots` before the `eval` call.

The evaluator itself hard-fails if asked to read a `*_LAG1` source against a NULL prev pointer (see §Shared-binary evaluator hazard). This keeps palette-chunk binaries safe: they never pass prev pointers, so any `slv-1` CSV they receive will abort loudly, not silently compute lag-0.

### Pass and row-flip boundaries

`sweep_cli.c:5162` orders solves pass-major with serpentine `i2`. Lag-1 is strictly index-based: at row flips within a pass, and at pass boundaries, `prev` is the last solve of the previous row/pass — physically distant from the current solve. **This is accepted behavior in v1.** The alternative (sentinel at every row/pass boundary) would require the C worker to know the pass/row structure, which it currently does not; expanding that scope is not worth it. The design makes the trade-off explicit: within-row within-pass lag is physically coherent, seams are index-coherent but spatially discontinuous.

## Contract Changes

### Chain DSL

Source tag grammar extends with a literal `-1` suffix only:

- `slv` / `slv-1`
- `cf` / `cf-1`
- `pm` / `pm-1`

Examples:

```text
proximity(q=0.1%, slv-1)
min_angular_separation(q=0.1%, cf-1)
avg(proximity(q=0.1%, slv), proximity(q=0.1%, slv-1))
```

Any other lag value (`slv-2`, `slv0`, `slv-`, etc.) is rejected by the parser with a clear error.

### Source CSV (the layer that actually changes)

`--score_sources=<csv>` grows a second vocabulary:

- Existing: `slv`, `cf`, `pm`
- New: `slv-1`, `cf-1`, `pm-1`

Both the clip prepass (`solve_proximity_stats`) and the fused raster (`roots2pix_mt`) receive the same CSV — they must parse identically.

`program_spec` does **not** change. It stays as the RPN opcode string.

### Compiled program (C side)

`enum SolveScoreMetricSource` gains three variants:

- `SOLVE_SCORE_SOURCE_SOLVE_LAG1`
- `SOLVE_SCORE_SOURCE_COEFF_LAG1`
- `SOLVE_SCORE_SOURCE_PARAM_LAG1`

`parse_solve_score_source_csv` at `solve_score.h:803-805` adds the three new tokens.

`solve_metric_supports_source` at `:702-724` must be reviewed: decide whether each existing metric is meaningful against a lag-1 source (e.g., `proximity(slv-1)` makes sense; `avg_mod(slv-1)` probably does too; some metrics may not). Document the allowed pairings.

### Evaluator signature

`solve_score_eval_program_with_sources` at `solve_score.h:1109` gains three more root pointer arguments:

```c
static double solve_score_eval_program_with_sources(
    const float *roots,      int degree,
    const float *coeffRoots, int coeffDegree,
    const float *paramValues, int paramDegree,
    const float *prevRoots,      int prevDegree,
    const float *prevCoeffRoots, int prevCoeffDegree,
    const float *prevParamValues, int prevParamDegree,
    const SolveScoreProgram *program);
```

Metric slots tagged with a `*_LAG1` source route to the `prev*` pointers. **If a `prev*` pointer is `NULL` but the corresponding `*_LAG1` source is referenced in the program, the evaluator aborts with a clear error message**. It does not silently fall back to the current-source pointer.

### Hard-fail mechanism (pinned)

`solve_score.h` currently has no error-return channel from the evaluator — it returns `double`. Options considered:

- `abort()` — crashes the process. Subprocess exits non-zero; stderr captures the message; Python handler surfaces it as a `RuntimeError`. Matches existing argv-validation style (`fprintf(stderr, ...); return 1;` in binary `main()`s). Chosen.
- Return `NAN` and require caller checks. Rejected — every caller would need to add a NaN check, and the existing silent `solve_score_clamp_unit(stack[0])` path already returns 0.0 on stack underflow, so NaN could be masked.
- Thread-local error flag. Rejected — adds thread-local storage dependencies to a header-only evaluator.

Implementation: before accessing a lag pointer, the evaluator emits `fprintf(stderr, "solve_score: metric slot %d requires %s source but prev pointer is NULL\n", i, source_name); abort();` when the prev pointer is null. `abort()` raises `SIGABRT`, the process exits non-zero, and every existing subprocess handler (`handler_raster_mt.py`, `handler_solve_proximity.py`, `handler_palette_chunk.py`) already surfaces non-zero exit as a handler-level failure.

Sentinel behavior for the global-first solve is implemented in the caller (see §Where the sentinel lives). Callers that do not support lag (e.g., palette-chunk binaries) pass `NULL` for all prev pointers. The hard-fail guarantees those callers cannot silently produce lag-0 output when handed a lag CSV.

`solve_score_eval_program` (no-source wrapper) stays, passes `NULL` for all prev pointers, and is only safe to call when the program contains no `*_LAG1` sources. Document this invariant at its call sites.

### Native argv

Both `solve_proximity_stats` and `roots2pix_mt` gain two optional flags:

- `--prelude_rows=<N>` — number of leading logical rows in the manifest that are warmup (not scored). Default 0.
- `--score_coeff_prelude_rows=<N>` and `--score_param_prelude_rows=<N>` — same for the coeff and param manifests.

`N` is either 0 or 1 for this feature. A future generalization could lift it, but validation rejects `N > 1` for now.

**Binary-side validation (mandatory)**: if `--score_sources=` contains any `slv-1` / `cf-1` / `pm-1` token, the corresponding prelude flag must be nonzero (except for the logical section whose `solve_start == 0`, where the caller instead sends prelude=0 and the sentinel path is taken). The binary aborts with a clear error when this invariant is violated. Prevents an old planner emitting a lag CSV without prelude from silently producing wrong output against a new binary.

For clip-prepass (`solve_proximity_stats --mode=clip`), the prelude flags are meaningful in a different way: the binary reads the full `lores.bin` (no manifest), so `prelude_rows` does not describe a warmup segment. Either (a) the clip binary ignores the prelude flags and derives `prev` by pointer offset within `arg->buf` (with sentinel at `s == 0`), or (b) the flag is repurposed as a `--enable_lag=<0|1>` switch. Pick (a) — simpler, matches existing flat-buffer flow.

### Manifest logical layout

`build_source_spans` and `build_native_multispan_manifest` in `logical_sections.py` gain a `prelude_rows` kwarg. When set:

- `logical_size` grows by `prelude_rows * row_bytes`.
- Every span's `byte_start` (logical offset) is shifted right by `prelude_rows * row_bytes`.
- The first `prelude_rows * row_bytes` of the logical range are filled by a new prepended span covering the upstream solves `[solve_start - prelude_rows, solve_start)`, clipped against segment boundaries.

Native-thread byte-range split inside `roots2pix_mt.c:909-914` is adjusted so each thread's `byteStart` begins `row_bytes` earlier (within the logical manifest) when the program uses lag-1. The thread's first `row_bytes` bytes are warmup; its first scored solve is at `byteStart + row_bytes`.

### Plan

`plan.raster` gains `prelude_rows`, `score_coeff_prelude_rows`, `score_param_prelude_rows` (all integers, 0 or 1).

`handler_render_plan.py` derives these from the compiled chain's source set:

- any `slv-1` source → `prelude_rows = 1`
- any `cf-1` source → `score_coeff_prelude_rows = 1`
- any `pm-1` source → `score_param_prelude_rows = 1`

Clip prepass plan is likewise augmented — `handler_solve_proximity.py` needs to receive the same prelude counts so the clip stage sees the same shifted manifest.

### ASL

The existing `$.plan.solve_score.chain` field already threads the chain string to the clip and raster tasks. Lag tokens inside that string flow through unchanged — no ASL change for the chain itself.

New ASL fields (threaded from plan into both clip and raster task payloads):

- `prelude_rows` — from `$.plan.raster.prelude_rows`
- `score_coeff_prelude_rows` — from `$.plan.raster.score_coeff_prelude_rows`
- `score_param_prelude_rows` — from `$.plan.raster.score_param_prelude_rows`

Threaded into:

- `ColorSolveScoreClipTask` payload
- `ColorRasterMap` ItemSelector (bilevel/coeff stay 0 — lag is fused-color-only in v1)

Decision: the plan carries the prelude values explicitly. The handler does **not** recompute from chain. This keeps the ASL-level contract explicit and testable, and matches the pattern of other plan-level raster knobs.

### Frontend

Neither chip UI nor free-text entry currently accepts unknown source tokens. The JS chain parser/serializer at `index.html:3829, 4012` must accept `slv-1`, `cf-1`, `pm-1`:

- `_compileSolveScoreChain` parses the new tokens into an internal representation.
- `_serializeSolveScoreChain` round-trips them.
- `_renderArtifactSolveDisplay` labels them in the Render summary.

UI chip widening (dropdown gains lag toggle) is optional for v1 — operators can still use JSON upload / direct chain edit — but the JS parser/serializer support is **required**, otherwise upload and saved-program fetch will reject lag-using programs.

## Implementation Plan By File

### 1. `lambda/solve_score_chain.py`

- Extend `VALID_SOLVE_SCORE_SOURCES` (`:52`) to include `"slv-1"`, `"cf-1"`, `"pm-1"`.
- Extend `_METRIC_ALLOWED_SOURCES` (`:71-78`) so each metric declares which lag variants it supports (default: if metric supports `slv`, also supports `slv-1`; same for cf/pm).
- Extend `_validate_metric_source` (`:156-160`) to accept the new tokens.
- Parse `slv-1`, `cf-1`, `pm-1` source tokens from chain text.
- Reject any other lag value (`slv-2`, `slv0`, …).
- Emit `metrics[i]["source"] = "slv-1"` etc. in the compiled chain output.
- `compile_solve_score_chain` returns `metrics` and `score_sources` fields; both must reflect the new values.
- `legacy_compatible` is false when any lag source is present (matches existing rule for cf/pm).
- `program_spec` is unchanged — lag does not affect the RPN.
- Add two helpers:
  ```python
  def lag_sources_used(compiled) -> dict:
      # Returns {"slv": bool, "cf": bool, "pm": bool}.
      # Used by handler_render_plan to decide per-family prelude_rows.
  
  def any_lag_source(compiled) -> bool:
      # Used by handlers that reject lag at entry (palette_chunk).
  ```
- `compiled_solve_score_fingerprint` already hashes `metrics[i].source` (`:401-415`); lag tokens naturally change the fingerprint. Document this — it means cached lag-0 clip artifacts are correctly rejected against a lag-1 request. No explicit code change needed.
- `legacy_compatible` naturally becomes false when any lag source is present because lag tokens are non-`slv` under the extended `VALID_SOLVE_SCORE_SOURCES`, and the existing check at `:565-576` excludes any non-`slv` source. **No new rule needed**; confirm with a test.

### 2. `lambda/solve_score.h`

- Extend `enum SolveScoreMetricSource` with the three `*_LAG1` variants.
- Extend `parse_solve_score_source_csv` at `:803-805` to parse the new tokens.
- Extend `solve_metric_supports_source` at `:702-724` to declare which metrics support each lag source (decide per metric; mirror the existing cf/pm rules as a starting point).
- Extend `solve_score_eval_program_with_sources` at `:1109` with three prev pointer arguments. Route `*_LAG1` metric slots to the prev pointers. **Hard-fail with a clear error when a `*_LAG1` source is tagged and its prev pointer is `NULL`.** Do not silently fall back — that would make the shared palette-chunk binaries silently produce lag-0.

### 3. `lambda/solve_proximity_stats.c` + `lambda/handler_solve_proximity.py`

- The clip binary reads the full `lores.bin` into a flat `nSolves × stride` float buffer; no manifest, no prelude needed. Concrete change in `score_worker_main` at `:366-395`:
  ```c
  const float *roots = arg->buf + s * arg->stride;
  const float *prevRoots = (s > 0) ? (arg->buf + (s - 1) * arg->stride) : roots;  // sentinel
  // (same pattern for coeffRoots / paramValues; prev = (s>0) ? ptr_at(s-1) : ptr_at(s))
  arg->scores[s] = eval_score_or_program_with_sources(
      roots, arg->degree, coeffRoots, arg->coeffDegree, paramValues, arg->paramDegree,
      prevRoots, arg->degree, prevCoeffRoots, arg->coeffDegree, prevParamValues, arg->paramDegree,
      arg->metric, arg->program, arg->rtChain, arg->nRt, wkRe, wkIm);
  ```
  One-line pointer add per source family. No new flag, no new binary mode.
- `eval_score_or_program_with_sources` (the binary's local wrapper around the evaluator) gets extended to accept and forward the three prev pointers.
- Python handler `handler_solve_proximity.py:180, 306` threads the lag-aware `--score_sources` CSV to the clip subprocess. No other change; the binary infers "lag is in use" from the CSV and routes accordingly.
- Sentinel `prev == current` for `s == 0` lives in the worker loop (see §Where the sentinel lives), not in the evaluator.

### 4. `lambda/logical_sections.py`

- `build_source_spans`, `build_native_multispan_manifest` gain a `prelude_rows` kwarg (default 0).
- When `prelude_rows > 0`, the section's logical size grows, a prepended span is inserted covering upstream solves `[solve_start - prelude_rows, solve_start)` (clipped at segment boundaries), and all subsequent spans' logical byte offsets shift right.
- For the global first logical section (`solve_start == 0`): `prelude_rows` is silently clamped to 0. No upstream row exists.

### 5. `lambda/handler_raster_mt.py`

- Read `plan.raster.prelude_rows` / `score_coeff_prelude_rows` / `score_param_prelude_rows` from the payload.
- Pass them to `build_native_multispan_manifest` per-family.
- Add `--prelude_rows=<N>`, `--score_coeff_prelude_rows=<N>`, `--score_param_prelude_rows=<N>` to the `roots2pix_mt` argv.
- Validate that `N in {0, 1}`.

### 6. `lambda/roots2pix_mt.c`

- Parse the three new argv flags.
- Binary-side validation: if `--score_sources` carries any `*-1` token and the corresponding `--prelude_rows*` is `0` while `solve_start != 0`, abort with a clear error. (For sections with `solve_start == 0`, prelude=0 is legitimate; the worker uses the sentinel path.)
- `worker_main`: when `preludeRows > 0`, each thread's `byteStart` is computed to start one row earlier, and its `sectionBytes` grows by `row_bytes`. Scored-range loop still iterates `[start, end)` in logical solve indices; the extra leading row lives at `sectionBuf + 0` bytes, and the first scored solve is at `sectionBuf + row_bytes`.
- Worker loop at `:534-542`: maintain `prevStep` alongside `step`. Compute both via `prepare_step` into two scratch buffers `stepBufCurrent` and `stepBufPrev`. For the first scored solve of the first logical section (global `p == 0`), explicitly set `prevStep = step` before calling the evaluator (sentinel, see §Where the sentinel lives).
- Same pattern for `prevCoeffStep` and `prevParamStep` in the sectioned branches at `:548-564`.
- Pass all six pointers into `solve_score_eval_program_with_sources` at `:572`. The evaluator will hard-fail if any `*_LAG1` source is tagged and its prev pointer is NULL — so the worker must guarantee a non-NULL prev for every iteration (either the prelude row, the in-section predecessor, or the sentinel).

### 7. `lambda/handler_render_plan.py`

- After compiling the chain, inspect the source set. Set `plan.raster.prelude_rows = 1` if any `slv-1` source is present; `score_coeff_prelude_rows = 1` if any `cf-1`; `score_param_prelude_rows = 1` if any `pm-1`. Otherwise 0.
- Thread the same values into the clip-task plan payload.
- For the first logical section (`section.solve_start == 0`), the manifest builder silently clamps to 0; no planner-side gating needed.

### 8. `stepfunctions/render_workflow.asl.json.template`

- Thread `plan.raster.prelude_rows` etc. into `ColorSolveScoreClipTask` payload and `ColorRasterMap` ItemSelector.
- Bilevel / coeff_bilevel paths do not use lag sources in v1 — their task payloads omit the flags (or pass 0).

### 9. `index.html`

- Extend `_solveScoreMetricAllowedSources` at `:3843-3847` to return the lag variants for metrics that support them. Without this, `_compileSolveScoreChain` rejects lag tokens before reaching the parser.
- `_compileSolveScoreChain` at `:3829` accepts `slv-1`, `cf-1`, `pm-1` source tokens.
- `_serializeSolveScoreChain` at `:4012` emits them unchanged.
- `_normalizeSolveScoreChain` preserves lag suffix on round-trip.
- `_renderArtifactSolveDisplay` (at `index.html:12720-12756` in current tree) shows lag suffix in the Render-tab summary so operators can see which statements use lag. Note: it already catches exceptions and falls back to plain metric name — a failure here wouldn't crash, but would silently lose chain display until `_compileSolveScoreChain` learns lag.
- Populate (`populateSelectedRenderArtifact`) and solve-score modal Load (`_applySolveScoreProgram`) both route through `_compileSolveScoreChain`; extending that single path covers both.
- Chain chip dropdown lag toggle is optional — if added, the source picker offers six values (`slv | slv-1 | cf | cf-1 | pm | pm-1`) gated by what the current metric actually supports (see step 2's `solve_metric_supports_source` decisions).

### 10. Explicit rejection elsewhere

The compiler (Python + JS) accepts lag tokens unconditionally. Context-specific rejection lives in the handlers that call into non-lag-aware C binaries:

- `handler_palette_chunk.py` — calls `solve_palette_chunk(_mt)`. Reject any lag source in the payload at entry with a clear error.
- `handler_extract_palette_from_step_scores.py` — no chain evaluation (shuffles existing raw scores); no rejection needed unless a future variant adds scoring.
- `handler_palette_finalize.py` — same; verify during implementation.
- Bilevel scoring paths — none reach the solve-score evaluator (bilevel uses its own single-metric clip).

Defense-in-depth: the evaluator's hard-fail-on-NULL-prev behavior (§Shared-binary evaluator hazard) already makes direct-CLI invocation of `solve_palette_chunk --score_sources=slv-1` abort at the evaluator call. The handler-level rejection is an operator-friendly earlier exit with a clearer error message.

### 11. `lambda/handler_storage.py` solve-score-programs routes

The `/save-solve-score-program` (`:142-162`) and `/fetch-solve-score-program` routes re-compile the chain server-side via `compile_solve_score_chain_or_legacy`. Once the Python compiler (step 1) accepts lag tokens, save and load work transparently. No additional changes in this file, but the behavior should be stated explicitly in tests:

- Save a lag-1 program → round-trip through list/fetch → program chain byte-identical.
- Server-side re-compile on save rejects `slv-2` just as client-side compile does.

The bundled seed catalog at `solve-score-programs/*.json` and `scripts/seed_solve_score_programs.py` are passthrough — no change.

## Implementation Order

Must land atomically (single commit) because of the evaluator signature change:

1. `solve_score.h` evaluator signature + source enum + CSV parser.
2. `solve_score_chain.py` source parsing + emission.
3. `solve_proximity_stats.c` clip prepass lag support + argv.
4. `roots2pix_mt.c` worker loop + per-thread byte-range overlap + argv.
5. `handler_solve_proximity.py` + `handler_raster_mt.py` argv wiring + validation.

Can land in a follow-up commit (after the atomic core):

6. `logical_sections.py` prelude_rows kwarg + manifest rewriting.
7. `handler_render_plan.py` derive prelude values + emit on plan.
8. ASL template threads prelude values.
9. `index.html` parser/serializer.
10. Other-handler rejection messages.
11. Tests.

## Tests

### Compiler

- `tests/test_solve_score_chain.py`: parse and compile programs using each of `slv-1`, `cf-1`, `pm-1`; assert `metrics[i].source` round-trips; assert `score_sources` CSV round-trips through `parse_solve_score_source_csv` (via subprocess of a native test binary or a Python bindings shim).
- Reject `slv-2`, `slv0`, `slv-X`.
- Mixed `slv` + `slv-1` in one chain compiles cleanly.

### Native source parser

- Subprocess test against `solve_proximity_stats` and `roots2pix_mt` with `--score_sources=slv,slv-1,cf-1` — the binary parses without error.
- Reject `--score_sources=slv-2` at binary level.

### Manifest

- `tests/test_logical_sections.py`: `build_source_spans(..., prelude_rows=1)` widens logical size and shifts span offsets; first-section case (`solve_start == 0`) silently clamps to 0.
- Segment-boundary clipping: prelude row that would cross into a prior segment picks up that segment's tail correctly.

### Clip prepass

- `tests/test_solve_proximity_handler.py`: clip ranges for a lag-1 metric on a synthetic uniform input match an explicit Python reference. The reference computes a lag-0 single-solve metric value at each index `i` using the roots at index `i - 1` (with `i = 0` using the roots at `0` per sentinel). Semantics: lag-1 = "same metric, shifted roots", not pairwise.
- `tests/test_solve_proximity_handler.py`: direct-CLI invocation of `solve_proximity_stats --mode=clip --score_sources=slv-1 --program=...` (no Python handler) produces the same clip ranges as the handler-dispatched run.
- Negative: evaluator hard-fails when called with a `slv-1` source but NULL prev pointer. Tested via a direct-CLI invocation of `solve_palette_chunk --score_sources=slv-1 ...` (this binary does not pass prev pointers) asserting nonzero exit and a clear error in stderr.

### Fused raster byte-level parity

- New `tests/test_lagged_scores_native.py`: run `roots2pix_mt` twice with synthetic inputs:
  1. `proximity(slv)` — control.
  2. `proximity(slv-1)` — the test. On a uniform-radius input where roots are shifted by one row, the lag-1 output at solve `i` should equal the control output at solve `i - 1` (serpentine-shifted), except solve 0 where both produce the same value (sentinel).
- **Sentinel at global solve 0**: explicit assertion that `output[0]` under `proximity(slv-1)` equals `output[0]` under `proximity(slv)` exactly.
- **Minimal lag-only program**: a chain containing only `proximity(slv-1)` and nothing else compiles, clips, and renders without error.
- **Mixed chain**: `avg(proximity(slv), proximity(slv-1))` produces output that matches the avg of the two control runs pixel-for-pixel.
- Per-source test: `cf-1` extends coeff manifest by one row; `pm-1` extends param manifest; `slv-1` leaves coeff/param manifests alone.

### Cache / fingerprint

- **Fingerprint-mismatch rejection**: generate a clip artifact with `slv` chain, then run raster with `slv-1` chain. Raster handler must reject with the chain-fingerprint-mismatch error at `handler_raster_mt.py:210-216`.
- **Fingerprint parity**: two chains that differ only in the source tag (`slv` vs `slv-1`) produce different `chain_fingerprint` values.

### Hard-fail mechanism

- **Evaluator hard-fail**: invoke `solve_palette_chunk --score_sources=slv-1 --program=<RPN>` (a binary that doesn't supply prev pointers). Expect nonzero exit and a stderr message matching `"solve_score: metric slot .* requires .* source but prev pointer is NULL"`.
- **Binary-side prelude validation**: invoke `roots2pix_mt --score_sources=slv-1 --prelude_rows=0` with `solve_start=100` (simulating a non-first section). Expect nonzero exit with a clear error.

### Round-trip through storage / modal

- **Populate round-trip**: save a lag-1 render, navigate to Populate, restore the chain into the form, dispatch same form → plan's `solve_score.chain` byte-identical to the original artifact's.
- **Solve-score-modal Load round-trip**: `/save-solve-score-program` with a lag-1 chain → `/list` returns it → `/fetch` returns identical chain → `_applySolveScoreProgram` restores into active tab's chain without error.

### Per-thread byte-range overlap

- `tests/test_raster_mt.py`: with `threads=4` and a manifest prelude of 1, each native thread's `byteStart` starts one row earlier than without prelude, and its `sectionBytes` is one row bigger. Assert this via a mocked or instrumented `multispan_reader_read_exact` that records the requested ranges.

### Root transforms

- `tests/test_lagged_scores_native.py`: with non-identity `rotation` and a non-empty `root_xforms` chain, the lag-1 metric's expected value is computed against the **transformed** previous roots, not the raw previous roots. A control test with identity transforms should produce identical results to the raw-prev case.

### Plan

- `tests/test_render_plan.py`: a chain using `slv-1` produces `plan.raster.prelude_rows == 1`; without lag, `== 0`. Same for `cf-1`/`pm-1` mapping to their respective plan fields.

### Workflow contract

- `tests/test_render_workflow_definition.py`: ASL threads `$.plan.raster.prelude_rows` etc. into the ClipTask payload and RasterMap ItemSelector.

### Rejection in other contexts

- `tests/test_palette_chunk_handler.py`, `tests/test_extract_palette_from_step_scores.py`: a payload with a lag source is rejected with `"lag sources not supported in <context>"` error.

### Frontend

- `tests/test_frontend_js.sh`: `_compileSolveScoreChain` accepts `slv-1`/`cf-1`/`pm-1`; `_serializeSolveScoreChain` round-trips them.
- `tests/e2e/render-solve-score.spec.js`: paste a JSON program with lag sources; assert it loads without error and the chain display includes the lag tag.

## Compatibility Notes

### Backward compatibility

- Programs with no lag sources produce byte-identical output (clip ranges, raster pixels) before and after the refactor.
- Chain JSON files without lag tags parse identically.
- `program_spec` is unchanged, so existing artifact fingerprints don't drift.
- The evaluator signature change is a C-internal ABI change. Because deploy is atomic per Lambda family, all callers update together.

### Forward compatibility

- If lag-N (N > 1) ever lands, `prelude_rows` becomes an integer >= 0, metric sources gain parameterized variants, and the byte-range overlap generalizes. Nothing in v1 blocks that.

### ASL

- New fields default to 0 in older plans. Old clip/raster binaries ignore the flag (argv is optional with default 0). Atomic deploy of ASL template + binaries avoids drift.

## Non-goals

- Arbitrary lag-N (lag-2, lag-3, …).
- Lag semantics in bilevel, coeff_bilevel, or any non-fused-color raster path.
- Lag semantics in palette extraction or solve-score histogram pipelines.
- Cross-thread reads or barriers.
- Supporting lag in the `INPUT_TMPFILE` or single-span `INPUT_SECTIONED` branches of `roots2pix_mt.c` (both are dead for fused).
- Mixed lag within a single statement (e.g., `avg(slv, slv-1)` is allowed because the stack machine composes two separately-lagged statements; `weighted_sum_of_lag0_and_lag1` as a new op is not).
- Spatial-adjacency sentinel at row-flips and pass boundaries. Lag is index-based; discontinuities at serpentine row flips and pass boundaries are accepted. Sentinel `prev = current` applies only at global solve 0.
- Clip-artifact schema version bump to v3. The v2 shape carries arbitrary source string tokens; lag tokens round-trip without format change.
- Pairwise/change metrics (`"how much did the roots move from solve i-1 to solve i"`). Out of scope for v1; would need a new metric family, not a new source tag.

## Acceptance Criteria

Feature is done when all of the following are true:

1. **Grammar**: `slv-1` / `cf-1` / `pm-1` are accepted (lowercase, exact, regex `^(slv|cf|pm)-1$`); all other lag forms rejected with a clear error, in both Python and JS compilers.
2. **Allowlist extensions**: `VALID_SOLVE_SCORE_SOURCES`, `_METRIC_ALLOWED_SOURCES`, `_validate_metric_source` (Python) and `_solveScoreMetricAllowedSources` (JS) all accept the new tokens.
3. `parse_solve_score_source_csv` in `solve_score.h` accepts the three new tokens.
4. **Evaluator hard-fail**: `solve_score_eval_program_with_sources` emits `fprintf(stderr, ...)` and calls `abort()` when a `*_LAG1` source is tagged and its prev pointer is NULL. No silent fallback.
5. **Sentinel in callers**: sentinel for global solve 0 (`prev = current`) is implemented in the raster worker and the clip worker by explicit assignment before the evaluator call.
6. **Lag-0 parity**: programs with no lag source produce byte-identical clip ranges and raster pixels before and after the refactor.
7. **Clip-prepass lag parity**: clip ranges for lag-1 metrics match a Python reference that, for each `i`, evaluates the metric against the roots at index `i - 1` (with `i = 0` using roots at index `0`).
8. **Raster lag semantics**: `proximity(slv-1)` output at solve `i > 0` equals `proximity(slv)` output at solve `i - 1`; outputs at solve 0 are equal (sentinel).
9. **Minimal lag-only program**: a chain containing only `proximity(slv-1)` compiles, clips, renders, and round-trips through storage without error.
10. **Mixed chain**: `avg(proximity(slv), proximity(slv-1))` produces pixel-for-pixel output matching the mean of two independent control runs.
11. **Per-source manifest extension**: `cf-1` extends coeff manifest only; `pm-1` extends param manifest only; `slv-1` leaves coeff/param manifests alone.
12. **Two-level overlap formula**: every native thread in `roots2pix_mt` has a valid `prevStep` at loop start without cross-thread reads; the three cases from §Per-thread `byteStart` formula all work in tests.
13. **Transformed-previous**: lag-1 metrics use the transformed-previous step (after `prepare_step`), not the raw previous roots.
14. **Frontend round-trip**: JS parser/serializer round-trips programs containing lag sources; Populate and solve-score-modal Load both correctly restore lag-using programs into the form without error.
15. **Storage round-trip**: `/save-solve-score-program` + `/fetch-solve-score-program` round-trip lag-using programs; server-side re-compile accepts the lag grammar and rejects `slv-2` / etc.
16. **Handler rejection**: `handler_palette_chunk.py` rejects any lag source at entry with a clear error using the `any_lag_source` helper.
17. **Direct-CLI rejection**: direct-CLI invocation of `solve_palette_chunk(_mt)` with a `--score_sources=slv-1` CSV aborts at the evaluator with a visible stderr message.
18. **Binary-side prelude validation**: `roots2pix_mt` rejects a `*-1` CSV when `prelude_rows=0` and `solve_start != 0`.
19. **Step-scores reuse**: cached `step_scores.raw` from a lag-1 render is reused correctly by repalette / recolor / extract-palette-from-step-scores (integration test, no new code).
20. **Fingerprint differentiation**: `chain_fingerprint` differs between otherwise-identical lag-0 and lag-1 chains; a cached lag-0 clip artifact is rejected when raster requests a lag-1 chain.
21. **`legacy_compatible`**: naturally false when any lag source is present, verified via test; no new branch needed in the compiler.
22. **Preview unchanged**: `handler_compute_preview.py` has no code change and no path invokes a lag source.
23. **Hist mode consistency**: `solve_proximity_stats --mode=hist` path inherits the prev-pointer change via the shared worker loop; dead for fused color, verified; if anything else invokes hist with a lag CSV, it produces the same lag-aware scores as clip.
24. **ASL threading**: `$.plan.solve_score.chain` carries lag tokens unchanged through ClipTask and RasterMap; `prelude_rows` / `score_coeff_prelude_rows` / `score_param_prelude_rows` are threaded into both tasks from `$.plan.raster.*`.
25. All new and existing solve-score tests pass.

## Implementation Map

- `lambda/solve_score.h` — evaluator signature (hard-fail on NULL prev), source enum, CSV parser (`:702-724`, `:787-805`, `:1109-1205`)
- `lambda/solve_score_chain.py` — allowlists + source parsing + emission (`:52`, `:71-78`, `:156-160`, `:324`, `:401-415`, `:430`), new `solve_score_uses_lag_source` helper
- `lambda/solve_proximity_stats.c` — clip worker loop gets prev pointers via `(s > 0) ? ptr_at(s-1) : ptr_at(s)` at `:366-395`; local `eval_score_or_program_with_sources` wrapper gains prev args
- `lambda/handler_solve_proximity.py` — thread lag-aware CSV + argv to clip subprocess (`:180`, `:306`, `:325`)
- `lambda/roots2pix_mt.c` — argv parse, binary-side validation, per-thread byte-range overlap (`:289-320`, `:909-914`), worker loop with `prevStep` + transformed-prev via `prepare_step` (`:356`, `:534-572`), sentinel at `p == 0`
- `lambda/handler_raster_mt.py` — thread prelude values into manifests and argv
- `lambda/logical_sections.py` — `prelude_rows` kwarg on `build_source_spans` (`:329`), `build_native_multispan_manifest` (`:378`); clamp to 0 when `solve_start == 0`
- `lambda/multispan_reader.c` / `.h` — **no change** (prelude fits inside existing logical layout)
- `lambda/handler_render_plan.py` — derive plan-level prelude values per source family from `solve_score_uses_lag_source`
- `stepfunctions/render_workflow.asl.json.template` — thread `prelude_rows` / `score_coeff_prelude_rows` / `score_param_prelude_rows` into ClipTask payload and RasterMap ItemSelector
- `index.html` — `_solveScoreMetricAllowedSources` (`:3843-3847`), `_compileSolveScoreChain` (`:3829`), `_serializeSolveScoreChain` (`:4012`), `_renderArtifactSolveDisplay`
- `lambda/handler_storage.py` — solve-score-programs save/load automatically inherits lag via server-side re-compile; no source change, add tests
- `lambda/raw_sidecar.py` — `source` is a free string (`:63`), lag tokens round-trip unchanged. No code change; document only.
- Rejecting handlers: `handler_palette_chunk.py` (primary, uses `any_lag_source` helper). `handler_extract_palette_from_step_scores.py`, `handler_palette_finalize.py`, bilevel scoring paths: **no chain evaluation present — verified via grep.** No rejection code required there; document verdict.
- Unchanged by design: `handler_color_repalette.py`, `color_recolor_raw.py`, `handler_recolor_from_raw.py` — all operate on cached raw greyscale, no rescoring
- Unchanged by design: `handler_compute_preview.py` — no chain evaluation, no rejection needed, no lag support
- Tests: `test_solve_score_chain.py`, `test_solve_proximity_handler.py`, `test_logical_sections.py`, `test_raster_mt.py`, `test_render_plan.py`, `test_render_workflow_definition.py`, `test_frontend_js.sh`, new `test_lagged_scores_native.py`, `tests/e2e/render-solve-score.spec.js`, `test_solve_score_program_storage.py` (save/load round-trip), `test_palette_chunk_handler.py` (rejection test)
