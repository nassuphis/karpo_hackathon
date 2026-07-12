# CR31 on Graviton: Benchmark Results

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

2. **Chunked t1 is flat on Linux too (−0.41%) — the syscall theory is dead on
   both platforms.** 131,072 → ~1,024 syscalls does not move single-worker
   time on macOS (+1.1%) or on Graviton (−0.4%); `chunked35_sin_t1` confirms
   it (−0.005%). The review's ≤132.8 ms t1 acceptance target was therefore
   unreachable via F2 anywhere; the remaining single-worker chunked cost is
   per-row program evaluation and chunk framing, not I/O. This folds the t1
   question into the post-mortem's residual item #2 (per-row eval cost).

3. **The param t2/t4 "+3–10% watch" resolves as an improvement.** On Graviton
   the static-range scheduler wins at every thread count: t2 **−8.1%**, t4
   **−5.9%**, t8 **−47.5%**. The M3 +3.8/+9.6% readings were host scheduling
   noise, as suspected.

4. **The 8-worker chunked *cliff* was macOS-only, but the blocked I/O still
   pays on Linux.** On the Graviton *base* build t8 was already the fastest
   count (55.8 ms < t4 108.0 ms) — the Linux kernel never serialized the
   syscall storm the way macOS did. F2 still buys **−15.3/−15.6/−17.5%** at
   t2/t4/t8. The param base, however, *did* plateau on Graviton (base t4
   5.68 ms ≈ base t8 5.81 ms — zero gain from 4→8 workers); F5 restores the
   scaling (cand 5.35 → 3.05 ms, 1.75× from t4→t8).

5. **The solve pair-metric bundle holds at 1.46×** (12,588.8 → 8,644.4 ns;
   M3: 1.61×). All other micro rows are flat as expected — the root-rotate
   prepared-chain win is structural and invisible to this micro by design
   (it times `rt_*` bodies, not the chain dispatch).

6. **Bonus determinism result: all 26 matrix outputs are byte-identical
   across platforms.** Every case's output SHA-256 on Graviton (gcc 11.5,
   glibc 2.34) equals the M3 value (Apple clang 17, macOS libm) — including
   the `sin`/expression cases. Both hosts are aarch64 and outputs are f32,
   but surviving two compilers, two libms, and two OSes bit-exactly is a
   strong property for the oracle suite: M3-harvested oracles are valid
   verbatim on the deployment architecture.

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

Lambda arm64 allocates vCPU roughly in proportion to memory. From the table:
every thread count ≥2 improves (chunked −11…−18%, param −6…−48%) and t1 is
flat (−0.4%), so the campaign helps at every memory size and regresses none.
The F5 CPU cap did not bind on this 8-vCPU box; its purpose (don't
oversubscribe when configured threads exceed allocated vCPU) applies at small
Lambda sizes and remains execution-plan-only. The param base's 4→8 plateau on
Graviton means pre-campaign binaries were wasting anything past ~4 vCPU on
param generation; post-campaign they are not — worth revisiting memory-size
choices for param/chunked-heavy stages if they were tuned around the old
plateau.

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

## Residual-work list, updated

1. ~~Graviton re-measurement~~ — **done (this document).** Watch items
   resolved: param t2/t4 regression was M3 noise; chunked t1 gap is not
   syscalls anywhere.
2. `poly = fn(poly)` selector-path in-place transforms (per-function alias
   audit) — now also the owner of the chunked-t1 question, since per-row eval
   is what's left there.
3. Full Coeff ownership redesign — unchanged (needs deg-128/256 evidence).
4. Remaining Solve feature families — unchanged.
5. Param superinstruction fusion — unchanged; note Graviton's ~3× slower VM
   loops make the ~13 ns/row upside proportionally bigger there, but the
   exactness risk is unchanged.
