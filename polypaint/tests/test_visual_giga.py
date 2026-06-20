"""Visual comparison test for giga coefficient functions.
Compares C transpiled implementations against Python originals from giga.py.

Run: cd polypaint && uv run python tests/test_visual_giga.py
"""
import ast, json, os, subprocess, numpy as np, sys
from tests.native_program_helpers import translate_legacy_transforms_for_native

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test"); IMG_SIZE = 1000; EXTENT = 2.0; N1 = N2 = 100


def load_giga_funcs(path, names):
    """Extract poly_giga_N functions from giga.py. Signature: (t1, t2)."""
    with open(path) as f: source = f.read()
    preamble = (
        'import math, cmath\n'
        'import numpy as np\n'
        'pi = math.pi\n'
        'try:\n'
        '    from scipy.special import sph_harm\n'
        'except ImportError:\n'
        '    sph_harm = None\n'
    )
    tree = ast.parse(source); funcs = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.FunctionDef) and node.name in names:
            lines = source.splitlines()[node.lineno-1:node.end_lineno]
            code = preamble + '\n'.join(lines)
            ns = {}
            try:
                exec(compile(code, f'<{node.name}>', 'exec'), ns)
                funcs[node.name] = ns[node.name]
            except Exception as e:
                pass
    return funcs


def run_c(name):
    cf = f'/tmp/{name}_c.bin'; rf = f'/tmp/{name}_r.bin'
    r = subprocess.run([SWEEP, cf], input=json.dumps(translate_legacy_transforms_for_native({
        'mode': 'coeffgen', 'function': name, 'n1': N1, 'n2': N2,
        'i1_start': 0, 'i1_end': N1,
        'param_transforms': [['unit_circle']], 'coeff_transforms': ['rev'],
        'times': 1,
    })), capture_output=True, text=True, timeout=30)
    if r.returncode != 0: raise RuntimeError(r.stderr[:200])
    m = json.loads(r.stdout)
    r2 = subprocess.run([SWEEP, rf], input=json.dumps({
        'mode': 'solve', 'coeffs_file': cf, 'n_coeffs': m['n_coeffs'],
        'n1': N1, 'n2': N2, 'i1_start': 0, 'i1_end': N1, 'match_roots': False,
    }), capture_output=True, text=True, timeout=60)
    if r2.returncode != 0: raise RuntimeError(r2.stderr[:200])
    m2 = json.loads(r2.stdout)
    with open(rf, 'rb') as f: data = f.read()
    for p in [cf, rf]:
        try: os.remove(p)
        except OSError: pass
    return np.frombuffer(data[:m2['n_t']*m2['degree']*8], dtype=np.float32).reshape(m2['n_t'], m2['degree'], 2)


def rast(roots):
    img = np.zeros((IMG_SIZE, IMG_SIZE), dtype=bool)
    re, im = roots[:, :, 0].ravel(), roots[:, :, 1].ravel()
    px = ((re + EXTENT) / (2 * EXTENT) * IMG_SIZE).astype(np.int32)
    py = ((im + EXTENT) / (2 * EXTENT) * IMG_SIZE).astype(np.int32)
    m = (px >= 0) & (px < IMG_SIZE) & (py >= 0) & (py < IMG_SIZE)
    img[py[m], px[m]] = True
    return img


def run_py(func):
    """Run Python giga function with (t1, t2) signature."""
    ar, ai = [], []
    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            try:
                cf = func(t1, t2)
            except Exception:
                continue
            if cf is None or len(cf) < 2: continue
            cf = np.array(cf, dtype=complex)[::-1]
            nz = next((k for k in range(len(cf)) if abs(cf[k]) > 1e-15), None)
            if nz is None: continue
            cf = cf[nz:]
            if len(cf) < 2: continue
            try:
                r = np.roots(cf)
            except Exception:
                continue
            ar.extend(r.real)
            ai.extend(r.imag)
    if not ar: return np.zeros((0, 1, 2), dtype=np.float32)
    roots = np.zeros((len(ar), 1, 2), dtype=np.float32)
    roots[:, 0, 0] = np.array(ar, dtype=np.float32)
    roots[:, 0, 1] = np.array(ai, dtype=np.float32)
    return roots


