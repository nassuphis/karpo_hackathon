"""
Param debug Lambda — renders transformed parameter pairs as bilevel TIFF.

Uses param_gen C binary (same transform pipeline as sweep_cli.c) to generate
transformed parameter points, then rasters them to bilevel TIFF via
bilevel_merge. No Python reimplementation of transforms.

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
PARAM_GEN = os.path.join(os.path.dirname(__file__), "param_gen")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")
PRESIGN_EXPIRY = 3600


def raster_points_from_bin(bin_path, n_floats_per_point, point_indices, pix):
    """Raster selected float pairs from a .bin file to a bitset."""
    with open(bin_path, "rb") as f:
        data = f.read()

    n_bytes = len(data)
    n_points = n_bytes // (n_floats_per_point * 4)
    stride = n_floats_per_point * 4

    # Collect the selected complex values
    reals = []
    imags = []
    for p in range(n_points):
        offset = p * stride
        for idx in point_indices:
            re_off = offset + idx * 8
            if re_off + 8 > n_bytes:
                break
            re, im = struct.unpack_from('<ff', data, re_off)
            if math.isfinite(re) and math.isfinite(im):
                reals.append(re)
                imags.append(im)

    if not reals:
        return bytearray((pix * pix + 7) // 8), 0

    # Auto-viewport with 1% quantile trim, 5% shim
    reals_sorted = sorted(reals)
    imags_sorted = sorted(imags)
    n = len(reals_sorted)
    lo = max(0, int(n * 0.01))
    hi = min(n - 1, int(n * 0.99))
    min_re, max_re = reals_sorted[lo], reals_sorted[hi]
    min_im, max_im = imags_sorted[lo], imags_sorted[hi]

    cx = (min_re + max_re) / 2
    cy = (min_im + max_im) / 2
    span = max((max_re - min_re) * 1.05, (max_im - min_im) * 1.05)
    if span < 1e-10:
        span = 1.0
    scale = pix / span
    half = pix / 2.0

    bitset = bytearray((pix * pix + 7) // 8)
    plotted = 0
    for re, im in zip(reals, imags):
        px = int(half + (re - cx) * scale)
        py = int(half - (im - cy) * scale)
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
    transform_chain = params.get("param_transforms", [])
    mode = params.get("mode", "together")  # "together" or "separate"
    pix = int(params.get("pix", 5000))
    job_id = params.get("job_id", "debug")

    t0 = time.time()

    # Generate transformed params using param_gen C binary (real transform pipeline)
    bin_path = "/tmp/params.bin"
    spec = json.dumps({
        "n1": n1, "n2": n2,
        "param_transforms": transform_chain,
    })
    gen_result = subprocess.run(
        [PARAM_GEN, bin_path],
        input=spec, capture_output=True, text=True, timeout=60
    )
    if gen_result.returncode != 0:
        raise RuntimeError(f"param_gen failed: {gen_result.stderr.strip()}")
    gen_meta = json.loads(gen_result.stdout)
    gen_ms = int((time.time() - t0) * 1000)

    # Output format: 4 floats per point (z1r, z1i, z2r, z2i)
    # point_indices: 0 = z1 (re at offset 0, im at offset 4)
    #                1 = z2 (re at offset 8, im at offset 12)
    results = []

    if mode == "separate":
        for label, indices in [("param1", [0]), ("param2", [1])]:
            bitset, plotted = raster_points_from_bin(bin_path, 4, indices, pix)
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
        # Together: both z1 and z2 in one image
        bitset, plotted = raster_points_from_bin(bin_path, 4, [0, 1], pix)
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

    os.remove(bin_path)
    total_ms = int((time.time() - t0) * 1000)

    return ok_response({
        "n1": n1, "n2": n2, "pix": pix, "mode": mode,
        "gen_ms": gen_ms, "total_ms": total_ms,
        "n_points": gen_meta["n_points"],
        "images": results,
    })
