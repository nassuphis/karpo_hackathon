#!/usr/bin/env python3
"""Visual comparison tests for poly_191 through poly_200: C sweep vs Python reference."""

import subprocess
import json
import struct
import sys
import os
import ast
import types
import numpy as np
from pathlib import Path

# We need matplotlib for PNG output
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

SWEEP_BIN = os.path.join(os.path.dirname(__file__), "lambda", "sweep_test")
POLY_PY = os.path.join(os.path.dirname(__file__), "lambda", "poly200.py")

CANVAS_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
N1, N2 = 100, 100


def load_python_functions():
    """Load poly_191..poly_200 from poly200.py using ast.parse."""
    with open(POLY_PY, "r") as f:
        source = f.read()

    tree = ast.parse(source)

    # We need the imports too
    namespace = {"np": np, "numpy": np}

    # Extract each function
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef):
            name = node.name
            if name.startswith("poly_"):
                try:
                    num = int(name.split("_")[1])
                except ValueError:
                    continue
                if 191 <= num <= 200:
                    # Extract just this function's source and compile it
                    func_source = ast.get_source_segment(source, node)
                    if func_source is None:
                        # Fallback: use line numbers
                        lines = source.split("\n")
                        func_lines = lines[node.lineno - 1 : node.end_lineno]
                        func_source = "\n".join(func_lines)
                    exec(compile(ast.Module(body=[node], type_ignores=[]), "<poly>", "exec"), namespace)
                    funcs[num] = namespace[name]

    return funcs


def run_c_pipeline(func_name, func_num):
    """Run coeffgen + solve for a given function. Returns set of (px, py) pixel coords or None on failure."""
    coeffs_file = f"/tmp/poly{func_num}_coeffs.bin"
    roots_file = f"/tmp/poly{func_num}_roots.bin"

    # Step 1: coeffgen
    coeffgen_input = json.dumps({
        "mode": "coeffgen",
        "function": func_name,
        "n1": N1,
        "n2": N2,
        "i1_start": 0,
        "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"],
        "times": 1,
        "dither_pass": 0
    })

    try:
        result = subprocess.run(
            [SWEEP_BIN, coeffs_file],
            input=coeffgen_input.encode(),
            capture_output=True,
            timeout=60
        )
        if result.returncode != 0:
            stderr = result.stderr.decode(errors="replace")
            print(f"  coeffgen failed (rc={result.returncode}): {stderr[:200]}")
            return None, 0, 0

        stdout = result.stdout.decode().strip()
        # Parse JSON from stdout
        meta = json.loads(stdout)
        n_coeffs = meta["n_coeffs"]
        n_t = meta["n_t"]
        print(f"  coeffgen: n_coeffs={n_coeffs}, n_t={n_t}")
    except Exception as e:
        print(f"  coeffgen exception: {e}")
        return None, 0, 0

    # Step 2: solve
    solve_input = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n1": N1,
        "n2": N2,
        "n_t": n_t
    })

    try:
        result = subprocess.run(
            [SWEEP_BIN, roots_file],
            input=solve_input.encode(),
            capture_output=True,
            timeout=120
        )
        if result.returncode != 0:
            stderr = result.stderr.decode(errors="replace")
            print(f"  solve failed (rc={result.returncode}): {stderr[:200]}")
            return None, n_coeffs, n_t
    except Exception as e:
        print(f"  solve exception: {e}")
        return None, n_coeffs, n_t

    # Step 3: Read roots binary - pairs of float32 (re, im)
    try:
        data = Path(roots_file).read_bytes()
        n_roots = len(data) // 8
        roots = struct.unpack(f"<{n_roots * 2}f", data)

        pixels = set()
        import math
        for i in range(n_roots):
            re = roots[2 * i]
            im = roots[2 * i + 1]
            if not (math.isfinite(re) and math.isfinite(im)):
                continue
            # Map to pixel coords: [-2,2] -> [0, 1000]
            px = int((re + EXTENT) / (2 * EXTENT) * CANVAS_SIZE)
            py = int((EXTENT - im) / (2 * EXTENT) * CANVAS_SIZE)  # flip y
            if 0 <= px < CANVAS_SIZE and 0 <= py < CANVAS_SIZE:
                pixels.add((px, py))

        return pixels, n_coeffs, n_t
    except Exception as e:
        print(f"  read roots exception: {e}")
        return None, n_coeffs, n_t


