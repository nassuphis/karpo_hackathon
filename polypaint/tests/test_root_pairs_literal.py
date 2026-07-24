"""root_pairs_literal: statement-only sugar over two roots_literal pushes.

The first half of the (even) argument list is the START root set, the
second half the END set; each expands to its monic coefficient vector and
both push — end on top — so stack blends sweep root trajectories. Pure
source-level sugar: the emitted chain carries two ordinary roots_literal
chips, so the deployed chain compiler and VM need nothing new.
"""
import json
import os
import subprocess
import sys
import unittest

import numpy as np

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

from coeff_program_source import (  # noqa: E402
    compile_coeff_program_source,
    parse_coeff_program_source,
)

BLEND_SRC = """root_pairs_literal(1, 2i, -1, -2i)
poly = multiply(pop, 0.25)
poly
swap
poly = multiply(pop, 0.75)
poly = add(poly, pop)
emit
"""


def _first_error(source):
    try:
        compiled = compile_coeff_program_source(source)
    except Exception as exc:  # compile errors surface as raised diagnostics
        return str(exc)
    errors = [d for d in (compiled.get("diagnostics") or [])
              if d.get("level") == "error"]
    return errors[0]["message"] if errors else ""


class TestRootPairsLiteral(unittest.TestCase):
    def test_lowers_to_two_roots_literal_chips(self):
        rows = parse_coeff_program_source(
            "root_pairs_literal(1, 2i, -1, -2i)\nemit\n")["chain"]
        pair_rows = [row for row in rows if row and row[0] == "roots_literal"]
        self.assertEqual(len(pair_rows), 2, rows)
        self.assertEqual(pair_rows[0][1:], ["1.0+0.0j", "0.0+2.0j"])    # start half
        self.assertEqual(pair_rows[1][1:], ["-1.0+0.0j", "0.0-2.0j"])   # end half
        self.assertNotIn("root_pairs_literal", [row[0] for row in rows if row])

    def test_statement_only_and_even_count_errors(self):
        self.assertIn("bare statement",
                      _first_error("poly = root_pairs_literal(1, -1)\nemit\n"))
        self.assertIn("only works as a bare statement",
                      _first_error("poly = add(cf, root_pairs_literal(1, -1))\nemit\n"))
        self.assertIn("even number of roots",
                      _first_error("root_pairs_literal(1, 2i, -1)\nemit\n"))
        # local aliases must not shadow the reserved name
        self.assertTrue(_first_error("root_pairs_literal = 3\nemit\n"))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_blend_matches_numpy(self):
        """0.75*poly(start) + 0.25*poly(end) via the SAFE stack idiom (the
        nested-pop trap of add(poly, multiply(pop, ...)) is avoided)."""
        compiled = compile_coeff_program_source(BLEND_SRC)
        errors = [d for d in (compiled.get("diagnostics") or [])
                  if d.get("level") == "error"]
        self.assertFalse(errors)
        payload = {k: compiled[k] for k in (
            "fingerprint", "tokens", "stack_max", "scalar_exprs", "vector_constants")}
        payload["version"] = 1
        proc = subprocess.run(
            [SWEEP_TEST, "/tmp/root_pairs_probe.bin"],
            input=json.dumps({
                "mode": "compute_debug", "function": "const", "cfpv": [1, 0, 0],
                "u": 0.3, "v": 0.7, "grid_n": 100, "coeff_transforms": [],
                "coeff_program": payload,
            }),
            capture_output=True, text=True, timeout=120)
        self.assertEqual(proc.returncode, 0, proc.stderr[:300])
        meta = json.loads(proc.stdout)
        got = np.array([complex(re, im) for re, im in meta["coeff"]["poly"]])
        ref = 0.75 * np.poly([1, 2j]) + 0.25 * np.poly([-1, -2j])
        self.assertLess(float(np.max(np.abs(got - ref))), 1e-12)


if __name__ == "__main__":
    unittest.main()
