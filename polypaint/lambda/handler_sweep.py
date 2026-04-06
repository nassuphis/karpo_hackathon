"""
Sweep Lambda handler — solves polynomial roots from pre-computed coefficient files.

Route:
  POST /sweep — solve roots from a coefficient chunk (.bin), upload roots to S3.

Input must include coeffs_key. The solver reads coefficient records sequentially
until EOF. Accepts either chunk-native n_steps or legacy N/row_start/row_end.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
SWEEP = os.path.join(os.path.dirname(__file__), "sweep")


def handler(event, context):
    params = parse_body(event)
    return handle_solve_from_coeffs(params)


def handle_solve_from_coeffs(params):
    """Solve roots from a coefficient file (one chunk or one stripe).

    Downloads the coefficient file from S3, runs sweep in 'solve' mode,
    uploads root .bin to S3. Reports status to DynamoDB for async polling.
    """
    job_id = params["job_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    coeffs_key = params["coeffs_key"]
    n_coeffs = params["n_coeffs"]
    # Accept chunk-native n_steps, or legacy N/row_start/row_end
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

        # Download coefficient file (whole file, no range reads)
        try:
            resp = s3.get_object(Bucket=BUCKET, Key=coeffs_key)
        except Exception as e:
            raise RuntimeError(f"Failed to download coefficients s3://{BUCKET}/{coeffs_key}: {e}") from e
        coeffs_data = resp["Body"].read()

        coeffs_file = "/tmp/coeffs_stripe.bin"
        with open(coeffs_file, "wb") as f:
            f.write(coeffs_data)

        bin_path = "/tmp/stripe.bin"
        # Solver reads until EOF; n2/i1 are only for metadata output.
        # Pass n_steps as a single-row grid so metadata reports correct counts.
        spec = {
            "mode": "solve",
            "coeffs_file": coeffs_file,
            "n_coeffs": n_coeffs,
            "n2": n_steps,
            "i1_start": 0,
            "i1_end": 1,
            "match_roots": False,
        }
        result = subprocess.run(
            [SWEEP, bin_path],
            input=json.dumps(spec),
            capture_output=True, text=True, timeout=840
        )
        if result.returncode != 0:
            raise RuntimeError(f"solve failed: {result.stderr.strip()}")
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
            "avg_iterations": compute_meta["avg_iterations"],
        }
        report_status(job_id, task_id, "done", result_data=result_data)

        return ok_response({**result_data, "n_procs": 1})

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
