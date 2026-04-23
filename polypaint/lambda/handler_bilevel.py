"""
Bilevel Lambda handler.

Legacy phases:
  phase=raster: one chunk -> per-tile bitset files (.bits)
  phase=coeff_raster: one coeff chunk -> per-tile bitset files (.bits)
  phase=merge: per-tile bitsets -> tile TIFF

Fused phases:
  phase=section_raster: one logical section -> one sparse occupancy fragment
  phase=finalize: assemble sparse occupancy fragments -> final TIFF + preview
  phase=from_raw_color: threshold a fused Color greyscale raw sidecar -> bilevel
"""
from __future__ import annotations

import glob
import json
import math
import os
import re
import subprocess
import threading
import time
from datetime import datetime, timezone

import boto3
from botocore.config import Config
from botocore.exceptions import ClientError

from color_artifact_meta import load_color_artifact_head
from logical_sections import build_source_spans, stitch_spans_to_file
from raw_sidecar import validate_raw_sidecar
from shared import BILEVEL_SPARSE_PIPELINE, BUCKET, imgpipe_env, ok_response, parse_body, report_status

s3 = boto3.client("s3")
BILEVEL_RASTER = os.path.join(os.path.dirname(__file__), "bilevel_raster")
BILEVEL_SECTION_RASTER = os.path.join(os.path.dirname(__file__), "bilevel_section_raster")
COEFFS_BILEVEL_RASTER = os.path.join(os.path.dirname(__file__), "coeffs_bilevel_raster")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")
ASSEMBLE_GREYSCALE = os.path.join(os.path.dirname(__file__), "assemble_greyscale")
RAW_TO_BILEVEL = os.path.join(os.path.dirname(__file__), "raw_to_bilevel")

_TMP_ROOTS = "/tmp/stripe.bin"
_TMP_COEFFS = "/tmp/coeffs.bin"
_TMP_SECTION_FRAGMENT = "/tmp/section.frag"
_TMP_FINAL_TIF = "/tmp/final.tif"
_TMP_FINAL_PREVIEW = "/tmp/final_preview.png"
_TMP_FINAL_RAW = "/tmp/final_bilevel.raw"
_TMP_FINAL_HIST = "/tmp/final_bilevel.hist.json"
_TMP_FINAL_URLS = "/tmp/bilevel_fragments.urls"
_TMP_SOURCE_RAW = "/tmp/source_greyscale.raw"
_TMP_ROOT_XFORMS = "/tmp/root_xforms.json"
DEFAULT_BILEVEL_FINALIZE_WORKERS = int(os.environ.get("BILEVEL_FINALIZE_WORKERS", "16") or "16")
MAX_BILEVEL_FINALIZE_WORKERS = 64
ASSEMBLE_PROGRESS_INTERVAL_S = float(os.environ.get("BILEVEL_ASSEMBLE_PROGRESS_INTERVAL_S", "20") or "20")
FRAGMENT_URL_EXPIRES_S = int(os.environ.get("BILEVEL_FRAGMENT_URL_EXPIRES_S", "1800") or "1800")
RAW_TO_BILEVEL_PROGRESS_INTERVAL_S = float(os.environ.get("RAW_TO_BILEVEL_PROGRESS_INTERVAL_S", "5") or "5")
MAX_BILEVEL_S3_METADATA_BYTES = 1800
_BILEVEL_UPLOAD_METADATA_LIMITS = {
    "artifact_id": 128,
    "created_at": 64,
    "format": 16,
    "mode": 32,
    "width": 16,
    "height": 16,
    "degree": 32,
    "pix": 32,
    "tile_size": 32,
    "view_mode": 32,
    "quantile": 64,
    "shim": 64,
    "square_extent": 64,
    "min_re": 64,
    "max_re": 64,
    "min_im": 64,
    "max_im": 64,
    "rotation": 64,
    "root_transforms": 512,
    "render_execution": 512,
    "bilevel_pipeline": 64,
    "bilevel_section_mode": 64,
    "bilevel_section_count": 32,
}
_BILEVEL_FORWARD_METADATA_KEYS = (
    "artifact_id",
    "created_at",
    "degree",
    "pix",
    "tile_size",
    "view_mode",
    "quantile",
    "shim",
    "square_extent",
    "min_re",
    "max_re",
    "min_im",
    "max_im",
    "rotation",
    "root_transforms",
    "render_execution",
    "format",
    "mode",
    "bilevel_pipeline",
    "bilevel_section_mode",
    "bilevel_section_count",
)
_DIRECT_VIEWPORT_METADATA_KEYS = (
    "view_mode",
    "quantile",
    "shim",
    "square_extent",
    "min_re",
    "max_re",
    "min_im",
    "max_im",
    "rotation",
    "root_transforms",
)


def _utc_now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status, result_data={"phase": phase, "phase_label": phase_label, **extra})


def _coerce_finite_float(value, field_name):
    try:
        num = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be numeric, got {value!r}")
    if not math.isfinite(num):
        raise RuntimeError(f"{field_name} must be finite, got {value!r}")
    return num


