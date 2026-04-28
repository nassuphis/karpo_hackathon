# Coeff Program Design

## Intent

Replace the current coefficient-transform chain with a program pipeline that uses
the same user-facing pattern as Param Program and Solve Score programs:

- chip-based editor
- stack diagnostics
- insert before / after
- saved programs
- read-only chip previews
- explicit output writes
- benchmark-first rollout

This should reuse the pattern and lessons from Param Program, not the exact
Param Program implementation. Coeff programs operate on complex coefficient
vectors, not on scalar complex parameters.

The goal is to make coefficient manipulation visible and composable. The current
coefficient transform row is a linear list of opaque transforms. Coeff Program
should let users branch, reuse the original coefficient vector, combine
intermediate vectors, save named constructions, and still keep the old chain
path intact.

## Core Model

For every coefficient-generation step, the coefficient function first produces a
read-only input vector:

```text
cf = [complex coefficient 0, complex coefficient 1, ...]
```

The program output vector is:

```text
poly
```

`poly` is initialized to `cf` before the program runs. This makes an empty
program identity and preserves the current pipeline's "start with coefficient
function output, then transform it" behavior.

The VM also has a stack of complex vectors. Vectors can have different lengths.
Length is part of the stack value.

Execution shape:

```text
cf    = coefficient_function(p1, p2)   # read-only
poly  = copy(cf)                       # mutable output/current register
stack = []

for chip in coeff_program:
    execute chip

return poly
```

`cf` never changes. `poly` is the final vector written to coeff output. Chips may
read `cf`, read/write `poly`, or consume/produce stack vectors depending on their
`src` / `tgt` selectors.

## Registers And Stack

### `cf`

`cf` is read-only.

It is the raw coefficient vector returned by the selected coefficient function
after parameter generation. It is useful for branching:

```text
legacy(deriv, src=cf, tgt=push)
legacy(rev, src=cf, tgt=push)
```

### `poly`

`poly` is the mutable output register.

It is initialized to `cf`. Legacy chain equivalence is:

```text
poly = cf
legacy(transform_1, src=poly, tgt=poly)
legacy(transform_2, src=poly, tgt=poly)
...
return poly
```

This preserves today's in-place coefficient-transform chain semantics.

### Stack

The stack stores complex vectors:

```text
Vector {
    length: int
    re[COEFF_PROGRAM_MAX_VECTOR_LEN]
    im[COEFF_PROGRAM_MAX_VECTOR_LEN]
}
```

Stack entries may have different lengths. Chips that combine two vectors must
define how length mismatch is handled before they ship.

V1 should avoid ambiguous vector-combine operations unless their length policy is
pinned.

The native implementation must not allocate stack vectors dynamically. The stack
is a fixed-size per-thread ring of vector slots:

```text
COEFF_PROGRAM_MAX_VECTOR_STACK = 256
COEFF_PROGRAM_MAX_VECTOR_LEN   = 256

stack_re[256][256]
stack_im[256][256]
stack_len[256]
stack_depth
stack_head
```

Push writes into the next ring slot. Pop returns the current top ring slot and
moves the head backward. The runtime tracks `stack_depth` separately so wrap
around never overwrites a live vector. Pushing when `stack_depth == 256` is a
hard error. This is still a stack semantically, but the physical storage is a
ring so no slot shifting or heap allocation happens in the row loop.

This is required because coeff programs run millions of times and the coeffgen
path is multi-threaded. Every worker thread owns its own ring workspace.

## Source And Target Selectors

Every compiled coefficient transform chip should have explicit `src` and `tgt`
selectors, in addition to its own parameters. The editor may hide those
selectors on direct sugar chips and default them to `poly -> poly`, but the
serialized execution form must not rely on implicit source or target behavior.

Allowed source selectors:

```text
poly   # read the current output/current working vector
cf     # read the immutable coefficient-function output
pop    # pop one vector from the stack and use it as input
peek   # optional v2; read top of stack without popping
```

Recommended v1 source set:

```text
poly
cf
pop
```

Allowed target selectors:

```text
poly   # write the transformed vector to the output/current working vector
push   # push the transformed vector onto the stack
```

Optional future target:

```text
discard
```

Do not use ambiguous names like `stack` as a selector. `pop` and `push` make the
stack effect explicit.

Examples:

```text
legacy(rev, src=poly, tgt=poly)
legacy(deriv, src=cf, tgt=push)
legacy(normalize, src=pop, tgt=poly)
```

## Push And Emit

### `push`

`push(source)` copies a vector onto the stack.

Allowed sources:

```text
cf
poly
```

Semantics:

```text
push(cf)   -> push copy(cf)
push(poly) -> push copy(poly)
```

`push` is useful when the user wants to preserve an intermediate vector before
continuing to mutate `poly`.

### `emit`

`emit` pops one vector from the stack and stores it into `poly`.

Semantics:

```text
emit -> poly = pop()
```

There is only one output register, so `emit` does not need a target dropdown in
v1. If a target is kept for UI consistency, it should have only one value:
`poly`.

The program is valid if it emits zero times. In that case `poly` remains whatever
the in-place transforms made it, or identity `cf` for an empty program.

Recommended v1 final-stack rule: final stack must be empty. If users need to
discard scratch vectors, use `pop` or `flush`.

## Stack Utility Chips

V1 stack utility chips:

```text
duplicate
swap
pop
flush
```

Semantics:

```text
duplicate -> duplicate top vector
swap      -> swap top two vectors
pop       -> discard top vector
flush     -> clear the whole vector stack
```

`flush` does not reset `poly` and does not modify `cf`.

## Transform Chips

Existing coefficient transforms should be exposed as `legacy(...)` chips first.
Each legacy coefficient transform receives:

```text
legacy(name, src, tgt, args...)
```

The compiler resolves `name` to an integer function index. Native execution must
never string-dispatch inside the hot row loop for Program mode.

Current coefficient-transform categories from `index.html`:

```text
Structural:
rev
conj
normalize
deriv
safe
negate_odd
max2one

Ordering + accumulation:
sort_mod_keep_angle
sort_angle_keep_mod
sort_abs
cumsum
cummax
sort_cumsum

Elementwise:
scale100
swirler
exp
cos
sin
tan
cosh
sinh
tanh
round
pow
power
invpower

Root-derived:
roots_cm
roots
```

Each transform must declare:

```text
name
fn_index
allowed_src
allowed_tgt
args
length_policy
changes_length
supports_andy
```

Do not guess these dynamically from the UI catalog. This needs a real registry.

## Andy Semantics

The existing coefficient-transform UI appends an `andy` parameter to every
coefficient transform. Native `dispatchCt` currently:

1. copies the input vector before the transform when `andy != 0`
2. runs the transform
3. blends the transformed vector with that copied input vector

Coeff Program must preserve that behavior for legacy transforms.

For Program mode, `andy` is local to the selected `src` vector:

```text
legacy(rev, src=cf, tgt=push, andy=0.25)
```

means "copy `cf`, reverse it, then blend the reversed result with the original
`cf` input at andy=0.25, then push the result."

It does not blend against global `poly` unless `src=poly`.

## Vector Length Policy

Vector length is load-bearing for coefficient programs.

Existing transforms can preserve length or change length:

```text
preserve length:
rev, conj, normalize, safe, negate_odd, max2one, sort_*, cumsum, cummax,
sort_cumsum, swirler, exp, cos, sin, tan, cosh, sinh, tanh, round, pow

may change length:
deriv
power
invpower
roots
roots_cm
```

The compiler registry must record length behavior at least coarsely:

```text
preserves_length
may_change_length
requires_nonempty
```

Native execution remains authoritative because some length changes depend on
runtime input length and transform parameters.

Hard runtime rules:

- vector length must be `>= 1`
- vector length must be `<= COEFF_PROGRAM_MAX_VECTOR_LEN`
- transforms that produce invalid length fail with contextual errors
- emitted `poly` length determines `n_coeffs` and `degree`

Degree probe must run the exact same coeff program as full coeffgen. Do not
infer degree from the source chain.

V1 Program mode has a deliberate fixed vector cap:

```text
COEFF_PROGRAM_MAX_VECTOR_LEN = 256
```

If the coefficient function or any transform produces more than 256 coefficients,
Program mode rejects with a clear error. Chain mode remains available for larger
legacy coefficient-transform jobs until a larger fixed arena is justified by
benchmarks.

## Vector Combine Chips

Vector combine chips are useful but risky because lengths can differ.

Candidate v2 chips:

```text
vadd
vsub
vmul
vscale
concat
slice
pad
blend
```

Recommended v1: do not ship generic vector-combine chips except stack utility
and legacy transforms. If `blend` is needed, ship only after pinning:

```text
same length required vs pad shorter vs truncate to min length
```

Hidden truncation would be a serious source of bad outputs.

## Macro Chips

### `macro`

`macro(name)` expands a named coefficient program from the program catalogue.

Rules mirror Param Program:

- macros expand before native execution
- expansion is recursive only up to a fixed depth
- cyclic macro references are compile errors
- missing macro names are compile errors
- expanded programs are validated after expansion
- cache keys use the fully-expanded canonical execution program

Saved catalogue programs live under a coefficient-program-specific S3 prefix:

```text
polypaint/coeff-programs/
```

Every saved program must include:

```json
{
  "program_kind": "coeff_program",
  "version": 1
}
```

Macro expansion is snapshot-based for compute artifacts. Compute plan expands
macros and stores the expanded compiled program in `calc.json`. Later render or
preview paths for that compute artifact do not do live S3 macro lookup.

Live editor preview may read the current macro body from S3.

## Chip Effect Table

Every chip needs explicit effects for editor diagnostics and compiler
validation.

```text
push(cf)            pops 0, pushes 1 vector, writes none
push(poly)          pops 0, pushes 1 vector, writes none
emit                pops 1, pushes 0, writes poly
macro(name)         source-only; expands before validation

duplicate           pops 1, pushes 2, writes none
swap                pops 2, pushes 2, writes none
pop                 pops 1, pushes 0, writes none
flush               pops all, pushes 0, writes none

legacy(name, cf,   poly, args...)   pops 0, pushes 0, writes poly
legacy(name, cf,   push, args...)   pops 0, pushes 1, writes none
legacy(name, poly, poly, args...)   pops 0, pushes 0, writes poly
legacy(name, poly, push, args...)   pops 0, pushes 1, writes none
legacy(name, pop,  poly, args...)   pops 1, pushes 0, writes poly
legacy(name, pop,  push, args...)   pops 1, pushes 1, writes none
```

`peek` is intentionally omitted from v1. It is easy to add later, but `pop`
makes ownership and stack depth obvious.

## Compatibility

Existing coefficient-transform chains must keep working unchanged.

The old Chain mode is not a migration path; it remains a first-class execution
path.

Old chain:

```json
[
  ["rev"],
  ["normalize"],
  ["roots", "8", "hi"]
]
```

Equivalent coeff program:

```json
[
  ["legacy", "rev", "poly", "poly"],
  ["legacy", "normalize", "poly", "poly"],
  ["legacy", "roots", "poly", "poly", "8", "hi"]
]
```

An empty old transform chain maps to an empty coeff program:

```text
poly = cf
return poly
```

## Chain Vs Program Mode

The Compute tab should have one pipeline mode dropdown:

```text
Use: Chain | Program
```

Semantics:

```text
Chain:
    use legacy param transform chain
    use legacy coeff transform chain
    ignore param_program_chain
    ignore coeff_program_chain

Program:
    use Param Program
    use Coeff Program
    ignore legacy param transform chain
    ignore legacy coeff transform chain
```

This replaces the current param-only `Use` dropdown.

Do not let users accidentally mix Param Program with legacy coefficient chains
in the main path. Mixed modes create hard-to-debug cache identity and preview
questions. If mixed modes are useful later, add an explicit advanced mode after
the two-program model is stable.

Compute Preview must use the same dropdown choice as full Compute:

- Chain mode sends `param_transforms` and `coeff_transforms`
- Program mode sends `param_program_chain` and `coeff_program_chain`

The displayed preview log must show the active mode:

```text
Compute preview (AE-MT, Program): [param_program...] [coeff_program...] ...
Compute preview (AE-MT, Chain): [param_transforms...] [coeff_transforms...] ...
```

## UI

The Coeff Program UI should reuse the Param Program editor pattern:

- chip strip
- add-chip popup grouped by category
- line selection
- `+ before` and `+ after`
- move arrows
- read-only saved program preview
- save/load/download/upload modal
- permissive editing with diagnostics

UI sections in Compute:

```text
Use: Chain | Program

Param transforms            # active only in Chain mode
Coeff transforms            # active only in Chain mode

Param Program               # active only in Program mode
Coeff Program               # active only in Program mode
```

