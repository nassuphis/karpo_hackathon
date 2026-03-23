"""Visual comparison test for g1-g99 coefficient functions.
Compares C transpiled implementations against Python originals from ops_poly.py.

Run: cd polypaint && uv run python tests/test_visual_g1_g99.py
"""
import ast, json, os, subprocess, numpy as np

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test"); IMG_SIZE = 1000; EXTENT = 2.0; N1 = N2 = 100


def load_g_funcs(path, names):
    """Extract g-functions from ops_poly.py, adapting (z,a,state) → (t1,t2) call convention."""
    with open(path) as f: source = f.read()
    # Also load _safe_div helper
    preamble = (
        'import math, cmath\n'
        'import numpy as np\n'
        'pi = math.pi\n'
        'def _safe_div(top, bot, eps=1e-12):\n'
        '    br=bot.real; bi=bot.imag; denom=br*br+bi*bi+eps*eps\n'
        '    tr=top.real; ti=top.imag\n'
        '    return (tr*br+ti*bi)/denom + 1j*(ti*br-tr*bi)/denom\n'
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
                print(f"  WARNING: could not load {node.name}: {e}")
    return funcs


def run_c(name):
    cf = f'/tmp/{name}_c.bin'; rf = f'/tmp/{name}_r.bin'
    r = subprocess.run([SWEEP, cf], input=json.dumps({
        'mode': 'coeffgen', 'function': name, 'n1': N1, 'n2': N2,
        'i1_start': 0, 'i1_end': N1,
        'param_transforms': [['unit_circle']], 'coeff_transforms': ['rev'],
        'times': 1,
    }), capture_output=True, text=True, timeout=30)
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
    """Run Python g-function with (z, a, state) signature."""
    ar, ai = [], []
    for i1 in range(N1):
        for i2 in range(N2):
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            try:
                cf = func(np.array([t1, t2]), None, None)
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


# Stubbed functions (transpiler couldn't handle, produce zero output)
# Manual stubs (compile errors): 7,8,11,31,38,48,49,50,51,71,82,84,90,92,95,97,98
# Auto-stubs (unhandled constructs): 27,32,47,52,57,58,62,67,81,83,88,91
STUBBED = {7, 8, 11, 27, 31, 32, 38, 47, 48, 49, 50, 51, 52, 57, 58,
           62, 67, 71, 81, 82, 83, 84, 88, 90, 91, 92, 95, 97, 98}

names = [f'g{i}' for i in range(1, 100)]
py_funcs = load_g_funcs(os.path.join(LAMBDA_DIR, 'ops_poly.py'), set(names))

pc = fc = ec = sc = 0
results = []
print(f"{'Func':<12} {'C px':>8} {'Py px':>8} {'Ov%':>8} {'St':<6}")
print('-' * 44)
for i in range(1, 100):
    name = f'g{i}'
    if i in STUBBED:
        print(f'{name:<12} {"STUB":>8} {"---":>8} {"---":>8} STUB')
        sc += 1
        continue
    try:
        cr = run_c(name)
        ci = rast(cr)
        cp = int(np.sum(ci))
        if name not in py_funcs:
            print(f'{name:<12} {cp:>8} {"???":>8} {"???":>8} NO_PY')
            ec += 1
            continue
        pr = run_py(py_funcs[name])
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
        print(f'{name:<12} {cp:>8} {pp:>8} {ov:>7.1f}% {st:<6}')
        results.append((name, cp, pp, ov, st))
    except Exception as e:
        print(f'{name:<12} {"ERR":>8} {"ERR":>8} {"ERR":>8} ERROR  {str(e)[:60]}')
        ec += 1
print('-' * 44)
print(f'PASS: {pc}  FAIL: {fc}  ERROR: {ec}  STUB: {sc}')
if fc:
    print('\nFailed:')
    for n, c, p, o, s in results:
        if s == 'FAIL':
            print(f'  {n}: C={c} Py={p} ov={o:.1f}%')
