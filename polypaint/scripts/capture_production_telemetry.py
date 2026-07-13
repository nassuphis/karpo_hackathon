#!/usr/bin/env python3
"""Production telemetry collector (cr-33-telemetry.md §9).

Reconstructs a normalized, timing-class-labeled report for one Step Functions
execution, joining three sources:

  1. Step Functions execution history  — state walls + task result payloads
  2. CloudWatch Lambda REPORT lines    — handler/billed/init/memory, joined
                                         EXACTLY by Lambda request ID
  3. Lambda GetFunctionConfiguration   — CodeSha256/LastModified/arch (the
                                         build-attribution snapshot)

Modes:
  live     --execution-arn arn:...      (describe + paginate + CW + Lambda)
  offline  --history-file history.json  (no AWS calls; joins marked absent)

Every metric is labeled with a timing class from §2 (wall, stage_wall,
worker_sum, bytes, count). A payload field whose class is unknown is NOT
silently summed — it lands in `unclassified` and is flagged in the report.

Outputs (raw history is never copied into the repository):
  reports/production/<date>-<kind>-<run-id>.json
  reports/production/<date>-<kind>-<run-id>.md
"""
import argparse
import json
import pathlib
import re
import statistics
import sys
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]

# ── §2 timing classes: the single registry of known payload fields ─────────
# class ∈ {stage_wall_us, stage_wall_ms, worker_sum_us, bytes, count, ident}
FIELD_CLASSES = {
    # compute fused chunk results
    "param_gen_us": "stage_wall_us",
    "elapsed_us": "stage_wall_us",     # generic native meta: elapsed span
    "compute_us": "stage_wall_us",     # solve stage span in solver results
    "coeffgen_us": "stage_wall_us",
    "solve_us": "stage_wall_us",
    "upload_params_us": "stage_wall_us",
    "upload_coeffs_us": "stage_wall_us",
    "upload_roots_us": "stage_wall_us",
    "params_size": "bytes",
    "coeffs_size": "bytes",
    "bin_size": "bytes",
    "n_t": "count",
    "reused_params": "count",
    "reused_coeffs": "count",
    "avg_iterations": "count",
    # fused stage_telemetry sub-fields
    "param_native_us": "stage_wall_us",
    "coeff_native_us": "stage_wall_us",
    "native_elapsed_us": "stage_wall_us",
    "wall_elapsed_us": "stage_wall_us",
    "param_tokens": "count",
    "param_legacy_static": "count",
    "param_legacy_dynamic": "count",
    "param_legacy_prepared": "count",
    "online_cpus": "count",
    "coeff_tokens": "count",
    "coeff_tok_typed_scalar": "count",
    "coeff_tok_typed_vector": "count",
    "coeff_tok_selector": "count",
    "coeff_tok_native": "count",
    "coeff_fused_regions": "count",
    "coeff_fused_tokens": "count",
    "roots_size": "bytes",
    "lambda_memory_mb": "count",
    # raster results (CR33 telemetry naming)
    "handler_wall_us": "stage_wall_us",
    "prep_wall_us": "stage_wall_us",
    "native_wall_us": "stage_wall_us",
    "download_wall_us": "stage_wall_us",
    "upload_wall_us": "stage_wall_us",
    "native_worker_us": "worker_sum_us",
    "download_worker_us": "worker_sum_us",
    # legacy raster names (worker sums, retained one cycle)
    "download_us": "worker_sum_us",
    "native_us": "worker_sum_us",
    "raster_us": "worker_sum_us",
    "upload_us": "stage_wall_us",
    "fragment_bytes_uploaded": "bytes",
    "associated_palette_fragment_bytes_uploaded": "bytes",
    "step_scores_bytes_uploaded": "bytes",
    "roots_plotted": "count",
    "roots_clipped": "count",
    "retries": "count",
    "fragment_files_uploaded": "count",
    "associated_palette_fragment_files_uploaded": "count",
    "step_score_channels": "count",
    # finalize results
    "assemble_ms": "stage_wall_ms",
    "dl_ms": "stage_wall_ms",          # clip task download span
    "compute_ms": "stage_wall_ms",     # clip task compute span
    # legacy finalize field: histories predating CR33-telemetry carry the
    # hardcoded 0 — class is known, the VALUE from old runs is untrustworthy
    "encode_ms": "stage_wall_ms",
    "render_ms": "stage_wall_ms",
    "render_encode_ms": "stage_wall_ms",
    "upload_ms": "stage_wall_ms",
    "raw_upload_ms": "stage_wall_ms",
    "assoc_palette_total_ms": "stage_wall_ms",
    "image_upload_ms": "stage_wall_ms",
    "preview_upload_ms": "stage_wall_ms",
    "meta_overlay_ms": "stage_wall_ms",
    "step_scores_fetch_concat_ms": "stage_wall_ms",
    "step_scores_upload_ms": "stage_wall_ms",
    "lut_ms": "stage_wall_ms",
    "file_size": "bytes",
    "step_scores_bytes": "bytes",
    "step_count": "count",
    "step_scores_count": "count",
}
IDENTITY_FIELDS = (
    "git_sha", "build_id", "param_scheduler", "execution_method", "engine",
    "input_mode", "solver_mode", "function", "arch",
)
TIME_SUFFIX_RE = re.compile(r"_(us|ms)$")


