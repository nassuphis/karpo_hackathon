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

The `both` order should be fixed and documented. The proposed order is `t1`
then `t2`, so `t2` ends up on top of the stack.

## Emit Chips

### `emit`

`emit(target)` pops one complex value and writes it to an output register.

Allowed targets:

```text
p1
p2
```

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
flush
```

V1 should start smaller:

```text
const
push
emit
duplicate
swap
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

The returned `p1`, `p2` then flow into the existing coefficient function path.

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
- Stack: `duplicate`, `swap`, `flush`
- Complex arithmetic: `add`, `subtract`, `mul`, `div`, `pow`
- Unary complex: `conj`, `negate`, `reciprocal`, `exp`
- Coordinate transforms: `unit_circle`, `square`, `cube`, `crd`, etc.
- Pair transforms: transforms that intentionally consume or write both
  parameters together

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

## Cache Keys

The render/compute cache key must use the effective execution program, not just
the displayed source chips.

This means:

- two equivalent old/new representations should collide intentionally if they
  lower to the same execution program
- a different program must change the compute artifact key
- program version must be included in the hash

## Logging

Param generation logs should print:

```text
Param program: chips=N stack_max=M emits=p1,p2
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
