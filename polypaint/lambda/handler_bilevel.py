"""
Bilevel Lambda handler.

Legacy phases:
  phase=raster: one chunk -> per-tile bitset files (.bits)
  phase=coeff_raster: one coeff chunk -> per-tile bitset files (.bits)
  phase=merge: per-tile bitsets -> tile TIFF

Fused phases:
  phase=section_raster: one logical section -> one full-frame section bitset
  phase=finalize: OR full-frame section bitsets -> final TIFF + preview
  phase=from_raw_color: threshold a fused Color greyscale raw sidecar -> bilevel
"""
from __future__ import annotations

import glob
import json
import os
import subprocess
import threading
import time
from datetime import datetime, timezone

import boto3
from botocore.exceptions import ClientError

from color_artifact_meta import load_color_artifact_head
from logical_sections import build_source_spans, stitch_spans_to_file
from raw_sidecar import validate_raw_sidecar
from shared import BUCKET, imgpipe_env, ok_response, parse_body, report_status

s3 = boto3.client("s3")
BILEVEL_RASTER = os.path.join(os.path.dirname(__file__), "bilevel_raster")
BILEVEL_SECTION_RASTER = os.path.join(os.path.dirname(__file__), "bilevel_section_raster")
COEFFS_BILEVEL_RASTER = os.path.join(os.path.dirname(__file__), "coeffs_bilevel_raster")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")
RAW_TO_BILEVEL = os.path.join(os.path.dirname(__file__), "raw_to_bilevel")

_TMP_ROOTS = "/tmp/stripe.bin"
_TMP_COEFFS = "/tmp/coeffs.bin"
_TMP_SECTION_BITS = "/tmp/section.bits"
_TMP_FINAL_TIF = "/tmp/final.tif"
_TMP_FINAL_PREVIEW = "/tmp/final_preview.png"
_TMP_SOURCE_RAW = "/tmp/source_greyscale.raw"
_TMP_ROOT_XFORMS = "/tmp/root_xforms.json"
RAW_TO_BILEVEL_PROGRESS_INTERVAL_S = float(os.environ.get("RAW_TO_BILEVEL_PROGRESS_INTERVAL_S", "5") or "5")


def _utc_now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status, result_data={"phase": phase, "phase_label": phase_label, **extra})


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


def _report_handler_entry_error(params, message, *, phase_raw=""):
    job_id = str((params or {}).get("job_id") or "").strip()
    if not job_id:
        return
    task_id = str((params or {}).get("task_id") or "bilevel").strip() or "bilevel"
    result_data = {
        "phase": "handler_entry",
        "phase_label": "BiLevel dispatch",
    }
    if phase_raw not in ("", None):
        result_data["phase_raw"] = str(phase_raw)
    for key in ("artifact_id", "source_artifact_id", "threshold"):
        value = (params or {}).get(key)
        if value not in ("", None):
            result_data[key] = str(value)
    report_status(job_id, task_id, "error", message, result_data=result_data)


def handler(event, context):
    params = parse_body(event)
    phase = str(params.get("phase") or "").strip()
    if not phase:
        message = "Bilevel handler requires non-empty phase"
        _report_handler_entry_error(params, message, phase_raw=params.get("phase"))
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
    _report_handler_entry_error(params, message, phase_raw=phase)
    raise ValueError(message)


