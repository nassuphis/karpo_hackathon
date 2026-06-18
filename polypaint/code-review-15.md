# Code Review 15

Reviewed current `main` at `ba5b67d` (`Fix CR14 pipeline errors and diagnostics`). Focus areas: the new shared pipeline helper, coefficient source compiler/error flow, compute preview/debug behavior, Step Functions workflow propagation, compute status error tracking, and deploy packaging.

## Verdict

No deployment blocker found. The CR14 follow-up is directionally correct: pipeline mode/source precedence is centralized, preview request validation now returns 400s, status rows preserve failed-phase context, and the new helper is packaged into the affected Lambda bundles.

The remaining issues are concentrated in error tracking, not hot-loop execution or VM correctness. The code now preserves parser line/column diagnostics for normal preview, but not for every user-facing path. Compute debug and chain-validation failures still flatten or lose useful source context.

## Findings

[concern] Compute debug still drops structured coefficient-source diagnostics.

Normal compute preview catches `CoeffSourceCompileError` explicitly and returns the diagnostic list:

- `lambda/handler_compute_preview.py:559`
- `lambda/handler_compute_preview.py:561`
- `lambda/handler_compute_preview.py:563`

The debug path does not. `handler()` routes debug requests through `_handle_compute_debug()` first, then catches every `ValueError` as a plain message:

- `lambda/handler_compute_preview.py:537`
- `lambda/handler_compute_preview.py:539`
- `lambda/handler_compute_preview.py:540`
- `lambda/handler_compute_preview.py:541`

Because `CoeffSourceCompileError` subclasses `ValueError`, a bad text program in the Compute Debug tab returns only:

```json
{"message": "coeff program source line ..., column ...: ..."}
```

It does not return:

```json
{"diagnostics": [{"line": ..., "column": ..., "message": ...}]}
```

That is exactly the editor/debug surface where structured diagnostics matter. Fix is small: catch `CoeffSourceCompileError` before `ValueError` in the debug branch and return the same `{message, diagnostics}` envelope as normal preview. Add a test with `debug_stage=poly` and invalid `coeff_program_source_text`.

[concern] Source programs that parse successfully but fail chain validation still lose source location.

`pipeline_programs.parse_coeff_source_for_run()` preserves parser/lowering diagnostics by calling `parse_coeff_program_source(..., strict=False)`:

- `lambda/pipeline_programs.py:70`
- `lambda/pipeline_programs.py:76`
- `lambda/pipeline_programs.py:78`

But after parsing succeeds, the handlers compile the lowered chip chain separately. If stack validation fails, the user sees a token/chain-level error, not a source-line error:

- `lambda/handler_compute_preview.py:329`
- `lambda/handler_compute_preview.py:333`
- `lambda/handler_compute_preview.py:337`
- `lambda/coeff_program_chain.py:1663`
- `lambda/coeff_program_chain.py:1834`

Concrete example: source text `cf` parses to `[['push', 'cf']]`, then chain compilation fails with:

```text
coeff program final stack depth is 1; expected 0
```

That message has no source line, source statement, or hint that the user probably meant `emit` / `poly = cf`. This is not a runtime bug, but it is still incomplete error tracking for the text compiler.

Recommendation: have `parse_coeff_program_source()` return a lowered-token/source-statement map, or have the source compiler call `compile_coeff_program_chain(strict=False)` and translate chain diagnostics back to source statement ranges before returning. At minimum, wrap source-originated chain validation errors as `invalid coeff_program_source_text` rather than `invalid coeff_program_chain`.

[concern] Failed-phase lookup should use a consistent DynamoDB read.

`handler_compute_status` now reads the previous top-level row before overwriting it with the final error row:

- `lambda/handler_compute_status.py:98`
- `lambda/handler_compute_status.py:134`
- `lambda/handler_compute_status.py:138`
- `lambda/handler_compute_status.py:157`

That is the right design because the previous row carries `phase`, `phase_label`, `expected`, and `subtask_prefix`. However, the `get_item` call does not request a strongly consistent read. The workflow writes a phase row, then a worker can fail immediately, then `ReportError` reads the just-written row. With DynamoDB's default eventually consistent read, the status handler can occasionally miss the phase row and write an error without `failed_phase`.

Fix: add `ConsistentRead=True` to `_get_existing_result_data()`. Add a test assertion that `get_item` is called with `ConsistentRead=True`.

[concern] Fused degree-probe source failures cannot preserve the diagnostic list.

For fused compute, the workflow runs degree probe before build plan:

- `stepfunctions/compute_workflow.asl.json.template:34`
- `stepfunctions/compute_workflow.asl.json.template:39`
- `stepfunctions/compute_workflow.asl.json.template:95`
- `stepfunctions/compute_workflow.asl.json.template:123`

Degree probe compiles `coeff_program_source_text` inside `handler_coeffgen`:

- `lambda/handler_coeffgen.py:596`
- `lambda/handler_coeffgen.py:597`
- `lambda/handler_coeffgen.py:607`
- `lambda/handler_coeffgen.py:609`
- `lambda/handler_coeffgen.py:132`
- `lambda/handler_coeffgen.py:134`

If parsing fails there, `CoeffSourceCompileError` has `.diagnostics`, but Lambda/Step Functions exception serialization preserves only message-ish fields. The final status row gets a useful one-line message and `failed_phase=degree_probe`, but the structured diagnostics list is lost.

