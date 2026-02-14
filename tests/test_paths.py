"""Tests for animPathFn and space-filling curve helpers."""

import math
import pytest


def eval_path(page, name, t, center_re, center_im, radius, extra=None):
    """Call animPathFn in the browser."""
    return page.evaluate("""([name, t, cRe, cIm, r, extra]) => {
        var p = animPathFn(name, t, {re: cRe, im: cIm}, r, extra);
        return [p.re, p.im];
    }""", [name, t, center_re, center_im, radius, extra])


class TestCirclePath:
    def test_t0_on_positive_x(self, page):
        """At t=0, circle is at center + (radius, 0)."""
        p = eval_path(page, "circle", 0, 0, 0, 1.0)
        assert abs(p[0] - 1.0) < 1e-10
        assert abs(p[1]) < 1e-10

    def test_quarter_turn(self, page):
        """At t=0.25 (quarter cycle), circle is at (0, radius)."""
        p = eval_path(page, "circle", 0.25, 0, 0, 1.0)
        assert abs(p[0]) < 1e-6
        assert abs(p[1] - 1.0) < 1e-6

    def test_full_cycle_returns(self, page):
        """t=0 and t=1 give the same point (periodic)."""
        p0 = eval_path(page, "circle", 0, 0, 0, 1.0)
        p1 = eval_path(page, "circle", 1, 0, 0, 1.0)
        assert abs(p0[0] - p1[0]) < 1e-6
        assert abs(p0[1] - p1[1]) < 1e-6

    def test_respects_center(self, page):
        """Circle is centered at the given center."""
        cx, cy = 3.0, -2.0
        p = eval_path(page, "circle", 0, cx, cy, 1.0)
        assert abs(p[0] - (cx + 1.0)) < 1e-10
        assert abs(p[1] - cy) < 1e-10


class TestHorizontalVertical:
    def test_horizontal_stays_on_y(self, page):
        """Horizontal path has constant im = center.im."""
        for t in [0, 0.25, 0.5, 0.75]:
            p = eval_path(page, "horizontal", t, 1.0, 2.0, 0.5)
            assert abs(p[1] - 2.0) < 1e-10

    def test_vertical_stays_on_x(self, page):
        """Vertical path has constant re = center.re."""
        for t in [0, 0.25, 0.5, 0.75]:
            p = eval_path(page, "vertical", t, 1.0, 2.0, 0.5)
            assert abs(p[0] - 1.0) < 1e-10


class TestLissajous:
    def test_bounded_by_radius(self, page):
        """Lissajous stays within radius of center."""
        radius = 2.0
        cx, cy = 1.0, -1.0
        bound = radius * math.sqrt(2) + 1e-6  # x,y each in [-r,r]
        for i in range(20):
            t = i / 20
            p = eval_path(page, "lissajous", t, cx, cy, radius, {"freqA": 3, "freqB": 2})
            dx = p[0] - cx
            dy = p[1] - cy
            dist = math.sqrt(dx*dx + dy*dy)
            assert dist <= bound, f"At t={t}: dist={dist} > bound={bound}"


class TestCardioid:
    def test_passes_through_center(self, page):
        """Cardioid touches center at t=0.5 (cos(π)=-1 → s=0)."""
        p = eval_path(page, "cardioid", 0.5, 0, 0, 1.0)
        dist = math.sqrt(p[0]**2 + p[1]**2)
        assert dist < 0.01, f"Cardioid at t=0.5 should be near origin, got {p}"


class TestParametricPaths:
    """Test several parametric paths for basic properties."""

    @pytest.mark.parametrize("name", [
        "figure8", "astroid", "deltoid", "rose",
        "epitrochoid", "hypotrochoid", "butterfly", "star", "square"
    ])
    def test_bounded(self, page, name):
        """All parametric paths should stay within ~2*radius of center."""
        radius = 1.0
        cx, cy = 0, 0
        max_dist = 0
        for i in range(100):
            t = i / 100
            p = eval_path(page, name, t, cx, cy, radius)
            dist = math.sqrt(p[0]**2 + p[1]**2)
            if dist > max_dist:
                max_dist = dist
        # All paths should stay within 2*radius (generous bound)
        assert max_dist <= 2 * radius + 0.5, f"{name}: max_dist={max_dist}"

    @pytest.mark.parametrize("name", [
        "circle", "figure8", "astroid", "deltoid",
        "epitrochoid", "star", "square"
    ])
    def test_periodic(self, page, name):
        """Most paths are periodic with period 1."""
        p0 = eval_path(page, name, 0, 0, 0, 1.0)
        p1 = eval_path(page, name, 1, 0, 0, 1.0)
        dist = math.sqrt((p0[0]-p1[0])**2 + (p0[1]-p1[1])**2)
        assert dist < 0.05, f"{name}: not periodic (d={dist})"

    def test_hypotrochoid_periodic_2(self, page):
        """Hypotrochoid uses cos(1.5*wt) so period is 2 (not 1)."""
        p0 = eval_path(page, "hypotrochoid", 0, 0, 0, 1.0)
        p2 = eval_path(page, "hypotrochoid", 2, 0, 0, 1.0)
        dist = math.sqrt((p0[0]-p2[0])**2 + (p0[1]-p2[1])**2)
        assert dist < 0.05, f"hypotrochoid: not periodic at t=2 (d={dist})"


