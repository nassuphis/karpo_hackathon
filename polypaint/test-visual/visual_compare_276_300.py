"""
Visual comparison tests for poly_276 through poly_300.
Compares C sweep pipeline vs Python reference (np.roots).

C pipeline:
  1. coeffgen: sweep_test generates coefficients binary
  2. solve: sweep_test solves for roots from coefficients binary
  3. Read roots (float32 pairs), plot on 1000x1000, extent [-2,2]x[-2,2]

Python pipeline:
  1. Load poly functions from poly300.py via ast.parse
  2. 100x100 grid, unit_circle params, call func, reverse coeffs, np.roots
  3. Plot on same 1000x1000 grid

Reports: C pixels, Py pixels, overlap % for each. Summary table at end.
"""

import ast
import json
import math
import struct
import subprocess
import sys
import textwrap
import numpy as np
import os

SWEEP = os.path.join(os.path.dirname(__file__), "..", "lambda", "sweep_test")
POLY_SRC = os.path.join(os.path.dirname(__file__), "..", "lambda", "poly300.py")

# Image parameters
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]

POLY_START = 276
POLY_END = 300


def load_poly_functions(src_path, start, end):
    """Load poly functions from source file using ast.parse + exec."""
    with open(src_path, "r") as f:
        source = f.read()

    tree = ast.parse(source)

    # Extract function defs for our range
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef):
            name = node.name
            if name.startswith("poly_"):
                try:
                    num = int(name.split("_")[1])
                except ValueError:
                    continue
                if start <= num <= end:
                    funcs[name] = node

    # Build a module with just numpy imports + the functions we need
    results = {}
    for name, func_node in sorted(funcs.items()):
        # Extract source lines for this function
        func_source = ast.get_source_segment(source, func_node)
        if func_source is None:
            # Fallback: use line numbers
            lines = source.split("\n")
            func_lines = lines[func_node.lineno - 1 : func_node.end_lineno]
            func_source = "\n".join(func_lines)

        module_code = (
            "import math\nimport cmath\nimport numpy as np\n\n" + func_source
        )
        ns = {}
        try:
            exec(compile(module_code, f"<{name}>", "exec"), ns)
            results[name] = ns[name]
        except Exception as e:
            print(f"WARNING: Failed to compile {name}: {e}")

    return results


def root_to_pixel(re, im):
    """Convert root position to pixel coordinates."""
    px = int((re + EXTENT) / (2 * EXTENT) * IMG_SIZE)
    py = int((-im + EXTENT) / (2 * EXTENT) * IMG_SIZE)
    return px, py


def roots_to_pixel_set(roots_re, roots_im):
    """Convert arrays of root positions to a set of (px, py) tuples."""
    pixels = set()
    for r, i in zip(roots_re, roots_im):
        if not (math.isfinite(r) and math.isfinite(i)):
            continue
        px, py = root_to_pixel(r, i)
        if 0 <= px < IMG_SIZE and 0 <= py < IMG_SIZE:
            pixels.add((px, py))
    return pixels


def run_c_pipeline(func_name, n1=100, n2=100):
    """Run C coeffgen + solve pipeline, return pixel set."""
    coeffs_file = f"/tmp/{func_name}_coeffs.bin"
    roots_file = f"/tmp/{func_name}_roots.bin"

    # Step 1: coeffgen
    spec = json.dumps({
        "mode": "coeffgen",
        "function": func_name,
        "n1": n1,
        "n2": n2,
        "i1_start": 0,
        "i1_end": n1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"],
        "times": 1,
        "dither_pass": 0,
    })

    result = subprocess.run(
        [SWEEP, coeffs_file],
        input=spec,
        capture_output=True,
        text=True,
        timeout=60,
    )
    if result.returncode != 0:
        raise RuntimeError(f"coeffgen failed for {func_name}: {result.stderr}")

    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]
    degree = meta["degree"]

    # Step 2: solve
    solve_spec = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n1": n1,
        "n2": n2,
        "i1_start": 0,
        "i1_end": n1,
        "match_roots": False,
    })

    result = subprocess.run(
        [SWEEP, roots_file],
        input=solve_spec,
        capture_output=True,
        text=True,
        timeout=120,
    )
    if result.returncode != 0:
        raise RuntimeError(f"solve failed for {func_name}: {result.stderr}")

    solve_meta = json.loads(result.stdout)
    solve_degree = solve_meta["degree"]
    solve_n_t = solve_meta["n_t"]

    # Step 3: Read roots
    with open(roots_file, "rb") as f:
        data = f.read()

    # Each step: degree * 2 floats (re, im pairs), float32
    step_size = solve_degree * 2 * 4  # bytes
    n_steps = len(data) // step_size

    all_re = []
    all_im = []
    for s in range(n_steps):
        offset = s * step_size
        floats = struct.unpack_from(f"<{solve_degree * 2}f", data, offset)
        for k in range(solve_degree):
            all_re.append(floats[k * 2])
            all_im.append(floats[k * 2 + 1])

    pixels = roots_to_pixel_set(all_re, all_im)

    # Cleanup
    try:
        os.unlink(coeffs_file)
        os.unlink(roots_file)
    except OSError:
        pass

    return pixels, len(all_re)


