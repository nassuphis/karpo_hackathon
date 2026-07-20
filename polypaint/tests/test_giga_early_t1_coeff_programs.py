"""
giga_1..giga_30 T1 (static-fill) parity.

Oracles are transcribed DIRECTLY from the reference generators (the R
scripts in /Users/nicknassuphis/pyroots/giga_N/ for the Dec-era ones,
polys/giga.py for the Jan-era ones), independently of the generator's
source-text expressions, and return the numpy-DESCENDING coefficient
vector the app must emit:

- R scripts ending rev(cf) (giga_1, 2): cf layout already descending.
- R scripts returning cf (ascending, base::polyroot): oracle reverses.
- python-era defs (giga_28..30): descending as written.

Params: uc for both axes (param_space composes x_01 then x_circle
over ONE sequence: t1 = e^(2 pi i u), t2 = e^(2 pi i v)).
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
    path = os.path.join(ROOT, "scripts", "gen_giga_early_t1_coeff_programs.py")
    spec = importlib.util.spec_from_file_location("gen_giga_early_t1", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _uc(u, v):
    return np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)


# ---- oracles: R text (1-based cf), returned in the R script's own
# ---- feed order, converted to descending at the end of each helper


def _asc(cf):
    return cf[::-1]


def o_giga_1(t1, t2):
    cf = np.zeros(25, dtype=complex)
    cf[0] = 30 * (t1**2 * t2)
    cf[1] = 30 * (t1 * t2**2)
    cf[2] = 40 * (t1**3)
    cf[3] = 40 * (t2**3)
    cf[4] = -25 * (t1**2)
    cf[5] = -25 * (t2**2)
    cf[6] = 10 * (t1 * t2)
    cf[9] = 100 * (t1**4 * t2**4)
    cf[11] = -5 * t1
    cf[13] = 5 * t2
    cf[24] = -10
    return cf          # R: rev(cf) into polyroot -> this IS descending


def o_giga_2(t1, t2):
    cf = np.zeros(25, dtype=complex)
    cf[9] = 200 * (t1**2 + t2**2)
    cf[15] = 150 * (t1**3 * t2**5)
    cf[7] = -80 * (t1**4 - t2**2)
    cf[5] = 50 * (t1**3 + t2)
    cf[2] = 20 * (t1 - t2)
    cf[1] = -10 * (t1 * t2)
    cf[0] = -5
    cf[19] = -30 * (t1**5 - t2**5)
    cf[24] = 10 * (t1 * t2**3)
    return cf          # rev(cf) script -> descending


def o_giga_3(t1, t2):
    n = 25
    cf = np.zeros(n, dtype=complex)
    cf[0] = 1
    cf[n - 11] = np.exp(t1 - t2)
    cf[n - 9] = np.exp(1j * t1)
    cf[n - 8] = np.exp(t1)
    cf[n - 7] = np.exp(-t1)
    cf[n - 6] = np.exp(-1j * t1)
    cf[n - 2] = np.exp(1j * t2)
    cf[n - 1] = 1 + 1j
    return _asc(cf)


def o_giga_4(t1, t2):
    cf = np.zeros(25, dtype=complex)
    cf[0] = 100
    cf[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
    cf[14] = 100 * t1**3 - 100 * t1**2 + 100 * t1 - 100
    cf[16] = 100 * t1**3 + 100 * t1**2 - 100 * t1 - 100
    cf[20] = -10
    cf[24] = np.exp(0.2j * t2)
    return _asc(cf)


def o_giga_5(t1, t2):
    cf = np.zeros(26, dtype=complex)
    cf[[0, 4, 12, 19, 20, 24]] = [1, 4, 4, -9, -1.9, 0.2]
    cf[6] = 100j * t2**3 + 100j * t2**2 - 100j * t2 - 100j
    cf[8] = 100j * t1**3 + 100j * t1**2 + 100j * t2 - 100j
    cf[14] = 100j * t2**3 - 100j * t2**2 + 100j * t2 - 100j
    return _asc(cf)


def o_giga_6(t1, t2):
    n = 10
    cf = np.zeros(n, dtype=complex)
    cf[0] = 150 * t2**3 - 150j * t1**2
    cf[n // 2 - 1] = 100 * (t1 - t2)
    cf[n - 1] = 10j
    return _asc(cf)


def o_giga_12(t1, t2):
    cf = np.zeros(25, dtype=complex)
    cf[0] = 2
    cf[2] = 50 * t1**3
    cf[4] = 50 * t2**3
    cf[6] = -30 * t1**2
    cf[8] = -30 * t2**2
    cf[10] = 100 * (t1 * t2)
    cf[12] = 50 * (t1**2 * t2)
    cf[14] = 50 * (t1 * t2**2)
    cf[19] = -75 * (t1**3 * t2**3)
    cf[20] = 3.5 * t2
    cf[24] = -2 * t1
    return _asc(cf)


def o_giga_13(t1, t2):
    cf = np.zeros(25, dtype=complex)
    cf[1] = 100 * t1**4
    cf[3] = 100 * t2**4
    cf[5] = 80 * (t1**3 * t2)
    cf[7] = 80 * (t1 * t2**3)
    cf[9] = t1
    cf[11] = -t2
    cf[13] = 5 * (t1**2 * t2**2)
    cf[17] = -0.5 * t1**5
    cf[18] = -0.5 * t2**5
    cf[22] = 2.3 * (t1**2 - t2**2)
    cf[24] = 10 * (t1**3 - t2**3)
    return _asc(cf)


def o_giga_14(t1, t2):
    cf = np.zeros(25, dtype=complex)
    cf[0] = 200 * (t1**3 * t2**2)
    cf[4] = 200 * (t1**2 * t2**3)
    cf[6] = 50 * t1**4
    cf[8] = 50 * t2**4
    cf[10] = -100 * t1**3
    cf[12] = -100 * t2**3
    cf[14] = 10 * (t1**2 - t2**2)
    cf[16] = 20 * (t1 - t2)
    cf[18] = 0.1 * t1**5
    cf[20] = 0.1 * t2**5
    cf[22] = 0.05 * (t1 * t2)
    cf[24] = -10
    return _asc(cf)


def o_giga_15(t1, t2):
    cf = np.zeros(18, dtype=complex)
    cf[1] = 200 * (t1**5 + t2**5)
    cf[3] = 100 * (t1**4 - t2**4)
    cf[5] = 80 * t1**6
    cf[7] = 80 * t2**6
    cf[9] = 2 * t1
    cf[11] = -2 * t2
    cf[13] = 5 * (t1**3 * t2**3)
    cf[17] = 5
    return _asc(cf)


def o_giga_16(t1, t2):
    cf = np.zeros(22, dtype=complex)
    cf[12] = 250 * (t1**5 - t2**3)
    cf[17] = 200 * (t1**4 * t2**4)
    cf[8] = 80 * (t1**2 * t2 - t2**2)
    cf[6] = -60 * t1**3
    cf[4] = 40 * t2**3
    cf[2] = 15 * (t1 - 0.5 * t2)
    cf[3] = -20 * (t1 * t2)
    cf[1] = 5 * t2
    cf[0] = -10
    cf[21] = -30 * (t1**6 + t2)
    return _asc(cf)


def o_giga_22(t1, t2):
    cf = np.zeros(26, dtype=complex)
    cf[0] = 100
    cf[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
    cf[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
    cf[16] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
    cf[20] = -10
    cf[24] = 0.2j
    return _asc(cf)


def o_giga_23(t1, t2):
    cf = np.zeros(26, dtype=complex)
    cf[[0, 4, 12, 19, 20, 24]] = [1, 4, 4, -9, -1.9, 0.2]
    cf[6] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
    cf[8] = 100 * t1**3 + 100 * t1**2 + 100 * t2 - 100
    cf[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
    return _asc(cf)


def o_giga_24(t1, t2):
    cf = np.zeros(26, dtype=complex)
    cf[[0, 4, 12, 19, 20, 24]] = [1, 4, 4, -9, -1.9, 0.2]
    cf[6] = 100j * t2**3 + 100j * t2**2 - 100 * t2 - 100
    cf[8] = 100j * t1**3 + 100j * t1**2 + 100 * t2 - 100
    cf[14] = 100j * t2**3 - 100j * t2**2 + 100 * t2 - 100
    return _asc(cf)


def o_giga_26(t1, t2):
    cf = np.zeros(26, dtype=complex)
    cf[0] = 100
    cf[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
    cf[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
    cf[16] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
    cf[20] = -10
    cf[24] = 0.2
    return _asc(cf)


def o_giga_28(t1, t2):
    n = 6
    cf = np.zeros(n, dtype=complex)
    cf[0] = 100 * t2**3 + 100j * t1**3
    cf[n // 2 - 1] = 150
    cf[n - 1] = 40j
    return cf          # python era: descending as written


def o_giga_29(t1, t2):
    n = 10
    cf = np.zeros(n, dtype=complex)
    cf[0] = 150 * t2**3 - 150j * t1**2
    cf[n // 2 - 1] = 100 * (t1 - t2)
    cf[n - 1] = 10j
    return cf


def o_giga_30(t1, t2):
    n = 10
    cf = np.zeros(n, dtype=complex)
    cf[0] = 150j * t2**2 + 100 * t1**3
    cf[n // 2 - 1] = 150 * np.abs(t1 + t2 - 2.5 * (1j + 1))
    cf[n - 1] = 100j * t1**3 + 150 * t2**2
    return cf


ORACLES = {
    "giga_1": o_giga_1, "giga_2": o_giga_2, "giga_3": o_giga_3,
    "giga_4": o_giga_4, "giga_5": o_giga_5, "giga_6": o_giga_6,
    "giga_12": o_giga_12, "giga_13": o_giga_13, "giga_14": o_giga_14,
    "giga_15": o_giga_15, "giga_16": o_giga_16, "giga_22": o_giga_22,
    "giga_23": o_giga_23, "giga_24": o_giga_24, "giga_25": o_giga_22,
    "giga_26": o_giga_26, "giga_28": o_giga_28, "giga_29": o_giga_29,
    "giga_30": o_giga_30,
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
        [SWEEP_TEST, "/tmp/giga_early_t1_row.bin"],
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
             os.path.join(ROOT, "scripts", "gen_giga_early_t1_coeff_programs.py"),
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

    def test_layout_conventions(self):
        generator = _load_generator()
        # rev(cf) R scripts are descending-direct; plain R scripts get
        # the explicit reverse; python-era defs are descending-direct
        self.assertEqual(generator.GIGAS["giga_1"]["layout"], "desc")
        self.assertEqual(generator.GIGAS["giga_2"]["layout"], "desc")
        for name in ("giga_3", "giga_22", "giga_26"):
            self.assertIn("poly = rev(poly)", generator.build_source_text(name))
        for name in ("giga_28", "giga_29", "giga_30"):
            self.assertNotIn("poly = rev(poly)", generator.build_source_text(name))

    def test_giga_25_is_the_giga_22_duplicate(self):
        # reference giga_25.png is byte-identical to giga_22.png; the
        # programs must match up to the name
        generator = _load_generator()
        self.assertEqual(generator.build_source_text("giga_25"),
                         generator.build_source_text("giga_22"))


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
            u, v = PROBES[0]
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
                self.assertLess(max(diffs) if diffs else 0.0, 1e-8, name)


if __name__ == "__main__":
    unittest.main()
