# Code Review 9

Reviewed: 2026-06-11 14:49 EEST  
Commit reviewed: `7307bf0 CR8 fixes: vocab ordering is UI contract, harness runs the real hydrator`

## Findings

No blocker, concern, or nit findings found in the current CR8 patch.

The review was grounded against the actual source, not prior memory. I checked the generator, generated JS, frontend hydrator, compiler registry loader, deploy/predeploy wiring, and the regression tests added around the CR8 failure modes.

## CR8 Fix Verification

The generated vocabulary order issue is fixed. `lambda/gen_coeff_vocab.py:72-77` now renders JSON without `sort_keys=True` and explicitly documents that insertion order is UI contract. The generated artifact preserves registry function order in `coeff_vocab_js.js:6-35` and category order in `coeff_vocab_js.js:350-366`.

The ordering contract is pinned by tests. `tests/test_coeff_program_drift.py:215-220` asserts `ctCatalog` keys match `names`, category order is `structural`, `accumulation`, `elementwise`, `roots`, and the checked-in generated file exactly matches `render_js()`.

The frontend harness now executes the real registry-chip hydrator. `tests/test_frontend_js.sh:519-598` loads `coeff_vocab_js.js`, extracts the real `_hydrateCtParamDef` / `_ctCatalog` / `_ctCategoryMeta` block from `index.html`, executes it in the VM harness, and bridges the hydrated objects into the test context. The assertions at `tests/test_frontend_js.sh:719-728` verify fn-index order, category order, placeholder hydration, param-shape hydration, and alias canonicalization.

The deploy path is wired. `deploy.sh:73-79` includes `coeff_vocab_js.js` in frontend assets, `deploy.sh:619-621` regenerates it before frontend tests/build work, and `scripts/predeploy_check.sh:17-23` gates stale generated output with `lambda/gen_coeff_vocab.py --check`. `tests/test_deploy_packaging.py:618-628` pins that wiring.

The frontend consumers use the generated maps. `index.html:11661-11676` hydrates `_ctCatalog` and `_ctCategoryMeta` from `_coeffRegistryVocab`; `index.html:11722-11728` canonicalizes transform aliases from the generated `aliasToCanonical`; `index.html:12884-12899` derives source aliases and registry-to-chip shadowing from the generated vocabulary.

The compiler side remains registry-derived. `lambda/coeff_program_chain.py:196-206` documents the one-registry model, and `lambda/coeff_program_chain.py:292-370` loads names, aliases, chain-only aliases, chip names, UI metadata, and stable fn indices from `coeff_legacy_registry.json`.

## Verification Run

- `python3 lambda/gen_coeff_vocab.py --check`  
  Result: `coeff_vocab_js.js: OK`

- `bash tests/test_frontend_js.sh`  
  Result: `Frontend fused render source checks: OK`, `Frontend fused render runtime checks: OK`

- `uv run python -m pytest tests/test_coeff_program_drift.py tests/test_coeff_program_chain.py tests/test_deploy_packaging.py -q`  
  Result: `105 passed in 1.38s`

- `git diff --check`  
  Result: clean

- `bash scripts/predeploy_check.sh`  
  First sandboxed run failed because `uv` could not read `/Users/nicknassuphis/.cache/uv/...` from the sandbox. Re-run outside the sandbox passed.  
  Result: `403 passed, 2 subtests passed in 2.31s`, frontend source/runtime checks passed, `Predeploy contract gate passed.`

## Residual Risk

There is still no real browser click-through/e2e test for the transform picker. The Node harness is now materially stronger because it executes the real hydrator and checks the hydrated catalog, but it does not verify rendered DOM interaction in an actual browser. That is not a deploy blocker for this patch because the data path, generated artifact, and UI-order contract are covered.

## Verdict

Deploy-facing state looks clean. The CR8 fixes address the actual problems: generated key order is preserved, the UI ordering contract is tested, hard-coded transform counts are gone from the relevant checks, and the frontend test now exercises the shipped registry hydrator rather than a duplicate stub.
