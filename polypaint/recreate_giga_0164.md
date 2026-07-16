# Recreating `giga_0164`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_0164/giga_0164.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_0164` stem.

## 1. Main conclusions

`giga_0164` is from the EARLY options-format era — and that era's
polyfun.txt **inlines the complete verbatim source** (xfrm, poly, zfrm
classes), making it self-documenting: no repository archaeology, no
snapshot pinning, no working-tree ambiguity.

```text
chain: z01 -> poly_445 -> p8,invp3,roots -> safe
run:   roots=2e9, res=50000, alpha=0.0, procs=14, chunk=4201680,
       degree=34, ROTATE=270
view:  re in [-1.7790089751511071, 1.786625284455147]
       im in [-1.7790089751511071, 1.786625284455147]
```

Verbatim per sample:

```python
t1' = t1 + 1j*t2;  t2' = t2 + 1j*t1                  # z01 (mirrored pair)
# poly_445: 35 slots, k = (5j+2)%12+1
cf[j] = log(|t1'|+j)*|sin(j pi/10)|
        * (t1*sin(j pi/k) + t2*cos(j pi/(k+1))
           + 1j*(t2*cos(j pi/k) - t1*sin(j pi/(k+1))))
        * (cos(A_j) + 1j*sin(A_j))
A_j   = angle(t1')*cos(j pi/8) + angle(t2')*sin(j pi/9)
cf    = (cf^8+...+cf+1) * arange(1..35)              # p8
s     = cf^3+cf^2+cf+1; cf = 1/s where |s|>1 else 1  # invp3
cf    = np.roots(cf)                                 # roots feedback
```

1. **The seven per-j trig tables are pool constants** (numpy's exact
   doubles); the three row scalars (`|t1'|`, `angle(t1')`,
   `angle(t2')`) are scalar expressions. The whole poly_445 assembly is
   vector algebra with the established stack discipline.
2. **`p8` IS the native `power_series` transform**: its C loop already
   scales by `(index+1)` — the transform was evidently built for this
   zfrm family. `invp3` is the native `invpower(3)` (same strict
   `|s| > 1` condition, else-1). Association dust only (ascending C
   accumulation vs the descending np spelling).
3. **Source trap recorded**: `power(poly, 8)` lowers to the TYPED
   elementwise `z^8` — a completely different artwork (first prototype
   was O(1) wrong on every row). The series transform's source spelling
   is `power_series(poly, 8)`; the suite pins the lowered chain.
4. **The roots feedback runs on every row** — its `sum|cf| < 1e-10`
   passthrough guard is DEAD on this chain (0/5000; the invp3 output
   always keeps a 0.2..1.0-magnitude slot). Leading slots reach
   ~3.6e-25, so `roots_cm`'s **exact-strip mode is load-bearing** once
   again (the legacy relative strip would delete them and their giant
   roots), and the zgeev emission order matters positionally (the roots
   vector IS the emitted polynomial) — covered by the established
   LAPACK-lineage result. The hi pad's leading zero strips in the
   solver, reproducing the reference's degree-33 solve of the 34-root
   vector.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_0164.png` | 50000 x 50000, 8-bit grayscale | 121,287,363 bytes | `7c0474ecf6cc3024ed458ce46b71c9248c68be7c8d2fd3830acd501a6f3ce352` |
| `giga_0164_sml.png` | 1000 x 1000, 8-bit grayscale | 445,705 bytes | `ba33c34cb871d4dfe0725ea347a376e6cc006cdd31194f54788a44ee0d048956` |
| `giga_0164_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 448,104 bytes | `6af30a0676749e6f88b38cc4bc30dc7b4b570c46250239999fb89fe5f21e9a39` |

Measured polarity: `_sml` mean 34.41 (dark background), `_sml_inv`
220.59; full-size mid-band ~73.3.

Sidecars: `_alpha` = `0.0`, `_paramconstruct` = `z01 / poly_445 /
p8,invp3,roots / safe`, `_paramcount` = `2000000000`, `_resolution` =
`50000`, `_solver` = `safe`.

## 3. Polypaint Coeff Program

Proven program (93 tokens, stack_max 6, seven pool constants of length
35; fingerprint-preserving round trip;
`0ed0c5e0f062b8a895331efc917a3663fde604b7`):

```text
poly = vector_literal(<C2>);  poly = multiply(poly, t2);  poly
poly = vector_literal(<S1>);  poly = multiply(poly, t1);  poly = add(poly, pop)
poly
poly = vector_literal(<S2>);  poly = multiply(poly, t1);  poly
poly = vector_literal(<C1>);  poly = multiply(poly, t2)
poly = subtract(poly, pop);  poly = multiply(poly, 1i);  poly = add(poly, pop)
poly
poly = vector_literal(<S9>);  poly = multiply(poly, angle(t2 + 1i*t1));  poly
poly = vector_literal(<C8>);  poly = multiply(poly, angle(t1 + 1i*t2))
poly = add(poly, pop)
poly;  poly = cos(poly);  poly;  swap
poly = sin(pop);  poly = multiply(poly, 1i);  poly = add(poly, pop)
poly
poly = arange(1, 36);  poly = add(poly, abs(t1 + 1i*t2));  poly = log(poly)
poly
poly = vector_literal(<S10>);  poly = multiply(poly, pop)
swap
poly = multiply(poly, pop)
poly = multiply(poly, pop)
poly = power_series(poly, 8)
poly = invpower(poly, 3)
poly = roots_cm(poly, hi, exact)
emit
```

(One statement per line in the actual document; grouped here for
reading. Full table values in `scripts/gen_giga_0164_coeff_program.py`.)

Native parity vs the run's own inlined source
(`tests/test_giga_0164_coeff_program.py`): worst `4.1e-8` over 45 rows
(acceptance 1e-5) — power-sum association dust through invpower and the
eigensolve, plus the f32 root cast. The suite also pins the dead guard,
the 1e-25 leading-magnitude regime, and the power_series-not-power
lowering.

Run settings:

```text
base coefficient function = const
degree                    = 34 emitted (leading zero -> degree-33 solve)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-1.7790089751511071, 1.786625284455147]
                            im in [-1.7790089751511071, 1.786625284455147]
render                    = ROTATE 270, dark background (no invert)
N = 7784  ->  1,999,395,648 roots (the reference's 2e9 scale)
N =  778  ->     19,975,572 roots (5K-class validation)
```

Note the render rotation: the reference PNG was rotated 270 degrees —
use the render pipeline's rotate control to match.

Saved through `/save-coeff-program` as id `giga-0164`, predeploy-gated.
No deploy needed.
