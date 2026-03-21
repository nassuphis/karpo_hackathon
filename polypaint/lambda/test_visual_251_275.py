"""
Visual comparison: C sweep pipeline vs Python reference for poly_251..poly_275.

C pipeline:
  1. coeffgen with unit_circle param_transforms + rev coeff_transforms
  2. solve from coefficients
  3. Read float32 root pairs, rasterize to 1000x1000, extent [-2,2]x[-2,2]

Python pipeline:
  Load poly functions from poly300.py via ast.parse+exec,
  100x100 grid, unit_circle params, call func, reverse coeffs, np.roots, rasterize same.

Reports: C pixels, Py pixels, overlap % for each function. Summary table at end.
"""
import ast
import json
import math
import struct
import subprocess
import sys
import time
import warnings

import numpy as np

warnings.filterwarnings("ignore")

SWEEP = "./sweep_test"
POLY_SRC = "./poly300.py"
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
N1, N2 = 100, 100

# ---------- Load Python functions from poly300.py via ast ----------

def load_poly_functions(src_path, func_names):
    """Parse poly300.py and extract specific function definitions."""
    with open(src_path, "r") as f:
        source = f.read()

    tree = ast.parse(source)

    # Build a module with only the needed functions plus imports
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in func_names:
            # Compile each function individually
            mod = ast.Module(body=[node], type_ignores=[])
            ast.fix_missing_locations(mod)
            code = compile(mod, src_path, "exec")
            ns = {"np": np, "math": __import__("math"), "cmath": __import__("cmath")}
            exec(code, ns)
            funcs[node.name] = ns[node.name]

    return funcs


# ---------- C pipeline ----------

def run_c_pipeline(func_name):
    """Run coeffgen + solve via the C binary, return root positions as float32 pairs."""
    coeffs_path = f"/tmp/{func_name}_coeffs.bin"
    roots_path = f"/tmp/{func_name}_roots.bin"

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
        [SWEEP, coeffs_path],
        input=json.dumps(spec),
        capture_output=True, text=True, timeout=60,
    )
    if result.returncode != 0:
        raise RuntimeError(f"coeffgen failed for {func_name}: {result.stderr.strip()}")

    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]

    # Step 2: solve
    solve_spec = {
        "mode": "solve",
        "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "match_roots": False,
    }
    result2 = subprocess.run(
        [SWEEP, roots_path],
        input=json.dumps(solve_spec),
        capture_output=True, text=True, timeout=120,
    )
    if result2.returncode != 0:
        raise RuntimeError(f"solve failed for {func_name}: {result2.stderr.strip()}")

    solve_meta = json.loads(result2.stdout)
    degree = solve_meta["degree"]

    # Step 3: read roots
    with open(roots_path, "rb") as f:
        data = f.read()

    n_roots_total = len(data) // 8  # 4 bytes re + 4 bytes im per root
    roots = np.frombuffer(data, dtype=np.float32).reshape(-1, 2)

    return roots, degree


# ---------- Python pipeline ----------

def run_python_pipeline(func, n1, n2):
    """Evaluate polynomial on unit_circle grid, reverse coeffs, find roots."""
    all_roots = []
    for i1 in range(n1):
        for i2 in range(n2):
            x1 = i1 / n1
            x2 = i2 / n2
            # unit_circle transform
            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)

            try:
                coeffs = func(t1, t2)
                if coeffs is None or len(coeffs) == 0:
                    continue
                coeffs = np.asarray(coeffs, dtype=complex)
                # Reverse coefficients (coeff_transforms: ["rev"])
                coeffs = coeffs[::-1]
                # Strip leading zeros
                while len(coeffs) > 1 and abs(coeffs[0]) < 1e-15:
                    coeffs = coeffs[1:]
                if len(coeffs) < 2:
                    continue
                roots = np.roots(coeffs)
                for r in roots:
                    all_roots.append((r.real, r.imag))
            except Exception:
                continue

    return np.array(all_roots, dtype=np.float32) if all_roots else np.zeros((0, 2), dtype=np.float32)


# ---------- Rasterization ----------

def rasterize(roots, img_size=IMG_SIZE, extent=EXTENT):
    """Rasterize root positions to a binary image (set of occupied pixels)."""
    if len(roots) == 0:
        return set()

    pixels = set()
    scale = img_size / (2 * extent)
    cx = img_size / 2
    cy = img_size / 2

    for re, im in roots:
        px = int(re * scale + cx)
        py = int(-im * scale + cy)  # flip y
        if 0 <= px < img_size and 0 <= py < img_size:
            pixels.add((px, py))

    return pixels


