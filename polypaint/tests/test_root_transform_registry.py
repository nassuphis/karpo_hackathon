import json
import os
import re
import sys
import unittest


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.join(ROOT, "lambda"))


_EXPECTED_ENUM_NAMES = {
    "rotate_roots": "RT_FN_ROTATE_ROOTS",
    "pull_unit_circle": "RT_FN_PULL_UNIT_CIRCLE",
    "roots_toline": "RT_FN_ROOTS_TOLINE",
    "line_to_unit_circle": "RT_FN_LINE_TO_UNIT_CIRCLE",
    "invert_roots": "RT_FN_INVERT_ROOTS",
    "add_complex": "RT_FN_ADD_COMPLEX",
    "mul_complex": "RT_FN_MUL_COMPLEX",
    "moebius": "RT_FN_MOEBIUS",
    "pull_towards_center": "RT_FN_PULL_TOWARDS_CENTER",
}


class TestRootTransformRegistry(unittest.TestCase):
    def test_root_registry_fn_indices_match_c_enum(self):
        registry_path = os.path.join(ROOT, "lambda", "root_legacy_registry.json")
        header_path = os.path.join(ROOT, "lambda", "root_xforms.h")
        with open(registry_path, "r", encoding="utf-8") as fh:
            registry = json.load(fh)
        with open(header_path, "r", encoding="utf-8") as fh:
            header = fh.read()

        enum_values = {
            name: int(value)
            for name, value in re.findall(r"\b(RT_FN_[A-Z0-9_]+)\s*=\s*([0-9]+)", header)
        }
        registry_values = {
            str(item["name"]): int(item["fn_index"])
            for item in registry.get("functions") or []
        }

        self.assertEqual(set(registry_values), set(_EXPECTED_ENUM_NAMES))
        for name, enum_name in _EXPECTED_ENUM_NAMES.items():
            self.assertIn(enum_name, enum_values)
            self.assertEqual(registry_values[name], enum_values[enum_name], name)

    def test_root_runtime_uses_numeric_dispatch_not_string_chain(self):
        header_path = os.path.join(ROOT, "lambda", "root_xforms.h")
        with open(header_path, "r", encoding="utf-8") as fh:
            header = fh.read()
        apply_body = header.split("static void apply_root_xforms", 1)[1]
        self.assertIn("fn_index", apply_body)
        self.assertNotIn("strcmp(e->name", apply_body)


if __name__ == "__main__":
    unittest.main()
