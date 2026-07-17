# Recreating the `giga_2898` family (2898, 2899, 2900, 2903, 2904)

## 0. Objective and safety boundary

Recreate five sibling artworks from their saved states under
`/Users/nicknassuphis/pyroots/giga_29xx/`. The reference tree is
read-only; no new run may use the `giga_29xx` (or the shared
`giga_2869` data-stem) names.

## 1. Main conclusions

All five are `poly_chess5` like giga_2897, with two twists: **phi=0**
(the two letterform copies displace IDENTICALLY, not in antiphase) and
a **coefficient-space perturbation zfrm** — the reference's `*_p`
family, `f(cf)*andy + cf`, where `andy` is a shatter dial:

```text
run        rloc    deg  rho  andy   zfrm                       view span
giga_2898  rjail2  21   0.1  1e-06  sort_angles_keep_moduli_p  ~24
giga_2899  rjail2  21   0.1  0.01   sort_abs_p                 ~31
giga_2900  rjail2  21   0.1  1e-09  symmetrize_p               ~14
giga_2903  rp3     25   0.1  10.0   sort_abs_p                 ~83
giga_2904  rjail6  8    0.2  0.1    sort_abs_p                 ~21
(all: a=0.7, speed=1, shape=circle, xfrm none, solve=solve,
 roots=2e9, res=50000, samples=100000)
```

Per row (polys/chess.py:279 with phi=0):

```python
t = np.random.rand()                  # unseeded — fossil draw 1
d = rho * exp(2j*pi*t) * (1+1j)       # ONE displacement, both copies
cf = np.poly(T+d)*a + np.poly(S+d)*(1-a),  a = bimodal_skewed(0.7)
cf = f(cf)*andy + cf                  # the *_p perturbation blend
```

1. **The `_p` blend is the artwork's texture dial.** `f(cf)` has the
   same magnitude multiset as `cf` (a permutation for sort_abs, a
   phase-reshuffle for sort_angles_keep_moduli, a palindrome sum for
   symmetrize), so `andy*f(cf)` perturbs the SMALL (leading-end)
   coefficients enormously in relative terms while leaving the large
   ones almost exact: the lattice core survives, the outer roots
   shatter. andy=1e-09 (2900) is a whisper; andy=10 (2903) lets the
   transform dominate and blows the view out to span ~83.
2. **bimodal_skewed(0.7)** has the non-integer exponent
   1/(1-0.7) = 3.333333333333333 — spelled `exp(E*log(.))` in the
   program (no `^` in the grammar; log IS in the deployed grammar).
   The floor-gated branchless form is exact at the u=0 edge
   (log(0) -> -inf -> exp -> 0, the correct limit).
3. **All three zfrms are native registry transforms**:
   `sort_abs` (fn 10) matches `cf[argsort(|cf|)]` including stable
   tie order on probes; `sort_angle_keep_mod` (fn 9) IS
   sort_angles_keep_moduli ("cf[k] keeps its own modulus but takes
   the k-th smallest angle"); symmetrize = `rev` + add.
4. **RNG remap** as always: per-run frac-cascade uniform pairs of
   (t1, t2) replace the two unseeded draws (distinct seeds per run so
   the five sweeps decorrelate).
5. **No solver in-program**; coefficient scales (1.2e6 for the
   degree-8 rjail6 up to 1.6e28 for degree-25 rp3) all sit inside f32
   transport. Layouts parse to exactly the recorded degrees:
   rjail2 21+21 (5x22 grid), rp3 25+25 (7x33), rjail6 8+8 (7x11).

## 2. Trap log (this wave)

- Nothing new fired; the giga_2897 doctrine held: statement-position
  scans for every tos[k] reference, coordinate literals instead of
  hand-transcribed ASCII art, multiply/add(poly, pop) for the parked
  operands.
- `1e-06`-style scientific literals compile fine in both scalar exprs
  and multiply args (probed before use).
- The oracle mirrors the VM weight (exp/log grouping, floor gates) in
  pure python; numpy's `**` differs only at the usual 1-ulp level.

## 3. Polypaint Coeff Programs

One generator emits all five documents
(`scripts/gen_giga_2898_family_coeff_programs.py`,
48-50 tokens each, stack_max 6; suite
`tests/test_giga_2898_family_coeff_programs.py`, predeploy-gated,
plain no-LAPACK build). Per program:

```text
branch(S, 1-a) -> push; branch(T, a); add(poly, pop)   # the mix
push mix; f(poly) [sort_abs | sort_angle_keep_mod | rev+add]
multiply(poly, andy); add(poly, pop); emit             # f(cf)*andy + cf
```

Verification (measured, pins with orders-of-magnitude headroom):
coefficient relative parity <= 2.7e-15 per run (f64 end-to-end), root
multiset vs oracle median <= 7.1e-11, worst tail 1.7e-8.

Uploaded 2026-07-17: ids `giga-2898`, `giga-2899`, `giga-2900`,
`giga-2903`, `giga-2904`.
