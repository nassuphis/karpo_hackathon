# Param Program Scalar Expressions Plan

## Problem

Param Program still exposes some numeric values as low-level fields. The worst
case is `const(re, im)`, which forces a complex value into two separate inputs.
Legacy Param chips have the same limitation for their numeric arguments:
`rtheta(p)`, `crd(size)`, `moebius(a,b,c,d)`, `rect(width,height,turns)`, etc.

Users should be able to write the value they mean:

```text
const(100j * p1)
const(exp(t1 * 2j) + p2)
legacy(rtheta, both, both, p1 / p2)
legacy(moebius, both, both, 1, p1, 0, exp(t2))
```

The expression must be compiled once and evaluated by the native Param Program
VM without string parsing or string lookup in the row loop.

Important implementation note: the first implementation compiled expressions
into a separate scalar-expression bytecode interpreted from inside the main
Param Program VM. That satisfied the "no string parsing in the row loop" rule,
but it introduced two bytecode interpreters and two stacks. The preferred target
architecture is now a single Param Program VM: expression fields are lowered
into ordinary Param Program stack tokens, and legacy chips pop expression
arguments from the main stack behind the scenes.

This document therefore has two layers:

- Stages 1-4 describe the transitional implementation that already proved the
  parser, UI, and native payload shape.
- Stage 5 is the architectural target. It replaces the nested expression VM with
  single-VM lowering.

Do not treat the nested expression VM as the final design.

## Goals

- Replace editor-facing `const(re, im)` with one field: `const(value)`.
- Allow all numeric Param Program argument fields to accept expression strings.
- Keep selector/name/id fields as selectors or strings, not expressions.
- Support complex scalar expressions over `t1`, `t2`, `p1`, and `p2`.
- Keep `t1` and `t2` immutable read-only inputs.
- Keep `p1` and `p2` mutable output registers initialized to `t1` and `t2`.
- Preserve old saved/imported `const(re, im)` chains forever.
- Compile expressions deterministically and include their lowered execution form
  in the fingerprint.
- Lower expression arguments into main Param Program stack tokens. Legacy chips
  that need dynamic arguments should consume hidden stack arguments explicitly.
- Use one native Param Program interpreter. Do not keep a nested scalar
  expression VM as the long-term design.

## Non-Goals

- No JavaScript, Python, or user-defined expression evaluation.
- No stack reads inside expressions in v1.
- No assignment inside expressions.
- No vector or coefficient access from Param Program expressions.
- No dynamic macro expansion during native execution.
- No expressions for selectors, function names, macro ids, or chip names.

Coeff Program expressions are a separate language. They may read coefficient
state such as `poly_len`, `cfN`, `polyN`, and `tosN`, and they now share the
same `t1` / `t2` / `p1` / `p2` register names. Do not add coefficient-vector
reads to Param Program expressions to make the two languages look identical;
the execution contexts are different.

## Prerequisite: Selector Hygiene

This work must not repeat the old selector-aliasing trap. Param Program has four
distinct concepts:

- `t1`, `t2`: immutable row inputs.
- `p1`, `p2`: mutable output registers.
- `src`/`tgt`: compile-time selectors for legacy chips.
- stack values: temporary runtime values.

Before or in the same change, remove any compiler/UI behavior that silently
aliases `t1` to `p1` or `t2` to `p2` in selector positions. These must be clear
errors:

```text
legacy src cannot be t1; t1/t2 are read-only inputs accessed via push(t1)
emit target cannot be t1/t2; emit writes p1 or p2
push source cannot be p1/p2 in v1
```

The expression language may read both `t1` and `p1`, but selectors must not
pretend they are interchangeable.

## User-Facing Semantics

### Registers

`t1`
: Immutable input parameter 1 for the current row.

`t2`
: Immutable input parameter 2 for the current row.

`p1`
: Current output parameter 1. Initialized to `t1`, then modified by earlier
  `emit(p1)` or legacy writes.

`p2`
: Current output parameter 2. Initialized to `t2`, then modified by earlier
  `emit(p2)` or legacy writes.

Expressions are evaluated at the point where their chip executes. Therefore
`p1` and `p2` are order-dependent by design.

Example:

```text
const(0); emit(p1)
const(p1 + t2); emit(p2)
```

The second expression sees the current `p1`, which is `0`, not the original
`t1`.

### Same Register Names In Coeff Program

Coeff Program expressions use the same scalar register names, but at a later
pipeline stage:

`t1`, `t2`
: Original serpentine scan inputs for the row, before any legacy parameter
  transform or Param Program runs. These are immutable source coordinates.

`p1`, `p2`
: Final transformed parameters after the Param Program / legacy parameter
  chain. These are the values passed into the coefficient function.

This distinction is intentional. A Coeff Program expression such as:

```text
push_const(poly_len, t1 + t2 + p1 + p2)
```

must see source coordinates in `t1/t2` and transformed coordinates in `p1/p2`.
Do not alias `t1` to `p1` or `t2` to `p2` in Coeff Program expressions or in
debug output.

For monolithic `coeffgen`, both source and transformed coordinates are available
in the row loop. For `coeffgen_chunked`, `params.bin` stores only transformed
`p1/p2`, so native coeffgen reconstructs source `t1/t2` from:

```text
source_step_start
source_n1
source_n2
```

This keeps the existing `params.bin` record format unchanged while making
`t1/t2` available to Coeff Program expressions in classic, fused, lores, and
debug paths.

### V1 Expression Language

V1 supports:

- Complex literals: `1`, `-2.5`, `1e-3`, `3j`, `1+2j`, `1-2i`.
- Named constants: `pi`, `pi2`, `pi2i` where `pi2 = 2*pi` and `pi2i = 2*pi*1j`.
- Registers: `t1`, `t2`, `p1`, `p2`.
- Operators: `+`, `-`, `*`, `/`.
- Parentheses.
- Unary minus.
- Functions: `exp(x)`, `real(x)`, `imag(x)`, `abs(x)`, `mod(x)`.

`abs(x)` is the canonical magnitude spelling. `mod(x)` is accepted as an alias
for users who think in modulus notation.

`real(x)`, `imag(x)`, `abs(x)`, and `mod(x)` return real-valued complex numbers
with zero imaginary component.

Division uses the same policy as Coeff Program expressions: constant-folded
division by zero is a compile-time error; runtime division by zero is a runtime
Param Program expression error with chip context. Do not silently turn an
expression divide-by-zero into zero.

### Non-Finite Results

If expression evaluation produces `nan` or `inf`, execution fails with a clear
Param Program error that includes the chip index and expression context. Do not
silently push zero for expression failures.

## Expression-Eligible Chips

### Accept Expression Strings

Any numeric value field can become an expression field.

`const`
: Change from `const(re, im)` to `const(value)` where `value` is a complex
  expression.

`legacy(...args)`
: Every numeric legacy argument can become an expression. The registry decides
  whether each argument is `real` or `complex`.

`moebius`
: `a`, `b`, `c`, and `d` should be complex expressions.

`inv_t_plus_2`
: Replace `re1, im1, re2, im2` UI fields with two complex expression fields.

Real-valued legacy arguments can be real expressions:

```text
rtheta(p)
crd(size)
hrt(size, turns)
spdl(va, vb, vp)
lmc(a, b)
rsc(amp, k)
lss(A, B, a, b, phase)
ast(scale)
asp(a, b)
lsp(a, b)
dlt(R)
rply(sides, radius, turns)
star(points, outer, inner)
rect(width, height, turns)
rrect(width, height, m)
```

For real-valued arguments, a complex expression is rejected unless it is
explicitly projected with `real(...)`, `imag(...)`, `abs(...)`, or `mod(...)`.

### Do Not Accept Expression Strings

These fields remain selectors or identifiers:

- `push(src)`: `t1`, `t2`, or `both`.
- `emit(target)`: `p1` or `p2`.
- `legacy(name)`: function name.
- `legacy(src)`: source selector.
- `legacy(tgt)`: target selector.
- `macro(program id)`: saved Param Program id.

These chips have no value field:

```text
duplicate
swap
pop
flush
add
subtract
mul
ratio
negate
conj
reciprocal
unit_circle
square
cube
exp
```

## Saved Form

### Canonical Source Form

The canonical source form for `const` is one argument:

```json
[["const", "exp(t1 * 2j) + p2"]]
```

The canonical source form for legacy chips keeps selectors separate and stores
expression text only in numeric arg positions:

```json
[["legacy", "rtheta", "both", "both", "abs(p1) / 2"]]
```

The UI should save only canonical forms.

### Compatibility Input

Old two-argument forms remain accepted:

```json
[["const", "1", "2"]]
```

The compiler treats this as:

```text
1 + 2j
```

The editor should rewrite old forms on load into the canonical visual model.
Historical S3 data may remain old-form, but in-memory editor state and any next
save should use the canonical one-field form.

## Compiler Design

Implementation target: `lambda/param_program_chain.py`.

Add a Param Program scalar-expression parser modeled on the Coeff Program
parser, with Param-specific identifiers and argument typing. Reuse the parser
ideas, not the long-term nested expression VM shape.

### Limits

Use explicit fixed limits for parsing/lowering:

```text
MAX_LOWERED_PARAM_TOKENS_PER_EXPR = 32
MAX_SCALAR_EXPR_TOKENS = 32          # transitional nested-VM alias only
PARAM_PROGRAM_MAX_SCALAR_EXPRS = 64  # transitional nested-VM payload only
```

`MAX_LOWERED_PARAM_TOKENS_PER_EXPR` is per expression. In the single-VM target
this limits the maximum number of main-token ops an expression may lower into.
The old `MAX_SCALAR_EXPR_TOKENS` name remains only while the nested-VM reader
and tests exist. The program-wide expression table limit only applies to the
transitional nested-VM payload.

### Expression Opcodes And Lowering Ops

The transitional nested-VM path uses a Param-specific expression opcode
namespace:

```text
PARAM_EXPR_LITERAL
PARAM_EXPR_T1
PARAM_EXPR_T2
PARAM_EXPR_P1
PARAM_EXPR_P2
PARAM_EXPR_ADD
PARAM_EXPR_SUB
PARAM_EXPR_MUL
PARAM_EXPR_DIV
PARAM_EXPR_NEG
PARAM_EXPR_EXP
PARAM_EXPR_REAL
PARAM_EXPR_IMAG
PARAM_EXPR_ABS
```

`mod(x)` compiles to the same opcode as `abs(x)`.

The single-VM refactor maps these expression opcodes to ordinary main VM stack
tokens instead of storing them in a separate `scalar_exprs` table.

### Constant Folding And Fingerprints

Constant subexpressions are folded greedily before fingerprinting. Any subtree
with no register references becomes a single `LITERAL(re, im)` token.

This canonicalization rule is part of the cache contract. Two equivalent source
expressions that differ only by literal grouping should produce the same
lowered execution form when the same greedy folding rule applies.

The lowered execution form is included in the compiled program fingerprint.
Source text formatting is not part of the execution fingerprint after parsing
and canonicalization.

This is intentionally not full algebraic canonicalization. Associative rewrites
are out of scope: `t1 + 1 + 2` may lower differently from `t1 + (1 + 2)`.
Both must evaluate the same, but they are allowed to hash differently. Avoid
claiming that semantically equivalent expressions always share a fingerprint.

Stage 5 changes the fingerprint shape because `scalar_exprs` / `expr_refs` are
removed and the expanded main-token stream is hashed instead. That is a
deliberate cache-invalidation event: source programs compiled before Stage 5 may
miss cache after deploy even when the displayed program text is unchanged.

### Argument Types

Extend `lambda/param_legacy_registry.json` so each numeric argument declares a
type:

```json
{"name": "size", "type": "real", "default": "1"}
{"name": "a", "type": "complex", "default": "1"}
```

Rules:

- `real` args compile from real-valued expressions only.
- `complex` args compile from any complex expression.
- Missing args use registry defaults, compiled through the same expression path.
- Old numeric defaults remain valid.

### Current Token Shape: Nested VM

The first implementation aligned with the Coeff Program approach and used
per-argument expression refs. This is the current shipped shape, but it is not
the preferred target after the single-VM refactor.

Recommended JSON token shape:

```json
{
  "op": 21,
  "fn_index": 18,
  "src": 3,
  "tgt": 3,
  "n_args": 1,
  "args": [0.0],
  "args_im": [0.0],
  "expr_refs": [0, -1, -1, -1, -1, -1, -1, -1]
}
```

For `const(value)`, use arg slot 0:

```json
{
  "op": 5,
  "n_args": 1,
  "args": [0.0],
  "args_im": [0.0],
  "expr_refs": [0, -1, -1, -1, -1, -1, -1, -1]
}
```

For pure static literals, the compiler may keep `expr_refs[slot] = -1` and put
the value directly in `args[slot]` / `args_im[slot]`. The native path must treat
static and dynamic args identically after argument resolution.

### Compiled Program Shape

Add a top-level expression table:

```json
{
  "tokens": [
    {"op": 5, "n_args": 1, "args": [0.0], "args_im": [0.0], "expr_refs": [0]}
  ],
  "scalar_exprs": [
    {
      "tokens": [
        {"op": "PARAM_EXPR_T1"},
        {"op": "PARAM_EXPR_LITERAL", "a": 0.0, "b": 2.0},
        {"op": "PARAM_EXPR_MUL"}
      ]
    }
  ]
}
```

The exact compact encoding can use integers, as Coeff Program does. The
important rule is that the native payload is already compiled and contains no
source strings.

After the single-VM refactor, newly compiled payloads should not contain
`scalar_exprs` or `expr_refs`. The expression's lowered tokens should live in
the main token stream, immediately before the chip that consumes their value.
Legacy chips with dynamic args should use `stack_arg_count` instead.

### Macro Expansion

Macro bodies are expanded before expression lowering and before hashing.

If `macro(spiral)` expands to a body containing `const(t1+t2)`, the parent
compiled program owns the final expression table. The macro-expanded fingerprint
must match a hand-written equivalent program with the same expression.

Macro bodies do not create a new register scope. `t1`, `t2`, `p1`, and `p2`
inside a macro resolve against the parent row context at the point where the
macro body is expanded and executed.

Compute artifacts remain snapshot-based: macro expansion is locked at compute
time. Live preview expands macros from the current saved macro body.

### Draft Diagnostics

`compile_param_program_diagnostics()` should keep draft expression strings in
the chip model while reporting parse/lower errors. During typing, invalid
partial expressions such as `const(p` should not destroy the draft chain.

Expected behavior:

- Strict compile raises.
- Draft diagnostics return chip-indexed errors.
- The editor shows inline diagnostics for expression parse errors.
- Bytecode is committed only for parseable expressions.

### Validation

Compiler errors should include:

- Unknown identifier.
- Unknown function.
- Wrong function arity.
- Invalid complex literal.
- Lowered expression longer than `MAX_LOWERED_PARAM_TOKENS_PER_EXPR`.
- More than `PARAM_PROGRAM_MAX_SCALAR_EXPRS` expressions in the transitional
  nested-VM payload.
- Non-finite literal.
- Complex expression used for a real-only argument.

Use clear messages:

```text
param expression at chip 4 arg 0: unknown identifier "poly"
param expression at chip 2 arg 0: exp requires 1 argument
param expression at chip 6 arg 1: real-valued argument got complex expression; use real(...), imag(...), abs(...), or mod(...)
```

## Native VM Design

Implementation target: `lambda/sweep_cli.c`.

### Current Implementation: Nested Expression VM

The first implementation added a second VM inside the Param Program path:

```text
Main Param Program VM:
  PARAM_OP_CONST expr_ref=0
  PARAM_OP_LEGACY expr_refs=[...]

Scalar-expression VM:
  PARAM_EXPR_T1
  PARAM_EXPR_T2
  PARAM_EXPR_ADD
```

This is compiled upfront and does not parse strings in the row loop, but it is
still a separate interpreter with a separate expression stack. The benchmark in
Stage 2 shows the cost:

```text
literal const baseline:      47.94 ns/row
nested expression const:     58.30 ns/row
flat main-stack equivalent:  56.00 ns/row
```

The nested VM is only modestly slower than the flat equivalent in this case, but
the architecture is unnecessarily split. Param Program values are already
complex scalars, so expression evaluation can use the main Param Program stack.

### Target Implementation: Single VM Lowering

The compiler should lower expression ASTs into ordinary Param Program tokens.
There should be no `scalar_exprs` table in newly compiled payloads.

Examples:

```text
const(t1+t2)
```

lowers to:

```text
push(t1)
push(t2)
add
```

and:

```text
legacy(rtheta, both, both, p1+p2)
```

lowers to:

```text
push(p1)        # compiler-internal or explicitly exposed opcode
push(p2)
add
legacy(rtheta, both, both, stack_arg_count=1)
```

The legacy opcode pops one stack argument immediately before invoking the
function. The argument pop is hidden from the chip's user-facing display but
must be visible to compiler diagnostics and debug output.

For multiple arguments, expressions lower left-to-right:

```text
legacy(moebius, both, both, a_expr, b_expr, c_expr, d_expr)
```

lowers to:

```text
<tokens for a_expr>  # leaves a on stack
<tokens for b_expr>  # leaves b on stack
<tokens for c_expr>  # leaves c on stack
<tokens for d_expr>  # leaves d on stack
legacy(moebius, both, both, stack_arg_count=4)
```

The native legacy opcode pops in reverse order and stores into argument slots in
source order:

```text
pop d, pop c, pop b, pop a
args = [a, b, c, d]
```

This preserves normal left-to-right argument meaning while matching RPN stack
order.

### Required Main VM Opcodes

Expression lowering needs these main VM operations:

```text
PUSH_T1
PUSH_T2
PUSH_P1
PUSH_P2
CONST_LITERAL
ADD
SUBTRACT
MULT
DIVIDE
NEGATE
EXP
REAL
IMAG
ABS
```

`PUSH_T1` and `PUSH_T2` already exist conceptually through `push(t1)` and
`push(t2)`. `PUSH_P1` and `PUSH_P2` may be compiler-internal at first. The
source-level selector hygiene rule still holds: do not silently alias `t1` to
`p1` or `t2` to `p2`.

The opcode space is intentionally broader than the source-authored chip set.
`PUSH_P1` and `PUSH_P2` are compiler-only in v1: users still cannot author
`push(p1)` or `push(p2)` chips unless the source language is explicitly
expanded later.

`REAL`, `IMAG`, and `ABS` push real-valued complex numbers with zero imaginary
component. `mod(x)` lowers to `ABS`.

`DIVIDE` uses the same divide-by-zero policy already specified for expressions:
constant-folded zero division is a compile-time error; runtime zero division is
a Param Program runtime error with chip context.

### Legacy Stack Arguments

Add an explicit token field:

```text
stack_arg_count
```

Rules:

- `stack_arg_count = 0`: legacy args come from static `args` / `args_im`.
- `stack_arg_count = n_args`: legacy pops all args from the main stack.
- Do not support mixed static/dynamic args in v1 of the refactor; if any arg is
  dynamic, lower all args for that legacy token onto the stack.
- This all-or-none rule is a conscious v1 scope cut. It creates a small
  performance cliff for chips such as `moebius(1, 0, 0, t2)`, where three
  literal args are still lowered through the stack because one arg is dynamic.
  Mixed static/dynamic arg delivery can be added later if benchmarks show it
  matters.
- The compiler must ensure the lowered expression tokens leave exactly
  `stack_arg_count` values above the pre-existing stack.
- The legacy opcode must restore the user's stack to the same depth after
  consuming its hidden args, except for explicit `src=pop1`, `src=pop2`,
  `tgt=push1`, or `tgt=push2` behavior.

Static literal-only legacy chips may keep using `args` / `args_im` so old
legacy-equivalent fast paths remain possible. Dynamic-arg legacy chips use
stack args. This fast-path policy must be re-benchmarked during Stage 5 because
earlier Param Program measurements showed the VM path can beat the old
`dispatchPt` path; preserving the fast path may preserve a slower route if the
old measurements still hold.

### Debug And Display

The editor should keep showing the compact user-facing chip:

```text
legacy(rtheta, both, both, p1+p2)
```

but debug views should expose the lowered form on demand:

```text
push(p1); push(p2); add; legacy(rtheta, both, both, stack_arg_count=1)
```

Stack diagnostics must count hidden expression tokens. A chip with expression
args may be user-facing stack-neutral while internally using temporary stack
slots.

### Current Native Data Structures: To Replace

The current implementation extended `ParamProgramToken` with per-arg expression
refs and imaginary arg storage:

```c
int expr_refs[PARAM_PROGRAM_MAX_ARGS];
double args[PARAM_PROGRAM_MAX_ARGS];
double args_im[PARAM_PROGRAM_MAX_ARGS];
```

Add fixed expression storage to `ParamProgram`:

```c
#define PARAM_PROGRAM_MAX_SCALAR_EXPRS 64
#define PARAM_PROGRAM_MAX_EXPR_TOKENS 32

typedef struct {
    int op;
    double a;
    double b;
} ParamScalarExprToken;

typedef struct {
    int n_tokens;
    ParamScalarExprToken tokens[PARAM_PROGRAM_MAX_EXPR_TOKENS];
} ParamScalarExpr;
```

The compiled program is immutable and shared across threads. Expression
evaluation uses only local stack arrays, so it is thread-safe.

This structure is acceptable as a transitional implementation. The single-VM
refactor should remove `ParamScalarExpr`, remove `scalar_exprs` from newly
compiled payloads, and keep only static `args` / `args_im` plus
`stack_arg_count` for dynamic legacy args.

### Current Evaluation: To Replace

The current implementation added:

```c
static int paramEvalScalarExpr(
    const ParamProgram *program,
    int expr_ref,
    ParamCx t1,
    ParamCx t2,
    ParamCx p1,
    ParamCx p2,
    ParamCx *out
);
```

Add a helper:

```c
static int paramArgValue(
    const ParamProgram *program,
    const ParamProgramToken *tok,
    int idx,
    ParamCx t1,
    ParamCx t2,
    ParamCx p1,
    ParamCx p2,
    ParamCx *out
);
```

`PARAM_OP_CONST` reads arg slot 0 through `paramArgValue()` and pushes the
result.

`PARAM_OP_LEGACY` resolves each legacy arg through `paramArgValue()` before
calling `paramLegacyApply()`.

The single-VM refactor replaces this with normal main-token execution. `const`
with a dynamic expression becomes the expression's lowered main tokens, and
`legacy` with dynamic arguments pops `stack_arg_count` values from the main
stack.

Real-only legacy args must already be compiler-validated. After Stage 5, native
must also carry real/complex per-arg type metadata in its legacy dispatch table
and defensively reject a non-negligible imaginary part for real-only paths. This
metadata is required, not optional, because dynamic legacy args arrive as
complex stack values at runtime.

### Error Handling

During the transitional nested-VM implementation, native errors should include:

- Invalid expression ref.
- Expression stack underflow.
- Expression stack overflow.
- Division by zero.
- Non-finite expression result.
- Unsupported expression opcode.
- Unsupported dynamic arg position.

After the single-VM refactor, native errors should include:

- Hidden legacy arg stack underflow.
- Lowered expression stack overflow.
- Division by zero.
- Non-finite lowered expression result.
- Unsupported main opcode.
- `stack_arg_count` larger than the legacy function arity.

Errors should mention Param Program and the chip/token index when possible.

## UI Design

Implementation target: `index.html`.

### Chip Catalog

Change Param Program `const` from:

```text
const(re, im)
```

to:

```text
const(value)
```

The input should be wide enough for expressions and visually match the wide
expression inputs in the Coeff Program editor.

Suggested placeholder:

```text
1+2j, pi2i, p1, exp(t1*pi2i)
```

Suggested tooltip:

```text
Complex expression. Registers: t1, t2, p1, p2. Constants: pi, pi2, pi2i. Functions: exp, real, imag, abs, mod.
```

### Legacy Argument Fields

For legacy chips:

- Keep `name`, `src`, and `tgt` as dropdowns.
- Render numeric args as expression inputs.
- Render complex args as one wide complex-expression input, not real/imag pairs.
- Render real args as expression inputs with a tooltip saying the expression must
  evaluate real.

`moebius` should show four complex fields: `a`, `b`, `c`, `d`.

`inv_t_plus_2` should show two complex fields rather than four real/imag fields.

### Compatibility Rendering

When an old saved chip has two const arguments:

```json
["const", "1", "2"]
```

the editor should render it as:

```text
const(1+2j)
```

or, if preserving the exact text is safer:

```text
const((1) + (2)j)
```

The in-memory editor model should be canonical after load.

### Read-Only Program Displays

Saved-program modals and other read-only chip displays should render expression
text with the same monospace treatment as the live editor. Long expressions
should wrap inside the chip/card instead of truncating or resizing the modal.

### Draft Editing

The editor must allow invalid partial expressions during typing. It should show
inline diagnostics but preserve the text the user typed.

