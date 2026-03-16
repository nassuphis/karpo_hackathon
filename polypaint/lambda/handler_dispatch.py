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
    target = params.get("target")
    if not target:
        return ok_response({"error": "Missing 'target' parameter", "fired": 0, "total": 0})
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

    result = {
        "fired": fired,
        "total": len(jobs),
    }
    if errors:
        result["errors"] = errors[:10]
    return ok_response(result)
