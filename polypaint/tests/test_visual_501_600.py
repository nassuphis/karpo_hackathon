"""Test poly600 (501-600) visual comparison.
Run: cd polypaint/tests && uv run python test_visual_501_600.py
"""
import ast, json, os, subprocess, textwrap, numpy as np

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
    spec = json.dumps({
        "mode": "coeffgen", "function": func_name,
        "n1": N1, "n2": N2, "i1_start": 0, "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"], "times": 1, "dither_pass": 0,
    })
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
    degree, n_t = meta2["degree"], meta2["n_t"]
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
    all_re, all_im = [], []
    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            try:
                coeffs = func(t1, t2)
            except Exception:
                continue
            if coeffs is None or len(coeffs) < 2:
                continue
            coeffs = coeffs[::-1]
            first_nz = next((k for k in range(len(coeffs)) if abs(coeffs[k]) > 1e-15), None)
            if first_nz is None: continue
            coeffs = coeffs[first_nz:]
            if len(coeffs) < 2: continue
            try:
                r = np.roots(coeffs)
            except Exception:
                continue
            all_re.extend(r.real)
            all_im.extend(r.imag)
    n = len(all_re)
    if n == 0:
        return np.zeros((0, 1, 2), dtype=np.float32)
    roots = np.zeros((n, 1, 2), dtype=np.float32)
    roots[:, 0, 0] = np.array(all_re, dtype=np.float32)
    roots[:, 0, 1] = np.array(all_im, dtype=np.float32)
    return roots

stubs = {535}
func_names = [f"poly_{i}" for i in range(501, 601)]
py_funcs = load_poly_funcs(os.path.join(LAMBDA_DIR, "poly600.py"), set(func_names))

pass_count = fail_count = error_count = stub_count = 0
results = []
print(f"{'Function':<12} {'C px':>8} {'Py px':>8} {'Overlap%':>10} {'Status':<8}")
print("-" * 50)

for i in range(501, 601):
    name = f"poly_{i}"
    if i in stubs:
        print(f"{name:<12} {'---':>8} {'---':>8} {'---':>10} {'STUB':<8}")
        stub_count += 1
        continue
    try:
        c_roots, degree = run_c_pipeline(name)
        c_img = rasterize(c_roots)
        c_px = int(np.sum(c_img))
        if name not in py_funcs:
            print(f"{name:<12} {c_px:>8} {'???':>8} {'???':>10} {'NO_PY':<8}")
            error_count += 1
            continue
        py_roots = run_python_pipeline(py_funcs[name])
        py_img = rasterize(py_roots)
        py_px = int(np.sum(py_img))
        if c_px == 0 and py_px == 0:
            ov = 100.0
        elif c_px == 0 or py_px == 0:
            ov = 0.0
        else:
            inter = int(np.sum(c_img & py_img))
            union = int(np.sum(c_img | py_img))
            ov = 100.0 * inter / union if union > 0 else 0.0
        status = "PASS" if ov >= 60.0 else "FAIL"
        if status == "PASS": pass_count += 1
        else: fail_count += 1
        print(f"{name:<12} {c_px:>8} {py_px:>8} {ov:>9.1f}% {status:<8}")
        results.append((name, c_px, py_px, ov, status))
    except Exception as e:
        print(f"{name:<12} {'ERR':>8} {'ERR':>8} {'ERR':>10} {'ERROR':<8}  {str(e)[:60]}")
        error_count += 1

print("-" * 50)
print(f"PASS: {pass_count}  FAIL: {fail_count}  ERROR: {error_count}  STUB: {stub_count}")
if fail_count:
    print(f"\nFailed:")
    for name, cp, pp, ov, st in results:
        if st == "FAIL":
            print(f"  {name}: C={cp} Py={pp} overlap={ov:.1f}%")
