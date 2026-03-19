"""
Param debug Lambda — renders transformed parameter pairs as bilevel TIFF.

Self-contained: generates grid, applies param transforms, computes viewport,
rasters to bitset, writes TIFF via bilevel_merge. No stripe fanout, no merge,
no stitch. One Lambda, one or two output TIFFs.

Results go to debug/{job_id}/ to avoid interfering with renders/.
"""
import json
import math
import os
import struct
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, imgpipe_env

s3 = boto3.client("s3")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")
PRESIGN_EXPIRY = 3600


# ---- Param transforms (matching sweep_cli.c exactly) ----

def pt_unit_circle(z1, z2, args):
    a1 = 2 * math.pi * z1.real
    a2 = 2 * math.pi * z2.real
    return complex(math.cos(a1), math.sin(a1)), complex(math.cos(a2), math.sin(a2))

def pt_rtheta(z1, z2, args):
    r1, r2 = z1.real, z2.real
    a1 = 2 * math.pi * z2.real
    a2 = 2 * math.pi * z1.real
    return complex(r1 * math.cos(a1), r1 * math.sin(a1)), complex(r2 * math.cos(a2), r2 * math.sin(a2))

def pt_square(z1, z2, args):
    return z1 * z1, z2 * z2

def pt_cube(z1, z2, args):
    return z1 * z1 * z1, z2 * z2 * z2

def pt_reciprocal(z1, z2, args):
    d1 = z1.real**2 + z1.imag**2
    d2 = z2.real**2 + z2.imag**2
    r1 = complex(z1.real / d1, -z1.imag / d1) if d1 > 1e-30 else 0j
    r2 = complex(z2.real / d2, -z2.imag / d2) if d2 > 1e-30 else 0j
    return r1, r2

def pt_conjugate(z1, z2, args):
    return z1.conjugate(), z2.conjugate()

def pt_swap(z1, z2, args):
    return z2, z1

def pt_add_sub(z1, z2, args):
    return z1 + z2, z1 - z2

def pt_mul_div(z1, z2, args):
    d = z2.real**2 + z2.imag**2
    div = complex((z1.real*z2.real + z1.imag*z2.imag)/d, (z1.imag*z2.real - z1.real*z2.imag)/d) if d > 1e-30 else 0j
    return z1 * z2, div

def pt_moebius(z1, z2, args):
    a1 = z1 + 2
    d1 = a1.real**2 + a1.imag**2
    r1 = complex(a1.real / d1, -a1.imag / d1) if d1 > 1e-30 else 0j
    a2 = z2 + 2
    d2 = a2.real**2 + a2.imag**2
    r2 = complex(a2.real / d2, -a2.imag / d2) if d2 > 1e-30 else 0j
    return r1, r2

def pt_shift1(z1, z2, args):
    return z1 + 1, z2 + 1

def pt_scale10(z1, z2, args):
    return z1 * 10, z2 * 10

def pt_negate(z1, z2, args):
    return -z1, -z2

def pt_zz(z1, z2, args):
    return complex(z1.real, z2.real), complex(z2.real, z1.real)

def pt_exp(z1, z2, args):
    import cmath
    return cmath.exp(z1), cmath.exp(z2)

def pt_radd(z1, z2, args):
    v = args[0] if args else 0.0
    return complex(z1.real + v, z1.imag), complex(z2.real + v, z2.imag)

def pt_iadd(z1, z2, args):
    v = args[0] if args else 0.0
    return complex(z1.real, z1.imag + v), complex(z2.real, z2.imag + v)

def pt_add(z1, z2, args):
    v = args[0] if args else 0.0
    return z1 + complex(v, v), z2 + complex(v, v)

def pt_cadd(z1, z2, args):
    re = args[0] if len(args) > 0 else 0.0
    im = args[1] if len(args) > 1 else 0.0
    c = complex(re, im)
    return z1 + c, z2 + c

def pt_rscale(z1, z2, args):
    v = args[0] if args else 1.0
    return complex(z1.real * v, z1.imag), complex(z2.real * v, z2.imag)

def pt_iscale(z1, z2, args):
    v = args[0] if args else 1.0
    return complex(z1.real, z1.imag * v), complex(z2.real, z2.imag * v)

def pt_scale(z1, z2, args):
    v = args[0] if args else 1.0
    return z1 * v, z2 * v

TRANSFORMS = {
    "unit_circle": pt_unit_circle, "rtheta": pt_rtheta,
    "square": pt_square, "cube": pt_cube,
    "reciprocal": pt_reciprocal, "conjugate": pt_conjugate,
    "swap": pt_swap, "add_sub": pt_add_sub, "mul_div": pt_mul_div,
    "moebius": pt_moebius, "shift1": pt_shift1, "scale10": pt_scale10,
    "negate": pt_negate, "zz": pt_zz, "exp": pt_exp,
    "radd": pt_radd, "iadd": pt_iadd, "add": pt_add, "cadd": pt_cadd,
    "rscale": pt_rscale, "iscale": pt_iscale, "scale": pt_scale,
}


