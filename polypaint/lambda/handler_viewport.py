"""
Viewport Lambda handler — computes view parameters from lores root data.

Single operation: download lores.bin from S3, compute bounding box, save view.json.
"""
import json
import os
import time

import boto3

from shared import BUCKET, parse_body, ok_response, compute_viewport_from_bin

s3 = boto3.client("s3")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    quantile = params.get("quantile", 0.0)
    shim = params.get("shim", 0.05)

    t0 = time.time()

    # Read calc.json to locate lores.bin
    obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/calc.json")
    calc = json.loads(obj["Body"].read())
    lores_key = calc["lores"]["bin_key"]

    # Download lores.bin
    obj = s3.get_object(Bucket=BUCKET, Key=lores_key)
    bin_data = obj["Body"].read()

    # Compute viewport from binary root data
    vp = compute_viewport_from_bin(bin_data, quantile=quantile, shim=shim)
    viewport_us = int((time.time() - t0) * 1e6)

    # Save view.json to S3
    view = {
        "job_id": job_id,
        "center_re": vp["center_re"],
        "center_im": vp["center_im"],
        "scale_ref": vp["scale"],  # for 4096x4096 reference
        "n_roots": vp["n_roots"],
        "q_re": vp["q_re"],
        "q_im": vp["q_im"],
        "quantile": quantile,
        "shim": shim,
    }
    s3.put_object(Bucket=BUCKET,
                  Key=f"renders/{job_id}/view.json",
                  Body=json.dumps(view),
                  ContentType="application/json")

    return ok_response({
        **view,
        "viewport_us": viewport_us,
    })
