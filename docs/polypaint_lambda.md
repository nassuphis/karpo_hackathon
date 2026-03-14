---
title: "PolyPaint Lambda: Server-Side Render System"
author: ""
date: "March 2026"
geometry: margin=1in
fontsize: 11pt
header-includes:
  - \usepackage{booktabs}
  - \usepackage{longtable}
  - \usepackage{fancyhdr}
  - \pagestyle{fancy}
  - \fancyhead[L]{PolyPaint Lambda}
  - \fancyhead[R]{\thepage}
  - \fancyfoot{}
  - \usepackage{enumitem}
  - \setlist{nosep}
---

# Overview

PolyPaint Lambda renders high-resolution images of polynomial root loci by scanning a 2D parameter grid, solving for roots at each grid point, and plotting them as colored pixels with additive blending. The system runs on AWS Lambda (ARM64 Graviton) and produces JPEG or PNG images stored in S3.

The pipeline has three stages:

1. **C binary** (`sweep_cli.c`) -- evaluates parametric polynomials on a grid, solves for roots via the Ehrlich-Aberth method, and writes a raw RGB pixel buffer.
2. **Python handler** (`handler.py`) -- orchestrates the C binary, encodes the RGB buffer to JPEG/PNG via Pillow, uploads to S3, and returns a presigned URL.
3. **Browser frontend** (`index.html`) -- provides a Render tab with controls for function, grid size, image resolution, format, and stripe parallelism.

For large grids, the handler fans out computation across multiple Lambda invocations ("stripes"), each processing a subset of grid rows in parallel. The stripe results are merged by additive compositing before final encoding.

# Architecture

```
Browser                    API Gateway (30s)         Lambda (900s)
+----------------+        +----------------+        +------------------+
| Render Tab     |--POST->| /render        |------->| handler.py       |
| n1,n2,W,H,fmt |        |                |        |   handle_render()|
| function,      |        +----------------+        |     |            |
| stripes        |                                  |     v            |
+----------------+                                  | Single-pass:     |
       ^                                            |   sweep(render)  |
       |                                            |   -> raw RGB     |
       |                                            |   -> Pillow enc  |
  presigned URL                                     |   -> S3 upload   |
  + metadata                                        |                  |
                                                    | Striped:         |
                                                    |   pre-pass(50x50)|
                                                    |   fan-out N      |
                                                    |   Lambdas        |
                                                    |   merge PNGs     |
                                                    |   encode + S3    |
                                                    +------------------+
```

# Coefficient Functions

Each coefficient function maps a pair of parameters $(x_1, x_2) \in [0,1)^2$ to a vector of complex polynomial coefficients. The parameters are lifted to the unit circle: $t_k = e^{2\pi i \, x_k}$.

## giga\_5 (degree 25)

26 coefficients. Fixed real entries at indices 0, 4, 12, 19, 20, 24 (0-based). Three dynamic entries computed from complex powers of $t_1$ and $t_2$:

- $c_6 = 100i \cdot (t_2^3 + t_2^2 - t_2 - 1)$
- $c_8 = 100i \cdot (t_1^3 + t_1^2 + t_2 - 1)$
- $c_{14} = 100i \cdot (t_2^3 - t_2^2 + t_2 - 1)$

## giga\_42 (degree 49)

50 coefficients. Fixed real entries at indices 0, 7, 15, 31, 39. Four dynamic entries using transcendental functions of complex arguments:

- $c_{11} = 100i \cdot \exp(t_1^2 + t_2^2)$ --- complex exponential of sum of squares
- $c_{19} = 50 \cdot (t_1^3 + t_2^3)$
- $c_{24} = \exp(i(t_1 - t_2)) + 10 \, t_1^2$
- $c_{44} = 200 \sin(t_1 + t_2) + i \cos(t_1 - t_2)$ --- complex $\sin$ and $\cos$

The complex sine and cosine are computed via the identities $\sin(a+bi) = \sin a \cosh b + i \cos a \sinh b$ and $\cos(a+bi) = \cos a \cosh b - i \sin a \sinh b$.

