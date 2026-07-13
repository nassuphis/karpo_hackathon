# CR33 Implementation Results

Date: 2026-07-13.
Implements code-review-33.md (adversarial performance audit of the Param,
Coeff, and Solve Score systems) in the review's phase order. Base for all
A/Bs: `32e01ff` (pre-CR31); CR33 HEAD: `e4e057d`; the CR32-final column
isolates this campaign from the earlier ones. **Every A/B row below reported
`bytes ok`** (SHA-256 of complete output files, interleaved reps, enforced
by the tool). Correctness gates at HEAD: full suite **1988 passed + 122
subtests**, including new family-engaging solve parity cases and the
stream-vs-file byte pins.

Commits: `e321fd4` (F1), `06750eb` (F12 seams), `985d95b` (F4+F2),
`649a73c` (F3), `53d8c5c` (F5/F6/F8/F11), `e4e057d` (F7).

## What shipped, by finding

- **F1 — fused scheduler fix (production).** `_run_param_gen_local` passes
  the real output path; `"-"` had routed every multithreaded fused param
  chunk onto the ordered-ring stdout scheduler (ring cost measured +14% t2 /
  +39% t4 / ~10× t8 against the static scheduler; byte-identical either
  way). Metadata now reads from stdout; `param_scheduler` is recorded in the
  handler meta. Invocation-contract tests + native stream-vs-file byte pins.
- **F12 — durable seams.** Stream-vs-path param invocation, static/dynamic
  legacy forms (rect/rply/star/rrect/rtheta), degree-128/256 coeff direct
  ops with repetition, and six non-pair solve family bundles are permanent
  benchmark cases (family baselines reproduced the review's probe numbers
  to within noise — axis_median3 967.5 vs 967.4 ns).
- **F4 — conditional centroid.** `solve_metric_uses_centroid()` is the
  single predicate; extrema/modulus/unit-circle/angular-separation metrics
  stop paying an O(d) pass they never read.
- **F2 — param legacy load-time plan.** Static args resolve + real-validate
  once at parse (NONE/STATIC/DYNAMIC classification); measured transform
  plans: rect (shared compiled body), rply + star (vertex tables whose
  entries use the identical per-k angle expressions; twin bodies pinned
  `FP_CONTRACT OFF` — the CR32 NaN-sign lesson). rrect inspected and
  skipped: per-point `pow`/trig dominates, nothing meaningful to hoist.
  Dynamic and stateful (dither) paths byte-verified untouched.
- **F3 — shared coeff kernels + in-place.** `coeffUnaryKernelVec` /
  `coeffBinaryKernelVec` own the cheap-op hoists and the non-finite clamps;
  typed ops, the legacy scratch path, and the new direct `poly -> poly`
  fast paths all call them. Native transforms/roll/argsort deliberately
  keep the scratch path.
- **F5/F6/F8/F11 — solve.** Extrema + radial family passes (exact original
  expressions and accumulation order; SD keeps its two-pass structure;
  min_mod keeps its zero-skip); `median_inplace` is deterministic bounded
  introselect (2400-case exact parity vs the sorted reference, qsort
  fallback on degenerate partitioning); the requirement plan (engagement,
  pair masks incl. new `needMin`, family counts) computes once at parse;
  program-level `usesLag` is a prepared bit. Angular histogram/orders and
  centroid/covariance family passes remain future work.
- **F7 — typed-scalar fusion.** Parse-time marking of straight-line
  {push_scalar, binary, unary, poke} regions with net-zero stack effect;
  a tight local-stack interpreter runs them with the same primitives,
  argument resolution, index conversion, and error messages. Anything else
  falls back to the token loop.

## M3 results (Apple M3 Max, clang -O3, 11 reps, medians, bytes ok)

| Case | 32e01ff | CR33 | vs 32e01ff | vs CR32-final |
|---|---:|---:|---:|---:|
| mqlacwaq_coeff (harvested) | 24.32 ms | 14.48 ms | **−40.5%** | −27.8% |
| mqlacwaq_param_coeff | 25.96 ms | 15.56 ms | **−40.0%** | −27.5% |
| coeff4_scalar21 | 3.73 ms | 2.78 ms | **−25.5%** | −15.8% |
| param_rect_static | 18.10 ms | 12.64 ms | **−30.2%** | −27.5% |
| param_rply_static | 26.23 ms | 13.43 ms | **−48.8%** | −49.3% |
| param_star_static | 24.48 ms | 12.41 ms | **−49.3%** | −49.1% |
| param_rrect_static | 29.92 ms | 27.42 ms | −8.4% | −7.6% (arg prep only) |
| param_rect/rply dynamic | — | — | +0.7…+2.6% (noise) | ~0% |
| coeff35_neg16 | 64.64 ms | 33.22 ms | **−48.6%** | −47.4% |
| coeff128_neg16 | 41.85 ms | 13.85 ms | **−66.9%** | −66.9% |
| coeff256_neg16 | 77.63 ms | 21.04 ms | **−72.9%** | −73.1% |
| coeff256_add | 20.00 ms | 16.32 ms | **−18.4%** | −18.4% |
| param_expr_t8 | 24.98 ms | 2.23 ms | **−91.1%** | −1.3% (CR32 work) |
| controls (baselines, rev, sin) | — | — | −1.6…+1.5% | ~0% |

