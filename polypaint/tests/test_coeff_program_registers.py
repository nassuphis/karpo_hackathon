"""
Coeff-program registers are REAL evaluate-once VM slots (CR35-F1/F2):
`r = RHS` with a vector-valued RHS lowers ONCE to an evaluate+
local_store sequence, and every reference is a local_load of the
STORED value — reading a register never re-runs its definition, so
nondeterministic operations (littlewood seeds by token index) are safe
to name, and rebinding chains grow linearly instead of exponentially.
Scalar-shaped RHS stays a text alias (deterministic pure-math grammar,
capped at define time). Programs without vector registers keep their
pre-register chains byte-for-byte.
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
    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_register_program_matches_hand_inlined_values(self):
        """The motivating program (registers naming a scan, a transform of
        it, and their sum) computes the SAME VALUES as the hand-inlined
        form. The chains differ by design now (store/load ops instead of
        duplicated definitions) — value equivalence is the contract."""
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source(USER_PROGRAM)
        inlined = compile_coeff_program_source(
            "poly = add("
            "sort_mod_keep_angle(scan(31, 0, -1*(t1-t2), cos(0.463801-prev)*prev*(t1+t2))), "
            "scan(31, 0, -1*(t1-t2), cos(0.463801-prev)*prev*(t1+t2)))\n"
            "emit"
        )
        for u, v in ((0.2, 0.3), (0.71, 0.13)):
            got = _run_vm(compiled, u, v)
            ref = _run_vm(inlined, u, v)
            self.assertEqual(len(got), len(ref))
            self.assertTrue(all(abs(a - b) < 1e-12 for a, b in zip(got, ref)))
        # the register form evaluates the scan ONCE; the inlined form twice
        scan_ops = [t for t in compiled["tokens"] if t.get("op") == 31]
        inlined_scan_ops = [t for t in inlined["tokens"] if t.get("op") == 31]
        self.assertEqual(len(scan_ops), 1)
        self.assertEqual(len(inlined_scan_ops), 2)

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_rebinding_is_register_like(self):
        """r1 = add(r1, r2): references inside a rebind read the PREVIOUS
        stored value; the rebind re-stores the same slot."""
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
        for u, v in ((0.2, 0.3), (0.9, 0.05)):
            got = _run_vm(rebound, u, v)
            ref = _run_vm(inlined, u, v)
            self.assertTrue(all(abs(a - b) < 1e-12 for a, b in zip(got, ref)))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_registers_evaluate_once_for_nondeterministic_rhs(self):
        """CR35-F1 regression: littlewood's native lowering seeds by token
        index, so under the old textual macros an EXTRA READ of r changed
        the value an earlier assignment had stored. With real registers the
        definition runs once; reads cannot perturb it."""
        from coeff_program_source import compile_coeff_program_source

        base = compile_coeff_program_source(
            "r = littlewood(0, 1)\npoly = r\nemit\n")
        extra_read = compile_coeff_program_source(
            "r = littlewood(0, 1)\npoly = r\nr\nemit\n")
        both_args = compile_coeff_program_source(
            "r = littlewood(0, 1)\npoly = add(r, r)\nemit\n")
        for u, v in ((0.2, 0.3), (0.77, 0.42)):
            a = _run_vm(base, u, v)
            b = _run_vm(extra_read, u, v)
            self.assertEqual(a, b, "an extra read changed a stored register value")
            c = _run_vm(both_args, u, v)
            self.assertEqual(len(c), 1)
            self.assertTrue(abs(c[0] - 2 * a[0]) < 1e-12,
                            "add(r, r) must read the same stored value twice")

    def test_rebind_chains_compile_linearly(self):
        """CR35-F2 regression: 16 rebinds used to expand to ~7.5 MB of
        chain text and seconds of compile; real registers grow linearly."""
        import time

        from coeff_program_source import compile_coeff_program_source

        src = "r = fill(1, 1)\n" + "r = add(r, r)\n" * 16 + "poly = r\nemit\n"
        t0 = time.time()
        compiled = compile_coeff_program_source(src)
        elapsed = time.time() - t0
        self.assertLess(elapsed, 0.5, f"rebind compile took {elapsed:.3f}s")
        self.assertLess(compiled["token_count"], 120)

    def test_scalar_rebind_expansion_is_capped(self):
        """The scalar text-alias side of CR35-F2: definition text growth is
        rejected at define/use time, cheaply."""
        import time

        from coeff_program_source import (
            CoeffProgramSourceCompileError,
            compile_coeff_program_source,
        )

        src = "a = 1+1\n" + "a = a+a\n" * 24 + "poly = fill(1, a)\nemit\n"
        t0 = time.time()
        with self.assertRaises(CoeffProgramSourceCompileError):
            compile_coeff_program_source(src)
        self.assertLess(time.time() - t0, 0.5)

    def test_local_free_programs_keep_their_chains(self):
        """Programs with no vector registers (including scalar aliases)
        compile to the SAME fingerprints as before the register VM ops —
        deployed programs must not shift."""
        from coeff_program_source import compile_coeff_program_source

        alias = compile_coeff_program_source(
            "a = 2+3\npoly = fill(3, a)\nemit\n")
        direct = compile_coeff_program_source(
            "poly = fill(3, 2+3)\nemit\n")
        self.assertEqual(alias["fingerprint"], direct["fingerprint"])

    def test_register_misuse_is_rejected(self):
        from coeff_program_source import (
            CoeffProgramSourceCompileError,
            compile_coeff_program_source,
        )

        cases = {
            "self-reference in first definition":
                "r = add(r, r)\npoly = r\nemit\n",
            "vector register in scalar expression":
                "r = fill(3, 1)\npoly = fill(3, 0)\npoly[0] = r*2\nemit\n",
            "scalar rebound to vector":
                "x = 5\nx = fill(3, 1)\npoly = x\nemit\n",
            "vector rebound to scalar":
                "r = fill(3, 1)\nr = 5\npoly = r\nemit\n",
            "too many registers":
                "".join(f"v{i} = fill(2, {i})\n" for i in range(9))
                + "poly = v0\nemit\n",
        }
        for label, src in cases.items():
            with self.subTest(label=label):
                with self.assertRaises(CoeffProgramSourceCompileError):
                    compile_coeff_program_source(src)

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
