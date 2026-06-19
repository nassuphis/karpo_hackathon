"""
Solve score Lambda — async 3-phase prepass for solve-level color modes.

Supports the full solve-metric set defined in solve_score.h.
Phases: clip, hist, merge.

Dispatched async via dispatch handler, reports status to DynamoDB.
"""
import json
import os
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

import boto3
from botocore.config import Config

from logical_sections import (
    build_native_manifest_urls,
    build_native_multispan_manifest,
    build_source_spans,
    resolve_solve_source_manifest,
    write_native_multispan_manifest,
)
from solve_score_chain import (
    SOLVE_SCORE_SPEC_VERSION,
    VALID_SOLVE_SCORE_METRICS,
    compiled_solve_score_fingerprint,
    compile_solve_score_chain_or_legacy,
    serialize_solve_score_chain,
    solve_score_lag_prelude_by_source,
    solve_score_program_cli_payload,
    solve_score_program_specs_match,
    solve_score_uses_source,
)
from shared import BUCKET, attach_contract_warnings, contract_param, parse_body, ok_response, parse_boolish, report_status

s3 = boto3.client("s3")
BINARY = os.path.join(os.path.dirname(__file__), "solve_proximity_stats")
SECTIONED_HIST_BINARY = os.path.join(os.path.dirname(__file__), "solve_proximity_hist_sectioned")

VALID_METRICS = set(VALID_SOLVE_SCORE_METRICS)
VALID_HIST_INPUT_MODES = {"tmpfile", "stdin", "sectioned"}

_TMP_INPUT = "/tmp/solve_prox_input.bin"
_TMP_COEFF_INPUT = "/tmp/solve_prox_coeff_input.bin"
_TMP_PARAM_INPUT = "/tmp/solve_prox_param_input.bin"
_TMP_XFORMS = "/tmp/solve_prox_root_xforms.json"
_TMP_CLIP = "/tmp/solve_prox_clip.json"
_TMP_HIST = "/tmp/solve_prox_hist.json"
_TMP_INPUT_MANIFEST = "/tmp/solve_prox_input_manifest.json"
_TMP_COEFF_INPUT_MANIFEST = "/tmp/solve_prox_coeff_manifest.json"
_TMP_PARAM_INPUT_MANIFEST = "/tmp/solve_prox_param_manifest.json"

_CLIP_RANGE_MIN_WIDTH = 1e-12
_CLIP_RANGE_WIDEN_REL = 1e-4


def _cleanup_tmp():
    for p in [
        _TMP_INPUT,
        _TMP_COEFF_INPUT,
        _TMP_PARAM_INPUT,
        _TMP_XFORMS,
        _TMP_CLIP,
        _TMP_HIST,
        _TMP_INPUT_MANIFEST,
        _TMP_COEFF_INPUT_MANIFEST,
        _TMP_PARAM_INPUT_MANIFEST,
    ]:
        try:
            os.remove(p)
        except OSError:
            pass


def _download(key, path):
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as e:
        raise RuntimeError(f"Failed to download s3://{BUCKET}/{key}: {e}") from e
    with open(path, "wb") as f:
        for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
            f.write(chunk)
    return os.path.getsize(path)


def _download_range(key, path, start, length):
    if length <= 0:
        raise RuntimeError(f"Invalid range length for s3://{BUCKET}/{key}: {length}")
    end = int(start) + int(length) - 1
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key, Range=f"bytes={int(start)}-{end}")
    except Exception as e:
        raise RuntimeError(
            f"Failed to download range bytes={int(start)}-{end} from s3://{BUCKET}/{key}: {e}"
        ) from e
    with open(path, "wb") as f:
        for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
            f.write(chunk)
    size = os.path.getsize(path)
    if size != int(length):
        raise RuntimeError(
            f"Short ranged download from s3://{BUCKET}/{key}: expected {int(length)} bytes, got {size}"
        )
    return size


def _write_xforms(root_transforms):
    if root_transforms:
        with open(_TMP_XFORMS, "w") as f:
            json.dump(root_transforms, f)
        return _TMP_XFORMS
    return None


def _validate_metric(metric):
    if metric not in VALID_METRICS:
        raise RuntimeError(f"Invalid metric: {metric} (valid: {', '.join(sorted(VALID_METRICS))})")


def _coerce_finite_float(value, label):
    try:
        number = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{label} must be numeric, got {value!r}")
    if not (number == number and abs(number) != float("inf")):
        raise RuntimeError(f"{label} must be finite, got {value!r}")
    return number


def _clip_widen_half_width(*values):
    scale = 1.0
    for value in values:
        if value in ("", None):
            continue
        try:
            number = float(value)
        except (TypeError, ValueError):
            continue
        if number == number and abs(number) != float("inf"):
            scale = max(scale, abs(number))
    return max(scale * _CLIP_RANGE_WIDEN_REL, _CLIP_RANGE_MIN_WIDTH)


def _sanitize_metric_clip(metric_row):
    row = dict(metric_row or {})
    slot = row.get("slot", "?")
    metric_name = row.get("metric", "unknown")
    clip_lo = _coerce_finite_float(row.get("clip_lo"), f"clip_lo for metric slot {slot}")
    clip_hi = _coerce_finite_float(row.get("clip_hi"), f"clip_hi for metric slot {slot}")
    width = clip_hi - clip_lo
    if width < -_CLIP_RANGE_MIN_WIDTH:
        raise RuntimeError(
            f"invalid clip range for metric slot {slot} ({metric_name}): lo={clip_lo} hi={clip_hi}"
        )
    if width < _CLIP_RANGE_MIN_WIDTH:
        center = 0.5 * (clip_lo + clip_hi)
        half_width = _clip_widen_half_width(center, clip_lo, clip_hi, row.get("min_score"), row.get("max_score"))
        clip_lo = center - half_width
        clip_hi = center + half_width
        row["clip_fallback"] = "degenerate_widened"
        row["clip_center"] = center
    row["clip_lo"] = clip_lo
    row["clip_hi"] = clip_hi
    return row


def _clone_metric_clips(metrics):
    sanitized = []
    for slot, item in enumerate(metrics or []):
        row = dict(item or {})
        row.setdefault("slot", slot)
        sanitized.append(_sanitize_metric_clip(row))
    return sanitized


def _clip_metric_slot(metric_name, quantile, degree, solve_score_omega, solve_score_omega_enabled, solve_score_threads, root_transforms, input_path):
    quantile_lo = quantile
    quantile_hi = 1.0 - quantile
    cmd = [
        BINARY,
        input_path,
        "--mode=clip",
        f"--degree={degree}",
        f"--metric={metric_name}",
        f"--quantile_lo={quantile_lo}",
        f"--quantile_hi={quantile_hi}",
        f"--omega={solve_score_omega}",
        f"--omega_enabled={1 if solve_score_omega_enabled else 0}",
        f"--threads={solve_score_threads}",
    ]
    xf_path = _write_xforms(root_transforms)
    if xf_path:
        cmd.append(f"--root_xforms={xf_path}")
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        raise RuntimeError(f"solve_proximity_stats clip failed for {metric_name}: {result.stderr.strip()}")
    data = json.loads(result.stdout)
    return _sanitize_metric_clip({
        "metric": metric_name,
        "quantile": quantile,
        "quantile_pct": quantile * 100.0,
        "clip_lo": data["clip_lo"],
        "clip_hi": data["clip_hi"],
        "min_score": data["min_score"],
        "max_score": data["max_score"],
        "n_solves": data["n_solves"],
        "threads": int(data.get("threads", solve_score_threads)),
    })


def _sanitize_score_output_clip(lo, hi, source="lores_q05_q95"):
    clip_lo = _coerce_finite_float(lo, "score_output_clip_lo")
    clip_hi = _coerce_finite_float(hi, "score_output_clip_hi")
    if clip_hi - clip_lo <= _CLIP_RANGE_MIN_WIDTH:
        return {
            "score_output_clip_lo": 0.0,
            "score_output_clip_hi": 1.0,
            "score_output_clip_source": "degenerate_identity",
        }
    return {
        "score_output_clip_lo": clip_lo,
        "score_output_clip_hi": clip_hi,
        "score_output_clip_source": source,
    }


def _score_output_clip_from_summary(data):
    has_quantiles = data.get("q05") is not None and data.get("q95") is not None
    return _sanitize_score_output_clip(
        data.get("q05") if has_quantiles else data.get("min_score"),
        data.get("q95") if has_quantiles else data.get("max_score"),
        source="lores_q05_q95" if has_quantiles else "lores_minmax",
    )


