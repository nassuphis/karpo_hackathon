"""Frozen-oracle equivalence gate: current compilers vs lambda/*_legacy.py.

ORACLE-EDIT POLICY (read before touching any *_legacy.py file):

1. The `*_legacy.py` modules are frozen references. Their value is that they
   do NOT change when production changes; edit production, never the oracle.
2. Known scope limits — this gate proves less than its name suggests:
   - `coeff_program_source_legacy.py` freezes only the parser SHELL; it
     imports production semantic lowerers (`_current._legacy_lower_statement`),
     so lowering regressions cancel out on both sides. The lowering nets are
     `test_coeff_wire_fingerprints.py` (golden hex) and
     `test_whole_sweep_oracle.py` (byte-exact native SHAs).
   - All oracles read the LIVE registries/profiles, so registry DATA
     regressions also cancel out. Registry shape is separately gated by
     `test_registry_schema.py` and the drift suites.
3. If a registry/schema change forces an oracle shim (it happened once:
   `cdc9a33` patched `coeff_program_chain_legacy._load_legacy_registry` for
   `shared_optional_args`), land the shim as its own commit with a
   before/after oracle-output diff over this corpus recorded in the commit
   message. Comment-only edits are exempt.
4. Corpus counts are pinned exactly (EXPECTED_TOTAL_CASES / EXPECTED_CHECKED)
   and the harvested production corpus is hard-required — a shrinking corpus
   fails instead of silently narrowing coverage. Grow the pins deliberately
   when adding cases.
"""

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


# Pinned per-kind case counts. A silently shrinking corpus (deleted harvested
# dir, blank-source rot) previously kept the gate green while dropping 6 of 9
# cases; grow these numbers deliberately when the corpus grows.
EXPECTED_TOTAL_CASES = 9
EXPECTED_CHECKED = {
    "coeff_source": 8,
    "coeff_chain": 7,
    "param_source": 7,
    "param_chain": 6,
}


def _load_cases():
    out = _load_corpus_cases("corpus.json")
    # Hard-require the harvested production corpus — it carries the real
    # saved calc.json cases; a missing directory must fail, not skip.
    out.extend(_load_corpus_cases(os.path.join("harvested", "corpus.json")))
    if len(out) != EXPECTED_TOTAL_CASES:
        raise AssertionError(
            f"M3 oracle corpus has {len(out)} cases, expected {EXPECTED_TOTAL_CASES}; "
            "update EXPECTED_TOTAL_CASES deliberately if the corpus grew"
        )
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
        self.assertEqual(checked, EXPECTED_CHECKED["coeff_source"])

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
        self.assertEqual(checked, EXPECTED_CHECKED["coeff_chain"])

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
        self.assertEqual(checked, EXPECTED_CHECKED["coeff_chain"])

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
        self.assertEqual(checked, EXPECTED_CHECKED["param_source"])

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
        self.assertEqual(checked, EXPECTED_CHECKED["param_chain"])


if __name__ == "__main__":
    unittest.main()