## File Map

### Compiler

`lambda/param_program_chain.py`

- Add tokenizer/parser for Param scalar expressions.
- Add Param expression bytecode encoder for the transitional nested-VM path.
- Add `lower_expr_to_param_tokens()` for the single-VM target.
- Add greedy constant folding.
- Update `const` lowering so dynamic expressions can become main stack tokens.
- Update legacy arg lowering so dynamic args can become main stack tokens plus
  `stack_arg_count`.
- Accept old `const(re, im)` input.
- Remove selector-aliasing behavior and add fail-closed tests.
- Add lowered expression execution form to compiled output and fingerprint.
- Add diagnostics for expression parse/lower errors.
- Add macro expansion tests with expression-containing macro bodies.

### Registry

`lambda/param_legacy_registry.json`

- Add `type` to every declared numeric arg.
- Use `real` for existing scalar shape parameters.
- Use `complex` for `moebius` coefficients and any future complex args.
- Keep no-arg legacy functions explicitly no-arg.

### Native

`lambda/sweep_cli.c`

- Transitional path: parse `scalar_exprs`, `args_im`, and `expr_refs`.
- Single-VM target: add main opcodes for `PUSH_P1`, `PUSH_P2`, `REAL`,
  `IMAG`, `ABS`, and expression division if not already present.
- Single-VM target: add `stack_arg_count` to `PARAM_OP_LEGACY`.
- Single-VM target: make `PARAM_OP_LEGACY` pop dynamic args from the main stack
  in reverse order and store them in source order.
- Single-VM target: remove `paramEvalScalarExpr()` and `paramArgValue()` from
  newly compiled paths after compatibility tests pass.
- Add defensive bounds checks.

Rebuild all deployed binaries produced from `sweep_cli.c`, including the
coeffgen and preview/debug binaries used by Param Program paths.

### Frontend

`index.html`

- Update `_ppCatalog.const`.
- Add one-field expression editor.
- Make expression inputs wide.
- Normalize old two-field const chips for display/save.
- Update legacy arg rendering according to registry arg types.
- Update tooltips and saved-program display.
- Keep selector/name/id controls as dropdowns or plain id inputs.

### Docs

`param-program.md`

- Replace `const(re, im)` with `const(value)` in the v1 chip table.
- Add a scalar expressions subsection linked from the chip table.
- Document expression-eligible chips.
- Document old saved-form compatibility.
- Add before/after snippets for `const` and at least one legacy chip.

### Tests

`tests/test_param_program_chain.py`

- Compile `const(1+2j)`.
- Compile `const(t1+t2)`.
- Compile `const(p1*2)` after a prior `emit(p1)`.
- Compile division.
- Compile `abs(p1)` and `mod(p1)` to the same opcode.
- Compile real legacy args from real expressions.
- Reject complex expressions for real-only legacy args.
- Compile `moebius` with complex expressions.
- Preserve old `const(re, im)` compatibility.
- Verify fingerprints include the canonical lowered expression execution form.
- Verify macro expansion with expression-containing macro bodies.
- Verify selector aliasing errors remain errors.

`tests/test_param_program_native.py`

- Native parity for `const(t1+t2); emit(p1)`.
- Native parity for `const(p1*2); emit(p2)`.
- Native parity for `/`, `exp`, `real`, `imag`, `abs`, and `mod`.
- Native parity for a legacy chip with a dynamic expression arg.

`tests/docker_runtime_regression.py`

- End-to-end preview or coeffgen path using dynamic Param const expressions.
- End-to-end case with a dynamic legacy arg.
- Multi-thread parity case if the path supports it.

`tests/test_frontend_js.sh`

- Assert Param Program `const` uses one wide expression field.
- Assert old two-field const UI does not appear.
- Assert expression tooltip/register text is present.
- Assert legacy numeric args render as expression inputs.
- Assert selector/name/id controls remain non-expression controls.

## Implementation Stages

### Stage 1: Compiler Only

- Remove selector-aliasing behavior as a hard deliverable. `t1`/`t2` must not
  be accepted as legacy selectors or emit targets, and `p1`/`p2` must not be
  accepted as source-level `push` targets unless the source language is
  explicitly expanded later.
- Add expression parser and initial bytecode output.
- Add arg type metadata to the registry.
- Keep native unchanged for literal-only expressions by compiling them to static
  arg slots.
- Add compiler tests.

Exit criteria:

- Dynamic expressions compile to deterministic payloads.
- Literal-only expressions still execute through the current native path.
- Macro-expanded expression programs fingerprint the same as hand-written
  equivalents.
- Selector-aliasing tests fail closed with clear errors.

### Stage 2: Native Execution

This stage describes the first implementation. It is functional but superseded
by Stage 5 as the preferred architecture.

- Add native expression parser for compiled JSON payloads.
- Add runtime evaluator.
- Wire `PARAM_OP_CONST` and `PARAM_OP_LEGACY` through arg refs.
- Add native parity tests.
- Add a microbenchmark comparing literal arg evaluation vs dynamic expression
  evaluation.

Exit criteria:

- Dynamic expression programs run in preview/debug/compute paths.
- Native row loops do not parse source expression strings.
- Report ns/eval for a representative expression such as `t1+t2` and for a
  literal constant baseline.

First local benchmark:

This benchmark was run locally against the rebuilt `lambda/sweep_test` binary
using `mode=param_dump`, `N=1400`, `1,960,000` rows, and 7 measured repetitions
after one warm-up run. It is an end-to-end ratio check for the native Param
Program path, so it includes output-file writing; it is not an isolated opcode
microbenchmark.

| Case | Tokens | Scalar exprs | Median | ns/row | Delta vs literal |
| --- | ---: | ---: | ---: | ---: | ---: |
| `const(1); emit(p1); const(2); emit(p2)` | 4 | 0 | 93.968 ms | 47.94 | baseline |
| `const(t1+t2); emit(p1); const((t1+t2)*2); emit(p2)` | 4 | 2 | 114.268 ms | 58.30 | +21.6% |
| stack equivalent using `push/add/mul/emit` | 10 | 0 | 109.756 ms | 56.00 | +16.8% |

Interpretation: the nested expression VM adds measurable overhead versus a flat
main-stack equivalent. The cost is bounded, but the flat equivalent is the
better architecture because it uses one interpreter and one stack. Stage 5
should rerun this benchmark after single-VM lowering; `const(t1+t2)` should then
match the stack-equivalent path except for any remaining token-count or branch
differences.

### Stage 3: Frontend

- Replace the Param Program const editor with one expression field.
- Normalize old two-field chips on load.
- Convert legacy numeric arg fields to expression inputs.
- Update read-only chip displays and modal views.
- Add frontend grep tests.

Exit criteria:

- Users can author `const(100j*p1)` naturally from the editor.
- Users can author dynamic legacy args naturally.
- Long expression text does not resize modals or break chip layout.

### Stage 4: Docs And Deploy Gates

- Update `param-program.md`.
- Rebuild native binaries.
- Run the Param Program compiler/native/frontend test gates.
- Run the deploy checklist section that covers rebuilt binaries.

Exit criteria: the deploy artifact contains the updated compiler, UI, and native
VM, and all relevant tests pass.

### Stage 5: Single-VM Refactor

Goal: remove the nested scalar-expression VM from newly compiled Param Programs.
Expressions should lower into the main Param Program token stream.

Compiler steps:

- Keep the expression parser and AST.
- Replace expression-bytecode emission with `lower_expr_to_param_tokens(ast)`.
- Lower `const(expr)` directly to main stack tokens.
- Add compiler-internal lowering for `p1` and `p2` reads using
  `PUSH_P1` / `PUSH_P2`.
- Lower legacy dynamic args left-to-right immediately before the legacy token.
- If any legacy arg is dynamic, lower all args for that legacy token onto the
  stack and set `stack_arg_count = n_args`.
- Keep static literal-only legacy args in `args` / `args_im` with
  `stack_arg_count = 0` so legacy fast-path detection remains possible.
- Remove `scalar_exprs` and `expr_refs` from new fingerprints. The fingerprint
  must hash the expanded main-token stream instead.

Native steps:

- Add main VM opcodes needed by expression lowering:
  `PUSH_P1`, `PUSH_P2`, `REAL`, `IMAG`, `ABS`, and any missing arithmetic op.
- Add `stack_arg_count` to `ParamProgramToken`.
- Add required per-arg real/complex metadata to the native legacy dispatch
  table.