def run_python_pipeline(func, n1=100, n2=100):
    """Run Python reference pipeline, return pixel set."""
    all_re = []
    all_im = []

    for i1 in range(n1):
        for i2 in range(n2):
            x1 = i1 / n1
            x2 = i2 / n2
            # unit_circle transform
            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)

            try:
                coeffs = func(t1, t2)
            except Exception:
                continue

            if coeffs is None or len(coeffs) < 2:
                continue

            # Reverse coefficients (coeff_transforms: ["rev"])
            coeffs = coeffs[::-1]

            # Skip if all zeros or leading coeff is zero
            if np.all(np.abs(coeffs) < 1e-30):
                continue

            # Strip leading zeros
            while len(coeffs) > 1 and abs(coeffs[0]) < 1e-30:
                coeffs = coeffs[1:]

            if len(coeffs) < 2:
                continue

            try:
                roots = np.roots(coeffs)
            except Exception:
                continue

            for r in roots:
                if np.isfinite(r.real) and np.isfinite(r.imag):
                    all_re.append(r.real)
                    all_im.append(r.imag)

    pixels = roots_to_pixel_set(all_re, all_im)
    return pixels, len(all_re)


def compute_overlap(set_a, set_b):
    """Compute overlap percentage between two pixel sets."""
    if len(set_a) == 0 and len(set_b) == 0:
        return 100.0
    if len(set_a) == 0 or len(set_b) == 0:
        return 0.0
    intersection = set_a & set_b
    union = set_a | set_b
    return 100.0 * len(intersection) / len(union) if len(union) > 0 else 0.0


def main():
    print("Loading Python poly functions from source...")
    py_funcs = load_poly_functions(POLY_SRC, POLY_START, POLY_END)
    print(f"Loaded {len(py_funcs)} functions: {', '.join(sorted(py_funcs.keys()))}")
    print()

    results = []

    for num in range(POLY_START, POLY_END + 1):
        func_name = f"poly_{num}"
        print(f"=== {func_name} ===")

        # C pipeline
        try:
            c_pixels, c_total_roots = run_c_pipeline(func_name)
            print(f"  C:  {len(c_pixels)} unique pixels ({c_total_roots} total roots)")
        except Exception as e:
            print(f"  C:  FAILED - {e}")
            c_pixels = set()
            c_total_roots = 0

        # Python pipeline
        if func_name in py_funcs:
            try:
                py_pixels, py_total_roots = run_python_pipeline(py_funcs[func_name])
                print(f"  Py: {len(py_pixels)} unique pixels ({py_total_roots} total roots)")
            except Exception as e:
                print(f"  Py: FAILED - {e}")
                py_pixels = set()
                py_total_roots = 0
        else:
            print(f"  Py: MISSING (function not found in source)")
            py_pixels = set()
            py_total_roots = 0

        overlap = compute_overlap(c_pixels, py_pixels)
        c_only = len(c_pixels - py_pixels)
        py_only = len(py_pixels - c_pixels)
        both = len(c_pixels & py_pixels)

        print(f"  Overlap: {overlap:.1f}% (both={both}, C-only={c_only}, Py-only={py_only})")
        print()

        results.append({
            "func": func_name,
            "c_pixels": len(c_pixels),
            "py_pixels": len(py_pixels),
            "overlap": overlap,
            "both": both,
            "c_only": c_only,
            "py_only": py_only,
        })

    # Summary table
    print()
    print("=" * 90)
    print(f"{'Function':<12} {'C pixels':>10} {'Py pixels':>10} {'Both':>8} {'C-only':>8} {'Py-only':>8} {'Overlap%':>10}")
    print("-" * 90)
    for r in results:
        print(
            f"{r['func']:<12} {r['c_pixels']:>10} {r['py_pixels']:>10} "
            f"{r['both']:>8} {r['c_only']:>8} {r['py_only']:>8} {r['overlap']:>9.1f}%"
        )
    print("-" * 90)

    # Averages
    overlaps = [r["overlap"] for r in results]
    avg_overlap = sum(overlaps) / len(overlaps) if overlaps else 0
    min_overlap = min(overlaps) if overlaps else 0
    max_overlap = max(overlaps) if overlaps else 0
    print(f"{'AVERAGE':<12} {'':>10} {'':>10} {'':>8} {'':>8} {'':>8} {avg_overlap:>9.1f}%")
    print(f"{'MIN':<12} {'':>10} {'':>10} {'':>8} {'':>8} {'':>8} {min_overlap:>9.1f}%")
    print(f"{'MAX':<12} {'':>10} {'':>10} {'':>8} {'':>8} {'':>8} {max_overlap:>9.1f}%")
    print("=" * 90)


if __name__ == "__main__":
    main()
