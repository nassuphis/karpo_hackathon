# Recreating `giga_2902`

## 0. Objective and safety boundary

Recreate the image represented by:

```text
/Users/nicknassuphis/pyroots/giga_2902/giga_2902.png
```

using the saved state in:

```text
/Users/nicknassuphis/pyroots/giga_2902/giga_2902_polyfun.txt
```

The existing `/Users/nicknassuphis/pyroots` tree and all existing `giga_*`
directories are reference material only. Nothing in that tree should be deleted,
renamed, checked out, regenerated, or overwritten.

All experiments and renders must run in a disposable local clone, with outputs in
a new scratch directory. In particular, never pass `giga_2902` or `giga_2869` as
the output stem of a new run.

## 1. Main conclusions

1. `giga_2902_polyfun.txt` is the authoritative reproduction specification.
2. `data.stem = "giga_2869"` is not an input-data dependency. This program
   generates roots directly and does not read a root dataset. The stale stem is
   evidence that the result was renamed or republished as `giga_2902` after the
   compute state was written.
3. `poly.i = 3029` is not a user parameter. It is the final loop index left by
   the 100,000-root view-estimation pass: `100000 // 33 == 3030`, so the last
   index is 3029.
4. The exact saved complex view is important. A normal `pyroots.py` invocation
   estimates a new random view and will not reproduce the framing.
5. The original calculation was nondeterministic. Worker seeds used the clock,
   PID, and worker number, and workers updated a shared histogram without a
   process-level synchronization contract. A rerun can reproduce the same image
   construction and composition, but not the original PNG byte-for-byte.
6. The target predates the later `polys/` package split. The closest recoverable
   historical snapshot is commit `fb854a4`: `rjail3` was committed there on
   5 February 2025, one day after the target files' 4 February timestamp. The
   target was likely produced from the uncommitted work that became that commit.
7. The current `polys/poly900.py::poly_chess5` has the same relevant behavior for
   this saved state. Its newer `shape_name` option defaults to `circle`, which is
   the shape used by the historical implementation.

## 2. Reference artifacts

Observed files:

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2902.png` | 50000 x 50000, 8-bit grayscale | 78,710,989 bytes | `14ba7eb78adc62d1f1b7370b57a4b6491968de70477bd2ce838c26506315f369` |
| `giga_2902_sml.png` | 1000 x 1000, 8-bit grayscale | 300,585 bytes | `5361c0ef3eb23b0d38492efe6ecd191daa47af11cf1a302f6a7de9f54610db74` |
| `giga_2902_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 302,805 bytes | `5f3ccfb868adba50d9d7630547051fe7836bd854aa40db0ff945d847e0fffc57` |

The thumbnail suffixes are counterintuitive:

- The full image has a light background.
- Downsampling the full image with libvips matches `giga_2902_sml_inv.png`
  within at most two gray levels per pixel; mean absolute difference is 0.025.
- Inverting that downsample matches `giga_2902_sml.png` to the same tolerance.

Therefore the reference naming contract is:

```text
giga_2902.png          full image, light background
giga_2902_sml_inv.png  1000px thumbnail, same polarity as full image
giga_2902_sml.png      1000px thumbnail, inverse/dark-background version
```

Do not infer polarity from the filenames.

## 3. Saved calculation

The meaningful saved state is:

```python
poly = {
    "a": 0.7,
    "andy": 0.01,
    "degree": 33,
    "phi": 0.0,
    "poly": "poly_chess5",
    "rho": 0.1,
    "rloc": "rjail3",
    "solve": "solve",
    "xfrm": "none",
    "zfrm": "sort_abs_p",
}

view = {
    "alpha": 0.0,
    "margin": 0.05,
    "res": 50000,
    "samples": 100000,
    "subview": "full",
    "view": (
        -27.25697676487409 - 18.351336173567184j,
        9.165316633769502 + 18.070957225076405j,
    ),
}

job = {
    "chunk": 4329004,
    "procs": 14,
    "roots": 2000000000.0,
}

png = {
    "png": "invert",
    "rotate": 90,
}
```

Two defaults are also part of the calculation even though they are absent from
the saved dictionary:

```text
speed = 1.0
shape_name = circle
```

`n = None` is unused by `poly_chess5`. `args.sets = "rloc=rjail3"` is CLI
bookkeeping; the effective value is the separate `rloc` key.

## 4. How the program hangs together

The selected path is:

```text
pyroots.py
  -> polyinit / polystate
  -> polyflow.sample()
  -> xfrm.none
  -> poly_chess5
  -> zfrm.sort_abs_p
  -> solve.solve (numpy.roots)
  -> polyview.pixels
  -> shared uint8 occupancy histogram
  -> threshold to binary
  -> rotate 90 degrees
  -> invert
  -> 50000px PNG and 1000px thumbnails
```

Relevant current sources are:

```text
polys/poly900.py:2419   bimodal_skewed
polys/poly900.py:2712   circle
polys/poly900.py:2830   rjail3
polys/poly900.py:3047   poly_chess5
polylayout.py:5         layout2coord
zfrm.py:349             sort_abs_p
solve.py:158            solve
polyflow.py:26          sample
polyview.py:116         pixels
pyroots.py:18           worker
pyroots.py:65           shared histogram
```

