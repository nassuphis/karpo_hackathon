# Recreating `giga_2880`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2880/giga_2880.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2880` (or its stale `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2880` (snapshot `8e28adb`) is the deepest chain in the series —
a counter-driven constellation through FIVE zfrm stages ending in a
root-find on nonlinearly transformed coefficients:

```text
chain: none -> nopoly_crazy1
       -> poly, sort_moduli_keep_angles_p, invuc, normalize, roots_p
       -> polyroot                                        (paramconstruct)
poly:  ascii=2, degree=32, andy=0.1, i=6252, norm=True (vestigial)
run:   roots=2e9, res=50000, alpha=1e-4, procs=14, chunk=4464285
view:  (-1.4649402686600372 - 1.3300933736000429j) ->
       ( 1.1453348320402872 + 1.280181727100281j)
```

Verbatim per sample (i is a GLOBAL counter, +1 per sample):

```python
offset = ((i%71)/5) * exp(2j*pi*(i%101)/3)     # 71 radii x 3 angles, 7171-cycle
rts    = b2_pixels + complex(0.05+0.09*t1, 0.05+0.09*t2) + offset
cf     = np.poly(rts)
cf     = (sort(|cf|) * exp(1j*angle(cf))) * 0.1 + cf
cf     = cf / exp(2j*pi * cf/max|cf|)          # invuc — nonlinear
cf     = cf / max|cf|
q      = 0.1 * append(np.roots(cf), 1) + cf    # roots_p
```

### 1.1 The counter -> uniform remap (measured)

The offset cycle is independent of the uniforms and, by CRT (71 and 101
coprime), uniform over all 7171 (radius, phase) combinations — the phase
`(i%101)/3` lands on only THREE angles (0, 120, 240 degrees) with exact
weights 34/34/33 per 101. Polypaint rows have no cross-sample state, so
the program splits each uniform into coarse + fine parts:

```text
m = floor(71*t1) mod 71     radius index     frac(71*t1) = jitter1
n = floor(101*t2) mod 101   phase step       frac(101*t2) = jitter2
```

The joint law (m, n, jitter1, jitter2) is IDENTICAL to the reference's
(offset x uniforms) — the marginals are exact (angle weights 34/34/33
scale precisely) and coarse/fine parts of a uniform are independent.
Measured on 20,000-row aggregate clouds in the saved view: corr 0.924
against a counter-faithful simulation, with same-ensemble noise floor
0.876. (The reference's partial last cycle overweights early offsets by
~0.16% — invisible. The workers' fork start value i=6252 only phases
the cycle; the distribution is start-independent.)

### 1.2 The chain stages

- `sort_moduli_keep_angles` is the native `sort_mod_keep_angle`
  transform, bit-for-bit the same construction (ascending moduli onto
  original angles). The 0.1 admixture is multiply + add.
- `invuc` and `normalize` are spelled in-language: `max|cf|` = abs of
  the last element of `sort_abs`, then elementwise divide / `exp` of
  `2j*pi` times the scaled vector. Front-chain parity vs verbatim
  numpy: **2.7e-14**.

### 1.3 roots_p: the root-find and two measured traps

The roots at this stage are genuinely unknown at compile time (invuc is
nonlinear), so the program uses the VM's companion-matrix eigensolver
`roots_cm` — the same zgeev-on-companion route np.roots takes, with the
same matrix layout. Two things stood between it and fidelity:

1. **The legacy leading-strip deleted the artwork.** invuc + normalize
   crush the leading coefficients (the mid-slot maxima reach ~1e29
   before normalize, leaving leads at 1e-20..1e-45); a mean of **16.3
   slots** sit below roots_cm's protective relative threshold
   (|cf|^2 < max^2*1e-15). np.roots keeps them — and their GIANT roots
   (up to ~1e19) feed q's leading half through `0.1*r[k] + cf[k]`.
   Stripping them collapses the aggregate cloud to corr **0.51** vs the
   reference ensemble. This wave added a `strip` mode to `roots_cm`
   (`rel` = legacy default, `exact` = np.roots semantics: only
   exactly-zero leads stripped); registry + enum plumbing + C, wired
   through both the legacy-transform and coeff-program dispatch paths.
