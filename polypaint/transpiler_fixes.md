# Transpiler Follow-Up Fixes

This is a follow-up to the `g1`-`g100` transpiler pass after the latest fixes landed.

## Fixed Since The Last Review

These previously reported structural bugs now look fixed in generated output.

### 1. `nCoeffs` extraction

Concrete generated results:

- [g_generated.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L649) / [g_generated.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L651) now emit `g9_c` with `*nCoeffs = 20`
- [g_generated.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L700) / [g_generated.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L702) now emit `g10_c` with `*nCoeffs = 120`

Relevant transpiler entry point:

- [extract_ncoeffs()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L1602)

### 2. Whole-array RHS lowering

The transpiler now has dedicated vector-assignment lowering for `cf = <array expr>`.

Relevant helpers:

- [_lower_vector_assign()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L669)
- [_expr_to_c_vector()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L679)

Effect:

- `g9_c` now writes coefficient values, instead of only creating temporaries and leaving `cRe` / `cIm` at zero

### 3. Named fancy-index lowering

Constant index arrays are now carried structurally through the transpiler instead of being re-scraped from emitted C.

Relevant pieces:

- [const_arrays init](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L46)
- [named-array capture](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L1087)
- [fancy-index lookup in assignment lowering](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L812)

Concrete generated result:

- [g43_c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L3893) now emits explicit stores at indices `0`, `4`, `14`, and `29` instead of the old broken cast of the array symbol

### 4. Complex local preservation

These still look correctly lowered:

- [g63_c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L5033)
- [g72_c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L5862)
- [g76_c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L6636)

### 5. Scalar-from-first-input plus complex exponent lowering

`g89` now appears semantically correct in the two places that were previously wrong:

- scalar extraction from `z[0]` is preserved in [g89_c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L7386)
- the non-real exponent path now goes through `c_powc(...)` in [g_generated.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/g_generated.c#L7448)

## Current State

The earlier “remaining broken items” list is mostly resolved.

The meaningful remaining work is no longer:

- `g9` vector lowering
- `g10` `nCoeffs`
- `g43` fancy indexing
- `g89` complex exponent

Those now look fixed in generated C.

At this point the main open areas are:

- the still-failing non-stubbed functions
- the auto-stubbed functions that could be redeemed by adding a few more lowering patterns
- the intentionally hard stubbed functions that should probably stay stubbed unless you want a much broader transpiler

Recovered from the previous auto-stub set:

- `g19`
- `g20`
- `g21`
- `g40`
- `g41`
- `g42`
- `g46`
- `g57`
- `g59`
- `g69`

Current remaining auto-stubbed `g1`-`g100` functions:

- `g27`
- `g32`
- `g47`
- `g52`
- `g58`
- `g62`
- `g67`
- `g81`
- `g83`
- `g88`
- `g91`

## Which Stubbed Functions Look Redeemable?

Yes. Some of the current auto-stubbed functions are realistic next targets.

### Best Next Redemption Targets

These look like the best payoff for modest transpiler broadening.

#### `g91`

Source:

- [g91](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L2222)

Why it is promising:

- scalar-real-from-`z[0]` is already fixed
- `.conjugate()` on arbitrary expressions is already implemented in [call_to_c()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L277)
- the remaining unsupported shape is narrow and local

What is probably missing now:

- slice self-read in expressions like:
  - [ops_poly.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L2230) `cf[2:70] = cf[2:70] + np.log(np.abs(t2-t1)+1)`

Expected effort:

- low to medium

Expected payoff:

- likely 1 recovered function

Concrete fixes to implement:

- Extend [expr_to_c_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L966) so reads from `cf[a:b]` on the RHS participate in the same per-element lowering path as named local arrays.
- The key technique is:
  - when lowering inside the generated slice loop,
  - treat `cf[lo:hi]` as “current element at `_si_idx`”, not as a first-class vector value

Suggested shape:

```python
if isinstance(node, ast.Subscript) and isinstance(node.value, ast.Name) and node.value.id == "cf":
    sl = node.slice
    if isinstance(sl, ast.Slice):
        tmp = CVar.fresh("cfelem")
        self.declare(tmp)
        self.emit(f"{tmp.r} = cRe[_si_idx]; {tmp.i} = cIm[_si_idx];")
        return tmp
```

- That should only apply inside [expr_to_c_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L966), not in the general [expr_to_c()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L67) path.
- Keep `.conjugate()` as-is.
  That part no longer looks like the blocker.

Success condition:

- `g91` no longer auto-stubs
- generated C contains a normal loop for the `cf[2:70]` update

#### `g62`

Sources:

- [g62](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L1179)

Why they are promising:

- no `np.roots`
- no `np.roots`
- no nested helper defs
- mostly slice and small vector-expression work

What is probably missing:

- list-comprehension slice assignment is present, but likely not recognized under all wrappers
- per-element lowering for:
  - `np.abs(cf[5:10])`
  - `np.angle(cf[0:5])`
- slice-to-slice elementwise math on already-written `cf` segments

Expected effort:

- medium

Expected payoff:

- likely 1 recovered function and useful slice/vector machinery

Concrete fixes to implement:

- Extend [expr_to_c_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L966) so it understands unary/vector calls over slice elements:
  - `np.abs(cf[5:10])`
  - `np.angle(cf[0:5])`
  - power by real constant on a slice element
- The right technique is not “support slice arrays as first-class values”.
  The right technique is:
  - when evaluating a slice assignment loop,
  - lower each supported slice operand to its per-element value at `loop_var`

Suggested pattern:

- if `expr_to_c_slice()` sees:
  - `np.abs(subscript-with-slice)`
  - compute the per-element subscript first, then apply scalar `c_abs`
- if it sees:
  - `np.angle(subscript-with-slice)`
  - compute the per-element subscript first, then apply scalar `c_arg`

Concretely, add a branch before the normal fallback in [expr_to_c_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L966):

```python
if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
    if isinstance(node.func.value, ast.Name) and node.func.value.id == "np":
        attr = node.func.attr
        if attr in ("abs", "angle") and node.args:
            arg = self.expr_to_c_slice(node.args[0], loop_var, lo, count)
            ...
```

- Also extend [assign_cf_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L847) to treat a list comprehension wrapped in `np.array(...)` the same way it already treats bare `ast.ListComp`.
- Reuse [transpile_listcomp_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L931) instead of inventing a second comprehension path.
- For `cf[5:10] = scalar_expr`, keep the current broadcast behavior.
  The new work is only for:
  - list-comp to slice
  - slice-to-slice elementwise expressions

Success condition:

- `g62` no longer auto-stubs
- both list-comprehension slice fills and `np.abs(...)` / `np.angle(...)` slice expressions emit normal loops

### Redeemable, But Only If You Want More Language Coverage

These are probably recoverable, but the required support is broader.

#### `g67`, `g88`

Sources:

- [g67](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L1280)
- [g88](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L2180)

Why they are harder:

- `g67` is loop-heavy and guard-heavy
- `g88` needs vectorized exponentiation against a constant exponent array plus scalar-real handling

What would help:

- for `g67`:
  - broader support for rich guarded scalar numerics in loops
- for `g88`:
  - elementwise exponentiation where the exponent source is a constant array slice like `primes[0:20]`

Verdict:

- probably redeemable, but not the highest-leverage next step

### Poor Redemption Targets For The Current Transpiler

These are the ones I would not chase next unless you are explicitly broadening the transpiler scope.

#### `g27`, `g32`, `g47`, `g52`

Sources:

- [g27](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L455)
- [g32](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L621)
- [g47](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L841)
- [g52](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L933)

Why not:

- they depend on `np.roots(...)`
- that is a fundamentally different lowering problem from the rest of the transpiler
- if you do this, you are effectively adding a small polynomial root solver backend or special-casing very specific low-degree forms

Verdict:

- technically redeemable
- not worth it as a general transpiler feature right now

#### `g27`, `g32`, `g47`, `g52`

Sources:

- [g27](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L455)
- [g32](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L621)
- [g47](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L841)
- [g52](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L933)

Why not:

- they depend on `np.roots(...)`
- that is still a fundamentally different lowering problem from the rest of the transpiler

Verdict:

- still not worth chasing as a general transpiler feature

#### `g58`, `g81`, `g83`

Sources:

- [g58](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L1034)
- [g81](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L1722)
- [g83](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/ops_poly.py#L1955)

Why not:

- they contain nested helper functions and “mini-library” local abstractions
- redeeming them cleanly means either:
  - local function inlining / specialization, or
  - a much broader subset of Python function semantics

Verdict:

- possible
- but a bad next target if the goal is fast improvement in `g1`-`g100` coverage

## Recommended Next Fix Order

If the goal is to maximize recovered auto-stubbed functions with controlled complexity:

1. Add missing scalar transcendental cases in [numpy_call()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L356)
   Targets:
   - `g57`

2. Add small slice/vector-assignment support
   Targets:
   - `g62`

3. Add narrow method-call lowering for `.conjugate()`
   Targets:
   - `g91`

4. Decide whether to broaden slice/vector exponent support
   Targets:
   - `g88`

5. Stop there unless you want a broader transpiler
   Reason:
   - after that point, the remaining auto-stubs start looking like either:
      - `np.roots` lowering
      - nested helper function lowering
      - much broader Python support

Implementation order inside the codebase:

1. Patch [expr_to_c_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L966) for `cf` slice self-reads
   Why first:
   - smallest remaining high-value surface area
   - should unlock `g91`

2. Patch [expr_to_c_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L966) and [assign_cf_slice()](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L847)
   Why second:
   - needed for `g62`

3. Decide whether to broaden constant-array exponent vector support
   Why third:
   - would be needed for `g88`

4. Only after each step, re-run the targeted subset before the full visual sweep
   Suggested subset progression:
   - `g91`
   - then `g62`
   - then `g88`

## Test Harness Note

The visual harness needs to stay in sync with the real stub list.

- [tests/test_visual_g1_g99.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_visual_g1_g99.py#L104) still lists `g57` as stubbed
- `/tmp/g_transpile.log` now shows 11 stubs and `g57` is no longer one of them

So the next visual sweep should first remove `57` from `STUBBED`, otherwise the test underreports recovered coverage.

## Concrete Regression Cases To Add

If you redeem the next bucket, add targeted regression tests for:

- inline fancy index with literal index array:
  - `cf[np.array([0, 6, 14])] = [1, -2, 3]`
- loop-carried guarded assignment:
  - `cf[k] = expr1 if np.isfinite(v) and v > eps else expr2`
- modulo inside loop expressions:
  - `((k+3) % 10)`
- slice vector assignment from list comprehension:
  - `cf[0:5] = np.array([abs(t1+t2)**(i+1) for i in range(5)])`
- slice-to-slice elementwise vector math:
  - `cf[15:20] = np.abs(cf[5:10])**0.5 + np.angle(cf[0:5])`

## Bottom Line

Yes, some of the current auto-stubbed functions can be redeemed.

The best next targets are:

- `g40`, `g41`, `g42`
- `g19`, `g20`, `g21`
- then maybe `g62`

I would not chase the `np.roots` group or the nested-helper group next unless you want to broaden the transpiler significantly.
