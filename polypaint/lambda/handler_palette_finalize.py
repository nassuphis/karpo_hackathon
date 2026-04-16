"""
Palette finalize Lambda — assemble pass-0 palette images while preserving durable
all-pass chunk-local palette data for later reuse.
"""
import json
import os
import subprocess
import time

import boto3

from solve_score_chain import emit_solve_score_metadata
from shared import (
    BUCKET,
    attach_contract_warnings,
    contract_param,
    parse_body,
    ok_response,
    report_status,
    imgpipe_env,
)

s3 = boto3.client("s3")
PALETTE_RENDER = os.path.join(os.path.dirname(__file__), "palette_bins_render")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")
PRESIGN_EXPIRY = 3600
S3_USER_METADATA_LIMIT_BYTES = 2048

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


def _parse_boolish(value, default=True):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _omega_display(enabled, omega):
    return f"w={omega:g}" if enabled else "w=off"


def _metadata_size_bytes(meta):
    total = 0
    for key, value in (meta or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


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


def _read_palette_bin_prefix(key, n_bytes):
    kwargs = {"Bucket": BUCKET, "Key": key}
    if n_bytes is not None:
        if n_bytes < 1:
            return b""
        kwargs["Range"] = f"bytes=0-{n_bytes - 1}"
    obj = s3.get_object(**kwargs)
    return obj["Body"].read()


def _copy_pass0_chunk_rows(dst, src, *, full_n, step_start):
    """Copy a contiguous pass-0 chunk into the serpentine image buffer."""
    src_len = len(src)
    if src_len == 0:
        return 0
    if step_start < 0:
        raise RuntimeError(f"Chunk writes negative solve index at {step_start}")

    g = step_start
    src_pos = 0
    end = step_start + src_len
    while g < end:
        row = g // full_n
        row_start = row * full_n
        row_end = row_start + full_n
        seg_end = min(end, row_end)
        seg_len = seg_end - g
        j0 = g - row_start
        j1 = j0 + seg_len
        segment = src[src_pos:src_pos + seg_len]
        if row % 2 == 0:
            dst0 = row_start + j0
            dst[dst0:dst0 + seg_len] = segment
        else:
            dst0 = row_start + (full_n - j1)
            dst1 = row_start + (full_n - j0)
            dst[dst0:dst1] = segment[::-1]
        src_pos += seg_len
        g = seg_end
    return src_len


def handler(event, context):
    params = parse_body(event)
    contract_warnings = []
    job_id = params["job_id"]
    task_id = params["task_id"]
    palette_id = params["palette_id"]
    full_n = int(params["N"])
    times = int(contract_param(params, "times", 1, contract_warnings) or 1)
    degree = int(params["degree"])
    metric = params["metric"]
    palette = params["palette"]
    q = contract_param(params, "solve_score_quantile", 0.001, contract_warnings)
    omega = float(contract_param(params, "solve_score_omega", 1.0, contract_warnings))
    omega_enabled = _parse_boolish(contract_param(params, "solve_score_omega_enabled", True, contract_warnings), True)
    solve_score_chain = contract_param(params, "solve_score_chain", "", contract_warnings)
    render_execution = contract_param(params, "render_execution", None, contract_warnings)
    root_transforms = contract_param(params, "root_transforms", [], contract_warnings)
    image_key = params["image_key"]
    preview_key = params["preview_key"]
    meta_key = params["meta_key"]
    chunks_prefix = params["chunks_prefix"]
    solve_score_prefix = params["solve_score_prefix"]
    solve_score_clip_key = params["solve_score_clip_key"]
    solve_score_bins_key = params["solve_score_bins_key"]
    cleanup_solve_score_scratch = _parse_boolish(
        contract_param(params, "cleanup_solve_score_scratch", True, contract_warnings),
        True,
    )
    chunk_scores_prefix = params.get("chunk_scores_prefix", chunks_prefix + "score_chunk_")
    chunk_bins_prefix = params.get("chunk_bins_prefix", chunks_prefix + "palette_bins_chunk_")
    chunk_meta_prefix = params.get("chunk_meta_prefix", chunks_prefix + "meta_chunk_")
    source_color_artifact_id = str(params.get("source_color_artifact_id") or "").strip()

    progress = attach_contract_warnings({"phase": "palette_finalize", "palette_id": palette_id}, contract_warnings)
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
        pass0_chunks_read = 0
        pass0_chunks_skipped = 0
        pass0_bytes_read = 0
        for meta in chunk_meta:
            step_start = int(meta["step_start"])
            step_count = int(meta["step_count"])
            if step_start < 0:
                raise RuntimeError(f"Chunk {meta.get('chunk_idx')} writes negative solve index at {step_start}")
            pass0_count = max(0, min(step_count, pass0_steps - step_start))
            if pass0_count <= 0:
                pass0_chunks_skipped += 1
                continue
            bin_key = meta["palette_bins_key"]
            range_count = pass0_count if pass0_count < step_count else None
            bin_bytes = _read_palette_bin_prefix(bin_key, range_count)
            expected_len = pass0_count if range_count is not None else step_count
            if len(bin_bytes) != expected_len:
                raise RuntimeError(f"Chunk {meta.get('chunk_idx')} bin length {len(bin_bytes)} != {expected_len}")
            if range_count is None:
                bin_bytes = bin_bytes[:pass0_count]

            filled += _copy_pass0_chunk_rows(bins, bin_bytes, full_n=full_n, step_start=step_start)
            pass0_chunks_read += 1
            pass0_bytes_read += len(bin_bytes)

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
        if _parse_boolish(bins_meta.get("omega_enabled", True), True) != omega_enabled:
            raise RuntimeError(f"Solve-score bins omega_enabled mismatch: expected {omega_enabled}, got {bins_meta.get('omega_enabled')}")
        assemble_ms = int((time.time() - t0) * 1000)
        assemble_stats = {
            "assemble_ms": assemble_ms,
            "pass0_chunks_read": pass0_chunks_read,
            "pass0_chunks_skipped": pass0_chunks_skipped,
            "pass0_chunk_count": len(chunk_meta),
            "pass0_bytes_read": pass0_bytes_read,
        }
        report_status(
            job_id,
            task_id,
            "assembled",
            result_data=attach_contract_warnings({**progress, **assemble_stats}, contract_warnings),
        )

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
            "palette": palette,
            "full_n": str(full_n),
            "times": str(times),
            "using_pass": "0",
            "clip_lo": str(bins_meta.get("clip_lo", "")),
            "clip_hi": str(bins_meta.get("clip_hi", "")),
        }
        metadata_size = _metadata_size_bytes(metadata)
        if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
            raise RuntimeError(
                f"Palette image metadata too large before upload: {metadata_size} bytes > "
                f"{S3_USER_METADATA_LIMIT_BYTES} limit"
            )
        with open(_TMP_JPEG, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, image_key, ExtraArgs={"ContentType": "image/jpeg", "Metadata": metadata})
        with open(_TMP_PREVIEW, "rb") as pf:
            s3.upload_fileobj(pf, BUCKET, preview_key, ExtraArgs={"ContentType": "image/png"})

        created_at = _utc_now_iso()
        meta_body = {
            "job_id": job_id,
            "palette_id": palette_id,
            "created_at": created_at,
            "display_name": f"{metric} q={(float(q) * 100):.1f}% {_omega_display(omega_enabled, omega)} {palette} {created_at}",
            "palette": palette,
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
        if isinstance(render_execution, dict):
            meta_body["render_execution"] = render_execution
        meta_body.update(
            {
                "metric": metric,
                "solve_score_quantile": float(q),
                "solve_score_omega": omega,
                "solve_score_omega_enabled": omega_enabled,
                "solve_score_chain": json.loads(
                    emit_solve_score_metadata(
                        "solve",
                        metric=metric,
                        quantile=q,
                        omega=omega,
                        omega_enabled=omega_enabled,
                        chain=solve_score_chain,
                    )["solve_score_chain"]
                ),
            }
        )
        if source_color_artifact_id:
            meta_body["derived_from_color_artifact_id"] = source_color_artifact_id
            meta_body["derivation_kind"] = "extract_palette"
        s3.put_object(Bucket=BUCKET, Key=meta_key, Body=json.dumps(meta_body), ContentType="application/json")

        # Cleanup only when this workflow owns the solve-score scratch.
        if cleanup_solve_score_scratch:
            _delete_keys(_list_keys(solve_score_prefix) + [solve_score_clip_key, solve_score_bins_key])

        result_data = attach_contract_warnings({
            "palette_id": palette_id,
            "image_key": image_key,
            "preview_key": preview_key,
            "file_size": file_size,
            **assemble_stats,
            "render_ms": render_ms,
            "encode_ms": encode_ms,
        }, contract_warnings)
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()
