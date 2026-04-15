"""
Raster Lambda handler — converts root data (.bin) to tile-bucketed .pix files.

Single operation: download .bin from S3, run roots2pix (tile-bucketed sparse
pixel output), upload non-empty .pix files to S3.
Reports completion status to DynamoDB for poll-based orchestration.
"""
import json
import os
import subprocess
import time

import boto3

from solve_score_chain import solve_score_program_cli_payload
from shared import BUCKET, attach_contract_warnings, contract_param, parse_body, ok_response, report_status

s3 = boto3.client("s3")
ROOTS2PIX = os.path.join(os.path.dirname(__file__), "roots2pix")
_TMP_SCORE_COEFFS = "/tmp/score_coeffs.bin"
_TMP_SCORE_PARAMS = "/tmp/score_params.bin"


def _tile_dense_bytes(tile_idx, width, height, tile_size, n_tile_cols):
    tile_idx = int(tile_idx)
    tile_size = int(tile_size)
    width = int(width)
    height = int(height)
    n_tile_cols = int(n_tile_cols)
    col = tile_idx % n_tile_cols
    row = tile_idx // n_tile_cols
    tile_w = max(0, min(tile_size, width - col * tile_size))
    tile_h = max(0, min(tile_size, height - row * tile_size))
    return tile_w * tile_h


def _parse_boolish(value, default=True):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _solve_score_program_args(ss_data):
    payload = solve_score_program_cli_payload({
        "metrics": ss_data.get("metrics") or [],
        "program_spec": str(ss_data.get("program") or ""),
    })
    args = [
        f"--score_metrics={payload['score_metrics']}",
        f"--score_clip_los={payload['score_clip_los']}",
        f"--score_clip_his={payload['score_clip_his']}",
        f"--score_program={payload['score_program']}",
    ]
    if payload.get("score_sources"):
        args.append(f"--score_sources={payload['score_sources']}")
    return args


def _solve_score_bins_uses_source(ss_data, source):
    for metric in (ss_data.get("metrics") or []):
        if str(metric.get("source", "slv")).strip().lower() == source:
            return True
    return False


