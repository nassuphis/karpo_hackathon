"""
Tests for solve_proximity_stats binary — legacy and new solve metrics.

Validates the algorithmic contract using the Docker runtime (deploy binary):
- proximity: clip/hist reference tests (existing)
- crowding: ranking test — globally clustered > spread-out
- spread: ranking test — large cloud > small cloud
- anisotropy: ranking test — line-like > isotropic
- area: ranking test — large 2D cloud > small 2D cloud
- centroid_re / centroid_im / centroid_dist ranking tests
- dist_unit_circle and asymmetry_re ranking tests
- min_mod / max_mod / min_angular_separation ranking tests
- hist for non-proximity metric (spread)
- invalid metric rejection
- root-transform metric test

Run: cd polypaint && uv run python tests/test_solve_proximity_stats.py
"""
import json
import math
import os
import shlex
import struct
import subprocess
import sys
import shutil

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
LAPACK_BUILD = os.path.join(LAMBDA_DIR, "layer-build-lapack")
ROOT = os.path.join(os.path.dirname(__file__), "..")
SOLVE_PROX_SRC = os.path.join(LAMBDA_DIR, "solve_proximity_stats.c")
SOLVE_SCORE_HDR = os.path.join(LAMBDA_DIR, "solve_score.h")
SOLVE_PROX_BIN = os.path.join(LAMBDA_DIR, "solve_proximity_stats")
_ENSURED_BINARY = False


def _ensure_solve_proximity_binary():
    global _ENSURED_BINARY
    if _ENSURED_BINARY:
        return
    newest_source_mtime = max(os.path.getmtime(SOLVE_PROX_SRC), os.path.getmtime(SOLVE_SCORE_HDR))
    if (os.path.exists(SOLVE_PROX_BIN)
            and os.path.getmtime(SOLVE_PROX_BIN) >= newest_source_mtime):
        _ENSURED_BINARY = True
        return
    cc = shutil.which("aarch64-linux-musl-gcc")
    if not cc:
        raise RuntimeError("aarch64-linux-musl-gcc not found; cannot rebuild lambda/solve_proximity_stats")
    r = subprocess.run(
        [cc, "-O3", "-static", "-pthread", "-o", SOLVE_PROX_BIN, SOLVE_PROX_SRC, "-lm"],
        capture_output=True,
        text=True,
        timeout=120,
    )
    if r.returncode != 0:
        raise RuntimeError(f"failed to rebuild solve_proximity_stats: {r.stderr.strip()}")
    _ENSURED_BINARY = True


def _docker_run(cmd_inside):
    """Run a command inside ARM64 Docker with /src mounted to lambda/."""
    _ensure_solve_proximity_binary()
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


def write_xforms(path, xforms):
    """Write root transforms JSON sidecar."""
    with open(path, "w") as f:
        json.dump(xforms, f)


def run_clip(bin_path, degree, metric="proximity", **kwargs):
    """Run clip mode via Docker."""
    docker_bin = "/src/_test_input.bin"
    host_bin = os.path.join(LAMBDA_DIR, "_test_input.bin")
    import shutil
    shutil.copy(bin_path, host_bin)
    # Copy xforms sidecar if present
    xforms_host = kwargs.pop("_xforms_host", None)
    try:
        args = f"/src/solve_proximity_stats {docker_bin} --mode=clip --degree={degree} --metric={metric}"
        for k, v in kwargs.items():
            args += f" --{k}={shlex.quote(str(v))}"
        r = _docker_run(args)
        if r.returncode != 0:
            return None, r.stderr
        return json.loads(r.stdout), None
    finally:
        try:
            os.remove(host_bin)
        except OSError:
            pass
        if xforms_host:
            try:
                os.remove(xforms_host)
            except OSError:
                pass


def run_clip_stdin(bin_path, degree, metric="proximity", **kwargs):
    """Run clip mode via Docker, streaming the input file on stdin."""
    docker_bin = "/src/_test_input.bin"
    host_bin = os.path.join(LAMBDA_DIR, "_test_input.bin")
    shutil.copy(bin_path, host_bin)
    try:
        size = os.path.getsize(host_bin)
        args = (
            f"/src/solve_proximity_stats - --input_size={size} --mode=clip "
            f"--degree={degree} --metric={metric} < {docker_bin}"
        )
        for k, v in kwargs.items():
            args += f" --{k}={shlex.quote(str(v))}"
        r = _docker_run(args)
        if r.returncode != 0:
            return None, r.stderr
        return json.loads(r.stdout), None
    finally:
        try:
            os.remove(host_bin)
        except OSError:
            pass


def run_hist(bin_path, degree, clip_lo, clip_hi, hist_bins=100, metric="proximity", **kwargs):
    """Run hist mode via Docker."""
    import shutil
    host_bin = os.path.join(LAMBDA_DIR, "_test_input.bin")
    shutil.copy(bin_path, host_bin)
    try:
        args = (f"/src/solve_proximity_stats /src/_test_input.bin --mode=hist "
                f"--degree={degree} --clip_lo={clip_lo} --clip_hi={clip_hi} "
                f"--hist_bins={hist_bins} --metric={metric}")
        for k, v in kwargs.items():
            args += f" --{k}={shlex.quote(str(v))}"
        r = _docker_run(args)
        if r.returncode != 0:
            return None, r.stderr
        return json.loads(r.stdout), None
    finally:
        try:
            os.remove(host_bin)
        except OSError:
            pass


def run_hist_stdin(bin_path, degree, clip_lo, clip_hi, hist_bins=100, metric="proximity", **kwargs):
    """Run hist mode via Docker, streaming the input file on stdin."""
    host_bin = os.path.join(LAMBDA_DIR, "_test_input.bin")
    shutil.copy(bin_path, host_bin)
    try:
        size = os.path.getsize(host_bin)
        args = (
            f"/src/solve_proximity_stats - --input_size={size} --mode=hist "
            f"--degree={degree} --clip_lo={clip_lo} --clip_hi={clip_hi} "
            f"--hist_bins={hist_bins} --metric={metric} < /src/_test_input.bin"
        )
        for k, v in kwargs.items():
            args += f" --{k}={shlex.quote(str(v))}"
        r = _docker_run(args)
        if r.returncode != 0:
            return None, r.stderr
        return json.loads(r.stdout), None
    finally:
        try:
            os.remove(host_bin)
        except OSError:
            pass


def expected_proximity_score(roots):
    """Python reference for proximity metric."""
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


# ================================================================
# 1. Proximity reference tests (existing)
# ================================================================

def test_clip_basic():
    """Clip with 3 known solves, verify proximity scores and bounds."""
    path = "/tmp/sp_test_clip.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_clip(path, 2, metric="proximity")
    assert result is not None, f"clip failed: {err}"
    assert result["n_solves"] == 3
    assert result["degree"] == 2
    assert result["metric"] == "proximity"
    assert result["threads"] == 1
    score_a = expected_proximity_score(SOLVE_A)
    score_b = expected_proximity_score(SOLVE_B)
    assert abs(result["min_score"] - score_a) < 0.01, f"min_score={result['min_score']}, expected ~{score_a}"
    assert abs(result["max_score"] - score_b) < 0.01, f"max_score={result['max_score']}, expected ~{score_b}"
    assert abs(result["clip_lo"] - result["min_score"]) < 1e-10
    assert abs(result["clip_hi"] - result["max_score"]) < 1e-10
    os.remove(path)


def test_clip_stdin_matches_file_input():
    path = "/tmp/sp_test_clip_stdin.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    file_result, file_err = run_clip(path, 2, metric="proximity")
    stdin_result, stdin_err = run_clip_stdin(path, 2, metric="proximity")
    assert file_result is not None, f"file clip failed: {file_err}"
    assert stdin_result is not None, f"stdin clip failed: {stdin_err}"
    assert stdin_result["n_solves"] == file_result["n_solves"]
    assert stdin_result["metric"] == file_result["metric"]
    assert abs(stdin_result["clip_lo"] - file_result["clip_lo"]) < 1e-12
    assert abs(stdin_result["clip_hi"] - file_result["clip_hi"]) < 1e-12
    assert abs(stdin_result["min_score"] - file_result["min_score"]) < 1e-12
    assert abs(stdin_result["max_score"] - file_result["max_score"]) < 1e-12
    os.remove(path)


