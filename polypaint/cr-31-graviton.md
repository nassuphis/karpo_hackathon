# CR31 on Graviton: Benchmark Results

> **CORRECTED per code-review-32 (2026-07-12).** Three conclusions in this
> report were wrong and are corrected inline below:
> **(1)** the "syscall theory is dead" t1 verdict — candidate t1 never
> executed blocked I/O (the `threadsUsed <= 1` branch kept per-row calls), so
> both t1 rows compared per-row against per-row; with CR32's unified engine,
> t1 improves −34.8% on M3 (Graviton re-run below);
> **(2)** "the M3 t2/t4 readings were host scheduling noise" — they were a
> repeatable +15–20% platform-specific regression of the one-pwrite-per-row
> scheduler, fixed by CR32 write batching (M3 t4 now −24.2%);
> **(3)** "the campaign helps at every memory size and regresses none" — an
> overclaim: the coeff fast kernels had changed non-finite output semantics
> (CR32-F1, CRITICAL) and one-slot solve-score programs had regressed 5–7× at
> the production entry (CR32-F2), neither visible to this matrix; both fixed.
> Also scoped honestly: the flat "micro" solve rows below measure the raw
> metric helper, NOT the changed production entry; EC2 Graviton results are
> architecture evidence, not Lambda sizing proof (Lambda matrix remains
> user-run future work). The macro A/B numbers themselves remain valid for
> the finite fixtures measured.

Date: 2026-07-12. This closes the #1 residual item from `cr-31-post-mortem.md`:
re-measure the CR31 VM campaign on the deployment architecture instead of the
M3 development host, and settle the three questions the M3 numbers left open
(the flat chunked t1, the param t2/t4 "+3–10% watch", and whether the 8-worker
wins were a macOS artifact).

## Setup

| | Graviton box | M3 reference |
|---|---|---|
| Hardware | EC2 **c7g.2xlarge** — Graviton3, 8 vCPU (physical cores, no SMT), us-east-1 | Apple M3 Max, arm64 |
| OS | Amazon Linux 2023, kernel 6.1.176, glibc 2.34 | macOS |
| Compiler | gcc 11.5.0, `-O3 -pthread` | Apple clang 17, `-O3 -pthread` |
| Builds | base = `32e01ff` (pre-campaign), cand = `9056919` (campaign HEAD) | same pair |
| Tool | `scripts/bench_program_vms.py` + `tests/native/vm_microbench.c`, unchanged | same |

Methodology identical to the post-mortem: medians (11 reps; 15 for the
harvested mqlacwaq group), interleaved base/candidate A/B, **SHA-256 of the
complete output file enforced per case — every A/B below reported `bytes ok`**.
The box was a throwaway instance (temp key pair + SSH-only security group),
created for this run and terminated after it; nothing touched S3, Lambda, or
the app. Reports: `reports/vm_bench_graviton_after-cr31.json` and
`reports/vm_bench_graviton_ab_{mqlacwaq,chunked,param}.json`.

## Headline verdicts on the open questions

1. **The harvested-program win roughly doubles on Graviton.**
   `compute_mqlacwaq` Coeff Program: **46.49 → 31.88 ms = −31.4%** (M3:
   −16.5%); with the Param stage in front, −28.5% (M3: −17.1%). The
   no-program control is flat (−0.8%), so the entire delta is the VM work
   (F1 arg-mode plan + F7 hoisted kernels + F3-scoped in-place). Graviton3
   runs the complex-arithmetic eval loops ~3× slower per core than M3, so the
   per-row fixed costs the campaign removed are a much larger fraction of
   wall time — **in production the campaign is worth about twice what the M3
   numbers suggested.**

2. ~~**Chunked t1 is flat on Linux too (−0.41%) — the syscall theory is dead
   on both platforms.**~~ **CR32 correction (F3): candidate t1 never executed
   blocked I/O — the single-worker branch kept the old per-row pread/pwrite
   loop, so this A/B compared two per-row implementations and said nothing
   about syscalls. The claimed "131,072 → ~1,024" transition never occurred
   at t1. After CR32 unified t1 into the block engine: M3 t1 −34.8%
   (153.1 → 99.8 ms, bytes identical) — syscalls were a major single-worker
   cost after all, and the review's ≤132.8 ms target is beaten. Graviton
   re-run below.**

