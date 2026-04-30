# Coeff Program Source Syntax: Review And Proposed Cleanup

## Purpose

The Coeff Program source format lets users author the same coefficient
generators that today are hard-coded as native C functions like `poly_1`,
`poly_2`, etc. A working concrete example below reproduces `poly_1` exactly
using the chip set already shipped today.

The current syntax shipped over several iterations and the seams show. This
doc:

1. Lays out the running example.
2. Catalogs every syntactic decision in the current source format.
3. Names the inconsistencies users hit when they hand-author programs.
4. Proposes a cleaner surface syntax that compiles to the same payload, plus
   a small set of new chips that close real gaps.

This is a design doc, not an implementation ticket. Compatibility with the
existing saved-form remains a hard requirement: any reform must accept old
saved programs and round-trip them through the new editor.

The reform is split into v1 (ship first; lowers entirely onto existing VM
tokens) and v2 (real compiler work, new opcodes, new diagnostics). Every
proposal in this doc is tagged accordingly.

## Long-Term Architecture: Shared Typed VM

The larger goal is not just nicer coefficient-program syntax. The larger
goal is to stop growing separate program machines that happen to look similar.
Param Program and Coeff Program should eventually share one typed VM core with
different execution profiles.

The VM value model should be explicit:

```text
scalar_complex   # one complex value
vector_complex   # bounded complex vector, length <= COEFF_PROGRAM_MAX_VECTOR_LEN
```

The program kind selects the register set, allowed operations, and required
outputs:

```text
profile: param
  readable scalar registers:  t1, t2
  writable scalar registers:  p1, p2
  readable vector registers:  none
  writable vector registers:  none
  required output contract:   p1 and p2 assigned
  typical ops:                scalar arithmetic, scalar functions, assignment

profile: coeff
  readable scalar registers:  t1, t2, p1, p2, poly_len
  readable vector registers:  cf, poly
  writable vector registers:  poly
  required output contract:   poly assigned or intentionally left as initial cf
  typical ops:                scalar arithmetic, vector construction,
                              vector arithmetic, vector assignment
```

The compiler should enforce these profiles. A Param Program must not read
`poly` or `cf`; a Coeff Program must not write `p1` or `p2`. The syntax can
be shared, but the capability set and output contract remain program-kind
specific.

Param Program already proved the desired direction: expression fields should
lower into the main VM token stream, not into a separate expression
interpreter. Coeff Program should converge on the same principle, with typed
stack values instead of scalar-only stack values. A coefficient expression
such as `fill(poly_len, p1*p2)` should compile to ordinary typed VM work:
compute a scalar `p1*p2`, then construct a vector from that scalar. A vector
expression such as `mul(poly, cf)` should compile to typed vector operations
using the same dispatch machinery.

This does not mean rewriting Param and Coeff together. The pragmatic order is:

1. Ship Coeff Program text source v1 by lowering source text to the existing
   chip-list form and existing native opcodes.
2. Design and implement the typed VM core in the Coeff path, where vector
   values make the need concrete.
3. Keep the current Param Program path stable while Coeff validates the typed
   VM design.
4. Migrate Param Program onto the same parser/typed-VM backend as a second
   profile only after the Coeff implementation has proven the model.

The hot-loop rule does not change: no string parsing, no string lookup, and no
heap allocation per row. Source text lowers before execution to typed tokens,
numeric function ids, register ids, and fixed-size stack operations.

## Running Example

Today's exact source for `poly_1` as a Coeff Program:

```text
push_range(poly_len)
legacy(linear, pop, push, 1, 1)
legacy(linear, pop, push, 0.5, 0)
push_const(poly_len, p1)
power(push, pop, pop)
legacy(sin, pop, push)
push_range(poly_len)
legacy(linear, pop, push, 1, 1)
legacy(linear, pop, push, 0.3333333333333333, 0)
push_const(poly_len, p2)
power(push, pop, pop)
legacy(cos, pop, push)
multiply(push, pop, pop)
push_range(poly_len)
legacy(linear, pop, push, 1, 1)
duplicate
multiply(push, pop, pop)
push_const(poly_len, p1*p2)
multiply(push, pop, pop)
add(push, pop, pop)
push_range(poly_len)
legacy(linear, pop, push, 1, 1)
push_const(poly_len, log(abs(p1+p2)+1)*1j)
multiply(push, pop, pop)
add(push, pop, pop)
emit
poke_poly(10, p1*p2*real(poly6) + imag(poly18)*p1*p1*p1)
poke_poly(21, p2*poly10 + real(poly34)*p1*p1*p1)
poke_poly(32, poly21 - real(poly16)*p1*p1)
```

This is 27 chips for what is conceptually:

```text
v = sin((1..n) * 0.5) ^ p1
  * cos((1..n) * (1/3)) ^ p2
  + (1..n)^2 * p1*p2
  + (1..n) * log(|p1+p2|+1) * 1j
poly = v
poly[10] = p1*p2*Re(poly[6]) + Im(poly[18])*p1^3
poly[21] = p2*poly[10] + Re(poly[34])*p1^3
poly[32] = poly[21] - Re(poly[16])*p1^2
```

