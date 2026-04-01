**Float64 Plan**

Status: historical numeric-design note. Some examples and field names still reference older stripe-era metadata such as `stripe_keys`.

`Solve64` is feasible, but it should not be implemented as a hand-maintained second solver.

The current solver math is already mostly `double` in [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c) and [lambda/solver_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solver_cli.c). The main precision loss happens at the serialized boundaries:

- coeffgen writes `float32` coefficient files in [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2808)
- solve reads `float32` coefficient files in [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2908)
- solve writes `float32` root stripe files in [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2909)
- param dump also writes `float32` in [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2733)
- render readers assume `float32` roots in [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c#L383) and [lambda/bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_raster.c#L119)

So `Solve64` should be thought of as:

- preserve `float64` through coefficient and root files
- keep the same solver logic
- compile the same source in two IO modes

**Recommendation**

Do not fork the solver into two separate codebases.

Use one source tree with a small precision abstraction for serialized data, then build two executables:

- `sweep32`
- `sweep64`

This can be done with a typedef or macro layer, for example:

- `pp_io_t`
- `PP_IO_SIZE`
- `PP_IO_TAG`

The internal iterative math should remain `double`. Only the external file representation should vary.

That gives:

- one implementation of coeffgen
- one implementation of solve
- one implementation of param dump
- two builds with different on-disk formats

**What Solve64 Really Buys**

The biggest likely gain is not from changing Newton / EA iteration precision. That is already `double`.

The gain is from avoiding early quantization:

- coefficient functions produce `double`
- param transforms run in `double`
- coefficient transforms run in `double`
- today those values are truncated to `float32` before the solve stage ever sees them

So the highest-value first step is:

- `Coeffgen64` writes `float64` coefficient bins
- `Solve64` reads `float64` coefficient bins

That alone may materially improve difficult polynomials.

Full end-to-end `root64` rendering support is a second step.

**Pipeline Options**

There are two realistic scopes.

Option A: `coeff64 -> solve64 -> root32`

- coefficients stored as `float64`
- solver consumes `float64`
- roots written back out as `float32`
- render pipeline unchanged

Pros:

- biggest likely numerical win for the least surface area
- no render changes
- existing viewport / raster binaries continue to work

Cons:

- final stored roots are still quantized to `float32`
- render-stage transforms and pixelization still see `float32` roots

Option B: `coeff64 -> solve64 -> root64`

- coefficients stored as `float64`
- solver consumes `float64`
- roots written as `float64`
- render readers gain `float64` support

Pros:

- true end-to-end higher-precision path
- best option for extremely tricky cases

Cons:

- larger change surface
- all root readers need precision awareness
- more storage and IO cost

Recommended rollout:

1. implement Option A first
2. only add Option B if the remaining failure mode is clearly root-file quantization rather than coefficient quantization

**Build Strategy**

The clean build shape is:

- compile `sweep32` with `-DPP_IO_F32`
- compile `sweep64` with `-DPP_IO_F64`

Inside `sweep_cli.c`, isolate only the serialization layer:

- file read buffers
- file write buffers
- `fread` / `fwrite` element size
- metadata reporting of `data_bytes`

Keep:

- coefficient generation math in `double`
- root solve math in `double`
- matching in `double`

Do not macro-ize the whole solver. That would create unnecessary complexity.

**Frontend / UX**

The Compute section should expose:

- `Solve32`
- `Solve64`

And, if coeffgen remains a separate explicit step:

- `Coeffgen32`
- `Coeffgen64`

But the cleaner user model is:

- `Solve32` means the whole coeffgen+solve pipeline uses 32-bit intermediates
- `Solve64` means the whole coeffgen+solve pipeline uses 64-bit intermediates

So the UI contract should carry a single precision field, for example:

- `precision: "f32"`
- `precision: "f64"`

**Metadata**

Every compute job should record precision in [calc.json](/Users/nicknassuphis/karpo_hackathon/polypaint/calc.json) metadata written to S3, for example:

- `precision`
- `coeff_precision`
- `root_precision`

Minimum recommended field:

- `precision: "f32" | "f64"`

And derive filenames or readers from that.

Also record:

- `coeffs_keys`
- `stripe_keys`
- `n_coeffs`
- `degree`

so downstream consumers do not guess.

**File Naming**

Do not overload the existing names without metadata.

Use one of these two approaches:

1. same filenames, precision carried in metadata
2. precision-tagged filenames

Examples:

- `coeffs_0003.bin` plus `precision=f64` in `calc.json`
- `coeffs64_0003.bin`

I recommend metadata-first, not filename-first, because it keeps the object layout cleaner. The readers should use job metadata, not string parsing.

**Code Areas To Change**

Compute:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)
- [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py)
- [lambda/handler_sweep.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_sweep.py)
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)

Potential render follow-on for full root64:

- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
- [lambda/bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_raster.c)
- [lambda/handler_viewport.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_viewport.py)

Also review:

- coeff render path
- lores coefficient viewport path
- param dump path if it is expected to mirror precision modes

**Storage / Cost Impact**

If coefficients and roots move from `float32` to `float64`, the raw binary size roughly doubles.

That means:

- more S3 storage
- more Lambda download time
- more `/tmp` usage
- more network transfer

So `Solve64` should remain an explicit premium path, not the new default.

That matches the intended UX:

- use `Solve32` for normal work
- use `Solve64` when the polynomial is numerically difficult enough to justify the cost

**Implementation Phases**

1. Add precision metadata and frontend selection.
2. Build `sweep32` and `sweep64` from the same source.
3. Change coeffgen and solve handlers to dispatch the correct binary from `precision`.
4. Implement Option A first:
   - coeff bins in `f64`
   - solver input in `f64`
   - root stripes still written as `f32`
5. Validate whether this already fixes the target class of problems.
6. Only then consider Option B:
   - root stripes in `f64`
   - render readers become precision-aware

**Validation**

Tests should compare:

- identical jobs through `Solve32` and `Solve64`
- output file sizes and metadata
- coefficient round-trip precision
- solver stability on numerically sensitive examples
- whether `Solve64` changes root continuity or convergence failures on known hard cases

For Option B, also test:

- viewport from `root64`
- color render from `root64`
- bilevel render from `root64`
- root transform path from `root64`

**Bottom Line**

This is not a second solver.

It is one solver with two intermediate-data precisions.

The right architecture is:

- one codebase
- two builds
- explicit precision metadata
- first improve coefficient precision
- only then decide whether root-file precision also needs to become `float64`
