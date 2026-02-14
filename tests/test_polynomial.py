"""Tests for polynomial math: rootsToCoefficients roundtrip, rankNormalize."""

import math
import pytest


class TestRootsToCoefficients:
    """rootsToCoefficients expands (z-r0)(z-r1)... into coefficient array."""

    def test_single_root(self, page):
        """z - 2 = 0 → coefficients [1, -2]."""
        result = page.evaluate("""() => {
            var c = rootsToCoefficients([{re: 2, im: 0}]);
            return c.map(x => [x.re, x.im]);
        }""")
        assert len(result) == 2
        assert abs(result[0][0] - 1) < 1e-10  # leading coeff
        assert abs(result[1][0] - (-2)) < 1e-10  # constant term

    def test_two_real_roots(self, page):
        """(z-1)(z+1) = z² - 1 → [1, 0, -1]."""
        result = page.evaluate("""() => {
            var c = rootsToCoefficients([{re: 1, im: 0}, {re: -1, im: 0}]);
            return c.map(x => [x.re, x.im]);
        }""")
        assert len(result) == 3
        assert abs(result[0][0] - 1) < 1e-10
        assert abs(result[1][0]) < 1e-10
        assert abs(result[2][0] - (-1)) < 1e-10

    def test_complex_conjugate_roots(self, page):
        """(z-i)(z+i) = z² + 1 → [1, 0, 1]."""
        result = page.evaluate("""() => {
            var c = rootsToCoefficients([{re: 0, im: 1}, {re: 0, im: -1}]);
            return c.map(x => [x.re, x.im]);
        }""")
        assert len(result) == 3
        assert abs(result[0][0] - 1) < 1e-10
        assert abs(result[1][0]) < 1e-10
        assert abs(result[1][1]) < 1e-10  # imaginary part 0
        assert abs(result[2][0] - 1) < 1e-10

    def test_roundtrip_real_roots(self, page):
        """roots → coefficients → solve (via global solveRoots) → should recover original roots."""
        result = page.evaluate("""() => {
            var roots = [{re: 1, im: 0}, {re: -2, im: 0}, {re: 3, im: 0}];
            var coeffs = rootsToCoefficients(roots);
            // Use global solveRoots with proper seeding
            coefficients = coeffs;
            currentRoots = [];  // force fresh seed
            solveRoots();
            var solved = currentRoots.map(r => [r.re, r.im]);
            return {roots: roots.map(r => [r.re, r.im]), solved: solved};
        }""")
        original = sorted(result["roots"], key=lambda r: (r[0], r[1]))
        solved = sorted(result["solved"], key=lambda r: (r[0], r[1]))
        for o, s in zip(original, solved):
            assert abs(o[0] - s[0]) < 1e-6, f"Re mismatch: {o} vs {s}"
            assert abs(o[1] - s[1]) < 1e-6, f"Im mismatch: {o} vs {s}"

    def test_roundtrip_complex_roots(self, page):
        """Complex roots survive roundtrip through coefficients and solver."""
        result = page.evaluate("""() => {
            var roots = [{re: 1, im: 2}, {re: 1, im: -2}, {re: -1, im: 0}];
            var coeffs = rootsToCoefficients(roots);
            coefficients = coeffs;
            currentRoots = [];
            solveRoots();
            var solved = currentRoots.map(r => [r.re, r.im]);
            return {roots: roots.map(r => [r.re, r.im]), solved: solved};
        }""")
        original = sorted(result["roots"], key=lambda r: (round(r[0], 6), round(r[1], 6)))
        solved = sorted(result["solved"], key=lambda r: (round(r[0], 6), round(r[1], 6)))
        for o, s in zip(original, solved):
            assert abs(o[0] - s[0]) < 1e-6, f"Re mismatch: {o} vs {s}"
            assert abs(o[1] - s[1]) < 1e-6, f"Im mismatch: {o} vs {s}"

    def test_degree_5_roundtrip(self, page):
        """5th roots of unity survive roundtrip."""
        result = page.evaluate("""() => {
            var roots = [];
            for (var i = 0; i < 5; i++) {
                roots.push({re: Math.cos(2*Math.PI*i/5), im: Math.sin(2*Math.PI*i/5)});
            }
            var coeffs = rootsToCoefficients(roots);
            coefficients = coeffs;
            currentRoots = [];
            solveRoots();
            var solved = currentRoots.map(r => [r.re, r.im]);
            return solved;
        }""")
        # All 5th roots of unity should be on the unit circle
        for s in result:
            r = math.sqrt(s[0]**2 + s[1]**2)
            assert abs(r - 1.0) < 1e-6, f"Root {s} not on unit circle (|r|={r})"


class TestRankNormalize:
    """rankNormalize: sort-based normalization to [0,1] with tie handling."""

    def test_basic(self, page):
        """Distinct values → evenly spaced ranks."""
        result = page.evaluate("""() => {
            var r = rankNormalize(new Float64Array([10, 20, 30]));
            return Array.from(r);
        }""")
        assert abs(result[0] - 0.0) < 1e-10
        assert abs(result[1] - 0.5) < 1e-10
        assert abs(result[2] - 1.0) < 1e-10

    def test_reversed(self, page):
        """Reversed input → ranks are flipped."""
        result = page.evaluate("""() => {
            var r = rankNormalize(new Float64Array([30, 20, 10]));
            return Array.from(r);
        }""")
        assert abs(result[0] - 1.0) < 1e-10
        assert abs(result[1] - 0.5) < 1e-10
        assert abs(result[2] - 0.0) < 1e-10

    def test_ties(self, page):
        """Tied values get the same rank."""
        result = page.evaluate("""() => {
            var r = rankNormalize(new Float64Array([5, 5, 10]));
            return Array.from(r);
        }""")
        assert result[0] == result[1], "Tied values should have same rank"
        assert result[2] > result[0], "Larger value should have higher rank"

    def test_single_element(self, page):
        """Single element → 0.5."""
        result = page.evaluate("""() => {
            var r = rankNormalize(new Float64Array([42]));
            return Array.from(r);
        }""")
        assert abs(result[0] - 0.5) < 1e-10

    def test_all_same(self, page):
        """All identical values → all rank 0 (normalized to 0)."""
        result = page.evaluate("""() => {
            var r = rankNormalize(new Float64Array([7, 7, 7, 7]));
            return Array.from(r);
        }""")
        for v in result:
            assert abs(v) < 1e-10  # all tied at rank 0

    def test_non_finite_replaced(self, page):
        """Non-finite values replaced with max finite."""
        result = page.evaluate("""() => {
            var r = rankNormalize(new Float64Array([1, Infinity, 3]));
            return Array.from(r);
        }""")
        # Infinity replaced with 3 (max finite), so: [1, 3, 3] → ranks [0, 1, 1]
        assert result is not None
        assert abs(result[0] - 0.0) < 1e-10
        assert result[1] == result[2]  # both mapped to max finite → same rank

    def test_empty(self, page):
        """Empty array → null."""
        result = page.evaluate("""() => {
            return rankNormalize(new Float64Array([]));
        }""")
        assert result is None

    def test_all_non_finite(self, page):
        """All non-finite → null."""
        result = page.evaluate("""() => {
            return rankNormalize(new Float64Array([Infinity, NaN, -Infinity]));
        }""")
        assert result is None
