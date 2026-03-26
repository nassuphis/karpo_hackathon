"""
Render orchestrator starter Lambda — validates and starts Step Functions execution.

This is NOT the old recursive orchestrator. It:
1. Validates the launch request
2. Writes the initial queued DDB row
3. Calls StartExecution on the render state machine
4. Returns immediately

It does NOT poll DDB, dispatch workers, self-invoke, or own phase transitions.
Step Functions Standard owns the workflow after StartExecution.
"""
import json
import os
import time

import boto3

from shared import JOBS_TABLE, parse_body, ok_response, report_status

sfn_client = boto3.client("stepfunctions", region_name=os.environ.get("AWS_REGION", "us-east-1"))

STATE_MACHINE_ARN = os.environ.get("RENDER_STATE_MACHINE_ARN", "")
VALID_MODES = {"color", "bilevel", "coeff_bilevel"}


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    mode = params["mode"]

    if mode not in VALID_MODES:
        raise RuntimeError(f"Invalid render mode: {mode} (valid: {', '.join(sorted(VALID_MODES))})")

    task_id = f"render_run_{mode}_{run_id}"
    execution_name = f"render_{mode}_{run_id}"
    now_ms = int(time.time() * 1000)

    # Start Step Functions execution
    sfn_input = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": mode,
        "params": params.get("params", {}),
    }

    execution = sfn_client.start_execution(
        stateMachineArn=STATE_MACHINE_ARN,
        name=execution_name,
        input=json.dumps(sfn_input),
    )
    execution_arn = execution["executionArn"]

    # Write initial queued DDB row
    result_data = {
        "job_id": job_id,
        "run_id": run_id,
        "mode": mode,
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
