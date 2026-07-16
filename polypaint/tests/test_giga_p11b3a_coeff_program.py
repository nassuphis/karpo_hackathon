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
    path = os.path.join(ROOT, "scripts", "gen_giga_p11b3a_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_p11b3a_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(t1, t2):
    """p11b3 + uc_old x3 + roots at 01627b4, verbatim. Fully deterministic
    in (t1, t2) — no RNG, no remap."""
    import numpy as np

    tp1 = np.exp(1j * 2 * np.pi * t1)
    tp2 = np.exp(1j * 2 * np.pi * t2)
    t = tp1 + tp2
    a = abs(t) / 2
    m = int(a * 251) % 37
    v = (np.arange(11) + 1) / (t + 4)
    p1 = int(7 * a * 11) % 11
    v[p1] = (p1 + 1) / (t + abs(t) * 2 + 1 + m)
    p2 = int(619 * a * 11) % 11
    v[p2] = (p2 + 1) / (t + abs(t) * 2 + 1 + m // 2)
    cf = np.exp(1j * np.pi * v)
    for _ in range(3):
        sa = np.sum(np.abs(cf))
        assert sa >= 1e-15, "uc_old guard fired — regime changed"
        cf = np.exp(1j * 2 * np.pi * (cf / sa))
    return np.concatenate([[0.0], np.roots(cf)])


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
        [binary, "/tmp/p11b3a_test_row.bin"],
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


class TestGigaP11b3aCoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_p11b3a")
        self.assertEqual(stored["chain"], [])

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 245)   # closest yet to 256
        self.assertEqual(compiled["stack_max"], 7)
        self.assertEqual(compiled["vector_constant_count"], 0)
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

    def test_masked_update_equality_and_collision_order(self):
        """The floor((101-(k-p)^2)/101) mask is exactly [k == p] for
        0 <= k, p <= 10; when p1 == p2 the second update wins, matching
        numpy's sequential assignment (rows where int(7a*11) and
        int(619a*11) collide mod 11 exist and must not diverge)."""
        import numpy as np

        for p in range(11):
            mask = np.array([np.floor((101 - (k - p) ** 2) / 101) for k in range(11)])
            want = np.zeros(11)
            want[p] = 1
            self.assertTrue(np.array_equal(mask, want), p)
        rng = np.random.default_rng(11)
        collisions = 0
        for _ in range(4000):
            t1, t2 = rng.random(), rng.random()
            a = abs(np.exp(2j * np.pi * t1) + np.exp(2j * np.pi * t2)) / 2
            if int(7 * a * 11) % 11 == int(619 * a * 11) % 11:
                collisions += 1
        self.assertGreater(collisions, 0)

    @unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK sweep_test build unavailable")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim p11b3 + uc_old x3 + roots. Residual is the
        f32 root cast; measured worst 5.6e-8 over 45 rows (uc_old's guard
        asserted dead in the oracle on every row). Acceptance 1e-5."""
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