def handler(event, context):
    params = parse_body(event)
    contract_warnings = []
    job_id = params["job_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    if chunk_idx is None:
        raise RuntimeError("raster requires chunk_idx")
    bin_key = params["bin_key"]
    n_tile_cols = params["n_tile_cols"]
    n_tile_rows = params["n_tile_rows"]
    n_tiles = n_tile_cols * n_tile_rows
    task_id = params.get("task_id", f"raster_{chunk_idx}")

    try:
        report_status(job_id, task_id, "started", result_data=attach_contract_warnings({"phase": "raster", "chunk_idx": chunk_idx}, contract_warnings))
        download_us = 0
        upload_us = 0

        # Download .bin from S3
        t_dl = time.perf_counter()
        bin_path = "/tmp/stripe.bin"
        saved_bins_path = "/tmp/palette_bins_chunk.bin"
        try:
            obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
        except Exception as e:
            raise RuntimeError(f"Failed to download root chunk s3://{BUCKET}/{bin_key}: {e}") from e
        with open(bin_path, "wb") as f:
            f.write(obj["Body"].read())

        report_status(job_id, task_id, "bin_downloaded")

        # Clean stale .pix files from previous invocations (warm container reuse)
        import glob
        for stale in glob.glob("/tmp/pix_t*.pix"):
            os.remove(stale)
        for stale in glob.glob("/tmp/pixbin_t*.pbx"):
            os.remove(stale)

        # Run roots2pix — writes tile-bucketed .pix files to /tmp/pix_t*.pix
        t1 = time.time()
        cmd = [
            ROOTS2PIX, bin_path, "/tmp/pix",
            f"--width={params['width']}", f"--height={params['height']}",
            f"--tile_size={params['tile_size']}",
            f"--n_tile_cols={n_tile_cols}",
            f"--n_tile_rows={n_tile_rows}",
            f"--center_re={params['center_re']}",
            f"--center_im={params['center_im']}",
            f"--scale={params['scale']}",
            f"--degree={params['degree']}",
            f"--color={contract_param(params, 'color', 'rainbow', contract_warnings)}",
            f"--match={contract_param(params, 'match', 'none', contract_warnings)}",
            f"--palette={contract_param(params, 'palette', 'inferno', contract_warnings)}",
            f"--constant_color={contract_param(params, 'constant_color', 'ffffff', contract_warnings)}",
            f"--rotation={contract_param(params, 'rotation', 0.0, contract_warnings)}",
        ]
        # Write root transforms sidecar if present
        rt_chain = contract_param(params, "root_transforms", [], contract_warnings)
        if rt_chain:
            rt_path = "/tmp/root_xforms.json"
            with open(rt_path, "w") as rtf:
                rtf.write(json.dumps(rt_chain))
            cmd.append(f"--root_xforms={rt_path}")
        emit_pixel_bins = _parse_boolish(params.get("emit_pixel_bins"), False)
        pixel_bins_drive_rgb = emit_pixel_bins and _parse_boolish(params.get("pixel_bins_drive_rgb"), False)
        if emit_pixel_bins:
            cmd.append("--pixel_bin_prefix=/tmp/pixbin")
        if pixel_bins_drive_rgb:
            cmd.append("--skip_pix_output=1")

        # Solve-score bins: download JSON, parse, pass as CLI args
        color = contract_param(params, "color", "rainbow", contract_warnings)
        ss_bins_key = params.get("solve_score_bins_key") or params.get("solve_proximity_bins_key")
        saved_palette_bins_key = params.get("saved_palette_bins_key")
        if color in ("solve_score", "solve_proximity") and not ss_bins_key:
            raise RuntimeError(f"{color} color mode requires solve_score_bins_key")
        if color == "saved_palette":
            if not saved_palette_bins_key:
                raise RuntimeError("saved_palette color mode requires saved_palette_bins_key")
            try:
                bins_obj = s3.get_object(Bucket=BUCKET, Key=saved_palette_bins_key)
            except Exception as e:
                raise RuntimeError(f"Failed to download saved palette bins s3://{BUCKET}/{saved_palette_bins_key}: {e}") from e
            with open(saved_bins_path, "wb") as bf:
                bf.write(bins_obj["Body"].read())
            cmd = [a for a in cmd if not a.startswith("--color=")]
            cmd.append("--color=saved_palette")
            cmd.append(f"--solve_bins_file={saved_bins_path}")
        if ss_bins_key and color in ("solve_score", "solve_proximity"):
            try:
                ss_obj = s3.get_object(Bucket=BUCKET, Key=ss_bins_key)
            except Exception as e:
                raise RuntimeError(f"Failed to download solve-score bins s3://{BUCKET}/{ss_bins_key}: {e}") from e
            ss_data = json.loads(ss_obj["Body"].read())
            # Validate bins artifact — must have family and matching metric
            if ss_data.get("family") != "solve_score":
                raise RuntimeError(f"Bins artifact missing or wrong family: {ss_data.get('family')}")
            cmd.append(f"--color=solve_score")
            cmd.append(f"--solve_score_cuts={','.join(str(c) for c in ss_data['cuts_norm'])}")
            if int(ss_data.get("version", 1) or 1) >= 2:
                if not ss_data.get("program") or not isinstance(ss_data.get("metrics"), list) or not ss_data.get("metrics"):
                    raise RuntimeError("v2 solve-score bins artifact is missing program or metrics")
                cmd.extend(_solve_score_program_args(ss_data))
                if _solve_score_bins_uses_source(ss_data, "cf"):
                    coeffs_key = str(params.get("coeffs_key") or "").strip()
                    n_coeffs = params.get("n_coeffs")
                    if not coeffs_key:
                        raise RuntimeError("mixed-source solve-score render requires coeffs_key")
                    try:
                        n_coeffs = int(n_coeffs)
                    except (TypeError, ValueError):
                        raise RuntimeError(f"mixed-source solve-score render requires numeric n_coeffs, got {n_coeffs!r}")
                    if n_coeffs < 1:
                        raise RuntimeError(f"mixed-source solve-score render requires n_coeffs >= 1, got {n_coeffs}")
                    try:
                        coeffs_obj = s3.get_object(Bucket=BUCKET, Key=coeffs_key)
                    except Exception as e:
                        raise RuntimeError(f"Failed to download coeff chunk s3://{BUCKET}/{coeffs_key}: {e}") from e
                    with open(_TMP_SCORE_COEFFS, "wb") as cf:
                        cf.write(coeffs_obj["Body"].read())
                    cmd.extend([
                        f"--score_coeffs_file={_TMP_SCORE_COEFFS}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                if _solve_score_bins_uses_source(ss_data, "pm"):
                    params_key = str(params.get("params_key") or "").strip()
                    params_step_start = params.get("params_step_start", params.get("step_start"))
                    params_step_count = params.get("params_step_count", params.get("step_count"))
                    step_count = params.get("step_count")
                    if not params_key:
                        raise RuntimeError("param-source solve-score render requires params_key")
                    try:
                        params_step_start = int(params_step_start)
                        params_step_count = int(params_step_count)
                        step_count = int(step_count)
                    except (TypeError, ValueError):
                        raise RuntimeError(
                            "param-source solve-score render requires numeric "
                            f"params_step_start/params_step_count/step_count, got {params_step_start!r}/{params_step_count!r}/{step_count!r}"
                        )
                    if params_step_start < 0 or params_step_count < 1 or step_count < 1:
                        raise RuntimeError(
                            "param-source solve-score render requires params_step_start >= 0 and "
                            f"params_step_count/step_count >= 1, got {params_step_start}/{params_step_count}/{step_count}"
                        )
                    if params_step_count != step_count:
                        raise RuntimeError(
                            f"param-source solve-score render requires params_step_count == step_count, got {params_step_count}/{step_count}"
                        )
                    try:
                        params_obj = s3.get_object(
                            Bucket=BUCKET,
                            Key=params_key,
                            Range=f"bytes={params_step_start * 16}-{params_step_start * 16 + params_step_count * 16 - 1}",
                        )
                    except Exception as e:
                        raise RuntimeError(f"Failed to download param slice s3://{BUCKET}/{params_key}: {e}") from e
                    with open(_TMP_SCORE_PARAMS, "wb") as pf:
                        pf.write(params_obj["Body"].read())
                    cmd.append(f"--score_params_file={_TMP_SCORE_PARAMS}")
            else:
                req_metric = contract_param(params, "solve_metric", "proximity", contract_warnings)
                if ss_data.get("metric") != req_metric:
                    raise RuntimeError(f"Bins metric mismatch: expected {req_metric}, got {ss_data.get('metric')}")
                req_q = contract_param(params, "solve_score_quantile", 0.001, contract_warnings)
                if "clip_quantile" not in ss_data:
                    raise RuntimeError("Bins artifact missing clip_quantile")
                if ss_data["clip_quantile"] != req_q:
                    raise RuntimeError(f"Bins quantile mismatch: expected {req_q}, got {ss_data['clip_quantile']}")
                req_omega = float(contract_param(params, "solve_score_omega", 1.0, contract_warnings))
                bins_omega = float(ss_data.get("omega", 1.0))
                if bins_omega != req_omega:
                    raise RuntimeError(f"Bins omega mismatch: expected {req_omega}, got {bins_omega}")
                req_omega_enabled = _parse_boolish(contract_param(params, "solve_score_omega_enabled", True, contract_warnings), True)
                bins_omega_enabled = _parse_boolish(ss_data.get("omega_enabled", True), True)
                if bins_omega_enabled != req_omega_enabled:
                    raise RuntimeError(f"Bins omega_enabled mismatch: expected {req_omega_enabled}, got {bins_omega_enabled}")
                ss_metric = ss_data.get("metric", params.get("solve_metric", "proximity"))
                cmd.append(f"--solve_metric={ss_metric}")
                cmd.append(f"--solve_score_clip_lo={ss_data['clip_lo']}")
                cmd.append(f"--solve_score_clip_hi={ss_data['clip_hi']}")
                cmd.append(f"--solve_score_omega={bins_omega}")
                cmd.append(f"--solve_score_omega_enabled={1 if bins_omega_enabled else 0}")
            # Override the color arg already in cmd (was set to "solve_proximity" or "solve_score")
            cmd = [a for a in cmd if not a.startswith("--color=") or a == f"--color=solve_score"]
        download_us = int((time.perf_counter() - t_dl) * 1e6)

        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            raise RuntimeError(f"roots2pix failed: {result.stderr.strip()}")
        raster_meta = json.loads(result.stdout)
        raster_us = int((time.time() - t1) * 1e6)

        os.remove(bin_path)
        if os.path.exists(saved_bins_path):
            os.remove(saved_bins_path)

        report_status(job_id, task_id, "rasterized")

        # Upload .pix files — stream from file, do NOT f.read() into memory
        t_up = time.perf_counter()
        uploaded = 0
        uploaded_pixel_bins = 0
        uploaded_pixel_bin_bytes = 0
        pixel_bin_tile_bytes = []
        skipped_pix_tiles = 0
        for t in range(n_tiles):
            pix_path = f"/tmp/pix_t{t:04d}.pix"
            if os.path.exists(pix_path) and os.path.getsize(pix_path) > 0:
                if pixel_bins_drive_rgb:
                    skipped_pix_tiles += 1
                else:
                    s3_key = f"renders/{job_id}/pix_chunk_{chunk_idx:04d}_t{t:04d}.pix"
                    with open(pix_path, "rb") as fh:
                        s3.upload_fileobj(fh, BUCKET, s3_key)
                    uploaded += 1
                os.remove(pix_path)
            pbx_path = f"/tmp/pixbin_t{t:04d}.pbx"
            if emit_pixel_bins and os.path.exists(pbx_path):
                pbx_size = os.path.getsize(pbx_path)
                if pbx_size > 0:
                    pbx_key = f"renders/{job_id}/pixbin_chunk_{chunk_idx:04d}_t{t:04d}.pbx"
                    with open(pbx_path, "rb") as fh:
                        s3.upload_fileobj(fh, BUCKET, pbx_key)
                    uploaded_pixel_bins += 1
                    uploaded_pixel_bin_bytes += pbx_size
                    pixel_bin_tile_bytes.append({
                        "tile_idx": t,
                        "bytes": pbx_size,
                        "dense_bytes": _tile_dense_bytes(t, params["width"], params["height"], params["tile_size"], n_tile_cols),
                    })
                os.remove(pbx_path)
        if pixel_bins_drive_rgb:
            skipped_pix_tiles = max(skipped_pix_tiles, int(raster_meta.get("tiles_with_data", 0) or 0))
        upload_us = int((time.perf_counter() - t_up) * 1e6)

        result_data = attach_contract_warnings({
            "engine": "single",
            "threads": 1,
            "download_us": download_us,
            "native_us": raster_us,
            "upload_us": upload_us,
            "tiles_uploaded": uploaded,
            "pixel_bin_tiles_uploaded": uploaded_pixel_bins,
            "pixel_bin_bytes_uploaded": uploaded_pixel_bin_bytes,
            "pixel_bin_tile_bytes": pixel_bin_tile_bytes,
            "pixel_bin_dense_bytes_if_full_tiles": sum(item["dense_bytes"] for item in pixel_bin_tile_bytes),
            "pixel_bins_drive_rgb": pixel_bins_drive_rgb,
            "rgb_source": "pixel_bins" if pixel_bins_drive_rgb else "pix",
            "pix_tiles_skipped": skipped_pix_tiles,
            "roots_plotted": raster_meta["roots_plotted"],
            "roots_clipped": raster_meta["roots_clipped"],
        }, contract_warnings)
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response({
            "chunk_idx": chunk_idx,
            "stripe_idx": chunk_idx,
            "tiles_uploaded": uploaded,
            "pixel_bin_tiles_uploaded": uploaded_pixel_bins,
            "pixel_bin_bytes_uploaded": uploaded_pixel_bin_bytes,
            "pixel_bin_tile_bytes": pixel_bin_tile_bytes,
            "pixel_bin_dense_bytes_if_full_tiles": sum(item["dense_bytes"] for item in pixel_bin_tile_bytes),
            "pixel_bins_drive_rgb": pixel_bins_drive_rgb,
            "rgb_source": "pixel_bins" if pixel_bins_drive_rgb else "pix",
            "pix_tiles_skipped": skipped_pix_tiles,
            "raster_us": raster_us,
            "roots_plotted": raster_meta["roots_plotted"],
            "roots_clipped": raster_meta["roots_clipped"],
            "engine": "single",
            "threads": 1,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=attach_contract_warnings({"phase": "raster", "chunk_idx": chunk_idx}, contract_warnings))
        raise
    finally:
        for tmp_path in ("/tmp/stripe.bin", "/tmp/palette_bins_chunk.bin", "/tmp/root_xforms.json", _TMP_SCORE_COEFFS):
            try:
                os.remove(tmp_path)
            except OSError:
                pass
