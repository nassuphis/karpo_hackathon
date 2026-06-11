# Code Review 8 — Generated Coeff Registry Vocab

**Date:** 2026-06-11  
**Commit reviewed:** `b274706` — `Phase B: registry chip catalog generated from coeff_legacy_registry.json`  
**Scope:** generated coefficient-transform vocabulary, browser hydration,
registry/compiler drift, deploy packaging, and frontend test coverage.

## Findings

### [concern] Generated vocab loses the intended UI ordering

`lambda/gen_coeff_vocab.py` builds `functions` in stable `fn_index` order and
also emits a `names` array in that order:

- `lambda/gen_coeff_vocab.py:28`
- `lambda/gen_coeff_vocab.py:43`
- `coeff_vocab_js.js:303-331`

But the generated JS body is rendered with `json.dumps(..., sort_keys=True)`:

- `lambda/gen_coeff_vocab.py:72-74`

That recursively sorts object keys in `ctCatalog` and `categoryMeta`, so the
browser receives:

```js
"categoryMeta": {
  "accumulation": ...,
  "elementwise": ...,
  "roots": ...,
  "structural": ...
}
```

Location: `coeff_vocab_js.js:12-29`.

The registry source order is different and more intentional:

```json
"category_meta": {
  "structural": ...,
  "accumulation": ...,
  "elementwise": ...,
  "roots": ...
}
```

Location: `lambda/coeff_legacy_registry.json:854-870`.

The frontend then uses object insertion order:

- `_ctCatalog` is hydrated via `Object.entries(specs)` at `index.html:11661-11673`.
- `_coeffProgramLegacyNames = Object.keys(_ctCatalog)` at `index.html:11696`.
- `_ctCategoryGroups()` iterates `Object.keys(_ctCategoryMeta)` and
  `Object.keys(_ctCatalog)` at `index.html:12099-12111`.

User-visible impact: the chain transform picker and legacy transform dropdown
are now ordered by JSON key sorting, not by registry/fn-index/category order.
This is not a compute bug, but it is a UI regression and contradicts the
generator's own `names` array.

Recommendation:

- Either stop using `sort_keys=True` and preserve registry insertion order, or
- Emit explicit `categoryOrder` and use the existing generated `names` array in
  the frontend for `_ctCatalog`, `_coeffProgramLegacyNames`, and grouped picker
  ordering.

### [concern] Frontend test hard-codes the current transform count

The frontend harness asserts:

```js
Object.keys(coeffRegistryVocab.ctCatalog).length === 28
```

Location: `tests/test_frontend_js.sh:715`.

This conflicts with the stated goal of the commit: adding a plain transform
should be a registry JSON entry plus a C implementation, with no frontend work.
With the current hard-coded count, every new registry transform needs a test
edit even if the generated vocab and frontend hydration are otherwise correct.

The stronger tests already exist on the Python side:

- `tests/test_coeff_program_drift.py:194-217` compares generated vocab to the
  actual registry and checks freshness.

Recommendation: replace the hard-coded `28` in the JS harness with a comparison
against the generated `names.length`, or execute a small Node snippet that loads
the registry JSON and compares counts dynamically.

### [concern] Frontend harness does not execute the real `_ctCatalog` hydrator

`tests/test_frontend_js.sh` reads the real `coeff_vocab_js.js`, but the runtime
context then stubs `_ctCatalog` manually:

- Real vocab read: `tests/test_frontend_js.sh:519-522`.
- Manual `_ctCatalog` stub: `tests/test_frontend_js.sh:645-650`.

The test checks that the generated file contains 28 entries and some expected
fields, but it does not execute the browser hydrator:

- `_hydrateCtParamDef`
- `_ctCatalog` IIFE
- `_ctCategoryMeta`
- `_coeffProgramWideParamDefs`

That means a bug in placeholder replacement, category order, or generated
program-mode exp/round definitions can pass the frontend runtime harness. I
verified the hydrator manually in this review, but the automated test should
own that check.

Recommendation: add a small frontend runtime assertion that evaluates the
hydrator block against the real `coeff_vocab_js.js` and asserts:

- `_ctCatalog` has the same names as `coeffRegistryVocab.names`.
- `linear`, `exp`, `round`, `pow`, `roots`, and `roots_cm` have expected param
  shapes.
- `{SCALAR_EXPR_HELP}` is fully replaced in hydrated titles.
- category order is the intended order.

