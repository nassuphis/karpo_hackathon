# Recreating `giga_2912` .. `giga_2918`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_29{12..18}/giga_29NN.png`
(python era, pngs 2025-03-06/07) from their saved per-artwork state files
(`*_polyfun.txt` records the full poly/view/job dicts). The reference tree
is read-only; no new run reuses `giga_*` stems.

Artifacts: `giga_2912.coeff-program.json` .. `giga_2918.coeff-program.json`
(one generator: `scripts/gen_giga_2912_2918_coeff_programs.py`).

## 1. The recursive_add family — sequential EMA chains

Every artwork's zfrm chain ends in a `recursive_add*`:

```python
out_i = cf_i*(1-decay) + state*decay;  state = out_i          # recursive_add
out_q = out*andy + cf*(1-andy)                                # recursive_add_q
```

The driver (`pyroots.py`) draws `t=(U(0,1),U(0,1))` per row from an
np.random stream seeded `time+id+pid`, so the chain runs over IID draws,
resetting only at process start (`chunk` ~7M rows/process; the recorded
`i` values are `samples/degree - 1` from small preview runs). A stateless
per-row app program cannot carry the state, so each chain is ported as the
giga_2920 STATELESS ANALOG: the EMA expanded over independent pseudo-draws
(frac cascades of the sweep's own t1,t2 — the 2883/2907 RNG-remap pattern),
validated by 2-D root-cloud correlation against the true sequential chain,
with the chain's half-vs-half sampling floor as the ceiling.

| giga | pipeline | decay/andy | analog | corr / floor |
|---|---|---|---|---|
| 2912 | unit_circle p11b2_v2 cumsum,rev,r_add_q | 1e-11 / 0.9999 | 2 draws (J=1 ghost) | 0.957 / 0.933 |
| 2913 | unit_circle p11b2_v2 cumsum,rev,r_add_q | 1e-3 / 0.999999 | 5 draws (J=4 ladder) | 0.953 / 0.917 |
| 2914 | unit_circle poly_giga_12 uc,r_add | 0.1 | 3 draws [.9,.09,.01] | 0.919 / 0.877 |
| 2915 | unit_circle poly_giga_88 r_add | 0.6 | 4 draws (J=3+tail) | 0.944 / 0.941 |
| 2916 | unit_circle p11b2_v2 rev,roots,r_add | 0.9999 | E + cur + 1 fluct draw | 0.927 / 0.916 |
| 2917 | uc,sum_prod poly_727 rev,r_add | 5e-5 | deterministic (below) | exact |
| 2918 | uc three_balls r_add | 0.7 | 9 draws (J=8+tail) | 0.725 / 0.618 |

All corr measurements: scratchpad `g2912_explore.py` / `g2912_design2.py` /
`g2916_design3.py` (5-8k chained rows, log1p 2-D histograms on the artwork
view boxes).

## 2. The load-bearing physics (measured)

- **p11b2_v2 explodes.** `u = 77*v^15/(t1+t2+3)` is COMPLEX, so
  `exp(i*pi*u)` spans ~40 decades: P(max|cf|>1e6)=0.30, log-flat tail to
  1e42.7. The EMA drags a decaying ghost of each big row across later rows
  — with decay 1e-3 (2913) the ladder steps 3 decades/row, so heavy-tailed
  scales make deep terms matter; a naive `0.999*cf+0.001*E` analog scores
  corr 0.16. The J-draw expansion (each pseudo-draw carrying its own
  heavy-tailed scale) reproduces the ladder: J=4 reaches 0.953. The f32
  transport knife (rows with |cf|>3.4e38, measured 0.9%) removes the
  extreme ghosts in-app; the reference paints them (documented loss,
  included in the corr measurements via knifed analogs).
- **2916's chain state is permanently heavy.** rev+roots turns p11b2's
  tiny-leading rows into giant reciprocal roots (root max p90=1e14,
  p99=1.6e29 — a ~log-flat, infinite-mean tail). At decay 0.9999 the state
  equilibrates at ~1e30 scale, dominated by its biggest recent injections;
  every chained row is state-shaped. The port bakes ONE MC realization of
  E[root-vector] (seed 2916, N=200000): E is realization-ARBITRARY (its
  giant slot is whatever the tail sampled) but cloud-EQUIVALENT — two
  independent realizations give analog-vs-analog corr 0.9995, because the
  visible cloud only sees the giant-slot-dominant shape. No-E analogs
  (state ~ one or two heavy draws) fail at corr 0.17/0.18.
- **2917's chain is deterministic.** decay 5e-5 with a 20000-row horizon
  makes state ≈ E (fluctuation 2.5e-7 relative — dropped):
  `out = (1-5e-5)*cf + 5e-5*E` exactly, E baked (seed 2917, N=2e6).
- **`safe` never fires** (2917/2918): sum|cf| measured [587,654] and
  [1.08,78.5] vs the (1e-10,1e10) window; no gate ported (259 precedent).

## 3. Grammar techniques (and one VM discovery)

- Frac-cascade pseudo-draws: `frac(t1*m1+t2*m2+c)` per uniform, 18-seed
  table (2920 pattern).
- p11b2 in ONE pushless scan against a parked `[S, m]` pair built by a
  GATED scan (floor gates select k==1 for the mod value; pokes compile
  chip-per-op and blow the 256-chip cap at 5 evals). `(k/10)^15` is
  `exp(15*log(k/10))` — no pow in the grammar; the k=0 slot is exactly 0
  via `clog(0) = -inf, cexp(-inf) = 0` (probed bitwise vs numpy).
- The 2920 push-literal-then-scan idiom everywhere heavy: scan init/step
  expressions ride the side table (free), while `multiply(poly, <expr>)`
  arguments compile chips.
- **`tos[j]` inside a vector-op ARGUMENT aliases poly itself** — only scan
  init/step see the pushed stack top. (Discovered by 2915's divide
  squaring the body.) Scalar-per-row divides therefore park a
  constant-filled vector (`scan(51, 0, 1/(X), prev)`) and consume it with
  an elementwise `multiply(poly, pop)`.
- **A leftover stack vector at emit hijacks the emitted poly** — every
  park must be consumed (`drop`, or a popping mix). `drop` is the discard;
  `add(poly, multiply(pop, 0))` is NOT one.
- 2916: `roots_cm(poly, lo, exact)` (np/zgeev order — the componentwise
  EMA mix depends on order parity) + `poly = poly[0:10]` pad strip.
- 2918: `1,2,4,8` spelled `floor(exp(k*ln2)+0.5)` — exact integers inside
  a scan expression.

## 4. Analog-vs-reference simplifications (all ~1e-13 or below)

`|exp(2*pi*i*u)|` taken as exactly 1 (reference: abs of the rounded
exponential, delta ~5e-15·k); 2914 divides by the provably-constant
max|cf| = 100; `(k/10)^15` via exp/log (1e-15 rel); exact-boundary sweep
rows (u or u+v exactly 0.5) can flip Arg sign — everywhere the angle
enters as `exp(i*angle)` the result is identical. 2915's exact-diagonal
rows (u==v) divide by zero and are transport-knifed (the reference's
random draws never collide; 1/N of grid rows, documented).

Ill-conditioned 2916 rows differ between LAPACK implementations by ~1e-2
(numpy vs Accelerate zgeev, measured at u=v=0.9): the 2864 dust doctrine —
the reference's own dust is equally arbitrary; the ensemble is the
artwork.

## 5. Verification

`tests/test_giga_2912_2918_coeff_programs.py` (predeploy-gated): VM vs the
analog oracles at 5 off-diagonal probes — coefficient rel <= 1e-12 for
2912/2913/2914/2915/2917/2918 (measured 3e-14 worst), 2916 per-slot
<= 1e-6 on the Accelerate binary (measured 1.7e-10; residual = the VM's
f32 root cast), a root-multiset check for the deterministic 2917, weight
convexity pins, and 2000-draw safe-knife sweeps. Tokens: 33/84/89/151/35/
16/107 of 256.