# ---------- Main ----------

def main():
    func_names = [f"poly_{n}" for n in range(251, 276)]

    print(f"Loading Python functions from {POLY_SRC}...")
    py_funcs = load_poly_functions(POLY_SRC, func_names)
    loaded = set(py_funcs.keys())
    missing_py = [f for f in func_names if f not in loaded]
    if missing_py:
        print(f"WARNING: Could not load Python functions: {missing_py}")

    results = []

    for func_name in func_names:
        print(f"\n{'='*60}")
        print(f"Testing {func_name}...")

        # --- C pipeline ---
        try:
            t0 = time.time()
            c_roots, degree = run_c_pipeline(func_name)
            c_time = time.time() - t0
            c_pixels = rasterize(c_roots)
            print(f"  C: {len(c_roots)} roots, {len(c_pixels)} pixels, degree={degree}, {c_time:.2f}s")
        except Exception as e:
            print(f"  C FAILED: {e}")
            results.append({
                "func": func_name, "c_pixels": 0, "py_pixels": 0,
                "overlap": 0, "overlap_pct": 0.0, "status": f"C_FAIL: {e}"
            })
            continue

        # --- Python pipeline ---
        if func_name not in py_funcs:
            print(f"  Python: SKIPPED (not loaded)")
            results.append({
                "func": func_name, "c_pixels": len(c_pixels), "py_pixels": 0,
                "overlap": 0, "overlap_pct": 0.0, "status": "PY_MISSING"
            })
            continue

        try:
            t0 = time.time()
            py_roots = run_python_pipeline(py_funcs[func_name], N1, N2)
            py_time = time.time() - t0
            py_pixels = rasterize(py_roots)
            print(f"  Py: {len(py_roots)} roots, {len(py_pixels)} pixels, {py_time:.2f}s")
        except Exception as e:
            print(f"  Python FAILED: {e}")
            results.append({
                "func": func_name, "c_pixels": len(c_pixels), "py_pixels": 0,
                "overlap": 0, "overlap_pct": 0.0, "status": f"PY_FAIL: {e}"
            })
            continue

        # --- Comparison ---
        overlap = c_pixels & py_pixels
        union = c_pixels | py_pixels
        overlap_pct = 100.0 * len(overlap) / len(union) if union else 0.0

        c_only = len(c_pixels - py_pixels)
        py_only = len(py_pixels - c_pixels)

        print(f"  Overlap: {len(overlap)} pixels ({overlap_pct:.1f}%)")
        print(f"  C-only: {c_only}, Py-only: {py_only}")

        status = "OK" if overlap_pct >= 90 else ("WARN" if overlap_pct >= 70 else "FAIL")

        results.append({
            "func": func_name,
            "c_pixels": len(c_pixels),
            "py_pixels": len(py_pixels),
            "overlap": len(overlap),
            "overlap_pct": overlap_pct,
            "c_only": c_only,
            "py_only": py_only,
            "status": status,
        })

    # --- Summary table ---
    print(f"\n\n{'='*80}")
    print(f"SUMMARY: Visual comparison poly_251 .. poly_275")
    print(f"Grid: {N1}x{N2}, Image: {IMG_SIZE}x{IMG_SIZE}, Extent: [-{EXTENT},{EXTENT}]")
    print(f"{'='*80}")
    print(f"{'Function':<12} {'C pix':>8} {'Py pix':>8} {'Overlap':>8} {'Overlap%':>9} {'Status':<10}")
    print(f"{'-'*12} {'-'*8} {'-'*8} {'-'*8} {'-'*9} {'-'*10}")

    pass_count = 0
    warn_count = 0
    fail_count = 0

    for r in results:
        status = r["status"]
        print(f"{r['func']:<12} {r['c_pixels']:>8} {r['py_pixels']:>8} {r['overlap']:>8} {r['overlap_pct']:>8.1f}% {status:<10}")
        if status == "OK":
            pass_count += 1
        elif status == "WARN":
            warn_count += 1
        else:
            fail_count += 1

    print(f"\nTotal: {len(results)} functions | PASS: {pass_count} | WARN: {warn_count} | FAIL: {fail_count}")

    if fail_count > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
