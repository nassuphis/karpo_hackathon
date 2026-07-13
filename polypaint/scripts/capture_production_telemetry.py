#!/usr/bin/env python3
"""Production telemetry collector (cr-33-telemetry.md §4/§10; post-mortem v3).

Reconstructs a normalized, role-scoped, schema-labeled report for one Step
Functions execution:

  1. Step Functions history — causal task reconstruction (previousEventId),
     state walls paired through event ancestry, retries counted.
  2. CloudWatch REPORT lines — joined EXACTLY by request ID, per function,
     FAIL-CLOSED: every successful task must join exactly once in live mode.
  3. Lambda GetFunctionConfiguration — immediate build-attribution snapshot.

Post-mortem requirements this version implements:
  F2  workload identity from the NESTED execution input (params), with a
      required-field schema per workflow kind; missing identity invalidates.
  F4  metrics aggregate BY ROLE (function); no cross-role mixing.
  F6  identity fields collect DISTINCT values; mixed builds/architectures
      fail validation instead of first-value-wins.
  F3  CloudWatch joins derive exact log groups from task functions, retain
      errors, validate coverage, and mark billed totals incomplete when
      failed attempts exist.
  F8  nearest-rank percentiles (ceil(q*n)-1) with min<=p50<=p95<=max.
  F10 every metric declares class, aggregation (additive|distribution|
      invariant), alias_of, and parent; sums are never emitted for
      non-additive fields; invariant disagreement is a validation problem.
  F11 reconstruction validation is fail-closed: cardinality compares run
      even when zero tasks reconstruct; statusCode, request IDs, function
      names, and bodies are required.

Outputs (raw history never enters the repository):
  reports/production/<date>-<kind>-<run-id>.json
  reports/production/<date>-<kind>-<run-id>.md
"""
import argparse
import hashlib
import json
import math
import pathlib
import re
import sys
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]


# ── F10: the metric schema ──────────────────────────────────────────────────
def _m(cls, agg, alias_of=None, parent=None):
    return {"cls": cls, "agg": agg, "alias_of": alias_of, "parent": parent}


