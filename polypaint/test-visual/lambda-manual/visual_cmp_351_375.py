"""
Visual comparison: C sweep pipeline vs Python reference for poly_351..poly_375.
Generates 1000x1000 root density images and computes pixel overlap.

Run: uv run python test-visual/lambda-manual/visual_cmp_351_375.py
"""
import ast
import json
import os
import struct
import subprocess
import sys
import textwrap
import time

import numpy as np

LAMBDA_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "lambda"))
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")
EXTENT = 2.0  # [-2,2] x [-2,2]
IMG_SIZE = 1000
N1 = 100
N2 = 100
POLY_RANGE = range(351, 376)

# ---- Load Python functions from poly400.py via ast ----

def load_poly_functions(path, func_range):
    """Parse poly400.py and extract poly_NNN functions via ast+exec."""
    with open(path, "r") as f:
        source = f.read()

    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef):
            name = node.name
            if name.startswith("poly_"):
                try:
                    num = int(name.split("_")[1])
                except (ValueError, IndexError):
                    continue
                if num in func_range:
                    funcs[name] = ast.get_source_segment(source, node)
    return funcs


def compile_poly_func(func_source, func_name):
    """Compile a single poly function and return callable."""
    # Build a minimal namespace with required imports
    ns = {}
    # sph_harm removed in scipy>=1.17; provide stub (not used by poly_351-375)
    exec("import math, cmath, numpy as np\ntry:\n    from scipy.special import sph_harm\nexcept ImportError:\n    sph_harm = None\npi = math.pi", ns)

    # Some functions reference helpers from the module; provide stubs
    # letters and zfrm are used by some polys but likely not 351-375
    exec(func_source, ns)
    return ns[func_name]


def roots_to_pixels(roots_re, roots_im, size=IMG_SIZE, extent=EXTENT):
    """Convert root positions to a set of pixel coordinates."""
    pixels = set()
    for re, im in zip(roots_re, roots_im):
        px = int((re + extent) / (2 * extent) * size)
        py = int((-im + extent) / (2 * extent) * size)  # flip y
        if 0 <= px < size and 0 <= py < size:
            pixels.add((px, py))
    return pixels


# ---- C pipeline ----

def run_c_pipeline(func_name):
    """Run coeffgen + solve for a function, return (roots_re, roots_im) arrays."""
    coeffs_file = f"/tmp/{func_name}_coeffs.bin"
    roots_file = f"/tmp/{func_name}_roots.bin"

    # Step 1: coeffgen
    spec = json.dumps({
        "mode": "coeffgen",
        "function": func_name,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"],
        "times": 1,
        "dither_pass": 0,
    })
    r = subprocess.run(
        [SWEEP, coeffs_file],
        input=spec, capture_output=True, text=True, timeout=60
    )
    if r.returncode != 0:
        raise RuntimeError(f"coeffgen failed for {func_name}: {r.stderr}")
    meta = json.loads(r.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]
    degree = meta["degree"]

    # Step 2: solve
    solve_spec = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n_t": n_t,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "match_roots": False,
    })
    r2 = subprocess.run(
        [SWEEP, roots_file],
        input=solve_spec, capture_output=True, text=True, timeout=120
    )
    if r2.returncode != 0:
        raise RuntimeError(f"solve failed for {func_name}: {r2.stderr}")

    # Step 3: read roots
    with open(roots_file, "rb") as f:
        data = f.read()
    n_roots_total = len(data) // 8  # float32 pairs, 4+4=8 bytes each
    floats = np.frombuffer(data, dtype=np.float32)
    re = floats[0::2].astype(np.float64)
    im = floats[1::2].astype(np.float64)
    # Filter out NaN/Inf values
    valid = np.isfinite(re) & np.isfinite(im)
    re = re[valid]
    im = im[valid]
    return re, im, degree


# ---- Python pipeline ----

