# Recreating `giga_2875`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2875/giga_2875.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2875` (or its stale `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2875` is the sixth `poly_letter` family member (snapshot `500685f`):
giga_2871's chain (`translate + sort_abs`) with giga_2874's `jump` xfrm
in front — at **double strength**:

```text
chain: jump -> poly_letter -> sort_abs -> polyroot       (paramconstruct)
poly:  ascii=2, degree=32, jmp=-0.1, levels=[0.1..0.9]   (polyfun)
run:   roots=2e9, res=50000, alpha=1e-4, procs=14, chunk=4464285
view:  (-10.683722226822201 - 10.863959531634595j) ->
       ( 11.0247733321544   + 10.844536027342006j)
```

1. **`ascii = 2`**: dictionary key `b2` = sheet cell 1, the CP437 smiley,
   horizontally mirrored per the transcription quirks — **32 lit pixels**:

   ```text
   .######.
   #......#
   #.#..#.#
   #......#
   #.####.#
   #..##..#
   #......#
   .######.
   ```

   32 roots -> 33 coefficients; sort_abs only permutes, so the saved
   **degree is 32** and `chunk = floor(2e9 / 32 / 14) = 4,464,285`
   matches the saved job (actual work `4,464,285 x 14 x 32 =
   1,999,999,680` roots).

2. **`jmp = -0.1`** doubles giga_2874's staircase: nine jumps of 0.1, so
   deltas span `[-0.76, 0.95]` per axis and the jumped-uniform image has
   **gaps** — e.g. `jt` can never equal 0.5 exactly (each branch
   `t - 0.1*count` misses it). Two consequences measured:
   - The sorted-leading == monic-1 regime of giga_2871 **breaks on
     59/20,000 seeded rows** (clustered near t ~ 0.44; `|cf|min` down to
     0.156). Unlike giga_2874 this needs no new machinery — sort_abs is
     verbatim in both regimes; the polynomial simply ships non-monic with
     a small leading on those rows, identically in both pipelines.
   - The leading is **never exactly zero**: a zero constant term needs a
     root exactly at the origin, i.e. a reachable delta equal to the
     negative of a lit pixel, and the staircase gaps exclude every such
     value (contrast giga_2870, where `(0.5, 0.5)` hit one exactly). The
     degree therefore holds at 32 on every row.
3. The small view (~±11) is the sort_abs signature, as with
   giga_2871/2870.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2875.png` | 50000 x 50000, 8-bit grayscale | 92,038,559 bytes | `0a8d0e5a35cfd516cc764e7903e8bb2356115b5acf11810c39ebfc10388fe248` |
| `giga_2875_sml.png` | 1000 x 1000, 8-bit grayscale | 387,926 bytes | `5549160d7b8a835c036fbda6fba3a5d6a70823aa2814bb7ec8e07dd74902d7a1` |
| `giga_2875_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 389,540 bytes | `b5d673119970f3f15c97d6d0704197862e9ffe8ab16a28a5a96efee36251a81c` |

Measured polarity: `_sml` mean 30.79 (dark background), `_sml_inv`
224.21; full-size mid-band sample ~67.2. No full-size `_inv` exists.

Sidecars: `_alpha` = `0.0001`, `_paramconstruct` = `jump / poly_letter /
sort_abs / polyroot`, `_paramcount` = `2000000000`, `_resolution` =
`50000`, `_solver` = `polyroot`. `data.stem` is the stale `giga_2869`.

## 3. Polypaint Coeff Program

Proven program (53 tokens, stack_max 9, pools [9 levels, 33 glyph
coeffs], 3 scalar expressions; fingerprint-preserving round trip;
`cead696863d1d263b71b3b2f0ca277ca1384dacd`):

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
poly = translate_roots(roots_ascii_literal(2), (0.05+0.9*(t1+(0-0.1)*poly[0])) + 1i*(0.05+0.9*(t2+(0-0.1)*poly[8])))
poly = sort_abs(poly)
emit
```

The jump block is giga_2874's verbatim (see recreate_giga_2874.md §4 for
the strict-> rationale, the in-poly cumsum, and the poke bridge); only
`jmp` and the glyph differ.

Native parity vs the verbatim formula
(`tests/test_giga_2875_coeff_program.py`): worst `3.4e-9` over 67 rows —
5 pinned probes (incl. `(0.3, 0.7)` exactly ON levels, pinning strict->
counts 6 and 2), 2 pinned non-monic rows, 60 random rows. Acceptance
1e-7 for BOTH regimes: sort_abs carries no cancellation, so the
non-monic rows cost nothing (3.1e-9 vs 7.7e-10 — compare giga_2874's
1.9e-4). Monic/non-monic leading pinned per probe.

Run settings:

```text
base coefficient function = const
degree                    = 32          (sort_abs permutes; degree holds)
solver                    = aberth_mt or companion_matrix
viewport                  = re in [-10.683722226822201, 11.0247733321544]
                            im in [-10.863959531634595, 10.844536027342006]
render                    = no rotation, dark background (no invert)
N = 7905  ->  1,999,648,800 roots (2-billion-class run)
N =  790  ->     19,971,200 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2875`, predeploy-gated.
