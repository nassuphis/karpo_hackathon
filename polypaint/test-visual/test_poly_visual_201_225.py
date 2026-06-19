#!/usr/bin/env python3
"""Visual comparison: C sweep vs Python reference for poly_201 through poly_225."""

import ast
import json
import struct
import subprocess
import sys
import textwrap
import numpy as np
from pathlib import Path

WORKDIR = Path(__file__).parent
LAMBDA_DIR = WORKDIR.parent / "lambda"
SWEEP_BIN = LAMBDA_DIR / "sweep_test"
POLY_SRC = LAMBDA_DIR / "poly300.py"

N1, N2 = 100, 100
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]


def load_poly_funcs():
    """Load poly functions from poly300.py using ast, stripping relative imports."""
    src = POLY_SRC.read_text()
    # Remove relative imports that won't resolve
    lines = src.split("\n")
    clean_lines = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("from . import") or stripped.startswith("from ."):
            continue
        if "sph_harm" in stripped:
            continue
        clean_lines.append(line)
    clean_src = "\n".join(clean_lines)

    namespace = {}
    exec(compile(clean_src, str(POLY_SRC), "exec"), namespace)
    return namespace


def run_c_pipeline(func_name):
    """Run C coeffgen + solve, return roots as complex array."""
    coeffs_file = f"/tmp/{func_name}_coeffs.bin"
    roots_file = f"/tmp/{func_name}_roots.bin"

    # Step 1: coeffgen
    coeffgen_req = json.dumps({
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
        [str(SWEEP_BIN), coeffs_file],
        input=coeffgen_req, capture_output=True, text=True,
        cwd=str(WORKDIR), timeout=120,
    )
    if result.returncode != 0:
        print(f"  coeffgen FAILED: {result.stderr[:200]}")
        return None
    info = json.loads(result.stdout.strip())
    n_coeffs = info["n_coeffs"]
    n_t = info["n_t"]

    # Step 2: solve
    solve_req = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "n_t": n_t,
    })
    result = subprocess.run(
        [str(SWEEP_BIN), roots_file],
        input=solve_req, capture_output=True, text=True,
        cwd=str(WORKDIR), timeout=120,
    )
    if result.returncode != 0:
        print(f"  solve FAILED: {result.stderr[:200]}")
        return None

    # Step 3: read roots (float32 pairs)
    data = Path(roots_file).read_bytes()
    n_floats = len(data) // 4
    floats = struct.unpack(f"{n_floats}f", data)
    roots = []
    for i in range(0, n_floats, 2):
        roots.append(complex(floats[i], floats[i + 1]))
    return np.array(roots)


def run_python_pipeline(func, func_name):
    """Run Python reference: for each (i1,i2), call func, reverse coeffs, np.roots."""
    all_roots = []
    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            try:
                coeffs = func(t1, t2)
                if coeffs is None or len(coeffs) == 0:
                    continue
                coeffs = np.array(coeffs, dtype=complex)
                # Reverse coefficients (same as coeff_transforms: ["rev"])
                coeffs = coeffs[::-1]
                # Strip leading zeros
                while len(coeffs) > 1 and coeffs[0] == 0:
                    coeffs = coeffs[1:]
                if len(coeffs) <= 1:
                    continue
                roots = np.roots(coeffs)
                all_roots.extend(roots)
            except Exception as e:
                pass  # Skip errors like the original try/except
    return np.array(all_roots) if all_roots else np.array([])


def roots_to_pixels(roots, img_size=IMG_SIZE, extent=EXTENT):
    """Convert complex roots to pixel set on img_size x img_size, extent [-ext, ext]."""
    if len(roots) == 0:
        return set()
    re = np.real(roots)
    im = np.imag(roots)
    # Map to pixel coords
    px = ((re + extent) / (2 * extent) * img_size).astype(int)
    py = ((extent - im) / (2 * extent) * img_size).astype(int)  # flip y
    # Filter in-bounds
    mask = (px >= 0) & (px < img_size) & (py >= 0) & (py < img_size)
    pixels = set(zip(px[mask], py[mask]))
    return pixels


def main():
    print("Loading Python functions from poly300.py...")
    ns = load_poly_funcs()

    results = []
    for n in range(201, 226):
        func_name = f"poly_{n}"
        print(f"\n{'='*60}")
        print(f"Testing {func_name}...")

        func = ns.get(func_name)
        if func is None:
            print(f"  SKIP: {func_name} not found in Python source")
            results.append((func_name, 0, 0, 0.0, "MISSING"))
            continue

        # C pipeline
        print(f"  Running C pipeline...")
        c_roots = run_c_pipeline(func_name)
        if c_roots is None:
            results.append((func_name, 0, 0, 0.0, "C_FAIL"))
            continue
        c_pixels = roots_to_pixels(c_roots)
        print(f"  C roots: {len(c_roots)}, C pixels: {len(c_pixels)}")

        # Python pipeline
        print(f"  Running Python pipeline...")
        py_roots = run_python_pipeline(func, func_name)
        py_pixels = roots_to_pixels(py_roots)
        print(f"  Py roots: {len(py_roots)}, Py pixels: {len(py_pixels)}")

        # Overlap
        if len(c_pixels) == 0 and len(py_pixels) == 0:
            overlap_pct = 100.0
            status = "BOTH_EMPTY"
        elif len(c_pixels) == 0 or len(py_pixels) == 0:
            overlap_pct = 0.0
            status = "ONE_EMPTY"
        else:
            overlap = c_pixels & py_pixels
            union = c_pixels | py_pixels
            overlap_pct = 100.0 * len(overlap) / len(union)
            status = "OK" if overlap_pct >= 90 else "LOW" if overlap_pct >= 50 else "FAIL"

        print(f"  Overlap: {overlap_pct:.1f}% [{status}]")
        results.append((func_name, len(c_pixels), len(py_pixels), overlap_pct, status))

    # Summary table
    print(f"\n\n{'='*70}")
    print(f"{'Function':<12} {'C pixels':>10} {'Py pixels':>10} {'Overlap%':>10} {'Status':>8}")
    print(f"{'-'*70}")
    for name, cp, pp, ov, st in results:
        print(f"{name:<12} {cp:>10} {pp:>10} {ov:>10.1f} {st:>8}")
    print(f"{'='*70}")

    # Overall stats
    ok_count = sum(1 for _, _, _, _, s in results if s in ("OK", "BOTH_EMPTY"))
    total = len(results)
    print(f"\nPassed (>=90% overlap): {ok_count}/{total}")


if __name__ == "__main__":
    main()
