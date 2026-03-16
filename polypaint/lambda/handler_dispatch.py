"""
Dispatch Lambda handler — fires target Lambdas asynchronously via direct invocation.

Receives an array of jobs and a target function name, invokes each as an async Lambda
(fire-and-forget), returns immediately with expected S3 keys. Browser polls for completion.

Uses ThreadPoolExecutor to invoke Lambdas in parallel (~65ms per invoke,
so 500 jobs with 50 threads = ~0.7s instead of ~32s sequential).
"""
import json
import os
from concurrent.futures import ThreadPoolExecutor, as_completed

import boto3

from shared import parse_body, ok_response

FUNCTIONS = {
    "render": os.environ.get("RENDER_FUNCTION", "polypaint-render"),
    "tile_reduce": os.environ.get("TILE_REDUCE_FUNCTION", "polypaint-tile-reduce"),
    "reduce": os.environ.get("REDUCE_FUNCTION", "polypaint-reduce"),
    "raster": os.environ.get("RASTER_FUNCTION", "polypaint-raster"),
    "finalize": os.environ.get("FINALIZE_FUNCTION", "polypaint-finalize"),
    "encode": os.environ.get("ENCODE_FUNCTION", "polypaint-encode"),
}
REGION = os.environ.get("AWS_REGION", "us-east-1")
MAX_WORKERS = int(os.environ.get("DISPATCH_THREADS", "50"))

lambda_client = boto3.client("lambda", region_name=REGION)


def _invoke_one(function_name, job):
    """Invoke a single Lambda asynchronously."""
    lambda_client.invoke(
        FunctionName=function_name,
        InvocationType="Event",
        Payload=json.dumps({"body": json.dumps(job)}),
    )


def handler(event, context):
    params = parse_body(event)
    target = params.get("target", "render")
    function_name = FUNCTIONS.get(target)
    if not function_name:
        return ok_response({"error": f"Unknown target: {target}", "fired": 0, "total": 0})

    jobs = params["jobs"]

    # Fire all Lambdas in parallel using thread pool
    fired = 0
    errors = []
    with ThreadPoolExecutor(max_workers=MAX_WORKERS) as pool:
        futures = {pool.submit(_invoke_one, function_name, job): i
                   for i, job in enumerate(jobs)}
        for future in as_completed(futures):
            try:
                future.result()
                fired += 1
            except Exception as e:
                errors.append(str(e))

    # Expected S3 keys: caller can provide them, otherwise build from render job fields
    expected_keys = params.get("expected_keys")
    if expected_keys is None:
        expected_keys = []
        for job in jobs:
            job_id = job["job_id"]
            stripe_idx = job["stripe_idx"]
            tile_idx = job.get("tile_idx")
            if tile_idx is not None:
                expected_keys.append(f"renders/{job_id}/stripe_{stripe_idx}_t{tile_idx}.raw")
            else:
                expected_keys.append(f"renders/{job_id}/stripe_{stripe_idx}.raw")

    result = {
        "fired": fired,
        "total": len(jobs),
        "expected_keys": expected_keys,
    }
    if errors:
        result["errors"] = errors[:10]
    return ok_response(result)
