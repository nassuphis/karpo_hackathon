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
  output contract:            p1 and p2 are emitted at end of row;
                              default to t1 and t2 if not explicitly written
  typical ops:                scalar arithmetic, scalar functions, assignment

profile: coeff
  readable scalar registers:  t1, t2, p1, p2, poly_len
  readable vector registers:  cf, poly
  writable vector registers:  poly
  output contract:            poly is emitted at end of row;
                              defaults to the initial cf-derived value if not
                              explicitly written
  typical ops:                scalar arithmetic, vector construction,
                              vector arithmetic, vector assignment
```

The compiler should enforce these profiles. A Param Program must not read
`poly` or `cf`; a Coeff Program must not write `p1` or `p2`. The syntax can
be shared, but the capability set and output contract remain program-kind
specific.

Param Program has already moved new compiler output in the desired direction:
expression fields lower into the main VM token stream, not into a separate
expression interpreter. Its native reader still accepts the old nested
`scalar_exprs` / `expr_refs` payloads during the compatibility window, but
the target design is single-VM lowering. Coeff Program should converge on the
same principle, with typed stack values instead of scalar-only stack values.
A coefficient expression such as `fill(poly_len, p1*p2)` should compile to
ordinary typed VM work: compute a scalar `p1*p2`, then construct a vector from
that scalar. A vector expression such as `mul(poly, cf)` should compile to
typed vector operations using the same dispatch machinery.

The runtime work is smaller than "rewrite everything" but larger than a
syntax patch. The current Coeff vector stack already stores vector slots with
real/imag arrays and a length; the typed VM adds three concrete pieces:

- dispatch metadata that marks each opcode operand as scalar, vector, or
  mixed
- mixed operations such as `get_scalar(vector, idx)`, `set_scalar(poly, idx,
  value)`, `dot(vector, vector)`, and broadcast-aware vector/scalar binary ops
- removal of the separate scalar-expression stack for new payloads, replacing
  it with typed main-VM stack tokens
- direct source transform calls lower to native transform tokens keyed by
  stable function index; source text no longer emits or accepts
  `legacy(...)`

This does not mean rewriting Param and Coeff together. The pragmatic order is:

1. Ship Coeff Program text source v1 by lowering source text to the existing
   chip-list form and existing native opcodes.
2. Design and implement the typed VM core in the Coeff path, where vector
   values make the need concrete.
3. Keep the current Param Program path stable while Coeff validates the typed
   VM design.
4. Migrate Param Program onto the same typed-VM backend as a second profile
   only after the Coeff implementation has proven the model.

The shared part is the parser/lowering library, typed IR, diagnostics model,
and native VM core. The frontend remains profile-specific: Param Program has
scalar output registers and scalar-oriented chips; Coeff Program has vector
output and vector-oriented source tools. The parser core should be shared, but
the grammar/catalog tables are profile-specific so Param and Coeff can expose
different registers, statements, and chip names without pretending to be the
same surface language.

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

This is 29 chips for what is conceptually:

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
mul; add`). v1 reduces this seam by adding native primitives for the common
assignment, affine, range, and unary-math cases and parsing source text into
canonical chip-list/native forms. The full v2 fix needs typed-VM lowering so
scalar and vector expressions share one execution model instead of nesting a
scalar evaluator inside vector-chip dispatch. Closing this seam is the largest
single v2 item.

## Proposed Cleanup

The cleanup is intentionally minimal, but three source-level ideas need real
VM support instead of legacy-transform abuse:

- `set(poly, src)` is a vector copy. Do not lower assignment through
  `legacy(linear, ..., 1, 0)`.
- `linear/scale/shift` are affine vector ops with complex scalar
  expressions. Do not lower them through legacy `linear`, whose current
  registry args are real-only.
- `arange(start, stop, step)` and generalized `linspace` are vector
  producers. Do not synthesize them by pushing a range and applying a
  chain of `shift`/`scale` transforms.

The compiler keeps accepting every shipped form. The editor learns to write
the new form on save. Old programs keep running.

### v1 Scope

v1 is line-oriented text source plus a small set of aliases. It mostly
lowers onto existing chip-list forms, but the source surface requires three
native/compiler updates: `COEFF_OP_SET`, `COEFF_OP_AFFINE`, and extended
`COEFF_OP_RANGE` / `COEFF_OP_LINSPACE` argument support.

```text
v1 features:
  - text source/load with comments and formatting preserved verbatim
  - one chip per line, optional ; separators
  - # single-line comments
  - aliases:
        arange   was push_range
        fill     was push_const
        scale(a) -> affine(a, 0)
        shift(b) -> affine(1, b)
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
  - explicit set form:
        set(poly, cf), set(poly, pop), set(poly, peek)
  - emit kept as back-compat sugar for poly = pop
  - direct native transform calls:
        rev(poly), cumsum(poly), roots(poly, 8, hi)
    The old `legacy(name, src, tgt, ...)` wrapper is Chain/chip-list
    compatibility only, not source syntax.
```

v1 is the "make poly_1 read like math" cut. Everything in the cleaned-up
example below compiles under v1.

### Phase 2A: Typed Stack, Dynamic Indexing, And Broadcast

Phase 2A is the first typed-VM cut. It is intentionally additive: existing
v1 chip-list tokens and saved programs keep their current behavior, while
source programs that need mixed scalar/vector behavior lower to hidden typed
tokens in the same `coeff_program.tokens` payload.

What Phase 2A implements:

- one shared ring stack that can hold either vector values or scalar complex
  values
- vector/scalar broadcast for `add`, `subtract`, `multiply`, `divide`, and
  `power`
- typed unary ops for `abs`/`mod`, `angle`, `neg`, `conj`, `sqrt`, `log`,
  `real`, and `imag`
- dynamic scalar reads such as `poly[poly_len-1]`, `cf[poly_len-2]`, and
  `tos[poly_len-1]`
- dynamic scalar writes such as `poly[poly_len-1] = p1`
- typed `fill(length, value)` so nested forms like
  `add(poly, fill(poly_len, p1))` do not fall back to the old scalar-argument
  VM

The hidden source-lowering opcodes are:

```text
_typed_push_scalar(expr)   -> scalar
_typed_push_vector(src)    -> vector
_typed_binary(op)          -> scalar or vector, with vector/scalar broadcast
_typed_unary(op)           -> scalar or vector, preserving shape
_typed_get_scalar          -> get_scalar(vector, scalar_index)
_typed_set_poly            -> poly = vector
_typed_poke_poly           -> set_scalar(poly, scalar_index, scalar_value)
_typed_fill                -> fill(scalar_length, scalar_value)
```

These names are compiler-internal. They may appear in compiled/debug output,
but they are not intended as hand-authored user syntax.

Phase 2A does not yet implement reductions (`dot`, `sum`, `norm2`), slices,
`where`, `select`, RNG constructors, `for_each_index`, or automatic Param
Program migration.

### v2 Scope

Full v2 continues the Phase 2A typed stack into a richer vector-expression
language. It needs slice semantics, reductions, conditionals, deterministic
random constructors, and richer diagnostics.

```text
v2 features:
  - continue unifying scalar expressions with the vector stack VM so + - * / and
    real/imag/abs/log/conj/neg are sugar for the same stack ops in both
    chip args and freestanding lines:
        poly + real(poly) * 5
        fill(poly_len, p1*p2)         # auto-routes to scalar-only path
  - scalar/vector type inference with broadcast (Phase 2A has binary
    vector/scalar broadcast; full v2 extends this to all typed primitives)
  - nested function-call lowering inside chip arguments:
        pow(poly, fill(poly_len, p1))
  - new stack-push ops for what the scalar sublanguage reads today:
        push_t1, push_t2, push_p1, push_p2, push_poly_len,
        push_poly_at(N), push_cf_at(N), push_tos_at(N)
  - dynamic-index reads and writes (Phase 2A has scalar element reads/writes;
    full v2 adds slices):
        poly[i_expr], cf[i_expr]
        poly[i_expr] = expr
    lowered through typed mixed ops:
        get_scalar(vector, scalar_index) -> scalar_complex
        set_scalar(vector_register, scalar_index, scalar_value)
  - vector reductions:
        dot(poly, cf) -> scalar_complex
        norm2(poly) -> scalar_real
        sum(poly), mean(poly), prod(poly)
        norm1(poly), norminf(poly), maxabs(poly)
        argmaxabs(poly), argminabs(poly)
    These are mixed typed ops, not LAPACK-dependent calls.
  - vector construction and structure:
        zeros(n), ones(n), basis(n, i, value)
        slice(v, start, stop, step), concat(a, b), pad(v, n, side), trim(v, n, side)
  - elementwise vector helpers:
        real(v), imag(v), unit(v), clip_abs(v, lo, hi), center(v), standardize(v)
  - conditionals:
        where(mask, a, b) -> vector_complex
        select(cond, x, y) -> scalar_complex
  - slice assignment:
        poly[a..b] = expr
  - for_each_index(idx) { ... } block bodies
  - new chips that need new opcodes (geom, mask_real, mask_imag, ...)
  - possible retirement of user-facing legacy(...) once enough surface
    names exist
```

