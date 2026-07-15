# Recreating `giga_2871`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2871/giga_2871.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2871` (or `giga_2869`) stem.

## 1. Main conclusions

`giga_2871` is the third member of the `poly_letter` family (same snapshot
`500685f`, 27 Jan 2025; same runner, stale `giga_2869` stem, 50K
resolution, no rotation, no inversion, literal thumbnail naming; the
full-size `_inv` is a 12-Feb publish artifact). It uses the same
checkerboard glyph as `giga_2872` (`ascii = 178` — dictionary key `b178`,
CP437 glyph 177 `▒` per the transcription quirks, 32 lit pixels) with the
SIMPLE coefficient transform:

1. **`zfrm = sort_abs`** (`zfrm.py:346`): `cf[argsort(|cf|)]` — the
   coefficients reordered by ascending magnitude, nothing differenced.
2. **Degree stays 32**: the monic leading coefficient (`|cf[0]| = 1`) is
   the magnitude minimum for every sampled row (5,000/5,000 measured), so
   sorting moves the `1` to the FRONT — the polynomial remains monic
   degree 32 and nothing is stripped. `chunk = floor(5e8 / 32 / 14) =
   1,116,071` matches the saved job exactly.
3. **`roots = 500,000,000`**: actual work
   `1,116,071 x 14 x 32 = 499,999,808` roots.
4. **A small view** — `(-6.730791289102695 - 6.662619306362197j)` to
   `(12.010349547436144 + 12.078521530176642j)` — because sort_abs merely
   permutes the coefficients: roots stay near the constellation scale
   instead of the `1e14`-difference explosions of the `andy1` siblings.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2871.png` | 50000 x 50000, 8-bit grayscale | 89,196,975 bytes | `a5e1626eedf6983dbc8047c44185da19942bc64f79eada09054110ec66151cdb` |
| `giga_2871_inv.png` | 50000 x 50000, 8-bit grayscale | 89,331,344 bytes | `0672dbe5c535b0e3ec1c432f40a61811188fb918d9300d2f8985501bcf72eabe` |
| `giga_2871_sml.png` | 1000 x 1000, 8-bit grayscale | 351,769 bytes | `926950780b60cc0c6ce933dcf61696d0e62da2ea8f9a79a55f73c4d6c047f105` |
| `giga_2871_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 353,657 bytes | `ce198c08464ab3f6612a5dfdfa1a431b067e61b0b0cfb084eda8ac69a83265aa` |

Measured polarity (literal naming): full/`_sml` mean 19.92/19.93 (dark
background); `_inv`/`_sml_inv` mean 235.08/235.07 (inverse).

Sidecars: `_alpha` = `0.0001`, `_paramconstruct` = `none / poly_letter /
sort_abs / polyroot`, `_paramcount` = `500000000`, `_resolution` = `50000`,
`_solver` = `polyroot`.

## 3. Polypaint Coeff Program

Proven program (18 tokens, stack depth 6, one pool constant of length 33;
native parity vs the verbatim formula: worst `1.2e-11` over four probe
rows; fingerprint-preserving round trip; `poly_len`-free because sort_abs
is length-agnostic — changing the glyph code is still the only edit):

```text
poly = translate_roots(roots_ascii_literal(178), (0.05+0.9*t1) + 1i*(0.05+0.9*t2))
poly = sort_abs(poly)
emit
```

The VM's `sort_abs` sorts by ascending magnitude with index-stable ties,
matching NumPy's stable argsort; this family's magnitudes are generically
distinct.

Run settings:

```text
base coefficient function = const
degree                    = 32          (monic; the sorted leading term is
                                         the constellation's 1)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-6.730791289102695, 12.010349547436144]
                            im in [-6.662619306362197, 12.078521530176642]
render                    = no rotation, dark background (no invert)
N = 3952  ->    499,790,528 roots (half-billion-class run)
N =  790  ->     19,971,200 roots (5K-class validation)
```

Implementation mirrors the siblings: generator
`scripts/gen_giga_2871_coeff_program.py` (`--check`), parity suite
`tests/test_giga_2871_coeff_program.py` (freshness, shape pins, round
trip, four-point verbatim-oracle parity plus the sorted-leading==1 regime
assertion), saved through `/save-coeff-program` as id `giga-2871`,
predeploy-gated.
