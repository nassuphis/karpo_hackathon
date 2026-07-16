# Solver Brushes

Root-finding error is paint. The giga recreations kept proving it:
LAPACK's pairing order painted giga_2877, division dust painted
giga_outflow, zgeev's eigensolve error painted giga_2864 — where
Aberth-Ehrlich converges to the "true" roots and renders something
boring, an *imperfect* solver renders structure. This wave makes that
observation an instrument: the solver is a selectable brush, and each
brush has its own signature error field. Speed over accuracy, by
design.

## The four brushes

| solver_mode | binary mode | binary | µs/step¹ | error signature |
|---|---|---|---|---|
| `aberth_mt` | `solve_mt` | sweep_mt | 40 | none at full 64 iters; with `solver_iters` = 1..63 the roots are caught mid-flight between the seed circle and the truth — a morph knob, not an error |
| `companion_matrix` | `solve_cm` | sweep_cm | 362 | zgeev backward error ~machine-eps scaled by the companion matrix's condition number; blooms on ill-conditioned rows (giga_2864) |
| `jenkins_traub` | `solve_jt` | sweep_cm | 66 | deflation compounds error root-by-root in *found* order (smallest-magnitude first via the Cauchy-bound shifts), so late roots carry the accumulated dust of early ones — an error *gradient* across each row |
| `newton` | `solve_newton` | sweep_cm | 41 | the wild one: every root hunted from the FIXED seed 0.4+0.9i, so which root the seed finds is a fractal function of the coefficients (Newton basin boundaries), then forward deflation compounds. At degree 35 roughly half of random rows deviate visibly from np.roots — that's the brush |

¹ measured 2026-07-16, degree 35, 4000 random rows, single thread, M3
(Accelerate build). Ratios feed `_solve_us_per_step` in
`lambda/compute_fused.py` with a ~2× safety margin on the CM anchor.

Correctness where correctness is due (test-pinned, tests/test_solver_brushes.py):

- **JT is a real solver.** The CPOLY port agrees with np.roots to the
  f32 I/O floor on random degree-35 rows (measured worst 5.8e-8). Its
  "error" is subtler than Newton's — it lives in ill-conditioned rows
  and the deflation gradient, not in generic ones.
- **Newton is right when it should be.** Degree-5 well-conditioned
  rows: worst 5.7e-8. The degree-35 wildness is compounding deflation
  from a fixed seed, not a bug; the test asserts the texture *exists*
  (a "fix" that makes Newton accurate everywhere kills the brush).
- **Everything is deterministic.** Fixed seed, fixed shift angles,
  fixed nudges, no RNG, no time, no thread-order dependence: every
  brush is byte-identical across runs and thread counts. A brush that
  repaints differently per run is useless.

## Implementation

### C layer

- `lambda/jt_solver.h` — re-entrant port of Jenkins-Traub CPOLY
  (TOMS 419) from the R polyroot lineage at
  `pyroots/src/polyroot.cpp`. All globals/statics moved into a
  per-worker `JtState` (heap-allocated per JT worker thread).
  Documented departures from the reference: `relstp` is initialized
  (the original read an uninitialized static on some paths), and
  exactly-zero leading coefficients strip before solving, matching
  `solve_companion_coeffs`. Entry: `solve_jt_coeffs(...)`, same
  contract as the CM solver (f32 out, found order, negative return on
  2-pass shift failure → row renders as zeros).
- `lambda/newton_solver.h` — Newton-Raphson + forward-Horner synthetic
  deflation, deliberately naive. Fixed seed per root, 50-step cap,
  accept-as-is on non-convergence (texture, not error), deterministic
  +0.5 nudge off exact critical points, one restart on non-finite,
  final linear root by direct division. Non-finite rows skip like the
  other brushes (zeros + skipped_overflow); otherwise never fails.
- `lambda/sweep_cm.c` — one binary, three row loops: `CM_KIND_ZGEEV /
  CM_KIND_JT / CM_KIND_NEWTON` picked by the spec's `mode` string. All
  three share the threaded static row partition (CM threading wave):
  byte-identical at any `n_threads`, guard rows (all-zero, overflow
  skip, leading strip) handled identically.
- `lambda/sweep_mt.c` — optional `max_iter` spec field (1..64) caps the
  Aberth loop (`g_max_iter`); absent/0/out-of-range keeps the full 64.
  Meta echoes `max_iter`.

### Pipeline plumbing

- **Plan** (`handler_compute_plan.py`): `solver_mode` validates against
  the four names; `solver_iters` (0..64, 0 = default) validates and
  lands in `plan.solve.iters`; `plan.solve.bin_mode` carries the native
  mode string. JT/Newton route to the sweep_cm lambda
  (`_solver_function_name`). `calc.json` persists `solver` +
  `solver_iters` so lores recompute reproduces the brush.
- **ASL** (`stepfunctions/compute_workflow.asl.json.template`):
  `LoresSolveTask` payload gains `solve_mode.$: $.plan.solve.bin_mode`
  and `max_iter.$: $.plan.solve.iters`; `FusedChunkMap`'s ItemSelector
  gains `solver_iters.$: $.plan.solve.iters`.
- **Fused chunk** (`handler_compute_chunk_fused.py`): JT/Newton solve
  via the bundled sweep_cm binary with `n_threads = fused_threads`;
  aberth spec gains `max_iter` when `solver_iters` is set. Progress
  streaming stays aberth-only (sweep_mt is the only binary with flush
  watermarks).
- **Dedicated solver lambdas**: `handler_sweep_cm.py` accepts
  `solve_mode` (default `solve_cm`) + optional `n_threads`;
  `handler_sweep_mt.py` accepts optional `max_iter`.
- **Previews**: compute-preview accepts the four modes + `solver_iters`
  (tags: AE-MT / CM / JT / NEWT); lores recompute normalizes
  `calc.solver` (accepting `jt`/`solve_jt` etc.) and replays
  `calc.solver_iters`.

### UI

- Two new toolbar buttons: **Calculate-JT**, **Calculate-NEWT** —
  same generic solver popup (`openComputeSolverPopup`), per-solver
  prefs, distinct task/run scoping (`compute_run_<solver>_<runid>`).
- Compute-preview solver select gains JT / Newton; an adjacent number
  input (0–64) is the Aberth iteration cap, preview-signature-aware.
- The solver popup shows an "AE iteration cap" row for aberth only;
  the value flows to `params.solver_iters` and is restored when a
  saved calc is loaded back into the popup.

## Gates

- `tests/test_solver_brushes.py` — JT parity vs np.roots, guard rows,
  Newton easy-degree exactness + texture presence + determinism,
  thread byte-identity for both brushes, sweep_mt max_iter semantics
  (64/absent/invalid ≡ default; small cap differs; echo in meta).
- `tests/test_compute_chunk_fused.py::TestRunSolveLocalPayloads` — the
  exact dispatched spec for all four modes, incl. `max_iter` presence
  rules.
- `tests/test_compute_plan.py` / preview / lores handler tests — mode
  validation, bin_mode/iters plan fields, routing, 400s.
- `scripts/test-roots-cm-strip-docker.sh` — 1-vs-6-thread byte
  identity for all three sweep_cm modes on the deployed netlib LAPACK
  lineage (ARM64 AL2023).

## Deploy note

This wave touches both native binaries (sweep_cm, sweep_mt), the ASL
template, and the lambda handlers — a full deploy is required, and the
plan lambda + state machine must ship together (the new ItemSelector
JSONPaths resolve fields the new plan always emits; in-flight
executions pin their old definition and are unaffected).