Phase 2A landed before full v2 because dynamic indices and broadcast are
load-bearing for real programs. The remaining full-v2 items should still be
added only when they collapse concrete user pain, not as speculative language
surface.

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
                  abs, mod, angle, neg, conj, sqrt, log          # native unary, pop -> push
                  sin, cos, exp                                 # registry-backed unary sugar

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
                  set(poly, peek)         # explicit copy spelling
                  poly = sin              # pop top, sin, write to poly
                  poly = sin(poly)        # poly -> sin -> poly (in-place)
                  poly = mul(poly, cf)    # in-place vector multiply
                  poly[10] = expr         # static-index typed poke
                  poly[i] = expr          # dynamic-index typed poke
                  poly[a..b] = expr       # slice poke (v2)

native transform: roots(poly, 8, hi)      # direct registry-backed transform
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

### Direct Native Transforms Replace `legacy(...)` In Source

`legacy(name, src, tgt, ...)` is not source syntax. Source text calls the
operation directly:

```text
poly = rev(poly)
cumsum(poly)
roots(poly, 8, hi)
```

These lower to `COEFF_OP_NATIVE_TRANSFORM` tokens keyed by stable function
index. There is still no string lookup in the hot loop. The native evaluator
uses the same underlying C transform functions, but the user-facing program
does not expose a generic `legacy(...)` wrapper.

For direct transform calls, a trailing `andy` argument keeps the same blend
semantics as Chain mode. Example: `poly = exp(poly, a, b, andy)` compiles `a`
and `b` as typed-stack scalar arguments, stores `andy` on the transform token,
and the native evaluator blends the transformed vector with the selected source
vector. Stack-argument transform tokens must validate their argument count
against `coeff_legacy_registry.json`; `rev` accepts zero stack args, `exp`
accepts at most two packed complex args, etc.

`legacy(...)` survives only as compatibility plumbing:

- Chain mode still uses the old coefficient-transform chain.
- The chip-list compiler may keep accepting `["legacy", name, src, tgt, ...]`
  while old saved development artifacts exist.
- The Compute-tab Program authoring surface should hide `legacy(...)` and
  offer direct transform chips instead.

## v1 Features In Detail

### `linear`, `scale`, And `shift`

`legacy(linear, pop, push, a, b)` becomes source-level `linear(a, b)`.
New source lowers through the typed stack as ordinary vector/scalar work:
push source vector, compute scalar `a`, multiply, compute scalar `b`, add.
That matters because source expressions may be complex (`1j*p1`, `p2`, etc.)
without depending on the old real-only legacy registry metadata.

The saved chip-list forms `["legacy", "linear", ...]` and `["affine", ...]`
may remain accepted for compatibility, but source `linear(...)`, `scale(...)`,
and `shift(...)` should not emit the old scalar-expression VM path.

For the two cases that show up over and over in real programs, add named
aliases:

- `scale(a)` lowers to affine multiply by `a` and offset `0`.
- `shift(b)` lowers to affine multiply by `1` and offset `b`.

The running example's `legacy(linear, pop, push, 1, 1)` becomes
`shift(1)`. `legacy(linear, pop, push, 0.5, 0)` becomes `scale(0.5)`.

### `arange` And `fill` Rename

`push_range` becomes `arange` (NumPy-style); `push_const` becomes `fill`;
`push_linspace` keeps its name. The `push_*` prefix only made sense before
producers and stack operators were distinguishable by category; now they
are.

### `arange(start, stop)` And `arange(start, stop, step)` Overloads

The current `push_range(length)` is the zero-based-only special case.
Real programs need explicit start and stop. Generalize `arange` by extending
`COEFF_OP_RANGE` so it carries `start`, `step`, and `count`/length metadata,
all resolved from scalar expressions before the vector is filled. This is a
native producer, not a hidden `range; shift; scale` lowering.

```text
arange(stop)              # 0, 1, ..., stop-1   (current behavior)
arange(start, stop)       # start, start+1, ..., stop-1
arange(start, stop, step) # start, start+step, ..., < stop
```

All three arguments are real-valued scalar expressions. They can reference
`poly_len`, `p1`, `p2`, `t1`, `t2`, and constants. The compiler validates
statically when it can; the native evaluator must also validate per row that
`step != 0`, the computed count is an integer in `[1, MAX_VECTOR_LEN]`, and
the generated values are finite.

This subsumes `arange_one_based` (`arange(1, poly_len+1)`),
`arange_centered` (`arange(-(poly_len-1)/2, (poly_len+1)/2)`), and any
future special case without inventing a new chip name. It also kills the
four `shift(1)` calls in the `poly_1` example — `arange(1, poly_len+1)`
is the one-based ordinal `1..poly_len` read directly as math.

For symmetry, `linspace` accepts the same generalization by extending
`COEFF_OP_LINSPACE` to carry `start`, `stop`, and `count`:

```text
linspace(count)               # 0..1 with count entries (current)
linspace(start, stop, count)  # start..stop with count entries
```

Old single-arg saved forms remain accepted forever.

### Bracketed Vector Element Access

Replace `cf6`, `poly18`, `tos32` with `cf[6]`, `poly[18]`, `tos[32]`.

- **v1**: literal indices (`poly[6]`, `cf[6]`, `tos[3]`) compile to the
  existing `EXPR_POLY_AT` / `EXPR_CF_AT` / `EXPR_TOS_AT` opcodes with the
  index in the operand field. v1 does not need a general index-expression
  evaluator.
- **Phase 2A**: dynamic scalar indices (`poly[i]`, `poly[poly_len-1]`,
  `cf[i_expr]`) lower naturally in the typed VM:

  ```text
  poly[poly_len - N]
    -> push_vector(poly)
       push_scalar(poly_len)
       push_scalar(N)
       scalar_sub
       get_scalar
  ```

  `get_scalar(vector, scalar_index) -> scalar_complex` is a mixed typed op:
  one vector input, one scalar input, one scalar output. It must reject
  non-real indices, non-integer indices, and indices outside
  `[0, vector.length)`. It must not clamp or wrap silently.

### Static-Index `poly[N] = expr`

The chip-as-list source form keeps its `["poke_poly", "10", "<expr>"]`
shape. The editor and the documented surface use assignment syntax:

```text
poly[10] = p1*p2*real(poly[6]) + imag(poly[18])*p1**3
```

The compiler parses both forms and lowers them to the same compiled token.
Dynamic-index `poly[i] = expr` and slice `poly[a..b] = expr` are v2.

In the typed VM, dynamic writes are the write-side mirror of dynamic reads:

```text
poly[poly_len - N] = expr
  -> push_scalar(poly_len)
     push_scalar(N)
     scalar_sub
     ... lower expr to scalar_complex ...
     set_scalar(poly, index, value)
```

`set_scalar(vector_register, scalar_index, scalar_value)` mutates a writable
vector register. It must reject non-real indices, non-integer indices, and
indices outside `[0, vector.length)`. It must not clamp or wrap silently.

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

`emit` is source-level sugar for `poly = pop` when the stack is non-empty.
Under the unified function-call/assignment surface, explicit assignment is the
clear spelling. Bespoke `emit` and `emit_keep` chip names disappear from new
authoring; both are subsumed by assignment / set:

```text
poly = pop      # pop top -> poly                       (canonical spelling)
poly = peek     # read top -> poly, stack unchanged     (replaces emit_keep)
set(poly, peek) # function spelling of the same copy
emit            # alias for poly = pop                  (back-compat sugar)
```

New source compiles `poly = pop`, `poly = peek`, and `set(poly, src)` to
`COEFF_OP_SET`. The existing `COEFF_OP_EMIT` opcode remains accepted for
legacy chip-list programs. Its empty-stack no-op behavior is preserved only
when loading old saved programs that depend on it. New authoring-time `emit`
(or its canonical equivalent `poly = pop`) compiles to a clear error if the
stack is provably empty at that program point, detected by the existing
stack-effect analysis. Programs that mutate `poly` directly via
`poly[...] = expr` or via `poly = chip(poly, ...)` simply have no `emit`
chip; there is no need for a separate "commit" marker.

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
current source       29    16  (4x explicit "+1" via legacy(linear,...,1,1),
                                12x selector triples)
