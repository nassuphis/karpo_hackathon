"""
Finalize-MT handler for the fused solve-score color path.

Consumes one global u32le_u8_v1 fragment per raster section, assembles the
canonical greyscale raw via assemble_greyscale, renders the final image and
preview through a single native score_raw_render pass, uploads the raw sidecar,
and writes color artifact metadata directly.
"""
from __future__ import annotations

import json
import os
import subprocess
import threading
import time

import boto3
from botocore.config import Config

from color_artifact_meta import split_color_artifact_metadata, write_color_artifact_meta_overlay
from raw_score_render import render_score_raw, write_equalization_lut
from raw_sidecar import background_color_hex, build_raw_sidecar
from shared import BUCKET, imgpipe_env, ok_response, parse_body, report_status
from solve_score_chain import read_solve_score_metadata


s3 = boto3.client("s3")
ASSEMBLE_GREYSCALE = os.path.join(os.path.dirname(__file__), "assemble_greyscale")
DEFAULT_FINALIZE_WORKERS = 16
MAX_FINALIZE_WORKERS = 64
ASSEMBLE_PROGRESS_INTERVAL_S = 20.0
S3_USER_METADATA_LIMIT_BYTES = 2048
FRAGMENT_MANIFEST_VERSION = 1
FRAGMENT_PAIR_ENCODING = "u32le_u8_v1"


def _validate_finalize_workers(value):
    if value in (None, ""):
        value = os.environ.get("FINALIZE_MT_WORKERS", DEFAULT_FINALIZE_WORKERS)
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"finalize_workers must be an integer, got {value!r}")
    if not (1 <= workers <= MAX_FINALIZE_WORKERS):
        raise RuntimeError(f"finalize_workers must be in [1, {MAX_FINALIZE_WORKERS}], got {workers}")
    return workers


def _validate_fragment_manifest(manifest, *, source_item_count, fragment_prefix, expected_chain_fingerprint):
    if not isinstance(manifest, dict):
        raise RuntimeError("FinalizeMT requires fragment_manifest")
    try:
        version = int(manifest.get("version") or 0)
    except (TypeError, ValueError):
        raise RuntimeError(f"fragment_manifest.version must be an integer, got {manifest.get('version')!r}")
    if version != FRAGMENT_MANIFEST_VERSION:
        raise RuntimeError(
            f"fragment_manifest.version must be {FRAGMENT_MANIFEST_VERSION}, got {version}"
        )
    pair_encoding = str(manifest.get("pair_encoding") or "").strip()
    if pair_encoding != FRAGMENT_PAIR_ENCODING:
        raise RuntimeError(
            f"fragment_manifest.pair_encoding must be {FRAGMENT_PAIR_ENCODING!r}, got {pair_encoding!r}"
        )
    try:
        manifest_item_count = int(manifest.get("item_count") or 0)
    except (TypeError, ValueError):
        raise RuntimeError(f"fragment_manifest.item_count must be an integer, got {manifest.get('item_count')!r}")
    if manifest_item_count != int(source_item_count):
        raise RuntimeError(
            f"fragment_manifest.item_count mismatch: expected {source_item_count}, got {manifest_item_count}"
        )
    manifest_prefix = str(manifest.get("fragment_prefix") or "").strip()
    if manifest_prefix != str(fragment_prefix or "").strip():
        raise RuntimeError(
            f"fragment_manifest.fragment_prefix mismatch: expected {fragment_prefix!r}, got {manifest_prefix!r}"
        )
    manifest_fingerprint = str(manifest.get("chain_fingerprint") or "").strip()
    if expected_chain_fingerprint and manifest_fingerprint != expected_chain_fingerprint:
        raise RuntimeError(
            "fragment_manifest.chain_fingerprint mismatch: "
            f"expected {expected_chain_fingerprint}, got {manifest_fingerprint!r}"
        )
    return {
        "version": version,
        "pair_encoding": pair_encoding,
        "item_count": manifest_item_count,
        "fragment_prefix": manifest_prefix,
        "chain_fingerprint": manifest_fingerprint,
    }


def _finalize_s3_client(max_workers):
    pool_size = max(16, int(max_workers) * 2)
    return boto3.client("s3", config=Config(max_pool_connections=pool_size))


def _fragment_key(fragment_prefix, section_idx):
    prefix = str(fragment_prefix or "").strip()
    if not prefix:
        raise RuntimeError("FinalizeMT requires fragment_prefix for fused fragments")
    return f"{prefix}{int(section_idx):04d}.frag"


