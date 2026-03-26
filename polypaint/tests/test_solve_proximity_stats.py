"""
Tests for solve_proximity_stats binary — all 5 solve metrics.

Validates the algorithmic contract using the Docker runtime (deploy binary):
- proximity: clip/hist reference tests (existing)
- crowding: ranking test — globally clustered > spread-out
- spread: ranking test — large cloud > small cloud
- anisotropy: ranking test — line-like > isotropic
- area: ranking test — large 2D cloud > small 2D cloud
- hist for non-proximity metric (spread)
- invalid metric rejection
- root-transform metric test

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
        if xforms_host:
            try:
                os.remove(xforms_host)
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
    score_a = expected_proximity_score(SOLVE_A)
    score_b = expected_proximity_score(SOLVE_B)
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
    result, err = run_clip(path, 2, metric="proximity")
    assert result is not None, f"clip failed: {err}"
    assert result["n_solves"] == 200
    assert result["clip_lo"] < result["clip_hi"]
    assert result["clip_lo"] >= result["min_score"] - 1e-10
    assert result["clip_hi"] <= result["max_score"] + 1e-10
    os.remove(path)


def test_hist_basic():
    """Histogram with known proximity scores and clip bounds."""
    path = "/tmp/sp_test_hist.bin"
    write_bin(path, [SOLVE_A, SOLVE_B, SOLVE_C], 2)
    result, err = run_hist(path, 2, clip_lo=0.0, clip_hi=2.0, hist_bins=10, metric="proximity")
    assert result is not None, f"hist failed: {err}"
    assert result["n_solves"] == 3
    assert result["metric"] == "proximity"
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


# ================================================================
# 12. Clip quantile narrows range
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
        # Non-proximity hist
        ("hist clusteriness metric", test_hist_clusteriness),
        ("hist spread metric", test_hist_spread),
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