METRIC_SCHEMA = {
    # compute fused chunk
    "param_gen_us": _m("stage_wall_us", "additive"),
    "coeffgen_us": _m("stage_wall_us", "additive"),
    "solve_us": _m("stage_wall_us", "additive"),
    "compute_us": _m("stage_wall_us", "additive", alias_of="solve_us"),
    "elapsed_us": _m("stage_wall_us", "distribution"),
    # CR34 §12-1: params/coeffs uploads run on background threads overlapping
    # the solve, so their spans are DISTRIBUTIONS — summing them against the
    # handler wall would double-count. The serial critical-path remainders
    # are the join wait and the roots tail (additive). upload_roots_us keeps
    # its original serial-PUT meaning (fallback path + pre-§12-1 histories).
    "upload_params_us": _m("stage_wall_us", "distribution"),
    "upload_coeffs_us": _m("stage_wall_us", "distribution"),
    "upload_roots_us": _m("stage_wall_us", "additive"),
    "pre_solve_upload_wait_us": _m("stage_wall_us", "additive"),
    "upload_roots_tail_us": _m("stage_wall_us", "additive"),
    "upload_roots_span_us": _m("stage_wall_us", "distribution"),
    "roots_parts_during_solve": _m("count", "additive"),
    "roots_parts_reverified": _m("count", "additive"),
    "roots_parts_repaired": _m("count", "additive"),
    "roots_upload_fallback": _m("count", "additive"),
    "params_size": _m("bytes", "additive"),
    "coeffs_size": _m("bytes", "additive"),
    "bin_size": _m("bytes", "additive"),
    "roots_size": _m("bytes", "additive", alias_of="bin_size"),
    "n_t": _m("count", "additive"),
    "reused_params": _m("count", "additive"),
    "reused_coeffs": _m("count", "additive"),
    "avg_iterations": _m("count", "distribution"),
    # fused stage_telemetry
    "param_native_us": _m("stage_wall_us", "additive"),
    "coeff_native_us": _m("stage_wall_us", "additive"),
    "solve_native_us": _m("stage_wall_us", "additive"),
    "handler_wall_us": _m("stage_wall_us", "distribution"),
    "native_elapsed_us": _m("stage_wall_us", "distribution"),
    "wall_elapsed_us": _m("stage_wall_us", "distribution"),
    "param_tokens": _m("count", "invariant"),
    "param_legacy_static": _m("count", "invariant"),
    "param_legacy_dynamic": _m("count", "invariant"),
    "param_legacy_prepared": _m("count", "invariant"),
    "online_cpus": _m("count", "invariant"),
    "coeff_tokens": _m("count", "invariant"),
    "coeff_tok_typed_scalar": _m("count", "invariant"),
    "coeff_tok_typed_vector": _m("count", "invariant"),
    "coeff_tok_selector": _m("count", "invariant"),
    "coeff_tok_native": _m("count", "invariant"),
    "coeff_fused_regions": _m("count", "invariant"),
    "coeff_fused_tokens": _m("count", "invariant"),
    "lambda_memory_mb": _m("count", "invariant"),
    # solve plan telemetry (invariant per program)
    "plan_metric_count": _m("count", "invariant"),
    "plan_dup_slots": _m("count", "invariant"),
    "plan_uses_lag": _m("count", "invariant"),
    # raster
    "prep_wall_us": _m("stage_wall_us", "distribution"),
    "subprocess_wall_us": _m("stage_wall_us", "distribution"),
    "native_wall_us": _m("stage_wall_us", "distribution", parent="subprocess_wall_us"),
    "download_wall_us": _m("stage_wall_us", "distribution", parent="subprocess_wall_us"),
    "upload_wall_us": _m("stage_wall_us", "distribution"),
    "native_worker_us": _m("worker_sum_us", "additive"),
    "download_worker_us": _m("worker_sum_us", "additive"),
    "download_us": _m("worker_sum_us", "additive", alias_of="download_worker_us"),
    "native_us": _m("worker_sum_us", "additive", alias_of="native_worker_us"),
    "raster_us": _m("worker_sum_us", "additive", alias_of="native_worker_us"),
    "upload_us": _m("stage_wall_us", "distribution", alias_of="upload_wall_us"),
    "input_bytes": _m("bytes", "additive"),
    "fragment_bytes_uploaded": _m("bytes", "additive"),
    "associated_palette_fragment_bytes_uploaded": _m("bytes", "additive"),
    "step_scores_bytes_uploaded": _m("bytes", "additive"),
    "roots_plotted": _m("count", "additive"),
    "roots_clipped": _m("count", "additive"),
    "roots_deduped": _m("count", "additive"),
    "retries": _m("count", "additive"),
    "fragment_files_uploaded": _m("count", "additive"),
    "associated_palette_fragment_files_uploaded": _m("count", "additive"),
    "step_score_channels": _m("count", "invariant"),
    "threads": _m("count", "invariant"),
    "section_count": _m("count", "invariant"),
    # clip task
    "dl_ms": _m("stage_wall_ms", "distribution"),
    "compute_ms": _m("stage_wall_ms", "distribution"),
    # finalize
    "handler_wall_ms": _m("stage_wall_ms", "distribution"),
    "presign_ms": _m("stage_wall_ms", "distribution"),
    "assemble_ms": _m("stage_wall_ms", "distribution"),
    "render_encode_ms": _m("stage_wall_ms", "distribution"),
    "render_ms": _m("stage_wall_ms", "distribution", alias_of="render_encode_ms"),
    "encode_ms": _m("stage_wall_ms", "distribution"),   # legacy pre-fix histories only
    "upload_ms": _m("stage_wall_ms", "distribution"),
    "raw_upload_ms": _m("stage_wall_ms", "distribution", parent="upload_ms"),
    "assoc_palette_total_ms": _m("stage_wall_ms", "distribution", parent="upload_ms"),
    "image_upload_ms": _m("stage_wall_ms", "distribution", parent="upload_ms"),
    "preview_upload_ms": _m("stage_wall_ms", "distribution", parent="upload_ms"),
    "meta_overlay_ms": _m("stage_wall_ms", "distribution", parent="upload_ms"),
    "step_scores_fetch_concat_ms": _m("stage_wall_ms", "distribution"),
    "step_scores_download_ms": _m("stage_wall_ms", "distribution", parent="step_scores_fetch_concat_ms"),
    "step_scores_concat_ms": _m("stage_wall_ms", "distribution", parent="step_scores_fetch_concat_ms"),
    "step_scores_upload_ms": _m("stage_wall_ms", "distribution"),
    "lut_ms": _m("stage_wall_ms", "distribution"),
    "file_size": _m("bytes", "additive"),
    "step_scores_bytes": _m("bytes", "additive"),
    "step_count": _m("count", "invariant"),
    "step_scores_count": _m("count", "invariant"),
}
IDENTITY_FIELDS = (
    "git_sha", "build_id", "param_scheduler", "execution_method", "engine",
    "input_mode", "solver_mode", "function", "arch",
)
TIME_SUFFIX_RE = re.compile(r"_(us|ms)$")

REQUIRED_IDENTITY = {
    "compute": ("job_id", "run_id", "N", "n_chunks", "function", "fused_threads"),
    "render": ("job_id", "run_id", "pix"),
}
IDENTITY_INPUT_KEYS = (
    "job_id", "run_id", "N", "times", "n_chunks", "degree", "n_coeffs",
    "function", "solver_mode", "execution_method", "fused_threads",
    "coeffgen_threads", "pix", "format", "quality", "palette",
    "raster_sections", "raster_workers", "raster_mt_threads",
    "finalize_workers", "associated_palette", "mode",
)
FINGERPRINT_KEYS = (
    "param_program_chain", "coeff_program_chain", "root_program_chain",
    "solve_score_chain", "param_transforms", "coeff_transforms",
    "param_program_source_text", "coeff_program_source_text",
    "root_program_source_text", "solve_score_program_source_text",
)