def _step_scores_fragment_key(fragment_prefix, section_idx):
    prefix = str(fragment_prefix or "").strip()
    if not prefix:
        raise RuntimeError("FinalizeMT requires fragment_prefix for fused step scores")
    return f"{prefix}{int(section_idx):04d}_step_scores.raw"


def _write_url_manifest(path, urls):
    with open(path, "w", encoding="utf-8") as fh:
        for url in urls:
            fh.write(str(url).strip())
            fh.write("\n")
    return path


def _presign_fragment_urls(*, finalize_s3, fragment_prefix, source_item_count):
    urls = []
    for section_idx in range(int(source_item_count)):
        key = _fragment_key(fragment_prefix, section_idx)
        urls.append(
            finalize_s3.generate_presigned_url(
                "get_object",
                Params={"Bucket": BUCKET, "Key": key},
                ExpiresIn=900,
            )
        )
    return urls


def _assemble_greyscale_raw(*, pix, raw_path, hist_path, workers, fragment_urls, manifest_path, progress_cb=None):
    _write_url_manifest(manifest_path, fragment_urls)
    cmd = [
        ASSEMBLE_GREYSCALE,
        f"--pix={int(pix)}",
        f"--output={raw_path}",
        f"--hist-output={hist_path}",
        f"--workers={int(workers)}",
        f"--url-manifest={manifest_path}",
    ]
    stop_heartbeat = threading.Event()
    heartbeat_thread = None
    if progress_cb is not None and ASSEMBLE_PROGRESS_INTERVAL_S > 0:
        started_at = time.time()

        def _heartbeat():
            next_tick = started_at + ASSEMBLE_PROGRESS_INTERVAL_S
            while not stop_heartbeat.wait(max(0.0, next_tick - time.time())):
                try:
                    progress_cb(int((time.time() - started_at) * 1000))
                except Exception:
                    pass
                next_tick += ASSEMBLE_PROGRESS_INTERVAL_S

        heartbeat_thread = threading.Thread(target=_heartbeat, daemon=True)
        heartbeat_thread.start()
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True, timeout=600, env=imgpipe_env())
    finally:
        stop_heartbeat.set()
        if heartbeat_thread is not None:
            heartbeat_thread.join(timeout=1.0)
    if proc.returncode != 0:
        raise RuntimeError(f"assemble_greyscale failed: {proc.stderr.strip() or 'unknown error'}")
    with open(hist_path, "r", encoding="utf-8") as fh:
        hist = json.load(fh)
    histogram = hist.get("histogram")
    if not isinstance(histogram, list) or len(histogram) != 256:
        raise RuntimeError("assemble_greyscale histogram sidecar missing 256-bin histogram")
    return {
        "histogram": [int(v) for v in histogram],
        "background_pixels": int(hist.get("background_pixels") or 0),
        "nonzero_pixels": int(hist.get("nonzero_pixels") or 0),
    }


def _concat_step_scores(*, finalize_s3, fragment_prefix, source_item_count, out_path):
    total_bytes = 0
    with open(out_path, "wb") as out_fh:
        for section_idx in range(int(source_item_count)):
            key = _step_scores_fragment_key(fragment_prefix, section_idx)
            obj = finalize_s3.get_object(Bucket=BUCKET, Key=key)
            body = obj["Body"]
            if hasattr(body, "iter_chunks"):
                for chunk in body.iter_chunks(chunk_size=1024 * 1024):
                    total_bytes += len(chunk)
                    out_fh.write(chunk)
            else:
                chunk = body.read()
                total_bytes += len(chunk)
                out_fh.write(chunk)
    return total_bytes


