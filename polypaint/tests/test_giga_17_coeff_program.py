"""
giga_17 parity: uc,sum_prod -> poly_727 -> rev (+recursive_add EMA at
decay 5e-05, NOT ported: stateless-vs-EMA root moves are sub-pixel
dust and the reference's own EMA state is chunk-order noise under
procs=14). The 'safe' knife never fires (sum|cf| in [587, 654]).
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

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.9, 0.9), (0.05, 0.6),
          (0.6, 0.95))   # u+v > 1: exercises the angle(v2) wrap


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_17_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_17", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle(u, v):
    z1, z2 = np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)
    t1, t2 = z1 + z2, z1 * z2                       # sum_prod
    k = np.arange(1, 10)
    mag = np.log(np.abs(t1) + np.abs(t2) + k) * k**2
    ang = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k)
    cf = mag * (np.cos(ang) + 1j * np.sin(ang))
    return cf[::-1]                                 # zfrm rev


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
        [SWEEP_TEST, "/tmp/giga_17_row.bin"],
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


class TestGiga17(unittest.TestCase):
    def test_document_fresh_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable,
             os.path.join(ROOT, "scripts", "gen_giga_17_coeff_program.py"),
             "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_17.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_17")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or [])
                  if d.get("level") == "error"]
        self.assertFalse(errors)

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_oracle(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                ref = _oracle(u, v)
                got = _run_vm(compiled, u, v)
                self.assertEqual(len(got), 9)
                scale = float(np.abs(ref).max())
                rel = float(np.abs(got - ref).max() / scale)
                self.assertLess(rel, 1e-12, f"giga_17 coeff parity {rel}")


if __name__ == "__main__":
    unittest.main()
