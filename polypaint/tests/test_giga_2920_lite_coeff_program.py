"""
giga_2920_lite parity: the single-eigensolve variant of giga_2920
(branch B = raw constructed lattice points at 10% weight; see
scripts/gen_giga_2920_lite_coeff_program.py). Same verification tiers
as the full program: whites are exact mix-roots, pooled-cloud anchors,
bounded dust on the surviving eigensolve, monic leading slot.
Lite-vs-full ensemble equivalence (0.979 vs 0.993 floor) was measured
at authoring.
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
    path = os.path.join(ROOT, "scripts", "gen_giga_2920_lite_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2920_lite_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle_points(u, v):
    import numpy as np

    idx = np.arange(8) - 3.5
    X, Y = np.meshgrid(idx, idx)
    base = (X + 1j * Y).flatten()
    parity = ((np.indices((8, 8)).sum(axis=0)) % 2).flatten()
    seeds = [(7919.7717, 104729.31, 0.5), (3571.3331, 27644.437, 0.25),
             (1299.7091, 15485.863, 0.75), (6997.9337, 86028.121, 0.125)]

    def frac(x):
        return x - np.floor(x)

    def sort_abs(z):
        return z[np.argsort(np.abs(z), kind="stable")]

    r = [frac(u * m1 + v * m2 + c) for m1, m2, c in seeds]
    cf1 = np.poly(base + parity * (0.5 * np.exp(2j * np.pi * r[0]) * (1 + 1j)))
    cf2 = np.poly(base + parity * (0.5 * (r[1] - 0.5) * (1 + 1j)))
    rts_a = sort_abs(np.roots(cf2 * r[2] + cf1 * (1 - r[2])))
    rts_b = sort_abs(base + parity * (0.5 * np.exp(2j * np.pi * r[3]) * (1 + 1j)))
    return rts_a * 0.9 + rts_b * 0.1


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
        [SWEEP_TEST_LAPACK, "/tmp/giga2920lite_test_row.bin"],
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


class TestGiga2920LiteDocument(unittest.TestCase):
    def test_generator_document_is_fresh_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_2920_lite_coeff_program.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_2920_lite.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_2920_lite")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)
        # the whole point: exactly ONE eigensolve remains
        self.assertEqual(stored["source_text"].count("roots_cm"), 1)


@unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK sweep_test build unavailable")
class TestGiga2920LiteVmParity(unittest.TestCase):
    def test_vm_rows_carry_the_lattice_and_bounded_dust(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        idx = np.arange(8) - 3.5
        X, Y = np.meshgrid(idx, idx)
        base = (X + 1j * Y).flatten()
        parity = ((np.indices((8, 8)).sum(axis=0)) % 2).flatten()
        white = base[parity == 0]

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        pooled = []
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                emitted = np.array(_run_vm(compiled, u, v))
                self.assertEqual(len(emitted), 65)
                self.assertEqual(emitted[0], 1.0)
                back = np.roots(emitted)
                self.assertTrue(np.all(np.isfinite(back.view(float))))
                pooled.append(back)

                pts = _oracle_points(u, v)
                pool = list(pts)
                diffs = []
                for b in sorted(back, key=abs):
                    j = int(np.argmin([abs(b - p) for p in pool]))
                    diffs.append(abs(b - pool.pop(j)))
                diffs = np.array(diffs)
                self.assertLess(float(np.median(diffs)), 0.6)
                self.assertLess(float(diffs.max()), 25.0)
                self.assertLess(float(np.abs(back).max()), 50.0)

        pooled = np.concatenate(pooled)
        d = np.abs(pooled[None, :] - white[:, None]).min(axis=1)
        self.assertLess(float(np.median(d)), 0.01)
        self.assertLess(float(d.max()), 0.6)


if __name__ == "__main__":
    unittest.main()
