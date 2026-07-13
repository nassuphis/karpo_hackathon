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
