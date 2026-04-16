"""
Tests for handler_solve_proximity.py merge phase logic.

The clip and hist phases delegate to the C binary (tested in test_solve_proximity_stats.py).
The merge phase is pure Python — derive 9 equal-density bin cuts from summed histograms.

Tests metric awareness: preserves requested metric, rejects mismatched artifacts,
output artifact has correct family and cuts_norm length.

Run: cd polypaint && uv run python tests/test_solve_proximity_handler.py
"""
import json
import os
import sys
import tempfile
import unittest.mock as mock

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _ChunkBody:
    def __init__(self, data):
        self._data = data

    def read(self):
        return self._data

    def iter_chunks(self, chunk_size=1024 * 1024):
        for i in range(0, len(self._data), chunk_size):
            yield self._data[i:i + chunk_size]

    def close(self):
        return None


def _make_mock_s3(clip_data, hist_responses):
    """Build a mock S3 client for merge tests."""
    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if "clip" in key:
            return {"Body": mock.MagicMock(read=lambda: json.dumps(clip_data).encode())}
        if key in hist_responses:
            data = hist_responses[key]
            return {"Body": mock.MagicMock(read=lambda d=data: json.dumps(d).encode())}
        raise mock_s3.exceptions.NoSuchKey({"Error": {"Code": "NoSuchKey"}}, "GetObject")

    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()
    mock_s3.exceptions = type('Exc', (), {'NoSuchKey': type('NoSuchKey', (Exception,), {})})()
    return mock_s3


def _make_hist_mock_s3(bin_bytes, clip_data):
    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if key == "renders/test/chunk_0.bin":
            return {"Body": _ChunkBody(bin_bytes), "ContentLength": len(bin_bytes)}
        if key == "renders/test/solve_scores/clip.json":
            return {"Body": _ChunkBody(json.dumps(clip_data).encode())}
        raise AssertionError(f"unexpected get_object key: {key}")

    mock_s3.get_object = mock_get
    mock_s3.head_object = mock.MagicMock(return_value={"ContentLength": len(bin_bytes)})
    mock_s3.generate_presigned_url = mock.MagicMock(return_value="https://example.com/range.bin")
    mock_s3.put_object = mock.MagicMock()
    mock_s3.exceptions = type('Exc', (), {'NoSuchKey': type('NoSuchKey', (Exception,), {})})()
    return mock_s3


def _run_merge(n_chunks, clip_data, hist_responses, metric="proximity", solve_score_quantile=0.001,
               solve_score_omega=1.0, solve_score_merge_workers=None):
    """Run merge phase with mocked S3."""
    import handler_solve_proximity as hsp
    mock_s3 = _make_mock_s3(clip_data, hist_responses)
    orig_s3, orig_report, orig_merge_client = hsp.s3, hsp.report_status, hsp._merge_s3_client
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp._merge_s3_client = mock.MagicMock(return_value=mock_s3)
    try:
        payload = {
            "job_id": "test",
            "task_id": "merge_test",
            "metric": metric,
            "solve_score_quantile": solve_score_quantile,
            "solve_score_omega": solve_score_omega,
            "n_chunks": n_chunks,
            "hist_prefix": "renders/test/solve_scores/",
            "clip_key": "renders/test/solve_scores/clip.json",
            "out_key": "renders/test/solve_scores/bins.json",
        }
        if solve_score_merge_workers is not None:
            payload["solve_score_merge_workers"] = solve_score_merge_workers
        result = hsp.handle_merge(payload)
        # Capture what was written to S3
        put_calls = mock_s3.put_object.call_args_list
        written_artifact = None
        if put_calls:
            body_bytes = put_calls[0][1].get("Body", b"{}")
            written_artifact = json.loads(body_bytes)
        return json.loads(result["body"]), written_artifact
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp._merge_s3_client = orig_merge_client


def _uniform_hist_data(prefix, n_chunks, metric="proximity", clip_quantile=0.001):
    """Generate uniform histogram responses for n_chunks."""
    clip_data = {
        "family": "solve_score", "metric": metric,
        "clip_quantile": clip_quantile,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {}
    for s in range(n_chunks):
        hist_responses[f"{prefix}chunk_{s}_hist.json"] = {
            "family": "solve_score", "metric": metric,
            "clip_quantile": clip_quantile,
            "omega": 1.0,
            "hist": [20] * 100, "n_solves": 2000,
        }
    return clip_data, hist_responses


def test_hist_tmpfile_mode_runs_binary_with_tmp_input():
    import handler_solve_proximity as hsp

    mock_s3 = _make_hist_mock_s3(
        b"\x00" * 32,
        {"clip_lo": 0.0, "clip_hi": 1.0, "family": "solve_score", "metric": "proximity", "clip_quantile": 0.001, "omega": 1.0, "omega_enabled": True},
    )
    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=0,
        stdout=json.dumps({"threads": 2, "n_solves": 2, "hist": [2, 0, 0, 0]}),
        stderr="",
    ))
    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_hist({
            "job_id": "test",
            "task_id": "hist_test",
            "chunk_idx": 0,
            "metric": "proximity",
            "bin_key": "renders/test/chunk_0.bin",
            "degree": 2,
            "clip_key": "renders/test/solve_scores/clip.json",
            "hist_bins": 4,
            "out_key": "renders/test/solve_scores/chunk_0_hist.json",
        })
        body = json.loads(result["body"])
        assert body["input_mode"] == "tmpfile"
        cmd = mock_run.call_args.args[0]
        assert cmd[1] == hsp._TMP_INPUT
        assert not any(arg.startswith("--input_size=") for arg in cmd)
        done_kwargs = hsp.report_status.call_args_list[-1].kwargs
        warned = {w["param"] for w in done_kwargs["result_data"]["contract_warnings"]}
        assert "solve_score_quantile" in warned
        assert "solve_score_omega" in warned
        assert "solve_score_omega_enabled" in warned
        assert "solve_score_threads" in warned
        assert "solve_score_hist_input_mode" in warned
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_hist_stdin_mode_streams_input_and_passes_input_size():
    import handler_solve_proximity as hsp

    mock_s3 = _make_hist_mock_s3(
        b"\x01" * 48,
        {"clip_lo": -1.0, "clip_hi": 2.0, "family": "solve_score", "metric": "centroid_re", "clip_quantile": 0.001, "omega": 1.0, "omega_enabled": True},
    )
    mock_stream = mock.MagicMock(return_value=(0, json.dumps({"threads": 5, "n_solves": 3, "hist": [1, 1, 1]}), "", 12, 34))
    orig_s3, orig_report, orig_stream = hsp.s3, hsp.report_status, hsp._run_binary_with_streamed_input
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp._run_binary_with_streamed_input = mock_stream
    try:
        result = hsp.handle_hist({
            "job_id": "test",
            "task_id": "hist_test",
            "chunk_idx": 0,
            "metric": "centroid_re",
            "bin_key": "renders/test/chunk_0.bin",
            "degree": 3,
            "clip_key": "renders/test/solve_scores/clip.json",
            "hist_bins": 3,
            "out_key": "renders/test/solve_scores/chunk_0_hist.json",
            "solve_score_hist_input_mode": "stdin",
        })
        body = json.loads(result["body"])
        assert body["input_mode"] == "stdin"
        cmd = mock_stream.call_args.args[0]
        assert cmd[1] == "-"
        assert any(arg == "--input_size=48" for arg in cmd)
        streamed_obj = mock_stream.call_args.args[1]
        assert streamed_obj["ContentLength"] == 48
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp._run_binary_with_streamed_input = orig_stream


