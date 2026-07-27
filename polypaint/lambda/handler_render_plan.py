"""
Render plan Lambda — computes the workflow plan for Step Functions.

Loads calc metadata, computes viewport, normalizes params, produces compact
logical section arrays and output keys. Does NOT dispatch workers or poll for
completion.

Called once per render execution as the BuildPlan step.
"""
import json
import math
import os
import hashlib
import re
from datetime import datetime, timezone

import boto3

from calc_chunks import (
    build_chunk_items,
    fallback_lores_coeffs_key as calc_fallback_lores_coeffs_key,
    fallback_lores_params_key as calc_fallback_lores_params_key,
)
from logical_sections import (
    MAX_LOGICAL_SECTIONS,
    build_logical_section_items,
    build_solve_source_manifest,
    coeff_row_bytes,
    compute_safe_sectioning,
    normalize_section_mode,
    param_row_bytes,
    root_row_bytes,
    section_source_row_bytes,
    summarize_chunk_items,
    validate_section_count,
    write_solve_source_manifest,
)
from palette_names import is_valid_palette_name, normalize_palette_display_name
from color_render_contract import DEFAULT_BACKGROUND_COLOR, normalize_background_color, validate_color_output_contract
from shared import BUCKET, BILEVEL_SPARSE_PIPELINE, REF_SIZE, parse_body, ok_response
from solve_score_chain import (
    compile_solve_score_chain,
    emit_solve_score_metadata,
    format_solve_score_chain_display,
    public_solve_score_chain,
    serialize_solve_score_chain,
    solve_score_lag_prelude_by_source,
    solve_score_uses_source,
)
from pipeline_programs import root_program_for_run, solve_score_program_for_run
from view_camera import (
    camera_execution_subset,
    camera_execution_hash,
    fragment_contract,
    validate_view_camera,
)
from view_snap_calibration import (
    calibration_artifact_mode,
    fixture_admission_digest,
    load_calibration_artifact,
)
from view_snap_cost_model import (
    MIB,
    enforce_hard_resource_limits,
    enforce_wall_time_limits,
    estimate_camera_sections,
    estimate_camera_wall_times,
)

s3 = boto3.client("s3")
lambda_client = boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1"))

VIEWPORT_FUNCTION = os.environ.get("VIEWPORT_FUNCTION", "polypaint-viewport")
STORAGE_FUNCTION = os.environ.get("STORAGE_FUNCTION", "polypaint-storage")
RASTER_MT_FUNCTION = os.environ.get("RASTER_MT_FUNCTION", "polypaint-raster-mt")

MAX_PLAN_BYTES = 200 * 1024  # 200 KB — fail fast before hitting 256 KB SFN limit
MAX_PIX = 32768
DEFAULT_BACKGROUND_THRESHOLD = 4
COLOR_ARTIFACT_ID_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,127}")
VIEW_SNAP_MEMORY_HEADROOM_MB = 1024
VIEW_SNAP_TMP_HEADROOM_MB = 512


def _view_snap_execution_config():
    fields = (
        ("raster_mt_threads", "VIEW_SNAP_RASTER_THREADS", 4),
        ("raster_workers", "VIEW_SNAP_RASTER_WORKERS", 10),
        ("finalize_workers", "VIEW_SNAP_FINALIZE_WORKERS", 16),
    )
    values = {}
    for key, env_name, default in fields:
        try:
            value = int(os.environ.get(env_name, str(default)) or default)
        except (TypeError, ValueError) as exc:
            raise RuntimeError(
                f"{env_name} must be a positive integer"
            ) from exc
        if value <= 0:
            raise RuntimeError(f"{env_name} must be a positive integer")
        values[key] = value
    # Solve-score execution is inside Raster and uses the same calibrated
    # native thread shape; it is not an independently tunable camera input.
    values["solve_score_threads"] = values["raster_mt_threads"]
    return values


def _view_snap_deployed_limits():
    raster_memory_mb = int(os.environ.get("VIEW_SNAP_RASTER_MEMORY_MB", "10240") or 10240)
    finalize_memory_mb = int(os.environ.get("VIEW_SNAP_FINALIZE_MEMORY_MB", "10240") or 10240)
    raster_tmp_mb = int(os.environ.get("VIEW_SNAP_RASTER_TMP_MB", "10240") or 10240)
    finalize_tmp_mb = int(os.environ.get("VIEW_SNAP_FINALIZE_TMP_MB", "10240") or 10240)
    values = {
        "raster_memory_mb": raster_memory_mb,
        "finalize_memory_mb": finalize_memory_mb,
        "raster_tmp_mb": raster_tmp_mb,
        "finalize_tmp_mb": finalize_tmp_mb,
    }
    if any(value <= 0 for value in values.values()):
        raise RuntimeError("ViewSnap deployed memory and /tmp limits must be positive")
    if raster_memory_mb <= VIEW_SNAP_MEMORY_HEADROOM_MB:
        raise RuntimeError("ViewSnap raster memory is smaller than its named headroom")
    if finalize_memory_mb <= VIEW_SNAP_MEMORY_HEADROOM_MB:
        raise RuntimeError("ViewSnap Finalize memory is smaller than its named headroom")
    if raster_tmp_mb <= VIEW_SNAP_TMP_HEADROOM_MB:
        raise RuntimeError("ViewSnap raster /tmp is smaller than its named headroom")
    if finalize_tmp_mb <= VIEW_SNAP_TMP_HEADROOM_MB:
        raise RuntimeError("ViewSnap Finalize /tmp is smaller than its named headroom")
    return {
        **values,
        "memory_headroom_mb": VIEW_SNAP_MEMORY_HEADROOM_MB,
        "tmp_headroom_mb": VIEW_SNAP_TMP_HEADROOM_MB,
        "raster_memory_bytes": (raster_memory_mb - VIEW_SNAP_MEMORY_HEADROOM_MB) * MIB,
        "finalize_memory_bytes": (finalize_memory_mb - VIEW_SNAP_MEMORY_HEADROOM_MB) * MIB,
        "raster_tmp_bytes": (raster_tmp_mb - VIEW_SNAP_TMP_HEADROOM_MB) * MIB,
        "finalize_tmp_bytes": (finalize_tmp_mb - VIEW_SNAP_TMP_HEADROOM_MB) * MIB,
    }


def _view_snap_fixture_source_identity(solve_source_manifest):
    """Bind a calibration fixture to every object the native reader consumes."""

    manifest = dict(solve_source_manifest or {})
    identity_manifest = {
        key: value
        for key, value in manifest.items()
        if key not in {"j", "job_id"}
    }
    required_by_key = {}
    sources = dict(manifest.get("s") or manifest.get("sources") or {})
    for family in ("slv", "cf", "pm"):
        source = dict(sources.get(family) or {})
        row_bytes = int(source.get("r") or source.get("row_bytes") or 0)
        keys = list(source.get("k") or source.get("keys") or [])
        segments = list(source.get("g") or source.get("segments") or [])
        if row_bytes <= 0:
            continue
        for segment in segments:
            if isinstance(segment, dict):
                key = str(segment.get("key") or "").strip()
                source_start = int(segment.get("source_solve_start") or 0)
                source_count = int(segment.get("solve_count") or 0)
            else:
                try:
                    key_idx, _solve_start, source_count, source_start = segment[:4]
                    key = str(keys[int(key_idx)] or "").strip()
                except (IndexError, TypeError, ValueError) as exc:
                    raise RuntimeError(
                        f"invalid ViewSnap fixture {family} source segment: {segment!r}"
                    ) from exc
            if not key or source_start < 0 or source_count <= 0:
                raise RuntimeError(
                    f"invalid ViewSnap fixture {family} source segment: {segment!r}"
                )
            required = (source_start + source_count) * row_bytes
            required_by_key[key] = max(required_by_key.get(key, 0), required)

    if not required_by_key:
        raise RuntimeError("ViewSnap calibration fixture has no source objects")

    objects = []
    for key in sorted(required_by_key):
        try:
            head = s3.head_object(Bucket=BUCKET, Key=key)
        except Exception as exc:
            raise RuntimeError(
                f"ViewSnap calibration fixture source is unreadable: s3://{BUCKET}/{key}"
            ) from exc
        size = int(head.get("ContentLength") or 0)
        required = int(required_by_key[key])
        if size < required:
            raise RuntimeError(
                "ViewSnap calibration fixture source is truncated: "
                f"s3://{BUCKET}/{key} has {size} bytes, requires {required}"
            )
        etag = str(head.get("ETag") or "").strip().strip('"')
        version_id = str(head.get("VersionId") or "").strip()
        checksum = str(head.get("ChecksumSHA256") or "").strip()
        if not (checksum or version_id or etag):
            raise RuntimeError(
                "ViewSnap calibration fixture source has no immutable content identity: "
                f"s3://{BUCKET}/{key}"
            )
        row = {
            "key": key,
            "size_bytes": size,
            "required_bytes": required,
        }
        if checksum:
            row["checksum_sha256"] = checksum
        if version_id:
            row["version_id"] = version_id
        if etag:
            row["etag"] = etag
        objects.append(row)

    descriptor = {
        "schema_version": 1,
        "manifest_sha256": fixture_admission_digest(identity_manifest),
        "objects": objects,
    }
    return {
        "descriptor_sha256": fixture_admission_digest(descriptor),
        "descriptor": descriptor,
    }


