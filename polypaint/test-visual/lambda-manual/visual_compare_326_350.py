"""
Visual comparison: C sweep pipeline vs Python reference for poly_326..poly_350.

C pipeline: coeffgen (unit_circle, rev) -> solve -> rasterize 1000x1000 [-2,2]x[-2,2]
Python pipeline: load poly func from poly400.py via ast, unit_circle params, reverse coeffs, np.roots, rasterize

Reports: C pixels, Py pixels, overlap % for each function. Summary table at end.
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
N1, N2 = 100, 100
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
STUBBED = {328, 332, 339, 347, 349}

POLY_SOURCE = os.path.join(LAMBDA_DIR, "poly400.py")


def load_poly_func(func_name):
    """Load a single poly function from poly400.py using ast.parse + exec."""
    with open(POLY_SOURCE, "r") as f:
        source = f.read()

    tree = ast.parse(source)

    # Find the function definition
    func_source = None
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name == func_name:
            func_source = ast.get_source_segment(source, node)
            break

    if func_source is None:
        raise ValueError(f"Function {func_name} not found in {POLY_SOURCE}")

    # Execute with numpy available
    ns = {"np": np, "math": __import__("math"), "cmath": __import__("cmath")}
    exec(func_source, ns)
    return ns[func_name]


def run_c_pipeline(func_name, poly_num):
    """Run C coeffgen + solve, return root positions as (n_t, degree, 2) array."""
    coeffs_path = f"/tmp/poly{poly_num}_coeffs.bin"
    roots_path = f"/tmp/poly{poly_num}_roots.bin"

    # Step 1: coeffgen
    spec = json.dumps({
        "mode": "coeffgen",
        "function": func_name,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"],
        "times": 1,
        "dither_pass": 0
    })

    result = subprocess.run(
        [SWEEP, coeffs_path],
        input=spec, capture_output=True, text=True, timeout=30
    )
    if result.returncode != 0:
        print(f"  C coeffgen FAILED: {result.stderr.strip()}")
        return None, 0, 0

    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]

    # Step 2: solve
    solve_spec = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1
    })

    result = subprocess.run(
        [SWEEP, roots_path],
        input=solve_spec, capture_output=True, text=True, timeout=60
    )
    if result.returncode != 0:
        print(f"  C solve FAILED: {result.stderr.strip()}")
        return None, n_coeffs, n_t

    solve_meta = json.loads(result.stdout)
    degree = solve_meta["degree"]
    n_t_solve = solve_meta["n_t"]

    # Step 3: read roots
    with open(roots_path, "rb") as f:
        data = f.read()

    n_roots_total = len(data) // 8  # float32 pairs
    n_steps = n_roots_total // degree if degree > 0 else 0

    roots = np.frombuffer(data, dtype=np.float32).reshape(n_steps, degree, 2)
    return roots, n_coeffs, n_t


def rasterize(roots, img_size=IMG_SIZE, extent=EXTENT):
    """Rasterize root positions to a boolean pixel grid."""
    if roots is None or len(roots) == 0:
        return np.zeros((img_size, img_size), dtype=bool)

    # roots shape: (n_steps, degree, 2) or flat list of (re, im)
    if roots.ndim == 3:
        re = roots[:, :, 0].ravel()
        im = roots[:, :, 1].ravel()
    else:
        re = roots[:, 0].ravel()
        im = roots[:, 1].ravel()

    # Map to pixel coords: [-extent, extent] -> [0, img_size)
    px = ((re + extent) / (2 * extent) * img_size).astype(np.int32)
    py = ((im + extent) / (2 * extent) * img_size).astype(np.int32)

    # Filter in-bounds
    mask = (px >= 0) & (px < img_size) & (py >= 0) & (py < img_size)
    px = px[mask]
    py = py[mask]

    grid = np.zeros((img_size, img_size), dtype=bool)
    grid[py, px] = True
    return grid


def run_python_pipeline(func_name):
    """Run Python reference: 100x100 grid, unit_circle params, reverse coeffs, np.roots."""
    py_func = load_poly_func(func_name)

    all_roots = []
    for i1 in range(N1):
        x1 = i1 / N1
        for i2 in range(N2):
            x2 = i2 / N2
            # unit_circle transform
            theta1 = 2 * np.pi * x1
            theta2 = 2 * np.pi * x2
            t1 = complex(np.cos(theta1), np.sin(theta1))
            t2 = complex(np.cos(theta2), np.sin(theta2))

            coeffs = py_func(t1, t2)
            # reverse (coeff_transforms: ["rev"])
            coeffs = coeffs[::-1]

            # Skip if degenerate
            if len(coeffs) < 2 or np.all(np.abs(coeffs) < 1e-30):
                continue

            # Strip leading zeros
            first_nonzero = 0
            while first_nonzero < len(coeffs) - 1 and abs(coeffs[first_nonzero]) < 1e-15:
                first_nonzero += 1
            coeffs = coeffs[first_nonzero:]

            if len(coeffs) < 2:
                continue

            try:
                r = np.roots(coeffs)
                for root in r:
                    all_roots.append([root.real, root.imag])
            except Exception:
                continue

    if not all_roots:
        return np.zeros((0, 2), dtype=np.float32)

    return np.array(all_roots, dtype=np.float32)


def compare_one(poly_num):
    """Compare C vs Python for one poly function. Return dict with results."""
    func_name = f"poly_{poly_num}"
    is_stubbed = poly_num in STUBBED

    t0 = time.time()

    # C pipeline
    c_roots, n_coeffs, n_t = run_c_pipeline(func_name, poly_num)
    t_c = time.time() - t0

    c_grid = rasterize(c_roots)
    c_pixels = int(c_grid.sum())

    # Python pipeline
    t0 = time.time()
    py_roots = run_python_pipeline(func_name)
    t_py = time.time() - t0

    py_grid = rasterize(py_roots.reshape(-1, 1, 2) if py_roots.ndim == 2 else py_roots)
    py_pixels = int(py_grid.sum())

    # Overlap
    overlap_grid = c_grid & py_grid
    overlap_pixels = int(overlap_grid.sum())

    union_pixels = int((c_grid | py_grid).sum())
    overlap_pct = (overlap_pixels / union_pixels * 100) if union_pixels > 0 else 0.0

    status = "STUB" if is_stubbed else ("OK" if overlap_pct > 50 else "LOW" if overlap_pct > 10 else "FAIL")
    if is_stubbed and c_pixels <= 1:
        status = "STUB(expected)"

    print(f"  {func_name}: C={c_pixels:>6}px  Py={py_pixels:>6}px  "
          f"overlap={overlap_pixels:>6}px ({overlap_pct:5.1f}%)  "
          f"C:{t_c:.1f}s Py:{t_py:.1f}s  [{status}]")

    return {
        "func": func_name,
        "c_pixels": c_pixels,
        "py_pixels": py_pixels,
        "overlap_pixels": overlap_pixels,
        "overlap_pct": overlap_pct,
        "status": status,
        "c_time": t_c,
        "py_time": t_py,
    }


def main():
    print("=" * 90)
    print("Visual Comparison: C sweep vs Python reference (poly_326 through poly_350)")
    print(f"Grid: {N1}x{N2}, Image: {IMG_SIZE}x{IMG_SIZE}, Extent: [-{EXTENT},{EXTENT}]")
    print(f"Stubbed (expected 0/1 C pixels): {sorted(STUBBED)}")
    print("=" * 90)
    print()

    results = []
    for num in range(326, 351):
        try:
            r = compare_one(num)
            results.append(r)
        except Exception as e:
            print(f"  poly_{num}: ERROR - {e}")
            results.append({
                "func": f"poly_{num}",
                "c_pixels": 0, "py_pixels": 0,
                "overlap_pixels": 0, "overlap_pct": 0.0,
                "status": "ERROR", "c_time": 0, "py_time": 0,
            })

    # Summary table
    print()
    print("=" * 90)
    print(f"{'Function':<12} {'C pixels':>9} {'Py pixels':>10} {'Overlap':>9} {'Overlap%':>9} {'Status':<16}")
    print("-" * 90)
    for r in results:
        print(f"{r['func']:<12} {r['c_pixels']:>9} {r['py_pixels']:>10} "
              f"{r['overlap_pixels']:>9} {r['overlap_pct']:>8.1f}% {r['status']:<16}")
    print("-" * 90)

    # Aggregate stats (excluding stubs)
    non_stub = [r for r in results if r["status"] not in ("STUB(expected)", "STUB", "ERROR")]
    if non_stub:
        avg_overlap = sum(r["overlap_pct"] for r in non_stub) / len(non_stub)
        passing = sum(1 for r in non_stub if r["overlap_pct"] > 50)
        print(f"\nNon-stub functions: {len(non_stub)}")
        print(f"Avg overlap: {avg_overlap:.1f}%")
        print(f"Passing (>50% overlap): {passing}/{len(non_stub)}")

    stubs = [r for r in results if "STUB" in r["status"]]
    if stubs:
        print(f"\nStubbed functions ({len(stubs)}): {', '.join(r['func'] for r in stubs)}")
        stub_ok = sum(1 for r in stubs if r["c_pixels"] <= 1)
        print(f"  Confirmed stubbed (<=1 C pixel): {stub_ok}/{len(stubs)}")

    print("\nDone.")


if __name__ == "__main__":
    main()