A reader who knows the math should be able to read the program at roughly
the same speed as the math. Today they cannot: `legacy(linear, pop, push,
0.5, 0)` hides the fact that this is just `*0.5`, and `power(push, pop,
pop)` hides the fact that powers always pop two and push one.

## Current Syntax Anatomy

Every chip is a list whose first element is the chip name and whose
remaining elements are positional arguments. Some chip names imply a fixed
source/target; others embed `src` and `tgt` selectors as the first two
positional args. The conventions in use today:

### Naming style

- Producer chips named with a `push_*` prefix push their result onto the
  stack with no source: `push_const`, `push_linspace`, `push_range`.
- Producer chips that read a register use `push(<src>)` with a selector
  arg: `push(cf)`, `push(poly)`.
- Direct register writers use `poke_*`: `poke_poly`, `poke_tos`.
- Side-effect-only commit uses `emit`.
- Vector binary ops use the bare math name with explicit `(tgt, src1,
  src2)` selector triple: `add(push, pop, pop)`, `multiply(push, pop,
  pop)`, `power(push, pop, pop)`.
- Stack manipulators use math/utility names with no selectors: `duplicate`,
  `swap`, `pop`, `flush`.
- Legacy coefficient-transform functions go through a generic
  `legacy(name, src, tgt, ...args)` wrapper: `legacy(linear, pop, push, 1,
  1)`, `legacy(sin, pop, push)`.

### Source selectors

`src` describes where a chip reads from. Allowed values:

```text
cf       read read-only cf vector
poly     read mutable poly register
pop      pop one vector from the stack
peek     read top of stack without popping
both     deprecated alias used by some legacy paths
```

### Target selectors

`tgt` describes where a chip writes. Allowed values:

```text
poly     write mutable poly register
push     push result onto the stack
both     deprecated alias used by some legacy paths
```

### Argument types

Numeric chip arguments accept a scalar expression sublanguage:

```text
identifiers:   t1, t2, p1, p2, poly_len, cfN, polyN, tosN, pi, pi2, pi2i
literals:      1, -2.5, 1e-6, 1+2j, -2i+4
operators:     + - * /
unary funcs:   conj(x), neg(x), real(x), imag(x), abs(x), log(x)
parentheses:   (p1 + p2) * 0.5
```

This grammar is accepted only inside chip arguments. Pain point #11 below
describes the seam between this expression grammar and the chip-line
grammar.

### Vector element access

Vector elements are read inside expressions with the `<name><digits>` form:
`cf6`, `poly18`, `tos32`. The index is part of the identifier and must be a
literal nonnegative integer in `[0, MAX_VECTOR_LEN)` (256 today).

### Implicit emit semantics

`emit` is overloaded:

- If the stack is non-empty, `emit` pops the top vector into `poly`.
- If the stack is empty, `emit` is a no-op commit marker that exists so
  legacy-style programs which mutate `poly` directly can still terminate
  cleanly.

### Implicit stack and register state

The runtime state per row is:

- `t1`, `t2`: source-coordinate scalars from the surrounding param sweep.
- `p1`, `p2`: param-program output scalars.
- `cf`: read-only vector of coefficient seed values.
- `poly`: mutable vector register that becomes the row's coefficient
  output.
- `stack`: ring of vectors operated on by the program.

`emit` is the only way for stack values to become the coefficient output.
`poke_poly(idx, expr)` is the only way to mutate `poly` from the scalar
expression language. Programs that never `emit` and never `poke_poly`
simply output whatever `poly` was when they started.

## Inconsistencies And Pain Points

These are the seams that show up every time someone tries to hand-write a
nontrivial program.

### 1. Selector ordering is not uniform across chip categories

```text
legacy(linear, pop, push, 1, 1)    # name, src, tgt, ...args
add(push, pop, pop)                # tgt, src1, src2
power(push, pop, pop)              # tgt, src1, src2
push_const(poly_len, p1)           # length, value (no selectors)
push(cf)                           # src
poke_poly(10, ...expr)             # index, value (no selectors)
```

Users either remember the order per chip or look it up every time.

### 2. The same word means different things in different positions

`pop` and `push` mean "consume from stack" and "produce to stack" in
selector positions, but `pop` is also the *name* of the standalone pop chip
and `push(cf)` is the *name* of the read-from-register chip. This forces
the catalog to disambiguate by context.

### 3. Legacy chips need an explicit selector that operators do not

```text
legacy(linear, pop, push, 1, 1)
multiply(push, pop, pop)
```

The `(pop, push)` and `(push, pop, pop)` portions are pure noise for the
common case where the chip pops its arguments and pushes its result. They
exist so the rare case (`legacy(sin, poly, poly)` for in-place mutation)
can be expressed.

### 4. `legacy(linear, pop, push, a, b)` is unreadable

`linear(z*a + b)` is the actual operation but you have to know the
`legacy_registry.json` semantics to read it. In the running example,
`legacy(linear, pop, push, 1, 1)` is "+1 to a vector", `legacy(linear, pop,
push, 0.5, 0)` is "scale by 0.5", and `legacy(linear, pop, push, 0.333…,
0)` is "scale by 1/3". None of those are visually obvious.