def _view_snap_fixture_payload(
    *,
    pix,
    times,
    degree,
    n_coeffs,
    channels,
    camera,
    fmt,
    quality,
    root_program_fingerprint,
    solve_score_fingerprint,
    solve_score_prelude,
    section_items,
    fused_params,
    palette,
    background_color,
    source_identity,
):
    return {
        "N": int(pix),
        "times": int(times),
        "degree": int(degree),
        "n_coeffs": int(n_coeffs),
        "channels": int(channels),
        "view_projection": "camera",
        "view_camera": camera_execution_subset(camera),
        "format": str(fmt),
        "quality": int(quality),
        "root_program_fingerprint": str(root_program_fingerprint or ""),
        "solve_score_program_fingerprint": str(solve_score_fingerprint or ""),
        "prelude": {
            "slv": int(solve_score_prelude.get("slv") or 0),
            "cf": int(solve_score_prelude.get("cf") or 0),
            "pm": int(solve_score_prelude.get("pm") or 0),
        },
        "sections": [
            {
                "section_idx": int(item.get("section_idx") or 0),
                "step_start": int(item.get("step_start") or 0),
                "step_count": int(item.get("step_count") or 0),
            }
            for item in section_items
        ],
        "execution": {
            "raster_mt_threads": int(fused_params["raster_mt_threads"]),
            "solve_score_threads": int(fused_params["solve_score_threads"]),
            "raster_workers": int(fused_params["raster_workers"]),
            "finalize_workers": int(fused_params["finalize_workers"]),
        },
        "output": {
            "color_interpretation": str(fused_params["color_interpretation"]),
            "solve_score_normalize": bool(fused_params["solve_score_normalize"]),
            "palette": str(palette),
            "background_color": str(background_color),
        },
        "source_identity": source_identity,
    }


def _plan_params_digest(
    *,
    viewport,
    pix,
    root_transforms=None,
    root_program_fingerprint="",
    solve_score_normalize=False,
    color_interpretation="scalar_lut",
    background_color=None,
    view_projection="plan",
    view_vertical="t2",
    view_camera=None,
):
    grid = {"pix": int(pix)}
    params = {
        "solve_score_normalize": bool(solve_score_normalize),
        "color_interpretation": str(color_interpretation or "scalar_lut"),
    }
    if root_program_fingerprint:
        params["root_program_fingerprint"] = str(root_program_fingerprint)
    else:
        params["root_transforms"] = root_transforms or []
    if background_color is not None:
        params["background_color"] = str(background_color or DEFAULT_BACKGROUND_COLOR).strip().lower()
    projection = str(view_projection or "plan").strip().lower()
    if projection != "plan":
        params["view_projection"] = projection
        params["view_vertical"] = str(view_vertical or "t2").strip().lower()
        if projection == "camera":
            params["view_camera_sha256"] = camera_execution_hash(view_camera)
    payload = {
        "viewport": viewport,
        "grid": grid,
        "params": params,
        "raster_binary_sha256": str(os.environ.get("RASTER_BINARY_SHA256") or ""),
    }
    encoded = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return "sha256:" + hashlib.sha256(encoded).hexdigest()


def _root_program_payload(compiled):
    return {
        "program_kind": "root_program",
        "spec_version": int(compiled.get("spec_version") or 2),
        "source_text": str(compiled.get("source_text") or ""),
        "chain": compiled.get("chain") or [],
        "root_transforms": compiled.get("root_transforms") or [],
        "display": compiled.get("display") or "",
        "fingerprint": compiled.get("fingerprint") or "",
        "execution_spec": compiled.get("execution_spec") or "",
        "tokens": compiled.get("tokens") or [],
        "token_count": int(compiled.get("token_count") or 0),
    }


def _apply_root_program_to_params(params):
    compiled = root_program_for_run(params)
    payload = _root_program_payload(compiled)
    params["root_transforms"] = payload["root_transforms"]
    params["root_program_source_text"] = payload["source_text"]
    params["root_program"] = payload
    params["root_program_fingerprint"] = payload["fingerprint"]
    params["root_spec_version"] = payload["spec_version"]
    return payload


def _root_program_metadata(payload):
    return {
        "root_transforms": json.dumps(payload.get("root_transforms") or [], separators=(",", ":")),
        "root_program_source_text": str(payload.get("source_text") or ""),
        "root_program": json.dumps(payload, separators=(",", ":")),
        "root_program_fingerprint": str(payload.get("fingerprint") or ""),
        "root_spec_version": str(payload.get("spec_version") or 2),
    }


def _fallback_lores_coeffs_key(job_id, calc):
    return calc_fallback_lores_coeffs_key(job_id, calc)


def _fallback_lores_params_key(job_id, calc):
    return calc_fallback_lores_params_key(job_id, calc)


def _compact_section_ranges(section_items, *, include_camera_estimate=False):
    compact = []
    for item in section_items or []:
        camera_estimate = dict(item.get("camera_estimate") or {})
        row = {
            "section_idx": int(item["section_idx"]),
            "section_count": int(item["section_count"]),
            "step_start": int(item.get("step_start") or 0),
            "step_count": int(item.get("step_count") or 0),
        }
        if include_camera_estimate:
            # Keep this object present for every Color Raster item. Step
            # Functions JSONPath selectors fail when an optional source path
            # is absent.
            row["camera_estimate"] = {
                key: int(camera_estimate[key])
                for key in (
                    "candidate_roots",
                    "occupied_pixels_upper",
                    "fragment_bytes_upper",
                    "raster_memory_bytes",
                    "raster_tmp_bytes",
                    "work_units",
                )
                if key in camera_estimate
            }
        compact.append(row)
    return compact


def _validate_boolish(value, field_name, default=False):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    text = str(value).strip().lower()
    if text in ("1", "true", "yes", "on"):
        return True
    if text in ("0", "false", "no", "off"):
        return False
    raise RuntimeError(f"{field_name} must be boolean-like, got {value!r}")


def _associated_palette_display_name(chain, metric, quantile, palette):
    label = format_solve_score_chain_display(chain, legacy_quantile=quantile)
    parts = [label] if label else [str(metric or "").strip()]
    if palette:
        parts.append(str(palette))
    return " ".join(part for part in parts if part)


def _color_interpretation_uses_palette(color_interpretation):
    return str(color_interpretation or "scalar_lut") in ("scalar_lut", "rgb_lut", "hsv_lut")


def _associated_palette_mode_for_output(color_interpretation, output_channel_count):
    mode = str(color_interpretation or "scalar_lut")
    count = int(output_channel_count or 1)
    if (mode == "scalar_lut" and count == 1) or (mode in ("rgb", "hsv", "rgb_lut", "hsv_lut") and count == 3):
        return "generated"
    raise RuntimeError(
        "associated palette saving requires Scalar LUT with one output channel "
        "or RGB/HSV/RGB LUT/HSV LUT with three output channels"
    )


def _root_identity_hash(root_transforms, root_program_fingerprint=""):
    fingerprint = str(root_program_fingerprint or "").strip()
    if fingerprint:
        return fingerprint.split(":", 1)[-1][:8]
    rt_json = json.dumps(root_transforms or [], separators=(",", ":"))
    return hashlib.sha1(rt_json.encode("utf-8")).hexdigest()[:8]


def _solve_score_scratch_key(job_id, compiled, root_transforms, score_normalize=False, root_program_fingerprint=""):
    rt_hash = _root_identity_hash(root_transforms, root_program_fingerprint)
    norm_suffix = "_sn1" if score_normalize else ""
    if compiled:
        metric_slug = str(compiled["metric"] or "score").replace(" ", "_")
        if compiled["legacy_compatible"]:
            prefix = f"renders/{job_id}/solve_scores/{metric_slug}{norm_suffix}_rt{rt_hash}/"
        else:
            encoded_chain = serialize_solve_score_chain(compiled["chain"])
            chain_id = hashlib.sha1(encoded_chain.encode("utf-8")).hexdigest()[:12]
            prefix = f"renders/{job_id}/solve_scores/{metric_slug}_{chain_id}{norm_suffix}_rt{rt_hash}/"
    else:
        prefix = f"renders/{job_id}/solve_scores/inactive_rt{rt_hash}/"
    return prefix + "clip.json"


def _coerce_positive_int(value, field_name, *, default=None):
    if value in (None, ""):
        if default is None:
            raise RuntimeError(f"{field_name} is required")
        return int(default)
    try:
        coerced = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if coerced < 1:
        raise RuntimeError(f"{field_name} must be >= 1, got {coerced}")
    return coerced


def _coerce_degree(calc):
    return _coerce_positive_int(calc.get("degree"), "calc.degree", default=1)


def _coerce_n_coeffs(calc, degree):
    return _coerce_positive_int(calc.get("n_coeffs"), "calc.n_coeffs", default=int(degree) + 1)


def _validate_raster_engine(value):
    engine = str(value or "single").strip().lower()
    if engine not in ("single", "mt"):
        raise RuntimeError(f"raster_engine must be 'single' or 'mt', got {value!r}")
    return engine


def _validate_thread_count(value, field_name):
    if value in (None, ""):
        return 4
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"{field_name} must be in [1, 16], got {threads}")
    return threads