def _ts(event):
    v = event["timestamp"]
    if isinstance(v, (int, float)):
        return float(v)
    return datetime.fromisoformat(str(v).replace("Z", "+00:00")).timestamp()


def load_history(path):
    data = json.loads(pathlib.Path(path).read_text())
    return data["events"] if isinstance(data, dict) else data


def fetch_history_live(execution_arn, region):
    import boto3
    sfn = boto3.client("stepfunctions", region_name=region)
    desc = sfn.describe_execution(executionArn=execution_arn)
    events = []
    kwargs = {"executionArn": execution_arn, "maxResults": 1000}
    while True:
        resp = sfn.get_execution_history(**kwargs)
        for e in resp["events"]:
            e = dict(e)
            e["timestamp"] = e["timestamp"].timestamp()
            events.append(e)
        token = resp.get("nextToken")
        if not token:
            break
        kwargs["nextToken"] = token
    return desc, events


def parse_state_walls(events):
    """§4.4: name-only FIFO pairing is unsafe when map iterations run the
    same state concurrently — an exit can pair with another iteration's
    enter, skewing max/p50 (sums survive by coincidence). Each StateExited
    walks its previousEventId ancestry to the StateEntered event that
    actually opened it; FIFO remains only as a last-resort fallback for
    histories without usable event links."""
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


def parse_task_results(events):
    """§4.4: reconstruct each successful task through its previousEventId
    causal chain (TaskSucceeded -> ... -> TaskScheduled), so the exact
    FunctionName and chunk/section identity travel with the metrics instead
    of being associated by array order or state name. Decode failures are
    RETAINED (flagged), never silently dropped (§4.10)."""
    by_id = {e["id"]: e for e in events if "id" in e}
    tasks = []
    for e in events:
        if e["type"] != "TaskSucceeded":
            continue
        raw = e.get("taskSucceededEventDetails", {}).get("output")
        out = None
        body = None
        decode_error = None
        if raw:
            try:
                out = json.loads(raw)
            except Exception as exc:
                decode_error = f"output: {exc}"
        if isinstance(out, dict):
            payload = out.get("Payload")
            if isinstance(payload, dict) and payload.get("body"):
                try:
                    body = json.loads(payload["body"])
                except Exception as exc:
                    decode_error = f"Payload.body: {exc}"
        # walk the causal chain to the scheduling event
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
            "function_name": function_name,
            "chunk_identity": chunk_identity,
            "decode_error": decode_error,
        })
    return tasks


def classify_and_aggregate(tasks):
    """Aggregate numeric task-body fields BY TIMING CLASS (§2). Unknown
    timing-suffixed fields are flagged, never silently summed."""
    per_field = {}
    identity = {}
    unclassified = set()
    task_count = 0
    for t in tasks:
        body = t.get("body")
        if not isinstance(body, dict):
            continue
        task_count += 1
        flat = dict(body)
        for sub in ("stage_telemetry", "timings"):
            if isinstance(body.get(sub), dict):
                for k, v in body[sub].items():
                    flat.setdefault(k, v)
        for k, v in flat.items():
            if k in IDENTITY_FIELDS and isinstance(v, str) and v:
                identity.setdefault(k, v)
                continue
            if not isinstance(v, (int, float)) or isinstance(v, bool):
                continue
            cls = FIELD_CLASSES.get(k)
            if cls is None:
                if TIME_SUFFIX_RE.search(k):
                    unclassified.add(k)
                continue
            per_field.setdefault(k, []).append(float(v))

    def stats(values):
        ordered = sorted(values)
        n = len(ordered)
        return {
            "sum": round(sum(ordered), 3),
            "mean": round(sum(ordered) / n, 3),
            "max": round(ordered[-1], 3),
            "p50": round(statistics.median(ordered), 3),
            "p95": round(ordered[min(n - 1, int(0.95 * (n - 1)))], 3),
            "n": n,
        }

    fields = {
        k: {"class": FIELD_CLASSES[k], **stats(v)}
        for k, v in sorted(per_field.items())
    }
    return {
        "task_count": task_count,
        "fields": fields,
        "identity": identity,
        "unclassified_time_fields": sorted(unclassified),
    }


