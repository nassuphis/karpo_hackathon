"""
Param debug Lambda — renders transformed parameter pairs as bilevel TIFF.

Uses param_gen C binary (same transform pipeline as sweep_cli.c) to generate
transformed parameter points, then rasters them to bilevel TIFF via the native
full-frame bitset assembler. No Python reimplementation of transforms.

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
from param_program_chain import compile_param_program_chain

s3 = boto3.client("s3")
SWEEP = os.path.join(os.path.dirname(__file__), "sweep")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")
PRESIGN_EXPIRY = 3600
MAX_SYNC_PARAM_DEBUG_N = 512
MAX_SYNC_PARAM_DEBUG_PIX = 2048


def error_response(status_code, message):
    return {
        "statusCode": int(status_code),
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps({"message": str(message)}),
    }


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

    # Auto-viewport: use full range (no quantile trim) with 10% shim
    min_re, max_re = min(reals), max(reals)
    min_im, max_im = min(imags), max(imags)

    cx = (min_re + max_re) / 2
    cy = (min_im + max_im) / 2
    range_re = (max_re - min_re) * 1.1
    range_im = (max_im - min_im) * 1.1
    span = max(range_re, range_im)
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
    """Write a square full-frame bitset to a 1-bit TIFF."""
    if int(w) != int(h):
        raise RuntimeError(f"param debug requires square output, got {w}x{h}")
    bits_path = out_path + ".bits"
    with open(bits_path, "wb") as f:
        f.write(bitset_bytes)
    preview_path = out_path.replace('.tif', '_preview.png')
    cmd = [BILEVEL_MERGE, "assemble", f"--pix={int(w)}",
           f"--output={out_path}",
           f"--preview={preview_path}", "--preview_size=512",
           bits_path]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120,
                            env=imgpipe_env())
    os.remove(bits_path)
    if result.returncode != 0:
        raise RuntimeError(f"bilevel_merge failed: {result.stderr.strip()}")
    return json.loads(result.stdout), preview_path


def handler(event, context):
    params = parse_body(event)
    grid_n = int(params.get("N", params.get("n1", 500)))
    n1 = grid_n
    n2 = grid_n
    pix = int(params.get("pix", min(grid_n * 2, MAX_SYNC_PARAM_DEBUG_PIX)))
    if grid_n < 8:
        return error_response(400, "param debug requires N >= 8")
    if grid_n > MAX_SYNC_PARAM_DEBUG_N:
        return error_response(
            400,
            f"param debug is synchronous; use N <= {MAX_SYNC_PARAM_DEBUG_N} "
            "or use Compute Preview for larger exploratory runs",
        )
    if pix < 64:
        return error_response(400, "param debug requires pix >= 64")
    if pix > MAX_SYNC_PARAM_DEBUG_PIX:
        return error_response(
            400,
            f"param debug is synchronous; use pix <= {MAX_SYNC_PARAM_DEBUG_PIX}",
        )
    transform_chain = params.get("param_transforms", [])
    param_program_chain = params.get("param_program_chain") or []
    param_program = None
    if param_program_chain:
        if not isinstance(param_program_chain, list):
            raise RuntimeError("param_program_chain must be a list")
        compiled_param_program = compile_param_program_chain(param_program_chain)
        if compiled_param_program["legacy_transforms"]:
            transform_chain = compiled_param_program["legacy_transforms"]
        else:
            transform_chain = []
            param_program = {
                "version": compiled_param_program["version"],
                "fingerprint": compiled_param_program["fingerprint"],
                "display": compiled_param_program["display"],
                "stack_max": compiled_param_program["stack_max"],
                "token_count": compiled_param_program["token_count"],
                "uses_legacy_fast_path": compiled_param_program["uses_legacy_fast_path"],
                "tokens": compiled_param_program["tokens"],
            }
    mode = params.get("mode", "together")  # "together" or "separate"
    job_id = params.get("job_id", "debug")

    t0 = time.time()

    # Generate transformed params using sweep in param_dump mode (exact same code path as coeffgen)
    bin_path = "/tmp/params.bin"
    spec_payload = {
        "mode": "param_dump",
        "n1": n1, "n2": n2,
        "param_transforms": transform_chain,
    }
    if param_program:
        spec_payload["param_program"] = param_program
    spec = json.dumps(spec_payload)
    gen_result = subprocess.run(
        [SWEEP, bin_path],
        input=spec, capture_output=True, text=True, timeout=60
    )
    if gen_result.returncode != 0:
        raise RuntimeError(f"sweep param_dump failed: {gen_result.stderr.strip()}")
    gen_meta = json.loads(gen_result.stdout)
    gen_ms = int((time.time() - t0) * 1000)

    # Output format: 4 floats per point (z1r, z1i, z2r, z2i)
    # point_indices: 0 = z1 (re at offset 0, im at offset 4)
    #                1 = z2 (re at offset 8, im at offset 12)
    results = []

    items = [("param1", [0]), ("param2", [1])] if mode == "separate" else [("combined", [0, 1])]
    for label, indices in items:
        bitset, plotted = raster_points_from_bin(bin_path, 4, indices, pix)
        out_path = f"/tmp/{label}_debug.tif"
        meta, preview_file = write_tiff(bitset, pix, pix, out_path)

        # Upload TIFF
        tif_key = f"debug/{job_id}/{label}_debug.tif"
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=tif_key, Body=f, ContentType="image/tiff")
        os.remove(out_path)
        tif_url = s3.generate_presigned_url("get_object",
            Params={"Bucket": BUCKET, "Key": tif_key}, ExpiresIn=PRESIGN_EXPIRY)

        # Upload preview PNG
        preview_url = ""
        if os.path.exists(preview_file):
            png_key = f"debug/{job_id}/{label}_preview.png"
            with open(preview_file, "rb") as f:
                s3.put_object(Bucket=BUCKET, Key=png_key, Body=f, ContentType="image/png")
            os.remove(preview_file)
            preview_url = s3.generate_presigned_url("get_object",
                Params={"Bucket": BUCKET, "Key": png_key}, ExpiresIn=PRESIGN_EXPIRY)

        results.append({
            "label": label, "key": tif_key, "url": tif_url,
            "preview_url": preview_url,
            "pixels_set": plotted, "file_size": meta.get("file_size", 0),
        })

    os.remove(bin_path)
    total_ms = int((time.time() - t0) * 1000)

    return ok_response({
        "N": grid_n, "pix": pix, "mode": mode,
        "gen_ms": gen_ms, "total_ms": total_ms,
        "n_points": gen_meta["n_points"],
        "images": results,
    })
