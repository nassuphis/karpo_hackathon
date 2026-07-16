# Recreating `giga_outflow`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_outflow/giga_outflow.png`
from its saved state. The reference tree is read-only; no new run may
use the `giga_outflow` stem.

## 1. Main conclusions

`giga_outflow` (run 2025-03-14; `poly_483`/`coeff6` at snapshot
`01627b4`) carries the strangest finding of the series: **part of the
image is a fossil of numpy's complex-division rounding noise.**

```text
chain: unit_circle,coeff6 -> poly_483 -> rev -> safe
run:   roots=1e9, res=50000, alpha=0.0, procs=14, chunk=2100840,
       degree=34, ROTATE=270
view:  ((-2.75-2.75j), (2.75+2.75j))
```

Verbatim per sample:

```python
z  = e^{2j pi t};  t' = (z^3 + 1j)/(z^3 - 1j)         # coeff6, per axis
# poly_483, j = 1..35:
part1 = re(t1')^j      * sin(j*angle(t2'))
part2 = im(t2')^(35-j) * cos(j*|t1'|)
part3 = log(|t1'| + |t2'| + j)
part4 = (re(t1')+j)*(im(t2')+j)*log(|t1'|+1)
cf[j-1] = (part1*part2 + part3*part4)
          * exp(1j*(angle(t1')*sin j + angle(t2')*cos j + log(|t1'|+1)/j))
cf = cf[::-1]                                          # rev
```

1. **The Mobius maps the circle onto the imaginary axis** — for |z|=1,
   `(z^3+i)/(z^3-i)` is purely imaginary EXACTLY. So `re(t')` is pure
   cancellation dust (~1e-16, sign erratic; up to ~1e-12 near the
   poles), and `part1 = re(t1')^j * ...` — amplified by
   `im(t2')^(35-j)` (up to ~1e24) — is DUST-DRIVEN STRUCTURE. The
   reference computed that dust with numpy's division algorithm; the
   VM's scaled-naive division produces dust of the same scale and
   character but different values. Neither can reproduce the other's
   noise per row — but the noise acts as a per-row PRNG on both sides,
   and the aggregate root clouds in the saved view are
   ensemble-indistinguishable: **corr 0.863 vs same-ensemble noise
   floor 0.712** over 15k rows. The giga_2877 pairing / giga_2883 RNG
   argument, in its purest form yet.
2. **Poles**: `z^3 = i` at `t in {1/12 + k/3}` — the reference
   propagated inf into its try/except (row zeroed); the VM errors such
   rows (scalar division by zero). Grid choices should avoid landing
   t-samples exactly on twelfths; the parameter set is measure-zero.
3. **The fused-scan idiom carries the whole polynomial**: the [t1',
   t2'] pair parks in a stack-side 2-vector; three scans (angle phase,
   the dust-free part3*part4, the dust-amplified part1*part2) read it
   via tos with a swap dance; integer powers spell `exp(n*log(x))`
   (scalar ** is integer-literal-only — for negative real bases this
   carries ~1e-16 imaginary dust, absorbed by the same ensemble
   argument). 95 tokens, zero pool constants, stack_max 5.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_outflow.png` | 50000 x 50000, 8-bit grayscale | 72,401,386 bytes | `b5c2a8e27cad1c97eb02e6c5c9aa02fa5657a7416aab0922cf1ed24575a52dca` |
| `giga_outflow_sml.png` | 1000 x 1000, 8-bit grayscale | 215,264 bytes | `0d353b7a0c69a684564435cdf06bac8881098fbc479b23bfc529854306e86ffa` |
| `giga_outflow_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 217,564 bytes | `2d08d66fa065dec679389cf5889eac0fe02d7a9fd8e68d971e7b4be3506717f1` |

Measured polarity: `_sml` mean 24.88 (dark background), `_sml_inv`
230.12; full-size mid-band ~48.9.

Sidecars: `_paramconstruct` = `unit_circle,coeff6 / poly_483 / rev /
safe`, `_paramcount` = `1000000000`, `_resolution` = `50000`,
`_solver` = `safe`, `_alpha` = `0.0`.

## 3. Polypaint Coeff Program

Proven program (95 tokens, stack_max 5, zero pool constants;
fingerprint-preserving round trip;
`5e9fb1fde9de47a0d53f4b8e0bac2a9751945d75`):

```text
poly = fill(2, 0)
poly[0] = ((E1*E1*E1) + 1i)/((E1*E1*E1) - 1i)      # E1 = exp(pi2i*t1)
poly[1] = ((E2*E2*E2) + 1i)/((E2*E2*E2) - 1i)
poly
poly = scan(35, 0, <A(0)>, <A(k)>)                  # angle phases
poly = multiply(poly, 1i)
poly = exp(poly)
poly
swap
poly = scan(35, 0, <F34(0)>, <F34(k)>)              # part3*part4
poly
swap
poly = scan(35, 0, <F12(0)>, <F12(k)>)              # part1*part2 (dust)
drop
poly = add(poly, pop)
poly = multiply(poly, pop)
poly = rev(poly)
emit
```

