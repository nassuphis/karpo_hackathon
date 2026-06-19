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
from shared import JOBS_TABLE, ok_response, parse_body, report_status

sfn_client = boto3.client("stepfunctions", region_name=os.environ.get("AWS_REGION", "us-east-1"))
ddb_client = boto3.client("dynamodb", region_name=os.environ.get("AWS_REGION", "us-east-1"))

STATE_MACHINE_ARN = os.environ.get("COMPUTE_STATE_MACHINE_ARN", "")
VALID_SOLVERS = {"aberth_mt", "companion_matrix"}
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
    job_id = params["job_id"]
    run_id = params["run_id"]
    run_params = dict(params.get("params", {}) or {})
    solver_mode = str(run_params.get("solver_mode", "aberth_mt")).strip().lower()
    if solver_mode not in VALID_SOLVERS:
        raise RuntimeError(f"Invalid compute solver_mode: {solver_mode!r}")
    if not STATE_MACHINE_ARN:
        raise RuntimeError("COMPUTE_STATE_MACHINE_ARN is not configured")

    execution_method = execution_method_from_params(run_params)
    run_params.pop("fused", None)
    run_params["execution_method"] = execution_method
    run_params.setdefault("param_transforms", [])
    run_params.setdefault("param_program_chain", [])
    run_params.setdefault("param_program_source_text", "")
    run_params.setdefault("coeff_transforms", [])
    run_params.setdefault("coeff_program_chain", [])
    run_params.setdefault("coeff_program_source_text", "")
    run_params.setdefault(
        "pipeline_mode",
        "program" if (
            run_params.get("param_program_chain")
            or str(run_params.get("param_program_source_text") or "").strip()
            or run_params.get("coeff_program_chain")
            or str(run_params.get("coeff_program_source_text") or "").strip()
        ) else "chain",
    )

    task_id = f"compute_run_{solver_mode}_{run_id}"
    execution_name = f"compute_{solver_mode}_{run_id}"
    now_ms = int(time.time() * 1000)
    active = _active_execution_for_job(job_id, f"compute_run_{solver_mode}_")
    if active:
        return _json_response(409, {
            "error": "A compute run is already active for this job and solver_mode.",
            "job_id": job_id,
            "solver_mode": solver_mode,
            "active_task_id": active["task_id"],
            "active_status": active["status"],
            "active_phase": active["phase"],
            "active_phase_label": active["phase_label"],
            "execution_arn": active["execution_arn"],
        })

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
