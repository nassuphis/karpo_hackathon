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

The final parameter values for the current row are also visible:

```text
p1 = final complex parameter 1   # read-only scalar
p2 = final complex parameter 2   # read-only scalar
```

`p1` and `p2` are the final values produced by the active parameter path:

- Chain mode: legacy parameter transform chain output
- Program mode: Param Program output registers

They are the exact values passed into the coefficient function. Coeff Program
can read them as scalar registers for chip arguments, but cannot modify them.
They are not vector stack entries.

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
p1, p2 = final_param_values()          # read-only scalar registers
cf     = coefficient_function(p1, p2)  # read-only vector
poly   = copy(cf)                      # mutable output/current vector register
stack  = []

for chip in coeff_program:
    execute chip

return poly
```

`cf` never changes. `poly` is the final vector written to coeff output. Chips may
read `cf`, read/write `poly`, or consume/produce stack vectors depending on their
`src` / `tgt` selectors. Chips may also read `p1` / `p2` in scalar argument
expressions.

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

### `p1` And `p2`

`p1` and `p2` are read-only scalar complex registers.

They are not stack vectors, and `push(p1)` / `push(p2)` are not valid vector
operations. To turn a parameter value into a coefficient vector, use the
coefficient-program `push_const(length, value)` chip:

```text
push_const(poly_len, p1)   # push a vector matching current poly length, filled with p1
push_const(poly_len, p2)   # push a vector matching current poly length, filled with p2
```

`p1` / `p2` are available to scalar argument expressions in chips that accept
complex values:

```text
push_const(poly_len, p1 + p2)
legacy(linear, src=poly, tgt=poly, multiplier=p1, offset=p2)
legacy(exp, src=poly, tgt=poly, multiplier=p1, offset=p2)
legacy(pow, src=poly, tgt=poly, a=1, b=0, pr=real(p2), pi=imag(p2))
```

The compiler lowers these expressions before native execution. Native hot loops
must never parse strings to evaluate `p1` / `p2` expressions.

In Chain mode, the historical coefficient transform `exp(a,b)` remains
`exp(z * (a + i*b))`. In Coeff Program mode, `legacy(exp, src, tgt, field1,
field2)` uses the richer two-complex-field form `exp(src * field1 + field2)`.
If `field2` is zero and `field1` is static, the compiler may still lower it to
the legacy chain-equivalent form.

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

V1 ships only pinned vector operations: equal-length binary ops, equal-length
`argsort`, unary per-element ops, and fixed-length roll ops. Any vector op that
combines two vectors must reject length mismatch immediately with chip context.

The native implementation must not allocate stack vectors dynamically. The stack
is a fixed-size per-thread ring of vector slots:

```text
COEFF_PROGRAM_MAX_VECTOR_STACK = 64
COEFF_PROGRAM_MAX_VECTOR_LEN   = 256

stack_re[64][256]
stack_im[64][256]
stack_len[64]
stack_depth
stack_head
```

Push writes into the next ring slot. Pop returns the current top ring slot and
moves the head backward. The runtime tracks `stack_depth` separately so wrap
around never overwrites a live vector. Pushing when `stack_depth == 64` is a
hard error. This is still a stack semantically, but the physical storage is a
ring so no slot shifting or heap allocation happens in the row loop.

This is required because coeff programs run millions of times and the coeffgen
path is multi-threaded. Every worker thread owns its own ring workspace.

## Source And Target Selectors

Every compiled coefficient transform chip should have explicit `src` and `tgt`
selectors, in addition to its own parameters. V1 does not expose direct
legacy-transform sugar chips. Use `legacy(name, src, tgt, ...)` so the data
source, write target, and legacy function are always visible.

Allowed source selectors:

```text
poly   # read the current output/current working vector
cf     # read the immutable coefficient-function output
pop    # pop one vector from the stack and use it as input
peek   # read top of stack without popping
```

Recommended v1 source set:

```text
poly
cf
pop
peek
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

`peek` is a read-only stack source. It is included in v1 because it makes
branching from a computed intermediate practical without forcing users through
duplicate/swap/pop sequencing puzzles.

```text
peek   # read top stack vector without popping it
```

Examples:

```text
legacy(rev, src=poly, tgt=poly)
legacy(deriv, src=cf, tgt=push)
legacy(normalize, src=pop, tgt=poly)
legacy(rev, src=peek, tgt=push)
```

First-class vector ops intentionally do not read `cf` directly in v1; use
`push(cf)` or `legacy(..., src=cf, tgt=push)` when the immutable coefficient
function output should participate in a vector expression. Their source
selectors are:

```text
poly
pop
peek
```

Their target selectors are:

```text
poly
push
```

## Push And Emit

### `push_const`

`push_const(length, value)` creates a vector filled with one complex scalar
value and pushes it onto the stack. Old saved coeff programs may still use
`const(length, value)` as a compatibility alias, but new UI saves should write
`push_const`.