def _validate_worker_count(value, field_name):
    if value in (None, ""):
        return 16
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if not (1 <= workers <= 64):
        raise RuntimeError(f"{field_name} must be in [1, 64], got {workers}")
    return workers


def _validate_retry_count(value, field_name):
    if value in (None, ""):
        return 2
    try:
        retries = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if not (0 <= retries <= 10):
        raise RuntimeError(f"{field_name} must be in [0, 10], got {retries}")
    return retries


def _validate_raster_input_mode(value):
    mode = str(value or "tmpfile").strip().lower()
    if mode not in ("tmpfile", "sectioned"):
        raise RuntimeError(f"raster_input_mode must be 'tmpfile' or 'sectioned', got {value!r}")
    return mode


def _coerce_finite_float(value, field_name):
    try:
        num = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be numeric, got {value!r}")
    if not math.isfinite(num):
        raise RuntimeError(f"{field_name} must be finite, got {value!r}")
    return num


def _explicit_viewport_from_params(rp):
    min_re = _coerce_finite_float(rp.get("min_re"), "min_re")
    max_re = _coerce_finite_float(rp.get("max_re"), "max_re")
    min_im = _coerce_finite_float(rp.get("min_im"), "min_im")
    max_im = _coerce_finite_float(rp.get("max_im"), "max_im")
    if not max_re > min_re:
        raise RuntimeError(f"explicit viewport requires max_re > min_re, got {min_re!r}/{max_re!r}")
    if not max_im > min_im:
        raise RuntimeError(f"explicit viewport requires max_im > min_im, got {min_im!r}/{max_im!r}")
    return {
        "min_re": min_re,
        "max_re": max_re,
        "min_im": min_im,
        "max_im": max_im,
    }


def _fused_render_execution_config(rp):
    return {
        "raster_engine": "mt",
        "save_associated_palette": rp.get("save_associated_palette", False),
        "background_color": normalize_background_color(rp.get("background_color")),
        "raster_mt_threads": int(rp.get("raster_mt_threads", 4) or 4),
        "raster_workers": int(rp.get("raster_workers", 10) or 10),
        "solve_score_threads": int(rp.get("solve_score_threads", 4) or 4),
        "raster_input_mode": "sectioned",
        "raster_sectioned_retries": int(rp.get("raster_sectioned_retries", 2) or 2),
        "raster_section_mode": str(rp.get("raster_section_mode", "logical_sections_auto") or "logical_sections_auto"),
        "raster_section_count": rp.get("raster_section_count", ""),
        "raster_section_count_auto": rp.get("raster_section_count_auto", ""),
        "finalize_workers": int(rp.get("finalize_workers", 16) or 16),
    }
def _reject_fused_unsupported_params(raw_params):
    unsupported = {
        "solve_score_hist_input_mode": "fused color runs a single clip prepass, not a separate histogram stage",
        "solve_score_hist_retries": "fused color runs a single clip prepass, not a separate histogram stage",
        "solve_score_merge_workers": "fused color has no solve-score merge stage",
        "solve_score_section_mode": "fused color has no separate solve-score section stage",
        "solve_score_section_count": "fused color has no separate solve-score section stage",
        "raster_bin_group_size": "fused color does not support grouped fragment output",
        "palette_chunk_threads": "fused color emits the associated palette inline during finalize",
        "palette_chunk_input_mode": "fused color emits the associated palette inline during finalize",
        "palette_chunk_retries": "fused color emits the associated palette inline during finalize",
        "palette_chunk_workers": "fused color emits the associated palette inline during finalize",
        "palette_section_mode": "fused color emits the associated palette inline during finalize",
        "palette_section_count": "fused color emits the associated palette inline during finalize",
        "solve_metric": "fused color takes only solve_score_chain; metric is derived from the chain",
        "solve_score_quantile": "fused color takes only solve_score_chain; quantiles live on metric chips",
        "solve_score_omega": "fused color takes only solve_score_chain; transfer ops live in the chain",
        "solve_score_omega_enabled": "fused color takes only solve_score_chain; transfer ops live in the chain",
    }
    for key, reason in unsupported.items():
        value = raw_params.get(key)
        if value not in (None, "", False):
            raise RuntimeError(f"{key} is not supported for fused color: {reason}")


def _require_pix_only_grid_params(rp):
    if rp.get("width") not in (None, "") or rp.get("height") not in (None, ""):
        raise RuntimeError("render plan no longer accepts width/height; pass pix for square output")
    try:
        pix = int(rp.get("pix"))
    except (TypeError, ValueError) as exc:
        raise RuntimeError("render plan requires integer pix") from exc
    if pix <= 0:
        raise RuntimeError(f"render plan requires pix > 0, got {pix}")
    if pix > MAX_PIX:
        raise RuntimeError(f"render plan requires pix <= {MAX_PIX}, got {pix}")
    rp["pix"] = pix
    return pix