- In `PARAM_OP_LEGACY`, pop `stack_arg_count` values from the main stack before
  reading `src`.
- Preserve argument order by popping reverse and storing source-order args.
- Keep the old `scalar_exprs` parser temporarily for compatibility with
  already-created in-flight Step Functions executions and any old compiled
  payloads.
- Remove `paramEvalScalarExpr()` and dynamic `expr_refs` handling only in a
  separate cleanup deploy after a compatibility window has passed and no active
  workflows can reference the old payload shape.

Migration plan:

- Deploy A: native accepts both nested-VM payloads and single-VM payloads;
  writers may still emit the old shape.
- Deploy B: compiler/UI/handlers emit only single-VM payloads; native still
  accepts both.
- Deploy C: after the maximum Step Functions execution lifetime plus a safety
  margin, remove the old nested-VM parser and tests for old in-flight payloads.

Diagnostics and UI steps:

- The user-facing chip display remains compact:
  `legacy(rtheta, both, both, p1+p2)`.
- The debug view should optionally show the lowered token stream:
  `push(p1); push(p2); add; legacy(..., stack_arg_count=1)`.
- Stack diagnostics must account for hidden expression tokens and hidden legacy
  arg pops.
- A chip with expression args may be visually stack-neutral while internally
  using temporary stack space.

Tests:

- Assert `const(t1+t2)` compiles to the same main-token sequence as
  `push(t1); push(t2); add`.
- Assert `legacy(rtheta, both, both, p1+p2)` lowers to
  `push(p1); push(p2); add; legacy(..., stack_arg_count=1)`.
- Assert multi-arg lowering preserves order for `moebius(a,b,c,d)`.
- Assert `moebius(1, 0, 0, t2)` uses all-or-none stack args when any arg is
  dynamic.
- Assert no `scalar_exprs` field appears in newly compiled payloads.
- Native parity: nested-expression payloads and single-VM payloads produce the
  same `p1`/`p2` for representative programs during the transition.
- Native parity: real-only legacy args with an imaginary dynamic value reject
  through the native dispatch metadata.
- Migration: native accepts both old nested-VM payloads and new single-VM
  payloads during Deploy A/B.
- Benchmark again and compare dynamic `const(t1+t2)` against the flat
  stack-equivalent case. The expected result is that they become effectively the
  same path.
- Re-benchmark the static legacy fast path against the VM path before preserving
  dispatchPt routing as an optimization.

## Concrete Stage 5 File Map

This section is the implementation ticket for the single-VM refactor. It assumes
Stages 1-4 already landed and the current code still emits `scalar_exprs` /
`expr_refs`.

### Implementation Order

Use this order. Do not start by deleting the nested VM.

1. Add native compatibility first: `sweep_cli.c` accepts both old nested-VM
   payloads and new single-VM payloads.
2. Add compiler single-VM output behind the existing
   `compile_param_program_chain()` API.
3. Update handlers to stop forwarding `scalar_exprs` for new compiled payloads.
4. Update debug/metadata/tests to expose the lowered token stream.
5. Rebuild native binaries and run parity tests against both payload formats.
6. Deploy with both native readers enabled.
7. Only after the compatibility window, remove the old nested reader/evaluator.

### `lambda/param_program_chain.py`

Keep this module as the single source of truth for source validation, macro
expansion, lowering, stack validation, and fingerprinting.

Constants:

- Keep all existing opcode numbers stable. Current source check:
  `lambda/param_program_chain.py` and `lambda/sweep_cli.c` define Param Program
  opcodes 1-21 only, with `PARAM_OP_LEGACY = 21`. IDs 22-26 are free in the
  current checkout.
- Add new compiler/native opcodes after 21:

```python
PARAM_OP_PUSH_P1 = 22
PARAM_OP_PUSH_P2 = 23
PARAM_OP_REAL = 24
PARAM_OP_IMAG = 25
PARAM_OP_ABS = 26
```

- Add those names to `_OP_NAMES`.
- Keep `EXPR_*` constants only while the nested-VM compatibility path exists.
  New fingerprints must not depend on `EXPR_*`.
- Add `MAX_LOWERED_PARAM_TOKENS_PER_EXPR = 32` as the per-expression lowering
  cap. Use this new name in new Stage 5 code so it is not confused with the old
  expression-bytecode limit.
- Keep `MAX_SCALAR_EXPR_TOKENS = 32` only as a compatibility alias while the
  nested-VM tests and payload reader still exist.
- Keep `MAX_SCALAR_EXPRS = 64` only for old nested payload compatibility.

Parser:

- Keep `_compile_expr()` and the expression parser.
- Keep greedy constant folding.
- Keep the existing error labels such as `legacy(rtheta) arg 0`.
- Do not emit `_flatten_expr()` for new payloads.
- Keep `_flatten_expr()` only for tests or compatibility helpers until Deploy C.

New lowering helper:

Add:

```python
def _lower_expr_to_param_tokens(expr, *, label):
    ...
```

Required mapping:

```text
literal(re, im) -> PARAM_OP_CONST with a=re, b=im, n_args omitted
t1              -> PARAM_OP_PUSH_T1
t2              -> PARAM_OP_PUSH_T2
p1              -> PARAM_OP_PUSH_P1
p2              -> PARAM_OP_PUSH_P2
add             -> PARAM_OP_ADD
sub             -> PARAM_OP_SUBTRACT
mul             -> PARAM_OP_MUL
div             -> PARAM_OP_RATIO
neg             -> PARAM_OP_NEGATE
exp             -> PARAM_OP_EXP
real            -> PARAM_OP_REAL
imag            -> PARAM_OP_IMAG
abs/mod         -> PARAM_OP_ABS
```

`_lower_expr_to_param_tokens()` must reject lowered expressions longer than
`MAX_LOWERED_PARAM_TOKENS_PER_EXPR`. It must return ordinary `_token(...)`
dictionaries.

`const` lowering:

- In `_lower_chip()`, replace the current `const` branch that calls
  `_add_arg_expr()`.
- For `const(value)`, compile the expression.
- If the expression is a folded literal, emit:

```python
_token(PARAM_OP_CONST, a=re, b=im)
```

- If the expression is dynamic, emit `_lower_expr_to_param_tokens(expr,
  label="const value")`.
- Keep accepting old `["const", re, im]` source by converting it to
  `({re})+({im})*1j` before compiling.
- Do not emit `PARAM_OP_CONST` with `expr_refs` in new payloads.

Legacy arg lowering:

- Replace `_add_arg_expr()` use in `_legacy_args()`.
- `_legacy_args()` should compile every logical arg to an expression object and
  return:

```python
{
    "static_args": [re0, re1, ...],
    "static_args_im": [im0, im1, ...],
    "arg_tokens": [token, ...],
    "stack_arg_count": n,
    "n_args": n,
}
```

- If every arg folds to a literal, use `static_args`, `static_args_im`, and
  `stack_arg_count = 0`.
- If any arg is dynamic, lower all args left-to-right into `arg_tokens`, use
  `stack_arg_count = n_args`, and do not rely on `expr_refs`.
- For static args inside a dynamic arg list, lower them as
  `PARAM_OP_CONST(a=..., b=...)` so argument order is uniform.
- `_legacy_token()` must prepend `arg_tokens` before the legacy token.
- The legacy token must include:

```python
_token(
    PARAM_OP_LEGACY,
    fn_index=spec["fn_index"],
    src=src_val,
    tgt=tgt_val,
    n_args=n_args,
    args=static_args,
    args_im=static_args_im,
    stack_arg_count=stack_arg_count,
)
```

- `_token()` must accept and serialize `stack_arg_count` when nonzero.
- `_token()` must stop serializing `expr_refs` for new payloads.

Special legacy arg forms:

- `moebius` canonical source remains four complex args: `a`, `b`, `c`, `d`.
- `moebius` old eight-real-component source remains accepted forever.
- If old eight-component `moebius` has any dynamic component, lower all eight
  components to stack args and set `n_args = stack_arg_count = 8`.
- If canonical four-arg `moebius` has any dynamic coefficient, lower all four
  coefficients to stack args and set `n_args = stack_arg_count = 4`.
- `inv_t_plus_2` canonical source remains two complex args.
- `inv_t_plus_2` old four-real-component source remains accepted forever.
- No-arg legacy functions must still reject any args with
  `legacy(<name>) takes no arguments`.

Stack validation:

- Update `_validate_stack()` so these ops push one value:

```text
PARAM_OP_PUSH_T1
PARAM_OP_PUSH_T2
PARAM_OP_PUSH_P1
PARAM_OP_PUSH_P2
PARAM_OP_CONST
```

- Update `_validate_stack()` so these ops require one value and leave one value:

```text
PARAM_OP_NEGATE
PARAM_OP_CONJ
PARAM_OP_RECIPROCAL
PARAM_OP_UNIT_CIRCLE
PARAM_OP_SQUARE
PARAM_OP_CUBE
PARAM_OP_EXP
PARAM_OP_REAL
PARAM_OP_IMAG
PARAM_OP_ABS
```

- For `PARAM_OP_LEGACY`, first subtract `stack_arg_count`, then apply the
  existing `src` pop behavior, then apply the existing `tgt` push behavior.
- Error text must include token index and whether the underflow came from hidden
  legacy args or from `src=pop1/pop2`.
- Enforce `MAX_STACK` after every lowered expression token and after the legacy
  token.

Fingerprinting:

- Change `_execution_spec()` to hash only the main token stream for new
  payloads:

```python
{"tokens": tokens}
```

- Do not include `scalar_exprs` in new fingerprints.
- Do not include source expression text in fingerprints.
- Keep `display` source-like and readable; do not replace display text with the
  lowered token stream.
- Add `execution_format: "single_vm"` to the compiled metadata if useful for
  tests/debugging, but native execution must be determined by the token payload,
  not by source strings.

Fast path:

- Current source check: `_legacy_fast_path()` already returns true only when
  every token is `PARAM_OP_LEGACY`, has no dynamic `expr_refs`, and has
  `src == PARAM_SEL_BOTH` and `tgt == PARAM_SEL_BOTH`.
- Stage 5 should preserve that shape and add only `stack_arg_count == 0` as the
  replacement for the old no-dynamic-`expr_refs` condition.
- Remove the `expr_refs` check after new payloads stop emitting `expr_refs`.
- Re-benchmark before using this as a performance optimization. If `dispatchPt`
  is slower than the VM, keep `legacy_transforms` empty in Program mode and use
  the VM even for literal legacy-equivalent chains.

Stage 5 exit criterion for `legacy_transforms`:

- Either preserve the fast path with a short checked-in rationale in
  `param-program.md` explaining why exact legacy dispatch is still needed, or
  remove `legacy_transforms` from Program mode entirely and always run compiled
  Param Programs through the VM.
- Do not leave the fast path as ambient dead/uncertain behavior.

Compiled return shape:

- `compile_param_program_chain()` should continue returning existing public keys
  used by handlers:

```text
version
fingerprint
display
stack_max
token_count
uses_legacy_fast_path
legacy_transforms
tokens
source_chain
expanded_chain
diagnostics
```

- Remove `scalar_exprs` from the new return shape, or return `scalar_exprs: []`
  only during Deploy A/B for backwards-compatible handler code.
- Add `lowered_display` or `debug_lowered_tokens` for debug UI if needed. This
  must be derived from tokens and must not participate in identity.
- Add `lowered_to_source` for debug/error mapping. It must be an array parallel
  to the lowered token stream:

```python
[
    {"source_chip": 0, "source_arg": None},
    {"source_chip": 1, "source_arg": "p", "source_arg_index": 0},
]
```

- `lowered_to_source` is debug metadata only. It must not be included in
  `_execution_spec()`, fingerprints, probe signatures, or cache keys.

Diagnostics:

- Current source check: `compile_param_program_diagnostics()` currently returns
  bare diagnostics like `{"level": "error", "message": "..."}`. Stage 5 needs a
  richer schema.
- New diagnostic entries should use this shape when source mapping is known:

```python
{
    "level": "error",
    "message": "legacy(rtheta) hidden arg underflow",
    "source_chip": 4,
    "arg_index": 0,
    "arg_name": "p",
    "lowered_token_start": 12,
    "lowered_token_end": 15,
}
```

- Keep `level` and `message` mandatory for backwards compatibility.
- The index fields are optional for errors that happen before lowering, but any
  post-lowering validation/runtime diagnostic should include them.
- `compile_param_program_diagnostics()` must report errors against source chip
  indices, not lowered token indices, when possible.
- For expression args, diagnostics should include both:

```text
source chip index
arg index/name
lowered token range
```

- For example:

```text
param expression at chip 4 arg p: division by zero
legacy(rtheta) hidden arg underflow at lowered token 12
```

- Runtime errors that can only report a lowered token index should be mapped
  back through `lowered_to_source` in debug/preview handlers before being shown
  in the UI.

### `lambda/param_legacy_registry.json`

Keep the registry as the Python compiler source of truth.

Required changes:

- Ensure every numeric arg has a `type`.
- Use `real` for existing shape parameters.
- Use `complex` for canonical `moebius` and canonical `inv_t_plus_2` arg specs
  if those special cases are moved into registry metadata later.
- No-arg functions must keep `"args": []`.
- Do not add `t1`/`t2` selectors to `allowed_src`; source register access happens
  through expression lowering or `push(t1)/push(t2)`, not legacy selectors.

Do not rely on this JSON at native runtime. The native side needs its own
generated or hand-maintained real/complex arg metadata table for defensive
runtime checks.

Drift control:

- Prefer generating the native real/complex metadata table from
  `param_legacy_registry.json` at build time.
- If generation is not done in Stage 5, add a test that loads
  `param_legacy_registry.json` and asserts the native table's fn-index, arity,
  and arg-type rows match the JSON plus the documented special cases for
  `moebius` and `inv_t_plus_2`.
- This test is required because Python compile-time validation and C runtime
  validation must not silently diverge.

### `lambda/sweep_cli.c`

Keep all hot-loop execution string-free.

Constants:

- Add to `enum ParamProgramOp` after `PARAM_OP_LEGACY = 21`:

```c
PARAM_OP_PUSH_P1 = 22,
PARAM_OP_PUSH_P2 = 23,
PARAM_OP_REAL = 24,
PARAM_OP_IMAG = 25,
PARAM_OP_ABS = 26
```

- Keep old `enum ParamProgramExprOp` until Deploy C, then delete it.
- Keep `PARAM_PROGRAM_MAX_SCALAR_EXPRS`, `PARAM_PROGRAM_EXPR_STRIDE`, and
  `PARAM_PROGRAM_MAX_EXPR_NUMS` until Deploy C, then delete them.

`ParamProgramToken`:

- Replace the current `reserved` byte with `stack_arg_count`:

```c
typedef struct {
    uint16_t op;
    uint16_t fn_index;
    uint8_t src;
    uint8_t tgt;
    uint8_t n_args;
    uint8_t stack_arg_count;
    double a;
    double b;
    double args[PARAM_PROGRAM_MAX_ARGS];
    double args_im[PARAM_PROGRAM_MAX_ARGS];
    int expr_refs[PARAM_PROGRAM_MAX_ARGS]; /* Deploy A/B only */
} ParamProgramToken;
```

- During Deploy A/B, keep `expr_refs` so old payloads run.
- In Deploy C, remove `expr_refs` from the struct if no old payloads can arrive.

Parser:

- Update `parseParamProgramTokenObject()` to read `"stack_arg_count"`.
- Validate `0 <= stack_arg_count <= n_args`.
- Validate `stack_arg_count == 0` or `stack_arg_count == n_args` for Stage 5.
- Current source check: `PARAM_OP_CONST` already supports all Deploy A/B literal
  shapes through `sweep_cli.c:paramArgValue()` and the existing `n_args == 0`
  branch. Stage 5 must preserve this behavior:

```text
n_args == 0:
  read literal from tok->a / tok->b
n_args == 1 and expr_refs[0] >= 0:
  old nested expression path via paramArgValue()
n_args == 1 and expr_refs[0] == -1:
  old static arg path using args[0] / args_im[0]
```

- New Stage 5 compiler output should use `n_args == 0` plus `a` / `b` for
  literal constants.
- Keep parsing `"expr_refs"` during Deploy A/B.
- Keep parsing top-level `"scalar_exprs"` during Deploy A/B.
- Do not require `"scalar_exprs"` for new payloads.

Native legacy arg type metadata:

- Add a native table or helper near `paramLegacyApply()`:

```c
typedef enum {
    PARAM_ARG_REAL = 1,
    PARAM_ARG_COMPLEX = 2
} ParamLegacyArgType;

static int paramLegacyArgType(int fnIndex, int nArgs, int argIndex);
```

