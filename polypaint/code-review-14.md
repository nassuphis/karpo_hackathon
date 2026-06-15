# Code Review 14

Reviewed current `main` at `eecadba` after the latest pushed changes. Focus areas: coefficient/parameter compilers, typed VM lowering, compute pipeline workflow, source/chain precedence, and error tracking.

## Verdict

No blocker found in the compiler or workflow wiring. The current implementation looks deployable from the reviewed paths: coefficient source compiles through the Python authority, raw source is kept out of downstream hot paths, compiled program fingerprints propagate into cache signatures, and the typed VM path has broad unit/native coverage.

The main remaining weaknesses are not execution correctness; they are error reporting shape and duplicated pipeline/source-selection logic. Those should be tightened before more language surface is added, because they will otherwise keep producing hard-to-debug preview/probe failures.

## Findings

[concern] Strict source-text compile paths discard structured line/column diagnostics.

`lambda/coeff_program_source.py:855` builds useful structured diagnostics in non-strict mode, including `line`, `column`, `statement`, and `message`. The storage compile endpoint preserves that shape in `lambda/handler_storage.py:1205`, which is the right behavior for the editor.

The compute paths do not preserve that structure. `lambda/handler_compute_preview.py:295` catches source compile errors and collapses them into a single `ValueError` string. `lambda/handler_compute_plan.py:239` calls `parse_coeff_program_source(..., strict=True)` and lets the strict exception bubble as plain text. This means the editor route can show good diagnostics, but preview/calculate/probe failures lose the structured error context users need.

Recommendation: add a shared program-compile exception/envelope, or have compute/preview/plan compile source with `strict=False`, then raise/return an error payload that preserves `line`, `column`, `statement`, and `message`. The UI can still display one summary line, but the backend should not throw away the diagnostic structure.

[concern] Pipeline/source resolution logic is duplicated across handlers.

Mode selection is independently implemented in `lambda/handler_compute_plan.py:124`, `lambda/handler_coeffgen.py:150`, `lambda/handler_compute_preview.py:89`, and `lambda/handler_compute_orchestrator.py:36`. Source-vs-chain precedence is also duplicated in `lambda/handler_compute_plan.py:239`, `lambda/handler_coeffgen.py:123`, and `lambda/handler_compute_preview.py:295`.

The current behavior is mostly consistent and tests cover the important cases, but this is exactly the kind of duplication that drifts. Program mode has subtle rules: source wins over chain at plan/preview compile time, downstream workflow steps execute compiled chain only, Chain mode must zero out programs, and Program mode must zero out legacy transforms.

Recommendation: move this into a small shared module such as `lambda/pipeline_programs.py` with helpers for `pipeline_mode_from_params`, selecting active param/coeff payloads, compiling coefficient source, and clearing inactive branches. Then update the handlers to call the shared functions and keep one source of truth.

[concern] Compute workflow error rows lose failed-phase metadata.

The Step Functions top-level catch in `stepfunctions/compute_workflow.asl.json.template:704` sends only `Error` and `Cause` into `ReportError`. `lambda/handler_compute_status.py:95` writes the final status row with `phase="error"` regardless of where the failure happened, and `_extract_error_message()` at `lambda/handler_compute_status.py:107` only extracts human-readable text.

This preserves the message but not the structured failure point. A degree-probe failure, build-plan failure, coeffgen failure, and fused-chunk failure all collapse to the same final phase. The phase-specific row often exists earlier, but the top-level task row no longer carries `failed_phase` or `failed_state`, so UI/status/debug consumers have to infer it from message text or Step Functions history.

Recommendation: pass and store `failed_state` or `failed_phase` in the `ReportError` payload, and persist it as a separate field instead of overloading the message. Add unit coverage for `_extract_error_message()` and one ASL/static test that confirms the failed-state context reaches `handler_compute_status`.

[concern] Compute-preview request validation still returns HTTP 500 for some client input errors.