def test_hist_invalid_input_mode_rejected():
    import handler_solve_proximity as hsp

    try:
        hsp.handle_hist({
            "job_id": "test",
            "task_id": "hist_test",
            "chunk_idx": 0,
            "metric": "proximity",
            "bin_key": "renders/test/chunk_0.bin",
            "degree": 2,
            "clip_key": "renders/test/solve_scores/clip.json",
            "hist_bins": 4,
            "out_key": "renders/test/solve_scores/chunk_0_hist.json",
            "solve_score_hist_input_mode": "pipe",
        })
        assert False, "should have raised on invalid solve_score_hist_input_mode"
    except RuntimeError as e:
        assert "solve_score_hist_input_mode" in str(e)


def test_hist_sectioned_mode_uses_presigned_url_and_sectioned_binary():
    import handler_solve_proximity as hsp

    mock_s3 = _make_hist_mock_s3(
        b"\x02" * 64,
        {"clip_lo": -2.0, "clip_hi": 3.0, "family": "solve_score", "metric": "centroid_re", "clip_quantile": 0.001, "omega": 1.0, "omega_enabled": True},
    )
    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=0,
        stdout=json.dumps({
            "threads": 4,
            "n_solves": 4,
            "download_ms": 123,
            "compute_ms": 45,
            "wall_ms": 101,
            "hist": [1, 1, 1, 1],
        }),
        stderr="",
    ))
    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_hist({
            "job_id": "test",
            "task_id": "hist_test",
            "chunk_idx": 0,
            "metric": "centroid_re",
            "bin_key": "renders/test/chunk_0.bin",
            "degree": 4,
            "clip_key": "renders/test/solve_scores/clip.json",
            "hist_bins": 4,
            "out_key": "renders/test/solve_scores/chunk_0_hist.json",
            "solve_score_hist_input_mode": "sectioned",
            "solve_score_threads": 4,
        })
        body = json.loads(result["body"])
        assert body["input_mode"] == "sectioned"
        assert body["threads"] == 4
        assert body["source_size"] == 64
        assert body["compute_ms"] == 45
        assert body["wall_ms"] == 101
        cmd = mock_run.call_args.args[0]
        assert cmd[0] == hsp.SECTIONED_HIST_BINARY
        assert any(arg == "--input_size=64" for arg in cmd)
        assert any(arg == "--threads=4" for arg in cmd)
        assert any(arg.startswith("--url=https://example.com/") for arg in cmd)
        mock_s3.generate_presigned_url.assert_called_once()
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_hist_sectioned_failure_includes_object_context():
    import handler_solve_proximity as hsp

    mock_s3 = _make_hist_mock_s3(
        b"\x02" * 64,
        {"clip_lo": -2.0, "clip_hi": 3.0, "family": "solve_score", "metric": "centroid_re", "clip_quantile": 0.001, "omega": 1.0, "omega_enabled": True},
    )
    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=1,
        stdout="",
        stderr="range GET failed for bytes 16-31: The requested URL returned error: 503",
    ))
    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    try:
        try:
            hsp.handle_hist({
                "job_id": "test",
                "task_id": "hist_test",
                "chunk_idx": 7,
                "metric": "centroid_re",
                "bin_key": "renders/test/chunk_7.bin",
                "degree": 4,
                "clip_key": "renders/test/solve_scores/clip.json",
                "hist_bins": 4,
                "out_key": "renders/test/solve_scores/chunk_7_hist.json",
                "solve_score_hist_input_mode": "sectioned",
                "solve_score_threads": 6,
            })
            assert False, "expected sectioned hist failure"
        except RuntimeError as e:
            msg = str(e)
            assert "s3://polypaint/renders/test/chunk_7.bin" in msg
            assert "clip=s3://polypaint/renders/test/solve_scores/clip.json" in msg
            assert "chunk=7" in msg
            assert "threads=6" in msg
            assert "range GET failed for bytes 16-31" in msg
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_hist_v2_clip_uses_program_cli_flags():
    import handler_solve_proximity as hsp

    mock_s3 = _make_hist_mock_s3(
        b"\x03" * 64,
        {
            "family": "solve_score",
            "version": 2,
            "metric": "spread",
            "clip_quantile": 0.02,
            "omega": 5.0,
            "omega_enabled": True,
            "clip_lo": -1.0,
            "clip_hi": 2.0,
            "program": "m0;m1;weighted_sum:0.7:0.3;omega_cosine:5",
            "metrics": [
                {"slot": 0, "metric": "spread", "quantile": 0.02, "clip_lo": -1.0, "clip_hi": 2.0},
                {"slot": 1, "metric": "shelliness", "quantile": 0.03, "clip_lo": -0.5, "clip_hi": 1.5},
            ],
        },
    )
    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=0,
        stdout=json.dumps({"threads": 2, "n_solves": 4, "hist": [1, 1, 1, 1]}),
        stderr="",
    ))
    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_hist({
            "job_id": "test",
            "task_id": "hist_test",
            "chunk_idx": 0,
            "metric": "spread",
            "solve_score_quantile": 0.02,
            "solve_score_omega": 5.0,
            "solve_score_omega_enabled": True,
            "solve_score_chain": [["spread", "2"], ["shelliness", "3"], ["weighted_sum", "0.7", "0.3"], ["omega_cosine", "5"]],
            "bin_key": "renders/test/chunk_0.bin",
            "degree": 4,
            "clip_key": "renders/test/solve_scores/clip.json",
            "hist_bins": 4,
            "out_key": "renders/test/solve_scores/chunk_0_hist.json",
        })
        body = json.loads(result["body"])
        assert body["metric"] == "spread"
        cmd = mock_run.call_args.args[0]
        assert any(arg == "--score_metrics=spread,shelliness" for arg in cmd)
        assert any(arg == "--score_clip_los=-1,-0.5" for arg in cmd)
        assert any(arg == "--score_clip_his=2,1.5" for arg in cmd)
        assert any(arg == "--score_program=m0;m1;weighted_sum:0.7:0.3;omega_cosine:5" for arg in cmd)
        assert not any(arg.startswith("--metric=") for arg in cmd)
        assert not any(arg.startswith("--clip_lo=") for arg in cmd)
        assert not any(arg.startswith("--clip_hi=") for arg in cmd)
        written = json.loads(hsp.s3.put_object.call_args.kwargs["Body"])
        assert written["version"] == 2
        assert written["program"] == "m0;m1;weighted_sum:0.7:0.3;omega_cosine:5"
        assert written["metric_count"] == 2
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_hist_v2_clip_preserves_omega_phase_in_program_cli():
    import handler_solve_proximity as hsp

    mock_s3 = _make_hist_mock_s3(
        b"\x03" * 64,
        {
            "family": "solve_score",
            "version": 2,
            "metric": "spread",
            "clip_quantile": 0.02,
            "omega": 5.0,
            "omega_enabled": True,
            "clip_lo": -1.0,
            "clip_hi": 2.0,
            "program": "m0;omega_cosine:5:1.25",
            "metrics": [
                {"slot": 0, "metric": "spread", "quantile": 0.02, "clip_lo": -1.0, "clip_hi": 2.0},
            ],
        },
    )
    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=0,
        stdout=json.dumps({"threads": 2, "n_solves": 4, "hist": [1, 1, 1, 1]}),
        stderr="",
    ))
    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_hist({
            "job_id": "test",
            "task_id": "hist_test_phase",
            "chunk_idx": 0,
            "metric": "spread",
            "solve_score_quantile": 0.02,
            "solve_score_omega": 5.0,
            "solve_score_omega_enabled": True,
            "solve_score_chain": [["spread", "2"], ["omega_cosine", "5", "1.25"]],
            "bin_key": "renders/test/chunk_0.bin",
            "degree": 4,
            "clip_key": "renders/test/solve_scores/clip.json",
            "hist_bins": 4,
            "out_key": "renders/test/solve_scores/chunk_0_hist.json",
        })
        body = json.loads(result["body"])
        assert body["metric"] == "spread"
        cmd = mock_run.call_args.args[0]
        assert any(arg == "--score_program=m0;omega_cosine:5:1.25" for arg in cmd)
        written = json.loads(hsp.s3.put_object.call_args.kwargs["Body"])
        assert written["program"] == "m0;omega_cosine:5:1.25"
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_clip_v2_mixed_source_writes_source_metadata_and_coeff_context():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if key == "renders/test/lores.bin":
            return {"Body": _ChunkBody(b"\x00" * 64)}
        if key == "renders/test/lores_coeffs.bin":
            return {"Body": _ChunkBody(b"\x11" * 80)}
        raise AssertionError(f"unexpected get_object key: {key}")

    clip_stdout = [
        json.dumps({"clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0, "n_solves": 4, "threads": 1}),
        json.dumps({"clip_lo": -2.0, "clip_hi": 3.0, "min_score": -2.0, "max_score": 3.0, "n_solves": 4, "threads": 1}),
    ]

    def mock_run(cmd, capture_output, text, timeout):
        return mock.MagicMock(returncode=0, stdout=clip_stdout.pop(0), stderr="")

    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()
    try:
        result = hsp.handle_clip({
            "job_id": "test",
            "task_id": "clip_test",
            "metric": "spread",
            "solve_score_quantile": 0.001,
            "solve_score_chain": [["spread", "slv", "1"], ["spread", "cf", "2"], ["avg"]],
            "degree": 4,
            "n_coeffs": 5,
            "lores_bin_key": "renders/test/lores.bin",
            "lores_coeffs_key": "renders/test/lores_coeffs.bin",
            "out_key": "renders/test/solve_scores/clip.json",
        })
        body = json.loads(result["body"])
        assert body["metric_count"] == 2
        written = json.loads(mock_s3.put_object.call_args.kwargs["Body"])
        assert written["version"] == 2
        assert written["program"] == "m0;m1;avg"
        assert written["lores_coeffs_key"] == "renders/test/lores_coeffs.bin"
        assert written["n_coeffs"] == 5
        assert [m["source"] for m in written["metrics"]] == ["slv", "cf"]
        done_kwargs = hsp.report_status.call_args_list[-1].kwargs
        assert done_kwargs["result_data"]["source_coeffs_key"] == "renders/test/lores_coeffs.bin"
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_hist_v2_mixed_source_sectioned_passes_coeff_url_cli_flags():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if key == "renders/test/solve_scores/clip.json":
            clip_data = {
                "family": "solve_score",
                "version": 2,
                "metric": "spread",
                "clip_quantile": 0.02,
                "omega": 1.0,
                "omega_enabled": False,
                "clip_lo": 0.0,
                "clip_hi": 1.0,
                "program": "m0;m1;max",
                "metrics": [
                    {"slot": 0, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 1, "metric": "spread", "source": "cf", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
            }
            return {"Body": _ChunkBody(json.dumps(clip_data).encode())}
        raise AssertionError(f"unexpected get_object key: {key}")

    mock_s3.get_object = mock_get
    mock_s3.generate_presigned_url.side_effect = [
        "https://example.com/coeffs.bin",
        "https://example.com/chunk.bin",
    ]
    mock_s3.put_object = mock.MagicMock()
    mock_s3.exceptions = type('Exc', (), {'NoSuchKey': type('NoSuchKey', (Exception,), {})})()

    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=0,
        stdout=json.dumps({
            "threads": 4,
            "n_solves": 4,
            "download_ms": 123,
            "compute_ms": 45,
            "wall_ms": 101,
            "hist": [1, 1, 1, 1],
        }),
        stderr="",
    ))
    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_hist({
            "job_id": "test",
            "task_id": "hist_test",
            "chunk_idx": 0,
            "metric": "spread",
            "solve_score_quantile": 0.02,
            "solve_score_chain": [["spread", "slv", "2"], ["spread", "cf", "2"], ["max"]],
            "bin_key": "renders/test/chunk_0.bin",
            "coeffs_key": "renders/test/coeffs_0000.bin",
            "bin_size": 64,
            "coeffs_bin_size": 80,
            "degree": 4,
            "n_coeffs": 5,
            "clip_key": "renders/test/solve_scores/clip.json",
            "hist_bins": 4,
            "out_key": "renders/test/solve_scores/chunk_0_hist.json",
            "solve_score_hist_input_mode": "sectioned",
            "solve_score_threads": 4,
        })
        body = json.loads(result["body"])
        assert body["input_mode"] == "sectioned"
        cmd = mock_run.call_args.args[0]
        assert "--score_sources=slv,cf" in cmd
        assert "--score_program=m0;m1;max" in cmd
        assert "--score_coeffs_url=https://example.com/coeffs.bin" in cmd
        assert "--score_coeff_input_size=80" in cmd
        assert "--score_coeff_degree=5" in cmd
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_hist_v2_logical_section_rebuilds_spans_from_chunk_manifest():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        rng = kwargs.get("Range")
        if key == "renders/test/solve_scores/clip.json":
            clip_data = {
                "family": "solve_score",
                "version": 2,
                "metric": "spread",
                "clip_quantile": 0.02,
                "omega": 1.0,
                "omega_enabled": False,
                "clip_lo": 0.0,
                "clip_hi": 1.0,
                "program": "m0;m1;max",
                "metrics": [
                    {"slot": 0, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 1, "metric": "spread", "source": "cf", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
            }
            return {"Body": _ChunkBody(json.dumps(clip_data).encode())}
        expected_ranges = {
            ("renders/test/chunk_0.bin", "bytes=0-47"): b"\x11" * 48,
            ("renders/test/chunk_1.bin", "bytes=0-31"): b"\x22" * 32,
            ("renders/test/coeffs_0000.bin", "bytes=0-71"): b"\x33" * 72,
            ("renders/test/coeffs_0001.bin", "bytes=0-47"): b"\x44" * 48,
        }
        data = expected_ranges.get((key, rng))
        if data is None:
            raise AssertionError(f"unexpected get_object key/range: {key} {rng}")
        return {"Body": _ChunkBody(data)}

    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()
    mock_s3.exceptions = type('Exc', (), {'NoSuchKey': type('NoSuchKey', (Exception,), {})})()

    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=0,
        stdout=json.dumps({
            "threads": 4,
            "n_solves": 5,
            "download_ms": 12,
            "compute_ms": 34,
            "wall_ms": 56,
            "hist": [2, 1, 1, 1],
        }),
        stderr="",
    ))
    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    try:
        with tempfile.TemporaryDirectory() as td:
            tmp_input = os.path.join(td, "input.bin")
            tmp_coeff = os.path.join(td, "coeff.bin")
            with mock.patch.object(hsp, "_TMP_INPUT", tmp_input), \
                 mock.patch.object(hsp, "_TMP_COEFF_INPUT", tmp_coeff):
                result = hsp.handle_hist({
                    "job_id": "test",
                    "task_id": "hist_test",
                    "chunk_idx": 0,
                    "section_idx": 0,
                    "section_count": 2,
                    "logical_section": True,
                    "chunk_manifest": [
                        {
                            "chunk_idx": 0,
                            "bin_key": "renders/test/chunk_0.bin",
                            "coeffs_key": "renders/test/coeffs_0000.bin",
                            "step_start": 0,
                            "step_count": 3,
                        },
                        {
                            "chunk_idx": 1,
                            "bin_key": "renders/test/chunk_1.bin",
                            "coeffs_key": "renders/test/coeffs_0001.bin",
                            "step_start": 3,
                            "step_count": 3,
                        },
                    ],
                    "metric": "spread",
                    "solve_score_quantile": 0.02,
                    "solve_score_chain": [["spread", "slv", "2"], ["spread", "cf", "2"], ["max"]],
                    "degree": 2,
                    "n_coeffs": 3,
                    "step_start": 0,
                    "step_count": 5,
                    "clip_key": "renders/test/solve_scores/clip.json",
                    "hist_bins": 4,
                    "out_key": "renders/test/solve_scores/chunk_0_hist.json",
                    "solve_score_threads": 4,
                })
        body = json.loads(result["body"])
        assert body["logical_section"] is True
        cmd = mock_run.call_args.args[0]
        assert cmd[0] == hsp.BINARY
        assert f"--score_coeffs_file={tmp_coeff}" in cmd
        done_kwargs = hsp.report_status.call_args_list[-1].kwargs
        assert done_kwargs["result_data"]["source_size"] == 80
        assert done_kwargs["result_data"]["source_coeffs_size"] == 120
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_clip_v2_param_source_downloads_lores_params_and_persists_metadata():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if key == "renders/test/lores.bin":
            return {"Body": _ChunkBody(b"\x00" * 64)}
        if key == "renders/test/lores_params.bin":
            return {"Body": _ChunkBody(b"\x22" * 64)}
        raise AssertionError(f"unexpected get_object key: {key}")

    clip_stdout = [
        json.dumps({"clip_lo": 0.0, "clip_hi": 2.0, "min_score": 0.0, "max_score": 2.0, "n_solves": 4, "threads": 1}),
        json.dumps({"clip_lo": -2.0, "clip_hi": 3.0, "min_score": -2.0, "max_score": 3.0, "n_solves": 4, "threads": 1}),
    ]

    def mock_run(cmd, capture_output, text, timeout):
        return mock.MagicMock(returncode=0, stdout=clip_stdout.pop(0), stderr="")

    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()
    try:
        result = hsp.handle_clip({
            "job_id": "test",
            "task_id": "clip_test",
            "metric": "t1_abs",
            "solve_score_quantile": 0.001,
            "solve_score_chain": [["t1_abs", "pm", "1"], ["spread", "slv", "2"], ["avg"]],
            "degree": 4,
            "lores_bin_key": "renders/test/lores.bin",
            "lores_params_key": "renders/test/lores_params.bin",
            "out_key": "renders/test/solve_scores/clip.json",
        })
        body = json.loads(result["body"])
        assert body["metric_count"] == 2
        written = json.loads(mock_s3.put_object.call_args.kwargs["Body"])
        assert written["version"] == 2
        assert written["program"] == "m0;m1;avg"
        assert written["lores_params_key"] == "renders/test/lores_params.bin"
        assert [m["source"] for m in written["metrics"]] == ["pm", "slv"]
        done_kwargs = hsp.report_status.call_args_list[-1].kwargs
        assert done_kwargs["result_data"]["source_params_key"] == "renders/test/lores_params.bin"
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_hist_v2_param_source_sectioned_passes_param_file_cli_flags():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if key == "renders/test/solve_scores/clip.json":
            clip_data = {
                "family": "solve_score",
                "version": 2,
                "metric": "t1_abs",
                "clip_quantile": 0.02,
                "omega": 1.0,
                "omega_enabled": False,
                "clip_lo": 0.0,
                "clip_hi": 1.0,
                "program": "m0;m1;max",
                "metrics": [
                    {"slot": 0, "metric": "t1_abs", "source": "pm", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 1, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
            }
            return {"Body": _ChunkBody(json.dumps(clip_data).encode())}
        if key == "renders/test/params.bin":
            assert kwargs["Range"] == "bytes=160-223"
            return {"Body": _ChunkBody(b"\x33" * 64)}
        raise AssertionError(f"unexpected get_object key: {key}")

    mock_s3.get_object = mock_get
    mock_s3.generate_presigned_url.side_effect = [
        "https://example.com/chunk.bin",
    ]
    mock_s3.put_object = mock.MagicMock()
    mock_s3.exceptions = type('Exc', (), {'NoSuchKey': type('NoSuchKey', (Exception,), {})})()

    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=0,
        stdout=json.dumps({
            "threads": 4,
            "n_solves": 4,
            "download_ms": 123,
            "compute_ms": 45,
            "wall_ms": 101,
            "hist": [1, 1, 1, 1],
        }),
        stderr="",
    ))
    orig_s3, orig_report, orig_run = hsp.s3, hsp.report_status, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_hist({
            "job_id": "test",
            "task_id": "hist_test",
            "chunk_idx": 0,
            "metric": "t1_abs",
            "solve_score_quantile": 0.02,
            "solve_score_chain": [["t1_abs", "pm", "2"], ["spread", "2"], ["max"]],
            "bin_key": "renders/test/chunk_0.bin",
            "params_key": "renders/test/params.bin",
            "step_start": 10,
            "step_count": 4,
            "bin_size": 64,
            "degree": 4,
            "clip_key": "renders/test/solve_scores/clip.json",
            "hist_bins": 4,
            "out_key": "renders/test/solve_scores/chunk_0_hist.json",
            "solve_score_hist_input_mode": "sectioned",
            "solve_score_threads": 4,
        })
        body = json.loads(result["body"])
        assert body["input_mode"] == "sectioned"
        cmd = mock_run.call_args.args[0]
        assert "--score_sources=pm,slv" in cmd
        assert "--score_program=m0;m1;max" in cmd
        assert f"--score_params_file={hsp._TMP_PARAM_INPUT}" in cmd
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report
        hsp.subprocess.run = orig_run


def test_merge_uniform_histogram():
    """Uniform histogram -> evenly spaced cuts."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 5, metric="proximity")
    body, artifact = _run_merge(5, clip_data, hist_responses, metric="proximity")
    cuts = body["cuts_norm"]
    assert len(cuts) == 9, f"expected 9 cuts, got {len(cuts)}"
    for i in range(9):
        assert abs(cuts[i] - (i + 1) / 10) < 0.02, f"cut[{i}]={cuts[i]}, expected ~{(i+1)/10}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8)), f"not monotonic: {cuts}"
    assert body["n_solves_total"] == 10000


def test_merge_skewed_histogram():
    """Skewed histogram -> concentrated cuts."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist = [900] * 10 + [11] * 90
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": hist, "n_solves": 9990,
        }
    }
    body, _ = _run_merge(1, clip_data, hist_responses, metric="proximity")
    cuts = body["cuts_norm"]
    assert len(cuts) == 9
    low_cuts = sum(1 for c in cuts if c < 0.15)
    assert low_cuts >= 7, f"expected most cuts < 0.15, got {low_cuts}: {[round(c,3) for c in cuts]}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8))


