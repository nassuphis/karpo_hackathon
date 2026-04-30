# Coeff Program Text Commands

This is the current manual for Coeff Program **Text** mode. It is based on the
implemented parser in `lambda/coeff_program_source.py` and compiler/runtime in
`lambda/coeff_program_chain.py` and `lambda/sweep_cli.c`.

## Mental Model

Each evaluation starts with:

- `cf`: read-only coefficient vector from the selected coefficient function.
- `poly`: mutable output vector, initially a copy of `cf`.
- `p1`, `p2`: final parameter values from the Param Program or Param Chain.
- `t1`, `t2`: read-only original parameter-grid values.
- `poly_len`: current length of `poly`.
- stack: typed stack that can hold vectors and scalars. Final stack depth must
  be zero.

The program's job is to leave the desired coefficient vector in `poly`.

## Syntax Basics

- Statements are separated by newlines or `;`.
- `#` starts a comment when it appears outside parentheses and brackets.
- Parentheses and brackets may span lines.
- Function names and register names are case-insensitive after parsing.
- `legacy(...)` is not valid in text mode. Use direct transform names instead,
  or use Chain mode for old legacy chains.
- Standalone `pop` and `peek` are invalid. Use `drop`, or write `poly = pop`
  / `poly = peek` explicitly.

Example:

```text
# Reverse the current poly vector.
poly = rev(poly)

# Add a p1-filled vector to poly.
poly = add(poly, fill(poly_len, p1))
```

## Sources And Targets

Vector sources:

- `cf`: read the immutable coefficient-function vector.
- `poly`: read the current output vector.
- `pop`: pop the top vector from the stack and use it.
- `peek`: read the top vector without popping it.

Vector targets:

- `poly = ...`: write the result into `poly`.
- Bare producer call: push the result onto the stack.
- `emit`: pop the top vector and write it to `poly`.

Examples:

```text
cf              # push cf
poly            # push poly
poly = pop      # pop vector into poly
poly = peek     # copy top vector into poly without popping
emit            # pop vector into poly
```

## Scalar Expressions

Scalar expressions are accepted in command arguments, index expressions, and
scalar/vector mixed operations.

Values:

- Numeric literals: `1`, `-2.5`, `1e-3`, `2j`, `1+2j`.
- Constants: `pi`, `pi2`, `tau`, `pi2i`, `tau_i`, `i`, `j`.
- Registers: `p1`, `p2`, `t1`, `t2`, `poly_len`.
- Indexed values: `cf[0]`, `poly[10]`, `tos[3]`.
- Short indexed values: `cf0`, `poly10`, `tos3`.
- Dynamic indices: `poly[poly_len-1]`, `cf[real(p1)]`, `tos[3+2]`.

Operators and functions:

- Binary operators: `+`, `-`, `*`, `/`.
- Power: `**` with an integer literal exponent only, magnitude <= 32.
- Unary signs: `+x`, `-x`.
- Functions: `conj(x)`, `neg(x)`, `real(x)`, `imag(x)`, `abs(x)`, `mod(x)`,
  `log(x)`.

Notes:

- No implicit multiplication. Write `2*p1`, not `2p1`.
- `abs(x)` and `mod(x)` are aliases and return a real-valued scalar.
- Dynamic indices must evaluate to real integers and must be in range.
- Scalar expression bytecode is bounded; very large expressions should be split
  into vector operations or native transforms.

Examples:

```text
poly[0] = 100j*p1
poly[poly_len-1] = p1 + p2
push_const(poly_len, log(abs(p1+p2)+1)*1j)
poly = add(poly, fill(poly_len, cf[0] + poly[poly_len-1]))
```

## Stack Commands

| Command | Effect |
| --- | --- |
| `cf` | Push a copy of `cf`. |
| `poly` | Push a copy of `poly`. |
| `emit` | Pop top vector and write it to `poly`. |
| `dup` / `duplicate` | Duplicate top stack item. Works for vector or scalar stack slots. |
| `swap` | Swap the top two stack items. |
| `drop` | Drop the top stack item. This is the standalone pop operation. |
| `flush` | Empty the stack. Does not modify `cf` or `poly`. |
| `macro(name)` | Expand a saved coeff program macro at compile time. |

Examples:

```text
cf
rev()
emit

poly
dup
sin()
swap
cos()
add()
emit
```

## Constructors

Constructors push vectors unless used on the right-hand side of `poly = ...`.

### `fill`, `const`, `push_const`

Aliases. Create a vector filled with one scalar value.

Forms:

```text
fill(value)            # length defaults to poly_len
fill(length, value)
const(value)
const(length, value)
push_const(value)
push_const(length, value)
```

Examples:

```text
push_const(poly_len, p1)
poly = fill(poly_len, 0)
poly = add(poly, fill(poly_len, p2*1j))
```

### `range`, `arange`, `push_range`

