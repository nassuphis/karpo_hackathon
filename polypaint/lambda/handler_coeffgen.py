"""
Coeffgen Lambda handler — generates coefficient vectors for the composable pipeline.

Single route:
  POST /coeffgen — run sweep in coeffgen mode, upload coeffs.bin to S3

The composable pipeline is:
  [param_transforms] → coefficient_function → [coeff_transforms]

Parameter transforms chain (z1,z2)→(z1,z2) operations on the grid inputs.
The coefficient function produces a polynomial coefficient vector.
Coefficient transforms chain vector→vector operations on the result.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response

s3 = boto3.client("s3")
SWEEP = os.path.join(os.path.dirname(__file__), "sweep")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]

    bin_path = "/tmp/coeffs.bin"
    t0 = time.time()

    spec = {
        "mode": "coeffgen",
        "param_transforms": params.get("param_transforms", []),
        "function": params["function"],
        "coeff_transforms": params.get("coeff_transforms", []),
        "n1": params["n1"],
        "n2": params["n2"],
        "i1_start": 0,
        "i1_end": params["n1"],
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
        raise RuntimeError(
            f"coeffs.bin size mismatch: expected {expected_size}, got {actual_size} "
            f"(possible /tmp space exhaustion)")

    coeffs_key = f"renders/{job_id}/coeffs.bin"
    with open(bin_path, "rb") as f:
        s3.put_object(Bucket=BUCKET, Key=coeffs_key,
                      Body=f, ContentType="application/octet-stream")

    try:
        os.remove(bin_path)
    except OSError:
        pass

    return ok_response({
        "job_id": job_id,
        "coeffs_key": coeffs_key,
        "coeffs_size": meta["data_bytes"],
        "n_coeffs": meta["n_coeffs"],
        "degree": meta["degree"],
        "elapsed_us": elapsed_us,
    })