def test_merge_single_bin_histogram():
    """All data in one bin -> degenerate but valid cuts."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist = [0] * 100
    hist[50] = 10000
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": hist, "n_solves": 10000,
        }
    }
    body, _ = _run_merge(1, clip_data, hist_responses, metric="proximity")
    cuts = body["cuts_norm"]
    assert len(cuts) == 9
    assert all(0 <= c <= 1 for c in cuts), f"cuts out of range: {cuts}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8))


def test_merge_error_missing_chunk():
    """Missing chunk histogram should raise."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(2, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised"
    except RuntimeError as e:
        assert "Missing histogram" in str(e), f"wrong error: {e}"


def test_merge_uses_chunk_artifacts_when_present():
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    body, artifact = _run_merge(1, clip_data, hist_responses, metric="proximity")
    assert body["n_solves_total"] == 1000
    assert artifact["n_solves_total"] == 1000


def test_merge_does_not_fallback_to_legacy_stripe_artifacts():
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised on missing chunk histogram"
    except RuntimeError as e:
        assert "Missing histogram: renders/test/solve_scores/chunk_0_hist.json" in str(e), f"wrong error: {e}"


def test_merge_reports_configured_worker_count():
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 2, metric="proximity")
    body, _ = _run_merge(
        2,
        clip_data,
        hist_responses,
        metric="proximity",
        solve_score_merge_workers=8,
    )
    assert body["threads"] == 2
    assert body["workers"] == 2


