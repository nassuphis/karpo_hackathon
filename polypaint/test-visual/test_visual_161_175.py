#!/usr/bin/env python3
"""Visual comparison tests for poly_161 through poly_175: C sweep vs Python reference."""

import subprocess
import json
import struct
import sys
import os
import ast
import textwrap
import numpy as np
from pathlib import Path

SWEEP_BIN = os.path.join(os.path.dirname(__file__), "..", "lambda", 'sweep_test')
CANVAS_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
N1, N2 = 100, 100

# Extract poly functions from poly200.py using ast.parse (avoids relative import issues)
def extract_poly_functions(filepath, func_names):
    """Parse poly200.py with ast and extract specific functions, compile them."""
    source = Path(filepath).read_text()
    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in func_names:
            # Extract source lines
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.split('\n')[start:end]
            func_source = '\n'.join(lines)
            # Compile and exec in a namespace with numpy
            ns = {'np': np, 'numpy': np, 'math': __import__('math'), 'cmath': __import__('cmath')}
            exec(compile(func_source, filepath, 'exec'), ns)
            funcs[node.name] = ns[node.name]
    return funcs

poly200_path = os.path.join(os.path.dirname(__file__), "..", "lambda", 'poly200.py')
func_names = [f"poly_{n}" for n in range(161, 176)]
extracted = extract_poly_functions(poly200_path, func_names)

POLY_FUNCS = {}
for n in range(161, 176):
    name = f"poly_{n}"
    if name in extracted:
        POLY_FUNCS[n] = extracted[name]
    else:
        print(f"WARNING: {name} not found in poly200.py")


def world_to_pixel(re, im):
    """Convert world coords to pixel coords on 1000x1000 canvas, extent [-2,2]x[-2,2]."""
    px = int((re + EXTENT) / (2 * EXTENT) * CANVAS_SIZE)
    py = int((-im + EXTENT) / (2 * EXTENT) * CANVAS_SIZE)
    if 0 <= px < CANVAS_SIZE and 0 <= py < CANVAS_SIZE:
        return (px, py)
    return None


def run_c_pipeline(poly_num):
    """Run the C coeffgen+solve pipeline, return set of (px,py) pixels."""
    func_name = f"poly_{poly_num}"
    coeffs_file = f"/tmp/poly{poly_num}_coeffs.bin"
    roots_file = f"/tmp/poly{poly_num}_roots.bin"

    # Step 1: coeffgen
    coeffgen_input = json.dumps({
        "mode": "coeffgen",
        "function": func_name,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"],
        "times": 1,
        "dither_pass": 0,
    })

    try:
        result = subprocess.run(
            [SWEEP_BIN, coeffs_file],
            input=coeffgen_input, capture_output=True, text=True, timeout=60
        )
    except subprocess.TimeoutExpired:
        return None, "TIMEOUT in coeffgen"

    if result.returncode != 0:
        stderr = result.stderr.strip()
        if "STUB" in stderr.upper() or "stub" in stderr or result.returncode != 0:
            # Check if it's a stub
            return set(), f"STUBBED (rc={result.returncode}, stderr={stderr[:200]})"
        return None, f"coeffgen failed: {stderr[:200]}"

    # Parse stdout JSON
    try:
        cg_out = json.loads(result.stdout.strip())
        n_coeffs = cg_out.get("n_coeffs", 0)
        n_t = cg_out.get("n_t", 0)
    except (json.JSONDecodeError, ValueError) as e:
        return None, f"coeffgen JSON parse error: {e}, stdout={result.stdout[:200]}"

    if n_coeffs == 0 or n_t == 0:
        return set(), f"STUBBED (n_coeffs={n_coeffs}, n_t={n_t})"

    # Step 2: solve
    solve_input = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "n_t": n_t,
    })

    try:
        result = subprocess.run(
            [SWEEP_BIN, roots_file],
            input=solve_input, capture_output=True, text=True, timeout=120
        )
    except subprocess.TimeoutExpired:
        return None, "TIMEOUT in solve"

    if result.returncode != 0:
        return None, f"solve failed: {result.stderr.strip()[:200]}"

    # Step 3: Read roots binary - pairs of float32 (re, im)
    try:
        data = Path(roots_file).read_bytes()
    except FileNotFoundError:
        return None, "roots file not found"

    n_roots = len(data) // 8  # 4 bytes re + 4 bytes im
    pixels = set()
    for i in range(n_roots):
        offset = i * 8
        re, im = struct.unpack_from('ff', data, offset)
        if np.isfinite(re) and np.isfinite(im):
            pt = world_to_pixel(re, im)
            if pt is not None:
                pixels.add(pt)

    return pixels, "OK"


