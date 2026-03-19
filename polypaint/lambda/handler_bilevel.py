"""
Bilevel Lambda handler — three phases for stripe-first bilevel rendering.

  phase=raster: one stripe → per-tile bitset files (.bits)
  phase=merge:  per-tile bitsets → tile PNG (1-bit, via libvips)
  phase=stitch: tile PNGs → final image PNG (via libvips arrayjoin)

All three phases are handled by one Lambda function. The dispatch
Lambda sends all bilevel jobs to the same function; the handler
routes based on the 'phase' parameter.
"""
import json
import os
import subprocess
import time

import boto3
from botocore.exceptions import ClientError

from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
BILEVEL_RASTER = os.path.join(os.path.dirname(__file__), "bilevel_raster")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")


def handler(event, context):
    params = parse_body(event)
    phase = params["phase"]

    if phase == "raster":
        return handle_raster(params)
    elif phase == "merge":
        return handle_merge(params)
    elif phase == "stitch":
        return handle_stitch(params)
    else:
        raise ValueError(f"Unknown bilevel phase: {phase}")


def handle_raster(params):
    """One stripe → per-tile bitset files. One Lambda per stripe."""
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    task_id = f"bilevel_raster_{stripe_idx}"

    try:
        report_status(job_id, task_id, "started")

        # Download one stripe .bin
        bin_key = f"renders/{job_id}/stripe_{stripe_idx}.bin"
        bin_path = "/tmp/stripe.bin"
        t0 = time.time()
        obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
        with open(bin_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)

        # Run bilevel_raster
        out_prefix = "/tmp/bits"
        cmd = [
            BILEVEL_RASTER, bin_path, out_prefix,
            f"--width={params['width']}", f"--height={params['height']}",
            f"--tile_size={params['tile_size']}",
            f"--n_tile_cols={params['n_tile_cols']}",
            f"--n_tile_rows={params['n_tile_rows']}",
            f"--center_re={params['center_re']}",
            f"--center_im={params['center_im']}",
            f"--scale={params['scale']}",
            f"--degree={params['degree']}",
            f"--rotation={params.get('rotation', 0.0)}",
        ]
        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            raise RuntimeError(f"bilevel_raster failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        raster_ms = int((time.time() - t1) * 1000)

        os.remove(bin_path)

        # Upload non-empty .bits files
        n_tiles = params['n_tile_cols'] * params['n_tile_rows']
        uploaded = 0
        for t in range(n_tiles):
            bits_path = f"/tmp/bits_t{t:04d}.bits"
            if os.path.exists(bits_path) and os.path.getsize(bits_path) > 0:
                s3_key = f"renders/{job_id}/bits_s{stripe_idx:04d}_t{t:04d}.bits"
                with open(bits_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, s3_key)
                os.remove(bits_path)
                uploaded += 1
            elif os.path.exists(bits_path):
                os.remove(bits_path)

        report_status(job_id, task_id, "done")
        return ok_response({
            "stripe_idx": stripe_idx,
            "tiles_with_hits": uploaded,
            "roots_plotted": meta["roots_plotted"],
            "roots_clipped": meta["roots_clipped"],
            "dl_ms": dl_ms,
            "raster_ms": raster_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        for p in ["/tmp/stripe.bin"]:
            try:
                os.remove(p)
            except OSError:
                pass
        raise


def handle_merge(params):
    """OR per-stripe bitsets for one tile → tile PNG. One Lambda per tile."""
    job_id = params["job_id"]
    tile_idx = params["tile_idx"]
    tile_w = params["tile_w"]
    tile_h = params["tile_h"]
    n_stripes = params["n_stripes"]
    task_id = f"bilevel_merge_{tile_idx}"

    try:
        report_status(job_id, task_id, "started")

        # Download stripe .bits files for this tile
        bits_paths = []
        for s in range(n_stripes):
            bits_key = f"renders/{job_id}/bits_s{s:04d}_t{tile_idx:04d}.bits"
            local_path = f"/tmp/bits_s{s}.bits"
            try:
                obj = s3.get_object(Bucket=BUCKET, Key=bits_key)
                with open(local_path, "wb") as f:
                    f.write(obj["Body"].read())
                bits_paths.append(local_path)
            except ClientError as e:
                if e.response['Error']['Code'] == 'NoSuchKey':
                    continue  # No hits from this stripe for this tile
                raise

        # Run bilevel_merge
        out_path = "/tmp/tile.png"
        cmd = [
            BILEVEL_MERGE, "merge",
            f"--tile_w={tile_w}", f"--tile_h={tile_h}",
            f"--output={out_path}",
        ] + bits_paths

        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=300,
            env=imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"bilevel_merge failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)

        # Clean up .bits files
        for p in bits_paths:
            try:
                os.remove(p)
            except OSError:
                pass

        # Upload tile PNG
        tile_key = f"renders/{job_id}/bilevel_t{tile_idx:04d}.png"
        with open(out_path, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, tile_key)
        os.remove(out_path)

        report_status(job_id, task_id, "done")
        return ok_response({
            "tile_idx": tile_idx,
            "pixels_set": meta.get("pixels_set", 0),
            "file_size": meta.get("file_size", 0),
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise


def handle_stitch(params):
    """Join tile PNGs → final image PNG. Single Lambda invocation."""
    job_id = params["job_id"]
    n_tile_cols = params["n_tile_cols"]
    n_tile_rows = params["n_tile_rows"]
    n_tiles = n_tile_cols * n_tile_rows
    out_key = params["out_key"]
    task_id = "bilevel_stitch"

    try:
        report_status(job_id, task_id, "started")

        # Download all tile PNGs
        tile_paths = []
        for t in range(n_tiles):
            tile_key = f"renders/{job_id}/bilevel_t{t:04d}.png"
            local_path = f"/tmp/tile_{t:04d}.png"
            obj = s3.get_object(Bucket=BUCKET, Key=tile_key)
            with open(local_path, "wb") as f:
                for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                    f.write(chunk)
            tile_paths.append(local_path)

        report_status(job_id, task_id, "stitching")

        # Run bilevel_merge stitch
        out_path = "/tmp/final.png"
        cmd = [
            BILEVEL_MERGE, "stitch",
            f"--n_cols={n_tile_cols}",
            f"--n_rows={n_tile_rows}",
            f"--output={out_path}",
        ] + tile_paths

        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=600,
            env=imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"bilevel_merge stitch failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)

        # Clean up tile PNGs
        for p in tile_paths:
            try:
                os.remove(p)
            except OSError:
                pass

        # Upload final PNG
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=out_key, Body=f, ContentType="image/png")
        os.remove(out_path)

        report_status(job_id, task_id, "done")
        return ok_response({
            "out_key": out_key,
            "file_size": meta.get("file_size", 0),
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