v1 reform            25     0
```

"Structural noise tokens" counts chips and chip arguments that the math
does not require — selector triples like `(push, pop, pop)` and the
`+1` shift after every `arange` for one-based ordinals. The compiled VM
token for `arange(1, poly_len+1)` is the extended `COEFF_OP_RANGE` producer.
The old spelling `push_range(poly_len); legacy(linear, pop, push, 1, 1)` is
only a historical equivalence, not the new lowering. What the author types
and the reader skims past changes from 16 noise tokens to zero.

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

**It eliminates `legacy(...)` from source authoring.** Direct transform
names cover the coefficient-transform registry. Chain mode remains the old
pipeline path; Program source should read like operations, not a bridge
wrapper around the registry.

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

**It should not regress runtime.** v1 replaces several legacy-transform
abuses with direct native opcodes (`SET`, `AFFINE`, extended ranges), so the
runtime claim must be proven by native parity and benchmark tests rather than
assumed from identical lowering. Existing saved S3 programs keep their old
chip-list form and do not need migration; new source programs compile to the
new canonical native form.

The acid test holds: a reader who knows the math should read the program
at roughly the speed of the math. The current source fails that test;
the v1 reform passes it.

## v2 Chips Worth Adding

These need new opcodes and are explicitly v2. Each must justify itself by
either collapsing a real multi-chip pattern or unblocking an expressive
gap.

### `geom(...)` Motivation

`geom` is specified in the Vector Construction catalog below. It belongs
there because geometric sequences appear in many coefficient generators
(Chebyshev-style seeds, exponential weighting). Today they require `arange;
pow` or a custom loop. A native construction chip is one fused op and a clear
tool for the job.

### `axis(length, lo, hi)` And `cheb_axis(length)`

Push readable basis axes for coefficient construction. `axis(n, -1, 1)` is
the old `vec_index_axis(length)` idea with explicit bounds. `cheb_axis(n)`
pushes Chebyshev nodes `cos(pi*k/(n-1))`, which are common enough to deserve
a named constructor.

### `mask_real`, `mask_imag`

Apply `Re(z)` or `Im(z)` element-wise to the top-of-stack vector. Today
this requires a `poke_tos` loop or a careful `mul` against a real-only
fill vector. One chip is much cleaner.

### Vector Reductions

These are reductions: vector inputs, scalar output. They are especially
useful for projections, normalization factors, and basis construction.

```text
alpha = dot(poly, cf)
scale(1 / norm2(poly))
```

They are not LAPACK features. For the current max vector length, implement
them as tight inline C loops in the typed VM.

Required reductions:

```text
sum(v)        -> scalar_complex   # sum(v[i])
mean(v)       -> scalar_complex   # sum(v) / len(v)
prod(v)       -> scalar_complex   # product(v[i]), sanitize overflow/non-finite
dot(a, b)     -> scalar_complex   # sum(conj(a[i]) * b[i])
dotu(a, b)    -> scalar_complex   # sum(a[i] * b[i])
norm2(v)      -> scalar_real      # sqrt(real(dot(v, v)))
norm1(v)      -> scalar_real      # sum(abs(v[i]))
norminf(v)    -> scalar_real      # max(abs(v[i]))
maxabs(v)     -> scalar_real      # alias for norminf(v)
argmaxabs(v)  -> scalar_int       # first index with maximum abs(v[i])
argminabs(v)  -> scalar_int       # first index with minimum abs(v[i])
```

All reductions reject empty vectors. `dot` / `dotu` require equal vector
lengths. Index-returning reductions produce real integer scalar values so
they can feed `get_scalar`, `set_scalar`, `slice`, and future `take`.

### Elementwise Vector Helpers

These are vector-to-vector ops that remove awkward expression loops and
legacy shims:

```text
real(v)       -> vector_complex   # re=v.re, im=0
imag(v)       -> vector_complex   # re=v.im, im=0
phase(v)      -> vector_complex   # alias for angle(v), re=atan2(im,re), im=0
arg(v)        -> vector_complex   # alias for angle(v)
unit(v)       -> vector_complex   # v / abs(v), zero -> 0
clip_abs(v, lo, hi) -> vector_complex
normalize(v)  -> vector_complex   # explicit normalized-vector transform
center(v)     -> vector_complex   # v - mean(v)
standardize(v)-> vector_complex   # center(v) / norm2(center(v)); zero norm -> 0
```

`clip_abs` preserves phase and clamps magnitude into `[lo, hi]`. `lo` and
`hi` are real scalar expressions; reject negative values and `lo > hi`.
`normalize` should use the existing legacy behavior if it is already well
defined; otherwise pin it as `v / norminf(v)` with zero norm -> zero vector.

### Vector Construction

These are readable aliases or fused constructors:

```text
zeros(n)              -> vector_complex   # fill(n, 0)
ones(n)               -> vector_complex   # fill(n, 1)
repeat(value, n)      -> vector_complex   # alias for fill(n, value)
basis(n, i, value=1)  -> vector_complex   # all zero except index i
geom(n, r)            -> vector_complex   # 1, r, r^2, ...
axis(n, lo, hi)       -> vector_complex   # linspace(lo, hi, n)
cheb_axis(n)          -> vector_complex   # cos(pi*k/(n-1)), k=0..n-1
```

`basis` rejects out-of-range indices; no clamp/wrap. `geom` should evaluate
iteratively (`x *= r`) instead of calling `pow` per element. `axis` is
mostly readability sugar over `linspace`; `cheb_axis` is worth having
because Chebyshev-style bases show up often and are easy to get off by one.

### Vector Structure

These reshape or combine vectors:

```text
reverse(v)                 -> vector_complex
diff(v)                    -> vector_complex   # v[i+1] - v[i], length n-1
slice(v, start, stop, step)-> vector_complex
concat(a, b)               -> vector_complex
pad(v, n, side="hi")       -> vector_complex
trim(v, n, side="hi")      -> vector_complex
take(v, indices)           -> vector_complex
```

`reverse` is the source-level name for legacy `rev`. `diff` rejects vectors
with length < 2. `concat` rejects output length > `MAX_VECTOR_LEN`. `pad`
extends with zeros; `trim` removes from the selected side. `side` is `"hi"`
or `"lo"`; `"hi"` means append/remove high-index coefficients. `slice` and
`take` depend on typed dynamic-index support and must reject non-real,
non-integer, or out-of-range indices.

### Masked Selection

`where(mask, a, b)` is the vector conditional:

```text
where(mask, a, b) -> vector_complex
```

Semantics:

```text
out[i] = a[i] if real(mask[i]) > 0 else b[i]
```

The condition is strictly `> 0`; zero selects `b`. The mask's imaginary part
is ignored. `a` and `b` may be vectors or broadcast scalars. All vector inputs
must have equal length after broadcast. This should land after
broadcast/type inference is stable.

The stack sugar is:

```text
mask
a
b
where
```

Execution pops in normal stack order:

```text
b    = pop first
a    = pop second
mask = pop third
```

Avoid a user-facing `popif` name. If a selector form is ever exposed, prefer
`where(push, pop, pop, pop)` only as a compiled/debug representation; the
source form should remain `where(mask, a, b)` because it names the roles.

Scalar conditionals use the same rule:

```text
select(cond, x, y) -> scalar_complex
```

`select` returns `x` when `real(cond) > 0`, otherwise `y`. It is the scalar
equivalent of `where`, useful inside scalar expressions and future typed
function arguments.

### Deterministic Random Generators

Random generators are useful for coefficient experiments but must be
deterministic per row and reproducible across preview/compute:

```text
random_uniform(n, seed_expr) -> vector_complex
random_normal(n, seed_expr)  -> vector_complex
```

The seed must mix `t1`, `t2`, `p1`, `p2`, the explicit `seed_expr`, and the
element index. Use `splitmix64` to derive deterministic per-row/per-element
state and `PCG32` for uniform 32-bit draws. `random_normal` uses Box-Muller
from two PCG32 uniform draws. These are not allowed to call host/runtime
random APIs.

### `roll(k)`, `reverse`, `cumsum`, `diff` As Bare Chip Names

These exist in the coefficient transform registry today. Promote them to
top-level chip/source names so authors do not have to remember the
compatibility wrapper. They lower to `COEFF_OP_NATIVE_TRANSFORM` with a
stable function index, not to user-authored `legacy(...)`.

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
- `chain` is the canonical lowered chip-as-list form produced by the
  backend parser/compiler. It is what the compiler accepts as input
  regardless of how it was produced (text editor, chip editor, macro
  expansion). Old saved programs that pre-date
  `source_text` continue to load through `chain` alone.
- `tokens` and `fingerprint` are the compiled runtime form, derived from
  `chain` by the compiler. They never participate in authoring.

When a program is loaded:

- If `source_text` is present, the editor displays it directly. `chain`
  is used only for compilation and preview display.
- If `source_text` is absent (legacy saved programs), the editor renders
  `chain` through the backend canonicalizer and treats that rendered text as
  the editing surface.

When a program is saved:

- The editor's text buffer is written to `source_text`.
- A Lambda-side Python parser lowers `source_text` to the canonical `chain`.
- The compiler produces `tokens` and `fingerprint` from `chain`.

If a client sends both `source_text` and `chain`, `source_text` is
authoritative. The server compiles `source_text` and ignores the
client-supplied `chain` for writes. A client-supplied `chain` is accepted
only when `source_text` is absent, which is the legacy saved-program path.
Read paths may use the stored `chain` as a display/compile optimization, but
save and compute must never trust a stale or mismatched client-generated
`chain` over `source_text`.

### Authoritative Parser

The authoritative source parser lives in Python Lambda code, not in
`index.html`. JavaScript is editor UX only:

- render the textarea/source editor
- provide a `Text` tab in the Compute tab's Coeff Program section alongside
  the chip/preview view
- request validation on debounce
- display diagnostics with line/column/source spans
- display the canonical chip preview returned by the backend
- import/export plain text

JavaScript may contain a lightweight tokenizer for highlighting and advisory
diagnostics during typing latency, but it must not be the authority for saved
or computed programs. Backend diagnostics override advisory JS diagnostics.
Storage and compute paths must be able to compile `source_text` without
trusting a JS-generated `chain`.

Add a backend compile endpoint, for example:

```text
POST /compile-coeff-program-source
{
  "name": "poly_1",
  "source_text": "arange(1, poly_len+1)\n..."
}
```

Compile endpoints are per-profile routes over the same backend parser/lowering
library. A future Param source editor should use a matching profile-specific
route such as `/compile-param-program-source`, not a single untyped universal
route.

Response:

```json
{
  "ok": true,
  "program": {
    "chain": [],
    "display": "...",
    "fingerprint": "sha1...",
    "statement_count": 0,
    "token_count": 0
  },
  "diagnostics": [
    {
      "level": "error",
      "line": 12,
      "column": 8,
      "end_line": 12,
      "end_column": 11,
      "message": "unknown chip: foo"
    }
  ]
}
```

The save route calls the same Python parser before writing S3. The compute
plan route also accepts `source_text` and compiles it through the same path
if a caller sends source instead of a pre-lowered `chain`. There must be only
one parser implementation that decides whether a program is valid.

### Comments And Formatting

Comments and whitespace survive in `source_text` exactly as authored.
They do not affect `chain`, `tokens`, or `fingerprint`. The fingerprint
hashes the compiled token stream only, so reformatting source never busts
artifact caches. Two programs that differ only in comments produce
identical compiled output.

### Old Saved Programs

- The compiler accepts both old and new spellings forever. The compatibility
  contract is semantic, not token identity: old chip-list JSON keeps its
  existing legacy tokens, while new source aliases may lower to newer native
  tokens such as `COEFF_OP_SET` or `COEFF_OP_AFFINE`.
- Saved-form JSON without `source_text` continues to load through
  `chain` alone. The editor synthesizes a `source_text` view from
  `chain` on load for display, and writes it back on the next save.
- The macro registry round-trips through the same parser/canonicalizer.
- Tests assert: every old-form program loads, compiles, and renders
  identically to its historical behavior. New source spellings must match
  old-form output numerically where they are semantic aliases, but they do
  not have to share fingerprints because they may lower to native opcodes.
  Fingerprints for old programs do not change when they acquire a
  `source_text` field on the next save.

### Macro Storage And Expansion

Coeff Program macros are saved Coeff Programs under the existing
`polypaint/coeff-programs/` prefix. A macro body may be stored as:

- `source_text` plus canonical `chain`
- legacy `chain` only

When compiling `macro(name)`, the resolver fetches the saved object. If
`source_text` exists, it is compiled through `coeff_program_source.py` and
the stored `chain` is treated only as a cached hint. If `source_text` is
absent, the legacy `chain` path is used. Macro expansion happens after
source-to-chain lowering and before token lowering/fingerprinting.

Compute plans snapshot the expanded macro body at plan time. Later edits to
the saved macro do not change an already planned compute job. Live preview
and save-time validation read the current saved macro body.

## Implementation Targets

v1 work lives in:

- **Compiler** — `lambda/coeff_program_chain.py`. Extend
  `_lower_chip`, `_lower_chain`, and the chain/source normalization entry
  points for the new aliases (`arange`, `fill`, `scale`, `shift`, `pow`);
  widen `arange`/`linspace` arity; add `**` and `tau`/`tau_i` to the scalar
  expression parser; add `poly[N]` bracket parsing alongside the existing
  `polyN` form; add the authoritative Python source parser; parse
  `poly[N] = expr` as sugar for `poke_poly`; add `COEFF_OP_SET` for
  `set(poly, src)` / `poly = src`; add `COEFF_OP_AFFINE` for complex
  `linear/scale/shift`; extend `COEFF_OP_RANGE` / `COEFF_OP_LINSPACE`
  argument support; produce diagnostics with line/column/source-span
  metadata.
- **Storage/API** — `lambda/handler_storage.py` or a dedicated handler.
  Add `/compile-coeff-program-source`; update save/fetch routes so
  `source_text` is stored verbatim and the backend parser/compiler produces
  `chain`, `display`, and `fingerprint`.
- **Compute Plan** — `lambda/handler_compute_plan.py`. Accept either
  canonical `coeff_program_chain` or `coeff_program_source_text`; when source
  is present, compile it through the same Python parser path before emitting
  workflow payloads.
- **Editor** — `index.html`. Update `_cpCatalog`,
  `_normalizeCoeffProgramChain`, `_serializeCoeffProgramChain`, and
  `_validateCoeffProgramUiChain` for the new chip names; add a text
  editor surface that round-trips through `source_text`; add a `Text` tab in
  the Compute tab's Coeff Program section so users can author source directly
  without opening the saved-program modal; call the backend compile endpoint
  for validation/canonical preview; do not implement the authoritative
  text-to-chain parser in JS.
- **Native** — `lambda/sweep_cli.c`. Add `COEFF_OP_SET` as a direct
  vector-copy opcode, add `COEFF_OP_AFFINE`, and extend `COEFF_OP_RANGE` /
  `COEFF_OP_LINSPACE` parsing/evaluation. Extend `COEFF_OP_VECTOR_UNARY`
  for `neg`, `conj`, `sqrt`, and `log`. No legacy transform should be used
  to implement these source primitives.
- **Tests** — `tests/test_coeff_program_chain.py`,
  `tests/test_coeff_program_native.py`, `tests/test_frontend_js.sh`,
  `tests/test_coeff_program_storage.py`. Add round-trip assertions for
  `source_text`, alias-equivalence assertions, and old-saved-form
  loading assertions. Add API tests proving save and compute reject invalid
  `source_text` even if the client sends a stale or mismatched `chain`.

Phase 2A work lives in the same files:

- **Compiler** — `lambda/coeff_program_chain.py` adds hidden typed opcodes
  and stack validation for scalar/vector stack entries. `lambda/coeff_program_source.py`
  lowers mixed source expressions and dynamic index syntax to those hidden
  typed rows.
- **Native** — `lambda/sweep_cli.c` uses the existing fixed ring stack for
  vector slots and adds per-slot scalar storage plus typed dispatch for
  broadcast, dynamic `get_scalar`, dynamic `set_scalar`, and typed `fill`.
- **Tests** — `tests/test_coeff_program_chain.py` checks typed lowering and
  stack typing; `tests/test_coeff_program_native.py` verifies the rebuilt
  native binary executes dynamic indexing and vector/scalar broadcast.

Remaining full-v2 work additionally:

- **Compiler** — vector-expression parser, scalar/vector type inference
  with broadcast, typed-VM token lowering, dynamic-index opcodes, slice
  lowering, `for_each_index` body compilation, new chips (`geom`,
  `mask_real`, `mask_imag`).
- **Editor** — richer diagnostics for the unified grammar.
- **Native** — shared typed-VM execution core with profile configuration;
  new opcodes (`EXPR_POLY_AT_DYN`, `COEFF_OP_GEOM`, `COEFF_OP_MASK_REAL`,
  `COEFF_OP_MASK_IMAG`, `COEFF_OP_FOR_EACH_INDEX`, slice-write opcodes);
  typed push ops for what the scalar sublanguage currently reads internally.

## Concrete v1 Implementation Steps

This section is the implementation ticket. It names the files, new symbols,
route wiring, and tests needed for the text-source v1 cut. Do these steps in
order; each step should be deployable without breaking existing chain-based
Coeff Programs.

### 0. Native/Compiler Source Primitives

Add the real source primitives before the source parser lands. Assignment,
affine transforms, and generalized range producers must not depend on legacy
identity/shift/scale chains.

Modify `lambda/coeff_program_chain.py`.

- add `COEFF_OP_SET = 19` after the existing `COEFF_OP_RANGE = 18`
- add display name mapping: `COEFF_OP_SET: "set"`
- add source parser support for chip-list form:

  ```python
  ["set", "poly", "peek"]
  ["set", "poly", "pop"]
  ["set", "poly", "cf"]
  ["set", "poly", "poly"]
  ```

- v1 target is only `poly`; reject `set(push, src)` because `push(src)`
  already exists and the assignment primitive is about writable registers
- source selectors allowed for v1: `cf`, `poly`, `pop`, `peek`
- stack validation:
  - `set(poly, pop)` requires depth >= 1 and decrements depth by 1
  - `set(poly, peek)` requires depth >= 1 and leaves depth unchanged
  - `set(poly, cf)` copies `cf`
  - `set(poly, poly)` is accepted as an idempotent no-op self-copy
- display form should render as `set(poly,<src>)`
- canonical source rendering should prefer assignment:
  - `set(poly,pop)` -> `poly = pop`
  - `set(poly,peek)` -> `poly = peek`
  - `set(poly,cf)` -> `poly = cf`
  - `set(poly,poly)` -> `poly = poly` only when explicitly loaded; the
    renderer should not invent this no-op

Modify `lambda/sweep_cli.c`.

- add enum value `COEFF_OP_SET = 19`
- no JSON shape change is needed; the token already carries `op`, `src`,
  and `tgt`
- compiled token JSON uses integer op/selectors, not selector strings:

  ```json
  {"op": 19, "src": 4, "tgt": 2}
  ```

  Selector constants are the existing values:

  ```text
  cf=1, poly=2, pop=3, peek=4, push=5
  ```

  `COEFF_OP_SET` only accepts `tgt=2` (`poly`). The example above is
  `set(poly, peek)` / `poly = peek`.
- evaluator behavior:

  ```text
  COEFF_OP_SET:
    reject unless tgt == COEFF_SEL_POLY
    resolve src vector from cf/poly/pop/peek
    copy src real/imag arrays and length into ws->poly_re/ws->poly_im/poly_len
    if src == pop, consume the stack slot
    if src == peek, leave stack depth/head unchanged
  ```

- do not call a legacy transform
- do not apply `andy`
- do not consult `coeff_legacy_registry.json`
- sanitize copied values the same way other vector-producing ops sanitize
  before writing `poly`
- error messages must include the source selector and current stack depth
  when `pop`/`peek` fails

Tests:

- add compiler tests for `["set","poly","peek"]`,
  `["set","poly","pop"]`, and invalid `["set","push","peek"]`
- add native parity test proving `set(poly,peek)` copies the top vector
  without consuming it
- add native parity test proving `set(poly,pop)` copies and consumes the top
  vector

Keep `COEFF_OP_AFFINE` for existing chip-list compatibility, but do not use
it for new source `linear`/`scale`/`shift` lowering. New source lowers those
forms to typed vector/scalar operations in the main stack.

Modify `lambda/coeff_program_chain.py`.

- add `COEFF_OP_AFFINE = 20`
- source/chip-list forms:

  ```python
  ["affine", "push", "pop", "a_expr", "b_expr"]
  ["affine", "push", "poly", "a_expr", "b_expr"]
  ["affine", "poly", "poly", "a_expr", "b_expr"]
  ```

- `linear(a,b)` lowers to typed `pop * a + b`
- `linear(src,a,b)` lowers to typed `src * a + b`
- `poly = linear(poly,a,b)` lowers to typed `poly * a + b` then
  `COEFF_OP_TYPED_SET_POLY`
- `scale(a)` lowers to typed multiply
- `shift(b)` lowers to typed add
- `a_expr` and `b_expr` are complex scalar expressions, not real-only args
- compiled token JSON reuses the existing `args`, `args_im`, and `expr_refs`
  fields. No new fields are added:

  ```json
  {
    "op": 20,
    "src": 3,
    "tgt": 5,
    "n_args": 2,
    "args": [0.0, 1.0],
    "args_im": [1.0, 0.0],
    "expr_refs": [-1, -1]
  }
  ```

  This example is `affine(push, pop, 1j, 1)`. Dynamic scalar expressions use
  the existing convention: the literal slot is `0.0 + 0.0j` and `expr_refs[i]`
  points into `scalar_exprs`; static args use `expr_refs[i] = -1`.
- stack validation mirrors a unary vector op:
  - `src=pop` requires depth >= 1 and consumes one vector
  - `src=peek` requires depth >= 1 and does not consume
  - `src=cf/poly` leaves stack unchanged
  - `tgt=push` pushes one vector; `tgt=poly` writes `poly`

Modify `lambda/sweep_cli.c`.

- add enum value `COEFF_OP_AFFINE = 20`
- token carries `src`, `tgt`, and two scalar-expression args
- evaluator computes `out[i] = src[i] * a + b` with complex `a` and `b`
- do not call legacy `linear`
- do not apply `andy`
- sanitize non-finite outputs to zero using the same policy as vector ops

Compatibility tests:

- old explicit `["affine", ...]` chip-list forms compile and run through
  `COEFF_OP_AFFINE`
- `scale(1j*p1)` and `shift(p2)` source compile and run through typed stack
  tokens, not `COEFF_OP_AFFINE`
- `linear(poly, 0.5, p1+p2)` does not produce a legacy token
- native parity confirms `affine` matches a Python complex affine reference

Canonical display / source rendering:

- `affine(push,pop,a,0)` renders as `scale(a)`
- `affine(push,pop,1,b)` renders as `shift(b)`
- `affine(push,pop,a,b)` renders as `linear(a,b)`
- `affine(push,src,a,b)` renders as `linear(src,a,b)`
- `affine(poly,src,a,b)` renders as `poly = linear(src,a,b)`
- do not render source `linear/scale/shift` as `legacy(linear,...)`

Extend `COEFF_OP_VECTOR_UNARY`.

Modify `lambda/coeff_program_chain.py`.

- keep existing native unary ids stable:
  - `angle = 1`
  - `mod = 2`
  - `abs = 3`
- add new native unary ids:
  - `neg = 4`
  - `conj = 5`
  - `sqrt = 6`
  - `log = 7`
- source/chip-list forms:

  ```python
  ["sqrt", "push", "pop"]
  ["log", "push", "poly"]
  ["neg", "poly", "poly"]
  ```

- bare `sqrt`, `log`, `neg`, and `conj` pop one vector and push one vector
- `sqrt(src)`, `log(src)`, `neg(src)`, and `conj(src)` read the explicit
  source and push the result
- `poly = sqrt(poly)`, `poly = log(poly)`, `poly = neg(poly)`, and
  `poly = conj(poly)` write `poly`
- stack validation mirrors all unary vector ops:
  - `src=pop` requires depth >= 1 and consumes one vector
  - `src=peek` requires depth >= 1 and does not consume
  - `src=cf/poly` leaves stack unchanged
  - `tgt=push` pushes one vector; `tgt=poly` writes `poly`

Modify `lambda/sweep_cli.c`.

- extend the `COEFF_OP_VECTOR_UNARY` switch
- `neg(z) = -z`
- `conj(z) = conjugate(z)`
- `sqrt(z)` uses the principal complex square root
- `log(z)` uses the principal complex logarithm; `log(0)` should follow the
  existing scalar-expression policy and produce a finite sanitized fallback,
  not NaN/inf
- sanitize non-finite outputs to zero using the existing vector-op policy

Tests:

- `sqrt`, `log`, `neg`, and `conj` compile to `COEFF_OP_VECTOR_UNARY`
- native parity checks each against Python/cmath references
- `poly = neg(poly)` writes `poly` without pushing
- `sqrt(peek)` does not consume the stack

Extend `COEFF_OP_RANGE` and `COEFF_OP_LINSPACE`.

Modify `lambda/coeff_program_chain.py`.

- keep old chip-list forms accepted:
  - `["push_range", "poly_len"]`
  - `["push_linspace", "poly_len"]`
- allow new forms:
  - `["push_range", stop]`
  - `["push_range", start, stop]`
  - `["push_range", start, stop, step]`
  - `["push_linspace", count]`
  - `["push_linspace", start, stop, count]`
- source aliases:
  - `arange(...)` lowers to `COEFF_OP_RANGE`
  - `linspace(...)` lowers to `COEFF_OP_LINSPACE`
- arguments are real-valued scalar expressions
- canonical display should prefer `arange(...)` and `linspace(...)`

Modify `lambda/sweep_cli.c`.

- keep enum ids `COEFF_OP_RANGE = 18` and `COEFF_OP_LINSPACE = 17`
- extend parser/evaluator to read expression-backed `start`, `stop`, `step`,
  and `count`
- `arange(stop)`: start=0, step=1, count=stop
- `arange(start, stop)`: step=1, count=ceil((stop-start)/step) with exact
  NumPy-style stop-exclusive semantics for positive/negative step
- `arange(start, stop, step)`: reject step=0; compute stop-exclusive count
- `linspace(count)`: old behavior, 0..1 inclusive with count entries
- `linspace(start, stop, count)`: inclusive start..stop with count entries
- `linspace(start, stop, 1)` produces `[start]`; it does not produce `stop`
  or the midpoint
- reject non-real args, non-integer counts, counts outside
  `[1, COEFF_PROGRAM_MAX_VECTOR_LEN]`, and non-finite generated values

Tests:

- `arange(poly_len)`, `arange(0, poly_len)`, and
  `arange(1, poly_len+1)` produce expected vectors
- `arange(5, 0, -1)` produces `[5, 4, 3, 2, 1]`
- `arange(0, 5, 0)` rejects cleanly
- `linspace(2, 4, 3)` produces `[2, 3, 4]`

### 1. Backend Parser Module

Add `lambda/coeff_program_source.py`.

Responsibilities:

- parse Coeff Program `source_text` into canonical chip-list `chain`
- render legacy `chain` back to canonical source text for old saved programs
- preserve user-authored `source_text` exactly on save; do not format it
- strip comments and whitespace only while lowering to `chain`
- produce diagnostics with line/column/source-span metadata
- expose one compile helper used by storage, compute plan, and preview paths

Public API:

```python
def parse_coeff_program_source(source_text: str) -> dict:
    """Return {'chain': list, 'diagnostics': list, 'statement_count': int}."""

