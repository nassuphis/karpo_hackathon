"""
Coeffgen Lambda handler — generates coefficient vectors for one stripe.

Fan out N stripes in parallel from the client, same as root solving.
Each invocation generates coefficients for rows i1_start to i1_end,
uploads coeffs_{stripe_idx:04d}.bin to S3, reports progress to DynamoDB.

The composable pipeline is:
  [param_transforms] → coefficient_function → [coeff_transforms]
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
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    i1_start = params["i1_start"]
    i1_end = params["i1_end"]
    task_id = f"coeffgen_{stripe_idx}"

    try:
        report_status(job_id, task_id, "started")

        bin_path = f"/tmp/coeffs_{stripe_idx}.bin"
        t0 = time.time()

        spec = {
            "mode": "coeffgen",
            "param_transforms": params.get("param_transforms", []),
            "function": params["function"],
            "coeff_transforms": params.get("coeff_transforms", []),
            "n1": params["n1"],
            "n2": params["n2"],
            "i1_start": i1_start,
            "i1_end": i1_end,
        }

        result = subprocess.run(
            [SWEEP, bin_path],
            input=json.dumps(spec),
            capture_output=True, text=True, timeout=840
        )
        if result.returncode != 0:
            raise RuntimeError(f"coeffgen failed: {result.stderr.strip()}")

        meta = json.loads(result.stdout)
        elapsed_us = int((time.time() - t0) * 1e6)

        # Verify file size matches expected data_bytes (catch /tmp truncation)
        actual_size = os.path.getsize(bin_path)
        expected_size = meta["data_bytes"]
        if actual_size != expected_size:
            try:
                os.remove(bin_path)
            except OSError:
                pass
            raise RuntimeError(
                f"coeffs.bin size mismatch: expected {expected_size}, got {actual_size}")

        # Support s3_key override (used for lores coefficients)
        coeffs_key = params.get("s3_key", f"renders/{job_id}/coeffs_{stripe_idx:04d}.bin")
        with open(bin_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=coeffs_key,
                          Body=f, ContentType="application/octet-stream")

        try:
            os.remove(bin_path)
        except OSError:
            pass

        report_status(job_id, task_id, "done")
        return ok_response({
            "job_id": job_id,
            "stripe_idx": stripe_idx,
            "coeffs_key": coeffs_key,
            "coeffs_size": meta["data_bytes"],
            "n_coeffs": meta["n_coeffs"],
            "degree": meta["degree"],
            "elapsed_us": elapsed_us,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