def _metadata_size_bytes(meta):
    total = 0
    for key, value in (meta or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _normalize_clip_slots(clip_slots):
    if not isinstance(clip_slots, list) or not clip_slots:
        raise RuntimeError("FinalizeMT requires clip_slots from the clip phase payload")
    normalized = []
    for idx, row in enumerate(clip_slots):
        if not isinstance(row, dict):
            raise RuntimeError(f"clip_slots[{idx}] must be an object")
        try:
            normalized.append(
                {
                    "slot": int(row.get("slot", idx)),
                    "metric": str(row.get("metric") or "").strip(),
                    "source": str(row.get("source", "slv") or "slv").strip(),
                    "clip_lo": float(row["clip_lo"]),
                    "clip_hi": float(row["clip_hi"]),
                }
            )
        except (KeyError, TypeError, ValueError) as exc:
            raise RuntimeError(f"clip_slots[{idx}] is invalid: {row!r}") from exc
        if not normalized[-1]["metric"]:
            raise RuntimeError(f"clip_slots[{idx}].metric must be non-empty")
        if not normalized[-1]["source"]:
            raise RuntimeError(f"clip_slots[{idx}].source must be non-empty")
    return normalized


def _clip_info_from_payload(params, metadata):
    expected_fingerprint = str(
        metadata.get("solve_score_chain_fingerprint") or metadata.get("chain_fingerprint") or ""
    ).strip()
    actual_fingerprint = str(params.get("chain_fingerprint") or expected_fingerprint).strip()
    if not actual_fingerprint:
        raise RuntimeError("FinalizeMT requires chain_fingerprint from the clip phase payload")
    if expected_fingerprint and actual_fingerprint != expected_fingerprint:
        raise RuntimeError(
            f"FinalizeMT clip chain_fingerprint mismatch: expected {expected_fingerprint}, got {actual_fingerprint!r}"
        )
    metadata_program = str(metadata.get("score_program") or "").strip()
    payload_program = str(params.get("score_program") or metadata_program).strip()
    if not payload_program:
        raise RuntimeError("FinalizeMT requires score_program from the clip phase payload")
    if metadata_program and payload_program != metadata_program:
        raise RuntimeError(
            f"FinalizeMT score_program mismatch: expected {metadata_program!r}, got {payload_program!r}"
        )
    return {
        "chain_fingerprint": actual_fingerprint,
        "score_program": payload_program,
        "clip_slots": _normalize_clip_slots(params.get("clip_slots")),
    }


def _finalize_associated_palette(
    *,
    finalize_s3,
    job_id,
    task_id,
    run_id,
    workers,
    source_item_count,
    associated_palette,
    associated_palette_grid_n,
    associated_palette_times,
    associated_palette_degree,
    plan_params_digest,
    render_execution,
    metadata,
    clip_slots,
    chain_fingerprint,
    score_program,
    parent_progress,
):
    grid_n = int(associated_palette_grid_n or 0)
    if grid_n <= 0:
        raise RuntimeError("associated palette finalize requires associated_palette_grid_n > 0")
    fragment_prefix = str(associated_palette.get("fragment_prefix") or "").strip()
    if not fragment_prefix:
        raise RuntimeError("associated palette finalize requires fragment_prefix")

    raw_path = "/tmp/assoc_palette_greyscale.raw"
    hist_path = "/tmp/assoc_palette_greyscale.hist.json"
    url_manifest_path = "/tmp/assoc_palette_fragments.urls"
    eq_lut_path = "/tmp/assoc_palette_eq_lut.bin"
    encode_out_path = "/tmp/assoc_palette.jpeg"
    preview_path = "/tmp/assoc_palette_preview.png"

    t_assemble = time.time()
    def _report_assemble_progress(elapsed_ms):
        heartbeat = dict(parent_progress or {})
        heartbeat["assemble_ms"] = int(elapsed_ms)
        heartbeat["finalize_stage"] = "assemble"
        report_status(job_id, task_id, "assembling_score_tiles", result_data=heartbeat)

    fragment_urls = _presign_fragment_urls(
        finalize_s3=finalize_s3,
        fragment_prefix=fragment_prefix,
        source_item_count=source_item_count,
    )
    hist_meta = _assemble_greyscale_raw(
        pix=grid_n,
        raw_path=raw_path,
        hist_path=hist_path,
        workers=workers,
        fragment_urls=fragment_urls,
        manifest_path=url_manifest_path,
        progress_cb=_report_assemble_progress,
    )
    assemble_ms = int((time.time() - t_assemble) * 1000)

    nonzero_pixels = write_equalization_lut(eq_lut_path, hist_meta["histogram"])
    t_render = time.time()
    encode_meta = render_score_raw(
        raw_path=raw_path,
        out_path=encode_out_path,
        preview_path=preview_path,
        pix=grid_n,
        eq_lut_path=eq_lut_path,
        palette=str(associated_palette.get("palette") or metadata.get("palette") or "inferno"),
        background_color=background_color_hex(metadata.get("background_color", [0, 0, 0])),
        quality=90,
    )
    render_ms = int((time.time() - t_render) * 1000)

    raw_key = str(associated_palette["raw_key"])
    raw_meta_key = str(associated_palette["raw_meta_key"])
    image_key = str(associated_palette["image_key"])
    preview_key = str(associated_palette["preview_key"])
    meta_key = str(associated_palette["meta_key"])
    palette_id = str(associated_palette.get("palette_id") or "")
    created_at = _utc_now_iso()
    source_score = read_solve_score_metadata("solve", metadata, default_metric="proximity")
    associated_score = read_solve_score_metadata(
        "solve",
        {
            "solve_score_chain": associated_palette.get("score_chain", metadata.get("solve_score_chain", "")),
            "solve_metric": associated_palette.get("metric", source_score["metric"]),
            "solve_score_quantile": associated_palette.get("quantile", source_score["quantile"]),
            "solve_score_omega": associated_palette.get("omega", source_score["omega"]),
            "solve_score_omega_enabled": associated_palette.get("omega_enabled", source_score["omega_enabled"]),
        },
        default_metric=source_score["metric"],
    )

    sidecar = build_raw_sidecar(
        job_id=job_id,
        run_id=run_id,
        artifact_family="palette",
        artifact_id=palette_id,
        width=grid_n,
        height=grid_n,
        chain_fingerprint=chain_fingerprint,
        score_chain=associated_palette.get("score_chain", metadata.get("solve_score_chain", "")),
        score_program=score_program,
        clip_slots=clip_slots,
        background_color=metadata.get("background_color", [0, 0, 0]),
        plan_params_digest=plan_params_digest,
        render_execution=render_execution,
        raw_key=raw_key,
        image_key=image_key,
        preview_key=preview_key,
        meta_key=meta_key,
        created_at=created_at,
        histogram=hist_meta["histogram"],
    )

    with open(raw_path, "rb") as raw_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=raw_key,
            Body=raw_fh,
            ContentType="application/octet-stream",
        )
    finalize_s3.put_object(
        Bucket=BUCKET,
        Key=raw_meta_key,
        Body=json.dumps(sidecar, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )

    image_metadata = {
        "pix": str(grid_n),
        "width": str(grid_n),
        "height": str(grid_n),
        "palette": str(associated_palette.get("palette") or metadata.get("palette") or ""),
        "full_n": str(grid_n),
        "times": str(int(associated_palette_times or 1)),
        "using_pass": "0",
    }
    metadata_size = _metadata_size_bytes(image_metadata)
    if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
        raise RuntimeError(
            f"associated palette image metadata too large before upload: {metadata_size} bytes > {S3_USER_METADATA_LIMIT_BYTES} limit"
        )
    with open(encode_out_path, "rb") as out_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=image_key,
            Body=out_fh,
            ContentType="image/jpeg",
            Metadata=image_metadata,
        )
    with open(preview_path, "rb") as preview_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=preview_key,
            Body=preview_fh,
            ContentType="image/png",
        )

    meta_body = {
        "job_id": job_id,
        "palette_id": palette_id,
        "created_at": created_at,
        "display_name": str(associated_palette.get("display_name") or palette_id),
        "palette": str(associated_palette.get("palette") or metadata.get("palette") or ""),
        "degree": int(associated_palette_degree or 0),
        "N": grid_n,
        "times": int(associated_palette_times or 1),
        "using_pass": 0,
        "image_pass": 0,
        "base_grid_solves": grid_n * grid_n,
        "total_solves": grid_n * grid_n,
        "pass_count": int(associated_palette_times or 1),
        "data_layout": "fused_pass0_raw_v1",
        "render_reusable": False,
        "file_size": int(encode_meta["file_size"]),
        "image_key": image_key,
        "preview_key": preview_key,
        "raw_key": raw_key,
        "raw_meta_key": raw_meta_key,
        "metric": associated_score["metric"],
        "solve_score_chain": associated_score["chain_json"],
        "chain_fingerprint": chain_fingerprint,
        "derived_from_color_artifact_id": str(associated_palette.get("source_color_artifact_id") or ""),
        "derivation_kind": "extract_palette",
        "assemble_ms": assemble_ms,
        "render_ms": render_ms,
        "encode_ms": 0,
        "file_size_bytes": int(encode_meta["file_size"]),
        "nonzero_pixels": int(nonzero_pixels),
        "background_pixels": int(hist_meta["background_pixels"]),
    }
    finalize_s3.put_object(
        Bucket=BUCKET,
        Key=meta_key,
        Body=json.dumps(meta_body, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )
    return {
        "palette_id": palette_id,
        "display_name": str(associated_palette.get("display_name") or palette_id),
        "palette": str(associated_palette.get("palette") or metadata.get("palette") or ""),
        "metric": associated_score["metric"],
        "score_chain": associated_score["chain_json"],
        "image_key": image_key,
        "preview_key": preview_key,
        "raw_key": raw_key,
        "raw_meta_key": raw_meta_key,
        "meta_key": meta_key,
        "file_size": int(encode_meta["file_size"]),
        "assemble_ms": assemble_ms,
        "render_ms": render_ms,
        "encode_ms": 0,
    }


def handler(event, context):
    params = parse_body(event)
    phase = str(params.get("phase") or "").strip()
    if phase != "finalize_mt":
        raise RuntimeError(f"FinalizeMT requires phase='finalize_mt', got {phase!r}")
    job_id = params["job_id"]
    run_id = str(params.get("run_id") or "")
    task_id = params.get("task_id", "finalize_mt")
    mode = str(params.get("mode") or "").strip()
    if params.get("width") not in ("", None) or params.get("height") not in ("", None):
        raise RuntimeError("FinalizeMT no longer accepts width/height; pass pix for square output")
    pix = int(params["pix"])
    width = pix
    height = pix
    source_item_count = int(params.get("source_item_count", params.get("raster_item_count", 0)) or 0)
    if source_item_count <= 0:
        raise RuntimeError("FinalizeMT requires source_item_count > 0")
    fmt = str(params.get("format", "jpeg") or "jpeg").lower()
    quality = int(params.get("quality", 90) or 90)
    palette = str(params.get("palette", "inferno") or "inferno")
    background_color = str(params.get("background_color", "000000") or "000000")
    image_key = str(params["image_key"])
    preview_key = str(params.get("preview_key") or "")
    meta_key = str(params.get("meta_key") or "")
    raw_key = str(params["raw_key"])
    raw_meta_key = str(params["raw_meta_key"])
    plan_params_digest = str(params.get("plan_params_digest") or "").strip()
    fragment_prefix = str(params.get("fragment_prefix") or "").strip()
    fragment_manifest = dict(params.get("fragment_manifest") or {})
    associated_palette = dict(params.get("associated_palette") or {})
    associated_palette_grid_n = int(params.get("associated_palette_grid_n") or 0)
    associated_palette_times = int(params.get("associated_palette_times") or 0)
    associated_palette_degree = int(params.get("associated_palette_degree") or 0)
    render_execution = dict(params.get("render_execution") or {})
    metadata = dict(params.get("metadata") or {})
    workers = _validate_finalize_workers(params.get("finalize_workers", DEFAULT_FINALIZE_WORKERS))
    if str(render_execution.get("raster_engine") or "") != "mt":
        raise RuntimeError(
            f"FinalizeMT requires render_execution.raster_engine='mt', got {render_execution.get('raster_engine')!r}"
        )
    expected_chain_fingerprint = str(
        metadata.get("solve_score_chain_fingerprint") or metadata.get("chain_fingerprint") or ""
    ).strip()
    _validate_fragment_manifest(
        fragment_manifest,
        source_item_count=source_item_count,
        fragment_prefix=fragment_prefix,
        expected_chain_fingerprint=expected_chain_fingerprint,
    )
    if str(metadata.get("color_mode") or "") != "solve_score":
        raise RuntimeError("FinalizeMT currently supports only solve_score color artifacts")

    finalize_s3 = _finalize_s3_client(workers)
    clip_info = _clip_info_from_payload(params, metadata)
    progress = {
        "phase": "finalize_mt",
        "source_item_count": source_item_count,
        "pix": pix,
        "width": width,
        "height": height,
        "workers": workers,
    }
    report_status(job_id, task_id, "started", result_data=progress)

    raw_path = "/tmp/greyscale.raw"
    hist_path = "/tmp/greyscale.hist.json"
    step_scores_path = "/tmp/greyscale.step_scores.raw"
    eq_lut_path = "/tmp/greyscale.eq.bin"
    url_manifest_path = "/tmp/main_fragments.urls"
    ext = "png" if fmt == "png" else "jpeg"
    encode_out_path = f"/tmp/finalize_mt_out.{ext}"
    preview_out_path = "/tmp/finalize_mt_preview.png" if preview_key else ""

    t_assemble = time.time()
    fragment_urls = _presign_fragment_urls(
        finalize_s3=finalize_s3,
        fragment_prefix=fragment_prefix,
        source_item_count=source_item_count,
    )
    hist_meta = _assemble_greyscale_raw(
        pix=width,
        raw_path=raw_path,
        hist_path=hist_path,
        workers=workers,
        fragment_urls=fragment_urls,
        manifest_path=url_manifest_path,
    )
    progress["assemble_ms"] = int((time.time() - t_assemble) * 1000)
    report_status(job_id, task_id, "assembled_score_tiles", result_data=progress)

    progress["raw_size"] = os.path.getsize(raw_path)
    progress["nonzero_pixels"] = hist_meta["nonzero_pixels"]
    progress["background_pixels"] = hist_meta["background_pixels"]
    report_status(job_id, task_id, "wrote_greyscale_raw", result_data=progress)

    step_scores_grid_n = associated_palette_grid_n
    step_scores_pass_count = associated_palette_times
    step_scores_key = ""
    step_scores_count = 0
    if step_scores_grid_n > 0 and step_scores_pass_count > 0:
        step_scores_count = int(step_scores_grid_n) * int(step_scores_grid_n) * int(step_scores_pass_count)
        actual_step_scores_count = _concat_step_scores(
            finalize_s3=finalize_s3,
            fragment_prefix=fragment_prefix,
            source_item_count=source_item_count,
            out_path=step_scores_path,
        )
        if actual_step_scores_count != step_scores_count:
            raise RuntimeError(
                "FinalizeMT step score byte count mismatch: "
                f"expected {step_scores_count}, got {actual_step_scores_count}"
            )
        step_scores_key = raw_key.rsplit("/", 1)[0] + "/step_scores.raw"
        with open(step_scores_path, "rb") as scores_fh:
            finalize_s3.put_object(
                Bucket=BUCKET,
                Key=step_scores_key,
                Body=scores_fh,
                ContentType="application/octet-stream",
            )
        progress["step_scores_key"] = step_scores_key
        progress["step_scores_count"] = step_scores_count
        progress["step_scores_grid_n"] = step_scores_grid_n
        report_status(job_id, task_id, "wrote_step_scores", result_data=progress)

    write_equalization_lut(eq_lut_path, hist_meta["histogram"])
    t_render = time.time()
    encode_meta = render_score_raw(
        raw_path=raw_path,
        out_path=encode_out_path,
        preview_path=preview_out_path,
        pix=pix,
        eq_lut_path=eq_lut_path,
        palette=palette,
        background_color=background_color,
        quality=quality,
    )
    progress["render_ms"] = int((time.time() - t_render) * 1000)
    progress["encode_ms"] = 0
    progress["file_size"] = int(encode_meta["file_size"])
    report_status(job_id, task_id, "rendered_rgb_tiles", result_data=progress)
    report_status(job_id, task_id, "encoded", result_data=progress)

    sidecar = build_raw_sidecar(
        job_id=job_id,
        run_id=run_id,
        artifact_family=str(metadata.get("family") or mode or "color"),
        artifact_id=str(metadata.get("artifact_id") or ""),
        width=width,
        height=height,
        chain_fingerprint=clip_info["chain_fingerprint"],
        score_chain=metadata.get("solve_score_chain", ""),
        score_program=clip_info["score_program"],
        clip_slots=clip_info["clip_slots"],
        background_color=background_color,
        plan_params_digest=plan_params_digest,
        render_execution=render_execution,
        raw_key=raw_key,
        image_key=image_key,
        preview_key=preview_key,
        meta_key=meta_key,
        created_at=str(metadata.get("created_at") or _utc_now_iso()),
        histogram=hist_meta["histogram"],
        step_scores_key=step_scores_key,
        step_count=step_scores_count,
        step_scores_grid_n=step_scores_grid_n if step_scores_key else None,
    )

    t_upload = time.time()
    with open(raw_path, "rb") as raw_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=raw_key,
            Body=raw_fh,
            ContentType="application/octet-stream",
        )
    finalize_s3.put_object(
        Bucket=BUCKET,
        Key=raw_meta_key,
        Body=json.dumps(sidecar, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )

    associated_palette_result = None
    if str(associated_palette.get("mode") or "") == "generated":
        associated_palette_result = _finalize_associated_palette(
            finalize_s3=finalize_s3,
            job_id=job_id,
            task_id=task_id,
            run_id=run_id,
            workers=workers,
            source_item_count=source_item_count,
            associated_palette=associated_palette,
            associated_palette_grid_n=associated_palette_grid_n,
            associated_palette_times=associated_palette_times,
            associated_palette_degree=associated_palette_degree,
            plan_params_digest=plan_params_digest,
            render_execution=render_execution,
            metadata=metadata,
            clip_slots=clip_info["clip_slots"],
            chain_fingerprint=clip_info["chain_fingerprint"],
            score_program=clip_info["score_program"],
            parent_progress=progress,
        )

    final_metadata = dict(metadata)
    final_metadata["render_execution"] = render_execution
    final_metadata["raw_key"] = raw_key
    final_metadata["raw_meta_key"] = raw_meta_key
    final_metadata["step_scores_key"] = step_scores_key
    final_metadata["step_count"] = step_scores_count if step_scores_key else ""
    final_metadata["step_scores_grid_n"] = step_scores_grid_n if step_scores_key else ""
    final_metadata["repalette_capable"] = False
    final_metadata["pixel_bins_prefix"] = ""
    final_metadata["pixel_bins_empty"] = ""
    final_metadata["pixel_bins_layout"] = ""
    if associated_palette_result:
        final_metadata["associated_palette_mode"] = "generated"
        final_metadata["associated_palette_id"] = associated_palette_result["palette_id"]
        final_metadata["associated_palette_display_name"] = associated_palette_result["display_name"]
        final_metadata["associated_palette_image_key"] = associated_palette_result["image_key"]
        final_metadata["associated_palette_preview_key"] = associated_palette_result["preview_key"]
        final_metadata["associated_palette_palette"] = associated_palette_result["palette"]
        final_metadata["associated_palette_metric"] = associated_palette_result["metric"]
        final_metadata["associated_palette_score_chain"] = associated_palette_result["score_chain"]
        final_metadata["associated_palette_raw_key"] = associated_palette_result["raw_key"]
        final_metadata["associated_palette_raw_meta_key"] = associated_palette_result["raw_meta_key"]
        final_metadata["associated_palette_meta_key"] = associated_palette_result["meta_key"]
    image_meta, overlay_meta = split_color_artifact_metadata(final_metadata)
    final_headers = {"pix": str(pix), "width": str(width), "height": str(height), **image_meta}
    metadata_size = _metadata_size_bytes(final_headers)
    if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
        raise RuntimeError(
            f"image metadata too large before upload: {metadata_size} bytes > {S3_USER_METADATA_LIMIT_BYTES} limit"
        )
    content_type = "image/png" if ext == "png" else "image/jpeg"
    with open(encode_out_path, "rb") as out_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=image_key,
            Body=out_fh,
            ContentType=content_type,
            Metadata=final_headers,
        )
    if preview_key:
        with open(preview_out_path, "rb") as preview_fh:
            finalize_s3.put_object(
                Bucket=BUCKET,
                Key=preview_key,
                Body=preview_fh,
                ContentType="image/png",
            )
    artifact_id = str(final_metadata.get("artifact_id") or "").strip()
    if artifact_id:
        write_color_artifact_meta_overlay(finalize_s3, BUCKET, job_id, artifact_id, overlay_meta)

    progress["upload_ms"] = int((time.time() - t_upload) * 1000)
    progress["image_key"] = image_key
    progress["raw_key"] = raw_key
    if associated_palette_result:
        progress["associated_palette_image_key"] = associated_palette_result["image_key"]
    report_status(job_id, task_id, "done", result_data=progress)

    result = {
        "image_key": image_key,
        "raw_key": raw_key,
        "raw_meta_key": raw_meta_key,
        "step_scores_key": step_scores_key,
        "step_count": step_scores_count if step_scores_key else 0,
        "step_scores_grid_n": step_scores_grid_n if step_scores_key else 0,
        "file_size": int(encode_meta["file_size"]),
        "timings": {
            "assemble_ms": progress["assemble_ms"],
            "render_ms": progress["render_ms"],
            "encode_ms": progress["encode_ms"],
            "upload_ms": progress["upload_ms"],
        },
    }
    if associated_palette_result:
        result["associated_palette"] = associated_palette_result
    return ok_response(result)
