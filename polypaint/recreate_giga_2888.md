# Recreating `giga_2888`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2888/giga_2888.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2888` (or its stale `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2888` (run 2025-01-31; `poly_chess1` committed at `1230acc`,
unchanged through `fb854a4`) is the **circling chessboard** — the
simplest chain since the sort_abs family, with two twists:

```text
chain: none -> poly_chess1 -> none -> solve            (paramconstruct)
poly:  degree=49
run:   roots=2e9, res=50000, alpha=0.0, procs=14, chunk=2915451
view:  (-7.245731288739578 - 7.322805016063989j) ->
       ( 7.145782644441431 + 7.0687089171170205j)
```

Verbatim per sample (`solve` = np.roots in a try/except — polyroot by
another name):

```python
N = 7
t = np.random.uniform(-2pi, 2pi)                  # RNG, ignores t1/t2
u = np.random.uniform(-2pi, 2pi)                  # RNG, ignores t1/t2
curve = grid_7x7 + (sin t + 1j cos t) - (4+4j)    # 49 roots ride a circle
cf1 = np.poly(curve + 0.1*cos(u))                 # + real jitter
cf3 = cf1 + 0.0001*(curve ++ [10j])               # roots blended INTO coeffs
cf  = cf3 + 1e-23*(cf3^2 + cf3 + 1)               # elementwise quadratic
```

1. **The cleanest random remap in the family.** Both uniforms ignore
   the sweep parameters entirely, so they map DIRECTLY onto them in
   numpy's own `low + (high-low)*r` form: `t = -2pi + 4pi*t1`,
   `u = -2pi + 4pi*t2`. Exact in distribution AND in arithmetic shape —
   no PRNG, no counter split, no ensemble argument needed. Rows with
   equal parameters are the same polynomial in both pipelines.
2. **The root-blend** `cf1 + 0.0001*(curve ++ [10j])` mixes the root
   POSITIONS into the coefficient vector (compare roots_p, which mixes
   solved roots; here the roots are known, so no solver). The append
   rides the giga_2877 guarded-map idiom: `k - 49*floor(k/49)` reads
   `tos[0]` harmlessly at k=49 while `floor(k/49)` selects the `10j`.
3. **The 1e-23 quadratic** looks like a no-op but is not: mid-slot
   coefficients of a degree-49 grid polynomial reach ~1e13-1e27, and
   `1e-23*cf3^2` overtakes `cf3` wherever `|cf3| > ~3.2e11` — it is an
   elementwise quadratic warp of the polynomial's largest coefficients
   (four vector ops: square, +peek, +1, scale, add).
4. Degree stays 49: the reversed... no reversal here — the leading is
   `1 + 0.0001*curve[0] + O(1e-23) ~ 1`. `chunk = floor(2e9/49/14) =
   2,915,451` matches the saved job.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2888.png` | 50000 x 50000, 8-bit grayscale | 160,082,761 bytes | `1a35aef3f266c122a295ff7ac48a6fc6c46533e6f81378851db48ccc1bc12d59` |
| `giga_2888_sml.png` | 1000 x 1000, 8-bit grayscale | 503,269 bytes | `69512fa538aa5556a9d6d557e0bdae01ae540de3ad8f14d913dfad065a418539` |
| `giga_2888_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 505,004 bytes | `a993aff60573266fa51cd8b77f74367542cc84866ad83629a33a5c7a00739fb4` |

Measured polarity: `_sml` mean 52.08 (dark background), `_sml_inv`
202.92; full-size mid-band ~88.7. No full-size `_inv` exists.

Sidecars: `_alpha` = `0.0`, `_paramconstruct` = `none / poly_chess1 /
none / solve`, `_paramcount` = `2000000000`, `_resolution` = `50000`,
`_solver` = `solve`. `data.stem` is the stale `giga_2869`.

## 3. Polypaint Coeff Program

Proven program (69 tokens, stack_max 7, pools [49 grid points, 50 monic
coeffs]; fingerprint-preserving round trip;
`4f65980bb73c571880a0f921ad3c7e6d402f60e3`):

```text
poly = vector_literal(<49 grid points>)
poly = add(poly, sin(T) + 1i*cos(T))
poly
poly = translate_roots(roots_literal(<49 grid points>), sin(T) + 0.1*cos(U) + 1i*cos(T))
poly = scan(50, 0, poly[0] + 0.0001*tos[0],
            poly[k] + 0.0001*(tos[k-49*floor(k/49)]*(1-floor(k/49)) + 10i*floor(k/49)))
drop
poly
poly = multiply(poly, poly)
poly = add(poly, peek)
poly = add(poly, 1)
poly = multiply(poly, 0.00000000000000000000001)
poly = add(pop, poly)
emit
```

with `T = (-6.283185307179586 + 12.566370614359172*t1)` and
`U = (...*t2)` — the exact `low + (high-low)*r` doubles numpy's uniform
uses. Scrub either constant for slower/faster circling.

Native parity vs the verbatim formula
(`tests/test_giga_2888_coeff_program.py`): worst `2.1e-9` over 45 rows
(acceptance 1e-7) — degree-49 translate dust only. No sorts, no solver
in the program, no tie rows. The suite also pins the tile/repeat point
order and the quadratic term's magnitude regime.

Run settings:

```text
base coefficient function = const
degree                    = 49
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-7.245731288739578, 7.145782644441431]
                            im in [-7.322805016063989, 7.0687089171170205]
render                    = no rotation, dark background (no invert)
N = 6388  ->  1,999,525,456 roots (2-billion-class run)
N =  638  ->     19,947,556 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2888`, predeploy-gated.
No deploy needed.
