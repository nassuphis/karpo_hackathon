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

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.031, 0.977), (0.5, 0.5),
          (0.999, 0.999), (0.9137, 0.412))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2864_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2864_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(t1, t2):
    """poly_creative10 under zz, verbatim (complex gp power + sign flips).
    Collapses to (-r)^(k+1) up to power-path dust; the wedge term is
    EXACTLY +0.0 in IEEE (x*y - y*x)."""
    import numpy as np

    z = t1 + 1j * t2
    a0, a1 = z.real, z.imag
    dot = a0 * a0 + a1 * a1
    wedge = a0 * a1 - a1 * a0
    assert wedge == 0.0
    gp = dot + 1j * wedge
    cf = np.array([gp ** (k + 1) for k in range(71)])
    cf[::2] *= -1
    return cf


def _run_vm(compiled, t1, t2):
    import numpy as np

    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [SWEEP_TEST, "/tmp/giga2864_test_row.bin"],
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
        capture_output=True, text=True, timeout=30,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:400])
    meta = json.loads(proc.stdout)
    return np.array(
        [complex(re, im) for re, im in meta["coeff"]["poly"]],
        dtype=np.complex128,
    )


class TestGiga2864CoeffProgram(unittest.TestCase):
    def test_generated_document_is_fresh_portable_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        expected = generator.build_payload()
        with open(generator.OUTPUT, "r", encoding="utf-8") as fh:
            stored = json.load(fh)
        self.assertEqual(stored, expected)
        self.assertEqual(
            set(stored),
            {"version", "program_kind", "name", "chain", "source_text"},
        )
        self.assertEqual(stored["program_kind"], "coeff_program")
        self.assertEqual(stored["name"], "giga_2864")
        self.assertEqual(stored["chain"], [])

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 3)   # shortest of the series
        self.assertEqual(compiled["stack_max"], 1)
        self.assertEqual(compiled["vector_constant_count"], 0)

    def test_round_trips_chain_to_source(self):
        import warnings

        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
        )

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            regenerated = coeff_source_text_from_chain(compiled["source_chain"])
        self.assertEqual([str(w.message) for w in caught], [])
        self.assertEqual(
            compile_coeff_program_source(regenerated)["fingerprint"],
            compiled["fingerprint"],
        )

    def test_formula_collapse_and_true_roots(self):
        """The redundancy: wedge(a, a) == +0.0 exactly, so the reference
        formula IS the geometric sequence (-r)^(k+1) — whose polynomial's
        TRUE roots are the circle of radius r with the point -r deleted
        (verified via the geometric-series identity on a well-conditioned
        row). The artwork is the eigensolve's deviation from that circle;
        Aberth would render the circle (the boring version)."""
        import numpy as np

        cf = _snapshot_row(0.9, 0.8)              # r ~ 1.45: well-conditioned
        r = 0.9 * 0.9 + 0.8 * 0.8
        angles = 2 * np.pi * np.arange(1, 71) / 71
        true_roots = -r * np.exp(1j * angles)
        vals = np.polyval(cf, true_roots)
        scale = np.abs(np.polyval(np.polyder(cf), true_roots)) * np.abs(true_roots)
        self.assertLess(float((np.abs(vals) / scale).max()), 1e-12)

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM (the one-scan geometric chain) vs the verbatim complex-
        power spelling: per-slot RELATIVE parity including the r^71 tinies.
        Measured worst 2.7e-15 over 45 rows; acceptance 1e-12."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in PROBES:
            actual = _run_vm(compiled, t1, t2)
            expected = _snapshot_row(t1, t2)
            self.assertEqual(len(actual), 71)
            rel = np.abs(actual - expected) / np.maximum(1e-300, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-12, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
