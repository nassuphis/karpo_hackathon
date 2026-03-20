"""
Test sweep param_dump mode: verifies transformed parameter output
matches expected values for various transform chains.

Run: cd polypaint/tests && uv run python test_param_dump.py
"""
import json
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


if __name__ == "__main__":
    test_identity()
    test_unit_circle()
    test_rtheta()
    test_swap()
    test_chain()
    test_t1radd()
    test_t2iadd()
    test_output_size()
    print("\nAll param_dump tests passed.")
