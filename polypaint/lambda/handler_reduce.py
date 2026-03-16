"""
Reduce Lambda handler — merges two raw pixel buffers via additive blending.

Single operation: download two .raw from S3, merge via rawreduce, upload result.
Reports completion status to DynamoDB for poll-based orchestration.
"""
import json
import os
import subprocess

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
RAWREDUCE = os.path.join(os.path.dirname(__file__), "rawreduce")


def handler(event, context):
    params = parse_body(event)
    out_key = params["out_key"]
    # Derive job_id and task_id from out_key: "renders/{job_id}/merge_0_42.raw"
    job_id = params.get("job_id") or out_key.split("/")[1]
    task_id = out_key.rsplit("/", 1)[-1].replace(".raw", "")

    try:
        report_status(job_id, task_id, "started")

        left_key = params["left_key"]
        right_key = params["right_key"]

        left_path = "/tmp/left.raw"
        right_path = "/tmp/right.raw"
        out_path = "/tmp/merged.raw"

        # Download both images
        obj = s3.get_object(Bucket=BUCKET, Key=left_key)
        with open(left_path, "wb") as f:
            f.write(obj["Body"].read())
        obj = s3.get_object(Bucket=BUCKET, Key=right_key)
        with open(right_path, "wb") as f:
            f.write(obj["Body"].read())

        report_status(job_id, task_id, "tiles_read")

        # Merge
        gamma = params.get("gamma", 2.2)
        cmd = [RAWREDUCE, left_path, right_path, out_path, f"--gamma={gamma}"]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        if result.returncode != 0:
            raise RuntimeError(f"rawreduce failed: {result.stderr.strip()}")

        report_status(job_id, task_id, "tiles_merged")

        # Upload result
        with open(out_path, "rb") as f:
            raw_data = f.read()
        s3.put_object(Bucket=BUCKET, Key=out_key,
                      Body=raw_data, ContentType="application/octet-stream")

        for p in [left_path, right_path, out_path]:
            try:
                os.remove(p)
            except OSError:
                pass

        report_status(job_id, task_id, "done")
        return ok_response({"out_key": out_key, "size": len(raw_data)})

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
