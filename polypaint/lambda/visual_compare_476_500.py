"""
Visual comparison: C sweep pipeline vs Python reference for poly_476..poly_500.

For each function:
  C: coeffgen -> solve -> read roots -> rasterize 1000x1000
  Py: load func from poly500.py via ast, 100x100 grid, unit_circle params, reverse coeffs, np.roots, rasterize
  Compare: pixel overlap %

Run: cd polypaint/lambda && uv run python visual_compare_476_500.py
"""
import ast
import json
import math
import struct
import subprocess
import sys
import os
import textwrap
import numpy as np

SWEEP = "./sweep_test"
IMG_SIZE = 1000
EXTENT = 2.0  # [-2, 2] x [-2, 2]
N1, N2 = 100, 100

# ---- Load Python functions from poly500.py ----

def load_poly_funcs(path, names):
    """Parse poly500.py and extract specific function definitions."""
    with open(path, "r") as f:
        source = f.read()

    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in names:
            # Extract source lines for this function
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.splitlines()[start:end]
            func_source = "\n".join(lines)

            # Build a module with necessary imports + the function
            module_code = textwrap.dedent("""\
                import math, cmath
                import numpy as np
                try:
                    from scipy.special import sph_harm
                except ImportError:
                    sph_harm = None
                pi = math.pi
            """) + "\n" + func_source

            ns = {}
            exec(compile(module_code, f"<{node.name}>", "exec"), ns)
            funcs[node.name] = ns[node.name]
    return funcs


# ---- C pipeline ----

def run_c_pipeline(func_name):
    """Run coeffgen + solve for a function, return root positions as (N, degree, 2) array."""
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
        input=spec, capture_output=True, text=True, timeout=30
    )
    if result.returncode != 0:
        raise RuntimeError(f"coeffgen failed for {func_name}: {result.stderr}")
    meta1 = json.loads(result.stdout)
    n_coeffs = meta1["n_coeffs"]
    n_t = meta1["n_t"]

    # Step 2: solve
    spec2 = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "match_roots": False,
    })
    result2 = subprocess.run(
        [SWEEP, roots_file],
        input=spec2, capture_output=True, text=True, timeout=60
    )
    if result2.returncode != 0:
        raise RuntimeError(f"solve failed for {func_name}: {result2.stderr}")
    meta2 = json.loads(result2.stdout)
    degree = meta2["degree"]
    n_t_solve = meta2["n_t"]

    # Step 3: read roots
    with open(roots_file, "rb") as f:
        data = f.read()
    expected = n_t_solve * degree * 2 * 4
    if len(data) < expected:
        raise RuntimeError(f"roots file too small: {len(data)} < {expected}")

    floats = np.frombuffer(data[:expected], dtype=np.float32)
    roots = floats.reshape(n_t_solve, degree, 2)
    return roots, degree


def rasterize(roots_array, img_size=IMG_SIZE, extent=EXTENT):
    """Rasterize roots to a boolean image. roots_array shape: (N, degree, 2)."""
    img = np.zeros((img_size, img_size), dtype=bool)
    re_vals = roots_array[:, :, 0].ravel()
    im_vals = roots_array[:, :, 1].ravel()

    # Map from [-extent, extent] to [0, img_size)
    px = ((re_vals + extent) / (2 * extent) * img_size).astype(np.int32)
    py = ((im_vals + extent) / (2 * extent) * img_size).astype(np.int32)

    # Flip y so positive imaginary is up (but for comparison it doesn't matter as long as consistent)
    mask = (px >= 0) & (px < img_size) & (py >= 0) & (py < img_size)
    img[py[mask], px[mask]] = True
    return img


# ---- Python pipeline ----

