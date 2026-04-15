"""
Test sweep param_dump mode: verifies transformed parameter output
matches expected values for various transform chains.

Run: cd polypaint/tests && uv run python test_param_dump.py
"""
import json
import cmath
import math
import os
import struct
import subprocess

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def run_param_dump(n, transforms):
    out_path = "/tmp/test_pdump.bin"
    spec = json.dumps({
        "mode": "param_dump",
        "n1": n, "n2": n,
        "param_transforms": transforms,
    })
    r = subprocess.run([SWEEP, out_path], input=spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"param_dump failed: {r.stderr}"
    meta = json.loads(r.stdout)
    assert meta["n_points"] == n * n
    assert meta["data_bytes"] == n * n * 16

    with open(out_path, "rb") as f:
        data = f.read()
    os.remove(out_path)
    return data, n


def get_point(data, n, i1, i2):
    offset = (i1 * n + i2) * 16
    return struct.unpack_from('<ffff', data, offset)


def test_identity():
    """Empty chain: z1=x1, z2=x2, no imaginary."""
    print("test_identity...")
    data, n = run_param_dump(10, [])
    for i1 in range(10):
        for i2 in range(10):
            z1r, z1i, z2r, z2i = get_point(data, n, i1, i2)
            x1, x2 = i1 / 10, i2 / 10
            assert abs(z1r - x1) < 1e-6, f"({i1},{i2}): z1r={z1r} != {x1}"
            assert abs(z1i) < 1e-6, f"({i1},{i2}): z1i={z1i} != 0"
            assert abs(z2r - x2) < 1e-6, f"({i1},{i2}): z2r={z2r} != {x2}"
            assert abs(z2i) < 1e-6, f"({i1},{i2}): z2i={z2i} != 0"
    print("  PASS")


def test_unit_circle():
    """unit_circle: z = exp(2*pi*i*x)."""
    print("test_unit_circle...")
    data, n = run_param_dump(20, [["unit_circle"]])
    for i1 in [0, 5, 10, 15]:
        for i2 in [0, 5, 10, 15]:
            z1r, z1i, z2r, z2i = get_point(data, n, i1, i2)
            x1, x2 = i1 / 20, i2 / 20
            exp_z1r = math.cos(2 * math.pi * x1)
            exp_z1i = math.sin(2 * math.pi * x1)
            exp_z2r = math.cos(2 * math.pi * x2)
            exp_z2i = math.sin(2 * math.pi * x2)
            assert abs(z1r - exp_z1r) < 1e-5, f"z1r mismatch at ({i1},{i2})"
            assert abs(z1i - exp_z1i) < 1e-5, f"z1i mismatch at ({i1},{i2})"
            assert abs(z2r - exp_z2r) < 1e-5, f"z2r mismatch at ({i1},{i2})"
            assert abs(z2i - exp_z2i) < 1e-5, f"z2i mismatch at ({i1},{i2})"
    print("  PASS")


def test_rtheta():
    """rtheta(p): z1 = pow(x1,p)*exp(2*pi*x2*i)."""
    print("test_rtheta...")
    data, n = run_param_dump(20, [["rtheta", "0.5"]])
    # Check (10, 5): x1=0.5, x2=0.25
    z1r, z1i, z2r, z2i = get_point(data, n, 10, 5)
    p = 0.5
    r1 = 0.5 ** p  # sqrt(0.5)
    a1 = 2 * math.pi * 0.25  # pi/2
    assert abs(z1r - r1 * math.cos(a1)) < 1e-5
    assert abs(z1i - r1 * math.sin(a1)) < 1e-5
    print("  PASS")


def test_swap():
    """swap: z1=x2, z2=x1."""
    print("test_swap...")
    data, n = run_param_dump(10, [["swap"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 3, 7)
    assert abs(z1r - 0.7) < 1e-6, f"swap z1r={z1r} should be 0.7"
    assert abs(z2r - 0.3) < 1e-6, f"swap z2r={z2r} should be 0.3"
    print("  PASS")


def test_chain():
    """Chained transforms: unit_circle then square."""
    print("test_chain...")
    data, n = run_param_dump(20, [["unit_circle"], ["square"]])
    # At (5, 0): x1=0.25 → unit_circle → z1 = exp(i*pi/2) = i
    # Then square: z1 = i^2 = -1
    z1r, z1i, z2r, z2i = get_point(data, n, 5, 0)
    assert abs(z1r - (-1.0)) < 1e-5, f"chain z1r={z1r} should be -1"
    assert abs(z1i) < 1e-5, f"chain z1i={z1i} should be 0"
    print("  PASS")


def test_t1radd():
    """t1radd(5): only t1.re shifted."""
    print("test_t1radd...")
    data, n = run_param_dump(10, [["t1radd", "5"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 3, 7)
    assert abs(z1r - (0.3 + 5)) < 1e-6
    assert abs(z1i) < 1e-6
    assert abs(z2r - 0.7) < 1e-6  # t2 unchanged
    assert abs(z2i) < 1e-6
    print("  PASS")


def test_t2iadd():
    """t2iadd(3): only t2.im shifted."""
    print("test_t2iadd...")
    data, n = run_param_dump(10, [["t2iadd", "3"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 3, 7)
    assert abs(z1r - 0.3) < 1e-6  # t1 unchanged
    assert abs(z1i) < 1e-6
    assert abs(z2r - 0.7) < 1e-6
    assert abs(z2i - 3.0) < 1e-6
    print("  PASS")


def test_inv_t_plus_2_parametric():
    """inv_t_plus_2(re1,im1,re2,im2): reciprocal after per-parameter complex shift."""
    print("test_inv_t_plus_2_parametric...")
    data, n = run_param_dump(10, [["inv_t_plus_2", "2", "0", "3", "1"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 3, 7)

    # t1 = 0.3 -> 1 / (0.3 + 2 + 0i)
    den1r, den1i = 2.3, 0.0
    d1 = den1r * den1r + den1i * den1i
    exp1r, exp1i = den1r / d1, -den1i / d1

    # t2 = 0.7 -> 1 / (0.7 + 3 + 1i)
    den2r, den2i = 3.7, 1.0
    d2 = den2r * den2r + den2i * den2i
    exp2r, exp2i = den2r / d2, -den2i / d2

    assert abs(z1r - exp1r) < 1e-6
    assert abs(z1i - exp1i) < 1e-6
    assert abs(z2r - exp2r) < 1e-6
    assert abs(z2i - exp2i) < 1e-6
    print("  PASS")


def test_roots2_quadratic():
    """roots2: roots of (9/64)z^2 + t1*z + t2."""
    print("test_roots2_quadratic...")
    transforms = [["t1iadd", "0.2"], ["t2iadd", "-0.1"], ["roots2"]]
    data, n = run_param_dump(10, transforms)
    z1r, z1i, z2r, z2i = get_point(data, n, 3, 7)

    a = 9.0 / 64.0
    t1 = complex(0.3, 0.2)
    t2 = complex(0.7, -0.1)
    disc = t1 * t1 - 4.0 * a * t2
    expected = [(-t1 + cmath.sqrt(disc)) / (2.0 * a), (-t1 - cmath.sqrt(disc)) / (2.0 * a)]
    got = [complex(z1r, z1i), complex(z2r, z2i)]

    # Root order is not semantically important; np.roots/eigensolvers do not
    # guarantee a stable order across implementations.
    err_a = abs(got[0] - expected[0]) + abs(got[1] - expected[1])
    err_b = abs(got[0] - expected[1]) + abs(got[1] - expected[0])
    assert min(err_a, err_b) < 1e-5, f"roots2 got={got}, expected={expected}"
    print("  PASS")


def test_output_size():
    """Verify output file size matches n1*n2*4*sizeof(float)."""
    print("test_output_size...")
    out_path = "/tmp/test_pdump_size.bin"
    for n in [10, 50, 100]:
        spec = json.dumps({"mode": "param_dump", "n1": n, "n2": n, "param_transforms": []})
        r = subprocess.run([SWEEP, out_path], input=spec, capture_output=True, text=True, timeout=30)
        assert r.returncode == 0
        size = os.path.getsize(out_path)
        expected = n * n * 4 * 4
        assert size == expected, f"n={n}: size={size} != expected={expected}"
        os.remove(out_path)
    print("  PASS")


def _spindle(xr, va, vb, vp):
    theta = 2.0 * math.pi * xr
    ct, st = math.cos(theta), math.sin(theta)
    e = 2.0 / max(vp, 0.01)
    rx = va * (1 if ct >= 0 else -1) * abs(ct) ** e
    iy = vb * (1 if st >= 0 else -1) * abs(st) ** e
    return rx, iy


def _limacon(xr, a, b):
    theta = 2.0 * math.pi * xr
    r = a + b * math.cos(theta)
    return r * math.cos(theta), r * math.sin(theta)


def _rose(xr, amp, k):
    theta = 2.0 * math.pi * xr
    r = amp * math.cos(k * theta)
    return r * math.cos(theta), r * math.sin(theta)


def _lissajous(xr, A, B, a, b, phase):
    theta = 2.0 * math.pi * xr
    delta = math.pi * phase
    return A * math.sin(a * theta + delta), B * math.sin(b * theta)


def _astroid(xr, scale):
    theta = 2.0 * math.pi * xr
    ct, st = math.cos(theta), math.sin(theta)
    return scale * ct * ct * ct, scale * st * st * st


def _archimedean(xr, a, b):
    theta = 2.0 * math.pi * xr
    r = a + b * theta
    return r * math.cos(theta), r * math.sin(theta)


def _logspiral(xr, a, b):
    theta = 2.0 * math.pi * xr
    r = a * math.exp(b * theta)
    return r * math.cos(theta), r * math.sin(theta)


def _deltoid(xr, R):
    theta = 2.0 * math.pi * xr
    return (R * (2 * math.cos(theta) + math.cos(2 * theta)) / 3,
            R * (2 * math.sin(theta) - math.sin(2 * theta)) / 3)


def _rply(xr, sides, radius, turns):
    ns = max(int(sides), 3)
    t = xr % 1.0
    pos = t * ns
    edge = min(int(pos), ns - 1)
    frac = pos - edge
    a0 = 2 * math.pi * edge / ns
    a1 = 2 * math.pi * (edge + 1) / ns
    px = radius * ((1 - frac) * math.cos(a0) + frac * math.cos(a1))
    py = radius * ((1 - frac) * math.sin(a0) + frac * math.sin(a1))
    ra = 2 * math.pi * turns
    return math.cos(ra) * px - math.sin(ra) * py, math.sin(ra) * px + math.cos(ra) * py


def _star(xr, points, outer, inner_ratio):
    np_ = max(int(points), 3)
    nv = 2 * np_
    t = xr % 1.0
    pos = t * nv
    edge = min(int(pos), nv - 1)
    frac = pos - edge
    r0 = outer if edge % 2 == 0 else outer * inner_ratio
    r1 = outer if (edge + 1) % 2 == 0 else outer * inner_ratio
    a0 = 2 * math.pi * edge / nv
    a1 = 2 * math.pi * (edge + 1) / nv
    return ((1 - frac) * r0 * math.cos(a0) + frac * r1 * math.cos(a1),
            (1 - frac) * r0 * math.sin(a0) + frac * r1 * math.sin(a1))


def _rect(xr, w, h, turns):
    perim = 2 * (w + h)
    t = xr % 1.0
    d = t * perim
    hw, hh = w / 2, h / 2
    if d < w:
        px, py = -hw + d, -hh
    elif d < w + h:
        px, py = hw, -hh + (d - w)
    elif d < 2 * w + h:
        px, py = hw - (d - w - h), hh
    else:
        px, py = -hw, hh - (d - 2 * w - h)
    ra = 2 * math.pi * turns
    return math.cos(ra) * px - math.sin(ra) * py, math.sin(ra) * px + math.cos(ra) * py


def _rrect(xr, w, h, m):
    theta = 2 * math.pi * (xr % 1.0)
    ct, st = math.cos(theta), math.sin(theta)
    e = 2.0 / max(m, 0.01)
    return ((w / 2) * (1 if ct >= 0 else -1) * abs(ct) ** e,
            (h / 2) * (1 if st >= 0 else -1) * abs(st) ** e)


def _cardioid(xr, size):
    """Python reference for cardioid: theta = 2*pi*xr, r = size*(1+cos(theta))."""
    import math
    theta = 2.0 * math.pi * xr
    r = size * (1.0 + math.cos(theta))
    return r * math.cos(theta), r * math.sin(theta)


def _heart(xr, size, turns):
    """Python reference for heart curve."""
    import math
    t = 2.0 * math.pi * xr + math.pi / 2.0
    st = math.sin(t)
    xh = 16.0 * st * st * st
    yh = 13.0*math.cos(t) - 5.0*math.cos(2*t) - 2.0*math.cos(3*t) - math.cos(4*t)
    hr = xh / 40.0
    hi = yh / 40.0 + 0.1
    ra = 2.0 * math.pi * turns
    rotr, roti = math.cos(ra), math.sin(ra)
    sr, si = size * hr, size * hi
    return rotr * sr - roti * si, rotr * si + roti * sr


def test_crd_t1():
    """crd(0, size): cardioid on t1 only, t2 unchanged."""
    print("test_crd_t1...")
    data, n = run_param_dump(10, [["crd", "0", "2.0"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 3, 7)
    x1, x2 = 3.0 / 10, 7.0 / 10
    er, ei = _cardioid(x1, 2.0)
    assert abs(z1r - er) < 1e-5, f"z1r={z1r} != {er}"
    assert abs(z1i - ei) < 1e-5, f"z1i={z1i} != {ei}"
    assert abs(z2r - x2) < 1e-6, "t2 should be unchanged"
    assert abs(z2i) < 1e-6
    print("  PASS")


def test_crd_t2():
    """crd(1, size): cardioid on t2 only, t1 unchanged."""
    print("test_crd_t2...")
    data, n = run_param_dump(10, [["crd", "1", "1.5"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 3, 7)
    x1, x2 = 3.0 / 10, 7.0 / 10
    er, ei = _cardioid(x2, 1.5)
    assert abs(z1r - x1) < 1e-6, "t1 should be unchanged"
    assert abs(z1i) < 1e-6
    assert abs(z2r - er) < 1e-5, f"z2r={z2r} != {er}"
    assert abs(z2i - ei) < 1e-5, f"z2i={z2i} != {ei}"
    print("  PASS")


def test_crd_both():
    """crd(2, size): cardioid on both t1 and t2."""
    print("test_crd_both...")
    data, n = run_param_dump(10, [["crd", "2", "1.0"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 3, 7)
    x1, x2 = 3.0 / 10, 7.0 / 10
    e1r, e1i = _cardioid(x1, 1.0)
    e2r, e2i = _cardioid(x2, 1.0)
    assert abs(z1r - e1r) < 1e-5, f"z1r={z1r} != {e1r}"
    assert abs(z1i - e1i) < 1e-5, f"z1i={z1i} != {e1i}"
    assert abs(z2r - e2r) < 1e-5, f"z2r={z2r} != {e2r}"
    assert abs(z2i - e2i) < 1e-5, f"z2i={z2i} != {e2i}"
    print("  PASS")


def test_hrt_t1():
    """hrt(0, size, turns): heart on t1 only."""
    print("test_hrt_t1...")
    data, n = run_param_dump(10, [["hrt", "0", "1.5", "0.25"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 4, 6)
    x1, x2 = 4.0 / 10, 6.0 / 10
    er, ei = _heart(x1, 1.5, 0.25)
    assert abs(z1r - er) < 1e-5, f"z1r={z1r} != {er}"
    assert abs(z1i - ei) < 1e-5, f"z1i={z1i} != {ei}"
    assert abs(z2r - x2) < 1e-6, "t2 should be unchanged"
    assert abs(z2i) < 1e-6
    print("  PASS")


def test_hrt_both():
    """hrt(2, size, turns): heart on both t1 and t2."""
    print("test_hrt_both...")
    data, n = run_param_dump(10, [["hrt", "2", "1.0", "0"]])
    z1r, z1i, z2r, z2i = get_point(data, n, 4, 6)
    x1, x2 = 4.0 / 10, 6.0 / 10
    e1r, e1i = _heart(x1, 1.0, 0)
    e2r, e2i = _heart(x2, 1.0, 0)
    assert abs(z1r - e1r) < 1e-5, f"z1r={z1r} != {e1r}"
    assert abs(z1i - e1i) < 1e-5, f"z1i={z1i} != {e1i}"
    assert abs(z2r - e2r) < 1e-5, f"z2r={z2r} != {e2r}"
    assert abs(z2i - e2i) < 1e-5, f"z2i={z2i} != {e2i}"
    print("  PASS")


def _test_targeted(name, args_t1, args_t2, args_both, ref_fn, ref_args, i1=3, i2=7, n=10):
    """Generic test helper for targeted transforms (t1/t2/both)."""
    x1, x2 = i1 / n, i2 / n

    # t1 only
    data, _ = run_param_dump(n, [[name] + args_t1])
    z1r, z1i, z2r, z2i = get_point(data, n, i1, i2)
    er, ei = ref_fn(x1, *ref_args)
    assert abs(z1r - er) < 1e-5, f"{name} t1: z1r={z1r} != {er}"
    assert abs(z1i - ei) < 1e-5, f"{name} t1: z1i={z1i} != {ei}"
    assert abs(z2r - x2) < 1e-6, f"{name} t1: t2 should be unchanged"
    assert abs(z2i) < 1e-6, f"{name} t1: t2i should be 0"

    # t2 only
    data, _ = run_param_dump(n, [[name] + args_t2])
    z1r, z1i, z2r, z2i = get_point(data, n, i1, i2)
    er, ei = ref_fn(x2, *ref_args)
    assert abs(z1r - x1) < 1e-6, f"{name} t2: t1 should be unchanged"
    assert abs(z1i) < 1e-6, f"{name} t2: t1i should be 0"
    assert abs(z2r - er) < 1e-5, f"{name} t2: z2r={z2r} != {er}"
    assert abs(z2i - ei) < 1e-5, f"{name} t2: z2i={z2i} != {ei}"

    # both
    data, _ = run_param_dump(n, [[name] + args_both])
    z1r, z1i, z2r, z2i = get_point(data, n, i1, i2)
    e1r, e1i = ref_fn(x1, *ref_args)
    e2r, e2i = ref_fn(x2, *ref_args)
    assert abs(z1r - e1r) < 1e-5, f"{name} both: z1r={z1r} != {e1r}"
    assert abs(z1i - e1i) < 1e-5, f"{name} both: z1i={z1i} != {e1i}"
    assert abs(z2r - e2r) < 1e-5, f"{name} both: z2r={z2r} != {e2r}"
    assert abs(z2i - e2i) < 1e-5, f"{name} both: z2i={z2i} != {e2i}"


def test_spdl():
    print("test_spdl...")
    _test_targeted("spdl",
        ["0", "0.5", "0.2", "1.5"], ["1", "0.5", "0.2", "1.5"], ["2", "0.5", "0.2", "1.5"],
        _spindle, (0.5, 0.2, 1.5))
    print("  PASS")


def test_lmc():
    print("test_lmc...")
    _test_targeted("lmc",
        ["0", "0.3", "0.5"], ["1", "0.3", "0.5"], ["2", "0.3", "0.5"],
        _limacon, (0.3, 0.5))
    print("  PASS")


def test_rsc():
    print("test_rsc...")
    _test_targeted("rsc",
        ["0", "0.5", "2"], ["1", "0.5", "2"], ["2", "0.5", "2"],
        _rose, (0.5, 2.0))
    print("  PASS")


def test_lss():
    print("test_lss...")
    _test_targeted("lss",
        ["0", "0.5", "0.5", "3", "2", "0.5"],
        ["1", "0.5", "0.5", "3", "2", "0.5"],
        ["2", "0.5", "0.5", "3", "2", "0.5"],
        _lissajous, (0.5, 0.5, 3.0, 2.0, 0.5))
    print("  PASS")


def test_ast():
    print("test_ast...")
    _test_targeted("ast",
        ["0", "1.0"], ["1", "1.0"], ["2", "1.0"],
        _astroid, (1.0,))
    print("  PASS")


def test_asp():
    print("test_asp...")
    _test_targeted("asp",
        ["0", "0", "0.1"], ["1", "0", "0.1"], ["2", "0", "0.1"],
        _archimedean, (0.0, 0.1))
    print("  PASS")


def test_lsp():
    print("test_lsp...")
    _test_targeted("lsp",
        ["0", "0.1", "0.15"], ["1", "0.1", "0.15"], ["2", "0.1", "0.15"],
        _logspiral, (0.1, 0.15))
    print("  PASS")


def test_dlt():
    print("test_dlt...")
    _test_targeted("dlt",
        ["0", "1.0"], ["1", "1.0"], ["2", "1.0"],
        _deltoid, (1.0,))
    print("  PASS")


def test_rply():
    print("test_rply...")
    _test_targeted("rply",
        ["0", "5", "1", "0"], ["1", "5", "1", "0"], ["2", "5", "1", "0"],
        _rply, (5.0, 1.0, 0.0))
    print("  PASS")


def test_rply_rotation():
    """rply with rotation: vertices should rotate."""
    print("test_rply_rotation...")
    data, n = run_param_dump(10, [["rply", "0", "4", "1.0", "0.25"]])
    z1r, z1i, _, _ = get_point(data, n, 3, 5)
    er, ei = _rply(0.3, 4, 1.0, 0.25)
    assert abs(z1r - er) < 1e-5
    assert abs(z1i - ei) < 1e-5
    print("  PASS")


def test_star():
    print("test_star...")
    _test_targeted("star",
        ["0", "5", "1", "0.5"], ["1", "5", "1", "0.5"], ["2", "5", "1", "0.5"],
        _star, (5.0, 1.0, 0.5))
    print("  PASS")


def test_star_vertex():
    """star: t=0 should be first outer vertex (1,0)."""
    print("test_star_vertex...")
    data, n = run_param_dump(100, [["star", "0", "5", "1.0", "0.5"]])
    z1r, z1i, _, _ = get_point(data, n, 0, 0)
    # t=0 → first outer vertex at angle 0, radius 1
    assert abs(z1r - 1.0) < 1e-4, f"z1r={z1r}"
    assert abs(z1i) < 1e-4, f"z1i={z1i}"
    print("  PASS")


def test_rect():
    print("test_rect...")
    _test_targeted("rect",
        ["0", "2", "1", "0"], ["1", "2", "1", "0"], ["2", "2", "1", "0"],
        _rect, (2.0, 1.0, 0.0))
    print("  PASS")


def test_rect_corners():
    """rect perimeter: t=0 → bottom-left (-1,-0.5), t=1/3 → bottom-right corner area."""
    print("test_rect_corners...")
    data, n = run_param_dump(100, [["rect", "0", "2", "1", "0"]])
    # t=0/100 → d=0 → bottom-left corner (-1, -0.5)
    z1r, z1i, _, _ = get_point(data, n, 0, 0)
    er, ei = _rect(0.0, 2, 1, 0)
    assert abs(z1r - er) < 1e-5, f"corner: z1r={z1r} != {er}"
    assert abs(z1i - ei) < 1e-5, f"corner: z1i={z1i} != {ei}"
    print("  PASS")


def test_rrect():
    print("test_rrect...")
    _test_targeted("rrect",
        ["0", "2", "1", "4"], ["1", "2", "1", "4"], ["2", "2", "1", "4"],
        _rrect, (2.0, 1.0, 4.0))
    print("  PASS")


def test_rrect_roundness():
    """rrect with large m → approaches rectangle; small m → approaches circle."""
    print("test_rrect_roundness...")
    # m=100 (very rectangular): at theta=pi/4, x ≈ w/2, y ≈ h/2
    data_sharp, n = run_param_dump(8, [["rrect", "0", "2", "1", "100"]])
    z1r, _, _, _ = get_point(data_sharp, n, 1, 0)  # t=1/8 → theta=pi/4
    er, _ = _rrect(1/8, 2, 1, 100)
    assert abs(z1r - er) < 1e-3
    # m=2 (ellipse): at theta=pi/4, x = (w/2)*cos(pi/4)
    data_round, _ = run_param_dump(8, [["rrect", "0", "2", "1", "2"]])
    z1r2, _, _, _ = get_point(data_round, n, 1, 0)
    er2, _ = _rrect(1/8, 2, 1, 2)
    assert abs(z1r2 - er2) < 1e-5
    # Sharp should be closer to 1.0 (w/2) than round
    assert abs(z1r) > abs(z1r2), f"sharp={z1r} should be > round={z1r2}"
    print("  PASS")


if __name__ == "__main__":
    test_identity()
    test_unit_circle()
    test_rtheta()
    test_swap()
    test_chain()
    test_t1radd()
    test_t2iadd()
    test_inv_t_plus_2_parametric()
    test_roots2_quadratic()
    test_output_size()
    test_crd_t1()
    test_crd_t2()
    test_crd_both()
    test_hrt_t1()
    test_hrt_both()
    test_spdl()
    test_lmc()
    test_rsc()
    test_lss()
    test_ast()
    test_asp()
    test_lsp()
    test_dlt()
    test_rply()
    test_rply_rotation()
    test_star()
    test_star_vertex()
    test_rect()
    test_rect_corners()
    test_rrect()
    test_rrect_roundness()
    print("\nAll param_dump tests passed.")
