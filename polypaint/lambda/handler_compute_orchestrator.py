"""
Compute orchestrator starter Lambda — validates and starts the Compute Step Functions workflow.

It mirrors render orchestration:
1. validates the launch request
2. writes the initial queued DDB row
3. calls StartExecution on the compute state machine
4. returns immediately
"""
import json
import os
import time

import boto3

from compute_fused import execution_method_from_params
from shared import ok_response, parse_body, report_status

sfn_client = boto3.client("stepfunctions", region_name=os.environ.get("AWS_REGION", "us-east-1"))

STATE_MACHINE_ARN = os.environ.get("COMPUTE_STATE_MACHINE_ARN", "")
VALID_SOLVERS = {"aberth", "aberth_mt", "companion_matrix"}


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    run_params = dict(params.get("params", {}) or {})
    solver_mode = str(run_params.get("solver_mode", "aberth")).strip().lower()
    if solver_mode not in VALID_SOLVERS:
        raise RuntimeError(f"Invalid compute solver_mode: {solver_mode!r}")
    if not STATE_MACHINE_ARN:
        raise RuntimeError("COMPUTE_STATE_MACHINE_ARN is not configured")

    if solver_mode == "aberth_mt":
        execution_method = execution_method_from_params(run_params)
    else:
        execution_method = "classic_chunk_pipeline"
    run_params.pop("fused", None)
    run_params["execution_method"] = execution_method

    task_id = f"compute_run_{solver_mode}_{run_id}"
    execution_name = f"compute_{solver_mode}_{run_id}"
    now_ms = int(time.time() * 1000)

    sfn_input = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "compute",
        "run_started_at_ms": now_ms,
        "params": run_params,
    }

    execution = sfn_client.start_execution(
        stateMachineArn=STATE_MACHINE_ARN,
        name=execution_name,
        input=json.dumps(sfn_input),
    )
    execution_arn = execution["executionArn"]

    result_data = {
        "job_id": job_id,
        "run_id": run_id,
        "mode": "compute",
        "solver_mode": solver_mode,
        "phase": "queued",
        "phase_label": "Queued",
        "execution_arn": execution_arn,
        "started_at_ms": now_ms,
        "run_started_at_ms": now_ms,
        "updated_at_ms": now_ms,
    }
    report_status(job_id, task_id, "queued", result_data=result_data)

    return ok_response({
        "execution_arn": execution_arn,
        "task_id": task_id,
        "run_id": run_id,
    })