def render_coeff_program_source(chain: list) -> str:
    """Return canonical source text for a legacy chain-only program."""

def compile_coeff_program_source(
    source_text: str,
    *,
    name: str = "",
    macro_resolver=None,
    program_id: str | None = None,
) -> dict:
    """Parse source, compile chain, return chain/display/fingerprint/etc."""

def compile_coeff_program_payload_from_source_or_chain(
    *,
    name: str,
    source_text=None,
    chain=None,
    saved_at=None,
    version=None,
    program_id=None,
    macro_resolver=None,
) -> dict:
    """source_text wins over chain. Used by storage save/fetch/compile routes."""
```

Diagnostic shape:

```json
{
  "level": "error",
  "line": 12,
  "column": 8,
  "end_line": 12,
  "end_column": 11,
  "message": "unknown chip: foo"
}
```

Parser v1 grammar:

Lexical rules:

```text
IDENT        ::= [A-Za-z_][A-Za-z0-9_]*
DIGITS       ::= [0-9]+
EXP          ::= [eE][+-]? DIGITS
FLOAT        ::= DIGITS "." DIGITS? EXP? | "." DIGITS EXP? | DIGITS EXP?
IMAG_NUMBER  ::= FLOAT [ij]
NUMBER       ::= FLOAT
COMMENT      ::= "#" .* until newline
```

`2.5j`, `1j`, `.5j`, and `1e-3j` lex as one `IMAG_NUMBER`, not as
`NUMBER * j`. Bare `i` and `j` remain identifiers that the scalar-expression
parser resolves to `0+1j`. Whitespace is insignificant except as a token
separator. Indentation has no meaning. Quoted strings are not part of v1.

Statement splitting:

- newline ends a statement only at bracket depth zero
- `;` ends a statement only at bracket depth zero
- bracket depth tracks `()`, `[]`, and `{}` even though `{}` is rejected in v1
- newline inside parentheses or brackets is a line continuation
- `#` starts a comment only outside parentheses/brackets; comments are
  removed before statement parsing but preserved verbatim in `source_text`
