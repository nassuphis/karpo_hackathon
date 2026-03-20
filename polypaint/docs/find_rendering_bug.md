# Rendering Bug: Stale /tmp Files on Warm Lambda Containers

## Symptom

BiLevel renders show "strange squares and copies" — tile artifacts that look like fragments from other tiles or previous renders duplicated into wrong positions.

## Root Cause

**Warm Lambda container reuse** leaves stale output files on `/tmp`.

When a Lambda execution environment is reused for a subsequent invocation, `/tmp` is NOT cleaned. The raster binary (`bilevel_raster`) only writes `.bits` files for tiles that contain root hits. It does not delete pre-existing `.bits` files from a previous invocation.

The handler then iterates over ALL tile indices and uploads whatever `.bits` files exist — including stale ones from the previous run. These stale bitsets get OR-merged into the final image, producing duplicated/misplaced root patterns.

### Concrete scenario

1. Lambda container runs stripe 7 → produces bits for tiles 5, 10, 15, 20
2. Same container reused for stripe 3 → produces bits for tiles 5, 8, 12
3. Handler uploads tiles 5, 8, 10, 12, 15, 20 — but tiles 10, 15, 20 are from the OLD stripe 7 run
4. Merge ORs these stale bitsets in, creating ghost images in wrong tiles

## Fix

Clean stale output files before running the raster binary:

```python
import glob
for stale in glob.glob("/tmp/bits_t*.bits"):
    os.remove(stale)
```

Applied to all three raster handlers:
- `handler_bilevel.py` raster phase (bits_t*.bits)
- `handler_bilevel.py` coeff_raster phase (coeff_bits_t*.bits)
- `handler_raster.py` color phase (pix_t*.pix)

## Affected Paths

| Handler | Output pattern | Fixed |
|---------|---------------|-------|
| bilevel raster | `/tmp/bits_t*.bits` | Yes |
| coeff bilevel raster | `/tmp/coeff_bits_t*.bits` | Yes |
| color raster | `/tmp/pix_t*.pix` | Yes |

## Why This Wasn't Caught

- The bug only manifests with warm container reuse
- Local testing always starts with clean `/tmp`
- Low-concurrency jobs may get fresh containers every time
- High-concurrency jobs (many stripes) increase the odds of reuse
- The stale data looks like valid root patterns, not random noise

## Testing

A reliable reproduction requires warm containers processing multiple stripes of the same or different jobs. This is hard to test locally. The fix is defensive cleanup that costs nothing on fresh containers.