2. **LAPACK interface vintages order differently.** The zgeev emission
   order pairs roots to slots (giga_2877's lesson: the pairing is the
   artwork). Accelerate's legacy CLAPACK-3.2 symbol (`zgeev_`) emits a
   DIFFERENT order than its modern `$NEWLAPACK` interface — which is
   what numpy links. Local test builds bind the modern symbol
   (`POLYPAINT_ACCELERATE_NEWLAPACK`); with it, VM rows match the
   verbatim chain **order-positionally** at worst **2.6e-7** over 30
   rows (pure float32 root-cast residual; corr impact of the cast
   measured 1.0000). The deployed layer's netlib LAPACK 3.10 was
   verified the same lineage: the ARM64 docker gate
   (`scripts/test-roots-cm-strip-docker.sh`) builds against the actual
   deploy layer libs and passes order-positional parity at **1.9e-7**.
   Unlike giga_2877, this recreation is order-exact everywhere.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2880.png` | 50000 x 50000, 8-bit grayscale | 134,447,775 bytes | `6fba146797df121c09065ef1ea549312088b610802e4016f4692a79cc1c80d93` |
| `giga_2880_sml.png` | 1000 x 1000, 8-bit grayscale | 554,435 bytes | `aad06b2f38274fda0cc20758d3b08c290b38da527a5675acdea95af538bda518` |
| `giga_2880_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 557,091 bytes | `c54bcd2a3c7b94fd9e6a18b7ee9c39e51c462e9c8df484a91bdaa25a03647031` |

Measured polarity: `_sml` mean 31.96 (dark background), `_sml_inv`
223.04; full-size mid-band sample ~31.1. No full-size `_inv` exists.

Sidecars: `_alpha` = `0.0001`, `_paramconstruct` = `none /
nopoly_crazy1 / poly,sort_moduli_keep_angles_p,invuc,normalize,roots_p /
polyroot`, `_paramcount` = `2000000000`, `_resolution` = `50000`,
`_solver` = `polyroot`. `data.stem` is the stale `giga_2869`; the
dict's `norm: True` is read by poly_letter, not nopoly_crazy1 —
vestigial here.

## 3. Polypaint Coeff Program

Proven program (125 tokens, stack_max 8, one pool constant of length
33; fingerprint-preserving round trip;
`bffcf2c67061f672701eff4bf88b9768b89831c6`):

```text
poly = fill(2, 0)
poly[0] = floor(71*t1) - 71*floor(floor(71*t1)/71)
poly[1] = floor(101*t2) - 101*floor(floor(101*t2)/101)
poly = translate_roots(roots_ascii_literal(2), (0.05+0.09*(71*t1-floor(71*t1))) + 1i*(0.05+0.09*(101*t2-floor(101*t2))) + (poly[0]/5)*exp(1i*6.283185307179586*(poly[1]/3)))
poly
poly = sort_mod_keep_angle(poly)
poly = multiply(poly, 0.1)
poly = add(pop, poly)
poly
poly = sort_abs(poly)
poly = divide(peek, abs(poly[32]))
poly = multiply(poly, 6.283185307179586i)
poly = exp(poly)
poly = divide(pop, poly)
poly
poly = sort_abs(poly)
poly = divide(pop, abs(poly[32]))
poly
poly = roots_cm(poly, lo, exact)
poly[32] = 1
poly = multiply(poly, 0.1)
poly = add(pop, poly)
emit
```

Idioms: the scratch-poke bridge carries the discrete offset indices
into the translate delta (poly-register reads are the ordering-safe
path); `divide(peek, abs(poly[32]))` reads the max modulus off the
freshly sorted register while the scalar chips still see it, keeping
the unsorted vector on the stack; `roots_cm(poly, lo, exact)` leaves
its zero pad in slot 32, exactly where the appended 1 goes.

Tests (`tests/test_giga_2880_coeff_program.py`): freshness/structure,
fingerprint round trip, the counter-remap marginals (angle weights
34/34/33 exact), the strip census (mean 16 slots below the legacy
threshold, all nonzero — why `exact` exists), order-positional VM
parity at 1e-5 acceptance (measured 2.6e-7), and a guard that the
plain (LAPACK-less) build fails loudly rather than silently zeroing.
The deploy-lineage gate is `scripts/test-roots-cm-strip-docker.sh`
(PASSED, worst 1.9e-7).

Run settings:

```text
base coefficient function = const
degree                    = 32
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-1.4649402686600372, 1.1453348320402872]
                            im in [-1.3300933736000429, 1.280181727100281]
render                    = no rotation, dark background (no invert)
N = 7905  ->  1,999,648,800 roots (2-billion-class run)
N =  790  ->     19,971,200 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2880` (requires the
strip=exact deploy first), predeploy-gated.
