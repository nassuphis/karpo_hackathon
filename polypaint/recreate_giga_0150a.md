# Recreating `giga_0150a`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_0150a/giga_0150a.png` from its
saved dict-format state (`giga_0150a_polyfun.txt`). The reference tree is
read-only; no new run reuses the `giga_0150a` stem.

Artifact: `giga_0150a.coeff-program.json` (generator
`scripts/gen_giga_0150a_coeff_program.py`, 27 tokens).

## 1. Pipeline

```text
xfrm none -> p7f -> zfrm none -> safe
run: roots=1e9, res=50000, alpha=0.0, degree 22, rotate 90, view sq2.5
```

`xfrm none` means **t1, t2 are the RAW sweep uniforms** — no unit-circle
map. The config's `n: 6` is unused (p7f hardcodes n=23). `safe` is the
usual knife (sum|cf| outside (1e-10, 1e10) or non-finite -> zeros).

## 2. p7f (verbatim)

```python
tt1  = exp(2*pi*i*t1)                    # unit-circle point
ttt1 = exp(2*pi*i*tt1)                   # COMPLEX exponent (|.| up to e^2pi)
v    = linspace(Re(tt1), Re(ttt1), 23)   # REAL grid, span up to ~535
b    = t2 decile -> band:
       amp  = [10*t1, 100, 599, 443, 293, 541, 379, 233, 173, 257][b]
       mult = [  11,   17,  83, 179, 127, 103, 283,   3,   5,  23][b]
       trig = cos for band 2, sin otherwise
f    = amp * exp(i * trig(mult * 2*pi * v))
f[22] += 211 * exp(2*pi*i * t2/7)
```

The artwork is ten interleaved "stations" — each t2-decile paints a fixed
amp/frequency family over the t1-parameterized v-grid, plus a rotating
anchor added to the leading slot.

## 3. Port: branchless band lookup via the scan pipeline

`b = floor(10*t2)` reproduces the decile comparisons exactly. The lookup
is one-hot gates against a constant table, staged through parked scans
(all expressions ride the side table; 27 tokens total):

```text
COMB  literal [amps 0..9 | mults 10..19 | 0,0,0]
SELV  = COMB . one-hot   gate = floor(1/(1+|floor(10*t2)-idx|)),
                          idx = k-10*floor(k/10); slot 0 also carries the
                          band-0 t1 factor via (1+floor(1/(1+k))*(t1-1))
CUM   = running sum       -> CUM[9] = amp_b (t1-folded), CUM[19]-CUM[9] = mult_b
B-A   (reads CUM)  = [X0, Xstep, amp, junk...]      X = mult*2pi*v pre-folded
B-B   (reads B-A)  = [X0, Xstep, amp, iscos, junk]  iscos = floor(amp/599)
F     (reads B-B)  = amp * exp(i*((1-iscos)*sin(X_k) + iscos*cos(X_k)))
+ slot-22 gated scan for the 211*exp(2*pi*i*t2/7) addition
```

Techniques (the 2912-2918 trap log applies):
- Quadratic one-hot gates `(k-2)(k-3)/2`, `(k-1)(3-k)`, `(k-1)(k-2)/2`
  over k in {1,2,3} — slots >= 4 of the bridge stages are DON'T-CARE junk
  (the F scan reads tos[0..3] only). First attempt used `(2-k)*k` for the
  k==1 gate, which is -3 at k==3 and polluted the iscos slot — caught by
  the stage-by-stage bisect, the exact trap the quadratic forms avoid.
- `iscos = floor(amp/599)`: 599 is the unique maximum amplitude, so the
  band-2 flag needs no second table.
- Every scan reads the PREVIOUS stage parked on the stack; `drop`
  discards (tos[] aliases poly outside scan expressions).

## 4. Analog-vs-reference deltas (measured 4.5e-11 worst, 210 rows)

- np.linspace forces `v[22] = Re(ttt1)` exactly; the scan computes
  `start + step*22` (ulp-level, amplified by mult*2pi*|v| ~ 1e6).
- X accumulates as `(M*2pi*rt) + (M*2pi*step)*k` vs the reference's
  `(M*2pi)*(rt + step*k)`: association dust ~1e-16*|X|.
- Band boundaries: `floor(10*t2)` vs decile comparisons can disagree only
  when `10*t2` rounds across an integer (ulp-wide grid slivers;
  measure-zero in the reference's random draws).

`safe` never fires: sum|cf| measured in [211.0, 13988.0] over 20000 draws
(the slot-22 addition keeps tiny band-0 rows above ~211).

## 5. Verification

`tests/test_giga_0150a_coeff_program.py` (predeploy-gated): VM vs the
analog oracle at 10 probes covering every band — coefficient rel <= 1e-12
(measured 4.9e-17); analog vs the VERBATIM numpy p7f <= 1e-9 (measured
4.5e-11, 210 rows); root-multiset check on the cos band; 20000-draw knife
sweep; table constants pinned.
