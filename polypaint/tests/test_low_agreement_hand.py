import importlib.util
import json
import os
import struct
import subprocess
import sys
import types

import numpy as np
import pytest


ROOT = os.path.dirname(os.path.dirname(__file__))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def _run_sweep(spec, out_path):
    result = subprocess.run(
        [SWEEP, out_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr)
    return json.loads(result.stdout)


def _read_params(path):
    data = open(path, "rb").read()
    floats = struct.unpack(f"<{len(data) // 4}f", data)
    return [
        (complex(floats[i], floats[i + 1]), complex(floats[i + 2], floats[i + 3]))
        for i in range(0, len(floats), 4)
    ]


def _read_coeffs(path, n_coeffs):
    data = open(path, "rb").read()
    floats = struct.unpack(f"<{len(data) // 4}f", data)
    stride = n_coeffs * 2
    coeffs = []
    for start in range(0, len(floats), stride):
        coeffs.append(np.array([
            complex(floats[start + j * 2], floats[start + j * 2 + 1])
            for j in range(n_coeffs)
        ], dtype=np.complex128))
    return coeffs


def _ensure_poly_import_stubs():
    pkg = sys.modules.get("lambda")
    if pkg is None:
        pkg = types.ModuleType("lambda")
        pkg.__path__ = []
        sys.modules["lambda"] = pkg

    for sub in ("polystate", "letters", "zfrm", "polychess", "xfrm", "polylayout"):
        full = f"lambda.{sub}"
        if full not in sys.modules:
            sys.modules[full] = types.ModuleType(full)

    scipy_mod = sys.modules.get("scipy")
    if scipy_mod is None:
        scipy_mod = types.ModuleType("scipy")
        scipy_mod.__path__ = []
        sys.modules["scipy"] = scipy_mod
    special = sys.modules.get("scipy.special")
    if special is None:
        special = types.ModuleType("scipy.special")
        special.sph_harm = lambda *args, **kwargs: 0
        sys.modules["scipy.special"] = special


_MODULE_CACHE = {}


def _load_poly_module(filename):
    cached = _MODULE_CACHE.get(filename)
    if cached is not None:
        return cached
    _ensure_poly_import_stubs()
    module_name = f"lambda.{filename[:-3]}"
    spec = importlib.util.spec_from_file_location(module_name, os.path.join(LAMBDA_DIR, filename))
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)
    _MODULE_CACHE[filename] = module
    return module


def _poly_802_reference(t1, t2):
    cf = np.zeros(25, dtype=np.complex128)
    cf[0:3] = [2 * t1 + 3 * t2, 3 * t1 - 2 * t2, t1**2 - t2**2]
    cf[3:5] = [np.real(t1 * t2), np.imag(t1 * t2)]
    for k in range(5, 20):
        cf[k] = np.sin(cf[k - 1]) + np.cos(cf[k - 2])
        mod_cf = np.abs(cf[k])
        if mod_cf != 0:
            cf[k] = cf[k] / mod_cf
        else:
            cf[k] = 1
    return cf.astype(np.complex128)


def _poly_812_reference(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 * t2
        for k in range(1, 25):
            cf[k] = (cf[k - 1] ** 2 + np.real(t1) + np.imag(1j * t2)) / (1 + np.abs(cf[k - 1]))
            if np.abs(cf[k]) > 1e6 or np.isnan(cf[k]) or np.isinf(cf[k]):
                cf[k] = cf[k - 1]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(25, dtype=np.complex128)


MANUAL_REFERENCES = {
    "poly_802": _poly_802_reference,
    "poly_812": _poly_812_reference,
}


CASES = [
    ("poly_111", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_112", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_504", "poly600.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_741", "poly800.py", 10, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_742", "poly800.py", 10, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_760", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_762", "poly800.py", 10, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_765", "poly800.py", 11, [["t1radd", "0.60"], ["t1iadd", "0.10"], ["t2radd", "0.20"], ["t2iadd", "0.00"]]),
    ("poly_765", "poly800.py", 11, [["t1radd", "0.10"], ["t1iadd", "0.10"], ["t2radd", "0.20"], ["t2iadd", "0.50"]]),
    ("poly_776", "poly800.py", 10, [["t1radd", "0.10"], ["t1iadd", "0.10"], ["t2radd", "0.20"], ["t2iadd", "0.00"]]),
    ("poly_776", "poly800.py", 10, [["t1radd", "0.10"], ["t1iadd", "0.10"], ["t2radd", "2.00"], ["t2iadd", "0.00"]]),
    ("poly_780", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_792", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_792", "poly800.py", 25, []),
    ("poly_799", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_799", "poly800.py", 25, []),
    ("poly_802", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_812", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
]


@pytest.mark.parametrize("func_name,source_file,n_coeffs,transforms", CASES)
def test_low_agreement_hand_overrides_match_python_reference(func_name, source_file, n_coeffs, transforms):
    params_path = f"/tmp/{func_name}_params.bin"
    coeffs_path = f"/tmp/{func_name}_coeffs.bin"
    try:
        _run_sweep({
            "mode": "param_dump",
            "n1": 1,
            "n2": 1,
            "param_transforms": transforms,
        }, params_path)
        param_pairs = _read_params(params_path)
        assert len(param_pairs) == 1
        t1, t2 = param_pairs[0]

        meta = _run_sweep({
            "mode": "coeffgen",
            "function": func_name,
            "n1": 1,
            "n2": 1,
            "i1_start": 0,
            "i1_end": 1,
            "param_transforms": transforms,
            "coeff_transforms": [],
            "times": 1,
        }, coeffs_path)

        assert meta["n_coeffs"] == n_coeffs
        coeff_steps = _read_coeffs(coeffs_path, n_coeffs)
        assert len(coeff_steps) == 1
        got = coeff_steps[0]

        if func_name in MANUAL_REFERENCES:
            reference = np.asarray(MANUAL_REFERENCES[func_name](t1, t2), dtype=np.complex128)
        else:
            module = _load_poly_module(source_file)
            reference = np.asarray(getattr(module, func_name)(t1, t2), dtype=np.complex128)
        assert reference.shape == (n_coeffs,)
        np.testing.assert_allclose(got.real, reference.real, rtol=2e-3, atol=5e-5)
        np.testing.assert_allclose(got.imag, reference.imag, rtol=2e-3, atol=5e-5)
    finally:
        for path in (params_path, coeffs_path):
            if os.path.exists(path):
                os.remove(path)
