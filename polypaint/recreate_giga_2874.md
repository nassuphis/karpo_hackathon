# Recreating `giga_2874`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2874/giga_2874.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2874` (or its stale `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2874` is the fifth `poly_letter` family member (snapshot `500685f`),
and the first with a **non-trivial `xfrm`**:

```text
chain: jump -> poly_letter -> andy1 -> polyroot          (paramconstruct)
poly:  ascii=16, degree=35, jmp=-0.05,
       levels=[0.1..0.9]                                  (polyfun)
run:   roots=3e9, res=50000, alpha=1e-4, procs=14, chunk=6122448
view:  (-473.44802382666614 - 592.3026556716079j) ->
       ( 457.4360277207128  + 338.5813958757711j)
```

1. **`ascii = 16`**: dictionary key `b16` = sheet cell 15, the CP437 sun
   glyph, horizontally mirrored per the transcription quirks — **36 lit
   pixels** (center 2x2 dark):

   ```text
   #..##..#
   .#.##.#.
   ..####..
   ###..###
   ###..###
   ..####..
   .#.##.#.
   #..##..#
   ```

   36 roots -> 37 coefficients; andy1's leading element is identically 0
   (csi[0] == cpi[0]), so the saved **degree is 35** and
   `chunk = floor(3e9 / 35 / 14) = 6,122,448` matches the saved job
   (actual work `6,122,448 x 14 x 35 = 2,999,999,520` roots).

2. **`xfrm = jump`** (the vector-conditional), verbatim at `500685f`:

   ```python
   jt = t + np.sum(levels > t) * jmp      # per uniform, BEFORE the letter shift
   ```

   With `jmp = -0.05` and nine levels, this is a descending staircase:
   rows near t=0 are pulled 0.45 left/down, rows near t=1 not at all, with
   nine discrete jumps across the sweep — the banding visible in the
   reference. The comparison is a **strict real `>`**; the polypaint
   program reproduces it with the vector `gt` op (real-part strict, exact
   0/1 — C `sweep_cli.c` comparison branch), NOT a floor() staircase:
   `floor(10*t)` diverges on rows whose t lands exactly on a level
   (`0.3*10 == 2.9999999999999996` in doubles). Counts are summed with the
   in-poly cumsum scan and bridged into the translate delta via a sparse
   poke (`poly[0] = tos[8]`) so both counts are readable from one register.

3. **`zfrm = andy1` needs BOTH gathers here.** giga_2872/2873 shipped the
   monic-collapse shortcut (`1 - cf[cpi]`): when `argsort(|cf|)[0] == 0`,
   `cumprod(argsort) % n` is identically zero and the csi gather collapses
   to the monic 1. That regime held 5000/5000 there, but the jump's
   negative deltas (down to `0.05+0.9*(t-0.45)` = -0.355 per axis) drag
   roots near the origin and **51/20,000 seeded rows violate it**
   (clustered around t ~ 0.26..0.30, exactly the reference's most-shifted
   bands). On those rows the reference computed the real thing —
   `cf[cumprod(argsort)%n] - cf[cumsum(argsort)%n]` with numpy **int64**
   cumprod, which WRAPS (37 factors averaging ~18 overflow 2^63 by element
   ~13). The wrapped value is deterministic and part of the image.

