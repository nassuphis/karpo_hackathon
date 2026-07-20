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

from shared import JOBS_TABLE, parse_body, ok_response, report_status, assert_safe_id, resolve_bound_execution_arn

sfn_client = boto3.client("stepfunctions", region_name=os.environ.get("AWS_REGION", "us-east-1"))
ddb_client = boto3.client("dynamodb", region_name=os.environ.get("AWS_REGION", "us-east-1"))

STATE_MACHINE_ARN = os.environ.get("RENDER_STATE_MACHINE_ARN", "")
VALID_MODES = {"color", "bilevel", "coeff_bilevel"}
TERMINAL_STATUSES = {"done", "error"}
ACTIVE_RUN_STALE_MS = int(os.environ.get("ACTIVE_RUN_STALE_MS", str(24 * 60 * 60 * 1000)) or 0)


def _json_response(status_code, body):
    return {
        "statusCode": int(status_code),
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(body),
    }


def _active_execution_for_job(job_id, task_prefix):
    now_ms = int(time.time() * 1000)
    resp = ddb_client.query(
        TableName=JOBS_TABLE,
        KeyConditionExpression="job_id = :job_id AND begins_with(task_id, :task_prefix)",
        ExpressionAttributeValues={
            ":job_id": {"S": str(job_id)},
            ":task_prefix": {"S": str(task_prefix)},
        },
        ConsistentRead=True,
    )
    active = []
    for item in resp.get("Items", []):
        status = item.get("task_status", {}).get("S", "")
        if status in TERMINAL_STATUSES:
            continue
        task_id = item.get("task_id", {}).get("S", "")
        updated = int(item.get("updated_at_ms", {}).get("N", "0") or 0)
        if ACTIVE_RUN_STALE_MS > 0 and updated > 0 and now_ms - updated > ACTIVE_RUN_STALE_MS:
            continue
        result_data = {}
        raw_result = item.get("result_data", {}).get("S")
        if raw_result:
            try:
                parsed = json.loads(raw_result)
                if isinstance(parsed, dict):
                    result_data = parsed
            except (TypeError, ValueError):
                result_data = {}
        active.append({
            "task_id": task_id,
            "status": status,
            "phase": result_data.get("phase", status),
            "phase_label": result_data.get("phase_label", ""),
            "execution_arn": result_data.get("execution_arn", ""),
            "updated_at_ms": updated,
        })
    if not active:
        return None
    active.sort(key=lambda row: row.get("updated_at_ms", 0), reverse=True)
    return active[0]


def handler(event, context):
    params = parse_body(event)
    if str(params.get("action") or "").strip().lower() == "stop":
        return handle_stop(params)
    job_id = params["job_id"]
    # run_id feeds the SFN execution name + DDB task ids below (F9)
    run_id = assert_safe_id(params["run_id"], "run_id")
    mode = params["mode"]

    if mode not in VALID_MODES:
        raise RuntimeError(f"Invalid render mode: {mode} (valid: {', '.join(sorted(VALID_MODES))})")

    task_id = f"render_run_{mode}_{run_id}"
    execution_name = f"render_{mode}_{run_id}"
    now_ms = int(time.time() * 1000)
    active = _active_execution_for_job(job_id, f"render_run_{mode}_")
    if active:
        return _json_response(409, {
            "error": "A render run is already active for this job and mode.",
            "job_id": job_id,
            "mode": mode,
            "active_task_id": active["task_id"],
            "active_status": active["status"],
            "active_phase": active["phase"],
            "active_phase_label": active["phase_label"],
            "execution_arn": active["execution_arn"],
        })

    # Start Step Functions execution
    sfn_input = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": mode,
        "run_started_at_ms": now_ms,
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
        "run_started_at_ms": now_ms,
        "updated_at_ms": now_ms,
    }
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
