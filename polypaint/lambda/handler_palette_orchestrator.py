"""
Palette orchestrator starter Lambda — validates and starts the Palette Step Functions workflow.

It mirrors render orchestration:
1. validates the launch request
2. writes the initial queued DDB row
3. calls StartExecution on the palette state machine
4. returns immediately
"""
import json
import os
import time

import boto3

from shared import parse_body, ok_response, report_status

sfn_client = boto3.client("stepfunctions", region_name=os.environ.get("AWS_REGION", "us-east-1"))

STATE_MACHINE_ARN = os.environ.get("PALETTE_STATE_MACHINE_ARN", "")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    task_id = params.get("task_id", f"palette_run_{run_id}")
    now_ms = int(time.time() * 1000)
    if not STATE_MACHINE_ARN:
        raise RuntimeError("PALETTE_STATE_MACHINE_ARN is not configured")

    sfn_input = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "palette",
        "params": params.get("params", {}),
    }

    execution = sfn_client.start_execution(
        stateMachineArn=STATE_MACHINE_ARN,
        name=f"palette_{run_id}",
        input=json.dumps(sfn_input),
    )
    execution_arn = execution["executionArn"]

    result_data = {
        "job_id": job_id,
        "run_id": run_id,
        "mode": "palette",
        "phase": "queued",
        "phase_label": "Queued",
        "execution_arn": execution_arn,
        "started_at_ms": now_ms,
        "updated_at_ms": now_ms,
    }
    report_status(job_id, task_id, "queued", result_data=result_data)

    return ok_response({
        "execution_arn": execution_arn,
        "task_id": task_id,
        "run_id": run_id,
    })
