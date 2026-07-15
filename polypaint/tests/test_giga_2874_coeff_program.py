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

# Collapse-violating rows from the seeded 20k sweep (np.argsort(|cf|)[0] != 0):
# the jump's negative deltas put these in the regime where verbatim andy1's
# cumprod gather — including numpy's int64 wraparound — is load-bearing.
VIOLATED_PROBES = (
    (0.2854043754229652, 0.2977107374293466),
    (0.26082604536214715, 0.2633253997276071),
)


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2874_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2874_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _jumped(t1, t2):
    """xfrm.jump at 500685f, verbatim: strict > against the level array."""
    import numpy as np

    levels = np.array([0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9])
    jmp = -0.05
    return (t1 + np.sum(levels > t1) * jmp, t2 + np.sum(levels > t2) * jmp)


def _row_cf(generator, t1, t2):
    """letters.square + np.poly at 500685f with the jumped uniforms
    (scale(t, 0.9) = (1-0.9)/2 + 0.9*t, factor = 1)."""
    import numpy as np

    roots = np.array(generator.letter_roots(), dtype=np.complex128)
    jt1, jt2 = _jumped(t1, t2)
    def scale(t, s):
        return ((1 - s) / 2 + s * t) * 1
    return np.poly(roots + complex(scale(jt1, 0.9), scale(jt2, 0.9))).astype(complex)


def _snapshot_row(generator, t1, t2):
    """zfrm.andy1 at 500685f, verbatim: cf[cumprod(argsort)%n] - cf[cumsum..].
    np.argsort yields int64, so the cumprod WRAPS; that wrapped value is part
    of the reference's actual output and is replicated by the program's
    32-bit-limb emulation."""
    import numpy as np

    cf = _row_cf(generator, t1, t2)
    s = np.argsort(np.abs(cf))
    csi = np.cumprod(s) % len(cf)
    cpi = np.cumsum(s) % len(cf)
    return cf[csi] - cf[cpi]


def _limb_csi_reference(s_vals, n):
    """Pure-double replica of the program's limb scan + decode maps."""
    import numpy as np

    two32 = 4294967296.0
    r32 = two32 - n * np.floor(two32 / n)
    r64 = r32 * r32 - n * np.floor(r32 * r32 / n)
    lo, hi = float(s_vals[0]), 0.0
    out = []
    for k, sv in enumerate(s_vals):
        sv = float(sv)
        if k > 0:
            product = lo * sv
            carry = np.floor(product / two32)
            lo = product - two32 * carry
            high = hi * sv + carry
            hi = high - two32 * np.floor(high / two32)
        u = (lo - n * np.floor(lo / n)) + r32 * (hi - n * np.floor(hi / n))
        val = u - r64 * np.floor(hi / 2147483648.0) + 2 * n
        out.append(int(val - n * np.floor(val / n)))
    return out


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
        [SWEEP_TEST, "/tmp/giga2874_test_row.bin"],
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


class TestGiga2874CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2874")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.letter_roots()), 36)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 157)
        self.assertEqual(compiled["stack_max"], 11)
        self.assertEqual(compiled["vector_constant_count"], 2)
        self.assertEqual(
            [v["length"] for v in compiled["vector_constants"]], [9, 37]
        )
        # Every chip arg fits the 256-char wire cap (the limb/decode
        # expressions are deliberately close to it).
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

    def test_collapse_regime_is_violated_so_full_andy1_is_required(self):
        """The giga_2872/2873 shortcut (leading gather == the monic 1) does
        NOT hold under jump: seeded rows land with argsort(|cf|)[0] != 0.
        This is the reason giga_2874 carries both gathers."""
        import numpy as np

        generator = _load_generator()
        violations = 0
        rng = np.random.default_rng(2874)
        for _ in range(5000):
            t1, t2 = rng.random(), rng.random()
            cf = _row_cf(generator, t1, t2)
            if np.argsort(np.abs(cf))[0] != 0:
                violations += 1
        self.assertGreater(violations, 0)
        for t1, t2 in VIOLATED_PROBES:
            cf = _row_cf(generator, t1, t2)
            self.assertNotEqual(int(np.argsort(np.abs(cf))[0]), 0)

    def test_limb_emulation_matches_numpy_int64_semantics(self):
        """The 32-bit-limb cumprod + signed-residue decode is integer-exact
        against numpy's wrapping int64 cumprod for every row tried."""
        import numpy as np

        generator = _load_generator()
        rng = np.random.default_rng(1874)
        for trial in range(2000):
            t1, t2 = rng.random(), rng.random()
            cf = _row_cf(generator, t1, t2)
            s = np.argsort(np.abs(cf))
            expected = (np.cumprod(s) % len(cf)).tolist()
            with self.subTest(trial=trial):
                self.assertEqual(
                    _limb_csi_reference(s.astype(float), float(len(cf))),
                    expected,
                )

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim jump+andy1. Generic rows measured ~5.8e-9
        worst (translate-vs-np.poly expansion dust); acceptance 1e-7.
        (0.3, 0.7) sits exactly ON level values — parity there pins the
        strict-> comparison semantics (counts 6 and 2)."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in ((0.137, 0.823), (0.75, 0.31), (0.0, 0.999), (0.5, 0.5),
                       (0.3, 0.7)):
            actual = _run_vm(compiled, t1, t2)
            expected = _snapshot_row(generator, t1, t2)
            self.assertEqual(len(actual), 37)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-7, (t1, t2, float(rel.max())))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_matches_on_collapse_violating_rows(self):
        """Rows where the cumprod gather is real (and wrapped): the gather
        indices are integer-exact (all 51 seeded violated rows verified),
        so the residual is pure cancellation amplification in
        cf[csi] - cf[cpi]. Measured worst 1.9e-4; acceptance 1e-3."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in VIOLATED_PROBES:
            actual = _run_vm(compiled, t1, t2)
            expected = _snapshot_row(generator, t1, t2)
            self.assertEqual(len(actual), 37)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-3, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
