import importlib.util
import json
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
PARITY_PATH = LAMBDA_DIR / "coeff_func_parity.json"
sys.path.insert(0, str(LAMBDA_DIR))
sys.path.insert(0, str(ROOT / "tests"))

from coeff_catalog_testlib import ensure_generated_coeff_catalog


def _load_module(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


def _load_js_catalog_entries():
    ensure_generated_coeff_catalog()
    text = (ROOT / "coeff_func_catalog_js.js").read_text()
    marker = "window._coeffFuncCatalog = "
    start = text.index(marker) + len(marker)
    end = text.rfind(";\n")
    return json.loads(text[start:end])


class TestCoeffParityResults(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        ensure_generated_coeff_catalog()

    def test_generated_parity_results_cover_all_available_parity_tests(self):
        gen_parity_results = _load_module(LAMBDA_DIR / "gen_parity_results.py", "gen_parity_results")
        parity = json.loads(PARITY_PATH.read_text())

        expected_funcs = set()
        for path in gen_parity_results.discover_parity_tests():
            module = _load_module(path, f"parity_{path.stem}")
            expected_funcs.update(getattr(module, "PARITY_FUNCTIONS", []))

        self.assertEqual(set(parity), expected_funcs)

    def test_generated_parity_results_are_verified_and_visible_in_js_catalog(self):
        parity = json.loads(PARITY_PATH.read_text())
        js_catalog = {
            entry["name"]: entry
            for entry in _load_js_catalog_entries()
        }

        for name, data in parity.items():
            self.assertEqual(data["agreement_pct"], 100, name)
            self.assertTrue(data["parity_verified"], name)
            self.assertGreater(data["parity_cases"], 0, name)
            self.assertTrue(data["parity_test_files"], name)

            self.assertIn(name, js_catalog, name)
            self.assertEqual(js_catalog[name].get("agreement_pct"), data["agreement_pct"], name)
            self.assertEqual(js_catalog[name].get("parity_verified"), data["parity_verified"], name)
            self.assertEqual(js_catalog[name].get("parity_cases"), data["parity_cases"], name)
            self.assertEqual(js_catalog[name].get("parity_test_files"), data["parity_test_files"], name)

    def test_catalog_loader_prefers_generated_parity_over_legacy_overlay(self):
        import gen_catalog

        parity = json.loads(PARITY_PATH.read_text())
        merged = gen_catalog.load_metrics()

        self.assertGreater(len(parity), 0)
        for name in parity:
            self.assertEqual(merged[name]["agreement_pct"], parity[name]["agreement_pct"], name)
            self.assertEqual(merged[name]["parity_verified"], parity[name]["parity_verified"], name)
            self.assertEqual(merged[name]["parity_cases"], parity[name]["parity_cases"], name)


if __name__ == "__main__":
    unittest.main()
