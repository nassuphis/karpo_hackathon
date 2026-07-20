"""
giga_18 / giga_19 / giga_20 / giga_21 (T4 recurrence + T5 conditional)
parity. Oracles replicate the R scripts verbatim (including the
|v| > 1e-10 guards the programs deliberately do not port — the probes
sit far from the guard sets, so parity is exact). giga_19/20 use RAW
[0,1] params (x_01 only); giga_18/21 use uc. giga_18's oracle uses
np.angle (true Arg) — agreement with the app's floor-based angle is
part of what the parity run proves.
"""
import importlib.util
import json
import math
import os
import subprocess
import sys
import unittest

import numpy as np

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.9, 0.9), (0.05, 0.6),
          (0.42, 0.42))   # on-diagonal: the toggle gate's x == 0 case


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_early_t45_coeff_programs.py")
    spec = importlib.util.spec_from_file_location("gen_giga_early_t45", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _uc(u, v):
    return np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)


def o_giga_18(u, v):
    t1, t2 = _uc(u, v)
    th1, th2 = np.angle(t1), np.angle(t2)
    cf = np.zeros(25, dtype=complex)
    base_scale = 1000 * np.exp(0.5 * np.sin(10 * th1 - 7 * th2))
    secondary_scale = 500 * (np.cos(12 * th1 + 15 * th2)) ** 3
    if np.sin(th1 - th2) > 0:
        toggle_scale = 2000 * np.sin(5 * th1) * np.cos(3 * th2)
    else:
        toggle_scale = -2000 * np.cos(4 * th1) * np.sin(2 * th2)
    if np.cos(th1 + th2) > 0.5:
        cf[20] = (t1**7 * t2**9) * base_scale * toggle_scale
    elif np.cos(th1 + th2) < -0.5:
        cf[18] = (t1**10 - t2**10) * secondary_scale * toggle_scale
    else:
        cf[12] = (t1**4 * t2**7 - t1**5) * base_scale * secondary_scale
    complex_scale = 300 * (np.sin(np.sin(3 * th1 + 4 * th2))) ** 2
    cf[8] = (t1**5 - t2) * complex_scale
    another_scale = 100 * np.exp(np.sin(th1) * np.cos(th2))
    cf[6] = -another_scale * (t1**2 * t2**2)
    sign_flip = 1 if (math.floor((th1 + th2) * 3) % 2) == 0 else -1
    cf[4] = sign_flip * 50 * (t1**3 + t2) * np.sin(2 * th1 - th2)
    cf[2] = -5 * (t1**2 - t2**2) * (10 * np.cos(5 * th2))
    cf[1] = (t2**3 - t1) * (200 * np.sin(3 * th1) * np.sin(th2))
    cf[0] = -5
    cf[24] = (t1**4 - t2**4) * 100 * np.cos(np.sin(th1) * th2) \
        * np.exp(np.cos(2 * th1 - 3 * th2))
    cf[22] = -10 * (t1**9 + t2**9) * (np.sin(7 * th1 - 8 * th2)) ** 3
    cf[15] = 500 * (t1**6 - t2**3) * np.sin((th1 + th2) ** 2) \
        * np.cos((th1 - th2) ** 2)
    return cf[::-1]


def _chain_oracle(first, u, unit):
    cf = np.zeros(90, dtype=complex)
    cf[0] = first
    for k in range(2, 91):
        val = np.sin(k * cf[k - 2]) + np.cos(k * u)
        av = np.abs(val)
        if np.isfinite(av) and av > 1e-10:
            cf[k - 1] = unit * val / av
        else:
            raise AssertionError("guard fired at a probe — pick another probe")
    return cf[::-1]


def o_giga_19(u, v):
    return _chain_oracle(u - v, u, 1j)          # RAW params, x_01 only


def o_giga_20(u, v):
    return _chain_oracle(u + 1j * v, u, 1j)


def o_giga_21(u, v):
    t1, t2 = _uc(u, v)
    cf = np.zeros(50, dtype=complex)
    cf[0] = t1 + t2
    for k in range(2, 51):
        val = np.sin(((k + 3) % 10) * cf[k - 2]) + np.cos(((k + 1) % 10) * t1)
        av = np.abs(val)
        if np.isfinite(av) and av > 1e-10:
            cf[k - 1] = val / av
        else:
            raise AssertionError("guard fired at a probe — pick another probe")
    return cf[::-1]


ORACLES = {
    "giga_18": o_giga_18,
    "giga_19": o_giga_19,
    "giga_20": o_giga_20,
    "giga_21": o_giga_21,
}


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
        [SWEEP_TEST, "/tmp/giga_early_t45_row.bin"],
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
             os.path.join(ROOT, "scripts", "gen_giga_early_t45_coeff_programs.py"),
             "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        self.assertEqual(set(generator.BUILDERS), set(ORACLES))
        for name in generator.BUILDERS:
            with self.subTest(name=name):
                stored = json.load(open(os.path.join(ROOT, f"{name}.coeff-program.json")))
                self.assertEqual(stored["source_text"], generator.build_source_text(name))
                compiled = compile_coeff_program_source(stored["source_text"])
                errors = [d for d in (compiled.get("diagnostics") or [])
                          if d.get("level") == "error"]
                self.assertFalse(errors)
                self.assertLessEqual(compiled["token_count"], 256)
                self.assertIn("poly = rev(poly)", stored["source_text"])


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestVmParity(unittest.TestCase):
    def test_giga_18_matches_exactly(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text("giga_18"))
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                ref = o_giga_18(u, v)
                got = _run_vm(compiled, u, v)
                self.assertEqual(len(got), len(ref))
                scale = float(np.abs(ref).max())
                rel = float(np.abs(got - ref).max() / scale)
                self.assertLess(rel, 1e-10, f"giga_18 coeff parity {rel}")

    def test_chain_gigas_match_head_and_structure(self):
        """The sin(k*prev) chains are CHAOTIC (measured Lyapunov ~10x
        error growth per element; derivatives ~cosh(k) for complex
        prev): 1-ulp libm differences fully decorrelate the tail, in
        the app AND in any two builds of the reference itself. The
        semantic pin is the chain HEAD (first 8 elements, before
        amplification) plus the invariant structure: every chain
        element is exactly unit magnitude."""
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name in ("giga_19", "giga_20", "giga_21"):
            compiled = compile_coeff_program_source(generator.build_source_text(name))
            for u, v in PROBES:
                with self.subTest(name=name, u=u, v=v):
                    ref = ORACLES[name](u, v)[::-1]     # ascending
                    got = _run_vm(compiled, u, v)[::-1]
                    self.assertEqual(len(got), len(ref))
                    head = float(np.abs(got[:8] - ref[:8]).max())
                    self.assertLess(head, 1e-11, f"{name} chain head {head}")
                    mags = np.abs(got[1:])
                    self.assertLess(float(np.abs(mags - 1).max()), 1e-12,
                                    f"{name} chain magnitudes")


if __name__ == "__main__":
    unittest.main()