(Full expression texts in `scripts/gen_giga_outflow_coeff_program.py`.)

Two-tier verification (`tests/test_giga_outflow_coeff_program.py`):

- **Structural tier**: the dust-free stages (park, phases, part3*part4,
  stack discipline) against an exact-semantics oracle — measured
  `8.9e-16` worst over 8 rows (acceptance 1e-9). The program IS the
  formula.
- **Full tier**: the complete chain on rows measured clean of dust
  domination — worst `3e-2` (acceptance 1e-1); structurally wrong
  programs fail at O(1) everywhere. Rows where the park's own exp/log
  dust diverges are oracle-unpinnable BY CONSTRUCTION and are covered
  by the cloud-equivalence measurement instead.
- The Mobius-to-imaginary-axis regime is pinned (re(t') dust-bounded
  relative to the payload on 2000 rows).

Run settings:

```text
base coefficient function = const
degree                    = 34
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-2.75, 2.75], im in [-2.75, 2.75]
render                    = ROTATE 270, dark background (no invert)
N = 5423  ->    999,847,586 roots (the reference's 1e9 scale)
N =  766  ->     19,951,304 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-outflow`,
predeploy-gated. No deploy needed.

## 4. How the noise paints

The claim "part of the image is rounding noise" is measurable, and the
measurement explains the artwork's name. Mechanism, end to end:

### 4.1 A quantity that is exactly zero

For `w` on the unit circle, `(w+i)/(w-i)` multiplied through by the
conjugate of its denominator gives numerator `w*conj(w) + i(w+conj(w))
+ i^2 = i*2Re(w)` — purely imaginary, EXACTLY. So `re(t')` has no
mathematical content at all: whatever number the machine produces is
the residue of catastrophic cancellation inside the complex division —
a few ulps whose sign and size depend on which way each intermediate
product rounded. Scale ~1e-16 (to ~1e-12 near the poles, where the
denominator shrinks). Deterministic, but erratic in t: neighboring
parameter values give unrelated dust — a hash of the input's low bits,
in effect. This is a high-quality pseudo-random source, free of charge.

### 4.2 A 1e24 lever arm

poly_483 then computes `part1 = re(t1')^j * im(t2')^(35-j) * (trig)`.
Two opposing exponentials collide:

- `re(t1')^j` collapses: dust^1 ~ 1e-16, dust^2 ~ 1e-32, ...
- `im(t2')^(35-j)` explodes: for typical |im| ~ 5, the j=1 slot
  carries im^34 ~ 6e23.

Their product survives ONLY at the lowest j: `dust * 6e23 ~ 1e8` at
j=1, ~1e-9 at j=2, nothing beyond. Measured per-slot census (400
rows): the dust changes exactly the post-rev slots 32-34 — the
CONSTANT-side coefficients — by more than 10%; on most rows the
constant term is dust-DOMINATED (the skeleton's part3*part4
contribution there is O(10-100) against the dust term's ~1e8). One
factor of noise, amplified twenty-four orders of magnitude, lands as
the polynomial's constant term with pseudo-random sign and a magnitude
that swings over several decades as t sweeps.

### 4.3 What the constant term does to roots

The constant term is (up to the leading coefficient) the PRODUCT of
all 34 roots. A constant term that jumps erratically over decades
forces the root set to accommodate an erratic product — which it does
by throwing a few roots far outward (or inward), row by row, in
directions organized by the deterministic phase structure but at radii
set by the noise. Ten thousand rows of that is a radial spray.

### 4.4 The kill switch (measured)

Re-render with the noise surgically removed — `re(t')` forced to its
mathematically exact value, zero — and compare (15k rows, saved view,
log-density):

- **Noise ON** (the artwork): a dark flower-medusa core wrapped in a
  fine RADIAL SPRAY — rays streaming outward in every direction, plus
  a diffuse halo of stray points filling the frame. The outflow.
- **Noise OFF** (the skeleton): the same core, tentacled and compact —
  and nothing else. No rays, no halo, no spray. Confined.
- Cloud correlation between the two: **0.70** (against a same-ensemble
  noise floor of ~0.71 for unrelated halves — i.e. the spray is a
  first-order feature, not a perturbation).

**The outflow that names the piece IS the rounding noise.** The
deterministic chain paints the flower; numpy's division residue paints
everything flying off it.

### 4.5 Why it is recreatable anyway

Neither numpy nor the VM can reproduce the OTHER's dust per row (the
bits differ; even the park's own exp/log dust differs), but both
produce dust of the same scale, sign symmetry, and t-decorrelation —
the same DISTRIBUTION. The spray's statistics (ray density, halo
falloff, radial law) depend only on that distribution, so the
aggregate images agree: corr 0.863 between the numpy-dust and VM-dust
ensembles, against the 0.712 same-ensemble floor. Per-row the dice
differ; per-image the artwork is the same. The reference was
co-authored by numpy's division routine; the recreation is co-authored
by the VM's — and the two collaborators, given the same brief, paint
the same picture.
