# Code Review 6 — Current State Review

**Date:** 2026-06-11  
**Scope:** current checked-in tree, focused on Coeff Program source/chip
bridging, backend source/chain precedence, native VM refactor, and deploy
packaging.

**Tests run:**

- `uv run python -m pytest tests/test_coeff_program_chain.py tests/test_coeff_program_native.py tests/test_coeff_program_drift.py tests/test_deploy_packaging.py -q`
- `bash tests/test_frontend_js.sh`

Result: all targeted tests passed (`138 passed`; frontend source/runtime checks
passed). The frontend issue below is not covered by the current JS test style.

---

## Findings

### [blocker] Coeff Program chip serialization recurses forever in the browser

`index.html:12933-12935`

```js
function _canonicalCoeffProgramChipName(name) {
    // 'const' is the historical alias of the push_const chip.
    return _canonicalCoeffProgramChipName(name);
}
```

This is called by `_serializeCoeffProgramChain()` at `index.html:13701`. Any
non-legacy Coeff Program chip serialization path can hit a `RangeError:
Maximum call stack size exceeded`.

User-visible impact:

- Saving a chip-mode Coeff Program can fail.
- Compute/preview/debug paths that serialize the active chip chain can fail.
- Any UI status path calling `_serializeCoeffProgramChain().length` can fail.

Why tests missed it:

- `tests/test_frontend_js.sh` is mostly grep/source-shape checks. It does not
execute `_serializeCoeffProgramChain()` with a real `_coeffProgramChain`.

Fix shape:

```js
function _canonicalCoeffProgramChipName(name) {
    const raw = String(name || '').trim();
    if (raw === 'const') return 'push_const';
    return _coeffProgramRegistryChipName(raw);
}
```

Add a real frontend runtime test that evaluates the relevant JS functions and
asserts:

```js
_coeffProgramChain = [{ name: 'push_vec', params: ['poly_len', '0'] }];
_serializeCoeffProgramChain(); // no throw
```

This should be fixed before deploy.

---

### [major] Empty `source_text` shadows a non-empty saved chain in backend saved-program paths

The compute request path has the right precedence rule:

`handler_compute_plan.py:237-244`

```python
coeff_program_source_text = (
    raw_coeff_program_source_text
    if pipeline_mode == "program"
    and "coeff_program_source_text" in run_params
    and (raw_coeff_program_source_text.strip() or not run_params.get("coeff_program_chain"))
    else None
)
```

Same pattern exists in `handler_compute_preview.py:298-305` and
`handler_coeffgen.py:121-130`: non-empty source wins, but empty source does not
discard a non-empty chain.

Saved-program and macro paths do not use the same rule:

- `handler_storage.py:353-355` parses `source_text` whenever the key exists.
- `handler_storage.py:561-565` passes `source_text=payload.get("source_text")`
  whenever the key exists.
- `handler_compute_plan.py:97-100`, `handler_compute_preview.py:159-161`, and
  `handler_coeffgen.py:96-98` do the same for saved coeff-program macros.

Concrete failure:

```json
{
  "program_kind": "coeff_program",
  "chain": [["push", "cf"], ["emit"]],
  "source_text": ""
}
```

UI-side loading already treats this as a chain program, but backend fetch/macro
expansion treats it as empty source and returns an empty chain. A user-imported
program or stale S3 object with an empty `source_text` field can silently become
identity/empty when fetched or used as a macro.

Fix shape:

- Add a shared helper in each backend module or a small shared utility:

```python
def _coeff_source_text_from_payload(payload):
    chain = payload.get("chain")
    raw = str(payload.get("source_text") or "")
    if "source_text" in payload and (raw.strip() or not chain):
        return raw
    return None
```

- Use that helper in storage fetch, storage macro resolver, compute-plan macro
resolver, compute-preview macro resolver, and coeffgen macro resolver.
- Add tests for saved/fetched and macro-expanded payloads with
  `source_text: ""` plus a non-empty `chain`.