There are duplicate copies of `poly_chess5` in the current tree. The current
runtime lookup in `polystate.update_flow()` resolves the top-level
`poly.py::poly_chess5`; `polys/poly900.py` and `polys/chess.py` contain matching
implementations. In the historical snapshot the function is directly in
`poly.py`. This duplication matters when choosing code to run, but does not alter
the formula described below.

## 5. What `poly_chess5` calculates

### 5.1 Layout

`rjail3` is an ASCII grid. `polylayout.layout2coord()` centers the grid and
turns each `S` and `T` cell into a complex-plane coordinate. It contains exactly:

```text
33 source (`S`) positions
33 target (`T`) positions
```

This produces two sets of 33 moving roots and therefore two monic degree-33
polynomials.

### 5.2 Moving root sets

For each polynomial sample, the function draws a fresh uniform value `t` and
computes:

```text
u  = exp(2*pi*i*t)
d1 = rho * u
d2 = rho * exp(2*pi*i*(t*speed + phi))
```

For this saved state, `rho=0.1`, `speed=1`, and `phi=0`, so `d1 == d2`.

The source roots are constructed literally as:

```text
(sX + d1) + i*(sY + d1)
```

and target roots use `tX`, `tY`, and `d2`. Since `d1` is complex, this is not
merely a conventional translation in x and y. Algebraically it is:

```text
sX + i*sY + (1+i)*d1
```

so the circular displacement is also rotated and scaled by `1+i`.

The two coefficient vectors are:

```text
source_coeff = numpy.poly(source_roots)
target_coeff = numpy.poly(target_roots)
```

### 5.3 Random coefficient interpolation

`a=0.7` is not used as a fixed 70% blend. It parameterizes
`bimodal_skewed(a)`, which draws another uniform random value `U` and returns a
random blend weight concentrated near zero or one:

```text
if U < 0.5:
    b = (2*U) ** (1/(1-a)) / 2
else:
    b = 1 - (2*(1-U)) ** (1/(1-a)) / 2
```

The polynomial coefficients are then:

```text
coeff = target_coeff*b + source_coeff*(1-b)
```

`polyflow.sample()` initially draws two random values for `xfrm.none`, but
`poly_chess5` overwrites and ignores those arguments. Those unused draws still
advance the worker RNG and are therefore part of the historical random sequence.

### 5.4 Coefficient transform

`sort_abs_p` computes:

```text
coeff2 = coeff + 0.01 * coeff[argsort(abs(coeff))]
```

This is an elementwise addition of the original coefficient vector and a copy
sorted by magnitude. It is not a convex 1% interpolation because the original
vector is not multiplied by `0.99`.

### 5.5 Solve and rasterize

`solve.solve()` calls `numpy.roots(coeff2)` and returns 33 roots. Exceptions are
converted to 33 zero roots.

Each root inside the saved complex view is mapped to one of 50,000 x 50,000
pixels. The imaginary axis is flipped during pixel mapping. Workers increment a
shared `uint8` histogram, after which every nonzero cell is set to 255. The full
image is therefore an occupancy bitmap; its grayscale-looking thumbnail comes
from downsampling dense binary marks, not from count intensity.

## 6. Counts and resource implications

The nominal request is two billion roots. The implementation calculates the
per-worker polynomial count as:

```text
chunk = floor(2,000,000,000 / 33 / 14) = 4,329,004
```

Actual work is:

```text
4,329,004 polynomial samples/worker
14 workers
33 roots/sample
1,999,999,848 roots total
```

The integer division drops 152 roots from the nominal request.

Memory requirements are substantial:

- Shared histogram: `50000 * 50000 * 1 byte` = 2.5 GB decimal.
- Final `hist.astype(..., copy=True)`: another 2.5 GB.
- Fourteen Python/NumPy/SciPy worker processes add significant resident memory.
- libvips needs additional cache and possible temporary-disk space while writing
  the 50K PNG.

Do not start the full run unless at least 16 GiB of memory is genuinely available
to the job; more is preferable. Reserve at least 10 GB of free temporary disk
even though the reference PNG itself is only about 79 MB compressed. The current
filesystem has ample disk space, but available RAM still needs to be checked
outside the sandbox immediately before execution.

The original implementation has no checkpoint. A worker failure near the end
loses the in-memory histogram. The robust sharded approach in section 11 is
therefore preferable to a single monolithic rerun.

## 7. Why a naive current-tree invocation is insufficient

A command reconstructed from the saved arguments would look approximately like:

```bash
python pyroots.py NEW_STEM \
  --mode write \
  --roots 2000 \
  --procs 14 \
  --samples 100000 \
  --alpha 0 \
  --margin 0.05 \
  --res 50000 \
  --view full \
  --xfrm none \
  --poly poly_chess5 \
  --zfrm sort_abs_p \
  --solve solve \
  --setf a=0.7,andy=0.01,phi=0.0,rho=0.1 \
  --sets rloc=rjail3 \
  --png invert \
  --rotate 90
```

Do not use that as the final reconstruction command for three reasons:

1. `polyinit.setup()` performs a new random view estimate and overwrites the
   exact saved bounds.
2. Current `polypng.py` no longer implements the historical rotate/invert/
   thumbnail pipeline represented by the saved PNG settings.
3. Current `main` has accumulated substantial changes since February 2025.