# ================================================================
# Metric-aware merge tests (spec 9.2)
# ================================================================

def test_merge_preserves_requested_metric():
    """Merge output contains the requested metric name."""
    for metric in ["proximity", "crowding", "spread", "anisotropy", "area", "min_mod", "min_angular_separation"]:
        clip_data, hist_responses = _uniform_hist_data(
            "renders/test/solve_scores/", 1, metric=metric)
        body, artifact = _run_merge(1, clip_data, hist_responses, metric=metric)
        assert artifact["metric"] == metric, \
            f"expected metric={metric}, got {artifact.get('metric')}"


def test_merge_rejects_clip_wrong_metric():
    """Merge with clip artifact having wrong metric raises."""
    clip_data = {
        "family": "solve_score", "metric": "crowding",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised on clip metric mismatch"
    except RuntimeError as e:
        assert "mismatch" in str(e).lower(), f"wrong error: {e}"


def test_merge_rejects_chunk_wrong_metric():
    """Merge with chunk histogram having wrong metric raises."""
    clip_data = {
        "family": "solve_score", "metric": "spread",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "crowding",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="spread")
        assert False, "should have raised on chunk metric mismatch"
    except RuntimeError as e:
        assert "mismatch" in str(e).lower(), f"wrong error: {e}"


def test_merge_preserves_requested_omega():
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 1, metric="anisotropy")
    clip_data["omega"] = 6.0
    hist_responses["renders/test/solve_scores/chunk_0_hist.json"]["omega"] = 6.0
    _, artifact = _run_merge(1, clip_data, hist_responses, metric="anisotropy", solve_score_omega=6.0)
    assert artifact["omega"] == 6.0