def _score_output_channels_from_summary(compiled, data, *, legacy_normalize=False):
    has_explicit = bool((compiled or {}).get("has_explicit_outputs"))
    compiled_channels = list((compiled or {}).get("output_channels") or []) if has_explicit else []
    summary_channels = data.get("score_output_channels")
    by_channel = {
        int(row.get("channel", idx)): row
        for idx, row in enumerate(summary_channels or [])
        if isinstance(row, dict)
    }
    if not compiled_channels:
        compiled_channels = [{
            "name": "score",
            "emit": "emit_norm" if legacy_normalize else "emit",
            "channel": 0,
            "range_normalized": bool(legacy_normalize),
        }]
    channels = []
    for idx, row in enumerate(compiled_channels):
        channel = int(row.get("channel", idx))
        summary = by_channel.get(channel) or (data if not has_explicit else {})
        emit = str(row.get("emit") or summary.get("emit") or "emit").strip()
        range_normalized = bool(
            row.get("range_normalized")
            or emit == "emit_norm"
            or summary.get("range_normalized")
        )
        out = {
            "channel": channel,
            "name": str(summary.get("display_name") or summary.get("name") or row.get("name") or f"channel_{channel}"),
            "emit": "emit_norm" if range_normalized else "emit",
            "range_normalized": range_normalized,
            "min_score": summary.get("min_score"),
            "max_score": summary.get("max_score"),
            "q05": summary.get("q05"),
            "q95": summary.get("q95"),
        }
        if range_normalized:
            clip = _sanitize_score_output_clip(
                summary.get("q05", summary.get("min_score", 0.0)),
                summary.get("q95", summary.get("max_score", 1.0)),
                source="lores_q05_q95" if summary.get("q05") is not None and summary.get("q95") is not None else "lores_minmax",
            )
            out["clip_lo"] = clip["score_output_clip_lo"]
            out["clip_hi"] = clip["score_output_clip_hi"]
            out["clip_source"] = clip["score_output_clip_source"]
        else:
            out["clip_lo"] = 0.0
            out["clip_hi"] = 1.0
            out["clip_source"] = "identity"
        channels.append(out)
    return channels


def _score_output_contract(compiled, data, *, legacy_normalize=False):
    channels = _score_output_channels_from_summary(compiled, data, legacy_normalize=legacy_normalize)
    has_explicit = bool((compiled or {}).get("has_explicit_outputs"))
    channel_count = len(channels)
    fragment_encoding = (
        "u32le_u8_v1"
        if channel_count == 1 and not has_explicit
        else "u32le_pixel_idx_plus_u8_channels_v1"
    )
    primary = channels[0] if channels else {
        "clip_lo": 0.0,
        "clip_hi": 1.0,
        "clip_source": "identity",
    }
    return {
        "score_output_normalize": bool(legacy_normalize),
        "score_output_clip_lo": float(primary.get("clip_lo", 0.0)),
        "score_output_clip_hi": float(primary.get("clip_hi", 1.0)),
        "score_output_clip_source": str(primary.get("clip_source", "identity")),
        "score_output_n_solves": int(data.get("n_solves") or 0),
        "score_output_summary_threads": int(data.get("threads", 0) or 0),
        "score_output_channel_count": channel_count,
        "score_output_has_explicit_outputs": has_explicit,
        # Interpretation is owned by render-plan/color consumers, not summary.
        "score_output_channels": channels,
        "fragment_pair_encoding": fragment_encoding,
        "fragment_encoding": fragment_encoding,
        "fragment_record_size_bytes": 4 + channel_count,
    }


def _clip_program_output_range(
    compiled,
    metrics_with_clips,
    degree,
    n_coeffs,
    solve_score_threads,
    solve_score_normalize,
    root_transforms,
    uses_coeff_source,
    uses_param_source,
):
    cmd = [
        BINARY,
        _TMP_INPUT,
        "--mode=summary",
        f"--degree={degree}",
        f"--threads={solve_score_threads}",
        *_build_program_cmd_args(compiled, metrics_with_clips),
    ]
    if uses_coeff_source:
        cmd.extend([
            f"--score_coeffs_file={_TMP_COEFF_INPUT}",
            f"--score_coeff_degree={n_coeffs}",
        ])
    if uses_param_source:
        cmd.append(f"--score_params_file={_TMP_PARAM_INPUT}")
    xf_path = _write_xforms(root_transforms)
    if xf_path:
        cmd.append(f"--root_xforms={xf_path}")

    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        raise RuntimeError(f"solve_proximity_stats score output summary failed: {result.stderr.strip()}")
    data = json.loads(result.stdout)
    output_clip = _score_output_contract(
        compiled,
        data,
        legacy_normalize=bool(solve_score_normalize),
    )
    output_clip["score_output_summary_threads"] = int(data.get("threads", solve_score_threads))
    return output_clip


def _legacy_metric_clips(metric, quantile, clip_lo, clip_hi):
    return [_sanitize_metric_clip({
        "slot": 0,
        "metric": metric,
        "quantile": float(quantile),
        "quantile_pct": float(quantile) * 100.0,
        "clip_lo": float(clip_lo),
        "clip_hi": float(clip_hi),
    })]


def _clip_artifact_metrics(clip_data, compiled, fallback_quantile):
    metrics = clip_data.get("metrics")
    if isinstance(metrics, list) and metrics:
        return _clone_metric_clips(metrics)
    return _legacy_metric_clips(
        clip_data.get("metric", compiled["metric"]),
        fallback_quantile,
        clip_data.get("clip_lo"),
        clip_data.get("clip_hi"),
    )


def _compile_request_chain(params, metric, *, default_metric="proximity"):
    return compile_solve_score_chain_or_legacy(
        params.get("solve_score_chain", ""),
        metric,
        params.get("solve_score_quantile", 0.001),
        params.get("solve_score_omega", 1.0),
        params.get("solve_score_omega_enabled", True),
        default_metric=default_metric,
    )


def _validate_artifact_chain_fingerprint(data, compiled, label):
    actual = str((data or {}).get("chain_fingerprint") or "").strip()
    if not actual:
        raise RuntimeError(f"{label} missing chain_fingerprint")
    expected = compiled_solve_score_fingerprint(compiled)
    if actual != expected:
        raise RuntimeError(f"{label} fingerprint mismatch: expected {expected}, got {actual}")


def _validate_clip_artifact(clip_data, compiled):
    if clip_data.get("family") != "solve_score":
        raise RuntimeError(f"Clip artifact missing or wrong family: {clip_data.get('family')}")
    if clip_data.get("version", 1) >= 2:
        _validate_artifact_chain_fingerprint(clip_data, compiled, "Clip")
        clip_program = str(clip_data.get("program") or "")
        if not clip_program and compiled.get("uses_lag"):
            raise RuntimeError("Clip artifact missing program for lagged solve-score chain")
        if clip_program and not solve_score_program_specs_match(
            clip_program,
            compiled["program_spec"],
            version=clip_data.get("solve_score_spec_version"),
        ):
            raise RuntimeError(f"Clip program mismatch: expected {compiled['program_spec']}, got {clip_program!r}")
        clip_metrics = _clip_artifact_metrics(clip_data, compiled, compiled["quantile"])
        if len(clip_metrics) != compiled["metric_count"]:
            raise RuntimeError(
                f"Clip metric slot count mismatch: expected {compiled['metric_count']}, got {len(clip_metrics)}"
            )
        for expected, actual in zip(compiled["metrics"], clip_metrics):
            if actual.get("metric") != expected["metric"]:
                raise RuntimeError(
                    f"Clip metric slot {expected['slot']} mismatch: expected {expected['metric']}, got {actual.get('metric')}"
                )
            if actual.get("source", "slv") != expected.get("source", "slv"):
                raise RuntimeError(
                    f"Clip source slot {expected['slot']} mismatch: expected {expected.get('source', 'slv')}, "
                    f"got {actual.get('source', 'slv')}"
                )
            if float(actual.get("quantile", -1)) != float(expected["quantile"]):
                raise RuntimeError(
                    f"Clip quantile slot {expected['slot']} mismatch: expected {expected['quantile']}, got {actual.get('quantile')}"
                )
        return clip_metrics

    if clip_data.get("metric") != compiled["metric"]:
        raise RuntimeError(f"Clip metric mismatch: expected {compiled['metric']}, got {clip_data.get('metric')}")
    if float(clip_data.get("clip_quantile", -1)) != float(compiled["quantile"]):
        raise RuntimeError(f"Clip quantile mismatch: expected {compiled['quantile']}, got {clip_data.get('clip_quantile')}")
    if float(clip_data.get("omega", 1.0)) != compiled["omega"]:
        raise RuntimeError(f"Clip omega mismatch: expected {compiled['omega']}, got {clip_data.get('omega')}")
    if _validate_omega_enabled(clip_data.get("omega_enabled", True)) != compiled["omega_enabled"]:
        raise RuntimeError(
            f"Clip omega_enabled mismatch: expected {compiled['omega_enabled']}, got {clip_data.get('omega_enabled')}"
        )
    return _legacy_metric_clips(compiled["metric"], compiled["quantile"], clip_data.get("clip_lo"), clip_data.get("clip_hi"))