def _viewport_bounds(params):
    has_any_bounds = any(params.get(key) not in (None, "") for key in ("min_re", "max_re", "min_im", "max_im"))
    if has_any_bounds:
        missing = [key for key in ("min_re", "max_re", "min_im", "max_im") if params.get(key) in (None, "")]
        if missing:
            raise RuntimeError(f"exact viewport requires {', '.join(missing)}")
        min_re = _coerce_finite_float(params.get("min_re"), "min_re")
        max_re = _coerce_finite_float(params.get("max_re"), "max_re")
        min_im = _coerce_finite_float(params.get("min_im"), "min_im")
        max_im = _coerce_finite_float(params.get("max_im"), "max_im")
        if not max_re > min_re:
            raise RuntimeError(f"exact viewport requires max_re > min_re, got {min_re!r}/{max_re!r}")
        if not max_im > min_im:
            raise RuntimeError(f"exact viewport requires max_im > min_im, got {min_im!r}/{max_im!r}")
        return {
            "min_re": min_re,
            "max_re": max_re,
            "min_im": min_im,
            "max_im": max_im,
        }
    raise RuntimeError("exact viewport requires min_re, max_re, min_im, and max_im")


def _finalize_worker_count():
    workers = DEFAULT_BILEVEL_FINALIZE_WORKERS
    if not (1 <= workers <= MAX_BILEVEL_FINALIZE_WORKERS):
        raise RuntimeError(
            f"bilevel finalize workers must be in [1, {MAX_BILEVEL_FINALIZE_WORKERS}], got {workers}"
        )
    return workers


def _pix_param(params, label):
    if params.get("width") not in ("", None) or params.get("height") not in ("", None):
        raise RuntimeError(f"{label} no longer accepts width/height; pass pix for square output")
    try:
        pix = int(params.get("pix"))
    except (TypeError, ValueError) as exc:
        raise RuntimeError(f"{label} requires integer pix") from exc
    if pix <= 0:
        raise RuntimeError(f"{label} requires pix > 0")
    return pix


def _tile_shape(tile_idx, width, height, tile_size, n_tile_cols):
    tile_idx = int(tile_idx)
    width = int(width)
    height = int(height)
    tile_size = int(tile_size)
    n_tile_cols = int(n_tile_cols)
    row = tile_idx // n_tile_cols
    col = tile_idx % n_tile_cols
    tile_w = max(0, min(tile_size, width - col * tile_size))
    tile_h = max(0, min(tile_size, height - row * tile_size))
    return tile_w, tile_h


def _cleanup_tmp(patterns):
    for pattern in patterns:
        for path in glob.glob(pattern):
            try:
                os.remove(path)
            except OSError:
                pass


def _write_root_xforms(path, chain):
    rt_chain = chain or []
    if not rt_chain:
        return ""
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(rt_chain, fh, separators=(",", ":"))
    return path


def _download_to_path(key, path):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"]
    with open(path, "wb") as fh:
        if hasattr(body, "iter_chunks"):
            for chunk in body.iter_chunks(chunk_size=1024 * 1024):
                fh.write(chunk)
        else:
            fh.write(body.read())


def _load_json_key(key):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"].read()
    data = json.loads(body) if body else {}
    if not isinstance(data, dict):
        raise RuntimeError(f"Expected JSON object in {key}")
    return data


def _upload_file(path, key, *, content_type, metadata=None):
    with open(path, "rb") as fh:
        s3.put_object(
            Bucket=BUCKET,
            Key=key,
            Body=fh,
            ContentType=content_type,
            Metadata={str(k): str(v) for k, v in (metadata or {}).items() if v not in ("", None)},
        )


def _metadata_str_value(value):
    if value in ("", None):
        return None
    if isinstance(value, (dict, list)):
        return json.dumps(value, separators=(",", ":"))
    return str(value)


def _set_bilevel_upload_metadata_value(target, key, value):
    text = _metadata_str_value(value)
    if text in (None, ""):
        return
    limit = _BILEVEL_UPLOAD_METADATA_LIMITS.get(str(key))
    if limit is None:
        raise RuntimeError(f"unsupported bilevel upload metadata key: {key}")
    encoded = text.encode("utf-8")
    if len(encoded) > int(limit):
        raise RuntimeError(f"bilevel finalize metadata {key} exceeds {limit} bytes")
    target[str(key)] = text


def _build_bilevel_upload_metadata(*, metadata, width, height, source_item_count):
    upload_meta = {}
    for key in _BILEVEL_FORWARD_METADATA_KEYS:
        if key in metadata:
            _set_bilevel_upload_metadata_value(upload_meta, key, metadata.get(key))
    for key, value in {
        "artifact_id": metadata.get("artifact_id") or "",
        "family": None,
        "created_at": metadata.get("created_at") or _utc_now_iso(),
        "format": "tif",
        "mode": "bilevel",
        "width": width,
        "height": height,
        "pix": metadata.get("pix") or width,
        "bilevel_pipeline": BILEVEL_SPARSE_PIPELINE,
        "bilevel_section_mode": metadata.get("bilevel_section_mode") or "",
        "bilevel_section_count": metadata.get("bilevel_section_count") or source_item_count,
    }.items():
        if key == "family":
            upload_meta["family"] = "bilevel"
            continue
        _set_bilevel_upload_metadata_value(upload_meta, key, value)
    total_metadata_bytes = sum(len(k.encode("utf-8")) + len(v.encode("utf-8")) for k, v in upload_meta.items())
    if total_metadata_bytes > MAX_BILEVEL_S3_METADATA_BYTES:
        raise RuntimeError(
            f"bilevel finalize metadata exceeds {MAX_BILEVEL_S3_METADATA_BYTES} bytes "
            f"({total_metadata_bytes} bytes)"
        )
    return upload_meta


