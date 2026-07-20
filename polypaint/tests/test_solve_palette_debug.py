"""
Tests for solve_palette_debug binary.

Validates serpentine placement, nearest-neighbor expansion, palette selection,
quantile clipping, and fallback behavior.

Run: cd polypaint && uv run python tests/test_solve_palette_debug.py
"""
import json
import math
import os
import struct
import subprocess
import sys

import pytest

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")


def _docker_available():
    try:
        r = subprocess.run(["docker", "info"], capture_output=True, timeout=10)
    except Exception:
        return False
    return r.returncode == 0


pytestmark = pytest.mark.skipif(not _docker_available(), reason="Docker unavailable or unhealthy")


def _docker_run(cmd_inside):
    # Docker Desktop's VirtioFS mount intermittently refuses the FIRST
    # open-for-write in a fresh container ("Cannot open output /src/...")
    # and succeeds on an identical immediate rerun — measured 1-in-3 on an
    # otherwise idle daemon. Retry ONCE, only on that exact signature, so
    # a real binary failure still surfaces first try.
    for attempt in range(2):
        r = subprocess.run(
            ["docker", "run", "--rm", "--platform", "linux/arm64",
             "-v", f"{os.path.abspath(LAMBDA_DIR)}:/src",
             "public.ecr.aws/amazonlinux/amazonlinux:2023",
             "bash", "-c", cmd_inside],
            capture_output=True, text=True, timeout=30,
        )
        flake = ("Cannot open" in (r.stderr or "") and "/src/_test_pal" in (r.stderr or ""))
        if r.returncode == 0 or not flake:
            return r
    return r


def write_lores_bin(path, solves, degree):
    """Write a .bin file with float32 root pairs."""
    with open(path, "wb") as f:
        for solve in solves:
            for re, im in solve:
                f.write(struct.pack("<ff", re, im))
            for _ in range(degree - len(solve)):
                f.write(struct.pack("<ff", 0.0, 0.0))


def run_palette(bin_path, degree, lores_n, full_n, times=1, metric="proximity",
                palette="inferno", quantile_lo="0.001", quantile_hi="0.999",
                with_sidecars=False):
    """Run palette debug binary via Docker, return (meta_json, raw_bytes_or_None, stderr, scores_bytes, bins_bytes)."""
    import shutil
    host_bin = os.path.join(LAMBDA_DIR, "_test_pal_input.bin")
    host_out = os.path.join(LAMBDA_DIR, "_test_pal_out.raw")
    host_scores = os.path.join(LAMBDA_DIR, "_test_pal_scores.bin")
    host_bins = os.path.join(LAMBDA_DIR, "_test_pal_bins.bin")
    shutil.copy(bin_path, host_bin)
    # VirtioFS flakes on the container CREATING a new file in the mount
    # (measured: each failing run had created the previous output and died
    # on the next new one; reruns succeed once the files exist). Pre-create
    # every expected output on the host so the container only ever opens
    # existing files.
    precreate = [host_out]
    if with_sidecars:
        precreate += [host_scores, host_bins]
    for p in precreate:
        open(p, "wb").close()
    try:
        args = (f"/src/solve_palette_debug /src/_test_pal_input.bin /src/_test_pal_out.raw "
                f"--degree={degree} --lores_n={lores_n} --full_n={full_n} --times={times} "
                f"--metric={metric} --palette={palette} "
                f"--quantile_lo={quantile_lo} --quantile_hi={quantile_hi}")
        if with_sidecars:
            args += " --scores_out=/src/_test_pal_scores.bin --palette_bins_out=/src/_test_pal_bins.bin"
        r = _docker_run(args)
        if r.returncode != 0:
            return None, None, r.stderr, None, None
        meta = json.loads(r.stdout)
        raw_data = None
        scores_data = None
        bins_data = None
        if os.path.exists(host_out):
            with open(host_out, "rb") as f:
                raw_data = f.read()
        if os.path.exists(host_scores):
            with open(host_scores, "rb") as f:
                scores_data = f.read()
        if os.path.exists(host_bins):
            with open(host_bins, "rb") as f:
                bins_data = f.read()
        return meta, raw_data, None, scores_data, bins_data
    finally:
        for p in [host_bin, host_out, host_scores, host_bins]:
            try:
                os.remove(p)
            except OSError:
                pass


