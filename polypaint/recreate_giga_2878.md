# Recreating `giga_2878`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2878/giga_2878.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2878` (or its stale `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2878` is giga_2877's sibling — same snapshot (`8e28adb`), same
glyph and offset — with the root-finder swapped for the gentlest of the
`_p` admixture family:

```text
chain: none -> poly_letter -> sort_abs_p -> polyroot    (paramconstruct)
poly:  ascii=178, degree=32, andy=0.075, ro=-0.5, io=-0.5
run:   roots=2e9, res=50000, alpha=0.001, procs=14, chunk=4464285
view:  (-4.537170896552147 - 4.657238838413984j) ->
       ( 4.6774404099295745 + 4.557372468067737j)
```

1. **`sort_abs_p`** (verbatim at 8e28adb, `andy = 0.075`):

   ```python
   q = sort_abs(cf) * andy + cf        # sort_abs = cf[argsort(|cf|)]
   ```

   A 7.5% admixture of the magnitude-sorted coefficients onto the
   original. Unlike giga_2877 there are no pairing dice: the sorted
   vector is a deterministic |.|-ascending permutation of cf itself, so
   the recreation is verbatim end to end.
2. Constellation as giga_2877: roots = `b178 pixels + delta - 0.5-0.5i`
   with `delta = complex(0.05+0.9*t1, 0.05+0.9*t2)` — the ro/io offset
   centers each pixel cell on an integer (the solid-square checkerboard).
3. **The degree holds at 32 on every row**: `q[0] = 1 + 0.075*s0` where
   s0 is the minimum-|.| coefficient; since the monic 1 is a candidate,
   `|s0| <= 1`, so `q[0]` lives in `[0.925, 1.075]` and never vanishes.
   The sorted-leading regime does break (s0 is not the monic 1 on
   21/5000 seeded rows — the centered cell keeps a root near the origin,
   shrinking the constant term below 1), but the formula is verbatim in
   both pipelines either way.
4. Only the fully symmetric `delta = 0` row `t = (0.5, 0.5)` — which
   also carries an EXACT origin root (`cf[32] = 0`) — has exact |cf|
   ties, where sort order is legitimately free; its cf multiset even
   contains exact duplicate values by symmetry.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2878.png` | 50000 x 50000, 8-bit grayscale | 132,774,707 bytes | `bd72ed216751626e0bd9aa8e4914258e24703e096c9ae6884a116391ae69dc67` |
| `giga_2878_sml.png` | 1000 x 1000, 8-bit grayscale | 551,062 bytes | `358586a26a6dda679074c7bf7a84566488419b3eaea8ad2851baea8457d34bac` |
| `giga_2878_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 551,527 bytes | `c2e984937c8d97299a5eca5b98d1600ead14e99c9d3fef945be1fda88b13ca23` |

Measured polarity: `_sml` mean 68.29 (dark background), `_sml_inv`
186.71; full-size mid-band sample ~82.9. No full-size `_inv` exists.

Sidecars: `_alpha` = `0.001` (10x the family's usual), `_paramconstruct`
= `none / poly_letter / sort_abs_p / polyroot`, `_paramcount` =
`2000000000`, `_resolution` = `50000`, `_solver` = `polyroot`.
`data.stem` is the stale `giga_2869`.

## 3. Polypaint Coeff Program

Proven program (32 tokens, stack_max 6, one pool constant of length 33;
fingerprint-preserving round trip;
`08e1712f4b420560401604c33b87a78217b147de`):

```text
poly = translate_roots(roots_ascii_literal(178), ((0.05+0.9*t1)+(0-0.5)) + 1i*((0.05+0.9*t2)+(0-0.5)))
poly
poly = sort_abs(poly)
poly = multiply(poly, 0.075)
poly = add(pop, poly)
emit
```

`add(pop, poly)` = cf + 0.075*sorted, commutative-exact against the
reference's `sorted*andy + cf`.

Native parity vs the verbatim formula
(`tests/test_giga_2878_coeff_program.py`): worst `1.3e-9` over 44
tie-free rows (acceptance 1e-7); probes asserted tie-free; the
`(0.5, 0.5)` tie row pinned structurally — recovered `(q - cf)/0.075`
against the VM's own cf probe must be an |.|-ascending permutation of
cf's exact multiset (lexsorted comparison, tolerances scaled by |cf|
since the recovery amplifies ulp(|cf| ~ 1e6)/0.075; nearest-neighbor
injectivity would falsely fail on the row's exact duplicate values).

Run settings:

```text
base coefficient function = const
degree                    = 32      (q[0] in [0.925, 1.075], never zero)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-4.537170896552147, 4.6774404099295745]
                            im in [-4.657238838413984, 4.557372468067737]
render                    = no rotation, dark background (no invert)
N = 7905  ->  1,999,648,800 roots (2-billion-class run)
N =  790  ->     19,971,200 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2878`, predeploy-gated.
The `andy = 0.075` admixture strength is scrubbable — 0 is the plain
checkerboard, larger values push toward the sorted polynomial's cloud.
