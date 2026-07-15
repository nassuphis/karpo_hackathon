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

# t = (0.5, 0.5) makes delta exactly 0, so rts = the raw symmetric glyph and
# distinct roots share magnitudes EXACTLY: the descending-|.| pairing has
# tie-order freedom there (as the reference's LAPACK order did in its own
# way). Pinned structurally instead of positionally.
TIE_ROW = (0.5, 0.5)


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2877_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2877_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _row(generator, t1, t2):
    """Verbatim 8e28adb chain (poly_letter with ro=io=-0.5, roots_p with
    andy=100) with the DOCUMENTED pairing — descending |root|, ties as
    reverse-of-ascending — in place of LAPACK's emission order, which is
    build-dependent and irreproducible (see recreate_giga_2877.md)."""
    import numpy as np

    pts = np.array(generator.letter_roots(), dtype=np.complex128)
    def scale(t, s):
        return ((1 - s) / 2 + s * t) * 1
    rts = (pts + complex(scale(t1, 0.9), scale(t2, 0.9))) + complex(-0.5, -0.5)
    cf = np.poly(rts).astype(complex)
    desc = rts[np.argsort(np.abs(rts))][::-1]
    return rts, cf, 100.0 * np.append(desc, 1) + cf


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
        [SWEEP_TEST, "/tmp/giga2877_test_row.bin"],
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


class TestGiga2877CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2877")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.letter_roots()), 32)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 60)
        self.assertEqual(compiled["stack_max"], 7)
        self.assertEqual(compiled["vector_constant_count"], 2)
        self.assertEqual(
            [v["length"] for v in compiled["vector_constants"]], [32, 33]
        )

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

    def test_probe_rows_are_tie_free(self):
        """The parity probes must not sit on exact magnitude ties, where the
        documented pairing (like the reference's own LAPACK order) has
        legitimate tie freedom. Only the fully symmetric delta=0 row is
        known to tie; it is pinned structurally instead."""
        import numpy as np

        generator = _load_generator()
        for t1, t2 in ((0.137, 0.823), (0.75, 0.31), (0.0, 0.999), (0.31, 0.77)):
            rts, _, _ = _row(generator, t1, t2)
            mags = np.abs(rts)
            self.assertEqual(len(np.unique(mags)), len(mags), (t1, t2))
        rts, _, _ = _row(generator, *TIE_ROW)
        mags = np.abs(rts)
        self.assertLess(len(np.unique(mags)), len(mags))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_documented_formula(self):
        """Native VM vs the documented chain (descending-|.| pairing).
        Measured worst 2.1e-9 over 44 tie-free rows; acceptance 1e-7."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in ((0.137, 0.823), (0.75, 0.31), (0.0, 0.999), (0.31, 0.77)):
            actual = _run_vm(compiled, t1, t2)
            _, _, expected = _row(generator, t1, t2)
            self.assertEqual(len(actual), 33)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-7, (t1, t2, float(rel.max())))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_tie_row_is_a_valid_descending_pairing(self):
        """At the delta=0 row the positional oracle is ambiguous, so pin the
        STRUCTURE: recovering r[k] = (q[k] - cf[k]) / 100 from the VM output
        must yield the exact root multiset in non-ascending |.| order, with
        the appended 1 in the last slot. cf is taken from the VM's own cf
        probe (build_cf_probe_source_text) so translate-vs-np.poly dust —
        which |cf[32]| ~ 4e9 amplifies to ~4e-4 here — cancels exactly."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        actual = _run_vm(compiled, *TIE_ROW)
        cf_probe = compile_coeff_program_source(generator.build_cf_probe_source_text())
        cf = _run_vm(cf_probe, *TIE_ROW)
        rts, _, _ = _row(generator, *TIE_ROW)
        recovered = (actual - cf) / 100.0
        self.assertAlmostEqual(abs(recovered[-1] - 1.0), 0.0, delta=1e-9)
        r = recovered[:-1]
        mags = np.abs(r)
        self.assertTrue(np.all(np.diff(mags) <= 1e-9))
        # multiset match against the true shifted points
        d = np.abs(r[:, None] - rts[None, :])
        match = d.argmin(axis=1)
        self.assertEqual(len(set(match.tolist())), len(rts))
        self.assertLess(float(d.min(axis=1).max()), 1e-9)


if __name__ == "__main__":
    unittest.main()