def _finalize_s3_client(max_workers):
    pool_size = max(16, int(max_workers) * 2)
    return boto3.client("s3", config=Config(max_pool_connections=pool_size))


def _fragment_key(fragment_prefix, section_idx):
    prefix = str(fragment_prefix or "").strip()
    if not prefix:
        raise RuntimeError("bilevel finalize requires fragment_prefix")
    return f"{prefix}{int(section_idx):04d}.frag"


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
                ExpiresIn=FRAGMENT_URL_EXPIRES_S,
            )
        )
    return urls


def _assemble_sparse_bilevel_raw(*, pix, raw_path, hist_path, workers, fragment_urls, manifest_path, progress_cb=None):
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
            while True:
                delay = max(0.0, next_tick - time.time())
                if stop_heartbeat.wait(delay):
                    break
                try:
                    progress_cb(int((time.time() - started_at) * 1000))
                except Exception:
                    pass
                now = time.time()
                while next_tick <= now:
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


def _report_phase_error(params, message, *, phase, phase_label, phase_raw="", extra_keys=()):
    job_id = str((params or {}).get("job_id") or "").strip()
    if not job_id:
        return
    task_id_default = str(phase or "bilevel").strip() or "bilevel"
    task_id = str((params or {}).get("task_id") or task_id_default).strip() or task_id_default
    result_data = {
        "phase": str(phase or ""),
        "phase_label": str(phase_label or phase or ""),
    }
    if phase_raw not in ("", None):
        result_data["phase_raw"] = str(phase_raw)
    for key in ("artifact_id", "source_artifact_id", "threshold"):
        value = (params or {}).get(key)
        if value not in ("", None):
            result_data[key] = str(value)
    for key in extra_keys:
        value = (params or {}).get(key)
        if value not in ("", None):
            result_data[str(key)] = str(value)
    report_status(job_id, task_id, "error", message, result_data=result_data)


def _extract_handler_entry_params(event):
    params = {}
    if not isinstance(event, dict):
        return params
    body = event.get("body")
    if isinstance(body, dict):
        for key in ("job_id", "task_id", "phase", "artifact_id", "source_artifact_id", "threshold"):
            value = body.get(key)
            if value not in ("", None):
                params[key] = value
        return params
    if isinstance(body, str):
        for key in ("job_id", "task_id", "phase", "artifact_id", "source_artifact_id"):
            match = re.search(rf'"{re.escape(key)}"\s*:\s*"([^"]*)"', body)
            if match and match.group(1) not in ("", None):
                params[key] = match.group(1)
        threshold_match = re.search(r'"threshold"\s*:\s*("(.*?)"|[-+]?\d+(?:\.\d+)?)', body)
        if threshold_match:
            params["threshold"] = threshold_match.group(2) if threshold_match.group(2) is not None else threshold_match.group(1)
    for key in ("job_id", "task_id", "phase", "artifact_id", "source_artifact_id", "threshold"):
        if key not in params:
            value = event.get(key)
            if value not in ("", None):
                params[key] = value
    return params


def handler(event, context):
    try:
        params = parse_body(event)
    except Exception as e:
        params = _extract_handler_entry_params(event)
        message = f"Bilevel handler could not parse request body: {e}"
        _report_phase_error(
            params,
            message,
            phase="handler_entry",
            phase_label="BiLevel dispatch",
            phase_raw=params.get("phase"),
        )
        raise RuntimeError(message) from e
    phase = str(params.get("phase") or "").strip()
    if not phase:
        message = "Bilevel handler requires non-empty phase"
        _report_phase_error(
            params,
            message,
            phase="handler_entry",
            phase_label="BiLevel dispatch",
            phase_raw=params.get("phase"),
        )
        raise ValueError(message)

    if phase == "raster":
        return handle_raster(params)
    if phase == "coeff_raster":
        return handle_coeff_raster(params)
    if phase == "merge":
        return handle_merge(params)
    if phase == "section_raster":
        return handle_section_raster(params)
    if phase == "finalize":
        return handle_finalize(params)
    if phase == "from_raw_color":
        return handle_from_raw_color(params)
    message = f"Unknown bilevel phase: {phase}"
    _report_phase_error(
        params,
        message,
        phase="handler_entry",
        phase_label="BiLevel dispatch",
        phase_raw=phase,
    )
    raise ValueError(message)