def test_clip_many_solves():
    """Clip with 200 solves to exercise quantile path."""
    path = "/tmp/sp_test_clip_many.bin"
    solves = []
    for i in range(200):
        d = 0.001 + (i / 200.0) * 2.0
        solves.append([(0.0, 0.0), (d, 0.0)])
    write_bin(path, solves, 2)
    result, err = run_clip(path, 2, metric="proximity")
    assert result is not None, f"clip failed: {err}"
    assert result["n_solves"] == 200
    assert result["clip_lo"] < result["clip_hi"]
    assert result["clip_lo"] >= result["min_score"] - 1e-10
    assert result["clip_hi"] <= result["max_score"] + 1e-10
    os.remove(path)


def test_hist_basic():
    """Histogram with known proximity scores and default omega remapping."""
    path = "/tmp/sp_test_hist.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_hist(path, 2, clip_lo=0.0, clip_hi=2.0, hist_bins=10, metric="proximity")
    assert result is not None, f"hist failed: {err}"
    assert result["n_solves"] == 3
    assert result["metric"] == "proximity"
    assert result["threads"] == 1
    assert len(result["hist"]) == 10
    assert sum(result["hist"]) == 3
    # With default omega=1, u=0 and u=1 both map to the top bin, while u=0.5 maps to the bottom bin.
    assert result["hist"][0] >= 1, f"bin 0 should have solve_c after omega remap: {result['hist']}"
    assert result["hist"][9] >= 2, f"bin 9 should have solve_a and solve_b after omega remap: {result['hist']}"
    os.remove(path)


def test_hist_stdin_matches_file_input():
    path = "/tmp/sp_test_hist_stdin.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    file_result, file_err = run_hist(path, 2, clip_lo=0.0, clip_hi=2.0, hist_bins=10, metric="proximity")
    stdin_result, stdin_err = run_hist_stdin(path, 2, clip_lo=0.0, clip_hi=2.0, hist_bins=10, metric="proximity")
    assert file_result is not None, f"file hist failed: {file_err}"
    assert stdin_result is not None, f"stdin hist failed: {stdin_err}"
    assert stdin_result["n_solves"] == file_result["n_solves"]
    assert stdin_result["metric"] == file_result["metric"]
    assert stdin_result["threads"] == file_result["threads"]
    assert stdin_result["hist"] == file_result["hist"]
    os.remove(path)


def test_hist_100_bins():
    """Histogram with default 100 bins."""
    path = "/tmp/sp_test_hist100.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_hist(path, 2, clip_lo=0.0, clip_hi=2.0, metric="proximity")
    assert result is not None, f"hist failed: {err}"
    assert len(result["hist"]) == 100
    assert sum(result["hist"]) == 3
    os.remove(path)


def test_identical_roots():
    """Degenerate: all roots identical -> very high proximity score."""
    path = "/tmp/sp_test_identical.bin"
    solve = [(1.0, 1.0), (1.0, 1.0), (1.0, 1.0)]
    write_bin(path, [solve], 3)
    result, err = run_clip(path, 3, metric="proximity")
    assert result is not None, f"identical roots failed: {err}"
    assert result["min_score"] > 100, f"score should be very high: {result['min_score']}"
    os.remove(path)


def test_degree_3():
    """3 roots: verify correct pair selection (i<j only)."""
    path = "/tmp/sp_test_d3.bin"
    solve = [(0.0, 0.0), (1.0, 0.0), (10.0, 0.0)]
    write_bin(path, [solve], 3)
    result, err = run_clip(path, 3, metric="proximity")
    assert result is not None, f"degree 3 failed: {err}"
    expected = expected_proximity_score(solve)
    assert abs(result["min_score"] - expected) < 0.01, f"score={result['min_score']}, expected={expected}"
    os.remove(path)


def test_empty_file():
    """Empty file should fail cleanly."""
    path = "/tmp/sp_test_empty.bin"
    with open(path, "wb") as f:
        pass
    result, err = run_clip(path, 2, metric="proximity")
    assert result is None, "empty file should have failed"
    os.remove(path)


def test_invalid_clip_range():
    """Hist with zero-width clip range should fail."""
    path = "/tmp/sp_test_badclip.bin"
    write_bin(path, [SOLVE_A], 2)
    result, err = run_hist(path, 2, clip_lo=5.0, clip_hi=5.0, metric="proximity")
    assert result is None, "invalid clip range should have failed"
    os.remove(path)


# ================================================================
# 2. Crowding ranking test
# ================================================================

def test_crowding_ranking():
    """Same d_min, different global spacing → clustered solve has higher crowding."""
    # Spread-out: roots at (0,0), (0.1,0), (100,0) — close pair + one far away
    # Clustered:  roots at (0,0), (0.1,0), (0.2,0) — all roots close together
    # Crowding = mean of -0.5*log10(d2) over all pairs.
    # Clustered: all 3 pairs small → high mean. Spread-out: 2 pairs huge → low mean.
    spread_solve = [(0.0, 0.0), (0.1, 0.0), (100.0, 0.0)]
    cluster_solve = [(0.0, 0.0), (0.1, 0.0), (0.2, 0.0)]

    # Write spread first (index 0), cluster second (index 1)
    path = "/tmp/sp_test_crowding.bin"
    write_bin(path, [spread_solve, cluster_solve], 3)
    result, err = run_clip(path, 3, metric="crowding")
    assert result is not None, f"crowding clip failed: {err}"
    assert result["metric"] == "crowding"
    assert result["n_solves"] == 2
    # Clustered (index 1) should be max, spread-out (index 0) should be min
    assert result["max_score"] > result["min_score"], \
        f"clustered should score higher than spread-out: max={result['max_score']}, min={result['min_score']}"

    # Verify by running each solve alone to confirm which is which
    path_sp = "/tmp/sp_test_crowding_spread.bin"
    path_cl = "/tmp/sp_test_crowding_cluster.bin"
    write_bin(path_sp, [spread_solve], 3)
    write_bin(path_cl, [cluster_solve], 3)
    r_sp, _ = run_clip(path_sp, 3, metric="crowding")
    r_cl, _ = run_clip(path_cl, 3, metric="crowding")
    assert r_sp is not None and r_cl is not None
    assert r_cl["min_score"] > r_sp["min_score"], \
        f"cluster score {r_cl['min_score']} should exceed spread score {r_sp['min_score']}"
    for p in [path, path_sp, path_cl]:
        os.remove(p)


# ================================================================
# 3. Spread ranking test
# ================================================================

def test_spread_ranking():
    """Same centroid, different scale → larger cloud has higher spread."""
    small = [(0.0, 0.1), (0.0, -0.1), (0.1, 0.0), (-0.1, 0.0)]
    large = [(0.0, 10.0), (0.0, -10.0), (10.0, 0.0), (-10.0, 0.0)]

    # Verify each solve's score individually
    path_sm = "/tmp/sp_test_spread_small.bin"
    path_lg = "/tmp/sp_test_spread_large.bin"
    write_bin(path_sm, [small], 4)
    write_bin(path_lg, [large], 4)
    r_sm, err = run_clip(path_sm, 4, metric="spread")
    assert r_sm is not None, f"small spread failed: {err}"
    r_lg, err = run_clip(path_lg, 4, metric="spread")
    assert r_lg is not None, f"large spread failed: {err}"
    assert r_sm["metric"] == "spread"
    assert r_lg["metric"] == "spread"
    assert r_lg["min_score"] > r_sm["min_score"], \
        f"large cloud ({r_lg['min_score']}) should have higher spread than small ({r_sm['min_score']})"
    for p in [path_sm, path_lg]:
        os.remove(p)


# ================================================================
# 4. Anisotropy ranking test
# ================================================================