Solve production-entry micro (M3, ns/call): one-slot max_re 53.0 → **41.2**
(centroid gate); extrema4 214 → **113**; radial7 1069 → **312**;
axis_median3 1025 → **396** (introselect); centroid9 1008 → **831**;
dist_unit_circle one-slot 110 → **100**; pair bundle 2522 → **2313**
(prepared plan + needMin); dup-slot and one-slot proximity flat as designed.

## Graviton results (fresh c7g.2xlarge, gcc 11.5 -O3, bytes ok, box torn down)

| Case | 32e01ff | CR33 | Δ |
|---|---:|---:|---:|
| mqlacwaq_coeff (15 reps) | 46.12 ms | 17.56 ms | **−61.9%** |
| mqlacwaq_param_coeff | 49.46 ms | 21.83 ms | **−55.9%** |
| mqlacwaq_baseline (control) | 2.23 ms | 2.25 ms | +0.9% |
| param_rect_static | 20.63 ms | 13.27 ms | **−35.7%** |
| param_rply_static | 40.65 ms | 16.20 ms | **−60.2%** |
| param_star_static | 36.24 ms | 14.64 ms | **−59.6%** |
| param_rrect_static | 44.93 ms | 44.65 ms | −0.6% |
| rect/rply dynamic | — | — | −0.4…+1.9% (untouched) |
| coeff35/128/256_neg16 | 136.6/61.3/98.3 ms | 103.3/36.0/49.5 ms | **−24.4/−41.3/−49.6%** |
| param_expr_t2/t4/t8 | 11.2/5.7/5.7 ms | 10.3/5.5/3.2 ms | −7.9/−5.0/−44.2% |

Graviton solve micro (base → CR33): extrema4 326 → **166** (−49%), radial7
3363 → **850** (−75%), axis_median3 1859 → **701** (−62%), centroid9 1666 →
**1401**, dup-slot 1553 → 982, pair bundle 12714 → 8121 (**1.57×**),
one-slot max_re 79 → 55.

The pattern from CR31/32 repeats: Graviton's slower cores make removed
per-point/per-token fixed costs worth MORE in production than on M3 — the
harvested program's −40% (M3) is −62% on the deployment architecture.

## Honest notes

- `param_expr_stream_t4` (the ring-scheduler seam case) is noisy across
  runs (+6.9% vs 32e01ff in the final matrix, +0.3% vs CR32-final); the
  ring is only production-relevant until F1's handler fix deploys, and the
  case exists to keep the seam measurable.
- rtheta static gains only ~2% — its transform math is one trig call the
  arg-prep can't remove; no transform-specific plan was warranted.
- Angular histogram/orders and centroid/covariance family passes (review
  F5's remaining families) are NOT implemented — the two implemented
  families carried the bulk of the measured opportunity; the angular-orders
  family in particular needs a no-recurrence contract decision first.
- The review's F9 (param peepholes/batch VM) and F10 (architectural fusion,
  upload overlap) are deliberately not started, per its own sequencing:
  production telemetry should first show the local evaluator work no longer
  dominates.
- **Production telemetry (the review's "Production Telemetry Needed" list)
  is implemented** as one structured summary per stage, never per row:
  the param meta line reports scheduler mode (serial / stream_ring /
  static_file), online CPUs, and legacy token classification
  (static/dynamic/prepared); the coeff meta lines (buffered and chunked)
  report the token histogram (typed-scalar / typed-vector / selector /
  native) and fusion coverage (regions + tokens); score-program parses emit
  one `pp_solve_plan` stderr line (metric/dup counts, lag bit, engagement,
  pair masks, family counts) when the caller opts in with
  `PP_PLAN_TELEMETRY=1` — Lambda stderr lands in CloudWatch; the fused
  chunk result carries `stage_telemetry` (all of the above forwarded, plus
  native-vs-wall elapsed per stage — the difference is process startup —
  file sizes, Lambda memory size, and architecture). All shapes are pinned
  by contract tests; the meta additions are byte-neutral on outputs
  (A/B verified).
- Deploy binaries rebuilt from `e4e057d` and Docker-gated; the fused
  handler change needs a real Lambda deploy (user-run) before
  `fused_threads` defaults are revisited.

Reports: `reports/vm_bench_arm64_ab_cr33_vs_32e01ff.json`,
`reports/vm_bench_arm64_ab_cr33_vs_cr32.json`,
`reports/vm_bench_arm64_cr33_micro.json`,
`reports/vm_bench_graviton_cr33_{mql,prep,star,neg,pt}.json` — all with
binary hashes and source commits pinned in metadata.
