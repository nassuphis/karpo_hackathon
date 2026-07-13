"""CR33-telemetry collector: parsing, timing-class discipline, and joins.

Uses a synthetic Step Functions history (real histories contain complete user
programs and are deliberately never committed). The offline smoke against a
real downloaded history runs only when one is present on the machine.
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
    FIELD_CLASSES,
    build_report,
    classify_and_aggregate,
    parse_state_walls,
    parse_task_results,
)


def _mk_history():
    """Two fused-chunk tasks + one unknown-timing field + map states."""
    def task_output(body, rid):
        return json.dumps({
            "Payload": {"statusCode": 200, "body": json.dumps(body)},
            "SdkResponseMetadata": {"RequestId": rid},
        })
    return [
        {"type": "ExecutionStarted", "timestamp": 1000.0, "id": 1,
         "executionStartedEventDetails": {"input": json.dumps({
             "job_id": "compute_test", "run_id": "run_test", "N": 100})}},
        {"type": "StateEntered", "timestamp": 1000.5, "id": 2,
         "stateEnteredEventDetails": {"name": "FusedChunkWorker"}},
        {"type": "TaskSucceeded", "timestamp": 1003.0, "id": 3,
         "taskSucceededEventDetails": {"output": task_output({
             "param_gen_us": 100, "coeffgen_us": 4000, "solve_us": 9000,
             "upload_params_us": 300, "upload_coeffs_us": 4000,
             "upload_roots_us": 5000, "params_size": 160,
             "mystery_probe_us": 12345,   # unknown class -> must be flagged
             "execution_method": "fused_chunk_pipeline",
             "git_sha": "abc123def",
             "stage_telemetry": {"param_scheduler": "static_file",
                                 "param_native_us": 90},
         }, "req-1")}},
        {"type": "StateExited", "timestamp": 1003.2, "id": 4,
         "stateExitedEventDetails": {"name": "FusedChunkWorker"}},
        {"type": "StateEntered", "timestamp": 1000.5, "id": 5,
         "stateEnteredEventDetails": {"name": "FusedChunkWorker"}},
        {"type": "TaskSucceeded", "timestamp": 1004.0, "id": 6,
         "taskSucceededEventDetails": {"output": task_output({
             "param_gen_us": 120, "coeffgen_us": 5000, "solve_us": 11000,
             "upload_params_us": 340, "upload_coeffs_us": 4400,
             "upload_roots_us": 5600, "params_size": 160,
             "execution_method": "fused_chunk_pipeline",
         }, "req-2")}},
        {"type": "StateExited", "timestamp": 1004.5, "id": 7,
         "stateExitedEventDetails": {"name": "FusedChunkWorker"}},
        {"type": "ExecutionSucceeded", "timestamp": 1010.0, "id": 8},
    ]


def test_state_walls_pair_repeated_names():
    walls = parse_state_walls(_mk_history())
    assert len(walls["FusedChunkWorker"]) == 2
    assert walls["FusedChunkWorker"][0] == pytest.approx(2700.0)
    assert walls["FusedChunkWorker"][1] == pytest.approx(4000.0)


def test_task_results_carry_request_ids():
    tasks = parse_task_results(_mk_history())
    assert [t["request_id"] for t in tasks] == ["req-1", "req-2"]
    assert tasks[0]["body"]["solve_us"] == 9000


def test_unknown_timing_fields_are_flagged_never_summed():
    agg = classify_and_aggregate(parse_task_results(_mk_history()))
    assert "mystery_probe_us" in agg["unclassified_time_fields"]
    assert "mystery_probe_us" not in agg["fields"]
    # known fields aggregate with their class
    assert agg["fields"]["solve_us"]["class"] == "stage_wall_us"
    assert agg["fields"]["solve_us"]["sum"] == 20000
    assert agg["fields"]["solve_us"]["max"] == 11000
    # stage_telemetry sub-dict is flattened and classified
    assert agg["fields"]["param_native_us"]["class"] == "stage_wall_us"


def test_report_shape_and_identity():
    report = build_report(_mk_history(), kind="compute", source="offline")
    assert report["workflow_wall_ms"] == pytest.approx(10000.0)
    ident = report["workload_identity"]
    assert ident["job_id"] == "compute_test"
    assert ident["git_sha"] == "abc123def"           # build attribution
    assert ident["param_scheduler"] == "static_file"
    assert report["task_aggregate"]["task_count"] == 2
    assert "unclassified_time_fields" in report["task_aggregate"]


def test_every_registered_class_is_a_known_timing_class():
    allowed = {"stage_wall_us", "stage_wall_ms", "worker_sum_us", "bytes", "count"}
    assert set(FIELD_CLASSES.values()) <= allowed


def test_interleaved_same_name_states_pair_causally():
    """§4.4: FIFO name pairing mispairs concurrent map iterations. Two
    overlapping FusedChunkWorker states — iteration A (2s) fully containing
    iteration B (0.5s) — must yield [2000, 500], not FIFO's [1400, 1100]."""
    events = [
        {"type": "StateEntered", "timestamp": 100.0, "id": 1,
         "stateEnteredEventDetails": {"name": "W"}},                    # A enter
        {"type": "StateEntered", "timestamp": 100.9, "id": 2,
         "stateEnteredEventDetails": {"name": "W"}},                    # B enter
        {"type": "StateExited", "timestamp": 101.4, "id": 3, "previousEventId": 2,
         "stateExitedEventDetails": {"name": "W"}},                     # B exit
        {"type": "StateExited", "timestamp": 102.0, "id": 4, "previousEventId": 1,
         "stateExitedEventDetails": {"name": "W"}},                     # A exit
    ]
    walls = sorted(parse_state_walls(events)["W"])
    assert walls == [pytest.approx(500.0), pytest.approx(2000.0)]