def _build_fused_color_plan(
    *,
    job_id,
    run_id,
    task_id,
    rp,
    viewport,
    calc,
    degree,
    calc_n_coeffs,
    full_n,
    times,
    chunk_items,
    chunk_summary,
):
    raw_params = dict(rp)
    view_vertical_supplied = "view_vertical" in raw_params
    _reject_fused_unsupported_params(raw_params)

    fused_params = {
        "pix": rp["pix"],
        "view_mode": rp.get("view_mode", "auto"),
        "quantile": rp.get("quantile", 0.0),
        "shim": rp.get("shim", 0.05),
        "square_extent": rp.get("square_extent", 2.0),
    }
    defaults = {
        "root_transforms": [],
        "root_program_source_text": "",
        "root_program": None,
        "rotation": 0,
        "palette": "inferno",
        "palette_display_name": "",
        "background_color": DEFAULT_BACKGROUND_COLOR,
        "match_mode": "none",
        "quality": 90,
        "fmt": "jpeg",
        "color_mode": "solve_score",
        "color_interpretation": "scalar_lut",
        "raster_engine": "mt",
        "raster_mt_threads": 4,
        "raster_workers": 10,
        "raster_input_mode": "sectioned",
        "raster_sectioned_retries": 2,
        "raster_section_mode": "logical_sections_auto",
        "raster_section_count": "",
        "solve_score_threads": "",
        "finalize_workers": 16,
        "solve_score_chain": "",
        "solve_score_program_source_text": "",
        "solve_score_program": None,
        "solve_score_normalize": False,
        "save_associated_palette": False,
        "view_projection": "plan",
        "view_vertical": "t2",
        "view_camera": {},
        "source_color_artifact_id": "",
        "source_sculpture_id": "",
    }
    for key, default in defaults.items():
        fused_params[key] = rp.get(key, default)
    # Views re-map only the plot pixel; validate here so a bad
    # request dies at plan time, not inside a raster section
    fused_params["view_projection"] = str(fused_params.get("view_projection") or "plan").strip().lower()
    if fused_params["view_projection"] not in (
        "plan", "front", "rear", "left", "right", "radial", "isometric", "camera"
    ):
        raise RuntimeError(
            "view_projection must be "
            f"plan/front/rear/left/right/radial/isometric/camera, got {fused_params['view_projection']!r}")
    camera = fused_params.get("view_camera")
    if fused_params["view_projection"] == "camera":
        camera = validate_view_camera(camera)
        requested_vertical = str(fused_params.get("view_vertical") or "").strip().lower()
        if view_vertical_supplied and requested_vertical != camera["vertical"]:
            raise RuntimeError(
                "view_vertical conflicts with authoritative view_camera.vertical: "
                f"{requested_vertical!r} != {camera['vertical']!r}"
            )
        fused_params["view_vertical"] = camera["vertical"]
        fused_params["view_camera"] = camera
    else:
        if camera not in (None, {}):
            raise RuntimeError("view_camera is only valid for view_projection=camera")
        fused_params["view_camera"] = {}
        fused_params["view_vertical"] = str(fused_params.get("view_vertical") or "t2").strip().lower()
    if fused_params["view_vertical"] not in ("t1", "t2"):
        raise RuntimeError(f"view_vertical must be t1 or t2, got {fused_params['view_vertical']!r}")
    is_view = fused_params["view_projection"] != "plan"
    source_color_artifact_id = str(fused_params.get("source_color_artifact_id") or "").strip()
    if is_view:
        if not source_color_artifact_id:
            raise RuntimeError("ViewRender requires source_color_artifact_id")
        if not COLOR_ARTIFACT_ID_RE.fullmatch(source_color_artifact_id):
            raise RuntimeError(
                f"invalid source_color_artifact_id: {source_color_artifact_id!r}")
    elif source_color_artifact_id:
        raise RuntimeError("source_color_artifact_id is only valid for ViewRender")
    fused_params["source_color_artifact_id"] = source_color_artifact_id
    source_sculpture_id = str(fused_params.get("source_sculpture_id") or "").strip()
    if fused_params["view_projection"] == "camera":
        if not source_sculpture_id:
            raise RuntimeError("ViewSnap requires source_sculpture_id")
        if not COLOR_ARTIFACT_ID_RE.fullmatch(source_sculpture_id):
            raise RuntimeError(f"invalid source_sculpture_id: {source_sculpture_id!r}")
    elif source_sculpture_id:
        raise RuntimeError("source_sculpture_id is only valid for view_projection=camera")
    fused_params["source_sculpture_id"] = source_sculpture_id
    root_program_payload = _apply_root_program_to_params(fused_params)

    fused_params["color_mode"] = str(fused_params.get("color_mode") or "solve_score").strip().lower()
    if fused_params["color_mode"] != "solve_score":
        raise RuntimeError("fused color supports only color_mode=solve_score")

    fused_params["match_mode"] = str(fused_params.get("match_mode") or "none").strip().lower()
    if fused_params["match_mode"] != "none":
        raise RuntimeError("fused color does not support match_mode overrides")

    palette = str(fused_params.get("palette") or "inferno").strip()
    if not is_valid_palette_name(palette):
        raise RuntimeError(f"Invalid palette: {palette}")
    fused_params["palette"] = palette
    palette_display_name = normalize_palette_display_name(
        fused_params.get("palette_display_name"),
        palette,
    )
    fused_params["palette_display_name"] = palette_display_name

    background_color = normalize_background_color(fused_params.get("background_color"))
    fused_params["background_color"] = background_color
    fmt = str(fused_params.get("fmt") or "jpeg").strip().lower()
    if fmt == "jpg":
        fmt = "jpeg"
    if fmt not in ("jpeg", "png"):
        raise RuntimeError(f"fmt must be jpeg or png, got {fmt!r}")
    fused_params["fmt"] = fmt
    try:
        quality = int(fused_params.get("quality", 90))
    except (TypeError, ValueError) as exc:
        raise RuntimeError("quality must be an integer in [1,100]") from exc
    if not (1 <= quality <= 100):
        raise RuntimeError("quality must be an integer in [1,100]")
    fused_params["quality"] = quality

    fused_params["raster_engine"] = _validate_raster_engine(fused_params.get("raster_engine", "mt"))
    if fused_params["raster_engine"] != "mt":
        raise RuntimeError("fused color requires raster_engine=mt")

    fused_params["raster_mt_threads"] = _validate_thread_count(
        fused_params.get("raster_mt_threads", 4),
        "raster_mt_threads",
    )
    fused_params["raster_workers"] = _validate_worker_count(
        fused_params.get("raster_workers", 10),
        "raster_workers",
    )
    solve_score_threads_value = fused_params.get("solve_score_threads", "")
    if solve_score_threads_value in (None, ""):
        solve_score_threads_value = fused_params["raster_mt_threads"]
    fused_params["solve_score_threads"] = _validate_thread_count(
        solve_score_threads_value,
        "solve_score_threads",
    )
    fused_params["raster_sectioned_retries"] = _validate_retry_count(
        fused_params.get("raster_sectioned_retries", 2),
        "raster_sectioned_retries",
    )
    fused_params["finalize_workers"] = _validate_worker_count(
        fused_params.get("finalize_workers", 16),
        "finalize_workers",
    )
    if fused_params["view_projection"] == "camera":
        # ViewSnap admission and telemetry are calibrated for one fixed
        # execution shape; request values are not competing authorities.
        fused_params.update(_view_snap_execution_config())
    fused_params["save_associated_palette"] = _validate_boolish(
        fused_params.get("save_associated_palette", False),
        "save_associated_palette",
        False,
    )
    if (
        fused_params["view_projection"] == "camera"
        and fused_params["save_associated_palette"]
    ):
        raise RuntimeError(
            "ViewSnap does not support save_associated_palette; "
            "camera admission prices only the Views artifact"
        )

    fused_input_mode = _validate_raster_input_mode(fused_params.get("raster_input_mode", "sectioned"))
    if fused_input_mode != "sectioned":
        raise RuntimeError("fused color requires raster_input_mode=sectioned")
    fused_params["raster_input_mode"] = "sectioned"

    fused_section_mode = normalize_section_mode(fused_params.get("raster_section_mode", "logical_sections_auto"))
    if fused_section_mode == "physical_chunks":
        raise RuntimeError("fused color does not support physical chunk raster sections")
    if fused_section_mode not in ("logical_sections", "logical_sections_auto"):
        raise RuntimeError(f"unsupported fused raster_section_mode: {fused_section_mode!r}")
    fused_params["raster_section_mode"] = fused_section_mode
    fused_params["raster_section_count"] = validate_section_count(
        fused_params.get("raster_section_count", ""),
        "raster_section_count",
        default="",
    )

    if not full_n:
        raise RuntimeError(
            "fused color requires calc N/n1 so step_scores.raw can be written "
            "for ExtractPalette and associated-palette parity"
        )
    if is_view:
        if full_n < 2:
            raise RuntimeError(f"ViewRender requires calc N >= 2, got {full_n}")
        if full_n > MAX_PIX:
            raise RuntimeError(
                f"ViewRender calc N={full_n} exceeds the renderer limit {MAX_PIX}"
            )
        # A View is a projection of the N x N parameter square. t1/t2 is one
        # axis and the selected root projection is the other, so the saved
        # Color image's independent pixel size is irrelevant.
        fused_params["pix"] = full_n
    if not chunk_summary["chunk_step_metadata_complete"]:
        raise RuntimeError(
            "fused color requires chunk step metadata on every chunk so logical sections "
            "and step_scores.raw are well-defined"
        )

    if fused_params.get("solve_score_chain") in ("", None, []) and not str(fused_params.get("solve_score_program_source_text") or "").strip() and not isinstance(fused_params.get("solve_score_program"), dict):
        raise RuntimeError("fused color requires solve_score_chain or solve_score_program_source_text")
    solve_score_compiled = solve_score_program_for_run(fused_params)
    solve_score_chain_internal = solve_score_compiled["chain"]
    solve_score_chain_public = solve_score_compiled.get("chain_public") or public_solve_score_chain(solve_score_chain_internal)
    solve_metric = solve_score_compiled["metric"]
    solve_score_quantile = solve_score_compiled["quantile"]
    solve_score_omega = solve_score_compiled["omega"]
    solve_score_omega_enabled = solve_score_compiled["omega_enabled"]
    solve_score_uses_coeff = bool(solve_score_uses_source(solve_score_compiled, "cf"))
    solve_score_uses_param = bool(solve_score_uses_source(solve_score_compiled, "pm"))
    solve_score_prelude = solve_score_lag_prelude_by_source(solve_score_compiled)
    solve_score_output_channel_count = int(solve_score_compiled.get("output_channel_count") or 1)
    color_contract = validate_color_output_contract(
        interpretation=fused_params.get("color_interpretation", fused_params.get("score_output_interpretation", "scalar_lut")),
        output_channel_count=solve_score_output_channel_count,
        output_channels=solve_score_compiled.get("output_channels") or [],
    )
    solve_score_output_interpretation = color_contract["interpretation"]
    solve_score_output_channels = color_contract["channels"]
    render_warnings = list(color_contract.get("warnings") or [])
    solve_score_normalize = _validate_boolish(
        fused_params.get("solve_score_normalize", False),
        "solve_score_normalize",
        False,
    )
    if solve_score_compiled.get("has_explicit_outputs") and solve_score_normalize:
        raise RuntimeError("score normalization checkbox is legacy-only; explicit emit/emit_norm programs own normalization")
    fused_params["solve_score_normalize"] = solve_score_normalize
    fused_params["solve_score_chain"] = solve_score_chain_public
    fused_params["solve_score_program_source_text"] = solve_score_compiled.get("source_text", "")
    fused_params["color_interpretation"] = solve_score_output_interpretation
    render_fragment_contract = fragment_contract(
        fused_params["view_projection"],
        solve_score_output_channel_count,
        has_explicit_outputs=bool(solve_score_compiled.get("has_explicit_outputs")),
    )
    solve_source_manifest = build_solve_source_manifest(
        chunk_items,
        job_id=job_id,
        degree=degree,
        n_coeffs=calc_n_coeffs,
        include_coeff=solve_score_uses_coeff,
        include_param=solve_score_uses_param,
    )

    pix = fused_params["pix"]
    camera_admission = {}
    if fused_params["view_projection"] == "camera":
        expected_camera_steps = int(full_n) * int(full_n) * int(times)
        if int(chunk_summary["total_solves"]) != expected_camera_steps:
            raise RuntimeError(
                "ViewSnap requires complete source steps: "
                f"chunks describe {int(chunk_summary['total_solves'])}, "
                f"expected N*N*times={expected_camera_steps}"
            )
        source_row_bytes = section_source_row_bytes(
            degree,
            calc_n_coeffs,
            include_coeff=solve_score_uses_coeff,
            include_param=solve_score_uses_param,
        )
        prelude_row_bytes = (
            (root_row_bytes(degree) if int(solve_score_prelude["slv"]) > 0 else 0)
            + (
                coeff_row_bytes(calc_n_coeffs)
                if solve_score_uses_coeff and int(solve_score_prelude["cf"]) > 0
                else 0
            )
            + (
                param_row_bytes()
                if solve_score_uses_param and int(solve_score_prelude["pm"]) > 0
                else 0
            )
        )
        deployed_limits = _view_snap_deployed_limits()
        calibration_mode = calibration_artifact_mode()
        timing_admission_enabled = calibration_mode in ("calibration", "production")
        production_calibration = None
        fixture_source_identity = {}
        if calibration_mode == "production":
            production_calibration = load_calibration_artifact()
        elif calibration_mode == "calibration":
            if fused_params["raster_section_mode"] == "logical_sections_auto":
                raise RuntimeError(
                    "ViewSnap calibration fixtures require an explicit raster_section_count"
                )
            fixture_source_identity = _view_snap_fixture_source_identity(
                solve_source_manifest
            )

        def evaluate_camera_sections(section_count, *, raster_only=False):
            items = build_logical_section_items(
                chunk_items,
                section_count=section_count,
                degree=degree,
                n_coeffs=calc_n_coeffs,
                include_coeff=solve_score_uses_coeff,
                include_param=solve_score_uses_param,
            )
            estimate = estimate_camera_sections(
                items,
                pix=pix,
                degree=degree,
                n_coeffs=calc_n_coeffs,
                channels=solve_score_output_channel_count,
                times=times,
                source_row_bytes=source_row_bytes,
                prelude_row_bytes=prelude_row_bytes,
                camera=camera,
            )
            limits = enforce_hard_resource_limits(
                estimate,
                raster_memory_bytes=deployed_limits["raster_memory_bytes"],
                raster_tmp_bytes=deployed_limits["raster_tmp_bytes"],
                finalize_memory_bytes=deployed_limits["finalize_memory_bytes"],
                finalize_tmp_bytes=deployed_limits["finalize_tmp_bytes"],
            )
            fixture_digest = ""
            calibration = None
            wall = {}
            gates = {}
            if timing_admission_enabled:
                fixture_payload = _view_snap_fixture_payload(
                    pix=pix,
                    times=times,
                    degree=degree,
                    n_coeffs=calc_n_coeffs,
                    channels=solve_score_output_channel_count,
                    camera=camera,
                    fmt=fused_params["fmt"],
                    quality=fused_params["quality"],
                    root_program_fingerprint=root_program_payload.get("fingerprint", ""),
                    solve_score_fingerprint=solve_score_compiled.get("fingerprint", ""),
                    solve_score_prelude=solve_score_prelude,
                    section_items=items,
                    fused_params=fused_params,
                    palette=palette,
                    background_color=background_color,
                    source_identity=fixture_source_identity,
                )
                fixture_digest = fixture_admission_digest(fixture_payload)
                calibration = production_calibration or load_calibration_artifact(
                    fixture_admission_digest=fixture_digest
                )
                wall = estimate_camera_wall_times(
                    estimate,
                    section_count=len(items),
                    fmt=fused_params["fmt"],
                    quality=fused_params["quality"],
                    calibration=calibration,
                )
                gates = enforce_wall_time_limits(
                    wall,
                    calibration=calibration,
                    stages=(
                        ("raster_native", "raster_handler")
                        if raster_only
                        else None
                    ),
                )
            return (
                items,
                estimate,
                limits,
                wall,
                gates,
                fixture_digest,
                calibration,
            )

        if fused_params["raster_section_mode"] == "logical_sections_auto":
            high = min(
                max(1, int(chunk_summary["total_solves"])),
                MAX_LOGICAL_SECTIONS,
            )
            # Resource-only admission must still use the Step Functions Map.
            # Preserve at least the source chunk fan-out and fill the configured
            # worker pool; resource or optional timing gates may raise it further.
            baseline_sections = min(
                high,
                max(
                    1,
                    len(chunk_items),
                    int(fused_params["raster_workers"]),
                ),
            )
            # Prove the most finely sectioned legal plan fits before binary
            # searching for the smallest safe fan-out. If this fails, no
            # larger section count exists to rescue the request.
            (
                best_items,
                best_estimate,
                best_limits,
                best_wall,
                _best_raster_gates,
                best_fixture_digest,
                best_calibration,
            ) = evaluate_camera_sections(high, raster_only=True)
            low = baseline_sections
            best_count = len(best_items)
            while low <= high:
                mid = (low + high) // 2
                try:
                    (
                        items,
                        estimate,
                        limits,
                        wall,
                        _raster_gates,
                        candidate_digest,
                        candidate_calibration,
                    ) = evaluate_camera_sections(mid, raster_only=True)
                except RuntimeError:
                    low = mid + 1
                    continue
                best_count = len(items)
                best_items, best_estimate, best_limits = items, estimate, limits
                best_wall = wall
                best_fixture_digest = candidate_digest
                best_calibration = candidate_calibration
                high = mid - 1
            raster_section_items = best_items
            selected_raster_sections = best_count
            best_gates = (
                enforce_wall_time_limits(
                    best_wall,
                    calibration=best_calibration,
                )
                if best_calibration is not None
                else {}
            )
        else:
            baseline_sections = None
            selected_raster_sections = fused_params["raster_section_count"]
            if selected_raster_sections in ("", None):
                selected_raster_sections = 1
            (
                raster_section_items,
                best_estimate,
                best_limits,
                best_wall,
                best_gates,
                best_fixture_digest,
                best_calibration,
            ) = evaluate_camera_sections(int(selected_raster_sections))
            selected_raster_sections = len(raster_section_items)

        fused_params["raster_section_count_auto"] = int(selected_raster_sections)
        fused_params["raster_section_count"] = int(selected_raster_sections)
        raster_section_auto = {
            "computed_section_count": int(selected_raster_sections),
            "min_safe_sections": int(selected_raster_sections),
            "budget_bytes": best_limits["raster_memory_bytes"],
            "memory_mb": deployed_limits["raster_memory_mb"],
            "fixed_bytes": best_estimate["fixed_raster_bytes"],
            "row_bytes": source_row_bytes + solve_score_output_channel_count,
            "prelude_row_bytes": prelude_row_bytes,
        }
        if baseline_sections is not None:
            raster_section_auto["baseline_section_count"] = int(
                baseline_sections
            )
        camera_admission = {
            "model_version": 1,
            "calibration_schema_version": (
                best_calibration["schema_version"]
                if best_calibration is not None
                else None
            ),
            "calibration_mode": calibration_mode,
            "calibration_cost_model_version": (
                best_calibration["cost_model_version"]
                if best_calibration is not None
                else None
            ),
            "timing_admission_enforced": best_calibration is not None,
            "fixture_admission_digest": best_fixture_digest,
            "section_count": int(selected_raster_sections),
            "baseline_section_count": (
                int(baseline_sections)
                if baseline_sections is not None
                else None
            ),
            "pixel_count": best_estimate["pixel_count"],
            "record_size_bytes": best_estimate["record_size_bytes"],
            "max_point_side": best_estimate["footprint"]["max_point_side"],
            "max_point_area": best_estimate["footprint"]["max_point_area"],
            "pricing_depth": best_estimate["footprint"]["pricing_depth"],
            "max_fragment_bytes": best_estimate["max_fragment_bytes"],
            "total_fragment_bytes": best_estimate["total_fragment_bytes"],
            "max_raster_memory_bytes": best_estimate["max_raster_memory_bytes"],
            "max_raster_tmp_bytes": best_estimate["max_raster_tmp_bytes"],
            "finalize_memory_bytes": best_estimate["final_memory_bytes"],
            "finalize_tmp_bytes": best_estimate["final_tmp_bytes"],
            "step_scores_bytes": best_estimate["step_scores_bytes"],
            "max_work_units": best_estimate["max_work_units"],
            "total_work_units": best_estimate["total_work_units"],
            "wall_seconds": {
                key: value
                for key, value in best_wall.items()
                if key != "raster_sections"
            },
            "wall_gates": best_gates,
            "limits": {
                **best_limits,
                "memory_headroom_mb": deployed_limits["memory_headroom_mb"],
                "tmp_headroom_mb": deployed_limits["tmp_headroom_mb"],
            },
        }
        camera_estimate_by_section = {
            int(row["section_idx"]): row
            for row in best_estimate["sections"]
        }
        for item in raster_section_items:
            item["camera_estimate"] = camera_estimate_by_section[
                int(item["section_idx"])
            ]
    else:
        raster_section_auto = compute_safe_sectioning(
            chunk_summary["total_solves"],
            degree,
            calc_n_coeffs,
            fused_params["raster_mt_threads"],
            "raster",
            include_coeff=solve_score_uses_coeff,
            include_param=solve_score_uses_param,
        )
        fused_params["raster_section_count_auto"] = raster_section_auto["computed_section_count"]
        selected_raster_sections = fused_params["raster_section_count"]
        if fused_params["raster_section_mode"] == "logical_sections_auto":
            selected_raster_sections = raster_section_auto["computed_section_count"]
        elif selected_raster_sections in ("", None):
            selected_raster_sections = raster_section_auto["min_safe_sections"]
        if int(selected_raster_sections) < int(raster_section_auto["min_safe_sections"]):
            raise RuntimeError(
                f"raster_section_count={selected_raster_sections} is below the safe minimum "
                f"{raster_section_auto['min_safe_sections']}"
            )
        fused_params["raster_section_count"] = int(selected_raster_sections)
        raster_section_items = build_logical_section_items(
            chunk_items,
            section_count=fused_params["raster_section_count"],
            degree=degree,
            n_coeffs=calc_n_coeffs,
            include_coeff=solve_score_uses_coeff,
            include_param=solve_score_uses_param,
        )
    raster_map_items = _compact_section_ranges(
        raster_section_items,
        include_camera_estimate=True,
    )

    solve_score_clip_key = _solve_score_scratch_key(
        job_id,
        solve_score_compiled,
        fused_params.get("root_transforms", []),
        score_normalize=solve_score_normalize,
        root_program_fingerprint=root_program_payload.get("fingerprint", ""),
    )
    solve_score = {
        "enabled": True,
        "threads": fused_params["solve_score_threads"],
        "chain": solve_score_chain_public,
        "source_text": solve_score_compiled.get("source_text", ""),
        "clip_key": solve_score_clip_key,
        "uses_lag": bool(solve_score_compiled.get("uses_lag")),
        "max_lag": int(solve_score_compiled.get("max_lag") or 0),
        "prelude_by_source": solve_score_prelude,
        "normalize": solve_score_normalize,
    }

    finalize = {
        "workers": fused_params["finalize_workers"],
    }

    raster = {
        "requested_engine": "mt",
        "requested_threads": fused_params["raster_mt_threads"],
        "requested_workers": fused_params["raster_workers"],
        "requested_input_mode": "sectioned",
        "requested_sectioned_retries": fused_params["raster_sectioned_retries"],
        "requested_section_mode": fused_section_mode,
        "requested_section_count": fused_params["raster_section_count"],
        "item_count": len(raster_map_items),
        "section_item_count": len(raster_section_items),
        "prelude_rows": int(solve_score_prelude["slv"]),
        "score_coeff_prelude_rows": int(solve_score_prelude["cf"]),
        "score_param_prelude_rows": int(solve_score_prelude["pm"]),
        "threads": fused_params["raster_mt_threads"],
        "workers": fused_params["raster_workers"],
        "engine": "mt",
        "input_mode": "sectioned",
        "sectioned_retries": fused_params["raster_sectioned_retries"],
        "section_mode": fused_section_mode,
        "section_count": fused_params["raster_section_count"],
        "section_count_auto": raster_section_auto["computed_section_count"],
        "section_budget_bytes": raster_section_auto["budget_bytes"],
        "section_memory_mb": raster_section_auto["memory_mb"],
        "section_fixed_bytes": raster_section_auto.get("fixed_bytes", 0),
        "section_row_bytes": raster_section_auto.get("row_bytes", 0),
        "section_min_safe_count": raster_section_auto["min_safe_sections"],
        "logical_section": True,
        "map_items": raster_map_items,
        "function_name": RASTER_MT_FUNCTION,
        "emit_raw_score_bins": True,
        "eligible": True,
        "reason": "fused_solve_score",
    }
    if camera_admission:
        raster["camera_admission"] = camera_admission

    artifact_family = "views" if is_view else "color"
    artifact_id = f"view_{run_id}" if is_view else f"color_{run_id}"
    artifact_prefix = f"renders/{job_id}/{artifact_family}/{artifact_id}/"
    created_at = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    ext = "png" if fused_params.get("fmt", "jpeg") == "png" else "jpeg"
    associated_palette = {
        "enabled": False,
        "mode": "none",
        "palette_id": "",
        "display_name": "",
        "image_key": "",
        "preview_key": "",
        "meta_key": "",
        "raw_key": "",
        "raw_meta_key": "",
        "fragment_prefix": "",
        "source_color_artifact_id": "",
        "metric": "",
        "palette": "",
        "quantile": None,
        "omega": None,
        "omega_enabled": True,
        "score_chain": "",
        "color_interpretation": "",
    }
    if fused_params["save_associated_palette"]:
        assoc_mode = _associated_palette_mode_for_output(
            solve_score_output_interpretation,
            solve_score_output_channel_count,
        )
        assoc_palette_id = f"pal_{artifact_id}"
        assoc_prefix = f"renders/{job_id}/palettes/{assoc_palette_id}/"
        assoc_uses_palette = _color_interpretation_uses_palette(solve_score_output_interpretation)
        assoc_palette_name = palette if assoc_uses_palette else ""
        assoc_palette_label = palette_display_name or assoc_palette_name
        associated_palette = {
            "enabled": True,
            "mode": assoc_mode,
            "palette_id": assoc_palette_id,
            "display_name": _associated_palette_display_name(
                solve_score_chain_internal,
                solve_metric,
                solve_score_quantile,
                assoc_palette_label or solve_score_output_interpretation.upper(),
            ),
            "image_key": assoc_prefix + "image.jpeg",
            "preview_key": assoc_prefix + "preview.png",
            "meta_key": assoc_prefix + "meta.json",
            "raw_key": assoc_prefix + "greyscale.raw",
            "raw_meta_key": assoc_prefix + "greyscale.meta.json",
            "fragment_prefix": assoc_prefix + "fragments/section_",
            "source_color_artifact_id": artifact_id,
            "metric": solve_metric,
            "palette": assoc_palette_name,
            "palette_display_name": palette_display_name if assoc_uses_palette else "",
            "quantile": solve_score_quantile,
            "omega": solve_score_omega,
            "omega_enabled": solve_score_omega_enabled,
            "score_chain": solve_score_chain_public,
            "color_interpretation": solve_score_output_interpretation,
        }

    render_execution = _fused_render_execution_config(fused_params)
    solve_source_manifest_ref = write_solve_source_manifest(
        s3,
        BUCKET,
        solve_source_manifest,
        job_id=job_id,
        run_id=run_id,
        suffix="color_solve_source_manifest",
    )

    artifact_meta = {
        "artifact_id": artifact_id,
        "family": artifact_family,
        "created_at": created_at,
        "degree": str(degree),
        "pix": str(pix),
        "view_mode": str(fused_params.get("view_mode", "auto")),
        "quantile": str(fused_params.get("quantile", 0.0)),
        "shim": str(fused_params.get("shim", 0.05)),
        "square_extent": str(fused_params.get("square_extent", 2.0)),
        "min_re": str(viewport["min_re"]),
        "max_re": str(viewport["max_re"]),
        "min_im": str(viewport["min_im"]),
        "max_im": str(viewport["max_im"]),
        "rotation": str(fused_params.get("rotation", 0.0)),
        "render_execution": json.dumps(render_execution, separators=(",", ":")),
        "format": "jpeg" if fused_params.get("fmt", "jpeg") != "png" else "png",
        # Views provenance: which projection this artifact IS (plan is the
        # classic top-down; other modes project roots with the selected
        # parameter axis to produce architecture views of the sculpture)
        "view_projection": str(fused_params.get("view_projection") or "plan"),
        "view_vertical": str(fused_params.get("view_vertical") or "t2"),
        "quality": str(fused_params.get("quality", 90)),
        "color_mode": "solve_score",
        "solve_score_normalize": "true" if solve_score_normalize else "false",
        "score_output_channel_count": str(solve_score_output_channel_count),
        "score_output_interpretation": solve_score_output_interpretation,
        "raw_channels": str(solve_score_output_channel_count),
        "raw_layout": "u8_scalar_row_major" if solve_score_output_channel_count == 1 else "u8_packed_channels_row_major",
        "color_interpretation": solve_score_output_interpretation,
        "render_warnings": json.dumps(render_warnings, separators=(",", ":")),
        "match_mode": "none",
        "palette": palette,
        "palette_display_name": palette_display_name,
        "background_color": background_color,
        "background_threshold": str(DEFAULT_BACKGROUND_THRESHOLD),
        "repalette_capable": "true",
        "rgb_source": (
            "raw_score_bins"
            if solve_score_output_channel_count == 1
            else f"{solve_score_output_interpretation}_raw"
        ),
        "raw_key": artifact_prefix + "greyscale.raw",
        "raw_meta_key": artifact_prefix + "greyscale.meta.json",
        "fragment_prefix": artifact_prefix + "fragments/section_",
    }
    if is_view:
        artifact_meta.update({
            "version": "2",
            "view_id": artifact_id,
            "job_id": job_id,
            "source_artifact_id": source_color_artifact_id,
            "projection": fused_params["view_projection"],
            "vertical": fused_params["view_vertical"],
            "lattice_n": str(full_n),
            "image_key": artifact_prefix + f"image.{ext}",
            "preview_key": artifact_prefix + "preview.png",
            "prefix": artifact_prefix,
        })
        if fused_params["view_projection"] == "camera":
            artifact_meta.update({
                "source_sculpture_id": source_sculpture_id,
                "camera_snapshot_version": str(camera["version"]),
                "view_camera": json.dumps(camera, sort_keys=True, separators=(",", ":")),
                "view_style": "solid_points",
            })
    artifact_meta.update(_root_program_metadata(root_program_payload))
    artifact_meta.update(
        emit_solve_score_metadata(
            "solve",
            metric=solve_metric,
            quantile=solve_score_quantile,
            omega=solve_score_omega,
            omega_enabled=solve_score_omega_enabled,
            chain=solve_score_chain_internal,
            include_legacy_scalars=False,
        )
    )
    artifact_meta["score_program"] = solve_score_compiled["program_spec"]
    artifact_meta["solve_score_program_source_text"] = solve_score_compiled.get("source_text", "")
    artifact_meta["score_output_channels"] = json.dumps(
        solve_score_output_channels,
        separators=(",", ":"),
    )
    if associated_palette["enabled"]:
        artifact_meta.update({
            "associated_palette_mode": associated_palette["mode"],
            "associated_palette_id": associated_palette["palette_id"],
            "associated_palette_display_name": associated_palette["display_name"],
            "associated_palette_image_key": associated_palette["image_key"],
            "associated_palette_preview_key": associated_palette["preview_key"],
            "associated_palette_palette": str(associated_palette["palette"]),
            "associated_palette_color_interpretation": str(associated_palette["color_interpretation"]),
        })
        artifact_meta.update(
            emit_solve_score_metadata(
                "associated_palette",
                metric=associated_palette["metric"],
                quantile=associated_palette["quantile"],
                omega=associated_palette["omega"],
                omega_enabled=associated_palette["omega_enabled"],
                chain=associated_palette["score_chain"],
            )
        )

    plan_params_digest = _plan_params_digest(
        viewport=viewport,
        pix=pix,
        root_transforms=fused_params.get("root_transforms", []),
        root_program_fingerprint=root_program_payload.get("fingerprint", ""),
        solve_score_normalize=solve_score_normalize,
        color_interpretation=solve_score_output_interpretation,
        background_color=background_color,
        view_projection=fused_params["view_projection"],
        view_vertical=fused_params["view_vertical"],
        view_camera=fused_params["view_camera"],
    )
    outputs = {
        "family": artifact_family,
        "artifact_id": artifact_id,
        "artifact_prefix": artifact_prefix,
        "created_at": created_at,
        "image_key": artifact_prefix + f"image.{ext}",
        "preview_key": artifact_prefix + "preview.png",
        "meta_key": artifact_prefix + "meta.json",
        "raw_key": artifact_prefix + "greyscale.raw",
        "raw_meta_key": artifact_prefix + "greyscale.meta.json",
        "fragment_prefix": artifact_prefix + "fragments/section_",
        "plan_params_digest": plan_params_digest,
        "metadata": artifact_meta,
        "repalette_capable": True,
        "warnings": render_warnings,
    }

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "color",
        "params": fused_params,
        "viewport": viewport,
        "calc": {
            "degree": degree,
            "N": full_n,
            "times": times,
            "n_chunks": len(chunk_items),
            "lores_bin_key": calc.get("lores", {}).get("bin_key", ""),
            "lores_coeffs_key": _fallback_lores_coeffs_key(job_id, calc),
            "lores_params_key": _fallback_lores_params_key(job_id, calc),
            "n_coeffs": calc_n_coeffs,
        },
        "grid": {
            "pix": pix,
        },
        "solve_source_manifest": {},
        "solve_source_manifest_key": solve_source_manifest_ref["key"],
        "solve_source_manifest_bytes": solve_source_manifest_ref["bytes"],
        "physical_source_items": [],
        "solve_score": solve_score,
        "finalize": finalize,
        "raster": raster,
        "fragment_contract": render_fragment_contract,
        "associated_palette": associated_palette,
        "render_execution": render_execution,
        "outputs": outputs,
        "warnings": render_warnings,
    }
    return plan


