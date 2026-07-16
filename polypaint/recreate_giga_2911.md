# Recreating `giga_2911`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2911/giga_2911.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2911` (or its `test16` data-stem) names.

## 1. Main conclusions

`giga_2911` (run 2025-03-06; `p11b2_v2`/`toline_q` at snapshot
`a9393c5`) is the p11-family sawtooth pushed through a **Cayley
transform** into a sliver of the upper half-plane — and, uniquely in the
recreated set, it contains **no randomness at all**: every stage is a
pure function of the sweep parameters. No remap, no PRNG, no ensemble
argument.

```text
chain: unit_circle -> p11b2_v2 -> cumsum,rev,toline_q -> solve
poly:  andy=1.0, degree=10
run:   roots=2e9, res=50000, alpha=0.001, procs=14, chunk=14285714
view:  ((-0.25+0j), (0.25+0.25j))    (subview llur-0.25:0.0:0.25:0.25)
```

Verbatim per sample:

```python
t' = e^{2j pi t}                      # unit_circle, both axes
denom = t1' + t2' + 3                 # |denom| >= 1 ALWAYS (|t1'+t2'| <= 2)
                                      # -> the normalize branch is DEAD
u  = 77 * linspace(0,1,11)^15 / denom
m  = int(4583*|t1'+t2'|) % 11 + 1     # integer regime, 1..11 (all hit)
cf = ((arange(11)+1) % m + 1) * exp(1j pi u)
cf = cumsum(cf); cf = cf[::-1]        # zfrm cumsum, rev
rts = np.roots(cf)                    # toline_q, andy=1.0:
q   = np.poly(1j*(1+rts)/(1-rts))     # Cayley disk -> upper half-plane
```

1. **The p11 signature**: the integer modulus m jumps discretely as
   `|t1'+t2'|` sweeps (the sawtooth-regime aesthetic from the fable
   waves), while `77*v^15` is a fixed spike profile (only the last few
   of the 11 slots are appreciably nonzero) phase-wound by `1/denom`.
2. **The Cayley stage** maps the unit disk to the upper half-plane —
   the tiny llur view `[-0.25, 0.25] x [0, 0.25]` is a magnifying glass
   on the line's foot. Mid-chain root-trip as giga_2910: `roots_cm(lo,
   exact)`, Mobius algebra in vector ops (`1i*(1+r)` first, matching
   Python's left-associative `1j * num/den`), then **expand_roots**
   (np.poly bitwise, solver emission order). `andy=1.0` keeps only the
   transformed polynomial — the blend is exact, not approximated.
3. The 77*v^15 constants are numpy's own doubles (linspace step dust
   included), baked at repr precision.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2911.png` | 50000 x 50000, 8-bit grayscale | 88,233,641 bytes | `aa5c5757b28d5d6736a2fad44c10d1f4b64d9da14b7f6c6a53f5337076fc55db` |
| `giga_2911_sml.png` | 1000 x 1000, 8-bit grayscale | 394,732 bytes | `af133e1b0f44541a8de20e07276417896aff82be3f1fc220712cdcfcf4df2c88` |
| `giga_2911_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 395,838 bytes | `721f3451472480c9d86aaab3d88676e8b2941781843550e50a2b341897c1288e` |

Measured polarity: `_sml` mean 33.73 (dark background), `_sml_inv`
221.27; full-size mid-band ~18.5.

Sidecars: `_alpha` = `0.001`, `_paramconstruct` = `unit_circle /
p11b2_v2 / cumsum,rev,toline_q / solve`, `_paramcount` = `2000000000`,
`_resolution` = `50000`, `_solver` = `solve`. `data.stem` is `test16`.

## 3. Polypaint Coeff Program

Proven program (83 tokens, stack_max 6, one pool constant [11 = the
77*v^15 profile]; fingerprint-preserving round trip;
`b52766d43afb2d096f05a6014d3476ad7863b2db`):

```text
poly = fill(2, 0)
poly[0] = exp(pi2i*t1) + exp(pi2i*t2)
poly[1] = floor(4583*abs(poly[0])) - 11*floor(floor(4583*abs(poly[0]))/11) + 1
scan(11, 0, (0+1) - poly[1]*floor((0+1)/poly[1]) + 1, (k+1) - poly[1]*floor((k+1)/poly[1]) + 1)
vector_literal(<77*v^15 doubles>)
poly = divide(pop, poly[0] + 3)
poly = multiply(poly, 3.141592653589793i)
poly = exp(poly)
poly = multiply(pop, poly)
poly = scan(poly_len, 0, poly[0], prev + poly[k])
poly = rev(poly)
poly = roots_cm(poly, lo, exact)
poly = scan(10, 0, poly[0], poly[k])
poly
poly = multiply(poly, 0-1)
poly = add(poly, 1)
poly
swap
poly = add(pop, 1)
poly = multiply(poly, 1i)
poly = divide(poly, pop)
poly = expand_roots(poly)
emit
```

Native parity vs the verbatim chain
(`tests/test_giga_2911_coeff_program.py`): worst `1.9e-7` over 45 rows
(acceptance 1e-5) — the residual is roots_cm's float32 root cast
amplified by the Cayley pole near r = 1. The suite also pins the dead
normalize branch (|denom| >= 1 on 3000 rows), the full 1..11 integer
regime, and determinism.

Run settings:

```text
base coefficient function = const
degree                    = 10
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-0.25, 0.25], im in [0, 0.25]
render                    = no rotation, dark background (no invert)
N = 14142 ->  1,999,961,640 roots (the reference's 2e9 scale)
N =  1414 ->     19,993,960 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2911` (requires the
expand_roots deploy first), predeploy-gated.
