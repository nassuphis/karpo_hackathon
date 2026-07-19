"""
giga_259 parity: uc,coeff2 -> poly_373 -> rev, solve 'safe' — a fully
DETERMINISTIC exact port (no RNG remap): v1 = z1+z2, v2 = z1*z2 of the
two unit-circle points feed 35 trig/log-modulated slots, reversed.
poly_373's linspace block is dead code (omitted); the reference's
'safe' knife (sum|cf| outside (1e-10,1e10) -> dropped) never fires on
this sweep (measured [57, 83]) so no gate is ported; coefficients are
O(1..7) — no transport rescale.

Measured at authoring (5 probes): coefficient relative parity 1.5e-15,
root multiset vs oracle median 1.7e-15 / max 1.9e-14 — machine
precision (well-conditioned O(1) coefficients).
"""
import importlib.util
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.9, 0.9), (0.05, 0.6))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_259_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_259_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle_coeffs(u, v):
    import numpy as np

    z1, z2 = np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)
    v1, v2 = z1 + z2, np.exp(2j * np.pi * (u + v))
    j = np.arange(1, 36)
    mag = np.log(np.abs(v1) + j) * np.sin(j * np.pi / 7) + np.cos(j * np.pi / 11) * np.real(v2)
    ang = np.angle(v1) + np.angle(v2) * j + np.sin(j * np.pi / 13)
    cf = (mag + np.imag(v1) * np.cos(j * np.pi / 5)) * np.exp(1j * ang) \
        + np.conj(v2) * np.sin(j * np.pi / 17)
    return cf[::-1]


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
        [SWEEP_TEST, "/tmp/giga259_test_row.bin"],
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
    return [complex(re, im) for re, im in meta["coeff"]["poly"]]


class TestGiga259Document(unittest.TestCase):
    def test_generator_document_is_fresh_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_259_coeff_program.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_259.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_259")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)
        src = stored["source_text"]
        import re
        self.assertIsNone(re.search(r"(?<!expand_)roots(?:_cm|_jt|_ae)?\(", src))
        self.assertNotIn("multiply(pop", src)
        self.assertIn("poly = rev(poly)", src)


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGiga259VmParity(unittest.TestCase):
    def test_vm_rows_match_oracle_at_machine_precision(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                got = np.array(_run_vm(compiled, u, v))
                ref = _oracle_coeffs(u, v)
                self.assertEqual(len(got), 35)
                rel = float(np.abs(got - ref).max() / np.abs(ref).max())
                self.assertLess(rel, 1e-12)
                pool = list(np.roots(ref))
                diffs = []
                for z in np.roots(got):
                    j = int(np.argmin([abs(z - p) for p in pool]))
                    diffs.append(abs(z - pool.pop(j)))
                self.assertLess(max(diffs), 1e-10)


if __name__ == "__main__":
    unittest.main()
