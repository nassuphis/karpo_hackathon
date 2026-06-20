"""
Test all transpiled poly400 (301-400) and poly500 (401-500) functions.
Compares C sweep pipeline vs Python reference via pixel overlap.

Run: cd polypaint/tests && uv run python test_visual_301_500.py
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
from tests.native_program_helpers import translate_legacy_transforms_for_native

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")
IMG_SIZE = 1000
EXTENT = 2.0
N1, N2 = 100, 100

def load_poly_funcs(path, names):
    with open(path, "r") as f:
        source = f.read()
    tree = ast.parse(source)
    funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in names:
            start = node.lineno - 1
            end = node.end_lineno
            lines = source.splitlines()[start:end]
            func_source = "\n".join(lines)
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

def run_c_pipeline(func_name):
    coeffs_file = f"/tmp/{func_name}_coeffs.bin"
    roots_file = f"/tmp/{func_name}_roots.bin"
    spec = json.dumps(translate_legacy_transforms_for_native({
        "mode": "coeffgen", "function": func_name,
        "n1": N1, "n2": N2, "i1_start": 0, "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"], "times": 1, "dither_pass": 0,
    }))
    result = subprocess.run([SWEEP, coeffs_file], input=spec, capture_output=True, text=True, timeout=30)
    if result.returncode != 0:
        raise RuntimeError(f"coeffgen failed: {result.stderr[:200]}")
    meta1 = json.loads(result.stdout)
    spec2 = json.dumps({
        "mode": "solve", "coeffs_file": coeffs_file,
        "n_coeffs": meta1["n_coeffs"], "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1, "match_roots": False,
    })
    result2 = subprocess.run([SWEEP, roots_file], input=spec2, capture_output=True, text=True, timeout=60)
    if result2.returncode != 0:
        raise RuntimeError(f"solve failed: {result2.stderr[:200]}")
    meta2 = json.loads(result2.stdout)
    degree = meta2["degree"]
    n_t = meta2["n_t"]
    with open(roots_file, "rb") as f:
        data = f.read()
    expected = n_t * degree * 2 * 4
    floats = np.frombuffer(data[:expected], dtype=np.float32)
    return floats.reshape(n_t, degree, 2), degree

def rasterize(roots_array):
    img = np.zeros((IMG_SIZE, IMG_SIZE), dtype=bool)
    re_vals = roots_array[:, :, 0].ravel()
    im_vals = roots_array[:, :, 1].ravel()
    px = ((re_vals + EXTENT) / (2 * EXTENT) * IMG_SIZE).astype(np.int32)
    py = ((im_vals + EXTENT) / (2 * EXTENT) * IMG_SIZE).astype(np.int32)
    mask = (px >= 0) & (px < IMG_SIZE) & (py >= 0) & (py < IMG_SIZE)
    img[py[mask], px[mask]] = True
    return img

def run_python_pipeline(func):
    all_roots_re = []
    all_roots_im = []
    for i1 in range(N1):
        for i2 in range(N2):
            x1 = i1 / N1
            x2 = i2 / N2
            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)
            try:
                coeffs = func(t1, t2)
            except Exception:
                continue
            if coeffs is None or len(coeffs) < 2:
                continue
            coeffs = coeffs[::-1]
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
    n = len(all_roots_re)
    if n == 0:
        return np.zeros((0, 1, 2), dtype=np.float32)
    roots = np.zeros((n, 1, 2), dtype=np.float32)
    roots[:, 0, 0] = np.array(all_roots_re, dtype=np.float32)
    roots[:, 0, 1] = np.array(all_roots_im, dtype=np.float32)
    return roots

def main():
    # Test poly400 (301-400) and poly500 (401-500)
    batches = [
        (os.path.join(LAMBDA_DIR, "poly400.py"), range(301, 401)),
        (os.path.join(LAMBDA_DIR, "poly500.py"), range(401, 501)),
    ]

    all_results = []
    pass_count = 0
    fail_count = 0
    error_count = 0
    stub_count = 0

    # Stubbed functions (not tested)
    stubs = {304, 325, 332, 339, 347, 349, 365, 370, 390, 391,
             407, 428, 441, 443, 447, 449, 454}

    print(f"{'Function':<12} {'C px':>8} {'Py px':>8} {'Overlap%':>10} {'Status':<8}")
    print("-" * 50)

    for src_file, num_range in batches:
        func_names = [f"poly_{i}" for i in num_range]
        py_funcs = load_poly_funcs(src_file, set(func_names))

        for i in num_range:
            name = f"poly_{i}"
            if i in stubs:
                print(f"{name:<12} {'---':>8} {'---':>8} {'---':>10} {'STUB':<8}")
                stub_count += 1
                continue

            try:
                c_roots, degree = run_c_pipeline(name)
                c_img = rasterize(c_roots)
                c_pixels = int(np.sum(c_img))

                if name not in py_funcs:
                    print(f"{name:<12} {c_pixels:>8} {'???':>8} {'???':>10} {'NO_PY':<8}")
                    error_count += 1
                    continue

                py_roots = run_python_pipeline(py_funcs[name])
                py_img = rasterize(py_roots)
                py_pixels = int(np.sum(py_img))

                if c_pixels == 0 and py_pixels == 0:
                    overlap_pct = 100.0
                elif c_pixels == 0 or py_pixels == 0:
                    overlap_pct = 0.0
                else:
                    intersection = int(np.sum(c_img & py_img))
                    union = int(np.sum(c_img | py_img))
                    overlap_pct = 100.0 * intersection / union if union > 0 else 0.0

                status = "PASS" if overlap_pct >= 60.0 else "FAIL"
                if status == "PASS":
                    pass_count += 1
                else:
                    fail_count += 1

                print(f"{name:<12} {c_pixels:>8} {py_pixels:>8} {overlap_pct:>9.1f}% {status:<8}")
                all_results.append((name, c_pixels, py_pixels, overlap_pct, status))

            except Exception as e:
                print(f"{name:<12} {'ERR':>8} {'ERR':>8} {'ERR':>10} {'ERROR':<8}  {str(e)[:60]}")
                error_count += 1

    print("-" * 50)
    print(f"PASS: {pass_count}  FAIL: {fail_count}  ERROR: {error_count}  STUB: {stub_count}")
    print(f"Total: {pass_count + fail_count + error_count + stub_count}")

    if fail_count > 0:
        print(f"\nFailed functions:")
        for name, cp, pp, ov, st in all_results:
            if st == "FAIL":
                print(f"  {name}: C={cp} Py={pp} overlap={ov:.1f}%")

if __name__ == "__main__":
    main()
