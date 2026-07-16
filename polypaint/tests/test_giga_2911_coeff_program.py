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
          (0.0, 0.999), (0.9137, 0.412))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2911_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2911_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(t1, t2):
    """p11b2_v2 + cumsum + rev + toline_q(andy=1) at a9393c5, verbatim.
    Fully deterministic in (t1, t2) — no remap of any kind."""
    import numpy as np

    n = 11
    v = np.linspace(0, 1, n)
    tp1 = np.exp(1j * 2 * np.pi * t1)
    tp2 = np.exp(1j * 2 * np.pi * t2)
    denom = tp1 + tp2 + 3
    if abs(denom) < 1:
        denom = denom / abs(denom)
    u = 7 * n * np.power(v, 15) / denom
    uc = np.exp(1j * np.pi * u)
    sf = (np.arange(n) + 1) % (int(4583 * abs(tp1 + tp2)) % 11 + 1)
    cf = (sf + 1) * uc
    cf = np.cumsum(cf)
    cf = cf[::-1]
    rts = np.roots(cf)
    cay = 1j * (1 + rts) / (1 - rts)
    return np.poly(cay)


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
        [binary, "/tmp/giga2911_test_row.bin"],
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


class TestGiga2911CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2911")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.power_vector()), 11)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 83)
        self.assertEqual(compiled["stack_max"], 6)
        self.assertEqual(compiled["vector_constant_count"], 1)
        self.assertEqual(compiled["vector_constants"][0]["length"], 11)

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

    def test_determinism_and_regimes(self):
        """No RNG anywhere: the chain is a pure function of (t1, t2).
        The normalize branch is DEAD (|t1'+t2'+3| >= 1 always, since
        |t1'+t2'| <= 2), and the integer regime m spans 1..11."""
        import numpy as np

        seen_m = set()
        rng = np.random.default_rng(2911)
        for _ in range(3000):
            t1, t2 = rng.random(), rng.random()
            tp1 = np.exp(1j * 2 * np.pi * t1)
            tp2 = np.exp(1j * 2 * np.pi * t2)
            self.assertGreaterEqual(abs(tp1 + tp2 + 3), 1.0)
            seen_m.add(int(4583 * abs(tp1 + tp2)) % 11 + 1)
        self.assertEqual(seen_m, set(range(1, 12)))

    @unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK sweep_test build unavailable")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim p11b2_v2 chain. Residual is roots_cm's f32
        root cast through the Cayley map (poles near r = 1 amplify);
        measured worst 1.9e-7 over 45 rows. Acceptance 1e-5."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in PROBES:
            actual = _run_vm(compiled, t1, t2, SWEEP_TEST_LAPACK)
            expected = _snapshot_row(t1, t2)
            self.assertEqual(len(actual), 11)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-5, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
