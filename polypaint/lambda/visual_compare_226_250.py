#!/usr/bin/env python3
"""
Visual comparison: C sweep pipeline vs Python reference for poly_226..poly_250.

C pipeline:
  1. coeffgen with unit_circle param_transform + rev coeff_transform
  2. solve from coefficients
  3. Read roots (float32 pairs), rasterize to 1000x1000, extent [-2,2]x[-2,2]

Python pipeline:
  1. 100x100 grid, unit_circle params, call func from poly300.py (via ast.parse)
  2. Reverse coefficients
  3. np.roots to find roots
  4. Rasterize same way

Report overlap % for each function.
"""

import ast
import json
import math
import os
import struct
import subprocess
import sys
import tempfile
import time

import numpy as np

# --- Config ---
N1, N2 = 100, 100
IMG_W, IMG_H = 1000, 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
SWEEP = "./sweep_test"
POLY_SRC = "poly300.py"
FUNCS = [f"poly_{n}" for n in range(226, 251)]


# --- Load Python functions from poly300.py via ast ---

def load_poly_functions(src_path, func_names):
    """Parse poly300.py and extract specified functions."""
    with open(src_path, "r") as f:
        source = f.read()

    tree = ast.parse(source)

    # Build a module with just numpy imports and the requested functions
    funcs = {}

    # Extract function source by line numbers
    func_nodes = []
    for node in tree.body:
        if isinstance(node, ast.FunctionDef) and node.name in func_names:
            func_nodes.append(node)

    if not func_nodes:
        raise RuntimeError(f"No matching functions found in {src_path}")

    # Build a mini-module with necessary imports
    header = "import math\nimport cmath\nimport numpy as np\n"
    header += "try:\n    from scipy.special import sph_harm\nexcept ImportError:\n    sph_harm = None\n"
    header += "pi = math.pi\n\n"

    lines = source.split('\n')
    ns = {}
    exec(header, ns)

    for fnode in func_nodes:
        # Extract source lines for this function
        start = fnode.lineno - 1  # 0-based
        end = fnode.end_lineno if hasattr(fnode, 'end_lineno') and fnode.end_lineno else start + 1
        func_src = '\n'.join(lines[start:end])
        exec(func_src, ns)
        funcs[fnode.name] = ns[fnode.name]

    return funcs


# --- C pipeline ---

def run_c_pipeline(func_name):
    """Run coeffgen + solve for one function, return roots as (N, degree, 2) array."""
    coeffs_bin = f"/tmp/{func_name}_coeffs.bin"
    roots_bin = f"/tmp/{func_name}_roots.bin"

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
    result = subprocess.run(
        [SWEEP, coeffs_bin],
        input=json.dumps(spec),
        capture_output=True, text=True, timeout=60
    )
    if result.returncode != 0:
        raise RuntimeError(f"coeffgen failed for {func_name}: {result.stderr.strip()}")

    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]
    degree = meta["degree"]

    # Step 2: solve
    solve_spec = {
        "mode": "solve",
        "coeffs_file": coeffs_bin,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "match_roots": False,
    }
    result2 = subprocess.run(
        [SWEEP, roots_bin],
        input=json.dumps(solve_spec),
        capture_output=True, text=True, timeout=120
    )
    if result2.returncode != 0:
        raise RuntimeError(f"solve failed for {func_name}: {result2.stderr.strip()}")

    solve_meta = json.loads(result2.stdout)

    # Step 3: Read roots - float32 pairs, degree roots per step
    with open(roots_bin, "rb") as f:
        data = f.read()

    n_roots_total = len(data) // 8  # 8 bytes per root (2 x float32)
    floats = np.frombuffer(data, dtype=np.float32)
    # reshape: each step has degree roots, each root is (re, im)
    roots = floats.reshape(-1, degree, 2)

    # Cleanup
    try:
        os.remove(coeffs_bin)
        os.remove(roots_bin)
    except OSError:
        pass

    return roots, degree


# --- Python pipeline ---

