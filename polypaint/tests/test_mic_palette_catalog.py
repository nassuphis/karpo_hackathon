"""Meditations-in-Color palette catalog contract.

The MIC picker (js/04) turns a catalog entry into a `custom:` wire spec
plus palette_display_name — no new palette machinery. These tests pin the
half of that contract the backend enforces: every catalog entry must
produce a spec CUSTOM_PALETTE_RE accepts and a display name
normalize_palette_display_name accepts, the deploy ships the catalog as a
frontend asset, and the generator's offline --check stays green.
"""
import json
import os
import re
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
CATALOG = os.path.join(ROOT, "data", "mic_palette_catalog.json")
sys.path.insert(0, LAMBDA_DIR)

from palette_names import (  # noqa: E402
    CUSTOM_PALETTE_RE,
    is_valid_palette_name,
    normalize_palette_display_name,
)


def _wire_from_colors(colors):
    """Mirror of js _micPaletteWire: single-color palettes duplicate."""
    stops = colors if len(colors) >= 2 else colors * 2
    return "custom:" + "-".join(stops)


class TestMicPaletteCatalog(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with open(CATALOG, encoding="utf-8") as fh:
            cls.doc = json.load(fh)
        cls.rows = cls.doc["palettes"]

    def test_generator_check_passes(self):
        proc = subprocess.run(
            [sys.executable,
             os.path.join(ROOT, "scripts", "fetch_mic_palettes.py"), "--check"],
            capture_output=True, text=True)
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

    def test_catalog_carries_credit_and_scale(self):
        self.assertEqual(self.doc["schema_version"], 1)
        self.assertIn("meditationsincolor.com", self.doc["origin"])
        self.assertIn("Meditations in Color", self.doc["credit"])
        self.assertGreaterEqual(len(self.rows), 20000)
        self.assertEqual(self.doc["count"], len(self.rows))

    def test_every_entry_is_wire_and_display_name_valid(self):
        """The full 20k catalog: each entry must survive the exact backend
        validators that gate render payloads."""
        for i, row in enumerate(self.rows):
            wire = _wire_from_colors(row["c"])
            self.assertTrue(
                CUSTOM_PALETTE_RE.match(wire),
                f"row {i} ({row['n']!r}) wire rejected: {wire[:60]}")
            self.assertTrue(is_valid_palette_name(wire), f"row {i}")
            # raises on >80 chars or non-printable — must round-trip verbatim
            self.assertEqual(
                normalize_palette_display_name(row["n"], wire), row["n"],
                f"row {i} display name rejected")

    def test_entries_have_source_credit_fields(self):
        with_source = sum(1 for r in self.rows if str(r.get("s") or "").strip())
        with_url = sum(1 for r in self.rows if str(r.get("u") or "").strip())
        self.assertGreater(with_source / len(self.rows), 0.99)
        self.assertGreater(with_url / len(self.rows), 0.99)

    def test_deploy_ships_the_catalog_as_frontend_asset(self):
        deploy_text = open(os.path.join(ROOT, "deploy.sh"), encoding="utf-8").read()
        self.assertIn('"data/mic_palette_catalog.json"', deploy_text)
        # .json assets get application/json in the content-type map
        self.assertIn('*.json) echo "application/json"', deploy_text)

    def test_client_and_catalog_agree_on_url(self):
        js_text = open(os.path.join(ROOT, "js", "04-palette-color.js"),
                       encoding="utf-8").read()
        m = re.search(r"MIC_CATALOG_URL = '([^']+)'", js_text)
        self.assertIsNotNone(m, "MIC_CATALOG_URL missing from js/04")
        self.assertEqual(m.group(1), "data/mic_palette_catalog.json")
        self.assertTrue(os.path.exists(CATALOG))


if __name__ == "__main__":
    unittest.main()
