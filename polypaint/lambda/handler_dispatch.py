"""
Dispatch Lambda handler — fires target Lambdas asynchronously via direct invocation.

Receives an array of jobs and a target function name, invokes each as an async Lambda
(fire-and-forget), returns immediately with expected S3 keys. Browser polls for completion.

Uses ThreadPoolExecutor to invoke Lambdas in parallel (~65ms per invoke,
so 500 jobs with 50 threads = ~0.7s instead of ~32s sequential).
"""
import json
import logging
import os
from concurrent.futures import ThreadPoolExecutor, as_completed

import boto3

from shared import parse_body, ok_response

logger = logging.getLogger()
logger.setLevel(logging.INFO)

FUNCTIONS = {
    "raster": os.environ.get("RASTER_FUNCTION", "polypaint-raster"),
    "finalize": os.environ.get("FINALIZE_FUNCTION", "polypaint-finalize"),
    "encode": os.environ.get("ENCODE_FUNCTION", "polypaint-encode"),
    "sweep": os.environ.get("SWEEP_FUNCTION", "polypaint-sweep"),
    "sweep_mt": os.environ.get("SWEEP_MT_FUNCTION", "polypaint-sweep-mt"),
    "bilevel": os.environ.get("BILEVEL_FUNCTION", "polypaint-bilevel"),
    "bilevel_stitch": os.environ.get("BILEVEL_STITCH_FUNCTION", "polypaint-bilevel-stitch"),
    "deepzoom_export": os.environ.get("DZ_EXPORT_FUNCTION", "polypaint-deepzoom-export"),
    "deepzoom_from_raw": os.environ.get("DZ_FROM_RAW_FUNCTION", "polypaint-deepzoom-from-raw"),
    "coeffgen": os.environ.get("COEFFGEN_FUNCTION", "polypaint-coeffgen"),
    "sweep_cm": os.environ.get("SWEEP_CM_FUNCTION", "polypaint-sweep-cm"),
    "render_preview": os.environ.get("RENDER_PREVIEW_FUNCTION", "polypaint-render-preview"),
    "solve_proximity": os.environ.get("SOLVE_PROXIMITY_FUNCTION", "polypaint-solve-proximity"),
    "render_orchestrator": os.environ.get("RENDER_ORCHESTRATOR_FUNCTION", "polypaint-render-orchestrator"),
    "compute_orchestrator": os.environ.get("COMPUTE_ORCHESTRATOR_FUNCTION", "polypaint-compute-orchestrator"),
    "palette_orchestrator": os.environ.get("PALETTE_ORCHESTRATOR_FUNCTION", "polypaint-palette-orchestrator"),
    "autolevels": os.environ.get("AUTOLEVELS_FUNCTION", "polypaint-autolevels"),
    "resize_artifact": os.environ.get("RESIZE_ARTIFACT_FUNCTION", "polypaint-resize-artifact"),
    "repalette": os.environ.get("REPALETTE_FUNCTION", "polypaint-repalette"),
    "color_repalette": os.environ.get("COLOR_REPALETTE_FUNCTION", "polypaint-color-repalette"),
    "recolor_from_raw": os.environ.get("RECOLOR_FROM_RAW_FUNCTION", "polypaint-recolor-from-raw"),
    "pdf_artifact": os.environ.get("PDF_ARTIFACT_FUNCTION", "polypaint-pdf-artifact"),
}
REGION = os.environ.get("AWS_REGION", "us-east-1")
MAX_WORKERS = int(os.environ.get("DISPATCH_THREADS", "50"))

lambda_client = boto3.client("lambda", region_name=REGION)


def _invoke_one(function_name, job):
    """Invoke a single Lambda asynchronously. Returns HTTP status code."""
    resp = lambda_client.invoke(
        FunctionName=function_name,
        InvocationType="Event",
        Payload=json.dumps({"body": json.dumps(job)}),
    )
    status = resp.get("StatusCode", 0)
    # Event invocations return 202 on success. Anything else is a problem.
    if status != 202:
        job_id = job.get("job_id", "?")
        task_hint = job.get(
            "tile_idx",
            job.get("chunk_idx", job.get("section_idx", job.get("stripe_idx", job.get("task_id", "?")))),
        )
        logger.warning(f"invoke {function_name} returned {status} for job_id={job_id} task={task_hint}")
    return status


def handler(event, context):
    params = parse_body(event)
    target = params.get("target")
    if not target:
        return ok_response({"error": "Missing 'target' parameter", "fired": 0, "total": 0})
    function_name = FUNCTIONS.get(target)
    if not function_name:
        return ok_response({"error": f"Unknown target: {target}", "fired": 0, "total": 0})

    jobs = params["jobs"]

    logger.info(f"DISPATCH target={target} function={function_name} jobs={len(jobs)}")

    # Fire all Lambdas in parallel using thread pool
    fired = 0
    errors = []
    non_202 = []
    with ThreadPoolExecutor(max_workers=MAX_WORKERS) as pool:
        futures = {pool.submit(_invoke_one, function_name, job): i
                   for i, job in enumerate(jobs)}
        for future in as_completed(futures):
            idx = futures[future]
            try:
                status = future.result()
                if status == 202:
                    fired += 1
                else:
                    non_202.append({"idx": idx, "status": status})
                    fired += 1  # still count as fired — Lambda may run
            except Exception as e:
                errors.append(f"job[{idx}]: {e}")

    logger.info(f"DISPATCH DONE fired={fired}/{len(jobs)} errors={len(errors)} non_202={len(non_202)}")

    result = {
        "fired": fired,
        "total": len(jobs),
    }
    if errors:
        result["errors"] = errors[:10]
    if non_202:
        result["non_202"] = non_202[:10]
    return ok_response(result)
