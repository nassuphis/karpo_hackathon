"""
giga_39..giga_43 parity: python-era static fills, xfrm unit_circle,
zfrm rev, solve safe (never fires here). Oracles are the polys/giga.py
defs verbatim, evaluated at unit-circle points and reversed.
"""
import importlib.util
import json
import os
import subprocess
import sys
import unittest

import numpy as np

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.9, 0.9), (0.05, 0.6))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_39_43_coeff_programs.py")
    spec = importlib.util.spec_from_file_location("gen_giga_39_43", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _uc(u, v):
    return np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)


def o_giga_39(t1, t2):
    cf = np.zeros(50, dtype=complex)
    cf[[0, 9, 19, 29, 39, 49]] = [1, 2, -3, 4, -5, 6]
    cf[14] = 100 * (t1**2 + t2**2)
    cf[24] = 50 * (np.sin(t1) + 1j * np.cos(t2))
    cf[34] = 200 * (t1 * t2) + 1j * (t1**3 - t2**3)
    cf[44] = np.exp(1j * (t1 + t2)) + np.exp(-1j * (t1 - t2))
    return np.flip(cf)


def o_giga_40(t1, t2):
    cf = np.zeros(35, dtype=complex)
    cf[[0, 6, 14, 19, 26, 34]] = [1, -2, 3, -4, 5, -6]
    cf[11] = 50j * np.sin(t1**2 - t2**2)
    cf[17] = 100 * (np.cos(t1) + 1j * np.sin(t2))
    cf[24] = 50 * (t1**3 - t2**3 + 1j * t1 * t2)
    cf[29] = 200 * np.exp(1j * t1) + 50 * np.exp(-1j * t2)
    return np.flip(cf)


def o_giga_41(t1, t2):
    cf = np.zeros(60, dtype=complex)
    cf[[0, 9, 29, 49]] = [1, -5, 10, -20]
    cf[19] = 100 * np.exp(t1 + t2)
    cf[39] = 50 * (t1**2 * t2 + 1j * t2**2)
    cf[54] = np.exp(1j * t1) * np.exp(-1j * t2) + 50 * t1**3
    cf[59] = 300 * np.sin(t1 + t2) + 1j * np.cos(t1 - t2)
    return np.flip(cf)


def o_giga_42(t1, t2):
    cf = np.zeros(50, dtype=complex)
    cf[[0, 7, 15, 31, 39]] = [1, -3, 3, -1, 2]
    cf[11] = 100j * np.exp(t1**2 + t2**2)
    cf[19] = 50 * (t1**3 + t2**3)
    cf[24] = np.exp(1j * (t1 - t2)) + 10 * t1**2
    cf[44] = 200 * np.sin(t1 + t2) + 1j * np.cos(t1 - t2)
    return np.flip(cf)


def o_giga_43(t1, t2):
    cf = np.zeros(40, dtype=complex)
    cf[[0, 4, 14, 29]] = [1, -5, 10, -20]
    cf[19] = 100j * (t1**3 - t2**3)
    cf[9] = 50 * (t1**2 * t2 + 1j * t2**2)
    cf[24] = np.exp(1j * t1) + np.exp(-1j * t2)
    cf[34] = 200 * t1 * t2 * np.sin(t1 + t2)
    return np.flip(cf)


ORACLES = {
    "giga_39": o_giga_39, "giga_40": o_giga_40, "giga_41": o_giga_41,
    "giga_42": o_giga_42, "giga_43": o_giga_43,
}


def _run_vm(compiled, t1, t2):
    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [SWEEP_TEST, "/tmp/giga_39_43_row.bin"],
        input=json.dumps({
            "mode": "compute_debug",
            "function": "const",
            "cfpv": [1, 0, 0],
            "u": t1,
            "v": t2,
            "grid_n": 1000,
            "coeff_transforms": [],
            "coeff_program": payload,
        }),
        capture_output=True, text=True, timeout=120,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:300])
    meta = json.loads(proc.stdout)
    return np.array([complex(re, im) for re, im in meta["coeff"]["poly"]])


class TestGeneratorDocuments(unittest.TestCase):
    def test_documents_fresh_and_compile(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable,
             os.path.join(ROOT, "scripts", "gen_giga_39_43_coeff_programs.py"),
             "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        self.assertEqual(set(generator.GIGAS), set(ORACLES))
        for name in generator.GIGAS:
            with self.subTest(name=name):
                stored = json.load(open(os.path.join(ROOT, f"{name}.coeff-program.json")))
                self.assertEqual(stored["name"], name)
                self.assertEqual(stored["source_text"], generator.build_source_text(name))
                compiled = compile_coeff_program_source(stored["source_text"])
                errors = [d for d in (compiled.get("diagnostics") or [])
                          if d.get("level") == "error"]
                self.assertFalse(errors)
                self.assertLessEqual(compiled["token_count"], 256)
                # zfrm rev is the reversal
                self.assertIn("poly = rev(poly)", stored["source_text"])


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestVmParity(unittest.TestCase):
    def test_vm_rows_match_oracles(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name, oracle in ORACLES.items():
            compiled = compile_coeff_program_source(generator.build_source_text(name))
            for u, v in PROBES:
                with self.subTest(name=name, u=u, v=v):
                    t1, t2 = _uc(u, v)
                    ref = oracle(t1, t2)
                    got = _run_vm(compiled, u, v)
                    self.assertEqual(len(got), len(ref))
                    scale = float(np.abs(ref).max())
                    rel = float(np.abs(got - ref).max() / scale)
                    self.assertLess(rel, 1e-12, f"{name} coeff parity {rel}")

    def test_root_multisets_match(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name, oracle in ORACLES.items():
            compiled = compile_coeff_program_source(generator.build_source_text(name))
            u, v = PROBES[2]
            with self.subTest(name=name):
                t1, t2 = _uc(u, v)
                ref = oracle(t1, t2)
                got = _run_vm(compiled, u, v)
                pool = list(np.roots(ref))
                diffs = []
                for z in np.roots(got):
                    j = int(np.argmin([abs(z - p) for p in pool]))
                    diffs.append(abs(z - pool.pop(j)))
                self.assertFalse(pool)
                self.assertLess(max(diffs) if diffs else 0.0, 1e-7, name)


if __name__ == "__main__":
    unittest.main()