def _build_program_cmd_args(compiled, metrics_with_clips):
    hydrated = []
    for slot, metric in enumerate(metrics_with_clips):
        row = dict(metric)
        row["slot"] = slot
        hydrated.append(row)
    payload = solve_score_program_cli_payload({"metrics": hydrated, "program_spec": compiled["program_spec"]})
    return [
        f"--score_metrics={payload['score_metrics']}",
        f"--score_clip_los={payload['score_clip_los']}",
        f"--score_clip_his={payload['score_clip_his']}",
        f"--score_program={payload['score_program']}",
    ] + ([f"--score_sources={payload['score_sources']}"] if payload.get("score_sources") else [])


def _json_response(status_code, body):
    return {
        "statusCode": int(status_code),
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(body),
    }


def _solve_score_error_fields(compiled):
    return {
        "metric": compiled["metric"],
        "metric_count": compiled["metric_count"],
        "chain_fingerprint": compiled_solve_score_fingerprint(compiled),
        "solve_score_spec_version": SOLVE_SCORE_SPEC_VERSION,
        "program": compiled["program_spec"],
        "program_id": compiled["program_id"],
        "score_metrics": [row["metric"] for row in compiled["metrics"]],
        "score_sources": [row.get("source", "slv") for row in compiled["metrics"]],
        "score_quantiles": [row["quantile"] for row in compiled["metrics"]],
        "solve_score_chain": json.loads(serialize_solve_score_chain(compiled["chain"])),
        "solve_score_display": compiled["display"],
        "clip_quantile": compiled["quantile"],
        "omega": compiled["omega"],
        "omega_enabled": compiled["omega_enabled"],
    }


def _score_program_error_suffix(compiled, metrics_with_clips=None):
    metric_names = [row["metric"] for row in (metrics_with_clips or compiled.get("metrics") or [])]
    metric_sources = [row.get("source", "slv") for row in (metrics_with_clips or compiled.get("metrics") or [])]
    parts = [
        f"program={compiled['program_spec']}",
        f"score_metrics={','.join(metric_names)}" if metric_names else "",
        f"score_sources={','.join(metric_sources)}" if metric_sources else "",
        f"display={compiled['display']}",
    ]
    parts = [part for part in parts if part]
    return f", {'; '.join(parts)}" if parts else ""


def _fallback_lores_coeffs_key(job_id, lores_bin_key):
    key = str(lores_bin_key or "").strip()
    if key.endswith("/lores.bin"):
        return key[:-len("/lores.bin")] + "/lores_coeffs.bin"
    job = str(job_id or "").strip()
    if job:
        return f"renders/{job}/lores_coeffs.bin"
    return ""


def _fallback_lores_params_key(job_id, lores_bin_key):
    key = str(lores_bin_key or "").strip()
    if key.endswith("/lores.bin"):
        return key[:-len("/lores.bin")] + "/lores_params.bin"
    job = str(job_id or "").strip()
    if job:
        return f"renders/{job}/lores_params.bin"
    return ""


def _summary_error_response(params, exc):
    payload = {
        "error": "solve histogram summary failed",
        "detail": str(exc),
        "phase": "summary",
        "job_id": params.get("job_id"),
        "degree": params.get("degree"),
        "lores_bin_key": params.get("lores_bin_key"),
        "lores_coeffs_key": params.get("lores_coeffs_key"),
        "lores_params_key": params.get("lores_params_key"),
    }
    try:
        compiled = _compile_request_chain(params, params.get("metric", "proximity"), default_metric="proximity")
        payload.update(_solve_score_error_fields(compiled))
    except Exception as compile_exc:
        payload["compile_error"] = str(compile_exc)
        raw_chain = params.get("solve_score_chain")
        if raw_chain not in ("", None, []):
            payload["solve_score_chain"] = raw_chain
    return _json_response(500, payload)


def handler(event, context):
    params = parse_body(event)
    phase = params["phase"]
    try:
        if phase == "clip":
            return handle_clip(params)
        elif phase == "hist":
            return handle_hist(params)
        elif phase == "merge":
            return handle_merge(params)
        elif phase == "summary":
            return handle_summary(params)
        else:
            raise RuntimeError(f"Unknown phase: {phase}")
    except Exception as exc:
        if phase == "summary":
            return _summary_error_response(params, exc)
        raise


def _validate_quantile(q):
    """Validate solve_score_quantile is in [0.001, 0.05]."""
    try:
        q = float(q)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_quantile must be numeric, got {q!r}")
    if not (0.001 <= q <= 0.05):
        raise RuntimeError(f"solve_score_quantile must be in [0.001, 0.05], got {q}")
    return q


def _validate_omega(value):
    try:
        omega = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega must be numeric, got {value!r}")
    if not (omega == omega and abs(omega) != float("inf")):
        raise RuntimeError(f"solve_score_omega must be finite, got {value!r}")
    return omega


def _validate_omega_enabled(value):
    if value in (None, ""):
        return True
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    text = str(value).strip().lower()
    if text in ("1", "true", "yes", "on"):
        return True
    if text in ("0", "false", "no", "off"):
        return False
    raise RuntimeError(f"solve_score_omega_enabled must be boolean-like, got {value!r}")


def _validate_threads(value, default=1):
    if value in (None, ""):
        return default
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"solve_score_threads must be in [1, 16], got {threads}")
    return threads


def _validate_merge_workers(value, default=None):
    if value in (None, ""):
        value = os.environ.get("SOLVE_SCORE_MERGE_WORKERS", default if default is not None else 16)
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_merge_workers must be an integer, got {value!r}")
    if not (1 <= workers <= 64):
        raise RuntimeError(f"solve_score_merge_workers must be in [1, 64], got {workers}")
    return workers


def _validate_hist_input_mode(value):
    mode = str(value or "tmpfile").strip().lower()
    if mode not in VALID_HIST_INPUT_MODES:
        raise RuntimeError(f"solve_score_hist_input_mode must be one of {', '.join(sorted(VALID_HIST_INPUT_MODES))}, got {value!r}")
    return mode


def _validate_sectioned_retries(value, field_name):
    if value in (None, ""):
        value = 2
    try:
        retries = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if not (0 <= retries <= 10):
        raise RuntimeError(f"{field_name} must be in [0, 10], got {retries}")
    return retries


def _sectioned_input_size_limit():
    try:
        memory_mb = int(os.environ.get("AWS_LAMBDA_FUNCTION_MEMORY_SIZE", "0") or 0)
    except (TypeError, ValueError):
        memory_mb = 0
    if memory_mb <= 0:
        return 0
    return (memory_mb * 1024 * 1024) // 2


def _run_binary_with_streamed_input(cmd, obj, input_size, timeout=120):
    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    body = obj["Body"]
    try:
        t_stream = time.time()
        try:
            if proc.stdin is None:
                raise RuntimeError("stdin pipe unavailable")
            for chunk in body.iter_chunks(chunk_size=1024 * 1024):
                if not chunk:
                    continue
                proc.stdin.write(chunk)
        except BrokenPipeError:
            pass
        finally:
            if proc.stdin is not None:
                proc.stdin.close()
        stream_ms = int((time.time() - t_stream) * 1000)
        t_wait = time.time()
        stdout = proc.stdout.read() if proc.stdout is not None else b""
        stderr = proc.stderr.read() if proc.stderr is not None else b""
        rc = proc.wait(timeout=timeout)
        wait_ms = int((time.time() - t_wait) * 1000)
        return rc, stdout.decode("utf-8", errors="replace"), stderr.decode("utf-8", errors="replace"), stream_ms, wait_ms
    except Exception:
        proc.kill()
        proc.wait()
        raise
    finally:
        try:
            body.close()
        except Exception:
            pass


def _merge_s3_client(max_workers):
    pool_size = max(10, int(max_workers) + 1)
    return boto3.client("s3", config=Config(max_pool_connections=pool_size))


def _load_json_body(obj):
    body = obj["Body"]
    try:
        data = body.read()
    finally:
        try:
            body.close()
        except Exception:
            pass
    return json.loads(data)


