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

from shared import BUCKET, ok_response, parse_body, report_status

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

    try:
        report_status(job_id, task_id, "started")
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

        result = subprocess.run(
            [SWEEP_MT, bin_path],
            input=json.dumps(spec),
            capture_output=True,
            text=True,
            timeout=840,
        )
        if result.returncode != 0:
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
        report_status(job_id, task_id, "error", str(e))
        raise