def _ts(event):
    v = event["timestamp"]
    if isinstance(v, (int, float)):
        return float(v)
    return datetime.fromisoformat(str(v).replace("Z", "+00:00")).timestamp()


def load_history(path):
    data = json.loads(pathlib.Path(path).read_text())
    return data["events"] if isinstance(data, dict) else data


def fetch_history_live(execution_arn, region, sfn_client=None):
    if sfn_client is None:
        import boto3
        sfn_client = boto3.client("stepfunctions", region_name=region)
    desc = sfn_client.describe_execution(executionArn=execution_arn)
    events = []
    kwargs = {"executionArn": execution_arn, "maxResults": 1000,
              "includeExecutionData": True}
    while True:
        resp = sfn_client.get_execution_history(**kwargs)
        for e in resp["events"]:
            e = dict(e)
            if hasattr(e["timestamp"], "timestamp"):
                e["timestamp"] = e["timestamp"].timestamp()
            events.append(e)
        token = resp.get("nextToken")
        if not token:
            break
        kwargs["nextToken"] = token
    return desc, events


def parse_state_walls(events):
    """§4.4: each StateExited walks its previousEventId ancestry to the
    StateEntered that actually opened it; FIFO is a last-resort fallback for
    histories without usable event links (name-FIFO mispairs concurrent map
    iterations — sums survive by coincidence, max/p50 do not)."""
    by_id = {e["id"]: e for e in events if "id" in e}
    entered_ids = {}
    fifo = {}
    walls = {}
    for e in events:
        d = e.get("stateEnteredEventDetails")
        if d:
            entered_ids[e.get("id")] = _ts(e)
            fifo.setdefault(d["name"], []).append(e.get("id"))
            continue
        d = e.get("stateExitedEventDetails")
        if not d:
            continue
        name = d["name"]
        enter_id = None
        cursor = e
        for _ in range(4096):
            prev_id = cursor.get("previousEventId")
            if not prev_id or prev_id not in by_id:
                break
            cursor = by_id[prev_id]
            if (cursor.get("stateEnteredEventDetails") or {}).get("name") == name:
                enter_id = cursor.get("id")
                break
        if enter_id is None and fifo.get(name):
            enter_id = fifo[name][0]
        if enter_id is None or enter_id not in entered_ids:
            continue
        if fifo.get(name) and enter_id in fifo[name]:
            fifo[name].remove(enter_id)
        t0 = entered_ids.pop(enter_id)
        walls.setdefault(name, []).append(round((_ts(e) - t0) * 1000.0, 3))
    return walls


def extract_request_id(output_obj):
    meta = output_obj.get("SdkResponseMetadata") or {}
    rid = meta.get("RequestId")
    if rid:
        return rid
    headers = (output_obj.get("SdkHttpMetadata") or {}).get("HttpHeaders") or {}
    return headers.get("x-amzn-RequestId") or headers.get("x-amzn-requestid")


def _role_from_function(function_name):
    if not function_name:
        return "unknown"
    return str(function_name).rsplit(":", 1)[-1]


def parse_task_results(events):
    """§4.4 causal reconstruction: TaskSucceeded -> TaskStarted ->
    TaskScheduled, carrying exact FunctionName, chunk/section identity, and
    the integration statusCode. Decode failures are retained and flagged."""
    by_id = {e["id"]: e for e in events if "id" in e}
    tasks = []
    for e in events:
        if e["type"] != "TaskSucceeded":
            continue
        raw = e.get("taskSucceededEventDetails", {}).get("output")
        out = None
        body = None
        status_code = None
        decode_error = None
        if raw:
            try:
                out = json.loads(raw)
            except Exception as exc:
                decode_error = f"output: {exc}"
        if isinstance(out, dict):
            payload = out.get("Payload")
            if isinstance(payload, dict):
                status_code = payload.get("statusCode")
                if payload.get("body"):
                    try:
                        body = json.loads(payload["body"])
                    except Exception as exc:
                        decode_error = f"Payload.body: {exc}"
        function_name = None
        chunk_identity = {}
        cursor = e
        for _ in range(64):
            prev_id = cursor.get("previousEventId")
            if not prev_id or prev_id not in by_id:
                break
            cursor = by_id[prev_id]
            if cursor["type"] == "TaskScheduled":
                params_raw = cursor.get("taskScheduledEventDetails", {}).get("parameters")
                if params_raw:
                    try:
                        params = json.loads(params_raw)
                        function_name = params.get("FunctionName")
                        inner = params.get("Payload")
                        if isinstance(inner, dict):
                            inner_body = inner.get("body")
                            probe = inner
                            if isinstance(inner_body, str):
                                try:
                                    probe = json.loads(inner_body)
                                except Exception:
                                    probe = inner
                            for key in ("chunk_idx", "section_idx", "group_idx", "task_id"):
                                if isinstance(probe, dict) and key in probe:
                                    chunk_identity[key] = probe[key]
                    except Exception as exc:
                        decode_error = decode_error or f"parameters: {exc}"
                break
        tasks.append({
            "timestamp": _ts(e),
            "request_id": extract_request_id(out if isinstance(out, dict) else {}),
            "body": body,
            "status_code": status_code,
            "function_name": function_name,
            "role": _role_from_function(function_name),
            "chunk_identity": chunk_identity,
            "decode_error": decode_error,
        })
    return tasks


