# CR34 — First comparison-grade production timings

Build `f3231c2df` (`build_id f3231c2df-20260713T100646Z`), deployed 2026-07-13 ~10:07–10:25 UTC.
This is the first capture taken **after** the CR33/34 telemetry post-mortem fixes went live, so it is the
first baseline that satisfies the post-mortem's own comparison rules: fleet-wide build identity in every
task result, truthful timing names, role-scoped aggregation, fail-closed CloudWatch joins. Everything
before this deploy stays classified exploratory.

Captured with `scripts/capture_production_telemetry.py` (schema `pp-production-telemetry-v3`, live mode)
— both runs exited 0 with **zero validation problems**: every task joined to exactly one CloudWatch
REPORT, cardinality checks satisfied, single build across all tasks. Full reports (gitignored, contain
job identity) in `reports/production/20260713-{compute,render}-*.{json,md}`.

| run | execution | workflow wall |
|---|---|---:|
| compute | `compute_aberth_mt_run_1783939740090_dv5xd9` | 54.593 s |
| render | `render_color_run_1783940285416_k2zpe3` | 77.634 s |

## 1. Compute run

Identity: job `compute_mrj3nt15`, N=10000 × times=1 (100M solver steps), 61 fused chunks,
function `const`, solver `aberth_mt`, fused_threads=4, coeffgen_threads=4, arm64 @ 10240MB
(6 online CPUs). Program fingerprints: coeff `f5f697159b44e8d9` (70 tokens: 53 typed-scalar,
4 typed-vector, 6 selector, 0 native), param `4d27a829416bdd94` (1 token, legacy-static).
0 task failures, 75/75 scheduled/succeeded, 0 retries.

### 1.1 Per-chunk stage decomposition (61 fused-chunk tasks)

Handler-wall sum 1592.1M µs (mean 26.10 s/chunk). Stage sums account for **98.96%** of it —
the instrumentation is effectively complete; residual ~0.27 s/chunk is handler plumbing
(S3 client setup, payload/status I/O).

| stage | sum (s) | mean/chunk (s) | share of handler wall |
|---|---:|---:|---:|
| solve | 790.3 | 12.96 | **49.6%** |
| uploads (total) | 643.5 | 10.55 | **40.4%** |
| — upload roots bin | 343.2 | 5.63 | 21.6% |
| — upload coeffs | 277.2 | 4.54 | 17.4% |
| — upload params | 23.1 | 0.38 | 1.5% |
| coeffgen | 136.6 | 2.24 | 8.6% |
| param gen | 5.1 | 0.083 | 0.32% |

Within-stage native fractions: solve_native/solve = **97.5%** (launcher+transport overhead 2.5%),
coeff_native/coeffgen = 99.7%. `avg_iterations` 2.32 (warm-start chain healthy).

### 1.2 Data volumes and upload throughput

**69.6 GB uploaded per run**: roots bin 33.6 GB + coeffs 34.4 GB + params 1.6 GB.
Per task that is 550.8 MB bin in 5.63 s (**~98 MB/s**) and 563.9 MB coeffs in 4.54 s (**~124 MB/s**),
serial, after solve completes.

### 1.3 Confirmations of deployed fixes

- `param_scheduler=static_file` in every chunk meta — the CR33 F1 fused-scheduler fix is what
  actually ran in production (real output path, static scheduler; no ordered-ring).
- `build_id`/`git_sha` present in all 61 results + coeffgen + sweep-mt (F1 post-mortem, fleet-wide env).
- `solve_native_us` vs `solve_us` split present (F13); plan invariants travel in results (F9).
- Cross-layer consistency: state wall p50 29.29 s > CW REPORT p50 28.23 s > handler wall p50 28.22 s
  > stage sum — each layer nests inside the previous with plausible gaps (~1.07 s SF dispatch per task;
  CW and handler agree to 9 ms at p50).

### 1.4 Caveats on this run's walls

- **Cold fleet**: 50/61 chunks were cold starts (first run on new code). Fused-chunk billed
  16,162 GB-s.
- **Concurrency ramp**: map wall 48.15 s vs slowest chunk 30.05 s — Lambda scaling staggered the
  61 dispatches. Warm repeat runs will show lower map walls with identical per-chunk numbers.
- Therefore: build-to-build comparisons should use **per-chunk stage sums and distributions**
  (concurrency-independent), not workflow/map walls, unless fleets are equally warm.
- The June exploratory run (49 chunks, workflow 36.0 s, solve 40.9%/uploads 40.9%) is **not** a valid
  delta against this run: different chunk count, different program, unattributed build. This run is
  baseline zero.

## 2. Render run