3. **The param t2/t4 "+3–10% watch" resolves as an improvement on Graviton**
   (t2 −8.1%, t4 −5.9%, t8 −47.5%). ~~The M3 +3.8/+9.6% readings were host
   scheduling noise, as suspected.~~ **CR32 correction (F6): not noise — a
   repeatable, platform-specific M3 regression (interleaved re-measurement:
   +15–20% at t4, several× the observed MAD) caused by one pwrite per row.
   The fused-plan default is four workers, so this was the default dev-host
   path. CR32 batches rows into 32-row block flushes (128-row blocks lost
   write/compute overlap on Linux): M3 t2 −18.9%, t4 −9.1%, t8 −91.1%;
   Graviton t2 −10.1%, t4 −5.6%, t8 −45.9% — improvements on BOTH platforms,
   with the opposite-sign history retained in the record rather than averaged
   away.**

4. **The 8-worker chunked *cliff* was macOS-only, but the blocked I/O still
   pays on Linux.** On the Graviton *base* build t8 was already the fastest
   count (55.8 ms < t4 108.0 ms) — the Linux kernel never serialized the
   syscall storm the way macOS did. F2 still buys **−15.3/−15.6/−17.5%** at
   t2/t4/t8. The param base, however, *did* plateau on Graviton (base t4
   5.68 ms ≈ base t8 5.81 ms — zero gain from 4→8 workers); F5 restores the
   scaling (cand 5.35 → 3.05 ms, 1.75× from t4→t8).

5. **The solve pair-metric bundle holds at 1.46×** (12,588.8 → 8,644.4 ns;
   M3: 1.61×). ~~All other micro rows are flat as expected.~~ **CR32
   correction (F8): the ordinary metric rows call the raw helper directly and
   bypass `solve_score_eval_metric_slots`, so they could not see the cache —
   which is exactly where a 5–7× one-slot regression was hiding (CR32-F2,
   fixed). The micro now has production-entry one-slot/duplicate-slot cases
   and a parsed prepared root-chain case, so flat rows mean flat production
   paths.** The root-rotate note stands: the old rotate row times `rt_*`
   bodies, not chain dispatch.

6. **Bonus determinism result: all 26 matrix outputs are byte-identical
   across platforms.** Every case's output SHA-256 on Graviton (gcc 11.5,
   glibc 2.34) equals the M3 value (Apple clang 17, macOS libm) — including
   the `sin`/expression cases. Both hosts are aarch64 and outputs are f32,
   but surviving two compilers, two libms, and two OSes bit-exactly is a
   strong property. **CR32 scoping: this validates those 26 finite workloads
   across the two arm64 hosts — it is evidence for, not proof of, general
   oracle portability (non-finite policy boundaries were outside this matrix,
   and CR32-F1 proved such boundaries can diverge silently).**

## Interleaved A/B (Graviton, bytes ok on every row)

| Case | Base | Cand | Graviton Δ | M3 Δ (post-mortem) |
|---|---:|---:|---:|---:|
| mqlacwaq_coeff (15 reps) | 46.49 ms | 31.88 ms | **−31.4%** | −16.5% |
| mqlacwaq_param_coeff (15 reps) | 49.22 ms | 35.18 ms | **−28.5%** | −17.1% |
| mqlacwaq_baseline (control, 15 reps) | 2.18 ms | 2.17 ms | −0.8% | +2.3% |
| chunked35_t1 | 402.39 ms | 400.76 ms | −0.4% | +1.1% |
| chunked35_t2 | 213.21 ms | 180.70 ms | **−15.3%** | −54.6% |
| chunked35_t4 | 107.97 ms | 91.10 ms | **−15.6%** | −62.2% |
| chunked35_t8 | 55.84 ms | 46.04 ms | **−17.5%** | −84.7% |
| chunked35_sin_t1 | 557.87 ms | 557.84 ms | −0.0% | +0.1% |
| chunked35_sin_t4 | 146.63 ms | 129.79 ms | **−11.5%** | −53.6% |
| param_expr_t2 | 11.23 ms | 10.32 ms | **−8.1%** | +3.8% (noise) |
| param_expr_t4 | 5.68 ms | 5.35 ms | **−5.9%** | +9.6% (noise) |
| param_expr_t8 | 5.81 ms | 3.05 ms | **−47.5%** | −80.3% |

