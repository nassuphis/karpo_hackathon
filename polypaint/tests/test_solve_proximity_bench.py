import json
import os
import sys
import unittest.mock as mock

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _Context:
    def __init__(self, remaining_values):
        self._values = list(remaining_values)

    def get_remaining_time_in_millis(self):
        if self._values:
            return self._values.pop(0)
        return 900000


def test_parse_concurrency_values_dedupes_and_sorts():
    import handler_solve_proximity_bench as bench

    assert bench._parse_concurrency_values("4,1,2,2") == [1, 2, 4]


def test_extract_chunk_items_uses_calc_metadata():
    import handler_solve_proximity_bench as bench

    calc = {
        "chunks": [
            {"idx": 2, "bin_key": "renders/j/stripe_2.bin", "bin_size": 300},
            {"idx": 0, "bin_key": "renders/j/stripe_0.bin", "bin_size": 100},
            {"idx": 1, "bin_key": "renders/j/stripe_1.bin", "bin_size": 200},
        ]
    }
    assert bench._extract_chunk_items(calc, "j") == [
        {"chunk_idx": 0, "bin_key": "renders/j/stripe_0.bin", "bin_size": 100},
        {"chunk_idx": 1, "bin_key": "renders/j/stripe_1.bin", "bin_size": 200},
        {"chunk_idx": 2, "bin_key": "renders/j/stripe_2.bin", "bin_size": 300},
    ]


def test_handler_returns_benchmark_summary():
    import handler_solve_proximity_bench as bench

    calc = {
        "degree": 50,
        "lores": {"bin_key": "renders/test/lores.bin"},
        "chunks": [
            {"idx": 0, "bin_key": "renders/test/stripe_0.bin", "bin_size": 64000000},
            {"idx": 1, "bin_key": "renders/test/stripe_1.bin", "bin_size": 64000000},
        ],
    }
    orig_load, orig_download, orig_clip, orig_run = (
        bench._load_calc,
        bench._download_to_file,
        bench._run_clip,
        bench._run_concurrency_benchmark,
    )
    bench._load_calc = mock.MagicMock(return_value=calc)
    bench._download_to_file = mock.MagicMock(return_value=2048)
    bench._run_clip = mock.MagicMock(return_value={"clip_lo": -1.0, "clip_hi": 2.0, "n_solves": 25})
    bench._run_concurrency_benchmark = mock.MagicMock(side_effect=[
        {
            "concurrency": 1,
            "chunk_count": 2,
            "wall_ms": 1000,
            "download_ms": 900,
            "compute_ms": 20,
            "bytes_downloaded": 128000000,
            "n_solves_total": 320000,
            "hist_checksum": 111,
            "avg_wall_ms_per_chunk": 500.0,
            "avg_download_ms_per_chunk": 450.0,
            "avg_compute_ms_per_chunk": 10.0,
        },
        {
            "concurrency": 2,
            "chunk_count": 2,
            "wall_ms": 700,
            "download_ms": 1200,
            "compute_ms": 20,
            "bytes_downloaded": 128000000,
            "n_solves_total": 320000,
            "hist_checksum": 111,
            "avg_wall_ms_per_chunk": 350.0,
            "avg_download_ms_per_chunk": 600.0,
            "avg_compute_ms_per_chunk": 10.0,
        },
    ])
    try:
        result = bench.handler(
            {"body": json.dumps({
                "job_id": "test",
                "metric": "centroid_re",
                "concurrency_values": [1, 2],
                "max_chunks": 2,
            })},
            _Context([900000, 900000]),
        )
        body = json.loads(result["body"])
        assert body["mode"] == "solve_hist_benchmark"
        assert body["job_id"] == "test"
        assert body["degree"] == 50
        assert body["concurrency_values"] == [1, 2]
        assert body["chunk_selection"]["selected_chunk_indices"] == [0, 1]
        assert body["clip"]["lores_size"] == 2048
        assert [row["concurrency"] for row in body["results"]] == [1, 2]
        assert bench._run_concurrency_benchmark.call_count == 2
    finally:
        bench._load_calc = orig_load
        bench._download_to_file = orig_download
        bench._run_clip = orig_clip
        bench._run_concurrency_benchmark = orig_run


def test_handler_stops_early_when_time_is_low():
    import handler_solve_proximity_bench as bench

    calc = {
        "degree": 50,
        "lores": {"bin_key": "renders/test/lores.bin"},
        "chunks": [
            {"idx": 0, "bin_key": "renders/test/stripe_0.bin", "bin_size": 64000000},
        ],
    }
    orig_load, orig_download, orig_clip, orig_run = (
        bench._load_calc,
        bench._download_to_file,
        bench._run_clip,
        bench._run_concurrency_benchmark,
    )
    bench._load_calc = mock.MagicMock(return_value=calc)
    bench._download_to_file = mock.MagicMock(return_value=1024)
    bench._run_clip = mock.MagicMock(return_value={"clip_lo": 0.0, "clip_hi": 1.0, "n_solves": 9})
    bench._run_concurrency_benchmark = mock.MagicMock(return_value={
        "concurrency": 1,
        "chunk_count": 1,
        "wall_ms": 100,
        "download_ms": 90,
        "compute_ms": 1,
        "bytes_downloaded": 64000000,
        "n_solves_total": 160000,
        "hist_checksum": 11,
        "avg_wall_ms_per_chunk": 100.0,
        "avg_download_ms_per_chunk": 90.0,
        "avg_compute_ms_per_chunk": 1.0,
    })
    try:
        result = bench.handler(
            {"body": json.dumps({
                "job_id": "test",
                "metric": "proximity",
                "concurrency_values": [1, 2, 4],
                "max_chunks": 1,
            })},
            _Context([900000, 30000]),
        )
        body = json.loads(result["body"])
        assert body["stopped_early"] is True
        assert [row["concurrency"] for row in body["results"]] == [1]
        assert bench._run_concurrency_benchmark.call_count == 1
    finally:
        bench._load_calc = orig_load
        bench._download_to_file = orig_download
        bench._run_clip = orig_clip
        bench._run_concurrency_benchmark = orig_run
