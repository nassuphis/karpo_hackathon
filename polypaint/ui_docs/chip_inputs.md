# UI Chip Input Plan

## Goal

Allow parameter-transform chips such as `crd` and `hrt` to accept a readable target selector in the UI:

- `t1`
- `t2`
- `t1,t2`

while keeping the backend wire format simple:

- `0` => apply to `t1`
- `1` => apply to `t2`
- `2` => apply to both

## Desired UX

For transforms whose first argument is currently `n(0|1)`, the chip should accept symbolic input instead of forcing numeric input.

Examples:

- `crd(t1, 1)`
- `crd(t2, 1)`
- `crd(t1,t2, 1)`
- `hrt(t1, 1, 0)`
- `hrt(t1,t2, 0.8, 0.25)`

The chip display should stay symbolic for readability.

## Proposed Semantics

For single-target curve transforms:

- `0` means transform only `t1`
- `1` means transform only `t2`
- `2` means transform both independently

For `both`, each parameter is transformed from its own current value.

Example for `crd(2, size)`:

- `t1' = cardioid(Re(t1), size)`
- `t2' = cardioid(Re(t2), size)`

This avoids needing two chips for the common "apply same transform to both parameters" case.

## UI Changes

File: [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Update chip metadata for transforms that currently use `n(0|1)`:

- `crd`
- `hrt`
- any other single-index transforms you want to extend at the same time

Recommended placeholder text:

- `target`

Recommended help text or accepted forms:

- `t1`
- `t2`
- `t1,t2`

Normalize user input before sending:

- `t1` -> `"0"`
- `t2` -> `"1"`
- `t1,t2` -> `"2"`

Accepted aliases can be a bit forgiving:

- `0`, `1`, `2`
- `t1`, `t2`
- `both`, `t1,t2`, `t2,t1`

Invalid target input should fail early in the UI with a clear message instead of silently falling back.

## Backend Changes

File: [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

### 1. Keep the transport numeric

The simplest path is:

- UI sends normalized numeric strings
- backend continues parsing numeric args as it already does

That means backend parser changes are optional if the UI always converts symbolic input to `0/1/2`.

### 2. Extend transform helpers to support `2`

Update helpers such as:

- `pt_crd(...)`
- `pt_hrt(...)`

Current behavior:

- `0` => one parameter
- `1` => the other parameter
- anything else => no-op

New behavior:

- `0` => `t1`
- `1` => `t2`
- `2` => both

Implementation approach:

- keep existing single-target logic
- add a `both` branch that applies the same formula once to `t1` and once to `t2`
- preserve existing semantics for `0` and `1`

### 3. Dispatch path

In `dispatchPt(...)`, keep:

- `int n = e->nArgs > 0 ? (int)e->args[0] : 0;`

No structural change is needed if the helper functions understand `2`.

## Parser Boundary

The native `parsePtChain(...)` JSON-chain parser has been retired. Chain-mode
rows are translated to Param Program payloads at the Lambda boundary, so
symbolic strings such as `t1` / `t2` must be handled by the Param Program
compiler and profile symbol table, not by native transform-chain parsing.

So backend symbolic parsing is currently unsafe.

Optional improvement:

- explicitly map `"t1"` => `0`
- `"t2"` => `1`
- `"t1,t2"` / `"both"` => `2`

But this is not required if UI normalization is enforced.

## Validation Rules

UI should validate target selectors for affected transforms:

- allow only `t1`, `t2`, `t1,t2` and numeric `0`, `1`, `2`
- reject anything else

Backend should remain safe if bad input arrives:

- unknown selector values should be a no-op or explicit error
- do not let malformed strings silently act like `t1`

## Tests

### UI-level

Add tests or manual checks for normalization:

- `t1` -> `0`
- `t2` -> `1`
- `t1,t2` -> `2`
- `both` -> `2` if alias supported
- invalid input rejected

### Backend-level

Add focused tests for transforms that gain `both` support:

- `crd(0, s)` changes only `t1`
- `crd(1, s)` changes only `t2`
- `crd(2, s)` changes both
- same for `hrt`

Important regression check:

- existing saved numeric transform specs still behave exactly the same

## Rollout Order

1. Add UI normalization for target argument.
2. Extend `pt_crd` and `pt_hrt` to support `2`.
3. Verify old numeric chips still work.
4. Add tests for `0/1/2` behavior.
5. Optionally add backend symbolic parsing later if desired.

## Recommendation

Implement symbolic input only at the UI layer first, and keep the persisted/wire representation numeric.

That gives:

- readable chips for users
- no ambiguity in the solver
- minimal parser risk
- backward compatibility with existing saved transform arrays