Inactive sections may remain visible but visually muted. They must not look like
they are being executed.

The Coeff Program add-chip popup should group chips:

```text
Input/output:
push(cf)
push(poly)
emit

Stack:
duplicate
swap
pop
flush

Legacy structural:
rev
conj
normalize
deriv
safe
negate_odd
max2one

Legacy ordering:
sort_mod_keep_angle
sort_angle_keep_mod
sort_abs
cumsum
cummax
sort_cumsum

Legacy elementwise:
scale100
swirler
exp
cos
sin
tan
cosh
sinh
tanh
round
pow
power
invpower

Legacy roots:
roots
roots_cm

Macros:
macro(name)

Bridge:
legacy(name, src, tgt, args...)
```

The direct chips like `rev` are source-level sugar. They should lower to:

```text
legacy(rev, src=poly, tgt=poly)
```

The generic `legacy(...)` chip remains available for advanced cases where the
user needs `src=cf`, `src=pop`, or `tgt=push`.

Follow `ui_docs/style_guide.md`:

- controls align vertically
- modal action rows remain visible
- saved-program tables do not cause modal height jumps
- no debug JSON as the primary representation
- chip previews look like the editor chips

## Serialization

Saved source format:

```json
{
  "version": 1,
  "program_kind": "coeff_program",
  "name": "reverse normalized roots",
  "chain": [
    ["rev"],
    ["normalize"],
    ["legacy", "roots", "poly", "poly", "8", "hi"]
  ]
}
```

Canonical compiler output:

```json
{
  "program_kind": "coeff_program",
  "version": 1,
  "source_chain": [["rev"], ["normalize"]],
  "tokens": [
    {"op": 1, "fn_index": 1, "src": 2, "tgt": 2, "args": []},
    {"op": 1, "fn_index": 3, "src": 2, "tgt": 2, "args": []}
  ],
  "fingerprint": "sha1...",
  "display": "rev; normalize",
  "stack_max": 0,
  "uses_legacy_chain_equivalent": true
}
```

The exact enum values do not matter in the doc. The stable contract is:

- source names are compiled before native execution
- source JSON is preserved for UI
- execution tokens drive cache keys
- macro-expanded canonical tokens drive fingerprints

## Cache Keys

Compute cache identity must include:

- pipeline mode: `chain` or `program`
- Param Program execution fingerprint when Program mode is active
- Coeff Program execution fingerprint when Program mode is active
- legacy param transform chain when Chain mode is active
- legacy coeff transform chain when Chain mode is active
- coefficient function and coefficient function parameters

Do not hash only the source chip labels. Macro bodies, defaults, aliases, and
compiled args must be reflected in the canonical fingerprint.

`compute_fused.build_probe_signature` must include the effective coefficient
program fingerprint. Otherwise stale coeff chunks can be reused after a program
change.

## Native Execution

This needs a new native coefficient-program evaluator used inside
`sweep_coeffgen`.

Do not route it through Param Program or Solve Score evaluators.

Native contract per coefficient row:

```c
CoeffProgramInputs:
    const double *cf_re;
    const double *cf_im;
    int cf_len;

CoeffProgramOutputs:
    double *poly_re;
    double *poly_im;
    int *poly_len;
```

Execution:

```text
copy cf -> poly
clear stack
for token in program:
    execute token
return poly
```

### Buffer Ownership

This is the most important implementation detail.

Coefficient vectors are much larger than Param Program scalar values. The hot
loop must not allocate per chip or per row.

Required model:

- compiled program is immutable
- each worker thread owns a `CoeffProgramWorkspace`
- workspace contains:
  - `poly` buffer
  - fixed ring-buffer stack slots
  - scratch buffers for legacy transforms
  - vector lengths for each slot
- stack slots store vector descriptors pointing into workspace-owned ring slots
- stack push/pop updates integer ring indices and depth only
- no `malloc` inside row execution
- no vector buffer allocation inside row execution
- no shifting stack slots on push/pop
- no global mutable evaluator state

Approximate memory:

```text
one complex vector = 256 * 2 * sizeof(double)
                   = 256 * 16
                   = 4 KiB

ring stack         = 256 vectors ~= 1 MiB/thread
poly + cf + scratch ~= a few additional vectors
```