def _sparse_section_plan(chunk_items, *, chunk_summary, degree, calc_n_coeffs, rp, source_family):
    if source_family not in ("slv", "cf"):
        raise RuntimeError(f"unsupported sparse section source family: {source_family!r}")
    include_coeff = source_family == "cf"
    if include_coeff and not all(str(item.get("coeffs_key") or "").strip() for item in (chunk_items or [])):
        raise RuntimeError("coeff bilevel logical sections require coeffs_key on every chunk")
    section_auto = compute_safe_sectioning(
        chunk_summary["total_solves"],
        0 if include_coeff else degree,
        calc_n_coeffs,
        1,
        "raster",
        include_coeff=include_coeff,
        include_param=False,
    )
    requested_mode = normalize_section_mode(rp.get("raster_section_mode", "logical_sections_auto"))
    if requested_mode == "physical_chunks":
        requested_mode = "logical_sections_auto"
    requested_count = validate_section_count(
        rp.get("raster_section_count", ""),
        "raster_section_count",
        default="",
    )
    if not chunk_summary["chunk_step_metadata_complete"]:
        label = "coeff bilevel" if include_coeff else "bilevel"
        raise RuntimeError(f"{label} logical sections require chunk step metadata on every chunk")

    selected_count = requested_count
    if requested_mode == "logical_sections_auto":
        selected_count = section_auto["computed_section_count"]
    elif selected_count in ("", None):
        selected_count = section_auto["min_safe_sections"]
    if int(selected_count) < int(section_auto["min_safe_sections"]):
        raise RuntimeError(
            f"raster_section_count={selected_count} is below the safe minimum "
            f"{section_auto['min_safe_sections']}"
        )
    section_items = build_logical_section_items(
        chunk_items,
        section_count=int(selected_count),
        degree=0 if include_coeff else degree,
        n_coeffs=calc_n_coeffs,
        include_coeff=include_coeff,
        include_param=False,
    )
    return {
        "section_mode": requested_mode,
        "section_count": int(selected_count),
        "section_count_auto": section_auto["computed_section_count"],
        "section_budget_bytes": section_auto["budget_bytes"],
        "section_memory_mb": section_auto["memory_mb"],
        "section_min_safe_count": section_auto["min_safe_sections"],
        "logical_section": True,
        "item_count": len(section_items),
        "section_items": _compact_section_ranges(section_items),
    }


