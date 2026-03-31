"""
Sweep-CM Lambda handler — companion-matrix eigenvalue solver for polynomial roots.

Mirrors handler_sweep.py but uses sweep_cm binary (LAPACK zgeev) instead of
Aberth-Ehrlich. Same input/output contract, same DynamoDB status reporting.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
SWEEP_CM = os.path.join(os.path.dirname(__file__), "sweep_cm")


def handler(event, context):
    params = parse_body(event)
    return handle_solve_cm(params)


def handle_solve_cm(params):
    """Solve roots from coefficient file using companion matrix eigenvalues."""
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
    task_id = f"sweep_{chunk_idx}"

    try:
        report_status(job_id, task_id, "started")
        t0 = time.time()

        # Download coefficient file
        resp = s3.get_object(Bucket=BUCKET, Key=coeffs_key)
        coeffs_data = resp["Body"].read()

        coeffs_file = "/tmp/coeffs_chunk.bin"
        with open(coeffs_file, "wb") as f:
            f.write(coeffs_data)

        bin_path = "/tmp/roots_chunk.bin"
        spec = {
            "mode": "solve_cm",
            "coeffs_file": coeffs_file,
            "n_coeffs": n_coeffs,
            "n_steps": n_steps,
        }
        result = subprocess.run(
            [SWEEP_CM, bin_path],
            input=json.dumps(spec),
            capture_output=True, text=True, timeout=840
        )
        print(f"sweep_cm rc={result.returncode} stdout={repr(result.stdout[:200])} stderr={repr(result.stderr[:500])}")
        if result.returncode != 0:
            raise RuntimeError(f"solve_cm failed (rc={result.returncode}): {result.stderr.strip()}")
        if not result.stdout.strip().startswith("{"):
            raise RuntimeError(f"solve_cm produced non-JSON stdout: {result.stdout[:200]!r} stderr: {result.stderr[:200]!r}")

        compute_meta = json.loads(result.stdout)
        compute_us = int((time.time() - t0) * 1e6)

        s3_key = params.get("s3_key", f"renders/{job_id}/chunk_{chunk_idx}.bin")
        bin_size = os.path.getsize(bin_path)
        with open(bin_path, "rb") as f:
            s3.upload_fileobj(f, BUCKET, s3_key)

        for p in [coeffs_file, bin_path]:
            try:
                os.remove(p)
            except OSError:
                pass

        result_data = {
            "chunk_idx": chunk_idx,
            "stripe_idx": chunk_idx,
            "s3_key": s3_key,
            "bin_size": bin_size,
            "compute_us": compute_us,
            "n_t": compute_meta["n_t"],
            "degree": compute_meta["degree"],
            "avg_iterations": compute_meta.get("avg_iterations", 0),
        }
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response({**result_data, "n_procs": 1})

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
