# CFPV Implementation Review

## Verdict

The work done is **not total bullshit**, but the status report materially overstates what was achieved.

What is actually implemented:

- the low-level CFPV ABI widening
- CFPV pass-through in coeffgen
- a hardcoded frontend special case for `creative9`
- one working example function (`creative9`)

What is **not** implemented:

- the shared catalog/spec design
- generic function-parameter UI
- runtime enforcement of constant coefficient length for fixed `(function, cfpv)`
- removal of old static-degree assumptions
- a test-backed general feature

So the right evaluation is:

- **ABI plumbing: mostly done**
- **one-off feature spike: done**
- **productized architecture described in the spec: not done**

The strongest problem is not cosmetic drift. It is that the core correctness invariant is still not enforced.

## What Was Claimed

The implementation was described as:

- CFPV ABI refactoring complete
- backend updated
- handler updated
- frontend updated
- verified on `creative9`
- remaining only:
  - catalog system
  - deploy

That summary is too generous.

It describes the mechanical signature refactor correctly, but it conflates:

- ABI completeness
with
- feature completeness

Those are not the same thing here.

## What Is Actually Done

### 1. ABI Widening Is Real

The coefficient function ABI has been widened from 7 args to 9 args:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3175)

The new signature is:

```c
typedef void (*CoeffFuncC)(double, double, double, double,
                           const double*, int,
                           double*, double*, int*);
```

This part is real and correctly broad.

### 2. Old Wrappers Were Updated

The `WRAP_OLD` macro now accepts and ignores CFPV:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3181)

This is fine.

### 3. Transpiled Functions Were Updated

The transpiler was changed to emit unused `(const double *cfpv, int n_cfpv)` arguments:

- [lambda/transpile_poly.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/transpile_poly.py#L1908)

and the generated files reflect that.

This part is also real.

### 4. Both Coeffgen Paths Parse and Pass CFPV

CFPV parsing exists in both C coeffgen modes:

- legacy coeffgen:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3999)
- chunked coeffgen:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4191)

And the handler threads it through in both backend modes:

- chunked:
  - [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py#L187)
- legacy:
  - [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py#L263)

This is correctly wired.

### 5. One Real Parametric Function Exists

`creative9` now uses `cfpv[0]` as `n`:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3462)

That function appears to work for:

- default `n = 71`
- overridden `n = 30`

So there is a working proof of concept.

### 6. Frontend Can Send a CFPV

The Compute tab now has:

- `_cfpv` state
- `parseCfpv()`
- `updateCfpvRow()`
- payload threading to coeffgen

References:

- UI row:
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1169)
- state + parser:
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1867)
- hires payload:
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3266)
- lores payload:
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3326)
- metadata save:
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3430)

This is a real, working narrow UI path.

## What Is Missing or Wrong

### 1. This Is Not the Catalog/Spec Implementation

The designed architecture called for:

- `coeff_func_catalog.json`
- `coeff_func_metrics.json`
- generated C metadata
- generated JS metadata
- generic function-param UI
- implementation kind / agreement metadata in the dropdown

None of that exists in the codebase.

There is:

- no `coeff_func_catalog.json`
- no `coeff_func_metrics.json`
- no generator script
- no generated JS catalog
- no `CoeffFuncSpec`
- no `lookupCoeffFuncSpec`

The backend still does raw function-pointer lookup:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3992)
- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4184)

The frontend still uses a hardcoded `<select>`:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L173)

So the catalog architecture was **not** implemented.

This is the clearest place where the “complete” framing is misleading.

### 2. The Correctness Invariant Is Still Not Enforced

This is the most important missing piece.

You explicitly defined the hard requirement as:

- for fixed `(function, cfpv)`, the coefficient function must return the same coefficient length for every `(t1, t2)` in the job

That is still **not enforced**.

Both coeffgen modes still do exactly one probe:

- legacy probe:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4005)
- chunked probe:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4206)

Then they assume the returned `nCoeffs` applies to all subsequent steps.

That means a function can still:

- return `n=30` at the probe point
- return `n=31` later in the same job

and the binary layout will be silently wrong.

This is not a small missing enhancement.
It is the main correctness requirement of the feature.

### 3. Static Degree Assumptions Are Still Present in the UI

The frontend still parses degree from dropdown text:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3160)

That is now conceptually wrong for CFPV-enabled functions, because:

- degree may vary by CFPV
- dropdown text is no longer authoritative

