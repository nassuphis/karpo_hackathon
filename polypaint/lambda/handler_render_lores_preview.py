"""
Ephemeral lores render preview.

This colorizes one of three preview sources with the current render parameters:
saved lores artifacts, a logical hires subset, or recomputed temporary artifacts.
It does not persist images, save palettes, or write metadata.
"""
import base64
import glob
import hashlib
import json
import math
import os
import re
import subprocess
import time

import boto3

try:
    # 32-connection pool: the parallel lattice materializer fans row fetches
    # across threads, and botocore's default pool of 10 would serialize them
    from botocore.config import Config as _BotoConfig
    _S3_CLIENT_CONFIG = _BotoConfig(max_pool_connections=32)
except ImportError:
    # the Docker runtime container stubs boto3 WITHOUT botocore (real Lambda
    # ships both); the stub's client ignores pooling anyway
    _S3_CLIENT_CONFIG = None

from color_artifact_meta import load_color_artifact_head, parse_root_transforms
from png_rgb import decode_png_rgb
from color_render_contract import normalize_background_color, validate_color_output_contract
from logical_lores import (
    _logical_row_mapping,
    calc_square_grid,
    estimate_logical_lores_bytes,
    logical_lores_default_n,
    materialize_logical_lores,
)
from logical_sections import (
    build_native_manifest_urls,
    build_native_multispan_manifest,
    build_solve_source_manifest,
    build_source_spans,
    coeff_row_bytes,
    param_row_bytes,
    root_row_bytes,
    write_native_multispan_manifest,
)
from raw_score_render import histogram_from_raw_path_channel0, render_score_raw, write_equalization_lut
from root_pipeline_programs import root_program_for_run
from shared import (BUCKET, REF_SIZE, compute_viewport_from_bin, is_missing_s3_error,
                    ok_response, parse_body, parse_boolish, random_b36, report_status,
                    sanitize_sculpture_view)
from solve_score_pipeline_programs import solve_score_program_for_run
from solve_score_chain import (
    compiled_solve_score_fingerprint,
    serialize_solve_score_chain,
    solve_score_program_cli_payload,
    solve_score_uses_source,
)
# Recompute boundary: old calc.json artifacts carry legacy param/coeff transform
# chains that the native runtime no longer accepts (sweep_cli rejectLegacyTransformChain).
# Translate them into compiled programs before dispatch (mirrors handler_coeffgen).
from coeff_program_chain import compile_coeff_program_chain
from param_program_chain import compile_param_program_chain
from pipeline_programs import (
    coeff_transforms_to_program_chain,
    param_transforms_to_program_chain,
)
from program_compile_helpers import (
    compiled_coeff_program_payload,
    compiled_param_program_payload,
)


s3 = (boto3.client("s3", config=_S3_CLIENT_CONFIG)
      if _S3_CLIENT_CONFIG is not None else boto3.client("s3"))

ROOTS2PIX_MT = os.path.join(os.path.dirname(__file__), "roots2pix_mt")
SPLAT_BAKE_BIN = os.path.join(os.path.dirname(__file__), "splat_bake")
SOLVE_PROXIMITY_STATS = os.path.join(os.path.dirname(__file__), "solve_proximity_stats")
SWEEP_COEFFGEN = os.path.join(os.path.dirname(__file__), "sweep_coeffgen")
SWEEP_MT = os.path.join(os.path.dirname(__file__), "sweep_mt")
SWEEP_CM = os.path.join(os.path.dirname(__file__), "sweep_cm")
TMP_ROOTS = "/tmp/render_lores_preview_roots.bin"
TMP_COEFFS = "/tmp/render_lores_preview_coeffs.bin"
TMP_PARAMS = "/tmp/render_lores_preview_params.bin"
TMP_INPUT_MANIFEST = "/tmp/render_lores_preview_input_manifest.json"
TMP_COEFFS_MANIFEST = "/tmp/render_lores_preview_coeffs_manifest.json"
TMP_PARAMS_MANIFEST = "/tmp/render_lores_preview_params_manifest.json"
TMP_XFORMS = "/tmp/render_lores_preview_xforms.json"
TMP_FRAGMENT_PREFIX = "/tmp/render_lores_preview_fragment"
TMP_FRAGMENT = TMP_FRAGMENT_PREFIX + ".frag"
TMP_PALETTE_FRAGMENT_PREFIX = "/tmp/render_lores_preview_palette_fragment"
TMP_PALETTE_FRAGMENT = TMP_PALETTE_FRAGMENT_PREFIX + ".frag"
TMP_RAW = "/tmp/render_lores_preview.raw"
TMP_PALETTE_RAW = "/tmp/render_lores_preview_palette.raw"
TMP_EQ_LUT = "/tmp/render_lores_preview_eq.bin"
TMP_PALETTE_EQ_LUT = "/tmp/render_lores_preview_palette_eq.bin"
TMP_IMAGE = "/tmp/render_lores_preview.png"
TMP_PALETTE_IMAGE = "/tmp/render_lores_preview_palette.png"
TMP_XFORMED_ROOTS = "/tmp/render_lores_preview_xformed_roots.bin"
TMP_STEP_SCORES = "/tmp/render_lores_preview_step_scores.raw"
TMP_SPLAT_ROOTS = "/tmp/render_lores_preview_splat_roots.bin"
TMP_SPLAT_COLORS = "/tmp/render_lores_preview_splat_colors.raw"
TMP_SPLAT_PACK = "/tmp/render_lores_preview_splat_pack.bin"

MAX_PREVIEW_PIX = int(os.environ.get("RENDER_LORES_PREVIEW_MAX_PIX", "1024"))
MAX_LOGICAL_LORES_N = int(os.environ.get("RENDER_LOGICAL_LORES_MAX_N", "256"))
DEFAULT_PREVIEW_PIX = 256


def _json_response(status_code, body):
    return {
        "statusCode": status_code,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(body),
    }


def _cleanup_tmp():
    for pattern in (
        TMP_ROOTS,
        TMP_COEFFS,
        TMP_PARAMS,
        TMP_INPUT_MANIFEST,
        TMP_COEFFS_MANIFEST,
        TMP_PARAMS_MANIFEST,
        TMP_XFORMS,
        TMP_FRAGMENT,
        TMP_PALETTE_FRAGMENT,
        TMP_RAW,
        TMP_PALETTE_RAW,
        TMP_EQ_LUT,
        TMP_PALETTE_EQ_LUT,
        TMP_IMAGE,
        TMP_PALETTE_IMAGE,
        TMP_XFORMED_ROOTS,
        TMP_STEP_SCORES,
        TMP_SPLAT_ROOTS,
        TMP_SPLAT_COLORS,
        TMP_SPLAT_PACK,
        TMP_FRAGMENT_PREFIX + "*",
        TMP_PALETTE_FRAGMENT_PREFIX + "*",
    ):
        for path in glob.glob(pattern):
            try:
                os.remove(path)
            except OSError:
                pass


def _coerce_int(value, field, *, default=None, min_value=None, max_value=None):
    if value in (None, "") and default is not None:
        value = default
    try:
        parsed = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field} must be an integer, got {value!r}")
    if min_value is not None and parsed < min_value:
        raise RuntimeError(f"{field} must be >= {min_value}, got {parsed}")
    if max_value is not None and parsed > max_value:
        raise RuntimeError(f"{field} must be <= {max_value}, got {parsed}")
    return parsed


def _coerce_n_coeffs(value, degree):
    default = int(degree) + 1
    if value in (0, "0"):
        value = default
    return _coerce_int(value, "n_coeffs", default=default, min_value=1)


def _coerce_finite_float(value, field, *, default=None):
    if value in (None, "") and default is not None:
        value = default
    try:
        parsed = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field} must be numeric, got {value!r}")
    if not math.isfinite(parsed):
        raise RuntimeError(f"{field} must be finite, got {value!r}")
    return parsed


def _download_to_file(key, path):
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        raise RuntimeError(f"Failed to download s3://{BUCKET}/{key}: {exc}") from exc
    with open(path, "wb") as fh:
        for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
            fh.write(chunk)
    return os.path.getsize(path)


def _load_calc(job_id):
    key = f"renders/{job_id}/calc.json"
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        raise RuntimeError(f"Failed to download s3://{BUCKET}/{key}: {exc}") from exc
    body = obj["Body"]
    if hasattr(body, "read"):
        raw = body.read()
    elif hasattr(body, "iter_chunks"):
        raw = b"".join(body.iter_chunks(chunk_size=1024 * 1024))
    else:
        raise RuntimeError(f"Failed to parse s3://{BUCKET}/{key}: body is not readable")
    try:
        return json.loads(raw.decode("utf-8"))
    except Exception as exc:
        raise RuntimeError(f"Failed to parse s3://{BUCKET}/{key}: {exc}") from exc


