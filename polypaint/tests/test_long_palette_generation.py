import importlib.util
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

import color_dicts


def _load_generator_module():
    path = ROOT / "scripts" / "generate_long_palettes.py"
    spec = importlib.util.spec_from_file_location("generate_long_palettes", path)
    mod = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)
    return mod


class TestLongPaletteGeneration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.mod = _load_generator_module()

    def test_long_spec_parses_many_stops(self):
        stops = self.mod.parse_long_spec(color_dicts.COLOR_LONG_STRINGS["marvel_spiderman_long"])
        self.assertEqual(len(stops), 11)

    def test_long_palette_expands_to_31_stops_and_preserves_endpoints(self):
        stops = self.mod.parse_long_spec(color_dicts.COLOR_LONG_STRINGS["marvel_spiderman_long"])
        expanded = self.mod.expand_long_palette_hsv(stops)
        self.assertEqual(len(expanded), 31)
        self.assertEqual(expanded[0], stops[0])
        self.assertEqual(expanded[-1], stops[-1])

    def test_unknown_color_name_fails(self):
        with self.assertRaises(ValueError):
            self.mod.parse_long_spec("red:bogusname:blue:green")

    def test_generated_js_and_python_outputs_include_long_palette(self):
        js_text = (ROOT / "long_palette_catalog_js.js").read_text(encoding="utf-8")
        self.assertIn('"palette_id": "long_marvel_spiderman_long"', js_text)

        spec = importlib.util.spec_from_file_location(
            "long_palette_names_generated",
            ROOT / "lambda" / "long_palette_names_generated.py",
        )
        mod = importlib.util.module_from_spec(spec)
        assert spec.loader is not None
        spec.loader.exec_module(mod)
        self.assertIn("long_marvel_spiderman_long", mod.LONG_PALETTE_NAMES)

    def test_gradient_css_uses_all_31_stops(self):
        records = self.mod.build_catalog_records()
        entry = next(rec for rec in records if rec["name"] == "marvel_spiderman_long")
        css = entry["gradient_css"]
        self.assertEqual(css.count("rgb("), 31)
        self.assertIn("100%", css)


if __name__ == "__main__":
    unittest.main()