def test_anisotropy_ranking():
    """Isotropic vs line-like → line-like has higher anisotropy."""
    # Isotropic: roots roughly circular (lambda_max ≈ lambda_min)
    iso = [(1.0, 0.0), (-1.0, 0.0), (0.0, 1.0), (0.0, -1.0)]
    # Line-like: all roots along x-axis (lambda_min ≈ 0)
    line = [(0.0, 0.0), (1.0, 0.0), (2.0, 0.0), (3.0, 0.0)]

    path_iso = "/tmp/sp_test_aniso_iso.bin"
    path_line = "/tmp/sp_test_aniso_line.bin"
    write_bin(path_iso, [iso], 4)
    write_bin(path_line, [line], 4)
    r_iso, err = run_clip(path_iso, 4, metric="anisotropy")
    assert r_iso is not None, f"iso anisotropy failed: {err}"
    r_line, err = run_clip(path_line, 4, metric="anisotropy")
    assert r_line is not None, f"line anisotropy failed: {err}"
    assert r_iso["metric"] == "anisotropy"
    assert r_line["metric"] == "anisotropy"
    assert r_line["min_score"] > r_iso["min_score"], \
        f"line-like ({r_line['min_score']}) should have higher anisotropy than isotropic ({r_iso['min_score']})"
    for p in [path_iso, path_line]:
        os.remove(p)


# ================================================================
# 5. Area ranking test
# ================================================================

def test_area_ranking():
    """Small 2D cloud vs large 2D cloud → large has higher area."""
    small = [(0.0, 0.0), (0.1, 0.0), (0.0, 0.1), (0.1, 0.1)]
    large = [(0.0, 0.0), (10.0, 0.0), (0.0, 10.0), (10.0, 10.0)]

    path_sm = "/tmp/sp_test_area_small.bin"
    path_lg = "/tmp/sp_test_area_large.bin"
    write_bin(path_sm, [small], 4)
    write_bin(path_lg, [large], 4)
    r_sm, err = run_clip(path_sm, 4, metric="area")
    assert r_sm is not None, f"small area failed: {err}"
    r_lg, err = run_clip(path_lg, 4, metric="area")
    assert r_lg is not None, f"large area failed: {err}"
    assert r_sm["metric"] == "area"
    assert r_lg["metric"] == "area"
    assert r_lg["min_score"] > r_sm["min_score"], \
        f"large cloud ({r_lg['min_score']}) should have higher area than small ({r_sm['min_score']})"
    for p in [path_sm, path_lg]:
        os.remove(p)


# ================================================================
# 6. Clusteriness ranking test
# ================================================================

def test_clusteriness_ranking():
    """One tight pair vs uniform spacing → tight pair has higher clusteriness."""
    # Uniform: evenly spaced along x-axis
    uniform = [(0.0, 0.0), (1.0, 0.0), (2.0, 0.0), (3.0, 0.0)]
    # One-pair: one very tight pair, rest normal
    one_pair = [(0.0, 0.0), (0.001, 0.0), (1.0, 0.0), (2.0, 0.0)]

    path_u = "/tmp/sp_test_cluster_uniform.bin"
    path_p = "/tmp/sp_test_cluster_onepair.bin"
    write_bin(path_u, [uniform], 4)
    write_bin(path_p, [one_pair], 4)
    r_u, err = run_clip(path_u, 4, metric="clusteriness")
    assert r_u is not None, f"uniform clusteriness failed: {err}"
    r_p, err = run_clip(path_p, 4, metric="clusteriness")
    assert r_p is not None, f"one-pair clusteriness failed: {err}"
    assert r_u["metric"] == "clusteriness"
    assert r_p["metric"] == "clusteriness"
    assert r_p["min_score"] > r_u["min_score"], \
        f"one-pair ({r_p['min_score']}) should have higher clusteriness than uniform ({r_u['min_score']})"
    for p in [path_u, path_p]:
        os.remove(p)


# ================================================================
# 7. Shelliness ranking test
# ================================================================

def test_shelliness_ranking():
    """Ring vs filled → ring has higher shelliness."""
    import math
    sq = math.sqrt(0.5)
    # Shell: all roots near unit circle
    shell = [(1.0, 0.0), (0.0, 1.0), (-1.0, 0.0), (0.0, -1.0), (sq, sq)]
    # Filled: 4 on circle + 1 at center
    filled = [(1.0, 0.0), (0.0, 1.0), (-1.0, 0.0), (0.0, -1.0), (0.0, 0.0)]

    path_sh = "/tmp/sp_test_shell_shell.bin"
    path_fi = "/tmp/sp_test_shell_filled.bin"
    write_bin(path_sh, [shell], 5)
    write_bin(path_fi, [filled], 5)
    r_sh, err = run_clip(path_sh, 5, metric="shelliness")
    assert r_sh is not None, f"shell shelliness failed: {err}"
    r_fi, err = run_clip(path_fi, 5, metric="shelliness")
    assert r_fi is not None, f"filled shelliness failed: {err}"
    assert r_sh["metric"] == "shelliness"
    assert r_fi["metric"] == "shelliness"
    assert r_sh["min_score"] > r_fi["min_score"], \
        f"shell ({r_sh['min_score']}) should have higher shelliness than filled ({r_fi['min_score']})"
    for p in [path_sh, path_fi]:
        os.remove(p)


# ================================================================
# 8. Outlierness ranking test
# ================================================================

def test_outlierness_ranking():
    """Compact cloud vs one outlier root → outlier has higher outlierness."""
    compact = [(-1.0, 0.0), (1.0, 0.0), (0.0, 1.0), (0.0, -1.0), (0.5, 0.5)]
    outlier = [(-1.0, 0.0), (1.0, 0.0), (0.0, 1.0), (0.0, -1.0), (100.0, 0.0)]

    path_c = "/tmp/sp_test_outlier_compact.bin"
    path_o = "/tmp/sp_test_outlier_outlier.bin"
    write_bin(path_c, [compact], 5)
    write_bin(path_o, [outlier], 5)
    r_c, err = run_clip(path_c, 5, metric="outlierness")
    assert r_c is not None, f"compact outlierness failed: {err}"
    r_o, err = run_clip(path_o, 5, metric="outlierness")
    assert r_o is not None, f"outlier outlierness failed: {err}"
    assert r_c["metric"] == "outlierness"
    assert r_o["metric"] == "outlierness"
    assert r_o["min_score"] > r_c["min_score"], \
        f"outlier ({r_o['min_score']}) should have higher outlierness than compact ({r_c['min_score']})"
    for p in [path_c, path_o]:
        os.remove(p)


# ================================================================
# 9. NN variation ranking test
# ================================================================

def test_nn_variation_ranking():
    """Uniform spacing vs mixed → mixed has higher nn_variation."""
    uniform = [(0.0, 0.0), (1.0, 0.0), (2.0, 0.0), (3.0, 0.0)]
    mixed = [(0.0, 0.0), (0.001, 0.0), (1.0, 0.0), (10.0, 0.0)]

    path_u = "/tmp/sp_test_nnvar_uniform.bin"
    path_m = "/tmp/sp_test_nnvar_mixed.bin"
    write_bin(path_u, [uniform], 4)
    write_bin(path_m, [mixed], 4)
    r_u, err = run_clip(path_u, 4, metric="nn_variation")
    assert r_u is not None, f"uniform nn_variation failed: {err}"
    r_m, err = run_clip(path_m, 4, metric="nn_variation")
    assert r_m is not None, f"mixed nn_variation failed: {err}"
    assert r_u["metric"] == "nn_variation"
    assert r_m["metric"] == "nn_variation"
    assert r_m["min_score"] > r_u["min_score"], \
        f"mixed ({r_m['min_score']}) should have higher nn_variation than uniform ({r_u['min_score']})"
    for p in [path_u, path_m]:
        os.remove(p)


# ================================================================
# 10. Real-axis proximity ranking test
# ================================================================