The command is useful only as a small smoke test in a disposable clone.

## 8. Isolated environment

Use a disposable clone, not a worktree attached to the reference repository:

```bash
git clone --no-hardlinks \
  /Users/nicknassuphis/pyroots \
  /private/tmp/pyroots-giga-2902

git -C /private/tmp/pyroots-giga-2902 \
  checkout --detach fb854a4

cd /private/tmp/pyroots-giga-2902
```

This leaves `/Users/nicknassuphis/pyroots` untouched.

The observed environment is Python 3.13 with at least:

```text
numpy 2.1.3
scipy 1.15.1
matplotlib 3.10.0
pyvips 2.2.3
python-flint 0.7.0a5
mpmath 1.3.0
boto3 1.35.95
```

`solve.py` imports `flint`, `polyroot`, and `mpsolve` at module load even though
this calculation selects `numpy.roots`. The existing compiled modules are:

```text
/Users/nicknassuphis/pyroots/build/src/polyroot.cpython-313-darwin.so
/Users/nicknassuphis/pyroots/build/src/mpsolve.cpython-313-darwin.so
```

The existing virtual environment has a `pyroots.pth` pointing at that build
directory. It can be used read-only for initial verification. Set
`PYTHONDONTWRITEBYTECODE=1` and put all Matplotlib/cache output under the scratch
directory so imports do not create files in the reference tree.

Before the full run, save the observed environment into the scratch run record:

```bash
/Users/nicknassuphis/pyroots/.venv/bin/pip freeze \
  > /private/tmp/pyroots-giga-2902/environment-observed.txt
```

No historical lock file exists, so exact NumPy/LAPACK behavior from the original
machine cannot be proven.

## 9. Phase A: protect and inspect the reference

1. Recompute the three reference SHA-256 hashes and compare them with section 2.
2. Confirm that `git status --short` in the reference repository is empty.
3. Record reference file dimensions with `file` or `vipsheader`.
4. Do not run any Python module from the reference checkout. Even an import can
   create `__pycache__` unless bytecode writing is disabled.
5. Copy `giga_2902_polyfun.txt` into the disposable clone before executing or
   importing it. The driver must read the scratch copy, not the reference path.
6. Choose a unique output name such as
   `recreate_giga_2902_YYYYMMDD_HHMMSS`; fail if it already exists.

## 10. Phase B: smoke-test the formula

Run a low-cost test in the disposable clone before building an exact-view driver:

```bash
PYTHONDONTWRITEBYTECODE=1 \
MPLCONFIGDIR=/private/tmp/pyroots-giga-2902/.mplconfig \
/Users/nicknassuphis/pyroots/.venv/bin/python pyroots.py \
  smoke_giga_2902 \
  --mode write \
  --roots 0.1 \
  --procs 2 \
  --samples 3300 \
  --alpha 0 \
  --margin 0.05 \
  --res 1000 \
  --view full \
  --xfrm none \
  --poly poly_chess5 \
  --zfrm sort_abs_p \
  --solve solve \
  --setf a=0.7,andy=0.01,phi=0.0,rho=0.1 \
  --sets rloc=rjail3 \
  --png invert \
  --rotate 90
```

Acceptance checks:

- Initialization derives degree 33.
- `rjail3` resolves to 33 `S` and 33 `T` coordinates.
- No solver/import exception occurs.
- A 1000 x 1000 grayscale PNG is produced under the scratch clone.
- The smoke image is only a pipeline check. Its sparse appearance and randomly
  estimated view are not grounds for rejecting the configuration.

## 11. Phase C: fixed-view reconstruction driver

The full reconstruction must not call `polyinit.setup()` or
`polyview.estimate_root_range()`. Instead, add a small runner only inside the
disposable clone with this behavior:

1. Execute the scratch copy of `giga_2902_polyfun.txt` as a Python data file and
   obtain its `poly`, `view`, `data`, `job`, and `png` dictionaries. Do not
   execute the file in the reference directory.
2. Load those dictionaries into `polystate`.
3. Replace only `data.stem` with the unique scratch output stem and set
   `data.mode = "write"`.
4. Preserve the exact saved `view.view` tuple. Do not recalculate it.
5. Recompute and assert:

   ```text
   degree == 33
   procs == 14
   chunk == 4,329,004
   roots == 2,000,000,000
   view.res == 50,000
   ```

6. Call `polystate.update_flow()` and assert the four selected callable names.
7. Save a scratch copy of the effective state. This is needed because historical
   workers reload `<stem>/<stem>_polyfun.txt` when they start.
8. Call the histogram worker directly, bypassing `pyroots.main()` and
   `polyinit.setup()`.
9. Apply the historical output operations explicitly: threshold, rotate 90,
   invert, and write the full image.
10. Generate thumbnails using the verified reference polarity contract in
    section 2 rather than trusting the historical suffix convention.

The driver must print the effective state and abort before allocating the 2.5 GB
histogram unless every assertion passes.

### Single-run mode

The closest execution-path reproduction uses one shared histogram and the saved
14-process layout. It reproduces the original architecture but retains its
nondeterministic seeds, shared-write races, uint8 wrap behavior, and lack of
checkpointing.

Use this only after the smoke test and only if matching the historical mechanics
is more important than restartability.

### Recommended sharded mode

