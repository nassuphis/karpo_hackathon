"""
Compute workflow plan/finalize Lambda.

Actions:
  build_plan (default): validate request and build compute workflow plan.
  post_coeffgen: derive degree/n_coeffs and lores sizing from coeffgen task rows.
  finalize_metadata: write calc.json after lores + hires solve complete.
"""
import json
import math
import os

import boto3

from shared import BUCKET, JOBS_TABLE, ok_response, parse_body

s3 = boto3.client("s3")
_ddb = None

SWEEP_FUNCTION = os.environ.get("SWEEP_FUNCTION", "polypaint-sweep")
SWEEP_MT_FUNCTION = os.environ.get("SWEEP_MT_FUNCTION", "polypaint-sweep-mt")
SWEEP_CM_FUNCTION = os.environ.get("SWEEP_CM_FUNCTION", "polypaint-sweep-cm")

TARGET_PREVIEW_ROOTS = 250000
MAX_N = 50000
MAX_TIMES = 1000
MAX_CHUNKS = 5000
MAX_TOTAL_STEPS = 2_500_000_000
MAX_PARAM_GEN_THREADS = 64


def _get_ddb():
    global _ddb
    if _ddb is None:
        _ddb = boto3.client("dynamodb")
    return _ddb


def handler(event, context):
    params = parse_body(event)
    action = str(params.get("action", "build_plan")).strip().lower()
    if action == "build_plan":
        return handle_build_plan(params)
    if action == "post_coeffgen":
        return handle_post_coeffgen(params)
    if action == "finalize_metadata":
        return handle_finalize_metadata(params)
    raise RuntimeError(f"Unknown compute_plan action: {action}")


def handle_build_plan(params):
    job_id = params["job_id"]
    run_id = params["run_id"]
    task_id = params["task_id"]
    run_params = params.get("params", {})

    solver_mode = _validate_solver_mode(run_params.get("solver_mode", "aberth"))
    n = _validate_positive_int(run_params.get("N"), "N", max_value=MAX_N)
    times = _validate_positive_int(run_params.get("times", 1), "times", max_value=MAX_TIMES)
    requested_chunks = _validate_positive_int(run_params.get("n_chunks", 10), "n_chunks", max_value=MAX_CHUNKS)
    param_gen_threads = _validate_positive_int(
        run_params.get("param_gen_threads", 1),
        "param_gen_threads",
        max_value=MAX_PARAM_GEN_THREADS,
    )
    coeffgen_threads = _validate_positive_int(
        run_params.get("coeffgen_threads", param_gen_threads),
        "coeffgen_threads",
        max_value=MAX_PARAM_GEN_THREADS,
    )
    lores_param_gen_threads = _validate_positive_int(
        run_params.get("lores_param_gen_threads", param_gen_threads),
        "lores_param_gen_threads",
        max_value=MAX_PARAM_GEN_THREADS,
    )
    lores_coeffgen_threads = _validate_positive_int(
        run_params.get("lores_coeffgen_threads", coeffgen_threads),
        "lores_coeffgen_threads",
        max_value=MAX_PARAM_GEN_THREADS,
    )
    function_name = str(run_params.get("function", "")).strip()
    if not function_name:
        raise RuntimeError("function is required")

    total_steps = n * n * times
    if total_steps > MAX_TOTAL_STEPS:
        raise RuntimeError(f"Total compute steps too large: {total_steps}")

    actual_chunks = min(requested_chunks, total_steps)
    chunk_size = int(math.ceil(total_steps / actual_chunks))
    chunk_items = []
    for chunk_idx in range(actual_chunks):
        step_start = chunk_idx * chunk_size
        step_count = min(chunk_size, total_steps - step_start)
        if step_count <= 0:
            break
        chunk_items.append({
            "chunk_idx": chunk_idx,
            "step_start": step_start,
            "step_count": step_count,
            "params_key": f"renders/{job_id}/params_{chunk_idx:04d}.bin",
            "params_bin_size": int(step_count) * 16,
            "params_step_start": 0,
            "params_step_count": step_count,
            "paramgen_task_id": f"compute_{run_id}_param_gen_{chunk_idx}",
            "coeffs_key": f"renders/{job_id}/coeffs_{chunk_idx:04d}.bin",
            "coeffgen_task_id": f"compute_{run_id}_coeffgen_{chunk_idx}",
            "solve_task_id": f"compute_{run_id}_solve_{chunk_idx}",
            "bin_key": f"renders/{job_id}/chunk_{chunk_idx}.bin",
        })

    coeff_transforms = run_params.get("coeff_transforms")
    if coeff_transforms is None:
        coeff_transforms = []
    if not isinstance(coeff_transforms, list):
        raise RuntimeError("coeff_transforms must be an array")

    param_transforms = run_params.get("param_transforms")
    if param_transforms is None:
        param_transforms = []
    if not isinstance(param_transforms, list):
        raise RuntimeError("param_transforms must be an array")

    cfpv = run_params.get("cfpv")
    if cfpv in (None, ""):
        cfpv = []
    if not isinstance(cfpv, list):
        raise RuntimeError("cfpv must be an array")

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "compute",
        "pipeline": {
            "function": function_name,
            "param_transforms": param_transforms,
            "param_transforms_display": param_transforms,
            "coeff_transforms": coeff_transforms,
            "cfpv": cfpv,
        },
        "compute": {
            "N": n,
            "times": times,
            "n_chunks": len(chunk_items),
            "n_steps": total_steps,
            "param_storage_mode": "chunked",
            "params_key": "",
            "param_gen_threads": param_gen_threads,
            "coeffgen_threads": coeffgen_threads,
            "lores_param_gen_threads": lores_param_gen_threads,
            "lores_coeffgen_threads": lores_coeffgen_threads,
        },
        "param_gen": {
            "task_id": f"compute_{run_id}_param_gen",
            "task_prefix": f"compute_{run_id}_param_gen_",
            "storage_mode": "chunked",
            "params_key": "",
            "threads": param_gen_threads,
        },
        "coeffgen": {
            "task_prefix": f"compute_{run_id}_coeffgen_",
            "threads": coeffgen_threads,
        },
        "solve": {
            "mode": solver_mode,
            "function_name": _solver_function_name(solver_mode),
            "task_prefix": f"compute_{run_id}_solve_",
        },
        "chunk_items": chunk_items,
    }
    return ok_response(plan)