- blank/comment-only statements are ignored

Formal grammar:

```text
program        ::= sep* statement? (sep+ statement)* sep*
sep            ::= NEWLINE | ";"
statement      ::= assignment | index_assignment | call_statement | bare_statement

assignment     ::= "poly" "=" rhs
index_assignment ::= "poly" "[" INT "]" "=" scalar_expr
rhs            ::= source_name | call_expr | bare_chip_name

call_statement ::= IDENT "(" arg_list? ")"
bare_statement ::= IDENT | source_name
arg_list       ::= arg ("," arg)*
arg            ::= source_name | target_name | IDENT | scalar_expr

call_expr      ::= IDENT "(" arg_list? ")"
source_name    ::= "cf" | "poly" | "pop" | "peek"
target_name    ::= "poly" | "push"
bare_chip_name ::= IDENT
```

Scalar-expression grammar:

```text
scalar_expr ::= sum
sum         ::= product (("+" | "-") product)*
product     ::= unary (("*" | "/") unary)*
unary       ::= ("+" | "-") unary | power
power       ::= primary ("**" unary)?        # right-associative
primary     ::= NUMBER | IMAG_NUMBER | IDENT | indexed_ref |
                IDENT "(" scalar_expr ")" | "(" scalar_expr ")"
indexed_ref ::= ("poly" | "cf" | "tos") "[" INT "]"
```

