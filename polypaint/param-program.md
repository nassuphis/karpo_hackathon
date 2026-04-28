# Param Program Design

## Intent

Replace the current fixed param-transform chain with a program pipeline that
uses the same interaction pattern as Solve Score programs:

- chip-based editor
- stack diagnostics
- insert before / after
- saved programs
- read-only chip previews
- explicit output writes

This should reuse the Solve Score UX pattern, not the Solve Score evaluator.
Param programs operate on complex parameter values, while Solve Score programs
operate on scalar score values.

The goal is user-facing expressivity, not just internal flexibility. A lot of
the current parameter logic lives in opaque transform chips. Param programs
should surface that complexity as readable, editable, reusable programs so users
can see what can be done and compose their own variants.

## Core Model

Each parameter-grid row starts with two read-only input registers:

```text
t1 = complex(u, 0)
t2 = complex(v, 0)
```

`u` and `v` are the existing normalized grid coordinates in `[0, 1]`, using the
same row ordering and serpentine behavior as the current param generation path.

Each row also has two output registers:

```text
p1 = t1
p2 = t2
```

`p1` and `p2` are the parameter pair fed into coefficient generation after the
program finishes. They are initialized to `t1` and `t2` before the program runs,
so an empty program is identity.

The VM also has a complex-value stack.

Nothing in this model prevents dithering. Dithering can be implemented by
calling the same compiled program repeatedly with the same `t1`, `t2` inputs and
combining the resulting parameter samples in the existing caller. The program
itself remains deterministic for a given call unless a future explicit random
chip is added.

## Registers

### Inputs

`t1` and `t2` are read-only.

They represent the original grid coordinates for the current solve row. Program
chips can read them repeatedly, but cannot mutate them.

### Outputs

`p1` and `p2` are write-only from the program's point of view.

`emit(p1)` pops one complex value from the stack and writes it to `p1`.

`emit(p2)` pops one complex value from the stack and writes it to `p2`.

After execution, the final values of `p1` and `p2` are passed to the coefficient
function.

## Stack Values

All stack values are complex numbers.

Real constants are represented as complex values with zero imaginary part.

This keeps v1 simple. Do not introduce mixed scalar/complex typing unless there
is a concrete need later.

## Push Chips

### `push`

`push(source)` reads one or both input registers and pushes values onto the
stack.

Allowed sources:

```text
t1
t2
both
```

Semantics:

```text
push(t1)   -> push t1
push(t2)   -> push t2
push(both) -> push t1, then push t2
```

`push(p1)` and `push(p2)` are not v1 syntax. The VM has no native
`PUSH_P1/PUSH_P2` op in v1, and silently aliasing those names to `t1/t2` would
produce wrong programs after `p1/p2` have been overwritten. If a program needs
to bridge current `p1/p2` register values onto the stack, use a `legacy(...)`
chip with an explicit `src=p1|p2` and `tgt=push1` where that legacy function
supports the mode.

The `both` order should be fixed and documented. The proposed order is `t1`
then `t2`, so `t2` ends up on top of the stack.

Worked example:

```text
push(both); add == push(t1); push(t2); add == t1 + t2
```

Reading order is left-to-right; the right input is on top.

## Emit Chips

### `emit`

`emit(target)` pops one complex value and writes it to an output register.

Allowed targets:

```text
p1
p2
```

`emit(t1)` and `emit(t2)` are invalid. `t1/t2` are read-only inputs; only
`p1/p2` are writable output registers.

Semantics:

```text
emit(p1) -> p1 = pop()
emit(p2) -> p2 = pop()
```

The program is valid even if it emits only one output. The other output keeps
its initialized identity value.

The program is valid if it leaves extra stack values at the end only if we
explicitly choose permissive semantics. Recommended v1 rule: final stack must be
empty. This catches accidental half-written programs early.

## Transform Chips

Some existing param transforms are naturally register-oriented and should remain
available as direct chips.

Examples:

```text
unit_circle(target=t1|t2|both)
square(target=t1|t2|both)
cube(target=t1|t2|both)
reciprocal(target=t1|t2|both)
conjugate(target=t1|t2|both)
negate(target=t1|t2|both)
exp(target=t1|t2|both)
```

In param-program form these can be modeled as stack macros:

```text
unit_circle(t1)   == push(t1); unit_circle; emit(p1)
unit_circle(t2)   == push(t2); unit_circle; emit(p2)
unit_circle(both) == push(t1); unit_circle; emit(p1);
                     push(t2); unit_circle; emit(p2)
```

However, for the user-facing editor it may be cleaner to keep these as
high-level chips with a target dropdown. The compiler can lower them to a pure
stack program.

## Stack Transform Chips

The VM should also support stack-native complex operations:

```text
const(re, im)
add
subtract
mul
div
ratio
pow
exp
log
sin
cos
conj
abs
phase
unit_circle
square
cube
reciprocal
duplicate
swap
pop
flush
```

V1 should start smaller:

```text
const
push
emit
duplicate
swap
pop
flush
add
subtract
mul
div
pow
exp
conj
unit_circle
square
cube
reciprocal
negate
```

Do not ship every mathematical function at once unless the native evaluator and
tests are ready.

Stack utility semantics:

```text
pop   -> remove one stack item
flush -> empty the whole stack
```

`flush` does not reset `p1` or `p2`. It only clears temporary stack values. It
is a shorthand for discarding all scratch values inside longer programs.

## Macro Chips

### `macro`

`macro(name)` expands a named param program from the program catalogue.

This is required for usability. Rich param programs will get long, and users
need a way to package common constructions without hiding them permanently in
native opaque chips.

Semantics:

```text
macro("spiral_fold") -> inline the saved program named spiral_fold
```

Compile-time rules:

- macros are expanded before native execution
- expansion is recursive only up to a fixed depth limit
- cyclic macro references are compile errors
- missing macro names are compile errors
- expanded programs are validated after expansion
- cache keys use the fully-expanded canonical execution program

Saved catalogue programs should live in S3 under a param-program-specific key
space, separate from Solve Score saved programs. Every saved item must include
`program_kind: "param_program"` so shared UI/storage tooling can disambiguate
chip names such as `emit`.

Macro expansion is snapshot-based for compute artifacts. A compute run expands
macros during planning and stores the expanded compiled program in `calc.json`.
Later render/preview paths for that computed job use the stored compiled
snapshot and do not re-read the macro from S3.

