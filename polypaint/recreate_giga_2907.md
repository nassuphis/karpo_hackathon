# Recreating `giga_2907`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2907/giga_2907.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2907` (or its `test5` data-stem) names.

## 1. Main conclusions

`giga_2907` (run 2025-02-15; `poly_chess4` at snapshot `8d554a2`, the
last commit before the run) is the **curve-blend chessboard** — and
structurally it is giga_2902's bimodal blend with the two
constellations riding DIFFERENT curves:

```text
chain: bkr,bkr -> poly_chess4 -> sort_abs_p,rev -> solve (paramconstruct)
poly:  n=8, mod=2, phi=0.125, iname=ipolygon (iplgn=4, irad=0.2),
       oname=circle (orad=0.5), a=1e-5, andy=0.01, degree=32
run:   roots=3e9, res=50000, alpha=0.01, procs=14, chunk=6696428
view:  (-0.8062112436318505 - 0.796851850659265j) ->
       ( 0.782111533066133  + 0.7914709260387185j)   (subview=center)
```

Verbatim per sample:

```python
t  = np.random.rand()                       # RNG, ignores t1/t2
to = 0.5 * exp(2j*pi*t)                     # outer curve: circle
ti = 0.2 * ipolygon(t + 0.125)              # inner curve: square path
cfi = np.poly(chess32 + ti*(1+1j))          # (X+ti) + 1j(Y+ti)
cfo = np.poly(chess32 + to*(1+1j))
a  = bimodal_skewed(1e-5)                   # second RNG (~uniform at a=1e-5)
cf = cfo*a + cfi*(1-a)
cf = sort_abs(cf)*0.01 + cf                 # zfrm sort_abs_p, andy=0.01
q  = cf[::-1]                               # zfrm rev
```

1. **The dict is full of ghosts.** `bkr,bkr` transforms uniforms that
   poly_chess4 never reads; `i=3124`, `tt`, `speed=2.0`, `off=0`,
   `oplgn`, `irad`-vs-`ispeed` defaults — all vestigial. The two live
   randoms (`np.random.rand()` and bimodal's internal uniform) ignore
   the sweep parameters entirely, so they remap DIRECTLY: `t := t1`,
   `u := t2` — exact, like giga_2888.
2. **The square path** (`ipolygon`, n=4): t*4 picks an edge and a
   fraction; the point interpolates linearly between consecutive
   vertices `exp(2j*pi*k/4)` — numpy's exact doubles, dust included
   (`cos(pi/2) = 6.1e-17`, baked as literals). In the program the four
   vertices sit in a scratch-extended pool vector, edge index / frac /
   next-index ride pokes, and the delta expression gathers with
   **dynamic double indexing** `poly[poly[4]]` — piecewise-linear
   curve evaluation in one expression. New idiom, no new primitive.
3. **The `(1+1j)` diagonal**: `(X+ti) + 1j*(Y+ti) = pts + ti*(1+1j)` —
   commutative-exact through `c_mul` (multiplying by 1+1i is two exact
   multiplies by 1.0).
4. **bimodal + blend are giga_2902's machinery verbatim**: the VM's
   `bimodal(u, a)` is bit-for-bit `bimodal_skewed` (same halving, same
   clip), and `blend(t) = below*(1-t) + top*t` matches
   `cfo*a + cfi*(1-a)` with CFI pushed first. At `a = 1e-5` the blend
   weight is nearly uniform (the exponent is 1.00001).
5. The tail is the 2878 pattern (`sort_abs`, x0.01, `add(pop)`) plus
   the native `rev`. Degree stays 32: both constellation polys are
   monic, the blend of leading 1s is 1, sort_abs_p moves it to
   [0.99, 1.01], and rev's new leading (the old constant) is nonzero
   almost surely. `chunk = floor(3e9/32/14) = 6,696,428` matches.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2907.png` | 50000 x 50000, 8-bit grayscale | 159,908,350 bytes | `a8828a07e6a3309bf02a890854469cceaa2a0362e64f1e2e5d5a189c229be054` |
| `giga_2907_sml.png` | 1000 x 1000, 8-bit grayscale | 520,693 bytes | `074009160b6965e270fa401be4218140b4323a1f20d0429050d525ed96fbf164` |
| `giga_2907_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 520,756 bytes | `7dc41182a19a2da827ec491b3af57239c5aa6bf9724aa8137c708826ea8a70bc` |

Measured polarity: `_sml` mean 38.24 (dark background), `_sml_inv`
216.76; full-size mid-band ~36.7. No full-size `_inv` exists.

Sidecars: `_alpha` = `0.01`, `_paramconstruct` = `bkr,bkr / poly_chess4
/ sort_abs_p,rev / solve`, `_paramcount` = `3000000000`, `_resolution`
= `50000`, `_solver` = `solve`. `data.stem` is `test5`.

## 3. Polypaint Coeff Program

Proven program (119 tokens, stack_max 6, pools [7 vertex+scratch,
33 chess coeffs — the two roots_literal spellings byte-dedup to one
pool]; fingerprint-preserving round trip;
`d69e3ce99e519f1b147ce646a1f474a3b43c65c5`):

```text
poly = vector_literal(<4 square vertices>, 0, 0, 0)
poly[4] = floor(4*(t1+0.125)) - 4*floor(floor(4*(t1+0.125))/4)
poly[5] = 4*(t1+0.125) - floor(4*(t1+0.125))
poly[6] = (floor(4*(t1+0.125))+1) - 4*floor((floor(4*(t1+0.125))+1)/4)
poly = translate_roots(roots_literal(<32 chess pts>), 0.2*((1-poly[5])*poly[poly[4]] + poly[5]*poly[poly[6]])*(1+1i))
poly
poly = translate_roots(roots_literal(<32 chess pts>), (0.5*exp(pi2i*t1))*(1+1i))
poly
poly = blend(bimodal(t2, 0.00001))
poly
poly = sort_abs(poly)
poly = multiply(poly, 0.01)
poly = add(pop, poly)
poly = rev(poly)
emit
```

Native parity vs the verbatim formula
(`tests/test_giga_2907_coeff_program.py`): worst `1.2e-10` over 45 rows
(acceptance 1e-7). The suite also pins the exact vertex doubles (the
dust IS the value), the mask parity (index-sum odd = coordinate-sum
even), the edge wraparound at the path's end, and the pool dedup.

Run settings:

```text
base coefficient function = const
degree                    = 32
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-0.8062112436318505, 0.782111533066133]
                            im in [-0.796851850659265, 0.7914709260387185]
render                    = no rotation, dark background (no invert)
N = 9682  ->  2,999,458,568 roots (3-billion-class run)
N =  790  ->     19,971,200 roots (5K-class validation)
```

Note the tight center view (~±0.8 on a ±4.2 constellation): the image
is the DENSE CORE of the root cloud, at alpha 0.01.

Saved through `/save-coeff-program` as id `giga-2907`, predeploy-gated.
No deploy needed. Scrub knobs: the 0.2/0.5 radii, the 0.125 phase, the
bimodal skew, and the 0.01 admixture are all live.