The intended output is binary occupancy, so independent occupancy maps can be
combined with a bitwise OR without changing the mathematical union of visited
pixels.

Use 14 shards corresponding to the 14 original worker streams:

```text
4,329,004 polynomial samples per shard
33 roots per sample
142,857,132 roots per shard
14 shards
1,999,999,848 roots total
```

For each shard:

1. Use the exact saved view.
2. Use a recorded deterministic seed unique to the shard.
3. Build a 50K binary occupancy image without rotation or inversion.
4. Write to a unique scratch key such as `occupancy_00.png` through
   `occupancy_13.png`.
5. Write a completion marker containing seed, sample count, dimensions, and
   SHA-256.

After all shards are complete:

1. Stream them through libvips and combine with bitwise OR.
2. Rotate the combined occupancy image by 90 degrees.
3. Invert it to get the light-background full image.
4. Write the final 50K grayscale PNG.
5. Generate the two thumbnails as described below.

This avoids the original cross-process write race and permits restart from a
failed shard. It is not byte-identical to the historical implementation, but it
is a more robust realization of the operation that implementation intended.
Run shards sequentially or with limited concurrency: every concurrent shard
requires its own 2.5 GB occupancy map.

## 12. Phase D: thumbnail generation

Generate the thumbnails from the completed full image with libvips:

```text
light thumbnail = resize(full, 1000 / 50000)
dark thumbnail  = invert(light thumbnail)
```

Name them to match the observed reference, not the intuitive suffix meaning:

```text
<stem>_sml_inv.png = light thumbnail, same polarity as full
<stem>_sml.png     = dark thumbnail
```

The reference was generated with libvips-style resize behavior. Avoid Pillow for
this step because a different resampling implementation can visibly alter the
dense gray structure of the 1000px image.

## 13. Validation

### Structural checks

- Effective state matches the saved function, transform, solver, and numeric
  parameters.
- Saved view bounds match bit-for-bit as Python floats.
- Full output is 50000 x 50000, one-band, 8-bit grayscale.
- Thumbnails are 1000 x 1000, one-band, 8-bit grayscale.
- Full and `_sml_inv` have a light corner; `_sml` has a dark corner.
- Actual generated root count is recorded as 1,999,999,848.

### Visual checks

Use `giga_2902_sml_inv.png` as the primary light-background reference. Compare:

- overall centered fan/crown silhouette;
- outer petal positions;
- central arch and lower vertical stem;
- left/right symmetry;
- saved-view framing and margins;
- polarity and 90-degree orientation.

An overlay or difference image is more useful than a raw SHA comparison. At two
billion roots, the large-scale occupancy should be stable even though individual
pixels differ.

### Hash policy

The hashes in section 2 protect the reference files from accidental mutation.
They are not acceptance hashes for a rerender. Exact equality is impossible to
promise because the original worker seeds were not saved and the histogram
updates were nondeterministic.

## 14. Failure handling

- Never resume by writing into an existing reference or completed output stem.
- A single-run crash requires restarting the single run; there is no persisted
  histogram checkpoint.
- A sharded crash should delete or replace only that shard's scratch output after
  verifying its completion marker is absent or invalid.
- Keep the raw occupancy shards until the final image and both thumbnails pass
  validation.
- Capture stdout/stderr, environment, commit hash, seeds, and effective state in
  the scratch run directory.
- Do not clean shared-memory objects or kill processes by broad name. Record PIDs
  for this run and operate only on those PIDs if cleanup is required.

## 15. Implementation order

1. Recheck reference hashes and confirm the reference Git tree remains clean.
2. Create the disposable clone at `fb854a4`.
3. Verify imports using the observed Python 3.13 environment with bytecode writes
   disabled.
4. Run the 1000px/100K-root smoke test.
5. Implement the scratch-only fixed-view driver.
6. Run the fixed-view driver at 1000px and a small root count.
7. Confirm orientation and polarity against `giga_2902_sml_inv.png`.
8. Implement and test two tiny deterministic shards plus their OR merge.
9. Select single-run or recommended 14-shard execution based on available RAM
   and restart requirements.
10. Run the full calculation, retaining logs and shard manifests.
11. Build the full light-background image and both thumbnails.
12. Perform structural and visual validation.
13. Archive the scratch run record; leave the reference tree untouched.

## 16. Historical uncertainties that do not block reconstruction

- The exact original clock/PID-derived worker seeds were not stored.
- The target's internal `data.stem` says `giga_2869` even though the files are
  named `giga_2902`; the repository contains no committed publish command that
  explains the rename.
- The full image and thumbnail polarity naming do not match the historical
  `polypng.py` naming order, indicating a later thumbnail/full-image postprocess.
- There is no historical dependency lock, so the exact original NumPy, BLAS, and
  libvips builds cannot be established solely from the repository.

None of these prevents a faithful reconstruction of the formula, saved view,
root count, orientation, polarity, and visual composition. They prevent only a
claim of byte-identical reproduction.

## 17. Polypaint Coeff Program implementation plan

### 17.0 Implementation status

Implemented as an ordinary saved Coeff Program. The only application changes
are the generic `vector_literal(...)`, `translate_roots(...)`, and
`bimodal(u,a)` language primitives. The generated
`giga_2902.coeff-program.json` document is saved through the existing
`/save-coeff-program` route and appears in the existing Coeff Programs list.
There is no built-in preset, special route, solver override, or combined
coeffgen/solve mode.