This is acceptable for normal thread counts, but must be measured. The point is
to trade a bounded per-thread memory reservation for predictable speed and zero
allocator pressure.

Recommended v1 limits:

```text
MAX_PROGRAM_TOKENS = 64 after macro expansion
MAX_VECTOR_STACK   = 256 ring slots
MAX_VECTOR_LEN     = 256 complex coefficients
MAX_MACRO_DEPTH    = 8
```

The existing native `MAX_COEFFS` may remain larger for Chain mode. Coeff Program
uses the stricter fixed arena limit unless benchmarks prove a larger ring is
safe.

### Native Token Shape

There must be no string lookup in Program-mode hot execution.

Compile:

```text
source chip name -> integer opcode
legacy transform name -> integer function index
src/tgt selector -> small integer
numeric parameters -> parsed doubles or enum args
choice parameters -> parsed enum values
```

Example C shape:

```c
typedef struct {
    uint16_t op;
    uint16_t fn_index;
    uint8_t src;
    uint8_t tgt;
    uint8_t n_args;
    uint8_t reserved;
    double args[COEFF_PROGRAM_MAX_ARGS];
} CoeffProgramToken;
```

Workspace shape:

```c
#define COEFF_PROGRAM_MAX_VECTOR_STACK 256
#define COEFF_PROGRAM_MAX_VECTOR_LEN   256

typedef struct {
    double stack_re[COEFF_PROGRAM_MAX_VECTOR_STACK][COEFF_PROGRAM_MAX_VECTOR_LEN];
    double stack_im[COEFF_PROGRAM_MAX_VECTOR_STACK][COEFF_PROGRAM_MAX_VECTOR_LEN];
    uint16_t stack_len[COEFF_PROGRAM_MAX_VECTOR_STACK];
    uint16_t stack_depth;
    uint16_t stack_head;

    double poly_re[COEFF_PROGRAM_MAX_VECTOR_LEN];
    double poly_im[COEFF_PROGRAM_MAX_VECTOR_LEN];
    uint16_t poly_len;

    double scratch_re[COEFF_PROGRAM_MAX_VECTOR_LEN];
    double scratch_im[COEFF_PROGRAM_MAX_VECTOR_LEN];
    uint16_t scratch_len;
} CoeffProgramWorkspace;
```

The exact struct can change, but these invariants cannot:

- one workspace per worker thread
- all vector buffers are fixed arrays
- no row-loop allocation
- stack slots are addressed by bounded ring indices
- overflow and vector-too-long cases are explicit runtime errors

Legacy function table:

```c
typedef int (*CoeffLegacyFn)(
    double *re,
    double *im,
    int *n_coeffs,
    const double *args,
    int n_args
);
```

The existing `dispatchCt` implementation can be refactored into callable
functions behind this table. The old chain path can keep `dispatchCt` until
Program mode is proven.

## Legacy Registry

Create a single source of truth:

```text
lambda/coeff_legacy_registry.json
```

Schema:

```json
{
  "version": 1,
  "functions": [
    {
      "name": "rev",
      "fn_index": 1,
      "category": "structural",
      "allowed_src": ["cf", "poly", "pop"],
      "allowed_tgt": ["poly", "push"],
      "args": [],
      "supports_andy": true,
      "length_policy": "preserve"
    },
    {
      "name": "roots",
      "fn_index": 30,
      "category": "roots",
      "allowed_src": ["cf", "poly", "pop"],
      "allowed_tgt": ["poly", "push"],
      "args": [
        {"name": "k", "default": 8, "type": "int"},
        {"name": "pad", "default": "hi", "choices": ["hi", "lo"]}
      ],
      "supports_andy": true,
      "length_policy": "may_change"
    }
  ]
}
```

Rules:

- stable `fn_index` once shipped
- no duplicate names
- no duplicate indices
- Python compiler and C function table are checked against the registry
- UI catalog should be generated from or verified against the registry
- argument defaults are applied at compile time
- unsupported selectors are compile errors

## Validation

### Editor Validation

The editor should allow temporary invalid programs while users compose.

Diagnostics:

```text
valid here
will underflow here
program leaves stack depth 1
emit needs a vector on the stack
legacy(roots) may change vector length
poly is identity cf unless a chip writes it
```

Do not force users into sequencing puzzles.

### Execution Validation

Execution is strict:

- chip names must be known
- macro expansion must succeed
- no macro cycles
- numeric args must be finite
- enum args must be valid
- stack underflow is an error
- final stack must be empty
- vector length must remain in `[1, COEFF_PROGRAM_MAX_VECTOR_LEN]`
- emitted `poly` must be a valid coefficient vector

Invalid Program-mode payloads should return clear 400-level errors in
synchronous preview paths, not generic HTTP 500s.

## Benchmark Plan

Coeff Program needs its own benchmarks. Param Program numbers do not prove this
feature because coefficient vectors are larger and transforms can copy or change
length.

### Native Microbenchmark

Compare:

```text
legacy coeff transform chain using dispatchCt
compiled coeff-program evaluator using integer tokens
```

Exclude:

- S3
- param generation
- coefficient function evaluation when possible
- solve
- rasterization

Measure:

```text
rows/sec
ns/row
ns/token
bytes copied/row
workspace bytes/thread
max vector stack depth
```

Benchmark matrix:

```text
n_coeffs=8, 16, 64, 128, 256
rows=10k, 100k, 1M where feasible
threads=1, 2, 4, 8

program=identity
program=rev
program=rev;normalize;conj
program=deriv
program=roots(k=8,hi)
program=push(cf); legacy(deriv,pop,push); push(cf); legacy(rev,pop,push); pop; emit
program=macro-expanded structural chain
```

Penalty formula:

```text
penalty_pct = 100 * (program_wall - chain_wall) / chain_wall
```

Acceptance target:

- legacy-equivalent Program mode should be within 20% of Chain mode, or Chain
  remains the default for old jobs
- Program-only vector branching must report its cost clearly in logs
- no per-row allocation in profiles
- fixed workspace allocation happens once per worker thread
- multi-thread scaling must not collapse from shared state or memory contention
- Program mode rejects vectors longer than 256 with a clear error

### End-To-End Coeffgen Benchmark

Run real `sweep_coeffgen` modes:

```text
coeffgen
coeffgen_chunked
degree_probe
compute preview
fused chunk path
```

Measure:

```text
coeffgen_us
n_coeffs
degree
coeffs_size
threads
program_fingerprint
workspace_bytes_per_thread
```

Output equivalence cases:

- Chain `rev; normalize` equals Program `legacy(rev,poly,poly); legacy(normalize,poly,poly)`
- Chain `roots(8,hi)` equals Program `legacy(roots,poly,poly,8,hi)`
- Program mode single-thread equals Program mode multi-thread

Local benchmark is sufficient for the first ratio check because we care about
relative evaluator cost. Docker runtime is still required before deploy-ready if
native binaries change.

## Concrete Implementation Plan

### Stage 0: Benchmark Spike

Goal: prove vector VM overhead and memory behavior before backend wiring.

Files:

- `lambda/coeff_program.h`
- `lambda/coeff_program_bench.c` or temporary `sweep_cli.c` benchmark mode
- `tests/docker_runtime_regression.py`

Work:

- implement minimal token evaluator:
  - `push(cf)`
  - `push(poly)`
  - `emit`
  - `pop`
  - `flush`
  - `legacy(rev)`
  - `legacy(conj)`
  - `legacy(normalize)`
- allocate per-thread workspace once
- compare against `dispatchCt`
- measure memory and rows/sec

Exit criteria:

- output matches for identity, `rev`, `rev;conj`, and `normalize`
- no row-loop allocation
- benchmark reports penalty and workspace bytes/thread

### Stage 1: Compiler And Registry

Goal: compile readable source chains into canonical execution tokens.

Files:

- `lambda/coeff_program_chain.py`
- `lambda/coeff_legacy_registry.json`
- `tests/test_coeff_program_chain.py`

Work:

- parse source chains
- expand macros
- validate stack effects
- resolve legacy names to indices
- parse numeric and enum args
- produce fingerprint
- produce diagnostics
- detect exact legacy-chain equivalence

Compiler output:

```json
{
  "program_kind": "coeff_program",
  "version": 1,
  "source_chain": [["rev"], ["normalize"]],
  "tokens": [],
  "fingerprint": "sha1...",
  "display": "rev; normalize",
  "stack_max": 0,
  "uses_legacy_chain_equivalent": true,
  "legacy_coeff_transforms": [["rev"], ["normalize"]]
}
```

