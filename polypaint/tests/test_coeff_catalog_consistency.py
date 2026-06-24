import json
import os
import re
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
sys.path.insert(0, str(LAMBDA_DIR))
sys.path.insert(0, str(ROOT / "tests"))

from coeff_catalog_testlib import ensure_generated_coeff_catalog


def _load_js_catalog_entries():
    ensure_generated_coeff_catalog()
    text = (ROOT / "coeff_func_catalog_js.js").read_text()
    marker = "window._coeffFuncCatalog = "
    start = text.index(marker) + len(marker)
    end = text.rfind(";\n")
    return json.loads(text[start:end])


class TestCoeffCatalogConsistency(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        ensure_generated_coeff_catalog()

    NEW_LOW_AGREEMENT_HAND_FIXES = [
        "poly_13",
        "poly_14",
        "poly_15",
        "poly_18",
        "poly_19",
        "poly_22",
        "poly_25",
        "poly_26",
        "poly_31",
        "poly_39",
        "poly_56",
        "poly_59",
        "poly_63",
        "poly_66",
        "poly_68",
        "poly_69",
        "poly_75",
        "poly_79",
        "poly_80",
        "poly_86",
        "poly_101",
        "poly_106",
        "poly_102",
        "poly_108",
        "poly_110",
        "poly_116",
        "poly_113",
        "poly_114",
        "poly_111",
        "poly_112",
        "poly_115",
        "poly_121",
        "poly_183",
        "poly_307",
        "poly_316",
        "poly_314",
        "poly_317",
        "poly_324",
        "poly_335",
        "poly_333",
        "poly_345",
        "poly_383",
        "poly_413",
        "poly_450",
        "poly_467",
        "poly_475",
        "poly_478",
        "poly_482",
        "poly_491",
        "poly_504",
        "poly_513",
        "poly_535",
        "poly_545",
        "poly_556",
        "poly_551",
        "poly_562",
        "poly_566",
        "poly_570",
        "poly_596",
        "poly_604",
        "poly_667",
        "poly_691",
        "poly_694",
        "poly_698",
        "poly_149",
        "poly_727",
        "poly_733",
        "poly_741",
        "poly_742",
        "poly_745",
        "poly_746",
        "poly_749",
        "poly_751",
        "poly_755",
        "poly_758",
        "poly_759",
        "poly_760",
        "poly_762",
        "poly_765",
        "poly_766",
        "poly_769",
        "poly_773",
        "poly_774",
        "poly_776",
        "poly_777",
        "poly_780",
        "poly_782",
        "poly_785",
        "poly_787",
        "poly_788",
        "poly_792",
        "poly_794",
        "poly_800",
        "poly_806",
        "poly_811",
        "poly_813",
        "poly_814",
        "poly_799",
        "poly_802",
        "poly_809",
        "poly_810",
        "poly_812",
        "poly_818",
    ]

    def test_poly_hand_overrides_are_reflected_in_catalog_and_lookup(self):
        poly_hand_text = (LAMBDA_DIR / "poly_hand.h").read_text()
        catalog = json.loads((LAMBDA_DIR / "coeff_func_catalog.json").read_text())
        by_name = {entry["name"]: entry for entry in catalog}
        js_catalog = {entry["name"]: entry for entry in _load_js_catalog_entries()}
        lookup_text = (LAMBDA_DIR / "coeff_func_lookup.h").read_text()

        hand_funcs = sorted(set(re.findall(r"static void (poly_\d+)_hand\b", poly_hand_text)))
        self.assertGreater(len(hand_funcs), 0, "no hand poly overrides found")

        problems = []
        for name in hand_funcs:
            entry = by_name.get(name)
            if not entry:
                problems.append(f"{name}: missing from coeff_func_catalog.json")
                continue
            if entry.get("c_symbol") != f"{name}_hand":
                problems.append(f"{name}: catalog c_symbol={entry.get('c_symbol')} expected {name}_hand")
            if entry.get("kind") != "hand":
                problems.append(f"{name}: catalog kind={entry.get('kind')} expected hand")
            if entry.get("source") != "poly_hand.h":
                problems.append(f"{name}: catalog source={entry.get('source')} expected poly_hand.h")

            js_entry = js_catalog.get(name)
            if not js_entry:
                problems.append(f"{name}: missing from coeff_func_catalog_js.js")
            else:
                if js_entry.get("kind") != "hand":
                    problems.append(f"{name}: JS kind={js_entry.get('kind')} expected hand")
                if js_entry.get("source") != "poly_hand.h":
                    problems.append(f"{name}: JS source={js_entry.get('source')} expected poly_hand.h")

            lookup_pat = rf'if \(strcmp\(name, "{re.escape(name)}"\) == 0\) return {re.escape(name)}_hand;'
            if not re.search(lookup_pat, lookup_text):
                problems.append(f"{name}: coeff_func_lookup.h does not route to {name}_hand")

            spec_pat = rf'\{{"{re.escape(name)}", {re.escape(name)}_hand, '
            if not re.search(spec_pat, lookup_text):
                problems.append(f"{name}: coeff_func_lookup.h spec table does not use {name}_hand")

        if problems:
            self.fail("\n".join(problems))

    def test_poly_645_metric_overlay_no_longer_reports_zero(self):
        metrics = json.loads((LAMBDA_DIR / "coeff_func_metrics.json").read_text())
        self.assertEqual(metrics["poly_645"]["agreement_pct"], 100)

    def test_poly_795_metric_overlay_tracks_hand_override(self):
        metrics = json.loads((LAMBDA_DIR / "coeff_func_metrics.json").read_text())
        self.assertEqual(metrics["poly_795"]["agreement_pct"], 100)

    def test_coeff_func_params_have_name_and_default(self):
        catalog = json.loads((LAMBDA_DIR / "coeff_func_catalog.json").read_text())
        problems = []
        for entry in catalog:
            for idx, param in enumerate(entry.get("params") or []):
                if not str(param.get("name", "")).strip():
                    problems.append(f"{entry.get('name', '<unknown>')}.params[{idx}] missing name")
                if "default" not in param:
                    problems.append(f"{entry.get('name', '<unknown>')}.params[{idx}] missing default")
        if problems:
            self.fail("\n".join(problems))

    def test_new_low_agreement_hand_fix_metrics_report_full_agreement(self):
        metrics = json.loads((LAMBDA_DIR / "coeff_func_metrics.json").read_text())
        for name in self.NEW_LOW_AGREEMENT_HAND_FIXES:
            self.assertEqual(metrics[name]["agreement_pct"], 100, name)

    def test_dedicated_hand_parity_tests_require_generated_parity_entries(self):
        parity = json.loads((LAMBDA_DIR / "coeff_func_parity.json").read_text())
        catalog = {
            entry["name"]: entry
            for entry in json.loads((LAMBDA_DIR / "coeff_func_catalog.json").read_text())
        }
        js_catalog = {
            entry["name"]: entry
            for entry in _load_js_catalog_entries()
        }

        parity_tested = []
        for path in sorted((ROOT / "tests").glob("test_poly*_hand.py")):
            stem = path.stem
            suffix = stem[len("test_poly"):-len("_hand")]
            name = f"poly_{suffix}"
            parity_tested.append(name)

        self.assertGreater(len(parity_tested), 0, "no dedicated hand parity tests found")

        for name in parity_tested:
            self.assertIn(name, parity, name)
            self.assertEqual(parity[name]["agreement_pct"], 100, name)
            self.assertTrue(parity[name]["parity_verified"], name)
            self.assertIn(name, catalog, name)
            self.assertEqual(catalog[name]["kind"], "hand", name)
            self.assertEqual(catalog[name]["c_symbol"], f"{name}_hand", name)
            self.assertIn(name, js_catalog, name)
            self.assertEqual(js_catalog[name]["kind"], "hand", name)
            self.assertEqual(js_catalog[name].get("agreement_pct"), 100, name)


if __name__ == "__main__":
    unittest.main()
