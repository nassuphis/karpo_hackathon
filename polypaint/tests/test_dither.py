"""
Test dither param transforms: sdith, ddith, ndith.

Uses sweep_test in param_dump mode to generate transformed params,
then verifies statistical properties.

Run: cd polypaint/tests && uv run python test_dither.py
"""
import json
import math
import os
import struct
import subprocess
import numpy as np

from tests.native_program_helpers import translate_legacy_transforms_for_native

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def run_param_dump(n, transforms):
    """Run sweep in param_dump mode, return array of (z1r, z1i, z2r, z2i)."""
    spec = json.dumps(translate_legacy_transforms_for_native({
        "mode": "param_dump",
        "n1": n, "n2": n,
        "param_transforms": transforms,
    }))
    out_path = "/tmp/test_dither.bin"
    r = subprocess.run([SWEEP, out_path], input=spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"param_dump failed: {r.stderr}"

    with open(out_path, "rb") as f:
        data = f.read()
    os.remove(out_path)

    n_points = len(data) // 16
    result = []
    for i in range(n_points):
        z1r, z1i, z2r, z2i = struct.unpack_from('<ffff', data, i * 16)
        result.append((z1r, z1i, z2r, z2i))
    return result, n


def offsets(points, n):
    """Compute offsets from grid positions for all points."""
    d1_re, d1_im, d2_re, d2_im = [], [], [], []
    radial1, radial2 = [], []
    for idx, (z1r, z1i, z2r, z2i) in enumerate(points):
        i1 = idx // n
        i2 = idx % n
        x1 = i1 / n
        x2 = i2 / n
        dr1 = z1r - x1
        di1 = z1i
        dr2 = z2r - x2
        di2 = z2i
        d1_re.append(dr1)
        d1_im.append(di1)
        d2_re.append(dr2)
        d2_im.append(di2)
        radial1.append(math.sqrt(dr1**2 + di1**2))
        radial2.append(math.sqrt(dr2**2 + di2**2))
    return d1_re, d1_im, d2_re, d2_im, radial1, radial2


def test_sdith_bounded():
    """sdith(1): all offsets within ±d/(2N) per component."""
    print("test_sdith_bounded...")
    d = 1.0
    n = 200
    points, _ = run_param_dump(n, [["sdith", str(d)]])
    d1_re, d1_im, d2_re, d2_im, _, _ = offsets(points, n)

    half_w = d / (2 * n)  # 0.0025
    tol = half_w * 1.01  # tiny float tolerance

    assert max(abs(x) for x in d1_re) <= tol, f"sdith t1.re exceeded bound"
    assert max(abs(x) for x in d1_im) <= tol, f"sdith t1.im exceeded bound"
    assert max(abs(x) for x in d2_re) <= tol, f"sdith t2.re exceeded bound"
    assert max(abs(x) for x in d2_im) <= tol, f"sdith t2.im exceeded bound"

    # Verify all 4 components are actually dithered (not zero)
    assert any(abs(x) > 1e-6 for x in d1_im), "sdith t1.im should be dithered"
    assert any(abs(x) > 1e-6 for x in d2_im), "sdith t2.im should be dithered"

    print(f"  max offsets: re1={max(abs(x) for x in d1_re):.6f} im1={max(abs(x) for x in d1_im):.6f}")
    print(f"  bound: {half_w:.6f}")
    print("  PASS")


def test_sdith_scaling():
    """sdith(2) should have twice the spread of sdith(1)."""
    print("test_sdith_scaling...")
    n = 200
    pts1, _ = run_param_dump(n, [["sdith", "1"]])
    pts2, _ = run_param_dump(n, [["sdith", "2"]])

    d1_re_1, _, _, _, _, _ = offsets(pts1, n)
    d1_re_2, _, _, _, _, _ = offsets(pts2, n)

    std1 = np.std(d1_re_1)
    std2 = np.std(d1_re_2)
    ratio = std2 / std1

    assert 1.5 < ratio < 2.5, f"sdith scaling ratio {ratio:.2f} not near 2.0"
    print(f"  std(d=1)={std1:.6f} std(d=2)={std2:.6f} ratio={ratio:.2f}")
    print("  PASS")


def test_ddith_bounded():
    """ddith(both,1,0.5): all offsets within disk of radius d/N."""
    print("test_ddith_bounded...")
    d = 1.0
    n = 200
    points, _ = run_param_dump(n, [["ddith", "2", str(d), "0.5"]])
    _, _, _, _, radial1, radial2 = offsets(points, n)

    rmax = d / n  # 0.005
    tol = rmax * 1.01

    assert max(radial1) <= tol, f"ddith t1 radial exceeded {max(radial1):.6f} > {rmax:.6f}"
    assert max(radial2) <= tol, f"ddith t2 radial exceeded {max(radial2):.6f} > {rmax:.6f}"

    # Not concentrated at center (sqrt sampling with exp=0.5)
    median1 = sorted(radial1)[len(radial1) // 2]
    assert median1 > rmax * 0.4, f"ddith median too small {median1:.6f}, may be biased toward center"

    print(f"  max radial: t1={max(radial1):.6f} t2={max(radial2):.6f} (limit={rmax:.6f})")
    print(f"  median radial: {median1:.6f} (should be ~{rmax * 0.7:.6f})")
    print("  PASS")


def test_ddith_target_t1_only():
    """ddith(0,1,0.5): only t1 is dithered, t2 unchanged."""
    print("test_ddith_target_t1_only...")
    n = 100
    points, _ = run_param_dump(n, [["ddith", "0", "1", "0.5"]])
    _, _, _, _, radial1, radial2 = offsets(points, n)
    assert max(radial1) > 0, "t1 should be dithered"
    assert max(radial2) < 1e-7, f"t2 should be unchanged, max radial={max(radial2):.2e}"
    print("  PASS")


def test_ddith_exponent():
    """ddith with exp=1 (linear radius) should be more center-biased than exp=0.5."""
    print("test_ddith_exponent...")
    n = 300
    # exp=0.5 (uniform area)
    pts_05, _ = run_param_dump(n, [["ddith", "2", "1", "0.5"]])
    _, _, _, _, rad1_05, _ = offsets(pts_05, n)
    # exp=1.0 (linear, biased toward center)
    pts_10, _ = run_param_dump(n, [["ddith", "2", "1", "1.0"]])
    _, _, _, _, rad1_10, _ = offsets(pts_10, n)
    median_05 = sorted(rad1_05)[len(rad1_05) // 2]
    median_10 = sorted(rad1_10)[len(rad1_10) // 2]
    # exp=1 should have smaller median (more points near center)
    assert median_10 < median_05, f"exp=1 median={median_10:.6f} should be < exp=0.5 median={median_05:.6f}"
    print(f"  median(exp=0.5)={median_05:.6f} median(exp=1.0)={median_10:.6f}")
    print("  PASS")


def test_ddith_isotropic():
    """ddith should be rotationally symmetric — similar spread in re and im."""
    print("test_ddith_isotropic...")
    n = 300
    points, _ = run_param_dump(n, [["ddith", "2", "1", "0.5"]])
    d1_re, d1_im, _, _, _, _ = offsets(points, n)

    std_re = np.std(d1_re)
    std_im = np.std(d1_im)
    ratio = std_re / std_im if std_im > 0 else float('inf')

    assert 0.7 < ratio < 1.4, f"ddith not isotropic: re/im std ratio = {ratio:.2f}"
    print(f"  std_re={std_re:.6f} std_im={std_im:.6f} ratio={ratio:.2f}")
    print("  PASS")


def test_ndith_gaussian():
    """ndith(1): offsets should be approximately normal with sigma = d/N."""
    print("test_ndith_gaussian...")
    d = 1.0
    n = 200
    points, _ = run_param_dump(n, [["ndith", str(d)]])
    d1_re, d1_im, d2_re, d2_im, _, _ = offsets(points, n)

    sigma = d / n  # 0.005
    all_offsets = d1_re + d1_im + d2_re + d2_im

    measured_std = np.std(all_offsets)
    ratio = measured_std / sigma

    # Should be near 1.0
    assert 0.7 < ratio < 1.3, f"ndith std ratio {ratio:.2f} not near 1.0"

    # Verify all 4 components dithered
    assert np.std(d1_im) > sigma * 0.5, "ndith t1.im should be dithered"
    assert np.std(d2_im) > sigma * 0.5, "ndith t2.im should be dithered"

    print(f"  expected sigma={sigma:.6f} measured={measured_std:.6f} ratio={ratio:.2f}")
    print("  PASS")


def test_ndith_unbounded():
    """ndith should occasionally produce offsets larger than d/N."""
    print("test_ndith_unbounded...")
    n = 100
    points, _ = run_param_dump(n, [["ndith", "1"]])
    _, _, _, _, radial1, _ = offsets(points, n)

    rmax_sdith = 1.0 / n
    exceeds = sum(1 for r in radial1 if r > rmax_sdith)

    # With Gaussian, ~32% of points should exceed 1 sigma in radius
    assert exceeds > len(radial1) * 0.1, f"ndith too bounded: only {exceeds} exceed d/N"
    print(f"  {exceeds}/{len(radial1)} points exceed d/N (expected ~32%)")
    print("  PASS")


def test_identity_no_dither():
    """Empty transform chain: no dither, exact grid positions."""
    print("test_identity_no_dither...")
    n = 10
    points, _ = run_param_dump(n, [])
    d1_re, d1_im, d2_re, d2_im, _, _ = offsets(points, n)

    assert all(abs(x) < 1e-7 for x in d1_re), "identity should have zero re offset"
    assert all(abs(x) < 1e-7 for x in d1_im), "identity should have zero im offset"
    assert all(abs(x) < 1e-7 for x in d2_re), "identity should have zero re offset"
    assert all(abs(x) < 1e-7 for x in d2_im), "identity should have zero im offset"
    print("  PASS")


def test_adth_bounded():
    """adth(both,1,0.4): all offsets within outer radius, none inside inner."""
    print("test_adth_bounded...")
    n = 200
    points, _ = run_param_dump(n, [["adth", "2", "1", "0.4"]])
    _, _, _, _, radial1, radial2 = offsets(points, n)
    rmax = 1.0 / n
    tol = rmax * 1.01
    rmin = 0.4 * rmax
    assert max(radial1) <= tol, f"adth outer exceeded: {max(radial1):.6f} > {rmax:.6f}"
    assert max(radial2) <= tol, f"adth outer exceeded: {max(radial2):.6f} > {rmax:.6f}"
    # Statistically: inner radius should exclude most points below rmin
    below_inner = sum(1 for r in radial1 if r < rmin * 0.9)
    total = len(radial1)
    frac_below = below_inner / total
    assert frac_below < 0.05, f"adth too many inside inner ring: {frac_below:.2%}"
    print(f"  outer ok, {frac_below:.1%} below inner (expected <5%)")
    print("  PASS")


def test_adth_target_t1():
    """adth(0,...): only t1 dithered."""
    print("test_adth_target_t1...")
    n = 100
    points, _ = run_param_dump(n, [["adth", "0", "1", "0.3"]])
    _, _, _, _, radial1, radial2 = offsets(points, n)
    assert max(radial1) > 0, "t1 should be dithered"
    assert max(radial2) < 1e-7, f"t2 should be unchanged: {max(radial2):.2e}"
    print("  PASS")


def test_ldth_on_line():
    """ldth samples lie on the specified line direction."""
    print("test_ldth_on_line...")
    n = 300
    angle = 0.0  # horizontal line
    points, _ = run_param_dump(n, [["ldth", "2", "1", "1", str(angle)]])
    d1_re, d1_im, _, _, _, _ = offsets(points, n)
    # For angle=0, all offsets should be on real axis (im ≈ 0)
    max_im = max(abs(x) for x in d1_im)
    max_re = max(abs(x) for x in d1_re)
    assert max_im < 1e-10, f"ldth angle=0: imag should be 0, got {max_im:.2e}"
    assert max_re > 0, "ldth should produce real offsets"
    print(f"  max_re={max_re:.6f}, max_im={max_im:.2e}")
    print("  PASS")


def test_ldth_bounded():
    """ldth bounded by d*len/N."""
    print("test_ldth_bounded...")
    n = 200
    points, _ = run_param_dump(n, [["ldth", "2", "1", "0.5", "0"]])
    _, _, _, _, radial1, _ = offsets(points, n)
    half_len = 1.0 * 0.5 / n
    tol = half_len * 1.01
    assert max(radial1) <= tol, f"ldth exceeded: {max(radial1):.6f} > {half_len:.6f}"
    print("  PASS")


def test_crdth_on_axes():
    """crdth samples lie on horizontal or vertical axes."""
    print("test_crdth_on_axes...")
    n = 300
    points, _ = run_param_dump(n, [["crdth", "2", "1"]])
    d1_re, d1_im, _, _, _, _ = offsets(points, n)
    # Each sample should have either re=0 or im=0 (on one axis)
    on_axis = 0
    for re, im in zip(d1_re, d1_im):
        if abs(re) < 1e-6 or abs(im) < 1e-6:
            on_axis += 1
    frac = on_axis / len(d1_re)
    assert frac > 0.95, f"crdth: only {frac:.1%} on axes (expected >95%)"
    # Check roughly balanced
    h_count = sum(1 for re, im in zip(d1_re, d1_im) if abs(im) < 1e-8 and abs(re) > 1e-8)
    v_count = sum(1 for re, im in zip(d1_re, d1_im) if abs(re) < 1e-8 and abs(im) > 1e-8)
    total_nz = h_count + v_count
    if total_nz > 0:
        ratio = h_count / total_nz
        assert 0.3 < ratio < 0.7, f"crdth arm balance: h={h_count} v={v_count} ratio={ratio:.2f}"
    print(f"  on_axis={frac:.1%}, h={h_count} v={v_count}")
    print("  PASS")


def test_crdth_bounded():
    """crdth bounded by d/N."""
    print("test_crdth_bounded...")
    n = 200
    points, _ = run_param_dump(n, [["crdth", "2", "1"]])
    d1_re, d1_im, _, _, _, _ = offsets(points, n)
    half_len = 1.0 / n
    tol = half_len * 1.01
    assert max(abs(x) for x in d1_re) <= tol, "crdth re exceeded"
    assert max(abs(x) for x in d1_im) <= tol, "crdth im exceeded"
    print("  PASS")


def test_scdth_bounded():
    """scdth bounded by d/N radius."""
    print("test_scdth_bounded...")
    n = 200
    points, _ = run_param_dump(n, [["scdth", "2", "1", "0.25", "0"]])
    _, _, _, _, radial1, _ = offsets(points, n)
    rmax = 1.0 / n
    tol = rmax * 1.01
    assert max(radial1) <= tol, f"scdth exceeded: {max(radial1):.6f} > {rmax:.6f}"
    print("  PASS")


def test_scdth_sector():
    """scdth angles stay within sector bounds."""
    print("test_scdth_sector...")
    n = 300
    center = 0.0
    half_ap_frac = 0.25  # half_ap = pi/4
    points, _ = run_param_dump(n, [["scdth", "2", "1", str(half_ap_frac), str(center)]])
    d1_re, d1_im, _, _, radial1, _ = offsets(points, n)
    half_ap = math.pi * half_ap_frac
    # Check angles of non-zero offsets
    out_of_sector = 0
    total_nz = 0
    for re, im, r in zip(d1_re, d1_im, radial1):
        if r < 1e-12:
            continue
        total_nz += 1
        angle = math.atan2(im, re)
        diff = abs(angle - center)
        if diff > math.pi:
            diff = 2 * math.pi - diff
        if diff > half_ap * 1.01:
            out_of_sector += 1
    frac = out_of_sector / max(total_nz, 1)
    assert frac < 0.02, f"scdth: {frac:.1%} outside sector (expected <2%)"
    print(f"  {total_nz} nonzero, {frac:.1%} outside sector")
    print("  PASS")


if __name__ == "__main__":
    test_identity_no_dither()
    test_sdith_bounded()
    test_sdith_scaling()
    test_ddith_bounded()
    test_ddith_target_t1_only()
    test_ddith_exponent()
    test_ddith_isotropic()
    test_ndith_gaussian()
    test_ndith_unbounded()
    test_adth_bounded()
    test_adth_target_t1()
    test_ldth_on_line()
    test_ldth_bounded()
    test_crdth_on_axes()
    test_crdth_bounded()
    test_scdth_bounded()
    test_scdth_sector()
    print("\nAll dither tests passed.")