def count_retries(events):
    """§4.5: a general collector must report retry counts and must not count
    multiple attempts as distinct successes. Failure-class task events are
    counted per state name; TaskScheduled in excess of TaskSucceeded for the
    same state is reported as attempts_in_excess."""
    failures = {}
    scheduled = {}
    succeeded = {}
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
            scheduled["all"] = scheduled.get("all", 0) + 1
        elif t == "TaskSucceeded":
            succeeded["all"] = succeeded.get("all", 0) + 1
    return {
        "task_failure_events": sum(failures.values()),
        "failures_by_resource": failures,
        "tasks_scheduled": scheduled.get("all", 0),
        "tasks_succeeded": succeeded.get("all", 0),
        "attempts_in_excess": max(0, scheduled.get("all", 0) - succeeded.get("all", 0)),
    }


def validate_reconstruction(events, tasks, state_walls, identity):
    """§4.10 invariants. Violations are REPORTED, and the CLI exits nonzero
    on them just as it does for unclassified timing fields."""
    problems = []
    decode_failures = [t for t in tasks if t.get("decode_error")]
    if decode_failures:
        problems.append(
            f"{len(decode_failures)} task result(s) failed to decode: "
            + "; ".join(sorted({t["decode_error"] for t in decode_failures}))[:400])
    rids = [t["request_id"] for t in tasks if t.get("request_id")]
    if len(rids) != len(set(rids)):
        problems.append("duplicate Lambda request IDs across task results")
    # enter/exit balance per state
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
    # expected map iterations vs observed (when the input declares them)
    expected = identity.get("n_chunks")
    if expected:
        chunked = [t for t in tasks
                   if isinstance(t.get("body"), dict) and "chunk_idx" in
                   (t.get("chunk_identity") or {})]
        if chunked and len(chunked) != int(expected):
            problems.append(
                f"expected {expected} chunk tasks, reconstructed {len(chunked)}")
    return problems


def join_cloudwatch(tasks, region, window):
    """Live only: join REPORT lines by request ID across the execution's
    Lambda log groups (discovered from the report lines themselves is not
    possible — the caller supplies function names or we scan by request ID
    via filter pattern across groups named in task resources)."""
    import boto3
    logs = boto3.client("logs", region_name=region)
    ids = {t["request_id"] for t in tasks if t.get("request_id")}
    if not ids:
        return {}
    groups = []
    paginator = logs.get_paginator("describe_log_groups")
    for page in paginator.paginate(logGroupNamePrefix="/aws/lambda/polypaint"):
        groups.extend(g["logGroupName"] for g in page.get("logGroups", []))
    t0, t1 = window
    joined = {}
    pattern = "?" + " ?".join(f'"{rid}"' for rid in list(ids)[:20]) if len(ids) <= 20 else "REPORT"
    for group in groups:
        try:
            kwargs = {
                "logGroupName": group,
                "startTime": int((t0 - 60) * 1000),
                "endTime": int((t1 + 120) * 1000),
                "filterPattern": '"REPORT RequestId"',
            }
            while True:
                resp = logs.filter_log_events(**kwargs)
                for ev in resp.get("events", []):
                    msg = ev["message"]
                    m = re.search(r"REPORT RequestId: (\S+)", msg)
                    if not m or m.group(1) not in ids:
                        continue
                    rec = {"log_group": group}
                    for key, pat, scale in (
                        ("duration_ms", r"\bDuration: ([\d.]+) ms", 1.0),
                        ("billed_ms", r"Billed Duration: ([\d.]+) ms", 1.0),
                        ("memory_mb", r"Memory Size: (\d+) MB", 1.0),
                        ("max_memory_mb", r"Max Memory Used: (\d+) MB", 1.0),
                        ("init_ms", r"Init Duration: ([\d.]+) ms", 1.0),
                    ):
                        mm = re.search(pat, msg)
                        if mm:
                            rec[key] = float(mm.group(1)) * scale
                    joined[m.group(1)] = rec
                token = resp.get("nextToken")
                if not token:
                    break
                kwargs["nextToken"] = token
        except Exception:
            continue
    _ = pattern
    return joined