def _bilevel_section_plan(chunk_items, *, chunk_summary, degree, calc_n_coeffs, rp):
    return _sparse_section_plan(
        chunk_items,
        chunk_summary=chunk_summary,
        degree=degree,
        calc_n_coeffs=calc_n_coeffs,
        rp=rp,
        source_family="slv",
    )


def _coeff_bilevel_section_plan(chunk_items, *, chunk_summary, degree, calc_n_coeffs, rp):
    return _sparse_section_plan(
        chunk_items,
        chunk_summary=chunk_summary,
        degree=degree,
        calc_n_coeffs=calc_n_coeffs,
        rp=rp,
        source_family="cf",
    )


def _build_non_color_plan(
    *,
    job_id,
    run_id,
    task_id,
    mode,
    rp,
    viewport,
    calc,
    degree,
    calc_n_coeffs,
    full_n,
    times,
    chunk_items,
    chunk_summary,
):
    if mode not in ("bilevel", "coeff_bilevel"):
        raise RuntimeError(f"unsupported non-color mode: {mode!r}")

    non_color_params = {
        "pix": rp["pix"],
        "view_mode": rp.get("view_mode", "auto"),
        "quantile": rp.get("quantile", 0.0),
        "shim": rp.get("shim", 0.05),
        "square_extent": rp.get("square_extent", 2.0),
        "root_transforms": rp.get("root_transforms", []),
        "root_program_source_text": rp.get("root_program_source_text", ""),
        "root_program": rp.get("root_program"),
        "rotation": rp.get("rotation", 0),
    }
    root_program_payload = _apply_root_program_to_params(non_color_params)
    non_color_params["raster_section_mode"] = normalize_section_mode(
        rp.get("raster_section_mode", "logical_sections_auto")
    )
    non_color_params["raster_section_count"] = validate_section_count(
        rp.get("raster_section_count", ""),
        "raster_section_count",
        default="",
    )

    pix = int(non_color_params["pix"])

    bilevel = {
        "enabled": mode == "bilevel",
        "section_mode": "",
        "section_count": "",
        "section_count_auto": "",
        "section_budget_bytes": 0,
        "section_memory_mb": 0,
        "section_min_safe_count": 0,
        "logical_section": False,
        "item_count": 0,
        "section_items": [],
        "fragment_prefix": "",
    }
    coeff_bilevel = {
        "enabled": mode == "coeff_bilevel",
        "section_mode": "",
        "section_count": "",
        "section_count_auto": "",
        "section_budget_bytes": 0,
        "section_memory_mb": 0,
        "section_min_safe_count": 0,
        "logical_section": False,
        "item_count": 0,
        "section_items": [],
        "fragment_prefix": "",
    }
    if mode == "bilevel":
        bilevel.update(
            _bilevel_section_plan(
                chunk_items,
                chunk_summary=chunk_summary,
                degree=degree,
                calc_n_coeffs=calc_n_coeffs,
                rp=non_color_params,
            )
        )
        bilevel["fragment_prefix"] = f"renders/{job_id}/bilevel_section_"
        non_color_params["raster_section_count_auto"] = bilevel["section_count_auto"]
        non_color_params["raster_section_count"] = bilevel["section_count"]
    elif mode == "coeff_bilevel":
        coeff_bilevel.update(
            _coeff_bilevel_section_plan(
                chunk_items,
                chunk_summary=chunk_summary,
                degree=degree,
                calc_n_coeffs=calc_n_coeffs,
                rp=non_color_params,
            )
        )
        coeff_bilevel["fragment_prefix"] = f"renders/{job_id}/coeff_bilevel_section_"
        non_color_params["raster_section_count_auto"] = coeff_bilevel["section_count_auto"]
        non_color_params["raster_section_count"] = coeff_bilevel["section_count"]

    solve_source_manifest = build_solve_source_manifest(
        chunk_items,
        job_id=job_id,
        degree=degree,
        n_coeffs=calc_n_coeffs,
        include_solve=mode == "bilevel",
        include_coeff=mode == "coeff_bilevel",
        include_param=False,
    )
    solve_source_manifest_ref = write_solve_source_manifest(
        s3,
        BUCKET,
        solve_source_manifest,
        job_id=job_id,
        run_id=run_id,
        suffix=f"{mode}_solve_source_manifest",
    )

    artifact_family = "coeffs" if mode == "coeff_bilevel" else mode
    artifact_id = f"{artifact_family}_{run_id}"
    artifact_prefix = f"renders/{job_id}/{artifact_family}/{artifact_id}/"
    created_at = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    render_execution = {}
    artifact_meta = {
        "artifact_id": artifact_id,
        "family": artifact_family,
        "created_at": created_at,
        "degree": str(degree),
        "pix": str(pix),
        "view_mode": str(non_color_params.get("view_mode", "auto")),
        "quantile": str(non_color_params.get("quantile", 0.0)),
        "shim": str(non_color_params.get("shim", 0.05)),
        "square_extent": str(non_color_params.get("square_extent", 2.0)),
        "min_re": str(viewport["min_re"]),
        "max_re": str(viewport["max_re"]),
        "min_im": str(viewport["min_im"]),
        "max_im": str(viewport["max_im"]),
        "rotation": str(non_color_params.get("rotation", 0.0)),
        "render_execution": json.dumps(render_execution, separators=(",", ":")),
    }
    artifact_meta.update(_root_program_metadata(root_program_payload))
    if mode == "bilevel":
        artifact_meta.update({
            "format": "tif",
            "mode": "bilevel",
            "bilevel_pipeline": BILEVEL_SPARSE_PIPELINE,
            "bilevel_section_mode": str(bilevel["section_mode"]),
            "bilevel_section_count": str(bilevel["section_count"]),
        })
    else:
        artifact_meta.update({
            "format": "tif",
            "mode": "coeffs",
            "bilevel_pipeline": BILEVEL_SPARSE_PIPELINE,
            "bilevel_section_mode": str(coeff_bilevel["section_mode"]),
            "bilevel_section_count": str(coeff_bilevel["section_count"]),
        })

    plan_params_digest = _plan_params_digest(
        viewport=viewport,
        pix=pix,
        root_transforms=non_color_params.get("root_transforms", []),
        root_program_fingerprint=root_program_payload.get("fingerprint", ""),
    )
    outputs = {
        "family": artifact_family,
        "artifact_id": artifact_id,
        "artifact_prefix": artifact_prefix,
        "created_at": created_at,
        "image_key": artifact_prefix + "image.tif",
        "preview_key": artifact_prefix + "preview.png",
        "meta_key": artifact_prefix + "meta.json",
        "raw_key": "",
        "raw_meta_key": "",
        "fragment_prefix": "",
        "plan_params_digest": plan_params_digest,
        "bilevel_key": artifact_prefix + "image.tif",
        "coeff_bilevel_key": artifact_prefix + "image.tif",
        "metadata": artifact_meta,
        "repalette_capable": False,
    }

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": mode,
        "params": non_color_params,
        "viewport": viewport,
        "calc": {
            "degree": degree,
            "N": full_n,
            "times": times,
            "n_chunks": len(chunk_items),
            "n_coeffs": calc_n_coeffs,
        },
        "grid": {
            "pix": pix,
        },
        "solve_source_manifest": {},
        "solve_source_manifest_key": solve_source_manifest_ref["key"],
        "solve_source_manifest_bytes": solve_source_manifest_ref["bytes"],
        "physical_source_items": [],
        "bilevel": bilevel,
        "coeff_bilevel": coeff_bilevel,
        "render_execution": render_execution,
        "outputs": outputs,
    }
    return plan