def handle_post_coeffgen(params):
    plan = params["plan"]
    coeffgen_results = list(params.get("coeffgen_results") or [])
    if not coeffgen_results:
        expected = int(plan.get("compute", {}).get("n_chunks") or len(plan.get("chunk_items", [])))
        coeffgen_results = _load_coeffgen_results_from_ddb(
            params["job_id"],
            params.get("task_prefix") or plan["coeffgen"]["task_prefix"],
            expected,
        )

    first = coeffgen_results[0]
    degree = _validate_positive_int(first.get("degree"), "degree", max_value=4096)
    n_coeffs = _validate_positive_int(first.get("n_coeffs"), "n_coeffs", max_value=4096)

    total_coeffs_size = 0
    for row in coeffgen_results:
        row_degree = _validate_positive_int(row.get("degree"), "degree", max_value=4096)
        row_n_coeffs = _validate_positive_int(row.get("n_coeffs"), "n_coeffs", max_value=4096)
        if row_degree != degree or row_n_coeffs != n_coeffs:
            raise RuntimeError(
                f"coeffgen result mismatch: expected degree={degree}, n_coeffs={n_coeffs}, "
                f"got degree={row_degree}, n_coeffs={row_n_coeffs}"
            )
        total_coeffs_size += int(row.get("coeffs_size", 0) or 0)

    n = int(plan["compute"]["N"])
    times = int(plan["compute"]["times"])
    lores_n = min(n, max(5, int(math.ceil(math.sqrt(TARGET_PREVIEW_ROOTS / max(1, degree * times))))))
    lores_steps = lores_n * lores_n * times
    job_id = plan["job_id"]
    run_id = plan["run_id"]

    post = {
        "degree": degree,
        "n_coeffs": n_coeffs,
        "total_coeffs_size": total_coeffs_size,
        "lores": {
            "N": lores_n,
            "n_steps": lores_steps,
            "params_key": f"renders/{job_id}/lores_params.bin",
            "coeffs_key": f"renders/{job_id}/lores_coeffs.bin",
            "bin_key": f"renders/{job_id}/lores.bin",
            "param_gen_threads": int(plan["compute"].get("lores_param_gen_threads", 1) or 1),
            "coeffgen_threads": int(plan["compute"].get("lores_coeffgen_threads", 1) or 1),
            "param_task_id": f"compute_{run_id}_lores_param_gen",
            "coeff_task_id": f"compute_{run_id}_lores_coeffgen",
            "solve_task_id": f"compute_{run_id}_lores_solve",
        },
    }
    return ok_response(post)


