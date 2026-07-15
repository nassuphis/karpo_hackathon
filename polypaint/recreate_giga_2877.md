# Recreating `giga_2877`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2877/giga_2877.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2877` (or its stale `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2877` is the seventh `poly_letter` family member and the first
whose zfrm runs a **root-finder inside the coefficient function**. Its
snapshot is `8e28adb` (2025-01-29) — NOT the `500685f` used by
giga_2870..2875 — because two functions changed together there, and this
run's dict keys prove it used them:

```text
chain: none -> poly_letter -> roots_p -> polyroot        (paramconstruct)
poly:  ascii=178, degree=32, andy=100.0, ro=-0.5, io=-0.5
run:   roots=2e9, res=50000, alpha=0.0, procs=14, chunk=4464285
view:  (-3.284383960259202 - 3.3194568380561082j) ->
       ( 3.3994587834346133 + 3.364385905637707j)
```

1. **`poly_letter` gained `ro`/`io`** (glyph offset): roots =
   `FONTXY[b178] + complex(0.05+0.9*t1, 0.05+0.9*t2) + (-0.5-0.5i)`.
   The offset re-centers the half-integer pixel grid onto integers, so
   each glyph root sweeps a 0.9x0.9 cell around an integer point — the
   checkerboard of SOLID grey squares in the image's center. (The run
   file predates the commit by a day; the run used the working tree the
   commit captured. giga_2870..2875 are unaffected: their dicts lack
   ro/io, and `poly.get("ro") or 0` degrades to the 500685f formula.)
2. **`roots_p`** (verbatim at 8e28adb, `andy = 100`):

   ```python
   q = np.append(np.roots(cf), 1) * 100 + cf
   ```

   Since `cf = np.poly(glyph + delta)`, the root VALUES are the shifted
   glyph points themselves, up to eigenvalue dust (~1e-13; invisible at
   100x, view pixel = 1.3e-4). **No solver is needed in the program** —
   the points ship as a second pool constant. The user's instinct
   ("a couple of Aberth iterations") would find the same values; neither
   an in-VM solver nor anything else can recover what actually matters:
3. **The pairing is LAPACK's, and it is irreproducible by construction.**
   `np.roots` returns eigenvalues in zgeev QR emission order. Which root
   lands on which coefficient slot decides the polynomial: `q[0] =
   100*r[0] + 1` — the leading coefficient IS 100x "whichever root LAPACK
   deflates first". Measured over 400 rows: **239 distinct permutations,
   no monotone rule** (not |.|-sorted in either direction, not angle, not
   pool order). The order is deterministic per input but changes
   chaotically across rows AND depends on the LAPACK build — the
   reference image is partly a fossil of one build's internal iteration
   order; even a numpy upgrade could re-dice it.

## 2. What the pairing does to the image (measured)

The image = checkerboard center (pairing-independent: roots stay near
the glyph where |cf| dwarfs 100|r|) + a ring of texture (the
pairing-sensitive low-|cf| slots at both ends). 40,000-row aggregate
clouds on the saved view, log-density correlation vs the LAPACK
ensemble, noise floor 0.96:

| pairing rule | corr | ring character |
|---|---:|---|
| descending |root|  (**chosen**) | **0.934** | feathered shards — same class as reference |
| random permutation per row | 0.770 | diffuse fuzz halo — ensemble washes out |
| fixed pool order | 0.655 | smooth coherent petals — a different artwork |
| ascending |root| | 0.470 | different again |

Why descending wins: shifted-QR deflation tends to emit eigenvalues in
roughly descending magnitude, so the CROSSING STRUCTURE — the parameter-
space curves where the permutation flips, which slice the ring into
feather shards — nearly coincides, even though the exact permutation
never matches (0/40,000 rows). Any argsort of a continuous key is
piecewise-constant with flips at key crossings; that is the texture
mechanism itself, and |.|-descending is the measured-closest expressible
key. The recreation is exact in everything reproducible — values,
formula, association — and honest about the dice: its feathers sit in
the same places with the same character, individually reshuffled.

