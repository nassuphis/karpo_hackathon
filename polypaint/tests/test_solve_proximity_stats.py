"""
Tests for solve_proximity_stats binary.

Validates the algorithmic contract:
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
BINARY = os.path.join(LAMBDA_DIR, "solve_proximity_stats_test")

passed = 0
failed = 0


def check(name, condition, detail=""):
    global passed, failed
    if condition:
        passed += 1
        print(f"  PASS: {name}")
    else:
        failed += 1
        print(f"  FAIL: {name} — {detail}")


def write_bin(path, solves, degree):
    """Write a .bin file with float32 root pairs."""
    with open(path, "wb") as f:
        for solve in solves:
            for re, im in solve:
                f.write(struct.pack("<ff", re, im))
            # Pad to degree if needed
            for _ in range(degree - len(solve)):
                f.write(struct.pack("<ff", 0.0, 0.0))


def run_clip(bin_path, degree, **kwargs):
    args = [BINARY, bin_path, "--mode=clip", f"--degree={degree}"]
    for k, v in kwargs.items():
        args.append(f"--{k}={v}")
    r = subprocess.run(args, capture_output=True, text=True, timeout=10)
    if r.returncode != 0:
        return None, r.stderr
    return json.loads(r.stdout), None


def run_hist(bin_path, degree, clip_lo, clip_hi, hist_bins=100, **kwargs):
    args = [BINARY, bin_path, "--mode=hist", f"--degree={degree}",
            f"--clip_lo={clip_lo}", f"--clip_hi={clip_hi}", f"--hist_bins={hist_bins}"]
    for k, v in kwargs.items():
        args.append(f"--{k}={v}")
    r = subprocess.run(args, capture_output=True, text=True, timeout=10)
    if r.returncode != 0:
        return None, r.stderr
    return json.loads(r.stdout), None


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

# Solve A: roots at (0,0) and (1,0) → d2_min = 1.0 → score = 0.0
SOLVE_A = [(0.0, 0.0), (1.0, 0.0)]
# Solve B: roots at (0,0) and (0.01, 0) → d2_min = 0.0001 → score = 2.0
SOLVE_B = [(0.0, 0.0), (0.01, 0.0)]
# Solve C: roots at (0,0) and (0.1, 0) → d2_min = 0.01 → score = 1.0
SOLVE_C = [(0.0, 0.0), (0.1, 0.0)]


def test_clip_basic():
    """Clip with 3 known solves, verify scores and bounds."""
    path = "/tmp/sp_test_clip.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_clip(path, 2)
    check("clip basic: runs", result is not None, err)
    if not result:
        return
    check("clip basic: n_solves=3", result["n_solves"] == 3)
    check("clip basic: degree=2", result["degree"] == 2)
    # With only 3 solves (< 100), should use exact min/max
    score_a = expected_score(SOLVE_A)  # 0.0
    score_b = expected_score(SOLVE_B)  # 2.0
    score_c = expected_score(SOLVE_C)  # 1.0
    check("clip basic: min_score ~ 0.0",
          abs(result["min_score"] - score_a) < 0.01,
          f"got {result['min_score']}, expected ~{score_a}")
    check("clip basic: max_score ~ 2.0",
          abs(result["max_score"] - score_b) < 0.01,
          f"got {result['max_score']}, expected ~{score_b}")
    # clip_lo = min, clip_hi = max (n < 100 fallback)
    check("clip basic: clip_lo = min_score",
          abs(result["clip_lo"] - result["min_score"]) < 1e-10)
    check("clip basic: clip_hi = max_score",
          abs(result["clip_hi"] - result["max_score"]) < 1e-10)
    os.remove(path)


def test_clip_many_solves():
    """Clip with 200 solves to exercise quantile path."""
    path = "/tmp/sp_test_clip_many.bin"
    # Create 200 solves with varying separations
    solves = []
    for i in range(200):
        d = 0.001 + (i / 200.0) * 2.0  # separation from 0.001 to ~2.0
        solves.append([(0.0, 0.0), (d, 0.0)])
    write_bin(path, solves, 2)
    result, err = run_clip(path, 2)
    check("clip many: runs", result is not None, err)
    if not result:
        return
    check("clip many: n_solves=200", result["n_solves"] == 200)
    # With 200 solves, should use quantile path
    # clip_lo should be between min and max, not equal to either
    check("clip many: clip_lo < clip_hi",
          result["clip_lo"] < result["clip_hi"],
          f"lo={result['clip_lo']}, hi={result['clip_hi']}")
    check("clip many: clip_lo >= min_score",
          result["clip_lo"] >= result["min_score"] - 1e-10)
    check("clip many: clip_hi <= max_score",
          result["clip_hi"] <= result["max_score"] + 1e-10)
    os.remove(path)


def test_hist_basic():
    """Histogram with known scores and clip bounds."""
    path = "/tmp/sp_test_hist.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    # Use exact min/max as clip bounds: [0.0, 2.0]
    result, err = run_hist(path, 2, clip_lo=0.0, clip_hi=2.0, hist_bins=10)
    check("hist basic: runs", result is not None, err)
    if not result:
        return
    check("hist basic: n_solves=3", result["n_solves"] == 3)
    check("hist basic: hist length=10", len(result["hist"]) == 10)
    # Total count should equal n_solves
    total = sum(result["hist"])
    check("hist basic: total count=3", total == 3, f"got {total}")
    # Score A (0.0) → u=0.0 → bin 0
    # Score C (~1.0, float32 rounds to ~0.9999) → u~0.5 → bin 4 or 5
    # Score B (2.0) → u=1.0 → bin 9 (clamped)
    check("hist basic: bin 0 has solve_a", result["hist"][0] >= 1,
          f"hist={result['hist']}")
    # Score C lands in bin 4 due to float32 rounding (0.1 → 0.100000001490...)
    check("hist basic: bin 4 has solve_c", result["hist"][4] >= 1,
          f"hist={result['hist']}")
    check("hist basic: bin 9 has solve_b", result["hist"][9] >= 1,
          f"hist={result['hist']}")
    os.remove(path)


def test_hist_100_bins():
    """Histogram with default 100 bins."""
    path = "/tmp/sp_test_hist100.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_hist(path, 2, clip_lo=0.0, clip_hi=2.0)
    check("hist 100 bins: runs", result is not None, err)
    if result:
        check("hist 100 bins: length=100", len(result["hist"]) == 100)
        check("hist 100 bins: total=3", sum(result["hist"]) == 3)
    os.remove(path)


def test_identical_roots():
    """Degenerate: all roots identical → d2_min = EPS2 → very high score."""
    path = "/tmp/sp_test_identical.bin"
    solve = [(1.0, 1.0), (1.0, 1.0), (1.0, 1.0)]
    write_bin(path, [solve], 3)
    result, err = run_clip(path, 3)
    check("identical roots: runs without crash", result is not None, err)
    if result:
        # Score should be very high (~150 for EPS2=1e-300)
        check("identical roots: high score",
              result["min_score"] > 100,
              f"score={result['min_score']}")
    os.remove(path)


def test_degree_3():
    """3 roots: verify correct pair selection (i<j only)."""
    path = "/tmp/sp_test_d3.bin"
    # Roots: (0,0), (1,0), (10,0) → closest pair is (0,0)-(1,0), d2_min=1.0
    solve = [(0.0, 0.0), (1.0, 0.0), (10.0, 0.0)]
    write_bin(path, [solve], 3)
    result, err = run_clip(path, 3)
    check("degree 3: runs", result is not None, err)
    if result:
        expected = expected_score(solve)  # -0.5 * log10(1.0) = 0.0
        check("degree 3: score ~ 0.0",
              abs(result["min_score"] - expected) < 0.01,
              f"got {result['min_score']}, expected {expected}")
    os.remove(path)


def test_empty_file():
    """Empty file should fail cleanly."""
    path = "/tmp/sp_test_empty.bin"
    with open(path, "wb") as f:
        pass
    result, err = run_clip(path, 2)
    check("empty file: rejected", result is None, "should have failed")
    os.remove(path)


def test_invalid_clip_range():
    """Hist with zero-width clip range should fail."""
    path = "/tmp/sp_test_badclip.bin"
    write_bin(path, [SOLVE_A], 2)
    result, err = run_hist(path, 2, clip_lo=5.0, clip_hi=5.0)
    check("invalid clip range: rejected", result is None, "should have failed")
    os.remove(path)


if __name__ == "__main__":
    if not os.path.exists(BINARY):
        print(f"ERROR: {BINARY} not found")
        print("Build: cd lambda && cc -O2 -Wall -o solve_proximity_stats_test solve_proximity_stats.c -lm")
        sys.exit(1)

    print("solve_proximity_stats tests")
    print("=" * 50)

    print("\n--- clip mode ---")
    test_clip_basic()
    test_clip_many_solves()

    print("\n--- hist mode ---")
    test_hist_basic()
    test_hist_100_bins()

    print("\n--- edge cases ---")
    test_identical_roots()
    test_degree_3()
    test_empty_file()
    test_invalid_clip_range()

    print("\n" + "=" * 50)
    total = passed + failed
    print(f"{passed}/{total} passed")
    if failed:
        print("FAIL")
        sys.exit(1)
    else:
        print("ALL PASS")