Aliases. Create a real vector using stop-exclusive range semantics.

Forms:

```text
range(length)              # [0, 1, ..., length-1]
range(start, stop)         # step defaults to 1
range(start, stop, step)
arange(...)
push_range(...)
```

Examples:

```text
push_range(poly_len)       # 0, 1, ..., poly_len-1
range(1, poly_len+1)       # 1, 2, ..., poly_len
range(5, 0, -1)            # 5, 4, 3, 2, 1
```

### `linspace`, `push_linspace`

Aliases. Create a real vector with inclusive endpoints.

Forms:

```text
linspace(length)               # length values from 0 to length
linspace(start, stop, count)   # count values from start to stop
push_linspace(...)
```

Examples:

```text
push_linspace(poly_len)
poly = linspace(-1, 1, poly_len)
```

### `littlewood`

Create a vector of current `poly_len`. Each element is chosen deterministically
from `value1` or `value2` with 50/50 odds for the current row.

Forms:

```text
littlewood(value1, value2)
littlewood(value1, value2, andy)
poly = littlewood(value1, value2)
poly = littlewood(value1, value2, andy)
```

Examples:

```text
poly = littlewood(0, 1)
poly = littlewood(10, -10j, 0.25)
```

`andy` is a real blend factor against the input `poly`.

## Assignment And Indexing

### Whole-vector assignment

```text
poly = cf
poly = pop
poly = peek
poly = rev(poly)
poly = add(poly, fill(poly_len, p1))
```

Bare source assignment uses a direct vector copy. Function assignment writes the
function result to `poly`.

### Scalar writes

```text
poly[index] = value
poke_poly(index, value)
poke_tos(index, value)
```

Examples:

```text
poly[0] = 100j*p1
poly[poly_len-1] = p2
poke_poly(10, p1*p2*real(poly6) + imag(poly18)*p1**3)
poke_tos(0, 1+0j)
```

`poke_poly` writes into `poly`. `poke_tos` writes into the top stack vector.

## Affine Helpers

These are text-mode helpers implemented with typed scalar/vector operations.

Forms:

```text
scale(multiplier)              # source defaults to pop
scale(source, multiplier)

shift(offset)                  # source defaults to pop
shift(source, offset)

linear(multiplier, offset)     # source defaults to pop
linear(source, multiplier, offset)
```

Examples:

```text
push_range(poly_len)
linear(1, 1)
emit

poly = linear(poly, 1j*p1, p2)
poly = scale(poly, 0.5)
poly = shift(poly, -poly0)
```

There is also a low-level explicit chip form:

```text
affine(target, source, multiplier, offset)
```

Example:

```text
affine(push, pop, 2, 1)
```

Prefer `scale`, `shift`, or `linear` in normal text programs.

## Vector Binary Operations

Binary operations work on vector/vector, vector/scalar, scalar/vector, or
scalar/scalar values when used through typed expressions. Vector/vector inputs
must have matching lengths. Vector/scalar combinations broadcast the scalar.

Commands:

- `add` / `+`
- `sub`, `subtract` / `-`
- `mul`, `multiply` / `*`
- `div`, `divide` / `/`
- `pow`, `power`

Command forms:

```text
add()                  # pop right, pop left, push left+right
add(src1, src2)        # src1/src2 are cf, poly, pop, or peek
poly = add(src1, src2)
```

Examples:

```text
poly = add(poly, cf)
poly = subtract(poly, fill(poly_len, p1))
poly = multiply(poly, p1*p2)
poly = divide(poly, fill(poly_len, 2))
poly = power(poly, fill(poly_len, 2))

push_range(poly_len)
push_const(poly_len, p1)
power()
emit
```

Important: in text mode `pow(...)` and `power(...)` mean vector/scalar binary
power. They shadow the old native coefficient-transform names `pow` and `power`.

## Vector Unary Operations

Unary operations work on vectors or scalars. Bare command calls use `pop`.

Commands:

- `angle`
- `abs`, `mod`
- `neg`
- `conj`
- `sqrt`
- `log`
- `real`
- `imag`

Forms:

```text
abs()              # pop, apply, push
abs(source)        # source is poly, pop, or peek
poly = abs(poly)
```

Examples:

```text
poly = abs(poly)
poly = log(abs(poly))
poly = conj(poly)
poly = real(poly)
```

For `cf`, use typed expression style:

```text
poly = abs(cf)
```

This is accepted and lowered through the typed VM.

## Reordering And Keyed Operations

### `roll` and `rolr`

Roll left or right by an integer amount.

Forms:

```text
roll(n)             # source defaults to pop; n is an integer literal
roll(source, n)
rolr(n)
rolr(source, n)
poly = roll(poly, n)
poly = rolr(poly, n)
```

Examples:

```text
poly = roll(poly, 1)
poly = rolr(poly, 3)
```

### `argsort`

