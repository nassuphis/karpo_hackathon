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
  "ui": {
    "categories": {...},
    "variable_arg_forms": {...}
  },
  "compat": {
    "target_arg_indexes": {...},
    "independent_targets": [...],
    "variable_arg_counts": {...}
  }
}
```

Important consumers:

- Runtime loader: `lambda/param_program_chain.py::_load_legacy_registry`.
- Source parser: `lambda/param_program_source.py::_lower_call`.
- JS generator: `lambda/gen_param_vocab.py`.
- Generated frontend payload: `param_vocab_js.js`.
- Frontend catalog and help adapter: `js/07-transform-catalogs.js`, `js/08-chip-editors.js`.
- Drift tests: `tests/test_param_program_drift.py`.

Recent local state is materially better than the old schema: Param functions now have function-local `ui.desc`, and runtime args have local `ui.params`. That is the right direction. The remaining issue is that Param is still not using the same registry infrastructure as Coeff.

### Coeff Registry

File: `lambda/coeff_legacy_registry.json`

Current shape:

```json
{
  "version": 1,
  "functions": [
    {
      "name": "rev",
      "fn_index": 1,
      "category": "structural",
      "allowed_src": ["cf", "poly", "pop", "peek"],
      "allowed_tgt": ["poly", "push"],
      "args": [],
      "length_policy": "same",
      "supports_andy": true,
      "ui": {
        "desc": "..."
      }
    }
  ],
  "category_meta": {...}
}
```

Additional Coeff-only fields are legitimate:

- `aliases`
- `chain_only_aliases`
- `chip_name`
- `compat_signatures`
- `length_policy`
- `supports_andy`
- `ui.program_params`

Important consumers:

- Runtime loader: `lambda/coeff_program_chain.py::_load_legacy_registry`.
- Source parser: `lambda/coeff_program_source.py::_canonical_native_name`, `_lower_call`.
- JS generator: `lambda/gen_coeff_vocab.py`.
- Generated frontend payload: `coeff_vocab_js.js`.
- Frontend catalog and help adapter: `js/07-transform-catalogs.js`, `js/08-chip-editors.js`.
- Drift tests: `tests/test_coeff_program_drift.py`.

Coeff is closer to generated-only frontend metadata than Param, but it still has a separate generator, separate runtime loader, separate frontend adapter, and an `andy` special case split between registry data and generator code.

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
- Currently carry universal `andy` support.
- Some functions need different chip params versus source-program params (`ui.params` vs `ui.program_params`).

Do not merge the compilers or force one registry to pretend it is the other. Merge the registry infrastructure around them.

## Spurious Differences And Drift Surfaces

### 1. Duplicate Generator Skeletons

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

`js/07-transform-catalogs.js` still has `_ptCategoryMeta` and `_ptInfo` as static Param legacy metadata, plus `_ptCatalog` enrichment.

That was useful before the generated registry became complete. It is now a drift surface:

- Help can silently fall back to stale text.
- Registry and frontend can disagree while tests still pass.
- `_ptInfo` and `_ptCategoryMeta` duplicate function descriptions/categories that now belong in `param_legacy_registry.json`.
- `_ptCatalog` is broader than Help fallback: it is shared chip/popup/preview metadata and is used outside this Help path. Do not delete it casually.

The final state should not use static Param legacy descriptions/categories when `_paramRegistryVocab` is present. `_ptCatalog` may remain until its non-Help consumers are explicitly migrated.

### 5. Param `variable_arg_forms` Is Inert

`lambda/param_legacy_registry.json` has `ui.variable_arg_forms`, but current consumers do not reference it.

This is dangerous because it looks authoritative but has no effect.

Decision required:

- Either wire variable forms into generated Help articles, or
- Move them into `compat` as backend-only comments/metadata, or
- Delete them if `compat.variable_arg_counts` is sufficient.

Preferred: move useful user-facing forms to function-local `ui.forms` or `ui.compat_forms` and render them in Help.

### 6. Source Parser Registry Handling Differs

Coeff source parsing canonicalizes registry aliases and rejects unknown registry names early.

Param source parsing accepts:

```text
legacy(name, src, tgt, ...)
```

and only lowercases `name`; compile-time chain validation catches unknown names later.

This is avoidable drift. Both parsers should use a shared registry-name validator/canonicalizer, configured per profile.

### 7. Frontend Registry Adapters Are Separate

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

Param and Coeff each have their own generated-vocab assertions. That lets the schemas diverge while both test suites remain green.

Add a shared registry-schema test that runs against both registries.

## Target Architecture

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
- `supports_andy`

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

### Phase 0: Freeze The Current Baseline

Before refactoring, preserve current behavior.

Important current baseline: the M3 wire/fingerprint infrastructure is already landed. `compat_signatures` and `effective_args` are not future design notes; they are load-bearing registry data used by the frozen M3 oracle and forward wire packer. This refactor must keep those gates green throughout.

Steps:

1. Regenerate current vocab artifacts:

```bash
uv run python lambda/gen_param_vocab.py
uv run python lambda/gen_coeff_vocab.py
```

2. Run focused gates:

```bash
uv run python -m pytest tests/test_param_program_drift.py tests/test_coeff_program_drift.py tests/test_param_program_source.py tests/test_coeff_program_source.py -q
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

