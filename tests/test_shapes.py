"""Tests for generateRootShape — root pattern generators."""

import math
import pytest


def gen_shape(page, pattern, degree, spread):
    """Call generateRootShape in the browser."""
    return page.evaluate("""([pattern, degree, spread]) => {
        var roots = generateRootShape(pattern, degree, spread);
        return roots.map(r => [r.re, r.im]);
    }""", [pattern, degree, spread])


PATTERNS = [
    "r-heart", "r-circle", "r-star", "r-spiral", "r-cross",
    "r-diamond", "r-grid", "r-smiley", "r-figure8", "r-butterfly",
    "r-trefoil", "r-polygon", "r-infinity"
]


class TestShapeCount:
    """All shapes should return exactly degree roots."""

    @pytest.mark.parametrize("pattern", PATTERNS)
    def test_correct_count(self, page, pattern):
        degree = 10
        roots = gen_shape(page, pattern, degree, 2.0)
        assert len(roots) == degree, f"{pattern}: expected {degree} roots, got {len(roots)}"

    @pytest.mark.parametrize("pattern", PATTERNS)
    def test_count_large_degree(self, page, pattern):
        degree = 30
        roots = gen_shape(page, pattern, degree, 2.0)
        assert len(roots) == degree


class TestShapeFinite:
    """All roots should have finite coordinates."""

    @pytest.mark.parametrize("pattern", PATTERNS)
    def test_all_finite(self, page, pattern):
        roots = gen_shape(page, pattern, 15, 2.0)
        for i, (re, im) in enumerate(roots):
            assert math.isfinite(re), f"{pattern} root {i}: re={re} not finite"
            assert math.isfinite(im), f"{pattern} root {i}: im={im} not finite"


class TestCircleShape:
    def test_on_circle(self, page):
        """r-circle roots should lie on a circle of given spread radius."""
        spread = 3.0
        roots = gen_shape(page, "r-circle", 8, spread)
        for re, im in roots:
            r = math.sqrt(re**2 + im**2)
            assert abs(r - spread) < 1e-10, f"Root ({re},{im}) at distance {r}, expected {spread}"

    def test_equally_spaced(self, page):
        """r-circle roots should be equally spaced in angle."""
        roots = gen_shape(page, "r-circle", 6, 1.0)
        angles = sorted(math.atan2(im, re) for re, im in roots)
        diffs = [angles[i+1] - angles[i] for i in range(len(angles)-1)]
        # Add wrap-around
        diffs.append(angles[0] + 2*math.pi - angles[-1])
        expected = 2 * math.pi / 6
        for d in diffs:
            assert abs(d - expected) < 1e-6


class TestStarShape:
    def test_alternating_radii(self, page):
        """r-star should alternate between two distinct radii."""
        spread = 2.0
        roots = gen_shape(page, "r-star", 8, spread)
        radii = [math.sqrt(re**2 + im**2) for re, im in roots]
        inner = [r for i, r in enumerate(radii) if i % 2 == 1]
        outer = [r for i, r in enumerate(radii) if i % 2 == 0]
        assert max(inner) < min(outer), "Inner radii should be smaller than outer"


class TestGridShape:
    def test_checkerboard_within_spread(self, page):
        """r-grid roots should be within the spread bounds."""
        spread = 2.0
        roots = gen_shape(page, "r-grid", 12, spread)
        for re, im in roots:
            assert abs(re) <= spread + 0.1, f"re={re} outside spread"
            assert abs(im) <= spread + 0.1, f"im={im} outside spread"


class TestDiamondShape:
    def test_diamond_norm(self, page):
        """r-diamond roots should satisfy |re| + |im| ≈ spread."""
        spread = 2.0
        roots = gen_shape(page, "r-diamond", 20, spread)
        for re, im in roots:
            l1 = abs(re) + abs(im)
            assert abs(l1 - spread) < 0.1, f"L1 norm {l1} ≠ spread {spread}"


class TestSpiralShape:
    def test_increasing_radius(self, page):
        """r-spiral roots should have generally increasing distance from origin."""
        roots = gen_shape(page, "r-spiral", 20, 2.0)
        radii = [math.sqrt(re**2 + im**2) for re, im in roots]
        # First root should be smaller than last
        assert radii[0] < radii[-1]