def capture_function_configs(events, region):
    """Live only: snapshot CodeSha256/LastModified per invoked function ARN,
    taken IMMEDIATELY so $LATEST drift is bounded (§7 option 3)."""
    import boto3
    lam = boto3.client("lambda", region_name=region)
    arns = set()
    for e in events:
        params = e.get("taskScheduledEventDetails", {}).get("parameters")
        if not params:
            continue
        try:
            fn = json.loads(params).get("FunctionName")
            if fn:
                arns.add(fn)
        except Exception:
            continue
    configs = {}
    for arn in sorted(arns):
        try:
            cfg = lam.get_function_configuration(FunctionName=arn)
            configs[arn] = {
                "code_sha256": cfg.get("CodeSha256"),
                "last_modified": cfg.get("LastModified"),
                "memory_mb": cfg.get("MemorySize"),
                "architectures": cfg.get("Architectures"),
                "ephemeral_mb": (cfg.get("EphemeralStorage") or {}).get("Size"),
                "layers": [l.get("Arn") for l in cfg.get("Layers") or []],
                "timeout_s": cfg.get("Timeout"),
            }
        except Exception as exc:
            configs[arn] = {"error": str(exc)}
    return configs


def build_report(events, *, kind, execution_arn="", cw_joined=None,
                 fn_configs=None, source="offline"):
    start = next(e for e in events if e["type"] == "ExecutionStarted")
    terminal = [e for e in events if e["type"].startswith("Execution") and
                ("Succeeded" in e["type"] or "Failed" in e["type"] or
                 "Aborted" in e["type"] or "TimedOut" in e["type"])]
    workflow_wall_ms = round((_ts(terminal[0]) - _ts(start)) * 1000.0, 3) if terminal else None

    tasks = parse_task_results(events)
    aggregate = classify_and_aggregate(tasks)
    retries = count_retries(events)
    walls = parse_state_walls(events)
    state_walls = {
        name: {
            "n": len(vals),
            "sum_ms": round(sum(vals), 3),
            "max_ms": round(max(vals), 3),
            "p50_ms": round(statistics.median(vals), 3),
        }
        for name, vals in sorted(walls.items())
    }

    handler = {}
    if cw_joined:
        durations = [r["duration_ms"] for r in cw_joined.values() if "duration_ms" in r]
        billed = [r.get("billed_ms", 0) for r in cw_joined.values()]
        mem = [r.get("max_memory_mb") for r in cw_joined.values() if r.get("max_memory_mb")]
        cold = sum(1 for r in cw_joined.values() if "init_ms" in r)
        gbs = sum((r.get("billed_ms", 0) / 1000.0) * (r.get("memory_mb", 0) / 1024.0)
                  for r in cw_joined.values())
        handler = {
            "joined_invocations": len(cw_joined),
            "cold_start_count": cold,
            "task_handler_ms": {
                "max": round(max(durations), 1) if durations else None,
                "p50": round(statistics.median(durations), 1) if durations else None,
                "sum": round(sum(durations), 1) if durations else None,
            },
            "billed_gb_seconds": round(gbs, 3),
            "max_memory_mb_range": [min(mem), max(mem)] if mem else None,
            "billed_ms_sum": round(sum(billed), 1),
        }

    # workload identity: first task body + execution input best-effort
    identity = dict(aggregate.pop("identity"))
    try:
        exec_input = json.loads(start.get("executionStartedEventDetails", {}).get("input") or "{}")
        for key in ("job_id", "run_id", "N", "times", "n_chunks", "degree",
                    "n_coeffs", "function", "solver_mode", "fused_threads",
                    "pix", "format", "quality"):
            if key in exec_input and key not in identity:
                identity[key] = exec_input[key]
    except Exception:
        pass

    validation = validate_reconstruction(events, tasks, state_walls, identity)
    per_function = {}
    for t in tasks:
        fn = t.get("function_name")
        if fn:
            per_function[fn] = per_function.get(fn, 0) + 1
    return {
        "schema": "pp-production-telemetry-v2",
        "captured_at": datetime.now(timezone.utc).isoformat(),
        "source": source,
        "kind": kind,
        "execution_arn": execution_arn,
        "workflow_wall_ms": workflow_wall_ms,
        "state_walls_ms": state_walls,
        "task_aggregate": aggregate,
        "tasks_by_function": per_function,
        "retries": retries,
        "validation_problems": validation,
        "handler_report": handler or {"note": "no CloudWatch join (offline mode)"},
        "function_configs": fn_configs or {},
        "workload_identity": identity,
    }