def test_merge_rejects_hist_wrong_omega():
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 1, metric="proximity")
    clip_data["omega"] = 4.0
    hist_responses["renders/test/solve_scores/chunk_0_hist.json"]["omega"] = 5.0
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity", solve_score_omega=4.0)
        assert False, "should have raised on omega mismatch"
    except RuntimeError as e:
        assert "omega mismatch" in str(e).lower(), f"wrong error: {e}"


def test_merge_artifact_has_solve_score_family():
    """Merge output artifact has family == 'solve_score'."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 2, metric="area")
    _, artifact = _run_merge(2, clip_data, hist_responses, metric="area")
    assert artifact["family"] == "solve_score", f"family={artifact.get('family')}"


def test_merge_artifact_cuts_norm_length_9():
    """Merge output artifact has exactly 9 cut values (10 bins → 9 cuts)."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 3, metric="anisotropy")
    _, artifact = _run_merge(3, clip_data, hist_responses, metric="anisotropy")
    assert len(artifact["cuts_norm"]) == 9, \
        f"expected 9 cuts, got {len(artifact['cuts_norm'])}"


def test_merge_new_metric_clusteriness():
    """Merge works for a v2 metric (clusteriness)."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 2, metric="clusteriness")
    body, artifact = _run_merge(2, clip_data, hist_responses, metric="clusteriness")
    assert artifact["metric"] == "clusteriness"
    assert artifact["family"] == "solve_score"
    assert len(artifact["cuts_norm"]) == 9


def test_merge_rejects_clip_mismatch_new_metric():
    """Merge rejects clip artifact with wrong metric (new metric name)."""
    clip_data = {
        "family": "solve_score", "metric": "shelliness",
        "clip_quantile": 0.001,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "real_axis_proximity",
            "clip_quantile": 0.001,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="real_axis_proximity")
        assert False, "should have raised on clip metric mismatch"
    except RuntimeError as e:
        assert "mismatch" in str(e).lower(), f"wrong error: {e}"


def test_merge_new_metric_artifact_family():
    """Merge output for nn_variation still has family == 'solve_score'."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 1, metric="nn_variation")
    _, artifact = _run_merge(1, clip_data, hist_responses, metric="nn_variation")
    assert artifact["family"] == "solve_score"
    assert artifact["metric"] == "nn_variation"


