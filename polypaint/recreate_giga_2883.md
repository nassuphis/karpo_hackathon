# Recreating `giga_2883`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2883/giga_2883.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2883` (or its stale `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2883` (run 2025-01-29; working tree between `8e28adb` and
`1230acc` — `ltlwd` is first committed at the latter, everything else at
the former) is a **random-coefficient / structured-constellation blend**
under a chaotic parameter map:

```text
chain: bkr -> combine(poly1=ltlwd, poly2=poly_letter) -> rev -> polyroot
poly:  ascii=178, degree=32, n=33, andy=0.75, ro=0.0, io=-1.0
       (factor=0.33 in the dict is read by nothing in this chain)
run:   roots=2e9, res=50000, alpha=0.01, procs=14, chunk=4464285
view:  (-1.945117801034551 - 1.3240234200121446j) ->
       ( 1.5817447791841508 + 2.2028391602065573j)
```

Verbatim per sample:

```python
t' = frac(2*(t mod 1)) + 0.5j*floor(2*(t mod 1))       # bkr, per axis
s  = np.random.choice([-1, 1], size=33)                # ltlwd — TRUE RNG
L  = np.poly(b178 + complex(0.05+0.9*t1', 0.05+0.9*t2') + (0-1j))
cf = s*0.75 + L*0.25                                   # combine, andy=0.75
q  = cf[::-1]                                          # zfrm rev
```

1. **The baker's map makes the uniforms COMPLEX.** Through `scale()`
   and `complex(a, b) = a + 1j*b` the glyph shift decomposes exactly to

   ```text
   Re = 0.05 + 0.9*frac(2u1) - 0.45*floor(2u2)
   Im = 0.45*floor(2u1) + (0.05 + 0.9*frac(2u2)) - 1
   ```

   — the four (floor bits) quadrants place a 2x2 grid of glyph copies,
   each swept by the frac parts. Pure frac/floor algebra; the four
   parts ride the scratch-poke bridge into one translate delta
   (decomposition verified against verbatim complex arithmetic to
   1e-15 in the test suite).
2. **`ltlwd` is true per-sample RNG** — `np.random.choice([-1,1], 33)`,
   independent of the uniforms. The VM is deterministic per row (by
   design: fingerprints, replayability), so NO new primitive was added.
   Instead the scan-PRNG idiom (giga_2877-v3's frac key, extended to a
   chained cascade) generates the signs:

   ```text
   x_0     = frac(t1*7919.7717 + t2*104729.31 + 0.5)
   x_{k+1} = frac(x_k*9821.4959 + 0.211327)
   s_k     = 1 - 2*floor(2*x_k)          # second binary digit -> +-1
   ```

   Measured: sign balance +0.005, intra-row lag-1 correlation -0.003,
   neighboring grid rows (1/7905 apart) agree on 50% +- noise of signs
   (i.e. unrelated), and the aggregate root cloud over 24,000 rows is
   **statistically indistinguishable from the true-RNG ensemble**:
   log-density corr 0.9271 vs same-ensemble noise floor 0.9087, and
   visually identical pinwheels. The user's expectation that this
   would need new primitives was reasonable — but the answer is no:
   the reference's randomness is independent of t, so ANY per-row
   pseudo-random source with good marginals reproduces the mixture.
3. **`rev` is the native reversal transform** — one chip.
4. The degree survives rev: the new leading is `0.75*s_32 + 0.25*L[32]`,
   zero only if the letter's constant term is exactly -+3 — measure
   zero. Saved degree 32; `chunk = floor(2e9/32/14) = 4,464,285` (the
   job dict's own arithmetic, consistent with every sibling).

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2883.png` | 50000 x 50000, 8-bit grayscale | 286,690,965 bytes | `bc9c9bff52d691cc955149b1a8e9f9d8301979bc39e4cbfd14a58b6c252a5d55` |
| `giga_2883_inv.png` | 50000 x 50000, 8-bit grayscale | 286,729,555 bytes | `379e1dbaddf30025e0c4e01c618bcb6c6ca6c3b60c9dfc96cd51d8ac4b30edf3` |
| `giga_2883_sml.png` | 1000 x 1000, 8-bit grayscale | 676,999 bytes | `9d433fc72fc57e698b841e17382040f01480256787c66371736062a78149e33f` |
| `giga_2883_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 677,129 bytes | `a823a84a98a2bf9f7314afd8506152dc6757a346e6de4bd9d75a374590da403c` |

Measured polarity: `_sml` mean 85.25 (dark background), `_sml_inv`
169.75; full-size mid-band ~119.2 / inv ~135.8. This stem HAS a
full-size `_inv` (first in the recreated family).

Sidecars: `_alpha` = `0.01` (the family's densest), `_paramconstruct` =
`bkr / combine / rev / polyroot`, `_paramcount` = `2000000000`,
`_resolution` = `50000`, `_solver` = `polyroot`. `data.stem` is the
stale `giga_2869`.

## 3. Polypaint Coeff Program

Proven program (104 tokens, stack_max 9, one pool constant of length
33; fingerprint-preserving round trip; every chip arg under the
256-char cap; `24d61b357b3e4434467718c7ba76b130907d986e`):

```text
poly = scan(33, 0, <prng state 0>, <prng state k>)
poly = scan(33, 0, 1-2*floor(2*poly[0]), 1-2*floor(2*poly[k]))
poly = multiply(poly, 0.75)
poly
poly = fill(4, 0)
poly[0] = 2*(t1-floor(t1)) - floor(2*(t1-floor(t1)))
poly[1] = floor(2*(t1-floor(t1)))
poly[2] = 2*(t2-floor(t2)) - floor(2*(t2-floor(t2)))
poly[3] = floor(2*(t2-floor(t2)))
poly = translate_roots(roots_ascii_literal(178), (0.05+0.9*poly[0]-0.45*poly[3]) + 1i*(0.45*poly[1]+(0.05+0.9*poly[2])-1))
poly = multiply(poly, 0.25)
poly = add(pop, poly)
poly = rev(poly)
emit
```

(Full PRNG expression texts in `scripts/gen_giga_2883_coeff_program.py`.)

Native parity vs the documented chain
(`tests/test_giga_2883_coeff_program.py`): worst `2.6e-10` over 46 rows
(acceptance 1e-7) — the PRNG arithmetic is bit-identical between the VM
and the oracle (same IEEE multiply/add/floor sequence), so the residual
is pure translate-vs-np.poly dust. No sorts anywhere, hence no tie rows
— the simplest parity story in the family. Suite also pins the PRNG
quality stats and the bkr delta decomposition.

Run settings:

```text
base coefficient function = const
degree                    = 32
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-1.945117801034551, 1.5817447791841508]
                            im in [-1.3240234200121446, 2.2028391602065573]
render                    = no rotation, dark background (no invert)
N = 7905  ->  1,999,648,800 roots (2-billion-class run)
N =  790  ->     19,971,200 roots (5K-class validation)
```

Note the reference alpha is 0.01 — 100x the family's usual; expect a
much denser look at matching N.

Saved through `/save-coeff-program` as id `giga-2883`, predeploy-gated.
No deploy was needed: bkr, the PRNG, combine, and rev all compile to
existing primitives.