def _run_json_binary(binary, out_path, spec, *, phase, timeout_s=300):
    t0 = time.time()
    result = subprocess.run(
        [binary, out_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=timeout_s,
    )
    elapsed_ms = int((time.time() - t0) * 1000)
    if result.returncode != 0:
        raise RuntimeError(f"{phase} failed: {result.stderr.strip() or 'unknown error'}")
    try:
        meta = json.loads(result.stdout or "{}")
    except Exception as exc:
        raise RuntimeError(f"{phase} returned invalid JSON: {(result.stdout or '')[:200]!r}") from exc
    meta["elapsed_ms"] = elapsed_ms
    return meta


def _preview_source_mode(params):
    raw = str(params.get("preview_source_mode") or "").strip().lower()
    if not raw:
        if parse_boolish(params.get("logical_lores", False), False, strict=True, label="logical_lores"):
            raw = "logical"
        else:
            raw = "lores"
    if raw in ("physical", "saved_lores", "use_lores"):
        raw = "lores"
    if raw not in ("lores", "logical", "recompute"):
        raise RuntimeError(f"preview_source_mode must be lores, logical, or recompute, got {raw!r}")
    return raw


def _calc_pipeline(calc):
    pipeline = (calc or {}).get("pipeline") or {}
    function_name = str(pipeline.get("function") or (calc or {}).get("function") or "").strip()
    if not function_name:
        raise RuntimeError("calc.json missing pipeline.function for recompute preview")
    param_transforms = pipeline.get("param_transforms")
    if not isinstance(param_transforms, list):
        param_transforms = pipeline.get("param_transforms_display")
    if not isinstance(param_transforms, list):
        param_transforms = []
    param_program = pipeline.get("param_program")
    if not isinstance(param_program, dict):
        param_program = {}
    coeff_transforms = pipeline.get("coeff_transforms")
    if not isinstance(coeff_transforms, list):
        coeff_transforms = []
    coeff_program = pipeline.get("coeff_program")
    if not isinstance(coeff_program, dict):
        coeff_program = {}
    cfpv = pipeline.get("cfpv")
    if not isinstance(cfpv, list):
        cfpv = []
    # The native runtime rejects non-empty legacy transform arrays. Old calc.json
    # artifacts predate Param/Coeff Programs, so compile any legacy transforms to a
    # program here and clear the transform arrays (mirrors handler_coeffgen). Legacy
    # transforms never reference macros, so no macro_resolver is needed.
    if param_program:
        param_transforms = []
    elif param_transforms:
        compiled = compile_param_program_chain(
            param_transforms_to_program_chain(param_transforms)
        )
        param_program = compiled_param_program_payload(compiled)
        param_transforms = []
    if coeff_program:
        coeff_transforms = []
    elif coeff_transforms:
        compiled = compile_coeff_program_chain(
            coeff_transforms_to_program_chain(coeff_transforms)
        )
        coeff_program = compiled_coeff_program_payload(compiled)
        coeff_transforms = []
    return {
        "function": function_name,
        "param_transforms": param_transforms,
        "param_program": param_program,
        "coeff_transforms": coeff_transforms,
        "coeff_program": coeff_program,
        "cfpv": cfpv,
    }


def _calc_solver_mode(calc):
    raw = str((calc or {}).get("solver") or "aberth_mt").strip().lower()
    if raw in ("jt64", "cm64", "ae64", "fused_jt64", "fused_cm64", "fused_ae64"):
        # fused artifacts recompute their preview FUSED (f64 in-process);
        # kin-mapping them to the split solvers repaints the f32
        # transport artifact the fused modes exist to avoid
        return raw.replace("fused_", "")
    if raw in ("companion_matrix", "cm", "solve_cm"):
        return "companion_matrix"
    if raw in ("jenkins_traub", "jt", "solve_jt"):
        return "jenkins_traub"
    if raw in ("newton", "solve_newton"):
        return "newton"
    return "aberth_mt"


def _calc_solver_iters(calc):
    """Aberth iteration cap persisted with the run (0 = solver default)."""
    try:
        iters = int((calc or {}).get("solver_iters") or 0)
    except (TypeError, ValueError):
        return 0
    return iters if 1 <= iters <= 64 else 0


def _read_file_bytes(path):
    with open(path, "rb") as fh:
        return fh.read()


def _fallback_lores_coeffs_key(job_id, lores_bin_key):
    key = str(lores_bin_key or "").strip()
    if key.endswith("/lores.bin"):
        return key[:-len("/lores.bin")] + "/lores_coeffs.bin"
    job = str(job_id or "").strip()
    return f"renders/{job}/lores_coeffs.bin" if job else ""


def _fallback_lores_params_key(job_id, lores_bin_key):
    key = str(lores_bin_key or "").strip()
    if key.endswith("/lores.bin"):
        return key[:-len("/lores.bin")] + "/lores_params.bin"
    job = str(job_id or "").strip()
    return f"renders/{job}/lores_params.bin" if job else ""


def _explicit_viewport(params):
    min_re = _coerce_finite_float(params.get("min_re"), "min_re")
    max_re = _coerce_finite_float(params.get("max_re"), "max_re")
    min_im = _coerce_finite_float(params.get("min_im"), "min_im")
    max_im = _coerce_finite_float(params.get("max_im"), "max_im")
    if not max_re > min_re:
        raise RuntimeError(f"explicit viewport requires max_re > min_re, got {min_re!r}/{max_re!r}")
    if not max_im > min_im:
        raise RuntimeError(f"explicit viewport requires max_im > min_im, got {min_im!r}/{max_im!r}")
    return {"min_re": min_re, "max_re": max_re, "min_im": min_im, "max_im": max_im}


def _compute_preview_viewport(params, roots_path):
    view_mode = str(params.get("view_mode") or "auto").strip().lower()
    if view_mode == "explicit":
        return _explicit_viewport(params)
    if view_mode == "square":
        extent = _coerce_finite_float(params.get("square_extent", 2.0), "square_extent")
        if extent <= 0.0:
            raise RuntimeError(f"square_extent must be > 0, got {extent}")
        return {"min_re": -extent, "max_re": extent, "min_im": -extent, "max_im": extent}
    if view_mode != "auto":
        raise RuntimeError(f"unsupported view_mode: {view_mode!r}")

    quantile = _coerce_finite_float(params.get("quantile", 0.0), "quantile")
    shim = _coerce_finite_float(params.get("shim", 0.05), "shim")
    if quantile < 0.0 or quantile >= 0.5:
        raise RuntimeError(f"quantile must be in [0, 0.5), got {quantile}")
    if shim < 0.0:
        raise RuntimeError(f"shim must be >= 0, got {shim}")

    vp = compute_viewport_from_bin(_read_file_bytes(roots_path), quantile=quantile, shim=shim)
    q_re = vp.get("q_re") or [0.0, 0.0]
    q_im = vp.get("q_im") or [0.0, 0.0]
    q_min_re = _coerce_finite_float(q_re[0], "q_re[0]")
    q_max_re = _coerce_finite_float(q_re[1], "q_re[1]")
    q_min_im = _coerce_finite_float(q_im[0], "q_im[0]")
    q_max_im = _coerce_finite_float(q_im[1], "q_im[1]")
    center_re = (q_min_re + q_max_re) / 2.0
    center_im = (q_min_im + q_max_im) / 2.0
    range_re = (q_max_re - q_min_re) * (1.0 + shim)
    range_im = (q_max_im - q_min_im) * (1.0 + shim)
    fallback_span = None
    if range_re <= 0.0 or range_im <= 0.0:
        positive_span = max(range_re, range_im)
        if positive_span > 0.0:
            fallback_span = positive_span
        else:
            scale_ref = _coerce_finite_float(vp.get("scale"), "scale")
            if scale_ref <= 0.0:
                raise RuntimeError(f"auto viewport returned non-positive scale: {scale_ref!r}")
            fallback_span = float(REF_SIZE) / scale_ref
    if range_re <= 0.0:
        range_re = fallback_span
    if range_im <= 0.0:
        range_im = fallback_span
    return {
        "min_re": center_re - (range_re / 2.0),
        "max_re": center_re + (range_re / 2.0),
        "min_im": center_im - (range_im / 2.0),
        "max_im": center_im + (range_im / 2.0),
    }


def _compile_request_chain(params):
    return solve_score_program_for_run(params)


def _root_transforms_for_run(params):
    return root_program_for_run(params)["chain"]


def _clip_widen_half_width(center, lo, hi, min_score=None, max_score=None):
    full = 0.0
    try:
        full = float(max_score) - float(min_score)
    except (TypeError, ValueError):
        full = 0.0
    if math.isfinite(full) and full > 1e-12:
        return max(full * 0.0001 * 0.5, 1e-9)
    scale = max(1.0, abs(float(center or 0.0)), abs(float(lo or 0.0)), abs(float(hi or 0.0)))
    return max(scale * 0.0001 * 0.5, 1e-9)


def _sanitize_metric_clip(metric_row):
    row = dict(metric_row or {})
    clip_lo = _coerce_finite_float(row.get("clip_lo"), "clip_lo")
    clip_hi = _coerce_finite_float(row.get("clip_hi"), "clip_hi")
    width = clip_hi - clip_lo
    if width < -1e-12:
        raise RuntimeError(f"invalid metric clip range for {row.get('metric', 'unknown')}: lo={clip_lo} hi={clip_hi}")
    if width < 1e-12:
        center = 0.5 * (clip_lo + clip_hi)
        half_width = _clip_widen_half_width(center, clip_lo, clip_hi, row.get("min_score"), row.get("max_score"))
        clip_lo = center - half_width
        clip_hi = center + half_width
        row["clip_fallback"] = "degenerate_widened"
        row["clip_center"] = center
    row["clip_lo"] = clip_lo
    row["clip_hi"] = clip_hi
    return row


def _clip_metric_slot(metric_row, *, degree, n_coeffs, compiled, threads, root_transforms):
    source = str(metric_row.get("source", "slv") or "slv")
    if source == "slv":
        input_path = TMP_ROOTS
        metric_degree = degree
        xforms = root_transforms
    elif source == "cf":
        input_path = TMP_COEFFS
        metric_degree = n_coeffs
        xforms = None
    elif source == "pm":
        input_path = TMP_PARAMS
        metric_degree = 2
        xforms = None
    else:
        raise RuntimeError(f"unsupported score metric source: {source}")

    cmd = [
        SOLVE_PROXIMITY_STATS,
        input_path,
        "--mode=clip",
        f"--degree={int(metric_degree)}",
        f"--metric={metric_row['metric']}",
        f"--quantile_lo={float(metric_row['quantile'])}",
        f"--quantile_hi={1.0 - float(metric_row['quantile'])}",
        f"--omega={compiled['omega']}",
        f"--omega_enabled={1 if compiled['omega_enabled'] else 0}",
        f"--threads={int(threads)}",
    ]
    if xforms:
        cmd.append(f"--root_xforms={_write_xforms(xforms)}")
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        raise RuntimeError(f"solve_proximity_stats clip failed for {metric_row['metric']}({source}): {result.stderr.strip()}")
    data = json.loads(result.stdout)
    row = _sanitize_metric_clip({
        "slot": int(metric_row.get("slot", 0)),
        "metric": str(metric_row["metric"]),
        "source": source,
        "quantile": float(metric_row["quantile"]),
        "quantile_pct": float(metric_row["quantile"]) * 100.0,
        "clip_lo": data["clip_lo"],
        "clip_hi": data["clip_hi"],
        "min_score": data.get("min_score"),
        "max_score": data.get("max_score"),
        "n_solves": int(data.get("n_solves") or 0),
        "threads": int(data.get("threads", threads)),
    })
    return row


def _score_output_clip_from_summary(data):
    has_quantiles = data.get("q05") is not None and data.get("q95") is not None
    lo = data.get("q05") if has_quantiles else data.get("min_score")
    hi = data.get("q95") if has_quantiles else data.get("max_score")
    clip_lo = _coerce_finite_float(lo, "score_output_clip_lo")
    clip_hi = _coerce_finite_float(hi, "score_output_clip_hi")
    if clip_hi - clip_lo <= 1e-12:
        return {
            "score_output_clip_lo": 0.0,
            "score_output_clip_hi": 1.0,
            "score_output_clip_source": "degenerate_identity",
        }
    return {
        "score_output_clip_lo": clip_lo,
        "score_output_clip_hi": clip_hi,
        "score_output_clip_source": "lores_q05_q95" if has_quantiles else "lores_minmax",
    }


def _score_output_channels_from_summary(compiled, summary, solve_score_normalize):
    compiled_channels = list(compiled.get("output_channels") or []) if compiled.get("has_explicit_outputs") else []
    summary_channels = {
        int(row.get("channel", idx)): row
        for idx, row in enumerate(summary.get("score_output_channels") or [])
        if isinstance(row, dict)
    }
    if not compiled_channels:
        compiled_channels = [{
            "name": "score",
            "emit": "emit_norm" if solve_score_normalize else "emit",
            "channel": 0,
            "range_normalized": bool(solve_score_normalize),
        }]
    out = []
    for idx, row in enumerate(compiled_channels):
        channel = int(row.get("channel", idx))
        src = summary_channels.get(channel) or (summary if not compiled.get("has_explicit_outputs") else {})
        range_normalized = bool(row.get("range_normalized") or src.get("range_normalized") or row.get("emit") == "emit_norm")
        item = {
            "channel": channel,
            "name": str(src.get("display_name") or src.get("name") or row.get("name") or f"channel_{channel}"),
            "emit": "emit_norm" if range_normalized else "emit",
            "range_normalized": range_normalized,
        }
        if range_normalized:
            clip = _score_output_clip_from_summary({
                "q05": src.get("q05"),
                "q95": src.get("q95"),
                "min_score": src.get("min_score", 0.0),
                "max_score": src.get("max_score", 1.0),
            })
            item["clip_lo"] = clip["score_output_clip_lo"]
            item["clip_hi"] = clip["score_output_clip_hi"]
            item["clip_source"] = clip["score_output_clip_source"]
        else:
            item["clip_lo"] = 0.0
            item["clip_hi"] = 1.0
            item["clip_source"] = "identity"
        out.append(item)
    return out


def _build_program_cmd_args(compiled, metrics_with_clips):
    hydrated = []
    for slot, metric in enumerate(metrics_with_clips):
        row = dict(metric)
        row["slot"] = slot
        hydrated.append(row)
    payload = solve_score_program_cli_payload({"metrics": hydrated, "program_spec": compiled["program_spec"]})
    args = [
        f"--score_metrics={payload['score_metrics']}",
        f"--score_clip_los={payload['score_clip_los']}",
        f"--score_clip_his={payload['score_clip_his']}",
        f"--score_program={payload['score_program']}",
    ]
    if payload.get("score_sources"):
        args.append(f"--score_sources={payload['score_sources']}")
    return args


def _preview_score_summary(params, *, degree, n_coeffs, compiled, include_coeff, include_param):
    threads = _coerce_int(params.get("solve_score_threads", 1), "solve_score_threads", default=1, min_value=1)
    root_transforms = _root_transforms_for_run(params)
    solve_score_normalize = parse_boolish(
        params.get("solve_score_normalize", False),
        False,
        strict=True,
        label="solve_score_normalize",
    )
    metric_clips = []
    for slot, metric_row in enumerate(compiled["metrics"]):
        slot_clip = _clip_metric_slot(
            metric_row,
            degree=degree,
            n_coeffs=n_coeffs,
            compiled=compiled,
            threads=threads,
            root_transforms=root_transforms,
        )
        slot_clip["slot"] = slot
        metric_clips.append(slot_clip)
    cmd = [
        SOLVE_PROXIMITY_STATS,
        TMP_ROOTS,
        "--mode=summary",
        f"--degree={int(degree)}",
        f"--threads={threads}",
        *_build_program_cmd_args(compiled, metric_clips),
    ]
    if include_coeff:
        cmd.extend([
            f"--score_coeffs_file={TMP_COEFFS}",
            f"--score_coeff_degree={int(n_coeffs)}",
        ])
    if include_param:
        cmd.append(f"--score_params_file={TMP_PARAMS}")
    if root_transforms:
        cmd.append(f"--root_xforms={_write_xforms(root_transforms)}")
    if solve_score_normalize:
        cmd.append("--score_output_normalize=1")

    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        raise RuntimeError(f"solve_proximity_stats summary failed: {result.stderr.strip() or 'unknown error'}")
    summary = json.loads(result.stdout)
    summary["metric"] = compiled["metric"]
    summary["metric_count"] = compiled["metric_count"]
    summary["program"] = compiled["program_spec"]
    summary["program_id"] = compiled["program_id"]
    summary["score_metrics"] = [row["metric"] for row in compiled["metrics"]]
    summary["score_sources"] = [row.get("source", "slv") for row in compiled["metrics"]]
    summary["score_quantiles"] = [row["quantile"] for row in compiled["metrics"]]
    summary["solve_score_display"] = compiled["display"]
    summary["solve_score_chain"] = json.loads(serialize_solve_score_chain(compiled["chain"]))
    summary["chain_fingerprint"] = compiled_solve_score_fingerprint(compiled)
    summary["clip_quantile"] = compiled["quantile"]
    summary["omega"] = compiled["omega"]
    summary["omega_enabled"] = compiled["omega_enabled"]
    output_channels = _score_output_channels_from_summary(compiled, summary, solve_score_normalize)
    primary = output_channels[0] if output_channels else {"clip_lo": 0.0, "clip_hi": 1.0, "clip_source": "identity"}
    summary.update({
        "score_output_normalize": solve_score_normalize,
        "score_output_clip_lo": primary.get("clip_lo", 0.0),
        "score_output_clip_hi": primary.get("clip_hi", 1.0),
        "score_output_clip_source": primary.get("clip_source", "identity"),
        "score_output_channel_count": len(output_channels),
        "score_output_has_explicit_outputs": bool(compiled.get("has_explicit_outputs")),
        "score_output_interpretation": "scalar_lut",
        "score_output_channels": output_channels,
    })
    summary["metrics"] = metric_clips
    return summary


def _metric_rows_from_summary(summary):
    metrics = summary.get("metrics")
    if isinstance(metrics, list) and metrics:
        return metrics
    score_metrics = summary.get("score_metrics") or []
    score_quantiles = summary.get("score_quantiles") or []
    score_sources = summary.get("score_sources") or []
    if len(score_metrics) != 1:
        raise RuntimeError("solve summary missing per-slot clip metadata")
    return [{
        "slot": 0,
        "metric": str(score_metrics[0]),
        "quantile": float(score_quantiles[0] if score_quantiles else summary.get("clip_quantile", 0.001)),
        "source": str(score_sources[0] if score_sources else "slv"),
        "clip_lo": float(summary.get("clip_lo")),
        "clip_hi": float(summary.get("clip_hi")),
    }]


def _uses_source(metrics, source):
    return any(str(row.get("source", "slv") or "slv").strip().lower() == source for row in metrics or [])


def _file_url(path):
    return "file://" + os.path.abspath(path)


def _write_local_manifest(path, *, family, source_path, row_bytes, step_count):
    logical_size = int(row_bytes) * int(step_count)
    actual_size = os.path.getsize(source_path)
    if actual_size < logical_size:
        raise RuntimeError(
            f"local {family} manifest source too small: {actual_size} bytes, need {logical_size}"
        )
    native = {
        "source_family": family,
        "logical_size": logical_size,
        "row_bytes": int(row_bytes),
        "solve_start": 0,
        "solve_count": int(step_count),
        "sources": [{
            "id": 0,
            "url": _file_url(source_path),
            "key": source_path,
        }],
        "spans": [{
            "source_id": 0,
            "logical_byte_start": 0,
            "byte_start": 0,
            "byte_length": logical_size,
        }],
    }
    write_native_multispan_manifest(path, native)
    return path


def _write_local_manifests(*, degree, n_coeffs, step_count, include_coeff, include_param):
    out = {
        "slv": _write_local_manifest(
            TMP_INPUT_MANIFEST,
            family="slv",
            source_path=TMP_ROOTS,
            row_bytes=root_row_bytes(degree),
            step_count=step_count,
        )
    }
    if include_coeff:
        out["cf"] = _write_local_manifest(
            TMP_COEFFS_MANIFEST,
            family="cf",
            source_path=TMP_COEFFS,
            row_bytes=coeff_row_bytes(n_coeffs),
            step_count=step_count,
        )
    if include_param:
        out["pm"] = _write_local_manifest(
            TMP_PARAMS_MANIFEST,
            family="pm",
            source_path=TMP_PARAMS,
            row_bytes=param_row_bytes(),
            step_count=step_count,
        )
    return out


def _materialize_recomputed_preview(*, params, calc, job_id, degree, n_coeffs, view_n):
    t0 = time.time()
    full_n, times = calc_square_grid(calc)
    view_n = int(view_n)
    if view_n < 1:
        raise RuntimeError(f"preview_source_size must be >= 1, got {view_n}")
    if view_n > full_n:
        raise RuntimeError(f"preview_source_size={view_n} exceeds source N={full_n}")
    n_steps = int(view_n) * int(view_n) * int(times)
    threads = _coerce_int(params.get("recompute_threads", params.get("solve_score_threads", 4)), "recompute_threads", default=4, min_value=1, max_value=16)
    pipeline = _calc_pipeline(calc)
    solver_mode = _calc_solver_mode(calc)
    solver_iters = _calc_solver_iters(calc)

    t_param = time.time()
    param_spec = {
        "mode": "param_gen",
        "n1": int(view_n),
        "n2": int(view_n),
        "gridN": int(full_n),
        "times": int(times),
        "param_transforms": pipeline["param_transforms"],
        "n_threads": int(threads),
    }
    if pipeline.get("param_program"):
        param_spec["param_program"] = pipeline["param_program"]
    param_meta = _run_json_binary(SWEEP_COEFFGEN, TMP_PARAMS, param_spec, phase="recompute param_gen", timeout_s=300)
    param_ms = int((time.time() - t_param) * 1000)
    param_size = os.path.getsize(TMP_PARAMS)
    expected_param_size = n_steps * param_row_bytes()
    if param_size != expected_param_size:
        raise RuntimeError(f"recompute param size mismatch: got {param_size}, expected {expected_param_size}")

    t_coeff = time.time()
    coeff_spec = {
        "mode": "coeffgen_chunked",
        "function": pipeline["function"],
        "params_file": TMP_PARAMS,
        "step_start": 0,
        "source_step_start": 0,
        "source_n1": int(view_n),
        "source_n2": int(view_n),
        "step_count": int(n_steps),
        "coeff_transforms": pipeline["coeff_transforms"],
        "n_threads": int(threads),
    }
    if pipeline["cfpv"]:
        coeff_spec["cfpv"] = pipeline["cfpv"]
    if pipeline.get("coeff_program"):
        coeff_spec["coeff_program"] = pipeline["coeff_program"]
    fused_recompute = solver_mode in ("jt64", "cm64", "ae64")
    if fused_recompute:
        # solve in-process from the f64 coefficients during coeffgen —
        # solving the f32 coeffs file afterwards repaints the transport
        # artifact (user-caught on the compute-job previews)
        coeff_spec["fused_solver"] = solver_mode
        coeff_spec["roots_file"] = TMP_ROOTS
    coeff_meta = _run_json_binary(SWEEP_COEFFGEN, TMP_COEFFS, coeff_spec, phase="recompute coeffgen", timeout_s=300)
    coeff_ms = int((time.time() - t_coeff) * 1000)
    actual_n_coeffs = int(coeff_meta.get("n_coeffs") or 0)
    actual_degree = int(coeff_meta.get("degree") or 0)
    if actual_degree != int(degree):
        raise RuntimeError(f"recompute degree mismatch: got {actual_degree}, expected {degree}")
    if actual_n_coeffs != int(n_coeffs):
        raise RuntimeError(f"recompute n_coeffs mismatch: got {actual_n_coeffs}, expected {n_coeffs}")
    coeff_size = os.path.getsize(TMP_COEFFS)
    expected_coeff_size = n_steps * coeff_row_bytes(n_coeffs)
    if coeff_size != expected_coeff_size:
        raise RuntimeError(f"recompute coeff size mismatch: got {coeff_size}, expected {expected_coeff_size}")

    t_solve = time.time()
    if fused_recompute:
        solve_binary = None
        solve_spec = None
        solve_meta = {
            "mode": f"fused_{solver_mode}",
            "n_t": int(n_steps),
            "degree": int(degree),
            "skipped_overflow": int(coeff_meta.get("solve_skipped", 0) or 0),
            "elapsed_us": int(coeff_meta.get("solve_us", 0) or 0),
        }
    elif solver_mode in ("companion_matrix", "jenkins_traub", "newton"):
        solve_binary = SWEEP_CM
        solve_spec = {
            "mode": {
                "companion_matrix": "solve_cm",
                "jenkins_traub": "solve_jt",
                "newton": "solve_newton",
            }[solver_mode],
            "coeffs_file": TMP_COEFFS,
            "n_coeffs": int(n_coeffs),
            "n_steps": int(n_steps),
        }
        if solver_mode == "newton" and solver_iters > 50:
            raise RuntimeError(
                f"newton solver_iters must be <= 50 (native ceiling), got {solver_iters}")
        if solver_mode == "newton" and solver_iters:
            # reproduce the run's capped-Newton brush at lores fidelity
            solve_spec["max_iter"] = solver_iters
    else:
        solve_binary = SWEEP_MT
        solve_spec = {
            "mode": "solve_mt",
            "coeffs_file": TMP_COEFFS,
            "n_coeffs": int(n_coeffs),
            "n2": int(n_steps),
            "i1_start": 0,
            "i1_end": 1,
            "match_roots": False,
            "n_threads": int(threads),
        }
        if solver_iters:
            # reproduce the run's capped-Aberth brush at lores fidelity
            solve_spec["max_iter"] = solver_iters
    if solve_binary is not None:
        solve_meta = _run_json_binary(solve_binary, TMP_ROOTS, solve_spec, phase="recompute solve", timeout_s=300)
    solve_ms = int((time.time() - t_solve) * 1000)
    if fused_recompute:
        solve_ms = int(round(int(coeff_meta.get("solve_us", 0) or 0) / 1000.0))
    root_size = os.path.getsize(TMP_ROOTS)
    expected_root_size = n_steps * root_row_bytes(degree)
    if root_size != expected_root_size:
        raise RuntimeError(f"recompute root size mismatch: got {root_size}, expected {expected_root_size}")

    return {
        "mode": "recompute",
        "full_N": int(full_n),
        "view_N": int(view_n),
        "times": int(times),
        "n_solves": int(n_steps),
        "solver_mode": solver_mode,
        "function": pipeline["function"],
        "threads": int(threads),
        "output_bytes": int(param_size + coeff_size + root_size),
        "families": {
            "pm": {"output_bytes": int(param_size), "elapsed_ms": param_ms, "meta": param_meta},
            "cf": {"output_bytes": int(coeff_size), "elapsed_ms": coeff_ms, "meta": coeff_meta},
            "slv": {"output_bytes": int(root_size), "elapsed_ms": solve_ms, "meta": solve_meta},
        },
        "elapsed_ms": int((time.time() - t0) * 1000),
        "job_id": str(job_id or ""),
    }


def _write_xforms(root_transforms):
    if not root_transforms:
        return ""
    with open(TMP_XFORMS, "w", encoding="utf-8") as fh:
        json.dump(root_transforms, fh, separators=(",", ":"))
    return TMP_XFORMS


def _write_manifests(
    *,
    job_id,
    degree,
    n_coeffs,
    step_count,
    lores_bin_key,
    root_size,
    lores_coeffs_key,
    coeff_size,
    lores_params_key,
    param_size,
    include_coeff,
    include_param,
):
    chunk = {
        "step_start": 0,
        "step_count": step_count,
        "bin_key": lores_bin_key,
        "bin_size": root_size,
        "coeffs_key": lores_coeffs_key if include_coeff else "",
        "coeffs_bin_size": coeff_size if include_coeff else 0,
        "params_key": lores_params_key if include_param else "",
        "params_bin_size": param_size if include_param else 0,
        "params_step_start": 0,
        "params_step_count": step_count if include_param else 0,
    }
    solve_manifest = build_solve_source_manifest(
        [chunk],
        job_id=job_id,
        degree=degree,
        n_coeffs=n_coeffs,
        include_solve=True,
        include_coeff=include_coeff,
        include_param=include_param,
    )

    out = {}
    for family, path in (("slv", TMP_INPUT_MANIFEST), ("cf", TMP_COEFFS_MANIFEST), ("pm", TMP_PARAMS_MANIFEST)):
        if family == "cf" and not include_coeff:
            continue
        if family == "pm" and not include_param:
            continue
        spans = build_source_spans(solve_manifest, source_family=family, solve_start=0, solve_count=step_count)
        urls = build_native_manifest_urls(s3, BUCKET, spans, expires_in=900)
        native = build_native_multispan_manifest(
            solve_manifest,
            source_family=family,
            solve_start=0,
            solve_count=step_count,
            url_by_key=urls,
        )
        write_native_multispan_manifest(path, native)
        out[family] = path
    return out


def _assemble_fragment_to_raw(fragment_path, raw_path, pix, channels=1):
    total_pixels = int(pix) * int(pix)
    channels = int(channels or 1)
    record_size = 4 + channels
    raw = bytearray(total_pixels * channels)
    channel_histograms = [[0] * 256 for _ in range(channels)]
    entries = 0
    if os.path.exists(fragment_path):
        with open(fragment_path, "rb") as fh:
            while True:
                rec = fh.read(record_size)
                if not rec:
                    break
                if len(rec) != record_size:
                    raise RuntimeError(f"truncated fragment record in {fragment_path}")
                idx = int.from_bytes(rec[:4], "little", signed=False)
                if idx < total_pixels:
                    start = idx * channels
                    values = rec[4:4 + channels]
                    raw[start:start + channels] = values
                    for channel, value in enumerate(values):
                        channel_histograms[channel][value] += 1
                    entries += 1
    with open(raw_path, "wb") as fh:
        fh.write(raw)
    return entries, channel_histograms


def _emission_histograms_from_channels(channel_histograms, output_channels):
    rows = []
    metadata = list(output_channels or [])
    for idx, histogram in enumerate(channel_histograms or []):
        meta = metadata[idx] if idx < len(metadata) and isinstance(metadata[idx], dict) else {}
        total = sum(int(v) for v in histogram)
        zero_count = int(histogram[0]) if histogram else 0
        rows.append({
            "channel": idx,
            "label": f"E{idx + 1}",
            "name": str(meta.get("display_name") or meta.get("name") or f"E{idx + 1}"),
            "emit": str(meta.get("emit") or "emit"),
            "range_normalized": bool(meta.get("range_normalized")),
            "histogram": [int(v) for v in histogram],
            "total": int(total),
            "zero_count": zero_count,
            "nonzero_count": int(total - zero_count),
        })
    return rows


def _preview_palette_grid_n(source_meta, step_count):
    try:
        step_count = int(step_count)
    except (TypeError, ValueError):
        return 0
    raw = source_meta.get("view_N") or source_meta.get("lores_N") or source_meta.get("N")
    try:
        grid_n = int(raw)
    except (TypeError, ValueError):
        grid_n = 0
    if grid_n > 0 and grid_n * grid_n > 0 and step_count % (grid_n * grid_n) == 0:
        return grid_n
    inferred = int(round(math.sqrt(step_count)))
    if inferred > 0 and inferred * inferred == step_count:
        return inferred
    return 0


def _run_roots2pix(*, params, summary, viewport, manifests, pix, degree, n_coeffs, step_count, include_coeff, include_param, palette_grid_n=0, xformed_roots_output=None, xformed_roots_format="f32", root_transforms=None):
    metrics = _metric_rows_from_summary(summary)
    payload = solve_score_program_cli_payload({
        "metrics": metrics,
        "program_spec": str(summary.get("program") or "m0"),
    })
    # artifact-sourced sculptures pass the COMPILED chain recorded in the
    # color artifact's metadata; live previews compile from request params
    if root_transforms is None:
        root_transforms = _root_transforms_for_run(params)
    xforms_path = _write_xforms(root_transforms)
    cmd = [
        ROOTS2PIX_MT,
        "/tmp/render_lores_preview_pix",
        f"--pix={int(pix)}",
        f"--min_re={viewport['min_re']}",
        f"--max_re={viewport['max_re']}",
        f"--min_im={viewport['min_im']}",
        f"--max_im={viewport['max_im']}",
        f"--degree={int(degree)}",
        f"--rotation={_coerce_finite_float(params.get('rotation', 0.0), 'rotation', default=0.0)}",
        f"--threads={_coerce_int(params.get('raster_mt_threads', 4), 'raster_mt_threads', default=4, min_value=1)}",
        f"--input_manifest={manifests['slv']}",
        f"--step_count={int(step_count)}",
        "--prelude_rows=0",
        "--score_coeff_prelude_rows=0",
        "--score_param_prelude_rows=0",
        f"--retries={_coerce_int(params.get('raster_sectioned_retries', 2), 'raster_sectioned_retries', default=2, min_value=0, max_value=10)}",
        f"--fragment_prefix={TMP_FRAGMENT_PREFIX}",
        f"--score_metrics={payload['score_metrics']}",
        f"--score_clip_los={payload['score_clip_los']}",
        f"--score_clip_his={payload['score_clip_his']}",
        f"--score_program={payload['score_program']}",
        f"--score_output_normalize={1 if parse_boolish(summary.get('score_output_normalize', False), False) else 0}",
        f"--score_output_clip_lo={_coerce_finite_float(summary.get('score_output_clip_lo', 0.0), 'score_output_clip_lo', default=0.0)}",
        f"--score_output_clip_hi={_coerce_finite_float(summary.get('score_output_clip_hi', 1.0), 'score_output_clip_hi', default=1.0)}",
    ]
    if int(palette_grid_n or 0) > 0:
        cmd.extend([
            f"--associated_palette_fragment_prefix={TMP_PALETTE_FRAGMENT_PREFIX}",
            f"--palette_grid_n={int(palette_grid_n)}",
            "--palette_step_start=0",
        ])
    output_channels = list(summary.get("score_output_channels") or [])
    if output_channels:
        cmd.append("--score_output_clip_los=" + ",".join(
            str(_coerce_finite_float(row.get("clip_lo", 0.0), "score_output_channel.clip_lo", default=0.0))
            for row in output_channels
        ))
        cmd.append("--score_output_clip_his=" + ",".join(
            str(_coerce_finite_float(row.get("clip_hi", 1.0), "score_output_channel.clip_hi", default=1.0))
            for row in output_channels
        ))
    if payload.get("score_sources"):
        cmd.append(f"--score_sources={payload['score_sources']}")
    if include_coeff:
        cmd.extend([
            f"--score_coeff_manifest={manifests['cf']}",
            f"--score_coeff_degree={int(n_coeffs)}",
        ])
    if include_param:
        cmd.append(f"--score_params_manifest={manifests['pm']}")
    if xforms_path:
        cmd.append(f"--root_xforms={xforms_path}")
    if xformed_roots_output:
        # sculpture data: the post-xform post-rotation roots exactly as the
        # raster plotted them — the 3D viewer must see what the 2D saw
        cmd.append(f"--xformed_roots_output={xformed_roots_output}")
        if xformed_roots_format and xformed_roots_format != "f32":
            cmd.append(f"--xformed_roots_format={xformed_roots_format}")

    result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
    if result.returncode != 0:
        raise RuntimeError(f"roots2pix_mt preview failed: {result.stderr.strip() or 'unknown error'}")
    try:
        meta = json.loads(result.stdout or "{}")
    except Exception as exc:
        raise RuntimeError(f"roots2pix_mt preview returned invalid JSON: {(result.stdout or '')[:200]!r}") from exc
    return meta


_ARTIFACT_ID_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,127}")


