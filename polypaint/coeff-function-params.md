# Coefficient Function Parameter Vector (CFPV) Spec

## Status Update

This document is still broadly correct about CFPV, but the compute pipeline has
since grown a second parameterized layer:

- coefficient transforms now support a mixed wire format:
  - plain names for no-arg transforms, e.g. `"rev"`
  - array form for parametric transforms, e.g. `["roots", "6", "hi"]`
- the Compute tab now renders coefficient-transform chips with inline argument
  inputs, the same way parameter/root transforms do
- currently shipped parametric coefficient transforms include:
  - `roots(k, hi|lo)`
  - `power(k)` shown in the UI as `p(k)`
  - `invpower(k)` shown in the UI as `invp(k)`
- currently shipped native coefficient transforms also include:
  - `roots_cm(hi|lo)`

So the effective pipeline is now:

```text
[parameter transforms] -> coefficient function(t1, t2, cfpv) -> [coefficient transforms, some with params]
```

## Goal

Add **optional constant parameters** to coefficient functions.

These parameters:

- are constant for the entire compute job
- do **not** vary across `(t1, t2)` samples
- apply only to coefficient functions
- are expected to be used primarily by **hand-written** coefficient functions
- remain structurally available to all coefficient functions through the same catalog/spec system

The feature should integrate cleanly with the existing pipeline:

```text
[parameter transforms] -> coefficient function(t1, t2, cfpv) -> [coefficient transforms]
```

This is worth doing because some hand-written coefficient families need a few runtime constants, while most functions need none.

## Non-goals

- No support for arbitrary named/dict parameters in transpiled Python functions.
- No changes to `param_gen`.
- No changes to solve, raster, finalize, or render formats.
- No runtime “query Lambda for metadata” dependency on page load.

## Design Summary

Replace the current “lookup returns only a function pointer” model with a **coefficient function spec** model:

- a compile-time master catalog describes each coefficient function
- the catalog includes:
  - function name
  - C function pointer
  - parameter count
  - default parameter values
  - UI labels/placeholders for those parameters
- the frontend uses that catalog to render coefficient-function parameter chips
- on compute, the frontend serializes a **CFPV** array
- coeffgen parses that CFPV once per invocation
- each coefficient function receives:
  - `t1`
  - `t2`
  - `cfpv`
  - `n_cfpv`

Generated/transpiled functions simply receive `cfpv == NULL` and `n_cfpv == 0` or ignore the arguments entirely.

The same catalog should also carry classification metadata:

- `kind`
  - `hand`
  - `transpiled`
  - `wrapped`
- optional agreement percentage from test reconciliation
- optional stubbed status from transpiler/test results

Important correction:

- the catalog must **not** be treated as the authoritative source of degree
- degree may depend on CFPV for some functions
- therefore degree must be derived at runtime for each compute job

## Current State

The current architecture is hard-wired to:

```c
f(t1, t2) -> coeffs
```

Key places:

- function pointer type:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3162)
- function lookup:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3472)
- coeffgen probe call:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3958)
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3971)
- coeffgen main call:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4008)
- chunked coeffgen call:
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4144)
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4173)
  - [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4198)
- compute payload assembly:
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3235)
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3297)
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3397)

So this change is real ABI surgery, but contained.

## Core Terminology

- `CFPV`: Coefficient Function Parameter Vector
- `n_cfpv`: number of entries in the vector
- “catalog”: the compile-time metadata table for coefficient functions
- “defaults”: the per-function default CFPV values used for UI initialization and as runtime fallback

## Recommended Architecture

### 1. Master Catalog

Introduce one shared master definition file that acts as the source of truth for both backend and UI.

Recommended source files:

- [`lambda/coeff_func_catalog.json`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_catalog.json)
- [`lambda/coeff_func_metrics.json`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_metrics.json)

Recommended generated outputs:

- [`lambda/coeff_func_lookup.h`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_lookup.h)
- [`coeff_func_catalog_js.js`](/Users/nicknassuphis/karpo_hackathon/polypaint/coeff_func_catalog_js.js)

This avoids drift.

The split should be:

- `coeff_func_catalog.json`
  - intrinsic/source-level metadata
- `coeff_func_metrics.json`
  - test-derived metadata such as agreement percentages

Do **not** make the browser query a Lambda for this metadata at runtime. That is unnecessary and fragile.

Use a build/generation step instead.

### 2. Intrinsic Metadata vs Test-Derived Metadata

The catalog needs two classes of data:

- intrinsic metadata:
  - function name
  - implementation kind
  - C symbol
  - parameter labels/defaults
- test-derived metadata:
  - agreement percentage
  - stubbed status
  - optional test timestamp

These should not be stored the same way.

Recommended rule:

- intrinsic metadata lives in `coeff_func_catalog.json`
- test-derived metadata lives in `coeff_func_metrics.json`
- the generator merges them

This matters because:

- `kind` is a source-level fact
- `agreement_pct` changes when tests change and should not be hand-maintained
- `degree` may depend on CFPV and is therefore a per-job runtime property, not stable catalog metadata
- `stubbed` is test-derived metadata and should not be treated as intrinsic catalog truth

### 2a. Lessons From Implementation

- `kind` is a better field name than `impl`
  - the real taxonomy is not only “how was this implemented”
  - `wrapped` is a meaningful category for legacy functions adapted into the new ABI
- backend and frontend do not need the same metadata richness
  - backend needs function pointer, param count, and default vector
  - frontend needs labels, bounds, agreement, probe status, and kind
- UI defaults are not enough by themselves
  - the backend must normalize CFPV from the spec too
  - old jobs, manual calls, and malformed payloads must still resolve correctly
- degree hints in the dropdown are useful, but are not authoritative
  - they are probed metadata
  - per-job coeffgen probe remains the source of truth
- if degree hints are generated from a host probe binary, deploy must validate them against the deploy binary
  - Docker/runtime validation is the right place for that check

### 3. New C ABI

Change the coefficient function signature from:

```c
typedef void (*CoeffFuncC)(
    double x1r, double x1i, double x2r, double x2i,
    double *cRe, double *cIm, int *nCoeffs
);
```

to:

```c
typedef void (*CoeffFuncC)(
    double x1r, double x1i, double x2r, double x2i,
    const double *cfpv, int n_cfpv,
    double *cRe, double *cIm, int *nCoeffs
);
```

This should be applied uniformly:

- hand-written functions
- wrappers for old real-only functions
- transpiled/generated functions

Even if a function never uses parameters, the signature should still match.

That keeps the call path simple and avoids special cases.

### 4. Lookup Returns a Spec, Not Just a Function

Introduce:

```c
typedef struct {
    const char *name;
    CoeffFuncC fn;
    int n_params;
    const double *defaults;
} CoeffFuncSpec;
```

Then replace:

```c
static CoeffFuncC lookupCoeffFuncC(const char *name)
```

with:

```c
static const CoeffFuncSpec *lookupCoeffFuncSpec(const char *name)
```

The frontend needs richer parameter metadata, but that can stay in generated JS.
The backend only needs the function pointer, parameter count, and default values.
The spec structure is still the right unit of lookup.

## Catalog Format

### Source Catalog

Recommended JSON source format:

```json
[
  {
    "name": "creative9",
    "c_symbol": "creative9_c",
    "kind": "hand",
    "params": []
  },
  {
    "name": "poly_1",
    "c_symbol": "poly_1_c",
    "kind": "transpiled",
    "params": []
  },
  {
    "name": "my_hand_func",
    "c_symbol": "my_hand_func_c",
    "kind": "hand",
    "params": [
      { "name": "alpha", "label": "Alpha", "default": 1.0 },
      { "name": "sigma", "label": "Sigma", "default": 0.75 },
      { "name": "turns", "label": "Turns", "default": 0.0 }
    ]
  },
  {
    "name": "old_giga_1",
    "c_symbol": "giga_1_c",
    "kind": "wrapped",
    "params": []
  },
  {
    "name": "creative8",
    "c_symbol": "creative8_c",
    "kind": "hand",
    "source": "sweep_cli.c",
    "params": [
      { "name": "n", "label": "Degree+1", "default": 71, "min": 2, "max": 256 }
    ]
  },
  {
    "name": "creative9",
    "c_symbol": "creative9_c",
    "kind": "hand",
    "source": "sweep_cli.c",
    "params": [
      { "name": "n", "label": "Degree+1", "default": 71, "min": 2, "max": 256 }
    ]
  }
]
```

Rules:

- all coefficient functions should appear in the catalog
- both hand-written and transpiled/generated functions live in the same file
- transpiled/generated functions usually get `"params": []`
- wrapped entries are legacy implementations adapted into the new CFPV ABI
- hand-written parametric functions define all parameter metadata here
- defaults are compile-time constants
- `kind` should be one of:
  - `"hand"`
  - `"transpiled"`
  - `"wrapped"`
- degree is intentionally omitted from the source catalog because it may vary with CFPV