Precedence matches Python for the supported operators: function/index calls,
then `**` (right associative), then unary `+`/`-`, then `*`/`/`, then `+`/`-`.
So `-2**2` parses as `-(2**2)`, and `2**-3` parses as `2**(-3)`.
General expression exponents are v2; v1 accepts integer literal exponents
only after parsing.

Parser error recovery:

- report diagnostics for as many independent statements as possible
- after an error in a statement, skip to the next top-level newline or `;`
- lexical/splitting errors such as unmatched parentheses or brackets halt
  parsing because statement boundaries are no longer reliable
- do not emit partial lowered chips for a statement that has an error
- the compile endpoint returns HTTP 400 when any `level="error"` diagnostic
  exists, but still includes all collected diagnostics

Empty or whitespace/comment-only source is valid and compiles to an empty
chain with `statement_count=0`. This means "leave `poly` at its initial
cf-derived value", matching an empty chip program.

`statement_count` counts source statements after comment stripping and
top-level `;` splitting, before lowering. A statement that lowers to multiple
chips still counts as one statement. Blank/comment-only statements do not
count.

Canonical expression rendering:

- `source_text` is preserved exactly and never rewritten
- v1 `chain` expression args are normalized strings, not full AST-rendered
  expressions; they strip internal whitespace and normalize supported aliases
- aliases normalize in chain args: `tau -> pi2`, `tau_i -> pi2i`,
  `mod(x) -> abs(x)`
- fingerprints hash compiled tokens and scalar-expression bytecode, not raw
  expression text
- therefore `1+2`, `1 + 2`, and `(1+2)` compile to the same fingerprint
- cosmetically different source may still produce different `chain` JSON in
  v1 if the remaining expression text differs after whitespace/alias
  normalization; full AST rendering is deferred to typed-VM v2

Statement lowerings:

```text
arange(n)                 -> ["push_range", n]
arange(start, stop)       -> ["push_range", start, stop]
arange(start, stop, step) -> ["push_range", start, stop, step]
fill(n, expr)             -> ["push_const", n, expr]
linspace(n)               -> ["push_linspace", n]
linspace(start,stop,n)    -> ["push_linspace", start, stop, n]
cf                        -> ["push", "cf"]
poly                      -> ["push", "poly"]
dup                       -> ["duplicate"]
drop                      -> ["pop"]
flush                     -> ["flush"]
scale(a)                  -> typed multiply
shift(b)                  -> typed add
linear(a,b)               -> typed multiply then typed add
linear(src,a,b)           -> typed source, multiply, add
abs/mod/angle/neg/conj/
sqrt/log                 -> native unary: [name, "push", "pop"]
sin / cos / exp          -> registry-backed unary sugar when available
sin(src)                  -> ["_native_transform", "sin", src, "push"]
add                       -> ["add", "push", "pop", "pop"]
mul                       -> ["multiply", "push", "pop", "pop"]
pow                       -> ["power", "push", "pop", "pop"]
sub                       -> ["subtract", "push", "pop", "pop"]
div                       -> ["divide", "push", "pop", "pop"]
mul(poly,cf)              -> ["multiply", "push", "poly", "cf"]
set(poly,src)             -> ["set", "poly", src]
poly = pop                -> ["set", "poly", "pop"]
poly = peek               -> ["set", "poly", "peek"]
poly = cf                 -> ["set", "poly", "cf"]
poly = sin                -> ["_native_transform","sin","pop","poly"]
poly = sin(poly)          -> ["_native_transform","sin","poly","poly"]
poly = mul(poly,cf)       -> ["multiply", "poly", "poly", "cf"]
poly[N] = expr            -> typed index/value + ["_typed_poke_poly"]
legacy(name,src,tgt,...)  -> invalid in source text; use direct names or Chain mode
macro(name)               -> ["macro", name]
```

Assignment lowering rule:

- `poly = <source>` lowers to `["set", "poly", <source>]`
- `poly = <bare-chip>` lowers by applying that chip to `pop` with `tgt=poly`
  (`poly = sin` -> `["legacy", "sin", "pop", "poly"]`)
- `poly = <chip>(...)` lowers through that chip's normal native/legacy opcode
  with `tgt=poly`
- `poly = mul(poly, cf)` is therefore `["multiply", "poly", "poly", "cf"]`,
  not `mul(...); emit`
- if a chip has no `tgt=poly` form, assignment to `poly` is a compile error

Two v1 caveats must be enforced by the parser:

- `poly = peek` and `set(poly, peek)` require `COEFF_OP_SET`. Do not lower
  them through `legacy(linear, peek, poly, 1, 0)`.
- `linear`, `scale`, and `shift` source requires typed vector/scalar lowering.
  Do not lower them through legacy `linear` or the old scalar-expression
  `COEFF_OP_AFFINE` path.
- `arange(start, stop, step)` and `linspace(start, stop, n)` require the
  extended native producer support above. Do not synthesize them by applying
  affine transforms to a zero-based range.
- Unary sugar is only available when the operation is either a native vector
  unary (`abs`, `mod`, `angle`, `neg`, `conj`, `sqrt`, `log`) or a
  registry-backed transform. `sqrt`, `log`, and `neg` are v1 native unary
  operations, not legacy aliases.

Expression updates in `lambda/coeff_program_chain.py`:

- add `tau` and `tau_i` as aliases for `pi2` and `pi2i`
- add bracket reads `poly[N]`, `cf[N]`, `tos[N]` alongside existing `polyN`,
  `cfN`, `tosN`
- add `**` for integer literal exponents only in v1
- reject general complex exponent expressions in v1 with a clear message
- preserve existing `polyN`/`cfN`/`tosN` forever for old chains

### 2. Storage/API Routes

Modify `lambda/handler_storage.py`.

Imports:

```python
from coeff_program_source import (
    compile_coeff_program_payload_from_source_or_chain,
    compile_coeff_program_source,
    render_coeff_program_source,
)
```

Constants:

```python
MAX_COEFF_PROGRAM_SOURCE_BYTES = 64 * 1024
```

The limit is measured in UTF-8 bytes. Exceeding it returns HTTP 400 with a
diagnostic in the same shape as parser errors; it must not write S3.

Modify `_read_coeff_program_source_chain(program_id)`:

- read saved object
- if `chain` exists, return it as today
- if only `source_text` exists, compile source through
  `compile_coeff_program_source(...).get("chain")`
- macro resolution must never use client-provided chains

Modify `_compile_coeff_program_payload(...)` or replace it with a wrapper:

- accept `source_text=None` and `chain=None`
- if `source_text` is present, validate byte size, compile source with Python
  parser, and ignore any client-supplied `chain`; empty/whitespace source is
  a valid empty program
- if `source_text` is absent, keep legacy chain path exactly as today
- include `source_text` in the saved program only when present
- include canonical `chain`, `display`, `expanded_display`, `fingerprint`,
  `execution_spec`, `statement_count`, `token_count`, `scalar_expr_count`,
  `stack_max`, `uses_legacy_chain_equivalent`, `macro_expansions`

All compile sites must use the same macro resolver semantics. Storage,
compute-plan, and compute-preview should all resolve through the same
`_coeff_program_macro_resolver` helper or a shared equivalent; no path may
expand macros from a client-supplied stale `chain` when `source_text` exists.
Macro bodies with `source_text` compile through the source parser during
resolution; macro bodies without `source_text` use the stored legacy `chain`.

Add handler:

```python
def handle_compile_coeff_program_source(event):
    params = parse_body(event)
    program = compile_coeff_program_payload_from_source_or_chain(
        name=params.get("name") or "coeff-program-source",
        source_text=params.get("source_text"),
        chain=params.get("chain"),
        program_id=params.get("id"),
        macro_resolver=_coeff_program_macro_resolver(params.get("id")),
    )
    return ok_response({"program": program, "diagnostics": program.get("diagnostics", [])})
```

Route wiring in `handler(event, context)`:

```python
elif path.endswith("/compile-coeff-program-source"):
    return _handle_storage_route(handle_compile_coeff_program_source, event)
```

Modify `handle_save_coeff_program(event)`:

- pass `params.get("source_text")` and `params.get("chain")`
- if both are sent, source wins
- store `source_text` verbatim
- S3 metadata remains name / statement_count / saved_at

Modify `handle_fetch_coeff_program(event)`:

- if stored object lacks `source_text`, add synthesized
  `source_text = render_coeff_program_source(program["chain"])` in the
  response only
- do not mutate S3 on fetch

Deploy route wiring in `deploy.sh`:

- add `lambda/coeff_program_source.py` to every Lambda zip that imports it:
  storage, compute-plan, compute-preview, coeffgen if needed by compute source
  fallback
- add API Gateway route:

```bash
ensure_route "POST /compile-coeff-program-source" "$STORAGE_INT"
```

### 3. Compute/Preview Source Support

Modify `lambda/handler_compute_plan.py`.

- import `compile_coeff_program_payload_from_source_or_chain` or a lower-level
  source compile helper from `coeff_program_source.py`
- read `coeff_program_source_text` by key presence, not truthiness; an empty
  string is a valid empty program
- in Program mode, source wins over `coeff_program_chain`
- compile source to canonical chain before existing coeff-program compilation
  payload generation
- persist in `plan["pipeline"]`:

```json
{
  "coeff_program_source_text": "...",
  "coeff_program_chain": [...],
  "coeff_program_display": "...",
  "coeff_program_fingerprint": "..."
}
```

- do not forward `source_text` to native payloads
- include source text in calc metadata only for UI populate/debugging

Modify `lambda/handler_compute_preview.py`.

- accept `coeff_program_source_text`
- source wins over `coeff_program_chain` when the key is present, even if the
  value is an empty string
- compile through the same Python parser path
- return diagnostics in `programs.coeff_program.diagnostics`

Modify `lambda/handler_coeffgen.py`.

- Accept `coeff_program_source_text` only as a defensive fallback for direct
  test/manual invocations. Normal workflow payloads should already contain
  compiled `coeff_program` or canonical `coeff_program_chain`.
- If both source and compiled program are present, compiled program wins in
  workflow execution; source is metadata/debug only.
- This is intentionally different from save/plan-time source precedence.
  At plan time the compile decision is final; downstream workflow steps
  execute the compiled program. `source_text` rides through metadata for
  traceability and must never re-decide program validity inside coeffgen.

Modify `lambda/handler_compute_orchestrator.py`.

- preserve `coeff_program_source_text` in `run_params`
- `pipeline_mode` auto-selection should treat source text as Program mode:

```python
"coeff_program_source_text" in run_params
```

Step Functions template:

- file: `stepfunctions/compute_workflow.asl.json.template`
- add selectors anywhere `coeff_program_chain.$` is passed:

```json
"coeff_program_source_text.$": "$.plan.pipeline.coeff_program_source_text"
```

If source is compiled only in compute-plan and never needed downstream, do
not add ASL selectors. Prefer this simpler route unless preview/direct
coeffgen requires source fallback.

### 4. Frontend Compute UI

Modify `index.html`.

Add a tabset inside the Compute tab's Coeff Program section, near
`id="cp-chips"`:

```html
<div class="in-panel-tabs coeff-program-tabs">
  <button id="cp-tab-chips" ...>Chips</button>
  <button id="cp-tab-text" ...>Text</button>
</div>
<div id="cp-chips-panel">existing chip editor</div>
<div id="cp-text-panel" hidden>
  <textarea id="cp-source-text" class="program-source-textarea"></textarea>
  <div id="cp-source-diagnostics" class="program-source-diagnostics"></div>
  <div id="cp-source-preview" class="solve-score-modal-display"></div>
</div>
```

CSS:

- textarea monospace
- fixed/min height comparable to chip editor
- vertical scroll inside textarea
- no modal/compute layout height jumps
- tabs must use the same visual language as existing tabsets, not generic
  buttons

State:

```js
let _coeffProgramEditorMode = 'chips'; // 'chips' | 'text'
let _coeffProgramSourceText = '';
let _coeffProgramSourceCompile = null;
let _coeffProgramSourceTimer = null;
```

New/modified JS functions:

```js
function _selectedCoeffProgramEditorMode()
function _setCoeffProgramEditorMode(mode)
function _readCoeffProgramSourceText()
function _setCoeffProgramSourceText(text, options = {})
async function _compileCoeffProgramSourceTextNow(options = {})
function _scheduleCoeffProgramSourceCompile()
function _effectiveCoeffProgramChainForCompute()
function _effectiveCoeffProgramSourceTextForCompute()
function _populateCoeffProgramTextFromChain(chain)
```

Behavior:

- Chips tab keeps current chip editor behavior.
- Text tab sends `source_text` to `/compile-coeff-program-source` on debounce.
- Backend response updates canonical preview and diagnostics.
- v1 adds the Text/Chips tabset only to the Compute tab's Coeff Program
  editor. The saved-program modal may display source text returned by fetch,
  but it does not get a separate authoritative textarea/editor in v1.
- Compute Preview / Calculate in Program mode:
  - if Text tab is active, send `coeff_program_source_text` even when empty
    (empty text is a valid empty program)
  - otherwise send `coeff_program_chain`
- Saved-program save/load flow:
  - saving from Text mode sends both `source_text` and current `chain` if
    available, but backend ignores `chain` when `source_text` exists
  - saving from Chips mode sends the chip `chain`; backend stores the chain
    and may return synthesized canonical `source_text` for display
  - if a Chips-mode save includes synthesized `source_text`, it must be
    generated by the backend canonicalizer, not by a JS parser
  - fetching a program with `source_text` populates the Compute Text tab and
    shows the canonical chain in preview
- Populate from existing compute:
  - if `pipeline.coeff_program_source_text` exists, fill Text tab
  - otherwise synthesize text from `pipeline.coeff_program_chain` via backend
    fetch/compile route or local display-only renderer

Do not implement authoritative source parsing in JS. JS may do highlighting
or advisory lint only.

### 5. Packaging

Modify `deploy.sh`.

Add `lambda/coeff_program_source.py` to:

- storage Lambda zip
- compute-plan Lambda zip
- compute-preview Lambda zip
- coeffgen Lambda zip only if direct coeffgen source fallback is implemented

Native rebuilds are required because `lambda/sweep_cli.c` changes:

- local smoke binary: `lambda/sweep_test`
- deployed coeffgen binary: `lambda/sweep_coeffgen`
- any Lambda package that includes `sweep_coeffgen`: coeffgen,
  compute-preview, render-lores-preview, and compute-chunk-fused packages

`deploy.sh` already builds `sweep_test` locally and rebuilds
`sweep_coeffgen` in the Docker/LAPACK step. The implementation must keep that
path active; no source-only deploy is valid for this feature.

Add route:

```bash
ensure_route "POST /compile-coeff-program-source" "$STORAGE_INT"
```

Modify `tests/test_deploy_packaging.py`:

- assert `coeff_program_source.py` is present in every zip that imports it
- assert API route list includes `POST /compile-coeff-program-source`

### 6. Tests

Add `tests/test_coeff_program_source.py`.

Required cases:

- empty / whitespace-only source compiles to empty chain with
  `statement_count=0`
- `arange(poly_len)` lowers to `push_range`
- `arange(1, poly_len+1)` lowers to extended `COEFF_OP_RANGE`
- `linspace(2, 4, 3)` lowers to extended `COEFF_OP_LINSPACE`
- `linspace(2, 4, 1)` produces `[2]`
- `fill(poly_len, p1+p2)` lowers to typed scalar work plus `COEFF_OP_TYPED_FILL`
- `scale(0.5)` lowers to typed vector/scalar multiply
- `shift(1)` lowers to typed vector/scalar add
- `scale(1j*p1)` compiles; it must not be rejected as a real-only legacy arg
- `set(poly,peek)` and `poly = peek` lower to `COEFF_OP_SET`, not to
  `legacy(linear,peek,poly,1,0)`
