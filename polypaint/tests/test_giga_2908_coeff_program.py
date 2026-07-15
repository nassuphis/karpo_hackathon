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

PROBES = ((0.137, 0.323), (0.25, 0.31), (0.031, 0.477), (0.4, 0.45),
          (0.75, 0.81), (0.9, 0.2))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2908_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2908_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _prng_u(t1, t2):
    import numpy as np

    x = t1 * 7919.7717 + t2 * 104729.31 + 0.5
    x = x - np.floor(x)
    x = x * 9821.4959 + 0.211327
    return float(x - np.floor(x))


def _pre_rot_row(generator, t1, t2):
    """The chain verbatim at 2e061ed UP TO the solve stage (before the
    rot45 ramp), with the documented remaps: s = bkr(t1)+bkr(t2) via the
    xfrm chain, bimodal's uniform from the giga_2883 frac cascade.
    np.sign here is numpy 2.x's z/|z| — on visible (real-argument) rows
    both numpy semantics coincide at exactly +-1."""
    import numpy as np

    idx = np.arange(8) - 3.5
    par = (np.indices((8, 8)).sum(axis=0)) % 2 != 0
    X, Y = np.meshgrid(idx, idx)
    def spindle(t, a=0.5, b=0.2, p=1.5):
        theta = 2 * np.pi * t
        x = a * np.sign(np.cos(theta)) * np.abs(np.cos(theta)) ** (2 / p)
        y = b * np.sign(np.sin(theta)) * np.abs(np.sin(theta)) ** (2 / p)
        return x + 1j * y
    def bkr1(t):
        xf = t % 1
        return (2 * xf) % 1 + 0.5j * np.floor(2 * xf)
    s = bkr1(t1) + bkr1(t2)
    tt1 = s + s
    ti = 0.2 * spindle(0.0 * 1.0 + 0.1)
    to = 5.0 * spindle(tt1 * 1.0)
    cfi = np.poly(((X[par] + ti) + 1j * (Y[par] + ti)).flatten())
    cfo = np.poly(((X[par] + to) + 1j * (Y[par] + to)).flatten())
    u = _prng_u(t1, t2)
    ex = 1 / (1 - 0.001)
    a = (2 * u) ** ex / 2 if u < 0.5 else 1 - (2 * (1 - u)) ** ex / 2
    a = min(1.0, max(0.0, a))
    cf = cfo * a + cfi * (1 - a)
    cf = cf[np.argsort(np.abs(cf))] * 0.01 + cf
    return cf[::-1]


def _snapshot_row(generator, t1, t2):
    """Plus solve_rot45 baked as the coefficient phase ramp: q(z) =
    p(z*w^-1) with w = e^(-i pi/4) has exactly the rotated roots."""
    import numpy as np

    cf = _pre_rot_row(generator, t1, t2)
    k = np.arange(len(cf))
    return cf * np.exp(1j * 0.7853981633974483 * (32 - k))


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
        [SWEEP_TEST, "/tmp/giga2908_test_row.bin"],
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


class TestGiga2908CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2908")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.chess_points()), 32)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 154)
        self.assertEqual(compiled["stack_max"], 7)
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

    def test_composition_collapse_and_view_regime(self):
        """The xfrm/tt composition: tt2 = 0 exactly (inner delta constant).
        Complex-argument rows (any t >= 0.5) push the pre-rev constellation
        to ~1e5+, which the rev inversion maps to a SUB-PIXEL origin dot —
        so np.sign's numpy-version semantics cannot visibly affect the
        image (the dot moves at ~1e-6..1e-9 scales, pixel ~2e-5)."""
        import numpy as np

        generator = _load_generator()
        di = generator.inner_delta()
        self.assertLess(abs(di), 1.0)
        # visible-quadrant row: roots at sensible scales
        q_vis = _pre_rot_row(generator, 0.2, 0.3)
        self.assertLess(float(np.abs(np.roots(q_vis)).max()), 20.0)
        # complex-argument row: post-rev roots collapse to the origin dot
        q_far = _pre_rot_row(generator, 0.75, 0.81)
        self.assertLess(float(np.abs(np.roots(q_far)).max()), 1e-3)

    def test_rot45_ramp_rotates_the_roots(self):
        """The coefficient ramp exp(i pi/4 (32-k)) is solve_rot45 exactly:
        the ramped polynomial's roots are e^(-i pi/4) times the original's
        (nearest-match under 1e-8 on a well-conditioned visible row)."""
        import numpy as np

        generator = _load_generator()
        cf = _pre_rot_row(generator, 0.2, 0.3)
        q = _snapshot_row(generator, 0.2, 0.3)
        rot = np.exp(-1j * np.pi / 4)
        expected = np.roots(cf) * rot
        actual = np.roots(q)
        d = np.abs(actual[:, None] - expected[None, :])
        match = d.argmin(axis=1)
        self.assertEqual(len(set(match.tolist())), len(expected))
        # degree-32 eigensolve dust on both sides; measured ~3.6e-8
        self.assertLess(float(d.min(axis=1).max()), 1e-6)

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs the documented chain. Measured worst 1.3e-11 over
        46 rows (including far-flung complex-argument rows and the
        exp/log-vs-pow spelling of the 4/3 power); acceptance 1e-7."""
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