- For registry-declared real args, return `PARAM_ARG_REAL`.
- For canonical four-arg `moebius`, return `PARAM_ARG_COMPLEX`.
- For old eight-component `moebius`, return `PARAM_ARG_REAL`.
- For canonical two-arg `inv_t_plus_2`, return `PARAM_ARG_COMPLEX`.
- For old four-component `inv_t_plus_2`, return `PARAM_ARG_REAL`.
- For no-arg functions, reject `nArgs > 0`.
- Before calling the switch body in `paramLegacyApply()`, validate each arg:

```c
if (type == PARAM_ARG_REAL && fabs(args[i].i) > 1e-12) {
    fprintf(stderr, "param legacy fn_index=%d arg %d must be real-valued\n", fnIndex, i);
    return 1;
}
```

Runtime evaluator:

- In `paramEvalProgram()`, add cases:

```c
case PARAM_OP_PUSH_P1: push p1
case PARAM_OP_PUSH_P2: push p2
case PARAM_OP_REAL: stack[top] = (real(stack[top]), 0)
case PARAM_OP_IMAG: stack[top] = (imag(stack[top]), 0)
case PARAM_OP_ABS:  stack[top] = (hypot(real, imag), 0)
```

- Keep `PARAM_OP_CONST` literal path:

```c
paramPush(stack, &sp, param_cx(tok->a, tok->b))
```

- During Deploy A/B only, if `PARAM_OP_CONST` has `n_args > 0` and `expr_refs[0]
  >= 0`, keep allowing the old `paramArgValue()` path.
- During Deploy A/B only, if `PARAM_OP_CONST` has `n_args == 1` and
  `expr_refs[0] == -1`, keep pushing `args[0]` / `args_im[0]` for old static
  payloads.
- After Deploy C, `PARAM_OP_CONST` must be literal-only.

Legacy execution order:

- In `PARAM_OP_LEGACY`, pop hidden stack args before reading `src`.
- Required order:

```c
ParamCx resolvedArgs[PARAM_PROGRAM_MAX_ARGS];
int sac = tok->stack_arg_count;
if (sac > tok->n_args || sac > sp) return 1;
for (int ai = sac - 1; ai >= 0; ai--) {
    resolvedArgs[ai] = stack[--sp];
}
if (sac == 0) {
    for (int ai = 0; ai < tok->n_args; ai++) {
        resolvedArgs[ai] = param_cx(tok->args[ai], tok->args_im[ai]);
    }
}
```

- Then apply existing `src` handling:

```text
p1/p2/both read registers
pop1/pop2 pop user stack values that are now below hidden args
```

- Then call `paramLegacyApply()`.
- Then apply existing `tgt` behavior.

Sanitization:

- Sanitize all pushed values, not just the final top stack slot, when a token
  pushes more than one value.
- After `PARAM_OP_LEGACY` with `tgt=push2`, sanitize both pushed outputs.
- After hidden args are popped, do not sanitize discarded hidden arg slots.

Nested VM compatibility:

- Keep `paramEvalScalarExpr()` and `paramArgValue()` only during Deploy A/B.
- Gate their use narrowly:

```text
if token has expr_refs >= 0, use old path
else use single-VM literal/stack_arg path
```

- Add comments marking this as temporary compatibility code with the planned
  Deploy C removal.

### Handlers And Payload Assembly

The compiler API should hide the payload-format change from most handlers, but
current handlers explicitly forward `scalar_exprs`. Update every one.

`lambda/handler_compute_plan.py`

- At the top helper level, add a `_compiled_param_program_payload(compiled)`
  helper mirroring `_compiled_coeff_program_payload()`.
- The helper should include:

```python
version
fingerprint
display
stack_max
token_count
uses_legacy_fast_path
tokens
```

- Include `scalar_exprs` only if `compiled.get("scalar_exprs")` is non-empty
  during Deploy A/B.
- Replace the manual payload at the current `param_program = { ... }` site with
  the helper.
- Keep `param_program_fingerprint` in the pipeline metadata.
- Keep `build_probe_signature()` input unchanged except that the fingerprint now
  reflects single-VM lowering.

`lambda/handler_compute_preview.py`

- Add the same `_compiled_param_program_payload()` helper or import/share one if
  a shared helper is introduced.
- Replace the manual payload containing `"scalar_exprs"` in the preview compile
  path.
- Ensure response metadata still reports `token_count`, `stack_max`, and
  `fingerprint`.
- Add debug response support for `lowered_display` if the compiler returns it.

`lambda/handler_coeffgen.py`

- Update the degree-probe and coeffgen spec assembly paths that currently put
  `"scalar_exprs"` into `param_program`.
- Ensure direct `params["param_program"]` pass-through still works for Deploy
  A/B old payloads.
- When compiling from `param_program_chain`, emit the new single-VM payload.

`lambda/handler_param_debug.py`

- Replace the manual payload containing `"scalar_exprs"`.
- Add output fields for debugging hidden args:

```text
param_program.tokens
param_program.lowered_display
param_program.stack_max
param_program.execution_format
```

- The debug endpoint must be able to evaluate both old and new payload shapes
  during Deploy A/B.

`lambda/compute_fused.py`

- No payload-shape parsing should be added here.
- Confirm `build_probe_signature()` continues to hash only
  `param_program_fingerprint`, not raw token payloads.
- Add a test that the Stage 5 fingerprint change changes the probe signature.

Workflow/cache fingerprint trace:

- `lambda/compute_fused.py:build_probe_signature()` reads
  `param_program["fingerprint"]`.
- `lambda/handler_compute_plan.py` stores that value in
  `pipeline.param_program_fingerprint`.
- `tests/test_compute_plan.py` already checks that changing
  `param_program_fingerprint` changes the probe signature.
- `tests/test_compute_workflow_definition.py` checks Step Functions passes the
  opaque `$.plan.pipeline.param_program` object through to downstream tasks.
- Stage 5 must grep the workflow and handlers for any additional direct
  fingerprint/hash usage before deploy. The only allowed routing/cache input is
  the compiler-produced fingerprint, not `scalar_exprs` or raw source text.

`lambda/handler_storage.py`

- Saved Param Programs should keep source chains, not compiled token payloads.
- Save/fetch/list routes do not need to persist `tokens`.
- Re-run save-time validation so macro programs compile under the Stage 5
  single-VM compiler.
- If returned program summaries include `compiled` metadata, ensure they do not
  expose stale `scalar_exprs` as if it were source.

### Workflow And State Payloads

`stepfunctions/compute_workflow.asl.json.template`

- Keep passing `$.plan.pipeline.param_program` through the workflow.
- No Step Functions field should directly reference `scalar_exprs`.
- During Deploy A/B, old executions may still contain `scalar_exprs` inside the
  opaque `param_program` object. Native must accept that.
- Coeffgen workflow payloads must pass the source grid size as `N` so
  `coeffgen_chunked` can reconstruct Coeff Program `t1/t2` from
  `source_step_start` plus `source_n1/source_n2` without changing `params.bin`.
- Current workflow references to verify:

```text
stepfunctions/compute_workflow.asl.json.template:105 param_program_chain.$
stepfunctions/compute_workflow.asl.json.template:210 param_program.$
stepfunctions/compute_workflow.asl.json.template:392 param_program.$
stepfunctions/compute_workflow.asl.json.template:549 param_program.$
```

Coeff Program `t1/t2` source metadata is not part of the Param Program payload,
but it is part of the same compute workflow contract:

```text
handler_coeffgen.py: coeffgen_chunked spec includes source_step_start=params_step_start and source_n1/source_n2=N
handler_compute_chunk_fused.py: fused coeffgen local spec includes source_step_start=chunk step_start and source_n1/source_n2=N
handler_render_lores_preview.py: lores coeffgen spec includes source_step_start=0 and source_n1/source_n2=view_N
stepfunctions/compute_workflow.asl.json.template: coeffgen payload forwards N for hires and lores coeffgen
sweep_cli.c: coeffgenSourceParamsForStep reconstructs source t1/t2 from the global serpentine step
```

If a direct `coeffgen_chunked` caller omits `source_n1/source_n2`, native falls
back to using the transformed params record for `t1/t2` for backward
compatibility. That fallback is not the intended behavior for UI-driven compute
jobs; the handlers should always provide the source metadata.

`calc.json` / compute artifact metadata:

- Keep:

```text
pipeline.param_program_chain
pipeline.param_program
pipeline.param_program_display
pipeline.param_program_fingerprint
pipeline.param_program_uses_legacy_fast_path
```

- New `pipeline.param_program` should contain no `scalar_exprs`.
- If adding `execution_format`, store it under `pipeline.param_program`, not as a
  separate top-level artifact field.

### Frontend `index.html`

Most source-facing UI stays unchanged because users should still see compact
chips such as `const(t1+t2)` and `legacy(rtheta,both,both,p1+p2)`.

Required changes:

- Do not add user-facing `push(p1)` or `push(p2)` chips in `_ppCatalog`.
- Keep `_ppCatalog.const` as one wide expression field.
- Keep legacy numeric args as expression fields.
- Add optional lowered-token display in debug views, not in normal chip strips.
- If using `lowered_display`, render it in the Compute Debug tab under a
  collapsible "Lowered Param Program" section.
- Ensure wide input rules from `ui_docs/style_guide.md` remain satisfied for
  expression fields.
- Current grep result: `scalar_exprs` does not appear in `index.html` or
  `tests/test_frontend_js.sh`. No frontend `scalar_exprs` cleanup is needed
  unless Stage 5 adds lowered-debug UI assertions.
- Backend `scalar_exprs` cleanup sites from the current grep:

```text
lambda/handler_compute_plan.py:52,218
lambda/handler_compute_preview.py:113,312
lambda/handler_coeffgen.py:51,614
lambda/handler_param_debug.py:153
```

Do not change the saved source-chain format for this refactor.

### Tests

`tests/test_param_program_chain.py`

Add or update:

- `const(t1+t2)` returns tokens equivalent to `push(t1); push(t2); add`.
- `const((1)+(2)*1j)` folds to a literal `PARAM_OP_CONST` with `a=1`, `b=2`.
- `const(t1+(1+2))` folds the literal subtree.
- `const(t1+1+2)` is allowed to lower differently from `const(t1+(1+2))`; do
  not assert identical fingerprints for this case.
- `legacy(rtheta,both,both,p1+p2)` lowers to
  `PUSH_P1`, `PUSH_P2`, `ADD`, `LEGACY stack_arg_count=1`.
- `legacy(moebius,both,both,1,0,0,t2)` lowers all four args to stack args.
- `lowered_to_source` maps every lowered token back to the source chip and arg
  where applicable, and it is not part of the fingerprint.
- Old eight-real `moebius` source still compiles.
- Old four-real `inv_t_plus_2` source still compiles.
- `legacy(roots6,both,both,1)` still rejects because roots6 takes no args.
- `push(p1)` and `emit(t1)` reject.
- New fingerprints do not include `scalar_exprs`.
- Macro containing `const(t1+t2)` expands to the same tokens as hand-written
  source.

`tests/test_param_program_native.py`

Add or update:

- Native parity for single-VM `const(t1+t2)`.
- Native parity for single-VM `legacy(rtheta,both,both,p1+p2)`.
- Native parity for `moebius` dynamic arg order.
- Native rejection for real-only arg with imaginary value.
- Native metadata table matches `param_legacy_registry.json` plus documented
  special cases, or a generated-table test proves the generated C data came from
  the JSON.
- Native compatibility test for old nested `scalar_exprs` payload during Deploy
  A/B.
- Native compatibility test for old static `PARAM_OP_CONST` payload with
  `n_args == 1`, `args[0]`, `args_im[0]`, and no dynamic `expr_refs`.
- Native test that `PARAM_OP_REAL`, `PARAM_OP_IMAG`, and `PARAM_OP_ABS` match
  Python reference values.

`tests/test_compute_plan.py`

- Assert compiled `pipeline.param_program` from source chain has no
  `scalar_exprs` for new payloads.
- Assert `param_program_fingerprint` changes for Stage 5 lowering compared with
  the old nested execution spec if a fixture is retained.
- Assert Program mode still zeroes inactive legacy chains unless
  `legacy_transforms` is intentionally used.
- Assert the final Stage 5 choice for `legacy_transforms`: either the fast path
  is intentionally preserved with documented rationale, or Program mode always
  emits a VM payload and `param_program_uses_legacy_fast_path` is false.

`tests/test_compute_preview_handler.py`

- Assert preview compiles and forwards single-VM payloads.
- Assert preview response still reports `token_count`, `stack_max`, and
  `fingerprint`.
- Assert dynamic legacy args work in preview.

`tests/test_coeffgen_param_gen.py`

- Assert coeffgen accepts single-VM `param_program`.
- Assert coeffgen still accepts old nested payload during Deploy A/B if direct
  payload pass-through is supported.

`tests/test_deploy_packaging.py`

- Keep asserting `param_program_chain.py` and `param_legacy_registry.json` are
  packaged with every Lambda that imports/uses the compiler.
- Add a packaging assertion for any new shared helper module if one is created.
- Add predeploy checklist assertions for the Stage 5 tests.

`tests/test_frontend_js.sh`

- Keep checks for one-field Param Program `const`.
- Keep checks for wide expression inputs.
- Add checks that `_ppCatalog` does not expose `push(p1)` / `push(p2)`.
- Add checks for lowered debug display only if implemented.

`tests/docker_runtime_regression.py`

- Add end-to-end native run for:

```text
param_program: const(t1+t2); emit(p1); const((t1+t2)*2); emit(p2)
coeff_function: const or a small stable function
```

- Add end-to-end run for:

```text
legacy(rtheta,both,both,p1+p2)
```

- Add multi-thread parity run if the existing Param Program path has one.

### Benchmarks

Re-run the local benchmark after Stage 5:

```text
literal const
const(t1+t2)
manual stack equivalent
legacy(rtheta,both,both,p1+p2)
static literal legacy-equivalent chain through VM
static literal legacy-equivalent chain through dispatchPt
```

Expected result:

- `const(t1+t2)` and manual stack equivalent should be effectively the same
  path.
- If dispatchPt remains slower than VM for legacy-equivalent chains, stop using
  `legacy_transforms` as a performance fast path in Program mode. Keep it only
  if exact legacy parity is needed and explicitly documented.

Record the numbers in this document before deploy.

### Command Gates

Minimum local gates before deploy:

```bash
python3 -m py_compile lambda/param_program_chain.py lambda/handler_compute_plan.py lambda/handler_compute_preview.py lambda/handler_coeffgen.py lambda/handler_param_debug.py
cc -O2 -I lambda -c lambda/sweep_cli.c -o /tmp/sweep_cli.o
cc -O2 -pthread -o lambda/sweep_test lambda/sweep_cli.c -lm
uv run python -m pytest tests/test_param_program_chain.py tests/test_param_program_native.py tests/test_compute_plan.py tests/test_compute_preview_handler.py tests/test_coeffgen_param_gen.py tests/test_deploy_packaging.py
bash tests/test_frontend_js.sh
bash scripts/predeploy_check.sh
```

If `uv` cache permissions fail locally, rerun the same command with the project
approved escalation pattern. Do not skip the native parity tests.

## Risks

### Runtime Cost

The nested expression VM adds work only when an expression-backed argument is
resolved, but it still creates a second dispatch loop and a second stack. The
single-VM refactor removes that layer by lowering expressions into ordinary
Param Program tokens. Runtime cost after the refactor should be comparable to
the manually-written stack equivalent.

### Register Meaning

The biggest semantic footgun is `p1` and `p2`. They are not aliases for `t1` and
`t2`; they are current mutable registers. This must be explicit in tooltips and
docs.

### Real Vs Complex Args

Legacy functions often expect real scalar shape parameters. The compiler must
reject complex expressions for real-only fields unless the user explicitly
projects a real value.

### Compatibility

Old `const(re, im)` chains must continue to load and execute. The UI may rewrite
them on load/save, but the compiler must accept them permanently.

### Scope Creep

Do not add stack reads, coefficient reads, arbitrary functions, or expression
selectors in v1. The feature is useful with registers, arithmetic, and dynamic
numeric legacy args.

## Opinion

This is worth doing, and the review points are correct. The right scope is not
only `const(value)`; it is all numeric Param Program argument fields. The safe
implementation path is to keep the expression parser and greedy constant
folding, but lower expressions into the main Param Program VM instead of keeping
a nested scalar-expression VM. The compiler must keep selectors strictly
separate from expression identifiers, and the native row loop should have one
interpreter, one stack, and no string parsing.
