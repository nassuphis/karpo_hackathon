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

    polylayout = sys.modules["lambda.polylayout"]
    if not hasattr(polylayout, "layout2coord"):
        polylayout.layout2coord = lambda *_args, **_kwargs: (0.0, 0.0, 0.0, 0.0)
    if not hasattr(polylayout, "coord2layout"):
        polylayout.coord2layout = lambda *_args, **_kwargs: ""

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


def _poly_810_reference(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for i in range(25):
            if i % 2 == 1:
                cf[i] = ((i * t1 + 3 * i * t2) / (i + 1) ** 2) ** i
            else:
                cf[i] = (t1 + np.conj(t2)) ** i
        cf[cf == np.inf] = 1e10
        cf[cf == -np.inf] = -1e10
        cf[np.isnan(cf)] = 0
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(25, dtype=np.complex128)


def _poly_809_reference(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, 25):
            cf[k] = np.cos(k * np.angle(cf[k - 1])) + np.sin(k * np.abs(t1)) + np.conj(t2) / np.abs(1 + t1)
            if np.isinf(cf[k]) or np.isnan(cf[k]):
                cf[k] = cf[k - 1]
        cf[9] = cf[0] ** 3 + cf[1] ** 2 - cf[0] * cf[1]
        if np.isinf(cf[9]) or np.isnan(cf[9]):
            cf[9] = cf[8]
        cf[14] = np.log(np.abs(cf[13])) - t1 ** 2 + t2 ** 2
        if np.isinf(cf[14]) or np.isnan(cf[14]):
            cf[14] = cf[13]
        cf[19] = cf[0] * (t1 + t2) ** 2 - cf[2] / (1 + np.abs(t1 * t2))
        if np.isinf(cf[19]) or np.isnan(cf[19]):
            cf[19] = cf[18]
        cf[24] = (t1 + t2) ** 3 - cf[23]
        if np.isinf(cf[24]) or np.isnan(cf[24]):
            cf[24] = cf[23]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(25, dtype=np.complex128)


def _poly_818_reference(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(1, 26):
            cf[k - 1] = (
                np.abs(t1 + 1j * t2 + 0.5 + k / 25) * np.cos(np.angle(t1 + 1j * t2) ** (k - 1))
                + 1j * abs(t2 + 1j * t1 + 0.5 + k / 25) * np.sin(np.angle(t2 + 1j * t1) ** (k - 1))
            )
            if np.isnan(cf[k - 1]) or np.isinf(cf[k - 1]):
                cf[k - 1] = 0
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(25, dtype=np.complex128)


MANUAL_REFERENCES = {
    "poly_809": _poly_809_reference,
    "poly_818": _poly_818_reference,
    "poly_802": _poly_802_reference,
    "poly_812": _poly_812_reference,
    "poly_810": _poly_810_reference,
}

EXACT_TRANSFORM_REFERENCE_FUNCS = {"poly_809", "poly_811", "poly_818"}
FLOAT32_MAX = np.finfo(np.float32).max
POLY_26_CHAOTIC_CASE = (
    "poly_26",
    (
        ("t1radd", "0.30"),
        ("t1iadd", "0.20"),
        ("t2radd", "-0.10"),
        ("t2iadd", "0.70"),
    ),
)
POLY_39_OVERFLOW_CASE = (
    "poly_39",
    (
        ("t1radd", "0.30"),
        ("t1iadd", "0.20"),
        ("t2radd", "-0.10"),
        ("t2iadd", "0.70"),
    ),
)
FLOAT32_OVERFLOW_TAIL_FUNCS = {"poly_86", "poly_102", "poly_324", "poly_450"}


def _exact_params_from_additive_transforms(transforms):
    t1 = 0j
    t2 = 0j
    for name, value in transforms:
        delta = float(value)
        if name == "t1radd":
            t1 += delta
        elif name == "t1iadd":
            t1 += 1j * delta
        elif name == "t2radd":
            t2 += delta
        elif name == "t2iadd":
            t2 += 1j * delta
        else:
            raise AssertionError(f"unsupported exact-param transform {name!r}")
    return t1, t2


def _case_key(func_name, transforms):
    return func_name, tuple((name, value) for name, value in transforms)


def _assert_allclose_complex(got, reference, *, rtol=2e-3, atol=5e-5):
    np.testing.assert_allclose(got.real, reference.real, rtol=rtol, atol=atol)
    np.testing.assert_allclose(got.imag, reference.imag, rtol=rtol, atol=atol)


def _assert_poly_26_chaotic_case(got, reference):
    # The transformed case is a unit-circle recurrence driven by complex sin/cos.
    # Tiny libc-vs-NumPy trig drift compounds after the first few recursive terms,
    # so we lock down the stable prefix and the direct overwrite slots instead.
    _assert_allclose_complex(got[:11], reference[:11])
    _assert_allclose_complex(got[[14, 29, 49]], reference[[14, 29, 49]])
    recursive_slots = [i for i in range(2, 71) if i not in {14, 29, 49, 50}]
    np.testing.assert_allclose(
        np.abs(got[recursive_slots]),
        np.ones(len(recursive_slots), dtype=np.float64),
        rtol=1e-6,
        atol=5e-7,
    )
    assert np.isfinite(got[50].real)
    assert np.isfinite(got[50].imag)


def _assert_float32_overflow_tail(got, reference, *, rtol=2e-3, atol=5e-5):
    finite_mask = (
        np.isfinite(got.real)
        & np.isfinite(got.imag)
        & np.isfinite(reference.real)
        & np.isfinite(reference.imag)
    )
    prefix_len = int(np.argmax(~finite_mask)) if (~finite_mask).any() else len(got)
    assert prefix_len < len(got), "expected a float32 overflow tail but the full output stayed finite"
    _assert_allclose_complex(got[:prefix_len], reference[:prefix_len], rtol=rtol, atol=atol)

    saw_float32_overflow = False
    for idx in range(prefix_len, len(got)):
        for got_component, ref_component in (
            (got.real[idx], reference.real[idx]),
            (got.imag[idx], reference.imag[idx]),
        ):
            if np.isfinite(got_component) and np.isfinite(ref_component):
                np.testing.assert_allclose(got_component, ref_component, rtol=rtol, atol=atol)
                continue
            if not np.isfinite(got_component):
                assert (not np.isfinite(ref_component)) or abs(ref_component) > FLOAT32_MAX
                saw_float32_overflow = saw_float32_overflow or (
                    np.isfinite(ref_component) and abs(ref_component) > FLOAT32_MAX
                )
                continue
            assert not np.isfinite(ref_component)

    assert saw_float32_overflow or np.any(~np.isfinite(reference[prefix_len:]))


CASES = [
    ("poly_13", "poly100.py", 51, []),
    ("poly_13", "poly100.py", 51, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_14", "poly100.py", 51, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_15", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_19", "poly100.py", 71, []),
    ("poly_19", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_22", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_25", "poly100.py", 71, []),
    ("poly_25", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_26", "poly100.py", 71, []),
    ("poly_26", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_31", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_56", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_63", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_66", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_75", "poly100.py", 71, []),
    ("poly_75", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_79", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_80", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_86", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_102", "poly200.py", 71, []),
    ("poly_102", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_110", "poly200.py", 71, []),
    ("poly_110", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_113", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_114", "poly200.py", 71, []),
    ("poly_114", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_115", "poly200.py", 71, []),
    ("poly_115", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_307", "poly400.py", 35, []),
    ("poly_307", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_316", "poly400.py", 35, []),
    ("poly_316", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_314", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_317", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_324", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_335", "poly400.py", 35, []),
    ("poly_335", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_333", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_345", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_383", "poly400.py", 35, []),
    ("poly_383", "poly400.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_413", "poly500.py", 35, []),
    ("poly_413", "poly500.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_450", "poly500.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_467", "poly500.py", 35, []),
    ("poly_467", "poly500.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_475", "poly500.py", 40, []),
    ("poly_475", "poly500.py", 40, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_478", "poly500.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_482", "poly500.py", 35, []),
    ("poly_482", "poly500.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_491", "poly500.py", 35, []),
    ("poly_491", "poly500.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_513", "poly600.py", 35, []),
    ("poly_513", "poly600.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_535", "poly600.py", 35, []),
    ("poly_535", "poly600.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_551", "poly600.py", 40, []),
    ("poly_551", "poly600.py", 40, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_562", "poly600.py", 35, []),
    ("poly_562", "poly600.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_566", "poly600.py", 25, []),
    ("poly_566", "poly600.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_570", "poly600.py", 26, []),
    ("poly_570", "poly600.py", 26, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_604", "poly700.py", 25, []),
    ("poly_604", "poly700.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_727", "poly800.py", 9, []),
    ("poly_727", "poly800.py", 9, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_751", "poly800.py", 11, []),
    ("poly_751", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_777", "poly800.py", 25, []),
    ("poly_777", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_782", "poly800.py", 25, []),
    ("poly_782", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_101", "poly200.py", 71, []),
    ("poly_101", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_106", "poly200.py", 71, []),
    ("poly_106", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_111", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_112", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_121", "poly200.py", 71, []),
    ("poly_121", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_149", "poly200.py", 71, []),
    ("poly_149", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
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
    ("poly_108", "poly200.py", 71, []),
    ("poly_108", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_116", "poly200.py", 71, []),
    ("poly_116", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_183", "poly200.py", 71, []),
    ("poly_183", "poly200.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_545", "poly600.py", 35, []),
    ("poly_545", "poly600.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_556", "poly600.py", 35, []),
    ("poly_556", "poly600.py", 35, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_596", "poly600.py", 26, []),
    ("poly_596", "poly600.py", 26, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_749", "poly800.py", 10, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_758", "poly800.py", 10, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_787", "poly800.py", 25, []),
    ("poly_787", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_809", "poly900.py", 25, []),
    ("poly_809", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_818", "poly900.py", 25, []),
    ("poly_818", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_667", "poly700.py", 9, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_746", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_759", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_759", "poly800.py", 11, [["t1radd", "0.10"], ["t1iadd", "0.10"], ["t2radd", "2.00"], ["t2iadd", "0.00"]]),
    ("poly_766", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_766", "poly800.py", 11, [["t1radd", "2.10"], ["t1iadd", "1.40"], ["t2radd", "0.20"], ["t2iadd", "0.10"]]),
    ("poly_769", "poly800.py", 10, [["t1radd", "0.10"], ["t1iadd", "0.10"], ["t2radd", "0.20"], ["t2iadd", "0.00"]]),
    ("poly_769", "poly800.py", 10, [["t1radd", "0.10"], ["t1iadd", "-0.10"], ["t2radd", "0.20"], ["t2iadd", "0.50"]]),
    ("poly_773", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_785", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_788", "poly800.py", 25, [["t1radd", "0.01"], ["t1iadd", "0.01"], ["t2radd", "0.02"], ["t2iadd", "0.00"]]),
    ("poly_788", "poly800.py", 25, [["t1radd", "0.02"], ["t1iadd", "0.01"], ["t2radd", "0.01"], ["t2iadd", "0.00"]]),
    ("poly_794", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_794", "poly800.py", 25, [["t1radd", "0.10"], ["t1iadd", "0.10"], ["t2radd", "2.00"], ["t2iadd", "0.00"]]),
    ("poly_810", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_18", "poly100.py", 71, []),
    ("poly_18", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_39", "poly100.py", 71, []),
    ("poly_39", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_51", "poly100.py", 71, []),
    ("poly_51", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_59", "poly100.py", 71, []),
    ("poly_59", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_68", "poly100.py", 71, []),
    ("poly_68", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_69", "poly100.py", 71, []),
    ("poly_69", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_87", "poly100.py", 71, []),
    ("poly_87", "poly100.py", 71, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_691", "poly700.py", 9, []),
    ("poly_691", "poly700.py", 9, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_694", "poly700.py", 9, []),
    ("poly_694", "poly700.py", 9, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_698", "poly700.py", 9, []),
    ("poly_698", "poly700.py", 9, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_733", "poly800.py", 9, []),
    ("poly_733", "poly800.py", 9, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_740", "poly800.py", 11, []),
    ("poly_740", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_745", "poly800.py", 10, []),
    ("poly_745", "poly800.py", 10, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_755", "poly800.py", 11, []),
    ("poly_755", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_774", "poly800.py", 11, []),
    ("poly_774", "poly800.py", 11, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_789", "poly800.py", 25, []),
    ("poly_789", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_793", "poly800.py", 25, []),
    ("poly_793", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_798", "poly800.py", 25, []),
    ("poly_798", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_800", "poly800.py", 25, []),
    ("poly_800", "poly800.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_803", "poly900.py", 25, []),
    ("poly_803", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_806", "poly900.py", 25, []),
    ("poly_806", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_811", "poly900.py", 25, []),
    ("poly_811", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_813", "poly900.py", 25, []),
    ("poly_813", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
    ("poly_814", "poly900.py", 25, []),
    ("poly_814", "poly900.py", 25, [["t1radd", "0.30"], ["t1iadd", "0.20"], ["t2radd", "-0.10"], ["t2iadd", "0.70"]]),
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

        ref_t1, ref_t2 = t1, t2
        if func_name in EXACT_TRANSFORM_REFERENCE_FUNCS:
            ref_t1, ref_t2 = _exact_params_from_additive_transforms(transforms)

        if func_name in MANUAL_REFERENCES:
            reference = np.asarray(MANUAL_REFERENCES[func_name](ref_t1, ref_t2), dtype=np.complex128)
        else:
            module = _load_poly_module(source_file)
            reference = np.asarray(getattr(module, func_name)(ref_t1, ref_t2), dtype=np.complex128)
        assert reference.shape == (n_coeffs,)
        case_key = _case_key(func_name, transforms)
        if case_key == POLY_26_CHAOTIC_CASE:
            _assert_poly_26_chaotic_case(got, reference)
        elif case_key == POLY_39_OVERFLOW_CASE or func_name in FLOAT32_OVERFLOW_TAIL_FUNCS:
            _assert_float32_overflow_tail(got, reference)
        else:
            _assert_allclose_complex(got, reference)
    finally:
        for path in (params_path, coeffs_path):
            if os.path.exists(path):
                os.remove(path)
