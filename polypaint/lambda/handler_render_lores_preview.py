"""
Ephemeral lores render preview.

This colorizes one of three preview sources with the current render parameters:
saved lores artifacts, a logical hires subset, or recomputed temporary artifacts.
It does not persist images, save palettes, or write metadata.
"""
import base64
import glob
import json
import math
import os
import subprocess
import time

import boto3

from logical_lores import (
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
from raw_score_render import histogram_from_raw_path, render_score_raw, write_equalization_lut
from shared import BUCKET, REF_SIZE, compute_viewport_from_bin, ok_response, parse_body, parse_boolish
from solve_score_chain import (
    compiled_solve_score_fingerprint,
    compile_solve_score_chain_or_legacy,
    serialize_solve_score_chain,
    solve_score_program_cli_payload,
    solve_score_uses_source,
)


s3 = boto3.client("s3")

ROOTS2PIX_MT = os.path.join(os.path.dirname(__file__), "roots2pix_mt")
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
TMP_RAW = "/tmp/render_lores_preview.raw"
TMP_EQ_LUT = "/tmp/render_lores_preview_eq.bin"
TMP_IMAGE = "/tmp/render_lores_preview.png"

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
        TMP_RAW,
        TMP_EQ_LUT,
        TMP_IMAGE,
        TMP_FRAGMENT_PREFIX + "*",
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
    coeff_transforms = pipeline.get("coeff_transforms")
    if not isinstance(coeff_transforms, list):
        coeff_transforms = []
    cfpv = pipeline.get("cfpv")
    if not isinstance(cfpv, list):
        cfpv = []
    return {
        "function": function_name,
        "param_transforms": param_transforms,
        "coeff_transforms": coeff_transforms,
        "cfpv": cfpv,
    }


def _calc_solver_mode(calc):
    raw = str((calc or {}).get("solver") or "aberth_mt").strip().lower()
    if raw in ("companion_matrix", "cm", "solve_cm"):
        return "companion_matrix"
    return "aberth_mt"


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
    return compile_solve_score_chain_or_legacy(
        params.get("solve_score_chain", ""),
        params.get("metric", "proximity"),
        params.get("solve_score_quantile", 0.001),
        params.get("solve_score_omega", 1.0),
        params.get("solve_score_omega_enabled", True),
        default_metric="proximity",
    )


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
    root_transforms = params.get("root_transforms") or []
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
    summary["score_output_normalize"] = solve_score_normalize
    if solve_score_normalize:
        summary.update(_score_output_clip_from_summary(summary))
    else:
        summary.update({
            "score_output_clip_lo": 0.0,
            "score_output_clip_hi": 1.0,
            "score_output_clip_source": "identity",
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
        "step_count": int(n_steps),
        "coeff_transforms": pipeline["coeff_transforms"],
        "n_threads": int(threads),
    }
    if pipeline["cfpv"]:
        coeff_spec["cfpv"] = pipeline["cfpv"]
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
    if solver_mode == "companion_matrix":
        solve_binary = SWEEP_CM
        solve_spec = {
            "mode": "solve_cm",
            "coeffs_file": TMP_COEFFS,
            "n_coeffs": int(n_coeffs),
            "n_steps": int(n_steps),
        }
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
    solve_meta = _run_json_binary(solve_binary, TMP_ROOTS, solve_spec, phase="recompute solve", timeout_s=300)
    solve_ms = int((time.time() - t_solve) * 1000)
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


def _assemble_fragment_to_raw(fragment_path, raw_path, pix):
    total_pixels = int(pix) * int(pix)
    raw = bytearray(total_pixels)
    entries = 0
    if os.path.exists(fragment_path):
        with open(fragment_path, "rb") as fh:
            while True:
                rec = fh.read(5)
                if not rec:
                    break
                if len(rec) != 5:
                    raise RuntimeError(f"truncated fragment record in {fragment_path}")
                idx = int.from_bytes(rec[:4], "little", signed=False)
                if idx < total_pixels:
                    raw[idx] = rec[4]
                    entries += 1
    with open(raw_path, "wb") as fh:
        fh.write(raw)
    return entries


def _run_roots2pix(*, params, summary, viewport, manifests, pix, degree, n_coeffs, step_count, include_coeff, include_param):
    metrics = _metric_rows_from_summary(summary)
    payload = solve_score_program_cli_payload({
        "metrics": metrics,
        "program_spec": str(summary.get("program") or "m0"),
    })
    root_transforms = params.get("root_transforms") or []
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

    result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
    if result.returncode != 0:
        raise RuntimeError(f"roots2pix_mt preview failed: {result.stderr.strip() or 'unknown error'}")
    try:
        meta = json.loads(result.stdout or "{}")
    except Exception as exc:
        raise RuntimeError(f"roots2pix_mt preview returned invalid JSON: {(result.stdout or '')[:200]!r}") from exc
    return meta


def handler(event, context):
    params = {}
    t_start = time.time()
    try:
        parsed = parse_body(event)
        if not isinstance(parsed, dict):
            raise RuntimeError("request body must be a JSON object")
        params = parsed
        _cleanup_tmp()
        job_id = str(params.get("job_id") or "").strip()
        if not job_id:
            raise RuntimeError("job_id is required")
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
        )
        raster_ms = int((time.time() - t_raster) * 1000)

        fragment_entries = _assemble_fragment_to_raw(TMP_FRAGMENT, TMP_RAW, pix)
        histogram = histogram_from_raw_path(TMP_RAW, expected_size=pix * pix)
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
        )
        with open(TMP_IMAGE, "rb") as fh:
            image_b64 = base64.b64encode(fh.read()).decode("ascii")

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
        logs.append(
            "Render preview timings: "
            f"summary={summary_ms / 1000.0:.2f}s viewport={viewport_ms / 1000.0:.2f}s "
            f"manifest={manifest_ms / 1000.0:.2f}s raster={raster_ms / 1000.0:.2f}s "
            f"total={total_ms / 1000.0:.2f}s"
        )

        return ok_response({
            "image_base64": image_b64,
            "content_type": "image/png",
            "preview_pix": pix,
            "degree": degree,
            "n_coeffs": n_coeffs,
            "n_solves": int(step_count),
            "source": source_meta,
            "fragment_entries": int(fragment_entries),
            "nonzero_pixels": int(nonzero_pixels),
            "viewport": viewport,
            "raster": raster_meta,
            "render": render_meta,
            "logs": logs,
            "solve_score": {
                "program": summary.get("program"),
                "display": summary.get("solve_score_display"),
                "score_output_normalize": bool(summary.get("score_output_normalize", False)),
                "score_output_clip_lo": summary.get("score_output_clip_lo"),
                "score_output_clip_hi": summary.get("score_output_clip_hi"),
                "score_output_clip_source": summary.get("score_output_clip_source"),
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
        return _json_response(500, {
            "error": "render lores preview failed",
            "phase": "render-lores-preview",
            "detail": str(exc),
            "job_id": params.get("job_id"),
            "lores_bin_key": params.get("lores_bin_key"),
        })
    finally:
        _cleanup_tmp()
