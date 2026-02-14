"""Tests for computeCurveN — animation curve point generation."""

import math
import pytest


def compute_curve(page, home_re, home_im, path_type, radius, angle, extra, n):
    """Call computeCurveN in the browser."""
    return page.evaluate("""([homeRe, homeIm, pathType, radius, angle, extra, N]) => {
        var pts = computeCurveN(homeRe, homeIm, pathType, radius, angle, extra, N);
        return pts.map(p => [p.re, p.im]);
    }""", [home_re, home_im, path_type, radius, angle, extra, n])


class TestCirclePath:
    def test_circle_starts_at_home(self, page):
        """Circle path's first point should be at home position."""
        home_re, home_im = 1.0, 0.5
        pts = compute_curve(page, home_re, home_im, "circle", 0.3, 0, None, 100)
        assert len(pts) == 100
        assert abs(pts[0][0] - home_re) < 1e-6
        assert abs(pts[0][1] - home_im) < 1e-6

    def test_circle_max_distance(self, page):
        """Circle path should reach max distance of 2*radius from home."""
        home_re, home_im = 1.0, 0.5
        radius = 0.3
        pts = compute_curve(page, home_re, home_im, "circle", radius, 0, None, 100)
        dists = [math.sqrt((re - home_re) ** 2 + (im - home_im) ** 2) for re, im in pts]
        # Max distance from home should be ~2*radius (diameter of the offset circle)
        assert max(dists) > 1.5 * radius
        assert max(dists) < 2.5 * radius

    def test_circle_full_loop(self, page):
        """First and last points should be close (nearly closed curve)."""
        pts = compute_curve(page, 0, 0, "circle", 1.0, 0, None, 360)
        d = math.sqrt((pts[0][0] - pts[-1][0]) ** 2 + (pts[0][1] - pts[-1][1]) ** 2)
        assert d < 0.05


class TestEllipsePath:
    def test_ellipse_points(self, page):
        """c-ellipse with non-zero home should produce points at varying distances."""
        home_re, home_im = 2.0, 1.0  # non-zero home (needed — ellipse between home and origin)
        extra = {"width": 50}
        pts = compute_curve(page, home_re, home_im, "c-ellipse", 1.0, 0, extra, 100)
        assert len(pts) == 100
        dists = [math.sqrt(re ** 2 + im ** 2) for re, im in pts]
        assert max(dists) > min(dists) + 0.1, "Ellipse should show eccentricity"

    def test_ellipse_passes_through_home_and_origin(self, page):
        """c-ellipse endpoints are at home and origin."""
        home_re, home_im = 2.0, 0.0
        extra = {"width": 50}
        pts = compute_curve(page, home_re, home_im, "c-ellipse", 1.0, 0, extra, 1000)
        # One point should be near home, another near origin
        min_home = min(math.sqrt((re - home_re) ** 2 + (im - home_im) ** 2) for re, im in pts)
        min_origin = min(math.sqrt(re ** 2 + im ** 2) for re, im in pts)
        assert min_home < 0.02, f"No point near home (closest: {min_home})"
        assert min_origin < 0.02, f"No point near origin (closest: {min_origin})"


class TestNonePath:
    def test_none_single_point(self, page):
        pts = compute_curve(page, 2.0, 3.0, "none", 0.5, 0, None, 1)
        assert len(pts) == 1
        assert abs(pts[0][0] - 2.0) < 1e-10
        assert abs(pts[0][1] - 3.0) < 1e-10


class TestPointCount:
    @pytest.mark.parametrize("n", [1, 10, 100, 500])
    def test_correct_count_circle(self, page, n):
        pts = compute_curve(page, 0, 0, "circle", 1.0, 0, None, n)
        assert len(pts) == n

    @pytest.mark.parametrize("n", [1, 10, 100])
    def test_correct_count_spiral(self, page, n):
        extra = {"turns": 3}
        pts = compute_curve(page, 1, 0, "spiral", 1.0, 0, extra, n)
        assert len(pts) == n