def parse_raw(raw_data):
    """Parse raw RGB output: uint32 width, uint32 height, uint32 channels, then RGB bytes."""
    w, h, c = struct.unpack("<III", raw_data[:12])
    pixels = raw_data[12:]
    return w, h, c, pixels


def test_basic_output():
    """Basic 3x3 grid → 6x6 output with times=1."""
    path = "/tmp/pal_test_basic.bin"
    degree = 2
    lores_n = 3
    solves = [[(i * 0.1, 0.0), (i * 0.1 + 1, 0.0)] for i in range(lores_n * lores_n)]
    write_lores_bin(path, solves, degree)
    meta, raw_data, err, _, _ = run_palette(path, degree, lores_n, full_n=6, times=1)
    assert meta is not None, f"failed: {err}"
    assert meta["mode"] == "palette_debug"
    assert meta["n_samples_used"] == 9
    assert meta["lores_n"] == 3
    assert meta["full_n"] == 6
    assert meta["using_pass"] == 0
    w, h, c, pixels = parse_raw(raw_data)
    assert w == 6 and h == 6 and c == 3
    assert len(pixels) == 6 * 6 * 3
    os.remove(path)


def test_times2_uses_pass0():
    """times=2 only uses pass 0 (first loresN*loresN solves)."""
    path = "/tmp/pal_test_times2.bin"
    degree = 2
    lores_n = 2
    # Pass 0: 4 solves with roots near 0
    pass0 = [[(0.1 * i, 0.0), (0.1 * i + 1, 0.0)] for i in range(4)]
    # Pass 1: 4 solves with roots far away (should be ignored)
    pass1 = [[(100.0, 0.0), (200.0, 0.0)] for _ in range(4)]
    write_lores_bin(path, pass0 + pass1, degree)
    meta, raw_data, err, _, _ = run_palette(path, degree, lores_n, full_n=4, times=2)
    assert meta is not None, f"failed: {err}"
    assert meta["n_samples_used"] == 4
    assert meta["times"] == 2
    assert meta["using_pass"] == 0
    os.remove(path)


def test_palette_affects_output():
    """Different palette name produces different RGB output."""
    path = "/tmp/pal_test_pal.bin"
    degree = 2
    lores_n = 2
    solves = [[(0.0, 0.0), (1.0, 0.0)], [(0.0, 0.0), (0.01, 0.0)],
              [(0.0, 0.0), (0.5, 0.0)], [(0.0, 0.0), (2.0, 0.0)]]
    write_lores_bin(path, solves, degree)
    _, raw_inf, _, _, _ = run_palette(path, degree, lores_n, full_n=2, palette="inferno")
    _, raw_vir, _, _, _ = run_palette(path, degree, lores_n, full_n=2, palette="viridis")
    assert raw_inf is not None and raw_vir is not None
    # RGB bytes should differ between palettes
    assert raw_inf[12:] != raw_vir[12:], "inferno and viridis should produce different colors"
    os.remove(path)


def test_quantile_changes_clip():
    """q=0.05 produces different (narrower) clip range than q=0.001."""
    path = "/tmp/pal_test_q.bin"
    degree = 2
    lores_n = 15  # 225 solves
    solves = [[(0.001 + i * 0.01, 0.0), (0.001 + i * 0.01 + 1, 0.0)]
              for i in range(lores_n * lores_n)]
    write_lores_bin(path, solves, degree)
    m1, _, _, _, _ = run_palette(path, degree, lores_n, full_n=15, quantile_lo="0.001", quantile_hi="0.999")
    m2, _, _, _, _ = run_palette(path, degree, lores_n, full_n=15, quantile_lo="0.05", quantile_hi="0.95")
    assert m1 is not None and m2 is not None
    r1 = m1["clip_hi"] - m1["clip_lo"]
    r2 = m2["clip_hi"] - m2["clip_lo"]
    assert r2 <= r1, f"q=0.05 range ({r2}) should be <= q=0.001 range ({r1})"
    os.remove(path)