def _plan_size_error_message(plan, plan_size):
    raster = dict(plan.get("raster") or {})
    bilevel = dict(plan.get("bilevel") or {})
    coeff_bilevel = dict(plan.get("coeff_bilevel") or {})
    calc = dict(plan.get("calc") or {})
    return (
        f"Plan too large: {plan_size} bytes > {MAX_PLAN_BYTES} limit. "
        f"Counts: chunks={int(calc.get('n_chunks') or 0)}, "
        f"raster_items={int(raster.get('item_count') or 0)}, "
        f"bilevel_sections={int(bilevel.get('item_count') or 0)}, "
        f"coeff_sections={int(coeff_bilevel.get('item_count') or 0)}. "
        f"Controls: reduce pix or lower logical section counts."
    )


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    task_id = params["task_id"]
    mode = params["mode"]
    rp = dict(params.get("params", {}) or {})
    _require_pix_only_grid_params(rp)

    # Clean previous render intermediates
    pipeline = "color" if mode == "color" else mode
    _storage_call("/clean-render", {"job_id": job_id, "pipeline": pipeline})

    # Load calc metadata
    calc = _load_calc(job_id)

    # Compute viewport
    viewport = _compute_viewport(job_id, rp)

    # Extract calc fields
    degree = _coerce_degree(calc)
    calc_n_coeffs = _coerce_n_coeffs(calc, degree)
    full_n = int(calc.get("N", calc.get("n1", 0)) or 0)
    times = int(calc.get("times", 1) or 1)
    chunk_items = build_chunk_items(calc, job_id)
    n_chunks = len(chunk_items)
    chunk_summary = summarize_chunk_items(chunk_items, degree, calc_n_coeffs)

    if mode == "color":
        plan = _build_fused_color_plan(
            job_id=job_id,
            run_id=run_id,
            task_id=task_id,
            rp=rp,
            viewport=viewport,
            calc=calc,
            degree=degree,
            calc_n_coeffs=calc_n_coeffs,
            full_n=full_n,
            times=times,
            chunk_items=chunk_items,
            chunk_summary=chunk_summary,
        )
        plan_json = json.dumps(plan)
        if len(plan_json) > MAX_PLAN_BYTES:
            raise RuntimeError(_plan_size_error_message(plan, len(plan_json)))
        return ok_response(plan)
    plan = _build_non_color_plan(
        job_id=job_id,
        run_id=run_id,
        task_id=task_id,
        mode=mode,
        rp=rp,
        viewport=viewport,
        calc=calc,
        degree=degree,
        calc_n_coeffs=calc_n_coeffs,
        full_n=full_n,
        times=times,
        chunk_items=chunk_items,
        chunk_summary=chunk_summary,
    )
    plan_json = json.dumps(plan)
    if len(plan_json) > MAX_PLAN_BYTES:
        raise RuntimeError(_plan_size_error_message(plan, len(plan_json)))
    return ok_response(plan)


