"""
Visual comparison test for 30 hand-written C poly functions vs Python references.

Runs C pipeline (coeffgen -> solve -> read roots -> rasterize) and Python pipeline
(extract function from poly200.py -> evaluate -> np.roots -> rasterize), then
compares overlap of rasterized pixels on a 1000x1000 grid with extent [-2,2]x[-2,2].

Run: cd polypaint/lambda && uv run python test_visual_30.py
"""
import ast
import json
import math
import struct
import subprocess
import sys
import textwrap
import time
import numpy as np

SWEEP = "./sweep_test"
N1, N2 = 100, 100
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]

FUNCTIONS = [
    "poly_103", "poly_123", "poly_134", "poly_135", "poly_142", "poly_146",
    "poly_150", "poly_152", "poly_153", "poly_157", "poly_161", "poly_164",
    "poly_167", "poly_171", "poly_179", "poly_180", "poly_184", "poly_187",
    "poly_188", "poly_189", "poly_190", "poly_191", "poly_192", "poly_193",
    "poly_194", "poly_195", "poly_196", "poly_197", "poly_198", "poly_199",
]


def extract_python_functions(py_path, func_names):
    """Extract function source from poly200.py using ast.parse, exec into namespace."""
    with open(py_path, "r") as f:
        source = f.read()

    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in func_names:
            # Extract source lines
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.splitlines()[start:end]
            func_source = "\n".join(lines)
            funcs[node.name] = func_source
    return funcs


def build_python_namespace():
    """Build a namespace with the imports needed by poly200 functions."""
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

    n_roots = len(data) // 8  # float32 pairs, 8 bytes each
    if n_roots == 0:
        return set(), None

    floats = struct.unpack(f"<{n_roots * 2}f", data)

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
    # Compile and exec the function into our namespace
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
                # Skip if leading coefficient is zero
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
        return None, "PYTHON CRASHED"

    return pixels, None


def main():
    print("=" * 80)
    print("Visual Comparison: 30 Hand-Written C Functions vs Python References")
    print(f"Grid: {N1}x{N2}, Image: {IMG_SIZE}x{IMG_SIZE}, Extent: [-{EXTENT},{EXTENT}]")
    print("=" * 80)
    print()

    # Extract Python functions
    py_path = "poly200.py"
    func_sources = extract_python_functions(py_path, set(FUNCTIONS))
    ns = build_python_namespace()

    results = []

    for func_name in FUNCTIONS:
        print(f"--- {func_name} ---")
        t0 = time.time()

        # C pipeline
        c_pixels, c_err = run_c_pipeline(func_name)
        t_c = time.time() - t0

        if c_err:
            print(f"  C: {c_err} ({t_c:.1f}s)")
        else:
            print(f"  C: {len(c_pixels)} pixels ({t_c:.1f}s)")

        # Python pipeline
        t1_start = time.time()
        if func_name not in func_sources:
            py_pixels, py_err = None, "FUNCTION NOT FOUND IN poly200.py"
        else:
            py_pixels, py_err = run_python_pipeline(func_sources[func_name], func_name, ns)
        t_py = time.time() - t1_start

        if py_err:
            print(f"  Python: {py_err} ({t_py:.1f}s)")
        else:
            print(f"  Python: {len(py_pixels)} pixels ({t_py:.1f}s)")

        # Compute overlap
        if c_pixels is not None and py_pixels is not None:
            overlap = len(c_pixels & py_pixels)
            union = len(c_pixels | py_pixels)
            if union > 0:
                pct = 100.0 * overlap / union
            else:
                pct = 100.0
            print(f"  Overlap: {overlap}/{union} = {pct:.1f}%")
            results.append((func_name, len(c_pixels), len(py_pixels), overlap, pct, None))
        else:
            err_msg = c_err or py_err or "UNKNOWN"
            print(f"  Overlap: N/A ({err_msg})")
            results.append((func_name,
                           len(c_pixels) if c_pixels is not None else None,
                           len(py_pixels) if py_pixels is not None else None,
                           None, None, err_msg))
        print()

    # Summary table
    print()
    print("=" * 80)
    print("SUMMARY TABLE")
    print("=" * 80)
    print(f"{'Function':<12} {'C pixels':>10} {'Py pixels':>10} {'Overlap':>10} {'Overlap%':>10} {'Status'}")
    print("-" * 80)
    for func_name, c_px, py_px, overlap, pct, err in results:
        c_str = str(c_px) if c_px is not None else "N/A"
        py_str = str(py_px) if py_px is not None else "N/A"
        ov_str = str(overlap) if overlap is not None else "N/A"
        pct_str = f"{pct:.1f}%" if pct is not None else "N/A"
        status = "OK" if pct is not None and pct >= 90 else (err if err else "LOW OVERLAP")
        print(f"{func_name:<12} {c_str:>10} {py_str:>10} {ov_str:>10} {pct_str:>10} {status}")

    # Overall stats
    ok_count = sum(1 for _, _, _, _, pct, _ in results if pct is not None and pct >= 90)
    total = len(results)
    tested = sum(1 for _, _, _, _, pct, _ in results if pct is not None)
    print("-" * 80)
    print(f"Tested: {tested}/{total}, Overlap >= 90%: {ok_count}/{tested}")


if __name__ == "__main__":
    main()