def test_merge_new_metric_centroid_dist():
    """Merge works for a v3 metric (centroid_dist)."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 2, metric="centroid_dist")
    _, artifact = _run_merge(2, clip_data, hist_responses, metric="centroid_dist")
    assert artifact["family"] == "solve_score"
    assert artifact["metric"] == "centroid_dist"


# ================================================================
# Quantile tests (spec 7.3)
# ================================================================

def test_merge_artifact_stores_clip_quantile():
    """Merge output stores clip_quantile from request."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 1, metric="proximity", clip_quantile=0.01)
    _, artifact = _run_merge(1, clip_data, hist_responses, metric="proximity",
                             solve_score_quantile=0.01)
    assert artifact["clip_quantile"] == 0.01, f"clip_quantile={artifact.get('clip_quantile')}"


def test_merge_rejects_clip_quantile_mismatch():
    """Merge rejects clip artifact with wrong clip_quantile."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.05,  # clip says 5%
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.01,  # hist says 1%
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    # merge with q=0.01 — clip artifact has 0.05 → mismatch
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised on quantile mismatch"
    except RuntimeError as e:
        assert "quantile" in str(e).lower(), f"wrong error: {e}"


def test_merge_rejects_chunk_quantile_mismatch():
    """Merge rejects chunk artifact with wrong clip_quantile."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/chunk_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.05,  # different from merge request
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised on chunk quantile mismatch"
    except RuntimeError as e:
        assert "quantile" in str(e).lower(), f"wrong error: {e}"


