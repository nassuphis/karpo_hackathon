"""
Shared utilities for all polypaint Lambda handlers.
"""
import json
import math
import os
import struct
import time

import boto3

BUCKET = os.environ.get("BUCKET", "polypaint")
JOBS_TABLE = os.environ.get("JOBS_TABLE", "polypaint-jobs")
PRESIGN_EXPIRY = 3600  # 1 hour
REF_SIZE = 4096  # reference image dimension for scale computation

_ddb = None


def s3_get_object(s3_client, key, bucket=None):
    """Wrapper around s3.get_object that includes the key in any error message."""
    bucket = bucket or BUCKET
    try:
        return s3_client.get_object(Bucket=bucket, Key=key)
    except Exception as e:
        raise RuntimeError(f"S3 GetObject failed: s3://{bucket}/{key} — {e}") from e


def _get_ddb():
    """Lazy-init DynamoDB client (avoids cold-start cost for Lambdas that don't use it)."""
    global _ddb
    if _ddb is None:
        _ddb = boto3.client("dynamodb")
    return _ddb


def report_status(job_id, task_id, status, error_msg=None, result_data=None):
    """Write task completion status to DynamoDB. TTL = 24h auto-cleanup.
    Optional result_data dict is stored as JSON string for later retrieval."""
    now_ms = int(time.time() * 1000)
    item = {
        "job_id": {"S": job_id},
        "task_id": {"S": task_id},
        "task_status": {"S": status},
        "updated_at_ms": {"N": str(now_ms)},
        "ttl": {"N": str(int(time.time()) + 86400)},
    }
    if error_msg:
        item["error_msg"] = {"S": str(error_msg)[:1000]}
    if result_data:
        item["result_data"] = {"S": json.dumps(result_data)}
    _get_ddb().put_item(TableName=JOBS_TABLE, Item=item)


def parse_body(event):
    """Parse request body from various invocation formats."""
    if isinstance(event.get("body"), str):
        return json.loads(event["body"])
    elif "body" in event and event["body"] is not None:
        return event["body"]
    return event


def ok_response(body):
    return {
        "statusCode": 200,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(body),
    }



def imgpipe_env():
    """Environment for imgpipe (needs libvips from /opt/lib)."""
    env = dict(os.environ)
    ld = env.get("LD_LIBRARY_PATH", "")
    if "/opt/lib" not in ld:
        env["LD_LIBRARY_PATH"] = "/opt/lib:" + ld
    return env


def compute_viewport_from_bin(bin_data, quantile=0.0, shim=0.05):
    """Compute viewport (center, scale) from binary root data.
    .bin format: raw f32 pairs [re, im, re, im, ...] with no header.
    Returns dict with center_re, center_im, scale (for REF_SIZE reference),
    n_roots, q_re, q_im.
    """
    n_floats = len(bin_data) // 4
    n_pairs = n_floats // 2
    if n_pairs == 0:
        return {"center_re": 0, "center_im": 0, "scale": 1.0, "n_roots": 0,
                "q_re": [0, 0], "q_im": [0, 0]}

    # Stream f32 pairs to avoid materializing all floats into a Python tuple
    usable = n_pairs * 8  # 2 floats × 4 bytes
    _isfinite = math.isfinite
    all_re = []
    all_im = []
    for re, im in struct.iter_unpack('<ff', bin_data[:usable]):
        if _isfinite(re) and _isfinite(im):
            all_re.append(re)
            all_im.append(im)

    n_roots = len(all_re)
    if n_roots == 0:
        return {"center_re": 0, "center_im": 0, "scale": 1.0, "n_roots": 0,
                "q_re": [0, 0], "q_im": [0, 0]}

    all_re.sort()
    all_im.sort()

    shim_mul = 1.0 + shim
    if n_roots > 10:
        lo = int(n_roots * quantile)
        hi = int(n_roots * (1.0 - quantile)) - 1
        lo = max(0, lo)
        hi = min(n_roots - 1, hi)
        if hi <= lo:
            lo, hi = 0, n_roots - 1
    else:
        lo, hi = 0, n_roots - 1

    q_min_re, q_max_re = all_re[lo], all_re[hi]
    q_min_im, q_max_im = all_im[lo], all_im[hi]

    center_re = (q_min_re + q_max_re) / 2.0
    center_im = (q_min_im + q_max_im) / 2.0

    range_re = (q_max_re - q_min_re) * shim_mul
    range_im = (q_max_im - q_min_im) * shim_mul

    if range_re > 0 and range_im > 0:
        scale = min(REF_SIZE / range_re, REF_SIZE / range_im)
    else:
        scale = 1.0

    return {
        "center_re": center_re,
        "center_im": center_im,
        "scale": scale,
        "n_roots": n_roots,
        "q_re": [q_min_re, q_max_re],
        "q_im": [q_min_im, q_max_im],
    }
