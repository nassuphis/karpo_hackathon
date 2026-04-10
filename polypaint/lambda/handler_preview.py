"""
Preview Lambda handler — single-call preview generation.

Downloads lores.bin, computes viewport, plots roots as white pixels on black
to a 256×256 grayscale buffer, encodes PNG, uploads preview.png to S3,
returns presigned URL.

Replaces 4 sequential Lambda calls (viewport → raster → finalize → encode)
with one fast call. Pure Python, 1024 MB, no layers.
"""
import json
import math
import time
import struct

import boto3

from shared import BUCKET, PRESIGN_EXPIRY, parse_body, ok_response, compute_viewport_from_bin, encode_png_gray

s3 = boto3.client("s3")

def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    quantile = params.get("quantile", 0.0)
    shim = params.get("shim", 0.05)
    preview_size = params.get("preview_size", 256)

    t0 = time.time()

    # Read calc.json for lores key and degree
    calc_obj = s3.get_object(Bucket=BUCKET,
                             Key=f"renders/{job_id}/calc.json")
    calc = json.loads(calc_obj["Body"].read())
    lores_key = calc["lores"]["bin_key"]
    degree = calc.get("degree", 1)

    # Download lores.bin
    obj = s3.get_object(Bucket=BUCKET, Key=lores_key)
    bin_data = obj["Body"].read()

    # Compute viewport
    vp = compute_viewport_from_bin(bin_data, quantile=quantile, shim=shim)
    REF_SIZE = 4096
    scale = vp["scale"] * preview_size / REF_SIZE
    cx = vp["center_re"]
    cy = vp["center_im"]

    # Plot roots as white pixels on black — grayscale, 1 byte per pixel
    W = H = preview_size
    half_w = W / 2.0
    half_h = H / 2.0
    gray = bytearray(W * H)  # black background
    _isfinite = math.isfinite

    n_floats = len(bin_data) // 4
    n_pairs = n_floats // 2
    usable = n_pairs * 8
    for (re, im) in struct.iter_unpack('<ff', bin_data[:usable]):
        if _isfinite(re) and _isfinite(im):
            px = int(half_w + (re - cx) * scale)
            py = int(half_h - (im - cy) * scale)
            if 0 <= px < W and 0 <= py < H:
                gray[py * W + px] = 255
    n_roots_total = len(bin_data) // 8
    del bin_data

    # Encode as grayscale PNG (half the size of RGB)
    png_data = encode_png_gray(W, H, gray)

    # Upload preview
    preview_key = f"renders/{job_id}/preview.png"
    s3.put_object(Bucket=BUCKET, Key=preview_key,
                  Body=png_data, ContentType="image/png")

    # Persist preview stats for later retrieval
    preview_stats = {
        "n_roots": vp["n_roots"],
        "n_roots_total": n_roots_total,
        "q_re": vp["q_re"],
        "q_im": vp["q_im"],
        "created_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
    }
    s3.put_object(Bucket=BUCKET,
                  Key=f"renders/{job_id}/preview_stats.json",
                  Body=json.dumps(preview_stats),
                  ContentType="application/json")

    # Generate presigned URL
    image_url = s3.generate_presigned_url(
        "get_object",
        Params={"Bucket": BUCKET, "Key": preview_key},
        ExpiresIn=PRESIGN_EXPIRY)

    elapsed_us = int((time.time() - t0) * 1e6)

    return ok_response({
        "job_id": job_id,
        "preview_key": preview_key,
        "image_url": image_url,
        "preview_size": preview_size,
        "n_roots": vp["n_roots"],
        "n_roots_total": n_roots_total,
        "degree": degree,
        "q_re": vp["q_re"],
        "q_im": vp["q_im"],
        "png_size": len(png_data),
        "elapsed_us": elapsed_us,
    })
