"""
Fused color raster MT Lambda handler.

This handler is the shipped color raster path:

- fused solve-score only
- logical sections only
- multispan sectioned reads only
- raw-score fragment output only
"""

import glob
import json
import math
import os
import re
import subprocess
import time

import boto3

from logical_sections import (
    build_native_manifest_urls,
    build_native_multispan_manifest,
    build_source_spans,
    resolve_solve_source_manifest,
    write_native_multispan_manifest,
)
from solve_score_chain import (
    canonicalize_solve_score_program_spec,
    compile_solve_score_chain,
    compiled_solve_score_fingerprint,
    solve_score_lag_prelude_by_source,
    solve_score_program_cli_payload,
)
from shared import (
    BUCKET,
    attach_contract_warnings,
    contract_param,
    ok_response,
    parse_body,
    parse_boolish,
    report_status,
)

s3 = boto3.client("s3")
ROOTS2PIX_MT = os.path.join(os.path.dirname(__file__), "roots2pix_mt")
DEFAULT_THREADS = int(os.environ.get("RASTER_MT_THREADS", "4") or "4")
VALID_RASTER_INPUT_MODES = {"sectioned"}
_TMP_INPUT_MANIFEST = "/tmp/raster_input_manifest.json"
_TMP_SCORE_COEFFS_MANIFEST = "/tmp/raster_score_coeffs_manifest.json"
_TMP_SCORE_PARAMS_MANIFEST = "/tmp/raster_score_params_manifest.json"


def _validate_threads(value):
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"raster_mt_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"raster_mt_threads must be in [1, 16], got {threads}")
    return threads


def _validate_raster_input_mode(value):
    mode = str(value or "sectioned").strip().lower()
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


def _cleanup_tmp():
    for pattern in (
        "/tmp/fused_fragment.frag",
        "/tmp/palette_fragment.frag",
        "/tmp/step_scores.bin",
        _TMP_INPUT_MANIFEST,
        _TMP_SCORE_COEFFS_MANIFEST,
        _TMP_SCORE_PARAMS_MANIFEST,
        "/tmp/root_xforms.json",
    ):
        for stale in glob.glob(pattern):
            try:
                os.remove(stale)
            except OSError:
                pass