4. **Exact int64 emulation in a double-only VM**: the program tracks the
   running product in two 32-bit limbs packed into one complex scan state
   (`prev = lo + 1i*hi`, both < 2^32; `lo*s < 2^38` stays exact), then
   decodes the signed residue: `u mod n` from the limbs via
   `2^32 mod n`, minus `(2^64 mod n) * floor(hi/2^31)` for the sign bit,
   re-buffered non-negative. All moduli are spelled as `poly_len`
   expressions, so the block is glyph-agnostic and the ascii code stays
   scrubbable. **Verified integer-exact against numpy's wrapping cumprod
   on 20,000 seeded rows including all 51 violated ones** (pure-double
   replica), and end-to-end through the native VM on the violated probes.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2874.png` | 50000 x 50000, 8-bit grayscale | 118,065,340 bytes | `82b4a7e7c1820354471a2ce065908883323ad1ed7db619c95b50b7bcb6154277` |
| `giga_2874_sml.png` | 1000 x 1000, 8-bit grayscale | 337,343 bytes | `c5345a1d12875310d83dda8b9763e67ed818182fc2a9def067879c980a0e4423` |
| `giga_2874_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 339,661 bytes | `8b614102e262a63603da6e67b7a1eba12ad8d8ba5228c75d8bb1c323bf7260a7` |

Measured polarity: `_sml` mean 25.91 (dark background), `_sml_inv` 229.09;
full-size mid-band sample ~51.4. No full-size `_inv` exists.

Sidecars: `_alpha` = `0.0001`, `_paramconstruct` = `jump / poly_letter /
andy1 / polyroot`, `_paramcount` = `3000000000`, `_resolution` = `50000`,
`_solver` = `polyroot`. The `data.stem` is the stale `giga_2869` (same
saved-state quirk as the rest of the family).

## 3. Polypaint Coeff Program

Proven program (157 tokens, stack_max 11, pools [9 levels, 37 glyph
coeffs], 12 scalar expressions — the largest recreation to date;
fingerprint-preserving round trip; `a738e4cf45770c6f03d11f45683b72053fd96131`):

```text
fill(9, t1)
poly = vector_literal(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9)
poly = gt(poly, pop)
poly = scan(9, 0, poly[0], prev + poly[k])
poly
fill(9, t2)
poly = vector_literal(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9)
poly = gt(poly, pop)
poly = scan(9, 0, poly[0], prev + poly[k])
poly[0] = tos[8]
drop
poly = translate_roots(roots_ascii_literal(16), (0.05+0.9*(t1+(0-0.05)*poly[0])) + 1i*(0.05+0.9*(t2+(0-0.05)*poly[8])))
poly
poly = arange(0, poly_len)
poly = argsort(poly, peek)
poly = scan(poly_len, 0, poly[0], <int64 limb step>)
poly
poly = scan(poly_len, 0, <sign flag map>)          # floor(imag/2^31)
poly = multiply(poly, <-(2^64 mod poly_len)>)
poly
swap
poly = scan(poly_len, 0, <hi mod poly_len map>)
poly = multiply(poly, <2^32 mod poly_len>)
poly
swap
poly = scan(poly_len, 0, <lo mod poly_len map>)
drop
poly = add(pop, poly)
poly = add(pop, poly)
poly = rem(poly, poly_len)
poly = add(poly, poly_len)
poly = rem(poly, poly_len)
poly = scan(poly_len, 0, tos[poly[k]], tos[poly[k]])   # A = cf[csi]
poly
swap
poly = arange(0, poly_len)
poly = argsort(poly, peek)
poly = scan(poly_len, 0, poly[0], prev + poly[k])
poly = rem(poly, poly_len)
poly = scan(poly_len, 0, tos[poly[k]], tos[poly[k]])   # B = cf[cpi]
drop
poly = subtract(pop, poly)                             # andy1 = A - B
emit
```

(Full expression texts in `scripts/gen_giga_2874_coeff_program.py`; each
chip argument sits under the 256-char wire cap — the sign/moduli
expressions are deliberately close to it, which is why the limb decode is
split into three cheap maps combined with vector ops instead of one
monolithic expression.)

Native parity vs the verbatim formula
(`tests/test_giga_2874_coeff_program.py`):

- Generic rows: worst `5.8e-9` over 60 random rows + 5 pinned probes
  (acceptance 1e-7). Probe `(0.3, 0.7)` sits exactly ON level values and
  pins the strict-> counts (6, 2).