This is less urgent than the frontend recursion, but it is a real semantic
drift between UI and backend.

---

## Current State Notes

The large CR5 cleanup pass has already been substantially addressed:

- `CoeffEvalContext` exists and the C evaluator now uses an enum-ordered switch
  dispatch (`sweep_cli.c:5093-5160`).
- `deploy.sh` now has a `deploy_lambda` wrapper and one shared Lambda spec list
  (`deploy.sh:1424-1568`).
- Lambda packaging now uses `zip -FS -r9`, so stale deleted files in existing
  `/tmp/polypaint-*.zip` archives are less likely to persist.
- Coeff Program source derives unary/binary vocabularies from
  `coeff_program_chain.py` instead of hand-copying most of them
  (`coeff_program_source.py:55-62`).
- `FN_LINEAR`, `FN_EXP`, `FN_ROUND`, and `FN_POW` constants exist and removed
  most raw magic IDs (`coeff_program_chain.py:37-40`).
- The JS `andy` metadata has a semantic `kind` comment, and the raw placeholder
  string is no longer the only documented contract (`index.html:11626-11628`).

Residual maintainability debt remains, but I would not block deploy on it:

- `deploy.sh` still has many hand-written packaging blocks, even though the
  create/update deployment list is unified.
- The frontend test harness needs at least a small executable JS path for core
  serialization functions. Grep tests are useful, but they cannot catch bugs
  like the recursive canonicalizer.
- Saved-program source/chain precedence should be centralized so storage,
  compute plan, preview, and coeffgen cannot drift again.

## Verdict

Do not deploy until the `_canonicalCoeffProgramChipName` recursion is fixed and
covered by a runtime frontend test.

After that, the remaining concrete semantic issue is the empty `source_text`
backend precedence bug. Fixing it before deploy is prudent if saved/imported
programs or macros may contain both keys. Otherwise it can be a near-follow-up,
but it should not be left undocumented because it contradicts the UI and compute
request precedence rule.

---

## Resolution — 2026-06-11

**[blocker] recursion — ✅ fixed.** The helper body is the real mapping
(`return name === 'const' ? 'push_const' : name;`). Root cause: the
aesthetics-pass replace-all that introduced the helper also rewrote the
helper's own body into a self-call (the same failure mode that hit `_str`,
where the harness caught it because that path executes). Guards added to
`tests/test_frontend_js.sh`:

- a source pin on the helper body, and
- a real runtime test: `_serializeCoeffProgramChain()` now executes in the
  harness VM over a four-row chain (canonical chip, `const` alias, legacy row
  with default andy, legacy row with non-default andy) and asserts the exact
  serialized rows.

Both verified to fail on the recursive body (FATAL) and pass on the fix. A
scan for other self-calling one-expression helpers found none.

**[major] blank `source_text` shadowing a saved chain — ✅ fixed.** New shared
helper `coeff_source_text_from_payload(payload)` in
`lambda/coeff_program_source.py` (bundled with all four handlers already —
no packaging change) implements the UI/compute-request rule: source wins
when non-blank or when there is no chain to fall back to; otherwise the
chain compiles. Applied at all six sites:

- `handler_storage.py` saved-program macro/source reader, fetch path, and
  the save route (the save route had the same hazard for non-UI clients
  posting both keys),
- the `_read_saved_program_source_chain` macro readers in
  `handler_compute_plan.py`, `handler_compute_preview.py`, and
  `handler_coeffgen.py`.

Tests: `tests/test_saved_program_source_precedence.py` (helper unit cases +
all three macro resolvers + non-blank-source-still-wins) and a storage
fetch/macro/save round-trip in `tests/test_coeff_program_storage.py`.
Flip-verified: reverting a site makes the new tests fail.

The residual notes stand: packaging blocks remain hand-written (the spec
list itself is unified), and centralized precedence now exists for the
saved-program paths.
