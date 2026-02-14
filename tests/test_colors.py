"""Tests for color functions: hslToRgb, sensitivityColor, proximity coloring."""

import math
import pytest


class TestHslToRgb:
    """hslToRgb(h, s, l) → [r, g, b] where h in [0,1], s in [0,1], l in [0,1]."""

    def test_red(self, page):
        """h=0, s=1, l=0.5 → pure red [255, 0, 0]."""
        result = page.evaluate("() => hslToRgb(0, 1, 0.5)")
        assert result == [255, 0, 0]

    def test_green(self, page):
        """h=1/3, s=1, l=0.5 → pure green [0, 255, 0]."""
        result = page.evaluate("() => hslToRgb(1/3, 1, 0.5)")
        assert result == [0, 255, 0]

    def test_blue(self, page):
        """h=2/3, s=1, l=0.5 → pure blue [0, 0, 255]."""
        result = page.evaluate("() => hslToRgb(2/3, 1, 0.5)")
        assert result == [0, 0, 255]

    def test_white(self, page):
        """l=1 → white [255, 255, 255]."""
        result = page.evaluate("() => hslToRgb(0, 0, 1)")
        assert result == [255, 255, 255]

    def test_black(self, page):
        """l=0 → black [0, 0, 0]."""
        result = page.evaluate("() => hslToRgb(0, 0, 0)")
        assert result == [0, 0, 0]

    def test_gray(self, page):
        """s=0 → grayscale (r=g=b)."""
        result = page.evaluate("() => hslToRgb(0.5, 0, 0.5)")
        assert result[0] == result[1] == result[2]
        assert result[0] == 128  # round(0.5 * 255)

    def test_yellow(self, page):
        """h=1/6, s=1, l=0.5 → yellow [255, 255, 0]."""
        result = page.evaluate("() => hslToRgb(1/6, 1, 0.5)")
        assert result == [255, 255, 0]


class TestSensitivityColor:
    """sensitivityColor maps [0,1] → blue-white-red gradient."""

    def test_zero_is_blue(self, page):
        result = page.evaluate("() => sensitivityColor(0)")
        assert result == "rgb(0,0,255)"

    def test_half_is_white(self, page):
        result = page.evaluate("() => sensitivityColor(0.5)")
        assert result == "rgb(255,255,255)"

    def test_one_is_red(self, page):
        result = page.evaluate("() => sensitivityColor(1)")
        assert result == "rgb(255,0,0)"

    def test_quarter_is_mid_blue(self, page):
        """0.25 → halfway between blue and white."""
        result = page.evaluate("() => sensitivityColor(0.25)")
        # t = 0.25 * 2 = 0.5, r = round(0.5*255) = 128, g = 128, b = 255
        assert result == "rgb(128,128,255)"


class TestProximityColoring:
    """Test the proximity min-distance computation used for root coloring."""

    def test_equidistant_roots(self, page):
        """Roots equally spaced on unit circle → all have same min distance."""
        result = page.evaluate("""() => {
            var n = 4;
            var roots = [];
            for (var i = 0; i < n; i++) {
                roots.push({re: Math.cos(2*Math.PI*i/n), im: Math.sin(2*Math.PI*i/n)});
            }
            // Compute min distances (same algorithm as paintBitmapFrame)
            var dists = [];
            for (var i = 0; i < n; i++) {
                var md = Infinity;
                for (var j = 0; j < n; j++) {
                    if (j === i) continue;
                    var dx = roots[i].re - roots[j].re;
                    var dy = roots[i].im - roots[j].im;
                    var d2 = dx*dx + dy*dy;
                    if (d2 < md) md = d2;
                }
                dists.push(Math.sqrt(md));
            }
            return dists;
        }""")
        # 4 roots on unit circle at 90° intervals: min distance = sqrt(2)
        expected = math.sqrt(2)
        for d in result:
            assert abs(d - expected) < 1e-10

    def test_clustered_and_isolated(self, page):
        """Two close roots and one far root → close roots have smaller min dist."""
        result = page.evaluate("""() => {
            var roots = [
                {re: 0, im: 0},
                {re: 0.1, im: 0},
                {re: 10, im: 0}
            ];
            var dists = [];
            for (var i = 0; i < 3; i++) {
                var md = Infinity;
                for (var j = 0; j < 3; j++) {
                    if (j === i) continue;
                    var dx = roots[i].re - roots[j].re;
                    var dy = roots[i].im - roots[j].im;
                    var d2 = dx*dx + dy*dy;
                    if (d2 < md) md = d2;
                }
                dists.push(Math.sqrt(md));
            }
            return dists;
        }""")
        assert abs(result[0] - 0.1) < 1e-10  # root 0 closest to root 1
        assert abs(result[1] - 0.1) < 1e-10  # root 1 closest to root 0
        assert abs(result[2] - 9.9) < 1e-10  # root 2 closest to root 1

    def test_prox_palette_exists(self, page):
        """PROX_PALETTE should have 16 entries with valid RGB values."""
        result = page.evaluate("""() => {
            return {
                length: PROX_PALETTE.length,
                first: PROX_PALETTE[0],
                last: PROX_PALETTE[15],
                allValid: PROX_PALETTE.every(c =>
                    c.length === 3 && c.every(v => v >= 0 && v <= 255)
                )
            };
        }""")
        assert result["length"] == 16
        assert result["allValid"]
        # First entry (dark) should be darker than last (bright)
        first_lum = sum(result["first"])
        last_lum = sum(result["last"])
        assert last_lum > first_lum, "Inferno palette should go dark → bright"