def quantile_nearest_rank(ordered, q):
    """F8: nearest-rank quantile — ceil(q*n)-1 on a sorted sample.
    Guarantees min <= p50 <= p95 <= max."""
    n = len(ordered)
    if n == 0:
        return None
    if n == 1:
        return ordered[0]
    idx = max(0, min(n - 1, math.ceil(q * n) - 1))
    return ordered[idx]


def _stats(values, agg):
    ordered = sorted(values)
    n = len(ordered)
    out = {
        "mean": round(sum(ordered) / n, 3),
        "max": round(ordered[-1], 3),
        "min": round(ordered[0], 3),
        "p50": round(quantile_nearest_rank(ordered, 0.50), 3),
        "p95": round(quantile_nearest_rank(ordered, 0.95), 3),
        "n": n,
    }
    if agg == "additive":
        out["sum"] = round(sum(ordered), 3)
    return out


def aggregate_by_role(tasks):
    """F4/F6/F10: per-role aggregation with schema semantics. Aliases fold
    into their canonical (skipped when the canonical is present in the same
    body). Invariants require exactly one distinct value per role. Unknown
    timing-suffixed fields are flagged, never aggregated. Identity fields
    collect DISTINCT values — conflicts are validation problems."""
    roles = {}
    identity_values = {}
    unclassified = set()
    problems = []

    for t in tasks:
        body = t.get("body")
        if not isinstance(body, dict):
            continue
        role = t.get("role") or "unknown"
        bucket = roles.setdefault(role, {"fields": {}, "invariants": {}, "task_count": 0})
        bucket["task_count"] += 1
        flat = dict(body)
        for sub in ("stage_telemetry", "timings"):
            if isinstance(body.get(sub), dict):
                for k, v in body[sub].items():
                    flat.setdefault(k, v)
        for k, v in flat.items():
            if k in IDENTITY_FIELDS and isinstance(v, str) and v:
                identity_values.setdefault(k, {}).setdefault(v, set()).add(role)
                continue
            if not isinstance(v, (int, float)) or isinstance(v, bool):
                continue
            spec = METRIC_SCHEMA.get(k)
            if spec is None:
                if TIME_SUFFIX_RE.search(k):
                    unclassified.add(k)
                continue
            if spec["alias_of"] and spec["alias_of"] in flat:
                continue   # canonical present in the same body: alias folds away
            canonical = spec["alias_of"] or k
            target_spec = METRIC_SCHEMA.get(canonical, spec)
            if target_spec["agg"] == "invariant":
                bucket["invariants"].setdefault(canonical, set()).add(v)
            else:
                bucket["fields"].setdefault(canonical, []).append(float(v))

    role_reports = {}
    for role, bucket in sorted(roles.items()):
        fields = {}
        for k, vals in sorted(bucket["fields"].items()):
            spec = METRIC_SCHEMA[k]
            entry = {"class": spec["cls"], "aggregation": spec["agg"],
                     **_stats(vals, spec["agg"])}
            if spec.get("parent"):
                entry["parent"] = spec["parent"]
            fields[k] = entry
        invariants = {}
        for k, vals in sorted(bucket["invariants"].items()):
            spec = METRIC_SCHEMA[k]
            if len(vals) != 1:
                problems.append(
                    f"invariant field {k!r} disagrees within role {role}: {sorted(vals)[:6]}")
                invariants[k] = {"class": spec["cls"], "aggregation": "invariant",
                                 "values": sorted(vals)[:16], "conflict": True}
            else:
                invariants[k] = {"class": spec["cls"], "aggregation": "invariant",
                                 "value": next(iter(vals))}
        role_reports[role] = {
            "task_count": bucket["task_count"],
            "fields": fields,
            "invariants": invariants,
        }

    identity = {}
    for k, values in identity_values.items():
        if len(values) == 1:
            identity[k] = next(iter(values))
        else:
            problems.append(
                f"identity field {k!r} has {len(values)} distinct values: "
                + ", ".join(f"{v} (roles: {sorted(r)})"
                            for v, r in sorted(values.items())[:4]))
            identity[k] = {"conflict": sorted(values.keys())}

    return {
        "roles": role_reports,
        "identity": identity,
        "unclassified_time_fields": sorted(unclassified),
        "problems": problems,
    }


