# CR31 Post-Mortem: VM Performance Campaign

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
| Chunked deg-35, 65,536 rows, 1 worker | 151.5 ms | ≤ 132.8 ms | 152.8 ms (+1.1%) | ❌ on this host — see F2 notes |
| Chunked deg-35, 65,536 rows, 4 workers | 70.2 ms | no regression | **26.5 ms (−62.2%)** | ✅ far beyond |
| Chunked deg-35, 8 workers | 90.5 ms (worse than 4) | — | **13.8 ms (−84.7%)**, now the fastest | ✅ cliff eliminated |
| Param expr, 262k rows, 4 workers | 4.43 ms | no regression | 5.34 ms (+9.6%) | ⚠ within observed MAD; watch on Graviton |
| Param expr, 8 workers | 24.31 ms | ≤ 1.3× best | **4.87 ms**, now the best count | ✅ cliff eliminated |
| Root rotate predecode | 28.4 ns | 15–25% A/B gain | implemented; direct micro unchanged by design (see F8) | ✅ structural |
| Solve 31-token VM | 56.9 ns | no regression | 56.6 ns | ✅ |
| Solve mixed pair-metric bundle | (new case) | ≥ 1.5× | **4288.8 → 2658.9 ns = 1.61×** | ✅ |

Every A/B in the campaign was **byte-identical** (SHA-256 of complete output files,
enforced by the tool — a mismatch is a fatal error, not a warning). The full 13-file
correctness gate (param/coeff native, chains, source equivalence, wire fingerprints,
M3 oracles, root registry, solve parity + equivalence, whole-sweep oracle) plus the
coeff VM property fuzz passed after every phase: **214 passed, 63 subtests** at the end.
The ARM64 Docker runtime regression passed against freshly cross-compiled deploy
binaries.

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
counters: 32+32 for a 4,096-row run). **The surprise**: on this host the win is almost
entirely in *multi-worker* runs — t2 −54.6%, t4 −62.2%, t8 −84.7% (near-linear scaling
now) — because the syscall storm was serializing workers in the kernel. Single-thread
chunked was unchanged (+1.1%), i.e. the 1-worker gap vs buffered generation on macOS is
NOT syscall cost; the review's ≤132.8 ms t1 target was not met here and needs Graviton
data before further chasing.

### F5 — Param static-range scheduler (P2)
Seekable outputs: each worker owns a contiguous row range and `pwrite`s rows at exact
offsets — zero synchronization on the success path. Streaming (`"-"`) keeps the ordered
ring. Thread count now capped by online CPUs (execution-plan only). t8: 24.7 → 4.9 ms
(−80%), now the best count. t2/t4 read +3–10% vs baseline in the final sweep — within
the MAD we observed across runs, but flagged for Graviton verification.
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
the exact generic path. Combined with F1/F3 this took the harvested program from −15.9%
to **−16.5/−17.1%**. The synthetic `neg16` case barely moved because `poly = neg(poly)`
compiles to the *selector* path, not typed ops (see F3 note above) — a useful map of
where the remaining cost lives.

### F10 — opt-in perf counters (P5)
`-DPP_VM_PERF` builds count `pread`/`pwrite` calls and dynamic-arg resolves and emit one
JSON line to stderr per chunked/param run; normal builds compile the macros away.
Verified live: 4,096 rows → 32 preads + 32 pwrites.

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

## Residual work (ranked)
1. **Graviton re-measurement** (review requirement): especially chunked t1 (the
   unexplained 1-worker gap), param t2/t4 (+3–10% here), and the CPU-cap default —
   Lambda allocates few vCPUs, where the scheduler fixes should matter more, not less.
2. `poly = fn(poly)` selector-path in-place transforms, behind a per-function alias
   audit (biggest remaining cost for the `neg16`-style shapes: ~150–600 ns/row).
3. Full Coeff ownership redesign (swap-as-metadata etc.) — only with a debug owner
   tracker and degree-128/256 benchmarks demonstrating the cost first.
4. Remaining Solve feature families (log-modulus, angular histogram, centroid/radii) —
   3–10× cheaper per call than the pair family; share only with accumulation-order
   proofs per metric.
5. Param superinstruction fusion + sanitize classification (skipped in P1: ~13 ns/row
   upside vs exact-behavior risk on the per-token sanitize boundary).