`lambda/handler_compute_preview.py:563` validates preview dimensions, preview size, quantile, and shim by raising `ValueError`. The outer handler catches only selected program-validation errors as HTTP 400; generic exceptions fall through to HTTP 500 at `lambda/handler_compute_preview.py:704`. Current tests in `tests/test_compute_preview_handler.py:500` codify 500 for invalid `preview_size` and invalid `shim`.

That is the wrong error class for user input. Invalid program source already returns 400; invalid preview parameters should do the same. This is not a compiler correctness issue, but it makes error tracking noisier and makes client bugs look like server faults.

Recommendation: introduce a small `PreviewRequestError` or catch `ValueError` from the explicit validation block and return HTTP 400. Keep true unexpected exceptions as 500.

[nit] Split-JS cache-busting comment is stale after versioned asset keys.

`js/02-preview-solvescore.js:1` still says deploy appends `?v=<BUILD_ID>` to script tags. `deploy.sh:173` now copies JS chunks to versioned object keys and rewrites `index.html` to reference those build-keyed assets. The code is correct; the comment is stale.

Recommendation: update the split-file header to say deploy rewrites script references to build-versioned asset keys. This avoids sending future reviewers hunting for query-string behavior that no longer exists.

## Confirmed Good

The coefficient source parser is correctly backend-authoritative. The JS side can help with editing, but `lambda/handler_storage.py:1205` compiles on save/compile through Python, and source text wins over client-supplied chain when both are present.

The coefficient source lowerer has the right shape. `lambda/coeff_program_source.py:537` handles explicit assignment, bare source pushes, typed scalar/vector forms, and native transform sugar. `lambda/coeff_program_source.py:689` rejects standalone `pop`/`peek` instead of silently treating them as emit-like operations. `lambda/coeff_program_source.py:748` returns a compiled program plus diagnostics without returning a partial executable chain when errors exist.

The coefficient chain compiler is now a real typed-program compiler rather than a legacy wrapper. `lambda/coeff_program_chain.py:1499` expands macros and enforces the 256-token budget. `lambda/coeff_program_chain.py:1651` validates stack type/depth. `lambda/coeff_program_chain.py:1838` builds fingerprints from the execution-relevant compiled spec rather than raw source formatting.

The native coefficient VM looks coherent. Dynamic scalar/vector reads are range checked in `lambda/sweep_cli.c:3846`. Typed binary/unary/get/set/poke/fill/blend operations enforce stack type and vector length in `lambda/sweep_cli.c:4438`. Native transforms evaluate `andy` before both expression-arg and stack-arg paths in `lambda/sweep_cli.c:4609`. The main evaluator dispatch covers the current opcode surface in `lambda/sweep_cli.c:5127`.

The previous param-program selector-aliasing trap is not recurring. `lambda/param_program_chain.py:125` keeps legacy selectors separate from `t1`/`t2`, `lambda/param_program_chain.py:775` rejects invalid legacy selector names, and `lambda/param_program_chain.py:1192` lowers dynamic argument expressions into the main VM token stream.

Pipeline propagation is correct at the important boundaries. Preview compiles source before invoking coeffgen in `lambda/handler_compute_preview.py:295`. Compute plan stores compiled program payloads in `lambda/handler_compute_plan.py:211`. Degree probe compiles source in `lambda/handler_coeffgen.py:123`, while chunked coeffgen consumes compiled payloads in `lambda/handler_coeffgen.py:606`. Fused chunks receive compiled program JSON in `lambda/handler_compute_chunk_fused.py:44`.

The Step Functions template forwards raw `coeff_program_source_text` only to degree-probe/build-plan style stages and forwards compiled `coeff_program` downstream. The relevant payload wiring is in `stepfunctions/compute_workflow.asl.json.template:95`, `stepfunctions/compute_workflow.asl.json.template:260`, `stepfunctions/compute_workflow.asl.json.template:429`, and `stepfunctions/compute_workflow.asl.json.template:535`.

