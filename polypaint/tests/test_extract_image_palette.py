"""extract_image_palette: MIC's artwork-page algorithm, ported.

Validated against the real site (2026-07-23, Arthur Dove 'Space Divided
by Line Motive'): all 8 stored works[] colors reproduced at median
0.004-0.012 Oklab across seeds (the site's own display rolls differ from
its stored palettes by ~0.076 — the pipeline is nondeterministic by
design there). These tests are offline: a synthetic block image with
known colors and areas must round-trip through the full pipeline.
"""
import importlib.util
import os
import sys
import unittest

import numpy as np
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), "..")


def _load():
    path = os.path.join(ROOT, "scripts", "extract_image_palette.py")
    spec = importlib.util.spec_from_file_location("extract_image_palette", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


EP = _load()

# vivid + earth + neutral blocks with distinct areas (fractions of width)
BLOCKS = [
    ("d62839", 0.30),   # vivid red
    ("2255aa", 0.25),   # blue
    ("1f9e8e", 0.15),   # teal
    ("e8a020", 0.10),   # orange
    ("8a8a8a", 0.12),   # neutral grey
    ("7a5c3a", 0.08),   # earth brown
]


def _hex_rgb(h):
    return [int(h[i:i + 2], 16) for i in (0, 2, 4)]


def _block_image(width=240, height=160):
    arr = np.zeros((height, width, 3), dtype=np.uint8)
    x = 0
    for hex_color, frac in BLOCKS:
        w = int(round(width * frac))
        arr[:, x:x + w] = _hex_rgb(hex_color)
        x += w
    arr[:, x:] = _hex_rgb(BLOCKS[-1][0])
    return Image.fromarray(arr, "RGB")


def _oklab_dist(hex_a, hex_b):
    lab = EP.srgb_to_oklab(np.array([_hex_rgb(hex_a), _hex_rgb(hex_b)], float))
    return float(np.linalg.norm(lab[0] - lab[1]))


class TestOklab(unittest.TestCase):
    def test_white_and_black_land_on_the_l_axis(self):
        lab = EP.srgb_to_oklab(np.array([[255.0, 255.0, 255.0], [0.0, 0.0, 0.0]]))
        self.assertAlmostEqual(lab[0][0], 1.0, places=3)
        self.assertAlmostEqual(abs(lab[0][1]) + abs(lab[0][2]), 0.0, places=3)
        self.assertAlmostEqual(lab[1][0], 0.0, places=3)


class TestExtraction(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.img = _block_image()
        cls.palette = EP.extract_palette(cls.img, colors=8, seed=7)

    def test_every_block_color_is_recovered(self):
        hexes = [p["hex"] for p in self.palette]
        for hex_color, _ in BLOCKS:
            best = min(_oklab_dist(hex_color, h) for h in hexes)
            self.assertLess(best, 0.03, f"block #{hex_color} unmatched ({best:.3f})")

    def test_shares_reflect_block_areas(self):
        def share_of(hex_color):
            return max((p["share"] for p in self.palette
                        if _oklab_dist(hex_color, p["hex"]) < 0.03), default=0.0)
        red, blue = share_of(BLOCKS[0][0]), share_of(BLOCKS[1][0])
        earth = share_of(BLOCKS[5][0])
        self.assertGreater(red, blue)      # 30% area > 25% area
        self.assertGreater(blue, earth)    # 25% area > 8% area
        self.assertAlmostEqual(red, 0.30, delta=0.06)

    def test_substrate_penalty_orders_scores_not_shares(self):
        """The earth block has more area than nothing but must score below
        the vivid blocks of comparable area — the 0.58 penalty at work."""
        def score_of(hex_color):
            return max((p["score"] for p in self.palette
                        if _oklab_dist(hex_color, p["hex"]) < 0.03), default=0.0)
        self.assertGreater(score_of("e8a020"), score_of("7a5c3a"))

    def test_seeded_runs_are_deterministic(self):
        again = EP.extract_palette(self.img, colors=8, seed=7)
        self.assertEqual([p["hex"] for p in self.palette],
                         [p["hex"] for p in again])

    def test_other_seeds_still_recover_all_blocks(self):
        palette = EP.extract_palette(self.img, colors=8, seed=8)
        hexes = [p["hex"] for p in palette]
        for hex_color, _ in BLOCKS:
            best = min(_oklab_dist(hex_color, h) for h in hexes)
            self.assertLess(best, 0.03, f"seed 8: block #{hex_color} unmatched")


if __name__ == "__main__":
    unittest.main()