Source form:

```text
push_const(length, value)
```

Semantics:

```text
v = [value, value, ..., value]  # length entries
push(v)
```

Rules:

- `length` is either `poly_len` or an integer literal in
  `[1, COEFF_PROGRAM_MAX_VECTOR_LEN]`
- `poly_len` means the current length of the mutable `poly` register at that
  point in execution
- `value` is a scalar complex argument expression
- `value` may reference read-only `t1`, `t2`, `p1`, and `p2`
- non-finite evaluated values are execution errors
- the chip has no `src` or `tgt` dropdown in v1
- UI display should label this as `push_const(length, value)` to avoid
  confusing it with the coefficient function `const(degree, value)`

Examples:

```text
push_const(poly_len, 1+2i)
push_const(35, p1)
push_const(poly_len, p1 + p2)
push_const(poly_len, p1 * conj(p2))
```

To make the constant vector the final polynomial:

```text
push_const(poly_len, p1)
emit
```

### `push_linspace`

`push_linspace(length)` creates and pushes a real vector with `length` entries
linearly spaced from `0` to `length`, inclusive. For `length=1`, the only value
is `0`.

Source form:

```text
push_linspace(poly_len)
push_linspace(35)
```

Rules:

- `length` uses the same `poly_len` or integer-literal rule as `push_const`
- imaginary parts are zero
- default UI value is `poly_len`

### `push_range`

`push_range(length)` creates and pushes a Python-style real index vector with
`length` entries:

```text
0, 1, 2, ..., length-1
```

Source form:

```text
push_range(poly_len)
push_range(35)
```

Rules:

- `length` uses the same `poly_len` or integer-literal rule as `push_const`
- imaginary parts are zero
- default UI value is `poly_len`
- use `legacy(linear, pop, push, 1, 1)` after `push_range(poly_len)` when a
  formula needs one-based coefficient ordinals `1..poly_len`

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

`emit` commits output. If the stack is non-empty, it pops one vector from the
stack and stores it into `poly`. If the stack is empty, it is a no-op commit of
the current `poly`.

Semantics:

```text
emit with stack    -> poly = pop()
emit with no stack -> poly remains unchanged
```

There is only one output register, so `emit` does not need a target dropdown in
v1. If a target is kept for UI consistency, it should have only one value:
`poly`.

The program is valid if it emits zero times. In that case `poly` remains whatever
the in-place transforms made it, or identity `cf` for an empty program.

This means a legacy chip that writes `poly` directly can be followed by `emit`
as a harmless commit marker. For example, `legacy(rev, poly, poly)` and
`legacy(rev, poly, poly); emit` are equivalent because the legacy chip mutates
`poly` directly.

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
linear
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

## Coefficient Function `const`

Add a coefficient function named `const`.

This is separate from the Coeff Program `push_const(length, value)` chip, even
though the parameters intentionally match. The context disambiguates them:

```text
coefficient function: const(length, value) -> cf
coeff-program chip:   push_const(length, value) -> push vector
```

Coefficient function semantics:

```text
cf = [value, value, ..., value]  # length entries
```

Inputs:

```text
length  # integer, degree + 1
value   # complex scalar literal
```

Rules:

- `length` must be an integer in `[1, MAX_COEFFS]`
- in Program mode, `length` must also be `<= COEFF_PROGRAM_MAX_VECTOR_LEN`
- `value` accepts the same complex literal syntax used elsewhere:
  `1`, `1+2i`, `-2j+4`, `1e-6-2e3i`
- non-finite values are rejected before execution
- output coefficient order matches all existing coefficient functions
- the produced vector is read-only `cf`; Coeff Program then modifies `poly`

Primary use:

```text
coefficient function: const(length=35, value=1+0i)
coeff program:
  push_const(poly_len, p1); legacy(linear, src=pop, tgt=push, multiplier=p2, offset=0); emit
```

This gives users a simple base coefficient vector and makes the interesting
row-dependent behavior visible in the Coeff Program instead of burying it inside
an opaque coefficient function.

Implementation locations:

- add native built-in in `lambda/sweep_cli.c`
- add catalog entry in `lambda/coeff_func_catalog.json`
- ensure generated root `coeff_func_catalog_js.js` includes it
- ensure `lambda/coeff_func_lookup.h` routes it for `CoeffFuncC`
- update catalog consistency tests
- add UI parameter fields for `length` and one complex `value` field

Native parameter encoding should use the existing `CoeffFuncC` `cfpv` path:

```text
cfpv[0] = length
cfpv[1] = value_re
cfpv[2] = value_im
n_cfpv = 3
```

Do not add a special JSON payload shape just for this function.

UI display names should disambiguate the two `const` contexts:

```text
coefficient function display: coeff_const(length, value)
coeff-program chip display:   push_const(length, value)
```