def _load_merge_histogram_artifact(client, hist_prefix, section_idx, compiled, hist_bins):
    key = f"{hist_prefix}section_{section_idx}_hist.json"
    try:
        obj = client.get_object(Bucket=BUCKET, Key=key)
        data = _load_json_body(obj)
    except client.exceptions.NoSuchKey:
        legacy_key = f"{hist_prefix}chunk_{section_idx}_hist.json"
        try:
            obj = client.get_object(Bucket=BUCKET, Key=legacy_key)
            data = _load_json_body(obj)
            key = legacy_key
        except client.exceptions.NoSuchKey:
            raise RuntimeError(f"Missing histogram: {key}")

    if data.get("family") == "solve_score" and data.get("version", 1) >= 2:
        _validate_artifact_chain_fingerprint(data, compiled, f"Section {section_idx}")
        if not solve_score_program_specs_match(
            data.get("program") or "",
            compiled["program_spec"],
            version=data.get("solve_score_spec_version"),
        ):
            raise RuntimeError(
                f"Section {section_idx} program mismatch: expected {compiled['program_spec']}, got {data.get('program')!r}"
            )
        hist_metrics = _clip_artifact_metrics(data, compiled, compiled["quantile"])
        if len(hist_metrics) != compiled["metric_count"]:
            raise RuntimeError(
                f"Section {section_idx} metric slot count mismatch: expected {compiled['metric_count']}, got {len(hist_metrics)}"
            )
    else:
        if data.get("family") == "solve_score" and data.get("metric") != compiled["metric"]:
            raise RuntimeError(f"Section {section_idx} metric mismatch: expected {compiled['metric']}, got {data.get('metric')}")
        if data.get("family") == "solve_score" and data.get("clip_quantile") != compiled["quantile"]:
            raise RuntimeError(f"Section {section_idx} quantile mismatch: expected {compiled['quantile']}, got {data.get('clip_quantile')}")
        if data.get("family") == "solve_score" and float(data.get("omega", 1.0)) != compiled["omega"]:
            raise RuntimeError(f"Section {section_idx} omega mismatch: expected {compiled['omega']}, got {data.get('omega')}")
        if data.get("family") == "solve_score" and _validate_omega_enabled(data.get("omega_enabled", True)) != compiled["omega_enabled"]:
            raise RuntimeError(f"Section {section_idx} omega_enabled mismatch: expected {compiled['omega_enabled']}, got {data.get('omega_enabled')}")

    chunk_hist = data["hist"]
    if len(chunk_hist) != hist_bins:
        raise RuntimeError(f"Section {section_idx} histogram has {len(chunk_hist)} bins, expected {hist_bins}")

    return {
        "section_idx": section_idx,
        "hist": chunk_hist,
        "n_solves": data["n_solves"],
        "key": key,
    }


def handle_clip(params):
    contract_warnings = []
    job_id = params["job_id"]
    task_id = params["task_id"]
    degree = params["degree"]
    metric = contract_param(params, "metric", "proximity", contract_warnings)
    _validate_metric(metric)
    solve_score_threads = _validate_threads(contract_param(params, "solve_score_threads", 1, contract_warnings), default=1)
    lores_bin_key = params["lores_bin_key"]
    lores_coeffs_key = str(params.get("lores_coeffs_key", "") or "").strip()
    lores_params_key = str(params.get("lores_params_key", "") or "").strip()
    n_coeffs = params.get("n_coeffs")
    root_transforms = contract_param(params, "root_transforms", [], contract_warnings)
    out_key = params["out_key"]
    compiled = _compile_request_chain(params, metric, default_metric="proximity")
    solve_score_normalize = parse_boolish(
        contract_param(params, "solve_score_normalize", False, contract_warnings),
        False,
        strict=True,
        label="solve_score_normalize",
    )
    metric = compiled["metric"]
    solve_score_omega = compiled["omega"]
    solve_score_omega_enabled = compiled["omega_enabled"]
    uses_coeff_source = solve_score_uses_source(compiled, "cf")
    uses_param_source = solve_score_uses_source(compiled, "pm")
    if uses_coeff_source:
        lores_coeffs_key = lores_coeffs_key or _fallback_lores_coeffs_key(job_id, lores_bin_key)
        if not lores_coeffs_key:
            raise RuntimeError("mixed-source solve score clip requires lores_coeffs_key")
        try:
            n_coeffs = int(n_coeffs)
        except (TypeError, ValueError):
            raise RuntimeError(f"mixed-source solve score clip requires numeric n_coeffs, got {n_coeffs!r}")
        if n_coeffs < 1:
            raise RuntimeError(f"mixed-source solve score clip requires n_coeffs >= 1, got {n_coeffs}")
    if uses_param_source:
        lores_params_key = lores_params_key or _fallback_lores_params_key(job_id, lores_bin_key)
        if not lores_params_key:
            raise RuntimeError("param-source solve score clip requires lores_params_key")
    progress = attach_contract_warnings(
        {
            "phase": "clip",
            "metric": metric,
            "metric_count": compiled["metric_count"],
            "source_key": lores_bin_key,
            "source_coeffs_key": lores_coeffs_key,
            "source_params_key": lores_params_key,
            "omega": solve_score_omega,
            "omega_enabled": solve_score_omega_enabled,
            "threads": solve_score_threads,
            "score_output_normalize": solve_score_normalize,
        },
        contract_warnings,
    )

    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        t0 = time.time()
        size = _download(lores_bin_key, _TMP_INPUT)
        coeff_size = 0
        param_size = 0
        if uses_coeff_source:
            coeff_size = _download(lores_coeffs_key, _TMP_COEFF_INPUT)
        if uses_param_source:
            param_size = _download(lores_params_key, _TMP_PARAM_INPUT)
        dl_ms = int((time.time() - t0) * 1000)
        progress["dl_ms"] = dl_ms
        progress["source_size"] = size
        if uses_coeff_source:
            progress["source_coeffs_size"] = coeff_size
        if uses_param_source:
            progress["source_params_size"] = param_size

        report_status(job_id, task_id, "bin_downloaded", result_data=progress)

        t1 = time.time()
        metric_clips = []
        clip_threads = solve_score_threads
        for slot, metric_row in enumerate(compiled["metrics"]):
            source = metric_row.get("source", "slv")
            slot_clip = _clip_metric_slot(
                metric_row["metric"],
                metric_row["quantile"],
                degree if source == "slv" else (n_coeffs if source == "cf" else 2),
                solve_score_omega,
                solve_score_omega_enabled,
                solve_score_threads,
                root_transforms if source == "slv" else None,
                _TMP_INPUT if source == "slv" else (_TMP_COEFF_INPUT if source == "cf" else _TMP_PARAM_INPUT),
            )
            slot_clip["slot"] = slot
            slot_clip["source"] = source
            metric_clips.append(slot_clip)
            clip_threads = max(clip_threads, int(slot_clip.get("threads", solve_score_threads)))
        score_output_clip = _score_output_contract(
            compiled,
            {
                "n_solves": metric_clips[0]["n_solves"] if metric_clips else 0,
                "threads": solve_score_threads,
            },
            legacy_normalize=solve_score_normalize,
        )
        if solve_score_normalize or compiled.get("has_explicit_outputs"):
            score_output_clip.update(
                _clip_program_output_range(
                    compiled,
                    metric_clips,
                    degree,
                    n_coeffs,
                    solve_score_threads,
                    solve_score_normalize,
                    root_transforms,
                    uses_coeff_source,
                    uses_param_source,
                )
            )
        compute_ms = int((time.time() - t1) * 1000)
        progress["compute_ms"] = compute_ms
        progress["threads"] = clip_threads
        progress["n_solves"] = metric_clips[0]["n_solves"] if metric_clips else 0
        progress["clip_lo"] = metric_clips[0]["clip_lo"] if metric_clips else 0.0
        progress["clip_hi"] = metric_clips[0]["clip_hi"] if metric_clips else 1.0
        progress["metrics"] = [
            {
                "metric": row["metric"],
                "q": row["quantile"],
                "clip_lo": row["clip_lo"],
                "clip_hi": row["clip_hi"],
            }
            for row in metric_clips
        ]
        progress["clip_slots"] = [
            {
                "slot": int(row["slot"]),
                "metric": str(row["metric"]),
                "source": str(row.get("source", "slv") or "slv"),
                "clip_lo": float(row["clip_lo"]),
                "clip_hi": float(row["clip_hi"]),
            }
            for row in metric_clips
        ]
        progress["score_program"] = compiled["program_spec"]
        progress["chain_fingerprint"] = compiled_solve_score_fingerprint(compiled)
        progress["solve_score_spec_version"] = SOLVE_SCORE_SPEC_VERSION
        progress.update(score_output_clip)

        report_status(job_id, task_id, "computed", result_data=progress)

        artifact = {
            "family": "solve_score",
            "version": 2,
            "job_id": job_id,
            "metric": metric,
            "clip_quantile": compiled["quantile"],
            "omega": solve_score_omega,
            "omega_enabled": solve_score_omega_enabled,
            "chain_fingerprint": compiled_solve_score_fingerprint(compiled),
            "solve_score_spec_version": SOLVE_SCORE_SPEC_VERSION,
            "clip_lo": metric_clips[0]["clip_lo"],
            "clip_hi": metric_clips[0]["clip_hi"],
            "n_solves": metric_clips[0]["n_solves"],
            "degree": degree,
            "lores_bin_key": lores_bin_key,
            "root_transforms": root_transforms or [],
            "chain": json.loads(serialize_solve_score_chain(compiled["chain"])),
            "program": compiled["program_spec"],
            "metrics": metric_clips,
            "metric_count": len(metric_clips),
            **score_output_clip,
        }
        if uses_coeff_source:
            artifact["lores_coeffs_key"] = lores_coeffs_key
            artifact["n_coeffs"] = n_coeffs
        if uses_param_source:
            artifact["lores_params_key"] = lores_params_key

        s3.put_object(Bucket=BUCKET, Key=out_key,
                      Body=json.dumps(artifact),
                      ContentType="application/json")

        progress["out_key"] = out_key
        report_status(job_id, task_id, "done", result_data=progress)
        return ok_response(progress)

    except Exception as e:
        progress["error"] = str(e)
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()


