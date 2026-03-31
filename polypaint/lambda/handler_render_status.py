"""
Render status Lambda — updates the top-level render run DDB row.

Called by Step Functions between phases to report progress to the browser.
Does NOT dispatch workers, poll for completion, or perform phase transitions.

Actions: queued, phase, done, error.
"""
import json
import time

from shared import JOBS_TABLE, parse_body, ok_response

import boto3

_ddb = None


def _get_ddb():
    global _ddb
    if _ddb is None:
        _ddb = boto3.client("dynamodb")
    return _ddb


def handler(event, context):
    params = parse_body(event)
    action = params["action"]
    job_id = params["job_id"]
    task_id = params["task_id"]

    if action == "queued":
        return _write_queued(params, job_id, task_id)
    elif action == "phase":
        return _write_phase(params, job_id, task_id)
    elif action == "done":
        return _write_done(params, job_id, task_id)
    elif action == "error":
        return _write_error(params, job_id, task_id)
    else:
        raise RuntimeError(f"Unknown status action: {action}")


def _write_queued(params, job_id, task_id):
    now_ms = int(time.time() * 1000)
    result_data = {
        "job_id": job_id,
        "run_id": params["run_id"],
        "mode": params["mode"],
        "phase": "queued",
        "phase_label": "Queued",
        "execution_arn": params.get("execution_arn", ""),
        "started_at_ms": params.get("started_at_ms", now_ms),
        "updated_at_ms": now_ms,
    }
    _put_row(job_id, task_id, "queued", result_data)
    return ok_response({"action": "queued", "task_id": task_id})


def _write_phase(params, job_id, task_id):
    now_ms = int(time.time() * 1000)
    result_data = {
        "job_id": job_id,
        "run_id": params.get("run_id", ""),
        "mode": params.get("mode", ""),
        "phase": params["phase"],
        "phase_label": params["phase_label"],
        "execution_arn": params.get("execution_arn", ""),
        "started_at_ms": params.get("started_at_ms", now_ms),
        "updated_at_ms": now_ms,
    }
    if "expected" in params:
        result_data["expected"] = params["expected"]
    if "subtask_prefix" in params:
        result_data["subtask_prefix"] = params["subtask_prefix"]
    _put_row(job_id, task_id, params["phase"], result_data)
    return ok_response({"action": "phase", "phase": params["phase"]})


def _write_done(params, job_id, task_id):
    now_ms = int(time.time() * 1000)
    result_data = {
        "job_id": job_id,
        "run_id": params.get("run_id", ""),
        "mode": params.get("mode", ""),
        "phase": "done",
        "phase_label": "Done",
        "execution_arn": params.get("execution_arn", ""),
        "started_at_ms": params.get("started_at_ms", now_ms),
        "updated_at_ms": now_ms,
    }
    if "image_key" in params:
        result_data["image_key"] = params["image_key"]
    if "artifact_id" in params:
        result_data["artifact_id"] = params["artifact_id"]
    if "family" in params:
        result_data["family"] = params["family"]
    if "palette_id" in params:
        result_data["palette_id"] = params["palette_id"]
    _put_row(job_id, task_id, "done", result_data)
    return ok_response({"action": "done", "task_id": task_id})


def _write_error(params, job_id, task_id):
    now_ms = int(time.time() * 1000)
    error_msg = _extract_error_message(params)
    result_data = {
        "job_id": job_id,
        "run_id": params.get("run_id", ""),
        "mode": params.get("mode", ""),
        "phase": "error",
        "phase_label": "Error",
        "execution_arn": params.get("execution_arn", ""),
        "started_at_ms": params.get("started_at_ms", now_ms),
        "updated_at_ms": now_ms,
    }
    _put_row(job_id, task_id, "error", result_data, error_msg=error_msg)
    return ok_response({"action": "error", "error_msg": error_msg})


def _extract_error_message(params):
    """Extract human-readable error from Step Functions error envelope or plain string."""
    # Direct error_msg field
    if "error_msg" in params:
        return str(params["error_msg"])[:1000]

    # Step Functions Catch output has Error + Cause
    cause = params.get("Cause", "")
    error = params.get("Error", "")

    if cause:
        # Cause is often a JSON string with nested errorMessage
        try:
            cause_obj = json.loads(cause)
            # Lambda error: {"errorMessage": "...", "errorType": "..."}
            if "errorMessage" in cause_obj:
                return str(cause_obj["errorMessage"])[:1000]
            # Nested Payload error
            payload = cause_obj.get("Payload", {})
            if isinstance(payload, str):
                payload = json.loads(payload)
            if isinstance(payload, dict) and "errorMessage" in payload:
                return str(payload["errorMessage"])[:1000]
            # Nested Cause
            if "Cause" in cause_obj:
                return str(cause_obj["Cause"])[:1000]
        except (json.JSONDecodeError, TypeError, AttributeError):
            pass
        return str(cause)[:1000]

    if error:
        return str(error)[:1000]

    return "Unknown error"


def _put_row(job_id, task_id, status, result_data, error_msg=None):
    now_ms = int(time.time() * 1000)
    item = {
        "job_id": {"S": job_id},
        "task_id": {"S": task_id},
        "task_status": {"S": status},
        "updated_at_ms": {"N": str(now_ms)},
        "ttl": {"N": str(int(time.time()) + 86400)},
        "result_data": {"S": json.dumps(result_data)},
    }
    if error_msg:
        item["error_msg"] = {"S": str(error_msg)[:1000]}
    _get_ddb().put_item(TableName=JOBS_TABLE, Item=item)