Sort the first vector by the magnitude of the second vector.

Forms:

```text
argsort(src_values, src_keys)
poly = argsort(src_values, src_keys)
```

Sources are `poly`, `pop`, or `peek` in the direct form.

Example:

```text
poly
abs(peek)
poly = argsort(poly, pop)
drop
```

## Blend

Blend consumes two vectors and a scalar `t`, then pushes:

```text
below*(1-t) + top*t
```

Form:

```text
blend(t)
```

Example:

```text
cf
poly = deriv(poly)
poly
blend(0.25)
emit
```

`blend` always writes to the stack. Use `emit` or `poly = pop` afterward if you
want the blended vector in `poly`.

## Native Coefficient Transforms

Most legacy coefficient transforms are called directly by name in text mode.

General forms:

```text
name()                   # source defaults to pop, target is push
name(source)             # target is push
name(source, args...)    # target is push
poly = name(source)
poly = name(source, args...)
```

If the transform supports `andy`, a trailing `andy` argument is accepted.

Examples:

```text
poly = rev(poly)
poly = cumsum(poly)
poly = sort_abs(poly)
poly = exp(poly, 1j+1, p2, 0.5)
poly = round(poly, 2+0j, 0.25)
poly = roots(poly, 8, hi)
poly = roots_cm(poly, lo)
```

Currently callable native transforms:

| Name | Args | Notes |
| --- | --- | --- |
| `rev` | none | Reverse coefficient order. |
| `normalize` | none | Normalize coefficients. |
| `deriv` | none | Derivative; may change length. |
| `safe` | none | Legacy safe transform. |
| `negate_odd` | none | Negate odd-indexed coefficients. |
| `max2one` | none | Legacy max-to-one normalization. |
| `sort_mod_keep_angle` | none | Sort by magnitude, keep angle. |
| `sort_angle_keep_mod` | none | Sort by angle, keep magnitude. |
| `sort_abs` | none | Sort by absolute value. |
| `cumsum` | none | Cumulative sum. |
| `cummax` | none | Cumulative max-style legacy transform. |
| `sort_cumsum` | none | Sort then cumulative sum. |
| `swirler` | none | Legacy swirler transform. |
| `exp` | `a`, `b`, optional `andy` | Applies legacy affine exp form; `a` and `b` may be complex expressions. |
| `cos` | none | Elementwise complex cosine. |
| `sin` | none | Elementwise complex sine. |
| `tan` | none | Elementwise complex tangent. |
| `cosh` | none | Elementwise complex hyperbolic cosine. |
| `sinh` | none | Elementwise complex hyperbolic sine. |
| `tanh` | none | Elementwise complex hyperbolic tangent. |
| `round` | `a`, optional `andy` | Legacy round affine form; `a` may be complex. |
| `invpower` | `k`, optional `andy` | Inverse power transform; may change length. |
| `roots_cm` | `pad`, optional `andy` | `pad` is `hi` or `lo`; may change length. |
| `roots` | `k`, `pad`, optional `andy` | `pad` is `hi` or `lo`; may change length. |

Names shadowed by first-class text syntax:

- `linear` is a text-mode affine helper, not the old native `linear` bridge.
- `conj` is a typed unary operation.
- `pow` and `power` are vector/scalar binary power operations.

## Complete Examples

Reverse the coefficient vector:

```text
poly = rev(poly)
```

Create a Littlewood vector and exponentiate it:

```text
poly = littlewood(10, -10j)
poly = exp(poly, 1j+1, 1)
```

Build `sin(p1**(i/2)) * cos(p2**(i/3))` over indices `i = 1..poly_len`:

```text
range(1, poly_len+1)
multiply(pop, 0.5)
push_const(poly_len, p1)
power(pop, pop)
sin()

range(1, poly_len+1)
multiply(pop, 0.3333333333333333)
push_const(poly_len, p2)
power(pop, pop)
cos()

multiply()
emit
```

Patch selected coefficients using scalar reads:

```text
poly[10] = p1*p2*real(poly6) + imag(poly18)*p1**3
poly[21] = p2*poly10 + real(poly34)*p1**3
poly[32] = poly21 - real(poly16)*p1**2
```

Use stack branching:

```text
poly
deriv(peek)
swap
rev(pop)
add()
emit
```

## Limits

- Source text size: 64 KiB.
- Compiled tokens after macro/source lowering: 256.
- Stack depth: 64.
- Max vector length: 256.
- Max scalar-expression tokens: 32.
- Macro expansion depth: 8.

If execution fails, common causes are:

- Final stack depth is not zero. Add `emit`, `drop`, or `flush`.
- Vector lengths do not match for vector/vector binary operations.
- A dynamic index is non-real, non-integer, or out of range.
- A command produced a vector longer than 256.
- A real-only argument got a complex expression without `real(...)` or
  `imag(...)`.