def _load_calc(job_id):
    """Load calc.json from S3."""
    resp = _storage_call("/detail", {"job_id": job_id})
    calc = resp.get("calc", {})
    if not calc:
        raise RuntimeError(f"calc.json missing for {job_id}")
    return calc


def _compute_viewport(job_id, rp):
    """Compute viewport from params."""
    view_mode = str(rp.get("view_mode") or "auto").strip().lower()
    if view_mode == "explicit":
        return _explicit_viewport_from_params(rp)
    if view_mode == "square":
        ext = _coerce_finite_float(rp.get("square_extent", 2.0), "square_extent")
        if ext <= 0.0:
            raise RuntimeError(f"square_extent must be > 0, got {ext}")
        return {
            "min_re": -ext,
            "max_re": ext,
            "min_im": -ext,
            "max_im": ext,
        }
    if view_mode != "auto":
        raise RuntimeError(f"unsupported view_mode: {view_mode!r}")

    shim = _coerce_finite_float(rp.get("shim", 0.05), "shim")
    vp = _invoke_sync(VIEWPORT_FUNCTION, {
        "job_id": job_id,
        "quantile": rp.get("quantile", 0.0),
        "shim": shim,
    })
    q_re = vp.get("q_re")
    q_im = vp.get("q_im")
    if not (isinstance(q_re, (list, tuple)) and len(q_re) == 2):
        raise RuntimeError("viewport lambda response missing q_re bounds")
    if not (isinstance(q_im, (list, tuple)) and len(q_im) == 2):
        raise RuntimeError("viewport lambda response missing q_im bounds")
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
            # When one axis collapses to a line, keep auto mode bounded by
            # inheriting the non-degenerate span instead of exploding to a
            # full REF_SIZE-derived camera box.
            fallback_span = positive_span
        else:
            fallback_scale_ref = _coerce_finite_float(vp.get("scale_ref", vp.get("scale")), "scale_ref")
            if fallback_scale_ref <= 0.0:
                raise RuntimeError(f"viewport lambda returned non-positive scale_ref: {fallback_scale_ref!r}")
            fallback_span = float(REF_SIZE) / fallback_scale_ref
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


def _invoke_sync(function_name, payload):
    """Invoke a Lambda synchronously and return parsed response."""
    r = lambda_client.invoke(
        FunctionName=function_name,
        InvocationType="RequestResponse",
        Payload=json.dumps(payload).encode(),
    )
    body = json.loads(r["Payload"].read())
    if isinstance(body, dict) and "body" in body:
        return json.loads(body["body"])
    return body


def _storage_call(path, body):
    """Call the storage Lambda synchronously."""
    return _invoke_sync(STORAGE_FUNCTION, {
        "body": json.dumps(body),
        "path": path,
        "requestContext": {"http": {"method": "POST", "path": path}},
    })