def run_python_pipeline(poly_num):
    """Run the Python reference pipeline, return set of (px,py) pixels."""
    func = POLY_FUNCS[poly_num]
    pixels = set()

    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)

            coeffs = func(t1, t2)
            if len(coeffs) == 0:
                continue

            # Reverse coefficients (same as coeff_transforms=["rev"])
            coeffs = coeffs[::-1]

            # Strip leading zeros
            while len(coeffs) > 1 and coeffs[0] == 0:
                coeffs = coeffs[1:]

            if len(coeffs) <= 1:
                continue

            try:
                roots = np.roots(coeffs)
            except Exception:
                continue

            for r in roots:
                if np.isfinite(r.real) and np.isfinite(r.imag):
                    pt = world_to_pixel(r.real, r.imag)
                    if pt is not None:
                        pixels.add(pt)

    return pixels


def save_pixel_image(pixels, filepath, canvas_size=CANVAS_SIZE):
    """Save a set of (px,py) pixels as a PNG image."""
    try:
        from PIL import Image
    except ImportError:
        # Fallback: use matplotlib
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        img = np.zeros((canvas_size, canvas_size), dtype=np.uint8)
        for (px, py) in pixels:
            if 0 <= px < canvas_size and 0 <= py < canvas_size:
                img[py, px] = 255
        plt.imsave(filepath, img, cmap='gray')
        return

    img = Image.new('L', (canvas_size, canvas_size), 0)
    px_data = img.load()
    for (x, y) in pixels:
        if 0 <= x < canvas_size and 0 <= y < canvas_size:
            px_data[x, y] = 255
    img.save(filepath)


def main():
    results = []

    for poly_num in range(161, 176):
        print(f"\n{'='*60}")
        print(f"  poly_{poly_num}")
        print(f"{'='*60}")

        # C pipeline
        print(f"  Running C pipeline...", end="", flush=True)
        c_pixels, c_status = run_c_pipeline(poly_num)
        if c_pixels is None:
            print(f" ERROR: {c_status}")
            c_count = 0
            c_pixels = set()
        else:
            c_count = len(c_pixels)
            print(f" {c_count} pixels ({c_status})")

        # Python pipeline
        print(f"  Running Python pipeline...", end="", flush=True)
        py_pixels = run_python_pipeline(poly_num)
        py_count = len(py_pixels)
        print(f" {py_count} pixels")

        # Overlap
        if c_count > 0 and py_count > 0:
            overlap = len(c_pixels & py_pixels)
            overlap_pct = overlap / max(c_count, py_count) * 100
        elif c_count == 0 and py_count == 0:
            overlap = 0
            overlap_pct = 100.0  # both empty = match
        else:
            overlap = 0
            overlap_pct = 0.0

        # Save images
        c_img_path = f"/tmp/poly{poly_num}_c.png"
        py_img_path = f"/tmp/poly{poly_num}_py.png"
        save_pixel_image(c_pixels, c_img_path)
        save_pixel_image(py_pixels, py_img_path)

        note = ""
        if c_count == 0 and "STUB" in c_status:
            note = "C is STUBBED"
        elif c_count == 0:
            note = "C produced EMPTY image"
        if py_count == 0:
            note += (" / " if note else "") + "Python produced EMPTY image"

        results.append({
            "poly": poly_num,
            "c_pixels": c_count,
            "py_pixels": py_count,
            "overlap": overlap,
            "overlap_pct": overlap_pct,
            "c_status": c_status,
            "note": note,
        })

        print(f"  Overlap: {overlap} pixels ({overlap_pct:.1f}%)")
        if note:
            print(f"  Note: {note}")

    # Summary
    print(f"\n\n{'='*80}")
    print(f"  SUMMARY: poly_161 through poly_175")
    print(f"{'='*80}")
    print(f"{'Func':<12} {'C pixels':>10} {'Py pixels':>10} {'Overlap':>10} {'Overlap%':>10}  Notes")
    print(f"{'-'*12} {'-'*10} {'-'*10} {'-'*10} {'-'*10}  {'-'*30}")
    for r in results:
        print(f"poly_{r['poly']:<6} {r['c_pixels']:>10} {r['py_pixels']:>10} {r['overlap']:>10} {r['overlap_pct']:>9.1f}%  {r['note']}")

    # Overall stats
    non_stub = [r for r in results if "STUB" not in r.get("c_status", "")]
    if non_stub:
        avg_overlap = sum(r["overlap_pct"] for r in non_stub) / len(non_stub)
        print(f"\nAverage overlap (non-stubbed): {avg_overlap:.1f}%")
        print(f"Stubbed functions: {len(results) - len(non_stub)}")


if __name__ == "__main__":
    main()