def _artifact_sculpture_summary():
    # minimal m0 scoring for the fused raster: the artifact's STORED
    # step_scores provide every color, so the raster pass exists only to
    # apply the artifact's transform chain + rotation and dump the plotted
    # positions. proximity/m0 keeps the C contract satisfied at ~O(degree)
    # per solve — negligible next to the range-GET materialization.
    return {
        "program": "m0",
        "metrics": [{
            "slot": 0,
            "metric": "proximity",
            "quantile": 1.0,
            "source": "slv",
            "clip_lo": 0.0,
            "clip_hi": 1.0,
        }],
        "score_output_normalize": False,
        "score_output_clip_lo": 0.0,
        "score_output_clip_hi": 1.0,
        "score_output_channels": [],
        "score_output_channel_count": 1,
        "score_output_interpretation": "scalar_lut",
    }


def _subsample_step_scores_pass0(src_path, out_path, *, full_n, view_n, channels):
    """Pick the artifact's stored per-solve scores at EXACTLY the lattice the
    logical roots materializer walks (_logical_row_mapping — same physical
    solves as the dump), writing the palette raw de-serpentined to row-major
    (row, col) — the palette PNG convention the viewer decodes. Pass 0 only,
    matching the sculpture's pass convention."""
    with open(src_path, "rb") as fh:
        raw = fh.read()
    need = full_n * full_n * channels
    if len(raw) < need:
        raise RuntimeError(
            f"step_scores.raw too small: got {len(raw)} bytes, need {need} for grid {full_n} x{channels}ch")
    dest = bytearray(view_n * view_n * channels)
    for lrow in range(view_n):
        physical_i1, physical_js = _logical_row_mapping(full_n, view_n, lrow)
        for lj, physical_j in enumerate(physical_js):
            src = (physical_i1 * full_n + physical_j) * channels
            lcol = (view_n - 1 - lj) if (lrow & 1) else lj
            dst = (lrow * view_n + lcol) * channels
            dest[dst:dst + channels] = raw[src:src + channels]
    with open(out_path, "wb") as fh:
        fh.write(bytes(dest))
    return len(dest)


