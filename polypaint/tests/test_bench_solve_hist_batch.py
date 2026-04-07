import argparse
import importlib.util
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve().parents[1] / "scripts" / "bench_solve_hist_batch.py"
SPEC = importlib.util.spec_from_file_location("bench_solve_hist_batch", SCRIPT_PATH)
bench = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(bench)


def test_extract_chunk_items_uses_explicit_chunk_metadata_sorted():
    calc = {
        "chunks": [
            {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "bin_size": 300},
            {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "bin_size": 100},
            {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "bin_size": 200},
        ]
    }

    assert bench.extract_chunk_items(calc, "j") == [
        {"chunk_idx": 0, "bin_key": "renders/j/chunk_0.bin", "bin_size": 100},
        {"chunk_idx": 1, "bin_key": "renders/j/chunk_1.bin", "bin_size": 200},
        {"chunk_idx": 2, "bin_key": "renders/j/chunk_2.bin", "bin_size": 300},
    ]


def test_extract_chunk_items_falls_back_to_dense_chunk_keys():
    calc = {"n_chunks": 3}

    assert bench.extract_chunk_items(calc, "compute_test") == [
        {"chunk_idx": 0, "bin_key": "renders/compute_test/chunk_0.bin"},
        {"chunk_idx": 1, "bin_key": "renders/compute_test/chunk_1.bin"},
        {"chunk_idx": 2, "bin_key": "renders/compute_test/chunk_2.bin"},
    ]


def test_extract_chunk_items_rejects_invalid_metadata():
    calc = {"chunks": [{"idx": 0}, {"bin_key": "renders/j/chunk_1.bin"}]}

    try:
        bench.extract_chunk_items(calc, "j")
        assert False, "expected invalid chunk metadata to raise"
    except RuntimeError as e:
        assert "Invalid chunk metadata" in str(e)


def test_batch_chunk_items_groups_items_by_batch_size():
    items = [{"chunk_idx": i, "bin_key": f"k{i}"} for i in range(5)]

    assert bench.batch_chunk_items(items, 2) == [
        [{"chunk_idx": 0, "bin_key": "k0"}, {"chunk_idx": 1, "bin_key": "k1"}],
        [{"chunk_idx": 2, "bin_key": "k2"}, {"chunk_idx": 3, "bin_key": "k3"}],
        [{"chunk_idx": 4, "bin_key": "k4"}],
    ]


def test_parse_batch_sizes_sorts_and_dedupes():
    assert bench.parse_batch_sizes("8, 1,4,4,2") == [1, 2, 4, 8]


def test_parse_batch_sizes_rejects_invalid_values():
    try:
        bench.parse_batch_sizes("1,0,2")
        assert False, "expected parse_batch_sizes to reject zero"
    except argparse.ArgumentTypeError as e:
        assert ">= 1" in str(e)


def test_aggregate_results_sums_batch_totals():
    rows = [
        {
            "chunks_processed": 2,
            "bytes_downloaded": 100,
            "download_ms": 10,
            "compute_ms": 20,
            "n_solves_total": 30,
            "hist_checksum": 40,
        },
        {
            "chunks_processed": 3,
            "bytes_downloaded": 200,
            "download_ms": 11,
            "compute_ms": 21,
            "n_solves_total": 31,
            "hist_checksum": 41,
        },
    ]

    assert bench.aggregate_results(rows) == {
        "batches": 2,
        "chunks_processed": 5,
        "bytes_downloaded": 300,
        "download_ms": 21,
        "compute_ms": 41,
        "n_solves_total": 61,
        "hist_checksum": 81,
    }


def test_solve_bytes_for_degree_matches_f32_complex_layout():
    assert bench.solve_bytes_for_degree(70) == 560


def test_solves_for_bin_bytes_uses_degree_stride():
    assert bench.solves_for_bin_bytes(5600, 70) == 10
    assert bench.solves_for_bin_bytes(None, 70) is None


def test_build_progress_line_includes_core_counters():
    line = bench.build_progress_line(
        batch_size=4,
        completed_batches=2,
        total_batches=5,
        chunks_processed=8,
        total_chunks=20,
        wall_ms=12500,
        download_ms=9900,
        compute_ms=2300,
        worker_mode="subprocess",
    )

    assert "batch_size=4" in line
    assert "batches=2/5" in line
    assert "chunks=8/20" in line
    assert "wall=12.5s" in line
    assert "download=9.9s" in line
    assert "compute=2.3s" in line