Complex-arg safety rule:

- Both profile configs may list `complex` as an allowed future type, but no current Coeff registry runtime arg uses `type: "complex"`.
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

Coeff-specific cleanup:

- Keep `ANDY_PARAM` behavior initially to avoid broad behavior change.
- Prefer moving `ANDY_PARAM` construction into a shared optional-param helper or registry data in a later phase.
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
    }
```

Coeff augmentation keeps:

- `category`
- `allowed_src`
- `allowed_tgt`
- `args`
- `effective_args`
- `compat_signatures`
- `supports_andy`
- `length_policy`
- `aliases`
- `chain_only_aliases`
- `chip_name`
- `ui`

Do not simplify or reorder `compat_signatures` or `effective_args` while doing the loader refactor. They are wire/fingerprint inputs, not display-only metadata.

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

Coeff adapter:

- Consumes `_coeffRegistryVocab`.
- Hydrates scalar-expression help placeholders.
- Canonicalizes aliases.
- Applies source aliases.
- Uses `programParamDefs` when present.

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
- Every saved/legacy Param chain row in the compatibility corpus can serialize to source and compile back without changing the compiled chain/fingerprint.
- `parse_coeff_program_source("poly = scale100(poly)")` keeps current chain-only/text-only behavior exactly as specified.
- Alias canonicalization remains byte/fingerprint stable for existing Coeff examples.

### Phase 10: Remaining Optional Arg / Complex Arg Follow-Up

This is related but should not be hidden inside the infrastructure refactor unless explicitly scheduled.

This is not an M3 rewrite. The M3 oracle and forward wire packer already exist, and Coeff `compat_signatures` already declares per-function wire layouts for the functions that need it, including `linear`, `exp`, `round`, and `pow`.

The larger desired model:

- Runtime args have type, default, optional status, and packing semantics.
- UI/source args are generated from the same arg model.
- Coeff `andy` stops being a magic generator append and becomes a declared optional arg or declared shared optional arg.
- Param supports complex runtime args cleanly where needed rather than preserving old real-lane compatibility as ad hoc frontend code.

The remaining Phase-10 work is mainly `andy` as a declared optional arg plus cleaner Param complex-arg modeling. It is likely its own follow-up after Phases 1-9 because it changes accepted source forms and compatibility behavior.

## Test Plan

Run focused gates after each phase, not only at the end.

### Python Registry Schema

```bash
uv run python -m pytest tests/test_registry_schema.py -q
```

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
uv run python -m pytest tests/test_param_program_source.py tests/test_coeff_program_source.py -q
```

### Frontend Help/Adapter

```bash
bash tests/test_frontend_js.sh
```

Add targeted frontend assertions for generated Help content.

### Full Gate

```bash
bash scripts/predeploy_check.sh
```

Because `uv` cache access is restricted under the sandbox, run this with escalation in this environment.

## Implementation Order

Recommended order:

1. Add shared schema tests.
2. Add `registry_common.py`.
3. Refactor generators.
4. Refactor runtime loaders.
5. Normalize Param `category_meta`.
6. Wire or remove Param `variable_arg_forms`.
7. Add frontend registry adapter.
8. Refactor Help builders onto adapter.
9. Remove static Param fallback metadata.
10. Share source parser registry-name validation.
11. Only then consider optional/complex arg model changes.

Do not start by changing accepted source syntax. First make current behavior shared, tested, and generated. Then extend the language.

## Risks

### Risk 1: Breaking Saved Legacy Programs

Param legacy transforms are compatibility-critical. Target-arg migration and variable arg forms must remain byte/fingerprint equivalent.

Mitigation:

- Keep chain compiler behavior unchanged during infrastructure phases.
- Add source/chain round-trip tests for representative old rows.

### Risk 2: Frontend Help Silently Falling Back To Stale Data

Static `_pt*` fallback can mask registry bugs.

Mitigation:

- Remove silent fallback.
- Render explicit registry-load failure state.
- Add tests that mutate/generated registry text is visible through Help.

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
- In the follow-up optional-arg model, represent `andy` as declared optional metadata, not generator code.

### Risk 5: Schema Migration Breaks Generated Assets

Changing Param from `ui.categories` to `category_meta` can stale `param_vocab_js.js`.

Mitigation:

- Keep generator `--check` in predeploy.
- Regenerate in the same commit as schema migration.
- Add tests requiring final canonical key.

## Definition Of Done

The refactor is complete when:

- One shared Python module owns generic registry loading/validation/render helpers.
- Param and Coeff generators both use the shared module.
- Param and Coeff runtime loaders both use the shared module.
- Param and Coeff registry schema share the same category metadata convention.
- Inert `variable_arg_forms` is either rendered or removed.
- Frontend Help consumes a normalized adapter rather than raw profile-specific globals.
- Static Param transform descriptions are no longer a silent fallback.
- Source parsers use shared registry-name validation.
- Shared schema tests cover both registries.
- M3 oracle and Coeff wire fingerprint tests remain green after generator and loader refactors.
- Full predeploy is green.
