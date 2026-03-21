#!/usr/bin/env python3
"""
Visual comparison test: C sweep pipeline vs Python reference for poly_426..poly_450.

C pipeline:  coeffgen -> solve -> rasterize roots to 1000x1000 image
Python pipeline: 100x100 grid, unit_circle params, call func, reverse coeffs, np.roots, rasterize

Reports: C pixels, Py pixels, overlap %, summary table.
"""

import ast
import json
import math
import struct
import subprocess
import sys
import textwrap
import numpy as np
from pathlib import Path

SWEEP = "./lambda/sweep_test"
POLY_SRC = "./lambda/poly500.py"
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
N1, N2 = 100, 100

STUBBED = {447, 449}


def load_poly_funcs(src_path, func_names):
    """Load specific poly functions from source using ast.parse + exec."""
    with open(src_path, "r") as f:
        source = f.read()

    tree = ast.parse(source)

    # Collect function source ranges
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in func_names:
            # Extract source lines for this function
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.split("\n")[start:end]
            func_source = "\n".join(lines)
            funcs[node.name] = func_source

    # Build a module with needed imports and all functions
    preamble = textwrap.dedent("""\
        import math
        import cmath
        import numpy as np
        pi = math.pi
    """)

    combined = preamble + "\n"
    for name in func_names:
        if name in funcs:
            combined += funcs[name] + "\n\n"
        else:
            print(f"WARNING: {name} not found in {src_path}")

    namespace = {}
    exec(combined, namespace)
    return namespace


def run_c_pipeline(func_name, poly_num):
    """Run C coeffgen + solve, return root positions as (N_t, degree) complex arrays."""
    coeffs_path = f"/tmp/vc_{poly_num}_coeffs.bin"
    roots_path = f"/tmp/vc_{poly_num}_roots.bin"

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

    result = subprocess.run(
        [SWEEP, coeffs_path],
        input=spec, capture_output=True, text=True, timeout=30
    )
    if result.returncode != 0:
        print(f"  C coeffgen FAILED: {result.stderr.strip()}")
        return None, None, None

    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]

    # Step 2: solve
    solve_spec = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "match_roots": False,
    })

    result2 = subprocess.run(
        [SWEEP, roots_path],
        input=solve_spec, capture_output=True, text=True, timeout=60
    )
    if result2.returncode != 0:
        print(f"  C solve FAILED: {result2.stderr.strip()}")
        return None, None, None

    solve_meta = json.loads(result2.stdout)
    degree = solve_meta["degree"]

    # Step 3: read roots - float32 pairs, 8 bytes per root
    with open(roots_path, "rb") as f:
        data = f.read()

    expected_size = n_t * degree * 2 * 4
    if len(data) != expected_size:
        print(f"  WARNING: expected {expected_size} bytes, got {len(data)}")

    n_roots_total = len(data) // 8
    floats = struct.unpack(f"<{n_roots_total * 2}f", data)

    roots = []
    for i in range(n_roots_total):
        roots.append(complex(floats[i * 2], floats[i * 2 + 1]))

    return roots, degree, n_t


def rasterize(roots, img_size=IMG_SIZE, extent=EXTENT):
    """Rasterize complex roots to a binary image. Returns set of (x, y) pixel coords."""
    pixels = set()
    scale = img_size / (2.0 * extent)
    cx = img_size / 2.0
    cy = img_size / 2.0

    for z in roots:
        re = z.real
        im = z.imag
        if not (math.isfinite(re) and math.isfinite(im)):
            continue
        px = int(re * scale + cx)
        py = int(-im * scale + cy)  # flip y
        if 0 <= px < img_size and 0 <= py < img_size:
            pixels.add((px, py))
    return pixels


