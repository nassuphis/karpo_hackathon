"""
Raster MT Lambda handler — true native multithreaded raster.

Downloads one chunk, invokes roots2pix_mt once with --threads=N, uploads the
standard chunk/tile .pix and .pbx outputs, and reports comparable perf data.
"""
import glob
import json
import os
import subprocess
import time

import boto3

from solve_score_chain import solve_score_program_cli_payload
from shared import BUCKET, attach_contract_warnings, contract_param, ok_response, parse_body, report_status

s3 = boto3.client("s3")
ROOTS2PIX_MT = os.path.join(os.path.dirname(__file__), "roots2pix_mt")
DEFAULT_THREADS = int(os.environ.get("RASTER_MT_THREADS", "4") or "4")
VALID_RASTER_INPUT_MODES = {"tmpfile", "sectioned"}
_TMP_SCORE_COEFFS = "/tmp/score_coeffs.bin"
_TMP_SCORE_PARAMS = "/tmp/score_params.bin"


def _validate_threads(value):
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"raster_mt_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"raster_mt_threads must be in [1, 16], got {threads}")
    return threads


def _parse_boolish(value, default=True):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _validate_raster_input_mode(value):
    mode = str(value or "tmpfile").strip().lower()
    if mode not in VALID_RASTER_INPUT_MODES:
        raise RuntimeError(f"raster_input_mode must be one of {', '.join(sorted(VALID_RASTER_INPUT_MODES))}, got {value!r}")
    return mode


def _validate_sectioned_retries(value):
    if value in (None, ""):
        value = 2
    try:
        retries = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"raster_sectioned_retries must be an integer, got {value!r}")
    if not (0 <= retries <= 10):
        raise RuntimeError(f"raster_sectioned_retries must be in [0, 10], got {retries}")
    return retries


def _sectioned_input_size_limit():
    try:
        memory_mb = int(os.environ.get("AWS_LAMBDA_FUNCTION_MEMORY_SIZE", "0") or 0)
    except (TypeError, ValueError):
        memory_mb = 0
    if memory_mb <= 0:
        return 0
    return (memory_mb * 1024 * 1024) // 2


def _cleanup_tmp():
    for pattern in (
        "/tmp/pix_t*.pix",
        "/tmp/pixbin_t*.pbx",
        "/tmp/stripe.bin",
        "/tmp/palette_bins_chunk.bin",
        _TMP_SCORE_COEFFS,
        _TMP_SCORE_PARAMS,
        "/tmp/root_xforms.json",
    ):
        for stale in glob.glob(pattern):
            try:
                os.remove(stale)
            except OSError:
                pass


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


