# Recreating `giga_p11b3a`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_p11b3a/giga_p11b3a.png` from
its saved state. The reference tree is read-only; no new run may use the
`giga_p11b3a` stem.

## 1. Main conclusions

`giga_p11b3a` (run 2025-03-14; `p11b3`/`uc_old` at snapshot `01627b4`)
is "balls with holes", per the source's own comment — a p11-family
phase polynomial with two parameter-picked slots knocked out, cooked
three times through the unit-circle map, then a roots feedback:

```text
chain: unit_circle -> p11b3 -> uc_old,uc_old,uc_old,roots -> safe
poly:  degree=9 (n=6 and i=11110 are ghosts — p11b3 reads neither)
run:   roots=2e9, res=50000, alpha=0.0, procs=14, chunk=15873015
view:  ((-2.75-2.75j), (2.75+2.75j))    ROTATE 90
```

Verbatim per sample (fully deterministic — no RNG, no remap):

```python
t' = e^{2j pi t}                      # unit_circle, both axes
t  = t1' + t2';  a = |t|/2            # a in [0, 1]
m  = int(a*251) % 37
v  = (arange(11)+1)/(t+4)
v[int(7a*11)%11]   = (p1+1)/(t + 2|t| + 1 + m)
v[int(619a*11)%11] = (p2+1)/(t + 2|t| + 1 + m//2)
cf = exp(1j pi v)
cf = exp(2j pi * cf/sum|cf|)          # uc_old, x3 (guard sum<1e-15 DEAD)
cf = np.roots(cf)                     # roots feedback
```

1. **Poke indexes must be literal** in the source language, so the
   reference's dynamic-position assignments become MASKED-UPDATE scans
   with the exact integer-equality mask `floor((101-(k-p)^2)/101)`
   (equal to [k == p] for distances up to 10). The scalars
   [a, val1, val2, p1, p2] ride a stack-parked 5-vector so every
   expression stays under the 256-char cap, and the two updates run in
   the reference's order — when the picked slots collide (rows exist:
   pinned in the suite), the second write wins, exactly like numpy's
   sequential assignment.
2. **uc_old's normalization** is the cumsum-of-abs idiom: push cf, take
   elementwise |cf|, cumsum in place, then `divide(pop, poly[10])` —
   the scalar chip reads the summed register before the op pops the
   stashed cf. Its `sum < 1e-15` passthrough guard is DEAD on this
   chain (asserted per row in the oracle; magnitudes stay O(1)).
3. **The roots feedback** is `roots_cm(hi, exact)`; the leading
   pad-zero strips in the solver, reproducing the reference's degree-9
   solve of the 10-root vector. zgeev order matters positionally and
   matches (the established LAPACK-lineage result).
4. At **245 tokens** this is the closest program yet to the 256-token
   cap — the three unrolled uc_old blocks cost 18 statements.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_p11b3a.png` | 50000 x 50000, 8-bit grayscale | 35,760,703 bytes | `86452572581d971ef48291c67870fe541d547849889efbfd1dbf04ecc995337e` |
| `giga_p11b3a_sml.png` | 1000 x 1000, 8-bit grayscale | 156,310 bytes | `6f797ece5ffc778cd752dc5e635acbbf6fcfffcdfc1626cfa6f303a1719d88b5` |
| `giga_p11b3a_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 158,670 bytes | `1b7ca967c3ffcc3705fcfbc3c8b5f7867343ac8080eafc63a5f73df140719413` |

Measured polarity: `_sml` mean 13.79 (dark background, sparse),
`_sml_inv` 241.21; full-size mid-band ~21.6.

Sidecars: `_alpha` = `0.0`, `_paramconstruct` = `unit_circle / p11b3 /
uc_old,uc_old,uc_old,roots / safe`, `_paramcount` = `2000000000`,
`_resolution` = `50000`, `_solver` = `safe`. `data.stem` is the run's
own name (no stale stem, unlike the giga_28xx family).

## 3. Polypaint Coeff Program

Proven program (245 tokens, stack_max 7, zero pool constants;
fingerprint-preserving round trip;
`3bd0f0a6754ffa3ff22c086f69d12eee521c7dfc`):

```text
poly = fill(5, 0)
poly[0] = abs(exp(pi2i*t1) + exp(pi2i*t2))/2
poly[3] = floor(7*poly[0]*11) - 11*floor(floor(7*poly[0]*11)/11)
poly[4] = floor(619*poly[0]*11) - 11*floor(floor(619*poly[0]*11)/11)
poly[1] = (poly[3]+1)/(T + abs(T)*2 + 1 + M)
poly[2] = (poly[4]+1)/(T + abs(T)*2 + 1 + floor(M/2))
poly
poly = arange(1, 12)
poly = divide(poly, T + 4)
poly = scan(11, 0, <masked update 1>)
poly = scan(11, 0, <masked update 2>)
drop
poly = multiply(poly, 3.141592653589793i)
poly = exp(poly)
<uc_old block> x3:
  poly / poly = abs(poly) / cumsum / divide(pop, poly[10]) /
  multiply 2 pi i / exp
poly = roots_cm(poly, hi, exact)
emit
```

with `T = (exp(pi2i*t1) + exp(pi2i*t2))` and `M = floor(poly[0]*251) -
37*floor(floor(poly[0]*251)/37)` (full texts in
`scripts/gen_giga_p11b3a_coeff_program.py`).

Native parity vs the verbatim chain
(`tests/test_giga_p11b3a_coeff_program.py`): worst `5.6e-8` over 45
rows (acceptance 1e-5) — the f32 root cast. The suite also pins the
equality mask, the collision order, and the dead guard.

Run settings:

```text
base coefficient function = const
degree                    = 10 emitted (leading zero -> degree-9 solve)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-2.75, 2.75], im in [-2.75, 2.75]
render                    = ROTATE 90, dark background (no invert)
N = 14907 ->  1,999,988,721 roots (the reference's 2e9 scale)
N =  1490 ->     19,980,900 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-p11b3a`,
predeploy-gated. No deploy needed.
