# Recreating `giga_2908`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2908/giga_2908.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2908` (or its `test8` data-stem) names.

## 1. Main conclusions

`giga_2908` (run 2025-02-16; `poly_chess4` at snapshot `2e061ed`, which
adds the `tt` selector) is the **spindle chessboard with a rotated
solver** — and its chain composition collapses in a way that makes it
recreatable:

```text
chain: bkr,t1_plus_t2 -> poly_chess4 -> sort_abs_p,rev -> solve_rot45
poly:  n=8, mod=2, phi=0.1, tt=t1pmt2,
       iname=oname=spindle, irad=0.2, orad=5.0, a=0.001, andy=0.01
run:   roots=2e7, res(png)=50000, alpha=0.01, procs=14, chunk=44642
view:  (-0.5261107238140506 - 0.5137381086479078j) ->
       ( 0.5339601411008776 + 0.5463327562670204j)   (subview=center)
```

1. **The composition collapses.** The xfrm chain maps both uniforms to
   the SAME complex value `s = bkr(t1) + bkr(t2)` (`t1_plus_t2` returns
   the sum twice), and then `tt='t1pmt2'` computes `tt1 = t1'+t2' = 2s`
   and `tt2 = t1'-t2' = 0` **exactly**. The inner spindle is therefore
   a CONSTANT (`0.2*spindle(0.1)`, baked as one complex literal); only
   the outer curve moves: `to = 5*spindle(2s)`.
2. **spindle gets a COMPLEX argument** on 3/4 of rows (`im(s) != 0`,
   i.e. any t >= 0.5): its cosines grow like cosh, pushing the pre-rev
   constellation to ~1e5-1e7 — which the `rev` inversion (roots ->
   1/roots) then collapses into a **sub-pixel dot at the origin**
   (~1e-6..1e-9 against a ~2e-5 pixel). This retires the one semantic
   hazard: `np.sign(complex)` CHANGED in numpy 2.0 (`sign(re)` ->
   `z/|z|`); the program uses `z/|z|` (spelled `C/abs(C)`), and the
   choice only moves that invisible dot. On visible rows (both t < 0.5)
   the argument is real and the definitions coincide at exactly +-1
   (IEEE cos/sin never return exact zero there).
3. **The 4/3 power** (`|cos|^(2/1.5)`) spells `exp(1.3333...*log|C|)` —
   the scalar `**` is integer-only. Sub-ulp vs numpy's pow; the
   measured parity (1.3e-11) includes it.
4. **`solve_rot45`** multiplies the SOLVED roots by `e^(-i pi/4)`.
   Algebraically `q(z) = p(z*w^-1)` has exactly those roots, so the
   program bakes the rotation into the coefficients with a final map
   scan: `cf[k] *= exp(i pi/4 (32-k))`. Root-level equivalence is
   pinned in the suite (nearest-match 3.6e-8, eigensolve dust).
5. **bimodal_skewed(0.001)** is the only true RNG left; the giga_2883
   frac-cascade (identical constants) supplies its uniform. Aggregate
   clouds in the saved view: corr 0.69 vs same-ensemble noise floor
   0.58 at 8k rows — indistinguishable.
6. Ghosts as usual: `i=3124`, `speed`, `off`, `iplgn/oplgn` (spindle
   ignores them), `phi` applies only to the CONSTANT inner argument.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2908.png` | 50000 x 50000, 8-bit grayscale | 142,493,285 bytes | `b091b81745428b868c6c7a390ddb24a1fc925ab4adf922368a8e18c0da5b4622` |
| `giga_2908_sml.png` | 1000 x 1000, 8-bit grayscale | 581,145 bytes | `7fc4a62685f779806693ba5db7917b58881ec35920dbf6d9f8d845ade221fed2` |
| `giga_2908_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 581,979 bytes | `1f540c6156ba43e7ef08c769f093efb02174f0e2cef62a3f12162557f9880a80` |

Measured polarity: `_sml` mean 52.87 (dark background), `_sml_inv`
202.13; full-size mid-band ~52.5. (The dict's `view.res` says 5000 but
the actual PNG is 50000^2 — the measured geometry is authoritative.)

Sidecars: `_alpha` = `0.01`, `_paramconstruct` = `bkr,t1_plus_t2 /
poly_chess4 / sort_abs_p,rev / solve_rot45`, `_paramcount` =
`20000000` (a small run), `_resolution` = `5000`, `_solver` =
`solve_rot45`. `data.stem` is `test8`.

## 3. Polypaint Coeff Program

Proven program (154 tokens, stack_max 7, one pool constant of length 33
— the two roots_literal spellings dedup; fingerprint-preserving round
trip; `818ebfb55380b81339e372bd261d080ff90bd3ff`):

```text
poly = fill(3, 0)
poly[0] = <s = bkr(t1)+bkr(t2), one complex poke>
poly[1] = cos(6.283185307179586*(2*poly[0]))
poly[2] = sin(6.283185307179586*(2*poly[0]))
poly[1] = 0.5*(poly[1]/abs(poly[1]))*exp(1.3333333333333333*log(abs(poly[1])))
poly[2] = 0.2*(poly[2]/abs(poly[2]))*exp(1.3333333333333333*log(abs(poly[2])))
poly = translate_roots(roots_literal(<32 chess pts>), (5*(poly[1] + 1i*poly[2]))*(1+1i))
poly
poly = translate_roots(roots_literal(<32 chess pts>), <0.2*spindle(0.1)*(1+1i) literal>)
poly
swap
poly = blend(bimodal(<frac-cascade u>, 0.001))
poly
poly = sort_abs(poly)
poly = multiply(poly, 0.01)
poly = add(pop, poly)
poly = rev(poly)
poly = scan(poly_len, 0, poly[0]*exp(1i*0.7853981633974483*32), poly[k]*exp(1i*0.7853981633974483*(32-k)))
emit
```

The self-read pokes (`poly[1] = f(poly[1])`) stage the spindle
coordinates; `swap` orders the stack so blend's `below*(1-t) + top*t`
lands on `cfi*(1-a) + cfo*a`.

Native parity vs the documented chain
(`tests/test_giga_2908_coeff_program.py`): worst `1.3e-11` over 46 rows
(acceptance 1e-7), including complex-argument rows and the exp/log
power spelling. The suite also pins the composition collapse (constant
inner delta, origin-dot regime), and the rot45 ramp's root-level
equivalence.

Run settings:

```text
base coefficient function = const
degree                    = 32
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-0.5261107238140506, 0.5339601411008776]
                            im in [-0.5137381086479078, 0.5463327562670204]
render                    = no rotation, dark background (no invert)
N =  790  ->     19,971,200 roots (the reference's own scale)
N = 9682  ->  2,999,458,568 roots (3-billion-class upgrade)
```

The reference run was only 2e7 roots — N=790 IS its native density; a
billions-class rerun of this program is an upgrade the original never
had. Saved through `/save-coeff-program` as id `giga-2908`,
predeploy-gated. No deploy needed.