The old coeff-program source opcode `const` remains accepted as an alias, but
new saves and user-facing display strings should use `push_const(...)`.

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

For `src=pop`, the andy reference is the popped vector:

```text
push(cf)
legacy(deriv, src=pop, tgt=push, andy=0.5)
legacy(rev, src=pop, tgt=poly, andy=0.5)
```

Semantics:

```text
tmp0 = pop()                         # cf
tmp1 = blend(deriv(tmp0), tmp0, 0.5)
push(tmp1)
tmp2 = pop()                         # tmp1
poly = blend(rev(tmp2), tmp2, 0.5)
```

The andy reference is always the input vector to that one legacy chip, after
`src` resolution and before the transform runs.

## Scalar Argument Expressions

Some coefficient-program chips need scalar complex arguments:

- `push_const(length, value)`
- legacy transform parameters such as `linear`, `exp`, `round`, and `pow`
- future vector arithmetic chips, if shipped

V1 should support a small compiled scalar-expression language for these
arguments. It is row-dependent because it can read the original parameter
inputs `t1`, `t2`, the final Param Program outputs `p1`, `p2`, the current
coefficient vectors, and the current vector lengths, but it is not
string-parsed in the row loop.

Allowed source syntax:

```text
complex literals:  1, -2.5, 1e-6, 1+2i, -2i+4
constants:         pi, pi2, pi2i
scalar registers:  t1, t2, p1, p2, poly_len
vector element:    cf0, cf1, poly0, poly31, tos0, tos32
unary funcs:       conj(x), neg(x), real(x), imag(x), abs(x), log(x)
binary ops:        x + y, x - y, x * y, x / y
parentheses:       (p1 + p2) * 0.5
```

Compile model:

```text
source expression -> scalar expression bytecode or folded literal
```

Native evaluation:

```text
eval_scalar_expr(expr, t1, t2, p1, p2, cf, poly, top_of_stack) -> complex scalar
```

For monolithic `coeffgen`, `t1/t2` are already available as the loop's source
coordinates. For `coeffgen_chunked`, `params.bin` stores only transformed
`p1/p2`; the handler must pass `source_step_start` plus the source grid
dimensions (`source_n1`, `source_n2`) so native code can reconstruct the
serpentine source coordinates without changing the `params.bin` record format.

Rules:

- expressions are compiled once with the coefficient program
- literals are folded at compile time
- expressions that do not reference row-dependent registers should be stored as
  constants
- row-loop evaluation walks integer expression opcodes, not strings
- `t1` and `t2` are the original serpentine scan inputs for the row, before
  Param Program or legacy parameter transforms run
- `p1` and `p2` are the final transformed parameters passed to the coefficient
  function
- `poly_len` evaluates to the current mutable `poly` register length
- `cfN` reads the Nth element of immutable coefficient-function output `cf`
- `polyN` reads the Nth element of current mutable output register `poly`
- `tosN` reads the Nth element of the current top-of-stack vector without
  popping it
- `cfN`, `polyN`, and `tosN` use zero-based indices and compile only for
  `N in [0,255]`; runtime rejects out-of-range reads with chip context
- `tosN` is a runtime error when the vector stack is empty
- argument declarations still own type checking:
  - `push_const(length,value)` accepts complex expression results for `value`
  - `linear` accepts two complex expressions: multiplier and offset
  - `exp`, `pow`, and Program-mode `round` also accept compact complex
    expressions; do not display these as `field1+i*field2` in the Program UI
  - other v1 legacy-bridge transform args are limited to real, integer, and
    enum declarations until the C bridge explicitly supports complex args for
    those transforms
  - real args require real-valued expression results, or explicit `real(x)` /
    `imag(x)`
  - integer and enum args are literal-only in v1 unless explicitly listed
- division by zero returns a clear execution error, not NaN propagation
- any non-finite expression result is an execution error with chip context
- expression bytecode has a fixed max token count, e.g. `MAX_SCALAR_EXPR_TOKENS = 32`
- fingerprints include the normalized expression bytecode, not raw text

Do not implement an unbounded general-purpose expression parser in the native
hot loop. The compiler owns parsing; native owns bounded evaluation.

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
- `poly_len` is updated by every transform or chip that writes to `poly`
- reads of `poly` after a length-changing chip see the new length

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

Reject as early as possible:

- if the selected coefficient function has statically-known output length and
  it exceeds 256, reject in render/compute plan before dispatch
- if a `const(length,value)` coefficient function or `push_const(length,value)`
  chip uses `length > 256`, reject at compile/plan time in Program mode
- if the registry proves a transform preserves length and the input length is
  already known to exceed 256, reject before execution
- for `may_change_length` transforms, the native runtime remains authoritative
  and must reject immediately after the chip that produces the over-limit vector

Required error shape:

```text
Coeff Program vector length exceeded at chip N (legacy(roots)): got 301, max 256
```

