"""
Escape-camera programs (mandel_camera / julia_camera): the deliberate
version of the accidental mini-Mandelbrot (docs/camera-programs.md).

Pinned here:
  * documents fresh + compile + token cap;
  * VM coefficient parity vs the exact python construction (scan
    arithmetic is IEEE-exact; the julia seed uses one libm exp on both
    sides — tight tolerance);
  * the CAMERA: for bounded rows, the smallest root of the emitted
    polynomial sits within branch-correction distance of the affine
    prediction -(a0)/a1;
  * the KNIFE: an escaping row's coefficients go non-finite (the f32
    transport then skips the row — that skip IS the fractal cutout).
"""
import cmath
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

# bounded probes: (t1, t2) whose c / seed stays inside the bounded locus
MANDEL_BOUNDED = ((0.65, 0.5), (0.5, 0.55), (0.73, 0.45))
MANDEL_ESCAPES = ((0.05, 0.05), (0.95, 0.95))
JULIA_BOUNDED = ((0.05, 0.3), (0.1, 0.7))
JULIA_ESCAPES = ((0.95, 0.31), (0.99, 0.77))

RABBIT_C = complex(-0.1226, 0.7449)


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_camera_programs.py")
    spec = importlib.util.spec_from_file_location("gen_camera_programs", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle_vec(name, t1, t2):
    """Exact mirror of the program in python floats. Returns the 122-slot
    vector (slot 0 leading) or None when the orbit overflows f64."""
    if name == "mandel_camera":
        c = complex(t1 * 2.6 - 2.1, t2 * 2.6 - 1.3)
        x = c
        step_c = c
        cam = 3 + 0.5 * c
    else:
        x = 1.4 * t1 * cmath.exp(634.6017010557239j * t2)
        step_c = RABBIT_C
        cam = 3 + 0.5 * x
    orbit = [x]
    for _ in range(119):
        try:
            x = x * x + step_c
        except OverflowError:
            return None
        if not (abs(x.real) < 1e308 and abs(x.imag) < 1e308):
            return None
        orbit.append(x)
    vec = [0j] * 122
    vec[:120] = orbit
    vec[120] = 20 + 0j
    vec[121] = cam
    return vec


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
        [SWEEP_TEST, "/tmp/camera_test_row.bin"],
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
        raise AssertionError(proc.stderr[:300])
    meta = json.loads(proc.stdout)
    return [complex(re, im) for re, im in meta["coeff"]["poly"]]


class TestCameraDocuments(unittest.TestCase):
    def test_generator_documents_fresh_and_compile(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_camera_programs.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        for name, builder in generator.PROGRAMS.items():
            with self.subTest(name=name):
                stored = json.load(open(os.path.join(ROOT, f"{name}.coeff-program.json")))
                self.assertEqual(stored["name"], name)
                self.assertEqual(stored["source_text"], builder())
                compiled = compile_coeff_program_source(stored["source_text"])
                errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
                self.assertFalse(errors)
                self.assertLessEqual(compiled["token_count"], 256)

    def test_escaping_probes_overflow(self):
        """The knife: escape probes must overflow f64 within the 120-step
        scan (the transported row then carries non-finite values and the
        solver skips it — that skip is the fractal silhouette)."""
        for name, escapes in (("mandel_camera", MANDEL_ESCAPES),
                              ("julia_camera", JULIA_ESCAPES)):
            for t1, t2 in escapes:
                with self.subTest(name=name, t1=t1, t2=t2):
                    self.assertIsNone(_oracle_vec(name, t1, t2))


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestCameraVmParity(unittest.TestCase):
    def test_bounded_rows_match_and_camera_points(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name, probes in (("mandel_camera", MANDEL_BOUNDED),
                             ("julia_camera", JULIA_BOUNDED)):
            compiled = compile_coeff_program_source(generator.PROGRAMS[name]())
            for t1, t2 in probes:
                with self.subTest(name=name, t1=t1, t2=t2):
                    expect = _oracle_vec(name, t1, t2)
                    self.assertIsNotNone(expect, "probe unexpectedly escaped")
                    expect = np.array(expect)
                    emitted = np.array(_run_vm(compiled, t1, t2))
                    self.assertEqual(len(emitted), 122)
                    rel = np.abs(emitted - expect) / np.maximum(1.0, np.abs(expect))
                    self.assertLess(float(rel.max()), 1e-9)

                    # the camera: smallest root ~ -(a0)/a1 with branch
                    # corrections O(|orbit|*|z1|/a1) ~ 2*0.16/20 ~ 2e-2
                    roots = np.roots(emitted)
                    z1 = roots[np.argmin(np.abs(roots))]
                    predicted = -expect[121] / expect[120]
                    self.assertLess(abs(z1 - predicted), 5e-3)


if __name__ == "__main__":
    unittest.main()