# ================================================================
# Summary phase tests
# ================================================================

def test_summary_validates_metric():
    """Summary rejects invalid metric."""
    import handler_solve_proximity as hsp
    try:
        hsp.handle_summary({"degree": 5, "metric": "bogus",
                            "lores_bin_key": "x", "solve_score_quantile": 0.001})
        assert False, "should have raised"
    except RuntimeError as e:
        assert "Invalid metric" in str(e)


def test_summary_validates_quantile():
    """Summary rejects out-of-range quantile."""
    import handler_solve_proximity as hsp
    try:
        hsp.handle_summary({"degree": 5, "metric": "proximity",
                            "lores_bin_key": "x", "solve_score_quantile": 0.5})
        assert False, "should have raised"
    except RuntimeError as e:
        assert "0.001" in str(e) or "0.05" in str(e)


def test_summary_handler_returns_structured_500_with_program_details():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()
    mock_s3.get_object = mock.MagicMock(return_value={"Body": _ChunkBody(b"\x00" * 64)})
    mock_run = mock.MagicMock(return_value=mock.MagicMock(
        returncode=1,
        stdout="",
        stderr="Invalid score program: invalid metric slot token 'mul'",
    ))
    orig_s3, orig_run = hsp.s3, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handler({
            "body": {
                "phase": "summary",
                "job_id": "job123",
                "degree": 4,
                "metric": "centroid_im",
                "solve_score_quantile": 0.05,
                "solve_score_chain": [["centroid_im", "5"], ["anisotropy", "5"], ["mul"]],
                "lores_bin_key": "renders/test/lores.bin",
            }
        }, None)
        assert result["statusCode"] == 500
        body = json.loads(result["body"])
        assert body["error"] == "solve histogram summary failed"
        assert body["phase"] == "summary"
        assert body["metric"] == "centroid_im"
        assert body["metric_count"] == 2
        assert body["program"] == "m0;m1;mul"
        assert body["score_metrics"] == ["centroid_im", "anisotropy"]
        assert body["score_sources"] == ["slv", "slv"]
        assert "mul" in body["solve_score_display"]
        assert body["solve_score_chain"] == [["centroid_im", "5"], ["anisotropy", "5"], "mul"]
        assert "invalid metric slot token 'mul'" in body["detail"]
    finally:
        hsp.s3 = orig_s3
        hsp.subprocess.run = orig_run


def test_summary_mixed_source_requires_n_coeffs_metadata():
    import handler_solve_proximity as hsp

    try:
        hsp.handle_summary({
            "degree": 5,
            "metric": "spread",
            "solve_score_quantile": 0.001,
            "solve_score_chain": [["spread", "slv", "1"], ["spread", "cf", "1"], ["avg"]],
            "lores_bin_key": "renders/test/lores.bin",
        })
        assert False, "should have raised"
    except RuntimeError as e:
        assert "n_coeffs" in str(e)