def handle_hist(params):
    contract_warnings = []
    job_id = params["job_id"]
    task_id = params["task_id"]
    section_idx = params.get("section_idx", params.get("chunk_idx", params.get("stripe_idx")))
    if section_idx is None:
        raise RuntimeError("hist requires section_idx (or legacy chunk_idx)")
    section_count = params.get("section_count")
    metric = contract_param(params, "metric", "proximity", contract_warnings)
    _validate_metric(metric)
    solve_score_threads = _validate_threads(contract_param(params, "solve_score_threads", 1, contract_warnings), default=1)
    solve_score_hist_input_mode = _validate_hist_input_mode(contract_param(params, "solve_score_hist_input_mode", "tmpfile", contract_warnings))
    solve_score_hist_retries = _validate_sectioned_retries(
        contract_param(params, "solve_score_hist_retries", 2, contract_warnings),
        "solve_score_hist_retries",
    )
    bin_key = str(params.get("bin_key") or "").strip()
    coeffs_key = str(params.get("coeffs_key", "") or "").strip()
    coeffs_bin_size = params.get("coeffs_bin_size")
    params_key = str(params.get("params_key", "") or "").strip()
    n_coeffs = params.get("n_coeffs")
    step_start = params.get("step_start")
    step_count = params.get("step_count")
    params_step_start = params.get("params_step_start", step_start)
    params_step_count = params.get("params_step_count", step_count)
    root_spans = list(params.get("root_spans") or [])
    coeff_spans = list(params.get("coeff_spans") or [])
    param_spans = list(params.get("param_spans") or [])
    logical_section = parse_boolish(params.get("logical_section"), bool(root_spans))
    solve_source_manifest = resolve_solve_source_manifest(
        params,
        s3,
        BUCKET,
        required_context="solve score hist",
    )
    degree = params["degree"]
    clip_key = params["clip_key"]
    hist_bins = params.get("hist_bins", 100)
    root_transforms = contract_param(params, "root_transforms", [], contract_warnings)
    out_key = params["out_key"]
    compiled = _compile_request_chain(params, metric, default_metric="proximity")
    metric = compiled["metric"]
    solve_score_omega = compiled["omega"]
    solve_score_omega_enabled = compiled["omega_enabled"]
    uses_coeff_source = solve_score_uses_source(compiled, "cf")
    uses_param_source = solve_score_uses_source(compiled, "pm")
    uses_lag = bool(compiled.get("uses_lag"))
    prelude_by_source = solve_score_lag_prelude_by_source(compiled)
    requested_solve_prelude = int(params.get("prelude_rows") or 0)
    requested_coeff_prelude = int(params.get("score_coeff_prelude_rows") or 0)
    requested_param_prelude = int(params.get("score_param_prelude_rows") or 0)
    if uses_lag:
        if not logical_section:
            raise RuntimeError("lagged solve-score hist requires logical_section=true")
        expected = (
            int(prelude_by_source.get("slv", 0)),
            int(prelude_by_source.get("cf", 0)),
            int(prelude_by_source.get("pm", 0)),
        )
        got = (requested_solve_prelude, requested_coeff_prelude, requested_param_prelude)
        if got != expected:
            raise RuntimeError(
                "lagged solve-score hist prelude contract mismatch: "
                f"expected slv/cf/pm={expected}, got {got}"
            )
    elif requested_solve_prelude or requested_coeff_prelude or requested_param_prelude:
        raise RuntimeError("prelude rows require a lagged solve-score program")
    if uses_coeff_source:
        coeffs_key = coeffs_key or f"renders/{job_id}/coeffs_{int(section_idx):04d}.bin"
        try:
            n_coeffs = int(n_coeffs)
        except (TypeError, ValueError):
            raise RuntimeError(f"mixed-source solve score hist requires numeric n_coeffs, got {n_coeffs!r}")
        if n_coeffs < 1:
            raise RuntimeError(f"mixed-source solve score hist requires n_coeffs >= 1, got {n_coeffs}")
    actual_solve_prelude = 0
    actual_coeff_prelude = 0
    actual_param_prelude = 0
    if logical_section:
        try:
            step_start = int(step_start)
            step_count = int(step_count)
            degree = int(degree)
        except (TypeError, ValueError):
            raise RuntimeError(
                "logical solve score hist requires numeric step_start/step_count/degree, "
                f"got {step_start!r}/{step_count!r}/{degree!r}"
            )
        if step_count < 1:
            raise RuntimeError(f"logical solve score hist requires step_count >= 1, got {step_count}")
        if not solve_source_manifest:
            raise RuntimeError("logical solve score hist requires solve_source_manifest")
        actual_solve_prelude = 1 if requested_solve_prelude and step_start > 0 else 0
        actual_coeff_prelude = 1 if requested_coeff_prelude and step_start > 0 else 0
        actual_param_prelude = 1 if requested_param_prelude and step_start > 0 else 0
    if logical_section and not root_spans:
        root_spans = build_source_spans(
            solve_source_manifest,
            source_family="slv",
            solve_start=step_start - actual_solve_prelude,
            solve_count=step_count + actual_solve_prelude,
        )
        coeff_spans = build_source_spans(
            solve_source_manifest,
            source_family="cf",
            solve_start=step_start - actual_coeff_prelude,
            solve_count=step_count + actual_coeff_prelude,
        ) if uses_coeff_source else []
        param_spans = build_source_spans(
            solve_source_manifest,
            source_family="pm",
            solve_start=step_start - actual_param_prelude,
            solve_count=step_count + actual_param_prelude,
        ) if uses_param_source else []
        if root_spans and not bin_key:
            bin_key = str(root_spans[0]["key"])
        if coeff_spans and not coeffs_key:
            coeffs_key = str(coeff_spans[0]["key"])
        if param_spans and not params_key:
            params_key = str(param_spans[0]["key"])
    if uses_param_source:
        params_key = str(params_key).strip()
        if not params_key:
            raise RuntimeError("param-source solve score hist requires params_key")
        try:
            params_step_start = int(params_step_start)
            params_step_count = int(params_step_count)
            step_count = int(step_count)
        except (TypeError, ValueError):
            raise RuntimeError(
                "param-source solve score hist requires numeric "
                f"params_step_start/params_step_count/step_count, got {params_step_start!r}/{params_step_count!r}/{step_count!r}"
            )
        if params_step_start < 0 or params_step_count < 1 or step_count < 1:
            raise RuntimeError(
                "param-source solve score hist requires params_step_start >= 0 and "
                f"params_step_count/step_count >= 1, got {params_step_start}/{params_step_count}/{step_count}"
            )
        if params_step_count != step_count:
            raise RuntimeError(
                f"param-source solve score hist requires params_step_count == step_count, got {params_step_count}/{step_count}"
            )
    progress = attach_contract_warnings({
        "phase": "hist",
        "metric": metric,
        "metric_count": compiled["metric_count"],
        "section_idx": section_idx,
        "section_count": section_count,
        "omega": solve_score_omega,
        "omega_enabled": solve_score_omega_enabled,
        "threads": solve_score_threads,
        "requested_input_mode": solve_score_hist_input_mode,
        "input_mode": solve_score_hist_input_mode,
        "logical_section": logical_section,
        "retries": solve_score_hist_retries,
        "source_bucket": BUCKET,
        "source_key": bin_key,
        "source_coeffs_key": coeffs_key,
        "source_params_key": params_key,
        "source_params_step_start": params_step_start if uses_param_source else "",
        "source_params_step_count": params_step_count if uses_param_source else "",
        "prelude_rows": actual_solve_prelude,
        "score_coeff_prelude_rows": actual_coeff_prelude,
        "score_param_prelude_rows": actual_param_prelude,
        "clip_key": clip_key,
    }, contract_warnings)
    progress.update(_solve_score_error_fields(compiled))

    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        t0 = time.time()
        clip_obj = s3.get_object(Bucket=BUCKET, Key=clip_key)
        clip_data = json.loads(clip_obj["Body"].read())
        metrics_with_clips = _validate_clip_artifact(clip_data, compiled)
        primary_metric_clip = metrics_with_clips[0] if metrics_with_clips else None
        program_args = _build_program_cmd_args(compiled, metrics_with_clips) if clip_data.get("version", 1) >= 2 else []
        program_suffix = _score_program_error_suffix(compiled, metrics_with_clips)

        xf_path = _write_xforms(root_transforms)
        hist_stdout = None
        hist_stderr = None
        hist_rc = 0
        effective_input_mode = solve_score_hist_input_mode
        if logical_section:
            if solve_score_hist_input_mode != "sectioned":
                raise RuntimeError(
                    "logical solve histogram sections reject "
                    f"solve_score_hist_input_mode={solve_score_hist_input_mode!r}; "
                    "use solve_score_hist_input_mode=sectioned"
                )
            root_urls = build_native_manifest_urls(s3, BUCKET, root_spans)
            input_manifest = build_native_multispan_manifest(
                solve_source_manifest,
                source_family="slv",
                solve_start=int(step_start) - actual_solve_prelude,
                solve_count=int(step_count) + actual_solve_prelude,
                url_by_key=root_urls,
            )
            input_manifest_path = write_native_multispan_manifest(_TMP_INPUT_MANIFEST, input_manifest)
            progress["source_size"] = int(input_manifest["logical_size"])
            cmd = [
                SECTIONED_HIST_BINARY,
                "--input_mode=multispan_sectioned",
                f"--input_manifest={input_manifest_path}",
                f"--degree={degree}",
                f"--hist_bins={hist_bins}",
                f"--threads={solve_score_threads}",
                f"--retries={solve_score_hist_retries}",
                f"--step_count={int(step_count)}",
                f"--prelude_rows={actual_solve_prelude}",
                f"--score_coeff_prelude_rows={actual_coeff_prelude}",
                f"--score_param_prelude_rows={actual_param_prelude}",
            ]
            if program_args:
                cmd.extend(program_args)
                if uses_coeff_source:
                    coeff_manifest = build_native_multispan_manifest(
                        solve_source_manifest,
                        source_family="cf",
                        solve_start=int(step_start) - actual_coeff_prelude,
                        solve_count=int(step_count) + actual_coeff_prelude,
                        url_by_key=build_native_manifest_urls(s3, BUCKET, coeff_spans),
                    )
                    coeff_manifest_path = write_native_multispan_manifest(
                        _TMP_COEFF_INPUT_MANIFEST,
                        coeff_manifest,
                    )
                    progress["source_coeffs_size"] = int(coeff_manifest["logical_size"])
                    cmd.extend([
                        f"--score_coeff_manifest={coeff_manifest_path}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                if uses_param_source:
                    param_manifest = build_native_multispan_manifest(
                        solve_source_manifest,
                        source_family="pm",
                        solve_start=int(step_start) - actual_param_prelude,
                        solve_count=int(step_count) + actual_param_prelude,
                        url_by_key=build_native_manifest_urls(s3, BUCKET, param_spans),
                    )
                    param_manifest_path = write_native_multispan_manifest(
                        _TMP_PARAM_INPUT_MANIFEST,
                        param_manifest,
                    )
                    progress["source_params_size"] = int(param_manifest["logical_size"])
                    cmd.append(f"--score_params_manifest={param_manifest_path}")
            else:
                cmd.extend([
                    f"--metric={metric}",
                    f"--clip_lo={primary_metric_clip['clip_lo']}",
                    f"--clip_hi={primary_metric_clip['clip_hi']}",
                    f"--omega={solve_score_omega}",
                    f"--omega_enabled={1 if solve_score_omega_enabled else 0}",
                ])
            if xf_path:
                cmd.append(f"--root_xforms={xf_path}")
            effective_input_mode = "multispan_sectioned"
            progress["input_mode"] = effective_input_mode
            pre_native_ms = int((time.time() - t0) * 1000)
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
            hist_rc = result.returncode
            hist_stdout = result.stdout
            hist_stderr = result.stderr
            if hist_rc != 0:
                stderr_summary = (hist_stderr or "").strip() or "unknown error"
                raise RuntimeError(
                    "solve_proximity_hist_sectioned failed for logical section "
                    f"(clip=s3://{BUCKET}/{clip_key}, job={job_id}, task={task_id}, "
                    f"section={section_idx}, input={effective_input_mode}, size={progress['source_size']}, "
                    f"threads={solve_score_threads}, retries={solve_score_hist_retries}, metric={metric}{program_suffix}): {stderr_summary}"
                )
            hist_data = json.loads(hist_stdout)
            progress["dl_ms"] = pre_native_ms + int(hist_data.get("download_ms", 0))
            progress["compute_ms"] = int(hist_data.get("compute_ms", 0))
            progress["wall_ms"] = int(hist_data.get("wall_ms", 0))
            report_status(job_id, task_id, "bin_downloaded", result_data=progress)
        elif solve_score_hist_input_mode == "sectioned":
            input_size = int(params.get("bin_size") or 0)
            if input_size <= 0:
                head = s3.head_object(Bucket=BUCKET, Key=bin_key)
                input_size = int(head.get("ContentLength") or 0)
            if input_size <= 0:
                raise RuntimeError(f"Failed to determine size for s3://{BUCKET}/{bin_key}")
            size_limit = _sectioned_input_size_limit()
            if size_limit > 0 and input_size > size_limit:
                raise RuntimeError(
                    f"sectioned hist input too large for current Lambda memory: "
                    f"{input_size} bytes > safe limit {size_limit} bytes"
                )
            progress["source_size"] = input_size
            coeff_input_size = 0
            coeff_presigned_url = None
            param_size = 0
            if uses_coeff_source:
                try:
                    coeff_input_size = int(coeffs_bin_size)
                except (TypeError, ValueError):
                    coeff_input_size = 0
                if coeff_input_size <= 0:
                    coeff_head = s3.head_object(Bucket=BUCKET, Key=coeffs_key)
                    coeff_input_size = int(coeff_head.get("ContentLength") or 0)
                if coeff_input_size <= 0:
                    raise RuntimeError(f"Failed to determine size for s3://{BUCKET}/{coeffs_key}")
                coeff_presigned_url = s3.generate_presigned_url(
                    "get_object",
                    Params={"Bucket": BUCKET, "Key": coeffs_key},
                    ExpiresIn=900,
                )
                progress["source_coeffs_size"] = coeff_input_size
            if uses_param_source:
                param_size = _download_range(
                    params_key,
                    _TMP_PARAM_INPUT,
                    int(params_step_start) * 4 * 4,
                    int(params_step_count) * 4 * 4,
                )
                progress["source_params_size"] = param_size
            presigned_url = s3.generate_presigned_url(
                "get_object",
                Params={"Bucket": BUCKET, "Key": bin_key},
                ExpiresIn=900,
            )
            cmd = [
                SECTIONED_HIST_BINARY,
                "--input_mode=sectioned",
                f"--url={presigned_url}",
                f"--input_size={input_size}",
                f"--degree={degree}",
                f"--hist_bins={hist_bins}",
                f"--threads={solve_score_threads}",
                f"--retries={solve_score_hist_retries}",
            ]
            if program_args:
                cmd.extend(program_args)
                if uses_coeff_source:
                    cmd.extend([
                        f"--score_coeffs_url={coeff_presigned_url}",
                        f"--score_coeff_input_size={coeff_input_size}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                if uses_param_source:
                    cmd.append(f"--score_params_file={_TMP_PARAM_INPUT}")
            else:
                cmd.extend([
                    f"--metric={metric}",
                    f"--clip_lo={primary_metric_clip['clip_lo']}",
                    f"--clip_hi={primary_metric_clip['clip_hi']}",
                    f"--omega={solve_score_omega}",
                    f"--omega_enabled={1 if solve_score_omega_enabled else 0}",
                ])
            if xf_path:
                cmd.append(f"--root_xforms={xf_path}")
            progress["input_mode"] = "sectioned"
            pre_native_ms = int((time.time() - t0) * 1000)
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
            hist_rc = result.returncode
            hist_stdout = result.stdout
            hist_stderr = result.stderr
            if hist_rc != 0:
                stderr_summary = (hist_stderr or "").strip() or "unknown error"
                raise RuntimeError(
                    "solve_proximity_hist_sectioned failed for "
                    f"s3://{BUCKET}/{bin_key} "
                    f"(clip=s3://{BUCKET}/{clip_key}, job={job_id}, task={task_id}, "
                    f"section={section_idx}, input=sectioned, size={input_size}, "
                    f"threads={solve_score_threads}, retries={solve_score_hist_retries}, metric={metric}{program_suffix}): {stderr_summary}"
                )
            hist_data = json.loads(hist_stdout)
            progress["dl_ms"] = pre_native_ms + int(hist_data.get("download_ms", 0))
            progress["compute_ms"] = int(hist_data.get("compute_ms", 0))
            progress["wall_ms"] = int(hist_data.get("wall_ms", 0))
            report_status(job_id, task_id, "bin_downloaded", result_data=progress)
        elif solve_score_hist_input_mode == "stdin":
            cmd = [
                BINARY,
                "-",
                "--mode=hist",
                f"--degree={degree}",
                f"--hist_bins={hist_bins}",
                f"--threads={solve_score_threads}",
            ]
            if program_args:
                cmd.extend(program_args)
                if uses_coeff_source:
                    coeff_size = _download(coeffs_key, _TMP_COEFF_INPUT)
                    progress["source_coeffs_size"] = coeff_size
                    cmd.extend([
                        f"--score_coeffs_file={_TMP_COEFF_INPUT}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                if uses_param_source:
                    param_size = _download_range(
                        params_key,
                        _TMP_PARAM_INPUT,
                        int(params_step_start) * 4 * 4,
                        int(params_step_count) * 4 * 4,
                    )
                    progress["source_params_size"] = param_size
                    cmd.append(f"--score_params_file={_TMP_PARAM_INPUT}")
            else:
                cmd.extend([
                    f"--metric={metric}",
                    f"--clip_lo={primary_metric_clip['clip_lo']}",
                    f"--clip_hi={primary_metric_clip['clip_hi']}",
                    f"--omega={solve_score_omega}",
                    f"--omega_enabled={1 if solve_score_omega_enabled else 0}",
                ])
            if xf_path:
                cmd.append(f"--root_xforms={xf_path}")
            bin_obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
            input_size = int(bin_obj.get("ContentLength") or 0)
            if input_size > 0:
                cmd.append(f"--input_size={input_size}")
            progress["source_size"] = input_size
            progress["input_mode"] = "stdin"
            pre_stream_ms = int((time.time() - t0) * 1000)
            hist_rc, hist_stdout, hist_stderr, stream_ms, compute_ms = _run_binary_with_streamed_input(cmd, bin_obj, input_size, timeout=120)
            progress["dl_ms"] = pre_stream_ms + stream_ms
            report_status(job_id, task_id, "bin_downloaded", result_data=progress)
        else:
            cmd = [
                BINARY,
                _TMP_INPUT,
                "--mode=hist",
                f"--degree={degree}",
                f"--hist_bins={hist_bins}",
                f"--threads={solve_score_threads}",
            ]
            if program_args:
                cmd.extend(program_args)
                if uses_coeff_source:
                    coeff_size = _download(coeffs_key, _TMP_COEFF_INPUT)
                    progress["source_coeffs_size"] = coeff_size
                    cmd.extend([
                        f"--score_coeffs_file={_TMP_COEFF_INPUT}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                if uses_param_source:
                    param_size = _download_range(
                        params_key,
                        _TMP_PARAM_INPUT,
                        int(params_step_start) * 4 * 4,
                        int(params_step_count) * 4 * 4,
                    )
                    progress["source_params_size"] = param_size
                    cmd.append(f"--score_params_file={_TMP_PARAM_INPUT}")
            else:
                cmd.extend([
                    f"--metric={metric}",
                    f"--clip_lo={clip_data['clip_lo']}",
                    f"--clip_hi={clip_data['clip_hi']}",
                    f"--omega={solve_score_omega}",
                    f"--omega_enabled={1 if solve_score_omega_enabled else 0}",
                ])
            if xf_path:
                cmd.append(f"--root_xforms={xf_path}")
            size = _download(bin_key, _TMP_INPUT)
            progress["source_size"] = size
            progress["input_mode"] = "tmpfile"
            progress["dl_ms"] = int((time.time() - t0) * 1000)
            report_status(job_id, task_id, "bin_downloaded", result_data=progress)
            t1 = time.time()
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
            compute_ms = int((time.time() - t1) * 1000)
            hist_rc = result.returncode
            hist_stdout = result.stdout
            hist_stderr = result.stderr

        if hist_rc != 0:
            stderr_summary = (hist_stderr or "").strip() or "unknown error"
            raise RuntimeError(
                "solve_proximity_stats hist failed "
                f"(job={job_id}, task={task_id}, section={section_idx}, input={effective_input_mode}, "
                f"metric={metric}, source=s3://{BUCKET}/{bin_key}{program_suffix}): {stderr_summary}"
            )

        hist_data = json.loads(hist_stdout)
        if effective_input_mode in ("sectioned", "multispan_sectioned"):
            progress["compute_ms"] = int(hist_data.get("compute_ms", progress.get("compute_ms", 0) or 0))
        else:
            progress["compute_ms"] = compute_ms
        progress["threads"] = int(hist_data.get("threads", solve_score_threads))
        progress["n_solves"] = hist_data["n_solves"]

        artifact = {
            "family": "solve_score",
            "version": 2 if program_args else 1,
            "job_id": job_id,
            "metric": metric,
            "clip_quantile": compiled["quantile"],
            "omega": solve_score_omega,
            "omega_enabled": solve_score_omega_enabled,
            "chain_fingerprint": compiled_solve_score_fingerprint(compiled),
            "solve_score_spec_version": SOLVE_SCORE_SPEC_VERSION,
            "chunk_idx": int(section_idx),
            "hist_bins": hist_bins,
            "clip_lo": primary_metric_clip["clip_lo"],
            "clip_hi": primary_metric_clip["clip_hi"],
            "n_solves": hist_data["n_solves"],
            "hist": hist_data["hist"],
        }
        if program_args:
            artifact.update({
                "chain": json.loads(serialize_solve_score_chain(compiled["chain"])),
                "program": compiled["program_spec"],
                "metrics": metrics_with_clips,
                "metric_count": len(metrics_with_clips),
            })
            if uses_coeff_source:
                artifact["n_coeffs"] = n_coeffs
            if uses_param_source:
                artifact["params_key"] = params_key
                artifact["step_start"] = step_start
                artifact["step_count"] = step_count

        s3.put_object(Bucket=BUCKET, Key=out_key,
                      Body=json.dumps(artifact),
                      ContentType="application/json")

        progress["out_key"] = out_key
        report_status(job_id, task_id, "done", result_data=progress)
        return ok_response(progress)

    except Exception as e:
        progress["error"] = str(e)
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()


def handle_merge(params):
    contract_warnings = []
    job_id = params["job_id"]
    task_id = params["task_id"]
    metric = contract_param(params, "metric", "proximity", contract_warnings)
    _validate_metric(metric)
    solve_score_threads = _validate_threads(contract_param(params, "solve_score_threads", 1, contract_warnings), default=1)
    n_chunks = params.get("n_chunks", params.get("n_stripes"))
    if n_chunks is None:
        raise RuntimeError("merge requires n_chunks")
    n_chunks = int(n_chunks)
    merge_workers = min(_validate_merge_workers(contract_param(params, "solve_score_merge_workers", None, contract_warnings, warning_default="auto")), max(1, n_chunks))
    hist_prefix = params["hist_prefix"]
    clip_key = params["clip_key"]
    out_key = params["out_key"]
    compiled = _compile_request_chain(params, metric, default_metric="proximity")
    metric = compiled["metric"]
    solve_score_omega = compiled["omega"]
    solve_score_omega_enabled = compiled["omega_enabled"]
    progress = attach_contract_warnings({
        "phase": "merge",
        "metric": metric,
        "metric_count": compiled["metric_count"],
        "n_chunks": n_chunks,
        "omega": solve_score_omega,
        "omega_enabled": solve_score_omega_enabled,
        "threads": merge_workers,
        "workers": merge_workers,
    }, contract_warnings)

    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        merge_s3 = _merge_s3_client(merge_workers)
        t_dl = time.time()
        # Load and validate clip data
        clip_obj = merge_s3.get_object(Bucket=BUCKET, Key=clip_key)
        clip_data = _load_json_body(clip_obj)
        metrics_with_clips = _validate_clip_artifact(
            clip_data,
            compiled,
        )
        hist_bins = 100

        total_hist = [0] * hist_bins
        total_solves = 0
        with ThreadPoolExecutor(max_workers=merge_workers) as executor:
            futures = {
                executor.submit(
                    _load_merge_histogram_artifact,
                    merge_s3,
                    hist_prefix,
                    c,
                    compiled,
                    hist_bins,
                ): c
                for c in range(n_chunks)
            }
            for future in as_completed(futures):
                loaded = future.result()
                chunk_hist = loaded["hist"]
                for i in range(hist_bins):
                    total_hist[i] += chunk_hist[i]
                total_solves += loaded["n_solves"]

        progress["dl_ms"] = int((time.time() - t_dl) * 1000)
        progress["n_solves_total"] = total_solves
        report_status(job_id, task_id, "merged", result_data=progress)

        # Derive 10 equal-density bins
        t_compute = time.time()
        final_bins = 10
        total_count = sum(total_hist)
        cuts_norm = []
        for k in range(1, final_bins):
            target = total_count * k / final_bins
            cum = 0
            cut = 1.0
            for i in range(hist_bins):
                cum_before = cum
                cum += total_hist[i]
                if cum >= target:
                    bucket_count = total_hist[i]
                    if bucket_count > 0:
                        frac = (target - cum_before) / bucket_count
                    else:
                        frac = 1.0
                    cut = (i + frac) / hist_bins
                    break
            cut = max(0.0, min(1.0, cut))
            if cuts_norm and cut < cuts_norm[-1]:
                cut = cuts_norm[-1]
            cuts_norm.append(cut)

        artifact = {
            "family": "solve_score",
            "version": 2 if clip_data.get("version", 1) >= 2 else 1,
            "job_id": job_id,
            "metric": metric,
            "clip_quantile": compiled["quantile"],
            "omega": solve_score_omega,
            "omega_enabled": solve_score_omega_enabled,
            "chain_fingerprint": compiled_solve_score_fingerprint(compiled),
            "solve_score_spec_version": SOLVE_SCORE_SPEC_VERSION,
            "hist_bins": hist_bins,
            "final_bins": final_bins,
            "clip_lo": metrics_with_clips[0]["clip_lo"],
            "clip_hi": metrics_with_clips[0]["clip_hi"],
            "cuts_norm": cuts_norm,
            "n_solves_total": total_solves,
            "root_transforms": clip_data.get("root_transforms", []),
        }
        if clip_data.get("version", 1) >= 2:
            artifact.update({
                "chain": json.loads(serialize_solve_score_chain(compiled["chain"])),
                "program": compiled["program_spec"],
                "metrics": metrics_with_clips,
                "metric_count": len(metrics_with_clips),
            })

        s3.put_object(Bucket=BUCKET, Key=out_key,
                      Body=json.dumps(artifact),
                      ContentType="application/json")

        progress["compute_ms"] = int((time.time() - t_compute) * 1000)
        progress["out_key"] = out_key
        progress["cuts_norm"] = cuts_norm
        report_status(job_id, task_id, "done", result_data=progress)
        return ok_response(progress)

    except Exception as e:
        progress["error"] = str(e)
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()


def handle_summary(params):
    """Synchronous debug summary — no side effects, no DDB writes, no S3 artifacts."""
    degree = params["degree"]
    metric = params.get("metric", "proximity")
    _validate_metric(metric)
    solve_score_threads = _validate_threads(params.get("solve_score_threads", 1), default=1)
    lores_bin_key = params["lores_bin_key"]
    lores_coeffs_key = params.get("lores_coeffs_key", "")
    lores_params_key = params.get("lores_params_key", "")
    n_coeffs = params.get("n_coeffs")
    root_transforms = params.get("root_transforms")
    solve_score_normalize = parse_boolish(
        params.get("solve_score_normalize", False),
        False,
        strict=True,
        label="solve_score_normalize",
    )
    compiled = _compile_request_chain(params, metric, default_metric="proximity")
    uses_coeff_source = solve_score_uses_source(compiled, "cf")
    uses_param_source = solve_score_uses_source(compiled, "pm")
    if uses_coeff_source:
        lores_coeffs_key = str(
            lores_coeffs_key or _fallback_lores_coeffs_key(params.get("job_id"), lores_bin_key)
        ).strip()
        if not lores_coeffs_key:
            raise RuntimeError("mixed-source solve-score summary requires lores_coeffs_key")
        try:
            n_coeffs = int(n_coeffs)
        except (TypeError, ValueError):
            raise RuntimeError(f"mixed-source solve-score summary requires numeric n_coeffs, got {n_coeffs!r}")
        if n_coeffs < 1:
            raise RuntimeError(f"mixed-source solve-score summary requires n_coeffs >= 1, got {n_coeffs}")
    if uses_param_source:
        lores_params_key = str(
            lores_params_key or _fallback_lores_params_key(params.get("job_id"), lores_bin_key)
        ).strip()
        if not lores_params_key:
            raise RuntimeError("param-source solve-score summary requires lores_params_key")

    try:
        _cleanup_tmp()

        t0 = time.time()
        size = _download(lores_bin_key, _TMP_INPUT)
        coeff_size = 0
        param_size = 0
        if uses_coeff_source:
            coeff_size = _download(lores_coeffs_key, _TMP_COEFF_INPUT)
        if uses_param_source:
            param_size = _download(lores_params_key, _TMP_PARAM_INPUT)
        dl_ms = int((time.time() - t0) * 1000)

        metric_clips = []
        for slot, metric_row in enumerate(compiled["metrics"]):
            source = metric_row.get("source", "slv")
            slot_clip = _clip_metric_slot(
                metric_row["metric"],
                metric_row["quantile"],
                degree if source == "slv" else (n_coeffs if source == "cf" else 2),
                compiled["omega"],
                compiled["omega_enabled"],
                solve_score_threads,
                root_transforms if source == "slv" else None,
                _TMP_INPUT if source == "slv" else (_TMP_COEFF_INPUT if source == "cf" else _TMP_PARAM_INPUT),
            )
            slot_clip["slot"] = slot
            slot_clip["source"] = source
            metric_clips.append(slot_clip)
        cmd = [
            BINARY,
            _TMP_INPUT,
            "--mode=summary",
            f"--degree={degree}",
            f"--threads={solve_score_threads}",
            *_build_program_cmd_args(compiled, metric_clips),
        ]
        if uses_coeff_source:
            cmd.extend([
                f"--score_coeffs_file={_TMP_COEFF_INPUT}",
                f"--score_coeff_degree={n_coeffs}",
            ])
        if uses_param_source:
            cmd.append(f"--score_params_file={_TMP_PARAM_INPUT}")
        xf_path = _write_xforms(root_transforms)
        if xf_path:
            cmd.append(f"--root_xforms={xf_path}")
        if solve_score_normalize:
            cmd.append("--score_output_normalize=1")

        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        compute_ms = int((time.time() - t1) * 1000)
        if result.returncode != 0:
            raise RuntimeError(
                "solve_proximity_stats summary failed "
                f"(rc={result.returncode}, metric={compiled['metric']}{_score_program_error_suffix(compiled)}): "
                f"{result.stderr.strip()}"
            )

        summary = json.loads(result.stdout)
        summary["dl_ms"] = dl_ms
        summary["compute_ms"] = compute_ms
        summary["source_size"] = size
        if uses_coeff_source:
            summary["source_coeffs_size"] = coeff_size
            summary["lores_coeffs_key"] = lores_coeffs_key
            summary["n_coeffs"] = n_coeffs
        if uses_param_source:
            summary["source_params_size"] = param_size
            summary["lores_params_key"] = lores_params_key
        summary["threads"] = int(summary.get("threads", solve_score_threads))
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
        summary.update(_score_output_contract(compiled, summary, legacy_normalize=solve_score_normalize))
        summary["metrics"] = metric_clips

        return ok_response(summary)

    finally:
        _cleanup_tmp()
