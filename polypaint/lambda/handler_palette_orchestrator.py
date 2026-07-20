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

from shared import JOBS_TABLE, parse_body, ok_response, report_status, assert_safe_id, resolve_bound_execution_arn

sfn_client = boto3.client("stepfunctions", region_name=os.environ.get("AWS_REGION", "us-east-1"))
ddb_client = boto3.client("dynamodb", region_name=os.environ.get("AWS_REGION", "us-east-1"))

STATE_MACHINE_ARN = os.environ.get("PALETTE_STATE_MACHINE_ARN", "")


def handler(event, context):
    params = parse_body(event)
    if str(params.get("action") or "").strip().lower() == "stop":
        return handle_stop(params)
    job_id = params["job_id"]
    # run_id feeds the SFN execution name (name=f"palette_{run_id}") below (F9)
    run_id = assert_safe_id(params["run_id"], "run_id")
    task_id = params.get("task_id", f"palette_run_{run_id}")
    artifact_id = str(params.get("artifact_id") or "").strip()
    mode = "extract_palette" if artifact_id else "palette"
    now_ms = int(time.time() * 1000)
    if not STATE_MACHINE_ARN:
        raise RuntimeError("PALETTE_STATE_MACHINE_ARN is not configured")

    sfn_input = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": mode,
        "params": params.get("params", {}),
    }
    if artifact_id:
        sfn_input["artifact_id"] = artifact_id

    execution = sfn_client.start_execution(
        stateMachineArn=STATE_MACHINE_ARN,
        name=f"palette_{run_id}",
        input=json.dumps(sfn_input),
    )
    execution_arn = execution["executionArn"]

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
    if artifact_id:
        result_data["artifact_id"] = artifact_id
    report_status(job_id, task_id, "queued", result_data=result_data)

    return ok_response({
        "execution_arn": execution_arn,
        "task_id": task_id,
        "run_id": run_id,
    })


def handle_stop(params):
    """Stop a running workflow from the jobs rail. The stop targets the
    execution recorded in the run's OWN status row (CR35-F11): the
    client ARN is only a staleness check, and a mismatched or unbound
    request is refused — a stale payload can no longer stop execution A
    while marking job B as stopped."""
    job_id = params["job_id"]
    task_id = str(params.get("task_id") or "").strip()
    bound_arn = resolve_bound_execution_arn(
        ddb_client, JOBS_TABLE,
        job_id=job_id, task_id=task_id,
        client_arn=params.get("execution_arn"),
        state_machine_arn=STATE_MACHINE_ARN,
    )
    try:
        sfn_client.stop_execution(
            executionArn=bound_arn,
            error="UserStopped",
            cause="Stopped from the jobs rail",
        )
    except sfn_client.exceptions.ExecutionDoesNotExist:
        pass   # already finished/expired; still mark the row stopped
    report_status(job_id, task_id, "error", "Stopped by user", result_data={
        "phase": "error",
        "phase_label": "Stopped",
        "stopped_by_user": True,
    })
    return ok_response({"stopped": True, "task_id": task_id})