The execution path is the standard path used by every Coeff Program:

```text
Coeff Program VM
  -> ordinary interleaved complex-float32 coefficient artifact
  -> selected existing solver (AE-MT or CM)
  -> ordinary interleaved complex-float32 root artifact
```

### 17.1 Verdict

This polynomial family can be implemented cleanly as a Coeff Program, but the
current language is missing two generic operations:

1. a compiled constant-vector pool, so a program can reuse fixed coefficient
   vectors without rebuilding or spelling them as per-row `poke` operations;
2. a coefficient-domain root translation, so a fixed polynomial `P(z)` can be
   changed to `P(z - delta)` without solving for roots and running a
   root-to-coefficient construction on every row.

The existing VM already supplies the remaining essential operations:

- scalar expressions over `t1`, `t2`, `p1`, `p2`, `pi`, `pi2`, and `pi2i`;
- two-vector blending;
- complex vector arithmetic and scalar broadcasting;
- `sort_abs` / `argsort` behavior;
- a fixed output-length contract up to 256 coefficients.

An optional third addition, `bimodal(u, a)`, is recommended because it expresses
the historical blend distribution directly and avoids duplicating a long,
error-prone `floor`/`log`/`exp` expression. It is not mathematically required.

Do not add a bespoke `poly_chess5` C coefficient function, a `rjail3` VM opcode,
or a per-row `poly_from_roots` primitive. Those approaches hardcode one artwork
or repeat work that is constant across every sample.

### 17.2 What is constant and what varies

The user's observation is correct, with one important distinction. The
coefficients returned by the old `numpy.poly(...)` calls are not literally
constant because the roots are shifted on every sample. The *unshifted* source
and target polynomials are constant, and every shifted polynomial can be
derived from those constants.

Let the fixed `rjail3` roots be:

```text
s_j = source_x[j] + i*source_y[j]
t_j = target_x[j] + i*target_y[j]
```

Precompute exactly once:

```text
S(z) = product_j (z - s_j)
T(z) = product_j (z - t_j)
```

`S` and `T` are two leading-first, monic, 34-element complex coefficient
vectors. They belong in the compiled program's immutable constant pool. They
must not be reconstructed from the 33 explicit roots in the browser, in the
Lambda handler, or in the per-row VM evaluator.

For one sample, the historical code computes:

```text
dS = rho * exp(pi2i*t1)
dT = rho * exp(pi2i*(speed*t1 + phi))
DeltaS = (1+i) * dS
DeltaT = (1+i) * dT
```

The actual source and target polynomials are therefore:

```text
S_shifted(z) = S(z - DeltaS)
T_shifted(z) = T(z - DeltaT)
```

Only the translation and blend weight vary per row. No root-to-coefficient
construction is needed at runtime.

For the saved parameters, `speed=1` and `phi=0`, hence
`DeltaS == DeltaT == Delta`. Polynomial translation is linear, so the work can
be reordered exactly:

```text
(1-b)*S(z-Delta) + b*T(z-Delta)
    == translate_roots((1-b)*S + b*T, Delta)
```

The canonical `giga_2902` saved program should consequently perform one vector blend
and one translation per sample. It must not translate both vectors separately.
The generic primitive must still support separate translations for future
programs where `speed` or `phi` differs.

### 17.3 Translation semantics and algorithm

Add a public Coeff Program operation with this contract:

```text
translate_roots(coefficients, delta)
```

If `coefficients` represents the leading-first polynomial `P(z)`, the result is
the leading-first coefficient vector for:

```text
Q(z) = P(z - delta)
```

Every root of `P` is therefore moved by `+delta`. The output length is exactly
the input length. The operation rejects an empty vector, a non-finite `delta`,
or a vector longer than `MAX_VECTOR_LEN`.

For leading-first coefficients `a[0..n]`, the exact coefficient relation is:

```text
q[j] = sum(k=0..j) a[k] * C(n-k, j-k) * (-delta)^(j-k)
```

Implement this in `O(n^2)`. Prepare the required binomial coefficients once per
encountered vector length, either at program load or in a workspace cache:

```text
choose[p,0] = choose[p,p] = 1
choose[p,m] = choose[p-1,m-1] + choose[p-1,m]
```

For each row, compute `powers[m] = (-delta)^m` once by recurrence, then evaluate
the triangular sum above. This avoids per-row factorials, divisions, repeated
complex `pow` calls, and root solving. Degree 33 requires 34 powers and 595
triangular complex contributions, which is negligible beside a degree-33
eigensolve. The binomial cache is immutable after construction and must not be
rebuilt for every row.

The operation should be a structural typed-vector opcode, not a legacy
registry transform. Append new wire values; never renumber an existing opcode:

```text
COEFF_OP_PUSH_VECTOR_CONST = 48
COEFF_OP_TRANSLATE_ROOTS   = 49
```

Values `35..44` are already occupied by later Coeff/Param VM work. The final
implementation therefore uses the explicit Coeff-extension range `48..49` in
the merged opcode registry and moves the reserved range to `50..63`.

`COEFF_OP_TRANSLATE_ROOTS` should consume a vector and a complex scalar from the
typed stack and push one vector. That makes the source form composable and
keeps the dynamic `delta` in the existing scalar-expression machinery.