def count_retries(events):
    failures = {}
    scheduled = succeeded = 0
    for e in events:
        t = e["type"]
        if t in ("TaskFailed", "TaskTimedOut", "TaskStartFailed"):
            for det_key in ("taskFailedEventDetails", "taskTimedOutEventDetails",
                            "taskStartFailedEventDetails"):
                det = e.get(det_key)
                if det:
                    name = det.get("resourceType", "") + ":" + det.get("resource", "")
                    failures[name] = failures.get(name, 0) + 1
                    break
            else:
                failures["unknown"] = failures.get("unknown", 0) + 1
        elif t == "TaskScheduled":
            scheduled += 1
        elif t == "TaskSucceeded":
            succeeded += 1
    return {
        "task_failure_events": sum(failures.values()),
        "failures_by_resource": failures,
        "tasks_scheduled": scheduled,
        "tasks_succeeded": succeeded,
        "attempts_in_excess": max(0, scheduled - succeeded),
    }


def extract_workload_identity(events, tasks, kind):
    """F2: identity from the OUTER input AND its nested params, plus program
    fingerprints (sha256 of chain JSON when no compiled fingerprint exists).
    Missing REQUIRED fields are validation problems."""
    identity = {}
    problems = []
    start = next((e for e in events if e["type"] == "ExecutionStarted"), None)
    exec_input = {}
    if start:
        try:
            exec_input = json.loads(
                start.get("executionStartedEventDetails", {}).get("input") or "{}")
        except Exception as exc:
            problems.append(f"execution input undecodable: {exc}")
    nested = exec_input.get("params") if isinstance(exec_input.get("params"), dict) else {}
    for source in (exec_input, nested):
        for key in IDENTITY_INPUT_KEYS:
            if key in source and key not in identity:
                value = source[key]
                if isinstance(value, (str, int, float, bool)):
                    identity[key] = value
                elif isinstance(value, dict) and key == "associated_palette":
                    identity[key] = str(value.get("mode") or "")
    for source in (exec_input, nested):
        for key in FINGERPRINT_KEYS:
            if key in source and source[key]:
                blob = json.dumps(source[key], sort_keys=True, separators=(",", ":"))
                identity[f"{key}_sha256"] = hashlib.sha256(blob.encode()).hexdigest()[:16]
    required = REQUIRED_IDENTITY.get(kind, ())
    missing = [k for k in required if k not in identity]
    if missing:
        problems.append(f"required workload identity missing for kind={kind}: {missing}")
    return identity, problems


def join_cloudwatch(tasks, region, window, logs_client=None):
    """F3 fail-closed: log groups derive EXACTLY from task function names;
    API errors are retained; the caller validates complete coverage."""
    if logs_client is None:
        import boto3
        logs_client = boto3.client("logs", region_name=region)
    expected = {t["request_id"]: t["role"] for t in tasks if t.get("request_id")}
    groups = sorted({f"/aws/lambda/{t['role']}" for t in tasks
                     if t.get("role") and t["role"] != "unknown"})
    t0, t1 = window
    joined = {}
    errors = []
    for group in groups:
        kwargs = {
            "logGroupName": group,
            "startTime": int((t0 - 60) * 1000),
            "endTime": int((t1 + 120) * 1000),
            "filterPattern": '"REPORT RequestId"',
        }
        try:
            while True:
                resp = logs_client.filter_log_events(**kwargs)
                for ev in resp.get("events", []):
                    msg = ev["message"]
                    m = re.search(r"REPORT RequestId: (\S+)", msg)
                    if not m:
                        continue
                    rid = m.group(1)
                    if rid not in expected:
                        continue
                    if rid in joined:
                        errors.append(f"duplicate REPORT join for request {rid} in {group}")
                        continue
                    rec = {"log_group": group, "role": expected[rid]}
                    for key, pat in (
                        ("duration_ms", r"\bDuration: ([\d.]+) ms"),
                        ("billed_ms", r"Billed Duration: ([\d.]+) ms"),
                        ("memory_mb", r"Memory Size: (\d+) MB"),
                        ("max_memory_mb", r"Max Memory Used: (\d+) MB"),
                        ("init_ms", r"Init Duration: ([\d.]+) ms"),
                    ):
                        mm = re.search(pat, msg)
                        if mm:
                            rec[key] = float(mm.group(1))
                    expected_group = f"/aws/lambda/{expected[rid]}"
                    if group != expected_group:
                        errors.append(
                            f"request {rid} joined in {group} but its task ran {expected_group}")
                    joined[rid] = rec
                token = resp.get("nextToken")
                if not token:
                    break
                kwargs["nextToken"] = token
        except Exception as exc:
            errors.append(f"log query failed for {group}: {exc}")
    missing = sorted(set(expected) - set(joined))
    if missing:
        errors.append(
            f"{len(missing)} task request IDs did not join a REPORT line: {missing[:5]}")
    return joined, errors


