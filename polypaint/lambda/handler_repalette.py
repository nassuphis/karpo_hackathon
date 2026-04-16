"""
RePalette Lambda — derive a new immutable palette artifact from an existing saved
palette artifact by reusing its persisted bin assignment.

New reusable palette artifacts copy their chunk-local all-pass numeric payload into
the new prefix so each artifact remains self-contained. Legacy pass-0-only palette
artifacts can still be repaletted, but remain non-reusable for GenerateFromPalette.
"""
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import os
import subprocess
import time

import boto3

from palette_names import VALID_PALETTE_NAMES
from shared import BUCKET, parse_body, ok_response, parse_boolish, report_status, imgpipe_env

s3 = boto3.client("s3")
PALETTE_RENDER = os.path.join(os.path.dirname(__file__), "palette_bins_render")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")

_TMP_BINS = "/tmp/repalette_bins.bin"
_TMP_RAW = "/tmp/repalette_image.raw"
_TMP_JPEG = "/tmp/repalette_image.jpeg"
_TMP_PREVIEW = "/tmp/repalette_preview.png"
_DEFAULT_COPY_WORKERS = 16
S3_USER_METADATA_LIMIT_BYTES = 2048


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _cleanup_tmp():
    for p in (_TMP_BINS, _TMP_RAW, _TMP_JPEG, _TMP_PREVIEW):
        try:
            os.remove(p)
        except OSError:
            pass


def _omega_display(enabled, omega):
    return f"w={omega:g}" if enabled else "w=off"


def _variant_id(metric, palette, q, omega, omega_enabled):
    q_label = f"{float(q) * 100:.1f}".replace(".", "p")
    if omega_enabled:
        omega_f = float(omega)
        omega_label = f"{omega_f:.0f}" if omega_f.is_integer() else str(omega_f).replace(".", "p")
    else:
        omega_label = "off"
    return f"pal_{int(time.time() * 1000)}_{metric}_{palette}_q{q_label}_w{omega_label}"


def _list_keys(prefix):
    keys = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        keys.extend(obj["Key"] for obj in page.get("Contents", []))
    return keys


def _copy_key(src_key, dst_key):
    s3.copy_object(
        Bucket=BUCKET,
        CopySource={"Bucket": BUCKET, "Key": src_key},
        Key=dst_key,
        MetadataDirective="COPY",
    )


def _copy_worker_count(total_chunks):
    raw = os.environ.get("REPALETTE_COPY_WORKERS", str(_DEFAULT_COPY_WORKERS))
    try:
        workers = int(raw)
    except (TypeError, ValueError):
        workers = _DEFAULT_COPY_WORKERS
    workers = max(1, workers)
    return min(total_chunks, workers)


