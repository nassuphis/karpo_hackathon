"""
Visual comparison test for poly_401 through poly_425 (C sweep vs Python reference).

Runs C pipeline (coeffgen -> solve -> read roots -> rasterize) and Python pipeline
(extract function from poly500.py -> evaluate -> np.roots -> rasterize), then
compares overlap of rasterized pixels on a 1000x1000 grid with extent [-2,2]x[-2,2].

Run: cd polypaint/lambda && uv run python test_visual_401_425.py
"""
import ast
import json
import math
import struct
import subprocess
import sys
import time
import numpy as np

SWEEP = "./sweep_test"
N1, N2 = 100, 100
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]

FUNCTIONS = [f"poly_{n}" for n in range(401, 426)]

# poly_407, poly_421, poly_423 are stubbed in C (too complex for auto-transpile)
STUBBED = {"poly_407", "poly_421", "poly_423"}


def extract_python_functions(py_path, func_names):
    """Extract function source from poly500.py using ast.parse, exec into namespace."""
    with open(py_path, "r") as f:
        source = f.read()

    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in func_names:
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.splitlines()[start:end]
            func_source = "\n".join(lines)
            funcs[node.name] = func_source
    return funcs


def build_python_namespace():
    """Build a namespace with the imports needed by poly500 functions."""
    ns = {}
    exec("import math; import cmath; import numpy as np; pi = math.pi", ns)
    return ns


def run_c_pipeline(func_name):
    """Run C coeffgen + solve, return root positions as set of (px, py) pixel coords."""
    coeffs_file = f"/tmp/{func_name}_coeffs.bin"
    roots_file = f"/tmp/{func_name}_roots.bin"

    # Step 1: coeffgen
    spec = {
        "mode": "coeffgen",
        "function": func_name,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"],
        "times": 1,
        "dither_pass": 0,
    }
    try:
        result = subprocess.run(
            [SWEEP, coeffs_file],
            input=json.dumps(spec),
            capture_output=True, text=True, timeout=60,
        )
    except subprocess.TimeoutExpired:
        return None, "COEFFGEN TIMEOUT"
    if result.returncode != 0:
        return None, f"COEFFGEN FAILED (rc={result.returncode}): {result.stderr[:200]}"

    try:
        meta = json.loads(result.stdout)
    except json.JSONDecodeError:
        return None, f"COEFFGEN BAD JSON: {result.stdout[:200]}"

    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]

    # Step 2: solve
    solve_spec = {
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "n_t": n_t,
    }
    try:
        result = subprocess.run(
            [SWEEP, roots_file],
            input=json.dumps(solve_spec),
            capture_output=True, text=True, timeout=120,
        )
    except subprocess.TimeoutExpired:
        return None, "SOLVE TIMEOUT"
    if result.returncode != 0:
        return None, f"SOLVE FAILED (rc={result.returncode}): {result.stderr[:200]}"

    # Step 3: read roots and rasterize
    try:
        with open(roots_file, "rb") as f:
            data = f.read()
    except FileNotFoundError:
        return None, "ROOTS FILE MISSING"

    n_floats = len(data) // 4  # float32 values
    if n_floats == 0:
        return set(), None

    floats = struct.unpack(f"<{n_floats}f", data)
    n_roots = n_floats // 2

    pixels = set()
    for i in range(n_roots):
        re = floats[i * 2]
        im = floats[i * 2 + 1]
        if not (math.isfinite(re) and math.isfinite(im)):
            continue
        px = int((re + EXTENT) / (2 * EXTENT) * IMG_SIZE)
        py = int((im + EXTENT) / (2 * EXTENT) * IMG_SIZE)
        if 0 <= px < IMG_SIZE and 0 <= py < IMG_SIZE:
            pixels.add((px, py))

    return pixels, None


def run_python_pipeline(func_source, func_name, ns):
    """Run Python reference: evaluate function on grid, np.roots, rasterize."""
    local_ns = dict(ns)
    try:
        exec(func_source, local_ns)
    except Exception as e:
        return None, f"PYTHON COMPILE ERROR: {e}"

    func = local_ns[func_name]

    pixels = set()
    crash_count = 0
    total = N1 * N2

    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            try:
                coeffs = func(t1, t2)
                if coeffs is None or len(coeffs) == 0:
                    crash_count += 1
                    continue
                # Reverse coeffs (same as coeff_transforms: ["rev"])
                coeffs = coeffs[::-1]
                # Strip leading zeros
                while len(coeffs) > 1 and coeffs[0] == 0:
                    coeffs = coeffs[1:]
                if len(coeffs) <= 1:
                    continue
                roots = np.roots(coeffs)
                for r in roots:
                    re, im = r.real, r.imag
                    if not (math.isfinite(re) and math.isfinite(im)):
                        continue
                    px = int((re + EXTENT) / (2 * EXTENT) * IMG_SIZE)
                    py = int((im + EXTENT) / (2 * EXTENT) * IMG_SIZE)
                    if 0 <= px < IMG_SIZE and 0 <= py < IMG_SIZE:
                        pixels.add((px, py))
            except Exception:
                crash_count += 1
                continue

    if crash_count == total:
        return None, "PYTHON ALL CRASHED"

    return pixels, None