def capture_function_configs(tasks, region, lambda_client=None):
    if lambda_client is None:
        import boto3
        lambda_client = boto3.client("lambda", region_name=region)
    configs = {}
    errors = []
    for fn in sorted({t["function_name"] for t in tasks if t.get("function_name")}):
        try:
            cfg = lambda_client.get_function_configuration(FunctionName=fn)
            configs[fn] = {
                "code_sha256": cfg.get("CodeSha256"),
                "last_modified": cfg.get("LastModified"),
                "memory_mb": cfg.get("MemorySize"),
                "architectures": cfg.get("Architectures"),
                "ephemeral_mb": (cfg.get("EphemeralStorage") or {}).get("Size"),
                "layers": [l.get("Arn") for l in cfg.get("Layers") or []],
                "timeout_s": cfg.get("Timeout"),
            }
        except Exception as exc:
            errors.append(f"get_function_configuration failed for {fn}: {exc}")
    return configs, errors


def handler_report_by_role(cw_joined, retries):
    by_role = {}
    for rec in cw_joined.values():
        by_role.setdefault(rec.get("role", "unknown"), []).append(rec)
    out = {}
    for role, recs in sorted(by_role.items()):
        durations = sorted(r["duration_ms"] for r in recs if "duration_ms" in r)
        mem = [r.get("max_memory_mb") for r in recs if r.get("max_memory_mb")]
        gbs = sum((r.get("billed_ms", 0) / 1000.0) * (r.get("memory_mb", 0) / 1024.0)
                  for r in recs)
        out[role] = {
            "joined_invocations": len(recs),
            "cold_start_count": sum(1 for r in recs if "init_ms" in r),
            "task_handler_ms": {
                "max": round(durations[-1], 1) if durations else None,
                "p50": round(quantile_nearest_rank(durations, 0.5), 1) if durations else None,
                "p95": round(quantile_nearest_rank(durations, 0.95), 1) if durations else None,
            },
            "billed_gb_seconds": round(gbs, 3),
            "max_memory_mb_range": [min(mem), max(mem)] if mem else None,
        }
    if retries.get("task_failure_events"):
        for role in out.values():
            role["billed_totals_incomplete"] = (
                "failed/retried attempts incurred billed duration not "
                "reconstructed from successful-task request IDs")
    return out


def validate_reconstruction(events, tasks, identity, kind, *, live, cw_errors,
                            cw_joined, fn_config_errors):
    """F11 fail-closed validation."""
    problems = []
    decode_failures = [t for t in tasks if t.get("decode_error")]
    if decode_failures:
        problems.append(
            f"{len(decode_failures)} task result(s) failed to decode: "
            + "; ".join(sorted({t["decode_error"] for t in decode_failures}))[:400])
    for label, predicate in (
        ("request_id", lambda t: not t.get("request_id")),
        ("function_name", lambda t: not t.get("function_name")),
        ("body", lambda t: t.get("body") is None),
    ):
        bad = sum(1 for t in tasks if predicate(t))
        if bad:
            problems.append(f"{bad} successful task(s) missing {label}")
    bad_status = [t for t in tasks
                  if t.get("status_code") is not None and int(t["status_code"]) != 200]
    if bad_status:
        problems.append(f"{len(bad_status)} task(s) returned statusCode != 200")
    # Build-identity coverage (CR34 review F2 follow-up): "single build" may
    # only be claimed when EVERY decoded task result carries build_id+git_sha.
    # Conflict rejection alone fails open on absence — a role that never
    # reports identity can silently run a different build. Reported per role
    # so the gap names its owner. Histories captured before the ok_response
    # injection deploy will fail this check — that is the correct
    # classification (identity coverage is a deploy-gated property).
    identity_gaps = {}
    for t in tasks:
        body = t.get("body")
        if not isinstance(body, dict):
            continue
        if not body.get("build_id") or not body.get("git_sha"):
            role = t.get("role") or "unknown"
            identity_gaps[role] = identity_gaps.get(role, 0) + 1
    for role in sorted(identity_gaps):
        problems.append(
            f"build identity missing on {identity_gaps[role]} task result(s) "
            f"for role {role}")
    rids = [t["request_id"] for t in tasks if t.get("request_id")]
    if len(rids) != len(set(rids)):
        problems.append("duplicate Lambda request IDs across task results")
    opened = {}
    for e in events:
        d = e.get("stateEnteredEventDetails")
        if d:
            opened[d["name"]] = opened.get(d["name"], 0) + 1
        d = e.get("stateExitedEventDetails")
        if d:
            opened[d["name"]] = opened.get(d["name"], 0) - 1
    unbalanced = {k: v for k, v in opened.items() if v != 0}
    if unbalanced:
        problems.append(f"unbalanced state enter/exit: {unbalanced}")
    # cardinality: runs even when reconstruction produced zero tasks (F11).
    # Scoped to the OWNING role — chunk_idx also rides in lores/probe task
    # inputs, and counting those was itself a cross-role contamination bug
    # (caught by this check's first run against the real history).
    if kind == "compute" and identity.get("n_chunks"):
        expected = int(identity["n_chunks"])
        chunk_tasks = [t for t in tasks
                       if "chunk_idx" in (t.get("chunk_identity") or {})
                       and "fused-chunk" in (t.get("role") or "")]
        if len(chunk_tasks) != expected:
            problems.append(
                f"expected {expected} fused chunk tasks, reconstructed {len(chunk_tasks)}")
    if kind == "render" and identity.get("raster_sections"):
        expected = int(identity["raster_sections"])
        section_tasks = [t for t in tasks
                         if "section_idx" in (t.get("chunk_identity") or {})
                         and "raster" in (t.get("role") or "")]
        if len(section_tasks) != expected:
            problems.append(
                f"expected {expected} raster sections, reconstructed {len(section_tasks)}")
    if live:
        problems.extend(cw_errors)
        problems.extend(fn_config_errors)
        expected_rids = {t["request_id"] for t in tasks if t.get("request_id")}
        if expected_rids and set(cw_joined) != expected_rids:
            missing = sorted(expected_rids - set(cw_joined))
            if missing:
                problems.append(
                    f"live CloudWatch join incomplete: {len(missing)} of "
                    f"{len(expected_rids)} request IDs unjoined")
    return problems


