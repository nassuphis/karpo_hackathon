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

# rows measured clean of dust domination (full-chain rel <= 3e-2); the
# park's own exp/log dust makes SOME rows oracle-unpinnable by design —
# see the module docstring of the generator.
FULL_PROBES = ((0.137, 0.823), (0.75, 0.31), (0.031, 0.977), (0.5, 0.5),
               (0.2012, 0.7958), (0.3384, 0.3772))
STRUCT_PROBES = FULL_PROBES + ((0.9684, 0.7187), (0.9, 0.1))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_outflow_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_outflow_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _c_div_full(a, b):
    """The VM's scaled-naive complex division, replicated."""
    br, bi = b.real, b.imag
    scale = max(abs(br), abs(bi))
    if scale == 0.0:
        return 0j
    sr, si = br / scale, bi / scale
    d = sr * sr + si * si
    nar, nai = a.real / scale, a.imag / scale
    return complex((nar * sr + nai * si) / d, (nai * sr - nar * si) / d)


def _parked(t1, t2):
    import numpy as np

    z1 = np.exp(1j * 2 * np.pi * t1)
    z2 = np.exp(1j * 2 * np.pi * t2)
    c1, c2 = (z1 * z1) * z1, (z2 * z2) * z2
    return _c_div_full(c1 + 1j, c1 - 1j), _c_div_full(c2 + 1j, c2 - 1j)


def _mypow(base, e):
    import numpy as np

    if base == 0.0:
        return 1.0 if e == 0 else 0.0
    return np.exp(e * np.log(complex(base)))


def _oracle_full(t1, t2):
    import numpy as np

    t1c, t2c = _parked(t1, t2)
    r1 = t1c.real
    i2 = t2c.imag
    cf = np.zeros(35, dtype=complex)
    for k in range(35):
        f12 = (_mypow(r1, k + 1) * np.sin((k + 1) * np.angle(t2c))
               * _mypow(i2, 34 - k) * np.cos((k + 1) * np.abs(t1c)))
        f34 = (np.log(np.abs(t1c) + np.abs(t2c) + (k + 1))
               * ((r1 + (k + 1)) * (i2 + (k + 1)) * np.log(np.abs(t1c) + 1)))
        aa = (np.angle(t1c) * np.sin(k + 1) + np.angle(t2c) * np.cos(k + 1)
              + np.log(np.abs(t1c) + 1) / (k + 1))
        cf[k] = (f12 + f34) * np.exp(1j * aa)
    return cf[::-1]


def _oracle_structural(t1, t2):
    import numpy as np

    t1c, t2c = _parked(t1, t2)
    r1 = t1c.real
    i2 = t2c.imag
    out = np.zeros(35, dtype=complex)
    for k in range(35):
        f34 = (np.log(np.abs(t1c) + np.abs(t2c) + (k + 1))
               * ((r1 + (k + 1)) * (i2 + (k + 1)) * np.log(np.abs(t1c) + 1)))
        aa = (np.angle(t1c) * np.sin(k + 1) + np.angle(t2c) * np.cos(k + 1)
              + np.log(np.abs(t1c) + 1) / (k + 1))
        out[k] = f34 * np.exp(1j * aa)
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
        [SWEEP_TEST, "/tmp/giga_outflow_test_row.bin"],
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


class TestGigaOutflowCoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_outflow")
        self.assertEqual(stored["chain"], [])

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 95)
        self.assertEqual(compiled["stack_max"], 5)
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

    def test_mobius_maps_circle_to_imaginary_axis(self):
        """coeff6 sends the unit circle to the imaginary axis: re(t') is
        pure cancellation dust (< 1e-13 on every row) — the reference's
        part1 is BUILT from numpy's division rounding noise, which is why
        per-row full-chain parity is dust-limited by construction (the
        ensemble equivalence was measured at cloud level: corr 0.863 vs
        noise floor 0.712 over 15k rows)."""
        import numpy as np

        rng = np.random.default_rng(483)
        for _ in range(2000):
            t1c, t2c = _parked(rng.random(), rng.random())
            # near the poles |im| grows and the division amplifies the
            # cancellation dust with it — bound RELATIVE to the payload
            self.assertLess(abs(t1c.real), 1e-12 * (1 + abs(t1c.imag)))
            self.assertLess(abs(t2c.real), 1e-12 * (1 + abs(t2c.imag)))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_structural_stages_match_exactly(self):
        """The dust-free stages (park, angle phases, part3*part4, the whole
        stack discipline) against an exact-semantics oracle: measured
        8.9e-16 worst; acceptance 1e-9."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(
            generator.build_structural_probe_source_text())
        for t1, t2 in STRUCT_PROBES:
            actual = _run_vm(compiled, t1, t2)
            expected = _oracle_structural(t1, t2)
            self.assertEqual(len(actual), 35)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-9, (t1, t2, float(rel.max())))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_full_chain_on_dust_clean_rows(self):
        """Full chain vs the VM-division oracle on rows measured clean of
        dust domination (worst 3e-2 there — the residual IS the dust term
        at partially-sensitive slots); acceptance 1e-1. Structurally wrong
        programs fail at O(1) on every row."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in FULL_PROBES:
            actual = _run_vm(compiled, t1, t2)
            expected = _oracle_full(t1, t2)
            self.assertEqual(len(actual), 35)
            self.assertTrue(np.all(np.isfinite(actual)))
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-1, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
