"""Telemetry collector v3: role scoping, schema semantics, fail-closed
validation, percentiles, identity, and mocked live joins (post-mortem
F2/F3/F4/F6/F8/F10/F11).

Uses synthetic Step Functions histories (real histories contain complete user
programs and are deliberately never committed). Offline smokes against real
downloaded histories run only when present on the machine.
"""
import json
import os
import pathlib
import subprocess
import sys

import pytest

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))

from capture_production_telemetry import (  # noqa: E402
    METRIC_SCHEMA,
    aggregate_by_role,
    build_report,
    count_retries,
    extract_workload_identity,
    join_cloudwatch,
    parse_state_walls,
    parse_task_results,
    quantile_nearest_rank,
    validate_reconstruction,
)


def _task_output(body, rid):
    return json.dumps({
        "Payload": {"statusCode": 200, "body": json.dumps(body)},
        "SdkResponseMetadata": {"RequestId": rid},
    })


def _sched(eid, fn, inner):
    return {"type": "TaskScheduled", "timestamp": 10.0, "id": eid,
            "taskScheduledEventDetails": {"parameters": json.dumps({
                "FunctionName": fn,
                "Payload": {"body": json.dumps(inner)}})}}


def _started(eid, prev):
    return {"type": "TaskStarted", "timestamp": 10.5, "id": eid,
            "previousEventId": prev}


def _done(eid, prev, rid, body, ts=11.0):
    return {"type": "TaskSucceeded", "timestamp": ts, "id": eid,
            "previousEventId": prev,
            "taskSucceededEventDetails": {"output": _task_output(body, rid)}}


def _mk_history(*, n_chunks=2, mixed_build=False):
    """Current-format synthetic history: fused workers + one lores task with
    same-named fields (the F4 contamination shape) + nested params input."""
    events = [
        {"type": "ExecutionStarted", "timestamp": 1000.0, "id": 1,
         "executionStartedEventDetails": {"input": json.dumps({
             "job_id": "compute_test", "run_id": "run_test", "mode": "compute",
             "params": {"N": 100, "times": 1, "n_chunks": n_chunks,
                        "function": "const", "fused_threads": 4,
                        "coeff_program_source_text": "poly[0] = p1"}})}},
    ]
    eid = 2
    for chunk in range(n_chunks):
        fused_body = {
            "param_gen_us": 100 + chunk, "coeffgen_us": 4000, "solve_us": 9000 + chunk * 2000,
            "upload_params_us": 300, "upload_coeffs_us": 4000, "upload_roots_us": 5000,
            "params_size": 160, "compute_us": 9000 + chunk * 2000,   # alias of solve_us
            "mystery_probe_us": 12345,
            "execution_method": "fused_chunk_pipeline",
            "git_sha": ("B" if (mixed_build and chunk == 1) else "A"),
            "stage_telemetry": {"param_scheduler": "static_file",
                                "param_native_us": 90, "lambda_memory_mb": 10240},
        }
        events += [
            _sched(eid, "arn:aws:lambda:us-east-1:1:function:polypaint-compute-fused-chunk",
                   {"chunk_idx": chunk}),
            _started(eid + 1, eid),
            _done(eid + 2, eid + 1, f"req-fused-{chunk}", fused_body, ts=1003.0 + chunk),
        ]
        eid += 3
    # a lores task with SAME-NAMED fields (must not contaminate the fused role)
    events += [
        _sched(eid, "arn:aws:lambda:us-east-1:1:function:polypaint-coeffgen", {}),
        _started(eid + 1, eid),
        _done(eid + 2, eid + 1, "req-lores",
              {"coeffgen_us": 999999, "compute_us": 888888, "elapsed_us": 5}),
    ]
    eid += 3
    events.append({"type": "ExecutionSucceeded", "timestamp": 1010.0, "id": eid})
    return events


# ── F8: percentiles ─────────────────────────────────────────────────────────

@pytest.mark.parametrize("values", [[1.0], [1.0, 2.0], [1.0, 2.0, 3.0],
                                    list(map(float, range(49))),
                                    [5.0] * 7, [3.0, 1.0, 2.0]])
def test_percentile_invariant_min_p50_p95_max(values):
    ordered = sorted(values)
    p50 = quantile_nearest_rank(ordered, 0.5)
    p95 = quantile_nearest_rank(ordered, 0.95)
    assert ordered[0] <= p50 <= p95 <= ordered[-1]


