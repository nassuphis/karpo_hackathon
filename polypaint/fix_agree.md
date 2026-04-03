# Fix Agree

## Goal

Reduce the misleading low-agreement coeff-function backlog by repeatedly applying the same loop:

1. pick a low-agreement transpiled function,
2. prove the mismatch with a parity test,
3. determine whether the bug is in the transpiler output or in the Python source,
4. fix the source if the reference is broken,
5. add a hand override in `lambda/poly_hand.h`,
6. route the catalog/runtime/UI to the hand override,
7. rerun parity and catalog consistency checks,
8. move to the next function.

This document describes that process and records the first completed batch.

## Current State

Completed in this pass:

- promoted these functions from `transpiled` to `hand`:
  - `poly_111`
  - `poly_112`
  - `poly_504`
  - `poly_741`
  - `poly_742`
  - `poly_760`
  - `poly_762`
  - `poly_765`
  - `poly_776`
  - `poly_780`
  - `poly_792`
  - `poly_799`
  - `poly_802`
  - `poly_812`
- fixed obvious Python-reference bugs before doing the hand ports:
  - [lambda/poly600.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly600.py)
  - [lambda/poly800.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly800.py)
  - [lambda/poly900.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly900.py)
- added batch parity coverage:
  - [tests/test_low_agreement_hand.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_low_agreement_hand.py)
- extended loose-end checks:
  - [tests/test_coeff_catalog_consistency.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_coeff_catalog_consistency.py)

After this pass there are still `83` remaining `transpiled` functions with `agreement_pct <= 30`.

## Files Involved

Source of truth and runtime wiring:

- [lambda/poly_hand.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly_hand.h)
- [lambda/coeff_func_catalog.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_catalog.json)
- [lambda/coeff_func_metrics.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_metrics.json)
- [lambda/gen_catalog.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/gen_catalog.py)
- [lambda/coeff_func_lookup.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_lookup.h)
- [coeff_func_catalog_js.js](/Users/nicknassuphis/karpo_hackathon/polypaint/coeff_func_catalog_js.js)
- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

Reference Python sources touched in this pass:

- [lambda/poly200.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly200.py)
- [lambda/poly600.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly600.py)
- [lambda/poly800.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly800.py)
- [lambda/poly900.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly900.py)

Test/process docs:

- [tests/test_low_agreement_hand.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_low_agreement_hand.py)
- [tests/test_coeff_catalog_consistency.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_coeff_catalog_consistency.py)
- [docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md)

## Repeatable Workflow

### 1. Prioritize the next batch

Do not sort purely by the badge.

Use this order:

1. `transpiled` functions with generated-C `WARNING: unhandled ...`
2. within that set, lowest agreement first
3. only after that, low-agreement functions without explicit transpiler warnings

Useful inventory command:

```bash
python3 - <<'PY'
import json, re
from pathlib import Path
metrics = json.loads(Path('lambda/coeff_func_metrics.json').read_text())
catalog = json.loads(Path('lambda/coeff_func_catalog.json').read_text())
by = {e['name']: e for e in catalog}
rows = []
for path in sorted(Path('lambda').glob('poly_generated_*.c')):
    text = path.read_text()
    for m in re.finditer(r'static void (poly_\d+)_c\b', text):
        name = m.group(1)
        start = m.start()
        end = text.find('static void ', start + 1)
        chunk = text[start:end if end != -1 else None]
        if 'WARNING: unhandled' in chunk and by.get(name, {}).get('kind') == 'transpiled':
            rows.append((metrics.get(name, {}).get('agreement_pct', -1), name, path.name))
rows.sort()
for row in rows:
    print(row)
PY
```

### 2. Verify the Python reference first

Some low-agreement functions are low because the Python source is itself broken.

In this pass the following were fixed before doing the hand C port:

- `poly_504`
- `poly_742`
- `poly_760`
- `poly_812`

Rule:

- if the Python source trivially exception-falls because of an obvious typo or API misuse, fix the Python reference first
- if the Python source is semantically unclear, stop and record it as blocked rather than inventing behavior

### 3. Add the hand override

Implement the fixed behavior in [lambda/poly_hand.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly_hand.h).

Requirements:

- exact coeff count must match the Python reference
- use existing complex helpers from `sweep_cli.c`
- zero out non-finite outputs at the end if the Python reference does the same by exception fallback or explicit guards

### 4. Route the runtime and UI

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

### 5. Rebuild the binary that actually uses the new lookup

```bash
cc -O3 -o lambda/sweep_test lambda/sweep_cli.c -lm
```

If this step is skipped, parity tests can accidentally run against stale routing.

### 6. Add parity coverage

Add the new function to [tests/test_low_agreement_hand.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_low_agreement_hand.py).

Current test design:

- uses `param_dump` to derive exact `t1`, `t2`
- uses `coeffgen` on `sweep_test`
- compares against the Python source function
- uses lightweight module stubs so poly source files can be imported without their full runtime dependencies
- uses manual local references only when importing the whole source file is too side-effect-heavy

### 7. Run the minimum green set

```bash
../.venv/bin/python -m pytest -q \
  tests/test_low_agreement_hand.py \
  tests/test_coeff_catalog_consistency.py
```

If the batch touches previously hand-routed functions too, also run:

```bash
../.venv/bin/python -m pytest -q \
  tests/test_poly645_hand.py \
  tests/test_poly795_hand.py
```

## Completed Batch Details

### Python reference fixes made first

- [lambda/poly600.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly600.py)
  - `poly_504`: fixed integer loop bound
- [lambda/poly800.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly800.py)
  - `poly_742`: `math.factorial(...)`
  - `poly_760`: `np.fft.ifft(...)`
- [lambda/poly900.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly900.py)
  - `poly_812`: corrected recurrence loop bound

### New hand overrides added

Added to [lambda/poly_hand.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly_hand.h):

- `poly_111_hand`
- `poly_112_hand`
- `poly_504_hand`
- `poly_741_hand`
- `poly_742_hand`
- `poly_760_hand`
- `poly_762_hand`
- `poly_765_hand`
- `poly_776_hand`
- `poly_780_hand`
- `poly_792_hand`
- `poly_799_hand`
- `poly_802_hand`
- `poly_812_hand`

## Remaining High-Priority Backlog

Still `transpiled` with `agreement_pct <= 30` and explicit generated-C warnings:

- `poly_746`
- `poly_759`
- `poly_766`
- `poly_769`
- `poly_773`
- `poly_785`
- `poly_788`
- `poly_810`
- `poly_667`
- `poly_794`

Recommended next pass:

1. `poly_746`
2. `poly_759`
3. `poly_766`
4. `poly_788`
5. `poly_667`
6. `poly_773`
7. `poly_785`
8. `poly_810`
9. `poly_769`
10. `poly_794`

## Non-goals

This pass does not claim that every low-agreement transpiled function is fixed.

It does establish the repair loop and completes the first high-confidence tranche so subsequent passes can continue without inventing new process each time.