def test_retry_counting_and_excess_attempts():
    from capture_production_telemetry import count_retries
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
    assert r["tasks_scheduled"] == 2
    assert r["tasks_succeeded"] == 1
    assert r["attempts_in_excess"] == 1


def test_validation_flags_decode_failures_and_chunk_mismatch():
    from capture_production_telemetry import validate_reconstruction
    history = _mk_history()
    # corrupt one task output so decoding fails
    history[2]["taskSucceededEventDetails"]["output"] = "{not json"
    tasks = parse_task_results(history)
    assert any(t["decode_error"] for t in tasks)
    problems = validate_reconstruction(history, tasks, {}, {"n_chunks": 7})
    assert any("failed to decode" in p for p in problems)


def test_causal_chain_extracts_function_and_chunk_identity():
    def sched(eid, fn, chunk):
        return {"type": "TaskScheduled", "timestamp": 10.0, "id": eid,
                "taskScheduledEventDetails": {"parameters": json.dumps({
                    "FunctionName": fn,
                    "Payload": {"body": json.dumps({"chunk_idx": chunk})}})}}
    def started(eid, prev):
        return {"type": "TaskStarted", "timestamp": 10.5, "id": eid,
                "previousEventId": prev}
    def done(eid, prev, rid):
        return {"type": "TaskSucceeded", "timestamp": 11.0, "id": eid,
                "previousEventId": prev,
                "taskSucceededEventDetails": {"output": json.dumps({
                    "Payload": {"body": json.dumps({"solve_us": 1})},
                    "SdkResponseMetadata": {"RequestId": rid}})}}
    events = [
        {"type": "ExecutionStarted", "timestamp": 9.0, "id": 1,
         "executionStartedEventDetails": {"input": "{}"}},
        sched(2, "fn-alpha", 7), started(3, 2), done(4, 3, "r1"),
        sched(5, "fn-beta", 9), started(6, 5), done(7, 6, "r2"),
        {"type": "ExecutionSucceeded", "timestamp": 20.0, "id": 8},
    ]
    tasks = parse_task_results(events)
    assert [(t["function_name"], t["chunk_identity"].get("chunk_idx")) for t in tasks] == [
        ("fn-alpha", 7), ("fn-beta", 9)]


REAL_HISTORY = "/private/tmp/cr33_compute_history.json"


@pytest.mark.skipif(not os.path.exists(REAL_HISTORY),
                    reason="real production history not present on this host")
def test_offline_smoke_against_real_history(tmp_path):
    proc = subprocess.run(
        [sys.executable, str(ROOT / "scripts" / "capture_production_telemetry.py"),
         "--history-file", REAL_HISTORY, "--kind", "compute",
         "--run-id", "smoke", "--out-dir", str(tmp_path)],
        capture_output=True, text=True, timeout=120,
    )
    assert proc.returncode == 0, proc.stdout + proc.stderr
    report = json.loads(next(tmp_path.glob("*compute-smoke.json")).read_text())
    assert report["workflow_wall_ms"] == pytest.approx(36001.0)
    assert report["task_aggregate"]["fields"]["param_gen_us"]["sum"] == pytest.approx(3672650.0)
    assert report["task_aggregate"]["unclassified_time_fields"] == []
    # §4.10 invariants against the real run: 63 tasks, 49 fused chunks, clean
    assert report["retries"]["tasks_succeeded"] == 63
    assert report["retries"]["attempts_in_excess"] == 0
    fused = [fn for fn in report["tasks_by_function"] if "fused-chunk" in fn]
    assert fused and report["tasks_by_function"][fused[0]] == 49
    assert report["validation_problems"] == []
