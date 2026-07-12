# CR31 Post-Mortem: VM Performance Campaign

> **CORRECTED per code-review-32 (2026-07-12).** The adversarial audit of this
> campaign found six defects the original text certified as sound, all
> verified and all fixed in the CR32 remediation wave (same day):
> **(1)** the F7 fast kernels dropped the evaluator's non-finite clamp —
> overflow emitted `inf` where the VM's contract says `0` (CRITICAL; clamp
> restored, policy now pinned by tests/test_coeff_fast_kernel_policy.py);
> **(2)** the F4 feature cache made one-slot production programs 5.3–6.7×
> slower at `solve_score_eval_metric_slots` (rewritten: requirement plan,
> direct path for single-slot sources, masked pair pass, lazy heap state —
> one-slot back to baseline, bundle improved to 1.73×);
> **(3)** chunked t1 NEVER entered the blocked I/O — the "t1 gap is not
> syscalls" conclusion below compared per-row vs per-row (t1 now runs the
> same block engine: **−34.8% on M3**, the review's original target beaten);
> **(4)** the CPU cap could flip a full request between seed policies by host
> topology (guarded: the cap never crosses the serial/threaded boundary);
> **(5)** worker failure flags were racy plain ints (now `atomic_int`,
> TSan-gated by scripts/test-tsan-races.sh);
> **(6)** the "all deploy binaries rebuilt" claim was false — five binaries
> predated the sources their Docker run claimed to verify (freshness is now
> machine-checked: scripts/check_binary_freshness.py gates the Docker
> regression, and scripts/build-deploy-binaries.sh rebuilds the FULL set).
> Corrected numbers: see "CR32 corrected results" at the end of this file.
> Sections below keep the original text with inline corrections where a
> specific claim was false.

Implemented: 2026-07-12, on top of `32e01ff`.
Commits: `7ad40be` (P0+F1), `eba68ef` (P1), `a73c136` (P2), `5159951` (P3), final wave (P4-scoped, F7, F10, portability fix).
Host for all numbers: Apple M3 Max, arm64, Apple clang 17, `cc -O3 -pthread` — the same
methodology as the review (medians, interleaved A/B, SHA-256 output equality). **These are
relative results for this host; Graviton re-measurement is required before changing any
production sizing default** (see Residual Work).

## Verdict against the review's acceptance matrix

| Case | Review baseline | Target | Achieved | Verdict |
|---|---:|---:|---:|---|
| Harvested `compute_mqlacwaq`, Coeff Program | 23.81 ms | ≤ 21.4 ms after F1 | **19.61 ms (−16.5%)** | ✅ beat (F1 alone measured −15.9%) |
| Chunked deg-35, 65,536 rows, 1 worker | 151.5 ms | ≤ 132.8 ms | ~~152.8 ms (+1.1%)~~ **CR32: t1 never entered blocked I/O; unified engine → 99.8 ms (−34.8%)** | ✅ after CR32 |
| Chunked deg-35, 65,536 rows, 4 workers | 70.2 ms | no regression | **26.5 ms (−62.2%)** | ✅ far beyond |
| Chunked deg-35, 8 workers | 90.5 ms (worse than 4) | — | **13.8 ms (−84.7%)**, now the fastest | ✅ cliff eliminated |
| Param expr, 262k rows, 4 workers | 4.43 ms | no regression | ~~5.34 ms (+9.6%) "within MAD"~~ **CR32: repeatable +15–20% M3 regression of one-pwrite-per-row; write batching → 4.22 ms (−24.2%)** | ✅ after CR32 |
| Param expr, 8 workers | 24.31 ms | ≤ 1.3× best | **4.87 ms**, now the best count | ✅ cliff eliminated |
| Root rotate predecode | 28.4 ns | 15–25% A/B gain | implemented; direct micro unchanged by design (see F8) | ✅ structural |
| Solve 31-token VM | 56.9 ns | no regression | 56.6 ns | ✅ |
| Solve mixed pair-metric bundle | (new case) | ≥ 1.5× | **4288.8 → 2658.9 ns = 1.61×** | ✅ |