## Verified Good

- The registry is now the single source for transform aliases:
  `lambda/coeff_program_chain.py:393-394` derives `LEGACY_NAME_ALIASES` and
  `TEXT_NAME_ALIASES` from `coeff_legacy_registry.json`.
- The source parser mirrors those text aliases through
  `lambda/coeff_program_source.py:107`.
- The browser canonicalizer now reads alias mappings from the generated vocab
  instead of a hand-maintained local table:
  `index.html:11722-11728`.
- `coeff_vocab_js.js` is loaded before the main inline app script:
  `index.html:2925-2931`.
- `deploy.sh` includes `coeff_vocab_js.js` in frontend assets and uploads it
  with JavaScript content type:
  `deploy.sh:73-90`.
- Predeploy checks generated vocab freshness:
  `scripts/predeploy_check.sh:20`.
- Deploy regenerates the generated vocab before the frontend JS execution test:
  `deploy.sh:621`.
- The generated file is currently fresh:
  `python3 lambda/gen_coeff_vocab.py --check` returned `coeff_vocab_js.js: OK`.

## Tests Run

```bash
uv run python -m pytest \
  tests/test_coeff_program_drift.py \
  tests/test_coeff_program_chain.py \
  tests/test_deploy_packaging.py -q
```

Result: `105 passed in 1.45s`.

```bash
bash tests/test_frontend_js.sh
```

Result: frontend source checks OK, frontend runtime checks OK.

```bash
python3 lambda/gen_coeff_vocab.py --check
```

Result: `coeff_vocab_js.js: OK`.

```bash
python3 -m py_compile \
  lambda/gen_coeff_vocab.py \
  lambda/coeff_program_chain.py \
  lambda/coeff_program_source.py
```

Result: passed.

```bash
git diff --check
```

Result: passed.

Manual hydration smoke:

```bash
node - <<'NODE'
// Loaded coeff_vocab_js.js and the actual index.html hydration block.
// Result: 28 hydrated _ctCatalog entries, exp titles resolved, program-mode
// exp defs resolved, category keys currently accumulation,elementwise,roots,structural.
NODE
```

## Not Run

I did not rerun the full predeploy gate or Docker runtime gate during this
review. The immediately preceding deploy run completed both successfully, but
this review's local verification was targeted at the generated-vocab change.

## Verdict

No blocker found. Runtime/compiler/deploy wiring is sound, and the registry
vocabulary extraction is the right architectural direction.

Two follow-ups should land soon:

- Preserve intentional UI ordering instead of using sorted JSON object order.
- Replace the hard-coded frontend transform count and execute the real hydrator
  in the frontend harness.

Those are quality and maintainability issues, not deploy-stopping compute
bugs.

---

## Resolution — 2026-06-11

**[concern] sorted JSON broke picker order — ✅ fixed.** `sort_keys=True`
removed from the generator with a comment stating that insertion order is UI
contract (ctCatalog keys drive the transform picker in fn\_index order —
which is exactly the old hand-catalog order within categories; categoryMeta
drives category order in registry order). The drift test now pins both:
`list(vocab["ctCatalog"]) == vocab["names"]` and the literal category order.

**[concern] hard-coded 28 — ✅ fixed.** The harness compares
`Object.keys(ctCatalog)` against the generated `names` array (full
order-sensitive equality), so a new registry transform needs zero frontend
or frontend-test work.

**[concern] hydrator not executed — ✅ fixed.** The harness now extracts and
runs the real hydrator block (`_hydrateCtParamDef`, the `_ctCatalog` IIFE,
`_ctCategoryMeta`, `_coeffProgramWideParamDefs`) against the real generated
file, with a `globalThis` bridge because vm scripts keep top-level consts
lexical. It asserts: hydrated names == `vocab.names` in order; category
order is the intended four; andy appended to every transform (linear 2+1,
rev 0+1); `{SCALAR_EXPR_HELP}` fully resolved into real help text; exp/round
wide defs hydrate (3 and 2 params); pow/power/roots param shapes match the
registry ui. The hand `_ctCatalog` stub was deleted — the serializer tests
now run against the hydrated real catalog.

Flip-verified: regenerating the vocab with sorted keys fails the harness
(`...should cover every registry function in fn_index order`) and the drift
suite. Gates after the fixes: full pytest + predeploy green.
