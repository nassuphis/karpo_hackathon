"""
Tests for solve_proximity_stats binary.

Validates the algorithmic contract using the Docker runtime (deploy binary):
- clip mode: score computation, sort, quantile clip bounds, fallback rules
- hist mode: histogram bin counts with known clip bounds
- degenerate cases: identical roots, single solve

Run: cd polypaint && uv run python tests/test_solve_proximity_stats.py
"""
import json
import math
import os
import struct
import subprocess
import sys

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
LAPACK_BUILD = os.path.join(LAMBDA_DIR, "layer-build-lapack")
ROOT = os.path.join(os.path.dirname(__file__), "..")


def _docker_run(cmd_inside):
    """Run a command inside ARM64 Docker with /src mounted to lambda/."""
    r = subprocess.run(
        ["docker", "run", "--rm", "--platform", "linux/arm64",
         "-v", f"{os.path.abspath(LAMBDA_DIR)}:/src",
         "public.ecr.aws/amazonlinux/amazonlinux:2023",
         "bash", "-c", cmd_inside],
        capture_output=True, text=True, timeout=30,
    )
    return r


def write_bin(path, solves, degree):
    """Write a .bin file with float32 root pairs."""
    with open(path, "wb") as f:
        for solve in solves:
            for re, im in solve:
                f.write(struct.pack("<ff", re, im))
            for _ in range(degree - len(solve)):
                f.write(struct.pack("<ff", 0.0, 0.0))


def run_clip(bin_path, degree, **kwargs):
    """Run clip mode via Docker."""
    # Copy test bin into lambda dir so Docker can see it
    docker_bin = "/src/_test_input.bin"
    host_bin = os.path.join(LAMBDA_DIR, "_test_input.bin")
    # Copy the test file to lambda dir
    import shutil
    shutil.copy(bin_path, host_bin)
    try:
        args = f"/src/solve_proximity_stats {docker_bin} --mode=clip --degree={degree}"
        for k, v in kwargs.items():
            args += f" --{k}={v}"
        r = _docker_run(args)
        if r.returncode != 0:
            return None, r.stderr
        return json.loads(r.stdout), None
    finally:
        try:
            os.remove(host_bin)
        except OSError:
            pass


def run_hist(bin_path, degree, clip_lo, clip_hi, hist_bins=100, **kwargs):
    """Run hist mode via Docker."""
    import shutil
    host_bin = os.path.join(LAMBDA_DIR, "_test_input.bin")
    shutil.copy(bin_path, host_bin)
    try:
        args = (f"/src/solve_proximity_stats /src/_test_input.bin --mode=hist "
                f"--degree={degree} --clip_lo={clip_lo} --clip_hi={clip_hi} --hist_bins={hist_bins}")
        for k, v in kwargs.items():
            args += f" --{k}={v}"
        r = _docker_run(args)
        if r.returncode != 0:
            return None, r.stderr
        return json.loads(r.stdout), None
    finally:
        try:
            os.remove(host_bin)
        except OSError:
            pass


def expected_score(roots):
    """Python reference implementation of solve score."""
    d2_min = float('inf')
    for i in range(len(roots)):
        for j in range(i + 1, len(roots)):
            dr = roots[i][0] - roots[j][0]
            di = roots[i][1] - roots[j][1]
            d2 = dr * dr + di * di
            if d2 < d2_min:
                d2_min = d2
    return -0.5 * math.log10(max(d2_min, 1e-300))


# ---- Test fixtures ----

SOLVE_A = [(0.0, 0.0), (1.0, 0.0)]
SOLVE_B = [(0.0, 0.0), (0.01, 0.0)]
SOLVE_C = [(0.0, 0.0), (0.1, 0.0)]


def test_clip_basic():
    """Clip with 3 known solves, verify scores and bounds."""
    path = "/tmp/sp_test_clip.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_clip(path, 2)
    assert result is not None, f"clip failed: {err}"
    assert result["n_solves"] == 3
    assert result["degree"] == 2
    score_a = expected_score(SOLVE_A)
    score_b = expected_score(SOLVE_B)
    assert abs(result["min_score"] - score_a) < 0.01, f"min_score={result['min_score']}, expected ~{score_a}"
    assert abs(result["max_score"] - score_b) < 0.01, f"max_score={result['max_score']}, expected ~{score_b}"
    assert abs(result["clip_lo"] - result["min_score"]) < 1e-10
    assert abs(result["clip_hi"] - result["max_score"]) < 1e-10
    os.remove(path)