- compiled token JSON for `set` and `affine` matches the documented
  `op`/`src`/`tgt`/`args`/`args_im`/`expr_refs` shape
- `mul(poly,cf)` lowers directly to `multiply(push,poly,cf)`
- `pow` lowers to vector `power`
- `sqrt`, `log`, `neg`, and `conj` lower to `COEFF_OP_VECTOR_UNARY`
- `poly[10] = expr` lowers to `COEFF_OP_TYPED_POKE_POLY`
- `poly[6]`, `cf[6]`, `tos[3]` compile in scalar expressions
- `poly[poly_len-1]` lowers to typed dynamic indexing
- `p1**3` lowers/compiles; `p1**p2` is rejected in v1
- `poly = neg(poly)` writes `poly` through `COEFF_OP_VECTOR_UNARY`, not
  through a legacy transform
- comments and blank lines are ignored for `chain`
- comments and formatting are preserved in `source_text`
- `legacy(...)` source is rejected; direct transform names lower to
  `COEFF_OP_NATIVE_TRANSFORM`
- invalid statement reports line/column
- multiple independent invalid statements report multiple diagnostics where
  recovery is possible
- expression canonicalization gives the same fingerprint for `1+2`, `1 + 2`,
  and `(1+2)`

Update `tests/test_coeff_program_storage.py`.

- `/compile-coeff-program-source` returns `{program:{chain,display,fingerprint}, diagnostics}`
- `/save-coeff-program` with `source_text` stores source verbatim
- if `source_text` and wrong `chain` are both sent, saved `chain` is compiled
  from source
- fetch of chain-only old program returns synthesized `source_text`
- save rejects invalid source and does not write S3
- macro source programs expand by compiling saved `source_text`; chain-only
  legacy macros expand through saved canonical chains

Update `tests/test_compute_plan.py`.

- `coeff_program_source_text` in Program mode compiles to canonical chain,
  including the empty-string case
- source wins over mismatched `coeff_program_chain`
- plan pipeline carries `coeff_program_source_text`
- probe signature/fingerprint changes when source semantics change, not when
  only comments change

Update `tests/test_compute_preview_handler.py`.

- preview accepts `coeff_program_source_text`
- invalid source returns HTTP 400 with parser diagnostic

Update `tests/test_frontend_js.sh`.

Grep/source checks:

- `id="cp-tab-text"`
- `id="cp-source-text"`
- `/compile-coeff-program-source`
- no function named `_parseCoeffProgramSource` or equivalent authoritative
  parser in JS
- compute payload includes `coeff_program_source_text` when Text mode active
- Text tab panel has fixed-height/scroll CSS

Update `tests/test_coeff_program_native.py`.

- compile the `poly_1` v1 source through the Python parser and assert native
  output matches the current chain-based equivalent
- assert `COEFF_OP_SET`, `COEFF_OP_AFFINE`, extended `COEFF_OP_RANGE`, and
  extended `COEFF_OP_LINSPACE` match Python references
- assert native `sqrt`, `log`, `neg`, and `conj` match Python/cmath
  references

### 7. Rollout Gates

Before deploy:

```bash
python3 -m py_compile lambda/coeff_program_source.py lambda/handler_storage.py lambda/handler_compute_plan.py lambda/handler_compute_preview.py lambda/handler_coeffgen.py
uv run python -m pytest tests/test_coeff_program_source.py tests/test_coeff_program_storage.py tests/test_compute_plan.py tests/test_compute_preview_handler.py tests/test_coeff_program_native.py tests/test_deploy_packaging.py -q
bash tests/test_frontend_js.sh
./deploy.sh show-build
bash scripts/test-docker-runtime.sh
bash scripts/predeploy_check.sh
```

Because v1 now adds native `SET`/`AFFINE` behavior and extends range
producers, native parity and the Docker runtime gate are mandatory before
deploy. Do not use the old "native unchanged" shortcut for this change.

### 8. Explicit Non-Work For v1

Do not implement these in v1:

- dynamic `poly[poly_len-N]` reads/writes
- vector-to-scalar reductions such as `dot` and `norm2`
- vector construction/structure primitives such as `basis`, `geom`, `slice`,
  `concat`, `pad`, `trim`, `take`, `where`, and scalar `select`
- deterministic random generators
- slice assignment
- `for_each_index`
- typed VM runtime beyond the additive Phase 2A hidden opcodes
- nested vector calls like `pow(poly, fill(poly_len, p1))`
- general expression exponent `p1**p2`
- authoritative JS parser
- automatic migration of Param Program to typed VM

## Decisions

Items already settled in the body of this document, gathered here so they
do not have to be re-litigated:

- **In-place register mutations** spell as `poly = chip(poly, ...)`. No
  `@`-decoration syntax. The redundancy of `poly = sin(poly)` is visually
  cheap and removes ambiguity about what the chip does.
- **`poly[i]` and `poly[i] = expr` with dynamic indices** are Phase 2A, not
  v1. v1 supports literal indices only. Phase 2A lowers dynamic reads through
  `get_scalar(vector, scalar_index)` and dynamic writes through
  `set_scalar(vector_register, scalar_index, scalar_value)`.
- **`dot` is vector-to-scalar and therefore v2.** It should use the Hermitian
  convention `sum(conj(a[i]) * b[i])`. If the non-conjugating version is
  needed, add `dotu` rather than changing `dot`.
- **All vector reducers and constructors are explicit primitives.** Do not
  hide `sum`, `mean`, `basis`, `geom`, `slice`, `concat`, `pad`, or `trim`
  behind unrelated legacy transforms. They are typed-VM operations with
  direct length/type/error rules.
- **Conditionals use `where` / `select`, not `popif`.** `where(mask,a,b)`
  is vector selection and `select(cond,x,y)` is scalar selection. Both use
  `real(condition) > 0`; zero chooses the false branch. Stack-pop order for
  bare `where` is `b`, then `a`, then `mask`.
- **Random generators must be deterministic.** They mix row parameters and
  an explicit seed expression, use the pinned `splitmix64` + `PCG32` family,
  and never call host runtime randomness.
- **User formatting and comments** survive in `source_text` verbatim.
  Canonicalization happens only when producing `chain` for the compiler;
  it never rewrites `source_text`.
- **Comments do not affect compiled output.** Stripped during
  `source_text -> chain` lowering. Two programs that differ only in
  comments produce identical fingerprints.
- **`tau` does not replace `pi2` in the saved form.** The display layer
  translates `pi2` to `tau` for users who prefer it. No fingerprint
  churn for the rename.
- **`legacy(...)` is removed from source authoring.** Direct transform names
  are the source surface; Chain mode remains the legacy path.

## Summary

The current syntax is not broken; it is just visibly assembled from
incremental decisions. The reform splits cleanly into two scopes.

v1 (ship first):

- Text source/load with `source_text` field; comments and formatting
  preserved verbatim.
- Aliases: `arange`, `fill`, `scale`, `shift`, `pow`.
- `arange(start, stop)` and `arange(start, stop, step)` overloads.
- Native/compiler updates: `COEFF_OP_SET`, `COEFF_OP_AFFINE`, extended
  `COEFF_OP_RANGE` / `COEFF_OP_LINSPACE`, `COEFF_OP_NATIVE_TRANSFORM`,
  `COEFF_OP_TYPED_BLEND`, and vector-unary support for `neg`, `conj`,
  `sqrt`, and `log`.
- `poly[N]`, `cf[N]`, `tos[N]` static-index brackets in expressions.
- `poly[N] = expr` static-index poke through typed stack tokens.
- `**`, `tau`, single-line `#` comments.
- Function-call source notation: `sin(poly)`, `mul(poly, cf)`.
- Assignment-form target: `poly = pop`, `poly = peek`,
  `poly = sin(poly)`.
- Direct native transform names replace user-facing `legacy(...)`.
- Every existing saved program loads, compiles, and renders identically.

Phase 2A:

- Additive typed stack with scalar/vector values.
- Dynamic scalar element reads and writes.
- Vector/scalar broadcast for core binary ops.
- Typed `fill`, typed unary ops, and nested `add(poly, fill(poly_len, p1))`
  style expressions.

Remaining full v2:

- Unified expression grammar across chip lines and chip args
  beyond the Phase 2A function-call lowering.
- Slice assignment and `for_each_index`.
- Vector reductions: `sum`, `mean`, `prod`, `dot`, `dotu`, norms, and
  arg-extrema.
- Vector construction and structure: `zeros`, `ones`, `repeat`, `basis`,
  `geom`, `axis`, `cheb_axis`, `slice`, `concat`, `pad`, `trim`, and `take`.
- Elementwise helpers: `real`, `imag`, `phase`, `arg`, `unit`, `clip_abs`,
  `normalize`, `center`, and `standardize`.
- Masked selection and conditionals: `where` and `select`.
- Deterministic random generators using the pinned `splitmix64` + `PCG32`
  family.

After v1 ships, the running example reads as math. That is the test.
