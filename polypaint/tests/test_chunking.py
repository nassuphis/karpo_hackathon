"""
Tests for the chunked coefficient pipeline:
  param_gen → coeffgen_chunked → solve

Verifies:
  - param_gen output size and determinism
  - coeffgen_chunked reads correct slice
  - chunked output matches monolithic coeffgen (within float32 quantization)
  - chunk planner: no gaps, no overlaps, full coverage
  - chunks > N works (the original motivation)

Run: cd polypaint && uv run python -m pytest tests/test_chunking.py -v
"""
import json
import os
import struct
import subprocess

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def run_mode(spec, out_path):
    r = subprocess.run([SWEEP, out_path], input=json.dumps(spec),
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"Failed: {r.stderr}"
    return json.loads(r.stdout)


def test_param_gen_size():
    """param_gen output is exactly N*N*times*16 bytes."""
    print("test_param_gen_size...")
    for n, times in [(10, 1), (5, 10), (20, 3)]:
        meta = run_mode({
            "mode": "param_gen", "n1": n, "n2": n, "times": times,
            "param_transforms": [],
        }, "/tmp/pg_size.bin")
        expected = n * n * times
        assert meta["n_steps"] == expected, f"{n},{times}: n_steps={meta['n_steps']} != {expected}"
        assert meta["data_bytes"] == expected * 16
        assert os.path.getsize("/tmp/pg_size.bin") == expected * 16
        os.remove("/tmp/pg_size.bin")
    print("  PASS")


def test_param_gen_deterministic():
    """Same inputs produce identical params.bin."""
    print("test_param_gen_deterministic...")
    spec = {"mode": "param_gen", "n1": 10, "n2": 10, "times": 3,
            "param_transforms": [["unit_circle"], ["ddith", "2", "1", "0.5"]]}
    run_mode(spec, "/tmp/pg_det1.bin")
    run_mode(spec, "/tmp/pg_det2.bin")
    with open("/tmp/pg_det1.bin", "rb") as f: d1 = f.read()
    with open("/tmp/pg_det2.bin", "rb") as f: d2 = f.read()
    assert d1 == d2, "param_gen not deterministic"
    os.remove("/tmp/pg_det1.bin")
    os.remove("/tmp/pg_det2.bin")
    print("  PASS")


def test_coeffgen_chunked_matches_monolithic():
    """Chunked coeffgen output matches monolithic (within float32 quantization)."""
    print("test_coeffgen_chunked_matches_monolithic...")
    n, times, func = 10, 2, "g39"
    ct = ["rev"]
    pt = [["unit_circle"]]

    # Old monolithic
    run_mode({
        "mode": "coeffgen", "function": func, "n1": n, "n2": n,
        "i1_start": 0, "i1_end": n,
        "param_transforms": pt, "coeff_transforms": ct, "times": times,
    }, "/tmp/ck_old.bin")

    # New chunked: param_gen then 3 chunks
    pg = run_mode({
        "mode": "param_gen", "n1": n, "n2": n, "times": times,
        "param_transforms": pt,
    }, "/tmp/ck_params.bin")
    n_steps = pg["n_steps"]

    chunks = []
    chunk_size = n_steps // 3
    for i in range(3):
        ss = i * chunk_size
        sc = chunk_size if i < 2 else n_steps - ss
        run_mode({
            "mode": "coeffgen_chunked", "function": func,
            "params_file": "/tmp/ck_params.bin",
            "step_start": ss, "step_count": sc,
            "coeff_transforms": ct,
        }, f"/tmp/ck_chunk{i}.bin")
        with open(f"/tmp/ck_chunk{i}.bin", "rb") as f:
            chunks.append(f.read())
        os.remove(f"/tmp/ck_chunk{i}.bin")

    new_all = b"".join(chunks)
    with open("/tmp/ck_old.bin", "rb") as f:
        old_all = f.read()

    assert len(old_all) == len(new_all), f"Size: {len(old_all)} vs {len(new_all)}"

    old_vals = struct.unpack(f"<{len(old_all)//4}f", old_all)
    new_vals = struct.unpack(f"<{len(new_all)//4}f", new_all)
    max_err = max(abs(a - b) for a, b in zip(old_vals, new_vals))
    # float32 quantization: params stored as float32, read back, used in coeff func
    assert max_err < 1e-3, f"Max error too large: {max_err:.2e}"
    print(f"  max error: {max_err:.2e} (float32 quantization expected)")

    for f in ["/tmp/ck_old.bin", "/tmp/ck_params.bin"]:
        os.remove(f)
    print("  PASS")


def test_coeffgen_chunked_parametric_coeff_transform_matches_monolithic():
    """Chunked coeffgen matches monolithic for parametric coeff transforms too."""
    print("test_coeffgen_chunked_parametric_coeff_transform_matches_monolithic...")
    n, times, func = 8, 1, "g39"
    ct = [["roots", "6"]]
    pt = [["unit_circle"]]

    run_mode({
        "mode": "coeffgen", "function": func, "n1": n, "n2": n,
        "i1_start": 0, "i1_end": n,
        "param_transforms": pt, "coeff_transforms": ct, "times": times,
    }, "/tmp/ck_roots_old.bin")

    pg = run_mode({
        "mode": "param_gen", "n1": n, "n2": n, "times": times,
        "param_transforms": pt,
    }, "/tmp/ck_roots_params.bin")
    n_steps = pg["n_steps"]

    chunks = []
    chunk_size = n_steps // 2
    for i in range(2):
        ss = i * chunk_size
        sc = chunk_size if i < 1 else n_steps - ss
        run_mode({
            "mode": "coeffgen_chunked", "function": func,
            "params_file": "/tmp/ck_roots_params.bin",
            "step_start": ss, "step_count": sc,
            "coeff_transforms": ct,
        }, f"/tmp/ck_roots_chunk{i}.bin")
        with open(f"/tmp/ck_roots_chunk{i}.bin", "rb") as f:
            chunks.append(f.read())
        os.remove(f"/tmp/ck_roots_chunk{i}.bin")

    new_all = b"".join(chunks)
    with open("/tmp/ck_roots_old.bin", "rb") as f:
        old_all = f.read()

    assert len(old_all) == len(new_all), f"Size: {len(old_all)} vs {len(new_all)}"
    old_vals = struct.unpack(f"<{len(old_all)//4}f", old_all)
    new_vals = struct.unpack(f"<{len(new_all)//4}f", new_all)
    max_err = max(abs(a - b) for a, b in zip(old_vals, new_vals))
    assert max_err < 1e-3, f"Max error too large: {max_err:.2e}"

    for f in ["/tmp/ck_roots_old.bin", "/tmp/ck_roots_params.bin"]:
        os.remove(f)
    print("  PASS")


def test_chunk_planner_coverage():
    """Chunk planner: no gaps, no overlaps, full coverage."""
    print("test_chunk_planner_coverage...")
    import math
    for n_steps, n_chunks in [(200, 3), (100, 100), (7, 10), (1000, 13)]:
        chunk_size = math.ceil(n_steps / n_chunks)
        ranges = []
        for c in range(n_chunks):
            ss = c * chunk_size
            sc = min(chunk_size, n_steps - ss)
            if sc <= 0:
                break
            ranges.append((ss, sc))

        # Full coverage
        total = sum(sc for _, sc in ranges)
        assert total == n_steps, f"n_steps={n_steps}, chunks={n_chunks}: total={total}"

        # No gaps, no overlaps
        for i in range(1, len(ranges)):
            prev_end = ranges[i-1][0] + ranges[i-1][1]
            curr_start = ranges[i][0]
            assert prev_end == curr_start, f"Gap/overlap at chunk {i}: {prev_end} vs {curr_start}"
    print("  PASS")


def test_chunks_greater_than_n():
    """chunks > N works (the original motivation for chunking)."""
    print("test_chunks_greater_than_n...")
    n, times = 5, 100  # N=5, times=100 → 2500 steps
    n_steps = n * n * times
    n_chunks = 50  # 50 chunks > N=5

    pg = run_mode({
        "mode": "param_gen", "n1": n, "n2": n, "times": times,
        "param_transforms": [],
    }, "/tmp/ck_big_params.bin")
    assert pg["n_steps"] == n_steps

    import math
    chunk_size = math.ceil(n_steps / n_chunks)
    total_coeffs = 0
    for c in range(n_chunks):
        ss = c * chunk_size
        sc = min(chunk_size, n_steps - ss)
        if sc <= 0:
            break
        meta = run_mode({
            "mode": "coeffgen_chunked", "function": "g39",
            "params_file": "/tmp/ck_big_params.bin",
            "step_start": ss, "step_count": sc,
            "coeff_transforms": [],
        }, f"/tmp/ck_big_{c}.bin")
        total_coeffs += meta["n_t"]
        os.remove(f"/tmp/ck_big_{c}.bin")

    assert total_coeffs == n_steps, f"total={total_coeffs} != {n_steps}"
    os.remove("/tmp/ck_big_params.bin")
    print(f"  {n_chunks} chunks > N={n}: {n_steps} steps processed correctly")
    print("  PASS")


def test_param_gen_with_dither():
    """param_gen with dither produces different values across passes."""
    print("test_param_gen_with_dither...")
    n = 5
    meta = run_mode({
        "mode": "param_gen", "n1": n, "n2": n, "times": 2,
        "param_transforms": [["ddith", "2", "1", "0.5"]],
    }, "/tmp/pg_dith.bin")

    with open("/tmp/pg_dith.bin", "rb") as f:
        data = f.read()
    os.remove("/tmp/pg_dith.bin")

    # Pass 0 and pass 1 should differ (different RNG seeds)
    pass_size = n * n * 16
    pass0 = data[:pass_size]
    pass1 = data[pass_size:2*pass_size]
    assert pass0 != pass1, "Dither should produce different values across passes"
    print("  PASS")


if __name__ == "__main__":
    test_param_gen_size()
    test_param_gen_deterministic()
    test_coeffgen_chunked_matches_monolithic()
    test_chunk_planner_coverage()
    test_chunks_greater_than_n()
    test_param_gen_with_dither()