def test_clip_many_solves():
    """Clip with 200 solves to exercise quantile path."""
    path = "/tmp/sp_test_clip_many.bin"
    solves = []
    for i in range(200):
        d = 0.001 + (i / 200.0) * 2.0
        solves.append([(0.0, 0.0), (d, 0.0)])
    write_bin(path, solves, 2)
    result, err = run_clip(path, 2)
    assert result is not None, f"clip failed: {err}"
    assert result["n_solves"] == 200
    assert result["clip_lo"] < result["clip_hi"]
    assert result["clip_lo"] >= result["min_score"] - 1e-10
    assert result["clip_hi"] <= result["max_score"] + 1e-10
    os.remove(path)


def test_hist_basic():
    """Histogram with known scores and clip bounds."""
    path = "/tmp/sp_test_hist.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_hist(path, 2, clip_lo=0.0, clip_hi=2.0, hist_bins=10)
    assert result is not None, f"hist failed: {err}"
    assert result["n_solves"] == 3
    assert len(result["hist"]) == 10
    assert sum(result["hist"]) == 3
    assert result["hist"][0] >= 1, f"bin 0 should have solve_a: {result['hist']}"
    assert result["hist"][4] >= 1, f"bin 4 should have solve_c: {result['hist']}"
    assert result["hist"][9] >= 1, f"bin 9 should have solve_b: {result['hist']}"
    os.remove(path)


def test_hist_100_bins():
    """Histogram with default 100 bins."""
    path = "/tmp/sp_test_hist100.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_hist(path, 2, clip_lo=0.0, clip_hi=2.0)
    assert result is not None, f"hist failed: {err}"
    assert len(result["hist"]) == 100
    assert sum(result["hist"]) == 3
    os.remove(path)


def test_identical_roots():
    """Degenerate: all roots identical -> very high score."""
    path = "/tmp/sp_test_identical.bin"
    solve = [(1.0, 1.0), (1.0, 1.0), (1.0, 1.0)]
    write_bin(path, [solve], 3)
    result, err = run_clip(path, 3)
    assert result is not None, f"identical roots failed: {err}"
    assert result["min_score"] > 100, f"score should be very high: {result['min_score']}"
    os.remove(path)


def test_degree_3():
    """3 roots: verify correct pair selection (i<j only)."""
    path = "/tmp/sp_test_d3.bin"
    solve = [(0.0, 0.0), (1.0, 0.0), (10.0, 0.0)]
    write_bin(path, [solve], 3)
    result, err = run_clip(path, 3)
    assert result is not None, f"degree 3 failed: {err}"
    expected = expected_score(solve)
    assert abs(result["min_score"] - expected) < 0.01, f"score={result['min_score']}, expected={expected}"
    os.remove(path)


def test_empty_file():
    """Empty file should fail cleanly."""
    path = "/tmp/sp_test_empty.bin"
    with open(path, "wb") as f:
        pass
    result, err = run_clip(path, 2)
    assert result is None, "empty file should have failed"
    os.remove(path)


def test_invalid_clip_range():
    """Hist with zero-width clip range should fail."""
    path = "/tmp/sp_test_badclip.bin"
    write_bin(path, [SOLVE_A], 2)
    result, err = run_hist(path, 2, clip_lo=5.0, clip_hi=5.0)
    assert result is None, "invalid clip range should have failed"
    os.remove(path)


if __name__ == "__main__":
    # Check Docker is available
    r = subprocess.run(["docker", "info"], capture_output=True, timeout=10)
    if r.returncode != 0:
        print("ERROR: Docker not available")
        sys.exit(1)

    # Check deploy binary exists
    binary = os.path.join(LAMBDA_DIR, "solve_proximity_stats")
    if not os.path.exists(binary):
        print(f"ERROR: {binary} not found. Run deploy.sh to compile.")
        sys.exit(1)

    print("solve_proximity_stats tests (Docker ARM64)")
    print("=" * 50)

    tests = [
        ("clip basic", test_clip_basic),
        ("clip many solves", test_clip_many_solves),
        ("hist basic", test_hist_basic),
        ("hist 100 bins", test_hist_100_bins),
        ("identical roots", test_identical_roots),
        ("degree 3", test_degree_3),
        ("empty file", test_empty_file),
        ("invalid clip range", test_invalid_clip_range),
    ]

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
        print("FAIL")
        sys.exit(1)
    else:
        print("ALL PASS")