def apply_transforms(z1, z2, transform_chain):
    """Apply a chain of param transforms."""
    for entry in transform_chain:
        name = entry[0] if isinstance(entry, list) else entry
        args = [float(a) for a in entry[1:]] if isinstance(entry, list) and len(entry) > 1 else []
        fn = TRANSFORMS.get(name)
        if fn:
            z1, z2 = fn(z1, z2, args)
    return z1, z2


def raster_points(points, pix):
    """Raster complex points to a bitset. Auto-viewport with 5% shim."""
    if not points:
        return bytearray((pix * pix + 7) // 8), 0

    reals = [p.real for p in points if math.isfinite(p.real) and math.isfinite(p.imag)]
    imags = [p.imag for p in points if math.isfinite(p.real) and math.isfinite(p.imag)]
    if not reals:
        return bytearray((pix * pix + 7) // 8), 0

    reals.sort()
    imags.sort()
    # 1% quantile trim
    lo = max(0, int(len(reals) * 0.01))
    hi = min(len(reals) - 1, int(len(reals) * 0.99))
    min_re, max_re = reals[lo], reals[hi]
    min_im, max_im = imags[lo], imags[hi]

    cx = (min_re + max_re) / 2
    cy = (min_im + max_im) / 2
    range_re = (max_re - min_re) * 1.05
    range_im = (max_im - min_im) * 1.05
    span = max(range_re, range_im)
    if span < 1e-10:
        span = 1.0
    scale = pix / span
    half = pix / 2.0

    bitset = bytearray((pix * pix + 7) // 8)
    plotted = 0
    for p in points:
        if not (math.isfinite(p.real) and math.isfinite(p.imag)):
            continue
        px = int(half + (p.real - cx) * scale)
        py = int(half - (p.imag - cy) * scale)
        if 0 <= px < pix and 0 <= py < pix:
            idx = py * pix + px
            byte_idx = idx >> 3
            mask = 1 << (idx & 7)
            if not (bitset[byte_idx] & mask):
                bitset[byte_idx] |= mask
                plotted += 1
    return bytes(bitset), plotted


def write_tiff(bitset_bytes, w, h, out_path):
    """Write bitset to 1-bit TIFF via bilevel_merge."""
    bits_path = out_path + ".bits"
    with open(bits_path, "wb") as f:
        f.write(bitset_bytes)
    cmd = [BILEVEL_MERGE, "merge", f"--tile_w={w}", f"--tile_h={h}",
           f"--output={out_path}", bits_path]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120,
                            env=imgpipe_env())
    os.remove(bits_path)
    if result.returncode != 0:
        raise RuntimeError(f"bilevel_merge failed: {result.stderr.strip()}")
    return json.loads(result.stdout)


def handler(event, context):
    params = parse_body(event)
    n1 = int(params.get("n1", 500))
    n2 = int(params.get("n2", 500))
    transform_chain = params.get("param_transforms", [["unit_circle"]])
    mode = params.get("mode", "together")  # "together" or "separate"
    pix = int(params.get("pix", 5000))
    job_id = params.get("job_id", "debug")

    t0 = time.time()

    # Generate grid and apply transforms
    points1 = []
    points2 = []
    for i1 in range(n1):
        for i2 in range(n2):
            x1 = i1 / n1
            x2 = i2 / n2
            z1 = complex(x1, 0)
            z2 = complex(x2, 0)
            z1, z2 = apply_transforms(z1, z2, transform_chain)
            points1.append(z1)
            points2.append(z2)

    gen_ms = int((time.time() - t0) * 1000)
    results = []

    if mode == "separate":
        # Two images: param1 and param2
        for label, points in [("param1", points1), ("param2", points2)]:
            bitset, plotted = raster_points(points, pix)
            out_path = f"/tmp/{label}_debug.tif"
            meta = write_tiff(bitset, pix, pix, out_path)

            s3_key = f"debug/{job_id}/{label}_debug.tif"
            with open(out_path, "rb") as f:
                s3.put_object(Bucket=BUCKET, Key=s3_key, Body=f, ContentType="image/tiff")
            os.remove(out_path)

            url = s3.generate_presigned_url("get_object",
                Params={"Bucket": BUCKET, "Key": s3_key}, ExpiresIn=PRESIGN_EXPIRY)
            results.append({
                "label": label, "key": s3_key, "url": url,
                "pixels_set": plotted, "file_size": meta.get("file_size", 0),
            })
    else:
        # One image: both params combined
        all_points = points1 + points2
        bitset, plotted = raster_points(all_points, pix)
        out_path = "/tmp/param_debug.tif"
        meta = write_tiff(bitset, pix, pix, out_path)

        s3_key = f"debug/{job_id}/param_debug.tif"
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=s3_key, Body=f, ContentType="image/tiff")
        os.remove(out_path)

        url = s3.generate_presigned_url("get_object",
            Params={"Bucket": BUCKET, "Key": s3_key}, ExpiresIn=PRESIGN_EXPIRY)
        results.append({
            "label": "combined", "key": s3_key, "url": url,
            "pixels_set": plotted, "file_size": meta.get("file_size", 0),
        })

    total_ms = int((time.time() - t0) * 1000)

    return ok_response({
        "n1": n1, "n2": n2, "pix": pix, "mode": mode,
        "gen_ms": gen_ms, "total_ms": total_ms,
        "n_points": n1 * n2,
        "images": results,
    })