def render_markdown(report):
    lines = [f"# Production telemetry — {report['kind']} ({report['source']})", ""]
    lines.append(f"Captured {report['captured_at']}; execution `{report['execution_arn'] or 'n/a'}`.")
    lines.append(f"\n**Workflow wall: {report['workflow_wall_ms']} ms**\n")
    ident = report.get("workload_identity") or {}
    if ident:
        lines.append("## Identity\n")
        for k, v in sorted(ident.items()):
            lines.append(f"- {k}: `{v}`")
        lines.append("")
    lines.append("## State walls (ms)\n")
    lines.append("| State | n | sum | max | p50 |")
    lines.append("|---|---:|---:|---:|---:|")
    for name, s in report["state_walls_ms"].items():
        lines.append(f"| {name} | {s['n']} | {s['sum_ms']} | {s['max_ms']} | {s['p50_ms']} |")
    agg = report["task_aggregate"]
    lines.append(f"\n## Task aggregate ({agg['task_count']} task results)\n")
    lines.append("| Field | class | sum | mean | max | p50 | p95 |")
    lines.append("|---|---|---:|---:|---:|---:|---:|")
    for k, s in agg["fields"].items():
        lines.append(f"| {k} | {s['class']} | {s['sum']} | {s['mean']} | {s['max']} | {s['p50']} | {s['p95']} |")
    if agg["unclassified_time_fields"]:
        lines.append("\n**FLAGGED — timing fields with unknown class (not aggregated):** "
                     + ", ".join(f"`{k}`" for k in agg["unclassified_time_fields"]))
    if report.get("tasks_by_function"):
        lines.append("\n## Invocations by function\n")
        for fn, n in sorted(report["tasks_by_function"].items()):
            lines.append(f"- `{fn}`: {n}")
    retr = report.get("retries") or {}
    lines.append("\n## Retries (§4.5)\n")
    lines.append(f"- task failure events: {retr.get('task_failure_events', 0)}")
    lines.append(f"- scheduled/succeeded: {retr.get('tasks_scheduled', 0)}/{retr.get('tasks_succeeded', 0)}"
                 f" (attempts in excess: {retr.get('attempts_in_excess', 0)})")
    if report.get("validation_problems"):
        lines.append("\n**VALIDATION PROBLEMS (§4.10):**\n")
        for pr in report["validation_problems"]:
            lines.append(f"- {pr}")
    hr = report.get("handler_report") or {}
    lines.append("\n## Lambda handler report\n")
    for k, v in hr.items():
        lines.append(f"- {k}: {v}")
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
                    help="live mode without the CloudWatch/Lambda joins")
    args = ap.parse_args()

    if args.history_file:
        events = load_history(args.history_file)
        execution_arn = ""
        cw = fn_cfg = None
        source = "offline"
    else:
        desc, events = fetch_history_live(args.execution_arn, args.region)
        execution_arn = args.execution_arn
        source = "live"
        cw = fn_cfg = None
        if not args.no_cloudwatch:
            tasks = parse_task_results(events)
            window = (_ts(events[0]), _ts(events[-1]))
            cw = join_cloudwatch(tasks, args.region, window)
            fn_cfg = capture_function_configs(events, args.region)

    kind = args.kind
    if kind == "auto":
        names = {e.get("stateEnteredEventDetails", {}).get("name", "") for e in events}
        kind = "compute" if any("FusedChunk" in n or "Compute" in n for n in names) else "render"

    report = build_report(events, kind=kind, execution_arn=execution_arn,
                          cw_joined=cw, fn_configs=fn_cfg, source=source)

    run_id = args.run_id or (report["workload_identity"].get("run_id") or "run")
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d")
    out_dir = pathlib.Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    base = out_dir / f"{stamp}-{kind}-{run_id}"
    base.with_suffix(".json").write_text(json.dumps(report, indent=2, sort_keys=True))
    base.with_suffix(".md").write_text(render_markdown(report))
    print(f"wrote {base}.json")
    print(f"wrote {base}.md")
    problems = list(report.get("validation_problems") or [])
    if report["task_aggregate"]["unclassified_time_fields"]:
        problems.append("unclassified timing fields: "
                        + ", ".join(report["task_aggregate"]["unclassified_time_fields"]))
    if problems:
        for pr in problems:
            print(f"WARNING: {pr}")
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
