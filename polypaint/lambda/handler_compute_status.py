"""
Compute status Lambda - updates the top-level compute run DDB row.

Called by Step Functions between phases to report progress to the browser.
Does NOT dispatch workers, poll for completion, or perform phase transitions.

Actions: queued, phase, done, error.
"""
import json
import time

import boto3

from shared import JOBS_TABLE, ok_response, parse_body

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
    if action == "phase":
        return _write_phase(params, job_id, task_id)
    if action == "done":
        return _write_done(params, job_id, task_id)
    if action == "error":
        return _write_error(params, job_id, task_id)
    raise RuntimeError(f"Unknown status action: {action}")


def _base_result_data(params, now_ms):
    run_started_at_ms = params.get("run_started_at_ms", params.get("started_at_ms", now_ms))
    return {
        "job_id": params["job_id"],
        "run_id": params.get("run_id", ""),
        "mode": params.get("mode", "compute"),
        "solver_mode": params.get("solver_mode", ""),
        "phase": params.get("phase", ""),
        "phase_label": params.get("phase_label", ""),
        "execution_arn": params.get("execution_arn", ""),
        "started_at_ms": params.get("started_at_ms", now_ms),
        "run_started_at_ms": run_started_at_ms,
        "updated_at_ms": now_ms,
    }


def _write_queued(params, job_id, task_id):
    now_ms = int(time.time() * 1000)
    result_data = _base_result_data({
        **params,
        "phase": "queued",
        "phase_label": "Queued",
    }, now_ms)
    _put_row(job_id, task_id, "queued", result_data)
    return ok_response({"action": "queued", "task_id": task_id})


def _write_phase(params, job_id, task_id):
    now_ms = int(time.time() * 1000)
    result_data = _base_result_data(params, now_ms)
    if "expected" in params:
        result_data["expected"] = params["expected"]
    if "subtask_prefix" in params:
        result_data["subtask_prefix"] = params["subtask_prefix"]
    _put_row(job_id, task_id, params["phase"], result_data)
    return ok_response({"action": "phase", "phase": params["phase"]})


def _write_done(params, job_id, task_id):
    now_ms = int(time.time() * 1000)
    result_data = _base_result_data({
        **params,
        "phase": "done",
        "phase_label": "Done",
    }, now_ms)
    for key in ("calc_key", "degree", "n_chunks", "solver"):
        if key in params:
            result_data[key] = params[key]
    _put_row(job_id, task_id, "done", result_data)
    return ok_response({"action": "done", "task_id": task_id})


def _write_error(params, job_id, task_id):
    now_ms = int(time.time() * 1000)
    error_msg = _extract_error_message(params)
    previous = _get_existing_result_data(job_id, task_id)
    result_data = _base_result_data({
        **params,
        "phase": "error",
        "phase_label": "Error",
    }, now_ms)
    _attach_failed_phase_context(result_data, params, previous)
    _put_row(job_id, task_id, "error", result_data, error_msg=error_msg)
    return ok_response({"action": "error", "error_msg": error_msg})


def _extract_error_message(params):
    if "error_msg" in params:
        return str(params["error_msg"])[:1000]
    cause = params.get("Cause", "")
    error = params.get("Error", "")
    if cause:
        try:
            cause_obj = json.loads(cause)
            if "errorMessage" in cause_obj:
                return str(cause_obj["errorMessage"])[:1000]
            payload = cause_obj.get("Payload", {})
            if isinstance(payload, str):
                payload = json.loads(payload)
            if isinstance(payload, dict) and "errorMessage" in payload:
                return str(payload["errorMessage"])[:1000]
            if "Cause" in cause_obj:
                return str(cause_obj["Cause"])[:1000]
        except (json.JSONDecodeError, TypeError, AttributeError):
            pass
        return str(cause)[:1000]
    if error:
        return str(error)[:1000]
    return "Unknown error"


def _get_existing_result_data(job_id, task_id):
    """Best-effort read of the last phase row before the error overwrite."""
    try:
        resp = _get_ddb().get_item(
            TableName=JOBS_TABLE,
            Key={"job_id": {"S": job_id}, "task_id": {"S": task_id}},
        )
        raw = resp.get("Item", {}).get("result_data", {}).get("S")
        if not raw:
            return {}
        parsed = json.loads(raw)
        return parsed if isinstance(parsed, dict) else {}
    except Exception:
        # Error reporting must not fail because the previous row is absent or
        # malformed; the current error message is still the authoritative row.
        return {}


def _attach_failed_phase_context(result_data, params, previous):
    failed_phase = params.get("failed_phase") or previous.get("phase")
    if failed_phase and failed_phase not in {"error", "done"}:
        result_data["failed_phase"] = str(failed_phase)

    failed_label = params.get("failed_phase_label") or previous.get("phase_label")
    if failed_label:
        result_data["failed_phase_label"] = str(failed_label)

    failed_subtask_prefix = params.get("failed_subtask_prefix") or previous.get("subtask_prefix")
    if failed_subtask_prefix:
        result_data["failed_subtask_prefix"] = str(failed_subtask_prefix)

    if "expected" in previous:
        result_data["failed_expected"] = previous["expected"]
    if "Error" in params:
        result_data["failed_error"] = str(params["Error"])[:1000]


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
