"""
Palette finalize Lambda — assemble pass-0 palette images while preserving durable
all-pass chunk-local palette data for later reuse.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
PALETTE_RENDER = os.path.join(os.path.dirname(__file__), "palette_bins_render")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")
PRESIGN_EXPIRY = 3600

_TMP_BINS = "/tmp/palette_bins_full.bin"
_TMP_RAW = "/tmp/palette_image.raw"
_TMP_JPEG = "/tmp/palette_image.jpeg"
_TMP_PREVIEW = "/tmp/palette_preview.png"


def _cleanup_tmp():
    for p in (_TMP_BINS, _TMP_RAW, _TMP_JPEG, _TMP_PREVIEW):
        try:
            os.remove(p)
        except OSError:
            pass


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _delete_keys(keys):
    keys = [k for k in keys if k]
    for i in range(0, len(keys), 1000):
        batch = keys[i:i + 1000]
        if not batch:
            continue
        s3.delete_objects(Bucket=BUCKET, Delete={"Objects": [{"Key": k} for k in batch]})


def _list_keys(prefix):
    keys = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        keys.extend(obj["Key"] for obj in page.get("Contents", []))
    return keys


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    task_id = params["task_id"]
    palette_id = params["palette_id"]
    full_n = int(params["N"])
    times = int(params.get("times", 1) or 1)
    degree = int(params["degree"])
    metric = params["metric"]
    palette = params["palette"]
    q = params["solve_score_quantile"]
    omega = float(params.get("solve_score_omega", 1.0))
    root_transforms = params.get("root_transforms", [])
    image_key = params["image_key"]
    preview_key = params["preview_key"]
    meta_key = params["meta_key"]
    chunks_prefix = params["chunks_prefix"]
    solve_score_prefix = params["solve_score_prefix"]
    solve_score_clip_key = params["solve_score_clip_key"]
    solve_score_bins_key = params["solve_score_bins_key"]
    chunk_scores_prefix = params.get("chunk_scores_prefix", chunks_prefix + "score_chunk_")
    chunk_bins_prefix = params.get("chunk_bins_prefix", chunks_prefix + "palette_bins_chunk_")
    chunk_meta_prefix = params.get("chunk_meta_prefix", chunks_prefix + "meta_chunk_")

    progress = {"phase": "palette_finalize", "palette_id": palette_id}
    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        t0 = time.time()
        meta_keys = [k for k in _list_keys(chunk_meta_prefix) if k.endswith(".json")]
        if not meta_keys:
            raise RuntimeError(f"No chunk metadata found under {chunks_prefix}")

        chunk_meta = []
        for key in meta_keys:
            obj = s3.get_object(Bucket=BUCKET, Key=key)
            chunk_meta.append(json.loads(obj["Body"].read()))
        chunk_meta.sort(key=lambda m: (m.get("step_start", 0), m.get("chunk_idx", 0)))

        pass0_steps = full_n * full_n
        bins = bytearray(pass0_steps)

        filled = 0
        for meta in chunk_meta:
            step_start = int(meta["step_start"])
            step_count = int(meta["step_count"])
            bin_obj = s3.get_object(Bucket=BUCKET, Key=meta["palette_bins_key"])
            bin_bytes = bin_obj["Body"].read()
            if len(bin_bytes) != step_count:
                raise RuntimeError(f"Chunk {meta.get('chunk_idx')} bin length {len(bin_bytes)} != {step_count}")

            for off in range(step_count):
                g = step_start + off
                if g < 0:
                    raise RuntimeError(f"Chunk {meta.get('chunk_idx')} writes negative solve index at {g}")
                if g >= pass0_steps:
                    continue
                row = g // full_n
                j = g % full_n
                col = j if (row % 2 == 0) else (full_n - 1 - j)
                idx = row * full_n + col
                bins[idx] = bin_bytes[off]
                filled += 1

        if filled != pass0_steps:
            raise RuntimeError(f"Palette finalize filled {filled} samples, expected {pass0_steps}")

        with open(_TMP_BINS, "wb") as bf:
            bf.write(bins)

        bins_obj = s3.get_object(Bucket=BUCKET, Key=solve_score_bins_key)
        bins_meta = json.loads(bins_obj["Body"].read())
        clip_obj = s3.get_object(Bucket=BUCKET, Key=solve_score_clip_key)
        clip_meta = json.loads(clip_obj["Body"].read())
        if float(bins_meta.get("omega", 1.0)) != omega:
            raise RuntimeError(f"Solve-score bins omega mismatch: expected {omega}, got {bins_meta.get('omega')}")
        assemble_ms = int((time.time() - t0) * 1000)
        report_status(job_id, task_id, "assembled", result_data={**progress, "assemble_ms": assemble_ms})

        t1 = time.time()
        env = imgpipe_env()
        result = subprocess.run(
            [PALETTE_RENDER, _TMP_BINS, _TMP_RAW, f"--n={full_n}", f"--palette={palette}"],
            capture_output=True, text=True, timeout=300,
        )
        render_ms = int((time.time() - t1) * 1000)
        if result.returncode != 0:
            raise RuntimeError(f"palette_bins_render failed: {result.stderr.strip()}")

        t2 = time.time()
        enc = subprocess.run(
            [RAW2JPEG, _TMP_RAW, _TMP_JPEG, "--quality=90"],
            capture_output=True, text=True, timeout=300, env=env,
        )
        encode_ms = int((time.time() - t2) * 1000)
        if enc.returncode != 0:
            raise RuntimeError(f"raw2jpeg failed: {enc.stderr.strip()}")

        vt_path = "/opt/bin/vipsthumbnail"
        prev = subprocess.run(
            [vt_path, _TMP_JPEG, "-s", "512x512", "-o", _TMP_PREVIEW + "[strip]"],
            capture_output=True, text=True, timeout=60, env=env,
        )
        if prev.returncode != 0:
            raise RuntimeError(f"Preview generation failed: {prev.stderr.strip()}")

        file_size = os.path.getsize(_TMP_JPEG)
        metadata = {
            "width": str(full_n),
            "height": str(full_n),
            "metric": metric,
            "palette": palette,
            "solve_score_quantile": str(q),
            "solve_score_omega": str(omega),
            "full_n": str(full_n),
            "times": str(times),
            "using_pass": "0",
            "clip_lo": str(bins_meta.get("clip_lo", "")),
            "clip_hi": str(bins_meta.get("clip_hi", "")),
        }
        with open(_TMP_JPEG, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, image_key, ExtraArgs={"ContentType": "image/jpeg", "Metadata": metadata})
        with open(_TMP_PREVIEW, "rb") as pf:
            s3.upload_fileobj(pf, BUCKET, preview_key, ExtraArgs={"ContentType": "image/png"})

        created_at = _utc_now_iso()
        meta_body = {
            "job_id": job_id,
            "palette_id": palette_id,
            "created_at": created_at,
            "display_name": f"{metric} q={(float(q) * 100):.1f}% w={omega:g} {palette} {created_at}",
            "metric": metric,
            "palette": palette,
            "solve_score_quantile": float(q),
            "solve_score_omega": omega,
            "root_transforms": root_transforms or [],
            "degree": degree,
            "N": full_n,
            "times": times,
            "using_pass": 0,
            "image_pass": 0,
            "base_grid_solves": pass0_steps,
            "total_solves": sum(int(m.get("step_count", 0)) for m in chunk_meta),
            "pass_count": times,
            "data_layout": "chunk_all_pass_v1",
            "render_reusable": True,
            "clip_lo": bins_meta.get("clip_lo"),
            "clip_hi": bins_meta.get("clip_hi"),
            "cuts_norm": bins_meta.get("cuts_norm", []),
            "clip_fallback": clip_meta.get("clip_fallback"),
            "clip_fallback_reason": clip_meta.get("clip_fallback_reason"),
            "file_size": file_size,
            "image_key": image_key,
            "preview_key": preview_key,
            "chunk_scores_prefix": chunk_scores_prefix,
            "chunk_bins_prefix": chunk_bins_prefix,
            "chunk_meta_prefix": chunk_meta_prefix,
        }
        s3.put_object(Bucket=BUCKET, Key=meta_key, Body=json.dumps(meta_body), ContentType="application/json")

        # Cleanup only this workflow's temporary solve-score scratch after success.
        _delete_keys(_list_keys(solve_score_prefix) + [solve_score_clip_key, solve_score_bins_key])

        result_data = {
            "palette_id": palette_id,
            "image_key": image_key,
            "preview_key": preview_key,
            "file_size": file_size,
            "assemble_ms": assemble_ms,
            "render_ms": render_ms,
            "encode_ms": encode_ms,
        }
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()