def test_summary_mixed_source_forwards_coeff_cli_args():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if key == "renders/test/lores.bin":
            return {"Body": _ChunkBody(b"\x00" * 64)}
        if key == "renders/test/lores_coeffs.bin":
            return {"Body": _ChunkBody(b"\x00" * 80)}
        raise AssertionError(f"unexpected get_object key: {key}")

    mock_s3.get_object = mock_get
    clip_stdout = [
        json.dumps({"clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0, "n_solves": 4, "threads": 1}),
        json.dumps({"clip_lo": -2.0, "clip_hi": 3.0, "min_score": -2.0, "max_score": 3.0, "n_solves": 4, "threads": 1}),
    ]
    summary_stdout = json.dumps({"n_solves": 4, "degree": 4, "threads": 1})

    calls = []

    def mock_run(cmd, capture_output, text, timeout):
        calls.append(list(cmd))
        if "--mode=clip" in cmd:
            return mock.MagicMock(returncode=0, stdout=clip_stdout.pop(0), stderr="")
        if "--mode=summary" in cmd:
            return mock.MagicMock(returncode=0, stdout=summary_stdout, stderr="")
        raise AssertionError(f"unexpected subprocess command: {cmd}")

    orig_s3, orig_run = hsp.s3, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_summary({
            "degree": 4,
            "n_coeffs": 5,
            "metric": "spread",
            "solve_score_quantile": 0.001,
            "solve_score_chain": [["spread", "slv", "1"], ["spread", "cf", "1"], ["avg"]],
            "lores_bin_key": "renders/test/lores.bin",
            "lores_coeffs_key": "renders/test/lores_coeffs.bin",
        })
        body = json.loads(result["body"])
        assert body["score_sources"] == ["slv", "cf"]
        assert body["lores_coeffs_key"] == "renders/test/lores_coeffs.bin"
        assert body["n_coeffs"] == 5
        summary_cmd = next(cmd for cmd in calls if "--mode=summary" in cmd)
        assert "--score_sources=slv,cf" in summary_cmd
        assert "--score_coeffs_file=/tmp/solve_prox_coeff_input.bin" in summary_cmd
        assert "--score_coeff_degree=5" in summary_cmd
    finally:
        hsp.s3 = orig_s3
        hsp.subprocess.run = orig_run


def test_summary_param_source_forwards_params_cli_args():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if key == "renders/test/lores.bin":
            return {"Body": _ChunkBody(b"\x00" * 64)}
        if key == "renders/test/lores_params.bin":
            return {"Body": _ChunkBody(b"\x00" * 64)}
        raise AssertionError(f"unexpected get_object key: {key}")

    mock_s3.get_object = mock_get
    clip_stdout = [
        json.dumps({"clip_lo": 0.0, "clip_hi": 2.0, "min_score": 0.0, "max_score": 2.0, "n_solves": 4, "threads": 1}),
        json.dumps({"clip_lo": -2.0, "clip_hi": 3.0, "min_score": -2.0, "max_score": 3.0, "n_solves": 4, "threads": 1}),
    ]
    summary_stdout = json.dumps({"n_solves": 4, "degree": 4, "threads": 1})

    calls = []

    def mock_run(cmd, capture_output, text, timeout):
        calls.append(list(cmd))
        if "--mode=clip" in cmd:
            return mock.MagicMock(returncode=0, stdout=clip_stdout.pop(0), stderr="")
        if "--mode=summary" in cmd:
            return mock.MagicMock(returncode=0, stdout=summary_stdout, stderr="")
        raise AssertionError(f"unexpected subprocess command: {cmd}")

    orig_s3, orig_run = hsp.s3, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_summary({
            "degree": 4,
            "metric": "t1_abs",
            "solve_score_quantile": 0.001,
            "solve_score_chain": [["t1_abs", "pm", "1"], ["spread", "1"], ["avg"]],
            "lores_bin_key": "renders/test/lores.bin",
            "lores_params_key": "renders/test/lores_params.bin",
        })
        body = json.loads(result["body"])
        assert body["score_sources"] == ["pm", "slv"]
        assert body["lores_params_key"] == "renders/test/lores_params.bin"
        summary_cmd = next(cmd for cmd in calls if "--mode=summary" in cmd)
        assert "--score_sources=pm,slv" in summary_cmd
        assert f"--score_params_file={hsp._TMP_PARAM_INPUT}" in summary_cmd
    finally:
        hsp.s3 = orig_s3
        hsp.subprocess.run = orig_run


def test_summary_mixed_source_falls_back_to_canonical_coeff_key():
    import handler_solve_proximity as hsp

    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if key == "renders/test/lores.bin":
            return {"Body": _ChunkBody(b"\x00" * 64)}
        if key == "renders/test/lores_coeffs.bin":
            return {"Body": _ChunkBody(b"\x00" * 80)}
        raise AssertionError(f"unexpected get_object key: {key}")

    mock_s3.get_object = mock_get
    clip_stdout = [
        json.dumps({"clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0, "n_solves": 4, "threads": 1}),
        json.dumps({"clip_lo": -2.0, "clip_hi": 3.0, "min_score": -2.0, "max_score": 3.0, "n_solves": 4, "threads": 1}),
    ]
    summary_stdout = json.dumps({"n_solves": 4, "degree": 4, "threads": 1})

    calls = []

    def mock_run(cmd, capture_output, text, timeout):
        calls.append(list(cmd))
        if "--mode=clip" in cmd:
            return mock.MagicMock(returncode=0, stdout=clip_stdout.pop(0), stderr="")
        if "--mode=summary" in cmd:
            return mock.MagicMock(returncode=0, stdout=summary_stdout, stderr="")
        raise AssertionError(f"unexpected subprocess command: {cmd}")

    orig_s3, orig_run = hsp.s3, hsp.subprocess.run
    hsp.s3 = mock_s3
    hsp.subprocess.run = mock_run
    try:
        result = hsp.handle_summary({
            "job_id": "test",
            "degree": 4,
            "n_coeffs": 5,
            "metric": "spread",
            "solve_score_quantile": 0.001,
            "solve_score_chain": [["spread", "slv", "1"], ["spread", "cf", "1"], ["avg"]],
            "lores_bin_key": "renders/test/lores.bin",
        })
        body = json.loads(result["body"])
        assert body["lores_coeffs_key"] == "renders/test/lores_coeffs.bin"
        summary_cmd = next(cmd for cmd in calls if "--mode=summary" in cmd)
        assert "--score_coeffs_file=/tmp/solve_prox_coeff_input.bin" in summary_cmd
        assert "--score_coeff_degree=5" in summary_cmd
    finally:
        hsp.s3 = orig_s3
        hsp.subprocess.run = orig_run


def test_summary_does_not_report_status():
    """Summary must not call report_status."""
    import handler_solve_proximity as hsp
    source = open(hsp.__file__).read()
    # Find handle_summary function body
    idx = source.index("def handle_summary")
    body = source[idx:]
    # It should not contain report_status
    assert "report_status(" not in body.split("def ")[0] if "def " in body[20:] else body, \
        "handle_summary must not call report_status"


if __name__ == "__main__":
    tests = [
        ("uniform histogram", test_merge_uniform_histogram),
        ("skewed histogram", test_merge_skewed_histogram),
        ("single bin histogram", test_merge_single_bin_histogram),
        ("missing chunk error", test_merge_error_missing_chunk),
        ("preserves requested metric", test_merge_preserves_requested_metric),
        ("rejects clip wrong metric", test_merge_rejects_clip_wrong_metric),
        ("rejects chunk wrong metric", test_merge_rejects_chunk_wrong_metric),
        ("artifact has solve_score family", test_merge_artifact_has_solve_score_family),
        ("artifact cuts_norm length 9", test_merge_artifact_cuts_norm_length_9),
        ("new metric clusteriness", test_merge_new_metric_clusteriness),
        ("rejects clip mismatch new metric", test_merge_rejects_clip_mismatch_new_metric),
        ("new metric artifact family", test_merge_new_metric_artifact_family),
        ("artifact stores clip_quantile", test_merge_artifact_stores_clip_quantile),
        ("rejects clip quantile mismatch", test_merge_rejects_clip_quantile_mismatch),
        ("rejects chunk quantile mismatch", test_merge_rejects_chunk_quantile_mismatch),
        ("summary validates metric", test_summary_validates_metric),
        ("summary validates quantile", test_summary_validates_quantile),
        ("summary does not report_status", test_summary_does_not_report_status),
    ]

    print("solve_proximity handler tests (metric-aware)")
    print("=" * 50)

    passed = 0
    failed = 0
    for name, fn in tests:
        try:
            fn()
            passed += 1
            print(f"  PASS: {name}")
        except AssertionError as e:
            failed += 1
            print(f"  FAIL: {name} — {e}")
        except Exception as e:
            failed += 1
            print(f"  ERROR: {name} — {type(e).__name__}: {e}")

    print(f"\n{'=' * 50}")
    print(f"{passed}/{passed + failed} passed")
    if failed:
        sys.exit(1)
    else:
        print("ALL PASS")