## Reversed variants

`rev_giga_5` and `rev_giga_42` reverse the coefficient order: $c_k \to c_{n-1-k}$. This transforms $p(z)$ into $z^n \cdot p(1/z)$, inverting roots through the unit circle and producing qualitatively different root loci.

# Ehrlich-Aberth Solver

The solver finds all roots of a complex polynomial simultaneously. It is the core computational kernel, consuming the majority of render time.

## Algorithm

Given a degree-$n$ polynomial $p(z) = \sum_{k=0}^{n} c_k z^k$ with initial root guesses $z_0, \ldots, z_{n-1}$:

1. **Horner evaluation**: For each root $z_i$, compute $p(z_i)$ and $p'(z_i)$ in $O(n)$ via nested multiplication.

2. **Newton step**: $w_i = p(z_i) / p'(z_i)$.

3. **Aberth correction**: $S_i = \sum_{j \neq i} \frac{1}{z_i - z_j}$, then update $z_i \leftarrow z_i - \frac{w_i}{1 - w_i S_i}$.

4. **Convergence**: Stop when $\max_i |w_i / (1 - w_i S_i)|^2 < 10^{-16}$, or after 64 iterations.

Complexity per iteration: $O(n^2)$ (the Aberth sum). For degree 25, a typical grid point converges in 3--4 iterations; degree 49 takes 3--5.

## Warm-starting

Roots from the previous grid point are used as initial guesses for the next, dramatically reducing iteration count. The grid is traversed in serpentine order (even rows left-to-right, odd rows right-to-left) to ensure adjacent grid points are solved consecutively.

## Root matching

After solving, roots are matched to the previous step's root ordering by greedy nearest-neighbor assignment (squared Euclidean distance). This ensures consistent root-to-color mapping across the grid.

# Rendering Pipeline (C Binary)

The `runRender()` function in `sweep_cli.c` performs the full grid scan and pixel accumulation.

## Input

JSON on stdin:

```json
{
  "mode": "render",
  "function": "giga_42",
  "n1": 1000, "n2": 1000,
  "width": 4096, "height": 4096,
  "auto_scale": true,
  "i1_start": 0, "i1_end": 1000
}
```

The `i1_start` and `i1_end` fields are optional and default to `[0, n1)`. They define the row range for striped parallelism.

## Auto-scale

When `auto_scale` is true and the render is not a stripe:

1. Sample a subset of the grid (every 10th row and column for grids larger than 50 in either dimension).
2. Solve each sampled point and track the bounding box of all roots.
3. Set `center = (min + max) / 2` and `scale = min(W / range_re, H / range_im)` with 5% margin.

## Rainbow palette

Each root index $i \in [0, \text{degree})$ maps to a fixed RGB color via HSL-to-RGB conversion with $H = i / \text{degree}$, $S = 1$, $L = 0.5$. Root 0 is red, cycling through the spectrum.

## Pixel accumulation

For each root $(r, s)$ at grid point $(x_1, x_2)$:

$$p_x = \lfloor W/2 + (\text{Re}(r) - c_{\text{re}}) \cdot \text{scale} \rfloor, \quad p_y = \lfloor H/2 - (\text{Im}(r) - c_{\text{im}}) \cdot \text{scale} \rfloor$$

If $(p_x, p_y)$ is within the image bounds, the root's palette color is additively blended:

$$\text{pixel}[p_y \cdot W + p_x][c] \leftarrow \min(255, \; \text{pixel}[\ldots][c] + \text{palette}[i][c])$$

Roots outside the viewport are counted as clipped but do not crash the renderer.

## Output

- Raw RGB bytes written to the output file path (row-major, 3 bytes per pixel, $W \times H \times 3$ total).
- JSON metadata printed to stdout: function name, degree, grid dimensions, viewport parameters, root counts, elapsed microseconds, average iterations.

# Stripe Parallelism