def _solve_score_program_args(score_artifact):
    payload = solve_score_program_cli_payload({
        "metrics": score_artifact.get("metrics") or [],
        "program_spec": str(score_artifact.get("program") or ""),
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


def _solve_score_output_args(score_artifact):
    enabled = parse_boolish(
        score_artifact.get("score_output_normalize", False),
        False,
        strict=True,
        label="score_output_normalize",
    )
    clip_lo = _coerce_finite_float(score_artifact.get("score_output_clip_lo", 0.0), "score_output_clip_lo")
    clip_hi = _coerce_finite_float(score_artifact.get("score_output_clip_hi", 1.0), "score_output_clip_hi")
    return [
        f"--score_output_normalize={1 if enabled else 0}",
        f"--score_output_clip_lo={clip_lo}",
        f"--score_output_clip_hi={clip_hi}",
    ] + _solve_score_output_channel_args(score_artifact)


def _solve_score_output_channel_args(score_artifact):
    channels = list(score_artifact.get("score_output_channels") or [])
    if not channels:
        return []
    try:
        channel_count = int(score_artifact.get("score_output_channel_count") or len(channels))
    except (TypeError, ValueError):
        raise RuntimeError(
            f"score_output_channel_count must be integer, got {score_artifact.get('score_output_channel_count')!r}"
        )
    if channel_count != len(channels):
        raise RuntimeError(
            f"score output channel count mismatch: count={channel_count}, channels={len(channels)}"
        )
    los = []
    his = []
    for idx, row in enumerate(channels):
        if not isinstance(row, dict):
            raise RuntimeError(f"score_output_channels[{idx}] must be an object")
        los.append(str(_coerce_finite_float(row.get("clip_lo", 0.0), f"score_output_channels[{idx}].clip_lo")))
        his.append(str(_coerce_finite_float(row.get("clip_hi", 1.0), f"score_output_channels[{idx}].clip_hi")))
    return [
        f"--score_output_clip_los={','.join(los)}",
        f"--score_output_clip_his={','.join(his)}",
    ]


def _solve_score_artifact_uses_source(score_artifact, source):
    for metric in (score_artifact.get("metrics") or []):
        if str(metric.get("source", "slv")).strip().lower() == source:
            return True
    return False


def _build_cmd(params):
    effective_input_mode = str(params.get("effective_input_mode") or "").strip().lower()
    if effective_input_mode != "multispan_sectioned":
        raise RuntimeError(
            "fused raster requires effective_input_mode='multispan_sectioned', "
            f"got {effective_input_mode!r}"
        )

    input_manifest_path = str(params.get("input_manifest_path") or "").strip()
    if not input_manifest_path:
        raise RuntimeError("fused raster requires input_manifest_path")

    viewport = _viewport_bounds(params)
    cmd = [
        ROOTS2PIX_MT,
        "/tmp/pix",
        f"--pix={params['pix']}",
        f"--min_re={viewport['min_re']}",
        f"--max_re={viewport['max_re']}",
        f"--min_im={viewport['min_im']}",
        f"--max_im={viewport['max_im']}",
        f"--degree={params['degree']}",
        f"--rotation={params.get('rotation', 0.0)}",
        f"--threads={params['raster_mt_threads']}",
        f"--input_manifest={input_manifest_path}",
        f"--step_count={int(params['step_count'])}",
        f"--prelude_rows={int(params.get('prelude_rows') or 0)}",
        f"--score_coeff_prelude_rows={int(params.get('score_coeff_prelude_rows') or 0)}",
        f"--score_param_prelude_rows={int(params.get('score_param_prelude_rows') or 0)}",
        f"--retries={params.get('raster_sectioned_retries', 2)}",
        "--fragment_prefix=/tmp/fused_fragment",
    ]

    if params.get("emit_associated_palette_bins"):
        cmd.append("--associated_palette_fragment_prefix=/tmp/palette_fragment")
        cmd.append(f"--palette_grid_n={int(params['associated_palette_grid_n'])}")
        cmd.append(f"--palette_step_start={int(params['step_start'])}")

    score_artifact = dict(params.get("solve_score_bins_data") or {})
    if score_artifact.get("family") != "solve_score":
        raise RuntimeError(f"solve-score clip artifact missing or wrong family: {score_artifact.get('family')}")
    if int(score_artifact.get("version", 1) or 1) < 2:
        raise RuntimeError("fused raster requires v2 solve-score clip metadata")
    compiled = params.get("solve_score_compiled")
    chain_fingerprint = str(params.get("solve_score_chain_fingerprint") or "").strip()
    if not score_artifact.get("program") and params.get("solve_score_chain_present") and compiled is not None:
        score_artifact["program"] = compiled["program_spec"]
    if not score_artifact.get("program") or not isinstance(score_artifact.get("metrics"), list) or not score_artifact.get("metrics"):
        raise RuntimeError("fused raster requires clip artifact program + metrics")
    output_channel_count = int(score_artifact.get("score_output_channel_count") or 1)
    if output_channel_count < 1:
        raise RuntimeError(f"score_output_channel_count must be >= 1, got {output_channel_count}")
    if output_channel_count in (1, 3):
        cmd.append("--step_scores_output=/tmp/step_scores.bin")

    if params.get("solve_score_chain_present") and compiled is not None:
        actual_fingerprint = str(score_artifact.get("chain_fingerprint") or "").strip()
        if not actual_fingerprint:
            raise RuntimeError("solve-score clip artifact missing chain_fingerprint")
        if chain_fingerprint and actual_fingerprint != chain_fingerprint:
            raise RuntimeError(
                f"solve-score clip artifact fingerprint mismatch: expected {chain_fingerprint}, got {actual_fingerprint}"
            )
        if canonicalize_solve_score_program_spec(score_artifact.get("program") or "") != compiled["program_spec"]:
            raise RuntimeError(
                f"solve-score clip artifact program mismatch: expected {compiled['program_spec']}, got {score_artifact.get('program')!r}"
            )
        if len(score_artifact["metrics"]) != compiled["metric_count"]:
            raise RuntimeError(
                f"solve-score clip artifact metric slot count mismatch: expected {compiled['metric_count']}, "
                f"got {len(score_artifact['metrics'])}"
            )
        for expected, actual in zip(compiled["metrics"], score_artifact["metrics"]):
            if str(actual.get("metric") or "") != expected["metric"]:
                raise RuntimeError(
                    f"solve-score clip artifact metric slot {expected['slot']} mismatch: "
                    f"expected {expected['metric']}, got {actual.get('metric')!r}"
                )
            if str(actual.get("source", "slv") or "slv") != expected.get("source", "slv"):
                raise RuntimeError(
                    f"solve-score clip artifact source slot {expected['slot']} mismatch: "
                    f"expected {expected.get('source', 'slv')}, got {actual.get('source', 'slv')!r}"
                )
            if float(actual.get("quantile", -1)) != float(expected["quantile"]):
                raise RuntimeError(
                    f"solve-score clip artifact quantile slot {expected['slot']} mismatch: "
                    f"expected {expected['quantile']}, got {actual.get('quantile')!r}"
                )

    cmd.extend(_solve_score_program_args(score_artifact))
    cmd.extend(_solve_score_output_args(score_artifact))

    if _solve_score_artifact_uses_source(score_artifact, "cf"):
        n_coeffs = params.get("n_coeffs")
        try:
            n_coeffs = int(n_coeffs)
        except (TypeError, ValueError):
            raise RuntimeError(f"mixed-source solve-score render requires numeric n_coeffs, got {n_coeffs!r}")
        if n_coeffs < 1:
            raise RuntimeError(f"mixed-source solve-score render requires n_coeffs >= 1, got {n_coeffs}")
        coeff_manifest_path = str(params.get("score_coeff_manifest_path") or "").strip()
        if not coeff_manifest_path:
            raise RuntimeError("mixed-source fused raster requires score_coeff_manifest_path")
        cmd.extend([
            f"--score_coeff_manifest={coeff_manifest_path}",
            f"--score_coeff_degree={n_coeffs}",
        ])

    if _solve_score_artifact_uses_source(score_artifact, "pm"):
        params_manifest_path = str(params.get("score_params_manifest_path") or "").strip()
        if not params_manifest_path:
            raise RuntimeError("param-source fused raster requires score_params_manifest_path")
        cmd.append(f"--score_params_manifest={params_manifest_path}")

    rt_path = params.get("root_xforms_path")
    if rt_path:
        cmd.append(f"--root_xforms={rt_path}")
    return cmd


def _prepare_fused_section_inputs(section_params):
    solve_source_manifest = resolve_solve_source_manifest(
        section_params,
        s3,
        BUCKET,
        required_context="fused raster",
    )
    if not solve_source_manifest:
        raise RuntimeError("fused raster requires solve_source_manifest_key")

    ss_data = dict(section_params.get("solve_score_bins_data") or {})
    step_start = int(section_params.get("step_start") or 0)
    step_count = int(section_params.get("step_count") or 0)
    if step_start < 0 or step_count < 1:
        raise RuntimeError(f"fused raster requires step_start >= 0 and step_count >= 1, got {step_start}/{step_count}")
    slv_prelude = 1 if int(section_params.get("prelude_rows") or 0) > 0 and step_start > 0 else 0
    coeff_prelude = 1 if int(section_params.get("score_coeff_prelude_rows") or 0) > 0 and step_start > 0 else 0
    param_prelude = 1 if int(section_params.get("score_param_prelude_rows") or 0) > 0 and step_start > 0 else 0
    section_params["prelude_rows"] = slv_prelude
    section_params["score_coeff_prelude_rows"] = coeff_prelude
    section_params["score_param_prelude_rows"] = param_prelude

    root_spans = build_source_spans(
        solve_source_manifest,
        source_family="slv",
        solve_start=step_start - slv_prelude,
        solve_count=step_count + slv_prelude,
    )
    if not root_spans:
        raise RuntimeError("fused raster logical section resolved to no solve spans")

    input_manifest = build_native_multispan_manifest(
        solve_source_manifest,
        source_family="slv",
        solve_start=step_start - slv_prelude,
        solve_count=step_count + slv_prelude,
        url_by_key=build_native_manifest_urls(s3, BUCKET, root_spans),
    )
    section_params["input_manifest_path"] = write_native_multispan_manifest(_TMP_INPUT_MANIFEST, input_manifest)
    section_params["effective_input_mode"] = "multispan_sectioned"
    section_params["sectioned_input_size"] = int(input_manifest["logical_size"])

    if _solve_score_artifact_uses_source(ss_data, "cf"):
        coeff_spans = build_source_spans(
            solve_source_manifest,
            source_family="cf",
            solve_start=step_start - coeff_prelude,
            solve_count=step_count + coeff_prelude,
        )
        coeff_manifest = build_native_multispan_manifest(
            solve_source_manifest,
            source_family="cf",
            solve_start=step_start - coeff_prelude,
            solve_count=step_count + coeff_prelude,
            url_by_key=build_native_manifest_urls(s3, BUCKET, coeff_spans),
        )
        section_params["score_coeff_manifest_path"] = write_native_multispan_manifest(
            _TMP_SCORE_COEFFS_MANIFEST,
            coeff_manifest,
        )

    if _solve_score_artifact_uses_source(ss_data, "pm"):
        param_spans = build_source_spans(
            solve_source_manifest,
            source_family="pm",
            solve_start=step_start - param_prelude,
            solve_count=step_count + param_prelude,
        )
        param_manifest = build_native_multispan_manifest(
            solve_source_manifest,
            source_family="pm",
            solve_start=step_start - param_prelude,
            solve_count=step_count + param_prelude,
            url_by_key=build_native_manifest_urls(s3, BUCKET, param_spans),
        )
        section_params["score_params_manifest_path"] = write_native_multispan_manifest(
            _TMP_SCORE_PARAMS_MANIFEST,
            param_manifest,
        )

    return section_params


def _fused_task_id(params):
    raw_task_id = str(params.get("task_id") or "").strip()
    if raw_task_id:
        return raw_task_id
    section_idx = params.get("section_idx")
    if section_idx in (None, ""):
        return "raster"
    return f"raster_{section_idx}"


def _report_handler_entry_error(params, message):
    params = params or {}
    job_id = str(params.get("job_id") or "").strip()
    if not job_id:
        return
    task_id = _fused_task_id(params)
    result_data = {
        "engine": "mt",
        "phase": "handler_entry",
        "phase_label": "Raster dispatch",
    }
    report_status(job_id, task_id, "error", message, result_data=result_data)


def _extract_handler_entry_params(event):
    params = {}
    if not isinstance(event, dict):
        return params
    body = event.get("body")
    if isinstance(body, dict):
        for key in ("job_id", "task_id", "section_idx"):
            value = body.get(key)
            if value not in ("", None):
                params[key] = value
        return params
    if isinstance(body, str):
        for key in ("job_id", "task_id"):
            match = re.search(rf'"{re.escape(key)}"\s*:\s*"([^"]*)"', body)
            if match and match.group(1) not in ("", None):
                params[key] = match.group(1)
        section_match = re.search(r'"section_idx"\s*:\s*([-+]?\d+)', body)
        if section_match:
            params["section_idx"] = section_match.group(1)
    for key in ("job_id", "task_id", "section_idx"):
        if key not in params:
            value = event.get(key)
            if value not in ("", None):
                params[key] = value
    return params


def _handle_fused_raster_request(params):
    contract_warnings = []
    job_id = str(params.get("job_id") or "").strip()
    task_id = _fused_task_id(params)
    perf = None
    try:
        if not job_id:
            raise RuntimeError("fused raster requires job_id")
        section_idx = params.get("section_idx")
        if section_idx is None:
            raise RuntimeError("fused raster requires section_idx")

        threads = _validate_threads(contract_param(params, "raster_mt_threads", DEFAULT_THREADS, contract_warnings))

        if "raster_input_mode" not in params:
            raise RuntimeError("fused raster requires raster_input_mode in the payload contract")
        raster_input_mode = _validate_raster_input_mode(params.get("raster_input_mode"))

        raster_sectioned_retries = _validate_sectioned_retries(
            contract_param(params, "raster_sectioned_retries", 2, contract_warnings)
        )

        if "logical_section" not in params:
            raise RuntimeError("fused raster requires logical_section in the payload contract")
        if not parse_boolish(params.get("logical_section"), False, strict=True, label="logical_section"):
            raise RuntimeError("fused raster requires logical_section=true")

        if params.get("sections"):
            raise RuntimeError("fused raster does not support grouped section payloads")

        if raster_input_mode != "sectioned":
            raise RuntimeError("fused raster requires raster_input_mode=sectioned")

        fragment_prefix = str(params.get("fragment_prefix") or "").strip()
        if not fragment_prefix:
            raise RuntimeError("fused raster requires fragment_prefix")

        associated_palette_mode = str(params.get("associated_palette_mode") or "").strip().lower()
        emit_associated_palette_bins = associated_palette_mode == "generated"
        associated_palette_fragment_prefix = str(params.get("associated_palette_fragment_prefix") or "").strip()
        associated_palette_grid_n = int(params.get("associated_palette_grid_n") or 0)
        if emit_associated_palette_bins:
            if associated_palette_grid_n < 1:
                raise RuntimeError("fused raster associated palette requires associated_palette_grid_n >= 1")
            if not associated_palette_fragment_prefix:
                raise RuntimeError("fused raster associated palette requires associated_palette_fragment_prefix")

        removed_contract_fields = []
        for key in (
            "width",
            "height",
            "color",
            "match",
            "palette",
            "solve_metric",
            "solve_score_quantile",
            "solve_score_omega",
            "solve_score_omega_enabled",
        ):
            value = params.get(key)
            if value not in ("", None):
                removed_contract_fields.append(key)
        if removed_contract_fields:
            fields = ", ".join(removed_contract_fields)
            raise RuntimeError(
                f"fused raster no longer accepts removed contract field(s): {fields}; "
                "pass pix and solve_score_chain only"
            )
        try:
            params["pix"] = int(params.get("pix"))
        except (TypeError, ValueError) as exc:
            raise RuntimeError("fused raster requires integer pix") from exc
        if params["pix"] <= 0:
            raise RuntimeError(f"fused raster requires pix > 0, got {params['pix']}")

        perf = attach_contract_warnings({
            "engine": "mt",
            "threads": threads,
            "input_mode": "sectioned",
            "retries": raster_sectioned_retries,
            "download_us": 0,
            "native_us": 0,
            "upload_us": 0,
            "fragment_files_uploaded": 0,
            "roots_plotted": 0,
            "roots_clipped": 0,
            "emit_raw_score_bins": True,
            "emit_associated_palette_bins": emit_associated_palette_bins,
            "emit_step_scores": True,
            "rgb_source": "raw_score_bins",
            "group_idx": int(section_idx),
            "section_indices": [int(section_idx)],
            "section_count": 1,
        }, contract_warnings)

        report_status(job_id, task_id, "started", result_data=perf)
        _cleanup_tmp()

        section_params = dict(params)
        section_params["raster_mt_threads"] = threads
        section_params["raster_input_mode"] = "sectioned"
        section_params["raster_sectioned_retries"] = raster_sectioned_retries
        section_params["logical_section"] = True
        section_params["rotation"] = contract_param(section_params, "rotation", 0.0, contract_warnings)
        section_params["emit_associated_palette_bins"] = emit_associated_palette_bins
        section_params["associated_palette_grid_n"] = associated_palette_grid_n
        section_params["associated_palette_fragment_prefix"] = associated_palette_fragment_prefix
        section_params["fragment_prefix"] = fragment_prefix

        rt_chain = contract_param(section_params, "root_transforms", [], contract_warnings)
        section_params["root_xforms_path"] = None
        if rt_chain:
            rt_path = "/tmp/root_xforms.json"
            with open(rt_path, "w") as rtf:
                json.dump(rt_chain, rtf)
            section_params["root_xforms_path"] = rt_path

        ss_clip_key = str(section_params.get("solve_score_clip_key") or "").strip()
        if not ss_clip_key:
            raise RuntimeError("fused raster requires solve_score_clip_key")
        ss_obj = s3.get_object(Bucket=BUCKET, Key=ss_clip_key)
        section_params["solve_score_bins_data"] = json.loads(ss_obj["Body"].read())
        step_score_channels = int(section_params["solve_score_bins_data"].get("score_output_channel_count") or 1)
        emit_step_scores = step_score_channels in (1, 3)
        perf["emit_step_scores"] = emit_step_scores
        perf["step_score_channels"] = step_score_channels if emit_step_scores else 0

        raw_chain = section_params.get("solve_score_chain", "")
        section_params["solve_score_chain_present"] = raw_chain not in ("", None, [])
        if not section_params["solve_score_chain_present"]:
            raise RuntimeError("fused raster requires solve_score_chain")
        compiled = compile_solve_score_chain(raw_chain)
        section_params["solve_score_compiled"] = compiled
        section_params["solve_score_chain_fingerprint"] = compiled_solve_score_fingerprint(compiled)
        prelude_by_source = solve_score_lag_prelude_by_source(compiled)
        for field, source in (
            ("prelude_rows", "slv"),
            ("score_coeff_prelude_rows", "cf"),
            ("score_param_prelude_rows", "pm"),
        ):
            if compiled.get("uses_lag") and field not in section_params:
                raise RuntimeError(f"fused raster lagged solve-score payload missing {field}")
            actual = int(section_params.get(field, prelude_by_source[source]) or 0)
            expected = int(prelude_by_source[source])
            if actual != expected:
                raise RuntimeError(f"fused raster {field} mismatch: expected {expected}, got {actual}")
            section_params[field] = expected

        section_params = _prepare_fused_section_inputs(section_params)
        report_status(job_id, task_id, "bin_downloaded_1/1")

        t_native = time.perf_counter()
        cmd = _build_cmd(section_params)
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        native_wall_us = int((time.perf_counter() - t_native) * 1e6)
        if result.returncode != 0:
            raise RuntimeError(f"roots2pix_mt failed for section {section_idx}: {result.stderr.strip()}")

        raster_meta = json.loads(result.stdout)
        perf["threads"] = int(raster_meta.get("threads", threads))
        perf["input_mode"] = str(raster_meta.get("input_mode", section_params.get("effective_input_mode", "multispan_sectioned")))
        perf["retries"] = int(raster_meta.get("retries", raster_sectioned_retries))
        if perf["input_mode"] in ("sectioned", "multispan_sectioned"):
            perf["download_us"] += int(raster_meta.get("download_us", 0))
        perf["native_us"] += int(raster_meta.get("native_us", native_wall_us))
        perf["roots_plotted"] += int(raster_meta.get("roots_plotted", 0))
        perf["roots_clipped"] += int(raster_meta.get("roots_clipped", 0))

        t_upload = time.perf_counter()

        fused_fragment_path = "/tmp/fused_fragment.frag"
        if os.path.exists(fused_fragment_path):
            fused_fragment_size = os.path.getsize(fused_fragment_path)
            with open(fused_fragment_path, "rb") as fh:
                s3.upload_fileobj(fh, BUCKET, f"{fragment_prefix}{int(section_idx):04d}.frag")
            os.remove(fused_fragment_path)
        else:
            fused_fragment_size = 0
            s3.put_object(
                Bucket=BUCKET,
                Key=f"{fragment_prefix}{int(section_idx):04d}.frag",
                Body=b"",
                ContentType="application/octet-stream",
            )

        palette_fragment_size = 0
        if emit_associated_palette_bins:
            palette_fragment_path = "/tmp/palette_fragment.frag"
            if os.path.exists(palette_fragment_path):
                palette_fragment_size = os.path.getsize(palette_fragment_path)
                with open(palette_fragment_path, "rb") as fh:
                    s3.upload_fileobj(
                        fh,
                        BUCKET,
                        f"{associated_palette_fragment_prefix}{int(section_idx):04d}.frag",
                    )
                os.remove(palette_fragment_path)
            else:
                s3.put_object(
                    Bucket=BUCKET,
                    Key=f"{associated_palette_fragment_prefix}{int(section_idx):04d}.frag",
                    Body=b"",
                    ContentType="application/octet-stream",
                )

        step_scores_size = 0
        step_scores_key = f"{fragment_prefix}{int(section_idx):04d}_step_scores.raw"
        if emit_step_scores:
            step_scores_path = "/tmp/step_scores.bin"
            if os.path.exists(step_scores_path):
                step_scores_size = os.path.getsize(step_scores_path)
                with open(step_scores_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, step_scores_key)
                os.remove(step_scores_path)
            else:
                s3.put_object(
                    Bucket=BUCKET,
                    Key=step_scores_key,
                    Body=b"",
                    ContentType="application/octet-stream",
                )

        perf["upload_us"] = int((time.perf_counter() - t_upload) * 1e6)
        perf["fragment_files_uploaded"] = 1
        perf["fragment_bytes_uploaded"] = fused_fragment_size
        perf["associated_palette_fragment_files_uploaded"] = 1 if emit_associated_palette_bins else 0
        perf["associated_palette_fragment_bytes_uploaded"] = palette_fragment_size
        perf["step_scores_bytes_uploaded"] = step_scores_size
        perf["step_score_channels"] = step_score_channels if emit_step_scores else 0

        report_status(job_id, task_id, "rasterized_1/1")
        report_status(job_id, task_id, "rasterized")
        attach_contract_warnings(perf, contract_warnings)
        report_status(job_id, task_id, "done", result_data=perf)
        return ok_response({
            "section_idx": int(section_idx),
            "group_idx": int(section_idx),
            "section_indices": [int(section_idx)],
            "fragment_files_uploaded": 1,
            "fragment_bytes_uploaded": fused_fragment_size,
            "associated_palette_fragment_files_uploaded": 1 if emit_associated_palette_bins else 0,
            "associated_palette_fragment_bytes_uploaded": palette_fragment_size,
            "step_scores_bytes_uploaded": step_scores_size,
            "step_score_channels": step_score_channels if emit_step_scores else 0,
            "rgb_source": "raw_score_bins",
            "raster_us": perf["native_us"],
            "roots_plotted": perf["roots_plotted"],
            "roots_clipped": perf["roots_clipped"],
            "engine": "mt",
            "threads": perf["threads"],
            "input_mode": perf["input_mode"],
        })
    except Exception as e:
        error_perf = dict(perf or {"engine": "mt", "phase": "raster", "phase_label": "Raster"})
        attach_contract_warnings(error_perf, contract_warnings)
        if job_id:
            report_status(job_id, task_id, "error", str(e), result_data=error_perf)
        raise
    finally:
        _cleanup_tmp()


def handler(event, context):
    try:
        params = parse_body(event)
    except Exception as e:
        params = _extract_handler_entry_params(event)
        message = f"raster_mt handler could not parse request body: {e}"
        _report_handler_entry_error(params, message)
        raise RuntimeError(message) from e
    return _handle_fused_raster_request(params)