def build_report(events, *, kind, execution_arn="", cw_joined=None,
                 cw_errors=None, fn_configs=None, fn_config_errors=None,
                 source="offline"):
    start = next(e for e in events if e["type"] == "ExecutionStarted")
    terminal = [e for e in events if e["type"].startswith("Execution") and
                ("Succeeded" in e["type"] or "Failed" in e["type"] or
                 "Aborted" in e["type"] or "TimedOut" in e["type"])]
    workflow_wall_ms = round((_ts(terminal[0]) - _ts(start)) * 1000.0, 3) if terminal else None

    tasks = parse_task_results(events)
    aggregate = aggregate_by_role(tasks)
    retries = count_retries(events)
    walls = parse_state_walls(events)
    state_walls = {
        name: {
            "n": len(vals),
            "sum_ms": round(sum(vals), 3),
            "max_ms": round(max(vals), 3),
            "p50_ms": round(quantile_nearest_rank(sorted(vals), 0.5), 3),
        }
        for name, vals in sorted(walls.items())
    }
    workload_identity, identity_problems = extract_workload_identity(events, tasks, kind)
    for k, v in aggregate.pop("identity").items():
        workload_identity.setdefault(k, v)

    validation = list(identity_problems)
    validation.extend(aggregate.pop("problems"))
    validation.extend(validate_reconstruction(
        events, tasks, workload_identity, kind,
        live=(source == "live"), cw_errors=cw_errors or [],
        cw_joined=cw_joined or {}, fn_config_errors=fn_config_errors or []))

    per_function = {}
    for t in tasks:
        if t.get("role"):
            per_function[t["role"]] = per_function.get(t["role"], 0) + 1

    return {
        "schema": "pp-production-telemetry-v3",
        "captured_at": datetime.now(timezone.utc).isoformat(),
        "source": source,
        "kind": kind,
        "execution_arn": execution_arn,
        "workflow_wall_ms": workflow_wall_ms,
        "state_walls_ms": state_walls,
        "task_aggregate_by_role": aggregate["roles"],
        "unclassified_time_fields": aggregate["unclassified_time_fields"],
        "tasks_by_function": per_function,
        "retries": retries,
        "validation_problems": validation,
        "handler_report_by_role": (handler_report_by_role(cw_joined, retries)
                                   if cw_joined else
                                   {"note": "no CloudWatch join (offline mode)"
                                    if source == "offline" else
                                    "LIVE RUN WITHOUT JOINS — see validation_problems"}),
        "function_configs": fn_configs or {},
        "workload_identity": workload_identity,
    }