- Collapse-violating rows: **gather indices integer-exact on all 51**
  seeded violated rows; residual is cancellation amplification in
  `cf[csi] - cf[cpi]` (nearly-equal huge coefficients), worst `1.9e-4`
  relative (acceptance 1e-3). Root-position impact is sub-pixel at 50K.
- The int64-limb decode itself: 2000-row integer-exactness subtest suite
  (20,000 rows verified during development, 0 mismatches).

Run settings:

```text
base coefficient function = const
degree                    = 35          (andy1's leading zero strips one)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-473.44802382666614, 457.4360277207128]
                            im in [-592.3026556716079, 338.5813958757711]
render                    = no rotation, dark background (no invert)
N = 9258  ->  2,999,869,740 roots (3-billion-class run)
N =  756  ->     20,003,760 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2874`, predeploy-gated.

## 4. Findings (what this recreation surfaced)

Things learned here that were not visible in any earlier wave, in the
order they were hit:

1. **Random regime checks have a shelf life.** The giga_2872/2873
   monic-collapse shortcut was *proven* there (5000/5000 rows). The same
   check on this chain failed at 15/5000 on the first run — the jump was
   the first transform that produces negative deltas, and no amount of
   confidence from sibling programs transfers across a regime boundary.
   The check is cheap; rerun it per program, never inherit it.

2. **The reference image contains integer overflow.** On collapse-
   violating rows, `np.cumprod` of int64 argsort indices wraps 2^63
   around element 13 of 37. That wrapped garbage is deterministic, feeds
   the gather, and is *in the pixels* of `giga_2874.png`. Recreating the
   image means recreating the overflow. "Approximately right" (double
   cumprod, which does NOT wrap) was measurably wrong on those rows in a
   structured, band-clustered way — precisely the rows the jump makes
   visually distinctive.

3. **Exact int64 in a double-only VM is a scan idiom.** Two 32-bit limbs
   packed into the complex scan state (`prev = lo + 1i*hi`) keep every
   intermediate below 2^38, where doubles are exact integers. The sign
   bit of the wrapped value is just `floor(hi/2^31)`, and the signed
   residue needs only `2^32 mod n` and `2^64 mod n`, both expressible as
   `poly_len` arithmetic. Nothing about this is 2874-specific; it is a
   general recipe for integer-semantics replication in the VM.

4. **`tos[...]` inside a `translate_roots` delta reads the wrong vector.**
   The lowering pushes the coefficient operand FIRST, so by the time the
   delta expression evaluates, top-of-stack is the just-pushed glyph
   constellation — not your data. Reads from the **poly register**
   (`poly[0]`, `poly[8]`) are the only ordering-safe bridge for computed
   values into a delta. Hence the sparse poke `poly[0] = tos[8]`, which
   parks both jump counts in one register before the translate.

5. **A scan may read `poly[k]` while assigning `poly`.** The scan writes
   to scratch and pushes, so `poly = scan(n, 0, poly[0], prev + poly[k])`
   is alias-free — the in-place-looking cumsum is legal. This removed
   every push/drop pair the 2872/2873 blocks used around their cumsums.

6. **The 256-char wire cap shapes programs.** The one-expression spelling
   of the limb decode was rejected at compile (`chip arg is too long`).
   The working form splits it into three cheap elementwise maps (lo mod
   n, hi mod n, sign flag) combined with vector multiply/add whose scalar
   arguments each fit — the `-(2^64 mod n)` expression is 236 chars,
   deliberately close to the cap. Long derivations want to be pipelines
   of short maps, not one giant formula.

7. **Cancellation is not error.** On violated rows the parity residual
   jumps from 5.8e-9 to 1.9e-4 — alarming until the gather indices were
   dumped and proven integer-exact. The residual is the shared coefficient
   dust amplified by subtracting nearly-equal huge values in
   `cf[csi] - cf[cpi]`; the reference's own float64 arithmetic amplifies
   identically. Diagnose parity blowups by checking the *discrete*
   structure first; only float dust should remain.