### 17.4 Compiled vector constants

#### Source form

Add a compile-time-only source form such as:

```text
vector_literal(c0, c1, ..., cn)
```

Each element must be a static, finite complex expression. Dynamic references
such as `t1`, `p1`, `cf[k]`, or `poly[k]` are invalid in a vector literal.

The existing `MAX_ARGS=8` limit is a runtime-token limit. A vector literal must
be parsed before that generic check and use its own `MAX_VECTOR_LEN=256` limit.
The compiler interns the vector and emits a one-argument
`COEFF_OP_PUSH_VECTOR_CONST` token containing only the pool index. It must not
emit 34 numeric token arguments.

Long literal source may remain one generated line in v1. If multiline call
parsing is added for readability, it must be a general source-parser feature,
not a `giga_2902` exception.

#### Compiled payload

Add `vector_constants` to the compiled Coeff Program payload. Use an explicit,
bounded representation such as:

```json
{
  "vector_constants": [
    {"length": 34, "values": [re0, im0, re1, im1]}
  ]
}
```

The shown `values` array is abbreviated. Its actual length must be exactly
`2*length`.

Required invariants:

- all numbers are finite binary64 values;
- `1 <= length <= 256`;
- a small pool-count cap, initially 8 vectors;
- a total pool cap, initially 1,024 complex elements;
- byte-for-byte deduplication after canonical zero normalization;
- constants are included in `execution_spec` and the fingerprint;
- a pool index is validated at C program-load time;
- the C program owns the decoded constants for the invocation;
- evaluation copies an already-decoded vector to the stack and does no parsing,
  polynomial construction, or allocation.

The pool cap is independent of the existing token, stack, and scalar-expression
caps. Preserve the handler's overall request/plan-size guard as a second line of
defense.

#### Persistence and v2 translation

Thread `vector_constants` through every payload boundary, not only the compiler:

- `coeff_program_chain.py` result payload and canonical execution spec;
- `program_compile_helpers.compiled_coeff_program_payload`;
- compute-plan, preview, coeffgen, and param-debug forwarding paths;
- saved-program storage and source/chain round trips;
- `program_v2_translate._execution_spec_v2` and translated payloads;
- PDF/Populate reconstruction;
- native JSON parsing in `sweep_cli.c`;
- deploy packaging and generated-vocabulary checks.

Source text remains authoritative. Recompiling the generated source must rebuild
the same pool and fingerprint. Do not store a pool index that depends on a
mutable external file or S3 object.

### 17.5 Historical bimodal blend

The target weight is:

```text
b = bimodal(t2, 0.7)
```

with:

```text
if u < 0.5:
    b = (2*u)^(1/(1-a)) / 2
else:
    b = 1 - (2*(1-u))^(1/(1-a)) / 2
```

The current scalar VM can spell this with `floor`, `log`, and `exp`, because the
exponent `10/3` is not an integer literal. That is a poor public program: it is
long, duplicates branch logic, and is easy to get wrong at `u=0`, `u=0.5`, or
`u=1`.

Recommended implementation:

- append `COEFF_EXPR_BIMODAL` to the scalar-expression opcode enum;
- expose `bimodal(u, a)` in the parser and generated Help;
- require finite real inputs, `0 <= u <= 1`, and `0 <= a < 1`;
- clamp the result to `[0,1]`, matching the old implementation;
- implement and parity-test the same formula in Python folding and the C VM;
- preserve all existing scalar-expression opcode numbers.

This helper is generic distribution math. It should not know about `rjail3` or
`poly_chess5`.

### 17.6 Canonical saved program

After the new primitives exist, the saved case should compile to the conceptual
program below. The generator fills in all 34 values of `S` and `T`.

```text
# Push the two immutable, unshifted degree-33 polynomials.
vector_literal(S0, S1, ..., S33)
vector_literal(T0, T1, ..., T33)

# Exact historical coefficient interpolation.
poly = blend(bimodal(t2, 0.7))

# speed=1 and phi=0 make the two root translations identical, so translate
# the blended polynomial once.
poly = translate_roots(poly, (1+i) * 0.1 * exp(pi2i*t1))

# sort_abs_p: C + 0.01*C[argsort(abs(C))]. Keep the original on the stack
# while sorting/scaling the mutable poly register.
poly
poly = sort_abs(poly)
poly = multiply(poly, 0.01)
poly = add(pop, poly)
```

The final parser spelling should be proved by a source parse/compile/round-trip
test. The comments above describe the required semantics, not permission to
special-case this source in the parser.

For a general chess-blend program with unequal shifts, the generated source is:

```text
vector_literal(S0, ..., S33)
poly = translate_roots(pop, DeltaS)
poly
vector_literal(T0, ..., T33)
poly = translate_roots(pop, DeltaT)
poly
poly = blend(b)
```

The exact stack spelling may be normalized by the existing source decompiler,
but it must translate each constant only once and blend the two translated
vectors.

### 17.7 Sampling in Polypaint

Use raw grid coordinates for the two independent uniforms:

```text
t1 = circular root-translation phase
t2 = bimodal blend uniform
```

This is a deterministic stratified square rather than the historical sequence
of worker-local pseudorandom draws. It samples the same two-dimensional
distribution and is preferable for a reproducible app program. The first two
discarded historical random draws do not affect the distribution and should not
be emulated.

