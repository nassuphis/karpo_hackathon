"""N7: property-based parity fuzz for the Coeff Program scalar-expr VM.

Generates random bounded expression trees from the scalar grammar, compiles
them through the real source compiler, executes them in the native VM via
sweep_test, and compares against a Python evaluation of the same tree with
matching semantics. Seeded, so failures are reproducible. Also pins P9
(scalars ride the stack through pop) and ipow's integer-power semantics.
"""
import cmath
import json
import os
import random
import struct
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from coeff_program_source import compile_coeff_program_source

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")

P1 = complex(0.685, 0.729)
P2 = complex(-0.249, -0.969)

UNARY = {
    "sin": cmath.sin, "cos": cmath.cos, "exp": cmath.exp,
    "conj": lambda z: z.conjugate(), "neg": lambda z: -z,
    "abs": lambda z: complex(abs(z), 0.0),
    "real": lambda z: complex(z.real, 0.0),
    "imag": lambda z: complex(z.imag, 0.0),
}


def _gen(rng, depth):
    """Random (text, python_value) expression pair, bounded away from poles."""
    if depth == 0 or rng.random() < 0.3:
        pick = rng.random()
        if pick < 0.35:
            return "p1", P1
        if pick < 0.7:
            return "p2", P2
        lit = round(rng.uniform(0.2, 3.0), 3)
        return str(lit), complex(lit, 0.0)
    roll = rng.random()
    if roll < 0.5:
        op = rng.choice(["+", "-", "*"])
        lt, lv = _gen(rng, depth - 1)
        rt, rv = _gen(rng, depth - 1)
        value = lv + rv if op == "+" else (lv - rv if op == "-" else lv * rv)
        return f"({lt} {op} {rt})", value
    if roll < 0.65:
        lt, lv = _gen(rng, depth - 1)
        # keep divisors away from zero
        return f"({lt} / (1.5 + abs(p1)))", lv / (1.5 + abs(P1))
    name = rng.choice(sorted(UNARY))
    it, iv = _gen(rng, depth - 1)
    return f"{name}({it})", UNARY[name](iv)


def _run_program(source_text, n_base=4):
    compiled = compile_coeff_program_source(source_text)
    payload = {"version": 1, "fingerprint": compiled["fingerprint"],
               "tokens": compiled["tokens"], "stack_max": compiled["stack_max"],
               "scalar_exprs": compiled["scalar_exprs"]}
    params = [P1.real, P1.imag, P2.real, P2.imag]
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as fh:
        params_path = fh.name
        fh.write(struct.pack("<4f", *params))
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        spec = {"mode": "coeffgen_chunked", "function": "const",
                "cfpv": [n_base, 0, 0], "params_file": params_path,
                "step_start": 0, "source_step_start": 0,
                "source_n1": 1, "source_n2": 1, "step_count": 1,
                "coeff_transforms": [], "coeff_program": payload}
        proc = subprocess.run([SWEEP, out_path], input=json.dumps(spec),
                              capture_output=True, text=True, timeout=30)
        if proc.returncode != 0:
            raise AssertionError(proc.stderr.strip()[:200])
        data = open(out_path, "rb").read()
    finally:
        for p in (params_path, out_path):
            try:
                os.remove(p)
            except FileNotFoundError:
                pass
    vals = struct.unpack("<" + "f" * (len(data) // 4), data)
    return [complex(vals[i], vals[i + 1]) for i in range(0, len(vals), 2)]


@unittest.skipUnless(os.path.exists(SWEEP), "sweep_test binary not built")
class TestScalarExprPropertyFuzz(unittest.TestCase):
    def test_random_scalar_expressions_match_python(self):
        # f32 params: quantize the reference inputs identically
        import numpy as np

        p1 = complex(np.float32(P1.real), np.float32(P1.imag))
        p2 = complex(np.float32(P2.real), np.float32(P2.imag))
        rng = random.Random(0xC0FFEE)
        checked = 0
        for _ in range(60):
            text, _ = _gen(rng, 3)
            # evaluate with the quantized inputs
            expected = eval(  # noqa: S307 - controlled grammar, test-only
                text.replace("p1", "(p1)").replace("p2", "(p2)"),
                {"p1": p1, "p2": p2, "abs": lambda z: complex(abs(z), 0.0),
                 **{k: v for k, v in UNARY.items()}},
            )
            if not (cmath.isfinite(expected) and abs(expected) < 1e30):
                continue
            got = _run_program(f"poly[0] = {text}\n")[0]
            self.assertLess(
                abs(got - expected) / max(abs(expected), 1.0), 5e-5,
                f"expr {text!r}: native {got} != python {expected}")
            checked += 1
        self.assertGreaterEqual(checked, 40, "fuzz corpus mostly filtered out")

    def test_p9_scalars_ride_the_stack_through_pop(self):
        got = _run_program(
            "push_scalar(p1 + 1)\nmultiply(pop, 2)\npoly[0] = tos[0]\ndrop\n")
        import numpy as np

        p1 = complex(np.float32(P1.real), np.float32(P1.imag))
        expected = (p1 + 1) * 2
        self.assertLess(abs(got[0] - expected), 1e-5)

    def test_ipow_matches_repeated_multiplication(self):
        got = _run_program("poly = ipow(fill(4, p1), 7)\n")
        import numpy as np

        p1 = complex(np.float32(P1.real), np.float32(P1.imag))
        expected = p1 ** 7
        for value in got:
            self.assertLess(abs(value - expected) / abs(expected), 5e-6)
        # negative exponent = reciprocal of the positive power
        got = _run_program("poly = ipow(fill(4, p1), -3)\n")
        expected = 1 / (p1 ** 3)
        for value in got:
            self.assertLess(abs(value - expected) / abs(expected), 5e-6)


if __name__ == "__main__":
    unittest.main()
