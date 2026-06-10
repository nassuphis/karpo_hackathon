# Code Review 2 — Coeff Program follow-up

**Date:** 2026-06-10
**Scope:** Review of the current Coeff Program stack after the fixes documented in
`code-review-1.md`.

## Summary

Most findings from `code-review-1.md` are genuinely addressed in the current
source. I verified the important old repros against the actual parser/compiler
and native paths: macro expansion is budgeted, scalar-expression count is capped,
range length bounds are enforced, diagnostics now report statement locations,
static/dynamic `tos` reads no longer read stale vector buffers, `blend` checks
vector slot types, `round` source fallback works, and signed-zero token args are
canonicalized.

One real user-visible issue remains around the legacy coefficient transform
`power`, plus one source-language footgun around `linear(poly, 5)`.

## Findings

### [major] Legacy coefficient transform `power` is still broken in Program chip/UI paths

Files:

- `index.html`
- `lambda/coeff_program_chain.py`
- `lambda/coeff_program_source.py`

The source layer has aliases for shadowed native-transform names:

```js
const _coeffProgramSourceAliasNames = { exp: 'exp_affine', pow: 'pow_affine', power: 'power_series' };
```

But the chip/load/copy paths only special-case `exp` in some places. The legacy
coefficient transform `power` is still shadowed by the vector-binary `power`
chip.

Concrete behavior:

```python
compile_coeff_program_chain([["power", "poly", "poly", "8"]])
```

fails with:

```text
power src2 selector is invalid: '8'
```

And source text:

```text
poly = power(poly, 8)
```

means elementwise vector exponentiation, not the legacy power-series transform.

Expected behavior:

- Legacy transform `power` should normalize to `power_series` when moving into
  Coeff Program source/chip UI.
- Old compatibility form should either remain as `legacy(power, ...)` or be
  mapped to a non-shadowed direct Program chip name.

Recommended fix:

- Extend the same aliasing used for `exp -> exp_affine` to `pow -> pow_affine`
  and `power -> power_series` in `_normalizeCoeffProgramChain`,
  `_copyCoeffTransformsIntoCoeffProgram`, and source synthesis paths.
- Add tests asserting old `["legacy", "power", ...]` and copied Chain `power`
  rows synthesize to `power_series(...)`, not `power(...)`.

### [concern] `linear(poly, 5)` is accepted but means the wrong thing

File:

- `lambda/coeff_program_source.py`

Current source lowering treats two-argument `linear(a, b)` as:

```text
linear(multiplier, offset)
```

with implicit source `pop`. Therefore:

```text
linear(poly, 5)
```

does not mean:

```text
linear(source=poly, multiplier=5, offset=0)
```

It means:

```text
pop * poly + 5
```

If the stack is empty, validation catches it. If the stack is non-empty, it
executes as valid but surprising math.

Recommended fix:

- If `linear` has two args and the first arg is a vector source (`cf`, `poly`,
  `pop`, `peek`, `tos`), reject it with a clear message:

```text
linear(source, multiplier) is ambiguous; use linear(source, multiplier, offset)
```

This preserves the useful `linear(multiplier, offset)` shorthand while blocking
the confusing source-like two-arg form.

### [nit] Alias documentation/comment drift

Files:

- `lambda/coeff_program_source.py`
- `lambda/coeff_program_chain.py`

`coeff_program_source.py` says the source aliases are mirrored by
`_LEGACY_NAME_ALIASES`, but only `exp_affine` is mirrored there. `pow_affine`
and `power_series` are source-only aliases.

Recommended fix:

- Either update the comment to say only `exp_affine` is accepted as a direct
  chain alias, or intentionally add `pow_affine` / `power_series` to
  `_LEGACY_NAME_ALIASES` too.

## Verification

Commands run:

```bash
uv run python -m pytest tests/test_coeff_program_chain.py tests/test_coeff_program_native.py tests/test_coeff_program_drift.py -q
bash tests/test_frontend_js.sh
```

Results:

```text
100 passed
Frontend fused render source checks: OK
Frontend fused render runtime checks: OK
```

Manual repro snippets executed against current source:

```python
compile_coeff_program_chain([["power", "poly", "poly", "8"]])
```

still fails as described above.

```python
parse_coeff_program_source("poly = power(poly, 8)")
```

lowers to typed vector `power`, not legacy/native `power`.

```python
compile_coeff_program_source("cf\npoly = linear(poly,5)")
```

compiles and lowers as `pop * poly + 5`, confirming the `linear(poly, 5)`
ambiguity remains.