MADs were tiny throughout (mostly ≤0.3% of the median; worst case
chunked35_t2 base at 717 µs on 213 ms). The M3 chunked deltas are larger only
because the macOS *base* was pathological at multi-worker; the Graviton
candidate absolutes below show the fixed code scales near-linearly on both.

## Candidate absolutes and scaling (Graviton, 11 reps)

Chunked deg-35, 65,536 rows: t1 401.4 → t2 180.8 → t4 91.1 → t8 **45.9 ms** —
8.7× from t1 to t8 on 8 physical cores (t1 runs the separate serial path,
which is slightly slower per row than a worker, hence nominally superlinear).
Param expr 262k rows: t2 10.32 → t4 5.35 → t8 **2.99 ms**. The same-workload
output SHA is identical across t2/t4/t8 (row-seeded dither), matching the
post-mortem's seam note: only serial-vs-threaded differs, and that seam
predates the campaign.

## Micro benchmark (ns/op, deg 35)

| Case | Base | Cand | Δ |
|---|---:|---:|---:|
| metric_bundle_pair4 | 12,588.8 | 8,644.4 | **−31.3% (1.46×)** |
| solve_vm_long_31tok | 103.4 | 103.4 | flat |
| solve_vm_transcendental_7tok | 82.0 | 82.0 | flat |
| metric_proximity | 768.6 | 768.2 | flat |
| metric_clusteriness | 2,313.9 | 2,333.7 | +0.9% (noise) |
| metric_min_angular | 1,073.6 | 1,057.5 | −1.5% |
| root_rotate | 43.9 | 43.5 | flat (prepared chain not visible here by design) |
| root_copy_only | 10.8 | 9.8 | −9.7% |

## Platform character (candidate builds, G3/M3 wall-time ratio)

- **Complex-arithmetic VM loops: Graviton ≈ 2.6–3.5× slower per core** —
  coeff35 cases 2.7–3.4×, chunked 2.6–3.5×, harvested mqlacwaq 1.63×.
- **Orchestration/I/O-bound cases: Graviton at parity or faster** —
  mqlacwaq_baseline 0.47×, coeff4_baseline 0.75×, param_baseline 0.84×,
  param_expr_t8 0.61× (Linux beats macOS at thread wake/writeback).
- Consequence: on the deployment architecture the program-VM share of wall
  time is *larger*, which is exactly why the mqlacwaq win doubled. M3 remains
  fine for correctness and for detecting big effects, but production sizing
  decisions should use these Graviton numbers.

## Lambda sizing implications

Lambda arm64 allocates vCPU roughly in proportion to memory.
~~From the table: every thread count ≥2 improves and t1 is flat, so the
campaign helps at every memory size and regresses none.~~ **CR32 correction
(F11): that sentence overclaimed twice. First, this table is EC2 Graviton
evidence — no Lambda memory size was measured (musl static binaries, cgroup
CPU visibility, cold starts, and noisy neighbors all differ); a real Lambda
matrix with the final binaries remains future work and is user-run. Second,
"regresses none" was false outside this matrix: CR32 found changed non-finite
output semantics (F1) and a 5–7× one-slot solve regression (F2), both since
fixed.** The F5 CPU cap did not bind on this 8-vCPU box; after CR32 it also
cannot flip a request's dither seed policy (the cap never crosses the
serial/threaded boundary — pinned by tests/test_param_seed_policy.py). The
param base's 4→8 plateau on Graviton means pre-campaign binaries were wasting
anything past ~4 vCPU on param generation; post-campaign they are not.

## Reproducing on a fresh box

The harness needs three repo files beside the sources (`tests/bench_vm.py`,
`tests/oracle_runner.py`, `tests/native_program_helpers.py`), the harvested
fixture `tests/fixtures/program-m3-oracle/harvested/compute_mqlacwaq.calc.json`,
and `PYTHONPATH` set to the tree root (oracle_runner does a package-qualified
`tests.…` import). Then:

```
python3 scripts/bench_program_vms.py --binary <cand> --reps 11 --no-micro --out after.json
python3 scripts/bench_program_vms.py --compare <base> <cand> --cases "mqlacwaq_*" --reps 15
python3 scripts/bench_program_vms.py --compare <base> <cand> --cases "chunked*"  --reps 11
python3 scripts/bench_program_vms.py --compare <base> <cand> --cases "param_expr_t*" --reps 11
cc -O3 -I lambda tests/native/vm_microbench.c -lm -o micro && ./micro
```

