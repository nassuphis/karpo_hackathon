# Code Review 23: Param/Coeff Registry Infrastructure Refactor

## Objective

Param legacy transforms and Coeff registry transforms should use shared registry infrastructure.

The execution models are different and must stay different:

- Param transforms operate on scalar complex registers (`p1`, `p2`) and Param stack selectors.
- Coeff transforms operate on coefficient vectors (`cf`, `poly`, vector stack) and Coeff selectors.

The registry mechanics should not be different:

- JSON loading and version checks.
- Function name and `fn_index` uniqueness.
- Arg normalization and arg-type validation.
- Required local `ui.desc`.
- UI param/default/help validation.
- Category metadata handling.
- JS vocabulary generation.
- Frontend Help adapter normalization.
- Source parser registry-name validation.
- Drift tests.

This review separates valid domain differences from duplicated infrastructure, then gives an implementation plan.

## Implementation Status — verified at `ba0f962` (2026-07-02)

> **FIX BATCH LANDED (2026-07-02, `6e5feb3..60641ee`, 8 commits):** the Superseding Implementation Order was executed. Per wave, each verified by repro + full predeploy gate:
> - **A `6e5feb3`** — 12 test files gated (registry schema, v2 migration, root family, source precedence, coeff source equivalence, whole-sweep SHA oracle, solve-score parity, source core, profile drift, parts contract, run boundaries) + `test_predeploy_gate_completeness.py` meta-test: new test files must be gated or explicitly excluded (H8 gate rot).
> - **B `f7e2008`** — H1 param save forwards `source_text` (was 400-on-every-save); H3 littlewood `_ctAndyIndex` ReferenceError; A2 partial-vocab `ctCatalog` crash; H7 const imaginary-lane drop + whitespace arg-split mangling. All harness-pinned.
> - **C `d1d8c10`** — Open #2 `inv_t_plus_2` arity drift (shadowing hardcodes deleted; arity reads registry compat; parity test compiles every compat count through parse AND chain); H5 null-chain absent-semantics (root + v2 copy), compat-block load-time raise, param_debug explicit-empty routing, optional-arg collision validation, `variable_arg_forms` count-3 form + schema gate; H6 OverflowError guards, exact-zero divide threshold, numeric-0 literal fix; H7 `macro(v2/…)` normalization, `/detail` coeff display; F3 chain diagnostics carry stable codes in both kinds.
> - **D `bb88b50`** — H2 v2 lifecycle: save drops the stale migrated copy, delete removes both keys; save→migrate→edit→fetch and delete→404 lifecycle tests.
> - **E `d8b49cf`** — dead payload deleted: coeff vocab `names`/`fnIndexByName`/`effectiveArgs`/`compatSignatures` (−535 generated lines, absence now gated), solve-score `delta`/`legacy_alias`, `root_source_text_from_payload`, `_paramProgramLegacyArgSpecs`; pinning tests repointed to loader invariants (B2/G7/F7).
> - **F `80de7f1`** — G2 root cap pinned three ways (profile corrected 64→16 == Python == parsed C `#define`); G8 four-case blank/empty matrix per kind (gated contract); G6 `solve_metric_is_param_metric` C-partition drift test; F6 storage-vs-compute macro resolver equivalence pins.
> - **G `a9732ef`** — H4 `c_abs`→`hypot` (EXPR + both param-VM sites), tan/tanh exact-pole now matches cmath for pure-real/imag input, `c_sqrt_c` exact negative-real branch; sweep_test rebuilt; native parity + M3 oracles + golden wire hex + **byte-exact whole-sweep SHAs all unchanged** (corpus doesn't touch the fixed regimes); new native regression test pins the underflow case.
> - **H `60641ee`** — 5E residue: andy arities + stack limits now derive from `compat_signatures` (proven equivalent for all 28 fns); oracle-edit policy + true-scope docstrings (H8 oracle-fiction).
>
> **Deliberately NOT fixed (design decisions, not safe silent fixes):** H7 chip respelling (moebius 8-packed→4-complex, native+andy→legacy op) — needs a spelling-preservation decision; H7 garbage-row fabrication defaults + param import validation parity — load-path behavior changes; G4 solve-score statement-local line/col — parser surgery; `_format_number` dedupe into named policies — touches fingerprint-producing code paths for zero behavior change; `solve_metric_min_roots` drift test — no Python counterpart table to pin against. **Follow-up for the user:** validate the C numeric changes in the Docker ARM64 runtime (`scripts/test-docker-runtime.sh` needs deploy binaries, which only the user builds/deploys).

Everything below was re-verified against source at HEAD, not inferred from commit messages. Full predeploy gate green.

### Done

| Plan item | Status / evidence |
|---|---|
| **Phase -1** live-bug batch (A1 signed-zero, A2 partial-vocab crash, A4 parse-time legacy validation, A5/F6 macro source precedence, A6 serializer round-trip guard, X1 solve-score `%g` collision, X2 root cap, F4 zero-defaults, root malformed-payload/mismatch diagnostics, v2 fingerprints exclude `source_text`) | ✅ landed in `f8c933e`; every repro re-run and confirmed fixed; equivalent-zero forms collapse while nonzero fingerprints stay distinct; M3 oracle green |
| **Phase 1** shared schema tests | ✅ `tests/test_registry_schema.py` exists and passes — **not yet predeploy-gated (see Open #1)** |
| **Phase 2** `lambda/registry_common.py` | ✅ exists; pure shared helpers |
| **Phase 3** generators on common helpers | ✅ both `gen_param_vocab.py` and `gen_coeff_vocab.py` import `registry_common` |
| **Phase 4 / B1** param compat registry-authoritative | ✅ all **six** compat families read via `_legacy_compat()` → `legacy_registry()["compat"]` (`param_program_chain.py:343-368`); the registry is no longer decorative at runtime. Python constants remain (`:239`) as drift-test fixtures per the transition rule, pinned by `test_param_program_drift.py:174-176` |
| **Phase 5A** category schema normalized | ✅ param registry now top-level `category_meta` (was `ui.categories`) |
| **Phase 5B** `variable_arg_forms` | ✅ no longer inert — top-level, consumed by `gen_param_vocab.py` |
| **Phase 10 / D — andy de-specialized** | ✅ `supports_andy` deleted from all 28 functions; andy declared **once as data** in top-level `shared_optional_args` (`{"name":"andy","type":"real","default":0,"optional":true,"role":"andy"}`); generator `ANDY_PARAM` hardcode deleted (0 refs); `effective_args` materializes it from the shared declaration (`coeff_program_chain` loader). Andy is now an ordinary declared optional argument |

### Open

> **Updated 2026-07-02:** a fifth deep-dive sweep (see **H-Series** below) found four broken-product-class bugs (H1 param save, H2 v2 lifecycle, H3 littlewood crash, H4 C `abs()` numeric), regressions inside the fix commits (H5), scalar-expression-layer defects (H6), persistence round-trip defects (H7), and systemic gate/oracle-integrity rot (H8: 100 of 140 test files ungated; the coeff source "frozen oracle" imports production). The **Superseding Implementation Order** (after the H-series) is now the plan of record; the item list below remains accurate but is subsumed by it.

1. **Gate gap (fix first):** `test_registry_schema.py` is not in `scripts/predeploy_check.sh` — **worse than one line: six files** (`test_registry_schema`, `test_program_v2_migration`, `test_root_program_source`, `test_saved_program_source_precedence`, `test_coeff_source_equivalence`, `test_whole_sweep_oracle`) are ungated; see H8.
2. **NEW BUG — `inv_t_plus_2` source-vs-chain arity drift:** `param_program_source.py:103` hardcodes arity `{0,1,2}` while registry `compat.variable_arg_counts` and the chain compiler allow `{0,1,2,3,4}`. Verified: `legacy(inv_t_plus_2, both, both, 1, 2, 3)` → source parse rejects `bad_arity`, chain compile accepts — so a saved 3-arg chain serializes to source that won't reparse. A fresh instance of the B1 flavor introduced *by* the A4 fix. Fix: read the arity set from `legacy_registry()["compat"]`.
3. **Phases 6–8 (frontend arc), partial:** per-profile adapters now exist (`_paramRegistryAdapter` js/07:15, `_coeffRegistryAdapter` js/07:254) with a converging shape, and the static `_pt*` fallbacks are **deleted** (explicit "registry not loaded" state instead) — but there is no shared `_makeProgramRegistryAdapter` factory, and legacy globals (`_paramProgramLegacyArgSpecs`, `_coeffProgramLegacyInputDefs`, …) still exist as delegates.
4. **Phase 9:** param validates at parse now, but via its own `_validate_legacy_source_entry`; no *shared* registry-name validator/canonicalizer across both parsers.
5. **C-executor runtime single-sourcing:** the **drift tests landed** (`67ff066`: `dispatchPt` defaults vs registry, `coeffLegacyApply` fallbacks, native packing special-cases vs `compat_signatures` — all pass), so the earlier "decorative registry" risk is now pinned. Still open: C remains the runtime authority for omitted args (no Python-always-expands contract), and `solve_score.h` metric-partition/min-roots tables have no drift test.
6. **Residuals:** `_split_native_transform_andy` still hardcodes arities by fn_index (`coeff_program_source.py:268`, registry `andy_arg_counts` exists but unread there); `_max_native_transform_stack_arg_count` still fn_index-hardcoded (both now drift-pinned); root profile still declares `program_tokens: 64` while Python/C enforce 16, no cap drift test; solve-score chain-stage diagnostics still report line 1/col 1; empty/blank-source policy still diverges by kind (G8); dead code: `root_source_text_from_payload` (zero callers), coeff vocab keys `fnIndexByName`/`effectiveArgs`/`compatSignatures`/`names` and solve-score `delta`/`legacy_alias` subfields (zero JS consumers).

### Verification sweep — all 37 findings re-verified at `ba0f962`

Every finding section below was re-checked against HEAD by re-running its repro (four parallel deep-dives; severity-critical items re-run independently). Summary — the body text of each finding describes the **as-found** state; this table is current truth:

| Status | Findings |
|---|---|
| **FIXED, repro re-run, test-pinned** | A1, A3 (round overlap resolved: split by call form, both signatures named in registry + drift-pinned; heading retitled accordingly), A4 (incl. bare-shortcut arity + coeff `roots` int/enum/extra-arg → coded at parse), A5, A6, B1 (all six compat families runtime-read + drift-pinned + C-drift checks), C1, C3, C4, C8, D1, D2 (andy = `shared_optional_args` data; `ANDY_PARAM` gone; schema test asserts `supports_andy` absent), F1-fallback-half, F2-param-half, F4, F6, G1, G3, G5, G9, G10, G11 (statics `_ptCatalog`/`_ptInfo`/`_ptCategoryMeta` gone repo-wide, regression-guarded) |
| **Resolved beyond the ask** | Spurious #4 (silent fallback → explicit registry-not-loaded article), #5 (`variable_arg_forms` rendered in Help via `variableForms()`), #8 (shared schema test exists — but un-gated, Open #1) |
| **PARTIAL** | A2 (documented guard applied, but `{}` vocab still crashes at the unguarded `ctCatalog` read, js/07:240), B2 (`supportsAndy` gone; 4 dead keys remain, now drift-pinned), F1-policy, F5 (under-arity policy explicit in code but not registry-declared; plus the new `inv_t_plus_2` drift), F8/G4 (root + solve-score have distinct codes now; solve-score chain-stage still line-1/col-1), G0 (signed-zero/`%g`/v2 leaks fixed; still 6 `_format_number` copies, `.17g` vs `repr`, sha1 vs sha256 — no shared contract), G2 (cap enforced 16; profile still 64, no drift tie), G6 (3 C-drift tests landed; solve-score partitions + runtime single-sourcing open), Spurious #1/#2 (generator/loader cores shared; residue: duplicated `main()` dispatch, per-arg normalization, memo boilerplate), #3 (top-level `category_meta` unified; per-function path still `ui.category` vs `category`), #6, #7 |
| **STILL-PRESENT (unchanged)** | C2 (`choices or _ENUM_ARG_VALUES`), C5 (`_profileSelectorChoices` `.length` fallback, now js/07:340), C6 (`selectors.get(name) or []`), C7 (display-only), D3, D4, E (intentional; now structurally encoded `aliases` vs `chain_only_aliases` + pinned), F3 (chain compilers still emit code-less `{level,message}`), F7 (+ `root_source_text_from_payload` now confirmed dead), G7, G8 |

Registry-shape note: both "Current State" JSON samples below are **stale** — param top keys are now `version/functions/category_meta/variable_arg_forms/compat` (no top-level `ui` block; `compat` has 6 keys), coeff top keys are `version/shared_optional_args/functions/category_meta` (`supports_andy` deleted everywhere). The consumer lists remain accurate.

## Current State

### Param Registry

File: `lambda/param_legacy_registry.json`

Current shape:

```json
{
  "version": 1,
  "functions": [
    {
      "name": "rect",
      "fn_index": 43,
      "kind": "shape",
      "allowed_src": ["p1", "p2", "both", "pop1", "pop2"],
      "allowed_tgt": ["p1", "p2", "both", "push1", "push2"],
      "args": [{"name": "w", "type": "real", "default": 2}],
      "ui": {
        "category": "shapes",
        "desc": "...",
        "params": [...]
      }
    }
  ],
  "category_meta": {...},
  "variable_arg_forms": {...},
  "compat": {
    "target_arg_indexes": {...},
    "independent_targets": [...],
    "variable_arg_counts": {...},
    "target_first": [...],
    "target_last": [...],
    "dither_target_first": [...]
  }
}
```

(Sample updated to the post-Phase-5A shape at `ba0f962`: `category_meta` and `variable_arg_forms` are top-level — the old top-level `ui` block is gone — and `compat` carries all six families.)

Important consumers:

- Runtime loader: `lambda/param_program_chain.py::_load_legacy_registry`.
- Source parser: `lambda/param_program_source.py::_lower_call`.
- JS generator: `lambda/gen_param_vocab.py`.
- Generated frontend payload: `param_vocab_js.js`.
- Frontend catalog and help adapter: `js/07-transform-catalogs.js`, `js/08-chip-editors.js`.
- Drift tests: `tests/test_param_program_drift.py`.

Recent local state is materially better than the old schema: Param functions now have function-local `ui.desc`, and runtime args have local `ui.params`. **Update at `ba0f962`:** Param now shares registry infrastructure with Coeff — both generator and runtime loader build on `lambda/registry_common.py`, and the compiler reads all six `compat` families from the registry at runtime (`param_program_chain.py:343-368`). The residue is listed in Spurious #1/#2 below.

### Coeff Registry

File: `lambda/coeff_legacy_registry.json`

Current shape:

```json
{
  "version": 1,
  "shared_optional_args": [
    {"name": "andy", "type": "real", "default": 0, "optional": true, "role": "andy"}
  ],
  "functions": [
    {
      "name": "rev",
      "fn_index": 1,
      "category": "structural",
      "allowed_src": ["cf", "poly", "pop", "peek"],
      "allowed_tgt": ["poly", "push"],
      "args": [],
      "length_policy": "same",
      "ui": {
        "desc": "..."
      }
    }
  ],
  "category_meta": {...}
}
```

(Sample updated to the post-Phase-10A shape at `ba0f962`: `supports_andy` is deleted everywhere — `test_registry_schema.py:178` asserts its absence — and andy is declared once as data in top-level `shared_optional_args`, materialized into each spec's `effective_args`/`optional_args` by the loader.)

Additional Coeff-only fields are legitimate:

- `aliases`
- `chain_only_aliases`
- `chip_name`
- `compat_signatures`
- `length_policy`
- `shared_optional_args` (top-level; replaces the deleted `supports_andy` flag)
- `ui.program_params`

Important consumers:

- Runtime loader: `lambda/coeff_program_chain.py::_load_legacy_registry`.
- Source parser: `lambda/coeff_program_source.py::_canonical_native_name`, `_lower_call`.
- JS generator: `lambda/gen_coeff_vocab.py`.
- Generated frontend payload: `coeff_vocab_js.js`.
- Frontend catalog and help adapter: `js/07-transform-catalogs.js`, `js/08-chip-editors.js`.
- Drift tests: `tests/test_coeff_program_drift.py`.

Coeff is closer to generated-only frontend metadata than Param. **Update at `ba0f962`:** the generator/loader cores are shared via `registry_common`, and the `andy` special case is gone from generator code — it flows through the generic optional-args path from `shared_optional_args`. The remaining per-profile pieces are the frontend adapters (two parallel implementations, no shared factory) and the profile-specific arg builders.

## Valid Differences

These should remain profile-specific.

### Param Semantics

Param registry functions:

- Run on scalar complex values.
- Use `p1`, `p2`, `both`, `pop1`, `pop2` sources.
- Use `p1`, `p2`, `both`, `push1`, `push2` targets.
- Preserve old chain compatibility where some functions encoded target as an argument.
- Preserve variable legacy arg counts for old forms such as `moebius`, `inv_t_plus_2`, and `add`.

### Coeff Semantics

Coeff registry functions:

- Run on coefficient vectors.
- Use `cf`, `poly`, `pop`, `peek` sources.
- Use `poly`, `push` targets.
- Need vector length policy.
- Need source aliases and chain-only aliases.
- Need native-transform packing compatibility.
- Carry universal `andy` support, now declared once as a shared optional arg (`shared_optional_args`), no longer a per-function flag.
- Some functions need different chip params versus source-program params (`ui.params` vs `ui.program_params`).

Do not merge the compilers or force one registry to pretend it is the other. Merge the registry infrastructure around them.

## Spurious Differences And Drift Surfaces

### 1. Duplicate Generator Skeletons

> **Status at `ba0f962`: MOSTLY RESOLVED.** Both generators import `load_json`/`default_text`/`require_registry_version`/`registry_functions`/`require_function_ui_desc`/`extract_category_meta`/`render_js_assignment`/`check_generated_file` from `registry_common`. Residue: the ~14-line `main()` `--check`/write dispatch is still duplicated; arg-display builders stay profile-specific by design.

`lambda/gen_param_vocab.py` and `lambda/gen_coeff_vocab.py` both do the same basic work:

- Load registry JSON.
- Sort functions by `fn_index`.
- Validate required UI descriptions.
- Build frontend catalog objects.
- Normalize params/defaults/help titles.
- Emit a `window._...RegistryVocab` JS file.
- Support a stale-file `--check` mode.

The profile-specific fields differ, but the generator core should be shared.

### 2. Duplicate Runtime Registry Loaders

> **Status at `ba0f962`: PARTIAL.** Both loaders share `load_json` + `require_registry_version` + `registry_functions` + `validate_function_identity` (5 of the 8 bullets below). Still per-profile: MAX_ARGS guard, per-arg normalization loop, `by_name`/`by_index` construction, memoized `legacy_registry()` boilerplate.

`param_program_chain.py::_load_legacy_registry` and `coeff_program_chain.py::_load_legacy_registry` both:

- Load JSON.
- Check `version == 1`.
- Validate non-empty names.
- Validate unique names.
- Validate positive unique `fn_index`.
- Normalize `args`.
- Validate max arg count.
- Build `by_name` and `by_index`.

Coeff has extra fields and alias validation. Param has compatibility fields. The common loader core should be shared and profile augmentation should be profile-specific.

### 3. Category Metadata Shape Differs

> **Status at `ba0f962`: PARTIAL.** Top-level divergence resolved — both registries use top-level `category_meta`, both generators use `extract_category_meta`. Residue: per-function path still diverges (param `fn.ui.category` vs coeff `fn.category`); the shared schema test bridges it with `fn.get("category") or fn["ui"].get("category")`.

Param:

```json
"ui": {
  "categories": {...}
}
```

Coeff:

```json
"category_meta": {...}
```

This is a pure schema difference. It creates adapter code and tests that should not exist. The target schema should use one form, preferably top-level `category_meta`.

### 4. Param Has Static Frontend Fallback Metadata

> **Status at `ba0f962`: RESOLVED (beyond the ask).** `_ptInfo`/`_ptCategoryMeta`/`_ptCatalog` no longer exist anywhere (removed in `af496e4` + `ba0f962`; `_ptCatalog`'s non-Help consumers were deleted as legacy-editor dead code). Missing vocab now renders an explicit "Param registry not loaded" article (`js/08:635`; coeff analog `:753`), pinned by `test_frontend_js.sh:144`. The text below is the as-found state.

`js/07-transform-catalogs.js` still has `_ptCategoryMeta` and `_ptInfo` as static Param legacy metadata, plus `_ptCatalog` enrichment.

That was useful before the generated registry became complete. It is now a drift surface:

- Help can silently fall back to stale text.
- Registry and frontend can disagree while tests still pass.
- `_ptInfo` and `_ptCategoryMeta` duplicate function descriptions/categories that now belong in `param_legacy_registry.json`.
- `_ptCatalog` is broader than Help fallback: it is shared chip/popup/preview metadata and is used outside this Help path. Do not delete it casually.

The final state should not use static Param legacy descriptions/categories when `_paramRegistryVocab` is present. `_ptCatalog` may remain until its non-Help consumers are explicitly migrated.

### 5. Param `variable_arg_forms` Is Inert

> **Status at `ba0f962`: RESOLVED.** Top-level `variable_arg_forms` → emitted as `variableArgForms` in `param_vocab_js.js` → consumed by `_paramRegistryAdapter.variableForms()` (`js/07:41`) → rendered into Help notes (`js/08:555-564`). Pinned by `test_frontend_js.sh:171` + `test_registry_schema.py:77`. The text below is the as-found state.

`lambda/param_legacy_registry.json` has `ui.variable_arg_forms`, but current consumers do not reference it.

This is dangerous because it looks authoritative but has no effect.

Decision required:

- Either wire variable forms into generated Help articles, or
- Move them into `compat` as backend-only comments/metadata, or
- Delete them if `compat.variable_arg_counts` is sufficient.

Preferred: move useful user-facing forms to function-local `ui.forms` or `ui.compat_forms` and render them in Help.

### 6. Source Parser Registry Handling Differs

> **Status at `ba0f962`: PARTIAL.** The behavioral gap is fixed — param validates name/selectors/arity at parse (`_validate_legacy_source_entry`, `param_program_source.py:112`) with coded diagnostics. Residue: no *shared* validator/canonicalizer (param and coeff each have their own), and the param copy introduced the `inv_t_plus_2` arity drift (Open #2).

Coeff source parsing canonicalizes registry aliases and rejects unknown registry names early.

Param source parsing accepts:

```text
legacy(name, src, tgt, ...)
```

and only lowercases `name`; compile-time chain validation catches unknown names later.

This is avoidable drift. Both parsers should use a shared registry-name validator/canonicalizer, configured per profile.

### 7. Frontend Registry Adapters Are Separate

> **Status at `ba0f962`: PARTIAL.** Two per-profile adapters now exist (`_paramRegistryAdapter` js/07:15, `_coeffRegistryAdapter` js/07:254) converging on `loaded/names/categoryMeta/spec()/params()/category()`; `_paramRegistryCategoryMeta`/`_paramRegistryUiFunctions` are gone (folded in). No shared `_makeProgramRegistryAdapter` factory yet; `_paramProgramLegacyArgSpecs`, `_coeffProgramLegacyInputDefs`, `_coeffRegistrySourceName` (now a one-line delegate), `_ctCategoryMeta` (vocab-sourced) still exist.

The Help renderer is shared, but the registry adapters are not.

Param uses:

- `_paramRegistryVocab`
- `_paramProgramLegacyNames`
- `_paramProgramLegacyArgSpecs`
- `_paramRegistryCategoryMeta`
- `_paramRegistryUiFunctions`

Coeff uses:

- `_coeffRegistryVocab`
- `_ctCatalog`
- `_coeffProgramLegacyNames`
- `_coeffProgramLegacyInputDefs`
- `_coeffRegistrySourceName`
- `_ctCategoryMeta`

These should normalize into one frontend shape:

```js
{
  kind,
  names,
  catalog,
  categoryMeta,
  argSpecs,
  aliases,
  sourceName(name),
  helpParams(name),
  snippet(name)
}
```

Profile-specific adapter logic can remain behind this interface.

### 8. Drift Tests Are Profile-Specific

> **Status at `ba0f962`: RESOLVED, except gating.** `tests/test_registry_schema.py` covers both registries (shared shape + param selectors/ui.params arity + compat pins + coeff compat_signatures pins + andy-absence). 4 tests pass — but the file is NOT in `scripts/predeploy_check.sh` (Open #1).

Param and Coeff each have their own generated-vocab assertions. That lets the schemas diverge while both test suites remain green.

Add a shared registry-schema test that runs against both registries.

## Verified Bugs And Trap Classes

These are not speculative refactor concerns. They were verified against the current code and should be fixed before or at the very start of the registry infrastructure work.

### A1. Param Fingerprints Split On Equivalent Signed Zero

> **Status at `ba0f962`: FIXED** (`f8c933e`). `param_program_chain.py:828-836` canonicalizes `a`/`b`/`args`/`args_im`; re-ran all four spellings → one fingerprint `840d5372…`; nonzero values stay distinct; `args_im` lane covered; pinned by `test_param_program_chain.py:217`. Surviving wart: `.17g` display formatting only (cosmetic).

Current behavior:

```text
legacy(rtheta,both,both,0)
legacy(rtheta,both,both,-(0))
legacy(rtheta,both,both,0*-1.0)
```

compile to three different Param fingerprints even though they are mathematically equivalent.

Cause:

- `lambda/coeff_program_chain.py::_token` canonicalizes numeric zero through `_canonical_zero`.
- `lambda/param_program_chain.py::_token` does not canonicalize `a`, `b`, `args`, or `args_im`.

Impact:

- Identical Param math can produce multiple cache keys and artifact identities.
- This is a live fingerprint bug, not a cosmetic formatting issue.

Fix:

- Port the Coeff `_canonical_zero` behavior into Param token construction.
- Apply it to scalar fields and all numeric arg lanes.
- Also fix `_legacy_tokens`: real-typed args currently populate `args_im` from `expr.value.imag` for every arg. For real args, the imaginary lane should be forced to canonical `0.0` like Coeff does, not whatever signed-zero expression happened to produce.
- Add a Param fingerprint test proving `0`, `-(0)`, `-0.0`, and folded zero expressions hash identically.

Secondary cosmetic cleanup:

- Param source formatting currently uses `.17g` where Coeff uses `repr()`, so Param can emit noisy values such as `0.10000000000000001`. This is not a fingerprint split, but it is a source readability wart.

### A2. Coeff Frontend Can Crash On Partial Registry Vocab

> **Status at `ba0f962`: PARTIAL.** The documented guard is applied verbatim (`js/07-transform-catalogs.js:290`, node-repro'd) — but a vocab missing `ctCatalog` still crashes module load at the unguarded `Object.entries(_coeffRegistryVocab.ctCatalog)` (`js/07:240`), and the only vocab-absence test is null-vocab, not a partial object.

Current code:

```js
const defs = (_coeffRegistryVocab && _coeffRegistryVocab.programParamDefs[name]) || [];
```

This guards only `_coeffRegistryVocab`, not `_coeffRegistryVocab.programParamDefs`. A partial or malformed vocab object can throw at module load.

Fix:

```js
const defs = (((_coeffRegistryVocab || {}).programParamDefs || {})[name]) || [];
```

Add a frontend test for a partial Coeff vocab object.

### A3. `round(1, 2)` Uses Complex Multiplier Plus Optional `andy`

> **Status at `ba0f962`: FIXED, body below is as-found.** The overlap is resolved by call form: `poly = round(poly, 1, 2)` → multiplier 1+0i, andy=2; `legacy(round, poly, poly, 1, 2)` → packed real lanes (multiplier 1+2i, andy 0) via `_legacy_round_real_lanes_args` (`coeff_program_chain.py:1446,1469`). Both signatures named in the registry (`complex_multiplier`, `packed_real_components_with_andy`); pinned by `test_coeff_program_drift.py:533,568` + chain round tests. The inverse display no longer collapses; `coeff_source_text_from_chain` deliberately preserves the `legacy(...)` spelling (fingerprint-preserving), not the semantic respelling this section proposed.

Current behavior:

```text
legacy(round, poly, poly, 1, 2)
```

is interpreted as:

```text
multiplier = 1
andy = 2
```

not as two real multiplier lanes. The inverse display collapses to `["round", "2"]`.

Cause:

- `_compat_signature_args` takes the first matching signature.
- `round` has overlapping historical shapes: one semantic shape is complex multiplier plus trailing `andy`; one old packed shape is two real multiplier lanes plus optional `andy`.
- Treating the packed real-lane shape as semantic source is the mistake. It is a wire/legacy compatibility shape.

Decision:

- The semantic source signature is `round(src, multiplier: complex = 1+0i, andy: real = 0)`.
- `round(poly, 1+2i)` means multiplier `1+2i`, default `andy=0`.
- `round(poly, 1+2i, 0.5)` means multiplier `1+2i`, `andy=0.5`.
- `round(poly, 1, 2)` means multiplier `1+0i`, `andy=2`.
- Old packed real-lane rows such as `legacy(round, poly, poly, 1, 2, 0.5)` are compatibility-shim inputs and should regenerate as semantic source `round(poly, 1+2i, 0.5)`.

Do not let additional overlapping `arg_counts` / `andy_arg_counts` appear silently. If an old wire shape overlaps a semantic optional-arg shape, document it as compatibility-only and add a round-trip/fingerprint test.

### A4. Param `legacy(name, src, tgt, ...)` Does Not Validate At Parse Time

> **Status at `ba0f962`: FIXED** for everything scoped here — all five repros return coded diagnostics at parse (`unknown_legacy_transform`, `bad_selector`, `bad_arity`, incl. bare shortcuts like `crd(5,9)`), via `_validate_legacy_source_entry` (`param_program_source.py:112-161`), pinned at `test_param_program_source.py:45`. Coeff `roots` int/enum/extra-arg is also coded at parse now (`bad_native_transform`). Residue moved to Open #2 (`inv_t_plus_2` arity drift), F5 (blanket under-arity), and F3/F8 (coeff raise sites still mostly `source_error`).

Current behavior:

```text
legacy(nope,both,both)
legacy(unit_circle,,both)
legacy(unit_circle,none,both)
legacy(unit_circle,both,nope)
```

parse with zero diagnostics, then fail later in the chain compiler as bare `RuntimeError`.

Coeff already validates registry names and selectors earlier.

Fix:

- Validate Param legacy transform name at source-parse time.
- Validate `src` and `tgt` against the selected profile.
- Emit stable diagnostic codes such as `unknown_legacy_transform` and `bad_selector`.
- Keep saved-chain round-trip compatibility: any name reachable from saved legacy chain rows must still serialize to source and compile back.

Additional confirmed validation gaps:

- Param bare shortcuts have the same problem, not only explicit `legacy(...)`. Examples such as `crd(5,9)`, `legacy(square,pop2,push2)`, and `legacy(zzznope,both,both)` parse with zero diagnostics and fail later or compile into surprising compatibility forms.
- Param under-arity can silently compile by filling defaults. `lss(1,2)` parses and compiles, even though the registry declares more runtime args. If reduced arity is intended for compatibility, it must be declared in `compat_forms` / `compat.variable_arg_counts`, not accepted accidentally.
- Param trailing numeric default `0.0` can fail because default filling passes a numeric zero into expression compilation. `legacy(hrt,both,both)` fails as `legacy(hrt) arg 1: param expression is empty`. Defaults passed through expression compilation must be stringified or handled as numeric literals.
- Coeff has the inverse problem: source parsing often accepts invalid native-transform arg shapes and leaves int/enum/arity failures to the chain compiler. `poly = roots(poly,2.7,lo); emit` parses clean and later raises an uncoded integer error. `poly = roots(poly,1,2,3); emit` compiles by treating the extra arg as `andy`.
- Coeff source errors currently collapse to the default `source_error` code because `CoeffProgramSourceError` raise sites do not pass specific codes. The diagnostics have locations, but frontend logic cannot distinguish unknown function, arity, selector, int/enum, or assignment errors by code.
- Empty source is asymmetric: Coeff empty source compiles to a no-op fingerprint, while Param reports `empty_source`. Decide whether this is intentional and pin it with tests.

### A5. Saved Param Macros Do Not Honor Source Text Precedence

> **Status at `ba0f962`: FIXED** (`f8c933e`). Shared resolver has the param branch (`program_compile_helpers.py:93-97`); live stub-S3 repro returns the source-derived chain; pinned by `test_saved_program_source_precedence.py:94`.

`lambda/program_compile_helpers.py::read_saved_program_source_chain` gives Coeff saved programs source precedence, but Param saved-program macros still read the saved `chain`.

Impact:

- Coeff and Param no longer share the same source-authoritative behavior for saved macros.
- A saved Param program with `source_text` and stale `chain` can expand through the stale chain.

Fix:

- Extend source-text precedence to Param saved programs.
- Add a macro resolver test where payload `source_text` and `chain` intentionally differ; Param and Coeff should both use the source.

### A6. Param Chain-To-Source Serialization Is Unguarded And Can Persist Unparseable Source

> **Status at `ba0f962`: FIXED.** `param_source_text_from_chain` now verifies/falls back/warns (`param_program_source.py:342-408`); the doc's `[['const','1','2'],…]` chain now emits `p1 = (1)+(2)*1j` which reparses fingerprint-identically; v2 fingerprints hash only the execution spec. Pinned at `test_param_program_source.py:103,123`.

`param_source_text_from_chain` has no equivalent of Coeff's source-preservation guard.

Confirmed behavior:

```python
param_source_text_from_chain([["const", "1", "2"], ["emit", "p1"]])
```

returns:

```text
const(1, 2)
emit_p1
```

but `parse_param_program_source` rejects that source because `const(value)` takes exactly one expression.

Impact:

- `translate_param_from_old` can persist unparseable `source_text` into the v2 payload.
- That unparseable `source_text` is included in the v2 fingerprint.
- Param Populate/detail paths and PDF generation can reconstruct source through the same unguarded serializer.
- This is a one-way migration trap: the migrated payload can contain source the system cannot reparse.

Fix:

- Add a Param equivalent of Coeff's `_source_text_preserves_compiled_chain`.
- `param_source_text_from_chain` must either return fingerprint-preserving, reparseable source or fall back to a safe representation with an explicit warning/error marker.
- `translate_param_from_old` must not hash unparseable synthesized source into the v2 fingerprint.
- Add tests for chain -> source -> parse -> compile round-trip on representative legacy, const, emit, stack, and macro-safe chains.

### B1. Param `compat` Is Not Runtime-Authoritative

> **Status at `ba0f962`: FIXED, one residue.** Loader retains normalized `compat`; all six families read via registry accessors at runtime (`param_program_chain.py:343-368`); all six drift-pinned (`test_param_program_drift.py:289-301`) plus C-drift checks. Residue: `_VARIABLE_LEGACY_ARG_COUNTS` is still production-read by `param_program_source.py` with hardcoded overrides — the source of the new `inv_t_plus_2` drift (Open #2); target_first/last are not yet derived from the index map.

`lambda/param_legacy_registry.json` has `compat`, and `lambda/gen_param_vocab.py` emits it, but `lambda/param_program_chain.py::_load_legacy_registry` strips it. The Param compiler still reads private Python constants:

- `_LEGACY_TARGET_ARG_INDEXES`
- `_LEGACY_TARGET_FIRST_CHIPS`
- `_LEGACY_TARGET_LAST_CHIPS`
- `_LEGACY_DITHER_TARGET_FIRST_CHIPS`
- `_VARIABLE_LEGACY_ARG_COUNTS`
- `_REDUNDANT_LEGACY_TARGET_ARG_NAMES`

Existing tests pin only part of this relationship. They cover `target_arg_indexes`, `independent_targets`, and `variable_arg_counts`; they do not cover `target_first`, `target_last`, or `dither_target_first`.

The current state is worse than missing drift assertions:

- `target_first`, `target_last`, and `dither_target_first` are registry data, but the generator does not emit them.
- The Param compiler never loads `compat`, so those three registry fields are decorative at runtime.
- `_LEGACY_TARGET_FIRST_CHIPS` and `_LEGACY_TARGET_LAST_CHIPS` re-encode facts already present in `_LEGACY_TARGET_ARG_INDEXES`.

There is also a fourth runtime copy in C:

- `lambda/sweep_cli.c::pt_is_targetable_independent` hardcodes independent-target function names.
- `lambda/sweep_cli.c` hardcodes `rtheta` target index behavior.
- `lambda/sweep_cli.c` hardcodes independent target extraction for target-first functions.

For legacy Param chains, the C executor is the final runtime authority. If it drifts from Python/registry behavior, tests that only inspect generated vocab or Python lowering will miss the production behavior.

Fix:

- Make runtime Param loader retain normalized `compat`.
- Make Param compiler read target/variable compatibility behavior from the registry-loaded data.
- Until then, add drift tests for all six compat families so private constants cannot drift silently.
- Add C-vs-registry drift checks mirroring the existing enum/selector guard style.
- Derive target-first/target-last sets from the target-arg index map where possible instead of maintaining three Python copies.

### B2. Coeff Vocab Emits Dead Or Contradictory Runtime Fields

> **Status at `ba0f962`: PARTIAL.** `supportsAndy` removed (pinned absent). `fnIndexByName`/`effectiveArgs`/`compatSignatures`/`names` still emitted with zero JS consumers — now drift-pinned as generated data rather than untested. `pow` still declares 4-real `args` + 2-complex `ui.params`, but the shapes are now formally related via named compat signatures with C packing parity (the "documented exception" option, not args normalization).

`lambda/gen_coeff_vocab.py` emits these fields:

- `fnIndexByName`
- `supportsAndy`
- `effectiveArgs`
- `compatSignatures`

Current frontend JS does not consume them. Meanwhile, `pow` has three shapes:

- raw registry `args`: four real lanes
- `ui.params`: two complex display/source params
- `compat_signatures`: wire-compatible packing

Fix:

- Do not treat emitted-but-unconsumed fields as authoritative frontend API.
- During Phase 5, either normalize `pow.args` to the two-complex display/source shape if M3/wire gates prove it safe, or keep a narrow documented `pow` exception.
- If dead generated fields are removed, gate with generated-vocab drift tests and frontend tests.

Additional false-authority data:

- Coeff `effective_args` is built by the runtime loader and mirrored into generated vocab, but the Python compiler does not read it for lowering. It is tested as generated data, not used as execution authority.
- `coeff_vocab_js.js` `names` has no frontend consumer.
- `solve_score_vocab_js.js` currently emits fields such as `sourceNames`, `quantilePercentRange`, and `lagDepths` with no JS consumers.

These are not all live bugs, but they are the same "looks authoritative but is not" pattern. Either attach consumers/tests to them or remove them from the public generated payload.

### C. Truthiness Fallbacks Hide Registry Data Bugs

> **Status at `ba0f962`: PARTIAL.** Fixed: the three `compat.get(...) or chain._*` generator fallbacks (gone — no chain import allowed in `registry_common`), `ui.get("categories") or {}` (now `extract_category_meta`, hard-fails), `int(version or 0)` in live loaders (`require_registry_version`), `categoryMeta || _ptCategoryMeta` (identifier deleted). Still present: `arg_spec.get("choices") or _ENUM_ARG_VALUES` (`coeff_program_chain.py:211`), `_profileSelectorChoices` `.length` fallback (`js/07:340-342`), `selectors.get(name) or []` (`program_source_core.py:91`), title/help display coalescing. No truthiness lint was added.

Examples:

- `compat.get("independent_targets") or chain._REDUNDANT_LEGACY_TARGET_ARG_NAMES`
- `compat.get("target_arg_indexes") or chain._LEGACY_TARGET_ARG_INDEXES`
- `compat.get("variable_arg_counts") or chain._VARIABLE_LEGACY_ARG_COUNTS`
- `arg_spec.get("choices") or _ENUM_ARG_VALUES`
- `str(arg.get("title") or arg.get("help") or "")`
- `int(payload.get("version") or 0)`
- `_profileSelectorChoices`: `Array.isArray(values) && values.length ? values : fallback.slice()` treats an intentional empty selector list as missing and silently restores hardcoded fallback selectors.
- Generated category metadata uses `ui.get("categories") or {}` / `payload.get("category_meta") or {}`. Emptying registry category metadata yields `{}`, and the frontend fallback is then dead because `{}` is truthy.

Not every `or 0` or `or ""` is a bug, but registry/config code must distinguish missing from deliberately empty/zero values.

Rule:

- For registry values, use explicit presence checks: `key in mapping`, `value is None`, or schema validation.
- Add a narrow lint/test that forbids truthiness fallback from registry data to compiler constants.
- For selector choices, `[]` must remain a valid intentional value unless the schema says otherwise.

### D. `andy` And Stack Arg Packing Are Hardcoded In Multiple Places

> **Status at `ba0f962`: PARTIAL.** The two registry/generator bullets are resolved exactly as prescribed: `supports_andy` deleted (schema-test-pinned), `ANDY_PARAM` deleted, andy flows from `shared_optional_args` through the generic optional-args path. Still hardcoded by fn_index: `_split_native_transform_andy` arities (`coeff_program_source.py:268-276`; registry `andy_arg_counts` exists but is unread there) and `_max_native_transform_stack_arg_count` (`coeff_program_chain.py:1571-1580`) — both now drift-pinned though not registry-derived.

Confirmed duplicate sources:

- `supports_andy: true` appears on every Coeff registry function, so it carries almost no information.
- `lambda/gen_coeff_vocab.py` appends a hardcoded `ANDY_PARAM`.
- `lambda/coeff_program_source.py::_split_native_transform_andy` hardcodes fn-index arities.
- `lambda/coeff_program_chain.py::_max_native_transform_stack_arg_count` hardcodes packed stack arg counts for `linear`, `exp`, `pow`, and `round`.

Architectural correction:

- `andy` is not a registry capability flag. It is just an argument accepted by Coeff transforms.
- The semantic registry is the source of truth for the current language. It should describe actual callable arguments and their types/defaults/help.
- Historical mistakes, such as storing `andy` in a separate compiled-token field or accepting old trailing-`andy` arity shapes, belong in a compatibility shim.
- Do not pollute the semantic registry with compatibility artifacts such as universal `supports_andy`, special generated `ANDY_PARAM`, or parser-only `andy_arg_counts` masquerading as function schema.

Fix:

- In the infrastructure refactor, do not add more `andy` special cases.
- Split the model into two layers:
  - Semantic function registry: `andy` appears only as a normal declared optional argument where the current source language accepts it.
  - Compatibility shim: old source forms, trailing-`andy` splitting, separate token field packing, and old wire arities are translated to/from the semantic arg model.
- Derive packed stack arg counts from compatibility metadata where available, but keep that metadata explicitly in the compatibility layer, not as sacred function schema.
- Delete universal `supports_andy` once the semantic argument list and compatibility shim can express the current behavior without it.

### E. Coeff Alias / Opcode Split Produces Equivalent-Looking Different Programs

> **Status at `ba0f962`: STILL-PRESENT by design; the prescribed fix landed.** Repro unchanged (`legacy(rev,cf,poly)` op 9 vs `poly = rev(cf)` op 29, distinct fingerprints; `scale100` legacy-only). The registry now structurally encodes `aliases` vs `chain_only_aliases:["scale100"]`, pinned by `test_coeff_program_drift.py:372-407` + `test_coeff_program_chain.py:37`.

Examples:

- `scale100` is accepted as a legacy alias but rejected as a direct source function.
- `legacy(rev, cf, poly)` lowers to legacy opcode 9.
- `poly = rev(cf); emit` lowers to native-transform opcode 29.
- Those forms produce different fingerprints.

This may be intentional compatibility behavior, but it is a trap for hand-authored source.

Fix:

- Document which aliases are chain-only versus source aliases.
- Add tests pinning the intended behavior.
- Do not silently widen chain-only aliases into source aliases without a fingerprint/compatibility decision.

### F1. Empty Program Policy Is Inconsistent And Falsy Fallbacks Can Resurrect Legacy Transforms

> **Status at `ba0f962`: PARTIAL.** Fallback-resurrection half FIXED: `explicit_program_chain_for_run` (`pipeline_programs.py:100-112`) used by all three handlers, each pinned by a handler test. Policy half remains: param empty rejected at source/save but accepted by compiler/run; coeff accepts empty everywhere.

Current compiler behavior:

- `compile_param_program_chain([])` produces a valid no-op fingerprint.
- `compile_coeff_program_chain([])` produces a valid no-op fingerprint.

Current source/save/run behavior is inconsistent:

- Param source rejects empty source as `empty_source`.
- Param saved-program compile rejects an empty chain as invalid.
- Coeff source accepts empty source as a no-op.
- Coeff saved-program compile accepts an empty chain.
- Compute paths often use `params.get("param_program_chain") or param_transforms_to_program_chain(...)`, so an explicit empty chain can be treated as absent and replaced by legacy transforms.

Confirmed affected paths:

- `lambda/handler_compute_plan.py`
- `lambda/handler_compute_preview.py`
- `lambda/handler_coeffgen.py`

Fix:

- Decide one empty-program policy per profile.
- If empty program is valid, use explicit key-presence checks instead of truthiness checks.
- If empty program is invalid, reject it consistently at compiler, source, save, and run boundaries.
- Add tests that explicit `param_program_chain: []` / `coeff_program_chain: []` never falls back to legacy transform arrays.

### F2. `pipeline_programs.py` Is Another Runtime Compatibility Copy

> **Status at `ba0f962`: PARTIAL — now Coeff-only.** The `_PARAM_*` private copies are deleted; param reads `legacy_registry()["compat"]` (`pipeline_programs.py:169-184`), drift-pinned. Coeff exp/round repack remains hand-coded (`:251-269`) but behavior drift-pinned (`test_coeff_program_drift.py:464`).

`lambda/pipeline_programs.py` translates old legacy transform arrays at compute/storage/PDF boundaries.

It has private copies of Param compatibility facts:

- `_PARAM_TARGET_FIRST_TRANSFORMS`
- `_PARAM_TARGET_LAST_TRANSFORMS`
- `_PARAM_DITHER_TARGET_FIRST_TRANSFORMS`

It also has private Coeff compatibility repacking:

- `exp(a,b[,andy])` old real-lane form -> packed complex multiplier plus zero offset.
- `round(a,b[,andy])` old real-lane form -> packed complex multiplier.

These facts overlap with `param_legacy_registry.json` `compat` and Coeff `compat_signatures`, but are not derived from either.

Impact:

- The registry can be correct while run-boundary translation is wrong.
- PDF/source reconstruction and compute plan/preview can disagree with runtime compiler rules.
- Tests that cover compiler-only behavior can miss boundary translation drift.

Fix:

- Drive Param transform-array translation from registry `compat`.
- Drive Coeff old-form repacking from `compat_signatures`, or add a narrow explicit compatibility map generated from the registry.
- Add drift tests comparing `pipeline_programs.py` translation sets to registry data.

### F3. Compiler Diagnostics Are Not Structured

> **Status at `ba0f962`: STILL-PRESENT.** Both chain compilers' strict=False diagnostics carry `{level,message}` only; end-to-end `/compile-coeff-program-source` repro with `poly = pop()` returns no code/line/column.

Even if parse-time errors get better codes, many user-visible failures still originate in chain compilers.

Current behavior:

- `compile_param_program_chain(..., strict=False)` catches exceptions and returns `{"level": "error", "message": ...}` with no `code`, `line`, `column`, or structured context.
- `compile_coeff_program_chain(..., strict=False)` does the same.
- `strict=True` raises the original `RuntimeError`.

Examples:

- Param stack underflow.
- Param final stack depth.
- Param macro errors.
- Param legacy selector/arity defaults that escape parse.
- Coeff int/enum validation.
- Coeff final stack depth.
- Coeff native-transform stack arg validation.

Fix:

- Introduce profile compiler diagnostic helpers that attach stable `code` values.
- Preserve strict behavior where needed, but strict=False API responses must return structured diagnostics.
- Source compile endpoints should not lose codes after parse succeeds and compiler validation fails.

### F4. Param Default Handling Bug Is Broad

> **Status at `ba0f962`: FIXED** (`param_program_chain.py:988` stringifies defaults). All 13 zero-default names compile via `legacy(name, both, both)`; pinned at `test_param_program_chain.py:227` (hrt/rect — a registry-wide omitted-args probe test is still a good idea).

A registry-driven probe found 13 Param transforms fail when called with defaulted args because numeric default `0.0` is passed to expression compilation and treated as an empty expression:

```text
hrt
asp
rply
rect
t1radd
t1iadd
t2radd
t2iadd
radd
iadd
cadd
ldth
scdth
```

Fix:

- Treat numeric defaults as literals before expression compilation, e.g. stringify defaults or compile a literal expression object directly.
- Add a registry-driven test that every function with declared defaults can compile with omitted args if omitted args are intended to be legal.

### F5. Param Reduced-Arity Behavior Is Accidental

> **Status at `ba0f962`: PARTIAL.** `lss(1,2)` still silently compiles with defaults; the 0..N policy is now explicit in code (`param_program_source.py:99-109`) with coded `bad_arity` for over-arity, but it is not registry-declared — and the same block hardcodes the `inv_t_plus_2` set that drifts from the registry (Open #2).

A registry-driven probe found many arg-bearing Param transforms compile with omitted args by filling defaults. This may be desired for compatibility for some transforms, but today it is not declared consistently.

Fix:

- Define a reduced-arity policy.
- If a function supports reduced arity, declare it in registry compatibility metadata.
- If it does not, parse/lower should emit a coded arity diagnostic.
- Add a test that every accepted reduced-arity form is registry-declared.

### F6. Storage And Compute Macro Resolution Can Disagree

> **Status at `ba0f962`: FIXED.** Same-payload repro through both paths agrees; compute-side pinned (`test_saved_program_source_precedence.py:94`). Residue: storage keeps a parallel implementation (`handler_storage.py:468-501`) rather than calling the shared resolver, and has no storage-side precedence test.

`handler_storage.py` has a Param macro resolver that honors Param `source_text`. The shared compute resolver in `program_compile_helpers.py` honors source text only for Coeff.

Affected compute-facing paths include:

- `handler_coeffgen.py`
- `handler_compute_plan.py`
- `handler_compute_preview.py`

Impact:

- The same saved Param macro can expand from source in storage compile/fetch paths but from stale chain in compute/preview paths.

Fix:

- Move the storage Param source-precedence behavior into `program_compile_helpers.py`.
- Make storage use the same helper or share a common lower-level implementation.
- Add tests against both storage resolver and shared compute resolver.

### F7. Root Source Precedence Is Field-Name Inconsistent

> **Status at `ba0f962`: STILL-PRESENT, worse than documented:** `root_source_text_from_payload` (3-tier) now has **zero callers** — dead code advertising a precedence nothing honors — while `root_source_text_for_run` stays 2-tier. Reconcile or delete.

`root_source_text_from_payload` accepts top-level `source_text`, but `root_source_text_for_run` accepts only:

- `root_program_source_text`
- `root_program.source_text`

If a v2-style root program payload is passed as run params with top-level `source_text`, runtime resolution ignores it.

Fix:

- Decide whether top-level `source_text` is accepted for root run params.
- If yes, add it to `root_source_text_for_run`.
- If no, add a test documenting that only `root_program_source_text` is accepted at run boundaries.

### F8. Root And Solve-Score Diagnostic Codes Are Also Sparse

> **Status at `ba0f962`: PARTIAL — root FIXED, solve-score half-fixed.** Root now emits distinct codes (`unknown_transform`, `bad_arity`, `bad_arg_type`, `bad_syntax`, `no_op_assignment`, …). Solve-score chain-compile errors are now classified (`bad_quantile`, `stack_error`, `bad_numeric_arg`, `unknown_operator`; pinned `test_solve_score_program_source.py:99`) but still report line 1/col 1, and some parse-stage raises still default to `source_error`.

Root and solve-score source compilers are safer than Param in serializer behavior, but most source errors still use the default `source_error` code.

Impact:

- Frontend or API clients cannot reliably switch on unknown transform, bad arity, bad selector/source, bad metric, or stack errors.

Fix:

- Lower priority than Param/Coeff data-corruption paths.
- Add stable codes as part of the broader source/diagnostic cleanup.

### F9. Existing Drift Tests Are Good But Incomplete

> **Status at `ba0f962`: MOSTLY RESOLVED.** Landed since: all six param compat families pinned; `pt_is_targetable_independent` C-drift; pipeline target-first/last/dither; coeff exp/round translator; three C-default/packing drift tests (`67ff066`). Remaining: no direct scrape of `pt_target_value` indices; solve-score partition tables unpinned (G6); `test_registry_schema.py` un-gated (Open #1).

Existing Param drift tests cover:

- C/Python opcode enum alignment.
- C/Python selector enum alignment.
- profile source grammar alignment.

They do not cover:

- C function-name compatibility sets such as `pt_is_targetable_independent`.
- C `rtheta` target-index behavior.
- C dither target-index behavior.
- `pipeline_programs.py` Param target-first/target-last/dither sets.
- `pipeline_programs.py` Coeff `exp`/`round` legacy real-lane repacking.

Fix:

- Add registry-vs-C drift tests for function-name compatibility sets.
- Add registry-vs-pipeline drift tests for legacy transform-array translators.

### G0. There Is No Shared Fingerprint/Canonicalization Contract

> **Status at `ba0f962`: PARTIAL — the leaks are fixed, the contract still doesn't exist.** Fixed rows of the table below: solve-score is now repr+zero-fold (not `%g`); v2 excludes source text; root regeneration is repr/lossless; param no longer leaks signed zero. Remaining: six per-module `_format_number` copies, two float policies (`.17g` param/coeff vs `repr` root/solve), two hash algos (sha1 v1 vs sha256 v2), and `format_numeric_literal` in `program_source_core.py` used by only one kind.

The newer findings are not isolated one-off bugs. The same missing contract appears in every program kind and layer.

Current fingerprint/canonicalization shape:

```text
kind         identity payload                         float policy          signed zero
param v1     json.dumps(tokens) + sha1                repr / mixed paths    leaks in args/args_im
coeff v1     custom spec string + sha1                .17g                 canonicalized
root v1/v2   json.dumps(execution_spec, chain)+sha256 repr after coercion   mostly canonicalized
solve v1     json.dumps(spec, metrics)+sha256         %g                   collides
v2 param     json.dumps(execution_spec, source_text)  mixed + raw source    spelling leaks
v2 coeff     json.dumps(execution_spec, source_text)  mixed + raw source    spelling leaks
v2 root      compiled execution fingerprint           root canonical path   excludes source_text
root source  regenerated text                         %g                   lossy round-trip
```

Impact:

- Each kind reinvents numeric canonicalization.
- v1 and v2 do not share a clear "semantic identity only" rule.
- Python source, Python chain, v2 migration, and C execution can disagree about what is canonical.
- The current drift tests guard many enum/id layers, but not enough value/default/packing/fingerprint layers.

Fix:

- Define one canonicalization contract for all program fingerprints: semantic execution data only, canonical floats, signed zero folded, no raw display/source text.
- Keep hash algorithm changes explicit and versioned where required.
- Add cross-kind regression tests for "equivalent spelling hashes the same" and "distinct numeric values hash differently".
- Add source regeneration tests: chain -> source -> parse -> compile must preserve semantic identity for every program kind that exposes regenerated source.

### G1. Solve-Score Fingerprints Can Collide On Distinct Numeric Programs

> **Status at `ba0f962`: FIXED** (`f8c933e`). `solve_score_chain.py:378` is now repr+zero-fold (not the `.17g` this section prescribed — equally collision-free); distinct omegas → distinct fingerprints; pinned `test_solve_score_chain.py:96-131`. The cache-invalidation callout still applied (numeric solve-score fingerprints changed).

This is the highest-severity new finding because it is an under-split, not an over-split.

Current behavior:

```text
omega_cosine(1.234567890123)
omega_cosine(1.234569999)
```

both format through `lambda/solve_score_chain.py::_format_number` as `1.23457` because it uses `%g`.

Verified smaller examples:

```text
0.30000001 -> 0.3
0.30000009 -> 0.3
```

Impact:

- Two different solve-score programs can produce the same `program_spec`.
- The fingerprint then collides.
- A cached render can be reused for the wrong scoring program.
- This affects any non-integer solve-score numeric argument: omega, phase, const, `weighted_sum`, `ema`, `pow`, sawtooth multiplier, etc.

This is worse than the Param signed-zero bug. Param currently over-splits equivalent math into multiple cache keys. Solve-score can under-split distinct math into one cache key.

Fix:

- Change solve-score numeric canonicalization to the Coeff policy: `.17g` with signed zero folded to `0.0`.
- Add a dedicated helper rather than using ad hoc `%g`.
- Add collision tests for close-but-distinct floats and signed zero.
- Call out cache invalidation: this changes solve-score program specs/fingerprints for numeric programs.

### G2. Root Program Cap Diverges Between Profile, Python, And C

> **Status at `ba0f962`: PARTIAL.** Python enforces `MAX_ROOT_TRANSFORMS=16` with coded `root_chain_too_long` (`root_program_source.py:27,309-321`) — the silent-mangling core is gone. Residue: profile still declares `program_tokens: 64`, and no drift test ties profile ↔ Python ↔ C (`MAX_RT_CHAIN`).

Current state:

- `lambda/program_profiles.json` declares root `program_tokens: 64`.
- `lambda/root_xforms.h` declares `#define MAX_RT_CHAIN 16`.
- `lambda/root_program_source.py::compile_root_program_chain` enforces no cap.

Verified behavior:

- A 25-transform root chain compiles cleanly in Python with `token_count == 25`.
- The C executor is capped at 16 transforms.

Impact:

- A request can compile and fingerprint a 17+ transform root program as if it is valid.
- The native executor can only consume the first 16 rows.
- That is silent program mangling, not just a diagnostic defect.

Fix options:

- Immediate safe fix: enforce the native cap of 16 in `compile_root_program_chain` with a structured diagnostic.
- Better long-term fix: generate the C cap and profile cap from one shared source.

Tests:

- A 16-transform root chain compiles.
- A 17-transform root chain fails before native execution with a stable code.
- Profile cap and C cap are pinned together by a drift test.

### G3. V2 Param/Coeff Fingerprints Include Raw Source Text

> **Status at `ba0f962`: FIXED** (`f8c933e`). `_v2_fingerprint` strips `source_text`/`source_display`/`display`/`expanded_display` (`program_v2_translate.py:135-141`); four zero-spellings through coeff v2 → identical fingerprints; pinned `test_program_v2_migration.py:96,205`.

Current behavior:

- `program_v2_translate.py::translate_param_from_old` hashes:

```json
{"execution_spec": "...", "source_text": "..."}
```

- `program_v2_translate.py::translate_coeff_from_old` does the same.
- `translate_root_from_old` does not: it uses the compiled root fingerprint from `root_program_source.py`, which hashes canonical execution data.

Impact:

- Equivalent Param/Coeff programs can split v2 fingerprints on spelling, comments, whitespace, or inverse-source formatting.
- Coeff v2 can regress signed-zero immunity that Coeff v1 had if the raw source text differs while execution is equivalent.
- This makes v2 fingerprints less canonical than v1 in exactly the place intended to be the new stable representation.

Fix:

- Do not include raw `source_text` in v2 fingerprints.
- Hash only canonical execution data: `execution_spec` and any other canonical semantic payload needed by that program kind.
- Keep `source_text` in the payload for display/editing, but not in the cache identity.
- Add tests proving two equivalent spellings with identical execution specs produce one v2 fingerprint.

### G4. Solve-Score Diagnostics And `strict=False` Lose Information

> **Status at `ba0f962`: PARTIAL.** The silent proximity substitution is fixed — strict=False now returns the fallback **with** `diagnostics` + `degraded: True`. Remaining: chain-stage errors still report line 1/col 1, and some parse-stage raises default to `source_error`.

Current behavior:

- Many `SolveScoreProgramSourceError` raise sites use the default `source_error` code.
- Chain-stage errors such as q-range, metric/source compatibility, omega finiteness, and stack depth are deferred to whole-chain compile and can be reported at line 1 / column 1 instead of the statement that caused them.
- `compile_solve_score_program_source(..., strict=False)` returns diagnostics with a fallback proximity program.
- `solve_score_pipeline_programs.py::solve_score_program_for_run(..., strict=False)` then recompiles the fallback chain and returns a clean proximity program without diagnostics.

Impact:

- Invalid solve-score source can silently execute as the default proximity score when strict mode is disabled.
- API callers using the non-strict path lose the signal that the user program failed.
- Error handling cannot distinguish bad metric, bad source, bad quantile, stack underflow, or arity problems by code.

Fix:

- Do not discard diagnostics when returning fallback programs.
- If fallback execution is intentionally supported, return diagnostics alongside the fallback and mark the program as degraded.
- Add statement-local codes for metric/source/quantile/operator/stack errors.
- Audit all exported non-strict APIs for silent substitution behavior.

### G5. Root Source Boundary Errors Are Bare And Some Rows Prefer `fn_index` Over `name`

> **Status at `ba0f962`: FIXED (one residue).** Malformed `root_transforms` payloads → structured `RootProgramSourceCompileError` with diagnostics; name/fn_index mismatch rejected (`root_program_source.py:121-133`); distinct codes exist. Note: the "`root_chain_error` is unreachable dead code" claim is now stale — it is the live code for chain-shape errors. Residue: `root_source_text_from_payload` still dead (F7).

Current behavior:

- `root_program_for_run({"root_transforms": "5"})` raises a bare `RootProgramSourceError` with no `.diagnostics`.
- `root_program_for_run({"root_transforms": "not-json"})` does the same.
- This path is request-reachable from root-consuming handlers, so it is not just a test-only API defect.
- A dict row with disagreeing `name` and `fn_index` lets `fn_index` win:

```python
{"name": "rotate_roots", "fn_index": 5}
```

compiles to `invert_roots()`.

Impact:

- Request-facing bad root input escapes the structured diagnostic path.
- Mismatched persisted rows can execute a different transform than the visible `name` says.

Fix:

- Normalize malformed `root_transforms` through `RootProgramSourceCompileError` diagnostics at the run boundary.
- For dict rows, reject `name`/`fn_index` disagreement unless it is a narrowly documented legacy compatibility case.
- Add stable root diagnostic codes instead of defaulting nearly every semantic error to `source_error`.

Dead/false-authority cleanup:

- `root_source_text_from_payload` currently has no consumers. It advertises a source precedence order that the live run resolver does not honor.
- The `root_chain_error` fallback code is effectively unreachable while `RootProgramSourceError` defaults to `source_error`.

### G6. C Executor Defaults And Packing Are Runtime Authority But Not Registry-Gated

> **Status at `ba0f962`: PARTIAL — the drift tests landed** (`67ff066`): `dispatchPt` default lanes vs registry (incl. target-lane shift), `coeffLegacyApply` fallbacks vs registry, native packing special-cases vs `compat_signatures` — all pass. Remaining: C is still the runtime authority for omitted args (no Python-always-expands contract), and `solve_score.h:215,257` metric-partition/min-roots tables have no drift test.

The existing drift tests guard many enum/id layers, but they do not pin value/default/packing behavior across Python and C.

Confirmed Param C defaults:

- `lambda/sweep_cli.c::dispatchPt` hardcodes omitted-arg defaults for Param legacy transforms such as `spdl`, `lmc`, `rsc`, `lss`, `rply`, `star`, `rect`, `rrect`, dither transforms, and many scalar helpers.
- If `param_legacy_registry.json` `args[].default` changes, C behavior does not change unless Python has already expanded the args before native execution.

Confirmed Coeff C packing/defaults:

- `lambda/sweep_cli.c::coeffLegacyApply` hardcodes Coeff legacy defaults.
- `lambda/sweep_cli.c::execCoeffNativeTransform` uses `fn_index` checks for `linear`, `exp`, `round`, and `pow` packing.
- Those facts overlap with Coeff `compat_signatures`, but C does not read the registry.

Confirmed Solve-score C partitions:

- `lambda/solve_score.h::solve_metric_is_param_metric` duplicates the Python metric/source partition.
- `lambda/solve_score.h::solve_metric_min_roots` duplicates per-metric minimum-root requirements.
- Existing value-parity tests cover important compute paths, but this partition metadata itself is still another hand-maintained Python/C copy.

Impact:

- The registry can claim a default or packing rule that native execution ignores.
- Tests that only compare Python registry/generator behavior can pass while production native execution differs.

Fix options:

- Preferred runtime contract: Python must always expand defaults and pack args into the exact native rows before C sees them; add tests proving that contract.
- Add C-vs-registry drift tests for every default and packing rule that C still owns.
- Add C-vs-Python drift tests for solve-score metric partitions, not just metric enum/value parity.
- Longer term: generate C tables or headers from the registry instead of hand-maintaining ladders.

### G7. Solve-Score Generated Vocab Has Live Sections With Dead Subfields

> **Status at `ba0f962`: STILL-PRESENT.** `delta` and `legacy_alias` still emitted (`gen_solve_score_vocab.py:159-160,175-176`), zero JS readers.

Correction to avoid deleting live data:

- `stackSpecs` and `outputSpecs` are used by `js/07-transform-catalogs.js`.
- The suspicious fields are subfields such as `stackSpecs.*.delta` and `outputSpecs.*.legacy_alias`; targeted JS checks found no frontend readers for those subfields.

Fix:

- Do not remove `stackSpecs` or `outputSpecs`.
- Either consume `delta` / `legacy_alias` in Help/editor UI or stop emitting those subfields.
- Add generated-vocab consumer tests only for fields that are intended API.

### G8. Blank Source-Key Semantics Diverge Across Program Kinds

> **Status at `ba0f962`: STILL-PRESENT (mitigated).** Divergence intact (param/coeff blank→None, root→empty no-op, solve-score→raise), but param/coeff blank-as-absent is now documented as deliberate (ASL JSONPath defaults) and solve-score gained blank-key tests. Remaining ask: per-kind four-case boundary tests/contract.

Current behavior:

- Solve-score treats a present-but-blank source key as an error if there is no fallback chain/metric.
- Param, Coeff, and Root can silently treat blank source text as absent and fall back to chain, legacy arrays, or identity/no-op behavior depending on the boundary.

Impact:

- The same UI/API shape means different things across program kinds.
- A caller cannot reason about whether "field present but blank" means explicit empty program, invalid user input, or "use older fallback fields".
- This interacts with the empty-program/falsy fallback bug family in F1.

Fix:

- Define per-kind semantics for:
  - source key absent,
  - source key present but blank,
  - chain key absent,
  - chain key present as `[]`.
- Use key-presence checks rather than truthiness.
- Add run-boundary tests for all four program kinds.

### G9. Root Source Regeneration Is Lossy For Non-Round Numeric Args

> **Status at `ba0f962`: FIXED** (`f8c933e`). `root_program_source.py:80` `_format_number` is repr+zero-fold; `1.234567890123` round-trips fingerprint-identically; nearby values stay distinct. (No dedicated non-round corpus test was added — formatter fix only.)

Current behavior:

```python
root_source_text_from_chain([["rotate_roots", "1.234567890123"]])
```

emits:

```text
rotate_roots(1.23457)
```

Recompiling that source produces a different root program and a different fingerprint.

Verified examples:

```text
1.234567890123 -> rotate_roots(1.23457) -> 1.23457
0.30000001     -> rotate_roots(0.3)     -> 0.3
0.30000009     -> rotate_roots(0.3)     -> 0.3
```

Cause:

- `lambda/root_program_source.py::_format_number` uses `%g`.
- Root fingerprints use canonical float JSON after coercion, but generated source uses a lossy display formatter.
- Root has no source-preservation guard analogous to Coeff's chain-to-source guard.

Impact:

- Any code path that displays regenerated root source and later treats it as authoritative can change the root program.
- Root v2 migration stores `source_text` from `root_source_text_from_chain`; because root v2 fingerprints do not hash source text, identity is safe there, but editing/re-saving that source can change execution.

Fix:

- Change root source numeric formatting to a non-lossy canonical format, e.g. `.17g` with signed zero folded.
- Add a root chain -> source -> parse -> compile preservation test.
- Add a root source-generation guard or at least a test corpus for all registry transforms with numeric args.

### G10. V2 Translation Has Additional Empty-Field Fallback Bugs

> **Status at `ba0f962`: FIXED** (`f8c933e`). Key-presence-ordered handling in `program_v2_translate.py:229-247`; empty-chain solve-score translation now raises explicitly; pinned `test_program_v2_migration.py:265,305`.

Current behavior:

- `translate_root_from_old({"root_transforms": [], "chain": [["rotate_roots", "1"]]})` migrates the stale `chain` instead of preserving the explicit empty `root_transforms`.
- `translate_solve_score_from_old({"chain": []})` migrates to default `proximity + omega_cosine(1)`.
- `translate_solve_score_from_old({"chain": [], "metric": "spread"})` still migrates to default `proximity + omega_cosine(1)`, ignoring the supplied legacy metric.

Impact:

- V2 migration can change explicit empty programs into stale fallback programs or defaults.
- The migration path repeats the same truthiness bug family already present in request/run boundaries.
- This is especially risky because migrated metadata looks canonical afterward.

Fix:

- In v2 translation, treat explicit keys as authoritative even when their value is `[]`.
- For Root, `root_transforms: []` must migrate as empty, not fall through to `chain`.
- For Solve-score, decide whether empty chain is invalid or a deliberate default; do not silently inject default proximity when the payload explicitly provides an empty chain.
- If legacy metric fallback is supported, it must be explicitly keyed and tested.

Negative findings to avoid re-filing:

- `solve_score_chain_id()` did not collide for close omega strings in the tested original-chain path; the confirmed solve-score data-integrity bug is the `program_spec` / fingerprint collision.
- Legacy `param_transforms` / `coeff_transforms` C `MAX_CHAIN=16` truncation is mostly blocked for non-empty arrays by `rejectLegacyTransformChain`; do not re-file it as a current silent truncation without a path that bypasses that rejection.
- C `cfpv` has legacy truncation code for arrays longer than `MAX_CFPV`, but generated coefficient functions currently max at four params, so that truncation path does not look reachable today.

### G11. Removed Frontend Editors Still Have Live-Looking Dead Code

> **Status at `ba0f962`: FIXED** (by HEAD commit `ba0f962` itself, −735 lines). Zero refs to the removed picker IDs / `_ptCatalog` family; regression guard at `test_frontend_js.sh:67-77` (a denylist of removed IDs rather than the proposed full DOM cross-check); kept boundaries (`rt-add`, serializers) intact; frontend suite passes.

The visible old Param Transform and Coeff Transform chip editors are gone:

- `index.html` exposes Param Program text via `pp-source-text`, not `pt-add` / `pt-add-popup`.
- `index.html` exposes Coeff Program text via `cp-source-text`, not `ct-add` / `ct-add-popup`.
- Frontend tests already assert the old read-only Param/Coeff Program chip panels are removed (`pp-chips`, `cp-chips`, program chip tabs).
- Root transform chips are not part of this bug: `rt-add` and `palette-rt-add` still exist and are active UI.

But stale JS still references removed DOM:

- `js/08-chip-editors.js` still contains Param Transform picker code for missing `pt-add-popup`, `pt-add-btn`, and `pt-add`.
- `js/07-transform-catalogs.js` still contains Coeff Transform picker code for missing `ct-add-popup`, `ct-add-btn`, and `ct-add`.
- `js/12-deepzoom-boot.js` still installs outside-click handlers for missing `pt-add-popup`, `pp-add-popup`, `cp-add-popup`, and `ct-add-popup`.
- `js/08-chip-editors.js` still contains Param/Coeff Program picker scaffolding for missing `pp-add-popup`, `cp-add-popup`, `pp-add`, and insert buttons.
- `js/07-transform-catalogs.js` still carries static Param transform metadata (`_ptCategoryMeta`, `_ptInfo`, `_ptCatalogEnriched`) that mostly supports removed Param Transform UI.
- `js/06-popup-init.js` still defines `_ptCatalog`, which is now suspicious: it may still support legacy-chain-to-source compatibility, but it no longer backs a visible Param Transform picker.

This is the same bug flavor as the registry false-authority issues: code looks live and authoritative, but the UI entry point is gone. It creates review noise and makes future refactors preserve dead paths because tests assert the functions still exist.

Do not delete all `pt` / `ct` code blindly:

- Legacy artifact detail still displays `param_transforms` / `coeff_transforms` in the Results sidebar.
- Populate-from-old-artifact still needs to synthesize source from old chain-only payloads.
- `_serializeParamProgramChain()` / `_serializeCoeffProgramChain()` still support chain-only saved programs by generating text source.
- Compatibility serializers and boundary translators are live until all old saved payloads have source text or v2 data.

Dead-code boundary:

- Keep compatibility code that converts old saved `pt` / `ct` rows into source text.
- Remove or quarantine UI picker/display code whose only caller is a missing DOM control.
- Remove boot-time sync/outside-click handlers for missing controls.
- Replace tests that require dead functions to exist with tests that prove removed UI cannot regress and compatibility conversion still works through direct helpers.

Add a narrow missing-DOM-ID test:

- Parse static `getElementById("...")` calls from `js/*.js`.
- Compare with `index.html` IDs.
- Maintain a small allowlist only for IDs created dynamically by live render-artifact HTML, such as render action buttons and download menus created by `refreshRenderArtifacts()`.
- Do not allow removed picker IDs (`pt-add-*`, `ct-add-*`, `pp-add-*`, `cp-add-*`) onto that allowlist.

## H-Series — Deep-Dive Sweep At `35f06fb` (2026-07-02)

A fifth adversarial sweep targeting the least-audited surfaces: the recent fix commits themselves, the scalar-expression layer + C VMs, the persistence/editor round-trip layer, and the test suite's own integrity. Every severity-critical claim below was independently re-verified ([V] = repro run; [T] = traced to file:line). These are NEW findings, distinct from A–G.

### H1. Param Program Save Is Broken For Text-Authored Programs — [V] HIGH

`js/03-program-modals.js:887-890` posts only `{name, chain}` — no `source_text` — while `_portableParamProgramPayload` sets `chain: sourceText.trim() ? [] : chain` (`js/08-chip-editors.js:2003`) and the param editor is pinned to text mode (`js/08:35,113`). Net: every save posts `{name, chain: []}` → server 400 `"param program chain must be a non-empty JSON array"` (`handler_storage.py:553-554`). Coeff forwards source (`js/03:1239-1240`); solve-score got this **exact** fix in `c3680a6`; param was missed. **Fix:** forward `source_text` in the save payload (two lines, copy coeff's).

### H2. The v1/v2 Saved-Program Lifecycle Is One-Way — Edits Vanish, Deletes Resurrect — [V] HIGH

Fetch prefers the `v2/` key (`handler_storage.py:670-672,704-705,734-735`) but save writes only v1 (`:1667-1675`, coeff `:1816-1824`, solve-score `:1500-1508`) and delete deletes only v1 (`:1684-1687`, `:1867-1870`, `:1517-1520`). Verified with stub S3: save → migrate → edit → re-save → **fetch returns the stale pre-edit program**; delete → fetch still 200 (zombie served from `v2/`; list hides it); re-migrate → 409 `_MigrationConflict` (`:992-1001`) — the stale copy is unrepairable through the API. Migration is user-reachable UI (`js/03:493-521`). **Fix:** save must rewrite-or-delete the kind's v2 key; delete must delete both.

### H3. Littlewood Chip Rendering Throws ReferenceError — [V] HIGH

The `ba0f962` dead-code sweep deleted `_ctAndyIndex` (was `js/07:703`) but a live call survives at `js/09-render-orchestration.js:223` (`_coeffProgramVectorFormulaHtml`, littlewood branch), reachable from the read-only chip strips (`js/09:324,347`) and the program modal (`js/03:1065`). `littlewood` is a real catalog chip with a starter snippet. Rendering any coeff program containing one throws `ReferenceError: _ctAndyIndex is not defined`. The commit's removed-ID grep guard doesn't cover function references. **Fix:** inline `(pDefs||[]).findIndex(_isAndyParam)` with last-index fallback (`_isAndyParam` still exists).

### H4. The C EXPR/Param VMs' `abs()` Missed The Hypot Fix — [V] HIGH (numeric)

`c_abs` = `sqrt(r*r + i*i)` (`sweep_cli.c:2575`) is used by `COEFF_EXPR_ABS` (`:4147`), the param VM (`:6920`), and legacy `paramEvalScalarExpr` (`:6536`) — while the typed/vector VM was explicitly fixed to `hypot` for exactly this failure ("same fix as c_log/c_powr", `:4458-4461`). Verified numerically: underflow → silent 0 (and `param_sanitize` at `:6985` **zeroes** the param-side overflow to inf); overflow → spurious "did not produce a single finite result" row failure — while the same expression as source text (typed VM) computes correctly. Three-way divergence: EXPR VM ≠ typed VM ≠ Python folds. **Fix:** `hypot()` at the three sites.

### H5. The Fix Commits Introduced Regressions (Beyond The Known `inv_t_plus_2`)

- **[V] MED · `f8c933e` made explicit JSON-`null` chains a hard error** in `_root_transform_items` (`root_program_source.py:233-253`) and its copy (`program_v2_translate.py:235-238`): `{"chain": null}` etc. now raise where they compiled to an empty program before — contradicting the "null = absent" convention its sibling commit codified in `explicit_program_chain_for_run` (`pipeline_programs.py:100-114`). Latent today (callers sanitize). **Fix:** skip a key whose value is None.
- **[V] MED · `af496e4` ships wrong Help text for `inv_t_plus_2`**: `_paramProgramLegacyVariableFormNotes` (`js/08:555-564`) zips `counts[idx]` with `forms[idx]`, but the registry has 5 counts and 4 forms — the 3-arg note gets the 4-component description and 4 args get none. No gate asserts `len(counts)==len(forms)`. **Fix:** key forms by count; add a schema gate.
- **[V] MED-LOW · `38c1165` compat accessors degrade silently** when the registry `compat` block is missing/typo'd (`param_program_chain.py:274-283`: `payload.get("compat") or {}` → six empty families, no error, misleading downstream failures — while moebius/add keep working through in-code special cases, masking the damage). **Fix:** require the key + six sub-keys at load, like `require_registry_version`.
- **[V] LOW-MED · `2ba3f65` skipped `handler_param_debug`**: `handler_param_debug.py:142` still uses `params.get("param_program_chain") or []`, so the debug endpoint disagrees with preview/plan/coeffgen on explicit-empty chains; `handler_compute_preview.py:235-250` assigns `*_chain_explicit` flags that are never read. **Fix:** route through `explicit_program_chain_for_run`.
- **[T] LOW · `cdc9a33` optional-args loader validates only optional-vs-optional collisions** (`coeff_program_chain.py:477-479`): a positional arg named `andy` in `args[]` loads cleanly and would double-consume with `_split_trailing_andy`; >1 shared optional arg emits UI params but can never compile. Both blocked today only by pytest pins (`test_registry_schema.py:180,188`). **Fix:** reject positional/optional name collisions and >1 shared optional at load.
- **[T] LOW · `f8c933e` introduced `MAX_ROOT_TRANSFORMS = 16`** (`root_program_source.py:27`) mirroring `MAX_RT_CHAIN 16` (`root_xforms.h:26`) with no drift test between them — ironic given `67ff066`'s purpose.

### H6. Scalar-Expression Layer (First Audit) — [V]

Beyond H4:

- **MED · Raw `OverflowError` escapes both compilers** on static folds (`cmath.exp(800)` at `param_program_chain.py:621-622`, `coeff_program_chain.py:982-995`) — not RuntimeError, so no chip/arg context; the same expression made dynamic errors the row in coeff but **silently becomes 0** in the param VM (sanitize). **Fix:** catch OverflowError in both folders.
- **MED · `c_tan`/`c_tanh` pole cutoff returns 0.0** (`sweep_cli.c:2515-2518,2554-2557`) where Python folds give the correct ±1.6e16 — `tan(p1-p1+pi/2)` → 0.0 dynamic vs 1.633e16 static.
- **MED · Chain→source decompile of dynamic-arg chips silently switches VMs**: `coeff_source_text_from_chain` renders chain scalar-expr args as typed-VM source (`coeff_program_source.py:1105`) — every dynamic-arg chip round-trips with a different fingerprint AND different numerics (typed hypot/forgiving-divide vs EXPR c_abs/erroring-divide), and `poke_poly(0, tos0*conj(p2))` emits source that fails to compile. Consumed by Populate (`handler_storage.py:4062`) and PDF captions; only a RuntimeWarning marks it. **Fix:** emit raw chain-call syntax for chips carrying expr_refs, or flag output display-only.
- **MED · Coeff compiles guaranteed-runtime-failure exprs param rejects at compile** (`t1+1/0`): param folds static subtrees eagerly (`param_program_chain.py:597-608`), coeff folds whole-expr only (`coeff_program_chain.py:771-781`).
- **LOW ·** static-vs-dynamic division threshold asymmetry (fold rejects `|b|≤1e-300`, VM only exact zero); `sqrt(-4)` fold = exactly 2j vs dynamic `c_powr(x,0.5)` = 1.2e-16+2j; param accepts `1j*1j` as a real arg (folds first) where coeff rejects; param literal path `str(value or "")` maps numeric 0 → "empty expression" (`param_program_chain.py:391`; coeff has the correct None-test).
- **CLEAN (verified):** EXPR opcode drift guards, expr_refs/-1 sentinel machinery, +−*/ precedence, branch cuts/log-floor, complex-literal parsers, param round-trip fingerprint stability for source-originated programs.

### H7. Persistence / Editor Round-Trip Extras — [V]

- **MED · JS populate fallback synthesizer drops the imaginary part of two-arg `const` rows**: `_paramProgramSourceFromRows([["const","1","2"],…])` → `p1 = 1` (`js/08:63-69`) vs Python's `p1 = (1)+(2)*1j`; the synthesized text is compute-authoritative on the Populate fallback path (`js/02:355-357`, server-side synth failures swallowed at `handler_storage.py:4138-4139`) → silent numeric change.
- **MED · Chip hydration silently respells equivalent forms**, changing fingerprints of unedited programs on load→save: moebius 8-packed → 4-complex (`js/08:1569-1576`; 5-7-arg rows silently truncate); coeff native sin+andy (op 29) → legacy chip (op 9) (`js/08:2064-2072`).
- **MED-LOW · `macro(v2/foo)` resolves different objects in storage vs compute**: storage normalizes away `v2/` (`handler_storage.py:278-289`), the shared compute resolver builds the key verbatim (`program_compile_helpers.py:74`) — a new hole in the F6 family. **Fix:** normalize (or reject `/`) in `read_saved_program_source_chain`.
- **LOW ·** legacy-chip load splits expression args on whitespace (`js/08:1500-1504`, mangles `"1 + 2"`); serializers fabricate real transforms from garbage rows (empty legacy name → `unit_circle` `js/08:1644`, coeff → `rev`); param JSON import skips the chip validation coeff has (`js/08:1944-1966` vs `:2163-2164`); `translate_solve_score_from_old` omits `program_kind`; `translate_root_from_old` has zero production callers; dead persisted fields (`solve_score_has_source_text` metadata, `coeff_transforms_display` written-never-read by JS, two dead JS populate lookups); `statement_count` semantics differ per kind.
- **CLEAN (verified):** the andy pipeline end-to-end (registry → vocab → chips → source → chain, fingerprint-identical three ways); duplicate-source precedence at save/populate/migrate; route error taxonomy; favorites; list handlers; solve-score modal save/load.

### H8. Test-Suite Integrity — The Gates Are Partly Fiction — [V]

- **HIGH · Gate rot is systemic**: 100 of 140 test files are ungated, including six guarding finished work — `test_registry_schema.py`, `test_program_v2_migration.py`, `test_root_program_source.py` (+ the root family), `test_saved_program_source_precedence.py`, `test_coeff_source_equivalence.py`, and `test_whole_sweep_oracle.py` (the byte-exact SHA native oracle). The gate list hasn't changed since `79895c0` (Jun 25). **Fix:** gate the six now + add a meta-test that every fast contract test file is in the gate (explicit exclusion list for slow suites).
- **HIGH · The "frozen" oracle is partly fiction**: `coeff_program_source_legacy.py:19,225` **imports production** (`import coeff_program_source as _current`, lowers via `_current._legacy_lower_statement`) — the flagship equivalence tests compare production against production for semantic lowering; all oracles read **live** registries/profiles (data regressions cancel out); and `coeff_program_chain_legacy.py` **was modified post-freeze** (`cdc9a33`, +8/−2 — a necessary schema shim, but no before/after oracle-output diff was recorded). **Fix:** document the oracle-edit policy, snapshot registry fixtures for oracle paths, rename the source-equivalence test to its true scope (parser shell, not lowering).
- **MED · Tests pin documented-dead code as required**: `test_coeff_program_drift.py:279-291,509-511` + `test_frontend_js.sh:298` assert the B2 dead vocab keys are present — the documented deletion now breaks three test sites. `test_frontend_js.sh:166` pins the dead `_paramProgramLegacyArgSpecs` line. **Fix:** pin absence (or TODO-tag the pins) when a field is scheduled for deletion.
- **MED ·** `test_program_m3_oracles.py:32-34` silently drops the 6 harvested corpus cases if the directory disappears (floor is `checked >= 2`); `test_solve_score_source_equivalence.py` (gated) is pure round-trip self-consistency with no frozen oracle; two structurally tautological assertions in `test_coeff_program_drift.py:504-505` (generated == generator-input).
- **LOW ·** `tests/test_program_profiles_drift.py:38` mutates a module-global cache without restore; the sweep-binary mtime heuristic; module-skips that make `test_companion_matrix.py` decorative on darwin; ~85% of `test_frontend_js.sh` assertions are text-presence greps (the vm-executed blocks are the real coverage).
- **The 5 highest-value missing tests:** source-vs-registry arity parity (fails today on `inv_t_plus_2`); root cap three-way drift (fails today, 64 vs 16 vs 16); `solve_score.h` partition/min-roots drift; four-case blank/empty matrix per kind; partial-vocab (`{}`) frontend load test (crashes today at `js/07:240`).

## Plan Review Verdict (2026-07-02) — Findings Excellent, Plan Now ~70% Archaeology

A first-principles review of the plan against the code at HEAD concluded:

- **Phases -1, 0, 1–5, 8, 10, 10A describe landed work.** The forward-looking remainder is ~10 small items scattered across four places (Open list, Residuals, PARTIAL table rows, stale phase texts).
- **The Target Architecture sketches were never built — and the landed design is better.** `registry_common.py` is 13 flat helpers, simpler than the planned `RegistryConfig` dataclass API. Neither landed frontend adapter matches the `_makeProgramRegistryAdapter` sketch.
- **Phase 6–8 factory: rejected.** The two adapters (36 + 33 lines, `js/07:15,254`) already converge on `loaded/names/categoryMeta/spec()/params()/category()`; their non-shared methods are irreducibly per-vocab. A factory = same LOC + indirection. The real residue is the `js/07:240` crash and the dead `_paramProgramLegacyArgSpecs`.
- **Phase 9 shared validator: obsolete.** Param's parse-time validation landed per-kind; the only defect is 6 shadowing hardcode lines (`param_program_source.py:101-106`) whose fallback already reads the correct registry-pinned data. Delete the 6 lines; a shared `canonical_registry_function_name` would be a 2-line dict lookup wearing a module.
- **Program-kind contract (G0): adopt as contract-as-tests, not contract-as-module.** The v1 fingerprints are frozen wire (cache keys) — one canonical formatter/hash would orphan caches for at least two kinds. Right shape: name the two frozen policies (`.17g`+sha1 for param/coeff v1; `repr`+sha256 for root/solve) in one documented place, dedupe the 4 **live** `_format_number` copies into those two named policies (frozen-oracle copies untouched), and enforce diagnostics/precedence/empty-policy per kind via conformance tests — pin the divergence, don't unify it.

## Superseding Implementation Order

Replaces the 14-step order below (12 of 14 steps are landed or superseded — kept for the record).

1. **Gate the six ungated test files** in `scripts/predeploy_check.sh` (H8) — all pass today; protects everything already built. (S)
2. **Fix H1** (param save: forward `source_text`, two lines) and **H2** (v2 lifecycle: save rewrites/deletes v2 key, delete deletes both). (S + M)
3. **Fix H3** (littlewood `_ctAndyIndex`) and the `js/07:240` partial-vocab crash (A2 residue), with a `{}`-vocab frontend test. (S)
4. **Fix `inv_t_plus_2`**: delete the 6 shadowing lines (`param_program_source.py:101-106`) + a source-vs-registry arity assertion. (S)
5. **Fix H4** (`hypot` at the 3 C sites) + OverflowError guards in both fold paths (H6). (S/M)
6. **Dead-code sweep**: `root_source_text_from_payload`; dead coeff vocab keys **and their pinning tests**; solve-score `delta`/`legacy_alias`; `_paramProgramLegacyArgSpecs` + its pin. Regenerate vocabs same commit. (M)
7. **Root cap**: profile 64→16 + three-way drift test (profile ↔ `MAX_ROOT_TRANSFORMS` ↔ `MAX_RT_CHAIN`). (S)
8. **Oracle integrity** (H8): document the freeze policy, snapshot registry/profile fixtures for oracle paths, rename the coeff source-equivalence test to its true scope. (M)
9. **Boundary conformance tests**: four-case blank/empty matrix per kind (pin, don't unify); `solve_score.h` partition drift test; storage-vs-compute macro resolver equivalence (+ the H7 `macro(v2/…)` normalization). (M)
10. **Optional polish**: derive the two remaining coeff fn_index hardcodes from `compat_signatures`; dedupe the 4 live `_format_number` copies into two named frozen policies; chain-compiler diagnostic codes (F3); solve-score statement-local error positions (G4) — cut if it needs parser surgery. (S/S/M/M)

**Deleted from the plan:** Phase 0; the Phase 6 factory; Phase 7's `_programRegistryTransformHelpItem`; Phase 9's shared validator; the Target Architecture `RegistryConfig`/frontend-factory sketches (superseded by the simpler landed design); old Implementation Order steps 1–13.

## Target Architecture

> **SUPERSEDED (2026-07-02):** the sketches below were never built — the landed design is simpler and better. `lambda/registry_common.py` exists as 13 flat helpers (no `RegistryConfig` dataclass); the frontend has two converging per-profile adapters (`js/07:15,254`) instead of a factory. See the Plan Review Verdict above. Kept as historical record.

### Shared Python Registry Module

Add:

```text
lambda/registry_common.py
```

Responsibilities:

- Load JSON registry files.
- Validate version.
- Iterate functions sorted by `fn_index`.
- Validate names and indices.
- Normalize args.
- Validate arg types.
- Validate required local UI fields.
- Normalize UI params.
- Normalize category metadata.
- Build `by_name` / `by_index` indexes.
- Validate aliases when configured.
- Provide shared JS rendering/check helper.

Suggested API:

```python
from dataclasses import dataclass

@dataclass(frozen=True)
class RegistryConfig:
    label: str
    allowed_arg_types: frozenset[str]
    lower_names: bool = True
    max_args: int = 8
    require_ui_desc: bool = True
    category_meta_path: tuple[str, ...] = ("category_meta",)

def load_registry_payload(path, config): ...
def sorted_registry_functions(payload): ...
def normalize_registry_arg(arg, *, function_name, index, allowed_types): ...
def validate_local_ui(function, *, label, require_desc=True): ...
def normalize_ui_params(function, *, runtime_args=None): ...
def registry_indexes(functions, config, augment_fn=None): ...
def category_meta(payload, *paths): ...
def render_window_assignment(var_name, payload, *, header): ...
def check_generated(path, text, *, label): ...
```

The shared module should not know Param or Coeff semantics. It should expose generic validation and normalization primitives.

### Profile-Specific Augmentation

Param still owns:

- `kind`
- Param selector sets.
- Param compatibility metadata.
- Variable arg count compatibility.

Coeff still owns:

- `length_policy`
- alias maps
- text aliases
- `effective_args`
- `compat_signatures`
- native packing function IDs

Coeff currently also has `supports_andy`, but that is compatibility debt, not valid profile-owned semantic schema. The target is to remove it once normal optional args plus the compatibility shim express the behavior.

The split should be:

- Shared module validates the universal registry shape.
- Profile modules add fields that only that profile understands.

### Shared Frontend Adapter

Add a thin frontend registry adapter in `js/07-transform-catalogs.js`:

```js
function _makeProgramRegistryAdapter(kind, vocab, options) {
  return {
    kind,
    names,
    catalog,
    categoryMeta,
    argSpecs,
    aliases,
    sourceName(name),
    helpParams(name),
    snippet(name),
  };
}
```

Then:

```js
const _paramRegistryAdapter = _makeProgramRegistryAdapter('param', _paramRegistryVocab, ...);
const _coeffRegistryAdapter = _makeProgramRegistryAdapter('coeff', _coeffRegistryVocab, ...);
```

Help builders should consume adapter methods rather than raw `_param...` and `_coeff...` globals.

## Implementation Plan

### Phase -1: Fix Verified Live Bugs

Do these before the shared infrastructure refactor. They are small, independently testable, and reduce noise before moving registry code.

#### -1A. Param Signed-Zero Fingerprint Canonicalization

Files:

```text
lambda/param_program_chain.py
tests/test_param_program_chain.py
```

Implementation:

- Add `_canonical_zero(value)` equivalent to the Coeff implementation.
- Use it in `_token` for `a`, `b`, `args`, and `args_im`.
- Do not change formatting or inverse display logic in the same patch.

Tests:

- Compile equivalent zero forms through `compile_param_program_source`.
- Assert identical fingerprints and execution specs for:

```text
legacy(rtheta,both,both,0)
legacy(rtheta,both,both,-0.0)
legacy(rtheta,both,both,-(0))
legacy(rtheta,both,both,0*-1.0)
```

#### -1B. Coeff Partial-Vocab Frontend Crash

Files:

```text
js/07-transform-catalogs.js
tests/test_frontend_js.sh
```

Implementation:

- Change `_coeffProgramWideParamDefs` to guard both the vocab object and `programParamDefs`.
- Add a frontend harness case where `window._coeffRegistryVocab = {}` or lacks `programParamDefs`, and verify the module does not throw.

#### -1C. Coeff `round` Ambiguous Arity

Files:

```text
lambda/coeff_program_chain.py
lambda/coeff_legacy_registry.json
tests/test_coeff_program_chain.py
tests/test_coeff_program_drift.py
```

Implementation decision:

- First add a drift test that detects overlaps between `arg_counts` and `andy_arg_counts` after subtracting the trailing `andy`.
- For current `round`, choose one explicit policy:
  - preserve current compatibility and document/pin `legacy(round, poly, poly, 1, 2)` as `(multiplier=1, andy=2)`, or
  - reject ambiguous source text with a coded diagnostic and require an unambiguous 3-arg real-lane form.

Do not leave this as first-match behavior with no test.

#### -1D. Param Parse-Time Legacy Validation

Files:

```text
lambda/param_program_source.py
lambda/program_source_core.py
tests/test_param_program_source.py
```

Implementation:

- Validate `legacy(name, src, tgt, ...)` names against the Param registry during parsing/lowering.
- Validate `src` and `tgt` selectors during parsing/lowering.
- Return coded diagnostics for unknown names and bad selectors.
- Preserve saved-chain round-trip compatibility.

Tests:

```text
parse_param_program_source("legacy(nope,both,both)") -> unknown_legacy_transform
parse_param_program_source("legacy(unit_circle,,both)") -> bad_selector
parse_param_program_source("legacy(unit_circle,none,both)") -> bad_selector
parse_param_program_source("legacy(unit_circle,both,nope)") -> bad_selector
parse_param_program_source("crd(5,9)") -> either valid declared shorthand or coded arity/selector diagnostic; not silent accidental lowering
parse_param_program_source("legacy(square,pop2,push2)") -> bad_selector
```

Also decide and test reduced-arity policy:

- If reduced arity is compatibility-supported for a function, it must be declared in registry compatibility metadata.
- If not declared, missing args should be a coded arity diagnostic.
- Numeric defaults, including `0.0`, must compile correctly when default filling is intentional.

#### -1E. Param Chain-To-Source Round-Trip Guard

Files:

```text
lambda/param_program_source.py
lambda/program_v2_translate.py
tests/test_param_program_source.py
tests/test_program_v2_migration.py
```

Implementation:

- Add a Param source-preservation guard mirroring Coeff.
- Make `param_source_text_from_chain` verify that synthesized source reparses and recompiles to the same chain/fingerprint.
- Make `translate_param_from_old` refuse to hash unparseable synthesized source into v2 fingerprints.
- Use the same guarded serializer for Populate/detail and PDF reconstruction paths.

Tests:

- `[['const','1','2'],['emit','p1']]` must not produce persisted unparseable source.
- Representative old Param chains round-trip source -> chain -> source safely.
- Migration fingerprints must not include unparseable synthesized source.

#### -1F. Param Macro Source Precedence

Files:

```text
lambda/program_compile_helpers.py
tests/test_param_program_storage.py or tests/test_param_program_chain.py
```

Implementation:

- Mirror the Coeff branch in `read_saved_program_source_chain` for Param saved programs.
- Use `param_source_text_from_payload` / `parse_param_program_source` when Param source text exists.
- Fall back to `chain` only when source text is absent.

Tests:

- Saved Param macro payload with deliberately different `source_text` and `chain` must expand from `source_text`.
- Existing saved-chain-only macro payloads continue to work.

#### -1G. Coeff Source Diagnostic Codes And Registry Arg Validation

Files:

```text
lambda/coeff_program_source.py
tests/test_coeff_source_equivalence.py or a new tests/test_coeff_program_source.py
```

Implementation:

- Give Coeff source errors stable codes instead of allowing all site-specific errors to collapse to `source_error`.
- Validate registry-backed int/enum args and arity during source parse/lowering where possible.
- Preserve existing compatibility behavior only where explicitly tested, such as accepted `andy` forms.

Tests:

- Unknown function, bad selector, bad arity, bad int arg, and bad enum arg produce distinguishable diagnostic codes.
- `poly = roots(poly,2.7,lo); emit` fails before chain compiler `RuntimeError`.
- Extra args that are really `andy` are documented and tested; accidental over-arity is rejected.

#### -1H. Empty Source Policy

Files:

```text
lambda/param_program_source.py
lambda/coeff_program_source.py
tests/test_param_program_source.py
tests/test_coeff_source_equivalence.py or a new tests/test_coeff_program_source.py
```

Implementation:

- Decide whether empty programs are valid no-ops or errors per profile.
- Pin the policy with tests.
- Do not leave Coeff as silent no-op and Param as `empty_source` unless this asymmetry is intentional and documented.

#### -1I. Empty Chain And Falsy Fallback Policy

Files:

```text
lambda/handler_coeffgen.py
lambda/handler_compute_plan.py
lambda/handler_compute_preview.py
tests/test_compute_plan_handler.py or existing compute/preview handler tests
```

Implementation:

- Replace `params.get("..._program_chain") or ..._transforms_to_program_chain(...)` with explicit key-presence logic.
- Decide whether `[]` is a valid explicit no-op chain or invalid at the run boundary.
- Apply the same policy for Param and Coeff unless there is a documented reason not to.

Tests:

- A payload with explicit `param_program_chain: []` and non-empty `param_transforms` must not silently use `param_transforms`.
- A payload with explicit `coeff_program_chain: []` and non-empty `coeff_transforms` must not silently use `coeff_transforms`.
- Storage/save/source/compiler behavior for empty programs must match the chosen policy.

#### -1J. Shared Macro Resolver Source Precedence

Files:

```text
lambda/program_compile_helpers.py
lambda/handler_storage.py
tests/test_param_program_storage.py
tests/test_compute_plan_handler.py or handler_compile helper tests
```

Implementation:

- Extend `read_saved_program_source_chain` to honor Param source text.
- Avoid keeping a better storage-only Param resolver and a worse compute resolver.
- Share one implementation or add a test that both paths produce identical macro expansion.

Tests:

- Same saved Param macro payload with divergent `source_text` and `chain` expands from source in storage and compute helper paths.

#### -1K. Boundary Translator Drift Tests

Files:

```text
lambda/pipeline_programs.py
tests/test_param_program_drift.py
tests/test_coeff_program_drift.py
```

Implementation:

- Add drift tests tying `pipeline_programs.py` Param target-first/target-last/dither sets to `param_legacy_registry.json`.
- Add drift tests tying `pipeline_programs.py` Coeff `exp`/`round` old real-lane repacking to `coeff_legacy_registry.json` `compat_signatures`.
- Do not refactor translation yet unless tests first pin current behavior.

#### -1L. Compiler Diagnostic Structure

Files:

```text
lambda/param_program_chain.py
lambda/coeff_program_chain.py
tests/test_param_program_chain.py
tests/test_coeff_program_chain.py
```

Implementation:

- Add stable diagnostic `code` fields for compiler-stage errors in strict=False mode.
- Preserve messages, but do not leave API diagnostics as message-only dicts.
- Cover stack underflow, final stack depth, macro errors, bad selector, bad arity, int/enum errors, and native-transform stack arg errors.

#### -1M. Root/Solve-Score Source Boundary Follow-Up

Files:

```text
lambda/root_pipeline_programs.py
lambda/root_program_source.py
lambda/solve_score_program_source.py
tests/test_root_program_source.py or existing root tests
tests/test_solve_score_program_source.py
```

Implementation:

- Pin whether `root_source_text_for_run` accepts top-level `source_text`.
- Add stable diagnostic codes for Root/Solve-score source errors over time.
- Keep this lower priority than Param/Coeff corruption and runtime-authority fixes unless a failing user path is found.

#### -1N. Solve-Score Fingerprint Collision Fix

Files:

```text
lambda/solve_score_chain.py
tests/test_solve_score_chain.py
tests/test_solve_score_program_source.py
```

Implementation:

- Replace `%g` formatting in solve-score canonical numeric output.
- Use a shared helper that:
  - converts to finite float,
  - folds `-0.0` to `0.0`,
  - formats with `.17g`.
- Apply it to every numeric value that contributes to `program_spec` / fingerprint.
- Treat this as a deliberate fingerprint/cache-version change for solve-score programs.

Tests:

```text
omega_cosine(1.234567890123) != omega_cosine(1.234569999)
omega_cosine(0.30000001) != omega_cosine(0.30000009)
omega_cosine(0.0) == omega_cosine(-0.0)
const/weighted_sum/ema/pow/sawtooth numeric args use the same canonicalization
```

#### -1O. Root Native Chain Cap

Files:

```text
lambda/root_program_source.py
lambda/root_xforms.h
lambda/program_profiles.json
tests/test_root_program_source.py
tests/test_program_profiles.py or tests/test_program_drift.py
```

Implementation:

- Enforce the current native cap of 16 root transform rows in Python compile/lower code.
- Return a structured diagnostic such as `root_chain_too_long`.
- Add a drift test tying the profile cap and C cap together.
- If the intended cap is 64, do not merely raise `MAX_RT_CHAIN`; audit C memory/layout first and then update both sides together.

Tests:

```text
16 root transforms compile
17 root transforms fail before native execution
profile cap == native cap
```

#### -1P. Canonical V2 Fingerprint Payloads

Files:

```text
lambda/program_v2_translate.py
tests/test_program_v2_migration.py
tests/test_program_m3_oracles.py
tests/test_coeff_wire_fingerprints.py
```

Implementation:

- Remove raw `source_text` from Param and Coeff `_v2_fingerprint` payloads.
- Hash canonical execution data only.
- Keep `source_text` stored for UI/editing, but not for identity.
- Verify Root already follows this pattern and keep it unchanged.
- Treat this as part of the cross-kind fingerprint contract: display/source text is editable metadata, not cache identity.

Tests:

```text
Param equivalent source spellings with the same execution spec produce the same v2 fingerprint.
Coeff equivalent source spellings with the same execution spec produce the same v2 fingerprint.
Changing execution spec changes the v2 fingerprint.
Param/coeff/root/solve fingerprint tests use the same signed-zero and distinct-float expectations where applicable.
M3 oracle and Coeff wire-fingerprint tests remain green.
```

#### -1Q. Root Run-Boundary Diagnostics And Name/Index Consistency

Files:

```text
lambda/root_program_source.py
lambda/root_pipeline_programs.py
tests/test_root_program_source.py
```

Implementation:

- Convert malformed `root_transforms` JSON/non-list errors into structured diagnostics at the run boundary.
- Reject dict rows where both `name` and `fn_index` are present but disagree.
- Add stable diagnostic codes for root source and root chain errors that matter to UI/API callers.
- Decide whether `root_source_text_from_payload` is dead and remove it, or wire its precedence into the live resolver.

Tests:

```text
root_program_for_run({"root_transforms": "5"}) returns/raises structured diagnostics.
root_program_for_run({"root_transforms": "not-json"}) returns/raises structured diagnostics.
{"name": "rotate_roots", "fn_index": 5} is rejected unless a compatibility flag is explicitly used.
```

#### -1R. C Runtime Default/Packing Drift Gates

Files:

```text
lambda/sweep_cli.c
lambda/param_legacy_registry.json
lambda/coeff_legacy_registry.json
tests/test_param_program_drift.py
tests/test_coeff_program_drift.py
```

Implementation:

- Add tests that compare Param registry defaults to either:
  - Python-expanded native rows sent to C, or
  - the C fallback defaults still hardcoded in `dispatchPt`.
- Add tests that compare Coeff `compat_signatures` packing rules to the C `fn_index` packing ladder for `linear`, `exp`, `round`, and `pow`.
- Add tests that prove omitted registry defaults are expanded before C if that is the intended contract.
- Add tests that compare solve-score metric source/min-root partitions between Python and `solve_score.h`.

Do not edit registry defaults or packing without these tests.

#### -1S. Solve-Score Non-Strict Fallback And Vocab API Cleanup

Files:

```text
lambda/solve_score_program_source.py
lambda/solve_score_pipeline_programs.py
lambda/gen_solve_score_vocab.py
solve_score_vocab_js.js
js/07-transform-catalogs.js
tests/test_solve_score_program_source.py
tests/test_frontend_js.sh
```

Implementation:

- Preserve diagnostics when `strict=False` returns a fallback proximity program.
- Mark fallback/degraded compile results explicitly.
- Add stable solve-score diagnostic codes for common error classes.
- Attach compile errors to the originating statement where possible instead of falling back to line 1 / column 1 for whole-chain failures.
- Keep `stackSpecs` and `outputSpecs`; they are live.
- Either consume or remove dead subfields such as `stackSpecs.*.delta` and `outputSpecs.*.legacy_alias`.

#### -1T. Cross-Kind Blank Source/Empty Program Contract

Files:

```text
lambda/pipeline_programs.py
lambda/root_pipeline_programs.py
lambda/solve_score_pipeline_programs.py
lambda/program_compile_helpers.py
tests/test_pipeline_programs.py or focused per-kind source/run tests
```

Implementation:

- Define source/chain fallback semantics for all program kinds:
  - source key absent,
  - source key present but blank,
  - chain key absent,
  - chain key present as empty list.
- Replace truthiness fallback with explicit key-presence checks.
- Make solve-score, Param, Coeff, and Root behavior intentionally different only where documented.

Tests:

```text
blank solve-score source keeps its current error behavior or a consciously changed policy
blank Param/Coeff/Root source behavior is pinned
empty chain never resurrects legacy arrays unless that is explicitly chosen
```

#### -1U. Root Lossless Source Regeneration

Files:

```text
lambda/root_program_source.py
tests/test_root_program_source.py
tests/test_program_v2_migration.py
```

Implementation:

- Replace root `%g` source formatting with a lossless canonical numeric format.
- Fold signed zero.
- Add a preservation check or test corpus proving chain -> source -> parse -> compile keeps the same root fingerprint.
- Ensure root v2 migrated `source_text` is safe to edit/re-save without changing execution.

Tests:

```text
rotate_roots(1.234567890123) round-trips through regenerated source without changing fingerprint.
rotate_roots(0.30000001) and rotate_roots(0.30000009) regenerate to distinct source values.
all root registry transforms with numeric defaults round-trip through source.
```

#### -1V. V2 Translation Empty-Field Precedence

Files:

```text
lambda/program_v2_translate.py
tests/test_program_v2_migration.py
```

Implementation:

- Replace truthiness fallback in `_root_transform_items` with explicit key-presence logic.
- Preserve explicit `root_transforms: []` as empty.
- Define solve-score migration behavior for explicit `chain: []`.
- If solve-score legacy metric fallback is supported during migration, read the legacy metric explicitly and test it.
- Do not silently migrate explicit empty solve-score chain to default proximity unless that is an intentionally versioned compatibility rule.

Tests:

```text
translate_root_from_old({"root_transforms": [], "chain": [["rotate_roots", "1"]]}) migrates an empty root chain.
translate_solve_score_from_old({"chain": []}) is rejected or preserves an explicit empty/default policy with diagnostics.
translate_solve_score_from_old({"chain": [], "metric": "spread"}) does not silently become proximity.
```

#### -1W. Removed Frontend Editor Dead-Code Cleanup

Files:

```text
index.html
js/06-popup-init.js
js/07-transform-catalogs.js
js/08-chip-editors.js
js/09-render-orchestration.js
js/12-deepzoom-boot.js
tests/test_frontend_js.sh
```

Implementation:

- Remove old Param Transform picker UI functions whose only DOM targets are missing:
  - `_renderParamTransformAddPopup`
  - `_setParamTransformPickerOpen`
  - `toggleParamTransformPicker`
  - `selectParamTransformChip`
  - `_syncParamTransformAddOptions`
- Remove old Coeff Transform picker UI functions whose only DOM targets are missing:
  - `_renderCoeffTransformAddPopup`
  - `_setCoeffTransformPickerOpen`
  - `toggleCoeffTransformPicker`
  - `selectCoeffTransformChip`
  - `_syncCoeffTransformAddOptions`
- Remove outside-click handlers and boot sync calls for missing `pt-add-*`, `ct-add-*`, `pp-add-*`, and `cp-add-*` controls.
- Remove or quarantine removed Program chip-picker scaffolding for `pp-add-popup` / `cp-add-popup`; the visible Param/Coeff Program editors are text-only.
- Keep source-synthesis compatibility helpers that are still used for old chain-only saved programs.
- Keep Results/detail display of legacy `param_transforms` / `coeff_transforms`.
- Audit `_ptCatalog`, `_ptInfo`, `_ptCategoryMeta`, and `_ptCatalogEnriched` after the picker code is removed:
  - If only dead UI code used a field, delete it.
  - If old-chain compatibility still uses it, move it behind an explicitly named compatibility helper, not a visible-editor catalog name.
- Remove tests that assert dead picker/converter functions exist merely as UI surface.
- Replace them with tests for:
  - `index.html` has no old `pt-add`, `ct-add`, `pp-add`, or `cp-add` controls.
  - old chain-only saved programs still synthesize source text.
  - preview/compute payloads still do not send editable legacy transform arrays.

Add a missing-static-DOM-id guard:

- Parse static `document.getElementById("...")` references in `js/*.js`.
- Compare against `index.html` IDs.
- Allowlist only IDs that are actually created dynamically by live code, e.g. render-artifact action buttons and menus created by `refreshRenderArtifacts()`.
- Fail on removed editor IDs such as `pt-add-popup`, `ct-add-popup`, `pp-add-popup`, `cp-add-popup`, `pt-add`, `ct-add`, and `pp-add`.

Tests:

```bash
bash tests/test_frontend_js.sh
uv run python -m pytest tests/test_frontend_parts_contract.py -q
```

Gates:

```bash
uv run python -m pytest tests/test_param_program_chain.py tests/test_param_program_source.py tests/test_program_v2_migration.py tests/test_coeff_program_chain.py tests/test_coeff_program_drift.py tests/test_coeff_source_equivalence.py tests/test_coeff_program_native.py -q
uv run python -m pytest tests/test_solve_score_chain.py tests/test_solve_score_program_source.py tests/test_root_program_source.py -q
uv run python -m pytest tests/test_program_m3_oracles.py tests/test_coeff_wire_fingerprints.py -q
bash tests/test_frontend_js.sh
```

### Phase 0: Freeze The Current Baseline

Before refactoring, preserve current behavior.

Important current baseline: the M3 wire/fingerprint infrastructure is already landed. `compat_signatures` and `effective_args` are not future design notes; they are load-bearing registry data used by the frozen M3 oracle and forward wire packer. This refactor must keep those gates green throughout.

Phase -1 fixes intentionally change broken behavior. Phase 0 starts after those fixes are either landed or explicitly test-pinned.

Steps:

1. Regenerate current vocab artifacts:

```bash
uv run python lambda/gen_param_vocab.py
uv run python lambda/gen_coeff_vocab.py
```

2. Run focused gates:

```bash
uv run python -m pytest tests/test_param_program_drift.py tests/test_coeff_program_drift.py tests/test_param_program_source.py tests/test_coeff_source_equivalence.py tests/test_coeff_program_native.py -q
uv run python -m pytest tests/test_program_m3_oracles.py tests/test_coeff_wire_fingerprints.py -q
bash tests/test_frontend_js.sh
```

3. If local Param registry migration is still uncommitted, commit or checkpoint it before larger infrastructure work.

Reason: the refactor should prove it preserves behavior. Do not mix schema migration, generator refactor, parser changes, and Help changes in one unreviewable patch.

### Phase 1: Add Shared Registry Schema Tests

Add:

```text
tests/test_registry_schema.py
```

Test both:

```text
lambda/param_legacy_registry.json
lambda/coeff_legacy_registry.json
```

Assertions:

- `version == 1`.
- `functions` is a non-empty list.
- Every function has a non-empty `name`.
- Names are unique.
- Every function has positive integer `fn_index`.
- `fn_index` values are unique.
- Every function has local `ui`.
- Every function has local `ui.desc`.
- Every runtime arg has `name`, `type`, and `default`.
- Every runtime arg type is allowed for that profile.
- If `ui.params` exists, it has the same arity as runtime `args` unless explicitly exempted by the profile rules below.
- Every UI param has a visible name (`name` or `ph`) and non-empty `title`/`help`.
- Every function category exists in category metadata.
- Selector names in `allowed_src` and `allowed_tgt` are valid for the profile.
- Registry names and aliases are lowercase canonical identifiers unless explicitly documented.
- Registry/config values do not fall back to compiler constants through truthiness coalescing.
- Param registry compat families are complete: `target_arg_indexes`, `independent_targets`, `variable_arg_counts`, `target_first`, `target_last`, and `dither_target_first`.
- The C executor's Param compatibility tables match the registry or an explicit checked export.

Variable-arity exemption rule:

- Param functions listed in `compat.variable_arg_counts` are the explicit exemption set.
- Today that set is `moebius`, `inv_t_plus_2`, and `add`.
- These functions may have `args: []` while still needing user-facing compatibility forms.
- No other function should be exempted silently.

Coeff packed-display exemption rule:

- Coeff functions with `compat_signatures` need special scrutiny because display/source params can differ from wire packing.
- Today that set is pinned by `tests/test_coeff_program_drift.py::test_coeff_compat_signature_transforms_are_pinned` as `linear`, `exp`, `round`, and `pow`.
- Of those, `pow` is the known outlier: it has four real packed runtime args but two complex/display params. `linear`, `exp`, and `round` currently have matching arg/display arity.
- These functions are not exempt from validation; they need a different assertion: `ui.params` / `ui.program_params` must match the declared display/source form, while `compat_signatures` and wire gates prove packed runtime compatibility.
- Functions without `compat_signatures` should still satisfy the normal arity rule.
- `compat_signatures` must not introduce ambiguous first-match behavior silently. If an `arg_counts` form overlaps an `andy_arg_counts` form after removing trailing `andy`, the function must be in a narrow documented exception set with a behavior test.

Complex-arg safety rule:

- Param currently uses `type: "complex"` for real runtime args and must keep supporting it.
- Coeff currently has no registry runtime arg with `type: "complex"`; Coeff complex source/display forms are handled through `compat_signatures`.
- Keep the existing guard `tests/test_coeff_program_drift.py::test_coeff_registry_has_no_unpinned_generic_complex_args` green with the schema tests.
- A future generic complex runtime arg is a wire-format change and must update the packer and wire corpus explicitly.

Category metadata lookup must be config-driven in this phase:

- Param still reads `ui.categories` until Phase 5A.
- Coeff reads top-level `category_meta`.
- The test must not assume the final normalized key before the schema migration happens.

Profile config:

```python
PARAM_ALLOWED_ARG_TYPES = {"real", "complex"}
COEFF_ALLOWED_ARG_TYPES = {"real", "complex", "int", "enum"}
```

This phase should not change production code.

### Phase 2: Add `lambda/registry_common.py`

Implement only shared helpers first. Do not refactor Param/Coeff yet.

Required helpers:

- `load_json(path)`.
- `require_registry_version(payload, label, version=1)`.
- `registry_functions_by_index(payload, label)`.
- `normalize_registry_name(value, lower=True)`.
- `normalize_arg(arg, allowed_types, label, function_name, index)`.
- `validate_function_identity(fn, seen_names, seen_indices, label)`.
- `validate_ui(fn, label, require_desc=True)`.
- `normalize_ui_param(arg, *, default_title)`.
- `extract_category_meta(payload, paths)`.
- `render_js_assignment(var_name, payload, header)`.
- `check_generated_file(path, rendered, label)`.

Keep the module pure:

- No imports from `param_program_chain.py`.
- No imports from `coeff_program_chain.py`.
- No compiler constants except values passed in by callers.

### Phase 3: Refactor Generators To Use Common Helpers

Refactor:

```text
lambda/gen_param_vocab.py
lambda/gen_coeff_vocab.py
```

Rules:

- Preserve existing generated JS payload keys in this phase.
- Preserve output ordering.
- Preserve stale-file behavior.
- Remove duplicate `_load_json`, `_fmt_default`, and JS check logic.
- Use common `validate_ui`.
- Use common category metadata extraction.
- Use common arg normalization.

Param-specific cleanup:

- Stop using chain fallbacks for registry `compat` fields:

```python
compat.get("target_arg_indexes") or chain._LEGACY_TARGET_ARG_INDEXES
```

The registry should be authoritative. In the current registry this fallback is already dead code: `registry.compat` carries the required keys, and `tests/test_param_program_drift.py` already pins `vocab == registry.compat == chain` side tables. Removing the fallback is therefore low-risk, but the acceptance gate must explicitly keep that existing drift test green.

- Extend the drift test to cover all Param compat families, not only the currently covered subset:
  - `target_arg_indexes`
  - `independent_targets`
  - `variable_arg_counts`
  - `target_first`
  - `target_last`
  - `dither_target_first`
- Use explicit key-presence checks instead of `compat.get(...) or chain._...`.

Coeff-specific cleanup:

- Keep current generated `andy` behavior initially to avoid broad behavior change, but mark it as temporary compatibility debt.
- Do not move `ANDY_PARAM` into semantic registry data. The later phase should replace it with a normal declared optional argument plus a compatibility shim for old trailing/split forms.
- Keep `programParamDefs` behavior unchanged.
- Keep M3 wire gates green. The generator reads the same registry fields that feed `effective_args` and `compat_signatures`; stale or reshaped generated data must not mask a wire-layout regression.

Gates:

```bash
uv run python lambda/gen_param_vocab.py --check
uv run python lambda/gen_coeff_vocab.py --check
uv run python -m pytest tests/test_param_program_drift.py tests/test_coeff_program_drift.py tests/test_registry_schema.py -q
uv run python -m pytest tests/test_program_m3_oracles.py tests/test_coeff_wire_fingerprints.py -q
```

### Phase 4: Refactor Runtime Registry Loaders

Refactor:

```text
lambda/param_program_chain.py::_load_legacy_registry
lambda/coeff_program_chain.py::_load_legacy_registry
```

Target shape:

```python
from registry_common import load_registry_payload, build_registry_indexes

def _load_legacy_registry():
    payload = load_registry_payload(_registry_path(), PARAM_REGISTRY_CONFIG)
    return build_registry_indexes(
        payload,
        PARAM_REGISTRY_CONFIG,
        augment_fn=_augment_param_registry_spec,
    )
```

And:

```python
def _augment_param_registry_spec(fn, normalized_args):
    return {
        "name": name,
        "fn_index": fn_index,
        "kind": ...,
        "allowed_src": ...,
        "allowed_tgt": ...,
        "args": tuple(normalized_args),
        "compat": ...,
    }
```

Param augmentation must retain normalized registry `compat`. The compiler should read target-first/target-last/dither/target-arg/variable-arg behavior from the loaded registry data, not from private Python constants. During the transition, keep constants only as test fixtures or compatibility assertions, not as production fallback.

Coeff augmentation keeps:

- `category`
- `allowed_src`
- `allowed_tgt`
- `args`
- `effective_args`
- `compat_signatures`
- `length_policy`
- `aliases`
- `chain_only_aliases`
- `chip_name`
- `ui`

`supports_andy` remains only as a temporary legacy field while behavior is preserved. It is not part of the target semantic schema.

Do not simplify or reorder `compat_signatures` or `effective_args` while doing the loader refactor. They are wire/fingerprint inputs, not display-only metadata.

Do not preserve dead generated frontend fields just because they exist today. If `fnIndexByName`, `supportsAndy`, `effectiveArgs`, or `compatSignatures` remain in `coeff_vocab_js.js`, document the consumer or add a test proving the field is intentionally public. Otherwise schedule removal after confirming frontend and packaging tests stay green.

Alias validation can be a common helper:

```python
build_alias_maps(by_name, specs, alias_fields=("aliases", "chain_only_aliases"))
```

Gates:

```bash
uv run python -m pytest tests/test_param_program_chain.py tests/test_coeff_program_chain.py tests/test_param_program_drift.py tests/test_coeff_program_drift.py -q
uv run python -m pytest tests/test_program_m3_oracles.py tests/test_coeff_wire_fingerprints.py -q
```

### Phase 5: Normalize Registry JSON Schema

Do this after generators and loaders use shared helpers.

#### 5A. Move Param Categories

Change Param from:

```json
"ui": {
  "categories": {...}
}
```

to:

```json
"category_meta": {...}
```

Transition rule:

- For one commit, generator/common helper may accept both.
- Tests should require the final canonical key once migrated.

#### 5B. Resolve Param `variable_arg_forms`

Current `ui.variable_arg_forms` is inert.

Preferred final shape:

```json
{
  "name": "moebius",
  "ui": {
    "desc": "...",
    "compat_forms": [
      {"argc": 0, "form": "legacy(moebius, both, both)", "meaning": "legacy default 1/(z+2)"},
      {"argc": 4, "form": "legacy(moebius, both, both, a, b, c, d)", "meaning": "complex Möbius coefficients"},
      {"argc": 8, "form": "legacy(moebius, both, both, a_re, a_im, ...)", "meaning": "old real-lane form"}
    ]
  }
}
```

Then generated Help can render these forms. Delete top-level `ui.variable_arg_forms`.

#### 5C. Clarify Param `ui.params`

For runtime args, `ui.params` must describe exactly those runtime args:

- Same arity.
- Same names.
- Same default semantics.
- Human title/help required.

For legacy variable forms, do not overload `ui.params`. Use `ui.compat_forms`.

#### 5D. Clarify Coeff `ui.params` vs `ui.program_params`

Keep both, but document the rule in the registry:

- `ui.params`: default chip/native-transform UI params.
- `ui.program_params`: source-program/editor params when the source form intentionally differs from the chip/default UI form.

Examples: `exp`, `round`.

Also resolve the current `pow` inconsistency:

- `linear`, `exp`, and `round` have `compat_signatures` but still declare runtime `args` in the same display arity.
- `pow` currently declares runtime `args` as four packed real lanes while its UI/source form is two complex params.
- Preferred Phase-5 outcome: normalize `pow.args` to the same two-complex representation as its display/source form, if M3 oracle and Coeff wire fingerprint tests prove this is wire-safe.
- If `pow.args` cannot be normalized safely, document `pow` as the only intentional packed-runtime/display-arity mismatch and keep a narrow schema-test exception for `pow`, not a blanket exception for all `compat_signatures` functions.

#### 5E. Start Removing `andy` Magic

Do not finish the optional-arg redesign here unless explicitly scheduled, but remove obvious duplicated hardcoding when it is safe:

- Derive native-transform stack arg limits from `compat_signatures` where available instead of hardcoded fn-index sets.
- Derive native-transform trailing-`andy` arities from `compat_signatures.andy_arg_counts` where available.
- Treat those derivations as compatibility-shim work, not semantic-registry work.
- Do not add new semantic-registry fields that exist only to explain old `andy` packing.
- Delete `supports_andy` rather than trying to make it meaningful unless a real per-function capability difference appears.
- The registry may declare `andy` as a normal optional argument in a normal arg list; it must not declare "andy support" as a separate magic capability.
- Keep M3 oracle and Coeff wire fingerprint tests green around each change.

### Phase 6: Add Shared Frontend Registry Adapter

Refactor `js/07-transform-catalogs.js`.

Add:

```js
function _makeProgramRegistryAdapter(kind, vocab, options = {}) {
    ...
}
```

Adapter output:

```js
{
    kind,
    names,
    categoryMeta,
    catalog,
    argSpecs,
    aliasToCanonical,
    sourceAliasByName,
    canonicalName(name),
    sourceName(name),
    paramsForHelp(name),
    paramsForSnippet(name),
    uiSpec(name),
}
```

Param adapter:

- Consumes `_paramRegistryVocab`.
- Provides `src/tgt` params for `legacy(name, src, tgt, ...)`.
- Uses Param selector choices from `program_profiles_js.js`.
- Uses generated `argSpecs`.
- Uses generated `uiFunctions`.
- If `_paramRegistryVocab` is missing, renders an explicit Param registry-load failure state. It must not silently fall back to stale `_ptInfo` descriptions.

Coeff adapter:

- Consumes `_coeffRegistryVocab`.
- Hydrates scalar-expression help placeholders.
- Canonicalizes aliases.
- Applies source aliases.
- Uses `programParamDefs` when present.
- Tolerates partial vocab objects without crashing, including missing `programParamDefs`.

Do not change Help rendering yet. First introduce the adapter and keep old wrapper functions delegating to it.

Gates:

```bash
bash tests/test_frontend_js.sh
```

### Phase 7: Refactor Help Builders Onto The Adapter

Refactor `js/08-chip-editors.js`.

Current separate helpers:

- `_paramProgramLegacyHelpItem`
- `_paramProgramLegacyCallParams`
- `_coeffNativeTransformHelpItem`
- `_coeffTransformParams`

Target:

```js
function _programRegistryTransformHelpItem(adapter, name) {
    const spec = adapter.uiSpec(name);
    const params = adapter.paramsForHelp(name);
    return _programHelpItem(...);
}
```

Profile-specific formatting remains adapter-owned:

- Param signatures are `legacy(name, src, tgt, ...)`.
- Coeff signatures are `poly = transform(poly, ...)`.

Tests should assert representative Help content:

- Param `rect` shows `w`, `h`, and `turns` with explanations.
- Param `moebius` shows variable compatibility forms.
- Param `unit_circle` shows formula/effect/notes.
- Coeff `sort_mod_keep_angle` shows all params including `andy`.
- Coeff `exp` uses the correct source/program params.
- Null/missing Coeff registry renders a visible “registry not loaded” article instead of crashing.

### Phase 8: Remove Static Param Drift Fallback

After Help and cheat sheets are adapter-backed:

- Delete or quarantine `_ptInfo` and `_ptCategoryMeta`.
- Do not delete `_ptCatalog` in this phase unless its non-Help consumers have been migrated. It is shared chip/popup/preview metadata, not only a Help fallback.
- If a fallback is kept, it must only render an explicit “Param registry not loaded” message.
- It must not silently provide old transform descriptions.

Add a frontend test:

- With `_paramRegistryVocab` present, Param Help must source legacy transform descriptions from generated registry data.
- A changed registry description must appear in Help without editing `js/07-transform-catalogs.js`.

### Phase 9: Share Source Parser Registry Call Validation

Add a small Python helper, likely in `registry_common.py` or `program_source_core.py`:

```python
def canonical_registry_function_name(registry, raw_name, *, alias_map=None, label):
    ...
```

Use it in both:

- `param_program_source.py`
- `coeff_program_source.py`

Param changes:

- `legacy(name, src, tgt, ...)` should reject unknown `name` at source-parse time.
- Error code should be stable, e.g. `unknown_legacy_transform`.
- Bare legacy shortcut names should use the same validator.
- Round-trip safety rule: the accepted-name set must include every registry name and every name that can appear in a saved legacy chain row. Do not add parse-time rejection that makes an existing saved-chain -> source -> compile path fail.

Coeff changes:

- Keep existing alias behavior.
- Move alias lookup through the shared helper.

Tests:

- `parse_param_program_source("legacy(nope, both, both)")` fails with `unknown_legacy_transform`.
- Bad Param source/target selectors fail during parse/lowering with `bad_selector`, not later as uncoded `RuntimeError`.
- Every saved/legacy Param chain row in the compatibility corpus can serialize to source and compile back without changing the compiled chain/fingerprint.
- `parse_coeff_program_source("poly = scale100(poly)")` keeps current chain-only/text-only behavior exactly as specified.
- Alias canonicalization remains byte/fingerprint stable for existing Coeff examples.

### Phase 10: Remaining Optional Arg / Complex Arg Follow-Up

This is related but should not be hidden inside the infrastructure refactor unless explicitly scheduled.

This is not an M3 rewrite. The M3 oracle and forward wire packer already exist, and Coeff `compat_signatures` already declares per-function wire layouts for the functions that need it, including `linear`, `exp`, `round`, and `pow`.

The larger desired model:

- Runtime args have type, default, optional status, and packing semantics.
- UI/source args are generated from the same arg model.
- Coeff `andy` stops being a magic generator append and becomes a normal declared optional argument in the semantic arg model.
- Old trailing-`andy` source forms and the separate compiled-token `andy` field are handled by an explicit compatibility shim outside the semantic registry.
- Param supports complex runtime args cleanly where needed rather than preserving old real-lane compatibility as ad hoc frontend code.

The remaining Phase-10 work is mainly separating `andy` semantics from `andy` compatibility packing, plus cleaner Param complex-arg modeling. It is likely its own follow-up after Phases 1-9 because it changes accepted source forms and compatibility behavior.

### Phase 10A: Coeff `andy` As A Normal Optional Argument

This is a dedicated refactor, not a drive-by cleanup. It touches the Coeff semantic registry, source parser, chain compiler, generated vocab/help, saved-program round trips, and legacy wire packing.

Goal:

- In the semantic language, `andy` is a normal optional argument like any other argument.
- In the legacy/wire compatibility layer, old `andy` storage and old trailing-argument forms are translated to and from that semantic argument.
- The registry does not use `supports_andy`, generated `ANDY_PARAM`, or parser-only `andy_arg_counts` as semantic schema.

Non-goals:

- Do not change the native C wire format in the same patch.
- Do not renumber function indices or alter legacy opcodes.
- Do not change saved-program fingerprints unless an explicit migration/fingerprint decision is made and pinned by tests.
- Do not resolve Param complex-arg cleanup in the same patch.

Target semantic registry shape:

```json
{
  "name": "rev",
  "args": [],
  "optional_args": [
    {
      "name": "andy",
      "type": "real",
      "default": 0.0,
      "ui": {
        "label": "andy",
        "desc": "Blend amount in [0,1]."
      }
    }
  ]
}
```

The exact key name can be `optional_args` or a unified `args` list with `optional: true`; choose one schema and use it everywhere. Preferred: one `args` list with `optional: true` once the loader can preserve positional compatibility cleanly. Use `optional_args` only if mixing required and optional positions would make old arity behavior ambiguous.

Compatibility shim responsibilities:

- Accept current source forms that use a trailing `andy`.
- Accept current saved chain rows and legacy native-transform rows.
- Translate semantic args into the existing compiled token shape, including the separate `token["andy"]` / `token["andy_expr_ref"]` fields where the wire still needs them.
- Translate old compiled/saved rows back into semantic source/help forms without exposing compatibility-only fields as registry schema.
- Own the old `compat_signatures.andy_arg_counts` meaning until those signatures can be represented as ordinary optional args plus explicit legacy aliases.

Implementation milestones:

1. Add loader support for optional args without changing generated vocab or compiler behavior.
   - Validate arg names/types/defaults/help text.
   - Reject duplicate names across required and optional args.
   - Add schema tests proving `andy` can be represented as a normal optional arg.
2. Add a Coeff compatibility adapter module/function.
   - Input: registry spec plus raw source/chain args.
   - Output: semantic required args, semantic optional args, and legacy packing metadata.
   - Keep existing behavior for every current fixture.
3. Move hardcoded generated `ANDY_PARAM` into temporary adapter output.
   - Help/rendering consumes normalized semantic args.
   - Generated output may still include compatibility fields, but they must be named as compatibility fields and have consumers/tests.
4. Update source parser and chain compiler to parse `andy` through the normal optional-arg path.
   - The final packing step may still emit `token["andy"]`.
   - The parser should no longer need a separate `_split_native_transform_andy` rule except inside the compatibility adapter.
5. Remove or deprecate `supports_andy`.
   - First ignore it when semantic optional args are present.
   - Then remove it from generated public vocab.
   - Finally remove it from the registry once tests prove no consumer needs it.
6. Encode the `round` decision explicitly.
   - Semantic signature: `round(src, multiplier: complex = 1+0i, andy: real = 0)`.
   - New source `round(poly, 1, 2)` preserves current meaning: multiplier `1+0i`, `andy=2`.
   - Old packed real-lane rows regenerate as explicit complex multiplier source, e.g. `round(poly, 1+2i, 0.5)`.
   - Do not leave first-match dispatch as implicit behavior; the compatibility adapter owns the conversion.

Required gates:

```bash
uv run python -m pytest \
  tests/test_coeff_program_chain.py \
  tests/test_coeff_program_drift.py \
  tests/test_coeff_program_native.py \
  tests/test_coeff_source_equivalence.py \
  tests/test_coeff_wire_fingerprints.py \
  tests/test_program_m3_oracles.py \
  tests/test_program_help_forms.py \
  tests/test_program_v2_migration.py
bash tests/test_frontend_js.sh
```

Acceptance criteria:

- Help shows `andy` because it is a normal optional argument, not because the generator appended a magic param.
- No semantic registry field exists whose only purpose is old `andy` packing.
- Old saved Coeff programs compile with identical execution specs and fingerprints unless a deliberate migration is documented.
- Source-to-chain-to-source round trips preserve the current corpus.
- `round(poly, 1, 2)` is pinned as multiplier `1+0i` plus `andy=2`.
- Old packed real-lane `round` rows serialize to explicit complex multiplier source.
- `round`, `linear`, `exp`, and `pow` compatibility signatures remain wire-stable and tested.
- Public generated vocab does not expose dead `supportsAndy`, `effectiveArgs`, or `compatSignatures` fields unless a concrete frontend consumer is named and tested.

## Test Plan

Run focused gates after each phase, not only at the end.

### Python Registry Schema

```bash
uv run python -m pytest tests/test_registry_schema.py -q
```

This suite should include:

- Param and Coeff common schema assertions.
- Param selector validation.
- Alias/name canonical-shape validation.
- Param compat-family completeness.
- Coeff ambiguous compat-signature overlap detection.
- Coeff generic-complex runtime arg guard.
- Registry-declared reduced-arity forms for every accepted reduced-arity Param transform.
- Registry-declared boundary translation compatibility for Param target-first/last/dither behavior and Coeff packed real-lane legacy forms.

### Generated Vocab Drift

```bash
uv run python lambda/gen_param_vocab.py --check
uv run python lambda/gen_coeff_vocab.py --check
uv run python -m pytest tests/test_param_program_drift.py tests/test_coeff_program_drift.py -q
uv run python -m pytest tests/test_program_m3_oracles.py tests/test_coeff_wire_fingerprints.py -q
```

The Coeff drift suite includes the generic-complex-arg guard; keep it in this gate because registry schema acceptance and wire safety must move together.

### Source Parser Behavior

```bash
uv run python -m pytest tests/test_param_program_source.py tests/test_coeff_source_equivalence.py tests/test_coeff_program_native.py -q
```

Add explicit tests for:

- Param unknown legacy name.
- Param bad source selector.
- Param bad target selector.
- Param defaulted calls for every registry function where omitted args are declared valid.
- Param rejected reduced-arity forms where omitted args are not declared valid.
- Coeff chain-only alias behavior, including `scale100`.
- Coeff legacy opcode versus native-transform opcode examples that must remain distinct unless a deliberate fingerprint migration is approved.
- Root top-level `source_text` run-param policy, whichever direction is chosen.
- Empty source policy for Param, Coeff, Root, and Solve-score.

### Fingerprint And Macro Behavior

```bash
uv run python -m pytest tests/test_param_program_chain.py tests/test_coeff_program_chain.py tests/test_program_v2_migration.py -q
```

Add explicit tests for:

- Param signed-zero canonicalization.
- Param saved macro source-text precedence.
- Existing saved-chain-only Param macro payloads.
- Param old-chain migration must not persist unparseable synthesized source into v2 fingerprints.
- Coeff `round` ambiguous arity behavior, either pinned as compatibility or rejected with a coded diagnostic.
- Explicit empty `param_program_chain: []` and `coeff_program_chain: []` do not fall through to legacy transform arrays.
- Compiler-stage diagnostics include stable `code` fields in strict=False mode.

### Boundary Translator Drift

```bash
uv run python -m pytest tests/test_param_program_drift.py tests/test_coeff_program_drift.py -q
```

Add explicit tests for:

- `pipeline_programs.py` Param target-first/target-last/dither sets match registry compat.
- C Param independent-target and target-index behavior matches registry compat or a checked generated export.
- `pipeline_programs.py` Coeff `exp`/`round` old-form repacking remains consistent with `compat_signatures`.

### Frontend Help/Adapter

```bash
bash tests/test_frontend_js.sh
```

Add targeted frontend assertions for generated Help content.

Add targeted frontend assertions for:

- Partial Coeff vocab does not crash.
- Missing Param vocab renders an explicit registry-load failure.
- Param Help text comes from generated vocab when present.

### Full Gate

```bash
bash scripts/predeploy_check.sh
```

Because `uv` cache access is restricted under the sandbox, run this with escalation in this environment.

## Implementation Order

> **SUPERSEDED (2026-07-02)** by the "Superseding Implementation Order" above — 12 of the 14 steps below are landed or obsolete. Kept as historical record.

Recommended order:

1. Fix Phase -1 live correctness bugs first: Solve-score numeric fingerprint collisions, root native chain cap mismatch, root lossless source regeneration, Param signed-zero fingerprints, Param chain-to-source round-trip safety, Param macro source precedence, canonical v2 fingerprint payloads, and v2 empty-field precedence.
2. Fix Phase -1 boundary/diagnostic/dead-code traps: empty-program and blank-source policy/falsy fallbacks, Coeff partial-vocab crash, Coeff `round` ambiguity, Param/Coeff parse-time validation/default handling, removed frontend editor dead-code cleanup, root run-boundary diagnostics, solve-score non-strict fallback diagnostics, boundary translator drift gates, C default/packing/metric-partition drift gates, and compiler diagnostic structure.
3. Add shared schema tests.
4. Add `registry_common.py`.
5. Refactor generators.
6. Refactor runtime loaders and make Param runtime compat registry-authoritative.
7. Normalize Param `category_meta`.
8. Wire or remove Param `variable_arg_forms`.
9. Add frontend registry adapter.
10. Refactor Help builders onto adapter.
11. Remove static Param fallback metadata.
12. Share source parser registry-name validation.
13. Implement Phase 10A as its own branch/commit series after registry plumbing, generator/help, and source-parser behavior are stable.
14. Only then consider broader optional/complex arg model changes.

Do not start by changing accepted source syntax. First make current behavior shared, tested, and generated. Then extend the language.

## Risks

### Risk 1: Breaking Saved Legacy Programs

Param legacy transforms are compatibility-critical. Target-arg migration and variable arg forms must remain byte/fingerprint equivalent.

Mitigation:

- Keep chain compiler behavior unchanged during infrastructure phases.
- Add source/chain round-trip tests for representative old rows.
- Add explicit tests for signed-zero canonicalization so fixing duplicate Param fingerprints does not create a new fingerprint split.

### Risk 2: Frontend Help Silently Falling Back To Stale Data

Static `_pt*` fallback can mask registry bugs.

Mitigation:

- Remove silent fallback.
- Render explicit registry-load failure state.
- Add tests that mutate/generated registry text is visible through Help.
- Add tests for missing Param vocab and partial Coeff vocab.

### Risk 3: Over-Generalizing The Compilers

Param and Coeff execution semantics differ.

Mitigation:

- Share registry plumbing only.
- Keep profile augmentation functions separate.
- Keep compiler lowering separate.

### Risk 4: `andy` Becomes More Entrenched

Moving shared infrastructure without addressing `andy` could preserve an ugly special case.

Mitigation:

- Do not expand `andy` magic.
- Keep the semantic registry sacred: it describes normal function arguments, not historical wire mistakes.
- In the follow-up optional-arg model, represent `andy` as a normal optional argument, not generator code and not a `supports_andy` capability.
- Move old trailing-`andy` arities and separate-token-field packing into a named compatibility shim.
- Before the follow-up, derive existing `andy` arities and packed stack limits from compatibility metadata where possible, but do not pretend that metadata is semantic function schema.

### Risk 5: Schema Migration Breaks Generated Assets

Changing Param from `ui.categories` to `category_meta` can stale `param_vocab_js.js`.

Mitigation:

- Keep generator `--check` in predeploy.
- Regenerate in the same commit as schema migration.
- Add tests requiring final canonical key.

### Risk 6: Registry Refactor Preserves Existing Private Runtime Authority

The refactor could accidentally share generator/schema code while leaving runtime behavior driven by private constants.

Mitigation:

- Treat Param `compat` as a runtime input, not only a generator input.
- Add drift tests for all Param compat families before moving loader code.
- Remove truthiness fallback from registry values to compiler constants.
- Include `pipeline_programs.py` and `sweep_cli.c` in the drift surface, not only Python compiler loaders.

### Risk 7: Coeff Compatibility Signatures Stay Ambiguous

`round` already has overlapping arity forms. A generic helper could make this look intentional everywhere.

Mitigation:

- Detect overlaps in schema tests.
- Keep a narrow documented exception only for existing compatibility behavior.
- Prefer coded ambiguity errors for new source forms.

### Risk 8: Empty Programs Mean Different Things At Different Boundaries

The chain compilers accept empty Param/Coeff programs, but source/save/run handlers do not agree on whether empty means no-op, invalid input, or "fall back to legacy transforms."

Mitigation:

- Decide profile policy explicitly.
- Use key-presence checks instead of truthiness.
- Test run-boundary precedence with explicit empty chains plus non-empty legacy transform arrays.

### Risk 9: Source Diagnostics Improve At Parse But Still Collapse At Compile

Parser errors can be coded while compiler-stage errors remain message-only.

Mitigation:

- Add compiler diagnostic codes in strict=False payloads.
- Add source compile endpoint tests for compiler-stage failures, not only parse-stage failures.

### Risk 10: Canonicalization Fixes Change Cache Identity

The solve-score numeric formatting fix and v2 fingerprint payload fix deliberately change identity behavior. Root source-regeneration fixes can also change visible source text.

Mitigation:

- Treat them as cache-version-affecting fixes.
- Land tests that prove the old behavior was unsafe.
- Prefer correctness over preserving broken cache hits.
- Communicate that existing artifacts with old solve-score fingerprints may not be reused under the fixed canonicalization.
- For root, verify source text changes are execution-preserving before changing stored fingerprints.

### Risk 11: Native C Runtime Remains The Hidden Authority

The registry refactor can make JSON/generators/tests look clean while C still owns omitted defaults, packing ladders, and caps.

Mitigation:

- Add C-vs-registry drift tests before changing defaults or packing.
- Make Python expand defaults and packing explicitly before native execution where that is the chosen contract.
- Keep profile/native caps pinned together.
- Do not claim the registry is authoritative for a field until native execution is either generated from it or drift-tested against it.

### Risk 12: Migration Looks Canonical While Preserving The Wrong Program

V2 translation is a high-risk boundary because it turns older ambiguous payloads into metadata that later looks clean.

Mitigation:

- Use explicit key-presence rules in translators.
- Never treat explicit empty lists as absent.
- Add tests where old and new fields disagree.
- Add tests where `source_text`, `chain`, and legacy fields disagree.
- Reject or diagnose unsupported empty solve-score payloads instead of silently injecting defaults.

## Definition Of Done

> **Rewritten 2026-07-02** against the verified state. Already-met bullets moved to the first block; unmeetable-as-written bullets dropped with reasons.

**Already met (verified at `ba0f962`/`35f06fb`):**

- Phase -1 live bugs fixed and covered (solve-score collisions, root cap enforcement, root regeneration, param signed-zero, chain-to-source guard, macro precedence, v2 fingerprints/empty-fields, partial-vocab `programParamDefs` guard, parse-time validation, zero-defaults, round policy).
- One shared Python module (`registry_common.py`); both generators and both loaders use it.
- Param runtime compat loaded from registry `compat`; boundary translation registry-derived (param) or drift-pinned (coeff).
- C param compat + C default/packing drift-tested against the registry (`67ff066`).
- Cross-kind fingerprint tests exclude display/source text and handle signed-zero.
- Category metadata convention shared; `variable_arg_forms` rendered; static Param fallback gone (explicit not-loaded state); Help consumes the per-profile adapters; shared schema tests exist.
- Coeff andy is declared registry data (`shared_optional_args`); M3 oracle + wire fingerprint tests green.

**Dropped (superseded by the Plan Review Verdict):**

- ~~"Source parsers use shared registry-name validation"~~ — the achieved per-kind validation is the better outcome; the remaining defect is the 6-line `inv_t_plus_2` hardcode, not a missing abstraction.
- ~~The `_makeProgramRegistryAdapter` factory~~ — two converging adapters are the endpoint.
- ~~One canonical formatter/hash across kinds~~ — impossible without orphaning frozen-wire caches; replaced by "two named frozen policies + conformance tests".

**The refactor is DONE when the Superseding Implementation Order lands:**

1. The six ungated test files are in `scripts/predeploy_check.sh`, plus a meta-test that keeps the gate list complete.
2. H1–H4 are fixed and regression-tested (param save, v2 lifecycle, littlewood, C `abs()` hypot).
3. The `inv_t_plus_2` shadowing lines are deleted with a source-vs-registry arity assertion.
4. The H5 fix-commit regressions are closed (null-chain, help mispairing, compat load-time raise, param_debug, optional-arg collision validation).
5. The dead-code sweep lands (dead vocab keys + their pins, dead helpers, dead subfields).
6. Root cap three-way pinned (profile = Python = C = 16).
7. Oracle integrity policy documented; registry fixtures snapshotted for oracle paths; source-equivalence test renamed to its true scope.
8. Boundary conformance tests pin the four-case blank/empty matrix per kind, the `solve_score.h` partitions, and storage-vs-compute macro resolution.
9. Full predeploy green, M3 oracle + wire fingerprints green throughout.
