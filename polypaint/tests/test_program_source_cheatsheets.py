import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


PARAM_CHEATSHEET_SNIPPETS = [
    "p1 = t1\np2 = t2",
    "p1 = t1 + t2\np2 = exp(t2*pi2i)",
    "p1 = exp(t1*pi2i)\np2 = exp(t2*pi2i)",
    "push(t1)\npush(t2)\nadd\nemit_p1",
    "const(exp(t1*pi2i))\nemit_p1",
    "square(p2)",
    "legacy(unit_circle, both, both)",
    "legacy(rtheta, both, both, real(p1+p2))",
]


COEFF_CHEATSHEET_SNIPPETS = [
    "cf\nemit",
    "poly = rev(poly)\nemit",
    "poly = arange(1, poly_len+1)\nemit",
    "poly = add(multiply(poly, p1), p2)\nemit",
    "poly = sin(poly)\nemit",
    "poly = littlewood(0, 1)\nemit",
    "push_vec(poly_len, 0)\nemit",
    "poly[0] = p1\nemit",
    "poly[(poly_len-1)] = p2\nemit",
]


class TestProgramSourceCheatsheets(unittest.TestCase):
    def test_param_cheatsheet_snippets_compile(self):
        from param_program_source import compile_param_program_source

        for snippet in PARAM_CHEATSHEET_SNIPPETS:
            with self.subTest(snippet=snippet):
                compiled = compile_param_program_source(snippet)
                self.assertGreater(len(compiled["tokens"]), 0)

    def test_coeff_cheatsheet_snippets_compile(self):
        from coeff_program_source import compile_coeff_program_source

        for snippet in COEFF_CHEATSHEET_SNIPPETS:
            with self.subTest(snippet=snippet):
                compiled = compile_coeff_program_source(snippet)
                self.assertGreater(len(compiled["tokens"]), 0)


if __name__ == "__main__":
    unittest.main()
