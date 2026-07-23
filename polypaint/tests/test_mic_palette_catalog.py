"""Meditations-in-Color palette catalog contract (schema v2).

The MIC picker (js/04) turns a catalog entry into a `custom:` wire spec
plus palette_display_name — no new palette machinery. v2 rows are minimal
by design (display name + packed hex string; site-level credit only).
These tests pin the half of the contract the backend enforces: every
entry must produce a spec CUSTOM_PALETTE_RE accepts and a display name
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


def _stops_from_packed(packed):
    """Mirror of the js loader's split: 6 lowercase hex chars per stop."""
    return re.findall(r"[0-9a-f]{6}", packed)


def _wire_from_packed(packed):
    """Mirror of js _micPaletteWire (the catalog floor is 3 stops, so the
    js duplication branch never fires on catalog data)."""
    return "custom:" + "-".join(_stops_from_packed(packed))


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
        self.assertEqual(self.doc["schema_version"], 2)
        self.assertIn("meditationsincolor.com", self.doc["origin"])
        self.assertIn("Meditations in Color", self.doc["credit"])
        self.assertGreaterEqual(len(self.rows), 20000)
        self.assertEqual(self.doc["count"], len(self.rows))

    def test_rows_are_name_and_packed_hex_only(self):
        """v2 is deliberately minimal: any extra field is schema drift."""
        for i, row in enumerate(self.rows):
            self.assertEqual(set(row.keys()), {"n", "c"}, f"row {i}")

    def test_every_entry_is_wire_and_display_name_valid(self):
        """The full catalog: each entry must survive the exact backend
        validators that gate render payloads. The packed split must also
        consume every character (no truncated/odd stops)."""
        for i, row in enumerate(self.rows):
            packed = row["c"]
            stops = _stops_from_packed(packed)
            self.assertEqual("".join(stops), packed,
                            f"row {i}: packed string not a whole number of stops")
            wire = _wire_from_packed(packed)
            self.assertTrue(
                CUSTOM_PALETTE_RE.match(wire),
                f"row {i} ({row['n']!r}) wire rejected: {wire[:60]}")
            self.assertTrue(is_valid_palette_name(wire), f"row {i}")
            # raises on >80 chars or non-printable — must round-trip verbatim
            self.assertEqual(
                normalize_palette_display_name(row["n"], wire), row["n"],
                f"row {i} display name rejected")

    def test_no_palettes_below_three_stops(self):
        """1-2 color palettes were removed by user request (monochromes and
        near-monochromes make no interesting image) — pin the floor so a
        refetch can't silently reintroduce them."""
        under = [r["n"] for r in self.rows if len(r["c"]) < 18]
        self.assertEqual(under, [])

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
