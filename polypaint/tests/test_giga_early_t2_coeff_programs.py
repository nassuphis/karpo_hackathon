"""
giga_7 / giga_9 / giga_10 / giga_11 (T2 vector-formula tier) parity.

Oracles transcribed directly from the R scripts (1-based, ascending,
uc params), returned descending. giga_11's m = as.integer(5*abs(t1+t2)
%% 17)+1 — the %%17 never fires (arg <= 10), so m = trunc+1.
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
    path = os.path.join(ROOT, "scripts", "gen_giga_early_t2_coeff_programs.py")
    spec = importlib.util.spec_from_file_location("gen_giga_early_t2", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _uc(u, v):
    return np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)


def o_giga_7(t1, t2):
    n = 30
    rec = np.linspace(np.real(t1), np.real(t2), n)
    imc = np.linspace(np.imag(t1), np.imag(t2), n)
    cf = np.exp(1j * np.sin(10 * np.pi * imc)) + np.exp(1j * np.cos(10 * np.pi * rec))
    return cf[::-1]


def o_giga_9(t1, t2):
    n = 20
    rec = np.linspace(np.real(t1), np.real(t2), n)
    imc = np.linspace(np.imag(t1), np.imag(t2), n)
    cf = 100j * imc**9 + 100 * rec**9
    return cf[::-1]


def o_giga_10(t1, t2):
    n = 120
    re1, im1 = np.real(t1), np.imag(t1)
    re2, im2 = np.real(t2), np.imag(t2)
    cf = np.zeros(n, dtype=complex)
    for k in range(1, n + 1):
        cf[k - 1] = (100 * (re1 + im2) * (k / 10) ** 2) * np.exp(1j * (re2 * k / 20)) \
            + (50 * (im1 - re2) * np.sin(k * 0.1 * im2)) * np.exp(-1j * k * 0.05 * re1)
    cf[29] = cf[29] + 1000j
    cf[59] = cf[59] - 500
    cf[89] = cf[89] + 250 * np.exp(1j * (t1 * t2))
    return cf[::-1]


def o_giga_11(t1, t2):
    n = 40
    m = int(5 * abs(t1 + t2) % 17) + 1
    modular = np.arange(n) % m
    cf = np.zeros(n, dtype=complex)
    for k in range(1, n + 1):
        cf[k - 1] = modular[k - 1] * np.exp(1j * np.pi * k / (m + t1 + t2))
    return cf[::-1]


ORACLES = {
    "giga_7": o_giga_7,
    "giga_9": o_giga_9,
    "giga_10": o_giga_10,
    "giga_11": o_giga_11,
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
        [SWEEP_TEST, "/tmp/giga_early_t2_row.bin"],
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
             os.path.join(ROOT, "scripts", "gen_giga_early_t2_coeff_programs.py"),
             "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        self.assertEqual(set(generator.BUILDERS), set(ORACLES))
        for name in generator.BUILDERS:
            with self.subTest(name=name):
                stored = json.load(open(os.path.join(ROOT, f"{name}.coeff-program.json")))
                self.assertEqual(stored["source_text"], generator.build_source_text(name))
                compiled = compile_coeff_program_source(stored["source_text"])
                errors = [d for d in (compiled.get("diagnostics") or [])
                          if d.get("level") == "error"]
                self.assertFalse(errors)
                self.assertLessEqual(compiled["token_count"], 256)
                # all four are ascending R builds
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
