# Solve Score Metrics Catalog Refactor

## Problem

Solve-score metric calculation is already centralized in native code:

- `lambda/solve_score.h` contains the actual formulas.
- Native binaries include that header and evaluate metric scores from the same implementation.

The drift risk is not formula duplication. The drift risk is the metric catalog:

- C has metric names, enum values, parser, serializer, min-root rules, source support, and CLI text.
- Python has metric validation and source rules in `lambda/solve_score_chain.py`.
- JS has the UI chip/dropdown list in `index.html`.
- Tests repeat expected metric names in a few places.

When adding metrics, every list has to be edited manually. That is how a metric can render in the UI but be rejected by the backend, or be accepted by Python but rejected by the native parser.

## Goal

Create one canonical metric registry and generate every non-formula catalog view from it.

The formulas stay in C. The names and contracts become generated.

## Canonical Registry

Add a tracked source file, for example:

```text
solve_score_metrics.json
```

Proposed shape:

```json
[
  {
    "name": "proximity",
    "enum": "SOLVE_METRIC_PROXIMITY",
    "min_roots": 2,
    "sources": ["slv", "cf"],
    "kind": "root_metric",
    "summary": "minimum pairwise root distance score"
  },
  {
    "name": "max_re",
    "enum": "SOLVE_METRIC_MAX_RE",
    "min_roots": 1,
    "sources": ["slv", "cf", "pm"],
    "kind": "root_or_param_metric",
    "summary": "maximum real component"
  },
  {
    "name": "t1_abs",
    "enum": "SOLVE_METRIC_T1_ABS",
    "min_roots": 1,
    "sources": ["pm"],
    "kind": "param_metric",
    "summary": "absolute value of first transform parameter"
  }
]
```

Required fields:

- `name`: public metric token used in score programs.
- `enum`: C enum symbol.
- `min_roots`: minimum finite roots required before the metric is meaningful.
- `sources`: supported score sources, using existing `slv`, `cf`, `pm`.
- `kind`: coarse UI/backend category.

Optional fields:

- `summary`: short human description.
- `introduced`: version label such as `v5`.
- `ui_group`: future grouping if the dropdown gets too large.

## Generated Outputs

Add a generator:

```text
scripts/generate_solve_score_metrics.py
```

It should support:

```bash
python3 scripts/generate_solve_score_metrics.py --write
python3 scripts/generate_solve_score_metrics.py --check
```

Generated files:

- `lambda/solve_score_metrics_generated.h`
- `lambda/solve_score_metrics.py`
- `solve_score_metrics_js.js`

### C Generated Header

`lambda/solve_score_metrics_generated.h` should contain:

- `enum SolveMetric`
- `SOLVE_SCORE_METRIC_LIST_TEXT`
- `parse_solve_metric(...)`
- `solve_metric_name(...)`
- `solve_metric_min_roots(...)`
- `solve_metric_supports_source(...)`

`lambda/solve_score.h` keeps:

- constants like `SOLVE_SCORE_EPS`
- helper math functions
- `compute_solve_metric_score(...)`
- score-program evaluator

The native formulas still switch on enum values, but the enum and metadata come from the generated header.

### Python Generated Module

`lambda/solve_score_metrics.py` should contain:

- `VALID_SOLVE_SCORE_METRICS`
- metric source map
- param metric set
- param-capable metric set if still needed
- small helpers such as `metric_allowed_sources(metric)`

`lambda/solve_score_chain.py` should import this module instead of declaring its own metric sets.

### JS Generated Catalog

`solve_score_metrics_js.js` should expose a browser global, for example:

```js
window._solveScoreMetrics = [
  { name: "proximity", sources: ["slv", "cf"], kind: "root_metric", min_roots: 2 }
];
```

`index.html` should use that array to build:

- `_solveScoreMetricNames`
- source choices
- param metric set

This removes the hand-maintained metric list in `index.html`.

## Deployment And Packaging

Because deployed Python handlers import `solve_score_chain.py`, any new generated Python module must be copied into every Lambda zip that bundles code using it.

Affected packaging blocks likely include:

- solve proximity
- palette render plan
- palette chunk
- palette finalize
- raster MT
- render plan
- render lores preview
- storage/finalize/recolor paths that read solve-score metadata

This must be covered by `tests/test_deploy_packaging.py`.

Frontend deploy must upload `solve_score_metrics_js.js` if it is loaded as a separate asset. If we want fewer frontend assets, the generator can instead write a script snippet that is inlined into `index.html`, but a separate asset is cleaner.

If it is separate, update:

- `deploy.sh` frontend asset list
- `api_manifest.json` only if service/config keys change, which this refactor should not require
- frontend stale-client behavior if `index.html` depends on the new asset

## Tests

Add generator tests:

- `--check` fails when generated files are stale.
- duplicate metric names are rejected.
- duplicate enum symbols are rejected.
- invalid source names are rejected.
- param metrics cannot accidentally support `slv` / `cf` unless explicitly allowed.

Add contract tests:

- C parser accepts every registry metric.
- C `solve_metric_name(parse(name)) == name` for every metric.
- Python accepts every registry metric.
- JS catalog exposes exactly the registry metric names in registry order.
- Native Docker runtime includes at least one metric from each source/category.

Update checklist gates:

- Add `python3 scripts/generate_solve_score_metrics.py --check` to `scripts/predeploy_check.sh`.
- Keep `bash scripts/test-docker-runtime.sh` as the hard gate when formulas or native binaries change.

## Migration Plan

1. Add `solve_score_metrics.json` matching the current metric list.
2. Add generator and generated files.
3. Update C to include `solve_score_metrics_generated.h`.
4. Update Python to import `solve_score_metrics.py`.
5. Update frontend to load/use `solve_score_metrics_js.js`.
6. Update deploy packaging and frontend asset upload.
7. Add stale-generated-file checks to predeploy.
8. Remove old hand-maintained metric lists.

Do this as a refactor-only change. Do not add new metrics in the same commit. The point is to prove the generated catalog preserves current behavior exactly.

## Non-Goals

- Do not move metric formulas out of C.
- Do not implement a Python or JS score evaluator.
- Do not change score-program semantics.
- Do not change existing metric names or source support.

## Open Questions

- Should `solve_score_metrics_js.js` be a separate deployed asset or generated inline into `index.html`?
- Should enum integer values remain stable forever, or can they be generated in registry order?
- Should the registry include UI descriptions/tooltips now, or keep it contract-only until the dropdown needs richer grouping?