def render_markdown(report):
    lines = [f"# Production telemetry — {report['kind']} ({report['source']})", ""]
    lines.append(f"Captured {report['captured_at']}; execution `{report['execution_arn'] or 'n/a'}`.")
    lines.append(f"\n**Workflow wall: {report['workflow_wall_ms']} ms**\n")
    if report.get("validation_problems"):
        lines.append("## VALIDATION PROBLEMS — report is NOT comparison-grade\n")
        for pr in report["validation_problems"]:
            lines.append(f"- {pr}")
        lines.append("")
    ident = report.get("workload_identity") or {}
    if ident:
        lines.append("## Workload identity\n")
        for k, v in sorted(ident.items()):
            lines.append(f"- {k}: `{v}`")
        lines.append("")
    lines.append("## State walls (ms)\n")
    lines.append("| State | n | sum | max | p50 |")
    lines.append("|---|---:|---:|---:|---:|")
    for name, s in report["state_walls_ms"].items():
        lines.append(f"| {name} | {s['n']} | {s['sum_ms']} | {s['max_ms']} | {s['p50_ms']} |")
    for role, agg in report["task_aggregate_by_role"].items():
        lines.append(f"\n## Role: `{role}` ({agg['task_count']} tasks)\n")
        if agg["fields"]:
            lines.append("| Field | class | agg | sum | mean | max | p50 | p95 |")
            lines.append("|---|---|---|---:|---:|---:|---:|---:|")
            for k, s in agg["fields"].items():
                lines.append(f"| {k} | {s['class']} | {s['aggregation']} | "
                             f"{s.get('sum', '-')} | {s['mean']} | {s['max']} | {s['p50']} | {s['p95']} |")
        if agg["invariants"]:
            lines.append("\n| Invariant | value |")
            lines.append("|---|---|")
            for k, s in agg["invariants"].items():
                val = s.get("value", f"CONFLICT: {s.get('values')}")
                lines.append(f"| {k} | {val} |")
    if report["unclassified_time_fields"]:
        lines.append("\n**FLAGGED — timing fields with unknown class (not aggregated):** "
                     + ", ".join(f"`{k}`" for k in report["unclassified_time_fields"]))
    retr = report.get("retries") or {}
    lines.append("\n## Retries\n")
    lines.append(f"- task failure events: {retr.get('task_failure_events', 0)}")
    lines.append(f"- scheduled/succeeded: {retr.get('tasks_scheduled', 0)}/{retr.get('tasks_succeeded', 0)}"
                 f" (attempts in excess: {retr.get('attempts_in_excess', 0)})")
    hr = report.get("handler_report_by_role") or {}
    lines.append("\n## Lambda handler report (by role)\n")
    if isinstance(hr, dict) and "note" in hr:
        lines.append(f"- {hr['note']}")
    else:
        for role, rec in hr.items():
            lines.append(f"- `{role}`: {json.dumps(rec)}")
    if report.get("function_configs"):
        lines.append("\n## Function configs (build attribution snapshot)\n")
        for arn, cfg in report["function_configs"].items():
            lines.append(f"- `{arn}`: {json.dumps(cfg)}")
    lines.append("")
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--execution-arn", help="live capture via AWS APIs")
    src.add_argument("--history-file", help="offline: saved get-execution-history JSON")
    ap.add_argument("--kind", choices=("compute", "render", "auto"), default="auto")
    ap.add_argument("--region", default="us-east-1")
    ap.add_argument("--run-id", default="", help="labels the output files")
    ap.add_argument("--out-dir", default=str(ROOT / "reports" / "production"))
    ap.add_argument("--no-cloudwatch", action="store_true",
                    help="live mode without the CloudWatch/Lambda joins "
                         "(the report will carry a validation problem)")
    args = ap.parse_args()

    cw = fn_cfg = None
    cw_errors = fn_cfg_errors = None
    if args.history_file:
        events = load_history(args.history_file)
        execution_arn = ""
        source = "offline"
    else:
        desc, events = fetch_history_live(args.execution_arn, args.region)
        execution_arn = args.execution_arn
        source = "live"
        tasks = parse_task_results(events)
        if not args.no_cloudwatch:
            window = (_ts(events[0]), _ts(events[-1]))
            cw, cw_errors = join_cloudwatch(tasks, args.region, window)
            fn_cfg, fn_cfg_errors = capture_function_configs(tasks, args.region)
        else:
            cw_errors = ["live run executed with --no-cloudwatch: attribution incomplete"]

    kind = args.kind
    if kind == "auto":
        names = {e.get("stateEnteredEventDetails", {}).get("name", "") for e in events}
        kind = "compute" if any("FusedChunk" in n or "Compute" in n for n in names) else "render"

    report = build_report(events, kind=kind, execution_arn=execution_arn,
                          cw_joined=cw, cw_errors=cw_errors, fn_configs=fn_cfg,
                          fn_config_errors=fn_cfg_errors, source=source)

    run_id = args.run_id or str(report["workload_identity"].get("run_id") or "run")
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d")
    out_dir = pathlib.Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    base = out_dir / f"{stamp}-{kind}-{run_id}"
    base.with_suffix(".json").write_text(json.dumps(report, indent=2, sort_keys=True))
    base.with_suffix(".md").write_text(render_markdown(report))
    print(f"wrote {base}.json")
    print(f"wrote {base}.md")
    problems = list(report.get("validation_problems") or [])
    if report["unclassified_time_fields"]:
        problems.append("unclassified timing fields: "
                        + ", ".join(report["unclassified_time_fields"]))
    if problems:
        for pr in problems:
            print(f"WARNING: {pr}")
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