Live editor preview is different: it expands `macro(name)` against the current
S3 catalogue body at preview time. Editing a macro can therefore change live
preview output without changing already-computed artifacts.

Macro chips are source-level readability tools. They should not survive into the
native token stream.

## Existing Coordinate/Shape Chips

Existing shape chips such as `crd`, `rect`, `rrect`, `spdl`, `hrt`, and related
curve/shape transforms need a careful split.

Some are naturally parameter mappers over one input. These can become
target-aware macros:

```text
crd(target=t1|t2|both, ...)
```

Some operate on the pair `(t1, t2)` together. Those should either:

- remain pair-level chips that write `p1` and `p2`, or
- lower to a stack operation that pops two complex values and pushes two complex
  values.

The design should explicitly mark every chip as one of:

```text
read-only input macro
single-value stack op
two-value stack op
pair-level p1/p2 writer
```

That classification avoids ambiguous stack effects.

## Chip Effect Table

Every chip needs an explicit stack/register effect. This table is part of the
compiler contract, not just documentation.

```text
const(re, im)       pops 0, pushes 1, writes none
push(t1)            pops 0, pushes 1, writes none
push(t2)            pops 0, pushes 1, writes none
push(both)          pops 0, pushes 2, writes none
macro(name)         source-only; expands before validation

emit(p1)            pops 1, pushes 0, writes p1
emit(p2)            pops 1, pushes 0, writes p2

duplicate           pops 1, pushes 2, writes none
swap                pops 2, pushes 2, writes none
pop                 pops 1, pushes 0, writes none
flush               pops all, pushes 0, writes none

add                 pops 2, pushes 1, writes none
subtract            pops 2, pushes 1, writes none
mul                 pops 2, pushes 1, writes none
div                 pops 2, pushes 1, writes none
ratio               pops 2, pushes 1, writes none
pow                 pops 2, pushes 1, writes none

exp                 pops 1, pushes 1, writes none
log                 pops 1, pushes 1, writes none
sin                 pops 1, pushes 1, writes none
cos                 pops 1, pushes 1, writes none
conj                pops 1, pushes 1, writes none
negate              pops 1, pushes 1, writes none
reciprocal          pops 1, pushes 1, writes none
unit_circle         pops 1, pushes 1, writes none
square              pops 1, pushes 1, writes none
cube                pops 1, pushes 1, writes none
```

Target-aware high-level chips are source-level macros. For example:

```text
unit_circle(target=both)
```

lowers to:

```text
push(t1); unit_circle; emit(p1);
push(t2); unit_circle; emit(p2)
```

Pair-level legacy transforms that cannot be lowered cleanly should remain
opaque in v1 or be deferred until they have explicit stack/register effects.

## Legacy Register Ops

Existing param-transform pipelines should translate directly into register ops
without using the stack.

Source form:

```text
legacy(name, src, tgt, args...)
```

`src` describes which current parameter registers the legacy function reads.
`tgt` describes where the result is written.

Allowed source selectors:

```text
p1
p2
both
pop1
pop2
```

Allowed target selectors:

```text
p1
p2
both
push1
push2
```

Legacy `src` and `tgt` selectors are deliberately named in the `p1/p2` register
space, not the immutable `t1/t2` input space. `legacy(..., src=t1, ...)` and
`legacy(..., tgt=t2, ...)` are invalid; use `push(t1)` / `push(t2)` if the
program needs the original inputs on the stack.

Use `both`, not `p1+p2`, in the serialized form. `p1+p2` looks like arithmetic
addition and will confuse both users and implementers.

Register-only legacy translation:

```text
old: [["unit_circle"], ["square"]]
new: legacy(unit_circle, src=both, tgt=both);
     legacy(square, src=both, tgt=both)
```

This preserves the current in-place pipeline semantics:

```text
p1 = t1
p2 = t2
legacy chip reads p1/p2
legacy chip writes p1/p2
next legacy chip reads the updated p1/p2
```

Stack bridge examples:

```text
legacy(crd, src=pop1, tgt=push1, size=1)
legacy(rtheta, src=both, tgt=push2, p=1)
push(t1); push(t2); legacy(rtheta, src=pop2, tgt=push2, p=1)
    == rtheta(t1, t2, p=1)
```

When `tgt=push2`, output 1 is pushed first and output 2 is pushed second, so
output 2 ends on top. To write those two pushed values back to registers in
natural order:

```text
legacy(rtheta, src=both, tgt=push2, p=1); emit(p2); emit(p1)
```

Each legacy chip must declare which source and target selectors are legal. Do
not allow every selector for every chip unless the semantics are real.

Native execution still uses only pre-resolved function indices. A legacy token
does not carry the string `name` in the hot loop.

### Legacy Registry

Legacy transform support needs a single source of truth.

Registry file:

```text
lambda/param_legacy_registry.json
```

The Python compiler reads this registry for validation and lowering. The C
function-pointer table is generated from it or verified against it by tests.
Do not maintain independent Python and C legacy mappings by hand.

Registry schema:

```json
{
  "version": 1,
  "functions": [
    {
      "name": "unit_circle",
      "fn_index": 1,
      "kind": "unary",
      "allowed_src": ["p1", "p2", "both", "pop1"],
      "allowed_tgt": ["p1", "p2", "both", "push1"],
      "args": []
    },
    {
      "name": "rtheta",
      "fn_index": 2,
      "kind": "pair",
      "allowed_src": ["both", "pop2"],
      "allowed_tgt": ["both", "push2"],
      "args": [
        {"name": "p", "default": 1.0}
      ]
    }
  ]
}
```

Rules:

- `fn_index` values are stable once shipped
- no duplicate `fn_index`
- no duplicate `name`
- argument defaults are applied at compile time
- unsupported `src`/`tgt` combinations are compile errors
- Python tests assert registry validity
- Docker runtime tests assert C table and registry stay in sync

Cross-register writes are allowed only for unary/single-output legacy functions.
For example:

```text
legacy(unit_circle, src=p1, tgt=p2)
```

is valid and writes the unary result from `p1` into `p2`.

Pair-level transforms such as `rtheta`, `roots2`, `sum_prod`, and shape
transforms that need both inputs must use pair-compatible modes such as
`both -> both` or `pop2 -> push2`.

## Use Cases

The feature is justified by programs that are visible to the user and not
comfortably expressible as a fixed sequence of opaque transform chips.

Examples:

```text
# Swap the parameter axes.
push(t1); emit(p2);
push(t2); emit(p1)

# Sum/difference reparameterization.
push(t1); push(t2); add; emit(p1);
push(t1); push(t2); subtract; emit(p2)

# Use t2 to modulate a transform of t1.
push(t1); unit_circle;
push(t2); const(0, 6.283185307179586); mul; exp;
mul; emit(p1)

# Reuse a named construction, then override only p2.
macro("spiral_fold");
push(t2); square; emit(p2)
```

These are the kinds of constructions that should become inspectable programs
rather than more special-purpose native chips.

## Compatibility

Existing param transform chains must continue to work.

Compatibility path:

1. Keep accepting the current serialized param-transform array.
2. Convert each old transform into an equivalent param program internally.
3. Save new programs in the param-program format.
4. Optionally keep exporting old-form transforms only for old saved jobs.

An empty old transform chain maps to an empty param program:

```text
p1 = t1
p2 = t2
```

Targetable old transforms map naturally:

```text
["unit_circle", "both"]
```

becomes either a high-level param-program chip:

```text
unit_circle(target=both)
```

or the lowered stack form:

```text
push(t1); unit_circle; emit(p1);
push(t2); unit_circle; emit(p2)
```

## Validation

Validation should happen in two layers.

### Editor Validation

The editor should allow temporary invalid programs while the user is composing.

It should show diagnostics such as:

```text
valid here
will underflow here
program leaves stack depth 1
p1 not emitted; identity p1 will be used
p2 not emitted; identity p2 will be used
```

Do not force users into sequencing puzzles while editing.

### Execution Validation

Execution should be strict.

Recommended v1 rules:

- every chip name must be known
- every numeric parameter must be finite
- macro expansion must succeed without cycles
- stack underflow is an error
- `emit(p1)` and `emit(p2)` require one complex stack value
- final stack must be empty
- it is allowed to emit only `p1` or only `p2`; the other register remains
  identity

If we later want scratch/debug stack values to remain, that can be a deliberate
`allow_trailing_stack` mode, not the default.

## Native Execution

This should be a new native evaluator used by param generation.

Do not route it through the Solve Score evaluator. The value type and outputs
are different.

Native contract per row:

```c
ParamProgramInputs:
    complex t1;
    complex t2;

ParamProgramOutputs:
    complex p1;
    complex p2;
```

Execution:

```text
p1 = t1
p2 = t2
stack = []
for chip in program:
    execute chip
return p1, p2
```

The stack is per row and starts empty for every row. No stack state carries from
one parameter row to the next.

Repeated calls with the same `t1`, `t2` are valid. This is the expected shape
for dithered parameter generation. Any dithering schedule, random seed, or
sample accumulation belongs to the caller, not to hidden mutable state inside
the param-program evaluator.

The returned `p1`, `p2` then flow into the existing coefficient function path.

### Native Evaluator Shape

Performance is a first-class requirement. Param generation runs once per solve
row, so an N=5000 job means 25M VM evaluations.

The compiler should do as much work as possible before row execution:

- expand macros
- lower high-level target chips to primitive ops
- resolve chip names to enum opcodes
- parse numeric constants into native doubles
- precompute constant-only subexpressions that are safe to fold
- compute max stack depth
- reject impossible stack effects before entering native code

Native execution should use a compact token array, similar in shape to the Solve
Score evaluator pattern:

```c
enum ParamProgramOp {
    PARAM_OP_PUSH_T1,
    PARAM_OP_PUSH_T2,
    PARAM_OP_EMIT_P1,
    PARAM_OP_EMIT_P2,
    PARAM_OP_CONST,
    PARAM_OP_ADD,
    ...
};
```

There must be no string lookup in the hot evaluator path. String chip names,
macro names, and legacy transform names exist only in source JSON and compiler
diagnostics. The compiled program passed to native execution is a vector of
integer opcodes and indices into pre-built function-pointer tables.

Dispatch shape:

```text
source names
-> compile-time catalogue lookup
-> integer opcode / function index
-> native token vector
-> row evaluator dispatches by opcode and calls function_table[index]
```

For legacy transform chips, compile `legacy(name, src, tgt, args...)` into:

```c
typedef struct {
    uint16_t op;        /* PARAM_OP_LEGACY_UNARY, PARAM_OP_LEGACY_PAIR, etc. */
    uint16_t fn_index;  /* index into immutable legacy function table */
    uint8_t src;
    uint8_t tgt;
    uint8_t n_args;
    double args[PARAM_PROGRAM_MAX_ARGS];
} ParamProgramToken;
```

The row loop must not call `strcmp`, parse numbers, resolve macro names, allocate
memory, or inspect source JSON. If an operation cannot be represented as a
pre-resolved opcode/function index, it is not compiled.

Recommended initial limits:

```text
MAX_PROGRAM_TOKENS = 64 after macro expansion
MAX_STACK          = 16 complex values
MAX_MACRO_DEPTH    = 8
```

These can change after measurement, but they should be bounded so the fused
multi-threaded pipeline can allocate predictable per-thread working memory.

Performance target: a typical compiled param program should add no more than
20% to param generation wall time compared with the current targetable-transform
path for large grids. If this cannot be met, keep a fast path for chains that
lower exactly to the existing transform pipeline.

### Benchmark Plan

We need to measure the penalty of the stack-machine evaluator against the
current pipeline evaluator before committing to it as the default path.

Use two benchmark layers.

Native microbenchmark:

- run the current transform pipeline evaluator over an in-memory grid
- run the equivalent lowered param-program token stream over the same grid
- exclude S3, file I/O, coefficient generation, root solving, and rasterization
- measure rows/sec, ns/evaluation, ns/token, max stack depth, and branch counts
- run identity, one unary transform, three chained unary transforms, arithmetic
  cross-coupling, and macro-expanded programs

End-to-end param-gen benchmark:

- run the existing param generation path for representative grids
- run the param-program path with equivalent programs
- include dither repetitions, because dithering means repeated calls with the
  same `t1`, `t2`
- measure wall time, CPU time, rows/sec, thread count, and output byte size
- keep coefficient generation and solve disabled or separately timed so the
  param-program cost is not hidden by later stages

Recommended benchmark matrix:

```text
N=100, 256, 1000, 5000
threads=1, 2, 4, 8
dither_calls=1, 2, 4
program=identity
program=unit_circle(both)
program=unit_circle; square; reciprocal
program=sum_difference
program=t2_modulates_t1
program=macro-expanded equivalent of one of the above
```

