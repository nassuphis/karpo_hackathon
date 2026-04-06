"""
Bilevel Lambda handler — raster and merge phases for chunk-first bilevel rendering.

  phase=raster: one chunk → per-tile bitset files (.bits)
  phase=merge:  per-tile bitsets → tile TIFF (1-bit CCITT G4, via libvips)

Stitch phase is handled by a separate Lambda (handler_bilevel_stitch.py)
with higher memory for libvips multithreaded encode.
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
COEFFS_BILEVEL_RASTER = os.path.join(os.path.dirname(__file__), "coeffs_bilevel_raster")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")


def handler(event, context):
    params = parse_body(event)
    phase = params["phase"]

    if phase == "raster":
        return handle_raster(params)
    elif phase == "coeff_raster":
        return handle_coeff_raster(params)
    elif phase == "merge":
        return handle_merge(params)
    else:
        raise ValueError(f"Unknown bilevel phase: {phase}")


def handle_raster(params):
    """One chunk → per-tile bitset files. One Lambda per chunk."""
    job_id = params["job_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    if chunk_idx is None:
        raise RuntimeError("bilevel raster requires chunk_idx")
    task_id = params.get("task_id", f"bilevel_raster_{chunk_idx}")

    try:
        report_status(job_id, task_id, "started")

        # Download one chunk .bin
        bin_key = f"renders/{job_id}/chunk_{chunk_idx}.bin"
        bin_path = "/tmp/stripe.bin"
        t0 = time.time()
        try:
            obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
        except Exception as e:
            raise RuntimeError(f"Failed to download root chunk s3://{BUCKET}/{bin_key}: {e}") from e
        with open(bin_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)
        report_status(job_id, task_id, "bin_downloaded")

        # Clean stale .bits files from previous invocations (warm container reuse)
        import glob
        for stale in glob.glob("/tmp/bits_t*.bits"):
            os.remove(stale)

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
        # Write root transforms sidecar if present
        rt_chain = params.get("root_transforms", [])
        if rt_chain:
            rt_path = "/tmp/root_xforms.json"
            with open(rt_path, "w") as rtf:
                rtf.write(json.dumps(rt_chain))
            cmd.append(f"--root_xforms={rt_path}")

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
                s3_key = f"renders/{job_id}/bits_chunk_{chunk_idx:04d}_t{t:04d}.bits"
                with open(bits_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, s3_key)
                os.remove(bits_path)
                uploaded += 1
            elif os.path.exists(bits_path):
                os.remove(bits_path)

        report_status(job_id, task_id, "done")
        return ok_response({
            "chunk_idx": chunk_idx,
            "stripe_idx": chunk_idx,
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


def handle_coeff_raster(params):
    """One coeff chunk → per-tile bitset files. One Lambda per chunk."""
    job_id = params["job_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    if chunk_idx is None:
        raise RuntimeError("coeff bilevel raster requires chunk_idx")
    task_id = params.get("task_id", f"coeff_bilevel_raster_{chunk_idx}")

    try:
        report_status(job_id, task_id, "started")

        # Download one coeff chunk .bin
        bin_key = params.get("coeffs_key", f"renders/{job_id}/coeffs_{chunk_idx:04d}.bin")
        bin_path = "/tmp/coeffs.bin"
        t0 = time.time()
        try:
            obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
        except Exception as e:
            raise RuntimeError(f"Failed to download coeffs chunk s3://{BUCKET}/{bin_key}: {e}") from e
        with open(bin_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)
        report_status(job_id, task_id, "bin_downloaded")

        # Clean stale .bits files from previous invocations (warm container reuse)
        import glob
        for stale in glob.glob("/tmp/coeff_bits_t*.bits"):
            os.remove(stale)

        # Run coeffs_bilevel_raster
        out_prefix = "/tmp/coeff_bits"
        cmd = [
            COEFFS_BILEVEL_RASTER, bin_path, out_prefix,
            f"--width={params['width']}", f"--height={params['height']}",
            f"--tile_size={params['tile_size']}",
            f"--n_tile_cols={params['n_tile_cols']}",
            f"--n_tile_rows={params['n_tile_rows']}",
            f"--center_re={params['center_re']}",
            f"--center_im={params['center_im']}",
            f"--scale={params['scale']}",
            f"--n_coeffs={params['n_coeffs']}",
            f"--rotation={params.get('rotation', 0.0)}",
        ]
        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            raise RuntimeError(f"coeffs_bilevel_raster failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        raster_ms = int((time.time() - t1) * 1000)

        os.remove(bin_path)

        # Upload non-empty .bits files with coeff-specific naming
        n_tiles = params['n_tile_cols'] * params['n_tile_rows']
        uploaded = 0
        for t in range(n_tiles):
            bits_path = f"/tmp/coeff_bits_t{t:04d}.bits"
            if os.path.exists(bits_path) and os.path.getsize(bits_path) > 0:
                s3_key = f"renders/{job_id}/coeff_bits_chunk_{chunk_idx:04d}_t{t:04d}.bits"
                with open(bits_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, s3_key)
                os.remove(bits_path)
                uploaded += 1
            elif os.path.exists(bits_path):
                os.remove(bits_path)

        report_status(job_id, task_id, "done")
        return ok_response({
            "chunk_idx": chunk_idx,
            "stripe_idx": chunk_idx,
            "tiles_with_hits": uploaded,
            "roots_plotted": meta["roots_plotted"],
            "roots_clipped": meta["roots_clipped"],
            "dl_ms": dl_ms,
            "raster_ms": raster_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        for p in ["/tmp/coeffs.bin"]:
            try:
                os.remove(p)
            except OSError:
                pass
        raise


def handle_merge(params):
    """OR per-chunk bitsets for one tile → tile TIFF. One Lambda per tile."""
    job_id = params["job_id"]
    tile_idx = params["tile_idx"]
    tile_w = params["tile_w"]
    tile_h = params["tile_h"]
    n_chunks = params.get("n_chunks", params.get("n_stripes"))
    if n_chunks is None:
        raise RuntimeError("bilevel merge requires n_chunks")
    # Support coeff vs root naming: coeff_bits_chunk_... vs bits_chunk_...
    bits_prefix = params.get("bits_prefix", "bits")
    tile_prefix = params.get("tile_prefix", "bilevel")
    task_prefix = params.get("task_prefix", "bilevel_merge")
    task_id = params.get("task_id", f"{task_prefix}_{tile_idx}")

    try:
        report_status(job_id, task_id, "started")

        # Download chunk .bits files for this tile
        bits_paths = []
        for c in range(n_chunks):
            bits_key = f"renders/{job_id}/{bits_prefix}_chunk_{c:04d}_t{tile_idx:04d}.bits"
            local_path = f"/tmp/bits_chunk_{c}.bits"
            try:
                try:
                    obj = s3.get_object(Bucket=BUCKET, Key=bits_key)
                except ClientError as e:
                    if e.response['Error']['Code'] != 'NoSuchKey':
                        raise
                    legacy_bits_key = f"renders/{job_id}/{bits_prefix}_s{c:04d}_t{tile_idx:04d}.bits"
                    obj = s3.get_object(Bucket=BUCKET, Key=legacy_bits_key)
                    bits_key = legacy_bits_key
                with open(local_path, "wb") as f:
                    f.write(obj["Body"].read())
                bits_paths.append(local_path)
            except ClientError as e:
                if e.response['Error']['Code'] == 'NoSuchKey':
                    continue  # No hits from this chunk for this tile
                raise
        report_status(job_id, task_id, "bits_downloaded")

        # Run bilevel_merge
        out_path = "/tmp/tile.tif"
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

        # Upload tile TIFF
        tile_key = f"renders/{job_id}/{tile_prefix}_t{tile_idx:04d}.tif"
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