def run_python_pipeline(func, func_num):
    """Run Python reference: sweep (i1,i2), compute coeffs, reverse, np.roots, collect pixels."""
    pixels = set()

    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)

            try:
                coeffs = func(t1, t2)
                if coeffs is None or len(coeffs) == 0:
                    continue

                # Reverse coefficients
                coeffs = coeffs[::-1]

                # Remove leading zeros
                while len(coeffs) > 1 and coeffs[0] == 0:
                    coeffs = coeffs[1:]

                if len(coeffs) < 2:
                    continue

                # Check for NaN/Inf
                if not np.all(np.isfinite(coeffs)):
                    continue

                roots = np.roots(coeffs)

                for r in roots:
                    re, im = r.real, r.imag
                    px = int((re + EXTENT) / (2 * EXTENT) * CANVAS_SIZE)
                    py = int((EXTENT - im) / (2 * EXTENT) * CANVAS_SIZE)
                    if 0 <= px < CANVAS_SIZE and 0 <= py < CANVAS_SIZE:
                        pixels.add((px, py))
            except Exception:
                continue

    return pixels


def pixels_to_image(pixels, canvas_size=CANVAS_SIZE):
    """Convert set of (px, py) to a binary image array."""
    img = np.zeros((canvas_size, canvas_size), dtype=np.uint8)
    for px, py in pixels:
        img[py, px] = 255
    return img


def save_png(img, path):
    """Save grayscale image as PNG."""
    fig, ax = plt.subplots(1, 1, figsize=(10, 10), dpi=100)
    ax.imshow(img, cmap='gray', origin='upper')
    ax.set_xlim(0, CANVAS_SIZE)
    ax.set_ylim(CANVAS_SIZE, 0)
    ax.axis('off')
    plt.tight_layout(pad=0)
    plt.savefig(path, bbox_inches='tight', pad_inches=0, dpi=100)
    plt.close()


def main():
    print("Loading Python functions from poly200.py...")
    py_funcs = load_python_functions()
    print(f"Loaded {len(py_funcs)} functions: {sorted(py_funcs.keys())}")

    results = []

    for num in range(191, 201):
        func_name = f"poly_{num}"
        print(f"\n{'='*60}")
        print(f"Testing {func_name}")
        print(f"{'='*60}")

        # C pipeline
        print("  Running C pipeline...")
        c_pixels, n_coeffs, n_t = run_c_pipeline(func_name, num)

        if c_pixels is None:
            c_count = 0
            c_status = "STUBBED/FAILED"
            c_img = np.zeros((CANVAS_SIZE, CANVAS_SIZE), dtype=np.uint8)
        else:
            c_count = len(c_pixels)
            if c_count <= 1:
                c_status = "STUBBED (all-zero coeffs)"
            elif c_count > 0:
                c_status = "OK"
            else:
                c_status = "EMPTY"
            c_img = pixels_to_image(c_pixels)

        print(f"  C pixels: {c_count} ({c_status})")

        # Python pipeline
        print("  Running Python pipeline...")
        if num in py_funcs:
            py_pixels = run_python_pipeline(py_funcs[num], num)
            py_count = len(py_pixels)
            py_status = "OK" if py_count > 0 else "EMPTY"
            py_img = pixels_to_image(py_pixels)
        else:
            py_pixels = set()
            py_count = 0
            py_status = "MISSING"
            py_img = np.zeros((CANVAS_SIZE, CANVAS_SIZE), dtype=np.uint8)

        print(f"  Python pixels: {py_count} ({py_status})")

        # Overlap
        if c_pixels is not None and py_pixels:
            overlap = c_pixels & py_pixels
            overlap_count = len(overlap)
            # Overlap as % of the smaller set
            min_count = min(c_count, py_count) if min(c_count, py_count) > 0 else 1
            overlap_pct = 100.0 * overlap_count / min_count
        else:
            overlap_count = 0
            overlap_pct = 0.0

        print(f"  Overlap: {overlap_count} pixels ({overlap_pct:.1f}%)")

        # Save PNGs
        c_path = f"/tmp/poly{num}_c.png"
        py_path = f"/tmp/poly{num}_py.png"
        save_png(c_img, c_path)
        save_png(py_img, py_path)
        print(f"  Saved: {c_path}, {py_path}")

        results.append({
            "func": func_name,
            "c_pixels": c_count,
            "c_status": c_status,
            "py_pixels": py_count,
            "py_status": py_status,
            "overlap": overlap_count,
            "overlap_pct": overlap_pct,
        })

    # Summary table
    print(f"\n{'='*80}")
    print("SUMMARY")
    print(f"{'='*80}")
    print(f"{'Function':<12} {'C pixels':>10} {'C status':<16} {'Py pixels':>10} {'Py status':<10} {'Overlap':>10} {'Overlap%':>10}")
    print("-" * 80)
    for r in results:
        print(f"{r['func']:<12} {r['c_pixels']:>10} {r['c_status']:<16} {r['py_pixels']:>10} {r['py_status']:<10} {r['overlap']:>10} {r['overlap_pct']:>9.1f}%")


if __name__ == "__main__":
    import warnings
    warnings.filterwarnings("ignore", category=RuntimeWarning)
    main()