def test_real_axis_proximity_ranking():
    """Near-real roots vs off-axis → near-real has higher real_axis_proximity."""
    near_real = [(0.0, 0.001), (1.0, -0.001), (2.0, 0.002), (3.0, -0.002)]
    off_axis = [(0.0, 10.0), (1.0, -10.0), (2.0, 12.0), (3.0, -12.0)]

    path_nr = "/tmp/sp_test_realprox_near.bin"
    path_oa = "/tmp/sp_test_realprox_off.bin"
    write_bin(path_nr, [near_real], 4)
    write_bin(path_oa, [off_axis], 4)
    r_nr, err = run_clip(path_nr, 4, metric="real_axis_proximity")
    assert r_nr is not None, f"near-real real_axis_proximity failed: {err}"
    r_oa, err = run_clip(path_oa, 4, metric="real_axis_proximity")
    assert r_oa is not None, f"off-axis real_axis_proximity failed: {err}"
    assert r_nr["metric"] == "real_axis_proximity"
    assert r_oa["metric"] == "real_axis_proximity"
    assert r_nr["min_score"] > r_oa["min_score"], \
        f"near-real ({r_nr['min_score']}) should have higher real_axis_proximity than off-axis ({r_oa['min_score']})"
    for p in [path_nr, path_oa]:
        os.remove(p)


# ================================================================
# 10b. Degree-1 centroid metric
# ================================================================

def test_centroid_re_degree_one_allowed():
    """Degree-1 solves should work for centroid metrics."""
    single = [(2.5, 0.0)]
    path = "/tmp/sp_test_centroid_re_deg1.bin"
    write_bin(path, [single], 1)
    r, err = run_clip(path, 1, metric="centroid_re")
    assert r is not None, f"centroid_re degree-1 failed: {err}"
    assert r["metric"] == "centroid_re"
    assert abs(r["min_score"] - 2.5) < 1e-9
    os.remove(path)


# ================================================================
# 10c. New v3 metric ranking tests
# ================================================================

def test_centroid_re_ranking():
    """Right-shifted cloud should have higher centroid_re than left-shifted."""
    left = [(-3.0, 0.0), (-2.0, 0.0), (-1.0, 0.0), (-2.0, 1.0)]
    right = [(1.0, 0.0), (2.0, 0.0), (3.0, 0.0), (2.0, 1.0)]
    path_l = "/tmp/sp_test_centroid_re_left.bin"
    path_r = "/tmp/sp_test_centroid_re_right.bin"
    write_bin(path_l, [left], 4)
    write_bin(path_r, [right], 4)
    r_l, err = run_clip(path_l, 4, metric="centroid_re")
    assert r_l is not None, f"left centroid_re failed: {err}"
    r_r, err = run_clip(path_r, 4, metric="centroid_re")
    assert r_r is not None, f"right centroid_re failed: {err}"
    assert r_r["min_score"] > r_l["min_score"]
    for p in [path_l, path_r]:
        os.remove(p)


def test_centroid_im_ranking():
    """Upper cloud should have higher centroid_im than lower cloud."""
    lower = [(0.0, -3.0), (0.0, -2.0), (0.0, -1.0), (1.0, -2.0)]
    upper = [(0.0, 1.0), (0.0, 2.0), (0.0, 3.0), (1.0, 2.0)]
    path_l = "/tmp/sp_test_centroid_im_lower.bin"
    path_u = "/tmp/sp_test_centroid_im_upper.bin"
    write_bin(path_l, [lower], 4)
    write_bin(path_u, [upper], 4)
    r_l, err = run_clip(path_l, 4, metric="centroid_im")
    assert r_l is not None, f"lower centroid_im failed: {err}"
    r_u, err = run_clip(path_u, 4, metric="centroid_im")
    assert r_u is not None, f"upper centroid_im failed: {err}"
    assert r_u["min_score"] > r_l["min_score"]
    for p in [path_l, path_u]:
        os.remove(p)


def test_centroid_dist_ranking():
    """Translated cloud should have higher centroid_dist than origin-centered cloud."""
    near = [(-0.5, 0.0), (0.5, 0.0), (0.0, 0.5), (0.0, -0.5)]
    far = [(4.5, 0.0), (5.5, 0.0), (5.0, 0.5), (5.0, -0.5)]
    path_n = "/tmp/sp_test_centroid_dist_near.bin"
    path_f = "/tmp/sp_test_centroid_dist_far.bin"
    write_bin(path_n, [near], 4)
    write_bin(path_f, [far], 4)
    r_n, err = run_clip(path_n, 4, metric="centroid_dist")
    assert r_n is not None, f"near centroid_dist failed: {err}"
    r_f, err = run_clip(path_f, 4, metric="centroid_dist")
    assert r_f is not None, f"far centroid_dist failed: {err}"
    assert r_f["min_score"] > r_n["min_score"]
    for p in [path_n, path_f]:
        os.remove(p)


def test_dist_unit_circle_ranking():
    """Roots on the unit circle should have smaller dist_unit_circle than translated roots."""
    on_circle = [(1.0, 0.0), (-1.0, 0.0), (0.0, 1.0), (0.0, -1.0)]
    off_circle = [(2.0, 0.0), (2.0, 0.0), (2.0, 0.0), (2.0, 0.0)]
    path_on = "/tmp/sp_test_unit_circle_on.bin"
    path_off = "/tmp/sp_test_unit_circle_off.bin"
    write_bin(path_on, [on_circle], 4)
    write_bin(path_off, [off_circle], 4)
    r_on, err = run_clip(path_on, 4, metric="dist_unit_circle")
    assert r_on is not None, f"on-circle dist_unit_circle failed: {err}"
    r_off, err = run_clip(path_off, 4, metric="dist_unit_circle")
    assert r_off is not None, f"off-circle dist_unit_circle failed: {err}"
    assert r_off["min_score"] > r_on["min_score"]
    for p in [path_on, path_off]:
        os.remove(p)


def test_asymmetry_re_ranking():
    """One-sided cloud should have higher asymmetry_re than symmetric cloud."""
    symmetric = [(-2.0, 0.0), (-1.0, 0.0), (1.0, 0.0), (2.0, 0.0)]
    one_sided = [(1.0, 0.0), (2.0, 0.0), (3.0, 0.0), (4.0, 0.0)]
    path_s = "/tmp/sp_test_asym_re_sym.bin"
    path_o = "/tmp/sp_test_asym_re_one.bin"
    write_bin(path_s, [symmetric], 4)
    write_bin(path_o, [one_sided], 4)
    r_s, err = run_clip(path_s, 4, metric="asymmetry_re")
    assert r_s is not None, f"symmetric asymmetry_re failed: {err}"
    r_o, err = run_clip(path_o, 4, metric="asymmetry_re")
    assert r_o is not None, f"one-sided asymmetry_re failed: {err}"
    assert r_o["min_score"] > r_s["min_score"]
    for p in [path_s, path_o]:
        os.remove(p)


def test_min_mod_ignores_zeros_and_ranking():
    """min_mod ignores exact zeros and returns the smallest non-zero modulus."""
    near = [(0.0, 0.0), (0.25, 0.0), (2.0, 0.0), (0.0, 3.0)]
    far = [(0.0, 0.0), (2.0, 0.0), (3.0, 0.0), (0.0, 4.0)]
    path_n = "/tmp/sp_test_min_mod_near.bin"
    path_f = "/tmp/sp_test_min_mod_far.bin"
    write_bin(path_n, [near], 4)
    write_bin(path_f, [far], 4)
    r_n, err = run_clip(path_n, 4, metric="min_mod")
    assert r_n is not None, f"near min_mod failed: {err}"
    r_f, err = run_clip(path_f, 4, metric="min_mod")
    assert r_f is not None, f"far min_mod failed: {err}"
    assert abs(r_n["min_score"] - 0.25) < 1e-6
    assert abs(r_f["min_score"] - 2.0) < 1e-6
    assert r_f["min_score"] > r_n["min_score"]
    for p in [path_n, path_f]:
        os.remove(p)