Every A/B in the campaign was **byte-identical** (SHA-256 of complete output files,
enforced by the tool — a mismatch is a fatal error, not a warning). **CR32
correction: byte equality held only for the finite fixtures measured — the F7
fast kernels changed non-finite semantics (overflow → `inf` instead of the
clamped `0`), which the matrix never exercised. Fixed and pinned; the policy
suite now covers overflow, subnormal, signed zero, and non-finite inputs.**
The full 13-file correctness gate plus the coeff VM property fuzz passed after
every phase: **214 passed, 63 subtests** at the end (now 326 + 63 with the
CR32 gates). ~~The ARM64 Docker runtime regression passed against freshly
cross-compiled deploy binaries.~~ **CR32 correction: five deploy binaries
(roots2pix_mt, solve_proximity_hist_sectioned, solve_palette_chunk_mt,
sweep_coeffgen, sweep_cm) predated the P1/P3/P4 source when that run
executed — it certified packaging, not the final code. Freshness is now a
machine-checked precondition of the Docker gate.**

## What shipped, per finding

### F0 — benchmark tool (P0)
`scripts/bench_program_vms.py` replaces the hidden dotfile probes (which are deleted):
deployment-level `-O3` build, buffered + chunked Coeff, Param thread sweeps, harvested
`compute_mqlacwaq`, direct Root/Solve microbench (`tests/native/vm_microbench.c`),
no-program baselines, per-case output SHA-256 (reps must agree), interleaved `--compare`
A/B that fails on any byte difference, git/compiler/arch/cpu metadata, median/min/max/MAD.
Reports land in `reports/vm_bench_<machine>_<git>.json`; the baseline and after reports
for this campaign are `vm_bench_arm64_baseline-32e01ff.json` and
`vm_bench_arm64_after-cr31.json`. Timing stays advisory — never in predeploy pass/fail.