With degree 33 and `times=1`:

```text
N=7785 -> 7,785^2 * 33 = 2,000,005,425 roots
N=778  ->   778^2 * 33 =    19,974,372 roots
```

Thus `N=7785` is the app analogue of the two-billion-root original, and `N=778`
is close to the 20-million-root 5K standalone test. Start much smaller for UI
and solver validation.

No RNG primitive is required for v1. If future users need independent random
passes for `times > 1`, add a deterministic scalar RNG keyed by the existing
evaluation seed and an explicit stream id. Never use process-global `rand()` or
host RNG state in a program fingerprinted for caching.

### 17.8 Precision and solver contract

The normal Polypaint coefficient artifact is interleaved complex binary32. The
Coeff Program VM evaluates a row in binary64, then coeffgen casts that row once
when it writes the standard artifact:

```text
sweep_cli.c runCoeffGen          -> float stepBuf
sweep_cli.c coeffGenWorkerMain   -> float outBlock
logical_sections.coeff_row_bytes -> 2 * 4 bytes per coefficient
solve-from-coefficients          -> reads float, promotes back to double
```

Both existing solvers read that same artifact, widen each coefficient to double,
and perform their solver arithmetic in double. CM is not a special binary64
coefficient path, and AE-MT is not a float32 arithmetic solver. Their common
input storage format is float32; their internal arithmetic is double.

The base coefficients fit inside the binary32 exponent range, but degree-33
root recovery is ill-conditioned. A local comparison found:

| Check | Result |
|---|---:|
| maximum base-source coefficient magnitude | about `1.27e19` |
| maximum base-target coefficient magnitude | about `1.35e22` |
| translation-vs-direct-`numpy.poly` relative coefficient error in binary64 | about `1.5e-15` to `5e-15` |
| coarse 100x100 density correlation after binary32 coefficient transport | about `0.91` |
| occupied-cell Jaccard at 500x500 | about `0.75` |

The float32 boundary can visibly change this ill-conditioned degree-33 family,
but that variation is acceptable for this art workflow. It is part of the
existing architecture and is not a reason to add another execution path. A
future coefficient-format migration would be a separate, system-wide design;
it must not be hidden inside this one program.

The saved program is solver-independent. The user may run it through AE-MT or
CM exactly as with any other Coeff Program. CM is useful when comparing with the
historical `numpy.roots` implementation; AE-MT is valid when its visual result
or performance is preferable.

### 17.9 Program generation and ownership

The deterministic repository script is:

```text
scripts/gen_giga_2902_coeff_program.py
```

It:

1. contain or read a checked-in canonical `rjail3` layout;
2. derive the 33 source and 33 target complex coordinates;
3. expand each unshifted root product once with exact rational arithmetic,
   converting only the final coefficients to binary64;
4. normalize signed zero and serialize every binary64 component with a
   round-trippable representation;
5. emits `giga_2902.coeff-program.json` in the ordinary portable Coeff Program
   upload shape;
6. exposes the layout hash to tests without adding nonstandard fields to the
   saved-program document;
7. support `--check` and fail when checked-in output is stale.

Generation is a build/development operation, never a browser or compute-row
operation. The generated program remains self-contained because the constants
are in its source and compiled pool; production does not read
`/Users/nicknassuphis/pyroots`.

Validation performed by the generator must include:

- exactly 33 `S` and 33 `T` cells;
- two coefficient vectors of length 34;
- leading coefficient exactly `1+0i`;
- finite real and imaginary components;
- `numpy.roots(base_coefficients)` matching the originating root set as an
  unordered set within a documented tolerance;
- stable generated-file hashes under a second `--check` run.

Save the document through the existing `/save-coeff-program` route. That route
compiles it, writes the canonical object and metadata at
`polypaint/coeff-programs/giga-2902.json`, and makes it appear in the unchanged
Coeff Programs modal. Do not add it to the coefficient registry, generated
vocabulary, Starter panel, deploy assets, or a special frontend list.

Suggested compute settings are operational guidance, not program metadata:

```text
base coefficient function = const
degree                    = 33
solver                    = aberth_mt or companion_matrix
times                     = 1
Coeff Program             = generated giga_2902 program
```

The saved viewport, 90-degree rotation, and light/dark polarity are render
settings, not coefficient-program semantics. Loading the ordinary saved program
must not mutate any of them or silently change the selected solver.

### 17.10 File-level implementation map

The implementation should touch these areas deliberately:

- `lambda/structural_chips.json`: append the two structural opcodes and public
  Help metadata;
- `lambda/coeff_program_source.py`: vector-literal grammar, static validation,
  typed lowering for `translate_roots`, and source reconstruction;
- `lambda/coeff_program_chain.py`: pool interning, limits, stack validation,
  execution-spec/fingerprint inclusion, and appended opcode names;
- `lambda/program_compile_helpers.py`: forward `vector_constants`;
- `lambda/program_v2_translate.py`: preserve constants in v2 tokens, payload,
  and execution spec;
- `lambda/sweep_cli.c`: append opcode/parser definitions, decode the constant
  pool once, execute constant pushes and translations, and add telemetry;
