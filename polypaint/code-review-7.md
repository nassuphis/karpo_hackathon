# Code Review 7 — Post-Fix Deep Dive

**Date:** 2026-06-11  
**Scope:** verification of `code-review-6.md` fixes plus a deeper pass over
Coeff Program source/chip bridging, saved-program source precedence, typed VM
coverage, Param Program legacy state, deploy packaging, and targeted tests.

## Findings

### [major] Coeff Program still uses the secondary scalar-expression VM in active paths

The typed-stack lowering is real and covers most source-text scalar expressions,
but the secondary `scalar_exprs` / `expr_refs` path is still active for current
Coeff Program features, not only old compatibility payloads.

Active source-text paths still lower expression strings into chip rows that
compile to `expr_refs`:

- `lambda/coeff_program_source.py:459-462`: `range(...)` / `linspace(...)`
  call `_lower_range(...)`, which emits `push_range` / `push_linspace` rows
  containing expression strings.
- `lambda/coeff_program_source.py:654-666`: `poke_tos(...)` still emits the
  old `poke_tos(index, value)` chip row instead of typed-stack `_typed_poke_poly`
  style lowering.
- `lambda/coeff_program_source.py:667-670`: `littlewood(...)` emits expression
  strings for value1/value2/andy.
- `lambda/coeff_program_source.py:632-633`: fallback native-transform aliases
  still route through `_lower_native_transform_call(...)`, which can produce
  expression-arg tokens.

Those rows compile through `lambda/coeff_program_chain.py` into `expr_refs`:

- `lambda/coeff_program_chain.py:975-1003`: dynamic length/range args compile to
  `expr_refs`.
- `lambda/coeff_program_chain.py:1166-1185`: `littlewood` fields compile to
  `expr_refs`.
- `lambda/coeff_program_chain.py:1268-1304`: packed legacy/native transform args
  still compile through scalar-expression references.

Native still evaluates those refs through the separate scalar-expression VM:

- `lambda/sweep_cli.c:3848`: `coeffEvalScalarExpr(...)`.
- `lambda/sweep_cli.c:4051-4063`: token arg lookup calls
  `coeffEvalScalarExpr(...)` when `expr_refs[idx] >= 0`.
- `lambda/sweep_cli.c:4111-4122`: `andy_expr_ref` also calls
  `coeffEvalScalarExpr(...)`.

Functional impact: current behavior is tested and passes. Architecture impact:
the stated target "typed VM only / no secondary scalar VM" is not complete. If
that target is a deploy blocker, this must be fixed before claiming completion.
If it is Phase 2 cleanup, document it explicitly and keep the compatibility path
bounded.

### [concern] Browser transform-alias canonicalization is narrower than backend canonicalization

Backend aliasing is broad:

- `lambda/coeff_program_chain.py:208-212` accepts `exp_affine`, `pow_affine`,
  `power_series`, and `scale100`.
- `lambda/coeff_program_source.py:105-109` mirrors the source aliases for
  `exp_affine`, `pow_affine`, and `power_series`.

The browser transform-name canonicalizer is narrower:

```js
function _canonicalCoeffTransformName(name) {
    const raw = String(name || '').trim();
    if (raw === 'exp_affine') return 'exp';
    return raw === 'scale100' ? 'linear' : raw;
}
```

Location: `index.html:11763-11767`.

Normal source text and normal named chip rows are mostly safe because
`_coeffProgramRegistryChipNames` maps registry names to the dedicated chip names.
The edge case is manual/imported `legacy` chip rows whose legacy-name field uses
`pow_affine` or `power_series`. Those names compile in the backend, but the UI
may miss the correct `_ctCatalog` metadata/default trimming because the browser
does not canonicalize them to `pow` / `power`.

Recommendation: extend `_canonicalCoeffTransformName(...)` to mirror backend
aliases exactly and add a frontend runtime assertion for `pow_affine` and
`power_series` legacy rows.

### [concern/policy] Param Program still supports legacy bridge chips in Program mode

Param Program still accepts `legacy` rows:

- `lambda/param_program_chain.py:999-1005`: `legacy(...)` and bare legacy names
  compile through `_legacy_tokens(...)`.
- `lambda/param_program_chain.py:1191-1214`: compiled output still reports
  `scalar_exprs`, `uses_legacy_fast_path`, and `legacy_transforms`.

Tests also assert legacy behavior:

- `tests/test_param_program_chain.py`
- `tests/test_param_program_native.py`
- `tests/test_compute_plan.py`

This is not a regression from `code-review-6.md`; it is current policy in the
tree. If the new policy is "Program mode should not expose or run legacy bridge
chips," that cleanup is separate work and is not done.

## Verified Fixed From `code-review-6.md`

### `_canonicalCoeffProgramChipName` recursion

Fixed.

Current implementation at `index.html:12933-12936`:

```js
function _canonicalCoeffProgramChipName(name) {
    // 'const' is the historical alias of the push_const chip.
    return name === 'const' ? 'push_const' : name;
}
```

The runtime frontend test now executes `_serializeCoeffProgramChain()` over a
multi-row chain and asserts exact serialized output. The old recursive helper
would fail this test with a call-stack overflow.

### Blank `source_text` shadowing saved chains

Fixed.

Central helper at `lambda/coeff_program_source.py:839-854`:

```python
def coeff_source_text_from_payload(payload):
    if not isinstance(payload, dict) or "source_text" not in payload:
        return None
    raw = str(payload.get("source_text") or "")
    if raw.strip() or not payload.get("chain"):
        return raw
    return None
```

Applied at all relevant saved-program reader sites:

- `lambda/handler_storage.py:353`
- `lambda/handler_storage.py:565`
- `lambda/handler_storage.py:1191`
- `lambda/handler_compute_plan.py:99`
- `lambda/handler_compute_preview.py:160`
- `lambda/handler_coeffgen.py:97`

The stale shape `{ "chain": [...], "source_text": "" }` now compiles the chain
instead of silently becoming an empty source program.

## Additional Checks

### Packaging

`coeff_program_source.py`, `coeff_program_chain.py`, and
`coeff_legacy_registry.json` are packaged into the relevant deployed zips:

- `deploy.sh:951`: coeffgen package.
- `deploy.sh:971`: storage package.
- `deploy.sh:1020`: compute-preview package.
- `deploy.sh:1310`: compute-plan package.

Param Program modules and registry are also packaged where expected:

- `deploy.sh:950`
- `deploy.sh:970`
- `deploy.sh:1019`
- `deploy.sh:1043`
- `deploy.sh:1309`

`deployment-checklist.md` section 19 is no longer duplicated; only
`## 19. Post-Deploy Reality Check` remains.

### Native/token limits

Coeff Program token limit is consistently raised to 256:

- `lambda/coeff_program_chain.py:20`: `MAX_PROGRAM_TOKENS = 256`.
- `lambda/sweep_cli.c:3343`: `COEFF_PROGRAM_MAX_TOKENS 256`.

This fixes the earlier 64-token expansion limit problem.

## Tests Run

```bash
uv run python -m pytest \
  tests/test_coeff_program_chain.py \
  tests/test_coeff_program_native.py \
  tests/test_coeff_program_drift.py \
  tests/test_coeff_program_storage.py \
  tests/test_saved_program_source_precedence.py \
  tests/test_compute_plan.py \
  tests/test_compute_preview_handler.py \
  tests/test_coeffgen_param_gen.py \
  tests/test_deploy_packaging.py -q
```

Result: `189 passed in 1.70s`.

```bash
uv run python -m pytest \
  tests/test_param_program_chain.py \
  tests/test_param_program_native.py \
  tests/test_param_program_storage.py -q
```

Result: `36 passed in 0.18s`.

```bash
bash tests/test_frontend_js.sh
```

Result: frontend source checks OK, frontend runtime checks OK.

```bash
bash -n deploy.sh
```

Result: passed.

```bash
python3 -m py_compile \
  lambda/coeff_program_chain.py \
  lambda/coeff_program_source.py \
  lambda/handler_storage.py \
  lambda/handler_compute_plan.py \
  lambda/handler_compute_preview.py \
  lambda/handler_coeffgen.py
```

Result: passed.

## Not Run

The full Docker runtime gate was not run:

```bash
bash scripts/test-docker-runtime.sh
```

The full predeploy gate was not run:

```bash
bash scripts/predeploy_check.sh
```

Per `deployment-checklist.md`, do not treat this review as full deploy
clearance if native binaries or deploy artifacts changed since the last Docker
runtime check.

## Verdict

The two concrete `code-review-6.md` defects are fixed and covered by tests.
Targeted backend/frontend/packaging tests pass.

The remaining important issue is architectural, not an immediate functional
test failure: Coeff Program still has active `scalar_exprs` / `expr_refs`
execution paths through `coeffEvalScalarExpr`. If "single typed VM only" is the
bar, this is incomplete. If the bar is "working current feature with bounded
compatibility debt," the code is in substantially better shape, with the UI
alias edge case as the main follow-up.

---

## Resolution — 2026-06-11

**[major] scalar-expression VM — ✗ premise corrected, ✅ documented.** No doc
in the tree states a "typed VM only / no secondary scalar VM" target (the only
"scalar VM" mention in review history is CR1's fold-parity bug note, fixed
then). The split is the design, not a migration in flight: the typed stack
evaluates stack *values*; scalar-expression bytecode evaluates per-row chip
*arguments* (range/linspace bounds, littlewood values, native-transform args,
andy). Decisively, `expr_refs` are part of the fingerprinted wire format that
saved programs persist — removing the layer is a fingerprint migration, not a
cleanup — and it is already bounded (64 distinct expressions × 32 tokens,
pinned by the drift tests). Per this review's own second branch, that is now
stated explicitly in three places: coeff-program.md (compile-model section),
the `_add_arg_expr` allocator in coeff_program_chain.py, and the C
"scalar expressions and arg plumbing" banner in sweep_cli.c.

**[concern] browser alias canonicalization — ✅ fixed.**
`_canonicalCoeffTransformName` now mirrors `LEGACY_NAME_ALIASES` exactly
(`exp_affine→exp`, `pow_affine→pow`, `power_series→power`,
`scale100→linear`), so imported legacy rows resolve the same `_ctCatalog`
metadata the backend compiles them against. The harness runtime test now
serializes `pow_affine` and `power_series` legacy rows end to end and asserts
the canonical output (including catalog-def resolution and andy trimming),
plus direct alias-mapping asserts. Flip-verified: removing the new mappings
fails the harness.

**[concern/policy] Param Program legacy bridge — ✅ documented as policy.**
Confirmed current policy, now stated in the `param_program_chain.py` module
docstring: legacy rows are the supported compatibility path for copied
Param-Chain stacks and old saved programs; retiring them requires a payload
migration, not a code deletion.

Because sweep_cli.c changed (banner comment only), the three artifacts built
from it were rebuilt before re-running gates: `lambda/sweep_test`,
`lambda/sweep` (ARM), and `lambda/sweep_coeffgen` (Docker + LAPACK layer),
followed by the Docker runtime regression.
