# Recreating `giga_2870`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2870/giga_2870.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2870` (or `giga_2869`) stem.

## 1. Main conclusions

`giga_2870` is the fourth `poly_letter` family member and the direct
sibling of `giga_2871` — identical chain (`poly_letter -> sort_abs ->
polyroot`), identical run shape (5e8 roots, 50K, no rotation, no
inversion, alpha 1e-4, stale `giga_2869` stem, snapshot `500685f`) — with
one substitution:

1. **`ascii = 52`**: dictionary key `b52` = sheet cell 51 = the digit
   `'3'` (ASCII 51), horizontally mirrored per the transcription quirks —
   **23 lit pixels**:

   ```text
   ...####.
   ..##..##
   ..##....
   ...###..
   ..##....
   ..##..##
   ...####.
   ........
   ```

   23 roots -> 24 coefficients; the monic leading `1` is the magnitude
   minimum on every RANDOM row (5,000/5,000), so `sort_abs` keeps the
   polynomial monic **degree 23** (saved), and
   `chunk = floor(5e8 / 23 / 14) = 1,552,795` matches the saved job.
   One measured exception unique to this glyph: it has a pixel at
   `(-0.5, -0.5)`, so the shift `(t1, t2) = (0.5, 0.5)` puts one root
   EXACTLY at the origin — the constant term is 0 and sorts FIRST,
   degrading that single row to degree 22. Both pipelines handle it
   identically (the leading zero strips); historical random sampling hit
   it with probability zero, and a deterministic even-N grid hits it on
   exactly one row of N^2 — visually nothing.
2. Actual work: `1,552,795 x 14 x 23 = 499,999,990` roots.
3. Saved view: `(-6.193136619066075 - 6.810696113351204j)` to
   `(12.3252732570267 + 11.70771376274157j)` — the small sort_abs-scale
   window, as with `giga_2871`.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2870.png` | 50000 x 50000, 8-bit grayscale | 133,083,489 bytes | `879cc070a860aab8556cd71edac429f0841ff02cb34b62b831f80b868ad5457f` |
| `giga_2870_sml.png` | 1000 x 1000, 8-bit grayscale | 423,774 bytes | `e370c0bbdd615cf98422c27305a670b405cb4b1558f66f2d71665206f709ae68` |
| `giga_2870_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 425,959 bytes | `a3592c60293131f826775e972433b244f3415bb18f241f7cea0c697165582520` |

Measured polarity (literal naming): full 21.87 (dark background), `_sml`
21.90, `_sml_inv` 233.10. No full-size `_inv` exists for this stem.

Sidecars: `_alpha` = `0.0001`, `_paramconstruct` = `none / poly_letter /
sort_abs / polyroot`, `_paramcount` = `500000000`, `_resolution` =
`50000`, `_solver` = `polyroot`.

## 3. Polypaint Coeff Program

Proven program (18 tokens, one pool constant of length 24; native parity
vs the verbatim formula: worst `1.2e-13` over four probe rows — the
smallest of the family, degree-23 conditioning; fingerprint-preserving
round trip; the generic sorted-leading == 1 regime tested on random rows,
with the (0.5, 0.5) origin-root degeneracy pinned explicitly):

```text
poly = translate_roots(roots_ascii_literal(52), (0.05+0.9*t1) + 1i*(0.05+0.9*t2))
poly = sort_abs(poly)
emit
```

Run settings:

```text
base coefficient function = const
degree                    = 23          (monic; sorted leading is the 1)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-6.193136619066075, 12.3252732570267]
                            im in [-6.810696113351204, 11.70771376274157]
render                    = no rotation, dark background (no invert)
N = 4662  ->    499,861,212 roots (half-billion-class run)
N =  932  ->     19,977,752 roots (5K-class validation)
```

Implementation mirrors `giga_2871`: generator
`scripts/gen_giga_2870_coeff_program.py` (`--check`), parity suite
`tests/test_giga_2870_coeff_program.py`, saved through
`/save-coeff-program` as id `giga-2870`, predeploy-gated.