def test_min_mod_all_zero_returns_zero():
    """min_mod returns 0 when every root is exactly zero."""
    zeros = [(0.0, 0.0), (0.0, 0.0), (0.0, 0.0)]
    path = "/tmp/sp_test_min_mod_all_zero.bin"
    write_bin(path, [zeros], 3)
    r, err = run_clip(path, 3, metric="min_mod")
    assert r is not None, f"all-zero min_mod failed: {err}"
    assert r["metric"] == "min_mod"
    assert r["min_score"] == 0.0
    os.remove(path)


def test_max_mod_ranking():
    """Cloud with farther root should have higher max_mod."""
    small = [(0.0, 0.0), (1.0, 0.0), (0.0, 2.0), (-1.0, 0.0)]
    large = [(0.0, 0.0), (1.0, 0.0), (0.0, 5.0), (-1.0, 0.0)]
    path_s = "/tmp/sp_test_max_mod_small.bin"
    path_l = "/tmp/sp_test_max_mod_large.bin"
    write_bin(path_s, [small], 4)
    write_bin(path_l, [large], 4)
    r_s, err = run_clip(path_s, 4, metric="max_mod")
    assert r_s is not None, f"small max_mod failed: {err}"
    r_l, err = run_clip(path_l, 4, metric="max_mod")
    assert r_l is not None, f"large max_mod failed: {err}"
    assert abs(r_s["min_score"] - 2.0) < 1e-6
    assert abs(r_l["min_score"] - 5.0) < 1e-6
    assert r_l["min_score"] > r_s["min_score"]
    for p in [path_s, path_l]:
        os.remove(p)


def test_min_angular_separation_ranking():
    """Evenly spaced angles should have higher min_angular_separation than clustered ones."""
    even = [(1.0, 0.0), (0.0, 1.0), (-1.0, 0.0), (0.0, -1.0)]
    clustered = [(1.0, 0.0), (math.cos(0.02), math.sin(0.02)), (-1.0, 0.0), (0.0, -1.0)]
    path_e = "/tmp/sp_test_min_ang_even.bin"
    path_c = "/tmp/sp_test_min_ang_clustered.bin"
    write_bin(path_e, [even], 4)
    write_bin(path_c, [clustered], 4)
    r_e, err = run_clip(path_e, 4, metric="min_angular_separation")
    assert r_e is not None, f"even min_angular_separation failed: {err}"
    r_c, err = run_clip(path_c, 4, metric="min_angular_separation")
    assert r_c is not None, f"clustered min_angular_separation failed: {err}"
    assert r_e["min_score"] > r_c["min_score"]
    assert abs(r_e["min_score"] - (math.pi / 2.0)) < 1e-6
    for p in [path_e, path_c]:
        os.remove(p)


# ================================================================
# 11. Hist for clusteriness
# ================================================================

def test_hist_clusteriness():
    """Histogram with clusteriness metric — counts sum, JSON has metric."""
    uniform = [(0.0, 0.0), (1.0, 0.0), (2.0, 0.0), (3.0, 0.0)]
    one_pair = [(0.0, 0.0), (0.001, 0.0), (1.0, 0.0), (2.0, 0.0)]
    path = "/tmp/sp_test_hist_cluster.bin"
    write_bin(path, [uniform, one_pair], 4)
    clip_r, err = run_clip(path, 4, metric="clusteriness")
    assert clip_r is not None, f"clusteriness clip failed: {err}"
    hist_r, err = run_hist(path, 4, clip_r["clip_lo"], clip_r["clip_hi"],
                           hist_bins=10, metric="clusteriness")
    assert hist_r is not None, f"clusteriness hist failed: {err}"
    assert hist_r["metric"] == "clusteriness"
    assert hist_r["n_solves"] == 2
    assert len(hist_r["hist"]) == 10
    assert sum(hist_r["hist"]) == 2
    os.remove(path)


def test_hist_centroid_dist():
    """Histogram with centroid_dist metric — counts sum, JSON has metric."""
    near = [(-0.5, 0.0), (0.5, 0.0), (0.0, 0.5), (0.0, -0.5)]
    far = [(4.5, 0.0), (5.5, 0.0), (5.0, 0.5), (5.0, -0.5)]
    path = "/tmp/sp_test_hist_centroid_dist.bin"
    write_bin(path, [near, far], 4)
    clip_r, err = run_clip(path, 4, metric="centroid_dist")
    assert clip_r is not None, f"centroid_dist clip failed: {err}"
    hist_r, err = run_hist(path, 4, clip_r["clip_lo"], clip_r["clip_hi"],
                           hist_bins=10, metric="centroid_dist")
    assert hist_r is not None, f"centroid_dist hist failed: {err}"
    assert hist_r["metric"] == "centroid_dist"
    assert hist_r["n_solves"] == 2
    assert sum(hist_r["hist"]) == 2
    os.remove(path)


# ================================================================
# 12. Summary mode
# ================================================================

def run_summary(bin_path, degree, metric="proximity", quantile_lo="0.001", quantile_hi="0.999", **kwargs):
    """Run summary mode via Docker."""
    import shutil
    host_bin = os.path.join(LAMBDA_DIR, "_test_input.bin")
    host_coeff_bin = os.path.join(LAMBDA_DIR, "_test_coeff_input.bin")
    shutil.copy(bin_path, host_bin)
    coeff_local = kwargs.pop("_coeffs_local", None)
    coeff_degree = kwargs.pop("_coeff_degree", None)
    if coeff_local:
        shutil.copy(coeff_local, host_coeff_bin)
    try:
        args = (f"/src/solve_proximity_stats /src/_test_input.bin --mode=summary "
                f"--degree={degree} --metric={metric} "
                f"--quantile_lo={quantile_lo} --quantile_hi={quantile_hi}")
        if coeff_local:
            args += f" --score_coeffs_file=/src/_test_coeff_input.bin --score_coeff_degree={int(coeff_degree)}"
        for k, v in kwargs.items():
            args += f" --{k}={shlex.quote(str(v))}"
        r = _docker_run(args)
        if r.returncode != 0:
            return None, r.stderr
        return json.loads(r.stdout), None
    finally:
        try:
            os.remove(host_bin)
        except OSError:
            pass
        if coeff_local:
            try:
                os.remove(host_coeff_bin)
            except OSError:
                pass


def test_summary_all_fields():
    """Summary returns all required fields including final 10-bin data."""
    path = "/tmp/sp_test_summary.bin"
    solves = []
    for i in range(200):
        d = 0.001 + (i / 200.0) * 2.0
        solves.append([(0.0, 0.0), (d, 0.0)])
    write_bin(path, solves, 2)
    result, err = run_summary(path, 2, metric="proximity", quantile_lo="0.05", quantile_hi="0.95")
    assert result is not None, f"summary failed: {err}"
    assert result["mode"] == "summary"
    assert result["metric"] == "proximity"
    assert result["n_solves"] == 200
    assert result["degree"] == 2
    assert result["threads"] == 1
    for field in ["min_score", "max_score", "mean_score", "stddev_score",
                  "q05", "q10", "q25", "q50", "q75", "q90", "q95",
                  "clip_lo", "clip_hi", "full_range", "clip_range",
                  "clip_below_count", "clip_inrange_count", "clip_above_count",
                  "clip_below_frac", "clip_inrange_frac", "clip_above_frac",
                  "clip_fallback", "clip_fallback_reason",
                  "metric_validity_policy", "metric_min_finite_roots", "total_root_slots", "finite_root_count",
                  "fully_finite_solve_count", "partial_finite_solve_count", "zero_finite_solve_count",
                  "usable_solve_count", "forced_zero_score_count", "finite_root_frac",
                  "fully_finite_solve_frac", "partial_finite_solve_frac", "zero_finite_solve_frac", "usable_solve_frac",
                  "exact_zero_root_count", "rows_with_any_exact_zero_root_count", "rows_all_exact_zero_roots_count",
                  "exact_zero_root_frac", "rows_with_any_exact_zero_root_frac", "rows_all_exact_zero_roots_frac",
                  "mean_finite_roots_per_solve", "min_finite_roots_per_solve", "max_finite_roots_per_solve",
                  "raw_hist_bins", "raw_hist_lo", "raw_hist_hi", "raw_hist_range", "raw_hist_space", "raw_hist_expanded",
                  "raw_bin_counts", "raw_bin_fracs",
                  "intermediate_hist_bins", "final_bins",
                  "cuts_norm", "cuts_score", "final_bin_counts", "final_bin_fracs",
                  "min_score_count", "max_score_count", "clip_lo_count", "clip_hi_count",
                  "n_unique_scores"]:
        assert field in result, f"missing field: {field}"
    assert result["raw_hist_bins"] == 32
    assert result["raw_hist_space"] == "metric_raw"
    assert len(result["raw_bin_counts"]) == 32
    assert len(result["raw_bin_fracs"]) == 32
    assert sum(result["raw_bin_counts"]) == result["n_solves"], \
        f"raw bin sum {sum(result['raw_bin_counts'])} != n_solves {result['n_solves']}"
    assert 0 <= result["rows_with_any_exact_zero_root_count"] <= result["n_solves"]
    assert 0 <= result["rows_all_exact_zero_roots_count"] <= result["n_solves"]
    assert result["intermediate_hist_bins"] == 100
    assert result["final_bins"] == 10
    assert len(result["cuts_norm"]) == 9
    assert len(result["cuts_score"]) == 9
    assert len(result["final_bin_counts"]) == 10
    assert len(result["final_bin_fracs"]) == 10
    # final bin counts must sum to clip_inrange_count
    assert sum(result["final_bin_counts"]) == result["clip_inrange_count"], \
        f"bin sum {sum(result['final_bin_counts'])} != inrange {result['clip_inrange_count']}"
    # No hist_full field (removed)
    assert "hist_full" not in result, "hist_full should be removed from summary"
    os.remove(path)