### Metrics Overlay

Recommended generated metrics format:

```json
{
  "creative9": {
    "agreement_pct": 100,
    "stubbed": false
  },
  "poly_1": {
    "agreement_pct": 100,
    "stubbed": false
  },
  "poly_13": {
    "agreement_pct": 0,
    "stubbed": true
  }
}
```

Rules:

- this file is produced from tests or test post-processing, even if it is checked in
- it should not be hand-maintained as primary source data
- missing entries mean “unknown”, not zero

## Generation Step

Add a small generator script:

- [`lambda/gen_catalog.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/gen_catalog.py)

Inputs:

- `lambda/coeff_func_catalog.json`
- `lambda/coeff_func_metrics.json` (optional but recommended)

Outputs:

- `lambda/coeff_func_lookup.h`
- `coeff_func_catalog_js.js`

### Generated C Header Shape

Example generated header:

```c
typedef struct {
    const char *name;
    CoeffFuncC func;
    int n_params;
    const double *defaults;
} CoeffFuncSpec;

static const double creative9_defaults[] = {71.0};
static const double my_hand_func_defaults[] = {1.0, 0.75, 0.0};

static const CoeffFuncSpec g_coeff_func_specs[] = {
    {"creative9", creative9_c, 1, creative9_defaults},
    {"my_hand_func", my_hand_func_c, 3, my_hand_func_defaults},
    ...
};
```

### Generated JS Shape

Example generated JS:

```js
window._coeffFuncCatalog = [
  {
    name: 'creative9',
    kind: 'hand',
    degree: 70,
    agreement_pct: 100,
    params: [
      { name: 'n', label: 'Degree+1', default: 71, min: 2, max: 256 }
    ]
  },
  {
    name: 'poly_1',
    kind: 'transpiled',
    degree: 70,
    agreement_pct: 100,
    params: []
  }
];
```

The backend does not need `agreement_pct`, `stubbed`, labels, or bounds in its generated header.
That is frontend/test metadata.

### Degree Probe Generation

The JS generator may probe degree using a local binary, but that metadata is only a hint.

Two hard rules:

- per-job coeffgen probe is still authoritative
- deploy/runtime validation must compare generated degree hints against the deploy binary

This is especially important if JS generation probes via a host binary such as `sweep_test`.

## Backend Changes

### 1. Change Function Signature Everywhere

Update the type definition in:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3162)

to:

```c
typedef void (*CoeffFuncC)(
    double, double, double, double,
    const double *, int,
    double*, double*, int*
);
```

### 2. Update Old Wrappers

Current wrapper:

```c
#define WRAP_OLD(fname) \
    static void fname##_c(double x1r, double x1i, double x2r, double x2i, \
                          double *cRe, double *cIm, int *nCoeffs) { \
        (void)x1i; (void)x2i; \
        fname(x1r, x2r, cRe, cIm, nCoeffs); \
    }
```

New wrapper:

```c
#define WRAP_OLD(fname) \
    static void fname##_c(double x1r, double x1i, double x2r, double x2i, \
                          const double *cfpv, int n_cfpv, \
                          double *cRe, double *cIm, int *nCoeffs) { \
        (void)x1i; (void)x2i; (void)cfpv; (void)n_cfpv; \
        fname(x1r, x2r, cRe, cIm, nCoeffs); \
    }
```

### 3. Update Hand-Written Functions

Any hand-written coefficient function should adopt the new signature.

Example:

```c
static void my_hand_func_c(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    double alpha = (n_cfpv > 0) ? cfpv[0] : 1.0;
    double sigma = (n_cfpv > 1) ? cfpv[1] : 0.75;
    ...
}
```

### 4. Update Generated/Transpiled Functions

Even though they will never use parameters, they should still compile under the new ABI.

Their emitted signatures should become:

```c
static void poly_123_c(double x1r, double x1i, double x2r, double x2i,
                       const double *cfpv, int n_cfpv,
                       double *cRe, double *cIm, int *nCoeffs)