For large grids, the handler splits the computation across multiple Lambda invocations to stay within the API Gateway 30-second timeout.

## When stripes activate

- **Auto**: If `n_stripes` is 0 or 1 and the grid exceeds 50,000 cells, auto-select $\min(\max(\lfloor \text{cells} / 50000 \rfloor, 2), 10)$ stripes.
- **Manual**: The frontend exposes a Stripes input (0 = auto, 1--50 manual).
- **Capping**: The handler caps stripes based on a merge-time budget. At 4096$\times$4096, each merge step takes approximately 1 second (download + `ImageChops.add`), so the maximum is approximately 25 stripes.

## Execution flow

1. **Pre-pass**: Run a tiny render (50$\times$50 grid, 64$\times$64 image) to determine the viewport (`center_re`, `center_im`, `scale`). Re-scale for actual image dimensions.

2. **Fan-out**: Divide `n1` rows into $N$ stripes. Invoke each as a separate Lambda via `lambda_client.invoke()` with `InvocationType=RequestResponse`. All invocations run concurrently via `ThreadPoolExecutor(max_workers=N)`.

3. **Stripe execution**: Each stripe Lambda:
   - Runs the C binary with `i1_start` / `i1_end` and explicit viewport (no auto-scale).
   - Gets a full $W \times H \times 3$ raw RGB buffer (mostly black, with pixels only where that stripe's roots land).
   - Compresses to PNG (level 1 for speed) and uploads to S3.
   - Returns metadata: elapsed time, roots plotted/clipped, average iterations.

4. **Merge**: The coordinator downloads each stripe's PNG from S3, decompresses it, and composites all stripes using Pillow's `ImageChops.add()` (per-channel additive with clamping at 255).

5. **Encode**: The merged image is encoded to the requested format (JPEG or PNG) and uploaded to S3. Stripe PNGs are cleaned up.

## Performance characteristics

Each stripe produces a full-resolution image buffer, so the transfer and merge cost scales linearly with stripe count. PNG compression helps significantly (a sparse stripe image compresses from 48 MB to approximately 1--2 MB), but the Pillow merge operation itself takes approximately 0.5 seconds per stripe at 4096 by 4096.

\begin{table}[h]
\centering
\begin{tabular}{lrrrrrr}
\toprule
Grid & Degree & Stripes & Wall compute & Merge & Total & Transfer \\
\midrule
$100 \times 100$   & 25 & 1  & 0.5s & ---  & 1s  & ---    \\
$500 \times 500$   & 25 & 5  & 7s   & 4s   & 14s & 5 MB   \\
$1000 \times 1000$ & 25 & 10 & 11s  & 11s  & 25s & 14 MB  \\
$500 \times 500$   & 49 & 5  & 14s  & 4s   & 21s & 10 MB  \\
\bottomrule
\end{tabular}
\caption{Render times at 4096$\times$4096 output resolution (JPEG Q90).}
\end{table}

The merge phase dominates for high stripe counts. Each stripe's RGB buffer is the full image size regardless of how few pixels it actually writes, so the PNG compression ratio depends on sparsity. A future improvement could transmit only the non-zero pixels (sparse pixel lists) to reduce both transfer and merge time.

# Image Encoding and Storage

## Pillow layer

The Lambda uses a custom ARM64 Pillow layer (`arn:aws:lambda:us-east-1:710848990594:layer:pillow:1`) containing Pillow 12.1.1 built for `aarch64-manylinux_2_28`. The layer is attached at deploy time.

## Encoding

- **JPEG**: `Image.save(buf, format="JPEG", quality=Q)` where $Q \in [1, 100]$, default 90.
- **PNG**: `Image.save(buf, format="PNG")` with default compression.

Typical file sizes for a 4096$\times$4096 render:

- JPEG Q90: 5--8 MB (lossy, fast)
- PNG: 15--25 MB (lossless, slower to encode)

## S3 storage

Images are stored at `s3://polypaint/renders/{job_id}/image.{ext}`. A presigned URL with 1-hour expiry is returned to the client.

# Frontend

## Render tab controls

| Control | Default | Range | Notes |
|---------|---------|-------|-------|
| Function | giga\_5 | dropdown | giga\_5, rev\_giga\_5, giga\_42, rev\_giga\_42 |
| n1, n2 | 500 | 10--5000 | Grid resolution per axis |
| Width, Height | 4096 | 256--16384 | Output image pixels |
| Format | JPEG | JPEG/PNG | |
| Quality | 90 | 1--100 | JPEG only |
| Stripes | 0 | 0--50 | 0 = auto |

## Presets

- **Fast**: 100$\times$100 grid, 1024$\times$1024 image (approximately 1s)
- **Medium**: 500$\times$500 grid, 4096$\times$4096 image (approximately 15s)
- **Hi-res**: 1000$\times$1000 grid, 4096$\times$4096 image (approximately 25s)

## Status display

After a successful render, the status line shows:

- **Single-pass**: `{roots} roots | {time}s | {size}MB {format} | network {ms}ms`
- **Striped**: `{roots} roots | {N} stripes | wall {W}s | compute {C}s | merge {M}s | xfer {X}MB | {size}MB {format} | network {ms}ms`

The result panel shows a clickable image preview and a download link.

# Infrastructure

## Lambda configuration

| Setting | Value |
|---------|-------|
| Function name | `polypaint-solver` |
| Runtime | Python 3.12 |
| Architecture | ARM64 (Graviton) |
| Memory | 512 MB |
| Timeout | 900 seconds |
| Layers | Pillow 12.1.1 (ARM64) |

## API Gateway

HTTP API (`smojhi4gqe`) with routes:

- `POST /solve` -- single polynomial solve
- `POST /sweep` -- parameter sweep (binary output)
- `POST /stripe` -- internal stripe worker (grid mode)
- `POST /render` -- image render
- `POST /render-stripe` -- internal stripe worker (render mode)

All routes proxy to the same Lambda. The API Gateway has a hard **30-second timeout**, which is the binding constraint for render operations.

## Build toolchain

The C binaries are cross-compiled for ARM64 Linux using `aarch64-linux-musl-gcc -O3 -static`, producing statically linked executables that run directly on Lambda's Graviton processors without any shared library dependencies.

## Memory budget

At 512 MB Lambda memory:

- 4096$\times$4096$\times$3 = 48 MB (raw RGB pixel buffer)
- 8192$\times$8192$\times$3 = 192 MB (feasible)
- 16384$\times$16384$\times$3 = 768 MB (would exceed memory)

The C binary caps image dimensions at 16384$\times$16384 and the grid at 10 million cells.

# Limitations and Future Work

## Current limitations

1. **API Gateway 30-second timeout**: The binding constraint. Large renders must complete within 30 seconds including stripe fan-out, merge, encoding, and S3 upload.

2. **Merge bottleneck**: Each stripe produces a full $W \times H$ image (mostly black pixels). Downloading and compositing $N$ full-resolution PNGs from S3 takes approximately 1 second per stripe at 4K. This limits practical stripe count to approximately 10--15 at 4096$\times$4096.

3. **Additive saturation**: Dense regions saturate to white as pixel values clamp at 255. No HDR accumulation or tone mapping is applied.

## Potential improvements

1. **Sparse pixel output**: Instead of full RGB buffers, stripes could output only non-zero pixel coordinates and colors. This would reduce transfer from approximately 1.5 MB/stripe to approximately 100 KB/stripe and eliminate the per-stripe Pillow decode + add.

2. **HDR accumulation**: Use 16-bit per channel (or 32-bit float) accumulators in the C binary, then tone-map to 8-bit for output. This would preserve detail in dense regions.

3. **DeepZoom (DZI) tiling**: Generate image pyramids for client-side deep-zoom viewing. Requires either a working pyvips setup or a custom tiler using Pillow's crop and resize.

4. **Async renders**: For very large grids, return a job ID immediately and let the client poll for completion, removing the 30-second constraint entirely.
