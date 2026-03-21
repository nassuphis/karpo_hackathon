"""
Visual comparison: C sweep pipeline vs Python reference for poly_301..poly_325.

Runs with: uv run python visual_cmp_301_325.py
"""
import ast
import json
import os
import struct
import subprocess
import sys
import time
import numpy as np

SWEEP = "./sweep_test"
N1, N2 = 100, 100
EXTENT = 2.0  # [-2,2] x [-2,2]
IMG_SIZE = 1000

# ── Load Python functions from poly400.py via ast ──────────────────────────

def load_poly_funcs(path, names):
    """Parse poly400.py with ast and exec individual functions."""
    with open(path) as f:
        source = f.read()

    # Build a minimal namespace with required modules
    ns = {
        "np": np,
        "math": __import__("math"),
        "cmath": __import__("cmath"),
        "pi": __import__("math").pi,
    }

    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in names:
            # Extract just this function
            func_source = ast.get_source_segment(source, node)
            if func_source is None:
                # Fallback: compile the single node
                mod = ast.Module(body=[node], type_ignores=[])
                code = compile(mod, path, "exec")
                exec(code, ns)
            else:
                exec(func_source, ns)
            if node.name in ns:
                funcs[node.name] = ns[node.name]
    return funcs


# ── C pipeline ─────────────────────────────────────────────────────────────

def run_c_pipeline(func_name):
    """Run coeffgen + solve via sweep_test, return root positions as (n_t, degree, 2) array."""
    coeffs_path = f"/tmp/{func_name}_coeffs.bin"
    roots_path = f"/tmp/{func_name}_roots.bin"

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
    r = subprocess.run([SWEEP, coeffs_path], input=spec, capture_output=True, text=True, timeout=60)
    if r.returncode != 0:
        print(f"  coeffgen FAILED: {r.stderr.strip()}")
        return None, None, None
    meta = json.loads(r.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]

    # Step 2: solve
    spec2 = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "match_roots": False,
    })
    r2 = subprocess.run([SWEEP, roots_path], input=spec2, capture_output=True, text=True, timeout=120)
    if r2.returncode != 0:
        print(f"  solve FAILED: {r2.stderr.strip()}")
        return None, None, None
    meta2 = json.loads(r2.stdout)
    degree = meta2["degree"]
    n_t_actual = meta2["n_t"]

    # Read roots: float32 pairs
    with open(roots_path, "rb") as f:
        data = f.read()
    n_roots_total = len(data) // 8  # 2 floats per root
    roots = np.frombuffer(data, dtype=np.float32).reshape(-1, 2)
    return roots, degree, n_t_actual


def roots_to_pixels(roots, extent, img_size):
    """Convert root positions to pixel set."""
    if roots is None or len(roots) == 0:
        return set()
    re = roots[:, 0]
    im = roots[:, 1]
    # Map [-extent, extent] -> [0, img_size)
    px = ((re + extent) / (2 * extent) * img_size).astype(np.int32)
    py = ((extent - im) / (2 * extent) * img_size).astype(np.int32)  # flip y
    # Filter in-bounds
    mask = (px >= 0) & (px < img_size) & (py >= 0) & (py < img_size)
    px, py = px[mask], py[mask]
    return set(zip(px.tolist(), py.tolist()))


# ── Python pipeline ────────────────────────────────────────────────────────

def run_python_pipeline(py_func):
    """100x100 grid, unit_circle params, call func, reverse coeffs, np.roots, collect."""
    all_roots = []
    for i1 in range(N1):
        x1 = i1 / N1
        t1 = np.exp(2j * np.pi * x1)
        for i2 in range(N2):
            x2 = i2 / N2
            t2 = np.exp(2j * np.pi * x2)
            try:
                coeffs = py_func(t1, t2)
                if coeffs is None or len(coeffs) < 2:
                    continue
                # Reverse (coeff_transforms: ["rev"])
                coeffs = coeffs[::-1]
                # Strip leading zeros
                while len(coeffs) > 1 and abs(coeffs[0]) < 1e-15:
                    coeffs = coeffs[1:]
                if len(coeffs) < 2:
                    continue
                # Check for all-zero
                if np.all(np.abs(coeffs) < 1e-15):
                    continue
                rts = np.roots(coeffs)
                for r in rts:
                    all_roots.append((r.real, r.imag))
            except Exception:
                continue
    if not all_roots:
        return None
    return np.array(all_roots, dtype=np.float32)


# ── Main ───────────────────────────────────────────────────────────────────

def main():
    func_names = [f"poly_{i}" for i in range(301, 326)]

    print("Loading Python functions from poly400.py...")
    py_funcs = load_poly_funcs("poly400.py", set(func_names))
    print(f"  Loaded: {sorted(py_funcs.keys())}\n")

    results = []

    for fname in func_names:
        print(f"{'='*60}")
        print(f"Testing {fname}")
        print(f"{'='*60}")

        # C pipeline
        t0 = time.time()
        c_roots, c_degree, c_nt = run_c_pipeline(fname)
        c_time = time.time() - t0
        c_pixels = roots_to_pixels(c_roots, EXTENT, IMG_SIZE)
        print(f"  C:  {len(c_pixels):>7} pixels  (degree={c_degree}, n_t={c_nt}, {c_time:.1f}s)")

        # Python pipeline
        py_func = py_funcs.get(fname)
        if py_func is None:
            print(f"  Python function {fname} not found in poly400.py!")
            results.append((fname, len(c_pixels), 0, 0, 0.0))
            continue

        t0 = time.time()
        py_roots = run_python_pipeline(py_func)
        py_time = time.time() - t0
        py_pixels = roots_to_pixels(py_roots, EXTENT, IMG_SIZE)
        print(f"  Py: {len(py_pixels):>7} pixels  ({py_time:.1f}s)")

        # Compare
        if len(c_pixels) == 0 and len(py_pixels) == 0:
            overlap = 0
            pct = 100.0
            print(f"  Both empty (stubbed?)")
        elif len(c_pixels) == 0 or len(py_pixels) == 0:
            overlap = 0
            pct = 0.0
            print(f"  One side empty => overlap 0%")
        else:
            overlap = len(c_pixels & py_pixels)
            union = len(c_pixels | py_pixels)
            pct = 100.0 * overlap / union if union > 0 else 0.0
            print(f"  Overlap: {overlap} / {union} = {pct:.1f}% (Jaccard)")

        results.append((fname, len(c_pixels), len(py_pixels), overlap, pct))
        print()

    # Summary table
    print("\n" + "=" * 72)
    print(f"{'Function':<12} {'C pixels':>10} {'Py pixels':>10} {'Overlap':>10} {'Jaccard%':>10}")
    print("-" * 72)
    for fname, cp, pp, ov, pct in results:
        print(f"{fname:<12} {cp:>10} {pp:>10} {ov:>10} {pct:>9.1f}%")
    print("=" * 72)

    # Overall stats
    total = len(results)
    high = sum(1 for _, _, _, _, p in results if p >= 80.0)
    mid = sum(1 for _, _, _, _, p in results if 50.0 <= p < 80.0)
    low = sum(1 for _, _, _, _, p in results if p < 50.0)
    print(f"\nTotal: {total}  |  >=80%: {high}  |  50-79%: {mid}  |  <50%: {low}")


if __name__ == "__main__":
    main()
