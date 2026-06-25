import json
import os
import sys
import unittest


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
FIXTURE_DIR = os.path.join(os.path.dirname(__file__), "fixtures", "program-m3-oracle")
sys.path.insert(0, LAMBDA_DIR)


def _load_json(rel_path):
    with open(os.path.join(FIXTURE_DIR, rel_path), "r", encoding="utf-8") as fh:
        return json.load(fh)


def _load_corpus_cases(rel_path):
    corpus = _load_json(rel_path)
    if int(corpus.get("version") or 0) != 1:
        raise AssertionError(f"unsupported M3 oracle corpus version in {rel_path}")
    out = []
    for entry in corpus["cases"]:
        calc = _load_json(entry["calc"])
        calc.setdefault("name", entry["name"])
        out.append(calc)
    return out


def _load_cases():
    out = _load_corpus_cases("corpus.json")
    harvested = os.path.join(FIXTURE_DIR, "harvested", "corpus.json")
    if os.path.exists(harvested):
        out.extend(_load_corpus_cases(os.path.join("harvested", "corpus.json")))
    return out


def _program_params(calc):
    return dict(calc.get("params") or calc.get("coeffgen") or {})


def _selected_coeff_fields(compiled):
    return {
        "source_chain": compiled["source_chain"],
        "expanded_chain": compiled["expanded_chain"],
        "tokens": compiled["tokens"],
        "scalar_exprs": compiled["scalar_exprs"],
        "execution_spec": compiled["execution_spec"],
        "fingerprint": compiled["fingerprint"],
        "token_count": compiled["token_count"],
        "scalar_expr_count": compiled["scalar_expr_count"],
        "stack_max": compiled["stack_max"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
        "legacy_coeff_transforms": compiled["legacy_coeff_transforms"],
    }


def _selected_param_fields(compiled):
    return {
        "chain": compiled.get("chain"),
        "source_chain": compiled["source_chain"],
        "expanded_chain": compiled["expanded_chain"],
        "tokens": compiled["tokens"],
        "scalar_exprs": compiled["scalar_exprs"],
        "execution_spec": compiled["execution_spec"],
        "fingerprint": compiled["fingerprint"],
        "token_count": compiled["token_count"],
        "stack_max": compiled["stack_max"],
        "emits": compiled["emits"],
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
        "legacy_transforms": compiled["legacy_transforms"],
    }


class TestProgramM3Oracles(unittest.TestCase):
    def test_coeff_source_corpus_matches_frozen_legacy_oracle(self):
        import coeff_program_source as current
        import coeff_program_source_legacy as legacy

        checked = 0
        for calc in _load_cases():
            params = _program_params(calc)
            source_text = str(params.get("coeff_program_source_text") or "")
            if not source_text.strip():
                continue
            with self.subTest(calc=calc["name"], kind="coeff_source"):
                old_parsed = legacy.parse_coeff_program_source(source_text)
                new_parsed = current.parse_coeff_program_source(source_text)
                self.assertEqual(new_parsed["chain"], old_parsed["chain"])
                self.assertEqual(new_parsed["statement_count"], old_parsed["statement_count"])

                old_compiled = legacy.compile_coeff_program_source(source_text)
                new_compiled = current.compile_coeff_program_source(source_text)
                self.assertEqual(_selected_coeff_fields(new_compiled), _selected_coeff_fields(old_compiled))
                checked += 1
        self.assertGreaterEqual(checked, 2)

    def test_coeff_chain_corpus_matches_frozen_legacy_oracle(self):
        import coeff_program_chain as current
        import coeff_program_chain_legacy as legacy

        checked = 0
        for calc in _load_cases():
            chain = _program_params(calc).get("coeff_program_chain")
            if not chain:
                continue
            with self.subTest(calc=calc["name"], kind="coeff_chain"):
                old_compiled = legacy.compile_coeff_program_chain(chain)
                new_compiled = current.compile_coeff_program_chain(chain)
                self.assertEqual(_selected_coeff_fields(new_compiled), _selected_coeff_fields(old_compiled))
                checked += 1
        self.assertGreaterEqual(checked, 1)

    def test_coeff_chain_corpus_source_regeneration_preserves_fingerprint(self):
        import coeff_program_chain as chain_compiler
        import coeff_program_source as source_compiler

        checked = 0
        for calc in _load_cases():
            chain = _program_params(calc).get("coeff_program_chain")
            if not chain:
                continue
            with self.subTest(calc=calc["name"], kind="coeff_chain_source"):
                source_text = source_compiler.coeff_source_text_from_chain(chain)
                self.assertTrue(str(source_text or "").strip())
                from_chain = chain_compiler.compile_coeff_program_chain(chain)
                from_source = source_compiler.compile_coeff_program_source(source_text)
                self.assertEqual(from_source["fingerprint"], from_chain["fingerprint"])
                self.assertEqual(from_source["execution_spec"], from_chain["execution_spec"])
                checked += 1
        self.assertGreaterEqual(checked, 1)

    def test_param_source_corpus_matches_frozen_legacy_oracle(self):
        import param_program_source as current
        import param_program_source_legacy as legacy

        checked = 0
        for calc in _load_cases():
            params = _program_params(calc)
            source_text = str(params.get("param_program_source_text") or "")
            if not source_text.strip():
                continue
            with self.subTest(calc=calc["name"], kind="param_source"):
                old_parsed = legacy.parse_param_program_source(source_text)
                new_parsed = current.parse_param_program_source(source_text)
                self.assertEqual(new_parsed["chain"], old_parsed["chain"])
                self.assertEqual(new_parsed["statement_count"], old_parsed["statement_count"])

                old_compiled = legacy.compile_param_program_source(source_text)
                new_compiled = current.compile_param_program_source(source_text)
                self.assertEqual(_selected_param_fields(new_compiled), _selected_param_fields(old_compiled))
                checked += 1
        self.assertGreaterEqual(checked, 2)

    def test_param_chain_corpus_source_regeneration_matches_frozen_legacy_oracle(self):
        import param_program_source as current
        import param_program_source_legacy as legacy

        checked = 0
        for calc in _load_cases():
            chain = _program_params(calc).get("param_program_chain")
            if not chain:
                continue
            with self.subTest(calc=calc["name"], kind="param_chain_source"):
                old_source = legacy.param_source_text_from_chain(chain)
                new_source = current.param_source_text_from_chain(chain)
                self.assertEqual(new_source, old_source)
                old_compiled = legacy.compile_param_program_source(old_source)
                new_compiled = current.compile_param_program_source(new_source)
                self.assertEqual(_selected_param_fields(new_compiled), _selected_param_fields(old_compiled))
                checked += 1
        self.assertGreaterEqual(checked, 1)


if __name__ == "__main__":
    unittest.main()
