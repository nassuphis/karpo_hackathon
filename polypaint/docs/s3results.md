# S3 Results Storage

How compute results are stored and retrieved from S3.

## Bucket & Table

- **S3 bucket**: `polypaint` (defined in `shared.py` and `deploy.sh`)
- **DynamoDB table**: `polypaint-jobs` (status tracking with 24h TTL auto-cleanup)

## Job ID Format

Generated in the frontend (`index.html` ~line 1576):

```javascript
const jobId = 'compute_' + Date.now().toString(36);
```

Example: `compute_mmvtc0zf`

## S3 Key Structure

All files live under `renders/{job_id}/`:

### Phase 1: Coefficient Generation (handler_coeffgen.py)

| Key | Content |
|-----|---------|
| `renders/{job_id}/coeffs_{stripe:04d}.bin` | Binary coefficients per stripe (float32 re/im pairs) |
| `renders/{job_id}/lores_coeffs.bin` | Low-resolution coefficients (~250K roots target) |

### Phase 2: Root Solving (handler_sweep.py)

| Key | Content |
|-----|---------|
| `renders/{job_id}/stripe_{stripe:04d}.bin` | Roots per stripe (float32 re/im pairs) |
| `renders/{job_id}/lores.bin` | Low-resolution roots for preview/viewport |

### Phase 3: Rasterization (handler_raster.py)

| Key | Content |
|-----|---------|
| `renders/{job_id}/pix_{stripe:04d}_t{tile:04d}.pix` | Sparse pixel data per stripe per tile |

Only non-empty tiles are uploaded.

### Phase 4: Tile Assembly (handler_finalize.py)

| Key | Content |
|-----|---------|
| `renders/{job_id}/tile_{tile:04d}.raw` | Raw RGB pixel buffer per 2D tile |

### Phase 5: Encoding (handler_encode.py)

| Key | Content |
|-----|---------|
| `renders/{job_id}/image.jpeg` | Final JPEG image |
| `renders/{job_id}/image.png` | Final PNG image |
| `renders/{job_id}/image_bilevel.png` | Bilevel (black/white) PNG |

### Preview (handler_preview.py)

| Key | Content |
|-----|---------|
| `renders/{job_id}/preview.png` | 256x256 grayscale preview (single Lambda call, no intermediate steps) |

### Metadata

| Key | Content |
|-----|---------|
| `renders/{job_id}/calc.json` | Compute metadata (see below) |
| `renders/{job_id}/view.json` | Viewport metadata (center, scale, quantiles) |

## calc.json Schema

Created by handler_storage.py `/save-metadata` endpoint:

```json
{
  "job_id": "compute_mmvtc0zf",
  "function": "poly_33",
  "degree": 70,
  "n1": 500,
  "n2": 500,
  "n_stripes": 10,
  "pipeline": {
    "param_transforms": [["unit_circle"]],
    "coeff_transforms": ["rev"]
  },
  "coeffs_keys": ["renders/.../coeffs_0000.bin", ...],
  "total_coeffs_size": 142000000,
  "lores": {
    "bin_key": "renders/.../lores_coeffs.bin",
    "n1": 50,
    "n2": 50,
    "bin_size": 1420000
  },
  "stripes": [
    {
      "idx": 0,
      "bin_key": "renders/.../coeffs_0000.bin",
      "bin_size": 14200000,
      "compute_us": 1234567,
      "n_t": 25000,
      "avg_iterations": 32.5
    }
  ]
}
```

## view.json Schema

Created by handler_viewport.py:

```json
{
  "center_re": 0.0,
  "center_im": 0.0,
  "scale_ref": 1024.0,
  "n_roots": 17500000,
  "q_re": [-1.5, 1.5],
  "q_im": [-1.5, 1.5],
  "quantile": 0.995,
  "shim": 0.1
}
```

## Compute Lifecycle

### 1. Compute (frontend orchestration)

```
Generate job_id
  ├── Fan-out nStripes × coeffgen  →  coeffs_{0000..N}.bin + lores_coeffs.bin
  ├── Lores solve                  →  lores.bin
  ├── Fan-out nStripes × sweep     →  stripe_{0000..N}.bin
  └── Save metadata                →  calc.json
```

### 2. Render (frontend orchestration)

```
Viewport (reads lores.bin)         →  view.json
  ├── Fan-out nStripes × raster    →  pix_{stripe}_t{tile}.pix
  ├── Fan-out nTiles × finalize    →  tile_{tile}.raw
  └── Encode (stitches tiles)      →  image.{jpeg,png}
```

### 3. Preview (single call)

```
Preview (reads lores.bin)          →  preview.png
```

## Storage Operations (handler_storage.py)

| Endpoint | Action |
|----------|--------|
| `/list` | Lists all jobs under `renders/` prefix, reads each `calc.json` |
| `/delete` | Deletes entire `renders/{job_id}/*` |
| `/clean-render` | Deletes only render artifacts (`.raw`, `.jpeg`, `.png`, `.pix`) but preserves `.bin` and `calc.json` |
| `/check-status` | DynamoDB-based task completion polling |
| `/save-metadata` | Uploads `calc.json` |
| `/presign` | Generates presigned download URL (1-hour expiry) |

## Design Patterns

- **Striped parallelism**: Computation split into `nStripes` (default 10) parallel Lambda invocations per phase
- **Sparse pixel output**: Only non-empty `.pix` tiles uploaded (avoids W*H*4 full buffers)
- **Per-stripe files**: Each stripe has its own coefficient and root files (no monolithic files)
- **Stream uploads**: All S3 uploads use `upload_fileobj()` to avoid loading entire files into memory
- **DynamoDB polling**: Task status tracked in DynamoDB (not S3 object counting) for lower latency
- **Presigned URLs**: Final images served via presigned S3 URLs with 1-hour expiry