def run_python_pipeline(func, func_name):
    """Run Python reference: 100x100 grid, unit_circle, call func, rev, np.roots."""
    all_roots = []
    degree = None

    for i1 in range(N1):
        for j in range(N2):
            # Match C's serpentine order
            i2 = (N2 - 1 - j) if (i1 & 1) else j
            x1 = i1 / N1
            x2 = i2 / N2

            # unit_circle transform
            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)

            # Call function
            try:
                coeffs = func(t1, t2)
            except Exception:
                coeffs = np.zeros(2, dtype=complex)

            # Rev transform (reverse)
            coeffs = coeffs[::-1].copy()

            if degree is None:
                degree = len(coeffs) - 1

            # Find roots
            # Strip leading zeros
            while len(coeffs) > 1 and abs(coeffs[0]) < 1e-15:
                coeffs = coeffs[1:]

            if len(coeffs) <= 1:
                roots = np.zeros(degree, dtype=complex)
            else:
                try:
                    r = np.roots(coeffs)
                except Exception:
                    r = np.zeros(degree, dtype=complex)
                # Pad to degree
                if len(r) < degree:
                    r = np.concatenate([r, np.zeros(degree - len(r), dtype=complex)])
                elif len(r) > degree:
                    r = r[:degree]
                roots = r

            all_roots.append(roots)

    return np.array(all_roots), degree


# --- Rasterize roots to pixel set ---

def rasterize(roots_array, degree):
    """Convert roots to set of (px, py) pixel coordinates."""
    # roots_array shape: (n_steps, degree, 2) for C or (n_steps, degree) complex for Python
    pixels = set()

    if roots_array.dtype == np.float32 or (roots_array.ndim == 3 and roots_array.shape[2] == 2):
        # C format: (n_steps, degree, 2) float32
        re = roots_array[:, :, 0].ravel()
        im = roots_array[:, :, 1].ravel()
    else:
        # Python format: complex array
        re = roots_array.real.ravel()
        im = roots_array.imag.ravel()

    # Filter valid (finite) roots
    valid = np.isfinite(re) & np.isfinite(im)
    re = re[valid]
    im = im[valid]

    # Map to pixel coords: [-EXTENT, EXTENT] -> [0, IMG_W)
    px = ((re + EXTENT) / (2 * EXTENT) * IMG_W).astype(np.int32)
    py = ((im + EXTENT) / (2 * EXTENT) * IMG_H).astype(np.int32)

    # Clip to image bounds
    mask = (px >= 0) & (px < IMG_W) & (py >= 0) & (py < IMG_H)
    px = px[mask]
    py = py[mask]

    return set(zip(px.tolist(), py.tolist()))


# --- Main ---

def main():
    print(f"Loading Python functions from {POLY_SRC}...")
    py_funcs = load_poly_functions(POLY_SRC, set(FUNCS))
    print(f"  Loaded {len(py_funcs)} functions.\n")

    results = []

    for func_name in FUNCS:
        print(f"--- {func_name} ---")
        t0 = time.time()

        # C pipeline
        try:
            c_roots, c_degree = run_c_pipeline(func_name)
            c_pixels = rasterize(c_roots, c_degree)
            c_ok = True
        except Exception as e:
            print(f"  C ERROR: {e}")
            c_pixels = set()
            c_ok = False
            c_degree = 0

        # Python pipeline
        if func_name not in py_funcs:
            print(f"  Python function not found, skipping.")
            results.append((func_name, len(c_pixels), 0, 0, 0.0))
            continue

        try:
            py_roots, py_degree = run_python_pipeline(py_funcs[func_name], func_name)
            py_pixels = rasterize(py_roots, py_degree)
            py_ok = True
        except Exception as e:
            print(f"  Python ERROR: {e}")
            py_pixels = set()
            py_ok = False

        elapsed = time.time() - t0

        overlap = len(c_pixels & py_pixels)
        union = len(c_pixels | py_pixels)
        overlap_pct = (overlap / union * 100) if union > 0 else 100.0

        print(f"  C pixels: {len(c_pixels):>7,}  |  Py pixels: {len(py_pixels):>7,}  |  "
              f"Overlap: {overlap:>7,}  |  Overlap%: {overlap_pct:6.2f}%  |  {elapsed:.1f}s")

        results.append((func_name, len(c_pixels), len(py_pixels), overlap, overlap_pct))

    # Summary table
    print("\n" + "=" * 80)
    print(f"{'Function':<12} {'C pixels':>10} {'Py pixels':>10} {'Overlap':>10} {'Overlap%':>10}")
    print("-" * 80)
    for name, cp, pp, ov, pct in results:
        print(f"{name:<12} {cp:>10,} {pp:>10,} {ov:>10,} {pct:>9.2f}%")
    print("=" * 80)

    # Overall stats
    pcts = [r[4] for r in results if r[2] > 0]  # only where Python had pixels
    if pcts:
        print(f"\nMean overlap: {np.mean(pcts):.2f}%  |  Min: {np.min(pcts):.2f}%  |  Max: {np.max(pcts):.2f}%")
    else:
        print("\nNo valid comparisons.")


if __name__ == "__main__":
    main()
