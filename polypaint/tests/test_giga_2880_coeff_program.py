import importlib.util
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
# roots_cm needs a LAPACK-linked binary. On macOS, bind Accelerate's modern
# $NEWLAPACK zgeev (numpy links the same one; the legacy CLAPACK-3.2 symbol
# emits eigenvalues in a DIFFERENT QR deflation order). The deployed netlib
# LAPACK 3.10 lineage is covered by scripts/test-roots-cm-strip-docker.sh.
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

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.031, 0.977), (0.9137, 0.412),
          (0.5192, 0.2652), (0.2364, 0.9452))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2880_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2880_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(generator, t1, t2):
    """Verbatim 8e28adb chain with the counter remapped onto the uniforms:
    m = floor(71*t1) mod 71 / jitter = frac(71*t1) (and 101 for t2), then
    poly -> sort_moduli_keep_angles_p(0.1) -> invuc -> normalize ->
    roots_p(0.1) with np.roots (float64, LAPACK order, exact-zero strip)."""
    import numpy as np

    pts = np.array(generator.letter_roots(), dtype=np.complex128)
    m = np.floor(71 * t1) % 71
    j1 = 71 * t1 - np.floor(71 * t1)
    n = np.floor(101 * t2) % 101
    j2 = 101 * t2 - np.floor(101 * t2)
    rts = pts + complex(0.05 + 0.09 * j1, 0.05 + 0.09 * j2) \
        + (m / 5) * np.exp(1j * 2 * np.pi * (n / 3))
    cf = np.poly(rts).astype(complex)
    cf = (np.sort(np.abs(cf)) * np.exp(1j * np.angle(cf))) * 0.1 + cf
    sa = np.max(np.abs(cf))
    cf = cf / np.exp(1j * 2 * np.pi * (cf / sa))
    cf = cf / np.max(np.abs(cf))
    return 0.1 * np.append(np.roots(cf), 1) + cf


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
        [binary, "/tmp/giga2880_test_row.bin"],
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


class TestGiga2880CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2880")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.letter_roots()), 32)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 125)
        self.assertEqual(compiled["stack_max"], 8)
        self.assertEqual(compiled["vector_constant_count"], 1)
        self.assertEqual(compiled["vector_constants"][0]["length"], 33)

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

    def test_counter_remap_matches_reference_distribution(self):
        """The reference's global counter walks a 7171-cycle of offsets (71
        radii x 101 phase steps, 3 distinct angles) independent of the
        uniforms. The remap m = floor(71*t1) mod 71 + jitter = frac(71*t1)
        has the identical joint law; check the offset marginals exactly."""
        import numpy as np

        # reference marginals over one full CRT cycle
        idx = np.arange(7171)
        ref_radii = np.sort(np.unique((idx % 71) / 5))
        ref_angle_counts = np.bincount((idx % 101) % 3, minlength=3)
        self.assertEqual(len(ref_radii), 71)
        self.assertEqual(ref_angle_counts.tolist(), [2414, 2414, 2343])
        # remap marginals: m uniform over 0..70, n over 0..100 -> mod 3
        n_counts = np.bincount(np.arange(101) % 3, minlength=3)
        self.assertEqual((n_counts * 71).tolist(), ref_angle_counts.tolist())

    def test_strip_semantics_are_load_bearing(self):
        """Mean ~16 leading slots of the transformed cf sit below the legacy
        relative strip threshold (max|cf|^2 * 1e-15 on squared magnitudes) —
        with a nonzero minimum, so strip=exact keeps ALL of them and their
        giant roots. This is why the mode exists."""
        import numpy as np

        generator = _load_generator()
        rng = np.random.default_rng(2880)
        counts = []
        for _ in range(300):
            t1, t2 = rng.random(), rng.random()
            q = _snapshot_row(generator, t1, t2)  # exercises the full chain
            # re-derive cf4 (chain minus roots_p) for the strip census
            pts = np.array(generator.letter_roots(), dtype=np.complex128)
            m = np.floor(71*t1) % 71; j1 = 71*t1 - np.floor(71*t1)
            n = np.floor(101*t2) % 101; j2 = 101*t2 - np.floor(101*t2)
            rts = pts + complex(0.05+0.09*j1, 0.05+0.09*j2) \
                + (m/5)*np.exp(1j*2*np.pi*(n/3))
            cf = np.poly(rts).astype(complex)
            cf = (np.sort(np.abs(cf)) * np.exp(1j*np.angle(cf))) * 0.1 + cf
            sa = np.max(np.abs(cf)); cf = cf / np.exp(1j*2*np.pi*(cf/sa))
            cf = cf / np.max(np.abs(cf))
            mag2 = np.abs(cf) ** 2
            below = mag2 < mag2.max() * 1e-15
            counts.append(int(below.sum()))
            self.assertGreater(float(np.abs(cf[below]).min()) if below.any() else 1.0, 0.0)
            self.assertEqual(len(q), 33)
        self.assertGreater(float(np.mean(counts)), 5.0)

    @unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK sweep_test build unavailable")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM (Accelerate $NEWLAPACK zgeev — numpy's interface) vs the
        verbatim chain. The residual is the roots_cm float32 root cast;
        measured worst 2.6e-7 over 30 rows. Acceptance 1e-5. Order-positional:
        a zgeev emission-order mismatch would show as O(1)."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in PROBES:
            actual = _run_vm(compiled, t1, t2, SWEEP_TEST_LAPACK)
            expected = _snapshot_row(generator, t1, t2)
            self.assertEqual(len(actual), 33)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-5, (t1, t2, float(rel.max())))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_plain_build_reports_companion_unavailable(self):
        """The predeploy sweep_test builds without LAPACK; roots_cm must fail
        loudly there (not silently zero) so a mis-built deploy is caught."""
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        with self.assertRaises(AssertionError) as ctx:
            _run_vm(compiled, 0.137, 0.823, SWEEP_TEST)
        self.assertIn("unavailable", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