def _load_coeffgen_results_from_ddb(job_id, task_prefix, expected):
    ddb = _get_ddb()
    kwargs = {
        "TableName": JOBS_TABLE,
        "ConsistentRead": True,
        "KeyConditionExpression": "job_id = :jid AND begins_with(task_id, :pfx)",
        "ExpressionAttributeValues": {
            ":jid": {"S": job_id},
            ":pfx": {"S": task_prefix},
        },
        "ProjectionExpression": "task_id, task_status, error_msg, result_data",
    }
    rows = []
    while True:
        resp = ddb.query(**kwargs)
        rows.extend(resp.get("Items", []))
        if "LastEvaluatedKey" not in resp:
            break
        kwargs["ExclusiveStartKey"] = resp["LastEvaluatedKey"]

    done_results = []
    errors = []
    for item in rows:
        status = item["task_status"]["S"]
        if status == "done":
            rd = item.get("result_data", {}).get("S")
            if not rd:
                raise RuntimeError(
                    f"post_coeffgen missing result_data for {item['task_id']['S']}"
                )
            try:
                done_results.append(json.loads(rd))
            except Exception as exc:
                raise RuntimeError(
                    f"post_coeffgen invalid result_data for {item['task_id']['S']}: {exc}"
                ) from exc
        elif status == "error":
            errors.append({
                "task_id": item["task_id"]["S"],
                "error_msg": item.get("error_msg", {}).get("S", "unknown"),
            })

    if errors:
        sample = ", ".join(f"{e['task_id']}: {e['error_msg']}" for e in errors[:3])
        raise RuntimeError(
            f"post_coeffgen saw {len(errors)} coeffgen errors for prefix {task_prefix}: {sample}"
        )
    if len(done_results) != expected:
        raise RuntimeError(
            f"post_coeffgen expected {expected} coeffgen results for prefix {task_prefix}, found {len(done_results)}"
        )
    return done_results