Editor diagnostics should surface length uncertainty:

```text
length: preserves 35
length: changes 35 -> 34
length: may change at runtime
length: may exceed Program cap
```

For transforms such as `deriv`, the editor can display known static deltas when
the input length is known. For transforms such as `roots`, `power`, and
`invpower`, the editor should display "may change at runtime" unless the
registry can prove the exact resulting length.

## Vector Ops

Vector ops are first-class Coeff Program chips, not legacy transforms. They
compile to integer opcodes and run in the native VM without string dispatch.

Binary ops:

```text
[tgt] = add([src1], [src2])
[tgt] = subtract([src1], [src2])
[tgt] = multiply([src1], [src2])
[tgt] = divide([src1], [src2])
[tgt] = power([src1], [src2])
```

Rules:

- `tgt` is `poly` or `push`
- `src1` / `src2` are `poly`, `pop`, or `peek`
- input lengths must match
- operations are elementwise complex operations
- `power` uses the principal branch, equivalent to `exp(src2 * log(src1))`
- non-finite element results are written as `0+0i`

Unary ops:

```text
[tgt] = angle([src])
[tgt] = mod([src])
[tgt] = abs([src])
```

Rules:

- `angle(z)` returns `atan2(im, re)` as a real vector
- `mod(z)` and `abs(z)` return `sqrt(re^2 + im^2)` as real vectors

Ordering and roll ops:

```text
[tgt] = argsort([src1], [src2])
[tgt] = roll([src], n)
[tgt] = rolr([src], n)
[tgt] = littlewood(field1, field2) andy=field3
```

`argsort` sorts `src1` by ascending magnitude of `src2`; ties preserve original
index order. `roll` rolls left by integer `n`; `rolr` rolls right by integer
`n`. Negative `n` is accepted and naturally rolls in the opposite direction.

`littlewood(field1, field2, andy)` creates a vector with the same length as the
current `poly`. Each coefficient independently chooses `field1` or `field2`
with 50/50 odds. `field1` and `field2` are complex scalar expressions; `andy` is
a real scalar expression. The result is blended against the current `poly` using
the same coefficient-transform convention:

```text
out = littlewood_random * (1 - andy) + poly * andy
```

The random bits must be deterministic for a given compute step, token index, and
parameter values. Do not use scheduler-dependent thread-local RNG state for this
chip; threaded and chunked coeffgen must produce identical results for identical
inputs.

For binary vector ops and `argsort`, source selectors are evaluated left to
right. If both sources are `pop`, `src1` receives the current stack top and
`src2` receives the next vector below it. This is explicit so the UI can explain
the order rather than hiding it.

`blend(t)` remains the stack-oriented two-vector combine chip.

`blend(t)` pops two vectors, requires equal length, and pushes:

```text
out = next * (1 - t) + top * t
```

Stack order:

```text
push(a)
push(b)
blend(0.25)
```

produces:

```text
out = a * 0.75 + b * 0.25
```

Rules:

- both vectors must have identical length
- `t` is a finite real scalar expression
- no padding
- no truncation
- length mismatch is an execution error with chip context

Defer the unbounded or shape-changing vector set:

```text
concat
slice
pad
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
push_const(length,value) pops 0, pushes 1 vector, writes none
push_linspace(length) pops 0, pushes 1 vector, writes none
push_range(length)    pops 0, pushes 1 vector, writes none
push(cf)            pops 0, pushes 1 vector, writes none
push(poly)          pops 0, pushes 1 vector, writes none
emit                pops 1 if present, pushes 0, writes/commits poly
macro(name)         source-only; expands before validation
blend(t)            pops 2, pushes 1 vector, writes none
poke_poly(i,value)  pops 0, pushes 0, writes poly[i]
poke_tos(i,value)   pops 0, pushes 0, mutates stack top at [i]
add(tgt,s1,s2)      pops per source selector, pushes if tgt=push, writes poly if tgt=poly
subtract(tgt,s1,s2) same as add
multiply(tgt,s1,s2) same as add
divide(tgt,s1,s2)   same as add
power(tgt,s1,s2)    same as add
argsort(tgt,s1,s2)  reorder s1 by ascending |s2|; same stack effect as binary ops
angle(tgt,src)      pops per source selector, pushes if tgt=push, writes poly if tgt=poly
mod(tgt,src)        same as angle
abs(tgt,src)        same as angle
roll(tgt,src,n)     same as angle; roll left by integer n
rolr(tgt,src,n)     same as angle; roll right by integer n
littlewood(tgt,a,b,andy) pops 0, pushes if tgt=push, writes poly if tgt=poly

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
legacy(name, peek, poly, args...)   pops 0, pushes 0, writes poly
legacy(name, peek, push, args...)   pops 0, pushes 1, writes none
```

