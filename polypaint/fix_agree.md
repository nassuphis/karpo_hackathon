# Fix Agree

## Goal

Clear the misleading low-agreement coeff-function backlog by repeating the same loop:

1. pick a low-agreement transpiled function
2. prove the mismatch with a parity test
3. determine whether the bug is in the generated C or in the Python reference
4. fix the Python reference first if it is obviously broken
5. add a hand override in `lambda/poly_hand.h`
6. route the catalog, runtime, and UI to the hand override
7. regenerate lookup/catalog artifacts
8. rerun parity and consistency tests

## Current State

The low-agreement `poly_*` backlog is now cleared:

- there are `0` `transpiled` `poly_*` functions with `agreement_pct <= 30` in [lambda/coeff_func_catalog.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_catalog.json)
- the repaired set is covered by [tests/test_low_agreement_hand.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_low_agreement_hand.py)
- source catalog, generated lookup, and generated browser catalog wiring are guarded by [tests/test_coeff_catalog_consistency.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_coeff_catalog_consistency.py)

Important parity caveats encoded in the suite:

- transformed `poly_26` is treated as a chaotic unit-circle recurrence
- `poly_39`, `poly_86`, `poly_102`, `poly_324`, and `poly_450` use a float32-overflow-tail contract
- transformed `poly_809`, `poly_811`, and `poly_818` compute the Python reference from exact transform doubles rather than `param_dump` float32 serialization
- `poly900.py` imports in the parity harness use lightweight `polylayout` stubs so the references can be imported without the full runtime environment

Generated C still contains `WARNING: unhandled ...` comments in places, but those warnings are no longer the same thing as the low-agreement backlog. The remaining warnings now belong to functions that are either already hand-routed or that currently pass parity as transpiled and therefore were not promoted blindly.

## Files Involved

Source of truth and routing:

- [lambda/poly_hand.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly_hand.h)
- [lambda/coeff_func_catalog.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_catalog.json)
- [lambda/coeff_func_metrics.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_metrics.json)
- [lambda/gen_catalog.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/gen_catalog.py)
- [lambda/coeff_func_lookup.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_lookup.h)
- [coeff_func_catalog_js.js](/Users/nicknassuphis/karpo_hackathon/polypaint/coeff_func_catalog_js.js)
- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

Reference Python files that were corrected during the repair pass:

- [lambda/poly100.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly100.py)
- [lambda/poly200.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly200.py)
- [lambda/poly400.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly400.py)
- [lambda/poly500.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly500.py)
- [lambda/poly600.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly600.py)
- [lambda/poly700.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly700.py)
- [lambda/poly800.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly800.py)
- [lambda/poly900.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly900.py)

Tests and process docs:

- [tests/test_low_agreement_hand.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_low_agreement_hand.py)
- [tests/test_coeff_catalog_consistency.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_coeff_catalog_consistency.py)
- [tests/test_poly645_hand.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_poly645_hand.py)
- [tests/test_poly795_hand.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_poly795_hand.py)
- [docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md)

## Repeatable Workflow

### 1. Prioritize carefully

Do not sort only by badge value.

Use this order:

1. transpiled functions whose generated C contains explicit `WARNING: unhandled ...`
2. within that set, the lowest agreement first
3. only then the unwarned low-agreement functions

### 2. Verify the Python reference first

Some low-agreement functions are low because the Python source is broken.

Rule:

- if the Python source trivially exception-falls because of an obvious typo, off-by-one, or invalid NumPy call, fix the Python reference first
- if the Python source semantics are unclear, stop and record it as blocked instead of inventing behavior

### 3. Add the hand override

Implement the intended behavior in [lambda/poly_hand.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly_hand.h).

Requirements:

- coeff count must match the Python reference
- use the shared complex helpers already present in the hand file
- match Python fallback behavior where the source intentionally collapses invalid output to zeros or prior values

### 4. Route runtime and UI together

Update [lambda/coeff_func_catalog.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_catalog.json):

- `c_symbol` -> `<name>_hand`
- `kind` -> `"hand"`
- `source` -> `"poly_hand.h"`

Update [lambda/coeff_func_metrics.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_metrics.json):

- set `agreement_pct` to `100`

Then regenerate:

```bash
cd lambda
python3 gen_catalog.py
```

This rewrites:

- [lambda/coeff_func_lookup.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_lookup.h)
- [coeff_func_catalog_js.js](/Users/nicknassuphis/karpo_hackathon/polypaint/coeff_func_catalog_js.js)

### 5. Rebuild the binary that uses the lookup

```bash
cc -O3 -o lambda/sweep_test lambda/sweep_cli.c -lm
```

If this step is skipped, parity tests can silently run against stale routing.

### 6. Add parity coverage

Add the repaired function to [tests/test_low_agreement_hand.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_low_agreement_hand.py).

Current harness behavior:

- uses `param_dump` and `coeffgen` from the rebuilt `sweep_test`
- imports Python references with lightweight module stubs
- supports manual local references for especially side-effect-heavy source files
- supports exact-transform reference cases for highly sensitive formulas
- supports float32-overflow-tail assertions for numerically explosive formulas
- supports specialized contracts for chaotic normalized recurrences

### 7. Run the close-out suite

Minimum repair suite:

```bash
../.venv/bin/python -m pytest -q \
  tests/test_low_agreement_hand.py \
  tests/test_coeff_catalog_consistency.py
```

If previously hand-routed functions or consistency guards were touched, also run:

```bash
../.venv/bin/python -m pytest -q \
  tests/test_poly645_hand.py \
  tests/test_poly795_hand.py
```

## Final Verification

The final green suite for this repair pass is:

```bash
cc -O3 -o lambda/sweep_test lambda/sweep_cli.c -lm
../.venv/bin/python -m pytest -q \
  tests/test_poly645_hand.py \
  tests/test_poly795_hand.py \
  tests/test_low_agreement_hand.py \
  tests/test_coeff_catalog_consistency.py
```

Expected result: all tests green, with no remaining `transpiled` `poly_*` functions at `agreement_pct <= 30`.
