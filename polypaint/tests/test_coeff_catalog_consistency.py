import json
import os
import re
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
sys.path.insert(0, str(LAMBDA_DIR))


def _load_js_catalog_entries():
    text = (ROOT / "coeff_func_catalog_js.js").read_text()
    marker = "window._coeffFuncCatalog = "
    start = text.index(marker) + len(marker)
    end = text.rfind(";\n")
    return json.loads(text[start:end])


class TestCoeffCatalogConsistency(unittest.TestCase):

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


if __name__ == "__main__":
    unittest.main()
