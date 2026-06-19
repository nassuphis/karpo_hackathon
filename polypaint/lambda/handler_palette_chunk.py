"""
Palette chunk Lambda — compute exact solve-score scores/bins for one full-solve chunk.

The durable outputs are all-pass chunk-local numeric data. Pass-0-only reduction
now happens later during palette image assembly.
"""
import json
import os
import subprocess
import time

import boto3

from logical_sections import (
    build_native_manifest_urls,
    build_native_multispan_manifest,
    build_source_spans,
    resolve_solve_source_manifest,
    stitch_spans_to_file,
    write_native_multispan_manifest,
)
from solve_score_chain import (
    canonicalize_solve_score_program_spec,
    compile_solve_score_chain_or_legacy,
    compiled_solve_score_fingerprint,
    solve_score_lag_prelude_by_source,
    solve_score_program_spec_uses_lag,
    solve_score_program_cli_payload,
)
from shared import BUCKET, attach_contract_warnings, contract_param, parse_body, ok_response, parse_boolish, report_status

s3 = boto3.client("s3")
BINARY = os.path.join(os.path.dirname(__file__), "solve_palette_chunk")
BINARY_MT = os.path.join(os.path.dirname(__file__), "solve_palette_chunk_mt")
VALID_INPUT_MODES = {"tmpfile", "sectioned"}

_TMP_INPUT = "/tmp/palette_chunk_input.bin"
_TMP_SCORES = "/tmp/palette_chunk_scores.bin"
_TMP_BINS = "/tmp/palette_chunk_bins.bin"
_TMP_XFORMS = "/tmp/palette_chunk_xforms.json"
_TMP_SCORE_COEFFS = "/tmp/palette_chunk_coeffs.bin"
_TMP_SCORE_PARAMS = "/tmp/palette_chunk_params.bin"
_TMP_INPUT_MANIFEST = "/tmp/palette_chunk_input_manifest.json"
_TMP_SCORE_COEFFS_MANIFEST = "/tmp/palette_chunk_coeffs_manifest.json"
_TMP_SCORE_PARAMS_MANIFEST = "/tmp/palette_chunk_params_manifest.json"


def _cleanup():
    for p in (
        _TMP_INPUT,
        _TMP_SCORES,
        _TMP_BINS,
        _TMP_XFORMS,
        _TMP_SCORE_COEFFS,
        _TMP_SCORE_PARAMS,
        _TMP_INPUT_MANIFEST,
        _TMP_SCORE_COEFFS_MANIFEST,
        _TMP_SCORE_PARAMS_MANIFEST,
    ):
        try:
            os.remove(p)
        except OSError:
            pass


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


def _validate_threads(value, default=1):
    if value in (None, ""):
        value = default
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"palette_chunk_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"palette_chunk_threads must be in [1, 16], got {threads}")
    return threads


def _validate_input_mode(value):
    mode = str(value or "tmpfile").strip().lower()
    if mode not in VALID_INPUT_MODES:
        raise RuntimeError(
            f"palette_chunk_input_mode must be one of {', '.join(sorted(VALID_INPUT_MODES))}, got {value!r}"
        )
    return mode


def _validate_retries(value):
    if value in (None, ""):
        value = 2
    try:
        retries = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"palette_chunk_retries must be an integer, got {value!r}")
    if not (0 <= retries <= 10):
        raise RuntimeError(f"palette_chunk_retries must be in [0, 10], got {retries}")
    return retries


def _validate_workers(value):
    if value in (None, ""):
        value = 1
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"palette_chunk_workers must be an integer, got {value!r}")
    if not (1 <= workers <= 64):
        raise RuntimeError(f"palette_chunk_workers must be in [1, 64], got {workers}")
    return workers


def _validate_output_channel_count(value):
    try:
        n = int(value or 1)
    except (TypeError, ValueError):
        raise RuntimeError(f"score_output_channel_count must be an integer, got {value!r}")
    if not (1 <= n <= 8):
        raise RuntimeError(f"score_output_channel_count must be in [1, 8], got {n}")
    return n


