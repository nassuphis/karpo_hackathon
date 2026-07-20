"""
giga_8 / giga_27 (T3 roots-inside-coefficients) parity.

giga_8's oracle uses Rscript-generated fixtures (R polyroot output
verbatim per probe): cpoly's deflation order is NOT a magnitude sort
at near-ties, and the app's roots_jt reproduced R's order exactly on
every probe — the fixture comparison pins that.

giga_27's oracle is polys/giga.py verbatim (np.roots order); the VM
side runs roots_cm on the Accelerate-linked LAPACK test binary (the
per-slot comparison IS the roots_cm == np.roots order verification).
"""
import importlib.util
import json
import os
import subprocess
import sys
import unittest

import numpy as np

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
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
    path = os.path.join(ROOT, "scripts", "gen_giga_early_t3_coeff_programs.py")
    spec = importlib.util.spec_from_file_location("gen_giga_early_t3", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _uc(u, v):
    return np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)


# R ground truth: pr(x) = rep(polyroot(x), length.out=length(x)) run by
# Rscript at authoring for each probe (roots1 from the first cubic,
# roots2 from the chained one) — cpoly's deflation order is NOT a
# magnitude sort at near-ties, so fixtures beat any modeled order.
R_FIXTURES = {
    (0.137, 0.823): (
        [-0.01786865122931617-0.01040454742586112j, -0.1544898975191762-0.4492848870352547j, -7.411260604138727+6.978026687469902j, -0.01786865122931617-0.01040454742586112j],
        [0.2559636196504553+0.4521960037604284j, -0.532445022824191-0.0009630545163483858j, 0.2543419209908163-0.45123294924408j, 0.2559636196504553+0.4521960037604284j],
    ),
    (0.75, 0.31): (
        [0.01824034793427403-0.00669765949062319j, -0.4650619848727426-0.1622795603721288j, 10.44682163693847+0.1689772198627502j, 0.01824034793427403-0.00669765949062319j],
        [0.2627078224817713+1.324079116946513j, 0.2570890456290815-1.314955903691673j, -3.272013913997179-0.009123213254839602j, 0.2627078224817713+1.324079116946513j],
    ),
    (0.31, 0.77): (
        [0.01116856193864125+0.01625210800463146j, -0.51242855142806+0.1298215546452525j, -8.796504869393093-3.827319189496664j, 0.01116856193864125+0.01625210800463146j],
        [0.6888116109078751+0.002366420160343578j, -0.2756802065992878+0.548665873905633j, -0.2792443345198117-0.5510322940659766j, 0.6888116109078751+0.002366420160343578j],
    ),
    (0.9, 0.9): (
        [0.005483730596619478-0.01944939006550907j, 0.5088572263441165-0.002432800489585193j, 5.363511565983997+8.112052134304566j, 0.005483730596619478-0.01944939006550907j],
        [0.2714122287860891+0.43140560405898j, -0.477197565689899-0.001538435155647346j, 0.268513881779926-0.4298671689033327j, 0.2714122287860891+0.43140560405898j],
    ),
    (0.05, 0.6): (
        [-0.01985031780057038-0.006447673167994663j, -0.4461503977335729-0.1699036743191702j, -2.62416922821533+9.686916510438699j, -0.01985031780057038-0.006447673167994663j],
        [0.2206054721255636+0.4048571400344521j, -0.4864757645121004-0.0004883032221172481j, 0.2198132837137773-0.4043688368123348j, 0.2206054721255636+0.4048571400344521j],
    ),
}


def o_giga_8(t1, t2, probe):
    roots1, roots2 = (np.array(x) for x in R_FIXTURES[probe])
    # the fixtures ARE roots of the intended cubics (guards against a
    # transcription slip): multiset check against np.roots
    for r, coeffs in ((roots1, [10j, 100 * t1, -50 * t2, t1**3]),
                      (roots2, [np.imag(roots1[2]), -np.real(roots1[1]),
                                roots1[0], 1])):
        pool = list(np.roots(coeffs))
        for z in r[:3]:
            j = int(np.argmin([abs(z - p) for p in pool]))
            assert abs(z - pool.pop(j)) < 1e-8
    cf = np.zeros(35, dtype=complex)
    cf[0:4] = roots1
    cf[9:13] = roots2
    cf[19] = 50 * t1 * t2 + np.real(roots2[0])
    cf[29] = np.exp(1j * t1) + 50 * t2**3
    cf[34] = 200 * np.exp(1j * t1**3) - np.exp(-1j * t2**2)
    return cf[::-1]


def o_giga_27(t1, t2):
    n = 12
    cf = np.zeros(n, dtype=complex)
    cf[0:3] = [-100j, -100j, -100j]
    cf[[n // 2 - 2, n // 2 - 1, n // 2]] = 100 * np.roots([t1, t2, t1, 1])
    cf[[n - 1, n - 2, n - 3]] = 100 * np.roots([t2, t1, t2, 10j])
    return cf


def _run_vm(binary, compiled, t1, t2):
    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [binary, "/tmp/giga_early_t3_row.bin"],
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
    return np.array([complex(re, im) for re, im in meta["coeff"]["poly"]])


class TestGeneratorDocuments(unittest.TestCase):
    def test_documents_fresh_and_compile(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable,
             os.path.join(ROOT, "scripts", "gen_giga_early_t3_coeff_programs.py"),
             "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        for name in generator.BUILDERS:
            with self.subTest(name=name):
                stored = json.load(open(os.path.join(ROOT, f"{name}.coeff-program.json")))
                self.assertEqual(stored["source_text"], generator.build_source_text(name))
                compiled = compile_coeff_program_source(stored["source_text"])
                errors = [d for d in (compiled.get("diagnostics") or [])
                          if d.get("level") == "error"]
                self.assertFalse(errors)
        # solver-construct provenance: giga_8 uses R's own JT order,
        # giga_27 uses np/LAPACK order
        self.assertEqual(
            _load_generator().build_source_text("giga_8").count("roots_jt("), 2)
        self.assertEqual(
            _load_generator().build_source_text("giga_27").count("roots_cm("), 2)


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGiga8VmParity(unittest.TestCase):
    def test_vm_rows_match_oracle(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text("giga_8"))
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                t1, t2 = _uc(u, v)
                ref = o_giga_8(t1, t2, (u, v))
                got = _run_vm(SWEEP_TEST, compiled, u, v)
                self.assertEqual(len(got), len(ref))
                scale = float(np.abs(ref).max())
                rel = float(np.abs(got - ref).max() / scale)
                # jt_solver-vs-R-cpoly: same order, ULP-level value
                # differences between the two JT implementations
                self.assertLess(rel, 1e-6, f"giga_8 coeff parity {rel}")


@unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK-linked test binary unavailable")
class TestGiga27VmParity(unittest.TestCase):
    def test_vm_rows_match_oracle_in_np_order(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text("giga_27"))
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                t1, t2 = _uc(u, v)
                ref = o_giga_27(t1, t2)
                got = _run_vm(SWEEP_TEST_LAPACK, compiled, u, v)
                self.assertEqual(len(got), len(ref))
                scale = float(np.abs(ref).max())
                rel = float(np.abs(got - ref).max() / scale)
                # Accelerate vs numpy's LAPACK: eigensolve-level agreement
                self.assertLess(rel, 1e-6, f"giga_27 coeff parity {rel}")


if __name__ == "__main__":
    unittest.main()
