# Recreating `giga_39` .. `giga_43`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_{39..43}/giga_N.png` from
their saved state. The reference tree is read-only; no new run may use
the `giga_*` names.

Uploaded 2026-07-20, ids `giga-39` .. `giga-43`, none overwriting.

## 1. Source and pipeline

Five python-era static slot-fill polynomials, identical in `poly.py`
and `polys/giga.py`. The driver (`polybook.sh`) is:

```text
-x unit_circle   -z rev   -s safe
```

- **`unit_circle`** (xfrm.py): t1 = e^(2 pi i t1), t2 = e^(2 pi i t2) —
  both axes are unit-circle points.
- **`rev`** (zfrm.py): `np.flip(cf)` — the coefficient vector is
  reversed before solving.
- **`safe`** (solve.py): the reference knife — sum|cf| outside
  (1e-10, 1e10), or any NaN/inf coefficient, returns zeros.

## 2. Key difference from the giga_1..30 T1 wave

The earlier T1 gigas used only monomials `t1^a t2^b`, which fold to
`exp(2 pi i (a t1 + b t2))` on unit-circle points. These five apply
`np.sin`, `np.cos`, `np.exp`, and powers DIRECTLY to the unit-circle
points, so that folding does not apply. Each slot is written with the
actual complex arguments:

```text
U1 = exp(6.283185307179586i*t1)     # e^(2 pi i t1), the uc point
U2 = exp(6.283185307179586i*t2)
```

so `np.sin(t1)` -> `sin(U1)`, `t1**2` -> `U1**2`
(= e^(2 pi i * 2 t1)), `np.exp(1j*(t1+t2))` -> `exp(1i*(U1+U2))`.

## 3. `safe` never fires

Measured across 500 random draws each, sum|cf| stays well inside the
knife's window and no coefficient is NaN/inf:

| giga | sum\|cf\| range |
|---|---|
| 39 | 277.6 .. 561.4 |
| 40 | 268.1 .. 993.1 |
| 41 | 199.5 .. 1382.1 |
| 42 | 44.1 .. 1019.9 |
| 43 | 242.5 .. 948.7 |

So no gate is ported (the giga_259 precedent). Coefficients are
O(1..1400): no transport rescale, no conditioning drama.

## 4. Layout

numpy-descending direct: the program builds `poly[k] = cf[k]` in the
def's own index order, then `poly = rev(poly)` mirrors the zfrm, then
`emit`. Degrees: giga_39 49, giga_40 34, giga_41 59, giga_42 49,
giga_43 39.

## 5. Verification (measured)

Coefficient relative parity < 1e-12 at 5 probes each; root multiset vs
`np.roots` oracle < 1e-7. One generator
(`scripts/gen_giga_39_43_coeff_programs.py`), one predeploy-gated suite
(`tests/test_giga_39_43_coeff_programs.py`).