def test_summary_reports_requested_threads():
    path = "/tmp/sp_test_summary_threads.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C, SOLVE_A], 2)
    result, err = run_summary(path, 2, metric="proximity", threads=2)
    assert result is not None, f"summary failed: {err}"
    assert result["threads"] == 2
    os.remove(path)


def test_summary_reports_finite_root_diagnostics():
    """Summary exposes finite-root / forced-zero diagnostics for partial-invalid solves."""
    path = "/tmp/sp_test_summary_finite_diag.bin"
    degree = 4
    solves = [
        [(0.0, 0.0), (1.0, 0.0), (0.0, 1.0), (-1.0, 0.0)],
        [(float("inf"), 0.0), (1.0, 0.0), (0.0, 1.0), (-1.0, 0.0)],
        [(float("inf"), 0.0), (float("nan"), 0.0), (float("inf"), 1.0), (0.0, float("nan"))],
    ]
    write_bin(path, solves, degree)
    r, err = run_summary(path, degree, metric="asymmetry_re")
    assert r is not None, f"summary failed: {err}"
    assert r["metric_validity_policy"] == "finite_only_min_roots"
    assert r["metric_min_finite_roots"] == 1
    assert r["total_root_slots"] == 12
    assert r["finite_root_count"] == 7
    assert r["fully_finite_solve_count"] == 1
    assert r["partial_finite_solve_count"] == 1
    assert r["zero_finite_solve_count"] == 1
    assert r["usable_solve_count"] == 2
    assert r["forced_zero_score_count"] == 1
    assert abs(r["finite_root_frac"] - (7.0 / 12.0)) < 1e-6
    assert abs(r["usable_solve_frac"] - (2.0 / 3.0)) < 1e-6
    assert abs(r["mean_finite_roots_per_solve"] - (7.0 / 3.0)) < 1e-6
    assert r["min_finite_roots_per_solve"] == 0
    assert r["max_finite_roots_per_solve"] == 4
    os.remove(path)


def test_summary_reports_exact_zero_root_diagnostics():
    """Summary exposes exact-zero root and row counts for placeholder visibility."""
    path = "/tmp/sp_test_summary_zero_roots.bin"
    degree = 4
    solves = [
        [(0.0, 0.0), (1.0, 0.0), (2.0, 0.0), (3.0, 0.0)],
        [(0.0, 0.0), (0.0, 0.0), (0.0, 0.0), (0.0, 0.0)],
        [(float("inf"), 0.0), (0.0, 0.0), (1.0, 0.0), (2.0, 0.0)],
    ]
    write_bin(path, solves, degree)
    r, err = run_summary(path, degree, metric="proximity")
    assert r is not None, f"summary failed: {err}"
    assert r["exact_zero_root_count"] == 6
    assert r["rows_with_any_exact_zero_root_count"] == 3
    assert r["rows_all_exact_zero_roots_count"] == 1
    assert abs(r["exact_zero_root_frac"] - (6.0 / 12.0)) < 1e-6
    assert abs(r["rows_with_any_exact_zero_root_frac"] - 1.0) < 1e-6
    assert abs(r["rows_all_exact_zero_roots_frac"] - (1.0 / 3.0)) < 1e-6
    os.remove(path)


def test_summary_mixed_source_program_uses_coeff_vectors():
    solve_path = "/tmp/sp_test_summary_mixed_source_solves.bin"
    coeff_path = "/tmp/sp_test_summary_mixed_source_coeffs.bin"
    solve_rows = [
        [(0.0, 0.0), (1.0, 0.0)],
        [(0.0, 0.0), (1.0, 0.0)],
    ]
    coeff_rows = [
        [(0.0, 0.0), (0.0, 0.0), (0.0, 0.0)],
        [(-1.0, 0.0), (0.0, 0.0), (1.0, 0.0)],
    ]
    write_bin(solve_path, solve_rows, 2)
    write_bin(coeff_path, coeff_rows, 3)
    result, err = run_summary(
        solve_path,
        2,
        metric="spread",
        score_metrics="spread,spread",
        score_sources="slv,cf",
        score_clip_los="-0.5,-150",
        score_clip_his="0,0",
        score_program="m0;m1;avg",
        _coeffs_local=coeff_path,
        _coeff_degree=3,
    )
    assert result is not None, f"mixed-source summary failed: {err}"
    assert result["raw_hist_space"] == "program_output"
    assert result["n_solves"] == 2
    assert 0.15 <= result["min_score"] <= 0.25, result["min_score"]
    assert 0.65 <= result["max_score"] <= 0.75, result["max_score"]
    os.remove(solve_path)
    os.remove(coeff_path)


def test_hist_param_program_does_not_require_legacy_clip_range():
    solve_path = "/tmp/sp_test_hist_param_program.bin"
    params_path = os.path.join(LAMBDA_DIR, "_test_param_input.bin")
    solve_rows = [
        [(0.0, 0.0), (1.0, 0.0)],
        [(0.0, 0.0), (1.0, 0.0)],
        [(0.0, 0.0), (1.0, 0.0)],
        [(0.0, 0.0), (1.0, 0.0)],
    ]
    param_rows = [
        (0.10, 0.20, 0.0, 0.0),
        (0.25, 0.15, 0.0, 0.0),
        (0.40, 0.50, 0.0, 0.0),
        (0.75, 0.80, 0.0, 0.0),
    ]
    write_bin(solve_path, solve_rows, 2)
    with open(params_path, "wb") as f:
        for row in param_rows:
            f.write(struct.pack("<ffff", *row))
    try:
        host_bin = os.path.join(LAMBDA_DIR, "_test_input.bin")
        shutil.copy(solve_path, host_bin)
        try:
            args = (
                "/src/solve_proximity_stats /src/_test_input.bin --mode=hist "
                "--degree=2 --hist_bins=8 --metric=t1_re "
                "--score_metrics=t1_re,t1_im "
                "--score_sources=pm,pm "
                "--score_clip_los=0,0 "
                "--score_clip_his=1,1 "
                f"--score_program={shlex.quote('m0;m1;mul')} "
                "--score_params_file=/src/_test_param_input.bin"
            )
            r = _docker_run(args)
            assert r.returncode == 0, r.stderr
            result = json.loads(r.stdout)
            assert result["metric"] == "t1_re"
            assert result["clip_lo"] == 0.0
            assert result["clip_hi"] == 1.0
            assert sum(result["hist"]) == 4
        finally:
            try:
                os.remove(host_bin)
            except OSError:
                pass
    finally:
        try:
            os.remove(solve_path)
        except OSError:
            pass
        try:
            os.remove(params_path)
        except OSError:
            pass