def handle_raster(params):
    """One chunk -> per-tile bitset files. One Lambda per chunk."""
    job_id = params["job_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    if chunk_idx is None:
        raise RuntimeError("bilevel raster requires chunk_idx")
    task_id = params.get("task_id", f"bilevel_raster_{chunk_idx}")

    try:
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

        cmd = [
            BILEVEL_RASTER, bin_path, "/tmp/bits",
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
        report_status(job_id, task_id, "error", str(e))
        _cleanup_tmp([_TMP_ROOTS, _TMP_ROOT_XFORMS, "/tmp/bits_t*.bits"])
        raise


def handle_coeff_raster(params):
    """One coeff chunk -> per-tile bitset files. One Lambda per chunk."""
    job_id = params["job_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    if chunk_idx is None:
        raise RuntimeError("coeff bilevel raster requires chunk_idx")
    task_id = params.get("task_id", f"coeff_bilevel_raster_{chunk_idx}")

    try:
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

        cmd = [
            COEFFS_BILEVEL_RASTER, bin_path, "/tmp/coeff_bits",
            f"--width={params['width']}", f"--height={params['height']}",
            f"--tile_size={params['tile_size']}",
            f"--n_tile_cols={params['n_tile_cols']}",
            f"--n_tile_rows={params['n_tile_rows']}",
            f"--center_re={params['center_re']}",
            f"--center_im={params['center_im']}",
            f"--scale={params['scale']}",
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
        report_status(job_id, task_id, "error", str(e))
        _cleanup_tmp([_TMP_COEFFS, "/tmp/coeff_bits_t*.bits"])
        raise


def handle_merge(params):
    """OR per-chunk bitsets for one tile -> tile TIFF. One Lambda per tile."""
    job_id = params["job_id"]
    tile_idx = params["tile_idx"]
    tile_w = params.get("tile_w")
    tile_h = params.get("tile_h")
    if tile_w in (None, "") or tile_h in (None, ""):
        tile_w, tile_h = _tile_shape(
            tile_idx,
            params["width"],
            params["height"],
            params["tile_size"],
            params["n_tile_cols"],
        )
    n_chunks = params.get("n_chunks", params.get("n_stripes"))
    if n_chunks is None:
        raise RuntimeError("bilevel merge requires n_chunks")
    bits_prefix = params.get("bits_prefix", "bits")
    tile_prefix = params.get("tile_prefix", "bilevel")
    task_prefix = params.get("task_prefix", "bilevel_merge")
    task_id = params.get("task_id", f"{task_prefix}_{tile_idx}")

    try:
        report_status(job_id, task_id, "started")

        bits_paths = []
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

        out_path = "/tmp/tile.tif"
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
        report_status(job_id, task_id, "error", str(e))
        raise


def handle_section_raster(params):
    """One logical section -> one full-frame section bitset."""
    job_id = params["job_id"]
    section_idx = int(params.get("section_idx", 0) or 0)
    task_id = params.get("task_id", f"bilevel_section_{section_idx}")
    solve_source_manifest = dict(params.get("solve_source_manifest") or {})
    step_start = int(params.get("step_start", 0) or 0)
    step_count = int(params.get("step_count", 0) or 0)
    section_bits_prefix = str(params.get("section_bits_prefix") or "").strip()
    if not solve_source_manifest:
        raise RuntimeError("bilevel section raster requires solve_source_manifest")
    if step_count <= 0:
        raise RuntimeError("bilevel section raster requires step_count > 0")
    if not section_bits_prefix:
        raise RuntimeError("bilevel section raster requires section_bits_prefix")

    try:
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

        _cleanup_tmp([_TMP_ROOTS, _TMP_SECTION_BITS, _TMP_ROOT_XFORMS])
        t0 = time.time()
        stitch_spans_to_file(s3, BUCKET, root_spans, _TMP_ROOTS)
        dl_ms = int((time.time() - t0) * 1000)
        _phase(job_id, task_id, "downloaded", "bilevel_raster", "BiLevel raster", dl_ms=dl_ms)

        cmd = [
            BILEVEL_SECTION_RASTER, _TMP_ROOTS, _TMP_SECTION_BITS,
            f"--width={params['width']}",
            f"--height={params['height']}",
            f"--center_re={params['center_re']}",
            f"--center_im={params['center_im']}",
            f"--scale={params['scale']}",
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

        section_bits_key = f"{section_bits_prefix}{section_idx:04d}.bits"
        with open(_TMP_SECTION_BITS, "rb") as fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=section_bits_key,
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
            bits_size=int(meta.get("file_size", 0) or 0),
            section_idx=section_idx,
        )
        return ok_response({
            "section_idx": section_idx,
            "roots_plotted": int(meta.get("roots_plotted", 0) or 0),
            "roots_clipped": int(meta.get("roots_clipped", 0) or 0),
            "roots_deduped": int(meta.get("roots_deduped", 0) or 0),
            "bits_size": int(meta.get("file_size", 0) or 0),
            "dl_ms": dl_ms,
            "raster_ms": raster_ms,
            "section_bits_key": section_bits_key,
        })
    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        _cleanup_tmp([_TMP_ROOTS, _TMP_SECTION_BITS, _TMP_ROOT_XFORMS])
        raise


def handle_finalize(params):
    """OR logical-section bitsets and write the final bilevel TIFF + preview."""
    job_id = params["job_id"]
    task_id = params.get("task_id", "bilevel_finalize")
    width = int(params.get("width", 0) or 0)
    height = int(params.get("height", 0) or 0)
    source_item_count = int(params.get("source_item_count", 0) or 0)
    section_bits_prefix = str(params.get("section_bits_prefix") or "").strip()
    out_key = str(params.get("out_key") or "").strip()
    preview_key = str(params.get("preview_key") or "").strip()
    metadata = dict(params.get("metadata") or {})
    if width <= 0 or height <= 0:
        raise RuntimeError("bilevel finalize requires width and height > 0")
    if source_item_count <= 0:
        raise RuntimeError("bilevel finalize requires source_item_count > 0")
    if not section_bits_prefix or not out_key or not preview_key:
        raise RuntimeError("bilevel finalize requires section_bits_prefix, out_key, and preview_key")

    section_paths = []
    try:
        _phase(job_id, task_id, "started", "bilevel_finalize", "Assemble + encode")
        t_dl = time.time()
        for idx in range(source_item_count):
            key = f"{section_bits_prefix}{idx:04d}.bits"
            path = f"/tmp/section_{idx:04d}.bits"
            _download_to_path(key, path)
            section_paths.append(path)
        dl_ms = int((time.time() - t_dl) * 1000)
        _phase(job_id, task_id, "section_bits_downloaded", "bilevel_finalize", "Assemble + encode", dl_ms=dl_ms)

        cmd = [
            BILEVEL_MERGE, "assemble",
            f"--width={width}",
            f"--height={height}",
            f"--output={_TMP_FINAL_TIF}",
            f"--preview={_TMP_FINAL_PREVIEW}",
            "--preview_size=1024",
        ] + section_paths
        t_assemble = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600, env=imgpipe_env())
        if result.returncode != 0:
            raise RuntimeError(f"bilevel_merge assemble failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        assemble_ms = int((time.time() - t_assemble) * 1000)

        t_upload = time.time()
        upload_meta = {
            "artifact_id": str(metadata.get("artifact_id") or ""),
            "family": "bilevel",
            "created_at": str(metadata.get("created_at") or _utc_now_iso()),
            "format": "tif",
            "mode": "bilevel",
            "width": str(width),
            "height": str(height),
            "pix": str(metadata.get("pix") or width),
            "render_execution": json.dumps(metadata.get("render_execution") or {}, separators=(",", ":")),
            "bilevel_pipeline": "logical_sections_v1",
            "bilevel_section_mode": str(metadata.get("bilevel_section_mode") or ""),
            "bilevel_section_count": str(metadata.get("bilevel_section_count") or source_item_count),
        }
        if metadata.get("derived_from_artifact_id"):
            upload_meta["derived_from_artifact_id"] = str(metadata["derived_from_artifact_id"])
        if metadata.get("derived_from_image_key"):
            upload_meta["derived_from_image_key"] = str(metadata["derived_from_image_key"])

        _upload_file(_TMP_FINAL_TIF, out_key, content_type="image/tiff", metadata=upload_meta)
        _upload_file(_TMP_FINAL_PREVIEW, preview_key, content_type="image/png")
        upload_ms = int((time.time() - t_upload) * 1000)

        _phase(
            job_id,
            task_id,
            "done",
            "bilevel_finalize",
            "Assemble + encode",
            dl_ms=dl_ms,
            assemble_ms=assemble_ms,
            upload_ms=upload_ms,
            pixels_set=int(meta.get("pixels_set", 0) or 0),
        )
        return ok_response({
            "out_key": out_key,
            "preview_key": preview_key,
            "pixels_set": int(meta.get("pixels_set", 0) or 0),
            "file_size": int(meta.get("file_size", 0) or 0),
            "dl_ms": dl_ms,
            "assemble_ms": assemble_ms,
            "upload_ms": upload_ms,
        })
    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
    finally:
        for path in section_paths:
            try:
                os.remove(path)
            except OSError:
                pass
        _cleanup_tmp([_TMP_FINAL_TIF, _TMP_FINAL_PREVIEW])


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
            f"--width={width}",
            f"--height={height}",
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
                while not stop_heartbeat.wait(max(0.0, next_tick - time.time())):
                    _report_progress(
                        "rendering",
                        "bilevel_from_raw_render",
                        "Color2Bilevel: threshold + encode",
                        dl_ms=dl_ms,
                        render_ms=int((time.time() - t_render) * 1000),
                        source_size=source_size,
                    )
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
