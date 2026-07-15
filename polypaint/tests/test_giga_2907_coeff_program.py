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
          (0.0, 0.999), (0.9137, 0.412))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2907_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2907_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(generator, t1, t2):
    """poly_chess4 at 8d554a2 + zfrm sort_abs_p, rev — verbatim, with the
    two t-independent randoms remapped: t := t1 (np.random.rand),
    bimodal_skewed's u := t2."""
    import numpy as np

    idx = np.arange(8) - 3.5
    par = (np.indices((8, 8)).sum(axis=0)) % 2 != 0
    X, Y = np.meshgrid(idx, idx)
    t = t1
    to = 0.5 * np.exp(1j * 2 * np.pi * t)
    ts = (t * 1.0 + 0.125) * 4
    e = int(np.floor(ts))
    frac = ts - e
    vv = np.exp(1j * 2 * np.pi * np.arange(4) / 4)
    ti = 0.2 * ((1 - frac) * vv[e % 4] + frac * vv[(e + 1) % 4])
    cfi = np.poly(((X[par] + ti) + 1j * (Y[par] + ti)).flatten())
    cfo = np.poly(((X[par] + to) + 1j * (Y[par] + to)).flatten())
    u = t2
    ex = 1 / (1 - 0.00001)
    a = (2 * u) ** ex / 2 if u < 0.5 else 1 - (2 * (1 - u)) ** ex / 2
    a = min(1.0, max(0.0, a))
    cf = cfo * a + cfi * (1 - a)
    cf = cf[np.argsort(np.abs(cf))] * 0.01 + cf
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
        [SWEEP_TEST, "/tmp/giga2907_test_row.bin"],
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


class TestGiga2907CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2907")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.chess_points()), 32)
        self.assertEqual(len(generator.square_vertices()), 4)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 119)
        self.assertEqual(compiled["stack_max"], 6)
        # the two roots_literal spellings byte-dedup to ONE pool constant
        self.assertEqual(compiled["vector_constant_count"], 2)
        self.assertEqual(
            [v["length"] for v in compiled["vector_constants"]], [7, 33]
        )
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

    def test_square_path_and_mask_structure(self):
        """The vertex pool carries numpy's exact exp(2j*pi*k/4) doubles
        (dust included), the mask is the 32-point checkerboard, and the
        edge index wraps 4 -> 0 at the path's end (t1 near 1: t_scaled in
        [4.0, 4.5) selects edge 0 again)."""
        import numpy as np

        generator = _load_generator()
        vv = np.array(generator.square_vertices())
        ref = np.exp(1j * 2 * np.pi * np.arange(4) / 4)
        self.assertTrue(np.array_equal(vv, ref))
        self.assertNotEqual(vv[1].real, 0.0)  # the dust IS the value
        pts = np.array(generator.chess_points())
        self.assertEqual(len(set(pts.tolist())), 32)
        # mask is INDEX-sum odd; coords are index-3.5, so coordinate sums
        # (i+j-7) are EVEN on the masked color
        self.assertTrue(np.all((pts.real + pts.imag) % 2 == 0))
        e_at_9 = int(np.floor((0.97 + 0.125) * 4)) % 4
        self.assertEqual(e_at_9, 0)

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim poly_chess4 + sort_abs_p + rev. The VM's
        bimodal primitive is bit-for-bit bimodal_skewed and blend matches
        cfo*a + cfi*(1-a) commutatively; measured worst 1.2e-10 over 45
        rows; acceptance 1e-7."""
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