def _artifact_meta_viewport(meta, artifact_id):
    viewport = {}
    for key in ("min_re", "max_re", "min_im", "max_im"):
        try:
            viewport[key] = float(meta.get(key))
        except (TypeError, ValueError):
            raise RuntimeError(
                f"color artifact {artifact_id} does not record its viewport ({key}) — "
                "re-render it to sculpture it")
        if not math.isfinite(viewport[key]):
            raise RuntimeError(f"color artifact {artifact_id} viewport {key} is not finite")
    if not (viewport["max_re"] > viewport["min_re"] and viewport["max_im"] > viewport["min_im"]):
        raise RuntimeError(f"color artifact {artifact_id} viewport is degenerate")
    return viewport


def _sculpture_cache_load(cache_prefix):
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=cache_prefix + "sculpture.json")
        body = obj["Body"]
        raw = body.read() if hasattr(body, "read") else b"".join(body.iter_chunks(chunk_size=1024 * 1024))
        block = json.loads(raw)
    except Exception:
        return None
    if not isinstance(block, dict) or int(block.get("grid_n") or 0) < 1:
        return None
    return block


def _sculpture_full_viewer_template():
    here = os.path.dirname(os.path.abspath(__file__))
    for cand in (os.path.join(here, "sculpture.html"),
                 os.path.join(here, "..", "sculpture.html")):
        if os.path.exists(cand):
            with open(cand, "rb") as fh:
                return fh.read()
    raise RuntimeError("sculpture.html viewer template not packaged with this lambda")


