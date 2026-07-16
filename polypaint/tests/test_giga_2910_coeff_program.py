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
K = 23


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2910_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2910_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _burnin_state(t1, t2, steps=K):
    """The program's burn-in chain in the same IEEE ops: PRNG drive pairs,
    the row's own uniforms as the final step, 11 independent component
    chains with unit-modulus renormalization."""
    import numpy as np

    kv = np.array([9, 100, 40, 9, 50, 9, 1, 4, 1, 40, 9], dtype=float)
    kvf = kv[::-1]
    T = np.zeros(steps, dtype=complex)
    xr = t1 * 7919.7717 + t2 * 104729.31 + 0.5
    xr -= np.floor(xr)
    xi = t1 * 104729.31 + t2 * 7919.7717 + 0.25
    xi -= np.floor(xi)
    T[0] = xr + 1j * xi
    for s in range(1, steps):
        pr, pi_ = T[s - 1].real, T[s - 1].imag
        nr = pr * 9821.4959 + 0.211327
        nr -= np.floor(nr)
        ni = pi_ * 9821.4959 + 0.531327
        ni -= np.floor(ni)
        T[s] = nr + 1j * ni
    T[steps - 1] = t1 + 1j * t2
    state = np.empty(11, dtype=complex)
    for j in range(11):
        cj = None
        for s in range(steps):
            z = kv[j] * T[s].real - 1j * kvf[j] * T[s].imag
            x = (1 + z) if s == 0 else (cj + z)
            cj = x / abs(x)
        state[j] = cj
    return state


def _snapshot_row(generator, t1, t2):
    """Burn-in state -> tohalf verbatim (theta = 0.25 Mobius, np.poly)."""
    import numpy as np

    a1, a2 = generator.mobius_constants()
    state = _burnin_state(t1, t2)
    rts = np.roots(state)
    return np.poly((rts - a1) / (a2 - rts))


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
        [binary, "/tmp/giga2910_test_row.bin"],
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


class TestGiga2910CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2910")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(
            generator.kabalistic_vector(),
            [9, 100, 40, 9, 50, 9, 1, 4, 1, 40, 9],
        )

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 47)
        self.assertEqual(compiled["stack_max"], 4)
        self.assertEqual(compiled["vector_constant_count"], 1)
        self.assertEqual(compiled["vector_constants"][0]["length"], 11)
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

    def test_chain_mixes_and_stays_on_torus(self):
        """The walk's drives (kv scaled) dwarf the unit state, so the chain
        forgets its initialization within a few steps: doubling the burn-in
        moves the state only at solver-dust scales. All components stay
        exactly unit-modulus (the per-step renormalization)."""
        import numpy as np

        for t1, t2 in ((0.31, 0.77), (0.05, 0.93)):
            s23 = _burnin_state(t1, t2, steps=K)
            self.assertTrue(np.allclose(np.abs(s23), 1.0, atol=1e-12))
            # init-forgetting: a chain started from a different init but the
            # same drives converges to the same state
            import numpy as np2
            kv = np.array([9, 100, 40, 9, 50, 9, 1, 4, 1, 40, 9], dtype=float)
            kvf = kv[::-1]
            state = -np.ones(11, dtype=complex)  # opposite init
            T = np.zeros(K, dtype=complex)
            xr = t1 * 7919.7717 + t2 * 104729.31 + 0.5
            xr -= np.floor(xr)
            xi = t1 * 104729.31 + t2 * 7919.7717 + 0.25
            xi -= np.floor(xi)
            T[0] = xr + 1j * xi
            for s in range(1, K):
                pr, pi_ = T[s - 1].real, T[s - 1].imag
                nr = pr * 9821.4959 + 0.211327
                nr -= np.floor(nr)
                ni = pi_ * 9821.4959 + 0.531327
                ni -= np.floor(ni)
                T[s] = nr + 1j * ni
            T[K - 1] = t1 + 1j * t2
            for s in range(K):
                z = kv * T[s].real - 1j * kvf * T[s].imag
                x = state + z
                state = x / np.abs(x)
            self.assertLess(float(np.abs(state - s23).max()), 1e-6)

    @unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK sweep_test build unavailable")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs the documented burn-in + verbatim tohalf. Residual
        is roots_cm's f32 root cast through the Mobius map; measured worst
        6.7e-7 over 45 rows. Acceptance 1e-5."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in PROBES:
            actual = _run_vm(compiled, t1, t2, SWEEP_TEST_LAPACK)
            expected = _snapshot_row(generator, t1, t2)
            self.assertEqual(len(actual), 11)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-5, (t1, t2, float(rel.max())))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_expand_roots_is_bitwise_np_poly(self):
        """The new primitive: expand_roots of a literal root multiset equals
        np.poly exactly (same convolution order, IEEE-identical ops)."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        src = "poly = vector_literal(1+1i, 2, -0.5i, -3, 0.25+0.75i)\npoly = expand_roots(poly)\nemit"
        compiled = compile_coeff_program_source(src)
        actual = _run_vm(compiled, 0.3, 0.7, SWEEP_TEST)
        expected = np.poly(np.array([1 + 1j, 2, -0.5j, -3, 0.25 + 0.75j]))
        self.assertEqual(len(actual), 6)
        self.assertEqual(float(np.abs(actual - expected).max()), 0.0)


if __name__ == "__main__":
    unittest.main()