## CR32 re-run on Graviton (fixed code, 2026-07-12)

Same methodology and instance class as above (fresh throwaway c7g.2xlarge,
gcc 11.5 `-O3`, AL2023; base = `32e01ff`, cand = CR32 remediation HEAD;
interleaved A/B, **bytes ok on every row**; binary SHA-256s pinned in the
reports — `reports/vm_bench_graviton_cr32_ab_{mqlacwaq,chunked,param}.json`).

| Case | Base | CR32 | Δ | Pre-CR32 Δ |
|---|---:|---:|---:|---:|
| mqlacwaq_coeff (15 reps) | 46.38 ms | 32.17 ms | **−30.6%** | −31.4% |
| mqlacwaq_param_coeff (15 reps) | 49.47 ms | 35.40 ms | **−28.5%** | −28.5% |
| mqlacwaq_baseline (control) | 2.23 ms | 2.28 ms | +2.2% | −0.8% |
| chunked35_t1 | 400.00 ms | 353.39 ms | **−11.7%** | −0.4% (never ran blocked I/O) |
| chunked35_sin_t1 | 555.90 ms | 509.32 ms | **−8.4%** | −0.0% (same) |
| chunked35_t2 | 214.03 ms | 179.38 ms | **−16.2%** | −15.3% |
| chunked35_t4 | 108.85 ms | 90.39 ms | **−17.0%** | −15.6% |
| chunked35_t8 | 56.42 ms | 45.64 ms | **−19.1%** | −17.5% |
| param_expr_t2 | 11.34 ms | 10.20 ms | **−10.1%** | −8.1% |
| param_expr_t4 | 5.74 ms | 5.42 ms | **−5.6%** | −5.9% |
| param_expr_t8 | 5.89 ms | 3.19 ms | **−45.9%** | −47.5% |

Micro (production entry, base → CR32): pair bundle 12,727.1 → 8,567.4 ns
(**1.49×**), duplicate-slot proximity 1,550.0 → 1,050.3 ns (**1.48×**),
one-slot max_re 77.7 → 79.2 ns and one-slot proximity 776.4 → 778.3 ns
(both ≈ flat — the 5–7× pre-CR32 one-slot regression is gone; a one-slot
program now returns through the baseline path before the plan scan).

What changed vs the corrected verdicts above:

- **t1 is now genuinely tested and genuinely faster on Linux** (−11.7%
  plain, −8.4% transcendental) — smaller than macOS's −34.8% because Linux
  handled the per-row syscalls better to begin with, but real. The original
  "flat t1" rows compared per-row against per-row.
- **Param write batching was tuned on both platforms**: 128-row blocks
  regressed Graviton t4 by +2.1% (one large end-of-range flush lost
  write/compute overlap); 32-row blocks win on both platforms at every
  count. The pwrite count is still 32× below per-row.
- The F1 clamp (typed-kernel non-finite policy) costs nothing measurable in
  any macro case on either platform; mqlacwaq retains its −30% Graviton win.

## Residual-work list, updated (CR32)

1. ~~Graviton re-measurement~~ — done (this document + the CR32 re-run below).
   ~~Watch items resolved: param t2/t4 regression was M3 noise; chunked t1
   gap is not syscalls anywhere.~~ **Both original resolutions were wrong;
   see the corrections above. Now actually resolved: t1 WAS syscalls (fixed,
   −34.8% M3), t4 WAS a real M3 regression (fixed, −24.2% M3).**
2. `poly = fn(poly)` selector-path in-place transforms (per-function alias
   audit) — the chunked-t1 question no longer belongs here (closed by CR32).
3. Full Coeff ownership redesign — unchanged (needs deg-128/256 evidence).
4. Remaining Solve feature families — only with requirement masks and
   accumulation-order proofs (the CR32 F2 lesson).
5. Param superinstruction fusion — unchanged; note Graviton's ~3× slower VM
   loops make the ~13 ns/row upside proportionally bigger there, but the
   exactness risk is unchanged.
6. Real-Lambda memory/concurrency matrix with the final static binaries
   (user-run; see corrected Lambda section above).
