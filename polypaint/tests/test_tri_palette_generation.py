import importlib.util
import json
import os
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

import color_dicts


def _load_generator_module():
    path = ROOT / "scripts" / "generate_tri_palettes.py"
    spec = importlib.util.spec_from_file_location("generate_tri_palettes", path)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)
    return mod


class TestTriPaletteGeneration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.mod = _load_generator_module()

    def test_canonicalization_collapses_alias_rows(self):
        records = self.mod.build_catalog_records()
        names = [rec["name"] for rec in records]
        self.assertIn("redgold", names)
        self.assertNotIn("rg", names)
        entry = next(rec for rec in records if rec["name"] == "redgold")
        self.assertIn("rg", entry["aliases"])

    def test_redgold_has_15_stops_and_exact_midpoint(self):
        tri = self.mod.parse_tri_spec(color_dicts.COLOR_TRI_STRINGS["redgold"])
        stops = self.mod.expand_tri_palette_hsv(tri)
        self.assertEqual(len(stops), 15)
        self.assertEqual(stops[0], tri[0])
        self.assertEqual(stops[7], tri[1])
        self.assertEqual(stops[14], tri[2])

    def test_unknown_color_name_fails(self):
        with self.assertRaises(ValueError):
            self.mod.parse_tri_spec("red:bogusname:blue")

    def test_generated_js_and_python_outputs_include_tri_redgold(self):
        js_text = (ROOT / "tri_palette_catalog_js.js").read_text(encoding="utf-8")
        self.assertIn('"palette_id": "tri_redgold"', js_text)

        spec = importlib.util.spec_from_file_location(
            "tri_palette_names_generated",
            ROOT / "lambda" / "tri_palette_names_generated.py",
        )
        mod = importlib.util.module_from_spec(spec)
        assert spec.loader is not None
        spec.loader.exec_module(mod)
        self.assertIn("tri_redgold", mod.TRI_PALETTE_NAMES)

    def test_gradient_css_uses_all_15_stops(self):
        records = self.mod.build_catalog_records()
        entry = next(rec for rec in records if rec["name"] == "redgold")
        css = entry["gradient_css"]
        self.assertEqual(css.count("rgb("), 15)
        self.assertIn("100%", css)


if __name__ == "__main__":
    unittest.main()
