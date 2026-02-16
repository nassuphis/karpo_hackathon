"""Tests for solveRootsEA — the main-thread Ehrlich-Aberth solver."""

import math
import pytest


def solve(page, coeffs, warm_start=None):
    """Call solveRootsEA in the browser and return results."""
    return page.evaluate("""([coeffs, warmStart]) => {
        var ws = warmStart ? warmStart.map(r => ({re: r[0], im: r[1]})) : null;
        var c = coeffs.map(r => ({re: r[0], im: r[1]}));
        var roots = solveRootsEA(c, ws);
        return {
            roots: roots.map(r => [r.re, r.im])
        };
    }""", [coeffs, warm_start])


def roots_close_to(actual, expected, tol=1e-6):
    """Check that each expected root has a match in actual (unordered)."""
    used = [False] * len(actual)
    for ere, eim in expected:
        found = False
        for j, (are, aim) in enumerate(actual):
            if not used[j] and abs(are - ere) < tol and abs(aim - eim) < tol:
                used[j] = True
                found = True
                break
        if not found:
            return False
    return True


class TestDegree1:
    def test_z_plus_1(self, page):
        # z + 1 = 0 => root at -1
        result = solve(page, [[1, 0], [1, 0]])
        roots = result["roots"]
        assert len(roots) == 1
        assert abs(roots[0][0] - (-1)) < 1e-10
        assert abs(roots[0][1]) < 1e-10

    def test_2z_minus_3(self, page):
        # 2z - 3 = 0 => root at 1.5
        result = solve(page, [[2, 0], [-3, 0]])
        roots = result["roots"]
        assert len(roots) == 1
        assert abs(roots[0][0] - 1.5) < 1e-10
        assert abs(roots[0][1]) < 1e-10


class TestDegree2:
    def test_z2_minus_1(self, page):
        # z^2 - 1 = 0 => roots at +1, -1
        result = solve(page, [[1, 0], [0, 0], [-1, 0]])
        roots = result["roots"]
        assert len(roots) == 2
        assert roots_close_to(roots, [[1, 0], [-1, 0]])

    def test_z2_plus_1(self, page):
        # z^2 + 1 = 0 => roots at +i, -i
        result = solve(page, [[1, 0], [0, 0], [1, 0]])
        roots = result["roots"]
        assert len(roots) == 2
        assert roots_close_to(roots, [[0, 1], [0, -1]])

    def test_z2_minus_4z_plus_4(self, page):
        # z^2 - 4z + 4 = 0 => double root at 2
        result = solve(page, [[1, 0], [-4, 0], [4, 0]])
        roots = result["roots"]
        assert len(roots) == 2
        for r in roots:
            assert abs(r[0] - 2) < 1e-4  # wider tolerance for double root
            assert abs(r[1]) < 1e-4


class TestDegree3:
    def test_cube_roots_of_unity(self, page):
        # z^3 - 1 = 0 => roots at 1, e^(2pi*i/3), e^(4pi*i/3)
        result = solve(page, [[1, 0], [0, 0], [0, 0], [-1, 0]])
        roots = result["roots"]
        assert len(roots) == 3
        expected = [
            [1, 0],
            [math.cos(2 * math.pi / 3), math.sin(2 * math.pi / 3)],
            [math.cos(4 * math.pi / 3), math.sin(4 * math.pi / 3)],
        ]
        assert roots_close_to(roots, expected)


class TestHigherDegree:
    def test_degree_5_roots_of_unity(self, page):
        # z^5 - 1 = 0
        coeffs = [[1, 0], [0, 0], [0, 0], [0, 0], [0, 0], [-1, 0]]
        result = solve(page, coeffs)
        roots = result["roots"]
        assert len(roots) == 5
        expected = [
            [math.cos(2 * math.pi * k / 5), math.sin(2 * math.pi * k / 5)]
            for k in range(5)
        ]
        assert roots_close_to(roots, expected)


class TestWarmStart:
    def test_warm_start_converges(self, page):
        # z^2 - 1 with warm start near the actual roots
        warm = [[0.99, 0.01], [-1.01, -0.01]]
        result = solve(page, [[1, 0], [0, 0], [-1, 0]], warm_start=warm)
        roots = result["roots"]
        assert len(roots) == 2
        assert roots_close_to(roots, [[1, 0], [-1, 0]])


class TestNaNResilience:
    def test_leading_zeros(self, page):
        # Leading near-zero coefficients: [~0, ~0, 1, 0, -1] => z^2 - 1
        coeffs = [[1e-20, 0], [0, 1e-20], [1, 0], [0, 0], [-1, 0]]
        result = solve(page, coeffs)
        roots = result["roots"]
        assert len(roots) == 2
        assert roots_close_to(roots, [[1, 0], [-1, 0]])

    def test_always_returns_degree_roots(self, page):
        # Even with tricky coefficients, should always return exactly degree roots
        coeffs = [[1, 0]] + [[0, 0]] * 9 + [[1, 0]]  # z^10 + 1
        result = solve(page, coeffs)
        roots = result["roots"]
        assert len(roots) == 10
        for r in roots:
            assert math.isfinite(r[0]) and math.isfinite(r[1])