Exit criteria:

- equivalent source chains hash the same after lowering
- invalid selectors fail clearly
- unknown transform names fail clearly
- old coefficient chains compile to Program-equivalent source

### Stage 2: Saved Coeff Programs

Goal: provide storage and macro support.

Files:

- `lambda/handler_storage.py`
- `tests/test_coeff_program_storage.py`
- `deploy.sh`
- `api_manifest.json`

Routes:

```text
/list-coeff-programs
/fetch-coeff-program
/save-coeff-program
/delete-coeff-program
```

Storage prefix:

```text
polypaint/coeff-programs/
```

Exit criteria:

- save/load/delete round trips source chain
- macro resolver reads saved coeff programs only
- `program_kind` mismatch is rejected
- routes appear in generated config/API manifest

### Stage 3: Native VM In `sweep_coeffgen`

Goal: make `sweep_coeffgen` accept compiled `coeff_program` tokens.

Files:

- `lambda/coeff_program.h`
- `lambda/sweep_cli.c`
- rebuilt `lambda/sweep_coeffgen`
- `tests/test_coeff_program_native.py`
- `tests/docker_runtime_regression.py`

Native payload:

```json
{
  "mode": "coeffgen_chunked",
  "coeff_program": {
    "version": 1,
    "fingerprint": "sha1...",
    "stack_max": 2,
    "tokens": []
  }
}
```

Work:

- parse compiled `coeff_program`
- run coeff program anywhere `coeff_transforms` currently run:
  - degree probe
  - monolithic coeffgen
  - coeffgen_chunked
  - fused chunk local coeffgen
- keep `coeff_transforms` path alive
- allocate one `CoeffProgramWorkspace` per worker thread
- emit metadata:
  - `coeff_program_tokens`
  - `coeff_program_stack_max`
  - `coeff_program_fingerprint`
  - `coeff_program_workspace_bytes`
  - `uses_legacy_chain_equivalent`

Exit criteria:

- Chain and Program output match for equivalent transforms
- threaded Program output is deterministic
- degree probe and full coeffgen agree on degree/n_coeffs

### Stage 4: Backend Wiring

Goal: thread Coeff Program through planning, preview, fused compute, and
metadata.

Files:

- `lambda/handler_compute_plan.py`
- `lambda/handler_coeffgen.py`
- `lambda/handler_compute_chunk_fused.py`
- `lambda/handler_compute_preview.py`
- `lambda/handler_param_debug.py`
- `lambda/compute_fused.py`
- `stepfunctions/compute_workflow.asl.json.template`
- `tests/test_compute_plan.py`
- `tests/test_coeffgen_param_gen.py`
- `tests/test_compute_preview_handler.py`
- `tests/test_compute_workflow_definition.py`

Work:

- add `pipeline_mode: "chain" | "program"`
- Chain mode forwards only:
  - `param_transforms`
  - `coeff_transforms`
- Program mode forwards only:
  - compiled `param_program`
  - compiled `coeff_program`
- compute preview uses the same mode
- fused probe signature includes both program fingerprints
- `calc.json` stores:
  - `pipeline_mode`
  - source program chains
  - compiled fingerprints
  - display strings
  - legacy chains for compatibility/display where relevant

Exit criteria:

- old jobs with only chains still run
- Program mode compute preview and full compute use the same effective programs
- stale cache is invalidated when either program changes
- errors include phase and program context

### Stage 5: Frontend UI

Goal: expose Chain/Program mode and a usable Coeff Program editor.

Files:

- `index.html`
- `tests/test_frontend_js.sh`
- Playwright e2e if available

Work:

- replace current param-only Use dropdown with:

```text
Use: Chain | Program
```

- Chain mode:
  - legacy Param transforms active
  - legacy Coeff transforms active
  - Param Program and Coeff Program muted/idle
- Program mode:
  - Param Program active
  - Coeff Program active
  - legacy transform rows muted/idle
- add Coeff Program editor:
  - chip picker
  - `+ before` / `+ after`
  - move arrows
  - saved program modal
  - read-only chip previews
  - macro selection
  - diagnostics
- compute preview payload follows selected mode
- full compute payload follows selected mode
- populate from existing calc/job restores the mode and source chains

Exit criteria:

- the user can run old Chain path side-by-side with Program path
- active/inactive state is visually obvious
- no controls jump or disappear in modals
- no debug JSON-first saved program display
- frontend tests assert payload mode and routes

### Stage 6: Packaging And Deploy Contract

Files:

- `deploy.sh`
- `api_manifest.json`
- `tests/test_deploy_packaging.py`

Package `coeff_program_chain.py` and `coeff_legacy_registry.json` into every
handler that imports them.

Expected consumers:

- `handler_storage.py`
- `handler_compute_plan.py`
- `handler_coeffgen.py`
- `handler_compute_chunk_fused.py`
- `handler_compute_preview.py`

If render/preview handlers read compiled coeff programs from `calc.json`, add
them too.

Exit criteria:

- `python3 api_manifest.py --check` passes
- packaging test knows every importing handler
- storage routes are in config and manifest
- Step Functions create/update rendering both include new selectors

### Stage 7: Test Gates

Targeted development gates:

```bash
git diff --check
bash tests/test_frontend_js.sh
uv run python -m pytest tests/test_coeff_program_chain.py tests/test_coeff_program_storage.py tests/test_coeff_program_native.py tests/test_compute_plan.py tests/test_compute_preview_handler.py tests/test_coeffgen_param_gen.py tests/test_compute_workflow_definition.py tests/test_deploy_packaging.py -q
```

Native/runtime gate after changing `sweep_cli.c`, `coeff_program.h`, or rebuilt
native binaries:

```bash
bash scripts/test-docker-runtime.sh
```

Predeploy gate before deploy-ready:

```bash
bash scripts/predeploy_check.sh
```

In this repo, `uv`, Playwright, `scripts/predeploy_check.sh`, and
`scripts/test-docker-runtime.sh` require immediate escalation in the local Codex
environment.

## V1 Scope

Ship in v1:

- Chain/Program dropdown controlling both Param and Coeff paths
- Coeff Program compiler
- Coeff Program saved program storage
- Coeff Program editor
- `push(cf)`, `push(poly)`, `emit`
- `duplicate`, `swap`, `pop`, `flush`
- `macro(name)`
- direct sugar for every existing coefficient transform as
  `legacy(name, src=poly, tgt=poly, args...)`
- generic `legacy(name, src, tgt, args...)`
- source selectors: `cf`, `poly`, `pop`
- target selectors: `poly`, `push`
- exact legacy-chain equivalence detection
- compute preview parity with selected mode
- full compute/fused/lores parity with selected mode
- benchmark results in this document before implementation is called ready

Do not ship in v1:

- generic vector arithmetic unless length policy is pinned
- mixed Chain/Program mode in the main UI
- dynamic macro lookup in native execution
- string lookup in Program-mode hot loops
- per-row heap allocation
- hidden truncation/padding of vector lengths
- `peek` unless there is a concrete UX need

## Open Questions

1. Should `emit` require final stack empty, or should it imply `flush` after
   writing `poly`? Recommended: final stack must be empty.
2. Should `poly` be readable by `push(poly)` in v1? Recommended: yes, because it
   makes branching from the current transformed vector natural.
3. Should all direct coeff chips expose `src`/`tgt`, or only the generic
   `legacy(...)` chip? Recommended: direct chips default to `poly -> poly` and
   have an advanced disclosure for `src`/`tgt`; generic legacy always shows
   selectors.
4. Should `andy` remain visible on every direct coeff chip? Recommended: yes
   for parity with existing chain UI, but layout must be improved so it does not
   crowd the chip.
5. Should legacy-equivalent Program mode run through the VM or old chain path?
   Recommended: measure first. Preserve Chain mode regardless.

## Opinion

The design makes sense, but it is harder than Param Program.

Param Program values are two scalar complex numbers. Coeff Program values are
variable-length vectors, and many existing transforms mutate in place or change
length. The risk is not the stack machine idea; the risk is sloppy buffer
ownership and unclear length semantics.

The right boundary is:

- frontend/source programs are expressive and readable
- compiler resolves all names, defaults, choices, macros, and selectors
- native evaluator receives immutable integer tokens
- each worker thread owns all vector workspace
- Chain mode remains untouched and available

If Stage 0 shows row-loop allocation or vector copies dominate runtime, fix the
workspace model before adding UI. The feature is only worth shipping if Program
mode is expressive without making normal compute feel unpredictable.