def test_percentile_two_sample_regression():
    """The exact case from the post-mortem: [1, 2] must never report
    p95 < p50."""
    ordered = [1.0, 2.0]
    assert quantile_nearest_rank(ordered, 0.5) == 1.0
    assert quantile_nearest_rank(ordered, 0.95) == 2.0


# ── F4/F10: role scoping and schema semantics ───────────────────────────────

def test_roles_do_not_mix_and_aliases_fold():
    agg = aggregate_by_role(parse_task_results(_mk_history()))
    fused = agg["roles"]["polypaint-compute-fused-chunk"]
    lores = agg["roles"]["polypaint-coeffgen"]
    # fused role sees exactly its own tasks
    assert fused["task_count"] == 2
    assert fused["fields"]["solve_us"]["n"] == 2
    assert fused["fields"]["solve_us"]["sum"] == 20000
    # compute_us folded into solve_us (alias present alongside canonical)
    assert "compute_us" not in fused["fields"]
    # the lores task's same-named fields stay in the lores role — and its
    # compute_us (no solve_us in that body) aggregates under the canonical
    assert lores["fields"]["coeffgen_us"]["sum"] == 999999
    assert lores["fields"]["solve_us"]["sum"] == 888888
    assert fused["fields"]["coeffgen_us"]["sum"] == 8000


def test_invariants_report_single_value_not_sum():
    agg = aggregate_by_role(parse_task_results(_mk_history()))
    fused = agg["roles"]["polypaint-compute-fused-chunk"]
    inv = fused["invariants"]["lambda_memory_mb"]
    assert inv["value"] == 10240
    assert "sum" not in inv


def test_distribution_fields_have_no_sum():
    agg = aggregate_by_role(parse_task_results(_mk_history()))
    lores = agg["roles"]["polypaint-coeffgen"]
    assert "sum" not in lores["fields"]["elapsed_us"]
    assert lores["fields"]["elapsed_us"]["aggregation"] == "distribution"


def test_unknown_timing_fields_are_flagged_never_summed():
    agg = aggregate_by_role(parse_task_results(_mk_history()))
    assert "mystery_probe_us" in agg["unclassified_time_fields"]
    for role in agg["roles"].values():
        assert "mystery_probe_us" not in role["fields"]


def test_schema_values_are_wellformed():
    allowed_cls = {"stage_wall_us", "stage_wall_ms", "worker_sum_us", "bytes", "count"}
    allowed_agg = {"additive", "distribution", "invariant"}
    for name, spec in METRIC_SCHEMA.items():
        assert spec["cls"] in allowed_cls, name
        assert spec["agg"] in allowed_agg, name
        if spec["alias_of"]:
            assert spec["alias_of"] in METRIC_SCHEMA, name
        if spec["parent"]:
            assert spec["parent"] in METRIC_SCHEMA, name


# ── F6: mixed builds fail, never first-value-wins ───────────────────────────

def test_mixed_build_identity_is_a_validation_problem():
    agg = aggregate_by_role(parse_task_results(_mk_history(mixed_build=True)))
    assert any("git_sha" in p and "distinct values" in p for p in agg["problems"])
    assert isinstance(agg["identity"]["git_sha"], dict)
    assert sorted(agg["identity"]["git_sha"]["conflict"]) == ["A", "B"]


def test_mixed_build_report_exits_invalid():
    report = build_report(_mk_history(mixed_build=True), kind="compute", source="offline")
    assert any("git_sha" in p for p in report["validation_problems"])


# ── F2: workload identity from the nested input ─────────────────────────────

def test_identity_extracted_from_nested_params_with_fingerprints():
    events = _mk_history()
    tasks = parse_task_results(events)
    identity, problems = extract_workload_identity(events, tasks, "compute")
    assert problems == []
    assert identity["N"] == 100
    assert identity["n_chunks"] == 2
    assert identity["function"] == "const"
    assert identity["fused_threads"] == 4
    assert "coeff_program_source_text_sha256" in identity


def test_missing_required_identity_invalidates():
    events = _mk_history()
    events[0]["executionStartedEventDetails"]["input"] = json.dumps(
        {"job_id": "compute_test", "run_id": "r"})
    identity, problems = extract_workload_identity(events, parse_task_results(events), "compute")
    assert any("required workload identity missing" in p for p in problems)


# ── F11: fail-closed reconstruction ─────────────────────────────────────────