def test_clip_centroid_re_uses_only_finite_roots():
    """Centroid metrics should score partial-invalid solves from their finite roots."""
    path = "/tmp/sp_test_clip_centroid_partial.bin"
    degree = 4
    solves = [
        [(1.0, 0.0), (3.0, 0.0), (float("inf"), 0.0), (float("nan"), 0.0)],
        [(-2.0, 0.0), (0.0, 0.0), (float("inf"), 0.0), (float("nan"), 0.0)],
    ]
    write_bin(path, solves, degree)
    r, err = run_clip(path, degree, metric="centroid_re")
    assert r is not None, f"clip failed: {err}"
    assert abs(r["min_score"] - (-1.0)) < 1e-6, r
    assert abs(r["max_score"] - 2.0) < 1e-6, r
    os.remove(path)


def test_clip_proximity_requires_two_finite_roots():
    """Pairwise metrics still require enough finite roots after filtering."""
    path = "/tmp/sp_test_clip_proximity_partial.bin"
    degree = 4
    solves = [
        [(0.0, 0.0), (0.01, 0.0), (float("inf"), 0.0), (float("nan"), 0.0)],
        [(7.0, 0.0), (float("inf"), 0.0), (float("inf"), 0.0), (float("nan"), 0.0)],
    ]
    write_bin(path, solves, degree)
    r, err = run_clip(path, degree, metric="proximity")
    assert r is not None, f"clip failed: {err}"
    assert abs(r["min_score"] - 0.0) < 1e-6, r
    assert r["max_score"] > 1.9, r
    os.remove(path)


def test_summary_centroid_re_smoke():
    """Summary mode works for signed centroid_re scores."""
    path = "/tmp/sp_test_summary_centroid_re.bin"
    solves = [
        [(-2.0, 0.0), (-1.0, 0.0)],
        [(1.0, 0.0), (2.0, 0.0)],
        [(3.0, 0.0), (4.0, 0.0)],
    ]
    write_bin(path, solves, 2)
    result, err = run_summary(path, 2, metric="centroid_re")
    assert result is not None, f"centroid_re summary failed: {err}"
    assert result["metric"] == "centroid_re"
    assert result["n_solves"] == 3
    assert math.isfinite(result["mean_score"])
    assert math.isfinite(result["clip_lo"])
    assert math.isfinite(result["clip_hi"])
    os.remove(path)


def test_summary_dist_unit_circle_smoke():
    """Summary mode works for logged dist_unit_circle scores."""
    path = "/tmp/sp_test_summary_dist_unit_circle.bin"
    solves = [
        [(1.0, 0.0), (-1.0, 0.0), (0.0, 1.0), (0.0, -1.0)],
        [(2.0, 0.0), (2.0, 0.0), (2.0, 0.0), (2.0, 0.0)],
    ]
    write_bin(path, solves, 4)
    result, err = run_summary(path, 4, metric="dist_unit_circle")
    assert result is not None, f"dist_unit_circle summary failed: {err}"
    assert result["metric"] == "dist_unit_circle"
    assert result["n_solves"] == 2
    assert math.isfinite(result["mean_score"])
    assert math.isfinite(result["clip_lo"])
    assert math.isfinite(result["clip_hi"])
    os.remove(path)


def test_summary_min_angular_separation_smoke():
    """Summary mode works for min_angular_separation."""
    path = "/tmp/sp_test_summary_min_ang.bin"
    solves = [
        [(1.0, 0.0), (0.0, 1.0), (-1.0, 0.0), (0.0, -1.0)],
        [(1.0, 0.0), (math.cos(0.05), math.sin(0.05)), (-1.0, 0.0), (0.0, -1.0)],
    ]
    write_bin(path, solves, 4)
    result, err = run_summary(path, 4, metric="min_angular_separation")
    assert result is not None, f"min_angular_separation summary failed: {err}"
    assert result["metric"] == "min_angular_separation"
    assert result["n_solves"] == 2
    assert result["max_score"] > result["min_score"]
    os.remove(path)


def test_summary_quantiles_monotone():
    """Quantiles are monotone: min <= q05 <= ... <= q95 <= max."""
    path = "/tmp/sp_test_summary_mono.bin"
    solves = [[(0.0, 0.0), (d, 0.0)] for d in [0.001 + i * 0.01 for i in range(200)]]
    write_bin(path, solves, 2)
    r, _ = run_summary(path, 2)
    assert r is not None
    vals = [r["min_score"], r["q05"], r["q10"], r["q25"], r["q50"], r["q75"], r["q90"], r["q95"], r["max_score"]]
    for i in range(len(vals) - 1):
        assert vals[i] <= vals[i + 1] + 1e-10, f"not monotone at index {i}: {vals}"
    os.remove(path)


def test_summary_final_bins_sum_to_inrange():
    """Final 10-bin counts sum to clip_inrange_count (not n_solves)."""
    path = "/tmp/sp_test_summary_bsum.bin"
    solves = [[(0.0, 0.0), (0.001 + i * 0.01, 0.0)] for i in range(200)]
    write_bin(path, solves, 2)
    r, _ = run_summary(path, 2, quantile_lo="0.05", quantile_hi="0.95")
    assert r is not None
    assert len(r["final_bin_counts"]) == 10
    assert sum(r["final_bin_counts"]) == r["clip_inrange_count"]
    os.remove(path)


def test_summary_occupancy_sum():
    """Clip occupancy counts sum to n_solves."""
    path = "/tmp/sp_test_summary_occ.bin"
    solves = [[(0.0, 0.0), (0.001 + i * 0.01, 0.0)] for i in range(200)]
    write_bin(path, solves, 2)
    r, _ = run_summary(path, 2, quantile_lo="0.05", quantile_hi="0.95")
    assert r is not None
    total = r["clip_below_count"] + r["clip_inrange_count"] + r["clip_above_count"]
    assert total == r["n_solves"], f"occupancy sum {total} != n_solves {r['n_solves']}"
    os.remove(path)


def test_summary_fallback_on_small_sample():
    """Small sample triggers fallback."""
    path = "/tmp/sp_test_summary_small.bin"
    write_bin(path, [SOLVE_A, SOLVE_B], 2)
    r, _ = run_summary(path, 2)
    assert r is not None
    assert r["clip_fallback"] is True
    assert r["clip_fallback_reason"] == "small_sample"
    os.remove(path)


def test_summary_anisotropy_near_singular_covariance_is_finite():
    """Near-rank-1 covariance should not produce NaN anisotropy summary stats."""
    path = "/tmp/sp_test_summary_aniso_finite.bin"
    solves = []
    degree = 35
    for _ in range(200):
        solve = []
        for k in range(degree):
            x = (k - 17) * 1e5
            y = x + ((-1) ** k) * 1e-4
            solve.append((x, y))
        solves.append(solve)
    write_bin(path, solves, degree)
    r, _ = run_summary(path, degree, metric="anisotropy")
    assert r is not None
    for field in ["min_score", "max_score", "mean_score", "stddev_score",
                  "q05", "q10", "q25", "q50", "q75", "q90", "q95",
                  "clip_lo", "clip_hi", "full_range", "clip_range"]:
        assert math.isfinite(r[field]), f"{field} should be finite, got {r[field]!r}"
    os.remove(path)


def test_summary_anisotropy_nonfinite_roots_is_finite():
    """Non-finite roots in the input must not poison anisotropy summary JSON."""
    path = "/tmp/sp_test_summary_aniso_inf.bin"
    degree = 35
    solves = []
    for s in range(200):
        solve = []
        for k in range(degree):
            if s == 0 and k == 0:
                solve.append((float("inf"), 0.0))
            else:
                x = (k - 17) * 1.0
                y = x * 0.5
                solve.append((x, y))
        solves.append(solve)
    write_bin(path, solves, degree)
    r, _ = run_summary(path, degree, metric="anisotropy")
    assert r is not None
    for field in ["min_score", "max_score", "mean_score", "stddev_score",
                  "q05", "q10", "q25", "q50", "q75", "q90", "q95",
                  "clip_lo", "clip_hi", "full_range", "clip_range"]:
        assert math.isfinite(r[field]), f"{field} should be finite, got {r[field]!r}"
    os.remove(path)


