# Recreating `giga_cf10p450` — and why the UI chain drew nothing

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_cf10p450/giga_cf10p450.png`
from its saved state, and explain the user's blank render. The
reference tree is read-only; no new run may use its names.

## 1. Why the UI chain produced no image

The user's setup — `legacy(unit_circle, both, both)`,
`legacy(coeff10, both, both)`, function `poly_450`, coeff program
`poly = rev(poly)` — is **semantically correct**: param registry fn 2
(`unit_circle`) and fn 46 (`coeff10`) are verbatim the reference's
xfrm chain, poly_450 is the right base, rev is the right zfrm.

It renders nothing because of scale, not semantics. poly_450 ends with
a cumulative ladder `cf[r-1] += cumprod(M+1..M+r)/(r+1)` (r to 35,
M = |v1|+|v2|), which reaches **6e38 on the luckiest row and ~3.5e41
at the median** (heavy tail to 1e80+ near the coeff10 poles, where
z1^4 = -z2 makes v1 blow up). The reference pipeline is f64 end-to-end
(max 1.8e308) — fine. This pipeline transports coefficients as **f32
(max 3.4e38)**: measured on 200 sampled rows, **100% overflow**, every
row transports as inf, gets skipped, and the image is empty. The same
knife that punches escape sets out of the camera programs here eats
the whole sweep.

Fix for the UI chain, no new program needed: roots are scale-invariant,
so append a rescale to the coeff program box —

```
poly = rev(poly)
poly = multiply(poly, 1e-30)
```

## 2. The mechanism (deterministic — no RNG remap needed)

```text
xfrm:  z1 = e^(2 pi i t1), z2 = e^(2 pi i t2)          (unit_circle)
       v1 = (z1^4-z2)/(z1^4+z2), v2 = (z2^4-z1)/(z2^4+z1)  (coeff10)
poly_450 (n=35):  A1 = angle(v1), A2 = angle(v2), M = |v1|+|v2|
       base[j-1] = log(M+j)(1+0.1j) (cos(AP+PS) + i sin(AP-PS))
           AP = sin(j A1 + cos(j A2)),
           PS = A1 cos(j pi/35) - A2 sin(j pi/35)
       base[35-k] = base[35-k] conj(v1)^((k%3)+1) + conj(v2)^(k%4), k=1..5
       cf[r-1] += cumprod(M+1..M+r)/(r+1)
zfrm:  rev            view: +/-0.15 (rotate 90), res 100000, 1e9 roots
```

Everything is a pure function of (t1, t2) — the first migration with
NO unseeded draws, so the polypaint sweep hits the same math on the
same parameters: an exact port, not an RNG remap. After rev, the
ladder becomes the leading end and the art is the tiny root cluster
|z| < 0.15 governed by the small constant-end slots. The ~1.85% most
pole-adjacent rows still exceed f32 even after the 1e-30 rescale and
are knifed (the reference kept them; invisible at ensemble scale).

## 3. Trap log (this wave) — two new caps, one inverted trap

- **Scalar expressions cap at 64 TOKENS** (not just 256 chars): the
  direct base spelling needs 90 tokens and cannot compile.
- **Pokes compile chip-per-op** (the 2920 chip trap in poke form):
  each V1/V2 instance costs ~14 chips, so naive "rebuild the parked
  helper before every scan" blows the 256-chip budget.
- **The giga_2897 trap is also a tool**: in
  `multiply(poly, scan(...))` the parked left operand IS tos during
  the arg-scan. The program exploits this with a self-carrying
  accumulator: length 41 = [P6 | 35 payload], header
  P6 = [A1, A2, M, pi/35, conj v1, conj v2], and every gated arg-scan
  reads the header from tos[0..5] while writing the payload
  (gate u = floor((k+34)/40): 0 on header, 1 on payload; pass-1 gates
  preserve the header through products, a zeroing last factor keeps a
  single header copy through add(poly, pop); cumprod's prev chains
  cleanly through the all-1 header region). Corrections then read the
  header via self-slot pokes (poly[4]/poly[5]); poly[6:41] strips it.
- Exact identity used to fit the token cap:
  cos(S+P) + i sin(S-P) = cos(P) e^{iS} - i sin(P) e^{-iS}.
- Grammar facts probed: angle/abs/real/imag/conj exist on complex
  scalars; log/sqrt exist; no ^ or scalar pow; k is live in scan init
  (k=0); poke RHS reads the true TOS.

## 4. Polypaint Coeff Program

`giga_cf10p450.coeff-program.json` (223 tokens, stack_max 5; generator
`scripts/gen_giga_cf10p450_coeff_program.py`; suite
`tests/test_giga_cf10p450_coeff_program.py`, predeploy-gated, plain
build). Verification (measured): coefficient relative parity <= 5.7e-15
vs the exact oracle; the |z|<1 cluster carries 34/35 roots, positional
match <= 9.5e-5; every probe's raw row max (8e40..9.8e57) proves the
f32 overflow; scaled rows all inside f32.

Uploaded 2026-07-17: id `giga-cf10p450`.