def _metadata_size_bytes(metadata):
    total = 0
    for key, value in (metadata or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


def _palette_image_metadata(full_n, palette):
    return {
        "width": str(full_n),
        "height": str(full_n),
        "palette": str(palette),
    }


def _load_palette_meta(job_id, source_palette_id):
    key = f"renders/{job_id}/palettes/{source_palette_id}/meta.json"
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    meta = json.loads(obj["Body"].read())
    if meta.get("job_id") and meta.get("job_id") != job_id:
        raise RuntimeError(f"Palette artifact {source_palette_id} belongs to {meta.get('job_id')}, not {job_id}")
    return meta


def _assemble_pass0_bins_from_chunk_meta(chunk_meta, full_n):
    pass0_steps = full_n * full_n
    bins = bytearray(pass0_steps)
    filled = 0
    for meta in chunk_meta:
        step_start = int(meta["step_start"])
        step_count = int(meta["step_count"])
        bin_obj = s3.get_object(Bucket=BUCKET, Key=meta["palette_bins_key"])
        bin_bytes = bin_obj["Body"].read()
        if len(bin_bytes) != step_count:
            raise RuntimeError(
                f"Chunk {meta.get('chunk_idx')} bin length {len(bin_bytes)} != {step_count}"
            )
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
        raise RuntimeError(f"RePalette assembled {filled} pass-0 samples, expected {pass0_steps}")
    return bins


def _copy_reusable_chunk(source_job_id, chunk_scores_prefix, chunk_bins_prefix, chunk_meta_prefix, key):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    meta = json.loads(obj["Body"].read())
    idx = int(meta["chunk_idx"])
    new_score_key = f"{chunk_scores_prefix}{idx}.bin"
    new_bins_key = f"{chunk_bins_prefix}{idx}.bin"
    new_meta_key = f"{chunk_meta_prefix}{idx}.json"

    _copy_key(meta["score_key"], new_score_key)
    _copy_key(meta["palette_bins_key"], new_bins_key)

    new_meta = dict(meta)
    new_meta.update({
        "job_id": source_job_id,
        "score_key": new_score_key,
        "palette_bins_key": new_bins_key,
    })
    s3.put_object(
        Bucket=BUCKET,
        Key=new_meta_key,
        Body=json.dumps(new_meta),
        ContentType="application/json",
    )
    return new_meta


def _copy_reusable_chunk_payload(source_meta, new_prefix, job_id=None, task_id=None, progress=None):
    source_meta_prefix = source_meta.get("chunk_meta_prefix")
    if not source_meta_prefix:
        raise RuntimeError("Reusable palette artifact missing chunk_meta_prefix")

    source_chunk_meta_keys = [k for k in _list_keys(source_meta_prefix) if k.endswith(".json")]
    if not source_chunk_meta_keys:
        raise RuntimeError(f"No chunk metadata found under {source_meta_prefix}")

    chunk_scores_prefix = new_prefix + "chunks/score_chunk_"
    chunk_bins_prefix = new_prefix + "chunks/palette_bins_chunk_"
    chunk_meta_prefix = new_prefix + "chunks/meta_chunk_"
    chunk_meta = []
    total_chunks = len(source_chunk_meta_keys)
    workers = _copy_worker_count(total_chunks)

    with ThreadPoolExecutor(max_workers=workers) as ex:
        futures = [
            ex.submit(
                _copy_reusable_chunk,
                source_meta["job_id"],
                chunk_scores_prefix,
                chunk_bins_prefix,
                chunk_meta_prefix,
                key,
            )
            for key in source_chunk_meta_keys
        ]
        for chunk_no, fut in enumerate(as_completed(futures), start=1):
            chunk_meta.append(fut.result())
            if job_id and task_id and progress and (chunk_no == 1 or chunk_no == total_chunks or chunk_no % 8 == 0):
                report_status(
                    job_id,
                    task_id,
                    "copying",
                    result_data={**progress, "phase_label": f"Copy numeric data {chunk_no}/{total_chunks}"},
                )

    chunk_meta.sort(key=lambda m: (int(m.get("step_start", 0)), int(m.get("chunk_idx", 0))))
    return {
        "chunk_scores_prefix": chunk_scores_prefix,
        "chunk_bins_prefix": chunk_bins_prefix,
        "chunk_meta_prefix": chunk_meta_prefix,
        "chunk_meta": chunk_meta,
    }


def _copy_legacy_pass0_payload(source_meta, metric, new_prefix):
    palette_bins_key = source_meta.get("palette_bins_key")
    if not palette_bins_key:
        raise RuntimeError(
            "Selected palette artifact has no reusable bin data. Regenerate it before using RePalette."
        )
    new_bins_key = new_prefix + "palette_bins.bin"
    _copy_key(palette_bins_key, new_bins_key)
    score_key = source_meta.get("score_key")
    new_score_key = None
    if score_key:
        new_score_key = new_prefix + f"score_{metric}.bin"
        _copy_key(score_key, new_score_key)
    bin_obj = s3.get_object(Bucket=BUCKET, Key=palette_bins_key)
    bins = bin_obj["Body"].read()
    return {
        "palette_bins_key": new_bins_key,
        "score_key": new_score_key,
        "bins": bins,
    }


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    task_id = params["task_id"]
    source_palette_id = params["source_palette_id"]
    new_palette = str(params["new_palette"]).strip()
    if new_palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {new_palette}")

    progress = {"phase": "repalette", "family": "palette", "source_palette_id": source_palette_id}
    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        source_meta = _load_palette_meta(job_id, source_palette_id)
        metric = source_meta.get("metric", "proximity")
        q = float(source_meta.get("solve_score_quantile", 0.001))
        omega = float(source_meta.get("solve_score_omega", 1.0))
        omega_enabled = parse_boolish(source_meta.get("solve_score_omega_enabled", True), True)
        full_n = int(source_meta.get("N", 0) or 0)
        if full_n <= 0:
            raise RuntimeError(f"Palette artifact {source_palette_id} missing valid N")

        created_at = _utc_now_iso()
        new_palette_id = _variant_id(metric, new_palette, q, omega, omega_enabled)
        new_prefix = f"renders/{job_id}/palettes/{new_palette_id}/"
        image_key = new_prefix + "image.jpeg"
        preview_key = new_prefix + "preview.png"
        meta_key = new_prefix + "meta.json"

        reusable = source_meta.get("data_layout") == "chunk_all_pass_v1" and parse_boolish(source_meta.get("render_reusable"), False)
        if reusable:
            report_status(job_id, task_id, "copying", result_data={**progress, "phase_label": "Copy numeric data"})
            copied = _copy_reusable_chunk_payload(source_meta, new_prefix, job_id=job_id, task_id=task_id, progress=progress)
            bins = _assemble_pass0_bins_from_chunk_meta(copied["chunk_meta"], full_n)
            chunk_scores_prefix = copied["chunk_scores_prefix"]
            chunk_bins_prefix = copied["chunk_bins_prefix"]
            chunk_meta_prefix = copied["chunk_meta_prefix"]
            score_key = None
            palette_bins_key = None
            render_reusable = True
            data_layout = "chunk_all_pass_v1"
            total_solves = int(source_meta.get("total_solves") or sum(int(m.get("step_count", 0)) for m in copied["chunk_meta"]))
        else:
            report_status(job_id, task_id, "copying", result_data={**progress, "phase_label": "Copy pass-0 data"})
            copied = _copy_legacy_pass0_payload(source_meta, metric, new_prefix)
            bins = copied["bins"]
            score_key = copied["score_key"]
            palette_bins_key = copied["palette_bins_key"]
            chunk_scores_prefix = ""
            chunk_bins_prefix = ""
            chunk_meta_prefix = ""
            render_reusable = False
            data_layout = source_meta.get("data_layout") or "legacy_pass0_v1"
            total_solves = int(source_meta.get("total_solves") or (full_n * full_n * int(source_meta.get("times", 1) or 1)))
            if len(bins) != full_n * full_n:
                raise RuntimeError(
                    f"Legacy palette bin length {len(bins)} does not match pass-0 image size {full_n * full_n}"
                )

        with open(_TMP_BINS, "wb") as bf:
            bf.write(bins)

        report_status(job_id, task_id, "rendering", result_data={**progress, "phase_label": "Render image"})
        render = subprocess.run(
            [PALETTE_RENDER, _TMP_BINS, _TMP_RAW, f"--n={full_n}", f"--palette={new_palette}"],
            capture_output=True,
            text=True,
            timeout=300,
        )
        if render.returncode != 0:
            raise RuntimeError(f"palette_bins_render failed: {render.stderr.strip()}")

        enc = subprocess.run(
            [RAW2JPEG, _TMP_RAW, _TMP_JPEG, "--quality=90"],
            capture_output=True,
            text=True,
            timeout=300,
            env=imgpipe_env(),
        )
        if enc.returncode != 0:
            raise RuntimeError(f"raw2jpeg failed: {enc.stderr.strip()}")

        prev = subprocess.run(
            ["/opt/bin/vipsthumbnail", _TMP_JPEG, "-s", "512x512", "-o", _TMP_PREVIEW + "[strip]"],
            capture_output=True,
            text=True,
            timeout=120,
            env=imgpipe_env(),
        )
        if prev.returncode != 0:
            raise RuntimeError(f"Preview generation failed: {prev.stderr.strip()}")

        file_size = os.path.getsize(_TMP_JPEG)
        metadata = _palette_image_metadata(full_n, new_palette)
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

        meta_body = {
            "job_id": job_id,
            "palette_id": new_palette_id,
            "created_at": created_at,
            "display_name": f"{metric} q={(q * 100):.1f}% {_omega_display(omega_enabled, omega)} {new_palette} {created_at}",
            "metric": metric,
            "palette": new_palette,
            "solve_score_quantile": q,
            "solve_score_omega": omega,
            "solve_score_omega_enabled": omega_enabled,
            "root_transforms": source_meta.get("root_transforms") or [],
            "degree": source_meta.get("degree"),
            "N": full_n,
            "times": int(source_meta.get("times", 1) or 1),
            "using_pass": 0,
            "image_pass": 0,
            "base_grid_solves": int(source_meta.get("base_grid_solves") or (full_n * full_n)),
            "total_solves": total_solves,
            "pass_count": int(source_meta.get("pass_count") or source_meta.get("times", 1) or 1),
            "data_layout": data_layout,
            "render_reusable": render_reusable,
            "clip_lo": source_meta.get("clip_lo"),
            "clip_hi": source_meta.get("clip_hi"),
            "cuts_norm": source_meta.get("cuts_norm", []),
            "clip_fallback": source_meta.get("clip_fallback"),
            "clip_fallback_reason": source_meta.get("clip_fallback_reason"),
            "file_size": file_size,
            "image_key": image_key,
            "preview_key": preview_key,
            "derived_from_palette_id": source_palette_id,
            "derived_from_palette_created_at": source_meta.get("created_at"),
            "derived_from_palette_name": source_meta.get("display_name", ""),
            "derivation_kind": "repalette",
        }
        if render_reusable:
            meta_body["chunk_scores_prefix"] = chunk_scores_prefix
            meta_body["chunk_bins_prefix"] = chunk_bins_prefix
            meta_body["chunk_meta_prefix"] = chunk_meta_prefix
        else:
            if score_key:
                meta_body["score_key"] = score_key
            if palette_bins_key:
                meta_body["palette_bins_key"] = palette_bins_key

        s3.put_object(Bucket=BUCKET, Key=meta_key, Body=json.dumps(meta_body), ContentType="application/json")

        result_data = {
            "family": "palette",
            "artifact_id": new_palette_id,
            "palette_id": new_palette_id,
            "image_key": image_key,
            "preview_key": preview_key,
            "source_palette_id": source_palette_id,
            "palette": new_palette,
            "render_reusable": render_reusable,
        }
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()
