"""
Visual comparison tests for poly_451 through poly_475.
C sweep pipeline vs Python reference (poly500.py).

Run: cd polypaint && uv run python test_visual_451_475.py
"""
import ast
import json
import math
import struct
import subprocess
import sys
import os
import numpy as np

# --------------- Load Python poly functions from poly500.py ---------------

POLY_SRC = os.path.join(os.path.dirname(__file__), "lambda", "poly500.py")
SWEEP_BIN = os.path.join(os.path.dirname(__file__), "lambda", "sweep_test")

# Parse source with ast, extract individual function defs
with open(POLY_SRC, "r") as f:
    source = f.read()

tree = ast.parse(source, POLY_SRC)

# Build namespace with needed modules
import math, cmath
_ns = {
    "np": np, "numpy": np, "math": math, "cmath": cmath,
    "__builtins__": __builtins__,
}

# Try to import scipy.special if available
try:
    from scipy.special import sph_harm
    _ns["sph_harm"] = sph_harm
except ImportError:
    pass

poly_funcs = {}
for node in ast.walk(tree):
    if isinstance(node, ast.FunctionDef):
        name = node.name
        # Match poly_451 through poly_475 (skip _old variants)
        if name.startswith("poly_") and not name.endswith("_old"):
            try:
                num = int(name.split("_")[1])
            except (ValueError, IndexError):
                continue
            if 451 <= num <= 475:
                # Compile just this function
                mod = ast.Module(body=[node], type_ignores=[])
                ast.fix_missing_locations(mod)
                code = compile(mod, POLY_SRC, "exec")
                exec(code, _ns)
                if name in _ns and callable(_ns[name]):
                    poly_funcs[num] = _ns[name]

# --------------- Parameters ---------------

N1, N2 = 100, 100
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
STUBBED = {456, 468}  # noted as stubbed by user (though they have code)

# --------------- C pipeline ---------------

def run_c_pipeline(poly_num):
    """Run coeffgen + solve via C binary, return root positions as (re, im) arrays."""
    func_name = f"poly_{poly_num}"
    coeffs_path = f"/tmp/poly{poly_num}_coeffs.bin"
    roots_path = f"/tmp/poly{poly_num}_roots.bin"

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
        capture_output=True, text=True, timeout=30
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
    result = subprocess.run(
        [SWEEP_BIN, roots_path],
        input=json.dumps(solve_spec),
        capture_output=True, text=True, timeout=60
    )
    if result.returncode != 0:
        raise RuntimeError(f"solve failed for {func_name}: {result.stderr.strip()}")

    solve_meta = json.loads(result.stdout)
    degree = solve_meta["degree"]
    total_steps = solve_meta["n_t"]

    # Step 3: Read roots
    with open(roots_path, "rb") as f:
        data = f.read()

    expected_size = total_steps * degree * 2 * 4
    if len(data) != expected_size:
        raise RuntimeError(f"roots file size mismatch: {len(data)} vs {expected_size}")

    floats = struct.unpack(f"<{total_steps * degree * 2}f", data)
    re_vals = []
    im_vals = []
    idx = 0
    for step in range(total_steps):
        for r in range(degree):
            re_vals.append(floats[idx])
            im_vals.append(floats[idx + 1])
            idx += 2

    return np.array(re_vals, dtype=np.float64), np.array(im_vals, dtype=np.float64), n_coeffs, degree

# --------------- Python pipeline ---------------

def run_python_pipeline(poly_num):
    """Run Python reference: 100x100 grid, unit circle params, call func, reverse coeffs, np.roots."""
    func = poly_funcs[poly_num]
    re_vals = []
    im_vals = []
    degree = None

    for i1 in range(N1):
        x1 = i1 / N1
        for i2 in range(N2):
            x2 = i2 / N2
            # unit_circle transform
            a1 = 2.0 * math.pi * x1
            a2 = 2.0 * math.pi * x2
            t1 = complex(math.cos(a1), math.sin(a1))
            t2 = complex(math.cos(a2), math.sin(a2))

            coeffs = func(t1, t2)
            # reverse (coeff_transform "rev")
            coeffs = coeffs[::-1]

            if degree is None:
                degree = len(coeffs) - 1

            # Solve roots
            if len(coeffs) < 2 or np.all(np.abs(coeffs) < 1e-30):
                roots = np.zeros(degree, dtype=complex)
            else:
                # Strip leading zeros
                first_nonzero = 0
                while first_nonzero < len(coeffs) - 1 and abs(coeffs[first_nonzero]) < 1e-15:
                    first_nonzero += 1
                trimmed = coeffs[first_nonzero:]
                if len(trimmed) < 2:
                    roots = np.zeros(degree, dtype=complex)
                else:
                    try:
                        roots = np.roots(trimmed)
                    except Exception:
                        roots = np.zeros(degree, dtype=complex)
                    # Pad to full degree with zeros (for trailing zeros that were factored out)
                    if len(roots) < degree:
                        roots = np.concatenate([roots, np.zeros(degree - len(roots), dtype=complex)])

            for r in roots[:degree]:
                re_vals.append(r.real)
                im_vals.append(r.imag)

    return np.array(re_vals, dtype=np.float64), np.array(im_vals, dtype=np.float64), degree