`peek` is intentionally source-only. There is no `tgt=peek`.
`poke_tos` requires stack depth at least 1 but does not pop; it mutates the
current top vector in place.

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
push_const(length, value)
push_linspace(length)
push_range(length)
poke_poly(index, value)
push(cf)
push(poly)
emit

Stack:
duplicate
swap
blend(t)
poke_tos(index, value)
pop
flush

Vector:
add(tgt, src1, src2)
subtract(tgt, src1, src2)
multiply(tgt, src1, src2)
divide(tgt, src1, src2)
power(tgt, src1, src2)
argsort(tgt, src1, src2)
angle(tgt, src)
mod(tgt, src)
abs(tgt, src)
roll(tgt, src, n)
rolr(tgt, src, n)
littlewood(tgt, field1, field2, andy)

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
linear
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

`linear` is the canonical name for the old `scale100` coefficient transform.
It has two complex parameters and displays as:

```text
z*[param1]+[param2]
```

The default is `linear(100, 0)`, which preserves the old scale-by-100 behavior.
Old saved/native `scale100` chains remain accepted as compatibility aliases.

Legacy coefficient transforms are represented only by the generic
`legacy(name, src, tgt, ...)` chip. Do not ship separate `poly-*` aliases in the
UI or compiler. This avoids two spellings for the same operation and keeps the
source/target selectors visible.

Follow `ui_docs/style_guide.md`:

- controls align vertically
- modal action rows remain visible
- saved-program tables do not cause modal height jumps
- no debug JSON as the primary representation
- chip previews look like the editor chips

## Compute Debug

The Compute Preview area should expose a sibling `Compute Debug` tab. It is not
a separate execution model. It uses the same selected `Use: Chain | Program`
mode, same coefficient function, same `cfpv`, same Param pipeline, and same
Coeff pipeline as Compute Preview.

Debug inputs:

```text
u in [0,1]
v in [0,1]
```

Actions:

```text
Param eval   -> show t1, t2, p1, p2 for that single point
Poly eval    -> show cf and final poly coefficients for that single point
SolveAE eval -> run AE-MT on that one poly vector and show roots
SolveCM eval -> run companion-matrix solve on that one poly vector and show roots
```

The backend should use the production native compiler/evaluator path. Do not
reimplement Param Program or Coeff Program semantics in JavaScript or Python for
debug display. First-pass debug output should show final register values,
coefficient arrays, roots when requested, token counts, stack maximums, and
final stack depth. A per-chip trace can be added later, but should be native
trace data if added.

## Serialization

Saved source format:

```json
{
  "version": 1,
  "program_kind": "coeff_program",
  "name": "reverse normalized roots",
  "chain": [
    ["legacy", "rev", "poly", "poly"],
    ["legacy", "normalize", "poly", "poly"],
    ["legacy", "roots", "poly", "poly", "8", "hi"]
  ]
}
```

Example with parameter-dependent constants:

```json
{
  "version": 1,
  "program_kind": "coeff_program",
  "name": "p1 constant base",
  "chain": [
    ["const", "35", "p1"],
    ["legacy", "linear", "pop", "push", "p2", "0"],
    ["emit"]
  ]
}
```

Canonical compiler output:

```json
{
  "program_kind": "coeff_program",
  "version": 1,
  "source_chain": [["legacy", "rev", "poly", "poly"], ["legacy", "normalize", "poly", "poly"]],
  "tokens": [
    {"op": 1, "fn_index": 1, "src": 2, "tgt": 2, "args": []},
    {"op": 1, "fn_index": 3, "src": 2, "tgt": 2, "args": []}
  ],
  "scalar_exprs": [],
  "fingerprint": "sha1...",
  "display": "legacy(rev,poly,poly); legacy(normalize,poly,poly)",
  "stack_max": 0,
  "uses_legacy_chain_equivalent": true
}
```

Bare legacy transform names such as `rev` and direct aliases such as `poly-rev`
are not valid Coeff Program chips. Old Chain mode still uses bare coefficient
transform names, but Program mode uses explicit `legacy(...)` chips.

The exact enum values do not matter in the doc. The stable contract is:

- source names are compiled before native execution
- source JSON is preserved for UI
- execution tokens drive cache keys
- macro-expanded canonical tokens drive fingerprints
- scalar argument expressions are normalized and included in the fingerprint

## Cache Keys

Compute cache identity must include:

- pipeline mode: `chain` or `program`
- Param Program execution fingerprint when Program mode is active
- Coeff Program execution fingerprint when Program mode is active
- legacy param transform chain when Chain mode is active
- legacy coeff transform chain when Chain mode is active
- coefficient function and coefficient function parameters, including
  `const(length,value)`

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
    double t1_re;
    double t1_im;
    double t2_re;
    double t2_im;
    double p1_re;
    double p1_im;
    double p2_re;
    double p2_im;
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
set read-only scalar registers p1, p2
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
- each worker/probe path owns a `CoeffProgramWorkspace`
- workspace is allocated once outside row execution; do not put the full
  workspace on pthread call stacks
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