def run_python_pipeline(func):
    """Run Python reference: 100x100 grid, unit_circle, reverse coeffs, np.roots."""
    all_roots_re = []
    all_roots_im = []

    for i1 in range(N1):
        for i2 in range(N2):
            x1 = i1 / N1
            x2 = i2 / N2
            # unit_circle transform
            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)

            try:
                coeffs = func(t1, t2)
            except Exception:
                continue

            if coeffs is None or len(coeffs) < 2:
                continue

            # reverse (coeff_transforms: ["rev"])
            coeffs = coeffs[::-1]

            # Strip leading zeros
            first_nonzero = 0
            for k in range(len(coeffs)):
                if abs(coeffs[k]) > 1e-15:
                    first_nonzero = k
                    break
            else:
                continue
            coeffs = coeffs[first_nonzero:]

            if len(coeffs) < 2:
                continue

            try:
                r = np.roots(coeffs)
            except Exception:
                continue

            all_roots_re.extend(r.real)
            all_roots_im.extend(r.imag)

    # Build roots array compatible with rasterize
    n = len(all_roots_re)
    if n == 0:
        return np.zeros((0, 1, 2), dtype=np.float32)
    roots = np.zeros((n, 1, 2), dtype=np.float32)
    roots[:, 0, 0] = np.array(all_roots_re, dtype=np.float32)
    roots[:, 0, 1] = np.array(all_roots_im, dtype=np.float32)
    return roots


# ---- Main ----

def main():
    func_names = [f"poly_{i}" for i in range(476, 501)]
    py_funcs = load_poly_funcs("poly500.py", set(func_names))

    results = []
    print(f"{'Function':<12} {'C pixels':>10} {'Py pixels':>10} {'Overlap':>10} {'Overlap%':>10}")
    print("-" * 56)

    for name in func_names:
        try:
            # C pipeline
            c_roots, degree = run_c_pipeline(name)
            c_img = rasterize(c_roots)
            c_pixels = int(np.sum(c_img))

            # Python pipeline
            if name not in py_funcs:
                print(f"{name:<12} {'C OK':>10} {'MISSING':>10} {'N/A':>10} {'N/A':>10}")
                results.append((name, c_pixels, -1, -1, -1))
                continue

            py_roots = run_python_pipeline(py_funcs[name])
            py_img = rasterize(py_roots)
            py_pixels = int(np.sum(py_img))

            # Overlap
            overlap = int(np.sum(c_img & py_img))
            union = int(np.sum(c_img | py_img))
            pct = 100.0 * overlap / union if union > 0 else 100.0

            print(f"{name:<12} {c_pixels:>10} {py_pixels:>10} {overlap:>10} {pct:>9.1f}%")
            results.append((name, c_pixels, py_pixels, overlap, pct))

        except Exception as e:
            print(f"{name:<12} ERROR: {e}")
            results.append((name, -1, -1, -1, -1))

    # Summary
    print("\n" + "=" * 56)
    print("SUMMARY")
    valid = [r for r in results if r[4] >= 0]
    if valid:
        pcts = [r[4] for r in valid]
        print(f"  Functions tested: {len(valid)}/{len(func_names)}")
        print(f"  Mean overlap:     {np.mean(pcts):.1f}%")
        print(f"  Min overlap:      {np.min(pcts):.1f}% ({[r[0] for r in valid if r[4] == np.min(pcts)][0]})")
        print(f"  Max overlap:      {np.max(pcts):.1f}% ({[r[0] for r in valid if r[4] == np.max(pcts)][0]})")
        print(f"  >=90% overlap:    {sum(1 for p in pcts if p >= 90)}/{len(valid)}")
        print(f"  >=80% overlap:    {sum(1 for p in pcts if p >= 80)}/{len(valid)}")
    else:
        print("  No valid results.")

    errors = [r for r in results if r[4] < 0]
    if errors:
        print(f"\n  Errors/missing: {len(errors)}")
        for r in errors:
            print(f"    {r[0]}")

    return 0 if all(r[4] >= 80 for r in valid) else 1


if __name__ == "__main__":
    sys.exit(main())