Penalty formula:

```text
penalty_pct = 100 * (param_program_wall - pipeline_wall) / pipeline_wall
```

For microbenchmarks also report:

```text
ns_per_eval = elapsed_ns / rows
ns_per_token = elapsed_ns / (rows * executed_token_count)
```

The useful decision point is not whether the stack VM is slower. It will be
slower than hand-coded transforms. The useful question is whether the slowdown is
small enough for the expressivity gain, and whether equivalent legacy chains can
be detected and kept on the old fast path.

Acceptance target:

- equivalent targetable chains should be within 20% of the current pipeline, or
  keep the old fast path
- expressive programs that cannot lower to the old pipeline should report
  rows/sec clearly in logs so users understand the cost
- multi-thread scaling should remain close to the current fused path; a large
  drop suggests shared state, cache contention, or avoidable allocation in the
  hot loop

Benchmark hygiene:

- run each case multiple times and report median plus min/max
- warm up before timing
- allocate stacks per thread before timing
- use the same grid ordering and serpentine behavior as production
- verify output equivalence for lowered legacy programs before comparing speed
- test with compiler optimizations matching deployed binaries
- include one large-N run where memory/cache effects are visible

First local benchmark:

These numbers came from the temporary `param_program_bench` mode in
`lambda/sweep_cli.c`, built locally with:

```bash
cc -O3 -pthread -o lambda/sweep_test lambda/sweep_cli.c -lm
```

The benchmark compares the existing `dispatchPt` param-transform path with a
small compiled-token VM over the same serpentine param-gen traversal. It excludes
S3, coefficient generation, solve, rasterization, and deployed Lambda packaging.
This is an architecture signal, not deploy verification.

```text
N=1000, times=1, reps=5
case                pipeline median   VM median   penalty    checksum
identity            5.667 ms          5.540 ms    -2.24%     match
unit_circle_both    74.184 ms         43.645 ms   -41.17%    match
unit_circle_square  163.523 ms        53.620 ms   -67.21%    match
sum_difference      72.913 ms         10.588 ms   -85.48%    match
t2_modulates_t1     n/a               29.462 ms   n/a        VM-only
```

```text
N=2000, times=1, reps=3
case                pipeline median   VM median   penalty    checksum
identity            22.659 ms         20.944 ms   -7.57%     match
unit_circle_both    296.924 ms        175.015 ms  -41.06%    match
unit_circle_square  638.664 ms        214.760 ms  -66.37%    match
sum_difference      294.460 ms        42.473 ms   -85.58%    match
t2_modulates_t1     n/a               119.380 ms  n/a        VM-only
```

```text
N=5000, times=1, reps=1
case                pipeline time     VM time      penalty    checksum
identity            135.145 ms        124.761 ms  -7.68%     match
unit_circle_both    1.861 s           1.093 s     -41.29%    match
unit_circle_square  3.967 s           1.345 s     -66.10%    match
sum_difference      1.843 s           265.482 ms  -85.60%    match
t2_modulates_t1     n/a               746.555 ms  n/a        VM-only
```

Initial conclusion: the compiled-token VM shape is not the performance
showstopper. In these local runs it is faster than the current legacy-equivalent
pipeline because the current `dispatchPt` path still performs string dispatch in
the row loop.

The "legacy fast path" name should be understood as a compatibility/parity path,
not a proven performance path. Existing old-style transform chains can still be
routed through `dispatchPt` to preserve deployed behavior and minimize rollout
risk. Before flipping legacy-equivalent programs to VM-by-default, benchmark
`PARAM_OP_LEGACY` dispatch directly with threaded and `times > 1` cases; the
microbench above did not measure that exact path.

### Thread Safety

The fused calculation pipeline is multi-threaded, so the evaluator must be
thread-safe.

Required constraints:

- compiled program objects are immutable after construction
- no global mutable evaluator state
- each worker thread owns its stack and output registers
- macro catalogue lookup happens before native execution, not during per-row
  evaluation
- any shared compiled-program cache is read-only during evaluation or protected
  by normal synchronization outside the hot row loop

The hot evaluator function should accept a compiled program pointer plus row
inputs, and write only to caller-owned outputs.

## UI

The UI should mirror the Solve Score program setup:

- chip strip
- add-chip popup grouped by category
- line selection
- `+ before` and `+ after`
- read-only saved program preview
- saved param programs
- compile diagnostics

Unlike Solve Score, there is no color interpretation. The output contract is
always exactly `p1`, `p2`.

Suggested chip categories:

- Input: `push(t1)`, `push(t2)`, `push(both)`
- Output: `emit(p1)`, `emit(p2)`
- Constants: `const(re, im)`
- Stack: `duplicate`, `swap`, `pop`, `flush`
- Complex arithmetic: `add`, `subtract`, `mul`, `div`, `pow`
- Unary complex: `conj`, `negate`, `reciprocal`, `exp`
- Macros: `macro(name)`
- Native coordinate transforms: `unit_circle`, `square`, `cube`
- Legacy bridge transforms: `legacy(crd, ...)`, `legacy(rect, ...)`,
  `legacy(rtheta, ...)`, and other existing shape/pair transforms

## Serialization

Use a versioned saved format.

Example:

```json
{
  "version": 1,
  "name": "unit circle then swirl",
  "program_kind": "param_program",
  "chain": [
    ["push", "t1"],
    ["unit_circle"],
    ["emit", "p1"],
    ["push", "t2"],
    ["square"],
    ["emit", "p2"]
  ]
}
```

High-level macro chips can be saved directly if we want editor readability:

```json
{
  "version": 1,
  "program_kind": "param_program",
  "chain": [
    ["unit_circle", "both"],
    ["crd", "t1", "2", "1", "0"]
  ]
}
```

The compiler should produce a canonical lowered execution form for cache keys.
The UI can preserve the higher-level source form for readability.

Macro example:

```json
{
  "version": 1,
  "program_kind": "param_program",
  "chain": [
    ["macro", "spiral_fold"],
    ["push", "t1"],
    ["unit_circle"],
    ["emit", "p1"]
  ]
}
```

## Cache Keys

The render/compute cache key must use the effective execution program, not just
the displayed source chips.

This means:

- two equivalent old/new representations should collide intentionally if they
  lower to the same execution program
- a different program must change the compute artifact key
- program version must be included in the hash
- macro names alone are not sufficient for the hash; the expanded canonical
  program must be hashed

Canonicalization rule for v1:

```text
source chain
-> expand macro(name)
-> lower target-aware source chips
-> parse constants
-> opcode token stream
-> hash opcode token stream + version
```

Pair-level legacy chips that cannot be lowered remain explicit canonical tokens.
They should not be silently rewritten unless the rewrite is exactly equivalent.

## Concrete Implementation Plan

This should land in stages. Do not try to replace the whole param-transform
pipeline in one commit.

### Stage 0: Benchmark Spike

Goal: prove the VM overhead before wiring it into compute.

Files:

- `lambda/param_program.h`: new native VM header with op enum, token structs,
  compiler-facing limits, and `param_program_eval`.
- `lambda/param_program_bench.c`: new benchmark binary or temporary test binary
  that compares current pipeline dispatch against equivalent VM token streams.
- `tests/docker_runtime_regression.py`: add a runtime benchmark/sanity case that
  runs inside the Lambda-like container.

Work:

- Implement only identity, `push`, `emit`, `const`, `add`, `mul`,
  `unit_circle`, `square`, `pop`, and `flush`.
- Benchmark equivalent programs against current `dispatchPt` for N=100,
  N=1000, and at least one large-N case.
- Measure single-thread and multi-thread evaluator calls with per-thread stacks.
- Do not expose this in UI yet.

Exit criteria:

- VM output matches current pipeline for identity and lowered `unit_circle(both)`.
- Benchmark reports penalty percentage and rows/sec.
- If equivalent lowered chains are slower than the 20% target, keep a fast-path
  requirement in the later stages.

### Stage 1: Python Compiler And Macro Expansion

Goal: compile readable source chains into a canonical execution program before
any native binary is called.

Files:

- `lambda/param_program_chain.py`: new shared compiler module.
- `tests/test_param_program_chain.py`: compiler, macro expansion, canonical hash,
  stack diagnostics, and legacy lowering tests.
- `lambda/handler_storage.py`: param-program save/list/fetch/delete route
  helpers, mirroring the Solve Score program storage pattern.
- `tests/test_param_program_storage.py`: S3 saved-program CRUD and validation.

Compiler outputs:

```json
{
  "program_kind": "param_program",
  "source_chain": [["unit_circle", "both"]],
  "execution_tokens": [["push_t1"], ["unit_circle"], ["emit_p1"], ["push_t2"], ["unit_circle"], ["emit_p2"]],
  "execution_spec": "push_t1;unit_circle;emit_p1;push_t2;unit_circle;emit_p2",
  "fingerprint": "sha1...",
  "display": "unit_circle(both)",
  "stack_max": 1,
  "emits": ["p1", "p2"],
  "uses_legacy_fast_path": true
}
```

Work:

- Accept both old `param_transforms` and new `param_program_chain`.
- Lower target-aware source chips to primitive stack tokens.
- Expand `macro(name)` from S3-backed saved param programs before hashing.
- Reject macro cycles, missing macros, over-depth expansion, non-finite
  constants, and stack underflow.
- Produce diagnostics suitable for a permissive editor, plus strict execution
  errors for compute.
- Make `program_kind: "param_program"` mandatory for saved param programs.

Storage:

- Add `PARAM_PROGRAMS_PREFIX = "polypaint/param-programs/"`.
- Add routes `/list-param-programs`, `/fetch-param-program`, `/save-param-program`,
  and `/delete-param-program`.
- Update `deploy.sh` and `api_manifest.json` for the new storage routes.
- Keep this storage separate from `polypaint/solve-score-programs/`.

Exit criteria:

- Two equivalent chains hash the same after lowering.
- Macro-expanded hash changes when the referenced macro body changes.
- Old param transform arrays still compile.
- No native code depends on S3 or macro lookup.

### Stage 2: Native VM In `sweep_coeffgen`

Goal: make `sweep_coeffgen` accept compiled param-program tokens while keeping
the old `param_transforms` path alive.

Files:

- `lambda/param_program.h`: real VM implementation.
- `lambda/sweep_cli.c`: include the VM, parse compiled token payload, and route
  param generation through either VM or legacy dispatch.
- `lambda/sweep_coeffgen`: rebuilt deploy artifact.
- `tests/test_param_dump.py`: add equivalence tests using compiled param-program
  payloads.
- `tests/docker_runtime_regression.py`: add VM param-gen runtime parity and
  threaded parity tests.

Native input contract:

```json
{
  "mode": "param_gen",
  "param_program": {
    "version": 1,
    "tokens": [...],
    "fingerprint": "sha1...",
    "stack_max": 4
  }
}
```

Work:

- Add parser for compiled `param_program` token payload.
- In `runParamDump`, `runCoeffGen`, `runParamGen`, and degree probe logic, apply
  `param_program` if present, otherwise use `param_transforms`.
- Keep `parsePtChain` and `dispatchPt` for backward compatibility and fast path.
- Ensure `runParamGenThreadedRange` allocates one stack per worker thread.
- Emit metadata fields such as `param_program_tokens`, `param_program_stack_max`,
  `param_program_fingerprint`, `uses_legacy_fast_path`, and `elapsed_us`.
- Do not do macro lookup, S3 reads, or dynamic allocation in the row hot loop.

Exit criteria:

- `param_dump` output matches legacy output for lowered target-aware transforms.
- Single-thread and multi-thread output are byte-identical for deterministic
  programs.
- Dither `times > 1` still means repeated calls with the same `t1`, `t2`.
- Docker runtime test runs the rebuilt `lambda/sweep_coeffgen`, not stale source.

### Stage 3: Compute Backend Wiring

Goal: pass compiled param programs through classic, fused, lores, preview, and
debug paths.

Files:

- `lambda/handler_compute_orchestrator.py`: preserve/pass both legacy and new
  fields only. Do not import the param-program compiler here.
- `lambda/handler_compute_plan.py`: compile param program during plan build and
  store compiler output under `plan.pipeline.param_program`.
- `lambda/compute_fused.py`: include the canonical param-program fingerprint in
  `build_probe_signature`.
- `lambda/handler_coeffgen.py`: compile or validate `param_program` for
  `param_gen`, `degree_probe`, and `legacy_coeffgen`.
- `lambda/handler_compute_chunk_fused.py`: forward `param_program` into local
  `sweep_coeffgen` calls.
- `lambda/handler_compute_preview.py`: accept new param-program payloads for
  synchronous preview.
- `lambda/handler_param_debug.py`: accept new param-program payloads for
  transformed-parameter visualization.