Identity: same job (`compute_mrj3nt15`), mode color, pix=5000, quality 90, engine mt,
`input_mode=multispan_sectioned`, 22 raster tasks (raster_mt_threads=4), finalize_workers=16,
palette `long_washington_stripe_teal_orange_11`, solve-score fingerprint `aec69d1df218df62`
(plan: 1 metric, 0 dup slots, no lag). 0 failures, 30/30, zero cold starts (warm fleet).
Workflow 77.63 s = raster map 61.65 s (79%) + finalize 12.70 s + plan 2.01 s + clip/status remainder.

### 2.1 Raster is download-bound (22 tasks)

Per-task means: handler 20.63 s = prep 0.09 + subprocess 19.96 + upload 0.58.

| component | mean/task (s) | share of handler wall |
|---|---:|---:|
| download wall (inside subprocess) | 18.69 | **90.6%** |
| native wall (inside subprocess, overlaps downloads) | 5.52 | 26.8% |
| fragment upload | 0.58 | 2.8% |

- Each task reads its own 1.527 GB section slice; total input = **exactly** the 33.6 GB bin —
  sectioned range-GETs have zero read amplification.
- Retrieval runs at **~82 MB/s per task** (aggregate ~1.8 GB/s across 22 concurrent tasks).
  Download-thread utilization is 90% (67.3 s busy across 4 threads inside an 18.7 s span) —
  the threads are genuinely bandwidth-bound, not idle.
- Native work already hides inside the download span (native worker-sum 5.03 s over a 5.52 s span
  while downloads continue): compute is **not** the raster bottleneck; retrieval is.
- Dedup scale: 190.2M roots deduped per task vs 0.66M plotted (roots_plotted 14.5M total,
  roots_clipped 0). Output: 72.6 MB image fragments + **500 MB associated-palette fragments**
  + 100 MB step scores.

### 2.2 Finalize decomposition (12.35 s handler wall, 100% honest timers)

| component | ms | share |
|---|---:|---:|
| associated-palette merge (download+decode+apply) | 7,274 | **58.9%** |
| step scores (download 2,212 + concat 80 + upload 921) | 3,278 | 26.5% |
| assemble fragments | 790 | 6.4% |
| render+encode (native, one process) | 407 | 3.3% |
| artifact uploads (raw 304 + image 125 + preview 40) | 469 | 3.8% |
| presign + meta overlay | 50 | 0.4% |
| unaccounted | 44 | 0.4% |

No fabricated zeros remain (the old `encode_ms: 0` sites are gone); `lut_ms` 0 = LUT cache hit.
Final image 11.19 MB; finalize billed 123.6 GB-s; raster fleet 4,544 GB-s.

## 3. Confirmed optimization order (§12, refined by these numbers)

1. **Fused-chunk upload/compute overlap** — 40.4% of chunk wall is serial S3 upload. The truthful
   split shows this is two different problems:
   - **(a) coeffs+params (18.9%)**: complete *before* solve starts; hiding them under solve is a
     straightforward background-upload change. Ceiling: chunk 26.1 → ~21.2 s (**~1.23×**).
   - **(b) roots bin (21.6%)**: solve *output*; hiding it requires streaming multipart upload as the
     solver fills sections (design needed: part sizing, memory at 10 GB, failure semantics).
     Cumulative ceiling: chunk → ~15.5 s (**~1.68×** throughput; warm map wall ~48 → ~30 s class).
2. **Raster retrieval** — 18.7 of 20.6 s per raster task is section download at ~82 MB/s/task.
   Levers to evaluate against this baseline: more download threads (utilization is already 90%, so
   only helps if per-stream throughput is the limit), bin compression, or consuming step scores in
   place of full root sections where the plan allows.
3. **Associated-palette removal via step scores** — 7.3 s of 12.4 s finalize plus 500 MB of fragment
   traffic (raster upload → finalize re-download) exists only to rebuild what step scores already
   carry (this run's plan: 1 metric, no lag).

## 4. Secondary observations

- **F7 typed-scalar fusion did not engage** on this program: 53 typed-scalar tokens but
  `coeff_fused_regions=0` — the 6 selector tokens break every straight-line region. Not a current
  lever (coeffgen is 8.6% and 99.7% native), but worth remembering when reading fusion telemetry:
  fusion coverage is program-shape-dependent, and `const` sweeps get none.
- solve_us − solve_native_us ≈ 0.32 s/chunk (2.5%): launcher/pipe overhead is real but small;
  no action warranted.
- Lores/probe roles aggregate separately and cleanly (coeffgen 3 tasks, sweep-mt 1 task) — the
  role-scoping that CR33/34 post-mortem F4 demanded is visible in practice: fused `solve_us` n=61
  with no lores contamination.

## 5. Comparison rules for the next capture

Same-program reruns can be matched on the identity block (N, n_chunks, function, fused_threads,
program fingerprints) — the collector rejects mixed builds and missing identity, so a §12-1 A/B is:
deploy candidate build → rerun same job spec → compare per-chunk `upload_*_us`/`solve_us`
distributions and warm map walls; bytes (bin_size/coeffs_size, fragment sizes) must be identical.