That means preflight size checks can be wrong, especially for `creative9`.

The correct source of truth is:

- runtime coeffgen probe result

not:

- hardcoded dropdown label text

### 4. The UI Is a Hardcoded Special Case

The UI support is not generic.

It is:

- one text input
- one hint map
- only shown for `creative9`

References:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1868)

This means:

- no parameter labels per function
- no defaults from metadata
- no generic render path
- no multi-function support

This is a spike, not the feature architecture.

### 5. CFPV Parsing Is Too Permissive

`parseCfpv()` currently does:

- split by comma
- `parseFloat`
- drop anything that becomes `NaN`

Reference:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1871)

So:

- `30,abc,40` becomes `[30, 40]`

with no error.

That is not safe for a parameter vector.

The parser should reject malformed input, not silently reinterpret it.

### 6. Results/Metadata UX Is Incomplete

The raw `cfpv` is saved:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3430)

But the Results panel does not display it:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1518)
- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1522)

and storage list does not surface it:

- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py#L91)

So jobs are not fully self-describing from the UI.

This weakens reproducibility.

### 7. Test Coverage Is Narrow

The only visible CFPV-specific test is:

- [tests/test_poly_creative9.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_poly_creative9.py)

That proves:

- one function works

It does **not** prove:

- generic CFPV parsing correctness
- malformed-input rejection
- old jobs with no `cfpv`
- multiple parametric functions
- invariance enforcement
- legacy coeffgen behavior under CFPV
- transpiled functions unaffected beyond “they still compile”

So the test story is not feature-complete.

## Evaluation by Area

### Backend ABI

Status:

- good

Assessment:

- mechanically sound
- broad enough
- likely the strongest part of the work

### Coeffgen Runtime Correctness

Status:

- incomplete

Assessment:

- single-probe assumption still unsafe
- hard requirement not enforced

### Frontend Integration

Status:

- weak

Assessment:

- functional for one hardcoded case
- not generic
- not based on metadata
- still tied to static dropdown degree parsing

### Architecture vs Spec

Status:

- mostly not implemented

Assessment:

- spec called for shared catalog + generated artifacts
- implementation delivered a narrow patch instead

### Overall Quality

Status:

- halfway done

Assessment:

- useful proof of concept
- not yet a robust feature

## Honest Status Summary

The honest summary should have been:

- CFPV ABI widening is complete
- coeffgen passes CFPV end-to-end
- `creative9` is converted into a working parametric coefficient function
- transpiled functions now accept and ignore CFPV
- frontend has a temporary hardcoded input path for `creative9`
- remaining work is substantial:
  - shared catalog/spec system
  - generic UI
  - runtime degree/invariance enforcement
  - removal of static-degree assumptions
  - broader tests

That would have been accurate.

What was actually said:

- made it sound like the only remainder was catalog/deploy

That is the overclaim.

## Recommended Fix Order

### 1. Enforce the Fixed-CFPV Length Invariant

Add multi-probe validation in coeffgen and coeffgen_chunked.

At minimum:

- probe a few representative transformed samples
- compare returned `nCoeffs`
- abort clearly if they differ

This is the highest-priority fix.

### 2. Remove Degree Parsing from Dropdown Text

Stop using:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3160)

for authoritative sizing logic.

Use runtime coeffgen probe output instead.

### 3. Replace the Hardcoded `creative9` UI with a Real Catalog

Implement:

- source catalog JSON
- metrics overlay JSON
- generator
- generated JS/C artifacts
- generic CFPV chip rendering

This is the architectural work that was skipped.

### 4. Tighten Frontend Parsing

Reject malformed CFPV input.

Do not silently drop bad tokens.

### 5. Surface CFPV in Results

Store and display:

- `pipeline.cfpv`
- `function_params_display`

so jobs are reproducible from the UI.

### 6. Add Real Feature Tests

Add tests for:

- default-backed old jobs
- malformed CFPV input
- invariant enforcement failure
- at least one second parametric hand-written function
- unchanged behavior for a representative transpiled function

## Final Evaluation

This work is:

- **not garbage**
- **not complete**
- **not faithful to the intended architecture**
- **good enough as a spike**
- **not good enough to call the feature done**

The strongest good part is the ABI refactor.
The strongest bad part is the missing invariance enforcement.

If you want one-line judgment:

> The plumbing is real, the demo works, but the implementation is still a hardcoded special-case spike with an unclosed correctness hole.