class TestSpaceFillingCurves:
    """Test _mooreXY, _peanoXY, _sierpinskiXY."""

    def test_moore_bounded(self, page):
        """All Moore curve points should be in [-1, 1]."""
        result = page.evaluate("""() => {
            var pts = [];
            for (var i = 0; i < 256; i++) {
                var p = _mooreXY(i, 4);
                pts.push(p);
            }
            var maxAbs = 0;
            for (var p of pts) {
                if (Math.abs(p[0]) > maxAbs) maxAbs = Math.abs(p[0]);
                if (Math.abs(p[1]) > maxAbs) maxAbs = Math.abs(p[1]);
            }
            return {count: pts.length, maxAbs: maxAbs};
        }""")
        assert result["count"] == 256
        assert result["maxAbs"] <= 1.01

    def test_moore_adjacent_connected(self, page):
        """Adjacent Moore curve points should be close (unit steps)."""
        result = page.evaluate("""() => {
            var maxDist = 0;
            for (var i = 0; i < 255; i++) {
                var p0 = _mooreXY(i, 4);
                var p1 = _mooreXY(i+1, 4);
                var d = Math.hypot(p0[0]-p1[0], p0[1]-p1[1]);
                if (d > maxDist) maxDist = d;
            }
            return maxDist;
        }""")
        # Each step should be roughly 2/15 (grid spacing for 16x16 normalized to [-1,1])
        assert result < 0.2, f"Max step distance {result} too large"

    def test_peano_bounded(self, page):
        """All Peano curve points should be in [-1, 1]."""
        result = page.evaluate("""() => {
            var maxAbs = 0;
            var count = 0;
            for (var i = 0; i < 729; i++) {
                var p = _peanoXY(i, 3);
                if (Math.abs(p[0]) > maxAbs) maxAbs = Math.abs(p[0]);
                if (Math.abs(p[1]) > maxAbs) maxAbs = Math.abs(p[1]);
                count++;
            }
            return {count: count, maxAbs: maxAbs};
        }""")
        assert result["count"] == 729
        assert result["maxAbs"] <= 1.01

    def test_peano_adjacent_connected(self, page):
        """Adjacent Peano points should be close."""
        result = page.evaluate("""() => {
            var maxDist = 0;
            for (var i = 0; i < 728; i++) {
                var p0 = _peanoXY(i, 3);
                var p1 = _peanoXY(i+1, 3);
                var d = Math.hypot(p0[0]-p1[0], p0[1]-p1[1]);
                if (d > maxDist) maxDist = d;
            }
            return maxDist;
        }""")
        assert result < 0.15, f"Max step distance {result} too large"

    def test_sierpinski_bounded(self, page):
        """All Sierpinski curve points should be in [-1, 1]."""
        result = page.evaluate("""() => {
            var maxAbs = 0;
            var n = Math.pow(3, 5);  // 243
            for (var i = 0; i <= n; i++) {
                var p = _sierpinskiXY(i, 5);
                if (Math.abs(p[0]) > maxAbs) maxAbs = Math.abs(p[0]);
                if (Math.abs(p[1]) > maxAbs) maxAbs = Math.abs(p[1]);
            }
            return maxAbs;
        }""")
        assert result <= 1.01

    def test_sierpinski_adjacent_connected(self, page):
        """Adjacent Sierpinski points should be close."""
        result = page.evaluate("""() => {
            var maxDist = 0;
            var n = Math.pow(3, 5);
            for (var i = 0; i < n; i++) {
                var p0 = _sierpinskiXY(i, 5);
                var p1 = _sierpinskiXY(i+1, 5);
                var d = Math.hypot(p0[0]-p1[0], p0[1]-p1[1]);
                if (d > maxDist) maxDist = d;
            }
            return maxDist;
        }""")
        assert result < 0.15, f"Max step distance {result} too large"

    def test_moore_wraps_around(self, page):
        """Moore curve index wraps (negative indices and overflow)."""
        result = page.evaluate("""() => {
            var p0 = _mooreXY(0, 4);
            var pWrap = _mooreXY(256, 4);  // should wrap to index 0
            return Math.hypot(p0[0]-pWrap[0], p0[1]-pWrap[1]);
        }""")
        assert result < 1e-10