```

and the generator should emit:

```c
(void)cfpv;
(void)n_cfpv;
```

near the top when unused.

This is the cleanest way to keep a uniform dispatch ABI.

### 5. Add CFPV Parser

Add a generic numeric-array parser in `sweep_cli.c`.

Recommended helper:

```c
static int parseNumArray(const char *p, double *out, int maxCount) {
    p = skip(p);
    if (!p || *p != '[') return 0;
    p++;
    int count = 0;
    while (count < maxCount) {
        p = skip(p);
        if (*p == ']') break;
        if (*p == ',') { p++; p = skip(p); }
        if (*p == '-' || *p == '+' || (*p >= '0' && *p <= '9')) {
            out[count++] = atof(p);
            while (*p && *p != ',' && *p != ']') p++;
        } else {
            break;
        }
    }
    return count;
}
```

Recommended constants:

```c
#define MAX_CFPV 16
```

### 6. Parse CFPV Once Per Invocation

In both coeffgen modes:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L3946)
- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L4132)

parse:

```json
"cfpv": [ ... ]
```

Workflow:

1. lookup function spec
2. parse provided `cfpv`
3. if absent, use defaults from spec
4. if shorter than `n_params`, pad with defaults
5. if longer than `n_params`, error
6. if `n_params == 0`, force `n_cfpv = 0`

Recommended runtime normalization:

```c
double cfpv[MAX_CFPV];
int n_cfpv = 0;

const char *cp = findKey(buf, "cfpv");
if (cp) n_cfpv = parseNumArray(cp, cfpv, MAX_CFPV);

if (spec->n_params == 0) {
    n_cfpv = 0;
} else {
    if (n_cfpv > spec->n_params) {
        fprintf(stderr, "Too many coefficient-function params for %s: got %d, expected %d\n",
                spec->name, n_cfpv, spec->n_params);
        return 2;
    }
    for (int i = n_cfpv; i < spec->n_params; i++) {
        cfpv[i] = spec->defaults[i];
    }
    n_cfpv = spec->n_params;
}
```

Server-side normalization is mandatory even if the UI pre-fills defaults.
That preserves compatibility for old jobs and non-UI callers.

### 6a. Degree Must Be Probed at Runtime

With CFPV-enabled functions, the only authoritative degree is the one returned by the coefficient function for the current job's CFPV.

Required invariant:

- for fixed `(function, cfpv)`, the coefficient function must return the same `nCoeffs` for every `(t1, t2)` in that compute job

This must be treated as a hard requirement.

Recommended implementation:

1. Probe the coefficient function once with the actual CFPV.
2. Use that `nCoeffs` as the job's degree.
3. Optionally validate with a few extra representative sample points.
4. Abort if length differs across probes.

Recommended strict check:

```c
int probe_lengths[3];
double probes[3][4] = {
    {0.0, 0.0, 0.0, 0.0},
    {0.5, 0.0, 0.5, 0.0},
    {0.25, 0.0, 0.75, 0.0},
};

