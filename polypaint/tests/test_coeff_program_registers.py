"""
Coeff-program registers: source locals with REBINDING plus native
transforms composable in expression position. Together these give the
register mental model (r1 = scan(...); r3 = sort_mod_keep_angle(r1);
r1 = r1 + r2) with zero VM changes — every definition inlines at its
use sites, so the lowered chain is byte-identical to hand-inlining and
the registry's existing src=pop/tgt=push selectors carry the
composition. Fingerprints, wire formats, and deployed VMs are untouched
(pinned here by fingerprint-equivalence and a live sweep_test parity
run).
"""
import importlib.util
import json
import math
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

USER_PROGRAM = """r1 = scan(31, 0, -1*(t1-t2), cos(0.463801-prev)*prev*(t1+t2))
r2 = rev(r1)
r3 = sort_mod_keep_angle(r1)
poly = add(r3, r1)
emit"""


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
        [SWEEP_TEST, "/tmp/coeff_registers_row.bin"],
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


class TestCoeffRegisters(unittest.TestCase):
    def test_register_program_compiles_and_equals_hand_inlined(self):
        """The motivating program (registers naming a scan, a transform of
        it, and their sum) compiles, and its chain is BYTE-IDENTICAL to the
        hand-inlined form: registers are pure compile-time splicing."""
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source(USER_PROGRAM)
        inlined = compile_coeff_program_source(
            "poly = add("
            "sort_mod_keep_angle(scan(31, 0, -1*(t1-t2), cos(0.463801-prev)*prev*(t1+t2))), "
            "scan(31, 0, -1*(t1-t2), cos(0.463801-prev)*prev*(t1+t2)))\n"
            "emit"
        )
        self.assertEqual(compiled["fingerprint"], inlined["fingerprint"])

    def test_rebinding_is_register_like(self):
        """r1 = add(r1, r2): the previous r1 inlines into the new definition
        — sequential register semantics without VM state."""
        from coeff_program_source import compile_coeff_program_source

        rebound = compile_coeff_program_source(
            "r1 = scan(5, 0, 1, prev*2)\n"
            "r2 = fill(5, 3)\n"
            "r1 = add(r1, r2)\n"
            "poly = r1\n"
            "emit"
        )
        inlined = compile_coeff_program_source(
            "poly = add(scan(5, 0, 1, prev*2), fill(5, 3))\nemit"
        )
        self.assertEqual(rebound["fingerprint"], inlined["fingerprint"])

    def test_transforms_compose_in_expression_position(self):
        """Native transforms accept expression arguments in statement AND
        value position (src=pop/tgt=push were always registry-legal; only
        the grammar refused). Enum-arg transforms compose the same way."""
        from coeff_program_source import compile_coeff_program_source

        for src in (
            "poly = sort_mod_keep_angle(scan(5, 0, 1, prev*2))\nemit",
            "poly = add(rev(fill(4, 2)), fill(4, 1))\nemit",
            "poly = roots_cm(vector_literal(1, 0, -1), lo, exact)\nemit",
        ):
            with self.subTest(src=src.splitlines()[0]):
                compiled = compile_coeff_program_source(src)
                self.assertTrue(compiled["tokens"])

    def test_existing_forms_unchanged(self):
        """The new lowering step must not perturb any currently-valid form:
        bare/selector/scalar-arg transform statements keep their chains."""
        from coeff_program_source import (
            compile_coeff_program_source,
            parse_coeff_program_source,
        )

        src = (
            "poly = fill(5, 2)\n"
            "poly = rev(poly)\n"
            "poly\n"
            "poly = sort_mod_keep_angle(pop)\n"
            "emit"
        )
        parsed = parse_coeff_program_source(src)
        self.assertIn(["_native_transform", "rev", "poly", "poly"], parsed["chain"])
        self.assertIn(
            ["_native_transform", "sort_mod_keep_angle", "pop", "poly"],
            parsed["chain"],
        )
        compile_coeff_program_source(src)

        # unknown names still fail loudly
        from coeff_program_source import CoeffProgramSourceCompileError

        with self.assertRaises(CoeffProgramSourceCompileError):
            compile_coeff_program_source("poly = add(nonsense_fn(poly), poly)\nemit")

    def test_round_trip_through_chain_serializer(self):
        """Saved register programs reconstruct: chain->source->chain keeps
        the fingerprint (the serializer emits the stack-form equivalent)."""
        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
            parse_coeff_program_source,
        )

        parsed = parse_coeff_program_source(USER_PROGRAM)
        round_text = coeff_source_text_from_chain(parsed["chain"])
        self.assertEqual(
            compile_coeff_program_source(USER_PROGRAM)["fingerprint"],
            compile_coeff_program_source(round_text)["fingerprint"],
        )


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestCoeffRegistersVmParity(unittest.TestCase):
    def test_user_program_matches_python_oracle(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source(USER_PROGRAM)

        def oracle(t1, t2):
            x = -1 * (t1 - t2)
            out = [x]
            for _ in range(30):
                x = math.cos(0.463801 - x) * x * (t1 + t2)
                out.append(x)
            s = np.array(out, dtype=complex)
            return (np.sort(np.abs(s)) * np.exp(1j * np.angle(s))) + s

        worst = 0.0
        for t1, t2 in ((0.137, 0.823), (0.75, 0.31), (0.4, 0.6)):
            actual = np.array(_run_vm(compiled, t1, t2))
            expected = oracle(t1, t2)
            self.assertEqual(len(actual), 31)
            worst = max(worst, float(np.max(np.abs(actual - expected))))
        self.assertLess(worst, 1e-12)

    def test_rebound_register_values(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source(
            "r1 = scan(5, 0, 1, prev*2)\n"
            "r2 = fill(5, 3)\n"
            "r1 = add(r1, r2)\n"
            "poly = r1\n"
            "emit"
        )
        values = np.array(_run_vm(compiled, 0.3, 0.7))
        self.assertTrue(np.array_equal(
            values, np.array([1, 2, 4, 8, 16], dtype=complex) + 3))


if __name__ == "__main__":
    unittest.main()
