"""
giga_2921 migration parity (recreate_giga_2921.md).

The reference is a stateful 240-particle repulsion run with unseeded
RNG — the ring radii are run fossils (a faithful re-simulation scores
2D corr ~0.0 vs the artwork). What CAN be pinned deterministically:
  * the exact formula + unit-circle params + poke layout,
  * the frac-cascade remap and measured-fossil ring constants,
  * VM-vs-python-oracle parity through roots_cm / jitter / expand_roots
    (order-positional in the reference's np.roots order),
  * the monic leading slot surviving f32 transport,
  * the ring-mass distribution the remap was fitted to.
"""
import importlib.util
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST_LAPACK = "/tmp/polypaint_sweep_test_lapack"
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.9, 0.9), (0.05, 0.6))


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


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2921_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2921_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle_points(u, v):
    """The analog, exactly as the program computes it (numpy layout)."""
    import numpy as np

    def frac(x):
        return x - np.floor(x)

    T1, T2 = np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)
    cf = np.zeros(25, dtype=complex)
    cf[0] = 30 * (T1**2 * T2); cf[1] = 30 * (T1 * T2**2)
    cf[2] = 40 * T1**3; cf[3] = 40 * T2**3
    cf[4] = -25 * T1**2; cf[5] = -25 * T2**2
    cf[6] = 10 * T1 * T2; cf[9] = 100 * (T1**4 * T2**4)
    cf[11] = -5 * T1; cf[13] = 5 * T2; cf[24] = -10
    rts = np.roots(cf)

    def cascade(m1, m2, add):
        x = frac(u * m1 + v * m2 + add)
        out = [x]
        for _ in range(23):
            x = frac(x * 9821.4959 + 0.211327)
            out.append(x)
        return np.array(out)

    su = cascade(7919.7717, 104729.31, 0.5)
    sv = cascade(3571.3331, 27644.437, 0.25)
    sw = cascade(1299.7091, 15485.863, 0.75)
    m1 = (su < 0.0934).astype(float)
    m2 = (su < 0.4501).astype(float)
    R = (m1 * (1.048 + 0.443 * (sv - 0.5))
         + (1 - m1) * (m2 * (2.569 + 0.686 * (sv - 0.5))
                       + (1 - m2) * (4.361 + 0.585 * (sv - 0.5))))
    return (rts / np.abs(rts)) * R * np.exp(1j * 0.3 * (sw - 0.5))


def _run_vm(compiled, t1, t2):
    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [SWEEP_TEST_LAPACK, "/tmp/giga2921_test_row.bin"],
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
        capture_output=True, text=True, timeout=120,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:300])
    meta = json.loads(proc.stdout)
    return [complex(re, im) for re, im in meta["coeff"]["poly"]]


class TestGiga2921Document(unittest.TestCase):
    def test_generator_document_is_fresh_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_2921_coeff_program.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_2921.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_2921")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)

    def test_ring_masses_match_measured_fossils(self):
        """The remap constants were fitted to the artwork's measured ring
        masses (.0934/.3567/.5399): the PRNG assignment must land within
        sampling tolerance over a probe ensemble."""
        import numpy as np

        rng = np.random.default_rng(11)
        radii = np.abs(np.concatenate(
            [_oracle_points(rng.random(), rng.random()) for _ in range(400)]))
        inner = float(np.mean(radii < 1.6))
        mid = float(np.mean((radii >= 1.6) & (radii < 3.3)))
        outer = float(np.mean(radii >= 3.3))
        self.assertAlmostEqual(inner, 0.0934, delta=0.02)
        self.assertAlmostEqual(mid, 0.3567, delta=0.03)
        self.assertAlmostEqual(outer, 0.5399, delta=0.03)


@unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK sweep_test build unavailable")
class TestGiga2921VmParity(unittest.TestCase):
    def test_vm_matches_oracle_and_round_trips(self):
        """Order-positional coefficient parity vs np.poly of the oracle
        points (the debug transport is f32, so ~1e-7 is the channel floor;
        1e-5 leaves margin, layout mistakes are O(1) — the descending/
        reciprocal bug this wave caught scored 1e13). The re-solved roots
        must recover the oracle points and stay monic."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                emitted = np.array(_run_vm(compiled, u, v))
                pts = _oracle_points(u, v)
                expect = np.poly(pts)
                self.assertEqual(len(emitted), 25)
                self.assertEqual(emitted[0], 1.0)   # monic leading, f32-alive
                rel = np.abs(emitted - expect) / np.maximum(1.0, np.abs(expect))
                self.assertLess(float(rel.max()), 1e-5)
                back = np.roots(emitted)
                pool = list(pts)
                worst = 0.0
                for b in back:
                    j = int(np.argmin([abs(b - p) for p in pool]))
                    worst = max(worst, abs(b - pool.pop(j)))
                self.assertLess(worst, 1e-5)


if __name__ == "__main__":
    unittest.main()