### F1 — Coeff load-time argument plan (P1) — the headline single win
`coeffPrepareArgPlan` classifies every token at parse: `NONE` (shared zero frame — no
per-row 160-byte memset/resolve), `STATIC` (frame resolved once at load), `DYNAMIC`
(expression args/blend keep per-row resolution; also the fallback for anything unusual,
preserving original error behavior). Measured alone: **−15.9%** end-to-end on the
harvested program (review's own A/B predicted 11.7%).

### F6 — Param prepared plan (P1)
Scalar-expression wire triples lower once into typed tokens (Coeff parity);
`reg_used_mask` limits per-row register zeroing to registers the program can read;
the dead `uses_legacy_fast_path` field is gone from the C parser (wire key ignored;
Python compiler and persisted programs untouched). Measured: unit-circle expression
−2.1%, native token −2.7% — **smaller than hoped**: the decode was not the dominant
cost; `exp()` and (before P2) the scheduler were.

### F8 — Root prepared chains (P1)
`rt_prepare_chain` (called inside `parse_root_xform_file`) resolves dispatch, applies
defaults, and hoists `sin/cos` + `1/σ²` once; `apply_root_xforms` dispatches prepared
entries to hoisted-constant twins whose inner arithmetic is bit-identical, with the
original per-row path kept for hand-built chains. All seven consumer binaries benefit
with zero call-site churn. Note: the direct microbench times `rt_*` functions, not the
prepared chain, so its rotate number is unchanged by design — the review's own
interleaved probe measured ~25% for this exact hoist.

### F9-lite — Solve lag flags (P1)
Per-slot lag flags computed at parse; the lagged-slot row loop stops rescanning every
token per slot per row. (The program/source-level lag scans were already hoisted at
their call sites — the review's citation was accurate but most callers were one-time.)

### F2 — blocked chunk I/O (P2)
128-row `pread`/`pwrite` blocks with short-transfer resume loops and first-affected-row
error text. 131,072 syscalls → ~1,024 for a 256×256 chunk (verified live by the F10
counters: 32+32 for a 4,096-row run). ~~Single-thread chunked was unchanged
(+1.1%), i.e. the 1-worker gap vs buffered generation on macOS is NOT syscall
cost.~~ **CR32 correction (F3): that conclusion was measured on a path that
never executed this optimization — the `threadsUsed <= 1` branch kept the old
per-row pread/pwrite loop, so the t1 A/B compared per-row against per-row.
CR32 routes t1 through the same block engine (the serial loop is deleted;
worker counts now differ only in partitioning), and syscalls turn out to be a
large single-worker cost after all: t1 −34.8% on M3 (153.1 → 99.8 ms),
sin_t1 −27.5%, bytes identical. The review's ≤132.8 ms target is beaten.**
Multi-worker wins stand as measured: t2 −54.6%, t4 −62.2%, t8 −84.7% —
the syscall storm was also serializing workers in the kernel.

### F5 — Param static-range scheduler (P2)
Seekable outputs: each worker owns a contiguous row range and `pwrite`s rows at exact
offsets — zero synchronization on the success path. Streaming (`"-"`) keeps the ordered
ring. Thread count now capped by online CPUs (execution-plan only). t8: 24.7 → 4.9 ms
(−80%), now the best count. ~~t2/t4 read +3–10% vs baseline in the final
sweep — within the MAD we observed across runs.~~ **CR32 correction (F6):
"within MAD" was wrong — interleaved re-measurement showed a repeatable
+15–20% regression at t4 on M3 (the scheduler wrote one row per pwrite;
the fused-plan default is four workers). CR32 batches contiguous rows into
32-row blocks flushed with one pwrite (crop only trims the global head/tail,
so bytes cannot change; 128-row blocks lost write/compute overlap on Linux —
Graviton t4 read +2.1% — so the block is sized for overlap, still 32× fewer
syscalls): M3 t2 −18.9%, t4 −9.1%, t8 −91.1%; Graviton t2 −10.1%, t4 −5.6%,
t8 −45.9%.**
**Pre-audit finding (pre-existing, unchanged)**: serial (t1) dither output differs from
threaded (t2+) — the serial path seeds per pass, threaded per row. Dither outputs at
t2/t4/t8 verified byte-identical across old scheduler, new scheduler, and thread counts.

### F4 — Solve shared feature cache (P3)
Per (source,row): finite filter once; ONE pair traversal feeding min-distance, the
crowding sum (original i<j order), and per-root NN distances (bit-equal: `(a−b)² ≡
(b−a)²`, min is order-free); clusteriness sorts a copy so nn_variation's summation order
is untouched; raw-score memo makes duplicate-quantile slots free. Production-shaped
bundle case: **1.61×** (4288.8 → 2658.9 ns). Param-style metrics keep the raw-pointer
path; lagged slots keep the direct path (their root sets vary per lag depth).
**CR32 correction (F2): as shipped, this cache imposed its preparation cost on
EVERY call — two ~16.6 KiB memsets per invocation and an all-features pair
pass — making one-slot production programs 5.3× (max_re) to 6.7× (proximity)
slower at the production entry, which the micro's ordinary rows could not see
(they bypassed the entry). Rewritten in CR32: a requirement plan scans slots
first; a source with fewer than two cacheable slots takes the pre-CR31 direct
path verbatim; the pair pass computes only requested features; feature state
is lazy heap (no large stack frames, no wholesale clearing). One-slot cost is
back to baseline (53.0 → 54.5 ns max_re, proximity flat), duplicate slots are
1.9× faster than baseline, and the dense bundle improved to 1.73×
(4493 → 2598 ns).**

### F3 — vector ownership (P4) — deliberately SCOPED
Shipped: in-place typed unary/binary destinations (safe because every push copies, so
slots are exclusively owned; pop parks the circular head on the popped slot, so the push
re-occupies it — the scratch round-trip was pure copy overhead). Parity/fuzz green,
bytes identical. Measured effect at degree 35: ~0 (−0.2% to −1.1%) — **the review's
copy-cost diagnosis at this degree was overweighted on this host; the per-element
dispatch (F7) and per-token plumbing dominate those cases.**
NOT shipped, with reasons:
- full `CoeffStackRef` indirection + free-list (swap-as-metadata, emit ownership
  transfer): the highest-risk change in the review, whose measured upside at deg ≤ 35 is
  now known to be small; per its own guidance ("revert rather than retaining complexity")
  it should be attempted only with a dedicated debug-owner build and degree-256
  workloads that actually exhibit the cost.
- `poly = fn(poly)` selector-path in-place: requires a per-registry-function alias audit
  (`rev` in place is wrong naively); the scratch round-trip stays.

### F7 — hoisted kernels (P5)
Cheap unary kernels (`neg`, `conj`, `real`, `imag`) and vector-vector `add`/`subtract`/
`multiply` now select once outside the element loop; broadcasts and transcendentals keep
the exact generic path. **CR32 correction (F1, CRITICAL): as shipped these
kernels reproduced the ladder's arithmetic but dropped its final non-finite
clamp — `1e308 + 1e308` through a typed vector add emitted `inf` where every
prior evaluator emitted `0` (reproduced byte-level against `32e01ff`). The
clamp is part of each operation's semantics and is now applied inside every
fast kernel; overflow, subnormal, signed-zero, and non-finite-input behavior
is pinned by tests/test_coeff_fast_kernel_policy.py. Also corrected: the
harvested program's typed ops are SCALAR (no `_typed_push_vector` tokens), so
the mqlacwaq win belongs to F1 alone — these vector kernels never run there.** Combined with F1/F3 this took the harvested program from −15.9%
to **−16.5/−17.1%**. The synthetic `neg16` case barely moved because `poly = neg(poly)`
compiles to the *selector* path, not typed ops (see F3 note above) — a useful map of
where the remaining cost lives.

### F10 — opt-in perf counters (P5)
`-DPP_VM_PERF` builds count `pread`/`pwrite` calls and dynamic-arg resolves and emit one
JSON line to stderr per chunked/param run; normal builds compile the macros away.
Verified live: 4,096 rows → 32 preads + 32 pwrites. **CR32 correction (F10):
as shipped the counters saw only the blocked helpers — t1's per-row calls and
the param static scheduler's pwrites were invisible, so those runs reported
zero I/O while doing plenty. All direct I/O now flows through the shared
helpers (t1 via the unified engine, param static via block flushes), the
fields are renamed `direct_pread_calls`/`direct_pwrite_calls` (stdio-buffered
serial param output is intentionally uncounted), and expected counts are
pinned by tests/test_vm_perf_counters.py.**

## Bugs found by the campaign itself
- **musl portability**: `solve_score.h` used `uint8_t` without `<stdint.h>` — masked by
  clang's transitive includes, caught by `aarch64-linux-musl-gcc` when rebuilding
  `solve_proximity_stats`. Fixed; all deploy binaries rebuilt and the ARM64 Docker
  runtime regression passed.
- **Pre-existing dither seam**: serial vs threaded param_gen produce different dither
  bytes (pass-seeded vs row-seeded). Documented above; not changed.

## Numbers table (medians, 11 reps, this host)

| Case | Before | After | Δ |
|---|---:|---:|---:|
| mqlacwaq_coeff | 23.48 ms | 19.61 ms | **−16.5%** |
| mqlacwaq_param_coeff | 25.75 ms | 21.35 ms | **−17.1%** |
| chunked35_t2 | 112.44 ms | 51.07 ms | **−54.6%** |
| chunked35_t4 | 70.02 ms | 26.47 ms | **−62.2%** |
| chunked35_t8 | 90.47 ms | 13.84 ms | **−84.7%** |
| chunked35_sin_t4 | 78.90 ms | 36.62 ms | **−53.6%** |
| param_expr_t8 | 24.69 ms | 4.87 ms | **−80.3%** |
| coeff4_scalar21 | 3.49 ms | 3.19 ms | −8.8% |
| solve pair bundle (micro) | 4288.8 ns | 2658.9 ns | **−38% (1.61×)** |
| chunked35_t1 | 151.21 ms | 152.83 ms | +1.1% (see F2) |
| param_expr_t4 | 4.87 ms | 5.34 ms | +9.6% (≤ MAD; watch) |

Reproduce: `python3 scripts/bench_program_vms.py` (full), or
`--compare <base> <cand> --cases '<glob>'` for interleaved A/Bs.

## CR32 corrected results (M3, interleaved A/B vs `32e01ff`, 11 reps, all bytes ok)

Final numbers from the CLEAN committed tree (`ee07ec1`, after the follow-up
audit fixes), report `reports/vm_bench_arm64_ab_cr32final_vs_32e01ff.json`
with base/cand binary hashes and source commits pinned in metadata:

| Case | Base | CR32 | Δ |
|---|---:|---:|---:|
| chunked35_t1 | 155.86 ms | 102.29 ms | **−34.4%** |
| chunked35_t2 | 119.80 ms | 52.41 ms | **−56.3%** |
| chunked35_t4 | 112.05 ms | 27.43 ms | **−75.5%** |
| chunked35_t8 | 97.20 ms | 14.20 ms | **−85.4%** |
| chunked35_sin_t1 | 197.61 ms | 143.57 ms | **−27.4%** |
| param_expr_t2 | 9.74 ms | 8.01 ms | **−17.7%** |
| param_expr_t4 | 5.11 ms | 4.13 ms | **−19.2%** |
| param_expr_t8 | 25.28 ms | 2.28 ms | **−91.0%** |
| mqlacwaq_coeff | 23.83 ms | 19.63 ms | **−17.6%** |
| mqlacwaq_param_coeff | 26.39 ms | 21.94 ms | **−16.9%** |
| coeff35 selector cases | — | — | −2.1% … +2.0% (noise; clamp costs nothing measurable) |
| micro: one-slot max_re (production entry) | 53.0 ns | 53.2 ns | flat (was 5.3× slower pre-CR32) |
| micro: one-slot proximity (production entry) | 365.2 ns | 361.8 ns | flat (was 6.7× slower pre-CR32) |
| micro: duplicate-slot proximity | 718.2 ns | 376.7 ns | **1.91×** (memo) |
| micro: two distinct cheap slots | 94.6 ns | 99.8 ns | +5 ns plan scan (was +27% before the reuse-gated engagement rule) |
| micro: pair bundle | 4493.1 ns | 2629.3 ns | **1.71×** |
| micro: prepared root affine3 chain (parsed) | 44.1 ns (fallback) | 37.8 ns | **−14%** (follow-up: the first "flat" reading compared fallback to fallback — parse_root_xform_json did not prepare; it does now) |

Controls: mqlacwaq_baseline −0.2%, param_baseline +0.1%. Param rows are from
the final 32-row-block build (11 reps, interleaved); the ordered-ring BASE
scheduler is itself noisy run-to-run on macOS (its replacement was the point),
so param deltas vary more than chunked ones — candidate absolutes are stable.
New gates added by CR32: fast-kernel numerical policy, param seed-policy/CPU-cap
byte pins, root prepared-vs-legacy parity (24 chains), cache-engaging solve
parity, PP_VM_PERF count pins, TSan failure-path gate (validated to catch the
pre-fix race), binary freshness checker wired into the Docker regression.

## CR32 follow-up audit (six findings, all verified and fixed)

A second audit of the remediation itself found six real gaps, closed the same
day:

1. **Cache gating** — engagement now requires provable reuse (duplicate
   slots or ≥2 pair-family slots per source), not merely two slots. Two
   distinct cheap metrics had regressed ~27%; now +5 ns (the plan scan),
   duplicate cheap metrics are 1.20× FASTER than baseline (memo), two-pair
   1.14×.
2. **Root prepared parity was vacuous** — `parse_root_xform_json` never
   called `rt_prepare_chain` (only the file wrapper did), so the probe and
   the micro compared fallback against fallback. With real preparation the
   16-transform non-finite chain FAILED bit parity: rotate and both pulls
   had duplicated inner loops whose separate compilation diverged in FMA
   contraction, flipping NaN sign bits (`7fc00000` vs `ffc00000`). Fixed by
   construction: ONE compiled inner loop per transform (fallback wrappers
   compute constants and delegate to the `_pre` body), and every parser
   output is now prepared (tuple entries zeroed — they previously inherited
   stack garbage in `prep_fn` for direct string-parser callers).
   **Documented seam**: production has emitted the `_pre` NaN bit pattern
   since CR31 shipped; NaN here is a sign-insensitive clip sentinel
   (`isnan()` checks; no consumer persists transformed roots), so the
   canonical bits are the prepared ones, not pre-CR31's. Genuinely prepared
   affine3: **37.8 vs 44.1 ns fallback (−14%)** — the earlier "flat" row
   was the vacuous comparison.
3. **Freshness is provenance, not just mtimes** — the manifest now records
   per-source sha256s and `--check` verifies them (touching a binary no
   longer defeats it); the binary table covers the FULL deploy set (24:
   musl + libcurl + libvips + LAPACK — assemble_greyscale and the libvips
   set were missing); `--check`, `--verify-manifest`, and the TSan gate now
   run inside `scripts/predeploy_check.sh`.
4. **Counters are complete** — the chunked probe pread is counted
   (`blocks + 1`), pinned by the counter tests.
5. **TSan gate is gated** — wired into predeploy (was a standalone script).
6. **Benchmark provenance** — the harness accepts `--meta key=value`
   provenance notes; the final reports are re-measured from a clean
   committed tree with binary/source hashes recorded, and micro results are
   retained in a committed report (see reports/).

## Residual work (ranked)
1. ~~Graviton re-measurement~~ — done twice: `cr-31-graviton.md` (pre-CR32,
   corrected in place) and the CR32 re-run recorded there.
2. `poly = fn(poly)` selector-path in-place transforms, behind a per-function alias
   audit (biggest remaining cost for the `neg16`-style shapes: ~150–600 ns/row).
   The chunked-t1 question is CLOSED (it was syscalls; see F2 correction).
3. Full Coeff ownership redesign (swap-as-metadata etc.) — only with a debug owner
   tracker and degree-128/256 benchmarks demonstrating the cost first.
4. Remaining Solve feature families (log-modulus, angular histogram, centroid/radii) —
   share only with accumulation-order proofs per metric AND requirement masks
   (the CR32 F2 lesson: reuse must be proven per program shape, not assumed).
5. Param superinstruction fusion + sanitize classification (skipped in P1: ~13 ns/row
   upside vs exact-behavior risk on the per-token sanitize boundary).
6. Real-Lambda memory-size matrix (F11): EC2 Graviton numbers are architecture
   evidence, not Lambda sizing proof — needs the final static binaries at
   representative memory allocations (user-run; deployment is user-only).