def handle_finalize_metadata(params):
    plan = params["plan"]
    post = params["post"]
    lores_solve = params["lores_solve"]
    solve_results = list(params.get("solve_results") or [])
    if not solve_results:
        raise RuntimeError("finalize_metadata requires non-empty solve_results")

    solve_results.sort(key=lambda row: int(row.get("chunk_idx", row.get("stripe_idx", 0)) or 0))
    coeffs_keys = [item["coeffs_key"] for item in plan["chunk_items"]]
    chunk_plan_by_idx = {}
    for fallback_idx, item in enumerate(plan["chunk_items"]):
        chunk_plan_by_idx[int(item.get("chunk_idx", fallback_idx))] = item
    total_bin_size = 0
    total_roots = 0
    chunks = []
    for row in solve_results:
        bin_size = int(row.get("bin_size", 0) or 0)
        total_bin_size += bin_size
        total_roots += bin_size // 8
        chunk_idx = int(row.get("chunk_idx", row.get("stripe_idx", 0)) or 0)
        plan_item = chunk_plan_by_idx.get(chunk_idx, {})
        chunk_entry = {
            "idx": chunk_idx,
            "bin_key": row.get("s3_key"),
            "bin_size": bin_size,
            "compute_us": int(row.get("compute_us", 0) or 0),
            "n_t": int(row.get("n_t", 0) or 0),
            "avg_iterations": float(row.get("avg_iterations", 0) or 0),
        }
        if "step_start" in plan_item:
            chunk_entry["step_start"] = int(plan_item["step_start"])
        if "step_count" in plan_item:
            chunk_entry["step_count"] = int(plan_item["step_count"])
        if "params_key" in plan_item:
            chunk_entry["params_key"] = str(plan_item.get("params_key") or "")
        if "params_bin_size" in plan_item:
            chunk_entry["params_bin_size"] = int(plan_item.get("params_bin_size") or 0)
        if "params_step_start" in plan_item:
            chunk_entry["params_step_start"] = int(plan_item.get("params_step_start") or 0)
        if "params_step_count" in plan_item:
            chunk_entry["params_step_count"] = int(plan_item.get("params_step_count") or 0)
        if "skipped_overflow" in row:
            chunk_entry["skipped_overflow"] = int(row.get("skipped_overflow", 0) or 0)
        chunks.append(chunk_entry)

    calc_meta = {
        "job_id": plan["job_id"],
        "pipeline": {
            "param_transforms": plan["pipeline"]["param_transforms"],
            "param_transforms_display": plan["pipeline"]["param_transforms_display"],
            "function": plan["pipeline"]["function"],
            "coeff_transforms": plan["pipeline"]["coeff_transforms"],
            "cfpv": plan["pipeline"]["cfpv"],
        },
        "function": plan["pipeline"]["function"],
        "N": int(plan["compute"]["N"]),
        "solver": plan["solve"]["mode"],
        "n_chunks": int(plan["compute"]["n_chunks"]),
        "n_steps": int(plan["compute"]["n_steps"]),
        "param_storage_mode": str(plan["compute"].get("param_storage_mode") or "global"),
        "params_key": str(plan["compute"].get("params_key") or ""),
        "param_gen_threads": int(plan["compute"].get("param_gen_threads", 1) or 1),
        "coeffgen_threads": int(plan["compute"].get("coeffgen_threads", 1) or 1),
        "lores_param_gen_threads": int(plan["compute"].get("lores_param_gen_threads", 1) or 1),
        "lores_coeffgen_threads": int(plan["compute"].get("lores_coeffgen_threads", 1) or 1),
        "times": int(plan["compute"]["times"]),
        "degree": int(post["degree"]),
        "n_coeffs": int(post["n_coeffs"]),
        "coeffs_keys": coeffs_keys,
        "total_coeffs_size": int(post["total_coeffs_size"]),
        "lores": {
            "bin_key": lores_solve.get("s3_key") or post["lores"]["bin_key"],
            "coeffs_key": post["lores"]["coeffs_key"],
            "params_key": post["lores"].get("params_key") or f'renders/{plan["job_id"]}/lores_params.bin',
            "param_gen_threads": int(post["lores"].get("param_gen_threads", 1) or 1),
            "coeffgen_threads": int(post["lores"].get("coeffgen_threads", 1) or 1),
            "N": int(post["lores"]["N"]),
            "n_steps": int(post["lores"]["n_steps"]),
            "bin_size": int(lores_solve.get("bin_size", 0) or 0),
        },
        "chunks": chunks,
        "total_roots": total_roots,
        "total_bin_size": total_bin_size,
    }

    calc_key = f"renders/{plan['job_id']}/calc.json"
    s3.put_object(
        Bucket=BUCKET,
        Key=calc_key,
        Body=json.dumps(calc_meta),
        ContentType="application/json",
    )

    return ok_response({
        "job_id": plan["job_id"],
        "calc_key": calc_key,
        "degree": calc_meta["degree"],
        "n_chunks": calc_meta["n_chunks"],
        "solver": calc_meta["solver"],
    })


def _validate_solver_mode(value):
    solver_mode = str(value or "aberth").strip().lower()
    if solver_mode not in ("aberth", "aberth_mt", "companion_matrix"):
        raise RuntimeError(f"solver_mode must be one of aberth, aberth_mt, companion_matrix; got {value!r}")
    return solver_mode


def _validate_positive_int(value, field_name, max_value=None):
    try:
        n = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if n < 1:
        raise RuntimeError(f"{field_name} must be >= 1, got {n}")
    if max_value is not None and n > max_value:
        raise RuntimeError(f"{field_name} must be <= {max_value}, got {n}")
    return n


def _solver_function_name(solver_mode):
    if solver_mode == "companion_matrix":
        return SWEEP_CM_FUNCTION
    if solver_mode == "aberth_mt":
        return SWEEP_MT_FUNCTION
    return SWEEP_FUNCTION