# ================================================================
# 13. Clip quantile narrows range
# ================================================================

def test_clip_quantile_narrows_range():
    """Wider quantile clip (q=0.05) produces strictly narrower range than q=0.001."""
    path = "/tmp/sp_test_quantile.bin"
    solves = []
    for i in range(200):
        d = 0.001 + (i / 200.0) * 2.0
        solves.append([(0.0, 0.0), (d, 0.0)])
    write_bin(path, solves, 2)

    r_narrow, err = run_clip(path, 2, metric="proximity",
                             quantile_lo="0.001", quantile_hi="0.999")
    assert r_narrow is not None, f"narrow clip failed: {err}"

    r_wide, err = run_clip(path, 2, metric="proximity",
                           quantile_lo="0.05", quantile_hi="0.95")
    assert r_wide is not None, f"wide clip failed: {err}"

    # q=0.05 should produce a narrower [clip_lo, clip_hi] range
    narrow_range = r_narrow["clip_hi"] - r_narrow["clip_lo"]
    wide_range = r_wide["clip_hi"] - r_wide["clip_lo"]
    assert wide_range < narrow_range, \
        f"q=0.05 range ({wide_range}) should be narrower than q=0.001 range ({narrow_range})"
    os.remove(path)


# ================================================================
# 13. Hist for non-proximity metric (spread)
# ================================================================

def test_hist_spread():
    """Histogram with spread metric — counts sum correctly, JSON has metric."""
    small = [(0.0, 0.1), (0.0, -0.1), (0.1, 0.0), (-0.1, 0.0)]
    large = [(0.0, 10.0), (0.0, -10.0), (10.0, 0.0), (-10.0, 0.0)]
    mid = [(0.0, 1.0), (0.0, -1.0), (1.0, 0.0), (-1.0, 0.0)]

    path = "/tmp/sp_test_hist_spread.bin"
    write_bin(path, [small, mid, large], 4)

    # Get clip bounds first
    clip_result, err = run_clip(path, 4, metric="spread")
    assert clip_result is not None, f"spread clip failed: {err}"

    hist_result, err = run_hist(path, 4, clip_result["clip_lo"], clip_result["clip_hi"],
                                hist_bins=10, metric="spread")
    assert hist_result is not None, f"spread hist failed: {err}"
    assert hist_result["metric"] == "spread"
    assert hist_result["n_solves"] == 3
    assert len(hist_result["hist"]) == 10
    assert sum(hist_result["hist"]) == 3
    os.remove(path)


# ================================================================
# 7. Invalid metric fails
# ================================================================

def test_invalid_metric():
    """Nonexistent metric name → nonzero exit code."""
    path = "/tmp/sp_test_invalid_metric.bin"
    write_bin(path, [SOLVE_A], 2)
    result, err = run_clip(path, 2, metric="bogus")
    assert result is None, f"invalid metric should fail, got: {result}"
    os.remove(path)


# ================================================================
# 8. Root-transform metric test
# ================================================================

def test_root_transform_affects_score():
    """Root transforms change the metric score (pull_towards_center changes spread)."""
    # Roots at distance ~1-2 from origin
    solve = [(0.0, 0.0), (1.0, 0.0), (2.0, 0.0), (0.0, 1.5)]
    path = "/tmp/sp_test_xform.bin"
    write_bin(path, [solve], 4)
    xforms_host = os.path.join(LAMBDA_DIR, "_test_xforms.json")

    try:
        # Without transforms
        result_plain, err = run_clip(path, 4, metric="spread")
        assert result_plain is not None, f"plain spread failed: {err}"

        # With pull_towards_center(alpha=0.9, sigma=2.0) — shrinks roots toward origin
        # Format: [["name", "arg1", "arg2"]]
        write_xforms(xforms_host, [["pull_towards_center", "0.9", "2.0"]])
        result_pulled, err = run_clip(path, 4, metric="spread",
                                      root_xforms="/src/_test_xforms.json")
        assert result_pulled is not None, f"pulled spread failed: {err}"

        # pull_towards_center shrinks → spread should decrease
        assert result_pulled["min_score"] != result_plain["min_score"], \
            f"transform should change score: plain={result_plain['min_score']}, pulled={result_pulled['min_score']}"
    finally:
        for p in [path, xforms_host]:
            try:
                os.remove(p)
            except OSError:
                pass


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

    print("solve_proximity_stats tests — all metrics (Docker ARM64)")
    print("=" * 50)

    tests = [
        # Proximity reference
        ("proximity clip basic", test_clip_basic),
        ("proximity clip many solves", test_clip_many_solves),
        ("proximity hist basic", test_hist_basic),
        ("proximity hist 100 bins", test_hist_100_bins),
        ("proximity identical roots", test_identical_roots),
        ("proximity degree 3", test_degree_3),
        ("proximity empty file", test_empty_file),
        ("proximity invalid clip range", test_invalid_clip_range),
        # Metric ranking (v1)
        ("crowding ranking", test_crowding_ranking),
        ("spread ranking", test_spread_ranking),
        ("anisotropy ranking", test_anisotropy_ranking),
        ("area ranking", test_area_ranking),
        # Metric ranking (v2)
        ("clusteriness ranking", test_clusteriness_ranking),
        ("shelliness ranking", test_shelliness_ranking),
        ("outlierness ranking", test_outlierness_ranking),
        ("nn_variation ranking", test_nn_variation_ranking),
        ("real_axis_proximity ranking", test_real_axis_proximity_ranking),
        ("centroid_re degree 1", test_centroid_re_degree_one_allowed),
        ("centroid_re ranking", test_centroid_re_ranking),
        ("centroid_im ranking", test_centroid_im_ranking),
        ("centroid_dist ranking", test_centroid_dist_ranking),
        ("dist_unit_circle ranking", test_dist_unit_circle_ranking),
        ("asymmetry_re ranking", test_asymmetry_re_ranking),
        ("min_mod ranking", test_min_mod_ignores_zeros_and_ranking),
        ("min_mod all zero", test_min_mod_all_zero_returns_zero),
        ("max_mod ranking", test_max_mod_ranking),
        ("min_angular_separation ranking", test_min_angular_separation_ranking),
        # Non-proximity hist
        ("hist clusteriness metric", test_hist_clusteriness),
        ("hist centroid_dist metric", test_hist_centroid_dist),
        ("hist spread metric", test_hist_spread),
        # Summary mode
        ("summary all fields", test_summary_all_fields),
        ("summary threads passthrough", test_summary_reports_requested_threads),
        ("summary exact zero root diagnostics", test_summary_reports_exact_zero_root_diagnostics),
        ("summary mixed-source coeff vectors", test_summary_mixed_source_program_uses_coeff_vectors),
        ("summary centroid_re smoke", test_summary_centroid_re_smoke),
        ("summary dist_unit_circle smoke", test_summary_dist_unit_circle_smoke),
        ("summary min_angular_separation smoke", test_summary_min_angular_separation_smoke),
        ("summary quantiles monotone", test_summary_quantiles_monotone),
        ("summary final bins sum to inrange", test_summary_final_bins_sum_to_inrange),
        ("summary occupancy sum", test_summary_occupancy_sum),
        ("summary fallback small", test_summary_fallback_on_small_sample),
        ("summary anisotropy finite near singular covariance", test_summary_anisotropy_near_singular_covariance_is_finite),
        ("summary anisotropy finite with non-finite roots", test_summary_anisotropy_nonfinite_roots_is_finite),
        # Quantile tests
        ("clip quantile narrows range", test_clip_quantile_narrows_range),
        # Error handling
        ("invalid metric", test_invalid_metric),
        # Root transforms
        ("root transform affects score", test_root_transform_affects_score),
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
