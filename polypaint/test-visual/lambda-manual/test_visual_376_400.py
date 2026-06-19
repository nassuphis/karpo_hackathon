"""
Visual comparison tests for poly_376 through poly_400.
Compares C sweep pipeline vs Python reference (roots plotted on 1000x1000 grid).

Run: uv run python test-visual/lambda-manual/test_visual_376_400.py
"""
import ast
import json
import struct
import subprocess
import sys
import os
import numpy as np
import textwrap

LAMBDA_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "lambda"))
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
N1, N2 = 100, 100

def load_poly_funcs_from_source():
    """Parse poly400.py source and extract poly_376..poly_400 functions."""
    src_path = os.path.join(LAMBDA_DIR, "poly400.py")
    with open(src_path, "r") as f:
        source = f.read()

    # Remove relative imports that won't work standalone
    lines = source.split("\n")
    filtered = []
    for line in lines:
        if line.startswith("from .") or (line.startswith("from ") and "import" in line and "polystate" in line):
            continue
        if line.startswith("from ") and "import" in line and "letters" in line:
            continue
        if line.startswith("from ") and "import" in line and "zfrm" in line:
            continue
        filtered.append(line)
    clean_source = "\n".join(filtered)

    # Create a namespace with required modules
    ns = {}
    exec("import math, cmath, numpy as np; pi = math.pi", ns)
    # Try scipy but don't fail if not available
    try:
        exec("from scipy.special import sph_harm", ns)
    except ImportError:
        pass

    # Parse and exec only the function defs we need
    tree = ast.parse(clean_source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name.startswith("poly_"):
            num = int(node.name.split("_")[1])
            if 376 <= num <= 400:
                func_source = ast.get_source_segment(clean_source, node)
                if func_source:
                    exec(func_source, ns)
                    funcs[num] = ns[node.name]
    return funcs


def run_c_pipeline(func_name):
    """Run coeffgen + solve for a poly function, return roots as float32 pairs."""
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

    result = subprocess.run(
        [SWEEP, coeffs_file],
        input=spec, capture_output=True, text=True, timeout=60
    )
    if result.returncode != 0:
        raise RuntimeError(f"coeffgen failed for {func_name}: {result.stderr}")

    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]

    # Step 2: solve
    solve_spec = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "match_roots": False,
    })

    result2 = subprocess.run(
        [SWEEP, roots_file],
        input=solve_spec, capture_output=True, text=True, timeout=120
    )
    if result2.returncode != 0:
        raise RuntimeError(f"solve failed for {func_name}: {result2.stderr}")

    solve_meta = json.loads(result2.stdout)
    degree = solve_meta["degree"]
    n_t_solve = solve_meta["n_t"]

    # Step 3: read roots binary - float32 pairs
    with open(roots_file, "rb") as f:
        data = f.read()

    n_roots_total = len(data) // 8  # 8 bytes per root (2 x float32)
    floats = struct.unpack(f"<{n_roots_total * 2}f", data)

    roots_re = []
    roots_im = []
    for i in range(n_roots_total):
        roots_re.append(floats[i * 2])
        roots_im.append(floats[i * 2 + 1])

    return np.array(roots_re, dtype=np.float32), np.array(roots_im, dtype=np.float32), degree, n_t_solve


def run_python_pipeline(py_func, n_coeffs_hint=None):
    """Run Python reference: 100x100 grid, unit circle params, reverse coeffs, np.roots."""
    all_roots_re = []
    all_roots_im = []

    for i1 in range(N1):
        for i2 in range(N2):
            # unit_circle transform: t = e^{2*pi*i * idx/N}
            x1 = i1 / N1
            x2 = i2 / N2
            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)

            coeffs = py_func(t1, t2)
            # reverse coefficients (coeff_transforms: ["rev"])
            coeffs = coeffs[::-1]

            # Skip if all zeros or too short
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
                roots = np.roots(coeffs)
                for r in roots:
                    all_roots_re.append(r.real)
                    all_roots_im.append(r.imag)
            except Exception:
                pass

    return np.array(all_roots_re, dtype=np.float64), np.array(all_roots_im, dtype=np.float64)


def roots_to_pixels(roots_re, roots_im):
    """Convert root positions to pixel set on a 1000x1000 grid, extent [-2,2]x[-2,2]."""
    pixels = set()
    scale = IMG_SIZE / (2 * EXTENT)
    cx, cy = IMG_SIZE / 2, IMG_SIZE / 2

    for re, im in zip(roots_re, roots_im):
        if not (np.isfinite(re) and np.isfinite(im)):
            continue
        px = int(re * scale + cx)
        py = int(-im * scale + cy)  # flip y
        if 0 <= px < IMG_SIZE and 0 <= py < IMG_SIZE:
            pixels.add((px, py))
    return pixels


