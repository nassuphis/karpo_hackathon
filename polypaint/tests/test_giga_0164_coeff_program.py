import importlib.util
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
SWEEP_TEST_LAPACK = "/tmp/polypaint_sweep_test_lapack"


def _ensure_lapack_binary():
    if sys.platform != "darwin":
        return False
    src = os.path.join(LAMBDA_DIR, "sweep_cli.c")
    hdr = os.path.join(LAMBDA_DIR, "companion_solver.h")
    if os.path.exists(SWEEP_TEST_LAPACK):
        built = os.path.getmtime(SWEEP_TEST_LAPACK)
        if built >= os.path.getmtime(src) and built >= os.path.getmtime(hdr):
            return True
    proc = subprocess.run(
        ["cc", "-O2", "-pthread", "-DHAVE_LAPACK_COMPANION",
         "-DPOLYPAINT_ACCELERATE_NEWLAPACK",
         "-o", SWEEP_TEST_LAPACK, src, "-framework", "Accelerate", "-lm"],
        capture_output=True, text=True,
    )
    return proc.returncode == 0


HAVE_LAPACK_BINARY = _ensure_lapack_binary()
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.031, 0.977), (0.5, 0.5),
          (0.9, 0.1), (0.9137, 0.412))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_0164_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_0164_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _chain_cf(t1, t2):
    """poly_445 -> p8 -> invp3, verbatim from the run's own polyfun.txt."""
    import numpy as np

    t1c = t1 + 1j * t2
    t2c = t2 + 1j * t1
    cf = np.zeros(35, dtype=complex)
    for j in range(1, 36):
        k = (j * 5 + 2) % 12 + 1
        r = t1c.real * np.sin(j * np.pi / k) + t2c.real * np.cos(j * np.pi / (k + 1))
        i_ = t1c.imag * np.cos(j * np.pi / k) - t2c.imag * np.sin(j * np.pi / (k + 1))
        mag = np.log(abs(t1c) + j) * abs(np.sin(j * np.pi / 10))
        ang = np.angle(t1c) * np.cos(j * np.pi / 8) + np.angle(t2c) * np.sin(j * np.pi / 9)
        cf[j - 1] = mag * (r + 1j * i_) * (np.cos(ang) + 1j * np.sin(ang))
    cf = (cf**8 + cf**7 + cf**6 + cf**5 + cf**4 + cf**3 + cf**2 + cf + 1) \
        * np.arange(1, 36, dtype=complex)
    c1 = cf**3 + cf**2 + cf + 1
    return np.where(np.abs(c1) > 1, 1 / c1, np.ones(35, dtype=complex))


def _snapshot_row(t1, t2):
    """Plus the roots feedback (guard never fires) with the hi pad the
    program uses: the leading zero strips in the solver, reproducing the
    reference's degree-33 solve of the 34-root vector."""
    import numpy as np

    return np.concatenate([[0.0], np.roots(_chain_cf(t1, t2))])


def _run_vm(compiled, t1, t2, binary):
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
        [binary, "/tmp/giga0164_test_row.bin"],
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
        capture_output=True, text=True, timeout=60,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:400])
    meta = json.loads(proc.stdout)
    return np.array(
        [complex(re, im) for re, im in meta["coeff"]["poly"]],
        dtype=np.complex128,
    )


class TestGiga0164CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_0164")
        self.assertEqual(stored["chain"], [])

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 93)
        self.assertEqual(compiled["stack_max"], 6)
        self.assertEqual(compiled["vector_constant_count"], 7)
        self.assertTrue(
            all(v["length"] == 35 for v in compiled["vector_constants"]))
        # power_series, NOT power: the bare name lowers to the typed
        # elementwise z^8 and produces an entirely different artwork
        chain_names = [ch[1] for ch in compiled["source_chain"]
                       if isinstance(ch, list) and ch[0] == "_native_transform"]
        self.assertIn("power", chain_names)      # registry name of the series
        self.assertIn("invpower", chain_names)

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

    def test_roots_guard_never_fires_and_magnitudes(self):
        """zfrm.roots' sum<1e-10 passthrough is DEAD on this chain: the
        invp3 output always carries at least one 0.2..1.0-magnitude slot.
        Leading slots reach ~1e-25 — the exact-strip mode is load-bearing
        (the legacy relative strip would delete them and their giant
        roots)."""
        import numpy as np

        rng = np.random.default_rng(164)
        min_lead = 1.0
        for _ in range(2000):
            cf = _chain_cf(rng.random(), rng.random())
            self.assertGreaterEqual(float(np.sum(np.abs(cf))), 1e-10)
            self.assertGreaterEqual(float(np.abs(cf).max()), 0.1)
            min_lead = min(min_lead, float(np.abs(cf).min()))
        self.assertLess(min_lead, 1e-15)

    @unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK sweep_test build unavailable")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs the run's own inlined source. Residual: power-sum
        association dust (ascending C loop vs descending np spelling)
        amplified through invpower and the eigensolve, plus the f32 root
        cast; measured worst 4.1e-8 over 45 rows. Acceptance 1e-5."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in PROBES:
            actual = _run_vm(compiled, t1, t2, SWEEP_TEST_LAPACK)
            expected = _snapshot_row(t1, t2)
            self.assertEqual(len(actual), 35)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-5, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