ring stack         = 64 vectors ~= 256 KiB/thread
poly + cf + scratch ~= a few additional vectors
```

This is acceptable for normal thread counts, but must be measured. The point is
to trade a bounded per-thread memory reservation for predictable speed and zero
allocator pressure.

Recommended v1 limits:

```text
MAX_PROGRAM_TOKENS = 64 after macro expansion
MAX_VECTOR_STACK   = 64 ring slots
MAX_VECTOR_LEN     = 256 complex coefficients
MAX_MACRO_DEPTH    = 8
```

The existing native `MAX_COEFFS` may remain larger for Chain mode. Coeff Program
uses the stricter fixed arena limit unless benchmarks prove a larger ring is
safe.

Fused chunk memory budget must account for this workspace:

```text
workspace_bytes_per_thread ~= 266,384
16 threads ~= 4.3 MiB
```

If Stage 0 or Docker runtime shows Lambda memory pressure, increase the fused
compute Lambda memory before shipping Program mode. Do not silently reduce the
workspace cap per thread at runtime; that would make behavior depend on deploy
configuration.

### Native Token Shape

There must be no string lookup in Program-mode hot execution.

Compile:

```text
source chip name -> integer opcode
legacy transform name -> integer function index
src/tgt selector -> small integer
numeric parameters -> parsed doubles, scalar expression refs, or enum args
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
    uint8_t n_exprs;
    uint8_t reserved;
    double args[COEFF_PROGRAM_MAX_ARGS];
    uint16_t expr_ref[COEFF_PROGRAM_MAX_ARGS];
} CoeffProgramToken;
```

`args[]` stores literal numeric/enum values. `expr_ref[]` stores indices into a
compiled scalar-expression table for arguments that depend on `p1` / `p2`.
Arguments that are not expressions use a sentinel such as `UINT16_MAX`.

Scalar expression table:

```c
typedef struct {
    uint8_t op;
    uint8_t a;
    uint8_t b;
    uint8_t reserved;
    double literal_re;
    double literal_im;
} CoeffScalarExprToken;

typedef struct {
    uint16_t start;
    uint16_t count;
} CoeffScalarExprRef;
```

The exact layout can change, but the compiled program must contain all scalar
expression bytecode needed to evaluate `p1` / `p2` references. Native execution
must not look up expression strings.

Workspace shape:

```c
#define COEFF_PROGRAM_MAX_VECTOR_STACK 64
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

    double scalar_arg_re[COEFF_PROGRAM_MAX_ARGS];
    double scalar_arg_im[COEFF_PROGRAM_MAX_ARGS];
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
emit commits current poly when the stack is empty
legacy(roots) may change vector length
length: preserves 35
length: changes 35 -> 34
length: may change at runtime
length: may exceed Program cap
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
- `push(p1)` and `push(p2)` are errors; use `push_const(poly_len,p1/p2)`
- scalar argument expressions must compile and evaluate finitely
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
program=legacy(rev,poly,poly)
program=legacy(rev,poly,poly);legacy(normalize,poly,poly);legacy(conj,poly,poly)
program=legacy(deriv,poly,poly)
program=legacy(roots,poly,poly,8,hi)
program=push_const(poly_len,p1);emit
program=push_const(poly_len,p1+p2);legacy(rev,pop,push);emit
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

### First Local Benchmark Result

Local native spike on 2026-04-28:

```text
binary: /tmp/coeff_program_bench
command: /tmp/coeff_program_bench 10000000 3 35 rev
transform: rev
input vector: 1+1i, 2+2i, ..., 35+35i
evaluations: 10,000,000 per repetition
repetitions: 3
workspace: stack=64, vector_len=256, bytes/thread=266,384
```

Results:

```text
legacy dispatchCt rev:
  median 519,540 us
  51.95 ns/eval
  19.25M eval/s

program legacy(rev, src=poly, tgt=poly):
  median 200,239 us
  20.02 ns/eval
  49.94M eval/s
  penalty vs legacy: -61.46%

program push(cf); legacy(rev, src=pop, tgt=push); emit:
  median 294,053 us
  29.41 ns/eval
  34.01M eval/s
  penalty vs legacy: -43.40%
```

All three checksums matched. The direct Program path is faster because it uses a
compiled function pointer instead of the old `dispatchCt` string cascade. The
stack form is also faster than legacy for this case, but it performs extra
vector copies, so heavier stack programs still need the full benchmark matrix.

Second local native spike on 2026-04-28:

```text
binary: /tmp/coeff_program_bench
command: /tmp/coeff_program_bench 10000000 3 35 chain
transform chain: rev, cumsum, sort_abs, exp
andy: 0.5 on every transform
input vector: 1+1i, 2+2i, ..., 35+35i
evaluations: 10,000,000 per repetition
repetitions: 3
workspace: stack=64, vector_len=256, bytes/thread=266,384
```