- `lambda/gen_coeff_vocab.py` and `coeff_vocab_js.js`: generated Help exposure;
- existing compute-plan/preview/fused handlers: remain on their normal paths;
  the generic compiled-program payload merely gains `vector_constants`;
- `scripts/predeploy_check.sh`: gate the generator check and all new tests;
- native build/deploy manifests: rebuild every binary embedding the Coeff VM.

The `bimodal` Python parser/folder, C scalar-expression enum, C evaluator,
generated Help, and opcode drift tests land together.

### 17.11 Phased implementation

#### Phase 0: freeze an independent oracle

Before changing the VM, generate a small deterministic corpus from
`make_giga_2902.py` or a dependency-light extraction of its math. For fixed
`(t1,t2)` pairs, store:

- unshifted `S` and `T` vectors;
- `Delta`, blend weight, and blended vector;
- translated vector before `sort_abs_p`;
- final coefficient vector;
- companion roots as an unordered set;
- a small occupancy image or histogram digest.

The oracle must compute directly from shifted roots with NumPy. It must not call
the new VM translation helper, or a shared bug would make the comparison hollow.

#### Phase 1: constant-vector wire support

Implement source parsing, pool interning, payload propagation, native parsing,
and `COEFF_OP_PUSH_VECTOR_CONST`. Keep all current Coeff Program fingerprints
unchanged when the new pool is empty. Gate source, storage, v2 migration, C
parser, and malformed-payload behavior before proceeding.

#### Phase 2: root translation

Implement `COEFF_OP_TRANSLATE_ROOTS` in Python validation and C execution. Test
degrees 1, 2, and 33; zero, real, imaginary, and general complex shifts; monic
and non-monic vectors; and failure cases. Compare with an independent
`numpy.poly(numpy.roots(P) + delta)` oracle where conditioning permits, plus
direct coefficient-form reference arithmetic.

#### Phase 3: bimodal scalar helper

Add and parity-test `bimodal(u,a)`. Test both branches, the midpoint, endpoints,
invalid `a`, non-finite inputs, static folding, dynamic C evaluation, and exact
source round trips.

#### Phase 4: ordinary saved program

Generate the two base vectors and canonical source as
`giga_2902.coeff-program.json`. Compile it in tests and compare its VM output at
fixed grid points against the Phase-0 shifted-root oracle. Save that document
through `/save-coeff-program`; do not add any app-specific loading code.

#### Phase 5: normal-pipeline regression

Prove that the program follows ordinary coeffgen and then the selected existing
solver in preview, lores recompute, staged chunks, and fused chunks. Exercise
both AE-MT and CM. Assert that no solver restriction, direct-solve flag, or
combined native mode is introduced.

#### Phase 6: end-to-end rendering

Run, in order:

1. a handful of fixed rows through compute-debug;
2. a very small `N` root scatter;
3. a 500px occupancy comparison;
4. the `N=778`, 5K-class reproduction;
5. only after those pass, consider the `N=7785`, two-billion-root job.

Apply the exact saved view and orientation from section 3 during render
validation. Record solver failures, non-finite rows, coefficient maxima, and
occupied-pixel statistics.

### 17.12 Required tests and gates

At minimum, add or extend tests for:

- static-only vector literals, finite values, length and total-pool limits;
- pool deduplication and canonical signed zero;
- pool inclusion in v1/v2 fingerprints and saved-program round trips;
- unchanged fingerprints for every existing oracle program with an empty pool;
- native rejection of missing pools, bad indexes, wrong lengths, NaN/Inf, and
  oversized payloads;
- exact constant push values in binary64;
- translation parity and output-length preservation;
- scalar `bimodal` Python/C parity;
- deterministic `sort_abs_p` tie behavior, plus NumPy parity on oracle rows
  whose coefficient magnitudes are distinct;
- canonical saved-program coefficient parity before the binary32 boundary;
- the exact portable document round-tripping through the normal
  save/list/fetch S3 contract;
- AE-MT and CM both consuming the ordinary float32 coefficient artifact;
- frontend Help for the generic primitives, with no hard-wired `giga_2902`
  Starter or catalog entry;
- deploy packaging of structural metadata and rebuilt binaries.

The relevant existing gates include `test_coeff_program_chain.py`,
`test_coeff_program_native.py`, `test_coeff_program_drift.py`,
`test_coeff_source_equivalence.py`, program-v2 migration/oracle tests, frontend
JS tests, and deploy packaging tests. Add a focused
`test_giga_2902_coeff_program.py` for the independent fixture and ordinary
portable program.

### 17.13 Acceptance criteria

The feature is complete when:

1. the two degree-33 base coefficient vectors are generated once and loaded as
   immutable program constants;
2. no production row reconstructs either polynomial from explicit roots;
3. the saved program blends first and performs exactly one root translation;
4. fixed-row binary64 coefficients match the independent historical formula;
5. the normal float32 coefficient artifact is accepted by both AE-MT and CM;
6. existing Coeff Program wire values and fingerprints remain unchanged;
7. the 5K app render reproduces the standalone image's composition, viewport,
   orientation, and principal structures;
8. all new generated assets and native binaries are predeploy-gated.

The shortest robust route is therefore: **precompute two base vectors once,
save one ordinary Coeff Program, blend them per row, translate the blend in
coefficient space, apply the existing `sort_abs_p` construction, and let the
normal selected solver consume the normal coefficient artifact.**