def compare(func_num, py_func):
    """Run both pipelines and compare pixel overlap."""
    func_name = f"poly_{func_num}"

    # C pipeline
    try:
        c_re, c_im, degree, n_t = run_c_pipeline(func_name)
        c_pixels = roots_to_pixels(c_re, c_im)
    except Exception as e:
        return {
            "func": func_name,
            "status": "C_ERROR",
            "error": str(e),
            "c_pixels": 0,
            "py_pixels": 0,
            "overlap_pct": 0.0,
        }

    # Python pipeline
    try:
        py_re, py_im = run_python_pipeline(py_func)
        py_pixels = roots_to_pixels(py_re, py_im)
    except Exception as e:
        return {
            "func": func_name,
            "status": "PY_ERROR",
            "error": str(e),
            "c_pixels": len(c_pixels),
            "py_pixels": 0,
            "overlap_pct": 0.0,
        }

    # Compute overlap
    overlap = c_pixels & py_pixels
    union = c_pixels | py_pixels
    overlap_pct = 100.0 * len(overlap) / len(union) if union else 0.0

    status = "PASS" if overlap_pct >= 80.0 else ("WARN" if overlap_pct >= 50.0 else "FAIL")

    return {
        "func": func_name,
        "status": status,
        "c_pixels": len(c_pixels),
        "py_pixels": len(py_pixels),
        "overlap": len(overlap),
        "union": len(union),
        "overlap_pct": overlap_pct,
    }


def main():
    print("Loading Python poly functions from poly400.py source...")
    funcs = load_poly_funcs_from_source()
    print(f"  Loaded {len(funcs)} functions: {sorted(funcs.keys())}")
    print()

    results = []
    for num in range(376, 401):
        if num not in funcs:
            print(f"poly_{num}: NOT FOUND in poly400.py, skipping")
            results.append({
                "func": f"poly_{num}",
                "status": "SKIP",
                "c_pixels": 0,
                "py_pixels": 0,
                "overlap_pct": 0.0,
            })
            continue

        if num == 390:
            print(f"poly_{num}: STUBBED (uses np.random), skipping Python comparison")
            # Still run C to get pixel count
            try:
                c_re, c_im, degree, n_t = run_c_pipeline(f"poly_{num}")
                c_pixels = roots_to_pixels(c_re, c_im)
                results.append({
                    "func": f"poly_{num}",
                    "status": "STUBBED",
                    "c_pixels": len(c_pixels),
                    "py_pixels": 0,
                    "overlap_pct": 0.0,
                    "note": "non-deterministic (np.random)",
                })
                print(f"  C pixels: {len(c_pixels)}")
            except Exception as e:
                results.append({
                    "func": f"poly_{num}",
                    "status": "STUBBED",
                    "c_pixels": 0,
                    "py_pixels": 0,
                    "overlap_pct": 0.0,
                    "note": f"C error: {e}",
                })
            continue

        print(f"poly_{num}: running C pipeline...", end=" ", flush=True)
        r = compare(num, funcs[num])
        print(f"C={r['c_pixels']}px  Py={r['py_pixels']}px  overlap={r.get('overlap_pct', 0):.1f}%  [{r['status']}]")
        if "error" in r:
            print(f"  ERROR: {r['error']}")
        results.append(r)

    # Summary table
    print()
    print("=" * 78)
    print(f"{'Function':<12} {'Status':<10} {'C pixels':>10} {'Py pixels':>10} {'Overlap%':>10}")
    print("-" * 78)
    for r in results:
        print(f"{r['func']:<12} {r['status']:<10} {r['c_pixels']:>10} {r['py_pixels']:>10} {r.get('overlap_pct', 0):>9.1f}%")
    print("=" * 78)

    # Stats
    compared = [r for r in results if r["status"] in ("PASS", "WARN", "FAIL")]
    if compared:
        pass_count = sum(1 for r in compared if r["status"] == "PASS")
        warn_count = sum(1 for r in compared if r["status"] == "WARN")
        fail_count = sum(1 for r in compared if r["status"] == "FAIL")
        avg_overlap = np.mean([r["overlap_pct"] for r in compared])
        print(f"\nCompared: {len(compared)}  PASS: {pass_count}  WARN: {warn_count}  FAIL: {fail_count}")
        print(f"Average overlap: {avg_overlap:.1f}%")

    # Return exit code
    any_fail = any(r["status"] == "FAIL" for r in results)
    return 1 if any_fail else 0


if __name__ == "__main__":
    sys.exit(main())
