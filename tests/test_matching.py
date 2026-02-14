"""Tests for matchRootOrder — greedy nearest-neighbor root tracking."""

import pytest


def match(page, new_roots, old_roots):
    """Call matchRootOrder in the browser."""
    return page.evaluate("""([newRoots, oldRoots]) => {
        var nr = newRoots.map(r => ({re: r[0], im: r[1]}));
        var or_ = oldRoots.map(r => ({re: r[0], im: r[1]}));
        var result = matchRootOrder(nr, or_);
        return result.map(r => [r.re, r.im]);
    }""", [new_roots, old_roots])


class TestIdentity:
    def test_same_order(self, page):
        roots = [[1, 0], [-1, 0], [0, 1]]
        result = match(page, roots, roots)
        for i in range(len(roots)):
            assert abs(result[i][0] - roots[i][0]) < 1e-10
            assert abs(result[i][1] - roots[i][1]) < 1e-10


class TestPermuted:
    def test_shuffled_back(self, page):
        old = [[1, 0], [-1, 0], [0, 1]]
        new = [[0, 1], [1, 0], [-1, 0]]  # shuffled
        result = match(page, new, old)
        # Should reorder new to match old's order
        for i in range(len(old)):
            assert abs(result[i][0] - old[i][0]) < 1e-10
            assert abs(result[i][1] - old[i][1]) < 1e-10

    def test_reversed(self, page):
        old = [[1, 0], [2, 0], [3, 0]]
        new = [[3, 0], [2, 0], [1, 0]]
        result = match(page, new, old)
        for i in range(len(old)):
            assert abs(result[i][0] - old[i][0]) < 1e-10


class TestCloseRoots:
    def test_slight_movement(self, page):
        old = [[1, 0], [-1, 0], [0, 1]]
        new = [[1.01, 0.01], [-0.99, -0.01], [0.01, 1.01]]
        result = match(page, new, old)
        # Each new root should be matched to the closest old root
        assert abs(result[0][0] - 1.01) < 1e-10  # matched to old[0]
        assert abs(result[1][0] - (-0.99)) < 1e-10  # matched to old[1]
        assert abs(result[2][1] - 1.01) < 1e-10  # matched to old[2]


class TestEdgeCases:
    def test_mismatched_lengths(self, page):
        old = [[1, 0], [-1, 0]]
        new = [[1, 0], [-1, 0], [0, 1]]  # extra root
        result = match(page, new, old)
        # Should return new unchanged
        assert len(result) == 3

    def test_empty_old(self, page):
        new = [[1, 0], [-1, 0]]
        result = match(page, new, [])
        assert len(result) == 2
        assert abs(result[0][0] - 1) < 1e-10
        assert abs(result[1][0] - (-1)) < 1e-10

    def test_single_root(self, page):
        old = [[1, 0]]
        new = [[1.1, 0.1]]
        result = match(page, new, old)
        assert len(result) == 1
        assert abs(result[0][0] - 1.1) < 1e-10