def _score_output_clip_csvs(bins_data, channel_count):
    channels = list((bins_data or {}).get("score_output_channels") or [])
    if not channels:
        return None, None
    if len(channels) != int(channel_count):
        raise RuntimeError(
            f"score_output_channels length mismatch: expected {channel_count}, got {len(channels)}"
        )
    lows = []
    highs = []
    for idx, channel in enumerate(channels):
        if not isinstance(channel, dict):
            raise RuntimeError(f"score_output_channels[{idx}] must be an object")
        lows.append(str(float(channel.get("clip_lo", 0.0))))
        highs.append(str(float(channel.get("clip_hi", 1.0))))
    return ",".join(lows), ",".join(highs)


def _sectioned_input_size_limit():
    try:
        memory_mb = int(os.environ.get("AWS_LAMBDA_FUNCTION_MEMORY_SIZE", "0") or 0)
    except (TypeError, ValueError):
        memory_mb = 0
    if memory_mb <= 0:
        return 0
    return (memory_mb * 1024 * 1024) // 2


def _solve_score_program_args(bins_data):
    program_spec = canonicalize_solve_score_program_spec(bins_data.get("program") or "")
    payload = solve_score_program_cli_payload({
        "metrics": bins_data.get("metrics") or [],
        "program_spec": program_spec,
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


def _solve_score_bins_uses_source(bins_data, source):
    for metric in (bins_data.get("metrics") or []):
        if str(metric.get("source", "slv")).strip().lower() == source:
            return True
    return False


def handler(event, context):
    params = parse_body(event)
    contract_warnings = []
    job_id = params["job_id"]
    task_id = params["task_id"]
    section_idx = params.get("section_idx", params.get("chunk_idx"))
    if section_idx is None:
        raise RuntimeError("palette chunk requires section_idx")
    bin_key = params["bin_key"]
    degree = params["degree"]
    metric = params["metric"]
    raw_chain = params.get("solve_score_chain", "")
    has_request_chain = raw_chain not in ("", None, [])
    if has_request_chain:
        compiled = compile_solve_score_chain_or_legacy(
            raw_chain,
            metric,
            params.get("solve_score_quantile", 0.001),
            params.get("solve_score_omega", 1.0),
            params.get("solve_score_omega_enabled", True),
            default_metric=metric,
        )
        metric = compiled["metric"]
        q = compiled["quantile"]
        omega = compiled["omega"]
        omega_enabled = compiled["omega_enabled"]
        chain_fingerprint = compiled_solve_score_fingerprint(compiled)
    else:
        compiled = None
        q = float(params.get("solve_score_quantile", 0.001))
        omega = float(params.get("solve_score_omega", 1.0))
        omega_enabled = parse_boolish(params.get("solve_score_omega_enabled", True), True)
        chain_fingerprint = str(params.get("solve_score_chain_fingerprint") or "").strip()
    bins_key = params["solve_score_bins_key"]
    step_start = int(params["step_start"])
    step_count = int(params["step_count"])
    section_count = params.get("section_count")
    root_transforms = contract_param(params, "root_transforms", [], contract_warnings)
    threads = _validate_threads(contract_param(params, "palette_chunk_threads", 1, contract_warnings), default=1)
    input_mode = _validate_input_mode(contract_param(params, "palette_chunk_input_mode", "tmpfile", contract_warnings))
    retries = _validate_retries(contract_param(params, "palette_chunk_retries", 2, contract_warnings))
    workers = _validate_workers(contract_param(params, "palette_chunk_workers", 1, contract_warnings))
    score_output_channel_count = _validate_output_channel_count(params.get("score_output_channel_count", 1))
    color_interpretation = str(params.get("color_interpretation") or "scalar_lut")
    score_output_has_explicit_outputs = parse_boolish(
        params.get("score_output_has_explicit_outputs", False),
        False,
    )
    score_output_channels = list(params.get("score_output_channels") or [])
    raw_output_path = score_output_has_explicit_outputs or score_output_channel_count != 1
    coeffs_key = str(params.get("coeffs_key") or "").strip()
    coeffs_bin_size = params.get("coeffs_bin_size")
    params_key = str(params.get("params_key") or "").strip()
    params_step_start = params.get("params_step_start", step_start)
    params_step_count = params.get("params_step_count", step_count)
    n_coeffs = params.get("n_coeffs")
    root_spans = list(params.get("root_spans") or [])
    coeff_spans = list(params.get("coeff_spans") or [])
    param_spans = list(params.get("param_spans") or [])
    logical_section = parse_boolish(params.get("logical_section"), bool(root_spans))
    solve_source_manifest = resolve_solve_source_manifest(
        params,
        s3,
        BUCKET,
        required_context="palette chunk",
    )
    uses_lag = bool(compiled and compiled.get("uses_lag"))
    prelude_by_source = solve_score_lag_prelude_by_source(compiled) if compiled else {"slv": 0, "cf": 0, "pm": 0}
    requested_solve_prelude = int(params.get("prelude_rows") or 0)
    requested_coeff_prelude = int(params.get("score_coeff_prelude_rows") or 0)
    requested_param_prelude = int(params.get("score_param_prelude_rows") or 0)
    if uses_lag:
        if not logical_section:
            raise RuntimeError("lagged palette chunk requires logical_section=true")
        if input_mode != "sectioned":
            raise RuntimeError("lagged palette chunk requires palette_chunk_input_mode=sectioned")
        expected = (
            int(prelude_by_source.get("slv", 0)),
            int(prelude_by_source.get("cf", 0)),
            int(prelude_by_source.get("pm", 0)),
        )
        got = (requested_solve_prelude, requested_coeff_prelude, requested_param_prelude)
        if got != expected:
            raise RuntimeError(
                "lagged palette chunk prelude contract mismatch: "
                f"expected slv/cf/pm={expected}, got {got}"
            )
    elif requested_solve_prelude or requested_coeff_prelude or requested_param_prelude:
        raise RuntimeError("prelude rows require a lagged solve-score program")
    actual_solve_prelude = 1 if requested_solve_prelude and step_start > 0 else 0
    actual_coeff_prelude = 1 if requested_coeff_prelude and step_start > 0 else 0
    actual_param_prelude = 1 if requested_param_prelude and step_start > 0 else 0
    score_key = params["score_key"]
    palette_bins_key = params["palette_bins_key"]
    meta_key = params["meta_key"]

    progress = attach_contract_warnings({
        "phase": "palette_chunk",
        "section_idx": section_idx,
        "section_count": section_count,
        "metric": metric,
        "chain_fingerprint": chain_fingerprint,
        "threads": threads,
        "requested_input_mode": input_mode,
        "input_mode": input_mode,
        "logical_section": logical_section,
        "retries": retries,
        "workers": workers,
        "dl_ms": 0,
        "compute_ms": 0,
        "upload_ms": 0,
        "step_count": step_count,
        "score_output_channel_count": score_output_channel_count,
        "color_interpretation": color_interpretation,
        "raw_output_path": raw_output_path,
        "prelude_rows": actual_solve_prelude,
        "score_coeff_prelude_rows": actual_coeff_prelude,
        "score_param_prelude_rows": actual_param_prelude,
    }, contract_warnings)
    try:
        _cleanup()
        report_status(job_id, task_id, "started", result_data=progress)

        sectioned_url = None
        source_size = int(params.get("bin_size") or 0)
        effective_input_mode = input_mode
        input_manifest_path = None
        coeff_manifest_path = None
        param_manifest_path = None
        if logical_section and not root_spans:
            if not solve_source_manifest:
                raise RuntimeError("logical palette chunk requires solve_source_manifest")
            root_spans = build_source_spans(
                solve_source_manifest,
                source_family="slv",
                solve_start=step_start - actual_solve_prelude,
                solve_count=step_count + actual_solve_prelude,
            )
            if root_spans and not bin_key:
                bin_key = str(root_spans[0]["key"])
        if logical_section and input_mode == "sectioned":
            root_urls = build_native_manifest_urls(s3, BUCKET, root_spans)
            input_manifest = build_native_multispan_manifest(
                solve_source_manifest,
                source_family="slv",
                solve_start=int(step_start) - actual_solve_prelude,
                solve_count=int(step_count) + actual_solve_prelude,
                url_by_key=root_urls,
            )
            input_manifest_path = write_native_multispan_manifest(_TMP_INPUT_MANIFEST, input_manifest)
            source_size = int(input_manifest["logical_size"])
            effective_input_mode = "multispan_sectioned"
            progress["dl_ms"] = 0
        elif logical_section:
            t0 = time.time()
            source_size = stitch_spans_to_file(s3, BUCKET, root_spans, _TMP_INPUT)
            progress["dl_ms"] = int((time.time() - t0) * 1000)
            effective_input_mode = "tmpfile"
        elif input_mode == "sectioned":
            if source_size <= 0:
                head = s3.head_object(Bucket=BUCKET, Key=bin_key)
                source_size = int(head.get("ContentLength") or 0)
            if source_size <= 0:
                raise RuntimeError(f"Failed to determine size for s3://{BUCKET}/{bin_key}")
            size_limit = _sectioned_input_size_limit()
            if size_limit > 0 and source_size > size_limit:
                raise RuntimeError(
                    f"sectioned palette chunk input too large for current Lambda memory: "
                    f"{source_size} bytes > safe limit {size_limit} bytes"
                )
            sectioned_url = s3.generate_presigned_url(
                "get_object",
                Params={"Bucket": BUCKET, "Key": bin_key},
                ExpiresIn=900,
            )
        else:
            t0 = time.time()
            try:
                obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
            except Exception as e:
                raise RuntimeError(f"Failed to download root chunk s3://{BUCKET}/{bin_key}: {e}") from e
            with open(_TMP_INPUT, "wb") as f:
                for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                    f.write(chunk)
            progress["dl_ms"] = int((time.time() - t0) * 1000)
            source_size = os.path.getsize(_TMP_INPUT)
        progress["source_size"] = source_size

        try:
            bins_obj = s3.get_object(Bucket=BUCKET, Key=bins_key)
        except Exception as e:
            raise RuntimeError(f"Failed to download solve-score bins s3://{BUCKET}/{bins_key}: {e}") from e
        bins_data = json.loads(bins_obj["Body"].read())
        if bins_data.get("family") != "solve_score":
            raise RuntimeError(f"Bins artifact missing or wrong family: {bins_data.get('family')}")
        cuts = bins_data.get("cuts_norm", [])
        if len(cuts) != 9:
            raise RuntimeError(f"Bins artifact must contain 9 cuts, got {len(cuts)}")
        bins_output_channel_count = int(bins_data.get("score_output_channel_count") or 1)
        if bins_output_channel_count != score_output_channel_count:
            raise RuntimeError(
                "Bins score_output_channel_count mismatch: "
                f"expected {score_output_channel_count}, got {bins_output_channel_count}"
            )
        score_output_clip_los_csv, score_output_clip_his_csv = _score_output_clip_csvs(
            bins_data,
            score_output_channel_count,
        )
        is_v2_bins = int(bins_data.get("version", 1) or 1) >= 2
        uses_coeff_source = is_v2_bins and _solve_score_bins_uses_source(bins_data, "cf")
        uses_param_source = is_v2_bins and _solve_score_bins_uses_source(bins_data, "pm")
        if is_v2_bins:
            if not bins_data.get("program") or not isinstance(bins_data.get("metrics"), list) or not bins_data.get("metrics"):
                raise RuntimeError("v2 solve-score bins artifact is missing program or metrics")
            bins_program_uses_lag = solve_score_program_spec_uses_lag(bins_data.get("program") or "")
            if bins_program_uses_lag and not compiled:
                raise RuntimeError("lagged palette chunk requires solve_score_chain in the request payload")
            if bins_program_uses_lag and not uses_lag:
                raise RuntimeError("lagged palette chunk bins do not match request chain")
            if has_request_chain:
                actual_fingerprint = str(bins_data.get("chain_fingerprint") or "").strip()
                if not actual_fingerprint:
                    raise RuntimeError("Bins artifact missing chain_fingerprint")
                if actual_fingerprint != chain_fingerprint:
                    raise RuntimeError(
                        f"Bins fingerprint mismatch: expected {chain_fingerprint}, got {actual_fingerprint}"
                    )
                if canonicalize_solve_score_program_spec(bins_data.get("program") or "") != compiled["program_spec"]:
                    raise RuntimeError(
                        f"Bins program mismatch: expected {compiled['program_spec']}, got {bins_data.get('program')!r}"
                    )
                if len(bins_data["metrics"]) != compiled["metric_count"]:
                    raise RuntimeError(
                        f"Bins metric slot count mismatch: expected {compiled['metric_count']}, got {len(bins_data['metrics'])}"
                    )
                for expected, actual in zip(compiled["metrics"], bins_data["metrics"]):
                    if str(actual.get("metric") or "") != expected["metric"]:
                        raise RuntimeError(
                            f"Bins metric slot {expected['slot']} mismatch: expected {expected['metric']}, got {actual.get('metric')!r}"
                        )
                    if str(actual.get("source", "slv") or "slv") != expected.get("source", "slv"):
                        raise RuntimeError(
                            f"Bins source slot {expected['slot']} mismatch: expected {expected.get('source', 'slv')}, "
                            f"got {actual.get('source', 'slv')!r}"
                        )
                    if float(actual.get("quantile", -1)) != float(expected["quantile"]):
                        raise RuntimeError(
                            f"Bins quantile slot {expected['slot']} mismatch: expected {expected['quantile']}, got {actual.get('quantile')!r}"
                        )
            if uses_coeff_source:
                try:
                    n_coeffs = int(n_coeffs)
                except (TypeError, ValueError):
                    raise RuntimeError(f"mixed-source palette chunk requires numeric n_coeffs, got {n_coeffs!r}")
                if n_coeffs < 1:
                    raise RuntimeError(f"mixed-source palette chunk requires n_coeffs >= 1, got {n_coeffs}")
                if not coeffs_key and not logical_section:
                    raise RuntimeError("mixed-source palette chunk requires coeffs_key")
            if uses_param_source:
                if not params_key and not logical_section:
                    raise RuntimeError("param-source palette chunk requires params_key")
                try:
                    params_step_start = int(params_step_start)
                    params_step_count = int(params_step_count)
                except (TypeError, ValueError):
                    raise RuntimeError(
                        "param-source palette chunk requires numeric "
                        f"params_step_start/params_step_count, got {params_step_start!r}/{params_step_count!r}"
                    )
                if params_step_start < 0 or params_step_count < 1:
                    raise RuntimeError(
                        "param-source palette chunk requires params_step_start >= 0 and "
                        f"params_step_count >= 1, got {params_step_start}/{params_step_count}"
                    )
                if params_step_count != step_count:
                    raise RuntimeError(
                        f"param-source palette chunk requires params_step_count == step_count, got {params_step_count}/{step_count}"
                    )
            if logical_section and (uses_coeff_source or uses_param_source) and not (coeff_spans or param_spans):
                if not solve_source_manifest:
                    raise RuntimeError("logical palette chunk requires solve_source_manifest")
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
        else:
            if bins_data.get("metric") != metric:
                raise RuntimeError(f"Bins metric mismatch: expected {metric}, got {bins_data.get('metric')}")
            if bins_data.get("clip_quantile") != q:
                raise RuntimeError(f"Bins quantile mismatch: expected {q}, got {bins_data.get('clip_quantile')}")
            if float(bins_data.get("omega", 1.0)) != omega:
                raise RuntimeError(f"Bins omega mismatch: expected {omega}, got {bins_data.get('omega')}")
            if parse_boolish(bins_data.get("omega_enabled", True), True) != omega_enabled:
                raise RuntimeError(f"Bins omega_enabled mismatch: expected {omega_enabled}, got {bins_data.get('omega_enabled')}")

        report_status(job_id, task_id, "bin_downloaded", result_data=progress)

        if uses_lag and effective_input_mode != "multispan_sectioned":
            raise RuntimeError("lagged palette chunk requires multispan_sectioned native input")
        use_legacy_binary = (not raw_output_path) and (not uses_lag) and threads == 1 and effective_input_mode == "tmpfile"
        cmd = [
            BINARY if use_legacy_binary else BINARY_MT,
            _TMP_INPUT,
            f"--degree={degree}",
            f"--cuts={','.join(str(c) for c in cuts)}",
            f"--step_count={step_count}",
            f"--scores_out={_TMP_SCORES}",
            f"--bins_out={_TMP_BINS}",
            f"--score_output_channel_count={score_output_channel_count}",
        ]
        if score_output_clip_los_csv and score_output_clip_his_csv:
            cmd.extend([
                f"--score_output_clip_los={score_output_clip_los_csv}",
                f"--score_output_clip_his={score_output_clip_his_csv}",
            ])
        if is_v2_bins:
            cmd.extend(_solve_score_program_args(bins_data))
            if uses_coeff_source:
                if logical_section and effective_input_mode == "multispan_sectioned":
                    coeff_urls = build_native_manifest_urls(s3, BUCKET, coeff_spans)
                    coeff_manifest = build_native_multispan_manifest(
                        solve_source_manifest,
                        source_family="cf",
                        solve_start=int(step_start) - actual_coeff_prelude,
                        solve_count=int(step_count) + actual_coeff_prelude,
                        url_by_key=coeff_urls,
                    )
                    coeff_manifest_path = write_native_multispan_manifest(_TMP_SCORE_COEFFS_MANIFEST, coeff_manifest)
                    progress["source_coeffs_size"] = int(coeff_manifest["logical_size"])
                    cmd.extend([
                        f"--score_coeff_manifest={coeff_manifest_path}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                elif logical_section and coeff_spans:
                    progress["source_coeffs_size"] = stitch_spans_to_file(s3, BUCKET, coeff_spans, _TMP_SCORE_COEFFS)
                    cmd.extend([
                        f"--score_coeffs_file={_TMP_SCORE_COEFFS}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                elif input_mode == "sectioned":
                    coeff_input_size = int(coeffs_bin_size or 0)
                    if coeff_input_size <= 0:
                        head = s3.head_object(Bucket=BUCKET, Key=coeffs_key)
                        coeff_input_size = int(head.get("ContentLength") or 0)
                    if coeff_input_size <= 0:
                        raise RuntimeError(f"Failed to determine coeff chunk size for s3://{BUCKET}/{coeffs_key}")
                    coeff_url = s3.generate_presigned_url(
                        "get_object",
                        Params={"Bucket": BUCKET, "Key": coeffs_key},
                        ExpiresIn=900,
                    )
                    progress["source_coeffs_size"] = coeff_input_size
                    cmd.extend([
                        f"--score_coeffs_url={coeff_url}",
                        f"--score_coeff_input_size={coeff_input_size}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
                else:
                    coeff_obj = s3.get_object(Bucket=BUCKET, Key=coeffs_key)
                    with open(_TMP_SCORE_COEFFS, "wb") as cf:
                        for chunk in coeff_obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                            cf.write(chunk)
                    progress["source_coeffs_size"] = os.path.getsize(_TMP_SCORE_COEFFS)
                    cmd.extend([
                        f"--score_coeffs_file={_TMP_SCORE_COEFFS}",
                        f"--score_coeff_degree={n_coeffs}",
                    ])
            if uses_param_source:
                if logical_section and effective_input_mode == "multispan_sectioned":
                    param_urls = build_native_manifest_urls(s3, BUCKET, param_spans)
                    param_manifest = build_native_multispan_manifest(
                        solve_source_manifest,
                        source_family="pm",
                        solve_start=int(step_start) - actual_param_prelude,
                        solve_count=int(step_count) + actual_param_prelude,
                        url_by_key=param_urls,
                    )
                    param_manifest_path = write_native_multispan_manifest(_TMP_SCORE_PARAMS_MANIFEST, param_manifest)
                    param_size = int(param_manifest["logical_size"])
                    cmd.append(f"--score_params_manifest={param_manifest_path}")
                elif logical_section and param_spans:
                    param_size = stitch_spans_to_file(s3, BUCKET, param_spans, _TMP_SCORE_PARAMS)
                    cmd.append(f"--score_params_file={_TMP_SCORE_PARAMS}")
                else:
                    param_size = _download_range(
                        params_key,
                        _TMP_SCORE_PARAMS,
                        int(params_step_start) * 4 * 4,
                        int(params_step_count) * 4 * 4,
                    )
                    cmd.append(f"--score_params_file={_TMP_SCORE_PARAMS}")
                progress["source_params_size"] = param_size
        else:
            cmd.extend([
                f"--metric={metric}",
                f"--clip_lo={bins_data['clip_lo']}",
                f"--clip_hi={bins_data['clip_hi']}",
                f"--omega={omega}",
                f"--omega_enabled={1 if omega_enabled else 0}",
            ])
        if not use_legacy_binary:
            cmd.extend([
                f"--threads={threads}",
                f"--input_mode={effective_input_mode}",
                f"--retries={retries}",
            ])
            if effective_input_mode == "sectioned":
                cmd.extend([
                    f"--url={sectioned_url}",
                    f"--input_size={source_size}",
                ])
            elif effective_input_mode == "multispan_sectioned":
                cmd.append(f"--input_manifest={input_manifest_path}")
            if uses_lag:
                cmd.extend([
                    f"--prelude_rows={actual_solve_prelude}",
                    f"--score_coeff_prelude_rows={actual_coeff_prelude}",
                    f"--score_param_prelude_rows={actual_param_prelude}",
                ])
        if root_transforms:
            with open(_TMP_XFORMS, "w") as xf:
                json.dump(root_transforms, xf)
            cmd.append(f"--root_xforms={_TMP_XFORMS}")

        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        fallback_compute_ms = int((time.time() - t1) * 1000)
        if result.returncode != 0:
            source_ctx = (
                f"s3://{BUCKET}/{bin_key} "
                f"(job={job_id}, task={task_id}, section={section_idx}, input={effective_input_mode}, "
                f"size={source_size}, threads={threads}, retries={retries})"
            )
            stderr = (result.stderr or "").strip()
            stdout = (result.stdout or "").strip()
            detail = stderr or stdout or "no stderr"
            raise RuntimeError(f"solve_palette_chunk failed for {source_ctx}: {detail}")
        meta = json.loads(result.stdout or "{}")
        progress["dl_ms"] = int(meta.get("download_ms", progress["dl_ms"]) or 0)
        progress["compute_ms"] = int(meta.get("compute_ms", fallback_compute_ms) or 0)
        progress["threads"] = int(meta.get("threads", threads) or threads)
        progress["input_mode"] = str(meta.get("input_mode", effective_input_mode) or effective_input_mode)
        progress["retries"] = int(meta.get("retries", retries) or retries)
        progress["n_samples"] = int(meta.get("n_samples", step_count) or step_count)
        progress["score_output_channel_count"] = int(
            meta.get("output_channel_count", score_output_channel_count) or score_output_channel_count
        )
        expected_scores_size = int(step_count) * int(score_output_channel_count) * 4
        expected_bins_size = int(step_count) * int(score_output_channel_count)
        actual_scores_size = os.path.getsize(_TMP_SCORES)
        actual_bins_size = os.path.getsize(_TMP_BINS)
        if actual_scores_size != expected_scores_size:
            raise RuntimeError(
                f"score output size mismatch: expected {expected_scores_size} bytes, got {actual_scores_size}"
            )
        if actual_bins_size != expected_bins_size:
            raise RuntimeError(
                f"palette/raw bin output size mismatch: expected {expected_bins_size} bytes, got {actual_bins_size}"
            )

        report_status(
            job_id,
            task_id,
            "computed",
            result_data=attach_contract_warnings(
                {
                    **progress,
                },
                contract_warnings,
            ),
        )

        t_up = time.time()
        with open(_TMP_SCORES, "rb") as sf:
            s3.upload_fileobj(sf, BUCKET, score_key, ExtraArgs={"ContentType": "application/octet-stream"})
        with open(_TMP_BINS, "rb") as bf:
            s3.upload_fileobj(bf, BUCKET, palette_bins_key, ExtraArgs={"ContentType": "application/octet-stream"})
        progress["upload_ms"] = int((time.time() - t_up) * 1000)

        chunk_meta = {
            "job_id": job_id,
            "section_idx": int(section_idx),
            "step_start": step_start,
            "step_count": step_count,
            "metric": metric,
            "omega": omega,
            "omega_enabled": omega_enabled,
            "clip_lo": bins_data.get("clip_lo"),
            "clip_hi": bins_data.get("clip_hi"),
            "cuts_norm": cuts,
            "score_key": score_key,
            "palette_bins_key": palette_bins_key,
            "score_output_channel_count": score_output_channel_count,
            "score_output_channels": score_output_channels,
            "raw_channels": score_output_channel_count,
            "color_interpretation": color_interpretation,
            "raw_output_path": raw_output_path,
            "score_bytes": actual_scores_size,
            "palette_bins_bytes": actual_bins_size,
            "min_score": meta.get("min_score"),
            "max_score": meta.get("max_score"),
        }
        if is_v2_bins:
            if chain_fingerprint:
                chunk_meta["chain_fingerprint"] = chain_fingerprint
            elif bins_data.get("chain_fingerprint"):
                chunk_meta["chain_fingerprint"] = bins_data.get("chain_fingerprint")
            chunk_meta["program"] = bins_data.get("program")
            chunk_meta["metrics"] = bins_data.get("metrics")
        s3.put_object(Bucket=BUCKET, Key=meta_key, Body=json.dumps(chunk_meta), ContentType="application/json")

        result_data = attach_contract_warnings({
            "section_idx": section_idx,
            "section_count": section_count,
            "step_start": step_start,
            "step_count": step_count,
            "score_key": score_key,
            "palette_bins_key": palette_bins_key,
            "meta_key": meta_key,
            "dl_ms": progress["dl_ms"],
            "compute_ms": progress["compute_ms"],
            "upload_ms": progress["upload_ms"],
            "threads": progress["threads"],
            "input_mode": progress["input_mode"],
            "retries": progress["retries"],
            "workers": workers,
            "source_size": source_size,
            "logical_section": logical_section,
            "prelude_rows": actual_solve_prelude,
            "score_coeff_prelude_rows": actual_coeff_prelude,
            "score_param_prelude_rows": actual_param_prelude,
            "score_output_channel_count": score_output_channel_count,
            "score_output_channels": score_output_channels,
            "raw_channels": score_output_channel_count,
            "color_interpretation": color_interpretation,
            "raw_output_path": raw_output_path,
        }, contract_warnings)
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup()
