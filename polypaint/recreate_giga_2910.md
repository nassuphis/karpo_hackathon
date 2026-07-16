# Recreating `giga_2910`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2910/giga_2910.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2910` (or its `test15` data-stem) names.

## 1. Main conclusions

`giga_2910` (run 2025-03-03; `kabala1`/`tohalf` at snapshot `8264c9c`)
is the **kabbalistic walk**: the word `isminiadami` becomes a gematria
vector and each sample advances a GLOBAL unit-modulus state — the third
cross-sample-state chain in the series (after 2880's counter and the
general RNG remaps), and the deepest:

```text
chain: div -> kabala1 -> tohalf -> solve                 (paramconstruct)
poly:  word=isminiadami, theta=0.25, div=1.0, i=9999, degree=10
run:   roots=1e9, res=50000, alpha=0.0, procs=14, chunk=7142857
view:  (-9.531520503922954 - 15.318684524194792j) ->
       ( 9.736653276906667 + 3.9494892566348274j)
```

Verbatim per sample (`kv = [9,100,40,9,50,9,1,4,1,40,9]` from the
gematria table; `w = flip(kv)`; the `div` xfrm divides by 1.0 —
vestigial; `i=9999` freezes the lazy init, so the run rode state
inherited at fork):

```python
cf <- (cf + kv*t1 - 1j*w*t2) / |cf + kv*t1 - 1j*w*t2|    # elementwise
q  = tohalf(cf):
     rts = np.roots(cf)
     q   = np.poly((rts - e^{2j pi 0.25}) / (e^{-2j pi 0.25} - rts))
```

1. **The image is the walk's occupation measure.** Each component of
   the state is a unit-modulus phase driven by `kv_j*t1 - 1j*w_j*t2`
   and renormalized every step. The drive magnitudes (gematria values
   up to 100, min pairwise max 4) dwarf the unit state, so the chain
   forgets its initialization within a few steps — the occupation
   measure IS the stationary distribution, and the unknowable fork-time
   initial state is irrelevant.
2. **Per-row burn-in samples that distribution.** The program runs
   K=23 steps per row: a complex frac-cascade PRNG supplies 22 drive
   pairs and the row's own `(t1, t2)` is the FINAL step — preserving
   the reference's correlation between a sample's drive and its emitted
   polynomial. All 11 component chains run back to back in ONE 253-long
   scan (11 x 23, under the 256-element vector cap) with floor-mask
   resets at component boundaries; the drives ride a precomputed
   Z-vector and a [cascade(23), kv(11)] concat so each scan expression
   stays under the 256-char cap. Measured: burn-in clouds vs the true
   sequential walk at the sampling noise floor (corr 0.824 vs floor
   0.817 at 12k rows); init-forgetting pinned in the suite (opposite
   init converges to the same state through the same drives).
3. **`tohalf` is a mid-chain root-trip**: `roots_cm(lo, exact)` (the
   2880 mode — same zgeev order as the reference's np.roots), a Mobius
   map `(r - i)/(-i - r)` in vector ops (the `e^{+-i pi/2}` constants
   are numpy's exact doubles, dust included), and **`expand_roots`** —
   the new primitive this wave adds: runtime roots-to-coefficients
   expansion, np.poly's convolution BITWISE (`(-r)*a == -(r*a)` and
   `x + (-y) == x - y` exactly), applied in the solver's emission order
   so even the expansion dust matches the reference's np.poly sequence.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2910.png` | 50000 x 50000, 8-bit grayscale | 46,955,687 bytes | `4f91e6eec91a603d5d703ee320231b612a32f7ce30416e382049f386b2921d7d` |
| `giga_2910_sml.png` | 1000 x 1000, 8-bit grayscale | 96,716 bytes | `dede77d4432b299f4efe50d53a9e0820cf3deb6dfe9b45a219274112fa60154b` |
| `giga_2910_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 99,019 bytes | `fe205cd9c664cc8b20b29699b57c39a4f2c019e2981d3cc46b44080a2d9e9c5a` |

Measured polarity: `_sml` mean 5.11 (dark background — a sparse, mostly
black image), `_sml_inv` 249.89. (The full PNG's center-band sample
reads ~254.9: the mid-band crosses a saturated bright region.)

Sidecars: `_alpha` = `0.0`, `_paramconstruct` = `div / kabala1 / tohalf
/ solve`, `_paramcount` = `1000000000`, `_resolution` = `50000`,
`_solver` = `solve`. `data.stem` is `test15`.

## 3. Polypaint Coeff Program

Proven program (47 tokens, stack_max 4, one pool constant [11 = kv];
fingerprint-preserving round trip;
`2cfc74c0694eb15c5cda5668a8bccbeed46f3a00`):

```text
poly = scan(23, 0, <cascade seed>, <cascade step>)     # PRNG drive pairs
poly[22] = t1 + 1i*t2                                  # final step = the row
poly
poly = vector_literal(9, 100, 40, 9, 50, 9, 1, 4, 1, 40, 9)
poly = scan(34, 0, tos[0], <concat [T, kv]>)
poly
swap
drop
poly = scan(253, 0, <Z first>, <Z step>)               # drives per (j, s)
drop
poly
poly = scan(253, 0, (1 + tos[0])/abs(1 + tos[0]), <chain step>)
drop
poly = scan(11, 0, poly[22], poly[23*k+22])            # component finals
poly = roots_cm(poly, lo, exact)
poly = scan(10, 0, poly[0], poly[k])                   # drop the pad slot
poly
poly = multiply(poly, 0-1)
poly = add(poly, <e^{-i pi/2} literal>)                # den = a2 - r
poly
swap
poly = subtract(pop, <e^{+i pi/2} literal>)            # num = r - a1
poly = divide(poly, pop)
poly = expand_roots(poly)
emit
```

(Full expression texts in `scripts/gen_giga_2910_coeff_program.py`.)

Native parity vs the documented chain
(`tests/test_giga_2910_coeff_program.py`): worst `6.7e-7` over 45 rows
(acceptance 1e-5) — the residual is roots_cm's float32 root cast
through the Mobius map. The suite also pins expand_roots' bitwise
np.poly equality, the torus invariant (all |state| == 1), and
init-forgetting.

Run settings:

```text
base coefficient function = const
degree                    = 10
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-9.531520503922954, 9.736653276906667]
                            im in [-15.318684524194792, 3.9494892566348274]
render                    = no rotation, dark background (no invert)
N = 10000 ->  1,000,000,000 roots (the reference's scale)
N =  1414 ->     19,993,960 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2910` (requires the
expand_roots deploy first), predeploy-gated.