for (int p = 0; p < 3; p++) {
    double z1r = probes[p][0], z1i = probes[p][1];
    double z2r = probes[p][2], z2i = probes[p][3];
    int nProbe = 0;
    spec->fn(z1r, z1i, z2r, z2i, cfpv, n_cfpv, probeRe, probeIm, &nProbe);
    probe_lengths[p] = nProbe;
}
if (!(probe_lengths[0] == probe_lengths[1] && probe_lengths[1] == probe_lengths[2])) {
    fprintf(stderr, "Coefficient function %s returned varying coefficient length for fixed CFPV\n", spec->name);
    return 2;
}
```

At minimum, the first probe is required.
The multi-probe consistency check can be made optional behind a debug/strict flag if startup cost matters.

### 7. Call Through the New ABI

Every coeff function call site changes from:

```c
coeffFunc(z1r, z1i, z2r, z2i, cRe, cIm, &nCoeffs);
```

to:

```c
spec->fn(z1r, z1i, z2r, z2i, cfpv, n_cfpv, cRe, cIm, &nCoeffs);
```

That includes probe calls and main calls.

## Frontend Changes

### 1. Add Shared Catalog Script

Load the generated JS file in `index.html`.

Example:

```html
<script src="coeff_func_catalog_js.js"></script>
```

This must load before the compute-tab chip logic initializes.

### 2. Replace Hardcoded Dropdown with Generated Population

The current coefficient-function dropdown is manually hardcoded in:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L260)

That should be replaced with generated population from `window._coeffFuncCatalog`.

Recommended label rules:

- hand with no agreement:
  - `creative9 (degree 70) (hand) *`
- transpiled with agreement:
  - `poly_1 [100%] (degree 70) (transpiled)`
- transpiled stubbed:
  - `poly_13 [stub] (degree ?) (transpiled)`
- wrapped:
  - `old_giga_1 (degree 34) (wrapped)`

Agreement values should come from generated metrics, not hardcoded HTML.

Degree text is a hint only.
Do not treat dropdown degree as authoritative for the job.

### 3. Add Coefficient Function Param State

Introduce:

```js
let _cfpv = [];
```

This state belongs alongside:

- `_ptChain`
- `_ctChain`
- `_rtChain`

### 4. Build UI from Catalog

When the selected coefficient function changes:

1. read the generated catalog entry
2. create one input row for that function’s params
3. initialize values from defaults

The concrete state shape can remain simple.
It does not need to match the original exploratory object-based sketch exactly.

### 5. Render Inputs

Add a dedicated input area in the Compute tab, near the coefficient function selector.

Recommended UI:

```html
<div id="cfpv-row" class="chip-row"></div>
```

Inputs should:

- be created from catalog metadata
- initialize from defaults
- enforce numeric parsing
- surface invalid input clearly

### 6. Serialize CFPV on Compute

Serialize the currently visible CFPV inputs on compute.

Rules:

- empty input should fall back to that param's default
- invalid numeric input should fail clearly
- the serialized array should be included in both hires and lores coeffgen jobs

### 7. Save Metadata

In `calc.json`, store:

```json
"pipeline": {
  "param_transforms": ...,
  "function": "my_hand_func",
  "cfpv": [1.0, 0.75, 0.0],
  "cfpv_display": "alpha=1, sigma=0.75, turns=0",
  "coeff_transforms": ...
}
```

Human-readable display metadata is useful and proved worth keeping during implementation.

## Function Spec Lookup

Replace the current if/else pointer ladder with a spec-returning lookup.

Recommended implementation:

```c
static const CoeffFuncSpec *lookupCoeffFuncSpec(const char *name) {
    for (int i = 0; i < g_n_coeff_func_specs; i++) {
        if (strcmp(name, g_coeff_func_specs[i].name) == 0) {
            return &g_coeff_func_specs[i];
        }
    }
    return NULL;
}
```

This is still O(n), but the current code is already effectively O(n) through `strcmp` ladders.
That is fine for this use case.

Do not optimize this prematurely.

## Defaults and Backward Compatibility

### Defaults

The defaults in the catalog are used for:

- initial chip values in the UI
- server-side fallback if `cfpv` is missing or short

### Old Jobs

Old jobs will not have `cfpv`.
That is fine.

Runtime behavior should be:

- if function has no params: no change
- if function has params and old job omits `cfpv`: use defaults

This preserves compatibility.

### Per-Job Degree

Degree should be stored in job metadata only after runtime probing.

That means:

- catalog: no authoritative degree
- dropdown: no authoritative degree
- coeffgen metadata: authoritative for that job
- `calc.json.degree`: authoritative for that job

### Transpiled Functions

They always have:

- a normal catalog entry
- `kind = "transpiled"`
- `n_params = 0`
- `defaults = NULL`
- `cfpv = NULL`
- `n_cfpv = 0`

No behavioral change.

### Wrapped Functions

Wrapped functions are legacy implementations adapted into the new uniform ABI.

They should behave like any other no-param function unless explicitly cataloged otherwise.

## Why Not Query the Lambda for the Catalog?

You asked whether the Lambda needs a way to query the master table.

It does not need to for normal operation.

Better model:

- source catalog lives in source control
- test metrics are generated or post-processed from tests
- generation step merges them and emits:
  - backend C metadata
  - frontend JS metadata

This gives:

- deterministic builds
- no extra runtime endpoint
- no startup dependency
- no drift, if generated from one source

Optional debug-only endpoint is fine later, but should not be required for UI behavior.

## Specific File Changes

### New files

- [lambda/coeff_func_catalog.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_catalog.json)
- [lambda/coeff_func_metrics.json](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_metrics.json)
- [coeff_func_catalog_js.js](/Users/nicknassuphis/karpo_hackathon/polypaint/coeff_func_catalog_js.js)
- [lambda/gen_catalog.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/gen_catalog.py)
- [lambda/coeff_func_lookup.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeff_func_lookup.h)

### Existing files to change

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)
- transpiler/emitter files that generate function signatures, if you want ABI uniformity for generated functions
- deploy pipeline:
  - regenerate catalog artifacts before compile
  - upload JS catalog with `index.html`
  - validate host-probed JS metadata against the deploy binary in Docker
- optionally docs:
  - [docs/coefficients.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/coefficients.md)

Additional refactor points required by runtime degree:

- remove preflight degree parsing from option text in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3160)
- replace any UI logic that assumes dropdown degree is authoritative
- keep runtime degree from coeffgen probe as the single source of truth for each job

## Rollout Plan

### Phase 1: Backend ABI and Catalog

1. Add source catalog JSON.
2. Add metrics overlay JSON format.
3. Add generator script.
4. Generate C spec header and JS catalog.
5. Change `CoeffFuncC` signature.
6. Update wrappers.
7. Update hand-written functions that need params.
8. Update generated signatures to accept and ignore `cfpv`.
9. Replace pointer lookup with spec lookup.
10. Add runtime degree probing and fixed-CFPV consistency enforcement.

### Phase 2: Frontend UI

1. Load generated JS catalog.
2. Replace hardcoded dropdown with generated population.
3. Show `kind` / `agreement_pct` / `stubbed` in dropdown labels.
4. Add coefficient-function parameter UI row.
5. Reset/render inputs on function change.
6. Serialize `cfpv`.
7. Include `cfpv` in coeffgen and lores coeffgen jobs.
8. Save `cfpv` in `calc.json`.
9. Remove any preflight logic that infers degree by parsing dropdown text.

### Phase 3: Validation and Cleanup

1. Verify functions with no params still compute exactly as before.
2. Verify parametric hand-written functions use defaults when `cfpv` absent.
3. Verify transpiled and wrapped functions appear with the correct `kind` marker.
4. Verify agreement percentages appear from generated metrics, not hardcoded HTML.
5. Verify runtime degree probing works for the selected CFPV.
6. Verify UI initializes defaults correctly.
7. Verify old jobs still load/render.
8. Update docs.

## Testing Checklist

### Unit/backend tests

- lookup returns correct spec for:
  - no-param transpiled function
  - no-param hand-written function
  - parametric hand-written function
- merged metrics attach correctly:
  - hand function with no agreement metric
  - transpiled function with agreement metric
  - stubbed transpiled function
- runtime degree probe:
  - fixed `(function, cfpv)` yields stable `nCoeffs`
  - varying-length function fails clearly
- `cfpv` parser:
  - empty array
  - exact count
  - short vector padded with defaults
  - too many args rejected
- coeffgen output unchanged for:
  - several no-param functions
- parametric function output changes when `cfpv` changes
- old payload with no `cfpv` produces default-backed output
- Docker/runtime validation:
  - deploy binary accepts CFPV behavior correctly
  - host-generated degree hints match deploy-binary probes

### Frontend tests/manual checks

- selecting a no-param function shows no extra inputs
- selecting a parametric function shows correct labeled inputs
- changing function resets inputs to that function’s defaults
- dropdown shows kind correctly
- dropdown shows agreement percentage correctly when available
- dropdown shows stub marker correctly when applicable
- compute payload includes `cfpv`
- `calc.json` stores `cfpv`
- UI no longer depends on parsing degree from dropdown text

### Regression tests

Add a dedicated small suite for a hand-written parametric function.

Recommended pattern:

1. compute with defaults
2. compute with changed `cfpv`
3. assert coeff output differs in expected way
4. omit `cfpv`, assert output equals default case

## Acceptance Criteria

The feature is complete when:

- every coefficient function is represented in the shared catalog
- every coefficient function can carry intrinsic metadata regardless of implementation style
- all functions compile under the new ABI
- transpiled functions remain behaviorally unchanged
- at least one hand-written function successfully uses `cfpv`
- dropdown metadata comes from generated catalog, not hardcoded `<option>` text
- frontend initializes chips from defaults
- compute sends `cfpv`
- coeffgen uses `cfpv`
- degree is derived per job from runtime probing, not from catalog text
- old jobs still work
- no runtime Lambda catalog query is required

## Recommended First Parametric Function

Do not retrofit many functions at once.

Start with one hand-written function:

- add 2-3 parameters
- prove:
  - UI defaults
  - payload serialization
  - backend parsing
  - coeff output changes correctly
  - old default behavior preserved

Then roll out to additional hand-written functions.

## Final Recommendation

Yes, this is surgery, but it is the right kind of surgery.

The important decisions are:

- do not bolt on a one-off `function_args` array with duplicated frontend/backend metadata
- do not make the browser discover parameter defaults from a Lambda call
- do normalize CFPV on the backend from the function spec
- do treat dropdown degree as advisory metadata only
- do validate host-probed metadata against the deploy binary before release

Instead:

- define a shared master catalog
- generate backend and frontend metadata from it
- move coefficient lookup from “function pointer only” to “function spec”
- pass `cfpv` through a uniform ABI

That gives you a stable foundation for any future hand-written parametric coefficient family.