## 3. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2877.png` | 50000 x 50000, 8-bit grayscale | 112,626,665 bytes | `fb35ac9fd0a4d6069113e9c06af26be8da8a27db01ce59136e5ed40647150707` |
| `giga_2877_sml.png` | 1000 x 1000, 8-bit grayscale | 330,322 bytes | `bc5b86adb5125eef36406aa3646dc042ef9db6d8f90bbb1d72bc89fd9fe96aa6` |
| `giga_2877_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 332,367 bytes | `18da4db72d6b78f1b06e2acd354dc80261db36e6970b311cd8fa91986ee23e6e` |

Measured polarity: `_sml` mean 61.65 (dark background), `_sml_inv`
193.35; full-size mid-band sample ~73.7. No full-size `_inv` exists.

Sidecars: `_alpha` = `0.0` (sic — this run alone), `_paramconstruct` =
`none / poly_letter / roots_p / polyroot`, `_paramcount` =
`2000000000`, `_resolution` = `50000`, `_solver` = `polyroot`.

## 4. Polypaint Coeff Program

Proven program (60 tokens, stack_max 7, pools [32 points, 33 glyph
coeffs], 4 scalar expressions; fingerprint-preserving round trip;
`788e97dfbd9f953e6efa6aada479a347bdb743e0`):

```text
poly = vector_literal(<32 shifted-glyph points>)
poly = add(poly, ((0.05+0.9*t1)+(0-0.5)) + 1i*((0.05+0.9*t2)+(0-0.5)))
poly = sort_abs(poly)
poly
poly = scan(32, 0, tos[31], tos[31-k])          # reverse: descending |.|
drop
poly
poly = translate_roots(roots_ascii_literal(178), <same delta>)
poly = scan(poly_len, 0, poly[0] + 100*tos[0],
            poly[k] + 100*(tos[k-32*floor(k/32)]*(1-floor(k/32)) + floor(k/32)))
drop
emit
```

Mechanics worth keeping (they generalize):

- The append-a-constant idiom: the combine map runs over all 33 slots;
  the guarded index `k - 32*floor(k/32)` reads `tos[0]` harmlessly at
  k = 32 while `floor(k/32)` selects the constant 1 there. No
  out-of-bounds read, no concat primitive needed, and every term is
  commutative-exact against `100*r[k] + cf[k]`.
- The delta expression preserves the reference's association
  (`(0.05+0.9*t) + (-0.5)`, scale-then-offset, as letters.square does).
- Ties: only the fully symmetric delta = 0 row `t = (0.5, 0.5)` has
  exact |.| ties (VM sort and np.argsort may legally differ there, as
  LAPACK's own tie behavior did); pinned structurally — recovered
  `(q - cf)/100` must be the exact root multiset, non-ascending, with
  the 1 in the last slot — using the VM's own cf probe so translate
  dust (|cf[32]| ~ 4e9 amplifies it to ~4e-4) cancels.

Native parity vs the documented formula
(`tests/test_giga_2877_coeff_program.py`): worst `2.1e-9` over 44
tie-free rows (acceptance 1e-7); probes asserted tie-free.

Run settings:

```text
base coefficient function = const
degree                    = 32      (q[0] = 100*r[0] + 1, never zero)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-3.284383960259202, 3.3994587834346133]
                            im in [-3.3194568380561082, 3.364385905637707]
render                    = no rotation, dark background (no invert)
N = 7905  ->  1,999,648,800 roots (2-billion-class run)
N =  790  ->     19,971,200 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2877`, predeploy-gated.

## 5. Variants: three artworks from one formula

Because the pairing is the artistic variable, two sibling programs swap
ONLY the pairing stage (generator `scripts/gen_giga_2877_variants.py`,
suite `tests/test_giga_2877_variants.py`, both parity-proven at 1.1e-9):

- **`giga_2877_v2` — pool order** (id `giga-2877-v2`, 55 tokens,
  fingerprint `c6a6bf20...`): the sort stage is simply removed, so every
  row pairs identically and the ring collapses into smooth coherent
  petals. Cloud corr vs the LAPACK ensemble: 0.655.
- **`giga_2877_v3` — chaotic key** (id `giga-2877-v3`, 59 tokens,
  fingerprint `7d18f94a...`): the descending-|.| stage is replaced by
  `argsort` of the hash-like key `frac(100000*|r|)` — deterministic, but
  it re-dices the permutation from row to row, washing the shards into a
  diffuse fuzz halo. Measured 0.956 cloud corr against a true
  random-per-row ensemble (the noise floor), i.e. visually
  indistinguishable from actual per-row dice.

The three programs differ only in the lines between the delta add and
the push before `translate_roots`:

```text
main:  poly = sort_abs(poly)                 v2:  (nothing)
       poly
       poly = scan(32, 0, tos[31], tos[31-k])
       drop

v3:    poly
       poly = scan(32, 0, 100000*abs(tos[0]) - floor(100000*abs(tos[0])),
                   100000*abs(tos[k]) - floor(100000*abs(tos[k])))
       poly = argsort(pop, poly)
```

Same checkerboard center in all three; the ring is feather shards
(main, 0.934 vs the reference's ensemble), petals (v2), or fuzz (v3).