def run_python_pipeline(func):
    """Evaluate poly function on 100x100 unit-circle grid, solve roots."""
    all_re = []
    all_im = []
    for i1 in range(N1):
        x1 = i1 / N1
        t1 = np.exp(2j * np.pi * x1)
        for i2 in range(N2):
            x2 = i2 / N2
            t2 = np.exp(2j * np.pi * x2)
            coeffs = func(t1, t2)
            if coeffs is None or len(coeffs) == 0:
                continue
            # Reverse coefficients (coeff_transforms: ["rev"])
            coeffs = coeffs[::-1]
            # np.roots expects highest-power first
            # After reversal, coeffs[0] is what was the last element
            try:
                roots = np.roots(coeffs)
            except Exception:
                continue
            all_re.extend(roots.real)
            all_im.extend(roots.imag)
    return np.array(all_re), np.array(all_im)


# ---- Main ----

def main():
    print("Loading Python poly functions from poly400.py...")
    func_sources = load_poly_functions(os.path.join(LAMBDA_DIR, "poly400.py"), set(POLY_RANGE))
    print(f"  Found {len(func_sources)} functions")

    results = []

    for n in POLY_RANGE:
        func_name = f"poly_{n}"
        print(f"\n{'='*60}")
        print(f"  {func_name}")
        print(f"{'='*60}")

        # C pipeline
        try:
            c_re, c_im, degree = run_c_pipeline(func_name)
            c_pixels = roots_to_pixels(c_re, c_im)
            c_count = len(c_pixels)
            print(f"  C:  {c_count:>7} pixels  (degree={degree}, {len(c_re)} roots total)")
        except Exception as e:
            print(f"  C:  FAILED - {e}")
            results.append((func_name, "FAIL", 0, 0, 0.0))
            continue

        # Python pipeline
        if func_name not in func_sources:
            print(f"  Py: MISSING from poly400.py")
            results.append((func_name, "MISSING", c_count, 0, 0.0))
            continue

        try:
            py_func = compile_poly_func(func_sources[func_name], func_name)
            py_re, py_im = run_python_pipeline(py_func)
            py_pixels = roots_to_pixels(py_re, py_im)
            py_count = len(py_pixels)
            print(f"  Py: {py_count:>7} pixels  ({len(py_re)} roots total)")
        except Exception as e:
            print(f"  Py: FAILED - {e}")
            results.append((func_name, "PY_FAIL", c_count, 0, 0.0))
            continue

        # Overlap
        overlap = c_pixels & py_pixels
        union = c_pixels | py_pixels
        overlap_pct = 100.0 * len(overlap) / len(union) if union else 0.0
        print(f"  Overlap: {len(overlap):>7} / {len(union):>7} = {overlap_pct:.1f}%")
        results.append((func_name, "OK", c_count, py_count, overlap_pct))

    # Summary table
    print(f"\n\n{'='*70}")
    print(f"  SUMMARY: poly_351 .. poly_375 visual comparison")
    print(f"{'='*70}")
    print(f"  {'Function':<12} {'Status':<10} {'C pixels':>10} {'Py pixels':>10} {'Overlap %':>10}")
    print(f"  {'-'*12} {'-'*10} {'-'*10} {'-'*10} {'-'*10}")
    for name, status, c_px, py_px, ovlp in results:
        if status == "OK":
            print(f"  {name:<12} {status:<10} {c_px:>10} {py_px:>10} {ovlp:>9.1f}%")
        else:
            print(f"  {name:<12} {status:<10} {c_px:>10} {py_px:>10} {'N/A':>10}")
    print(f"{'='*70}")

    ok_results = [r for r in results if r[1] == "OK"]
    if ok_results:
        avg_overlap = np.mean([r[4] for r in ok_results])
        print(f"  Average overlap (OK only): {avg_overlap:.1f}%")
        min_r = min(ok_results, key=lambda r: r[4])
        max_r = max(ok_results, key=lambda r: r[4])
        print(f"  Min overlap: {min_r[0]} = {min_r[4]:.1f}%")
        print(f"  Max overlap: {max_r[0]} = {max_r[4]:.1f}%")


if __name__ == "__main__":
    main()