def test_cardinality_check_runs_even_with_zero_reconstructed_tasks():
    problems = validate_reconstruction(
        [], [], {"n_chunks": 7}, "compute",
        live=False, cw_errors=[], cw_joined={}, fn_config_errors=[])
    assert any("expected 7 fused chunk tasks, reconstructed 0" in p for p in problems)


def test_bad_status_code_and_decode_failures_flagged():
    events = _mk_history()
    # corrupt one output
    for e in events:
        if e["type"] == "TaskSucceeded":
            e["taskSucceededEventDetails"]["output"] = "{not json"
            break
    tasks = parse_task_results(events)
    problems = validate_reconstruction(
        events, tasks, {}, "compute",
        live=False, cw_errors=[], cw_joined={}, fn_config_errors=[])
    assert any("failed to decode" in p for p in problems)


def test_causal_chain_carries_function_and_chunk():
    tasks = parse_task_results(_mk_history())
    fused = [t for t in tasks if t["role"] == "polypaint-compute-fused-chunk"]
    assert [t["chunk_identity"]["chunk_idx"] for t in fused] == [0, 1]
    assert all(t["status_code"] == 200 for t in fused)


def test_interleaved_same_name_states_pair_causally():
    events = [
        {"type": "StateEntered", "timestamp": 100.0, "id": 1,
         "stateEnteredEventDetails": {"name": "W"}},
        {"type": "StateEntered", "timestamp": 100.9, "id": 2,
         "stateEnteredEventDetails": {"name": "W"}},
        {"type": "StateExited", "timestamp": 101.4, "id": 3, "previousEventId": 2,
         "stateExitedEventDetails": {"name": "W"}},
        {"type": "StateExited", "timestamp": 102.0, "id": 4, "previousEventId": 1,
         "stateExitedEventDetails": {"name": "W"}},
    ]
    walls = sorted(parse_state_walls(events)["W"])
    assert walls == [pytest.approx(500.0), pytest.approx(2000.0)]


def test_retry_counting_and_excess_attempts():
    events = [
        {"type": "TaskScheduled", "timestamp": 1.0, "id": 1},
        {"type": "TaskFailed", "timestamp": 2.0, "id": 2,
         "taskFailedEventDetails": {"resourceType": "lambda", "resource": "invoke"}},
        {"type": "TaskScheduled", "timestamp": 3.0, "id": 3},
        {"type": "TaskSucceeded", "timestamp": 4.0, "id": 4,
         "taskSucceededEventDetails": {"output": "{}"}},
    ]
    r = count_retries(events)
    assert r["task_failure_events"] == 1
    assert r["attempts_in_excess"] == 1


# ── F3: mocked live CloudWatch joins, fail-closed ───────────────────────────

class _FakeLogs:
    """Mocked CloudWatch Logs client: pagination, a missing group, and a
    partial-coverage scenario."""
    def __init__(self, reports_by_group, missing_groups=()):
        self.reports_by_group = reports_by_group
        self.missing_groups = set(missing_groups)
        self.calls = []

    def filter_log_events(self, **kwargs):
        group = kwargs["logGroupName"]
        self.calls.append(group)
        if group in self.missing_groups:
            raise RuntimeError("ResourceNotFoundException: log group missing")
        msgs = self.reports_by_group.get(group, [])
        # paginate: first call returns half + token, second returns rest
        if "nextToken" not in kwargs and len(msgs) > 1:
            half = len(msgs) // 2
            return {"events": [{"message": m} for m in msgs[:half]],
                    "nextToken": "t1"}
        start = len(msgs) // 2 if kwargs.get("nextToken") else 0
        return {"events": [{"message": m} for m in msgs[start:]]}


def _report_line(rid, dur=1234.5, billed=1300.0, mem=10240, maxmem=2400, init=None):
    line = (f"REPORT RequestId: {rid}\tDuration: {dur} ms\t"
            f"Billed Duration: {billed} ms\tMemory Size: {mem} MB\t"
            f"Max Memory Used: {maxmem} MB")
    if init is not None:
        line += f"\tInit Duration: {init} ms"
    return line