- `lambda/handler_render_lores_preview.py`: recompute preview uses calc pipeline;
  load and forward `param_program` from `calc.json` when present.
- `stepfunctions/compute_workflow.asl.json.template`: thread
  `param_program.$` anywhere `param_transforms.$` is currently threaded.
- `tests/test_compute_workflow_definition.py`: assert the new ASL selectors.

Work:

- The plan should preserve `param_transforms` for legacy display and add
  `param_program` for execution.
- Degree probe signature must use the canonical execution fingerprint, not the
  UI source chain alone.
- Fused chunk S3 reuse metadata should include the param-program fingerprint so
  stale params are not reused after a program change.
- `calc.json` should store `pipeline.param_program`, `pipeline.param_program_display`,
  and `pipeline.param_program_fingerprint`.
- All error messages must include phase and chain/program context.

Exit criteria:

- Classic compute, fused compute, lores compute, recompute preview, compute
  preview, and param debug all choose the same effective param program.
- Old `calc.json` files with only `param_transforms` still load.
- New `calc.json` files are self-contained for render/preview without needing
  live S3 macro lookup.

### Stage 4: Frontend Param Program Editor

Goal: replace the rough param-transform strip with a Solve-Score-style program
editor while preserving old chains.

Files:

- `index.html`: new param-program catalogue, editor state, compiler diagnostics,
  macro picker, save/load modal, insert before/after, move arrows, and read-only
  chip previews.
- `tests/test_frontend_js.sh`: assert UI controls, route calls, serialization
  shape, macro chips, insert controls, and stale error text.
- `tests/e2e/...`: add Playwright coverage for editing, saving, loading, and
  running a simple param program.

Work:

- Add `_paramProgramChain` alongside or replacing `_ptChain`.
- Add `_serializeParamProgram()` and keep `_serializeParamTransforms()` for
  legacy compatibility during the transition.
- Extend `addChip` insertion logic so `pt` gets selected-line `+ before` and
  `+ after`, not append-only behavior.
- Add a param-program Add Chip popup with categories: input, output, stack,
  arithmetic, unary, target macros, macros, legacy opaque.
- Add UI-side diagnostics that mirror Python compiler rules but do not block
  temporary invalid editing states.
- Add saved program modal equivalent to the improved Solve Scores modal:
  stable height, saved name/time, non-editable chip preview, no debug JSON-first
  display.
- Add `macro(name)` chooser populated from `/list-param-programs`.

Exit criteria:

- The compute payload includes `param_program_chain` for new programs.
- Existing old transform chips still serialize as `param_transforms`.
- UI makes invalid program state visible before Execute, but does not force
  sequencing puzzles during editing.
- Param transform controls remain visually stable and follow `ui_docs/style_guide.md`.

### Stage 5: Legacy And Fast-Path Strategy

Goal: keep old behavior reliable and avoid paying VM overhead when unnecessary.

Files:

- `lambda/param_program_chain.py`: mark programs that lower exactly to legacy
  pipeline transforms.
- `lambda/handler_compute_plan.py`: choose execution mode metadata.
- `lambda/sweep_cli.c`: keep legacy `dispatchPt` execution path.
- `tests/test_param_program_chain.py`: fast-path eligibility tests.
- `tests/docker_runtime_regression.py`: legacy fast-path and VM output parity.

Rules:

- Old payload with only `param_transforms` stays on legacy path by default.
- New param-program payload uses VM unless compiler proves exact legacy
  equivalence and marks `uses_legacy_fast_path`.
- Macro-expanded programs can still be fast-path eligible if their fully
  expanded form lowers exactly to legacy transforms.
- Pair-level legacy chips stay opaque until each has documented stack/register
  effects.

Exit criteria:

- Existing jobs do not get slower just because the feature exists.
- Equivalent targetable transform programs can reuse the old pipeline when
  marked safe.

### Stage 6: Packaging, Routes, And Permissions

Goal: make deployed Lambda zips and routes match local imports and API calls.

Files:

- `deploy.sh`: package `param_program_chain.py` with every handler that imports
  it, package rebuilt `sweep_coeffgen`, add storage routes and config keys.
- `api_manifest.json`: add `/list-param-programs`, `/fetch-param-program`,
  `/save-param-program`, and `/delete-param-program`.
- `tests/test_deploy_packaging.py`: assert new helper packaging and route/config
  wiring.
- `tests/test_api_route_contracts.py`: assert route presence if applicable.

Required packaging consumers:

- `handler_storage.py`
- `handler_compute_plan.py`
- `handler_coeffgen.py`
- `handler_compute_chunk_fused.py`
- `handler_compute_preview.py`
- `handler_param_debug.py`
- `handler_render_lores_preview.py`

Exit criteria:

- `python3 api_manifest.py --check` passes.
- `tests/test_deploy_packaging.py` knows every handler that imports the new
  helper.
- API Gateway route, config generation, and storage handler routes stay in sync.

### Stage 7: Test Gates

Because this touches native binaries, frontend behavior, storage routes,
workflow contracts, and packaging, the full feature is not ready without these
gates.

Minimum targeted gates during development:

```bash
git diff --check
python3 api_manifest.py --check
bash tests/test_frontend_js.sh
python3 -m py_compile lambda/param_program_chain.py lambda/handler_compute_plan.py lambda/handler_coeffgen.py lambda/handler_compute_chunk_fused.py lambda/handler_storage.py
uv run python -m pytest tests/test_param_program_chain.py tests/test_param_program_storage.py tests/test_compute_workflow_definition.py tests/test_coeffgen_param_gen.py tests/test_param_dump.py tests/test_deploy_packaging.py -q
```

Native/runtime gates after changing `lambda/sweep_cli.c` or
`lambda/param_program.h`:

```bash
# Rebuild the deployed binary first.
bash scripts/test-docker-runtime.sh
```

Predeploy gate before calling the implementation deploy-ready:

```bash
bash scripts/predeploy_check.sh
```

In this repo, `uv`, Playwright, `scripts/predeploy_check.sh`, and
`scripts/test-docker-runtime.sh` need immediate escalation in the local Codex
environment. Do not run stale binaries through Docker and call that verified.

## V1 Build Contract

This section freezes the implementation contract for v1. Anything not listed
here is v2 unless explicitly pulled forward.

### V1 Chip Set

Stack/input/output chips:

