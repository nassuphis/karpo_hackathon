#!/usr/bin/env python3
"""Visual comparison tests for poly_146 through poly_160: C sweep vs Python reference."""

import subprocess
import json
import struct
import sys
import os
import ast
import textwrap
import numpy as np
from PIL import Image

# ── Config ──────────────────────────────────────────────────────────────────
FUNCTIONS = [f"poly_{n}" for n in range(146, 161)]
N1, N2 = 100, 100
CANVAS = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
SWEEP_BIN = os.path.join(os.path.dirname(__file__), "..", "lambda", "sweep_test")
POLY200_PATH = os.path.join(os.path.dirname(__file__), "..", "lambda", "poly200.py")


# ── Extract Python functions from poly200.py using ast ──────────────────────
def load_poly_functions(path, names):
    """Parse poly200.py and extract requested function objects."""
    with open(path, "r") as f:
        source = f.read()

    tree = ast.parse(source)
    func_sources = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in names:
            # Extract source lines
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.splitlines()[start:end]
            func_sources[node.name] = "\n".join(lines)

    # Compile and exec each function in a namespace with numpy etc.
    ns = {"np": np, "math": __import__("math"), "cmath": __import__("cmath"), "pi": np.pi}
    # Also need scipy for sph_harm if any function uses it
    try:
        from scipy.special import sph_harm
        ns["sph_harm"] = sph_harm
    except ImportError:
        pass

    funcs = {}
    for name in names:
        if name not in func_sources:
            print(f"WARNING: {name} not found in {path}")
            continue
        exec(func_sources[name], ns)
        funcs[name] = ns[name]
    return funcs


# ── Pixel plotting helpers ──────────────────────────────────────────────────
def roots_to_pixels(roots_re, roots_im, canvas=CANVAS, extent=EXTENT):
    """Convert root coordinates to pixel set."""
    pixels = set()
    for re_val, im_val in zip(roots_re, roots_im):
        if not (np.isfinite(re_val) and np.isfinite(im_val)):
            continue
        px = int((re_val + extent) / (2 * extent) * canvas)
        py = int((-im_val + extent) / (2 * extent) * canvas)
        if 0 <= px < canvas and 0 <= py < canvas:
            pixels.add((px, py))
    return pixels


def pixels_to_image(pixels, canvas=CANVAS):
    """Create a grayscale image from pixel set."""
    img = np.zeros((canvas, canvas), dtype=np.uint8)
    for px, py in pixels:
        img[py, px] = 255
    return Image.fromarray(img, mode="L")


# ── C pipeline ──────────────────────────────────────────────────────────────
def run_c_pipeline(func_name):
    """Run coeffgen + solve via sweep_test, return set of (px, py) pixels."""
    idx = func_name.replace("poly_", "")
    coeffs_path = f"/tmp/{func_name}_coeffs.bin"
    roots_path = f"/tmp/{func_name}_roots.bin"

    # Step 1: coeffgen
    spec = {
        "mode": "coeffgen",
        "function": func_name,
        "n1": N1,
        "n2": N2,
        "i1_start": 0,
        "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"],
        "times": 1,
        "dither_pass": 0,
    }
    result = subprocess.run(
        [SWEEP_BIN, coeffs_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=120,
    )
    if result.returncode != 0:
        print(f"  C coeffgen FAILED for {func_name}: {result.stderr.strip()}")
        return set()

    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]

    # Step 2: solve
    solve_spec = {
        "mode": "solve",
        "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs,
        "n1": N1,
        "n2": N2,
        "n_t": n_t,
    }
    result = subprocess.run(
        [SWEEP_BIN, roots_path],
        input=json.dumps(solve_spec),
        capture_output=True,
        text=True,
        timeout=120,
    )
    if result.returncode != 0:
        print(f"  C solve FAILED for {func_name}: {result.stderr.strip()}")
        return set()

    # Step 3: read roots binary — pairs of float32 (re, im)
    with open(roots_path, "rb") as f:
        data = f.read()

    n_roots = len(data) // 8  # 4 bytes re + 4 bytes im
    roots_re = []
    roots_im = []
    for i in range(n_roots):
        re_val, im_val = struct.unpack_from("<ff", data, i * 8)
        roots_re.append(re_val)
        roots_im.append(im_val)

    pixels = roots_to_pixels(roots_re, roots_im)

    # Cleanup temp files
    for p in [coeffs_path, roots_path]:
        try:
            os.remove(p)
        except OSError:
            pass

    return pixels


# ── Python pipeline ─────────────────────────────────────────────────────────
def run_python_pipeline(func):
    """Run Python reference: for each (i1,i2), compute coeffs, reverse, find roots."""
    all_re = []
    all_im = []
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
            first_nonzero = 0
            for k in range(len(cf_rev)):
                if cf_rev[k] != 0:
                    first_nonzero = k
                    break
            cf_rev = cf_rev[first_nonzero:]
            if len(cf_rev) < 2:
                continue
            try:
                roots = np.roots(cf_rev)
            except Exception:
                continue
            for r in roots:
                if np.isfinite(r.real) and np.isfinite(r.imag):
                    all_re.append(r.real)
                    all_im.append(r.imag)
    return roots_to_pixels(all_re, all_im)


# ── Main ────────────────────────────────────────────────────────────────────
def main():
    print("Loading Python functions from poly200.py...")
    funcs = load_poly_functions(POLY200_PATH, set(FUNCTIONS))
    print(f"Loaded {len(funcs)} functions.\n")

    results = []
    for func_name in FUNCTIONS:
        print(f"=== {func_name} ===")

        # C pipeline
        print(f"  Running C pipeline...")
        c_pixels = run_c_pipeline(func_name)
        c_count = len(c_pixels)
        print(f"  C pixels: {c_count}")

        # Python pipeline
        print(f"  Running Python pipeline...")
        if func_name not in funcs:
            print(f"  SKIPPED (function not found)")
            results.append((func_name, c_count, 0, 0.0, "Python function not found"))
            continue
        py_pixels = run_python_pipeline(funcs[func_name])
        py_count = len(py_pixels)
        print(f"  Python pixels: {py_count}")

        # Overlap
        overlap = len(c_pixels & py_pixels)
        union = len(c_pixels | py_pixels)
        overlap_pct = (overlap / union * 100) if union > 0 else 0.0
        print(f"  Overlap: {overlap} pixels ({overlap_pct:.1f}%)")

        # Warnings
        notes = ""
        if c_count < 100:
            notes += "C near-empty; "
        if py_count < 100:
            notes += "Python near-empty; "

        results.append((func_name, c_count, py_count, overlap_pct, notes))

        # Save images
        c_img = pixels_to_image(c_pixels)
        c_img.save(f"/tmp/{func_name}_c.png")
        py_img = pixels_to_image(py_pixels)
        py_img.save(f"/tmp/{func_name}_py.png")
        print(f"  Saved /tmp/{func_name}_c.png and /tmp/{func_name}_py.png\n")

    # Summary table
    print("\n" + "=" * 80)
    print(f"{'Function':<12} {'C pixels':>10} {'Py pixels':>10} {'Overlap%':>10}  Notes")
    print("-" * 80)
    for name, c_px, py_px, ovlp, notes in results:
        print(f"{name:<12} {c_px:>10} {py_px:>10} {ovlp:>9.1f}%  {notes}")
    print("=" * 80)


if __name__ == "__main__":
    main()
