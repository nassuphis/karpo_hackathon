"""Tests for stats computation functions: pairwise distances, etc."""

import math
import pytest


def set_roots(page, roots):
    """Set currentRoots in the browser."""
    page.evaluate("""(roots) => {
        currentRoots = roots.map(r => ({re: r[0], im: r[1]}));
    }""", roots)


class TestComputeMinDist:
    def test_two_roots(self, page):
        set_roots(page, [[0, 0], [3, 4]])
        result = page.evaluate("() => computeMinDist()")
        assert abs(result - 5.0) < 1e-10

    def test_three_roots(self, page):
        """Min distance among three roots."""
        set_roots(page, [[0, 0], [1, 0], [10, 0]])
        result = page.evaluate("() => computeMinDist()")
        assert abs(result - 1.0) < 1e-10

    def test_single_root(self, page):
        set_roots(page, [[5, 5]])
        result = page.evaluate("() => computeMinDist()")
        assert result == 0

    def test_coincident_roots(self, page):
        set_roots(page, [[1, 1], [1, 1], [5, 5]])
        result = page.evaluate("() => computeMinDist()")
        assert result == 0.0


class TestComputeMaxDist:
    def test_two_roots(self, page):
        set_roots(page, [[0, 0], [3, 4]])
        result = page.evaluate("() => computeMaxDist()")
        assert abs(result - 5.0) < 1e-10

    def test_three_roots(self, page):
        set_roots(page, [[0, 0], [1, 0], [10, 0]])
        result = page.evaluate("() => computeMaxDist()")
        assert abs(result - 10.0) < 1e-10

    def test_single_root(self, page):
        set_roots(page, [[5, 5]])
        result = page.evaluate("() => computeMaxDist()")
        assert result == 0


class TestComputeMeanDist:
    def test_two_roots(self, page):
        """Mean of single pair = that pair's distance."""
        set_roots(page, [[0, 0], [3, 4]])
        result = page.evaluate("() => computeMeanDist()")
        assert abs(result - 5.0) < 1e-10

    def test_three_equidistant(self, page):
        """Equilateral triangle on unit circle."""
        roots = [[math.cos(2*math.pi*i/3), math.sin(2*math.pi*i/3)] for i in range(3)]
        set_roots(page, roots)
        result = page.evaluate("() => computeMeanDist()")
        # All pairs have distance sqrt(3)
        expected = math.sqrt(3)
        assert abs(result - expected) < 1e-6

    def test_collinear(self, page):
        """Three collinear roots: 0, 1, 3 → pairs: 1, 3, 2 → mean = 2."""
        set_roots(page, [[0, 0], [1, 0], [3, 0]])
        result = page.evaluate("() => computeMeanDist()")
        assert abs(result - 2.0) < 1e-10


class TestPercentileSorted:
    def test_median(self, page):
        result = page.evaluate("() => percentileSorted([1, 2, 3, 4, 5], 0.5)")
        assert abs(result - 3.0) < 1e-10

    def test_min(self, page):
        result = page.evaluate("() => percentileSorted([10, 20, 30], 0)")
        assert abs(result - 10.0) < 1e-10

    def test_max(self, page):
        result = page.evaluate("() => percentileSorted([10, 20, 30], 1)")
        assert abs(result - 30.0) < 1e-10

    def test_interpolation(self, page):
        """q=0.25 on [0, 10] → 2.5."""
        result = page.evaluate("() => percentileSorted([0, 10], 0.25)")
        assert abs(result - 2.5) < 1e-10

    def test_empty(self, page):
        result = page.evaluate("() => percentileSorted([], 0.5)")
        assert result == 0

    def test_single(self, page):
        result = page.evaluate("() => percentileSorted([42], 0.5)")
        assert abs(result - 42) < 1e-10
