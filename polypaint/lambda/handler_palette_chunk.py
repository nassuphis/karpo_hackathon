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

from solve_score_chain import solve_score_program_cli_payload
from shared import BUCKET, attach_contract_warnings, contract_param, parse_body, ok_response, report_status

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


def _cleanup():
    for p in (_TMP_INPUT, _TMP_SCORES, _TMP_BINS, _TMP_XFORMS, _TMP_SCORE_COEFFS, _TMP_SCORE_PARAMS):
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


def _parse_boolish(value, default=True):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


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


def _sectioned_input_size_limit():
    try:
        memory_mb = int(os.environ.get("AWS_LAMBDA_FUNCTION_MEMORY_SIZE", "0") or 0)
    except (TypeError, ValueError):
        memory_mb = 0
    if memory_mb <= 0:
        return 0
    return (memory_mb * 1024 * 1024) // 2


def _solve_score_program_args(bins_data):
    payload = solve_score_program_cli_payload({
        "metrics": bins_data.get("metrics") or [],
        "program_spec": str(bins_data.get("program") or ""),
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
    chunk_idx = params["chunk_idx"]
    bin_key = params["bin_key"]
    degree = params["degree"]
    metric = params["metric"]
    q = contract_param(params, "solve_score_quantile", 0.001, contract_warnings)
    omega = float(contract_param(params, "solve_score_omega", 1.0, contract_warnings))
    omega_enabled = _parse_boolish(contract_param(params, "solve_score_omega_enabled", True, contract_warnings), True)
    bins_key = params["solve_score_bins_key"]
    step_start = int(params["step_start"])
    step_count = int(params["step_count"])
    root_transforms = contract_param(params, "root_transforms", [], contract_warnings)
    threads = _validate_threads(contract_param(params, "palette_chunk_threads", 1, contract_warnings), default=1)
    input_mode = _validate_input_mode(contract_param(params, "palette_chunk_input_mode", "tmpfile", contract_warnings))
    retries = _validate_retries(contract_param(params, "palette_chunk_retries", 2, contract_warnings))
    workers = _validate_workers(contract_param(params, "palette_chunk_workers", 1, contract_warnings))
    coeffs_key = str(params.get("coeffs_key") or "").strip()
    coeffs_bin_size = params.get("coeffs_bin_size")
    params_key = str(params.get("params_key") or "").strip()
    params_step_start = params.get("params_step_start", step_start)
    params_step_count = params.get("params_step_count", step_count)
    n_coeffs = params.get("n_coeffs")
    score_key = params["score_key"]
    palette_bins_key = params["palette_bins_key"]
    meta_key = params["meta_key"]

    progress = attach_contract_warnings({
        "phase": "palette_chunk",
        "chunk_idx": chunk_idx,
        "metric": metric,
        "threads": threads,
        "input_mode": input_mode,
        "retries": retries,
        "workers": workers,
        "dl_ms": 0,
        "compute_ms": 0,
        "upload_ms": 0,
        "step_count": step_count,
    }, contract_warnings)
    try:
        _cleanup()
        report_status(job_id, task_id, "started", result_data=progress)

        sectioned_url = None
        source_size = int(params.get("bin_size") or 0)
        if input_mode == "sectioned":
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
        is_v2_bins = int(bins_data.get("version", 1) or 1) >= 2
        if is_v2_bins:
            if not bins_data.get("program") or not isinstance(bins_data.get("metrics"), list) or not bins_data.get("metrics"):
                raise RuntimeError("v2 solve-score bins artifact is missing program or metrics")
            if _solve_score_bins_uses_source(bins_data, "cf"):
                try:
                    n_coeffs = int(n_coeffs)
                except (TypeError, ValueError):
                    raise RuntimeError(f"mixed-source palette chunk requires numeric n_coeffs, got {n_coeffs!r}")
                if n_coeffs < 1:
                    raise RuntimeError(f"mixed-source palette chunk requires n_coeffs >= 1, got {n_coeffs}")
                if not coeffs_key:
                    raise RuntimeError("mixed-source palette chunk requires coeffs_key")
            if _solve_score_bins_uses_source(bins_data, "pm"):
                if not params_key:
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
        else:
            if bins_data.get("metric") != metric:
                raise RuntimeError(f"Bins metric mismatch: expected {metric}, got {bins_data.get('metric')}")
            if bins_data.get("clip_quantile") != q:
                raise RuntimeError(f"Bins quantile mismatch: expected {q}, got {bins_data.get('clip_quantile')}")
            if float(bins_data.get("omega", 1.0)) != omega:
                raise RuntimeError(f"Bins omega mismatch: expected {omega}, got {bins_data.get('omega')}")
            if _parse_boolish(bins_data.get("omega_enabled", True), True) != omega_enabled:
                raise RuntimeError(f"Bins omega_enabled mismatch: expected {omega_enabled}, got {bins_data.get('omega_enabled')}")

        report_status(job_id, task_id, "bin_downloaded", result_data=progress)

        use_legacy_binary = threads == 1 and input_mode == "tmpfile"
        cmd = [
            BINARY if use_legacy_binary else BINARY_MT,
            _TMP_INPUT,
            f"--degree={degree}",
            f"--cuts={','.join(str(c) for c in cuts)}",
            f"--step_count={step_count}",
            f"--scores_out={_TMP_SCORES}",
            f"--bins_out={_TMP_BINS}",
        ]
        if is_v2_bins:
            cmd.extend(_solve_score_program_args(bins_data))
            if _solve_score_bins_uses_source(bins_data, "cf"):
                if input_mode == "sectioned":
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
            if _solve_score_bins_uses_source(bins_data, "pm"):
                param_size = _download_range(
                    params_key,
                    _TMP_SCORE_PARAMS,
                    int(params_step_start) * 4 * 4,
                    int(params_step_count) * 4 * 4,
                )
                progress["source_params_size"] = param_size
                cmd.append(f"--score_params_file={_TMP_SCORE_PARAMS}")
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
                f"--input_mode={input_mode}",
                f"--retries={retries}",
            ])
            if input_mode == "sectioned":
                cmd.extend([
                    f"--url={sectioned_url}",
                    f"--input_size={source_size}",
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
                f"(job={job_id}, task={task_id}, chunk={chunk_idx}, input={input_mode}, "
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
        progress["input_mode"] = str(meta.get("input_mode", input_mode) or input_mode)
        progress["retries"] = int(meta.get("retries", retries) or retries)
        progress["n_samples"] = int(meta.get("n_samples", step_count) or step_count)

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
            "chunk_idx": chunk_idx,
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
            "min_score": meta.get("min_score"),
            "max_score": meta.get("max_score"),
        }
        if is_v2_bins:
            chunk_meta["program"] = bins_data.get("program")
            chunk_meta["metrics"] = bins_data.get("metrics")
        s3.put_object(Bucket=BUCKET, Key=meta_key, Body=json.dumps(chunk_meta), ContentType="application/json")

        result_data = attach_contract_warnings({
            "chunk_idx": chunk_idx,
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
        }, contract_warnings)
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup()