Results:

```text
legacy dispatchCt chain:
  median 5,860,223 us
  586.02 ns/eval
  1.71M eval/s

program chain, src=poly, tgt=poly:
  median 3,723,291 us
  372.33 ns/eval
  2.69M eval/s
  penalty vs legacy: -36.47%

program stack chain, push(cf); per-transform pop/push; emit:
  median 3,860,300 us
  386.03 ns/eval
  2.59M eval/s
  penalty vs legacy: -34.13%
```

All three checksums matched. This is a better stress case than `rev` because
`exp` dominates runtime and `andy=0.5` forces per-transform original-vector
copies and blend work. Program mode still wins because dispatch is compiled to
function pointers and selectors instead of re-entering the string-based legacy
chain dispatch for every row and transform.

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
  - read-only scalar registers `p1`, `p2`
  - scalar expression evaluation for literals, `p1`, and `p2`
  - `push_const(length,value)`
  - `push_linspace(length)`
  - `blend(t)`
  - `push(cf)`
  - `push(poly)`
  - `peek` source selector
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

- output matches for identity, `push_const(poly_len,p1);emit`, `rev`, `rev;conj`, and
  `normalize`
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
- validate `push_const(length,value)` chip and old `const` alias
- validate `push_linspace(length)` chip
- validate `push_range(length)` chip
- validate `blend(t)` same-length requirement where statically knowable
- support `peek` source selector
- emit length diagnostics from registry length policies
- compile scalar argument expressions that reference `p1` / `p2`
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
  "source_chain": [["legacy", "rev", "poly", "poly"], ["legacy", "normalize", "poly", "poly"]],
  "tokens": [],
  "scalar_exprs": [],
  "fingerprint": "sha1...",
  "display": "legacy(rev,poly,poly); legacy(normalize,poly,poly)",
  "stack_max": 0,
  "uses_legacy_chain_equivalent": true,
  "legacy_coeff_transforms": [["rev"], ["normalize"]]
}
```

Exit criteria:

- equivalent source chains hash the same after lowering
- `push_const(length,value)` and `push_linspace(length)` stack effects and
  vector length validation are covered
- `blend(t)` requires equal lengths and reports mismatch with chip context
- `peek` reads stack top without changing stack depth
- scalar expressions using `p1` / `p2` compile to canonical bytecode
- invalid selectors fail clearly
- unknown transform names fail clearly
- old coefficient chains compile to Program-equivalent source

### Stage 1.5: Coefficient Function `const`

Goal: add a real coefficient function that returns a fixed-length constant
coefficient vector.

Files:

- `lambda/sweep_cli.c`
- `lambda/coeff_func_catalog.json`
- `lambda/gen_catalog.py` if catalog generation needs a special built-in
- `lambda/coeff_func_lookup.h`
- root `coeff_func_catalog_js.js`
- `tests/test_coeff_catalog_consistency.py`
- `tests/test_sweep_smoke.py`
- `tests/test_frontend_js.sh`

Work:

- implement `const` as a `CoeffFuncC` built-in
- encode parameters as:
  - `length`
  - `value_re`
  - `value_im`
- parse UI value as one complex field, not separate `.re` / `.im` fields
- reject non-finite values
- reject `length < 1`
- reject Program mode when `length > COEFF_PROGRAM_MAX_VECTOR_LEN`
- expose the function in the coefficient-function dropdown
- label length as `length / degree+1`

Exit criteria:

- `coeff_function=const(length=35,value=1+2i)` produces 35 identical
  coefficients
- Chain mode with `const` and no coefficient transforms works
- Program mode with `const` and an empty coeff program works
- Program mode with coefficient function `const` plus
  `push_const(poly_len,p1); emit` proves the coefficient function and Coeff
  Program chip are disambiguated correctly
- user-facing display strings are disambiguated as `coeff_const(...)` and
  `push_const(...)`
- catalog, generated JS, and native lookup stay in sync

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
    "tokens": [],
    "scalar_exprs": []
  }
}
```

Work:

- parse compiled `coeff_program`
- parse compiled scalar expression table
- pass final row `p1` / `p2` into the coeff-program evaluator
- run coeff program anywhere `coeff_transforms` currently run:
  - degree probe
  - monolithic coeffgen
  - coeffgen_chunked
  - fused chunk local coeffgen
- keep `coeff_transforms` path alive
- allocate one `CoeffProgramWorkspace` per worker/probe path outside the row
  loop, not on pthread call stacks
