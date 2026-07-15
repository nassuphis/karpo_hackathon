# Recreating `giga_2873`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2873/giga_2873.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2873` (or `giga_2869`) stem.

## 1. Main conclusions

`giga_2873` is the direct sibling of `giga_2872` — same snapshot commit
(`500685f`, files dated 27 Jan 2025), same `poly_letter -> andy1 ->
polyroot` chain, same 50K resolution with no rotation and no inversion, the
same stale `giga_2869` stem, and the same andy1 mechanics (leading output
element identically zero; int64-cumprod collapse regime measured
5,000/5,000). Everything in `recreate_giga_2872.md` applies with three
parameter substitutions:

1. **`ascii = 186`**: dictionary key `b186` = sheet cell 185 (CP437 `╣`,
   mirrored per the transcriber quirk) — the double-pipe junction glyph with
   **34 lit pixels**:

   ```text
   .##.##..
   .##.##..
   .##.####
   .##.....
   .##.####
   .##.##..
   .##.##..
   .##.##..
   ```

   34 roots -> 35 coefficients -> `andy1` zeroes the leading term ->
   **degree 33** (saved), and `chunk = floor(3e9 / 33 / 14) = 6,493,506`
   matches the saved job exactly.
2. **`roots = 3,000,000,000`** (vs 2e9): actual work
   `6,493,506 x 14 x 33 = 2,999,999,772` roots.
3. **A different saved view**:
   `(-272.3842227400068 - 300.28083799720156j)` to
   `(245.64648285640746 + 217.7498675992127j)`.

Coefficient scale: `max sum|cf| = 3.9e15` over 5,000 sampled rows — within
binary32 range; the float32 transport boundary perturbs fine structure as
with the sibling.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2873.png` | 50000 x 50000, 8-bit grayscale | 177,657,048 bytes | `deae10443cf2097d55636eabab88b90d18c5d13f41c1f80972c31a0b3d4bd79c` |
| `giga_2873_sml.png` | 1000 x 1000, 8-bit grayscale | 451,288 bytes | `d5fd809039874f0e71c3eb31127ebffbc71528cb4b191398c86f83ef2caa519b` |
| `giga_2873_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 453,254 bytes | `49bb1f681308fbdb0e3113faf16b1083cd9d0caea75b91fa5045554e7b07fa72` |

Measured polarity (literal naming): full 31.34 (dark background), `_sml`
31.32 (same polarity), `_sml_inv` 223.68 (inverse). No full-size `_inv`
publish artifact exists for this stem.

Sidecars: `_alpha` = `0.0001`, `_paramconstruct` = `none / poly_letter /
andy1 / polyroot`, `_paramcount` = `3000000000`, `_resolution` = `50000`,
`_solver` = `polyroot`.

## 3. Polypaint Coeff Program

Proven program (40 tokens, stack depth 6, one pool constant of length 35;
native parity vs the verbatim `andy1` oracle: worst `4.0e-11` over four
probe rows — degree-34 binomial dust at the `3.9e15` coefficient scale;
fingerprint-preserving round trip):

```text
poly = translate_roots(roots_ascii_literal(186), (0.05+0.9*t1) + 1i*(0.05+0.9*t2))
poly
poly = arange(0, poly_len)
poly = argsort(poly, peek)
poly
poly = scan(poly_len, 0, tos[0], prev + tos[k])
poly = rem(poly, poly_len)
drop
poly = scan(poly_len, 0, tos[poly[k]], tos[poly[k]])
drop
poly = multiply(poly, 0-1)
poly = add(poly, 1)
emit
```

This is the `giga_2872` program with the glyph code and the vector length
(35 = lit pixels + 1) substituted — the parameterization
`roots_ascii_literal` was built for.

Run settings:

```text
base coefficient function = const
degree                    = 34          (35 coefficients; solvers strip the
                                         zero leading term -> 33 roots/row)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-272.3842227400068, 245.64648285640746]
                            im in [-300.28083799720156, 217.7498675992127]
render                    = no rotation, dark background (no invert)
N = 9534  ->  2,999,436,948 roots (three-billion-class run)
N =  778  ->     19,974,372 roots (5K-class validation)
```

Implementation mirrors the siblings: generator
`scripts/gen_giga_2873_coeff_program.py` (`--check` gated), parity suite
`tests/test_giga_2873_coeff_program.py` (freshness, shape pins, round trip,
four-point verbatim-oracle parity with the collapse-regime assertion),
saved through `/save-coeff-program` as id `giga-2873`, predeploy-gated.
