# Solve Score Parameter Metrics Plan

Status: design note only.

This extends the current solve-score system to support metrics derived from the
persisted parameter stream, not just:

- solve roots (`slv`)
- coefficient vectors (`cf`)

It builds on:

- [solve-score-mixed-source.md](/Users/nicknassuphis/karpo_hackathon/polypaint/solve-score-mixed-source.md)
- [solve-score-extend.md](/Users/nicknassuphis/karpo_hackathon/polypaint/solve-score-extend.md)

## Objective

Expose parameter-derived scalar metrics such as:

- `t1_re`
- `t1_im`
- `t1_abs`
- `t1_phase`
- `t2_re`
- `t2_im`
- `t2_abs`
- `t2_phase`

so they can participate in the same solve-score RPN programs as existing solve
and coeff metrics, for example:

- `t1_abs(q=1%)`
- `spread(cf,q=2%)`
- `avg`

## Key Facts

### 1. The data already exists

The compute workflow already generates:

- `renders/<job>/lores_params.bin`
- `renders/<job>/params.bin`

The lores key is planned in
[lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py)
and written by the `param_gen` phase in
[lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py).

### 2. Row alignment is clean

`param_gen` writes one row per solve step.

Each row is:

- `t1.re`
- `t1.im`
- `t2.re`
- `t2.im`

as `4 x float32`, in the same solve order used by coeffgen and solve.

So:

- one lores params row lines up with one lores solve row
- one full params row lines up with one full solve row

This is the main reason the feature is feasible.

### 3. These values are not inherently in `[0,1]`

Important correction:

- the raw grid starts in `[0,1)`
- the persisted params do not

`param_gen` stores the transformed parameter pair after the param-transform
chain. That means values may be:

- signed
- complex
- unbounded
- angular / periodic

So these metrics still need the exact same per-slot clip and normalize flow as
every other solve-score metric.

There is no shortcut where param values can bypass clipping because they are
"already normalized."

## Recommendation

Do **not** add vague user-facing chips like:

- `param1`
- `param2`

Those names are too ambiguous.

The right first model is explicit scalar metrics:

- `t1_re`
- `t1_im`
- `t1_abs`
- `t1_phase`
- `t2_re`
- `t2_im`
- `t2_abs`
- `t2_phase`

This is better because:

- the meaning is obvious in the UI
- clipping semantics are metric-local
- no extra chip parameter is needed just to say "real vs imag vs abs vs phase"
- it matches the way users already think about the transformed parameters

Internally these can still be tagged as a third metric source family if useful,
but the user-facing metric identity should be explicit.

## Parameter Metric Semantics

### Real / imaginary parts

- `t1_re = Re(t1)`
- `t1_im = Im(t1)`
- `t2_re = Re(t2)`
- `t2_im = Im(t2)`

These are ordinary signed scalar metrics.

### Magnitude

- `t1_abs = |t1|`
- `t2_abs = |t2|`

These are non-negative scalar metrics.

### Phase

- `t1_phase = atan2(Im(t1), Re(t1))`, wrapped to `[0, 2π)`
- `t2_phase = atan2(Im(t2), Re(t2))`, wrapped to `[0, 2π)`

Phase needs one explicit caveat:

- it is periodic
- clipping in a linear scalar space introduces a seam at `0 / 2π`

That does not make the metric invalid, but it does mean:

- phase distributions near the seam can look artificially split

This is acceptable for a first implementation if it is documented honestly.

If this becomes visually annoying later, a better follow-up is not to remove
phase, but to add seam-free alternatives such as:

- `t1_unit_re = cos(arg(t1))`
- `t1_unit_im = sin(arg(t1))`
- same for `t2`

Those are not phase replacements for v1. They are later polish if needed.

## Canonical Model

User-facing chain:

- `t1_abs(q=1%)`
- `spread(cf,q=2%)`
- `avg`

Compiled structure:

- slot 0
  - family=`param`
  - metric=`t1_abs`
  - quantile=`0.01`
- slot 1
  - family=`coeff`
  - metric=`spread`
  - quantile=`0.02`
- program=`m0;m1;avg`

The RPN model does not need to change.

Only the slot metadata and evaluator dispatch need to grow.

## Why This Is Slightly Different From `cf`

Coefficient mixed-source support was relatively clean because:

- each solve chunk already had a matching coeff chunk object

Parameter metrics are slightly more awkward because:

- the full compute path stores one global `params.bin`
- not chunked `params_0000.bin`, `params_0001.bin`, ...

So full runtime support needs a way for chunk workers to read the correct slice
of the global params stream.

That is still feasible, but it requires explicit metadata plumbing.

## Required Plumbing

### 1. Persist `lores.params_key` in `calc.json`

Today `calc.json` keeps:

- `lores.bin_key`
- `lores.coeffs_key`

It should also persist:

- `lores.params_key`

For backward compatibility, histogram debug should also have a canonical
fallback:

- `renders/<job>/lores_params.bin`

### 2. Persist chunk step ranges in `calc.json`

Today chunk metadata in `calc.json` does not keep:

- `step_start`
- `step_count`

Parameter-runtime support needs those so a chunk worker can range-read the
matching slice from the global `params.bin`.

Without this, chunk workers cannot reliably map:

- chunk roots
- chunk coeffs
- chunk params

to the same solve rows.

### 3. Add params-key plumbing to render and palette workers

For full runtime support, the handlers need:

- root chunk input
- coeff chunk input, if coeff metrics are used
- param slice input, if parameter metrics are used

This likely means:

- pass `params_key`
- pass `step_start`
- pass `step_count`

into the solve-score runtime worker calls.

## Runtime Design

### Histogram debug

For lores histogram debug:

- download `lores.bin` if any solve metrics are used
- download `lores_coeffs.bin` if any coeff metrics are used
- download `lores_params.bin` if any param metrics are used
- compute raw per-slot metrics
- clip each slot independently
- normalize each slot independently to `[0,1]`
- evaluate the existing RPN program

This is the easiest first landing.

### Full render / palette runtime

For hires runtime:

- keep the current per-slot clip contract
- extend the runtime evaluator so each metric slot can read from:
  - roots
  - coeffs
  - params

For parameter slots specifically:

- range-read the relevant slice of `params.bin`
- decode rows as `t1.re, t1.im, t2.re, t2.im`
- compute scalar metric per solve row

## UI Recommendation

Keep the current solve-score chip model.

Do not add a new "parameter source" widget.

Instead:

- add the new metric names to the metric adder
- let users compose them exactly like any other metric

Examples:

- `t1_abs(q=1%)`
- `t2_phase(q=5%)`
- `t1_re(q=2%) t2_re(q=2%) abs_diff`
- `t1_abs(q=1%) spread(q=1%) max`

This is simpler than making the user choose:

- source=`param`
- subject=`t1`
- component=`abs`

for every chip.

## Phased Rollout

### Phase 1: lores histogram debug only

Implement:

- metric names:
  - `t1_re`, `t1_im`, `t1_abs`, `t1_phase`
  - `t2_re`, `t2_im`, `t2_abs`, `t2_phase`
- `calc.json` / fallback support for `lores_params.bin`
- summary-mode evaluation from `lores_params.bin`
- raw histogram and final bins for these metrics

Do not yet support hires render or palette runtime.

Why:

- fastest way to validate semantics
- easiest way to inspect whether the metrics are actually useful

### Phase 2: full render solve-score path

Implement:

- `params_key`, `step_start`, `step_count` plumbing into runtime workers
- parameter-slice loading for `roots2pix` / `roots2pix_mt`
- parameter metrics in actual color rendering

### Phase 3: palette and associated-palette paths

Implement the same support in:

- palette chunk generation
- associated palette generation

This should reuse the same source-aware metric dispatch, not fork again.

## Testing Plan

### Python / plan / metadata tests

Add tests for:

- metric registration in compiler / validators
- `calc.json` persistence of `lores.params_key`
- `calc.json` persistence of chunk `step_start` / `step_count`
- render/palette plan wiring of params metadata

### Native metric tests

Add ranking tests for:

- `t1_re`
  - right-shifted parameter rows > left-shifted
- `t1_im`
  - upper > lower
- `t1_abs`
  - larger radius > smaller radius
- `t1_phase`
  - deterministic wrapped-angle ordering case

Same for `t2_*` as needed, at least one representative set.

### Histogram / handler tests

Add summary tests proving:

- lores params are downloaded when param metrics are used
- per-slot clip metadata is produced
- mixed programs such as:
  - `t1_abs spread(cf) avg`
  - `t1_re t2_re abs_diff`
  work in summary mode

### Frontend tests

Update:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)

to verify:

- the new metrics appear in the chip adder
- dispatch payloads preserve the chosen metric names
- histogram debug can be requested with parameter metrics

### Runtime gate

Because this touches native solve-score evaluation:

- `bash scripts/test-docker-runtime.sh` remains mandatory

## Non-Goals For V1

Do not add:

- raw grid-coordinate metrics (`x1`, `x2`) unless explicitly requested
- arbitrary param-expression chips
- nested parameter selectors in the UI

Those can come later if needed.

## Recommendation

Proceed, but in this order:

1. histogram-debug-only parameter metrics
2. inspect whether they produce useful score structure
3. only then wire full hires runtime

And use explicit metric names:

- `t1_re`, `t1_im`, `t1_abs`, `t1_phase`
- `t2_re`, `t2_im`, `t2_abs`, `t2_phase`

not generic:

- `param1`
- `param2`

That is the cleanest model and the least confusing UI.