- emit metadata:
  - `coeff_program_tokens`
  - `coeff_program_stack_max`
  - `coeff_program_fingerprint`
  - `coeff_program_scalar_expr_count`
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
- add coefficient function `const(length,value)` to plan/preview parameter
  handling
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
  - `push_const(length,value)` chip with a `poly_len`-or-integer length field
    and one complex expression field
  - `push_linspace(length)` chip with the same length field and default
    `poly_len`
  - `poke_poly(index,value)` and `poke_tos(index,value)` chips with one integer
    index field and one complex expression field
  - complex expression fields that make `p1` / `p2` availability clear
  - `+ before` / `+ after`
  - move arrows
  - saved program modal
  - read-only chip previews
  - macro selection
  - diagnostics
- compute preview payload follows selected mode
- compute debug payload follows selected mode and uses native single-point
  evaluation, not a duplicate frontend/Python evaluator
- full compute payload follows selected mode
- coefficient function dropdown exposes `const`
- `const` coefficient function shows aligned fields:
  - `length / degree+1`
  - `value`
- mode toggle shows a one-time hint when switching away from non-empty inactive
  Chain or Program content
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
- `lambda/coeff_func_catalog.json`
- root `coeff_func_catalog_js.js`
- `lambda/coeff_func_lookup.h`

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
- coefficient function `const` is present in backend catalog, frontend catalog,
  and native lookup
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

Required targeted tests:

- compiler accepts `push_const(poly_len, 1+2i)`,
  `push_const(35, p1+p2)`, and old alias `const(35, p1+p2)`
- compiler rejects `push_const(0, 1)`, `push_const(257, 1)`, and non-finite values
- scalar expression bytecode for `p1`, `p2`, and `p1+p2` is stable in the
  fingerprint
- real-valued legacy args accept `real(p1)` / `imag(p1)` and reject complex
  expression results without explicit projection
- `blend(t)` same-length success and mismatch failure are covered
- `peek` source selector does not change stack depth
- length diagnostics cover preserve, known static change, runtime-dependent
  change, and cap-risk cases
- over-limit vectors reject before dispatch when statically knowable and at the
  exact offending chip otherwise
- native Program mode evaluates `push_const(poly_len,p1); emit` and
  `push_linspace(poly_len); emit` and `push_range(poly_len); emit`
  deterministically
- native Program mode passes final transformed parameters as `p1` / `p2`, not
  raw serpentine inputs
- coefficient function `const(length,value)` produces the expected vector in
  Chain mode and Program mode
- compute preview and full compute agree for `coeff_function=const` plus a
  Coeff Program that reads `p1` / `p2`
- frontend tests cover the `const` coeff function fields and Coeff Program
  `push_const` / `push_linspace` chip fields

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
- read-only `p1` / `p2` scalar registers inside Coeff Program
- coefficient function `const(length,value)`
- Coeff Program compiler
- Coeff Program saved program storage
- Coeff Program editor
- Coeff Program chip `push_const(length,value)` plus old `const` alias
- Coeff Program chip `push_linspace(length)`
- Coeff Program chip `push_range(length)`
- `blend(t)` vector-combine chip with same-length-required semantics
- pinned vector ops: `add`, `subtract`, `multiply`, `divide`, `power`,
  `argsort`, `angle`, `mod`, `abs`, `roll`, `rolr`, `littlewood`
- `poke_poly(index,value)` direct poly coefficient write
- `poke_tos(index,value)` direct top-of-stack vector coefficient write
- `push(cf)`, `push(poly)`, `emit`
- `duplicate`, `swap`, `pop`, `flush`
- `macro(name)`
- generic `legacy(name, src, tgt, args...)`
- source selectors: `cf`, `poly`, `pop`, `peek`
- target selectors: `poly`, `push`
- scalar argument expressions with literals, `t1`, `t2`, `p1`, `p2`, basic
  arithmetic, `conj`, `neg`, `real`, `imag`, `abs`, and `log`
- exact legacy-chain equivalence detection
- compute preview parity with selected mode
- full compute/fused/lores parity with selected mode
- benchmark results in this document before implementation is called ready

Do not ship in v1:

- unbounded or shape-changing vector operations without pinned length policy
- mixed Chain/Program mode in the main UI
- dynamic macro lookup in native execution
- string lookup in Program-mode hot loops
- native row-loop parsing of scalar expression strings
- per-row heap allocation
- hidden truncation/padding of vector lengths

## Open Questions

1. Should `emit` require final stack empty, or should it imply `flush` after
   writing `poly`? Resolved: final stack must be empty; `emit` itself only pops
   one vector when present and otherwise commits current `poly`.
2. Should `poly` be readable by `push(poly)` in v1? Recommended: yes, because it
   makes branching from the current transformed vector natural.
3. Should all direct coeff chips expose `src`/`tgt`, or only the generic
   `legacy(...)` chip? Resolved: only the generic `legacy(...)` chip ships for
   coefficient transforms. Direct `poly-*` aliases are redundant and confusing.
4. Should `andy` remain visible on legacy coeff transform chips? Recommended:
   yes for parity with existing chain UI, but layout must be improved so it does
   not crowd the chip.
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