def _sculpture_save_full(save_spec, *, job_id, export, source):
    """Persist a durable full viewer (sculptures/{id}/) INSIDE the generate
    task, from immutable sources: the local /tmp files on a fresh run, or
    the content-addressed cache prefix on a hit. Never the job-wide mutable
    sculpture_* keys — a concurrent same-job generate could swap those
    between steps (CR: SaveFull silent-corruption race). Runs BEFORE the
    task reports done, so rail-complete means durable."""
    sid = _mint_sculpture_id()
    raw_title = "".join(ch for ch in str((save_spec or {}).get("title") or "").strip() if ch.isprintable())[:80]
    title = raw_title or sid          # ids name saves (user doctrine)
    sprefix = f"sculptures/{sid}/"
    cache_forever = "public, max-age=31536000, immutable"
    if source["kind"] == "local":
        with open(source["roots_path"], "rb") as fh:
            s3.put_object(Bucket=BUCKET, Key=sprefix + "roots.bin", Body=fh.read(),
                          ContentType="application/octet-stream", CacheControl=cache_forever)
        with open(source["palette_path"], "rb") as fh:
            s3.put_object(Bucket=BUCKET, Key=sprefix + "palette.png", Body=fh.read(),
                          ContentType="image/png", CacheControl=cache_forever)
    else:
        s3.copy_object(Bucket=BUCKET, Key=sprefix + "roots.bin",
                       CopySource={"Bucket": BUCKET, "Key": source["prefix"] + "roots.bin"},
                       MetadataDirective="REPLACE",
                       ContentType="application/octet-stream", CacheControl=cache_forever)
        s3.copy_object(Bucket=BUCKET, Key=sprefix + "palette.png",
                       CopySource={"Bucket": BUCKET, "Key": source["prefix"] + "palette.png"},
                       MetadataDirective="REPLACE",
                       ContentType="image/png", CacheControl=cache_forever)
    meta = {
        "version": 1,
        "id": sid,
        "title": title,
        "job_id": job_id,
        "grid_n": int(export["grid_n"]),
        "degree": int(export["degree"]),
        "step_count": int(export["step_count"]),
        "pass_count": int(export.get("pass_count") or 1),
        "roots_key": "roots.bin",
        "palette_key": "palette.png",
        "format": export.get("format") or "u16",
        "roots_bytes": int(export.get("roots_bytes") or 0),
        "viewport": export["viewport"],
        "palette": "".join(ch for ch in str(export.get("palette") or "") if ch.isprintable())[:64],
        "score_display": "",
        "created_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
    }
    source_artifact_id = str(export.get("source_artifact_id") or "").strip()
    if source_artifact_id and _ARTIFACT_ID_RE.fullmatch(source_artifact_id):
        meta["source_artifact_id"] = source_artifact_id
    view = sanitize_sculpture_view((save_spec or {}).get("view"))
    if view:
        meta["view"] = view
    # meta.json is the PUBLICATION MARKER the listing trusts — it goes
    # LAST, after the viewer. On failure the cleanup is state-resolving,
    # ordered, and checked (CR: DeleteObjects reports per-key failures
    # without raising, and an ambiguous marker PUT may have landed):
    #   1. if the marker PUT was attempted, READ IT BACK — the exact bytes
    #      present means the publication actually completed: succeed;
    #   2. confirmed-absent marker: delete-and-check the marker key FIRST,
    #      only then the payload, so no interleaving can leave a marker
    #      over deleted files;
    #   3. unknown marker state (read failed non-404): PRESERVE everything
    #      — orphaned bytes are recoverable, a broken catalog row is not.
    meta_bytes = json.dumps(meta).encode("utf-8")
    marker_key = sprefix + "meta.json"

    def _delete_checked(keys, phase):
        resp = s3.delete_objects(Bucket=BUCKET, Delete={"Objects": [{"Key": k} for k in keys]})
        errs = (resp or {}).get("Errors") or []
        if errs:
            failed = ", ".join(str(e.get("Key")) for e in errs)
            raise RuntimeError(f"save cleanup ({phase}) could not delete: {failed}")

    marker_attempted = False
    try:
        s3.put_object(Bucket=BUCKET, Key=sprefix + "viewer.html",
                      Body=_sculpture_full_viewer_template(),
                      ContentType="text/html", CacheControl=cache_forever)
        marker_attempted = True
        s3.put_object(Bucket=BUCKET, Key=marker_key,
                      Body=meta_bytes,
                      ContentType="application/json", CacheControl="no-cache")
    except Exception as publish_exc:
        marker_state = "absent"
        if marker_attempted:
            try:
                got = s3.get_object(Bucket=BUCKET, Key=marker_key)
                marker_state = "published" if got["Body"].read() == meta_bytes else "foreign"
            except Exception as read_exc:
                marker_state = "absent" if is_missing_s3_error(read_exc) else "unknown"
        if marker_state == "published":
            # the "failed" PUT landed: payload + viewer + marker all exist —
            # this publication is complete and reports as SUCCESS
            pass
        elif marker_state == "absent":
            _delete_checked([marker_key], "marker")
            _delete_checked([sprefix + k for k in ("roots.bin", "palette.png", "viewer.html")],
                            "payload")
            raise
        else:
            raise RuntimeError(
                f"save publication state for {sprefix} is {marker_state} after a failed "
                f"marker write — objects PRESERVED for manual inspection "
                f"({type(publish_exc).__name__}: {publish_exc})") from publish_exc
    region = os.environ.get("AWS_REGION", "us-east-1")
    out = dict(meta)
    out["prefix"] = sprefix
    out["share_url"] = f"https://{BUCKET}.s3.{region}.amazonaws.com/{sprefix}viewer.html"
    return out


def _b36_bake_id(v):
    digits = "0123456789abcdefghijklmnopqrstuvwxyz"
    out = ""
    v = int(v)
    while v:
        v, rem = divmod(v, 36)
        out = digits[rem] + out
    return out or "0"


def _mint_sculpture_id():
    """scu_{ms}{6 random b36}: two Lambdas landing in the same millisecond
    must not share a global sculptures/ prefix (CR: silent object mixing).
    Fail closed on the publication marker — a taken prefix is an error,
    never an overwrite."""
    sid = f"scu_{_b36_bake_id(int(time.time() * 1000))}{random_b36(6)}"
    try:
        s3.head_object(Bucket=BUCKET, Key=f"sculptures/{sid}/meta.json")
    except Exception as exc:
        # CR28-F13 taxonomy: only a CONFIRMED 404 is availability. A
        # throttle/timeout/AccessDenied must propagate — treating it as
        # "available" would reopen the overwrite this mint exists to close.
        if is_missing_s3_error(exc):
            return sid
        raise RuntimeError(f"sculpture id mint could not verify {sid} is free "
                           f"({type(exc).__name__}) — retry the save") from exc
    raise RuntimeError(f"sculpture id collision on {sid} — retry the save")