def run_python_pipeline(py_func, poly_num):
    """Run Python reference: 100x100 grid, unit_circle, call func, reverse, np.roots."""
    all_roots = []
    for i1 in range(N1):
        x1 = i1 / N1
        for i2 in range(N2):
            x2 = i2 / N2
            # unit_circle transform
            a1 = 2.0 * math.pi * x1
            a2 = 2.0 * math.pi * x2
            t1 = complex(math.cos(a1), math.sin(a1))
            t2 = complex(math.cos(a2), math.sin(a2))

            try:
                coeffs = py_func(t1, t2)
            except Exception as e:
                continue

            if coeffs is None or len(coeffs) < 2:
                continue

            # Reverse coefficients (coeff_transforms: ["rev"])
            coeffs = coeffs[::-1]

            # np.roots expects highest-power-first (standard numpy convention)
            # After reversal, if original was [c0, c1, ..., cn] (c0 = const),
            # reversed is [cn, ..., c1, c0] which is highest-power-first.
            # So we can pass directly to np.roots.

            # Strip leading zeros
            first_nonzero = 0
            for k in range(len(coeffs)):
                if abs(coeffs[k]) > 1e-30:
                    first_nonzero = k
                    break
            else:
                continue

            coeffs = coeffs[first_nonzero:]
            if len(coeffs) < 2:
                continue

            try:
                r = np.roots(coeffs)
                for root in r:
                    all_roots.append(root)
            except Exception:
                continue

    return all_roots


def compare(poly_num, py_func):
    """Compare C and Python pipelines for a single poly function."""
    func_name = f"poly_{poly_num}"
    is_stubbed = poly_num in STUBBED

    print(f"\n{'='*60}")
    print(f"  {func_name}" + (" [STUBBED in C]" if is_stubbed else ""))
    print(f"{'='*60}")

    # C pipeline
    c_roots, degree, n_t = run_c_pipeline(func_name, poly_num)
    if c_roots is None:
        return {
            "poly": func_name,
            "c_pixels": 0,
            "py_pixels": 0,
            "overlap": 0,
            "overlap_pct": 0.0,
            "status": "C_FAIL",
        }

    c_pixels = rasterize(c_roots)
    print(f"  C: {len(c_roots)} roots, degree={degree}, n_t={n_t}, {len(c_pixels)} unique pixels")

    # Python pipeline
    py_roots = run_python_pipeline(py_func, poly_num)
    py_pixels = rasterize(py_roots)
    print(f"  Py: {len(py_roots)} roots, {len(py_pixels)} unique pixels")

    # Compute overlap
    overlap = c_pixels & py_pixels
    union = c_pixels | py_pixels
    overlap_pct = 100.0 * len(overlap) / len(union) if len(union) > 0 else 0.0

    status = "STUBBED" if is_stubbed else ("PASS" if overlap_pct >= 80.0 else ("LOW" if overlap_pct >= 50.0 else "FAIL"))

    print(f"  C pixels: {len(c_pixels):,}  Py pixels: {len(py_pixels):,}  "
          f"overlap: {len(overlap):,}  union: {len(union):,}  overlap%: {overlap_pct:.1f}%  [{status}]")

    return {
        "poly": func_name,
        "c_pixels": len(c_pixels),
        "py_pixels": len(py_pixels),
        "overlap": len(overlap),
        "overlap_pct": overlap_pct,
        "status": status,
    }


def main():
    poly_nums = list(range(426, 451))
    func_names = [f"poly_{n}" for n in poly_nums]

    print("Loading Python poly functions from source...")
    ns = load_poly_funcs(POLY_SRC, func_names)

    results = []
    for num in poly_nums:
        fname = f"poly_{num}"
        if fname not in ns:
            print(f"\nSKIP {fname}: not found in Python source")
            results.append({
                "poly": fname, "c_pixels": 0, "py_pixels": 0,
                "overlap": 0, "overlap_pct": 0.0, "status": "MISSING",
            })
            continue
        res = compare(num, ns[fname])
        results.append(res)

    # Summary table
    print("\n" + "=" * 80)
    print("SUMMARY TABLE")
    print("=" * 80)
    print(f"{'Poly':<12} {'C pix':>8} {'Py pix':>8} {'Overlap':>8} {'Overlap%':>9} {'Status':<10}")
    print("-" * 80)
    for r in results:
        print(f"{r['poly']:<12} {r['c_pixels']:>8,} {r['py_pixels']:>8,} "
              f"{r['overlap']:>8,} {r['overlap_pct']:>8.1f}% {r['status']:<10}")

    # Counts
    statuses = [r["status"] for r in results]
    n_pass = statuses.count("PASS")
    n_low = statuses.count("LOW")
    n_fail = statuses.count("FAIL")
    n_stub = statuses.count("STUBBED")
    n_other = len(statuses) - n_pass - n_low - n_fail - n_stub
    print("-" * 80)
    print(f"PASS: {n_pass}  LOW: {n_low}  FAIL: {n_fail}  STUBBED: {n_stub}  OTHER: {n_other}  TOTAL: {len(results)}")

    return 0 if n_fail == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