def handle_raster(params):
    """One chunk -> per-tile bitset files. One Lambda per chunk."""
    try:
        job_id = str(params.get("job_id") or "").strip()
        if not job_id:
            raise RuntimeError("bilevel raster requires job_id")
        chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
        if chunk_idx is None:
            raise RuntimeError("bilevel raster requires chunk_idx")
        task_id = params.get("task_id", f"bilevel_raster_{chunk_idx}")
        report_status(job_id, task_id, "started")

        bin_key = f"renders/{job_id}/chunk_{chunk_idx}.bin"
        bin_path = _TMP_ROOTS
        t0 = time.time()
        try:
            _download_to_path(bin_key, bin_path)
        except Exception as e:
            raise RuntimeError(f"Failed to download root chunk s3://{BUCKET}/{bin_key}: {e}") from e
        dl_ms = int((time.time() - t0) * 1000)
        report_status(job_id, task_id, "bin_downloaded")

        _cleanup_tmp(["/tmp/bits_t*.bits", _TMP_ROOT_XFORMS])

        viewport = _viewport_bounds(params)
        pix = _pix_param(params, "bilevel raster")
        cmd = [
            BILEVEL_RASTER, bin_path, "/tmp/bits",
            f"--pix={pix}",
            f"--tile_size={params['tile_size']}",
            f"--n_tile_cols={params['n_tile_cols']}",
            f"--n_tile_rows={params['n_tile_rows']}",
            f"--min_re={viewport['min_re']}",
            f"--max_re={viewport['max_re']}",
            f"--min_im={viewport['min_im']}",
            f"--max_im={viewport['max_im']}",
            f"--degree={params['degree']}",
            f"--rotation={params.get('rotation', 0.0)}",
        ]
        rt_path = _write_root_xforms(_TMP_ROOT_XFORMS, params.get("root_transforms", []))
        if rt_path:
            cmd.append(f"--root_xforms={rt_path}")

        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            raise RuntimeError(f"bilevel_raster failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        raster_ms = int((time.time() - t1) * 1000)

        try:
            os.remove(bin_path)
        except OSError:
            pass

        n_tiles = params["n_tile_cols"] * params["n_tile_rows"]
        uploaded = 0
        for t in range(n_tiles):
            bits_path = f"/tmp/bits_t{t:04d}.bits"
            if os.path.exists(bits_path) and os.path.getsize(bits_path) > 0:
                s3_key = f"renders/{job_id}/bits_chunk_{int(chunk_idx):04d}_t{t:04d}.bits"
                with open(bits_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, s3_key)
                uploaded += 1
            if os.path.exists(bits_path):
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
        _report_phase_error(
            params,
            str(e),
            phase="bilevel_raster",
            phase_label="BiLevel raster",
            extra_keys=("chunk_idx", "stripe_idx"),
        )
        _cleanup_tmp([_TMP_ROOTS, _TMP_ROOT_XFORMS, "/tmp/bits_t*.bits"])
        raise


def handle_coeff_raster(params):
    """One coeff chunk -> per-tile bitset files. One Lambda per chunk."""
    try:
        job_id = str(params.get("job_id") or "").strip()
        if not job_id:
            raise RuntimeError("coeff bilevel raster requires job_id")
        chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
        if chunk_idx is None:
            raise RuntimeError("coeff bilevel raster requires chunk_idx")
        task_id = params.get("task_id", f"coeff_bilevel_raster_{chunk_idx}")
        report_status(job_id, task_id, "started")

        bin_key = params.get("coeffs_key", f"renders/{job_id}/coeffs_{int(chunk_idx):04d}.bin")
        bin_path = _TMP_COEFFS
        t0 = time.time()
        try:
            _download_to_path(bin_key, bin_path)
        except Exception as e:
            raise RuntimeError(f"Failed to download coeffs chunk s3://{BUCKET}/{bin_key}: {e}") from e
        dl_ms = int((time.time() - t0) * 1000)
        report_status(job_id, task_id, "bin_downloaded")

        _cleanup_tmp(["/tmp/coeff_bits_t*.bits"])

        try:
            n_coeffs = int(params["n_coeffs"])
        except (TypeError, ValueError, KeyError) as e:
            raise RuntimeError(f"coeff_bilevel requires numeric n_coeffs, got {params.get('n_coeffs')!r}") from e
        if n_coeffs < 1:
            raise RuntimeError(f"coeff_bilevel requires n_coeffs >= 1, got {n_coeffs}")

        viewport = _viewport_bounds(params)
        pix = _pix_param(params, "coeff bilevel raster")
        cmd = [
            COEFFS_BILEVEL_RASTER, bin_path, "/tmp/coeff_bits",
            f"--pix={pix}",
            f"--tile_size={params['tile_size']}",
            f"--n_tile_cols={params['n_tile_cols']}",
            f"--n_tile_rows={params['n_tile_rows']}",
            f"--min_re={viewport['min_re']}",
            f"--max_re={viewport['max_re']}",
            f"--min_im={viewport['min_im']}",
            f"--max_im={viewport['max_im']}",
            f"--n_coeffs={n_coeffs}",
            f"--rotation={params.get('rotation', 0.0)}",
        ]
        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            raise RuntimeError(f"coeffs_bilevel_raster failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        raster_ms = int((time.time() - t1) * 1000)

        try:
            os.remove(bin_path)
        except OSError:
            pass

        n_tiles = params["n_tile_cols"] * params["n_tile_rows"]
        uploaded = 0
        for t in range(n_tiles):
            bits_path = f"/tmp/coeff_bits_t{t:04d}.bits"
            if os.path.exists(bits_path) and os.path.getsize(bits_path) > 0:
                s3_key = f"renders/{job_id}/coeff_bits_chunk_{int(chunk_idx):04d}_t{t:04d}.bits"
                with open(bits_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, s3_key)
                uploaded += 1
            if os.path.exists(bits_path):
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
        _report_phase_error(
            params,
            str(e),
            phase="coeff_bilevel_raster",
            phase_label="Coeffs raster",
            extra_keys=("chunk_idx", "stripe_idx", "n_coeffs"),
        )
        _cleanup_tmp([_TMP_COEFFS, "/tmp/coeff_bits_t*.bits"])
        raise


def handle_merge(params):
    """OR per-chunk bitsets for one tile -> tile TIFF. One Lambda per tile."""
    bits_paths = []
    out_path = "/tmp/tile.tif"
    try:
        job_id = str(params.get("job_id") or "").strip()
        if not job_id:
            raise RuntimeError("bilevel merge requires job_id")
        tile_idx = params.get("tile_idx")
        if tile_idx in (None, ""):
            raise RuntimeError("bilevel merge requires tile_idx")
        try:
            tile_idx = int(tile_idx)
        except (TypeError, ValueError) as e:
            raise RuntimeError(f"bilevel merge requires integer tile_idx, got {tile_idx!r}") from e
        if tile_idx < 0:
            raise RuntimeError(f"bilevel merge requires tile_idx >= 0, got {tile_idx}")
        tile_w = params.get("tile_w")
        tile_h = params.get("tile_h")
        if tile_w in (None, "") or tile_h in (None, ""):
            pix = _pix_param(params, "bilevel merge")
            tile_w, tile_h = _tile_shape(
                tile_idx,
                pix,
                pix,
                params["tile_size"],
                params["n_tile_cols"],
            )
        n_chunks = params.get("n_chunks", params.get("n_stripes"))
        if n_chunks is None:
            raise RuntimeError("bilevel merge requires n_chunks")
        bits_prefix = params.get("bits_prefix", "bits")
        tile_prefix = params.get("tile_prefix", "bilevel")
        task_prefix = params.get("task_prefix", "bilevel_merge")
        task_id = str(params.get("task_id") or f"{task_prefix}_{tile_idx}").strip() or f"{task_prefix}_{tile_idx}"
        report_status(job_id, task_id, "started")

        for c in range(n_chunks):
            bits_key = f"renders/{job_id}/{bits_prefix}_chunk_{c:04d}_t{int(tile_idx):04d}.bits"
            local_path = f"/tmp/bits_chunk_{c}.bits"
            try:
                obj = s3.get_object(Bucket=BUCKET, Key=bits_key)
                with open(local_path, "wb") as f:
                    f.write(obj["Body"].read())
                bits_paths.append(local_path)
            except ClientError as e:
                if e.response["Error"]["Code"] == "NoSuchKey":
                    continue
                raise
        report_status(job_id, task_id, "bits_downloaded")

        cmd = [
            BILEVEL_MERGE, "merge",
            f"--tile_w={tile_w}", f"--tile_h={tile_h}",
            f"--output={out_path}",
        ] + bits_paths
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300, env=imgpipe_env())
        if result.returncode != 0:
            raise RuntimeError(f"bilevel_merge failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)

        for path in bits_paths:
            try:
                os.remove(path)
            except OSError:
                pass

        tile_key = f"renders/{job_id}/{tile_prefix}_t{int(tile_idx):04d}.tif"
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
        _report_phase_error(
            params,
            str(e),
            phase="bilevel_merge",
            phase_label="BiLevel merge",
            extra_keys=("tile_idx", "n_chunks", "n_stripes"),
        )
        raise
    finally:
        for path in bits_paths:
            try:
                os.remove(path)
            except OSError:
                pass
        try:
            os.remove(out_path)
        except OSError:
            pass


def handle_section_raster(params):
    """One logical section -> one sparse occupancy fragment."""
    try:
        job_id = str(params.get("job_id") or "").strip()
        if not job_id:
            raise RuntimeError("bilevel section raster requires job_id")
        section_idx_raw = params.get("section_idx")
        if section_idx_raw in (None, ""):
            raise RuntimeError("bilevel section raster requires section_idx")
        section_idx = int(section_idx_raw)
        if section_idx < 0:
            raise RuntimeError(f"bilevel section raster requires section_idx >= 0, got {section_idx}")
        task_id = params.get("task_id", f"bilevel_section_{section_idx}")
        solve_source_manifest = dict(params.get("solve_source_manifest") or {})
        step_start_raw = params.get("step_start")
        if step_start_raw in (None, ""):
            raise RuntimeError("bilevel section raster requires step_start")
        step_start = int(step_start_raw)
        if step_start < 0:
            raise RuntimeError(f"bilevel section raster requires step_start >= 0, got {step_start}")
        step_count = int(params.get("step_count", 0) or 0)
        fragment_prefix = str(params.get("fragment_prefix") or "").strip()
        if not solve_source_manifest:
            raise RuntimeError("bilevel section raster requires solve_source_manifest")
        if step_count <= 0:
            raise RuntimeError("bilevel section raster requires step_count > 0")
        if not fragment_prefix:
            raise RuntimeError("bilevel section raster requires fragment_prefix")
        _phase(job_id, task_id, "started", "bilevel_raster", "BiLevel raster")
        root_spans = build_source_spans(
            solve_source_manifest,
            source_family="slv",
            solve_start=step_start,
            solve_count=step_count,
        )
        if not root_spans:
            raise RuntimeError(
                f"bilevel section raster produced no root spans for solve_start={step_start}, step_count={step_count}"
            )

        _cleanup_tmp([_TMP_ROOTS, _TMP_SECTION_FRAGMENT, _TMP_ROOT_XFORMS])
        t0 = time.time()
        stitch_spans_to_file(s3, BUCKET, root_spans, _TMP_ROOTS)
        dl_ms = int((time.time() - t0) * 1000)
        _phase(job_id, task_id, "downloaded", "bilevel_raster", "BiLevel raster", dl_ms=dl_ms)

        viewport = _viewport_bounds(params)
        pix = _pix_param(params, "bilevel section raster")
        cmd = [
            BILEVEL_SECTION_RASTER, _TMP_ROOTS, _TMP_SECTION_FRAGMENT,
            f"--pix={pix}",
            f"--min_re={viewport['min_re']}",
            f"--max_re={viewport['max_re']}",
            f"--min_im={viewport['min_im']}",
            f"--max_im={viewport['max_im']}",
            f"--degree={params['degree']}",
            f"--rotation={params.get('rotation', 0.0)}",
        ]
        rt_path = _write_root_xforms(_TMP_ROOT_XFORMS, params.get("root_transforms", []))
        if rt_path:
            cmd.append(f"--root_xforms={rt_path}")

        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            raise RuntimeError(f"bilevel_section_raster failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        raster_ms = int((time.time() - t1) * 1000)

        fragment_key = f"{fragment_prefix}{section_idx:04d}.frag"
        with open(_TMP_SECTION_FRAGMENT, "rb") as fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=fragment_key,
                Body=fh,
                ContentType="application/octet-stream",
            )

        _phase(
            job_id,
            task_id,
            "done",
            "bilevel_raster",
            "BiLevel raster",
            dl_ms=dl_ms,
            raster_ms=raster_ms,
            fragment_size=int(meta.get("file_size", 0) or 0),
            section_idx=section_idx,
        )
        return ok_response({
            "section_idx": section_idx,
            "roots_plotted": int(meta.get("roots_plotted", 0) or 0),
            "roots_clipped": int(meta.get("roots_clipped", 0) or 0),
            "roots_deduped": int(meta.get("roots_deduped", 0) or 0),
            "fragment_size": int(meta.get("file_size", 0) or 0),
            "dl_ms": dl_ms,
            "raster_ms": raster_ms,
            "fragment_key": fragment_key,
        })
    except Exception as e:
        _report_phase_error(
            params,
            str(e),
            phase="bilevel_raster",
            phase_label="BiLevel raster",
            extra_keys=("section_idx", "step_start", "step_count", "fragment_prefix"),
        )
        _cleanup_tmp([_TMP_ROOTS, _TMP_SECTION_FRAGMENT, _TMP_ROOT_XFORMS])
        raise


def handle_finalize(params):
    """Assemble sparse occupancy fragments and write the final bilevel TIFF + preview."""
    try:
        job_id = str(params.get("job_id") or "").strip()
        if not job_id:
            raise RuntimeError("bilevel finalize requires job_id")
        task_id = str(params.get("task_id") or "bilevel_finalize").strip() or "bilevel_finalize"
        pix = _pix_param(params, "bilevel finalize")
        width = pix
        height = pix
        source_item_count = int(params.get("source_item_count", 0) or 0)
        fragment_prefix = str(params.get("fragment_prefix") or "").strip()
        out_key = str(params.get("out_key") or "").strip()
        preview_key = str(params.get("preview_key") or "").strip()
        metadata = dict(params.get("metadata") or {})
        if source_item_count <= 0:
            raise RuntimeError("bilevel finalize requires source_item_count > 0")
        if not fragment_prefix or not out_key or not preview_key:
            raise RuntimeError("bilevel finalize requires fragment_prefix, out_key, and preview_key")
        _phase(job_id, task_id, "started", "bilevel_finalize", "Assemble + encode")
        assemble_workers = _finalize_worker_count()
        finalize_s3 = _finalize_s3_client(assemble_workers)
        t_prep = time.time()
        fragment_urls = _presign_fragment_urls(
            finalize_s3=finalize_s3,
            fragment_prefix=fragment_prefix,
            source_item_count=source_item_count,
        )
        prep_ms = int((time.time() - t_prep) * 1000)
        _phase(
            job_id,
            task_id,
            "fragments_ready",
            "bilevel_finalize",
            "Assemble + encode",
            prep_ms=prep_ms,
            workers=assemble_workers,
        )

        t_assemble = time.time()
        hist_meta = _assemble_sparse_bilevel_raw(
            pix=pix,
            raw_path=_TMP_FINAL_RAW,
            hist_path=_TMP_FINAL_HIST,
            workers=assemble_workers,
            fragment_urls=fragment_urls,
            manifest_path=_TMP_FINAL_URLS,
            progress_cb=lambda assemble_ms: _phase(
                job_id,
                task_id,
                "assembling",
                "bilevel_finalize",
                "Assemble + encode",
                prep_ms=prep_ms,
                assemble_ms=assemble_ms,
                workers=assemble_workers,
            ),
        )
        assemble_ms = int((time.time() - t_assemble) * 1000)
        if not os.path.exists(_TMP_FINAL_RAW):
            raise RuntimeError("assemble_greyscale did not produce final raw output")
        raw_size = os.path.getsize(_TMP_FINAL_RAW)

        t_render = time.time()
        result = subprocess.run(
            [
                RAW_TO_BILEVEL,
                _TMP_FINAL_RAW,
                _TMP_FINAL_TIF,
                f"--pix={pix}",
                "--threshold=0",
                f"--preview={_TMP_FINAL_PREVIEW}",
                "--preview_size=1024",
            ],
            capture_output=True,
            text=True,
            timeout=600,
            env=imgpipe_env(),
        )
        if result.returncode != 0:
            raise RuntimeError(f"raw_to_bilevel failed: {result.stderr.strip()}")
        render_meta = json.loads(result.stdout)
        render_ms = int((time.time() - t_render) * 1000)

        t_upload = time.time()
        upload_meta = _build_bilevel_upload_metadata(
            metadata=metadata,
            width=width,
            height=height,
            source_item_count=source_item_count,
        )

        _upload_file(_TMP_FINAL_TIF, out_key, content_type="image/tiff", metadata=upload_meta)
        _upload_file(_TMP_FINAL_PREVIEW, preview_key, content_type="image/png")
        upload_ms = int((time.time() - t_upload) * 1000)

        _phase(
            job_id,
            task_id,
            "done",
            "bilevel_finalize",
            "Assemble + encode",
            prep_ms=prep_ms,
            assemble_ms=assemble_ms,
            render_ms=render_ms,
            upload_ms=upload_ms,
            workers=assemble_workers,
            raw_size=raw_size,
            file_size=int(render_meta.get("file_size", 0) or 0),
            pixels_set=int(hist_meta.get("nonzero_pixels", 0) or 0),
        )
        return ok_response({
            "out_key": out_key,
            "preview_key": preview_key,
            "pixels_set": int(hist_meta.get("nonzero_pixels", 0) or 0),
            "file_size": int(render_meta.get("file_size", 0) or 0),
            "prep_ms": prep_ms,
            "assemble_ms": assemble_ms,
            "render_ms": render_ms,
            "upload_ms": upload_ms,
            "workers": assemble_workers,
            "raw_size": raw_size,
        })
    except Exception as e:
        _report_phase_error(
            params,
            str(e),
            phase="bilevel_finalize",
            phase_label="Assemble + encode",
            extra_keys=("pix", "source_item_count", "fragment_prefix", "out_key", "preview_key"),
        )
        raise
    finally:
        _cleanup_tmp([_TMP_FINAL_RAW, _TMP_FINAL_HIST, _TMP_FINAL_TIF, _TMP_FINAL_PREVIEW, _TMP_FINAL_URLS])


def handle_from_raw_color(params):
    """Create a bilevel artifact by thresholding a fused Color raw sidecar."""
    job_id = str(params.get("job_id") or "").strip()
    task_id = str(params.get("task_id") or "color_to_bilevel").strip() or "color_to_bilevel"
    artifact_id = str(params.get("artifact_id") or "").strip()
    source_artifact_id = str(params.get("source_artifact_id") or "").strip()
    threshold_raw = params.get("threshold", 0)
    progress = {
        "phase": "bilevel_from_raw_prepare",
        "phase_label": "Color2Bilevel: source",
    }
    if artifact_id:
        progress["artifact_id"] = artifact_id
    if source_artifact_id:
        progress["source_artifact_id"] = source_artifact_id
    if threshold_raw not in ("", None):
        progress["threshold_raw"] = str(threshold_raw)

    def _report_progress(status, phase, phase_label, **extra):
        progress["phase"] = phase
        progress["phase_label"] = phase_label
        progress.update(extra)
        report_status(job_id, task_id, status, result_data=dict(progress))

    try:
        if not job_id:
            raise RuntimeError("from_raw_color requires job_id")
        if not artifact_id:
            raise RuntimeError("from_raw_color requires artifact_id")
        if not source_artifact_id:
            raise RuntimeError("from_raw_color requires source_artifact_id")
        try:
            threshold = int(threshold_raw)
        except (TypeError, ValueError) as e:
            raise RuntimeError(f"threshold must be an integer in [0,255], got {threshold_raw!r}") from e
        if not (0 <= threshold <= 255):
            raise RuntimeError(f"threshold must be in [0,255], got {threshold}")

        progress["threshold"] = threshold
        progress.pop("threshold_raw", None)
        progress["source_artifact_id"] = source_artifact_id
        _report_progress("started", "bilevel_from_raw_prepare", "Color2Bilevel: source")
        source_head = load_color_artifact_head(s3, BUCKET, job_id, source_artifact_id)
        source_meta = dict(source_head.get("metadata") or {})
        raw_key = str(source_meta.get("raw_key") or "").strip()
        raw_meta_key = str(source_meta.get("raw_meta_key") or "").strip()
        if not raw_key or not raw_meta_key:
            raise RuntimeError("Color2Bilevel requires a fused greyscale raw sidecar on the source artifact")
        raw_sidecar = validate_raw_sidecar(
            _load_json_key(raw_meta_key),
            expected_raw_key=raw_key,
            expected_artifact_family="color",
        )
        width = int(raw_sidecar["width"])
        height = int(raw_sidecar["height"])
        if width != height:
            raise RuntimeError(f"Color2Bilevel requires square raw sidecar, got {width}x{height}")
        pix = width
        _report_progress(
            "source_ready",
            "bilevel_from_raw_prepare",
            "Color2Bilevel: source",
            width=width,
            height=height,
            source_key=raw_key,
        )
        image_key = f"renders/{job_id}/bilevel/{artifact_id}/image.tif"
        preview_key = f"renders/{job_id}/bilevel/{artifact_id}/preview.png"

        t_dl = time.time()
        _download_to_path(raw_key, _TMP_SOURCE_RAW)
        dl_ms = int((time.time() - t_dl) * 1000)
        source_size = os.path.getsize(_TMP_SOURCE_RAW)
        _report_progress(
            "raw_downloaded",
            "bilevel_from_raw_download",
            "Color2Bilevel: raw download",
            dl_ms=dl_ms,
            source_size=source_size,
        )

        cmd = [
            RAW_TO_BILEVEL, _TMP_SOURCE_RAW, _TMP_FINAL_TIF,
            f"--pix={pix}",
            f"--threshold={threshold}",
            f"--preview={_TMP_FINAL_PREVIEW}",
            "--preview_size=1024",
        ]
        t_render = time.time()
        stop_heartbeat = threading.Event()
        heartbeat_thread = None
        if RAW_TO_BILEVEL_PROGRESS_INTERVAL_S > 0:
            def _heartbeat():
                next_tick = t_render + RAW_TO_BILEVEL_PROGRESS_INTERVAL_S
                while True:
                    delay = max(0.0, next_tick - time.time())
                    if stop_heartbeat.wait(delay):
                        break
                    _report_progress(
                        "rendering",
                        "bilevel_from_raw_render",
                        "Color2Bilevel: threshold + encode",
                        dl_ms=dl_ms,
                        render_ms=int((time.time() - t_render) * 1000),
                        source_size=source_size,
                    )
                    now = time.time()
                    while next_tick <= now:
                        next_tick += RAW_TO_BILEVEL_PROGRESS_INTERVAL_S

            heartbeat_thread = threading.Thread(target=_heartbeat, daemon=True)
            heartbeat_thread.start()
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=600, env=imgpipe_env())
        finally:
            stop_heartbeat.set()
            if heartbeat_thread is not None:
                heartbeat_thread.join(timeout=1.0)
        if result.returncode != 0:
            raise RuntimeError(f"raw_to_bilevel failed: {result.stderr.strip()}")
        render_meta = json.loads(result.stdout)
        render_ms = int((time.time() - t_render) * 1000)
        file_size = int(render_meta.get("file_size", 0) or 0)
        _report_progress(
            "rendered",
            "bilevel_from_raw_render",
            "Color2Bilevel: threshold + encode",
            dl_ms=dl_ms,
            render_ms=render_ms,
            source_size=source_size,
            file_size=file_size,
        )

        created_at = _utc_now_iso()
        out_meta = {
            "artifact_id": artifact_id,
            "family": "bilevel",
            "created_at": created_at,
            "format": "tif",
            "mode": "bilevel",
            "width": str(width),
            "height": str(height),
            "pix": str(source_meta.get("pix") or width),
            "derived_from_artifact_id": source_artifact_id,
            "derived_from_image_key": str(source_head.get("image_key") or source_meta.get("image_key") or ""),
            "postprocess_kind": "color_to_bilevel",
            "postprocess_profile": "raw_threshold_v1",
            "threshold": str(threshold),
        }
        for key in _DIRECT_VIEWPORT_METADATA_KEYS:
            value = source_meta.get(key)
            if value not in ("", None):
                out_meta[key] = value
        _report_progress(
            "uploading",
            "bilevel_from_raw_upload",
            "Color2Bilevel: upload",
            dl_ms=dl_ms,
            render_ms=render_ms,
            source_size=source_size,
            file_size=file_size,
        )
        t_upload = time.time()
        _upload_file(_TMP_FINAL_TIF, image_key, content_type="image/tiff", metadata=out_meta)
        _upload_file(_TMP_FINAL_PREVIEW, preview_key, content_type="image/png")
        upload_ms = int((time.time() - t_upload) * 1000)

        report_status(
            job_id,
            task_id,
            "done",
            result_data={
                "phase": "done",
                "family": "bilevel",
                "artifact_id": artifact_id,
                "image_key": image_key,
                "preview_key": preview_key,
                "derived_from_artifact_id": source_artifact_id,
                "threshold": threshold,
                "dl_ms": dl_ms,
                "render_ms": render_ms,
                "upload_ms": upload_ms,
                "source_size": source_size,
                "file_size": file_size,
                "width": progress.get("width"),
                "height": progress.get("height"),
            },
        )
        return ok_response({
            "artifact_id": artifact_id,
            "image_key": image_key,
            "preview_key": preview_key,
            "threshold": threshold,
            "dl_ms": dl_ms,
            "render_ms": render_ms,
            "upload_ms": upload_ms,
            "file_size": file_size,
        })
    except Exception as e:
        if job_id:
            report_status(job_id, task_id, "error", str(e), result_data=dict(progress))
        raise
    finally:
        _cleanup_tmp([_TMP_SOURCE_RAW, _TMP_FINAL_TIF, _TMP_FINAL_PREVIEW])