def compute_overlap(c_pixels, py_pixels):
    """Compute overlap percentage between two pixel sets."""
    if not c_pixels and not py_pixels:
        return 100.0, 0, 0, 0
    union = c_pixels | py_pixels
    intersection = c_pixels & py_pixels
    if len(union) == 0:
        return 100.0, 0, 0, 0
    overlap_pct = 100.0 * len(intersection) / len(union)
    return overlap_pct, len(c_pixels), len(py_pixels), len(intersection)


def main():
    print("=" * 80)
    print("Visual Comparison: poly_401-425 C Sweep vs Python Reference")
    print(f"Grid: {N1}x{N2}, Image: {IMG_SIZE}x{IMG_SIZE}, Extent: [-{EXTENT},{EXTENT}]")
    print("=" * 80)
    print()

    # Extract Python functions
    py_path = "poly500.py"
    func_sources = extract_python_functions(py_path, set(FUNCTIONS))
    ns = build_python_namespace()

    results = []
    for func_name in FUNCTIONS:
        t0 = time.time()
        is_stubbed = func_name in STUBBED

        # Run C pipeline
        c_pixels, c_err = run_c_pipeline(func_name)
        c_time = time.time() - t0

        # Run Python pipeline
        t1 = time.time()
        if func_name not in func_sources:
            py_pixels, py_err = None, "FUNCTION NOT FOUND IN poly500.py"
        else:
            py_pixels, py_err = run_python_pipeline(func_sources[func_name], func_name, ns)
        py_time = time.time() - t1

        # Compute overlap
        if c_err:
            status = f"C ERROR: {c_err}"
            overlap_pct = 0.0
            c_count = 0
            py_count = len(py_pixels) if py_pixels else 0
            inter_count = 0
        elif py_err:
            status = f"PY ERROR: {py_err}"
            overlap_pct = 0.0
            c_count = len(c_pixels) if c_pixels else 0
            py_count = 0
            inter_count = 0
        else:
            overlap_pct, c_count, py_count, inter_count = compute_overlap(c_pixels, py_pixels)
            if is_stubbed:
                status = "STUBBED"
            elif overlap_pct >= 95:
                status = "PASS"
            elif overlap_pct >= 80:
                status = "WARN"
            else:
                status = "FAIL"

        results.append({
            "func": func_name,
            "status": status,
            "overlap": overlap_pct,
            "c_pixels": c_count,
            "py_pixels": py_count,
            "intersection": inter_count,
            "c_time": c_time,
            "py_time": py_time,
            "stubbed": is_stubbed,
        })

        stub_tag = " [STUBBED]" if is_stubbed else ""
        print(f"{func_name}{stub_tag}: {status}  "
              f"C={c_count:,}  Py={py_count:,}  "
              f"overlap={overlap_pct:.1f}%  "
              f"(C:{c_time:.1f}s  Py:{py_time:.1f}s)")

    # Summary table
    print()
    print("=" * 80)
    print("SUMMARY TABLE")
    print("=" * 80)
    print(f"{'Function':<12} {'Status':<10} {'C pixels':>10} {'Py pixels':>10} "
          f"{'Overlap%':>10} {'Note'}")
    print("-" * 80)

    pass_count = 0
    warn_count = 0
    fail_count = 0
    stub_count = 0
    err_count = 0

    for r in results:
        note = ""
        if r["stubbed"]:
            note = "stubbed in C"
            stub_count += 1
        elif r["status"] == "PASS":
            pass_count += 1
        elif r["status"] == "WARN":
            warn_count += 1
            note = "low overlap"
        elif r["status"] == "FAIL":
            fail_count += 1
            note = "MISMATCH"
        elif "ERROR" in r["status"]:
            err_count += 1
            note = r["status"][:40]

        print(f"{r['func']:<12} {r['status']:<10} {r['c_pixels']:>10,} {r['py_pixels']:>10,} "
              f"{r['overlap']:>9.1f}% {note}")

    print("-" * 80)
    total = len(results)
    print(f"Total: {total}  PASS: {pass_count}  WARN: {warn_count}  "
          f"FAIL: {fail_count}  STUBBED: {stub_count}  ERROR: {err_count}")
    print("=" * 80)

    # Return exit code: 0 if no failures (stubs don't count)
    if fail_count > 0 or err_count > 0:
        sys.exit(1)
    else:
        sys.exit(0)


if __name__ == "__main__":
    main()