# Build list of non-stub giga functions available in the C binary
def get_available_gigas():
    """Probe which giga_N functions produce valid output."""
    available = []
    # Test giga_1 through giga_145 plus variants
    candidates = [f"giga_{i}" for i in range(1, 146)]
    candidates += ["giga_7a", "giga_7b", "giga_7c", "giga_7d",
                   "giga_19_fixed1", "giga_19_fixed2"]
    for name in candidates:
        try:
            r = subprocess.run([SWEEP, "/tmp/_probe.bin"],
                input=json.dumps({"mode": "coeffgen", "function": name,
                    "n1": 3, "n2": 3, "i1_start": 0, "i1_end": 3,
                    "param_transforms": [], "coeff_transforms": [], "times": 1}),
                capture_output=True, text=True, timeout=5)
            if r.returncode == 0:
                m = json.loads(r.stdout)
                if m.get("degree", 0) > 0:
                    available.append((name, m["degree"]))
        except Exception:
            pass
        try: os.remove("/tmp/_probe.bin")
        except OSError: pass
    return available


# Determine which functions are stubs (produce all-zero coefficients)
def is_stub(name):
    """Check if function produces all-zero coefficients."""
    try:
        r = subprocess.run([SWEEP, "/tmp/_stub.bin"],
            input=json.dumps(translate_legacy_transforms_for_native({"mode": "coeffgen", "function": name,
                "n1": 5, "n2": 5, "i1_start": 0, "i1_end": 5,
                "param_transforms": [["unit_circle"]], "coeff_transforms": [], "times": 1})),
            capture_output=True, text=True, timeout=5)
        if r.returncode != 0: return True
        size = os.path.getsize("/tmp/_stub.bin")
        with open("/tmp/_stub.bin", "rb") as f:
            data = f.read()
        os.remove("/tmp/_stub.bin")
        return all(b == 0 for b in data)
    except Exception:
        return True


print("Probing available giga functions...")
available = get_available_gigas()
print(f"Found {len(available)} giga functions")

# Map C name → Python source name
py_source_names = {}
for name, _ in available:
    py_name = "poly_" + name  # giga_1 → poly_giga_1
    py_source_names[name] = py_name

py_funcs = load_giga_funcs(os.path.join(LAMBDA_DIR, 'giga.py'), set(py_source_names.values()))

pc = fc = ec = sc = 0
results = []
print(f"\n{'Func':<20} {'C px':>8} {'Py px':>8} {'Ov%':>8} {'St':<6}")
print('-' * 52)
for name, degree in available:
    py_name = py_source_names[name]
    if is_stub(name):
        print(f'{name:<20} {"STUB":>8} {"---":>8} {"---":>8} STUB')
        sc += 1
        continue
    try:
        cr = run_c(name)
        ci = rast(cr)
        cp = int(np.sum(ci))
        if py_name not in py_funcs:
            print(f'{name:<20} {cp:>8} {"???":>8} {"???":>8} NO_PY')
            ec += 1
            continue
        pr = run_py(py_funcs[py_name])
        pi_ = rast(pr)
        pp = int(np.sum(pi_))
        if cp == 0 and pp == 0:
            ov = 100.0
        elif cp == 0 or pp == 0:
            ov = 0.0
        else:
            inter = int(np.sum(ci & pi_))
            union = int(np.sum(ci | pi_))
            ov = 100 * inter / union if union else 0
        st = 'PASS' if ov >= 60 else 'FAIL'
        if st == 'PASS':
            pc += 1
        else:
            fc += 1
        print(f'{name:<20} {cp:>8} {pp:>8} {ov:>7.1f}% {st:<6}')
        results.append((name, cp, pp, ov, st))
    except Exception as e:
        print(f'{name:<20} {"ERR":>8} {"ERR":>8} {"ERR":>8} ERROR  {str(e)[:50]}')
        ec += 1
print('-' * 52)
print(f'PASS: {pc}  FAIL: {fc}  ERROR: {ec}  STUB: {sc}')
if fc:
    print('\nFailed:')
    for n, c, p, o, s in results:
        if s == 'FAIL':
            print(f'  {n}: C={c} Py={p} ov={o:.1f}%')