def test_live_join_complete_coverage_and_pagination():
    tasks = parse_task_results(_mk_history())
    group = "/aws/lambda/polypaint-compute-fused-chunk"
    lores_group = "/aws/lambda/polypaint-coeffgen"
    fake = _FakeLogs({
        group: [_report_line("req-fused-0", init=500.0), _report_line("req-fused-1")],
        lores_group: [_report_line("req-lores")],
    })
    joined, errors = join_cloudwatch(tasks, "us-east-1", (1000.0, 1010.0),
                                     logs_client=fake)
    assert errors == []
    assert set(joined) == {"req-fused-0", "req-fused-1", "req-lores"}
    assert joined["req-fused-0"]["role"] == "polypaint-compute-fused-chunk"
    # pagination was exercised (two calls for the two-message group)
    assert fake.calls.count(group) >= 2


def test_live_join_fail_closed_on_missing_group_and_partial_coverage():
    tasks = parse_task_results(_mk_history())
    fake = _FakeLogs(
        {"/aws/lambda/polypaint-coeffgen": [_report_line("req-lores")]},
        missing_groups={"/aws/lambda/polypaint-compute-fused-chunk"})
    joined, errors = join_cloudwatch(tasks, "us-east-1", (1000.0, 1010.0),
                                     logs_client=fake)
    assert any("log query failed" in e for e in errors)
    assert any("did not join" in e for e in errors)
    # and the report treats a live run with these errors as invalid
    report = build_report(_mk_history(), kind="compute", source="live",
                          cw_joined=joined, cw_errors=errors)
    assert any("did not join" in p or "join incomplete" in p
               for p in report["validation_problems"])


def test_duplicate_report_lines_are_errors():
    tasks = parse_task_results(_mk_history())
    group = "/aws/lambda/polypaint-compute-fused-chunk"
    fake = _FakeLogs({
        group: [_report_line("req-fused-0"), _report_line("req-fused-0"),
                _report_line("req-fused-1")],
        "/aws/lambda/polypaint-coeffgen": [_report_line("req-lores")],
    })
    joined, errors = join_cloudwatch(tasks, "us-east-1", (1000.0, 1010.0),
                                     logs_client=fake)
    assert any("duplicate REPORT join" in e for e in errors)


# ── report shape + offline smokes against retained real histories ──────────

def test_report_shape_v3():
    report = build_report(_mk_history(), kind="compute", source="offline")
    assert report["schema"] == "pp-production-telemetry-v3"
    assert report["workflow_wall_ms"] == pytest.approx(10000.0)
    assert "polypaint-compute-fused-chunk" in report["task_aggregate_by_role"]
    assert report["workload_identity"]["param_scheduler"] == "static_file"
    # the only expected problem in the synthetic run is the unclassified probe
    assert report["validation_problems"] == []
    assert report["unclassified_time_fields"] == ["mystery_probe_us"]


REAL_COMPUTE = "/private/tmp/cr33_compute_history.json"
REAL_RENDER = "/private/tmp/cr34_render_history.json"


@pytest.mark.skipif(not os.path.exists(REAL_COMPUTE),
                    reason="real production history not present on this host")
def test_offline_smoke_real_compute(tmp_path):
    proc = subprocess.run(
        [sys.executable, str(ROOT / "scripts" / "capture_production_telemetry.py"),
         "--history-file", REAL_COMPUTE, "--kind", "compute",
         "--run-id", "smoke", "--out-dir", str(tmp_path)],
        capture_output=True, text=True, timeout=120,
    )
    assert proc.returncode == 0, proc.stdout + proc.stderr
    report = json.loads(next(tmp_path.glob("*compute-smoke.json")).read_text())
    fused = report["task_aggregate_by_role"]["polypaint-compute-fused-chunk"]
    assert fused["task_count"] == 49
    assert fused["fields"]["solve_us"]["n"] == 49
    assert fused["fields"]["param_gen_us"]["sum"] == pytest.approx(3672650.0)
    assert report["workload_identity"]["N"] == 10000
    assert report["workload_identity"]["n_chunks"] == 49
    assert report["validation_problems"] == []


@pytest.mark.skipif(not os.path.exists(REAL_RENDER),
                    reason="real production history not present on this host")
def test_offline_smoke_real_render(tmp_path):
    proc = subprocess.run(
        [sys.executable, str(ROOT / "scripts" / "capture_production_telemetry.py"),
         "--history-file", REAL_RENDER, "--kind", "render",
         "--run-id", "smoke", "--out-dir", str(tmp_path)],
        capture_output=True, text=True, timeout=120,
    )
    assert proc.returncode == 0, proc.stdout + proc.stderr