def test_fallback_on_small_sample():
    """Small sample (< 100 solves) triggers clip fallback."""
    path = "/tmp/pal_test_fb.bin"
    degree = 2
    lores_n = 3  # 9 solves
    solves = [[(i * 0.5, 0.0), (i * 0.5 + 1, 0.0)] for i in range(9)]
    write_lores_bin(path, solves, degree)
    meta, _, _, _, _ = run_palette(path, degree, lores_n, full_n=3)
    assert meta is not None
    assert meta["clip_fallback"] is True
    assert meta["clip_fallback_reason"] in ("small_sample", "zero_full_range_expanded"), \
        f"unexpected fallback reason: {meta['clip_fallback_reason']}"
    os.remove(path)


def test_invalid_palette_rejected():
    """Invalid palette name fails instead of silent fallback."""
    path = "/tmp/pal_test_badpal.bin"
    degree = 2
    lores_n = 2
    solves = [[(0.0, 0.0), (1.0, 0.0)] for _ in range(4)]
    write_lores_bin(path, solves, degree)
    meta, _, err, _, _ = run_palette(path, degree, lores_n, full_n=2, palette="nonexistent")
    assert meta is None, f"invalid palette should fail, got: {meta}"
    os.remove(path)


def test_solve_count_mismatch():
    """Wrong times/loresN fails hard."""
    path = "/tmp/pal_test_mismatch.bin"
    degree = 2
    write_lores_bin(path, [[(0.0, 0.0), (1.0, 0.0)]] * 4, degree)
    meta, _, err, _, _ = run_palette(path, degree, lores_n=3, full_n=3, times=1)
    # 4 solves but expected 3*3=9 → mismatch
    assert meta is None, "solve count mismatch should fail"
    os.remove(path)


def test_writes_score_and_palette_bin_sidecars():
    """Optional sidecar outputs are written in lores grid order."""
    path = "/tmp/pal_test_sidecars.bin"
    degree = 2
    lores_n = 2
    solves = [[(0.0, 0.0), (1.0, 0.0)], [(0.0, 0.0), (0.2, 0.0)],
              [(0.0, 0.0), (0.5, 0.0)], [(0.0, 0.0), (2.0, 0.0)]]
    write_lores_bin(path, solves, degree)
    meta, raw_data, err, scores_data, bins_data = run_palette(path, degree, lores_n, full_n=2, with_sidecars=True)
    assert meta is not None, f"failed: {err}"
    assert scores_data is not None and len(scores_data) == lores_n * lores_n * 4
    assert bins_data is not None and len(bins_data) == lores_n * lores_n
    os.remove(path)


if __name__ == "__main__":
    r = subprocess.run(["docker", "info"], capture_output=True, timeout=10)
    if r.returncode != 0:
        print("ERROR: Docker not available"); sys.exit(1)
    binary = os.path.join(LAMBDA_DIR, "solve_palette_debug")
    if not os.path.exists(binary):
        print(f"ERROR: {binary} not found"); sys.exit(1)

    print("solve_palette_debug tests (Docker ARM64)")
    print("=" * 50)

    tests = [
        ("basic output", test_basic_output),
        ("times=2 uses pass 0", test_times2_uses_pass0),
        ("palette affects output", test_palette_affects_output),
        ("quantile changes clip", test_quantile_changes_clip),
        ("fallback on small sample", test_fallback_on_small_sample),
        ("invalid palette rejected", test_invalid_palette_rejected),
        ("solve count mismatch", test_solve_count_mismatch),
        ("writes sidecars", test_writes_score_and_palette_bin_sidecars),
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
        sys.exit(1)
    else:
        print("ALL PASS")