### 5. Vector element access reads as identifiers, not as indexes

```text
real(poly6) + imag(poly18)*p1*p1*p1
```

The reader has to mentally parse `poly6` as `poly[6]`. It also breaks when
the user wants a computed or symbolic index: there is no syntax for
`poly[i]` or `poly[poly_len-1]`.

### 6. Hardcoded indices in `poke_poly` make programs degree-bound

```text
poke_poly(10, ...)
poke_poly(21, ...)
poke_poly(32, ...)
```

These only run on programs whose `poly_len >= 33`. There is no way today
to write `poke_poly(poly_len-1, ...)` or `poke_poly(0.3 * poly_len, ...)`.

### 7. Power notation in expressions is awkward

The expression language has `+ - * /` but no `**` or `^`. Users write
`p1*p1*p1` for `p1^3` and the example does so three times.

### 8. `power` and `legacy(power, ...)` could collide

`power` is a vector binary op (element-wise complex power). The legacy
registry also includes a scalar-power transform. Today the compiler
resolves the conflict by routing the bare `power(...)` chip to the vector
binary op, but the existence of two namespaces under the same name is a
footgun.

### 9. `emit` has two meanings

Pop-and-commit when the stack has a value; commit-current-poly when it
does not. The current docs explain this, but users routinely forget which
mode they are in and either lose the result or commit a stale `poly`.

### 10. `pi2`, `pi2i` are cryptic

`pi2` reads as "pi squared" to most readers but is `2*pi`. `pi2i` is
`2*pi*1j`. Standard math notation would call these `tau` and `tau*1j` (or
`i*tau`). The current names exist for consistency with internal C
constants; the surface syntax does not need to inherit that.

### 11. Surface grammar splits expressions from chip lines

Coeff Program currently has two execution layers: the main coefficient-vector
VM and a scalar-expression evaluator called from that VM when a chip argument
has an expression reference. `coeffEvalScalarExpr` (in `lambda/sweep_cli.c`)
is stack-based, but it owns a separate scalar `(stackR, stackI)` stack and a
separate scalar opcode stream. That is better than row-loop string parsing,
but it is not the final architecture.

Param Program has already moved in the cleaner direction: expression fields
lower into the main Param Program VM token stream. Coeff Program should
converge on the same idea through the shared typed-VM model described above:
scalar expressions and vector operations should lower into one typed token
stream, with the compiler enforcing where scalar values and vector values are
legal.

The seam is entirely in the surface grammar and compiler. Today:

- An expression like `p1*p2 + log(abs(p1+p2)+1)*1j` is legal inside a chip
  argument such as `push_const(poly_len, ...)`, but
- the same expression cannot appear as a freestanding line, and
- a vector-typed sub-expression like `real(poly)` cannot appear inside a
  chip argument either, because the arg-position parser does not know how
  to lower a vector reference into the scalar opcode set.

Authors who learn one form expect the other to work and discover the
boundary by surprise. The fix is in the compiler: extend the source
grammar so chip-line positions and chip-arg positions accept the same
expression syntax, and lower freestanding expressions to chip tokens
(e.g. `poly + real(poly)*5` -> `push(poly); push(poly); real; scale(5);
mul; add`). v1 can improve this seam by parsing source text into existing
chip-list forms. The full v2 fix needs typed-VM lowering so scalar and vector
expressions share one execution model instead of nesting a scalar evaluator
inside vector-chip dispatch. Closing this seam is the largest single v2 item.

## Proposed Cleanup

The cleanup is intentionally minimal. The compiler keeps accepting every
shipped form. The editor learns to write the new form on save. Old
programs keep running.

### v1 Scope

v1 is line-oriented text source plus a small set of aliases. Everything in
v1 lowers onto existing chip-list forms and existing VM tokens. No new
opcodes are required.

```text
v1 features:
  - text source/load with comments and formatting preserved verbatim
  - one chip per line, optional ; separators
  - # single-line comments
  - aliases:
        arange   was push_range
        fill     was push_const
        scale(a) -> linear(a, 0)
        shift(b) -> linear(1, b)
        pow      -> the existing vector binary power op
  - arange(start, stop) and arange(start, stop, step) overloads
  - poly[N], cf[N], tos[N] static-index brackets in expressions
  - poly[N] = expr static-index poke (assignment-form sugar for poke_poly)
  - ** in scalar expressions
  - tau, tau_i constants alongside pi2, pi2i
  - explicit-source function calls for unary/binary chips:
        sin(poly), sin(cf), mul(poly, cf)
  - explicit-target assignment for emit and in-place forms:
        poly = pop, poly = peek
        poly = sin, poly = sin(poly), poly = mul(poly, cf)
  - emit kept as back-compat sugar for poly = pop
  - legacy(name, src, tgt, ...) remains a visible authoring chip
```

v1 is the "make poly_1 read like math" cut. Everything in the cleaned-up
example below compiles under v1.

### v2 Scope