```text
push(source=t1|t2|both)
emit(target=p1|p2)
const(re, im)
duplicate
swap
pop
flush
macro(name)
```

Arithmetic chips:

```text
add
subtract
mul
ratio
```

Accepted parser aliases for imported/manual JSON: `sub` -> `subtract`, `div`
-> `ratio`. The editor should display only the canonical names above.

Unary complex chips:

```text
negate
conj
reciprocal
unit_circle
square
cube
exp
```

Legacy bridge chip:

```text
legacy(name, src, tgt, args...)
```

Deferred from v1:

```text
pow
log
sin
cos
abs
phase
pair-level editable chips without legacy wrapper
random/noise chips
general typed scalar/complex stack values
```

`exp` is included in v1 because complex exponential is straightforward and is
important for useful parameter programs. `pow` is deferred because branch
semantics need a separate policy.

### V1 Selector Semantics

Register selectors:

```text
p1   -> current p1 register
p2   -> current p2 register
both -> p1 followed by p2
```

Stack source selectors:

```text
pop1 -> pop one stack value and use it as the first input
pop2 -> pop two stack values and use the second-popped value as input 1 and the
        first-popped value as input 2
```

Stack target selectors:

```text
push1 -> push first output only
push2 -> push first output, then second output
```

`legacy(name, src=both, tgt=both)` is the direct old-pipeline translation. It
reads current `p1,p2`, applies the legacy function, and writes both outputs back
to `p1,p2`.

`legacy(name, src=p1, tgt=p1)` reads only `p1` and writes the first legacy output
back to `p1`. `p2` is unchanged.

`legacy(name, src=p2, tgt=p2)` reads only `p2` and writes the first legacy output
back to `p2`. `p1` is unchanged.

`legacy(name, src=pop1, tgt=push1)` pops one stack value, applies the legacy
function in single-input mode, and pushes the first output.

`legacy(name, src=pop2, tgt=push2)` pops two stack values, applies the legacy
function in pair-input mode, and pushes output 1 then output 2. Output 2 is on
top after the push.

Every legacy function declares which `src` and `tgt` modes it supports. The
compiler rejects unsupported combinations. Do not emulate unsupported modes with
guesswork.

### V1 Native Token ABI

The native row evaluator receives a compiled program, not source JSON.

Required enum shapes:

```c
typedef enum {
    PARAM_OP_PUSH_T1 = 1,
    PARAM_OP_PUSH_T2 = 2,
    PARAM_OP_EMIT_P1 = 3,
    PARAM_OP_EMIT_P2 = 4,
    PARAM_OP_CONST = 5,
    PARAM_OP_DUPLICATE = 6,
    PARAM_OP_SWAP = 7,
    PARAM_OP_POP = 8,
    PARAM_OP_FLUSH = 9,
    PARAM_OP_ADD = 10,
    PARAM_OP_SUBTRACT = 11,
    PARAM_OP_MUL = 12,
    PARAM_OP_RATIO = 13,
    PARAM_OP_NEGATE = 14,
    PARAM_OP_CONJ = 15,
    PARAM_OP_RECIPROCAL = 16,
    PARAM_OP_UNIT_CIRCLE = 17,
    PARAM_OP_SQUARE = 18,
    PARAM_OP_CUBE = 19,
    PARAM_OP_EXP = 20,
    PARAM_OP_LEGACY = 21
} ParamProgramOp;

typedef enum {
    PARAM_SEL_P1 = 1,
    PARAM_SEL_P2 = 2,
    PARAM_SEL_BOTH = 3,
    PARAM_SEL_POP1 = 4,
    PARAM_SEL_POP2 = 5,
    PARAM_SEL_PUSH1 = 6,
    PARAM_SEL_PUSH2 = 7
} ParamProgramSelector;
```

Required token shape:

```c
#define PARAM_PROGRAM_MAX_ARGS 8

typedef struct {
    uint16_t op;
    uint16_t fn_index;
    uint8_t src;
    uint8_t tgt;
    uint8_t n_args;
    uint8_t reserved;
    double a;
    double b;
    double args[PARAM_PROGRAM_MAX_ARGS];
} ParamProgramToken;
```

`PARAM_OP_CONST` uses `a` as real and `b` as imaginary.

`PARAM_OP_LEGACY` uses `fn_index`, `src`, `tgt`, `n_args`, and `args`.

Non-legacy arithmetic/unary ops ignore `fn_index`, `src`, `tgt`, and `args`.

JSON token fields not provided by the compiler default to zero or an empty
array. The native parser must initialize every byte of the C token struct
deterministically before filling fields from JSON.

Native limits:

```text
MAX_PROGRAM_TOKENS = 64 after macro expansion
MAX_STACK          = 16 complex values
MAX_MACRO_DEPTH    = 8
MAX_ARGS           = 8
```

Hot-loop rule:

```text
no strcmp
no JSON parsing
no number parsing
no macro lookup
no heap allocation
```

### V1 Compiled JSON Payload

Python handlers pass this shape to native binaries:

```json
{
  "version": 1,
  "fingerprint": "sha1...",
  "display": "unit_circle(both)",
  "stack_max": 1,
  "token_count": 6,
  "uses_legacy_fast_path": false,
  "tokens": [
    {"op": 1},
    {"op": 17},
    {"op": 3},
    {"op": 2},
    {"op": 17},
    {"op": 4}
  ]
}
```

Legacy token example:

```json
{
  "op": 21,
  "fn_index": 3,
  "src": 3,
  "tgt": 3,
  "args": [1.0]
}
```

The native parser may accept this JSON shape initially. It must convert it to
the C token array once per process invocation before row evaluation begins.

Use `uses_legacy_fast_path` as the single flag name everywhere. Do not introduce
parallel names such as `param_program_fast_path`.

Display string rule:

```text
display := chip("; " chip)*
chip    := name | name(arg, ...)
```

Examples:

```text
[["unit_circle", "both"]] -> unit_circle(both)
[["push", "t1"], ["unit_circle"], ["emit", "p1"]] -> push(t1); unit_circle; emit(p1)
[["macro", "spiral_fold"]] -> macro(spiral_fold)
```

### V1 Storage API

Storage routes:

```text
POST /list-param-programs
POST /fetch-param-program
POST /save-param-program
POST /delete-param-program
```

S3 prefix:

```text
polypaint/param-programs/{id}.json
```

Save request:

```json
{
  "name": "spiral fold",
  "chain": [
    ["push", "t1"],
    ["unit_circle"],
    ["emit", "p1"]
  ]
}
```

