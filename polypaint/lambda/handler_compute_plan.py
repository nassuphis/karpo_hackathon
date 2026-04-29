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

from compute_fused import (
    build_chunk_items,
    build_probe_signature,
    estimate_fused_chunking,
    execution_method_from_params,
    validate_fused_threads,
)
from coeff_program_chain import compile_coeff_program_chain
from param_program_chain import compile_param_program_chain
from shared import BUCKET, JOBS_TABLE, ok_response, parse_body

s3 = boto3.client("s3")
_ddb = None

SWEEP_MT_FUNCTION = os.environ.get("SWEEP_MT_FUNCTION", "polypaint-sweep-mt")
SWEEP_CM_FUNCTION = os.environ.get("SWEEP_CM_FUNCTION", "polypaint-sweep-cm")
PARAM_PROGRAMS_PREFIX = "polypaint/param-programs/"
COEFF_PROGRAMS_PREFIX = "polypaint/coeff-programs/"

TARGET_PREVIEW_ROOTS = 250000
MAX_N = 50000
MAX_TIMES = 1000
MAX_CHUNKS = 5000
MAX_TOTAL_STEPS = 2_500_000_000
MAX_PARAM_GEN_THREADS = 64


def _compiled_coeff_program_payload(compiled):
    return {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "display": compiled["display"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "scalar_expr_count": compiled["scalar_expr_count"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
        "tokens": compiled["tokens"],
        "scalar_exprs": compiled["scalar_exprs"],
    }


def _is_missing_s3_error(exc):
    response = getattr(exc, "response", {}) or {}
    code = str((response.get("Error") or {}).get("Code") or "")
    return code in {"NoSuchKey", "NoSuchBucket", "404", "NotFound"}


def _read_saved_program_source_chain(prefix, program_kind, program_id):
    macro_id = str(program_id or "").strip()
    if not macro_id:
        raise RuntimeError(f"{program_kind} macro name is required")
    key = f"{prefix}{macro_id}.json"
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise RuntimeError(f"{program_kind} macro not found: {macro_id}") from None
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"{program_kind} macro is not valid JSON: {macro_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"{program_kind} macro must be a JSON object: {macro_id}")
    chain = payload.get("chain")
    if not isinstance(chain, list):
        raise RuntimeError(f"{program_kind} macro chain must be a JSON array: {macro_id}")
    return chain


def _param_program_macro_resolver():
    return lambda macro_id: _read_saved_program_source_chain(
        PARAM_PROGRAMS_PREFIX,
        "param program",
        macro_id,
    )


def _coeff_program_macro_resolver():
    return lambda macro_id: _read_saved_program_source_chain(
        COEFF_PROGRAMS_PREFIX,
        "coeff program",
        macro_id,
    )


def _pipeline_mode_from_params(run_params):
    raw = str(run_params.get("pipeline_mode") or run_params.get("compute_pipeline_mode") or "").strip().lower()
    if not raw:
        raw = "program" if (
            run_params.get("param_program_chain")
            or run_params.get("coeff_program_chain")
            or run_params.get("param_program")
            or run_params.get("coeff_program")
        ) else "chain"
    aliases = {"legacy": "chain", "chains": "chain", "programs": "program"}
    raw = aliases.get(raw, raw)
    if raw not in {"chain", "program"}:
        raise RuntimeError("pipeline_mode must be one of chain, program")
    return raw


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
    pipeline_mode = _pipeline_mode_from_params(run_params)

    solver_mode = _validate_solver_mode(run_params.get("solver_mode", "aberth_mt"))
    execution_method = execution_method_from_params(run_params)
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
    if pipeline_mode == "program":
        # Program mode is explicit: legacy transform chains are idle unless a
        # compiled program lowers exactly to the legacy chain representation.
        param_transforms = []
        coeff_transforms = []
    param_program_chain = run_params.get("param_program_chain") if pipeline_mode == "program" else []
    param_program = None
    param_program_metadata = None
    if param_program_chain:
        if not isinstance(param_program_chain, list):
            raise RuntimeError("param_program_chain must be an array")
        compiled_param_program = compile_param_program_chain(
            param_program_chain,
            macro_resolver=_param_program_macro_resolver(),
        )
        param_program_metadata = {
            "version": compiled_param_program["version"],
            "fingerprint": compiled_param_program["fingerprint"],
            "display": compiled_param_program["display"],
            "stack_max": compiled_param_program["stack_max"],
            "token_count": compiled_param_program["token_count"],
            "uses_legacy_fast_path": compiled_param_program["uses_legacy_fast_path"],
        }
        if compiled_param_program["legacy_transforms"]:
            param_transforms = compiled_param_program["legacy_transforms"]
        else:
            param_transforms = []
            param_program = {
                **param_program_metadata,
                "tokens": compiled_param_program["tokens"],
            }
    coeff_program_chain = run_params.get("coeff_program_chain") if pipeline_mode == "program" else []
    coeff_program = None
    coeff_program_metadata = None
    if coeff_program_chain:
        if not isinstance(coeff_program_chain, list):
            raise RuntimeError("coeff_program_chain must be an array")
        compiled_coeff_program = compile_coeff_program_chain(
            coeff_program_chain,
            macro_resolver=_coeff_program_macro_resolver(),
        )
        coeff_program_metadata = {
            "version": compiled_coeff_program["version"],
            "fingerprint": compiled_coeff_program["fingerprint"],
            "display": compiled_coeff_program["display"],
            "stack_max": compiled_coeff_program["stack_max"],
            "token_count": compiled_coeff_program["token_count"],
            "scalar_expr_count": compiled_coeff_program["scalar_expr_count"],
            "uses_legacy_chain_equivalent": compiled_coeff_program["uses_legacy_chain_equivalent"],
        }
        if compiled_coeff_program["legacy_coeff_transforms"]:
            coeff_transforms = compiled_coeff_program["legacy_coeff_transforms"]
        else:
            coeff_transforms = []
            coeff_program = _compiled_coeff_program_payload(compiled_coeff_program)

    cfpv = run_params.get("cfpv")
    if cfpv in (None, ""):
        cfpv = []
    if not isinstance(cfpv, list):
        raise RuntimeError("cfpv must be an array")

    actual_chunks = min(requested_chunks, total_steps)
    fused_threads = None
    auto_hires_chunks = bool(run_params.get("auto_hires_chunks"))
    probe = params.get("probe") or {}
    post_seed = None
    fused_estimate = None
    if execution_method == "fused_chunk_pipeline":
        fused_threads = validate_fused_threads(run_params.get("fused_threads", 4))
        param_gen_threads = int(fused_threads)
        coeffgen_threads = int(fused_threads)
        probe_degree = _validate_positive_int(probe.get("degree"), "probe.degree", max_value=4096)
        probe_n_coeffs = _validate_positive_int(probe.get("n_coeffs"), "probe.n_coeffs", max_value=4096)
        if not bool(probe.get("probe_stable")):
            raise RuntimeError("fused compute requires a stable degree probe")
        expected_signature = build_probe_signature(
            function_name=function_name,
            param_transforms=param_transforms,
            coeff_transforms=coeff_transforms,
            cfpv=cfpv,
            param_program=param_program,
            coeff_program=coeff_program,
        )
        got_signature = str(probe.get("probe_signature") or "").strip()
        if not got_signature or got_signature != expected_signature:
            raise RuntimeError("fused compute probe signature mismatch")
        fused_estimate = estimate_fused_chunking(
            n=n,
            times=times,
            requested_chunks=requested_chunks,
            degree=probe_degree,
            n_coeffs=probe_n_coeffs,
            fused_threads=fused_threads,
            solver_mode=solver_mode,
            auto_hires_chunks=auto_hires_chunks,
        )
        if not auto_hires_chunks and requested_chunks < int(fused_estimate["min_safe_chunks"]):
            raise RuntimeError(
                f"fused compute requires at least {int(fused_estimate['min_safe_chunks'])} chunks; "
                f"requested {requested_chunks}"
            )
        actual_chunks = min(int(fused_estimate["actual_chunks"]), total_steps)

    chunk_items = build_chunk_items(
        job_id=job_id,
        run_id=run_id,
        total_steps=total_steps,
        n_chunks=actual_chunks,
    )

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "compute",
        "pipeline": {
            "pipeline_mode": pipeline_mode,
            "function": function_name,
            "param_transforms": param_transforms,
            "param_transforms_display": param_transforms,
            "param_program_chain": param_program_chain or [],
            "param_program": param_program or {},
            "param_program_display": str((param_program_metadata or {}).get("display") or ""),
            "param_program_fingerprint": str((param_program_metadata or {}).get("fingerprint") or ""),
            "param_program_uses_legacy_fast_path": bool((param_program_metadata or {}).get("uses_legacy_fast_path")),
            "coeff_transforms": coeff_transforms,
            "coeff_program_chain": coeff_program_chain or [],
            "coeff_program": coeff_program or {},
            "coeff_program_display": str((coeff_program_metadata or {}).get("display") or ""),
            "coeff_program_fingerprint": str((coeff_program_metadata or {}).get("fingerprint") or ""),
            "coeff_program_uses_legacy_chain_equivalent": bool((coeff_program_metadata or {}).get("uses_legacy_chain_equivalent")),
            "cfpv": cfpv,
        },
        "compute": {
            "N": n,
            "times": times,
            "n_chunks": len(chunk_items),
            "n_steps": total_steps,
            "execution_method": execution_method,
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
    if execution_method == "fused_chunk_pipeline":
        lores_n = _compute_lores_n(n, times, int(probe_degree))
        lores_steps = lores_n * lores_n * times
        post_seed = {
            "degree": int(probe_degree),
            "n_coeffs": int(probe_n_coeffs),
            "total_coeffs_size": 0,
            "lores": {
                "N": lores_n,
                "n_steps": lores_steps,
                "params_key": f"renders/{job_id}/lores_params.bin",
                "coeffs_key": f"renders/{job_id}/lores_coeffs.bin",
                "bin_key": f"renders/{job_id}/lores.bin",
                "param_gen_threads": int(lores_param_gen_threads),
                "coeffgen_threads": int(lores_coeffgen_threads),
                "param_task_id": f"compute_{run_id}_lores_param_gen",
                "coeff_task_id": f"compute_{run_id}_lores_coeffgen",
                "solve_task_id": f"compute_{run_id}_lores_solve",
            },
            "execution_method": execution_method,
            "probe_degree": int(probe_degree),
            "probe_n_coeffs": int(probe_n_coeffs),
            "probe_signature": str(probe.get("probe_signature") or ""),
            "fused_threads": int(fused_threads),
        }
        plan["compute"].update({
            "auto_hires_chunks": bool(auto_hires_chunks),
            "fused_threads": int(fused_threads),
            "probe_degree": int(probe_degree),
            "probe_n_coeffs": int(probe_n_coeffs),
            "probe_signature": str(probe.get("probe_signature") or ""),
            "min_safe_chunks": int(fused_estimate["min_safe_chunks"]),
            "safe_chunk_limit_reason": str(fused_estimate["safe_chunk_limit_reason"]),
        })
        plan["fused"] = {
            "task_prefix": f"compute_{run_id}_fused_",
            "threads": int(fused_threads),
            "auto_hires_chunks": bool(auto_hires_chunks),
            "estimate": fused_estimate,
        }
        plan["post_seed"] = post_seed
    return ok_response(plan)


def handle_post_coeffgen(params):
    plan = params["plan"]
    coeffgen_results = list(params.get("coeffgen_results") or [])
    if not coeffgen_results:
        expected = int(plan.get("compute", {}).get("n_chunks") or len(plan.get("chunk_items", [])))
        coeffgen_results = _load_done_results_from_ddb(
            params["job_id"],
            params.get("task_prefix") or plan["coeffgen"]["task_prefix"],
            expected,
            phase_name="post_coeffgen",
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
    lores_n = _compute_lores_n(n, times, degree)
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


def _load_done_results_from_ddb(job_id, task_prefix, expected, *, phase_name):
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
                    f"{phase_name} missing result_data for {item['task_id']['S']}"
                )
            try:
                done_results.append(json.loads(rd))
            except Exception as exc:
                raise RuntimeError(
                    f"{phase_name} invalid result_data for {item['task_id']['S']}: {exc}"
                ) from exc
        elif status == "error":
            errors.append({
                "task_id": item["task_id"]["S"],
                "error_msg": item.get("error_msg", {}).get("S", "unknown"),
            })

    if errors:
        sample = ", ".join(f"{e['task_id']}: {e['error_msg']}" for e in errors[:3])
        raise RuntimeError(
            f"{phase_name} saw {len(errors)} task errors for prefix {task_prefix}: {sample}"
        )
    if len(done_results) != expected:
        raise RuntimeError(
            f"{phase_name} expected {expected} results for prefix {task_prefix}, found {len(done_results)}"
        )
    return done_results


def handle_finalize_metadata(params):
    plan = params["plan"]
    post = params["post"]
    lores_solve = params["lores_solve"]
    solve_results = list(params.get("solve_results") or [])
    if not solve_results:
        expected = int(plan.get("compute", {}).get("n_chunks") or len(plan.get("chunk_items", [])))
        solve_results = _load_done_results_from_ddb(
            params.get("job_id") or plan["job_id"],
            _finalize_results_task_prefix(plan, params),
            expected,
            phase_name="finalize_metadata",
        )

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
        if "coeffs_size" in row:
            chunk_entry["coeffs_size"] = int(row.get("coeffs_size", 0) or 0)
        if "params_size" in row:
            chunk_entry["params_size"] = int(row.get("params_size", 0) or 0)
        if "param_gen_us" in row:
            chunk_entry["param_gen_us"] = int(row.get("param_gen_us", 0) or 0)
        if "coeffgen_us" in row:
            chunk_entry["coeffgen_us"] = int(row.get("coeffgen_us", 0) or 0)
        if "solve_us" in row:
            chunk_entry["solve_us"] = int(row.get("solve_us", 0) or 0)
        if "upload_params_us" in row:
            chunk_entry["upload_params_us"] = int(row.get("upload_params_us", 0) or 0)
        if "upload_coeffs_us" in row:
            chunk_entry["upload_coeffs_us"] = int(row.get("upload_coeffs_us", 0) or 0)
        if "upload_roots_us" in row:
            chunk_entry["upload_roots_us"] = int(row.get("upload_roots_us", 0) or 0)
        if "fused_threads" in row:
            chunk_entry["fused_threads"] = int(row.get("fused_threads", 0) or 0)
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

    total_coeffs_size = int(post.get("total_coeffs_size", 0) or 0)
    if total_coeffs_size <= 0:
        total_coeffs_size = sum(int(row.get("coeffs_size", 0) or 0) for row in solve_results)

    calc_meta = {
        "job_id": plan["job_id"],
        "pipeline": {
            "pipeline_mode": plan["pipeline"].get("pipeline_mode", "chain"),
            "param_transforms": plan["pipeline"]["param_transforms"],
            "param_transforms_display": plan["pipeline"]["param_transforms_display"],
            "param_program_chain": plan["pipeline"].get("param_program_chain", []),
            "param_program": plan["pipeline"].get("param_program", {}),
            "param_program_display": plan["pipeline"].get("param_program_display", ""),
            "param_program_fingerprint": plan["pipeline"].get("param_program_fingerprint", ""),
            "param_program_uses_legacy_fast_path": bool(plan["pipeline"].get("param_program_uses_legacy_fast_path")),
            "function": plan["pipeline"]["function"],
            "coeff_transforms": plan["pipeline"]["coeff_transforms"],
            "coeff_program_chain": plan["pipeline"].get("coeff_program_chain", []),
            "coeff_program": plan["pipeline"].get("coeff_program", {}),
            "coeff_program_display": plan["pipeline"].get("coeff_program_display", ""),
            "coeff_program_fingerprint": plan["pipeline"].get("coeff_program_fingerprint", ""),
            "coeff_program_uses_legacy_chain_equivalent": bool(plan["pipeline"].get("coeff_program_uses_legacy_chain_equivalent")),
            "cfpv": plan["pipeline"]["cfpv"],
        },
        "function": plan["pipeline"]["function"],
        "N": int(plan["compute"]["N"]),
        "solver": plan["solve"]["mode"],
        "n_chunks": int(plan["compute"]["n_chunks"]),
        "n_steps": int(plan["compute"]["n_steps"]),
        "execution_method": str(plan["compute"].get("execution_method") or "classic_chunk_pipeline"),
        "param_storage_mode": str(plan["compute"].get("param_storage_mode") or "global"),
        "params_key": str(plan["compute"].get("params_key") or ""),
        "param_gen_threads": int(plan["compute"].get("param_gen_threads", 1) or 1),
        "coeffgen_threads": int(plan["compute"].get("coeffgen_threads", 1) or 1),
        "lores_param_gen_threads": int(plan["compute"].get("lores_param_gen_threads", 1) or 1),
        "lores_coeffgen_threads": int(plan["compute"].get("lores_coeffgen_threads", 1) or 1),
        "fused_threads": int(plan["compute"].get("fused_threads", 0) or 0),
        "auto_hires_chunks": bool(plan["compute"].get("auto_hires_chunks")),
        "probe_degree": int(plan["compute"].get("probe_degree", 0) or 0),
        "probe_n_coeffs": int(plan["compute"].get("probe_n_coeffs", 0) or 0),
        "probe_signature": str(plan["compute"].get("probe_signature") or ""),
        "min_safe_chunks": int(plan["compute"].get("min_safe_chunks", 0) or 0),
        "safe_chunk_limit_reason": str(plan["compute"].get("safe_chunk_limit_reason") or ""),
        "times": int(plan["compute"]["times"]),
        "degree": int(post["degree"]),
        "n_coeffs": int(post["n_coeffs"]),
        "coeffs_keys": coeffs_keys,
        "total_coeffs_size": int(total_coeffs_size),
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


def _finalize_results_task_prefix(plan, params):
    explicit = str(params.get("task_prefix") or "").strip()
    execution_method = str(plan.get("compute", {}).get("execution_method") or "").strip().lower()
    if execution_method == "fused_chunk_pipeline":
        fused_prefix = str(plan.get("fused", {}).get("task_prefix") or "").strip()
        if fused_prefix:
            return fused_prefix
    if explicit:
        return explicit
    solve_prefix = str(plan.get("solve", {}).get("task_prefix") or "").strip()
    if solve_prefix:
        return solve_prefix
    raise RuntimeError("finalize_metadata missing task_prefix")


def _validate_solver_mode(value):
    solver_mode = str(value or "aberth_mt").strip().lower()
    if solver_mode not in ("aberth_mt", "companion_matrix"):
        raise RuntimeError(f"solver_mode must be one of aberth_mt, companion_matrix; got {value!r}")
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
    return SWEEP_MT_FUNCTION


def _compute_lores_n(n, times, degree):
    return min(n, max(5, int(math.ceil(math.sqrt(TARGET_PREVIEW_ROOTS / max(1, degree * times))))))