v2 unifies the chip-line grammar with the chip-arg expression grammar
(pain point #11). It needs a real vector-expression parser, nested
temporary lowering, a shared typed-VM token stream, dynamic-index opcodes,
slice semantics, and richer diagnostics.

```text
v2 features:
  - unify scalar expressions with the vector stack VM so + - * / and
    real/imag/abs/log/conj/neg are sugar for the same stack ops in both
    chip args and freestanding lines:
        poly + real(poly) * 5
        fill(poly_len, p1*p2)         # auto-routes to scalar-only path
  - scalar/vector type inference with broadcast (length-1 broadcasts
    against poly_len-length vectors)
  - nested function-call lowering inside chip arguments:
        pow(poly, fill(poly_len, p1))
  - new stack-push ops for what the scalar sublanguage reads today:
        push_t1, push_t2, push_p1, push_p2, push_poly_len,
        push_poly_at(N), push_cf_at(N), push_tos_at(N)
  - dynamic-index reads and writes:
        poly[i_expr], cf[i_expr]
        poly[i_expr] = expr
  - slice assignment:
        poly[a..b] = expr
  - for_each_index(idx) { ... } block bodies
  - new chips that need new opcodes (geom, mask_real, mask_imag, ...)
  - possible retirement of user-facing legacy(...) once enough surface
    names exist
```

v2 should land only after v1 ships and after at least one user has tried
authoring nontrivial programs in v1. The v1 surface is enough to confirm
the syntactic direction; v2 either inherits from confirmed user value or
is deferred.

### Principle: Implicit Stack, Explicit Deviation

Most coefficient programs are pure stack programs that pop their arguments
and push their results. The selector triples should disappear from the
common case and reappear only when the user wants in-place mutation or a
non-default source/target.

Default rule (no selectors needed):

- Producer chip: pushes its result.
- Operator chip: pops its arity and pushes one result.
- Consumer chip: pops one or more values and writes a register or
  external state.

Deviation syntax uses function-call notation for explicit sources and `=`
for explicit targets. No special `@`-decorations or selector triples are
needed.

### Surface Syntax (v1)

```text
producers:        arange(poly_len)        # 0..n-1, pushed
                  arange(1, poly_len+1)   # 1..n, pushed
                  fill(poly_len, p1)      # constant vector, pushed
                  linspace(poly_len)      # 0..1 spaced, pushed
                  cf                      # push cf
                  poly                    # push poly

stack ops:        dup, swap, drop, flush

vector ops:       add, sub, mul, div, pow                       # binary, pop pop -> push
                  conj, neg, sin, cos, exp, sqrt, log, abs      # unary, pop -> push

scalar ops:       linear(a, b)            # tos -> z*a + b, pops one, pushes one
                  scale(a)                # alias for linear(a, 0)
                  shift(b)                # alias for linear(1, b)

explicit source:  sin(poly)               # poly -> push
                  sin(cf)                 # cf -> push
                  mul(poly, cf)           # poly * cf -> push
                  linear(poly, 0.5, 0)    # 0.5 * poly -> push
                  pow(poly, fill(poly_len, p1))   # nested call (v2)

explicit target:  poly = pop              # pop top -> poly (canonical emit)
                  poly = peek             # read top -> poly, stack unchanged
                  poly = sin              # pop top, sin, write to poly
                  poly = sin(poly)        # poly -> sin -> poly (in-place)
                  poly = mul(poly, cf)    # in-place vector multiply
                  poly[10] = expr         # static-index poke (v1)
                  poly[i] = expr          # dynamic-index poke (v2)
                  poly[a..b] = expr       # slice poke (v2)

escape hatch:     legacy(name, src, tgt, ...)   # any shipped legacy chip
```

The unifying rule: a chip name with no parens pops its source from the
stack. A chip name with parens reads its source from the parenthesized
argument. The destination is push by default; `target = chip(...)`
redirects the result to a named register or poke target.

For chips that take both vector sources and scalar parameters (like
`linear(a, b)` where `a` and `b` are scalars and the source is a vector),
each chip's catalog entry declares which parameter positions are vector
sources and which are scalar parameters. Vector-source positions default
to `pop` when omitted; scalar parameters must always be supplied.

```text
linear(0.5, 0)               # vector src = pop (implicit); scalars (0.5, 0)
linear(poly, 0.5, 0)         # vector src = poly; scalars (0.5, 0)
poly = linear(poly, 0.5, 0)  # in-place: poly -> linear -> poly
```

For binary vector ops, both vector positions accept the same explicit-or-
pop rule:

```text
mul                          # mul(pop, pop) -> push    ; below * top
mul(poly, cf)                # poly * cf -> push
mul(poly)                    # poly * pop -> push       ; one pop
poly = mul(poly, cf)         # in-place
```

RPN ordering convention is preserved: `sub` and `div` evaluate `(below)
op (top)`, matching how the stack-only forms compile today.

### `legacy(...)` In v1

`legacy(name, src, tgt, ...)` stays visible in the v1 surface syntax. It
is the explicit, fully-general way to invoke any function in the legacy
registry, and it remains the canonical compiled token name. Real programs
exercise dozens of legacy functions with different argument and length
policies; v1 only ships first-class surface names for the most common
arithmetic chips (`linear`, `scale`, `shift`, `pow`, the unary math ops).
For everything else, `legacy(...)` is the right answer in v1 and reads
clearly enough.

v2 may eventually retire `legacy(...)` from the user-facing surface by
promoting more legacy functions to first-class names, but that is an
intentionally deferred decision. The first cut should make common math
pleasant, not hide every old primitive.

`legacy(...)` always survives in two non-surface roles:

- The compiler keeps accepting `["legacy", name, src, tgt, ...]` as
  saved-form input forever. Old saved programs load and round-trip.
- `legacy(...)` remains the canonical compiled token name. Compiled
  payloads, fingerprints, and the JSON storage shape continue to use
  `op=COEFF_OP_LEGACY` exactly as today.

## v1 Features In Detail

### `linear`, `scale`, And `shift`

`legacy(linear, pop, push, a, b)` becomes `linear(a, b)`. For the two
cases that show up over and over in real programs, add named aliases:

- `scale(a)` lowers to `linear(a, 0)`. Reads as "multiply by a".
- `shift(b)` lowers to `linear(1, b)`. Reads as "add b".

The running example's `legacy(linear, pop, push, 1, 1)` becomes
`shift(1)`. `legacy(linear, pop, push, 0.5, 0)` becomes `scale(0.5)`.

### `arange` And `fill` Rename

`push_range` becomes `arange` (NumPy-style); `push_const` becomes `fill`;
`push_linspace` keeps its name. The `push_*` prefix only made sense before
producers and stack operators were distinguishable by category; now they
are.

### `arange(start, stop)` And `arange(start, stop, step)` Overloads

The current `push_range(length)` is the zero-based-only special case.
Real programs need explicit start and stop. Generalize `arange` to accept
up to three real-valued expression arguments, NumPy-style:

```text
arange(stop)              # 0, 1, ..., stop-1   (current behavior)
arange(start, stop)       # start, start+1, ..., stop-1
arange(start, stop, step) # start, start+step, ..., < stop
```

All three arguments are real-valued scalar expressions. They can reference
`poly_len`, `p1`, `p2`, `t1`, `t2`, and constants. The compiler validates
that `step != 0` (compile-time error if statically zero) and that the
result length is bounded by `MAX_VECTOR_LEN` (256).

This subsumes `arange_one_based` (`arange(1, poly_len+1)`),
`arange_centered` (`arange(-(poly_len-1)/2, (poly_len+1)/2)`), and any
future special case without inventing a new chip name. It also kills the
four `shift(1)` calls in the `poly_1` example — `arange(1, poly_len+1)`
is the one-based ordinal `1..poly_len` read directly as math.

For symmetry, `linspace` accepts the same generalization:

```text
linspace(count)               # 0..1 with count entries (current)
linspace(start, stop, count)  # start..stop with count entries
```

Old single-arg saved forms remain accepted forever.

### Bracketed Vector Element Access

Replace `cf6`, `poly18`, `tos32` with `cf[6]`, `poly[18]`, `tos[32]`.

- **v1**: literal indices (`poly[6]`, `cf[6]`) and constant-folded
  compile-time index expressions (`poly[poly_len-1]` if `poly_len` is
  known statically; otherwise rejected in v1) compile to the existing
  `EXPR_POLY_AT` / `EXPR_CF_AT` / `EXPR_TOS_AT` opcodes with the index in
  the operand field.
- **v2**: full dynamic indices (`poly[i]`, `poly[i_expr]`) need a new
  runtime opcode that pops or evaluates an index expression at row time.

### Static-Index `poly[N] = expr`

The chip-as-list source form keeps its `["poke_poly", "10", "<expr>"]`
shape. The editor and the documented surface use assignment syntax:

```text
poly[10] = p1*p2*real(poly[6]) + imag(poly[18])*p1**3
```

The compiler parses both forms and lowers them to the same compiled
token. Dynamic-index `poly[i] = expr` and slice `poly[a..b] = expr` are
v2.

### `**` For Power In Expressions

Add `**` to the scalar expression language. Right-associative, binds
tighter than `*` / `/` and looser than unary minus, matching Python
convention. `p1**3` lowers to three `EXPR_MUL` opcodes today; v2 may add
a dedicated `EXPR_POW` or `EXPR_POW_INT` opcode if the benchmark asks
for it.

### `tau` Constant

Add `tau = 2*pi` and `tau_i = 2*pi*1j` as named constants. Keep `pi2`
and `pi2i` as aliases forever. The saved form preserves whatever the
compiler emitted (no fingerprint churn for the rename).

### Disambiguate `power`

Rename the vector binary op from `power` to `pow` (both forms accepted;
new saves write `pow`). `legacy(power, ...)` is a separate scalar-power
chip in the legacy registry and is rarely what the user wants; the chip
catalog and the compiler should give a clear message when the user picks
it accidentally.

### `emit` Becomes Sugar For `poly = pop`

`emit` is exactly `poly = pop` when the stack is non-empty. Under the
unified function-call/assignment surface, that is the clear spelling.
Bespoke `emit` and `emit_keep` chip names disappear from new authoring;
both are subsumed by the assignment form:

```text
poly = pop      # pop top -> poly                       (canonical spelling)
poly = peek     # read top -> poly, stack unchanged     (replaces emit_keep)
emit            # alias for poly = pop                  (back-compat sugar)
```

The compiler emits the same `COEFF_OP_EMIT` opcode for `poly = pop` and
`emit`. The existing `emit`-with-empty-stack no-op behavior is preserved
only when loading legacy saved programs that depend on it. New
authoring-time `emit` (or its canonical equivalent `poly = pop`) compiles
to a clear error if the stack is provably empty at that program point,
detected by the existing stack-effect analysis. Programs that mutate
`poly` directly via `poly[...] = expr` or via `poly = chip(poly, ...)`
simply have no `emit` chip; there is no need for a separate "commit"
marker.

`peek` and `pop` are reserved identifiers that may appear only on the
right-hand side of an explicit-target assignment or as an explicit source
inside a chip call. They are not standalone chip names. The standalone
chip that drops the top of stack is `drop`, not `pop`.

### Comments

Single-line `#` comments are accepted in source. Comments survive in the
saved `source_text` exactly as authored (see "Compatibility And
Migration") and never reach the compiled output:

```text
# A = sin((1..n) * 0.5) ^ p1
arange(1, poly_len+1); scale(0.5)
fill(poly_len, p1); pow; sin
```

Multi-chip lines with `;` separators are accepted but optional. The
canonical form on save is one chip per line for diff stability; users
who type multi-chip lines see them preserved in `source_text`.

## Cleaned-Up Example

`poly_1` rewritten using only v1 features:

```text
# poly_1: full v1 reform.
#
# v = sin((1..n)*0.5)^p1
#   * cos((1..n)/3)^p2
#   + (1..n)^2 * p1*p2
#   + (1..n) * log(|p1+p2|+1)*1j

# A = sin((1..n)*0.5)^p1
arange(1, poly_len+1)
scale(0.5)
fill(poly_len, p1)
pow
sin

# B = cos((1..n)/3)^p2
arange(1, poly_len+1)
scale(1/3)
fill(poly_len, p2)
pow
cos

# A * B
mul

# + (1..n)^2 * p1*p2
arange(1, poly_len+1)
dup
mul
fill(poly_len, p1*p2)
mul
add

# + (1..n) * log(|p1+p2|+1)*1j
arange(1, poly_len+1)
fill(poly_len, log(abs(p1+p2)+1)*1j)
mul
add

emit

# Targeted overrides on three coefficients.
poly[10] = p1*p2*real(poly[6]) + imag(poly[18])*p1**3
poly[21] = p2*poly[10] + real(poly[34])*p1**3
poly[32] = poly[21] - real(poly[16])*p1**2
```

### Comparison

```text
form              chips    structural noise tokens
current source       27    16  (4x explicit "+1" via legacy(linear,...,1,1),
                                12x selector triples)
v1 reform            23     0
```

"Structural noise tokens" counts chips and chip arguments that the math
does not require — selector triples like `(push, pop, pop)` and the
`+1` shift after every `arange` for one-based ordinals. The compiled VM
tokens for `arange(1, poly_len+1)` and `push_range(poly_len);
legacy(linear, pop, push, 1, 1)` are equivalent in semantics; the v1
reform's lowering may collapse the two into a single `EXPR_*`/legacy pair
or keep them separate depending on how the compiler chooses to emit
non-zero-based ranges. Either way, what the author types and the reader
skims past changes from 16 noise tokens to zero.

### Why This Is Better

**It reads as math.** Side by side with the math one-liner above, every
line of the program corresponds to one operation in the formula. There is
no glossary needed. `legacy(linear, pop, push, 0.5, 0)` required the
reader to know the legacy convention and recognize that `(0.5, 0)` means
"scale by 0.5"; `scale(0.5)` does not.

**It removes patterns by generalizing primitives, not by inventing
specializations.** `arange(n); shift(1)` appears four times in `poly_1`.
The wrong fix is to add `arange_one_based(n)`; the right fix is to give
`arange` proper start/stop expression arguments so `arange(1, poly_len+1)`
reads as "the integers 1..poly_len" directly. Generalizing the primitive
subsumes every special case (one-based, centered, strided, partial) and
matches the NumPy convention users already know.

**It eliminates `legacy(...)` from the common case.** In the current
source `legacy(...)` accounts for 11 of the 27 chips. In the v1 reform it
accounts for zero. The escape hatch still exists for unusual chips
(`legacy(roots6, ...)`, `legacy(blend, ...)`), but the core arithmetic of
coefficient generation no longer routes through it. That is what makes
Coeff Programs a real authoring surface rather than a thin shim around
the legacy registry.

**`poly[i]` and `**` matter disproportionately.** The three `poke_poly`
expressions at the bottom of the program are dense — they reference five
distinct coefficient indices and apply integer powers of `p1`. In the
current syntax those three lines mix `polyN`-as-identifier with
`p1*p1*p1`-as-multiplication; the eye has to recompute the index and the
exponent each time. With brackets and `**`, each line is a small
algebraic expression that the reader can verify against the math
directly.

**It scales to other coefficient functions.** `poly_1` happens not to
need `geom`, `mask_real`, `for_each_index`, or `poly[a..b] = expr`. Other
coefficient functions in the existing C library do — `poly_4` and the
mirror-pair generators in particular. Those benefits land in v2.

**It does not regress runtime.** Every transformation in the v1 reform
lowers to the same compiled tokens that the current source produces.
There is no perf change, no fingerprint churn for already-deployed
programs, no migration burden on saved S3 programs. The investment is
compiler parser changes plus an editor canonicalizer, both isolated to
the source-input layer.

The acid test holds: a reader who knows the math should read the program
at roughly the speed of the math. The current source fails that test;
the v1 reform passes it.

## v2 Chips Worth Adding

These need new opcodes and are explicitly v2. Each must justify itself by
either collapsing a real multi-chip pattern or unblocking an expressive
gap.

### `geom(length, ratio)` Or `geom(length, base, exponent_ratio)`

Geometric sequences appear in many coefficient generators (Chebyshev-style
seeds, exponential weighting). Today they require `arange; pow` or a
custom loop. A native `geom(length, r)` chip pushing
`1, r, r^2, ..., r^(length-1)` is one fused op and a clear tool for the
job.

### `vec_index_axis(length)`

Push `(-1, -1+2/n, ..., +1)` real values. Same as `linspace(-1, 1, n)`.
Useful for parametric basis evaluations. Composable from `linspace` plus
`linear` once `linspace(start, stop, count)` ships in v1, so this chip is
optional v2 sugar rather than a v2 requirement.

### `mask_real`, `mask_imag`

Apply `Re(z)` or `Im(z)` element-wise to the top-of-stack vector. Today
this requires a `poke_tos` loop or a careful `mul` against a real-only
fill vector. One chip is much cleaner.

### `roll(k)`, `reverse`, `cumsum`, `diff` As Bare Chip Names

These exist as legacy registry entries today. Promote the most common
ones to top-level chip names so authors do not have to remember the
registry. They still lower to `legacy(...)` underneath but read as
`cumsum`, `reverse`, `roll(2)`.

### `for_each_index(idx) { ... }`

This is the one structural addition worth considering. It would take a
body of expression-only chips and apply them per-index, with the index
visible as a binding inside the body:

```text
for_each_index(i) {
    poly[i] = poly[i] * exp(i * tau_i / poly_len)
}
```

This unblocks per-index transforms that today require a custom legacy
entry. It also crosses a complexity boundary: the body needs its own
scope, the compiler needs to verify stack-neutrality, and native
execution needs an inner loop with bounded body size. Worth a separate
design pass.

### `poly[a..b] = expr` Slice Assignment

A slice form of `poke_poly` for ranges:

```text
poly[0..3] = 0          # zero the first four
poly[poly_len-3..] = 1  # last three become 1
```

Lowers to a small loop in the compiler. Keeps programs degree-flexible.

## Compatibility And Migration

Every shipped program must keep loading and executing.

### Saved-Form Layout

A saved Coeff Program contains three layers, written together so the
compiler, the editor, and the runtime each see the form they need:

```json
{
  "program_kind": "coeff_program",
  "version": 1,
  "name": "poly_1",
  "source_text": "# poly_1...\narange(1, poly_len+1)\nscale(0.5)\n...",
  "chain": [
    ["push_range", "poly_len"],
    ["legacy", "linear", "pop", "push", "1", "1"],
    "..."
  ],
  "tokens": [],
  "fingerprint": "sha1...",
  "display": "..."
}
```

- `source_text` is the exact text the user authored, comments and blank
  lines and formatting preserved verbatim. This is the canonical
  authoring artifact. Save-as-text and load-as-text round-trip through
  this field.
- `chain` is the canonical lowered chip-as-list form. It is what the
  compiler accepts as input regardless of how it was produced (UI editor,
  text editor, macro expansion). Old saved programs that pre-date
  `source_text` continue to load through `chain` alone.
- `tokens` and `fingerprint` are the compiled runtime form, derived from
  `chain` by the compiler. They never participate in authoring.

When a program is loaded:

- If `source_text` is present, the editor displays it directly. `chain`
  is used only for compilation.
- If `source_text` is absent (legacy saved programs), the editor renders
  `chain` through the same canonicalizer it would use for a fresh save
  and treats that rendered text as the editing surface.

When a program is saved:

- The editor's text buffer is written to `source_text`.
- The parser lowers `source_text` to the canonical `chain`.
- The compiler produces `tokens` and `fingerprint` from `chain`.

### Comments And Formatting

Comments and whitespace survive in `source_text` exactly as authored.
They do not affect `chain`, `tokens`, or `fingerprint`. The fingerprint
hashes the compiled token stream only, so reformatting source never busts
artifact caches. Two programs that differ only in comments produce
identical compiled output.

### Old Saved Programs

- The compiler accepts both old and new spellings forever. New names
  lower to the same compiled tokens as the old names.
- Saved-form JSON without `source_text` continues to load through
  `chain` alone. The editor synthesizes a `source_text` view from
  `chain` on load for display, and writes it back on the next save.
- The macro registry round-trips through the same parser/canonicalizer.
- Tests assert: every old-form program loads, compiles, fingerprints,
  and renders identically to the corresponding new-form program.
  Fingerprints for old programs do not change when they acquire a
  `source_text` field on the next save.

## Implementation Targets

v1 work lives in:

- **Compiler** — `lambda/coeff_program_chain.py`. Extend
  `_lower_chip`, `_lower_chain`, and the chain/source normalization entry
  points for the new aliases (`arange`, `fill`, `scale`, `shift`, `pow`);
  widen `arange`/`linspace` arity; add `**` and `tau`/`tau_i` to the scalar
  expression parser; add `poly[N]` bracket parsing alongside the existing
  `polyN` form; parse `poly[N] = expr` as sugar for `poke_poly`.
- **Editor** — `index.html`. Update `_cpCatalog`,
  `_normalizeCoeffProgramChain`, `_serializeCoeffProgramChain`, and
  `_validateCoeffProgramUiChain` for the new chip names; add a text
  editor surface that round-trips through `source_text`; add the
  text-to-chain parser and the chain-to-text canonicalizer.
- **Native** — `lambda/sweep_cli.c`. No changes for v1. The runtime
  already handles every v1 lowering through existing opcodes.
- **Tests** — `tests/test_coeff_program_chain.py`,
  `tests/test_coeff_program_native.py`, `tests/test_frontend_js.sh`,
  `tests/test_coeff_program_storage.py`. Add round-trip assertions for
  `source_text`, alias-equivalence assertions, and old-saved-form
  loading assertions.

v2 work additionally:

- **Compiler** — vector-expression parser, scalar/vector type inference
  with broadcast, typed-VM token lowering, dynamic-index opcodes, slice
  lowering, `for_each_index` body compilation, new chips (`geom`,
  `mask_real`, `mask_imag`).
- **Editor** — richer diagnostics for the unified grammar.
- **Native** — shared typed-VM execution core with profile configuration;
  new opcodes (`EXPR_POLY_AT_DYN`, `COEFF_OP_GEOM`, `COEFF_OP_MASK_REAL`,
  `COEFF_OP_MASK_IMAG`, `COEFF_OP_FOR_EACH_INDEX`, slice-write opcodes);
  typed push ops for what the scalar sublanguage currently reads internally.

## Decisions

Items already settled in the body of this document, gathered here so they
do not have to be re-litigated:

- **In-place register mutations** spell as `poly = chip(poly, ...)`. No
  `@`-decoration syntax. The redundancy of `poly = sin(poly)` is visually
  cheap and removes ambiguity about what the chip does.
- **`poly[i] = expr` with dynamic indices** is v2, not v1. v1 supports
  literal indices and statically constant-foldable expressions only.
- **User formatting and comments** survive in `source_text` verbatim.
  Canonicalization happens only when producing `chain` for the compiler;
  it never rewrites `source_text`.
- **Comments do not affect compiled output.** Stripped during
  `source_text -> chain` lowering. Two programs that differ only in
  comments produce identical fingerprints.
- **`tau` does not replace `pi2` in the saved form.** The display layer
  translates `pi2` to `tau` for users who prefer it. No fingerprint
  churn for the rename.
- **`legacy(...)` stays visible in v1.** It is the explicit, fully-
  general way to invoke any function in the legacy registry. v1 only
  promotes the most common arithmetic chips to first-class names.
  Retiring `legacy(...)` entirely is v2 and intentionally deferred.

## Summary

The current syntax is not broken; it is just visibly assembled from
incremental decisions. The reform splits cleanly into two scopes.

v1 (ship first):

- Text source/load with `source_text` field; comments and formatting
  preserved verbatim.
- Aliases: `arange`, `fill`, `scale`, `shift`, `pow`.
- `arange(start, stop)` and `arange(start, stop, step)` overloads.
- `poly[N]`, `cf[N]`, `tos[N]` static-index brackets in expressions.
- `poly[N] = expr` static-index poke.
- `**`, `tau`, single-line `#` comments.
- Function-call source notation: `sin(poly)`, `mul(poly, cf)`.
- Assignment-form target: `poly = pop`, `poly = peek`,
  `poly = sin(poly)`.
- `legacy(name, src, tgt, ...)` remains a visible authoring chip.
- Every existing saved program loads, compiles, and renders identically.

v2 (after v1 ships and gets used):

- Unified expression grammar across chip lines and chip args
  (pain point #11).
- Dynamic-index reads and writes, slice assignment, `for_each_index`.
- New opcodes for `geom`, `mask_real`, `mask_imag`, and dynamic
  `poly[i]`.
- Possible retirement of user-facing `legacy(...)`.

After v1 ships, the running example reads as math. That is the test.
