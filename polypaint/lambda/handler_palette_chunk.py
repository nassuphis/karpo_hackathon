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

from shared import BUCKET, attach_contract_warnings, contract_param, parse_body, ok_response, report_status

s3 = boto3.client("s3")
BINARY = os.path.join(os.path.dirname(__file__), "solve_palette_chunk")

_TMP_INPUT = "/tmp/palette_chunk_input.bin"
_TMP_SCORES = "/tmp/palette_chunk_scores.bin"
_TMP_BINS = "/tmp/palette_chunk_bins.bin"
_TMP_XFORMS = "/tmp/palette_chunk_xforms.json"


def _cleanup():
    for p in (_TMP_INPUT, _TMP_SCORES, _TMP_BINS, _TMP_XFORMS):
        try:
            os.remove(p)
        except OSError:
            pass


def _parse_boolish(value, default=True):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


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
    score_key = params["score_key"]
    palette_bins_key = params["palette_bins_key"]
    meta_key = params["meta_key"]

    progress = attach_contract_warnings({"phase": "palette_chunk", "chunk_idx": chunk_idx, "metric": metric}, contract_warnings)
    try:
        _cleanup()
        report_status(job_id, task_id, "started", result_data=progress)

        t0 = time.time()
        try:
            obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
        except Exception as e:
            raise RuntimeError(f"Failed to download root chunk s3://{BUCKET}/{bin_key}: {e}") from e
        with open(_TMP_INPUT, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)
        progress["dl_ms"] = dl_ms
        progress["source_size"] = os.path.getsize(_TMP_INPUT)

        try:
            bins_obj = s3.get_object(Bucket=BUCKET, Key=bins_key)
        except Exception as e:
            raise RuntimeError(f"Failed to download solve-score bins s3://{BUCKET}/{bins_key}: {e}") from e
        bins_data = json.loads(bins_obj["Body"].read())
        if bins_data.get("family") != "solve_score":
            raise RuntimeError(f"Bins artifact missing or wrong family: {bins_data.get('family')}")
        if bins_data.get("metric") != metric:
            raise RuntimeError(f"Bins metric mismatch: expected {metric}, got {bins_data.get('metric')}")
        if bins_data.get("clip_quantile") != q:
            raise RuntimeError(f"Bins quantile mismatch: expected {q}, got {bins_data.get('clip_quantile')}")
        if float(bins_data.get("omega", 1.0)) != omega:
            raise RuntimeError(f"Bins omega mismatch: expected {omega}, got {bins_data.get('omega')}")
        if _parse_boolish(bins_data.get("omega_enabled", True), True) != omega_enabled:
            raise RuntimeError(f"Bins omega_enabled mismatch: expected {omega_enabled}, got {bins_data.get('omega_enabled')}")
        cuts = bins_data.get("cuts_norm", [])
        if len(cuts) != 9:
            raise RuntimeError(f"Bins artifact must contain 9 cuts, got {len(cuts)}")

        report_status(job_id, task_id, "bin_downloaded", result_data=progress)

        cmd = [
            BINARY,
            _TMP_INPUT,
            f"--degree={degree}",
            f"--metric={metric}",
            f"--clip_lo={bins_data['clip_lo']}",
            f"--clip_hi={bins_data['clip_hi']}",
            f"--cuts={','.join(str(c) for c in cuts)}",
            f"--omega={omega}",
            f"--omega_enabled={1 if omega_enabled else 0}",
            f"--step_count={step_count}",
            f"--scores_out={_TMP_SCORES}",
            f"--bins_out={_TMP_BINS}",
        ]
        if root_transforms:
            with open(_TMP_XFORMS, "w") as xf:
                json.dump(root_transforms, xf)
            cmd.append(f"--root_xforms={_TMP_XFORMS}")

        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        compute_ms = int((time.time() - t1) * 1000)
        if result.returncode != 0:
            raise RuntimeError(f"solve_palette_chunk failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)

        report_status(
            job_id,
            task_id,
            "computed",
            result_data=attach_contract_warnings(
                {
                    **progress,
                    "compute_ms": compute_ms,
                    "step_count": step_count,
                },
                contract_warnings,
            ),
        )

        with open(_TMP_SCORES, "rb") as sf:
            s3.upload_fileobj(sf, BUCKET, score_key, ExtraArgs={"ContentType": "application/octet-stream"})
        with open(_TMP_BINS, "rb") as bf:
            s3.upload_fileobj(bf, BUCKET, palette_bins_key, ExtraArgs={"ContentType": "application/octet-stream"})

        chunk_meta = {
            "job_id": job_id,
            "chunk_idx": chunk_idx,
            "step_start": step_start,
            "step_count": step_count,
            "metric": metric,
            "omega": omega,
            "omega_enabled": omega_enabled,
            "clip_lo": bins_data["clip_lo"],
            "clip_hi": bins_data["clip_hi"],
            "cuts_norm": cuts,
            "score_key": score_key,
            "palette_bins_key": palette_bins_key,
            "min_score": meta.get("min_score"),
            "max_score": meta.get("max_score"),
        }
        s3.put_object(Bucket=BUCKET, Key=meta_key, Body=json.dumps(chunk_meta), ContentType="application/json")

        result_data = attach_contract_warnings({
            "chunk_idx": chunk_idx,
            "step_start": step_start,
            "step_count": step_count,
            "score_key": score_key,
            "palette_bins_key": palette_bins_key,
            "meta_key": meta_key,
            "compute_ms": compute_ms,
        }, contract_warnings)
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup()
