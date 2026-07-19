# Recreating `giga_259`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_259/giga_259.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_259` names.

## 1. Main conclusions

`giga_259` is a fully DETERMINISTIC pipeline — an exact port like
giga_cf10p450, no RNG remap:

```text
poly:  poly_373 (degree 34), xfrm 'uc,coeff2', zfrm rev, solve 'safe'
run:   roots=3e9, res=50000, samples=100000, procs=14, alpha=1e-4
view:  (-12.52-12.80j) -> (14.75+14.48j), rotate 270
```

```python
z1, z2 = e^(2 pi i t1), e^(2 pi i t2)          # uc
v1, v2 = z1 + z2, z1 * z2                      # coeff2: the symmetric pair
for j in 1..35:
    mag = log(|v1|+j) sin(j pi/7) + cos(j pi/11) Re(v2)
    ang = angle(v1) + angle(v2) j + sin(j pi/13)
    cf[j-1] = (mag + Im(v1) cos(j pi/5)) e^(i ang) + conj(v2) sin(j pi/17)
emit rev(cf)
```

1. **coeff2 feeds the SYMMETRIC pair** of the two circle points:
   v1 = z1+z2 (magnitude 0..2, the interference envelope) and
   v2 = z1 z2 = e^(2 pi i (t1+t2)) (a pure unit-circle phase) — so
   conj(v2) and Re(v2) inline exactly from (t1+t2) without any
   header, and only v1-derived scalars need parking.
2. **poly_373's rec/imc linspace block is dead code** (computed, never
   used) — omitted.
3. **solve='safe' is the reference's own knife**: rows with sum|cf|
   outside (1e-10, 1e10) return zeros (dropped). Measured across the
   sweep: sum|cf| stays in [57, 83] — the knife NEVER fires here, so
   no gate is ported and 'safe' is behaviorally identical to 'solve'.
4. Coefficients are O(1..7): no transport rescale, no conditioning
   drama — the cleanest migration parity so far.

## 2. Trap log (this wave)

Nothing fired. The slot expression exceeds the 64-token scalar cap in
one piece, so the row builds as MAG-scan x PHASE-scan against the
parked header P = [angle(v1), angle(v2), |v1|, Re(v2), Im(v1)]
(statement-position tos scans, header rebuilt per scan — pokes are
chip-per-op but two builds fit easily) plus a no-tos arg-scan tail
for conj(v2) sin(j pi/17). Mix via multiply(poly, pop) — the verified
safe form.

## 3. Polypaint Coeff Program

`giga_259.coeff-program.json` (132 tokens, stack_max 5; generator
`scripts/gen_giga_259_coeff_program.py`; suite
`tests/test_giga_259_coeff_program.py`, predeploy-gated, plain
no-LAPACK build).

Verification (measured): coefficient relative parity 1.5e-15, root
multiset vs oracle median 1.7e-15 / max 1.9e-14 — machine precision.

Uploaded 2026-07-19: id `giga-259`.