def _run_artifact_sculpture(params, job_id, spec, t_start):
    """Artifact-sourced sculpture: the ONLY generation path. Everything comes
    from the selected color artifact's recorded provenance — viewport,
    rotation, compiled root-transform chain, palette, interpretation, and the
    stored per-solve scores — so the job is nearly pure I/O: range-GET the
    transport roots at the lattice, apply the artifact's transforms in the
    fused raster (trivial m0 score), subsample the stored step_scores for
    colors, and upload the ephemeral viewer pair. No score evaluation."""
    artifact_id = str((spec or {}).get("artifact_id") or "").strip()
    if not _ARTIFACT_ID_RE.fullmatch(artifact_id):
        raise RuntimeError("artifact_sculpture requires a valid artifact_id")
    task_id = str(params.get("sculpture_task_id") or "").strip()
    if task_id:
        report_status(job_id, task_id, "running")
    view_n = _coerce_int(params.get("preview_source_size"), "preview_source_size", min_value=2, max_value=512)
    fmt = str(params.get("sculpture_format") or "u16").strip().lower()
    if fmt not in ("f32", "u16"):
        raise RuntimeError(f"sculpture_format must be f32 or u16, got {fmt!r}")

    head = load_color_artifact_head(s3, BUCKET, job_id, artifact_id)
    meta = dict(head.get("metadata") or {})
    step_scores_key = str(meta.get("step_scores_key") or "").strip()
    if not step_scores_key:
        raise RuntimeError(
            f"color artifact {artifact_id} has no stored per-solve scores (step_scores) — "
            "re-render it to sculpture it")
    full_n = int(meta.get("step_scores_grid_n") or 0)
    if full_n < 1:
        raise RuntimeError(f"color artifact {artifact_id} records step_scores_grid_n={full_n}")
    channels = int(meta.get("score_output_channel_count") or 1)
    if channels not in (1, 3):
        raise RuntimeError(f"color artifact {artifact_id} has unsupported score channel count {channels}")
    interpretation = str(meta.get("score_output_interpretation") or ("scalar_lut" if channels == 1 else "rgb"))
    palette = str(meta.get("palette") or "inferno")
    background_color = normalize_background_color(meta.get("background_color"))
    viewport = _artifact_meta_viewport(meta, artifact_id)
    rotation = float(meta.get("rotation") or 0.0)
    root_transforms = parse_root_transforms(meta.get("root_transforms"))

    # content-hash reuse: same artifact + size + format + color/transform
    # provenance -> the previous run's dump serves instantly (server-side
    # copies into the fixed ephemeral keys the viewer and Save read). The
    # signature covers everything that shapes the output, so a repalette
    # of the SAME artifact_id (new palette metadata) misses and rebuilds.
    cache_sig = json.dumps({
        "cache_schema": 2,
        "artifact_id": artifact_id,
        "view_n": view_n,
        "format": fmt,
        "step_scores_key": step_scores_key,
        "grid_n": full_n,
        "channels": channels,
        "interpretation": interpretation,
        "palette": palette,
        "background_color": background_color,
        "rotation": rotation,
        "viewport": viewport,
        "root_transforms": root_transforms,
    }, sort_keys=True, separators=(",", ":"))
    cache_prefix = f"renders/{job_id}/sculpture_cache/{hashlib.sha1(cache_sig.encode('utf-8')).hexdigest()[:16]}/"
    sculpture_roots_key = f"renders/{job_id}/sculpture_roots.bin"
    sculpture_palette_key = f"renders/{job_id}/sculpture_palette.png"
    cached = _sculpture_cache_load(cache_prefix)
    if cached is not None:
        # the fallback catch covers ONLY cache loading/validation — once
        # the hit is materialized, save/report failures must PROPAGATE:
        # swallowing them re-ran the whole generation and could mint a
        # SECOND sculpture after a successful save (CR)
        sculpture_export = None
        try:
            s3.copy_object(
                Bucket=BUCKET, Key=sculpture_roots_key,
                CopySource={"Bucket": BUCKET, "Key": cache_prefix + "roots.bin"},
                MetadataDirective="REPLACE",
                ContentType="application/octet-stream", CacheControl="no-cache")
            s3.copy_object(
                Bucket=BUCKET, Key=sculpture_palette_key,
                CopySource={"Bucket": BUCKET, "Key": cache_prefix + "palette.png"},
                MetadataDirective="REPLACE",
                ContentType="image/png", CacheControl="no-cache")
            region = os.environ.get("AWS_REGION", "us-east-1")
            base_url = f"https://{BUCKET}.s3.{region}.amazonaws.com"
            stamp = int(time.time() * 1000)
            sculpture_export = dict(cached)
            sculpture_export.update({
                "cache_prefix": cache_prefix,
                "roots_key": sculpture_roots_key,
                "roots_url": f"{base_url}/{sculpture_roots_key}?v={stamp}",
                "palette_key": sculpture_palette_key,
                "palette_url": f"{base_url}/{sculpture_palette_key}?v={stamp}",
            })
        except Exception:
            # cached binaries missing/unreadable -> fall through to a full run
            sculpture_export = None
        if sculpture_export is not None:
            saved_row = None
            if params.get("save_full") is not None:
                # immutable source: the content-addressed cache prefix
                saved_row = _sculpture_save_full(
                    params.get("save_full"), job_id=job_id,
                    export=sculpture_export,
                    source={"kind": "cache", "prefix": cache_prefix})
            total_ms = int((time.time() - t_start) * 1000)
            if task_id:
                done_data = {
                    "sculpture": sculpture_export,
                    "n_solves": int(sculpture_export.get("step_count") or 0),
                    "total_ms": total_ms,
                    "cache_hit": True,
                }
                if saved_row:
                    done_data["saved_sculpture"] = saved_row
                report_status(job_id, task_id, "done", result_data=done_data)
            body = {
                "sculpture": sculpture_export,
                "cache": {"hit": True, "prefix": cache_prefix},
                "n_solves": int(sculpture_export.get("step_count") or 0),
                "logs": [f"Artifact sculpture cache hit: {cache_prefix}"],
                "timings_ms": {"total": total_ms},
            }
            if saved_row:
                body["saved_sculpture"] = saved_row
            return ok_response(body)

    calc = _load_calc(job_id)
    degree = int(calc.get("degree") or 0)
    if degree < 1:
        raise RuntimeError("calc.json carries no degree")
    n_coeffs = int(calc.get("n_coeffs") or (degree + 1))
    if view_n > full_n:
        raise RuntimeError(f"sculpture size {view_n} exceeds the solve grid {full_n}")

    t_materialize = time.time()
    source_meta = materialize_logical_lores(
        s3_client=s3,
        bucket=BUCKET,
        calc=calc,
        job_id=job_id,
        degree=degree,
        n_coeffs=n_coeffs,
        view_n=view_n,
        out_paths={"slv": TMP_ROOTS},
        include_coeff=False,
        include_param=False,
    )
    materialize_ms = int((time.time() - t_materialize) * 1000)
    if int(source_meta["full_N"]) != full_n:
        raise RuntimeError(
            f"artifact step_scores grid {full_n} does not match the transport grid "
            f"{source_meta['full_N']} — the artifact belongs to a different solve")
    step_count = int(source_meta["n_solves"])

    manifests = _write_local_manifests(
        degree=degree,
        n_coeffs=n_coeffs,
        step_count=step_count,
        include_coeff=False,
        include_param=False,
    )
    t_raster = time.time()
    raster_meta = _run_roots2pix(
        params={
            "rotation": rotation,
            "raster_mt_threads": params.get("raster_mt_threads", 4),
            "raster_sectioned_retries": params.get("raster_sectioned_retries", 2),
        },
        summary=_artifact_sculpture_summary(),
        viewport=viewport,
        manifests=manifests,
        pix=64,
        degree=degree,
        n_coeffs=n_coeffs,
        step_count=step_count,
        include_coeff=False,
        include_param=False,
        palette_grid_n=0,
        xformed_roots_output=TMP_XFORMED_ROOTS,
        xformed_roots_format=fmt,
        root_transforms=root_transforms,
    )
    raster_ms = int((time.time() - t_raster) * 1000)
    xformed_size = os.path.getsize(TMP_XFORMED_ROOTS) if os.path.exists(TMP_XFORMED_ROOTS) else 0
    expected_xformed = step_count * degree * 2 * (2 if fmt == "u16" else 4)
    if xformed_size != expected_xformed:
        raise RuntimeError(
            f"sculpture transformed-roots dump size mismatch: got {xformed_size}, expected {expected_xformed}")

    t_scores = time.time()
    _download_to_file(step_scores_key, TMP_STEP_SCORES)
    _subsample_step_scores_pass0(
        TMP_STEP_SCORES,
        TMP_PALETTE_RAW,
        full_n=full_n,
        view_n=view_n,
        channels=channels,
    )
    palette_histogram = histogram_from_raw_path_channel0(
        TMP_PALETTE_RAW,
        channels=channels,
        expected_size=view_n * view_n * channels,
    )
    write_equalization_lut(TMP_PALETTE_EQ_LUT, palette_histogram)
    palette_render_meta = render_score_raw(
        raw_path=TMP_PALETTE_RAW,
        out_path=TMP_PALETTE_IMAGE,
        preview_path="",
        pix=view_n,
        eq_lut_path=TMP_PALETTE_EQ_LUT,
        palette=palette,
        background_color=background_color,
        quality=90,
        channels=channels,
        interpretation=interpretation,
        zero_background=False,
    )
    scores_ms = int((time.time() - t_scores) * 1000)

    region = os.environ.get("AWS_REGION", "us-east-1")
    base_url = f"https://{BUCKET}.s3.{region}.amazonaws.com"
    with open(TMP_XFORMED_ROOTS, "rb") as fh:
        s3.put_object(
            Bucket=BUCKET, Key=sculpture_roots_key, Body=fh.read(),
            ContentType="application/octet-stream", CacheControl="no-cache")
    with open(TMP_PALETTE_IMAGE, "rb") as fh:
        s3.put_object(
            Bucket=BUCKET, Key=sculpture_palette_key, Body=fh.read(),
            ContentType="image/png", CacheControl="no-cache")
    # version-stamped URLs: fixed keys + browser heuristic caching served
    # stale viewer data across runs
    stamp = int(time.time() * 1000)
    sculpture_export = {
        "format": fmt,
        "cache_prefix": cache_prefix,
        "grid_n": int(view_n),
        "degree": int(degree),
        "step_count": int(step_count),
        "pass_count": int(step_count // (view_n * view_n)),
        "roots_bytes": int(xformed_size),
        "viewport": viewport,
        "palette": palette,
        "source_artifact_id": artifact_id,
        "roots_key": sculpture_roots_key,
        "roots_url": f"{base_url}/{sculpture_roots_key}?v={stamp}",
        "palette_key": sculpture_palette_key,
        "palette_url": f"{base_url}/{sculpture_palette_key}?v={stamp}",
    }
    try:
        # the cache is written from the bytes THIS invocation produced —
        # never via the job-wide mutable keys, which a concurrent same-job
        # run can overwrite between steps (CR: poisoned cache under A's
        # signature). sculpture.json goes last: it is the commit marker
        # the cache loader trusts.
        with open(TMP_XFORMED_ROOTS, "rb") as fh:
            s3.put_object(Bucket=BUCKET, Key=cache_prefix + "roots.bin",
                          Body=fh.read(), ContentType="application/octet-stream")
        with open(TMP_PALETTE_IMAGE, "rb") as fh:
            s3.put_object(Bucket=BUCKET, Key=cache_prefix + "palette.png",
                          Body=fh.read(), ContentType="image/png")
        cache_block = {k: v for k, v in sculpture_export.items() if k not in ("roots_url", "palette_url")}
        s3.put_object(
            Bucket=BUCKET, Key=cache_prefix + "sculpture.json",
            Body=json.dumps(cache_block).encode("utf-8"),
            ContentType="application/json")
        cache_state = {"hit": False, "prefix": cache_prefix}
    except Exception as exc:
        # the cache is an optimization — a failed write must not fail the run
        cache_state = {"hit": False, "prefix": cache_prefix, "write_error": str(exc)}
    saved_row = None
    if params.get("save_full") is not None:
        # immutable source: the bytes this very invocation produced — never
        # the job-wide ephemeral keys another run could have overwritten
        saved_row = _sculpture_save_full(
            params.get("save_full"), job_id=job_id,
            export=sculpture_export,
            source={"kind": "local",
                    "roots_path": TMP_XFORMED_ROOTS,
                    "palette_path": TMP_PALETTE_IMAGE})
    total_ms = int((time.time() - t_start) * 1000)
    logs = [
        "Artifact sculpture: "
        f"source={artifact_id} grid={view_n}x{view_n} degree={degree} "
        f"channels={channels} palette={palette} rotation={rotation}",
        "Artifact sculpture materialize: "
        f"full_N={source_meta.get('full_N')} ranges={source_meta.get('range_gets')} "
        f"read={int(source_meta.get('bytes_read') or 0) / (1024 * 1024):.1f}MB "
        f"wall={materialize_ms / 1000.0:.2f}s",
        "Artifact sculpture timings: "
        f"materialize={materialize_ms / 1000.0:.2f}s raster={raster_ms / 1000.0:.2f}s "
        f"scores={scores_ms / 1000.0:.2f}s total={total_ms / 1000.0:.2f}s",
    ]
    if task_id:
        done_data = {
            "sculpture": sculpture_export,
            "n_solves": int(step_count),
            "total_ms": total_ms,
        }
        if saved_row:
            done_data["saved_sculpture"] = saved_row
        report_status(job_id, task_id, "done", result_data=done_data)
    return ok_response({
        "sculpture": sculpture_export,
        "saved_sculpture": saved_row,
        "cache": cache_state,
        "source": source_meta,
        "raster": raster_meta,
        "palette_render": palette_render_meta,
        "n_solves": int(step_count),
        "logs": logs,
        "timings_ms": {
            "materialize": materialize_ms,
            "raster": raster_ms,
            "scores": scores_ms,
            "total": total_ms,
        },
    })


_SPLAT_BAKE_RES = (64, 96, 128, 192)
_SPLAT_BAKE_SAVED_ID = re.compile(r"scu_[a-z0-9]{1,32}")
_SPLAT_BAKE_CACHE_RE = re.compile(r"renders/[A-Za-z0-9_-]{1,64}/sculpture_cache/[0-9a-f]{16}/")


def _splat_bake_template():
    here = os.path.dirname(os.path.abspath(__file__))
    for cand in (os.path.join(here, "splat_bake_template.html"),
                 os.path.join(here, "..", "splat_bake_template.html")):
        if os.path.exists(cand):
            with open(cand, "r", encoding="utf-8") as fh:
                return fh.read()
    raise RuntimeError("splat_bake_template.html not packaged with this lambda")


def _splat_bake_triple(raw, default):
    if not isinstance(raw, (list, tuple)) or len(raw) != 3:
        return list(default)
    out = []
    for v in raw:
        try:
            f = float(v)
        except (TypeError, ValueError):
            return list(default)
        if not math.isfinite(f) or abs(f) > 100:
            return list(default)
        out.append(f)
    return out


def _splat_bake_params(raw):
    """Sanitize the bake's view/settings blob — the whole point of the
    server-side bake is that THIS is all a client sends."""
    p = raw if isinstance(raw, dict) else {}

    def clampf(key, lo, hi, dflt):
        try:
            v = float(p.get(key))
        except (TypeError, ValueError):
            return dflt
        if not math.isfinite(v):
            return dflt
        return min(hi, max(lo, v))

    res = int(p.get("res") or 96)
    if res not in _SPLAT_BAKE_RES:
        raise RuntimeError(f"splat res must be one of {_SPLAT_BAKE_RES}, got {res}")
    try:
        slices = int(p.get("slices") or 0)
    except (TypeError, ValueError):
        slices = 0
    if not (0 <= slices <= 64):
        raise RuntimeError(f"slices out of range: {slices}")
    mode = p.get("mode")
    mode = int(mode) if mode in (0, 1, 2, 0.0, 1.0, 2.0) else 2
    tour = p.get("tour") if p.get("tour") in ("off", "orbit", "wave", "grand", "weave") else "off"
    speed = clampf("tourSpeed", 0.5, 4.0, 1.0)
    if speed not in (0.5, 1.0, 2.0, 4.0):
        speed = 1.0
    title = "".join(ch for ch in str(p.get("title") or "") if ch.isprintable())[:120]
    return {
        "res": res,
        "zaxis": p.get("zaxis") if p.get("zaxis") in ("t1", "t2") else "t2",
        "slices": slices,
        "mode": mode,
        "intensity": clampf("intensity", 0.01, 10.0, 1.0),
        "yscale": clampf("yscale", 0.0, 1.0, 0.1),
        "scalemul": clampf("scalemul", 0.05, 10.0, 1.0),
        "cam": _splat_bake_triple(p.get("cam"), (1.25, 0.85, 1.25)),
        "target": _splat_bake_triple(p.get("target"), (0.0, 0.0, 0.0)),
        "tour": tour,
        "tourSpeed": speed,
        "title": title,
    }


def _splat_bake_get_json(key):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"]
    raw = body.read() if hasattr(body, "read") else b"".join(body.iter_chunks(chunk_size=1024 * 1024))
    return json.loads(raw)


def _splat_bake_get_bytes(key):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"]
    return body.read() if hasattr(body, "read") else b"".join(body.iter_chunks(chunk_size=1024 * 1024))


def _run_splat_bake(params, job_id, spec, t_start):
    """Server-side SplatBake: the splats are a pure function of (roots dump
    x per-solve colors x a small settings blob), all server-resident — so a
    hosted baked viewer is minted from a ~1KB request, never a browser
    upload. Sources: a generate's content-addressed cache, any saved
    sculpture prefix, or artifact+size (which runs the generate core first
    — parameters -> baked share in one job, no tabs)."""
    task_id = str(params.get("sculpture_task_id") or "").strip()
    if task_id:
        report_status(job_id, task_id, "running")
    source = (spec or {}).get("source") or {}
    p = _splat_bake_params((spec or {}).get("params"))
    kind = str(source.get("kind") or "")
    source_ids = {}
    title_default = "PolyPaint splats"
    provenance = {}          # what actually happened — the anti-"fishy" record

    if kind == "saved":
        sid = str(source.get("saved_id") or "").strip()
        if not _SPLAT_BAKE_SAVED_ID.fullmatch(sid):
            raise RuntimeError("splat_bake saved source requires a valid saved_id")
        meta = _splat_bake_get_json(f"sculptures/{sid}/meta.json")
        if meta.get("kind") == "splatbake":
            raise RuntimeError("that row is already a baked viewer")
        grid_n = int(meta.get("grid_n") or 0)
        degree = int(meta.get("degree") or 0)
        fmt = "u16" if meta.get("format") == "u16" else "f32"
        viewport = {k: float((meta.get("viewport") or {}).get(k)) for k in ("min_re", "max_re", "min_im", "max_im")}
        _download_to_file(f"sculptures/{sid}/roots.bin", TMP_SPLAT_ROOTS)
        palette_png = _splat_bake_get_bytes(f"sculptures/{sid}/palette.png")
        source_ids = {"source_saved_id": sid}
        if meta.get("source_artifact_id"):
            source_ids["source_artifact_id"] = meta["source_artifact_id"]
        title_default = f"{meta.get('title') or sid} · baked"
    elif kind == "cache":
        prefix = str(source.get("cache_prefix") or "")
        if not _SPLAT_BAKE_CACHE_RE.fullmatch(prefix) or f"/{job_id}/" not in f"/{prefix}":
            raise RuntimeError("splat_bake cache source requires this job's cache prefix")
        block = _splat_bake_get_json(prefix + "sculpture.json")
        grid_n = int(block.get("grid_n") or 0)
        degree = int(block.get("degree") or 0)
        fmt = "u16" if block.get("format") == "u16" else "f32"
        viewport = {k: float((block.get("viewport") or {}).get(k)) for k in ("min_re", "max_re", "min_im", "max_im")}
        _download_to_file(prefix + "roots.bin", TMP_SPLAT_ROOTS)
        palette_png = _splat_bake_get_bytes(prefix + "palette.png")
        if block.get("source_artifact_id"):
            source_ids["source_artifact_id"] = block["source_artifact_id"]
        title_default = f"{block.get('source_artifact_id') or job_id} · {grid_n}²"
    elif kind == "artifact":
        artifact_id = str(source.get("artifact_id") or "").strip()
        try:
            n = int(source.get("n"))
        except (TypeError, ValueError):
            raise RuntimeError("splat_bake artifact source requires integer n")
        # run the generate core (cache-aware: a warm cache returns in
        # seconds); its response body is its contract — compose, don't fork
        t_gen = time.time()
        gen = _run_artifact_sculpture(
            {"preview_source_size": n, "sculpture_format": "u16"},
            job_id, {"artifact_id": artifact_id}, time.time())
        body = json.loads(gen["body"])
        sc = body["sculpture"]
        grid_n = int(sc["grid_n"])
        degree = int(sc["degree"])
        fmt = "u16" if sc.get("format") == "u16" else "f32"
        viewport = {k: float(sc["viewport"][k]) for k in ("min_re", "max_re", "min_im", "max_im")}
        provenance = {
            "generate_cache_hit": bool((body.get("cache") or {}).get("hit")),
            "generate_ms": int((time.time() - t_gen) * 1000),
        }
        # the generate just refreshed the fixed keys — and on a MISS its
        # local /tmp outputs are still on disk: reuse them instead of a
        # 36MB S3 round trip (user-caught "why so long": one avoidable
        # re-download; the solve-score is never recomputed either way)
        expected_dump = int(sc.get("roots_bytes") or 0)
        if (os.path.exists(TMP_XFORMED_ROOTS)
                and expected_dump > 0
                and os.path.getsize(TMP_XFORMED_ROOTS) == expected_dump):
            os.replace(TMP_XFORMED_ROOTS, TMP_SPLAT_ROOTS)
            provenance["roots_reused_locally"] = True
        else:
            # cache-hit generate left no local files: read the IMMUTABLE
            # content-addressed prefix, never the mutable job-wide keys a
            # concurrent run can swap (CR)
            _download_to_file(str(sc.get("cache_prefix") or "") + "roots.bin", TMP_SPLAT_ROOTS)
            provenance["roots_reused_locally"] = False
        if os.path.exists(TMP_PALETTE_IMAGE) and os.path.getsize(TMP_PALETTE_IMAGE) > 0:
            with open(TMP_PALETTE_IMAGE, "rb") as fh:
                palette_png = fh.read()
        else:
            palette_png = _splat_bake_get_bytes(str(sc.get("cache_prefix") or "") + "palette.png")
        source_ids = {"source_artifact_id": artifact_id}
        title_default = f"{artifact_id} · {grid_n}²"
    else:
        raise RuntimeError(f"splat_bake source kind must be cache/saved/artifact, got {kind!r}")

    if grid_n < 2 or degree < 1:
        raise RuntimeError(f"splat_bake source carries no usable geometry (grid {grid_n}, degree {degree})")
    if not (viewport["max_re"] > viewport["min_re"] and viewport["max_im"] > viewport["min_im"]):
        raise RuntimeError("splat_bake source viewport is degenerate")

    t_colors = time.time()
    w, h, rgb = decode_png_rgb(palette_png)
    if w != grid_n or h != grid_n:
        raise RuntimeError(f"palette PNG is {w}x{h}, expected {grid_n}x{grid_n}")
    with open(TMP_SPLAT_COLORS, "wb") as fh:
        fh.write(rgb)
    colors_ms = int((time.time() - t_colors) * 1000)

    t_tool = time.time()
    cmd = [
        SPLAT_BAKE_BIN,
        f"--roots={TMP_SPLAT_ROOTS}",
        f"--colors={TMP_SPLAT_COLORS}",
        f"--out={TMP_SPLAT_PACK}",
        f"--roots_format={fmt}",
        f"--grid_n={grid_n}",
        f"--degree={degree}",
        f"--min_re={viewport['min_re']}",
        f"--max_re={viewport['max_re']}",
        f"--min_im={viewport['min_im']}",
        f"--max_im={viewport['max_im']}",
        f"--res={p['res']}",
        f"--zaxis={p['zaxis']}",
        f"--slices={p['slices']}",
        f"--yscale={p['yscale']}",
        f"--scalemul={p['scalemul']}",
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if proc.returncode != 0:
        raise RuntimeError(f"splat_bake failed: {proc.stderr.strip() or 'unknown error'}")
    try:
        tool = json.loads(proc.stdout or "{}")
    except Exception as exc:
        raise RuntimeError(f"splat_bake returned invalid JSON: {(proc.stdout or '')[:200]!r}") from exc
    tool_ms = int((time.time() - t_tool) * 1000)

    title = p["title"] or title_default
    header = {
        "v": 1,
        "count": int(tool["count"]),
        "cmin": tool["cmin"],
        "cmax": tool["cmax"],
        "amax": tool["amax"],
        "mode": p["mode"],
        "intensity": p["intensity"],
        "cam": p["cam"],
        "target": p["target"],
        "tour": p["tour"],
        "tourSpeed": p["tourSpeed"],
        "title": title,
    }
    with open(TMP_SPLAT_PACK, "rb") as fh:
        pack = fh.read()
    if len(pack) != 22 * int(tool["count"]):
        raise RuntimeError(f"splat pack size {len(pack)} != 22*{tool['count']}")
    title_html = str(title).replace("&", "&amp;").replace("<", "&lt;")
    html = (_splat_bake_template()
            .replace("__TITLE_HTML__", title_html)
            .replace("__HEADER_JSON__", json.dumps(header))
            .replace("__B64__", base64.b64encode(pack).decode("ascii")))
    html_bytes = html.encode("utf-8")

    sid_new = _mint_sculpture_id()
    sprefix = f"sculptures/{sid_new}/"
    s3.put_object(
        Bucket=BUCKET, Key=sprefix + "viewer.html",
        Body=html_bytes, ContentType="text/html",
        CacheControl="public, max-age=31536000, immutable")
    row = {
        "version": 1,
        "kind": "splatbake",
        "id": sid_new,
        "title": title,
        "job_id": job_id,
        "splat_count": int(tool["count"]),
        "bytes": len(html_bytes),
        "bake_params": {k: p[k] for k in ("res", "zaxis", "slices", "mode", "tour")},
        "created_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
    }
    row.update(source_ids)
    s3.put_object(
        Bucket=BUCKET, Key=sprefix + "meta.json",
        Body=json.dumps(row).encode("utf-8"),
        ContentType="application/json", CacheControl="no-cache")
    region = os.environ.get("AWS_REGION", "us-east-1")
    out_row = dict(row)
    out_row["prefix"] = sprefix
    out_row["share_url"] = f"https://{BUCKET}.s3.{region}.amazonaws.com/{sprefix}viewer.html"

    total_ms = int((time.time() - t_start) * 1000)
    logs = [
        f"Splat bake: source={kind} grid={grid_n}² degree={degree} res={p['res']} "
        f"splats={tool['count']} points={tool['points_used']} clipped={tool['points_clipped']}"
        + (f" · generate {'CACHE HIT' if provenance.get('generate_cache_hit') else 'materialized'}"
           f" {provenance.get('generate_ms', 0) / 1000.0:.1f}s" if kind == "artifact" else ""),
        f"Splat bake timings: colors={colors_ms / 1000.0:.2f}s tool={tool_ms / 1000.0:.2f}s "
        f"total={total_ms / 1000.0:.2f}s html={len(html_bytes) / (1024 * 1024):.1f}MB",
    ]
    if task_id:
        report_status(job_id, task_id, "done", result_data={
            "sculpture": out_row,
            "provenance": provenance,
            "total_ms": total_ms,
        })
    return ok_response({
        "sculpture": out_row,
        "provenance": provenance,
        "tool": tool,
        "logs": logs,
        "timings_ms": {"colors": colors_ms, "tool": tool_ms, "total": total_ms},
    })


def handler(event, context):
    params = {}
    t_start = time.time()
    try:
        parsed = parse_body(event)
        if not isinstance(parsed, dict):
            raise RuntimeError("request body must be a JSON object")
        params = parsed
        params["background_color"] = normalize_background_color(params.get("background_color"))
        _cleanup_tmp()
        job_id = str(params.get("job_id") or "").strip()
        if not job_id:
            raise RuntimeError("job_id is required")
        if params.get("splat_bake") is not None:
            # server-side SplatBake: settings in, hosted baked viewer out
            return _run_splat_bake(params, job_id, params.get("splat_bake"), t_start)
        if params.get("artifact_sculpture") is not None:
            # artifact-sourced sculpture: a self-contained flow — geometry,
            # colors, and provenance all come from the saved color artifact,
            # never from live render state (degree comes from calc.json)
            return _run_artifact_sculpture(params, job_id, params.get("artifact_sculpture"), t_start)
        degree = _coerce_int(params.get("degree"), "degree", min_value=1)
        n_coeffs = _coerce_n_coeffs(params.get("n_coeffs"), degree)
        pix = _coerce_int(
            params.get("preview_pix", DEFAULT_PREVIEW_PIX),
            "preview_pix",
            default=DEFAULT_PREVIEW_PIX,
            min_value=16,
            max_value=MAX_PREVIEW_PIX,
        )
        source_mode = _preview_source_mode(params)
        lores_bin_key = str(params.get("lores_bin_key") or "").strip()
        if not lores_bin_key and source_mode == "lores":
            raise RuntimeError("lores_bin_key is required")

        lores_coeffs_key = str(params.get("lores_coeffs_key") or _fallback_lores_coeffs_key(job_id, lores_bin_key)).strip()
        lores_params_key = str(params.get("lores_params_key") or _fallback_lores_params_key(job_id, lores_bin_key)).strip()

        compiled = _compile_request_chain(params)
        solve_score_normalize = parse_boolish(
            params.get("solve_score_normalize", False),
            False,
            strict=True,
            label="solve_score_normalize",
        )
        if compiled.get("has_explicit_outputs") and solve_score_normalize:
            raise RuntimeError("score normalization checkbox is legacy-only; explicit emit/emit_norm programs own normalization")
        include_coeff = solve_score_uses_source(compiled, "cf")
        include_param = solve_score_uses_source(compiled, "pm")

        source_meta = {"mode": "physical"}
        materialize_ms = 0
        coeff_size = 0
        param_size = 0
        if source_mode in ("logical", "recompute"):
            calc = _load_calc(job_id)
            calc_degree = int(calc.get("degree") or degree)
            calc_n_coeffs = int(calc.get("n_coeffs") or n_coeffs)
            if calc_degree != degree:
                raise RuntimeError(f"request degree={degree} does not match calc.json degree={calc_degree}")
            if include_coeff and calc_n_coeffs != n_coeffs:
                raise RuntimeError(f"request n_coeffs={n_coeffs} does not match calc.json n_coeffs={calc_n_coeffs}")
            default_logical_n = logical_lores_default_n(calc)
            logical_n = _coerce_int(
                params.get("preview_source_size", params.get("logical_lores_size", default_logical_n)),
                "preview_source_size",
                default=default_logical_n,
                min_value=5,
                max_value=MAX_LOGICAL_LORES_N,
            )
        if source_mode == "logical":
            estimate = estimate_logical_lores_bytes(
                calc=calc,
                degree=degree,
                n_coeffs=n_coeffs,
                view_n=logical_n,
                include_coeff=include_coeff,
                include_param=include_param,
            )
            t_materialize = time.time()
            source_meta = materialize_logical_lores(
                s3_client=s3,
                bucket=BUCKET,
                calc=calc,
                job_id=job_id,
                degree=degree,
                n_coeffs=n_coeffs,
                view_n=logical_n,
                out_paths={"slv": TMP_ROOTS, "cf": TMP_COEFFS, "pm": TMP_PARAMS},
                include_coeff=include_coeff,
                include_param=include_param,
            )
            materialize_ms = int((time.time() - t_materialize) * 1000)
            source_meta["estimated_source_bytes"] = int(estimate["estimated_source_bytes"])
            source_meta["estimated_output_bytes"] = int(estimate["estimated_output_bytes"])
            root_size = int(source_meta["families"]["slv"]["output_bytes"])
            coeff_size = int(source_meta["families"].get("cf", {}).get("output_bytes") or 0)
            param_size = int(source_meta["families"].get("pm", {}).get("output_bytes") or 0)
            step_count = int(source_meta["n_solves"])
        elif source_mode == "recompute":
            t_materialize = time.time()
            source_meta = _materialize_recomputed_preview(
                params=params,
                calc=calc,
                job_id=job_id,
                degree=degree,
                n_coeffs=n_coeffs,
                view_n=logical_n,
            )
            materialize_ms = int((time.time() - t_materialize) * 1000)
            root_size = int(source_meta["families"]["slv"]["output_bytes"])
            coeff_size = int(source_meta["families"]["cf"]["output_bytes"])
            param_size = int(source_meta["families"]["pm"]["output_bytes"])
            step_count = int(source_meta["n_solves"])
        else:
            requested_lores_n = _coerce_int(
                params.get("lores_N", params.get("lores_n", 0)),
                "lores_N",
                default=0,
                min_value=0,
            )
            if requested_lores_n > 0:
                source_meta["view_N"] = int(requested_lores_n)
            root_size = _download_to_file(lores_bin_key, TMP_ROOTS)
            rb = root_row_bytes(degree)
            if root_size % rb != 0:
                raise RuntimeError(f"lores root size {root_size} is not divisible by root row size {rb}")
            step_count = root_size // rb
            if step_count < 1:
                raise RuntimeError("lores root artifact has no solve rows")

            if include_coeff:
                if n_coeffs < 1:
                    raise RuntimeError("coeff-source preview requires n_coeffs >= 1")
                if not lores_coeffs_key:
                    raise RuntimeError("coeff-source preview requires lores_coeffs_key")
                coeff_size = _download_to_file(lores_coeffs_key, TMP_COEFFS)
                needed = step_count * coeff_row_bytes(n_coeffs)
                if coeff_size < needed:
                    raise RuntimeError(f"lores coeffs artifact too small: got {coeff_size}, need at least {needed}")

            if include_param:
                if not lores_params_key:
                    raise RuntimeError("param-source preview requires lores_params_key")
                param_size = _download_to_file(lores_params_key, TMP_PARAMS)
                needed = step_count * param_row_bytes()
                if param_size < needed:
                    raise RuntimeError(f"lores params artifact too small: got {param_size}, need at least {needed}")

        t_summary = time.time()
        summary = _preview_score_summary(
            params,
            degree=degree,
            n_coeffs=n_coeffs,
            compiled=compiled,
            include_coeff=include_coeff,
            include_param=include_param,
        )
        summary_ms = int((time.time() - t_summary) * 1000)
        color_contract = validate_color_output_contract(
            interpretation=params.get("color_interpretation", params.get("score_output_interpretation", "scalar_lut")),
            output_channel_count=int(summary.get("score_output_channel_count") or 1),
            output_channels=summary.get("score_output_channels") or [],
        )
        summary["score_output_interpretation"] = color_contract["interpretation"]
        summary["score_output_channels"] = color_contract["channels"]
        preview_warnings = list(color_contract.get("warnings") or [])
        metrics = _metric_rows_from_summary(summary)
        include_coeff = _uses_source(metrics, "cf")
        include_param = _uses_source(metrics, "pm")

        t_viewport = time.time()
        viewport = _compute_preview_viewport(params, TMP_ROOTS)
        viewport_ms = int((time.time() - t_viewport) * 1000)
        t_manifest = time.time()
        if source_mode in ("logical", "recompute"):
            manifests = _write_local_manifests(
                degree=degree,
                n_coeffs=n_coeffs,
                step_count=step_count,
                include_coeff=include_coeff,
                include_param=include_param,
            )
        else:
            manifests = _write_manifests(
                job_id=job_id,
                degree=degree,
                n_coeffs=n_coeffs,
                step_count=step_count,
                lores_bin_key=lores_bin_key,
                root_size=root_size,
                lores_coeffs_key=lores_coeffs_key,
                coeff_size=coeff_size,
                lores_params_key=lores_params_key,
                param_size=param_size,
                include_coeff=include_coeff,
                include_param=include_param,
            )
        manifest_ms = int((time.time() - t_manifest) * 1000)
        palette_grid_n = _preview_palette_grid_n(source_meta, step_count)

        t_raster = time.time()
        raster_meta = _run_roots2pix(
            params=params,
            summary=summary,
            viewport=viewport,
            manifests=manifests,
            pix=pix,
            degree=degree,
            n_coeffs=n_coeffs,
            step_count=step_count,
            include_coeff=include_coeff,
            include_param=include_param,
            palette_grid_n=palette_grid_n,
        )
        raster_ms = int((time.time() - t_raster) * 1000)

        output_channels = int(summary.get("score_output_channel_count") or 1)
        fragment_entries, emission_channel_histograms = _assemble_fragment_to_raw(
            TMP_FRAGMENT,
            TMP_RAW,
            pix,
            channels=output_channels,
        )
        emission_histograms = _emission_histograms_from_channels(
            emission_channel_histograms,
            summary.get("score_output_channels") or [],
        )
        histogram = histogram_from_raw_path_channel0(TMP_RAW, channels=output_channels, expected_size=pix * pix * output_channels)
        nonzero_pixels = write_equalization_lut(TMP_EQ_LUT, histogram)
        render_meta = render_score_raw(
            raw_path=TMP_RAW,
            out_path=TMP_IMAGE,
            preview_path="",
            pix=pix,
            eq_lut_path=TMP_EQ_LUT,
            palette=str(params.get("palette") or "inferno"),
            background_color=str(params.get("background_color") or "000000"),
            quality=_coerce_int(params.get("quality", 90), "quality", default=90, min_value=1, max_value=100),
            channels=output_channels,
            interpretation=summary["score_output_interpretation"],
        )
        with open(TMP_IMAGE, "rb") as fh:
            image_b64 = base64.b64encode(fh.read()).decode("ascii")

        palette_image_b64 = ""
        palette_render_meta = {}
        palette_entries = 0
        if palette_grid_n > 0:
            palette_entries, _palette_channel_histograms = _assemble_fragment_to_raw(
                TMP_PALETTE_FRAGMENT,
                TMP_PALETTE_RAW,
                palette_grid_n,
                channels=output_channels,
            )
            expected_palette_entries = int(palette_grid_n) * int(palette_grid_n)
            if int(palette_entries) != expected_palette_entries:
                raise RuntimeError(
                    "preview palette fragment entry count mismatch: "
                    f"expected {expected_palette_entries}, got {int(palette_entries)}"
                )
            palette_histogram = histogram_from_raw_path_channel0(
                TMP_PALETTE_RAW,
                channels=output_channels,
                expected_size=palette_grid_n * palette_grid_n * output_channels,
            )
            write_equalization_lut(TMP_PALETTE_EQ_LUT, palette_histogram)
            palette_render_meta = render_score_raw(
                raw_path=TMP_PALETTE_RAW,
                out_path=TMP_PALETTE_IMAGE,
                preview_path="",
                pix=palette_grid_n,
                eq_lut_path=TMP_PALETTE_EQ_LUT,
                palette=str(params.get("palette") or "inferno"),
                background_color=str(params.get("background_color") or "000000"),
                quality=_coerce_int(params.get("quality", 90), "quality", default=90, min_value=1, max_value=100),
                channels=output_channels,
                interpretation=summary["score_output_interpretation"],
                zero_background=False,
            )
            with open(TMP_PALETTE_IMAGE, "rb") as fh:
                palette_image_b64 = base64.b64encode(fh.read()).decode("ascii")

        total_ms = int((time.time() - t_start) * 1000)
        logs = []
        if source_mode == "logical":
            fam = source_meta.get("families") or {}
            families_label = ",".join(sorted(fam.keys()))
            logs.append(
                "Logical lores: "
                f"source full_N={source_meta.get('full_N')} view_N={source_meta.get('view_N')} "
                f"times={source_meta.get('times')} solves={source_meta.get('n_solves')} "
                f"families={families_label}"
            )
            logs.append(
                "Logical lores materialize: "
                f"read={int(source_meta.get('bytes_read') or 0) / (1024 * 1024):.1f}MB "
                f"compact={int(source_meta.get('output_bytes') or 0) / (1024 * 1024):.1f}MB "
                f"ranges={source_meta.get('range_gets')} wall={materialize_ms / 1000.0:.2f}s"
            )
            for family in ("slv", "cf", "pm"):
                if family not in fam:
                    continue
                row = fam[family]
                logs.append(
                    f"Logical lores {family}: "
                    f"rows={row.get('source_rows')} "
                    f"read={int(row.get('bytes_read') or 0) / (1024 * 1024):.1f}MB "
                    f"compact={int(row.get('output_bytes') or 0) / (1024 * 1024):.1f}MB "
                    f"ranges={row.get('range_gets')} wall={int(row.get('elapsed_ms') or 0) / 1000.0:.2f}s"
                )
        elif source_mode == "recompute":
            fam = source_meta.get("families") or {}
            logs.append(
                "Recompute preview: "
                f"source full_N={source_meta.get('full_N')} view_N={source_meta.get('view_N')} "
                f"times={source_meta.get('times')} solves={source_meta.get('n_solves')} "
                f"solver={source_meta.get('solver_mode')} function={source_meta.get('function')}"
            )
            logs.append(
                "Recompute preview materialize: "
                f"tmp={int(source_meta.get('output_bytes') or 0) / (1024 * 1024):.1f}MB "
                f"threads={source_meta.get('threads')} wall={materialize_ms / 1000.0:.2f}s"
            )
            for family, label in (("pm", "param_gen"), ("cf", "coeffgen"), ("slv", "solve")):
                if family not in fam:
                    continue
                row = fam[family]
                logs.append(
                    f"Recompute {label}: "
                    f"size={int(row.get('output_bytes') or 0) / (1024 * 1024):.1f}MB "
                    f"wall={int(row.get('elapsed_ms') or 0) / 1000.0:.2f}s"
                )
        else:
            logs.append(f"Physical lores: solves={int(step_count)} roots={int(root_size) / (1024 * 1024):.1f}MB")
        emit_modes = ",".join(str(row.get("emit") or "emit") for row in (summary.get("score_output_channels") or []))
        logs.append(
            "Render preview output: "
            f"channels={output_channels} interpretation={summary.get('score_output_interpretation')} "
            f"emit={emit_modes or 'emit'}"
        )
        if palette_grid_n > 0:
            logs.append(
                "Render preview palette: "
                f"grid={palette_grid_n}x{palette_grid_n} entries={int(palette_entries)} "
                f"zero=data"
            )
        for warning in preview_warnings:
            logs.append(f"Render preview warning: {warning}")
        logs.append(
            "Render preview timings: "
            f"summary={summary_ms / 1000.0:.2f}s viewport={viewport_ms / 1000.0:.2f}s "
            f"manifest={manifest_ms / 1000.0:.2f}s raster={raster_ms / 1000.0:.2f}s "
            f"total={total_ms / 1000.0:.2f}s"
        )

        return ok_response({
            "image_base64": image_b64,
            "palette_image_base64": palette_image_b64,
            "content_type": "image/png",
            "palette_content_type": "image/png",
            "preview_pix": pix,
            "palette_pix": int(palette_grid_n),
            "degree": degree,
            "n_coeffs": n_coeffs,
            "n_solves": int(step_count),
            "source": source_meta,
            "fragment_entries": int(fragment_entries),
            "nonzero_pixels": int(nonzero_pixels),
            "viewport": viewport,
            "raster": raster_meta,
            "render": render_meta,
            "palette_render": palette_render_meta,
            "palette_fragment_entries": int(palette_entries),
            "emission_histograms": emission_histograms,
            "logs": logs,
            "solve_score": {
                "program": summary.get("program"),
                "display": summary.get("solve_score_display"),
                "score_output_normalize": bool(summary.get("score_output_normalize", False)),
                "score_output_clip_lo": summary.get("score_output_clip_lo"),
                "score_output_clip_hi": summary.get("score_output_clip_hi"),
                "score_output_clip_source": summary.get("score_output_clip_source"),
                "score_output_channel_count": output_channels,
                "score_output_interpretation": summary.get("score_output_interpretation"),
                "score_output_channels": summary.get("score_output_channels"),
                "emission_histograms": emission_histograms,
                "warnings": preview_warnings,
            },
            "timings_ms": {
                "materialize": materialize_ms,
                "summary": summary_ms,
                "viewport": viewport_ms,
                "manifest": manifest_ms,
                "raster": raster_ms,
                "total": total_ms,
            },
        })
    except Exception as exc:
        try:
            task_id = str(params.get("sculpture_task_id") or "").strip()
            if task_id and params.get("job_id"):
                report_status(str(params["job_id"]), task_id, "error", error_msg=str(exc))
        except Exception:
            pass
        return _json_response(500, {
            "error": "render lores preview failed",
            "phase": "render-lores-preview",
            "detail": str(exc),
            "job_id": params.get("job_id"),
            "lores_bin_key": params.get("lores_bin_key"),
        })
    finally:
        _cleanup_tmp()