Save response:

```json
{
  "program": {
    "version": 1,
    "program_kind": "param_program",
    "id": "spiral-fold",
    "name": "spiral fold",
    "chain": [...],
    "display": "push(t1); unit_circle; emit(p1)",
    "fingerprint": "sha1...",
    "statement_count": 3,
    "saved_at": "2026-04-28T00:00:00Z"
  },
  "overwritten": false
}
```

List response:

```json
{
  "programs": [
    {
      "id": "spiral-fold",
      "name": "spiral fold",
      "statement_count": 3,
      "saved_at": "2026-04-28T00:00:00Z"
    }
  ],
  "count": 1,
  "order": "saved_at_desc"
}
```

Fetch response returns the full `program`.

Delete response:

```json
{"id": "spiral-fold", "deleted": 1}
```

### V1 UI Rollout

Ship the param-program editor as the primary editor for parameter transforms,
but keep old transform-chain serialization available internally.

UI requirements:

- existing old transform chains load as translated param programs
- old chains can be displayed as readable chips
- saved param programs use the same stable-height modal discipline as Solve
  Score saved programs
- add-chip supports selected line plus `+ before` and `+ after`
- macro picker loads saved param programs
- compile diagnostics are visible but temporary invalid edits are allowed
- Execute is blocked only when strict execution validation fails

Payload rule:

- new UI sends `param_program_chain`
- compatibility payload may also include `param_transforms`
- backend plan compiler is authoritative and produces `param_program`

### V1 Program Management UI

The param-program editor needs the same management affordances as Solve Score
programs, with param-specific terminology.

Required controls:

```text
Save
Load
Export JSON
Import JSON
Delete saved program
Duplicate / Save As
```

Save:

- opens a stable-height modal
- asks for a program name
- writes to `/save-param-program`
- overwrites same slug only after confirmation
- shows saved timestamp after success
- stores the source chain, not only compiled tokens

Load:

- opens a stable-height modal with saved programs sorted by `saved_at desc`
- left side lists name, saved time, statement count
- right side shows a non-editable chip preview matching the editor rendering
- Load replaces the current editor chain after confirmation if there are
  unsaved changes

Export JSON:

- exports a JSON object containing `program_kind`, `version`, `name`, and
  `chain`
- export does not include compiled native tokens, because those are build/ABI
  dependent
- export may include `fingerprint` and `display` as informational fields only

Import JSON:

- accepts the exported JSON shape or a raw chain array
- validates `program_kind == "param_program"` when present
- loads into the editor as an unsaved draft
- does not write to S3 until the user clicks Save
- rejects Solve Score programs with a clear message

Macro catalogue:

- `macro(name)` picker is populated from `/list-param-programs`
- selecting a macro stores the stable program id, not the display name
- the chip displays the saved program name when available
- if a macro id is missing, the editor shows a diagnostic and strict execution
  fails
- a program cannot save if it directly or indirectly references itself

Visual requirements:

- modal body height is fixed; selection changes must not resize the modal
- chip previews use the same read-only chip renderer as the live editor
- details panel scrolls internally when a saved program is long
- action buttons stay aligned and do not jump as selection changes
- layout follows `ui_docs/style_guide.md`

### V1 Compatibility Rules

Old compute requests with only `param_transforms` must still work.

Old `calc.json` files with only `pipeline.param_transforms` must still render,
preview, and debug.

New `calc.json` files store both:

```json
{
  "pipeline": {
    "param_transforms": [["unit_circle", "2"]],
    "param_transforms_display": [["unit_circle", "both"]],
    "param_program_chain": [["unit_circle", "both"]],
    "param_program": {...},
    "param_program_fingerprint": "sha1..."
  }
}
```

The compiled `param_program` in `calc.json` is self-contained. Render/preview
paths must not need live S3 macro lookup for an already-computed job.

Artifact/job metadata should surface:

```text
param_program_fingerprint
param_program_display
param_program_token_count
param_program_stack_max
uses_legacy_fast_path
```

The Results detail view should show the param program display next to the
function and coefficient transforms. This is important because the feature is
intended to make parameter complexity visible, not hidden.

### V1 Benchmark Gate

Before enabling the param-program VM by default:

- run legacy pipeline versus VM equivalent for `identity`, `unit_circle(both)`,
  `unit_circle;square`, `sum_difference`, and one macro-expanded program
- run N=100, N=1000, and one large-N case
- run threads=1 and the default fused thread count
- run dither `times=1` and `times=4`
- report rows/sec and penalty percentage

Failure policy:

- if equivalent legacy chains exceed 20% penalty, keep them on the legacy fast
  path
- if expressive non-legacy programs are slower, allow them but log the VM path,
  token count, stack max, and rows/sec
- if multi-thread scaling regresses significantly, stop and fix thread-local
  stack/state before shipping

## Logging

Param generation logs should print:

```text
Param program: chips=N stack_max=M emits=p1,p2
Param program macros: expanded=K depth=D
Param program lowered: <short execution form>
```

On failure:

```text
Param program error at chip 7: emit(p2) requires stack depth 1, got 0
```

Diagnostics need chip indices because chip pipelines become hard to debug
without them.

## Opinion

This design makes sense.

The strongest part is the register model:

- `t1`, `t2` as immutable inputs keeps the base grid stable and debuggable.
- `p1`, `p2` initialized to identity makes empty/partial programs safe.
- `emit(p1)` / `emit(p2)` makes output writes explicit instead of relying on
  final stack shape.

It also maps well to the Solve Score UX without forcing the Solve Score runtime
onto a different problem.

The main risk is chip ambiguity. Some existing param transforms are
single-value operations, while others are pair-level coordinate maps. If the
program editor pretends they are all the same kind of stack op, users will get
confusing stack effects. The implementation needs an explicit chip-effect table
from day one.

The second risk is compatibility. Old param transforms are already used by
compute jobs, so v1 should compile old chains into param programs internally
rather than deleting the old format immediately.

Recommended v1:

1. Add a param-program compiler and native evaluator behind a feature flag or
   alternate UI mode.
2. Support complex-only stack values.
3. Support `push`, `emit`, constants, stack utilities, basic arithmetic, and a
   small set of existing single-value transforms.
4. Lower existing targetable transforms to param-program form.
5. Keep pair-level/shape transforms explicit until each one has a documented
   stack effect.

This would make param transforms much more powerful without making the current
workflow harder to reason about.