This is acceptable for now because the message includes first line/column, but it means Calculate-AE-MT still has weaker compiler diagnostics than the `/compile-coeff-program-source` and normal preview routes. If we want the pipeline to carry full diagnostics, degree probe needs an explicit structured failure path rather than relying on Lambda exception serialization.

[nit] Explicit packaging assertions were not updated for `pipeline_programs.py`.

`deploy.sh` packages the new helper correctly:

- `deploy.sh:965`
- `deploy.sh:1035`
- `deploy.sh:1325`

The generic dependency parser in `tests/test_deploy_packaging.py:212` should catch a missing local import, so this is not a functional gap. Still, the explicit high-value assertions around compute/coeffgen packages do not name `pipeline_programs.py`:

- `tests/test_deploy_packaging.py:438`
- `tests/test_deploy_packaging.py:446`
- `tests/test_deploy_packaging.py:512`

Worth adding direct `assertIn("pipeline_programs.py", packaged[...])` checks for `handler_coeffgen.py`, `handler_compute_preview.py`, and `handler_compute_plan.py` so future readers see the dependency intentionally.

## Confirmed Good

The shared helper captures the intended precedence rules. `pipeline_mode_from_params()` preserves Chain/Program aliases and inference, and `coeff_source_text_for_run()` keeps the subtle blank-source-text rule: non-blank source wins, blank source falls back to chain when a chain exists.

- `lambda/pipeline_programs.py:35`
- `lambda/pipeline_programs.py:52`
- `lambda/pipeline_programs.py:64`
- `lambda/pipeline_programs.py:65`

The handlers now use that helper instead of maintaining independent copies:

- `lambda/handler_compute_preview.py:290`
- `lambda/handler_compute_preview.py:294`
- `lambda/handler_compute_plan.py:156`
- `lambda/handler_compute_plan.py:230`
- `lambda/handler_coeffgen.py:444`
- `lambda/handler_coeffgen.py:541`
- `lambda/handler_coeffgen.py:597`

Preview client-input errors now return HTTP 400 instead of 500, with tests updated to pin that contract:

- `lambda/handler_compute_preview.py:547`
- `lambda/handler_compute_preview.py:553`
- `tests/test_compute_preview_handler.py:492`
- `tests/test_compute_preview_handler.py:499`
- `tests/test_compute_preview_handler.py:504`
- `tests/test_compute_preview_handler.py:509`

Status error rows now preserve the last reported phase context. The implementation reads the previous row, attaches `failed_phase`, `failed_phase_label`, `failed_subtask_prefix`, and `failed_error`, and tests cover both present and absent previous-row cases:

- `lambda/handler_compute_status.py:98`
- `lambda/handler_compute_status.py:151`
- `lambda/handler_compute_status.py:157`
- `tests/test_compute_status_handler.py:13`
- `tests/test_compute_status_handler.py:57`

The Step Functions template still forwards `Error` and `Cause` into `ReportError`, and a test now pins that:

- `stepfunctions/compute_workflow.asl.json.template:715`
- `stepfunctions/compute_workflow.asl.json.template:733`
- `stepfunctions/compute_workflow.asl.json.template:734`
- `tests/test_compute_workflow_definition.py:85`

IAM/deploy is not obviously broken by the status `GetItem` addition. The deployed role policy already includes `dynamodb:GetItem`:

- `deploy.sh:1745`
- `deploy.sh:1871`

The split-JS header comment was cleaned up and now describes build-versioned asset keys rather than the stale `?v=<BUILD_ID>` query-param model:

- `js/01-core-compute.js:1`
- `js/02-preview-solvescore.js:1`
- `js/12-deepzoom-boot.js:1`

## Verification Run

Commands run:

```text
python3 api_manifest.py --check
bash tests/test_frontend_js.sh
python3 -m py_compile lambda/pipeline_programs.py lambda/handler_compute_status.py lambda/handler_compute_preview.py lambda/handler_compute_plan.py lambda/handler_coeffgen.py
uv run python -m pytest tests/test_compute_preview_handler.py tests/test_compute_status_handler.py tests/test_compute_workflow_definition.py tests/test_compute_plan.py tests/test_coeffgen_param_gen.py tests/test_deploy_packaging.py -q
uv run python -m pytest tests/test_coeff_program_chain.py tests/test_coeff_program_native.py tests/test_coeff_program_storage.py tests/test_saved_program_source_precedence.py tests/test_coeff_program_drift.py tests/test_param_program_chain.py tests/test_param_program_native.py tests/test_param_program_storage.py tests/test_compute_plan.py tests/test_coeffgen_param_gen.py tests/test_compute_preview_handler.py tests/test_compute_status_handler.py tests/test_compute_workflow_definition.py tests/test_compute_orchestrator.py tests/test_compute_chunk_fused.py tests/test_compute_fused.py tests/test_deploy_packaging.py tests/test_frontend_parts_contract.py -q
```

Results:

```text
api_manifest.json: OK
frontend JS checks: OK
py_compile: OK
83 passed in 1.57s
275 passed in 2.01s
```

## Recommended Next Patch

Patch the small items first:

1. Catch `CoeffSourceCompileError` in the compute-debug branch and return diagnostics.
2. Add `ConsistentRead=True` to the compute-status previous-row lookup.
3. Add explicit deploy-packaging assertions for `pipeline_programs.py`.

Then decide how far to go on source-to-chain diagnostic mapping. That is the only remaining non-trivial design issue in this review.