def _build_cmd(params, bin_path, saved_bins_path=None):
    cmd = [
        ROOTS2PIX_MT, bin_path, "/tmp/pix",
        f"--width={params['width']}",
        f"--height={params['height']}",
        f"--tile_size={params['tile_size']}",
        f"--n_tile_cols={params['n_tile_cols']}",
        f"--n_tile_rows={params['n_tile_rows']}",
        f"--center_re={params['center_re']}",
        f"--center_im={params['center_im']}",
        f"--scale={params['scale']}",
        f"--degree={params['degree']}",
        f"--color={params.get('color', 'rainbow')}",
        f"--match={params.get('match', 'none')}",
        f"--palette={params.get('palette', 'inferno')}",
        f"--constant_color={params.get('constant_color', 'ffffff')}",
        f"--rotation={params.get('rotation', 0.0)}",
        f"--threads={params['raster_mt_threads']}",
        f"--input_mode={params.get('raster_input_mode', 'tmpfile')}",
    ]
    if params.get("raster_input_mode") == "sectioned":
        cmd.extend([
            f"--url={params['sectioned_url']}",
            f"--input_size={params['sectioned_input_size']}",
            f"--retries={params.get('raster_sectioned_retries', 2)}",
        ])
    if params.get("emit_pixel_bins"):
        cmd.append("--pixel_bin_prefix=/tmp/pixbin")

    if params.get("color") == "saved_palette":
        if not saved_bins_path:
            raise RuntimeError("saved_palette color mode requires saved_palette_bins_key")
        cmd = [a for a in cmd if not a.startswith("--color=")]
        cmd.append("--color=saved_palette")
        cmd.append(f"--solve_bins_file={saved_bins_path}")

    ss_data = params.get("solve_score_bins_data")
    color = params.get("color", "rainbow")
    if ss_data and color in ("solve_score", "solve_proximity"):
        if ss_data.get("family") != "solve_score":
            raise RuntimeError(f"Bins artifact missing or wrong family: {ss_data.get('family')}")
        cmd = [a for a in cmd if not a.startswith("--color=")]
        cmd.extend(["--color=solve_score", f"--solve_score_cuts={','.join(str(c) for c in ss_data['cuts_norm'])}"])
        if int(ss_data.get("version", 1) or 1) >= 2:
            if not ss_data.get("program") or not isinstance(ss_data.get("metrics"), list) or not ss_data.get("metrics"):
                raise RuntimeError("v2 solve-score bins artifact is missing program or metrics")
            cmd.extend(_solve_score_program_args(ss_data))
            if _solve_score_bins_uses_source(ss_data, "cf"):
                n_coeffs = params.get("n_coeffs")
                try:
                    n_coeffs = int(n_coeffs)
                except (TypeError, ValueError):
                    raise RuntimeError(f"mixed-source solve-score render requires numeric n_coeffs, got {n_coeffs!r}")
                if n_coeffs < 1:
                    raise RuntimeError(f"mixed-source solve-score render requires n_coeffs >= 1, got {n_coeffs}")
                if params.get("raster_input_mode") == "sectioned":
                    coeffs_url = str(params.get("sectioned_score_coeffs_url") or "").strip()
                    coeff_input_size = int(params.get("sectioned_score_coeffs_input_size") or 0)
                    if not coeffs_url or coeff_input_size <= 0:
                        raise RuntimeError("mixed-source sectioned raster requires coeff presign URL and size")
                    cmd.extend([
                        f"--score_coeffs_url={coeffs_url}",
                        f"--score_coeff_input_size={coeff_input_size}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                else:
                    coeffs_path = str(params.get("solve_score_coeffs_path") or "").strip()
                    if not coeffs_path:
                        raise RuntimeError("mixed-source tmpfile raster requires solve_score_coeffs_path")
                    cmd.extend([
                        f"--score_coeffs_file={coeffs_path}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
            if _solve_score_bins_uses_source(ss_data, "pm"):
                params_path = str(params.get("solve_score_params_path") or "").strip()
                if not params_path:
                    raise RuntimeError("param-source raster requires solve_score_params_path")
                cmd.append(f"--score_params_file={params_path}")
        else:
            req_metric = params.get("solve_metric", "proximity")
            if ss_data.get("metric") != req_metric:
                raise RuntimeError(f"Bins metric mismatch: expected {req_metric}, got {ss_data.get('metric')}")
            req_q = params.get("solve_score_quantile", 0.001)
            if "clip_quantile" not in ss_data:
                raise RuntimeError("Bins artifact missing clip_quantile")
            if ss_data["clip_quantile"] != req_q:
                raise RuntimeError(f"Bins quantile mismatch: expected {req_q}, got {ss_data['clip_quantile']}")
            req_omega = float(params.get("solve_score_omega", 1.0))
            bins_omega = float(ss_data.get("omega", 1.0))
            if bins_omega != req_omega:
                raise RuntimeError(f"Bins omega mismatch: expected {req_omega}, got {bins_omega}")
            req_omega_enabled = _parse_boolish(params.get("solve_score_omega_enabled", True), True)
            bins_omega_enabled = _parse_boolish(ss_data.get("omega_enabled", True), True)
            if bins_omega_enabled != req_omega_enabled:
                raise RuntimeError(
                    f"Bins omega_enabled mismatch: expected {req_omega_enabled}, got {bins_omega_enabled}"
                )
            cmd.extend([
                f"--solve_metric={ss_data.get('metric', req_metric)}",
                f"--solve_score_clip_lo={ss_data['clip_lo']}",
                f"--solve_score_clip_hi={ss_data['clip_hi']}",
                f"--solve_score_omega={bins_omega}",
                f"--solve_score_omega_enabled={1 if bins_omega_enabled else 0}",
            ])

    rt_path = params.get("root_xforms_path")
    if rt_path:
        cmd.append(f"--root_xforms={rt_path}")
    return cmd


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
    threads = _validate_threads(contract_param(params, "raster_mt_threads", DEFAULT_THREADS, contract_warnings))
    raster_input_mode = _validate_raster_input_mode(contract_param(params, "raster_input_mode", "tmpfile", contract_warnings))
    raster_sectioned_retries = _validate_sectioned_retries(contract_param(params, "raster_sectioned_retries", 2, contract_warnings))

    perf = attach_contract_warnings({
        "engine": "mt",
        "threads": threads,
        "input_mode": raster_input_mode,
        "retries": raster_sectioned_retries,
        "download_us": 0,
        "native_us": 0,
        "upload_us": 0,
        "tiles_uploaded": 0,
        "pixel_bin_tiles_uploaded": 0,
        "roots_plotted": 0,
        "roots_clipped": 0,
    }, contract_warnings)

    bin_path = "/tmp/stripe.bin"
    saved_bins_path = "/tmp/palette_bins_chunk.bin"
    emit_pixel_bins = bool(params.get("emit_pixel_bins"))

    try:
        report_status(job_id, task_id, "started", result_data=perf)
        _cleanup_tmp()

        params = dict(params)
        params["raster_mt_threads"] = threads
        params["raster_input_mode"] = raster_input_mode
        params["raster_sectioned_retries"] = raster_sectioned_retries
        params["match"] = contract_param(params, "match", "none", contract_warnings)
        params["palette"] = contract_param(params, "palette", "inferno", contract_warnings)
        params["constant_color"] = contract_param(params, "constant_color", "ffffff", contract_warnings)
        params["rotation"] = contract_param(params, "rotation", 0.0, contract_warnings)
        params["root_xforms_path"] = None
        rt_chain = contract_param(params, "root_transforms", [], contract_warnings)
        if rt_chain:
            rt_path = "/tmp/root_xforms.json"
            with open(rt_path, "w") as rtf:
                json.dump(rt_chain, rtf)
            params["root_xforms_path"] = rt_path

        ss_bins_key = params.get("solve_score_bins_key") or params.get("solve_proximity_bins_key")
        color = contract_param(params, "color", "rainbow", contract_warnings)
        if color in ("solve_score", "solve_proximity"):
            params["solve_metric"] = contract_param(params, "solve_metric", "proximity", contract_warnings)
            params["solve_score_quantile"] = contract_param(params, "solve_score_quantile", 0.001, contract_warnings)
            params["solve_score_omega"] = contract_param(params, "solve_score_omega", 1.0, contract_warnings)
            params["solve_score_omega_enabled"] = contract_param(params, "solve_score_omega_enabled", True, contract_warnings)
        if color in ("solve_score", "solve_proximity") and not ss_bins_key:
            raise RuntimeError(f"{color} color mode requires solve_score_bins_key")
        if color == "saved_palette":
            saved_palette_bins_key = params.get("saved_palette_bins_key")
            if not saved_palette_bins_key:
                raise RuntimeError("saved_palette color mode requires saved_palette_bins_key")
            bins_obj = s3.get_object(Bucket=BUCKET, Key=saved_palette_bins_key)
            with open(saved_bins_path, "wb") as bf:
                bf.write(bins_obj["Body"].read())
        elif ss_bins_key and color in ("solve_score", "solve_proximity"):
            ss_obj = s3.get_object(Bucket=BUCKET, Key=ss_bins_key)
            params["solve_score_bins_data"] = json.loads(ss_obj["Body"].read())
            if _solve_score_bins_uses_source(params["solve_score_bins_data"], "cf"):
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
                params["n_coeffs"] = n_coeffs
            if _solve_score_bins_uses_source(params["solve_score_bins_data"], "pm"):
                params_key = str(params.get("params_key") or "").strip()
                step_start = params.get("step_start")
                step_count = params.get("step_count")
                if not params_key:
                    raise RuntimeError("param-source solve-score render requires params_key")
                try:
                    step_start = int(step_start)
                    step_count = int(step_count)
                except (TypeError, ValueError):
                    raise RuntimeError(
                        f"param-source solve-score render requires numeric step_start/step_count, got {step_start!r}/{step_count!r}"
                    )
                if step_start < 0 or step_count < 1:
                    raise RuntimeError(
                        f"param-source solve-score render requires step_start >= 0 and step_count >= 1, got {step_start}/{step_count}"
                    )
                params["step_start"] = step_start
                params["step_count"] = step_count

        if raster_input_mode == "sectioned":
            input_size = int(params.get("bin_size") or 0)
            if input_size <= 0:
                head = s3.head_object(Bucket=BUCKET, Key=bin_key)
                input_size = int(head.get("ContentLength") or 0)
            if input_size <= 0:
                raise RuntimeError(f"Failed to determine size for s3://{BUCKET}/{bin_key}")
            size_limit = _sectioned_input_size_limit()
            if size_limit > 0 and input_size > size_limit:
                raise RuntimeError(
                    f"sectioned raster input too large for current Lambda memory: "
                    f"{input_size} bytes > safe limit {size_limit} bytes"
                )
            params["sectioned_input_size"] = input_size
            params["sectioned_url"] = s3.generate_presigned_url(
                "get_object",
                Params={"Bucket": BUCKET, "Key": bin_key},
                ExpiresIn=900,
            )
            if _solve_score_bins_uses_source(params.get("solve_score_bins_data") or {}, "cf"):
                coeff_input_size = int(params.get("coeffs_bin_size") or 0)
                coeffs_key = str(params.get("coeffs_key") or "").strip()
                if coeff_input_size <= 0:
                    head = s3.head_object(Bucket=BUCKET, Key=coeffs_key)
                    coeff_input_size = int(head.get("ContentLength") or 0)
                if coeff_input_size <= 0:
                    raise RuntimeError(f"Failed to determine coeff chunk size for s3://{BUCKET}/{coeffs_key}")
                params["sectioned_score_coeffs_input_size"] = coeff_input_size
                params["sectioned_score_coeffs_url"] = s3.generate_presigned_url(
                    "get_object",
                    Params={"Bucket": BUCKET, "Key": coeffs_key},
                    ExpiresIn=900,
                )
            if _solve_score_bins_uses_source(params.get("solve_score_bins_data") or {}, "pm"):
                params_key = str(params.get("params_key") or "").strip()
                step_start = int(params["step_start"])
                step_count = int(params["step_count"])
                params_obj = s3.get_object(
                    Bucket=BUCKET,
                    Key=params_key,
                    Range=f"bytes={step_start * 16}-{step_start * 16 + step_count * 16 - 1}",
                )
                with open(_TMP_SCORE_PARAMS, "wb") as pf:
                    pf.write(params_obj["Body"].read())
                params["solve_score_params_path"] = _TMP_SCORE_PARAMS
        else:
            t_dl = time.perf_counter()
            obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
            with open(bin_path, "wb") as f:
                f.write(obj["Body"].read())
            perf["download_us"] = int((time.perf_counter() - t_dl) * 1e6)
            if _solve_score_bins_uses_source(params.get("solve_score_bins_data") or {}, "cf"):
                coeffs_key = str(params.get("coeffs_key") or "").strip()
                coeff_obj = s3.get_object(Bucket=BUCKET, Key=coeffs_key)
                with open(_TMP_SCORE_COEFFS, "wb") as cf:
                    cf.write(coeff_obj["Body"].read())
                params["solve_score_coeffs_path"] = _TMP_SCORE_COEFFS
            if _solve_score_bins_uses_source(params.get("solve_score_bins_data") or {}, "pm"):
                params_key = str(params.get("params_key") or "").strip()
                step_start = int(params["step_start"])
                step_count = int(params["step_count"])
                params_obj = s3.get_object(
                    Bucket=BUCKET,
                    Key=params_key,
                    Range=f"bytes={step_start * 16}-{step_start * 16 + step_count * 16 - 1}",
                )
                with open(_TMP_SCORE_PARAMS, "wb") as pf:
                    pf.write(params_obj["Body"].read())
                params["solve_score_params_path"] = _TMP_SCORE_PARAMS

        report_status(job_id, task_id, "bin_downloaded")

        t_native = time.perf_counter()
        cmd = _build_cmd(params, bin_path, saved_bins_path if os.path.exists(saved_bins_path) else None)
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        native_wall_us = int((time.perf_counter() - t_native) * 1e6)
        if result.returncode != 0:
            raise RuntimeError(f"roots2pix_mt failed: {result.stderr.strip()}")
        raster_meta = json.loads(result.stdout)
        perf["threads"] = int(raster_meta.get("threads", threads))
        perf["input_mode"] = str(raster_meta.get("input_mode", raster_input_mode))
        perf["retries"] = int(raster_meta.get("retries", raster_sectioned_retries))
        perf["download_us"] = int(raster_meta.get("download_us", perf["download_us"]))
        perf["native_us"] = int(raster_meta.get("native_us", native_wall_us))
        perf["roots_plotted"] = int(raster_meta.get("roots_plotted", 0))
        perf["roots_clipped"] = int(raster_meta.get("roots_clipped", 0))

        report_status(job_id, task_id, "rasterized")

        t_up = time.perf_counter()
        uploaded = 0
        uploaded_pixel_bins = 0
        for t in range(n_tiles):
            pix_path = f"/tmp/pix_t{t:04d}.pix"
            if os.path.exists(pix_path) and os.path.getsize(pix_path) > 0:
                s3_key = f"renders/{job_id}/pix_chunk_{chunk_idx:04d}_t{t:04d}.pix"
                with open(pix_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, s3_key)
                os.remove(pix_path)
                uploaded += 1
            pbx_path = f"/tmp/pixbin_t{t:04d}.pbx"
            if emit_pixel_bins and os.path.exists(pbx_path):
                if os.path.getsize(pbx_path) > 0:
                    pbx_key = f"renders/{job_id}/pixbin_chunk_{chunk_idx:04d}_t{t:04d}.pbx"
                    with open(pbx_path, "rb") as fh:
                        s3.upload_fileobj(fh, BUCKET, pbx_key)
                    uploaded_pixel_bins += 1
                os.remove(pbx_path)
        perf["upload_us"] = int((time.perf_counter() - t_up) * 1e6)
        perf["tiles_uploaded"] = uploaded
        perf["pixel_bin_tiles_uploaded"] = uploaded_pixel_bins

        attach_contract_warnings(perf, contract_warnings)
        report_status(job_id, task_id, "done", result_data=perf)
        return ok_response({
            "chunk_idx": chunk_idx,
            "stripe_idx": chunk_idx,
            "tiles_uploaded": uploaded,
            "pixel_bin_tiles_uploaded": uploaded_pixel_bins,
            "raster_us": perf["native_us"],
            "roots_plotted": perf["roots_plotted"],
            "roots_clipped": perf["roots_clipped"],
            "engine": "mt",
            "threads": perf["threads"],
            "input_mode": perf["input_mode"],
        })

    except Exception as e:
        attach_contract_warnings(perf, contract_warnings)
        report_status(job_id, task_id, "error", str(e), result_data=perf)
        raise
    finally:
        _cleanup_tmp()