Cache invalidation uses compiled fingerprints. `lambda/compute_fused.py:34` includes `param_program_fingerprint` and `coeff_program_fingerprint` in the probe signature; raw source text does not control cache identity directly.

## Verification Run

Commands run locally:

```text
python3 api_manifest.py --check
bash tests/test_frontend_js.sh
python3 -m py_compile lambda/coeff_program_source.py lambda/coeff_program_chain.py lambda/param_program_chain.py lambda/handler_compute_plan.py lambda/handler_coeffgen.py lambda/handler_compute_preview.py lambda/handler_compute_status.py lambda/handler_compute_orchestrator.py lambda/handler_compute_chunk_fused.py lambda/compute_fused.py
uv run python -m pytest tests/test_coeff_program_chain.py tests/test_coeff_program_native.py tests/test_coeff_program_storage.py tests/test_saved_program_source_precedence.py tests/test_coeff_program_drift.py tests/test_param_program_chain.py tests/test_param_program_native.py tests/test_param_program_storage.py tests/test_compute_plan.py tests/test_coeffgen_param_gen.py tests/test_compute_preview_handler.py tests/test_compute_workflow_definition.py tests/test_compute_orchestrator.py tests/test_compute_chunk_fused.py tests/test_compute_fused.py tests/test_deploy_packaging.py tests/test_frontend_parts_contract.py -q
git diff --check
```

Results:

```text
api_manifest.json: OK
frontend JS checks: OK
py_compile: OK
272 passed in 2.02s
git diff --check: OK
```

## Next Fixes

Fix the diagnostic-preservation issue first. It directly affects the user's ability to debug source programs from preview/calculate/probe failures.

Then centralize pipeline mode/source resolution into a shared helper. That is not urgent because tests are green, but it is the main structural drift risk in the workflow.

After that, tighten workflow error metadata so the final task row records where the pipeline failed, not just that it failed.

## Resolution Applied

The CR14 follow-up change addressed the five findings:

- Added `lambda/pipeline_programs.py` as the shared source of truth for pipeline mode selection, coefficient source precedence, and structured source compile errors.
- Rewired compute preview, compute plan, and coeffgen to use the shared helper, and packaged the helper in the affected Lambda bundles.
- Changed compute preview request-validation failures from HTTP 500 to HTTP 400 and kept structured coefficient source diagnostics in preview responses.
- Updated compute status error rows to preserve the last reported phase as `failed_phase`, `failed_phase_label`, and related context by reading the previous task row before the error overwrite.
- Updated the split-JS headers to describe build-versioned asset keys instead of stale `?v=<BUILD_ID>` query stamping.

Follow-up verification:

```text
python3 api_manifest.py --check
bash tests/test_frontend_js.sh
python3 -m py_compile lambda/pipeline_programs.py lambda/handler_compute_status.py lambda/handler_compute_preview.py lambda/handler_compute_plan.py lambda/handler_coeffgen.py
uv run python -m pytest tests/test_coeff_program_chain.py tests/test_coeff_program_native.py tests/test_coeff_program_storage.py tests/test_saved_program_source_precedence.py tests/test_coeff_program_drift.py tests/test_param_program_chain.py tests/test_param_program_native.py tests/test_param_program_storage.py tests/test_compute_plan.py tests/test_coeffgen_param_gen.py tests/test_compute_preview_handler.py tests/test_compute_status_handler.py tests/test_compute_workflow_definition.py tests/test_compute_orchestrator.py tests/test_compute_chunk_fused.py tests/test_compute_fused.py tests/test_deploy_packaging.py tests/test_frontend_parts_contract.py -q
git diff --check
```

Results:

```text
api_manifest.json: OK
frontend JS checks: OK
py_compile: OK
275 passed in 2.03s
git diff --check: OK
```
