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

from logical_sections import (
    build_native_manifest_urls,
    build_native_multispan_manifest,
    build_source_spans,
    stitch_spans_to_file,
    write_native_multispan_manifest,
)
from solve_score_chain import solve_score_program_cli_payload
from shared import BUCKET, attach_contract_warnings, contract_param, ok_response, parse_body, parse_boolish, report_status

s3 = boto3.client("s3")
ROOTS2PIX_MT = os.path.join(os.path.dirname(__file__), "roots2pix_mt")
PIXBINASSEMBLE = os.path.join(os.path.dirname(__file__), "pixbinassemble")
DEFAULT_THREADS = int(os.environ.get("RASTER_MT_THREADS", "4") or "4")
VALID_RASTER_INPUT_MODES = {"tmpfile", "sectioned"}
_TMP_SCORE_COEFFS = "/tmp/score_coeffs.bin"
_TMP_SCORE_PARAMS = "/tmp/score_params.bin"
_TMP_INPUT_MANIFEST = "/tmp/raster_input_manifest.json"
_TMP_SCORE_COEFFS_MANIFEST = "/tmp/raster_score_coeffs_manifest.json"
_TMP_SCORE_PARAMS_MANIFEST = "/tmp/raster_score_params_manifest.json"


def _tile_dense_bytes(tile_idx, width, height, tile_size, n_tile_cols):
    tile_w, tile_h = _tile_shape(tile_idx, width, height, tile_size, n_tile_cols)
    return tile_w * tile_h


def _tile_shape(tile_idx, width, height, tile_size, n_tile_cols):
    tile_idx = int(tile_idx)
    tile_size = int(tile_size)
    width = int(width)
    height = int(height)
    n_tile_cols = int(n_tile_cols)
    col = tile_idx % n_tile_cols
    row = tile_idx // n_tile_cols
    tile_w = max(0, min(tile_size, width - col * tile_size))
    tile_h = max(0, min(tile_size, height - row * tile_size))
    return tile_w, tile_h


def _validate_threads(value):
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"raster_mt_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"raster_mt_threads must be in [1, 16], got {threads}")
    return threads


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
    _cleanup_chunk_tmp()
    for stale in glob.glob("/tmp/root_xforms.json"):
        try:
            os.remove(stale)
        except OSError:
            pass