# --------------- Rasterize to pixel set ---------------

def rasterize(re_arr, im_arr, img_size=IMG_SIZE, extent=EXTENT):
    """Convert root positions to set of (px, py) pixel coordinates."""
    pixels = set()
    scale = img_size / (2 * extent)
    cx = img_size / 2
    cy = img_size / 2

    for i in range(len(re_arr)):
        re = re_arr[i]
        im = im_arr[i]
        # Skip NaN/Inf
        if not (math.isfinite(re) and math.isfinite(im)):
            continue
        px = int(re * scale + cx)
        py = int(-im * scale + cy)  # flip y
        if 0 <= px < img_size and 0 <= py < img_size:
            pixels.add((px, py))

    return pixels

# --------------- Main ---------------

def main():
    results = []
    print(f"{'Poly':>10} | {'C pixels':>10} | {'Py pixels':>10} | {'Overlap':>10} | {'Overlap%':>10} | {'Status'}")
    print("-" * 78)

    for n in range(451, 476):
        if n not in poly_funcs:
            print(f"{'poly_'+str(n):>10} | {'N/A':>10} | {'N/A':>10} | {'N/A':>10} | {'N/A':>10} | SKIP (no func)")
            results.append((n, 0, 0, 0, 0.0, "SKIP"))
            continue

        try:
            c_re, c_im, n_coeffs, c_degree = run_c_pipeline(n)
        except Exception as e:
            print(f"{'poly_'+str(n):>10} | {'ERR':>10} | {'---':>10} | {'---':>10} | {'---':>10} | C ERROR: {e}")
            results.append((n, 0, 0, 0, 0.0, f"C_ERR: {e}"))
            continue

        try:
            py_re, py_im, py_degree = run_python_pipeline(n)
        except Exception as e:
            print(f"{'poly_'+str(n):>10} | {'---':>10} | {'ERR':>10} | {'---':>10} | {'---':>10} | Py ERROR: {e}")
            results.append((n, 0, 0, 0, 0.0, f"PY_ERR: {e}"))
            continue

        c_pixels = rasterize(c_re, c_im)
        py_pixels = rasterize(py_re, py_im)

        overlap = c_pixels & py_pixels
        # Overlap % relative to the union
        union = c_pixels | py_pixels
        union_sz = len(union) if len(union) > 0 else 1
        overlap_pct = 100.0 * len(overlap) / union_sz

        stubbed_note = " (stubbed)" if n in STUBBED else ""
        status = "OK" if overlap_pct > 90 else ("WARN" if overlap_pct > 50 else "FAIL")
        status += stubbed_note

        print(f"{'poly_'+str(n):>10} | {len(c_pixels):>10} | {len(py_pixels):>10} | {len(overlap):>10} | {overlap_pct:>9.1f}% | {status}")
        results.append((n, len(c_pixels), len(py_pixels), len(overlap), overlap_pct, status))

    # Summary
    print("\n" + "=" * 78)
    print("SUMMARY")
    print("=" * 78)
    ok = sum(1 for r in results if r[5].startswith("OK"))
    warn = sum(1 for r in results if r[5].startswith("WARN"))
    fail = sum(1 for r in results if r[5].startswith("FAIL"))
    skip = sum(1 for r in results if r[5].startswith("SKIP"))
    err = sum(1 for r in results if "ERR" in r[5])
    total = len(results)
    print(f"  OK (>90%):  {ok}/{total}")
    print(f"  WARN (50-90%): {warn}/{total}")
    print(f"  FAIL (<50%): {fail}/{total}")
    print(f"  SKIP/ERR:   {skip + err}/{total}")

    if fail + err > 0:
        print("\nFailing functions:")
        for r in results:
            if r[5].startswith("FAIL") or "ERR" in r[5]:
                print(f"  poly_{r[0]}: {r[5]} (overlap={r[4]:.1f}%)")

    avg_overlap = np.mean([r[4] for r in results if not r[5].startswith("SKIP") and "ERR" not in r[5]])
    print(f"\nAverage overlap (non-skip): {avg_overlap:.1f}%")

    return 0 if fail + err == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
