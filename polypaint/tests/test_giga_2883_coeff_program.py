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
          (0.9999, 0.0001), (0.25, 0.75))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2883_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2883_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _prng_signs(t1, t2):
    """The program's scan-PRNG, verbatim in the same IEEE double ops the VM
    executes: frac-cascade states, +-1 from the second binary digit."""
    import numpy as np

    x = t1 * 7919.7717 + t2 * 104729.31 + 0.5
    x = x - np.floor(x)
    out = np.empty(33)
    for k in range(33):
        x = x * 9821.4959 + 0.211327
        x = x - np.floor(x)
        out[k] = 1.0 - 2.0 * np.floor(2.0 * x)
    return out


def _snapshot_row(generator, t1, t2):
    """Verbatim chain (bkr -> combine(ltlwd, poly_letter) -> rev) with the
    documented PRNG replacing np.random.choice (measured ensemble-
    indistinguishable; see recreate_giga_2883.md)."""
    import numpy as np

    pts = np.array(generator.letter_roots(), dtype=np.complex128)
    def bkr1(t):
        xf = t % 1
        return (2 * xf) % 1 + 0.5j * np.floor(2 * xf)
    def scale(t, s):
        return ((1 - s) / 2 + s * t) * 1
    t1b, t2b = bkr1(t1), bkr1(t2)
    shift = (scale(t1b, 0.9)) + 1j * (scale(t2b, 0.9)) + (0.0 - 1.0j)
    letter = np.poly(pts + shift).astype(complex)
    cf = _prng_signs(t1, t2) * 0.75 + letter * 0.25
    return cf[::-1]


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
        [SWEEP_TEST, "/tmp/giga2883_test_row.bin"],
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


class TestGiga2883CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2883")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.letter_roots()), 32)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 104)
        self.assertEqual(compiled["stack_max"], 9)
        self.assertEqual(compiled["vector_constant_count"], 1)
        self.assertEqual(compiled["vector_constants"][0]["length"], 33)
        for chip in compiled["source_chain"]:
            if isinstance(chip, list):
                for arg in chip[1:]:
                    self.assertLessEqual(len(str(arg)), 256)

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

    def test_prng_sign_quality(self):
        """The scan-PRNG must look like ltlwd's iid +-1 draws: balanced,
        uncorrelated within rows, decorrelated across neighboring rows.
        (Ensemble indistinguishability was measured at cloud level: corr
        0.9271 vs noise floor 0.9087 over 24k rows.)"""
        import numpy as np

        rng = np.random.default_rng(2883)
        S = np.array([_prng_signs(rng.random(), rng.random())
                      for _ in range(4000)])
        self.assertLess(abs(float(S.mean())), 0.02)
        lag1 = float(np.mean(S[:, :-1] * S[:, 1:]))
        self.assertLess(abs(lag1), 0.03)
        # neighboring grid rows (1/7905 apart) must get unrelated signs
        agree = []
        for _ in range(500):
            t1, t2 = rng.random(), rng.random()
            a = _prng_signs(t1, t2)
            b = _prng_signs(t1 + 1.0 / 7905.0, t2)
            agree.append(float(np.mean(a == b)))
        self.assertLess(abs(float(np.mean(agree)) - 0.5), 0.05)

    def test_bkr_grid_structure(self):
        """The baker's map folds each uniform into (frac(2u), floor(2u)):
        four (b1, b2) quadrants shift the glyph by -0.45 in re (b2) and
        +0.45 in im (b1) — the 2x2 grid the image shows. Verify the delta
        decomposition against verbatim complex arithmetic."""
        import numpy as np

        generator = _load_generator()
        for t1, t2 in ((0.2, 0.3), (0.7, 0.3), (0.2, 0.8), (0.7, 0.8), (1.0, 0.5)):
            xf1, xf2 = t1 % 1, t2 % 1
            t1b = (2 * xf1) % 1 + 0.5j * np.floor(2 * xf1)
            t2b = (2 * xf2) % 1 + 0.5j * np.floor(2 * xf2)
            verbatim = (0.05 + 0.9 * t1b) + 1j * (0.05 + 0.9 * t2b) + (0 - 1j)
            f1 = 2 * (t1 - np.floor(t1)) - np.floor(2 * (t1 - np.floor(t1)))
            b1 = np.floor(2 * (t1 - np.floor(t1)))
            f2 = 2 * (t2 - np.floor(t2)) - np.floor(2 * (t2 - np.floor(t2)))
            b2 = np.floor(2 * (t2 - np.floor(t2)))
            mine = complex(0.05 + 0.9 * f1 - 0.45 * b2,
                           0.45 * b1 + (0.05 + 0.9 * f2) - 1)
            self.assertLess(abs(mine - verbatim), 1e-15, (t1, t2))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs the documented chain. The PRNG arithmetic is
        bit-identical (same IEEE ops), so the residual is translate dust:
        measured worst 2.6e-10 over 46 rows; acceptance 1e-7."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in PROBES:
            actual = _run_vm(compiled, t1, t2)
            expected = _snapshot_row(generator, t1, t2)
            self.assertEqual(len(actual), 33)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-7, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