def _cleanup_chunk_tmp():
    for pattern in (
        "/tmp/pix_t*.pix",
        "/tmp/pixbin_t*.pbx",
        "/tmp/group_pixbin_t*.u8",
        "/tmp/stripe.bin",
        "/tmp/palette_bins_chunk.bin",
        _TMP_SCORE_COEFFS,
        _TMP_SCORE_PARAMS,
        _TMP_INPUT_MANIFEST,
        _TMP_SCORE_COEFFS_MANIFEST,
        _TMP_SCORE_PARAMS_MANIFEST,
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
        f"--input_mode={params.get('effective_input_mode', params.get('raster_input_mode', 'tmpfile'))}",
    ]
    effective_input_mode = params.get("effective_input_mode", params.get("raster_input_mode", "tmpfile"))
    if effective_input_mode == "sectioned":
        cmd.extend([
            f"--url={params['sectioned_url']}",
            f"--input_size={params['sectioned_input_size']}",
            f"--retries={params.get('raster_sectioned_retries', 2)}",
        ])
    elif effective_input_mode == "multispan_sectioned":
        cmd.extend([
            f"--input_manifest={params['input_manifest_path']}",
            f"--retries={params.get('raster_sectioned_retries', 2)}",
        ])
    if params.get("emit_pixel_bins"):
        cmd.append("--pixel_bin_prefix=/tmp/pixbin")
    if params.get("pixel_bins_drive_rgb"):
        cmd.append("--skip_pix_output=1")

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
                if effective_input_mode == "sectioned":
                    coeffs_url = str(params.get("sectioned_score_coeffs_url") or "").strip()
                    coeff_input_size = int(params.get("sectioned_score_coeffs_input_size") or 0)
                    if not coeffs_url or coeff_input_size <= 0:
                        raise RuntimeError("mixed-source sectioned raster requires coeff presign URL and size")
                    cmd.extend([
                        f"--score_coeffs_url={coeffs_url}",
                        f"--score_coeff_input_size={coeff_input_size}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                elif effective_input_mode == "multispan_sectioned":
                    coeff_manifest_path = str(params.get("score_coeff_manifest_path") or "").strip()
                    if not coeff_manifest_path:
                        raise RuntimeError("mixed-source multispan raster requires score_coeff_manifest_path")
                    cmd.extend([
                        f"--score_coeff_manifest={coeff_manifest_path}",
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
                if effective_input_mode == "multispan_sectioned":
                    params_manifest_path = str(params.get("score_params_manifest_path") or "").strip()
                    if not params_manifest_path:
                        raise RuntimeError("param-source multispan raster requires score_params_manifest_path")
                    cmd.append(f"--score_params_manifest={params_manifest_path}")
                else:
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
            req_omega_enabled = parse_boolish(params.get("solve_score_omega_enabled", True), True)
            bins_omega_enabled = parse_boolish(ss_data.get("omega_enabled", True), True)
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


def _normal_section_item(item):
    return {
        "section_idx": int(item["section_idx"]),
        "section_count": int(item["section_count"]),
        "bin_key": str(item["bin_key"]),
        "coeffs_key": str(item.get("coeffs_key") or ""),
        "coeffs_bin_size": int(item.get("coeffs_bin_size") or 0),
        "params_key": str(item.get("params_key") or ""),
        "params_bin_size": int(item.get("params_bin_size") or 0),
        "params_step_start": int(item.get("params_step_start", item.get("step_start") or 0) or 0),
        "params_step_count": int(item.get("params_step_count", item.get("step_count") or 0) or 0),
        "step_start": int(item.get("step_start") or 0),
        "step_count": int(item.get("step_count") or 0),
        "bin_size": int(item.get("bin_size") or 0),
    }


def _section_items_from_params(params, dense_grouped):
    if dense_grouped:
        sections = params.get("sections")
        if not isinstance(sections, list) or not sections:
            raise RuntimeError("dense_grouped raster requires non-empty sections list")
        return [_normal_section_item(item) for item in sections]
    return [_normal_section_item({
        "section_idx": params.get("section_idx", params.get("chunk_idx", params.get("stripe_idx"))),
        "section_count": params.get("section_count", 1),
        "bin_key": params.get("bin_key"),
        "coeffs_key": params.get("coeffs_key", ""),
        "coeffs_bin_size": params.get("coeffs_bin_size", 0),
        "params_key": params.get("params_key", ""),
        "params_bin_size": params.get("params_bin_size", 0),
        "params_step_start": params.get("params_step_start", params.get("step_start", 0)),
        "params_step_count": params.get("params_step_count", params.get("step_count", 0)),
        "step_start": params.get("step_start", 0),
        "step_count": params.get("step_count", 0),
        "bin_size": params.get("bin_size", 0),
    })]


def _apply_section_item(params, item):
    out = dict(params)
    for key, value in item.items():
        out[key] = value
    return out


def _saved_palette_bins_key_for_section(params, section_idx):
    prefix = str(params.get("saved_palette_bins_prefix") or "").strip()
    if prefix:
        return f"{prefix}{int(section_idx)}.bin"
    return str(params.get("saved_palette_bins_key") or "").strip()


def _prepare_section_inputs(section_params, *, bin_path, saved_bins_path, perf):
    bin_key = section_params["bin_key"]
    raster_input_mode = section_params["raster_input_mode"]
    logical_section = parse_boolish(section_params.get("logical_section"), False)
    solve_source_manifest = dict(section_params.get("solve_source_manifest") or {})
    step_start = int(section_params.get("step_start") or 0)
    step_count = int(section_params.get("step_count") or 0)
    color = section_params.get("color", "rainbow")

    try:
        os.remove(saved_bins_path)
    except OSError:
        pass
    if color == "saved_palette":
        saved_palette_bins_key = _saved_palette_bins_key_for_section(section_params, section_params["section_idx"])
        if not saved_palette_bins_key:
            raise RuntimeError("saved_palette color mode requires saved_palette_bins_key or saved_palette_bins_prefix")
        bins_obj = s3.get_object(Bucket=BUCKET, Key=saved_palette_bins_key)
        with open(saved_bins_path, "wb") as bf:
            bf.write(bins_obj["Body"].read())
        section_params["saved_palette_bins_key"] = saved_palette_bins_key

    ss_data = section_params.get("solve_score_bins_data") or {}
    if color in ("solve_score", "solve_proximity") and _solve_score_bins_uses_source(ss_data, "cf"):
        coeffs_key = str(section_params.get("coeffs_key") or "").strip()
        n_coeffs = section_params.get("n_coeffs")
        if not coeffs_key and not logical_section:
            raise RuntimeError("mixed-source solve-score render requires coeffs_key")
        try:
            n_coeffs = int(n_coeffs)
        except (TypeError, ValueError):
            raise RuntimeError(f"mixed-source solve-score render requires numeric n_coeffs, got {n_coeffs!r}")
        if n_coeffs < 1:
            raise RuntimeError(f"mixed-source solve-score render requires n_coeffs >= 1, got {n_coeffs}")
        section_params["n_coeffs"] = n_coeffs
    if color in ("solve_score", "solve_proximity") and _solve_score_bins_uses_source(ss_data, "pm"):
        params_key = str(section_params.get("params_key") or "").strip()
        params_step_start = section_params.get("params_step_start", section_params.get("step_start"))
        params_step_count = section_params.get("params_step_count", section_params.get("step_count"))
        step_count = section_params.get("step_count")
        if not params_key and not logical_section:
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
        section_params["params_step_start"] = params_step_start
        section_params["params_step_count"] = params_step_count
        section_params["step_count"] = step_count

    effective_input_mode = raster_input_mode
    if logical_section and not bin_key:
        root_spans = build_source_spans(
            solve_source_manifest,
            source_family="slv",
            solve_start=step_start,
            solve_count=step_count,
        )
        if root_spans:
            bin_key = str(root_spans[0]["key"] or "")
            section_params["bin_key"] = bin_key

    if raster_input_mode == "sectioned" and logical_section:
        if not solve_source_manifest:
            raise RuntimeError("logical raster section requires solve_source_manifest")
        root_spans = build_source_spans(
            solve_source_manifest,
            source_family="slv",
            solve_start=step_start,
            solve_count=step_count,
        )
        input_manifest = build_native_multispan_manifest(
            solve_source_manifest,
            source_family="slv",
            solve_start=step_start,
            solve_count=step_count,
            url_by_key=build_native_manifest_urls(s3, BUCKET, root_spans),
        )
        section_params["input_manifest_path"] = write_native_multispan_manifest(_TMP_INPUT_MANIFEST, input_manifest)
        effective_input_mode = "multispan_sectioned"
        section_params["sectioned_input_size"] = int(input_manifest["logical_size"])
        if _solve_score_bins_uses_source(ss_data, "cf"):
            coeff_spans = build_source_spans(
                solve_source_manifest,
                source_family="cf",
                solve_start=step_start,
                solve_count=step_count,
            )
            coeff_manifest = build_native_multispan_manifest(
                solve_source_manifest,
                source_family="cf",
                solve_start=step_start,
                solve_count=step_count,
                url_by_key=build_native_manifest_urls(s3, BUCKET, coeff_spans),
            )
            section_params["score_coeff_manifest_path"] = write_native_multispan_manifest(
                _TMP_SCORE_COEFFS_MANIFEST,
                coeff_manifest,
            )
        if _solve_score_bins_uses_source(ss_data, "pm"):
            param_spans = build_source_spans(
                solve_source_manifest,
                source_family="pm",
                solve_start=step_start,
                solve_count=step_count,
            )
            param_manifest = build_native_multispan_manifest(
                solve_source_manifest,
                source_family="pm",
                solve_start=step_start,
                solve_count=step_count,
                url_by_key=build_native_manifest_urls(s3, BUCKET, param_spans),
            )
            section_params["score_params_manifest_path"] = write_native_multispan_manifest(
                _TMP_SCORE_PARAMS_MANIFEST,
                param_manifest,
            )
    elif raster_input_mode == "sectioned":
        input_size = int(section_params.get("bin_size") or 0)
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
        section_params["sectioned_input_size"] = input_size
        section_params["sectioned_url"] = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": bin_key},
            ExpiresIn=900,
        )
        if _solve_score_bins_uses_source(ss_data, "cf"):
            coeff_input_size = int(section_params.get("coeffs_bin_size") or 0)
            coeffs_key = str(section_params.get("coeffs_key") or "").strip()
            if coeff_input_size <= 0:
                head = s3.head_object(Bucket=BUCKET, Key=coeffs_key)
                coeff_input_size = int(head.get("ContentLength") or 0)
            if coeff_input_size <= 0:
                raise RuntimeError(f"Failed to determine coeff chunk size for s3://{BUCKET}/{coeffs_key}")
            section_params["sectioned_score_coeffs_input_size"] = coeff_input_size
            section_params["sectioned_score_coeffs_url"] = s3.generate_presigned_url(
                "get_object",
                Params={"Bucket": BUCKET, "Key": coeffs_key},
                ExpiresIn=900,
            )
        if _solve_score_bins_uses_source(ss_data, "pm"):
            params_key = str(section_params.get("params_key") or "").strip()
            params_step_start = int(section_params["params_step_start"])
            params_step_count = int(section_params["params_step_count"])
            params_obj = s3.get_object(
                Bucket=BUCKET,
                Key=params_key,
                Range=f"bytes={params_step_start * 16}-{params_step_start * 16 + params_step_count * 16 - 1}",
            )
            with open(_TMP_SCORE_PARAMS, "wb") as pf:
                pf.write(params_obj["Body"].read())
            section_params["solve_score_params_path"] = _TMP_SCORE_PARAMS
    else:
        t_dl = time.perf_counter()
        if logical_section:
            if not solve_source_manifest:
                raise RuntimeError("logical raster tmpfile mode requires solve_source_manifest")
            root_spans = build_source_spans(
                solve_source_manifest,
                source_family="slv",
                solve_start=step_start,
                solve_count=step_count,
            )
            stitch_spans_to_file(s3, BUCKET, root_spans, bin_path)
        else:
            obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
            with open(bin_path, "wb") as f:
                f.write(obj["Body"].read())
        if _solve_score_bins_uses_source(ss_data, "cf"):
            if logical_section:
                coeff_spans = build_source_spans(
                    solve_source_manifest,
                    source_family="cf",
                    solve_start=step_start,
                    solve_count=step_count,
                )
                stitch_spans_to_file(s3, BUCKET, coeff_spans, _TMP_SCORE_COEFFS)
            else:
                coeffs_key = str(section_params.get("coeffs_key") or "").strip()
                coeff_obj = s3.get_object(Bucket=BUCKET, Key=coeffs_key)
                with open(_TMP_SCORE_COEFFS, "wb") as cf:
                    cf.write(coeff_obj["Body"].read())
            section_params["solve_score_coeffs_path"] = _TMP_SCORE_COEFFS
        if _solve_score_bins_uses_source(ss_data, "pm"):
            if logical_section:
                param_spans = build_source_spans(
                    solve_source_manifest,
                    source_family="pm",
                    solve_start=step_start,
                    solve_count=step_count,
                )
                stitch_spans_to_file(s3, BUCKET, param_spans, _TMP_SCORE_PARAMS)
            else:
                params_key = str(section_params.get("params_key") or "").strip()
                params_step_start = int(section_params["params_step_start"])
                params_step_count = int(section_params["params_step_count"])
                params_obj = s3.get_object(
                    Bucket=BUCKET,
                    Key=params_key,
                    Range=f"bytes={params_step_start * 16}-{params_step_start * 16 + params_step_count * 16 - 1}",
                )
                with open(_TMP_SCORE_PARAMS, "wb") as pf:
                    pf.write(params_obj["Body"].read())
            section_params["solve_score_params_path"] = _TMP_SCORE_PARAMS
        perf["download_us"] += int((time.perf_counter() - t_dl) * 1e6)
    section_params["effective_input_mode"] = effective_input_mode
    return section_params


def _feed_file_to_stdin(path, stdin):
    with open(path, "rb") as fh:
        while True:
            data = fh.read(1024 * 1024)
            if not data:
                break
            stdin.write(data)


def _ensure_group_bin_proc(group_bins, tile_idx, *, width, height, tile_size, n_tile_cols, pixel_bins_empty):
    if tile_idx in group_bins:
        return group_bins[tile_idx]
    tile_w, tile_h = _tile_shape(tile_idx, width, height, tile_size, n_tile_cols)
    out_path = f"/tmp/group_pixbin_t{tile_idx:04d}.u8"
    proc = subprocess.Popen(
        [PIXBINASSEMBLE,
         f"--tile_w={tile_w}",
         f"--tile_h={tile_h}",
         f"--empty={pixel_bins_empty}",
         f"--output={out_path}"],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE)
    group_bins[tile_idx] = {"proc": proc, "path": out_path, "tile_w": tile_w, "tile_h": tile_h, "sparse_bytes": 0, "files": 0}
    return group_bins[tile_idx]


def _close_group_bin_procs(group_bins, task_id):
    for info in group_bins.values():
        info["proc"].stdin.close()
    for tile_idx, info in sorted(group_bins.items()):
        rc = info["proc"].wait(timeout=120)
        stderr_out = info["proc"].stderr.read().decode("utf-8", errors="replace")
        if rc != 0:
            raise RuntimeError(f"pixbinassemble group tile {tile_idx} failed (rc={rc}), stderr: {stderr_out[:500]}")
        if stderr_out:
            print(f"[{task_id}] pixbinassemble group tile {tile_idx} stderr: {stderr_out[:500]}")


def handler(event, context):
    params = parse_body(event)
    contract_warnings = []
    job_id = params["job_id"]
    section_idx = params.get("section_idx", params.get("chunk_idx", params.get("stripe_idx")))
    if section_idx is None and not isinstance(params.get("sections"), list):
        raise RuntimeError("raster requires section_idx")
    n_tile_cols = params["n_tile_cols"]
    n_tile_rows = params["n_tile_rows"]
    n_tiles = n_tile_cols * n_tile_rows
    task_id = params.get("task_id", f"raster_{section_idx if section_idx is not None else 'group'}")
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
    emit_pixel_bins = parse_boolish(params.get("emit_pixel_bins"), False)
    pixel_bins_drive_rgb = emit_pixel_bins and parse_boolish(params.get("pixel_bins_drive_rgb"), False)
    perf["pixel_bins_drive_rgb"] = pixel_bins_drive_rgb
    perf["rgb_source"] = "pixel_bins" if pixel_bins_drive_rgb else "pix"

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
            if not (params.get("saved_palette_bins_key") or params.get("saved_palette_bins_prefix")):
                raise RuntimeError("saved_palette color mode requires saved_palette_bins_key or saved_palette_bins_prefix")
        elif ss_bins_key and color in ("solve_score", "solve_proximity"):
            ss_obj = s3.get_object(Bucket=BUCKET, Key=ss_bins_key)
            params["solve_score_bins_data"] = json.loads(ss_obj["Body"].read())
        pixel_bin_fragment_mode = str(params.get("pixel_bin_fragment_mode") or "sparse_chunks").strip().lower()
        dense_grouped = pixel_bin_fragment_mode == "dense_grouped" and pixel_bins_drive_rgb and emit_pixel_bins
        section_items = _section_items_from_params(params, dense_grouped)
        group_idx = int(params.get("group_idx", section_items[0]["section_idx"]))
        section_indices = [int(item["section_idx"]) for item in section_items]
        perf["pixel_bin_fragment_mode"] = "dense_grouped" if dense_grouped else "sparse_chunks"
        perf["group_idx"] = group_idx
        perf["section_indices"] = section_indices
        perf["section_count"] = len(section_items)

        group_bins = {}
        uploaded = 0
        uploaded_pixel_bins = 0
        uploaded_pixel_bin_bytes = 0
        pixel_bin_tile_bytes = []
        skipped_pix_tiles = 0
        grouped_sparse_bytes_in = 0
        grouped_sparse_files_in = 0
        upload_us_accum = 0

        for item_idx, item in enumerate(section_items):
            _cleanup_chunk_tmp()
            section_idx = int(item["section_idx"])
            section_params = _apply_section_item(params, item)
            section_params = _prepare_section_inputs(
                section_params,
                bin_path=bin_path,
                saved_bins_path=saved_bins_path,
                perf=perf,
            )
            report_status(job_id, task_id, f"bin_downloaded_{item_idx+1}/{len(section_items)}")

            t_native = time.perf_counter()
            cmd = _build_cmd(section_params, bin_path, saved_bins_path if os.path.exists(saved_bins_path) else None)
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
            native_wall_us = int((time.perf_counter() - t_native) * 1e6)
            if result.returncode != 0:
                raise RuntimeError(f"roots2pix_mt failed for section {section_idx}: {result.stderr.strip()}")
            raster_meta = json.loads(result.stdout)
            perf["threads"] = int(raster_meta.get("threads", threads))
            perf["requested_input_mode"] = raster_input_mode
            perf["input_mode"] = str(raster_meta.get("input_mode", section_params.get("effective_input_mode", raster_input_mode)))
            perf["retries"] = int(raster_meta.get("retries", raster_sectioned_retries))
            if perf["input_mode"] in ("sectioned", "multispan_sectioned"):
                perf["download_us"] += int(raster_meta.get("download_us", 0))
            perf["native_us"] += int(raster_meta.get("native_us", native_wall_us))
            perf["roots_plotted"] += int(raster_meta.get("roots_plotted", 0))
            perf["roots_clipped"] += int(raster_meta.get("roots_clipped", 0))

            t_chunk_up = time.perf_counter()
            chunk_skipped_pix_tiles = 0
            for t in range(n_tiles):
                pix_path = f"/tmp/pix_t{t:04d}.pix"
                if os.path.exists(pix_path) and os.path.getsize(pix_path) > 0:
                    if pixel_bins_drive_rgb:
                        chunk_skipped_pix_tiles += 1
                    else:
                        s3_key = f"renders/{job_id}/pix_chunk_{section_idx:04d}_t{t:04d}.pix"
                        with open(pix_path, "rb") as fh:
                            s3.upload_fileobj(fh, BUCKET, s3_key)
                        uploaded += 1
                    os.remove(pix_path)
                pbx_path = f"/tmp/pixbin_t{t:04d}.pbx"
                if emit_pixel_bins and os.path.exists(pbx_path):
                    pbx_size = os.path.getsize(pbx_path)
                    if pbx_size > 0:
                        dense_bytes = _tile_dense_bytes(t, section_params["width"], section_params["height"], section_params["tile_size"], n_tile_cols)
                        if dense_grouped:
                            info = _ensure_group_bin_proc(
                                group_bins,
                                t,
                                width=section_params["width"],
                                height=section_params["height"],
                                tile_size=section_params["tile_size"],
                                n_tile_cols=n_tile_cols,
                                pixel_bins_empty=int(section_params.get("pixel_bins_empty", 255) or 255),
                            )
                            _feed_file_to_stdin(pbx_path, info["proc"].stdin)
                            info["sparse_bytes"] += pbx_size
                            info["files"] += 1
                            grouped_sparse_bytes_in += pbx_size
                            grouped_sparse_files_in += 1
                        else:
                            pbx_key = f"renders/{job_id}/pixbin_chunk_{section_idx:04d}_t{t:04d}.pbx"
                            with open(pbx_path, "rb") as fh:
                                s3.upload_fileobj(fh, BUCKET, pbx_key)
                            uploaded_pixel_bins += 1
                            uploaded_pixel_bin_bytes += pbx_size
                            pixel_bin_tile_bytes.append({
                                "tile_idx": t,
                                "bytes": pbx_size,
                                "dense_bytes": dense_bytes,
                            })
                    os.remove(pbx_path)
            if pixel_bins_drive_rgb:
                skipped_pix_tiles += max(chunk_skipped_pix_tiles, int(raster_meta.get("tiles_with_data", 0) or 0))
            upload_us_accum += int((time.perf_counter() - t_chunk_up) * 1e6)
            report_status(job_id, task_id, f"rasterized_{item_idx+1}/{len(section_items)}")

        if dense_grouped:
            t_group_up = time.perf_counter()
            _close_group_bin_procs(group_bins, task_id)
            for t, info in sorted(group_bins.items()):
                out_path = info["path"]
                dense_size = os.path.getsize(out_path)
                pbx_key = f"renders/{job_id}/pixbin_group_{group_idx:04d}_t{t:04d}.u8"
                with open(out_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, pbx_key)
                uploaded_pixel_bins += 1
                uploaded_pixel_bin_bytes += dense_size
                pixel_bin_tile_bytes.append({
                    "tile_idx": t,
                    "bytes": dense_size,
                    "dense_bytes": dense_size,
                    "sparse_bytes_in": int(info["sparse_bytes"]),
                    "sparse_files_in": int(info["files"]),
                })
                os.remove(out_path)
            upload_us_accum += int((time.perf_counter() - t_group_up) * 1e6)

        report_status(job_id, task_id, "rasterized")
        perf["upload_us"] = upload_us_accum
        perf["tiles_uploaded"] = uploaded
        perf["pixel_bin_tiles_uploaded"] = uploaded_pixel_bins
        perf["pixel_bin_bytes_uploaded"] = uploaded_pixel_bin_bytes
        perf["pixel_bin_tile_bytes"] = pixel_bin_tile_bytes
        perf["pixel_bin_dense_bytes_if_full_tiles"] = sum(item["dense_bytes"] for item in pixel_bin_tile_bytes)
        perf["pix_tiles_skipped"] = skipped_pix_tiles
        if dense_grouped:
            perf["pixel_bin_sparse_bytes_in"] = grouped_sparse_bytes_in
            perf["pixel_bin_sparse_files_in"] = grouped_sparse_files_in

        attach_contract_warnings(perf, contract_warnings)
        report_status(job_id, task_id, "done", result_data=perf)
        return ok_response({
            "section_idx": group_idx if dense_grouped else int(section_items[0]["section_idx"]),
            "group_idx": group_idx,
            "section_indices": section_indices,
            "tiles_uploaded": uploaded,
            "pixel_bin_tiles_uploaded": uploaded_pixel_bins,
            "pixel_bin_bytes_uploaded": uploaded_pixel_bin_bytes,
            "pixel_bin_tile_bytes": pixel_bin_tile_bytes,
            "pixel_bin_dense_bytes_if_full_tiles": perf["pixel_bin_dense_bytes_if_full_tiles"],
            "pixel_bin_fragment_mode": perf["pixel_bin_fragment_mode"],
            "pixel_bin_sparse_bytes_in": perf.get("pixel_bin_sparse_bytes_in", 0),
            "pixel_bin_sparse_files_in": perf.get("pixel_bin_sparse_files_in", 0),
            "pixel_bins_drive_rgb": pixel_bins_drive_rgb,
            "rgb_source": perf["rgb_source"],
            "pix_tiles_skipped": skipped_pix_tiles,
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
