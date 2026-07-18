"""
Sweep-MT Lambda handler — multithreaded AE solve from pre-computed coefficient files.

Route:
  POST /sweep-mt — solve roots from a coefficient chunk (.bin), upload roots to S3.

Input mirrors handler_sweep.py and adds optional n_threads override.
"""
import json
import os
import subprocess
import time

import boto3

from shared import (
    BUCKET,
    build_tmp_enospc_message,
    is_enospc,
    ok_response,
    parse_body,
    report_status,
)

s3 = boto3.client("s3")
SWEEP_MT = os.path.join(os.path.dirname(__file__), "sweep_mt")


def handler(event, context):
    params = parse_body(event)
    return handle_solve_mt_from_coeffs(params)


def handle_solve_mt_from_coeffs(params):
    job_id = params["job_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    coeffs_key = params["coeffs_key"]
    n_coeffs = params["n_coeffs"]
    n_steps = params.get("n_steps")
    if n_steps is None:
        n2 = params.get("N", params.get("n2"))
        i1_start = params.get("row_start", params.get("i1_start"))
        i1_end = params.get("row_end", params.get("i1_end"))
        n_steps = (i1_end - i1_start) * n2
    task_id = params.get("task_id", f"sweep_{chunk_idx}")
    solve_mode = str(params.get("solve_mode") or "").strip().lower()

    try:
        report_status(job_id, task_id, "started")

        if solve_mode.startswith("fused_"):
            # JT64/CM64/AE64 lores: the roots were already solved
            # IN-PROCESS from the f64 coefficients during the lores
            # coeffgen and uploaded to s3_key — solving the f32 coeffs
            # file here would repaint the AE-MT transport artifact the
            # fused modes exist to avoid (user-caught: all three fused
            # job previews rendered identically). Verify and report.
            s3_key = params.get("s3_key", f"renders/{job_id}/chunk_{chunk_idx}.bin")
            head = s3.head_object(Bucket=BUCKET, Key=s3_key)
            bin_size = int(head["ContentLength"])
            expected = int(n_steps) * (int(n_coeffs) - 1) * 8
            if bin_size != expected:
                raise RuntimeError(
                    f"fused lores roots missing/mismatched at {s3_key}: "
                    f"expected {expected} bytes, got {bin_size}")
            result_data = {
                "chunk_idx": chunk_idx,
                "stripe_idx": chunk_idx,
                "s3_key": s3_key,
                "bin_size": bin_size,
                "compute_us": 0,
                "n_t": int(n_steps),
                "degree": int(n_coeffs) - 1,
                "avg_iterations": 0.0,
                "n_threads": 0,
                "fused_lores": solve_mode,
            }
            report_status(job_id, task_id, "done", result_data=result_data)
            return ok_response({**result_data, "n_procs": 0})

        t0 = time.time()

        resp = s3.get_object(Bucket=BUCKET, Key=coeffs_key)
        coeffs_data = resp["Body"].read()

        coeffs_file = "/tmp/coeffs_chunk_mt.bin"
        with open(coeffs_file, "wb") as fh:
            fh.write(coeffs_data)

        bin_path = "/tmp/roots_chunk_mt.bin"
        spec = {
            "mode": "solve_mt",
            "coeffs_file": coeffs_file,
            "n_coeffs": n_coeffs,
            "n2": n_steps,
            "i1_start": 0,
            "i1_end": 1,
            "match_roots": False,
        }
        if params.get("n_threads") is not None:
            spec["n_threads"] = int(params["n_threads"])
        max_iter = int(params.get("max_iter") or 0)
        if 1 <= max_iter <= 64:
            # solver-brush knob: capped Aberth (0/absent = full convergence)
            spec["max_iter"] = max_iter

        result = subprocess.run(
            [SWEEP_MT, bin_path],
            input=json.dumps(spec),
            capture_output=True,
            text=True,
            timeout=840,
        )
        if result.returncode != 0:
            if "No space left on device" in (result.stderr or ""):
                raise RuntimeError(build_tmp_enospc_message(
                    solver_label="solve_mt",
                    phase="native solve",
                    tmp_file=bin_path,
                    coeffs_key=coeffs_key,
                    coeffs_size=len(coeffs_data),
                    n_coeffs=n_coeffs,
                    n_steps=n_steps,
                    job_id=job_id,
                    chunk_idx=chunk_idx,
                    task_id=task_id,
                ))
            raise RuntimeError(f"solve_mt failed: {result.stderr.strip()}")
        compute_meta = json.loads(result.stdout)

        compute_us = int((time.time() - t0) * 1e6)

        s3_key = params.get("s3_key", f"renders/{job_id}/chunk_{chunk_idx}.bin")
        bin_size = os.path.getsize(bin_path)
        with open(bin_path, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, s3_key)

        for path in [coeffs_file, bin_path]:
            try:
                os.remove(path)
            except OSError:
                pass

        n_threads = int(compute_meta.get("n_threads", 1))
        result_data = {
            "chunk_idx": chunk_idx,
            "stripe_idx": chunk_idx,
            "s3_key": s3_key,
            "bin_size": bin_size,
            "compute_us": compute_us,
            "n_t": compute_meta["n_t"],
            "degree": compute_meta["degree"],
            "avg_iterations": compute_meta["avg_iterations"],
            "n_threads": n_threads,
        }
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response({**result_data, "n_procs": n_threads})

    except Exception as e:
        if is_enospc(e):
            err = RuntimeError(build_tmp_enospc_message(
                solver_label="solve_mt",
                phase="local temp write",
                tmp_file="/tmp",
                coeffs_key=coeffs_key,
                coeffs_size=len(coeffs_data) if 'coeffs_data' in locals() else 0,
                n_coeffs=n_coeffs,
                n_steps=n_steps,
                job_id=job_id,
                chunk_idx=chunk_idx,
                task_id=task_id,
            ))
            report_status(job_id, task_id, "error", str(err))
            raise err from e
        report_status(job_id, task_id, "error", str(e))
        raise
