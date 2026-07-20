# Recreating `giga_87`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_87/giga_87.png` from its saved
definition. The reference tree is read-only; no new run may reuse the
`giga_*` names except as the recreation id.

Artifact: `giga_87.coeff-program.json` (upload id `giga-87`).

## 1. Which era, which source

`giga_87.png` is dated **2025-01-11** — the python era (R era ends at
giga_26, Dec 2024). So `poly.py` / `polys/giga.py` `poly_giga_87`
(0-indexed) is authoritative. The `poly.csv` R version is a **buggy
1-indexed transliteration**: its loop `for (i in 4:51) cf[i] = i*t1 +
(51-i)*t2 + ...` uses the 1-based `i` in the arithmetic, so it computes a
different coefficient per index than the python `for i in range(3,51):
cf[i] = i*t1 + (51-i)*t2 + ...`. The python def rendered the png.

## 2. Pipeline

Same as the giga_39..43 wave (`unit_circle / rev / safe`); giga_87 is not
in `polybook.sh` (a curated book subset) but is the same era and the same
slot-fill style, applying `sin/cos/log` DIRECTLY to the unit-circle points
(so the monomial->exp folding does not apply — `t1` in the program is the
raw sweep param and the source builds `U1 = e^(2 pi i t1)` itself).

- **`unit_circle`** (xfrm): t1, t2 = e^(2 pi i t).
- **`rev`** (zfrm): `np.flip(cf)` before solving.
- **`safe`** (solve): reference knife — sum|cf| outside (1e-10, 1e10), or
  any NaN/inf, returns zeros.

## 3. The polynomial (dense k-formula + sequential overrides)

```text
cf[0] = t1 + t2
cf[1] = 1 + t1*t2 + log(|t1 + t2| + 1)
cf[2] = t1 + t2 + log(|1 - t1*t2| + 1)
for i in 3..50: cf[i] = i*t1 + (51-i)*t2 + log(|t1 - i*t2| + 1)
cf[10] = cf[0]  + cf[9]  - sin(t1)
cf[20] = cf[30] + cf[40] - cos(t2)   # LOOP cf[30], cf[40]
cf[30] = cf[20] + cf[40] + sin(t1)   # NEW cf[20], LOOP cf[40]
cf[40] = cf[30] + cf[20] - cos(t2)   # NEW cf[30], NEW cf[20]
cf[50] = cf[40] + cf[20] + sin(t2)   # NEW cf[40], NEW cf[20]
```

The k-formula is a DIRECT map (no `cf[i-1]` dependency), so it compiles to
one `scan(51, 0, F, F)` whose init and step are the SAME expression
`F = k*U1 + (51-k)*U2 + log(|U1 - k*U2| + 1)` (the scan's `prev` is unused;
every element k -> F(k)). `scan` fills all 51 slots incl. 0/1/2; the
statements then override 0/1/2 and the five deps in the numpy def's exact
order, so the sequential dependency is preserved (`poly[i]` reads resolve
in statement order). 144 tokens, one deduplicated scalar expression.

## 4. `safe` never fires

`log(|.| + 1) >= 0` is always finite, and sum|cf| across 500 random draws
stays **1341 .. 3310** — far inside (1e-10, 1e10). So no gate is ported
(the giga_259 / giga_39..43 precedent).

## 5. Verification (measured)

Machine-floor parity at 5 probes: coefficient rel <= 5e-18, root multiset
vs `np.roots` <= 6e-15. One generator
(`scripts/gen_giga_87_coeff_program.py`), one predeploy-gated suite
(`tests/test_giga_87_coeff_program.py`).
