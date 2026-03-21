#!/usr/bin/env python3
"""Visual comparison: C sweep vs Python reference for poly_131 through poly_145."""

import ast
import json
import struct
import subprocess
import sys
import os
import numpy as np
from PIL import Image

SWEEP_BIN = os.path.join(os.path.dirname(__file__), "lambda", "sweep_test")
POLY_SRC = os.path.join(os.path.dirname(__file__), "lambda", "poly200.py")

N1, N2 = 100, 100
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]

FUNCTIONS = [f"poly_{n}" for n in range(131, 146)]


def load_python_functions():
    """Parse poly200.py with ast and extract poly_131..poly_145 as callables."""
    with open(POLY_SRC, "r") as f:
        source = f.read()

    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in FUNCTIONS:
            # Extract source lines for this function
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.split("\n")[start:end]
            func_source = "\n".join(lines)
            # Compile in a namespace with numpy
            ns = {"np": np, "math": __import__("math"), "cmath": __import__("cmath")}
            exec(compile(ast.parse(func_source), f"<{node.name}>", "exec"), ns)
            funcs[node.name] = ns[node.name]
    return funcs


def run_c_pipeline(func_name):
    """Run coeffgen + solve via sweep_test, return root positions as (re, im) arrays."""
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
        [SWEEP_BIN, coeffs_path],
        input=json.dumps(spec),
        capture_output=True, text=True, timeout=120,
    )
    if result.returncode != 0:
        print(f"  coeffgen FAILED: {result.stderr.strip()}")
        return None, 0, 0
    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]
    print(f"  coeffgen: n_coeffs={n_coeffs}, n_t={n_t}")

    # Step 2: solve
    solve_spec = {
        "mode": "solve",
        "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "n_t": n_t,
    }
    result = subprocess.run(
        [SWEEP_BIN, roots_path],
        input=json.dumps(solve_spec),
        capture_output=True, text=True, timeout=300,
    )
    if result.returncode != 0:
        print(f"  solve FAILED: {result.stderr.strip()}")
        return None, n_coeffs, n_t

    # Step 3: read roots binary (float32 pairs)
    with open(roots_path, "rb") as f:
        data = f.read()
    n_roots = len(data) // 8
    roots = np.frombuffer(data, dtype=np.float32).reshape(-1, 2)

    # Clean up
    for p in [coeffs_path, roots_path]:
        try:
            os.remove(p)
        except OSError:
            pass

    return roots, n_coeffs, n_t


def roots_to_image(roots, img_size=IMG_SIZE, extent=EXTENT):
    """Plot roots on a binary image. Returns (image, pixel_count)."""
    img = np.zeros((img_size, img_size), dtype=np.uint8)
    if roots is None or len(roots) == 0:
        return img, 0

    re = roots[:, 0]
    im = roots[:, 1]

    # Map to pixel coords: [-extent, extent] -> [0, img_size)
    px = ((re + extent) / (2 * extent) * img_size).astype(np.int32)
    py = ((extent - im) / (2 * extent) * img_size).astype(np.int32)

    # Filter in-bounds
    mask = (px >= 0) & (px < img_size) & (py >= 0) & (py < img_size)
    px, py = px[mask], py[mask]

    img[py, px] = 255
    pixel_count = np.count_nonzero(img)
    return img, pixel_count


def run_python_reference(func, n_coeffs_expected):
    """Run Python reference: for each (i1,i2), compute coeffs, reverse, find roots."""
    all_roots = []
    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            try:
                cf = func(t1, t2)
            except Exception:
                continue
            if cf is None or len(cf) == 0:
                continue
            # Reverse coefficients (same as coeff_transforms: ["rev"])
            cf_rev = cf[::-1]
            # Strip leading zeros
            while len(cf_rev) > 1 and cf_rev[0] == 0:
                cf_rev = cf_rev[1:]
            if len(cf_rev) < 2:
                continue
            try:
                r = np.roots(cf_rev)
            except Exception:
                continue
            for root in r:
                all_roots.append([root.real, root.imag])

    if not all_roots:
        return None
    return np.array(all_roots, dtype=np.float32)


def main():
    py_funcs = load_python_functions()
    print(f"Loaded {len(py_funcs)} Python functions: {sorted(py_funcs.keys())}")

    results = []

    for func_name in FUNCTIONS:
        print(f"\n{'='*60}")
        print(f"Processing {func_name}")
        print(f"{'='*60}")

        # C pipeline
        print("  Running C pipeline...")
        c_roots, n_coeffs, n_t = run_c_pipeline(func_name)
        c_img, c_pixels = roots_to_image(c_roots)

        # Python pipeline
        print("  Running Python pipeline...")
        if func_name not in py_funcs:
            print(f"  WARNING: {func_name} not found in poly200.py!")
            py_roots = None
        else:
            py_roots = run_python_reference(py_funcs[func_name], n_coeffs)
        py_img, py_pixels = roots_to_image(py_roots)

        # Overlap
        if c_pixels > 0 and py_pixels > 0:
            overlap = np.count_nonzero(c_img & py_img)
            overlap_pct = overlap / max(c_pixels, py_pixels) * 100
        elif c_pixels == 0 and py_pixels == 0:
            overlap_pct = 100.0
        else:
            overlap_pct = 0.0

        # Save PNGs
        c_png = f"/tmp/{func_name}_c.png"
        py_png = f"/tmp/{func_name}_py.png"
        Image.fromarray(c_img).save(c_png)
        Image.fromarray(py_img).save(py_png)

        empty_note = ""
        if c_pixels < 10:
            empty_note += " [C EMPTY/NEAR-EMPTY]"
        if py_pixels < 10:
            empty_note += " [Python EMPTY/NEAR-EMPTY]"

        results.append((func_name, c_pixels, py_pixels, overlap_pct, empty_note))
        print(f"  C pixels: {c_pixels}, Python pixels: {py_pixels}, Overlap: {overlap_pct:.1f}%{empty_note}")

    # Summary
    print(f"\n{'='*60}")
    print("SUMMARY")
    print(f"{'='*60}")
    print(f"{'Function':<12} {'C pixels':>10} {'Py pixels':>10} {'Overlap%':>10} Notes")
    print("-" * 60)
    for func_name, c_px, py_px, ovl, note in results:
        print(f"{func_name:<12} {c_px:>10} {py_px:>10} {ovl:>9.1f}% {note}")


if __name__ == "__main__":
    main()
