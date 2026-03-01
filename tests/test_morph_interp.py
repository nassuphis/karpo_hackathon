"""Tests for morphInterpPoint() — the morph C-D interpolation function.

Verifies all 7 path types against the formulas in coefficient-path-calc.tex:
  c-node:  returns C position
  d-node:  returns D position
  c+d:     returns C + D (sum)
  line:    mu = (1-cos(theta))/2; lerp(C, D, mu)
  circle:  m + (-a*cos(theta))*u + (sign*a*sin(theta))*v
  ellipse: m + (-a*cos(theta))*u + (sign*b*sin(theta))*v, b = minor*a
  figure8: m + (-a*cos(theta))*u + (sign*(a/2)*sin(2*theta))*v
"""

import math
import pytest


# ── helpers ──────────────────────────────────────────────────────────

def _call_interp(page, cRe, cIm, dRe, dIm, theta, pathType, ccw=False, minor=0.5):
    """Call morphInterpPoint in the browser and return {re, im}."""
    return page.evaluate(f"""() => {{
        return morphInterpPoint({cRe}, {cIm}, {dRe}, {dIm},
            {theta}, "{pathType}", {str(ccw).lower()}, {minor});
    }}""")


# ── c-node path ──────────────────────────────────────────────────────

class TestMorphInterpCNode:
    """pathType='c-node' always returns the C-node position."""

    def test_returns_c_at_theta_zero(self, page):
        r = _call_interp(page, 1, 2, 5, 6, 0, "c-node")
        assert r["re"] == 1 and r["im"] == 2

    def test_returns_c_at_theta_pi(self, page):
        r = _call_interp(page, 1, 2, 5, 6, math.pi, "c-node")
        assert r["re"] == 1 and r["im"] == 2

    def test_ignores_d_position(self, page):
        r = _call_interp(page, 0, 0, 100, 200, math.pi / 2, "c-node")
        assert r["re"] == 0 and r["im"] == 0


# ── d-node path ──────────────────────────────────────────────────────

class TestMorphInterpDNode:
    """pathType='d-node' always returns the D-node position."""

    def test_returns_d_at_theta_zero(self, page):
        r = _call_interp(page, 1, 2, 5, 6, 0, "d-node")
        assert r["re"] == 5 and r["im"] == 6

    def test_returns_d_at_theta_pi(self, page):
        r = _call_interp(page, 1, 2, 5, 6, math.pi, "d-node")
        assert r["re"] == 5 and r["im"] == 6

    def test_ignores_c_position(self, page):
        r = _call_interp(page, 100, 200, 3, 4, math.pi / 2, "d-node")
        assert r["re"] == 3 and r["im"] == 4


# ── c+d path ─────────────────────────────────────────────────────────

class TestMorphInterpCPlusD:
    """pathType='c+d' returns the sum C + D."""

    def test_sum_basic(self, page):
        r = _call_interp(page, 1, 2, 3, 4, 0, "c+d")
        assert r["re"] == 4 and r["im"] == 6

    def test_sum_at_arbitrary_theta(self, page):
        r = _call_interp(page, -1, 0.5, 2, -0.5, 1.23, "c+d")
        assert abs(r["re"] - 1) < 1e-12 and abs(r["im"]) < 1e-12


# ── line path ────────────────────────────────────────────────────────

class TestMorphInterpLine:
    """pathType='line': mu = (1-cos(theta))/2, lerp(C, D, mu)."""

    def test_at_theta_zero_returns_c(self, page):
        """theta=0 ⇒ mu=0 ⇒ position = C."""
        r = _call_interp(page, 1, 0, 3, 0, 0, "line")
        assert abs(r["re"] - 1) < 1e-12 and abs(r["im"]) < 1e-12

    def test_at_theta_pi_returns_d(self, page):
        """theta=π ⇒ mu=1 ⇒ position = D."""
        r = _call_interp(page, 1, 0, 3, 0, math.pi, "line")
        assert abs(r["re"] - 3) < 1e-12 and abs(r["im"]) < 1e-12

    def test_at_theta_half_pi_returns_midpoint(self, page):
        """theta=π/2 ⇒ mu=0.5 ⇒ midpoint."""
        r = _call_interp(page, 0, 0, 4, 0, math.pi / 2, "line")
        assert abs(r["re"] - 2) < 1e-12 and abs(r["im"]) < 1e-12

    def test_at_theta_two_pi_returns_c(self, page):
        """theta=2π ⇒ mu=0 ⇒ back to C (full cycle)."""
        r = _call_interp(page, 1, 0, 3, 0, 2 * math.pi, "line")
        assert abs(r["re"] - 1) < 1e-12

    def test_complex_positions(self, page):
        """Line path with complex C and D."""
        cRe, cIm, dRe, dIm = 1, 2, 5, 6
        theta = math.pi / 3
        mu = 0.5 - 0.5 * math.cos(theta)
        expected_re = cRe * (1 - mu) + dRe * mu
        expected_im = cIm * (1 - mu) + dIm * mu
        r = _call_interp(page, cRe, cIm, dRe, dIm, theta, "line")
        assert abs(r["re"] - expected_re) < 1e-10
        assert abs(r["im"] - expected_im) < 1e-10


# ── circle path ──────────────────────────────────────────────────────

class TestMorphInterpCircle:
    """pathType='circle': C-D segment is the diameter.
    M = m + (-a*cos(theta))*u + (sign*a*sin(theta))*v
    """

    def test_at_theta_zero_returns_c(self, page):
        """theta=0 ⇒ at C position."""
        r = _call_interp(page, 0, 0, 4, 0, 0, "circle")
        assert abs(r["re"]) < 1e-12 and abs(r["im"]) < 1e-12

    def test_at_theta_pi_returns_d(self, page):
        """theta=π ⇒ at D position."""
        r = _call_interp(page, 0, 0, 4, 0, math.pi, "circle")
        assert abs(r["re"] - 4) < 1e-10 and abs(r["im"]) < 1e-10

    def test_at_theta_half_pi_at_circle_top(self, page):
        """theta=π/2: midpoint + perpendicular offset.
        C=(0,0), D=(4,0): m=(2,0), a=2, u=(1,0), v=(0,1)
        lx=-2*cos(π/2)=0, ly=-1*2*sin(π/2)=-2 (CW, sign=-1)
        position = (2, -2)
        """
        r = _call_interp(page, 0, 0, 4, 0, math.pi / 2, "circle", ccw=False)
        assert abs(r["re"] - 2) < 1e-10
        assert abs(r["im"] - (-2)) < 1e-10

    def test_ccw_reverses_perpendicular(self, page):
        """CCW flag should reverse the perpendicular direction."""
        r = _call_interp(page, 0, 0, 4, 0, math.pi / 2, "circle", ccw=True)
        assert abs(r["re"] - 2) < 1e-10
        assert abs(r["im"] - 2) < 1e-10  # +2 instead of -2

    def test_full_cycle_returns_to_c(self, page):
        """theta=2π ⇒ back to C."""
        r = _call_interp(page, 1, 1, 5, 1, 2 * math.pi, "circle")
        assert abs(r["re"] - 1) < 1e-10
        assert abs(r["im"] - 1) < 1e-10

    def test_formula_arbitrary(self, page):
        """Verify against exact formula for arbitrary values."""
        cRe, cIm, dRe, dIm = 1, 2, 5, 6
        theta = 1.3
        dx, dy = dRe - cRe, dIm - cIm
        length = math.sqrt(dx * dx + dy * dy)
        ux, uy = dx / length, dy / length
        vx, vy = -uy, ux
        midRe = (cRe + dRe) * 0.5
        midIm = (cIm + dIm) * 0.5
        semi = length * 0.5
        sign = -1  # CW (ccw=False)
        lx = -semi * math.cos(theta)
        ly = sign * semi * math.sin(theta)
        expected_re = midRe + lx * ux + ly * vx
        expected_im = midIm + lx * uy + ly * vy
        r = _call_interp(page, cRe, cIm, dRe, dIm, theta, "circle", ccw=False)
        assert abs(r["re"] - expected_re) < 1e-10
        assert abs(r["im"] - expected_im) < 1e-10


# ── ellipse path ─────────────────────────────────────────────────────

class TestMorphInterpEllipse:
    """pathType='ellipse': semi_b = minorPct * semi_a.
    M = m + (-a*cos(theta))*u + (sign*b*sin(theta))*v
    """

    def test_at_theta_zero_returns_c(self, page):
        r = _call_interp(page, 0, 0, 4, 0, 0, "ellipse", minor=0.5)
        assert abs(r["re"]) < 1e-12 and abs(r["im"]) < 1e-12

    def test_at_theta_pi_returns_d(self, page):
        r = _call_interp(page, 0, 0, 4, 0, math.pi, "ellipse", minor=0.5)
        assert abs(r["re"] - 4) < 1e-10

    def test_minor_axis_half(self, page):
        """theta=π/2: perpendicular offset is semi_b = 0.5*semi_a.
        C=(0,0), D=(4,0): semi=2, semi_b=1
        lx=0, ly=-1*1*1=-1 (CW, sign=-1)
        position = (2, -1)
        """
        r = _call_interp(page, 0, 0, 4, 0, math.pi / 2, "ellipse", ccw=False, minor=0.5)
        assert abs(r["re"] - 2) < 1e-10
        assert abs(r["im"] - (-1)) < 1e-10

    def test_minor_equals_one_is_circle(self, page):
        """When minorPct=1, ellipse = circle."""
        rc = _call_interp(page, 0, 0, 4, 0, 1.0, "circle")
        re = _call_interp(page, 0, 0, 4, 0, 1.0, "ellipse", minor=1.0)
        assert abs(rc["re"] - re["re"]) < 1e-12
        assert abs(rc["im"] - re["im"]) < 1e-12


# ── figure-8 path ────────────────────────────────────────────────────

class TestMorphInterpFigure8:
    """pathType='figure8': Lissajous 1:2.
    lx = -a*cos(theta), ly = sign*(a/2)*sin(2*theta)
    """

    def test_at_theta_zero_returns_c(self, page):
        r = _call_interp(page, 0, 0, 4, 0, 0, "figure8")
        assert abs(r["re"]) < 1e-12

    def test_at_theta_pi_returns_d(self, page):
        r = _call_interp(page, 0, 0, 4, 0, math.pi, "figure8")
        assert abs(r["re"] - 4) < 1e-10

    def test_at_midpoint_crosses_center(self, page):
        """theta=π/2: lx=0, ly=sin(π)=0 ⇒ at midpoint exactly."""
        r = _call_interp(page, 0, 0, 4, 0, math.pi / 2, "figure8")
        assert abs(r["re"] - 2) < 1e-10
        assert abs(r["im"]) < 1e-10

    def test_at_3pi_2_also_at_center(self, page):
        """theta=3π/2: lx=0, ly=sin(3π)=0 ⇒ at midpoint again."""
        r = _call_interp(page, 0, 0, 4, 0, 3 * math.pi / 2, "figure8")
        assert abs(r["re"] - 2) < 1e-10
        assert abs(r["im"]) < 1e-10

    def test_max_perpendicular_at_pi_4(self, page):
        """theta=π/4: ly = sign*(a/2)*sin(π/2) = ±a/2.
        C=(0,0), D=(4,0): semi=2, a/2=1
        CW (sign=-1): ly = -1
        position = (2-2*cos(π/4), -1*(-sin(π/4))) = ...
        """
        theta = math.pi / 4
        r = _call_interp(page, 0, 0, 4, 0, theta, "figure8", ccw=False)
        semi = 2.0
        sign = -1  # CW
        expected_re = 2 + (-semi * math.cos(theta))
        expected_im = sign * (semi / 2) * math.sin(2 * theta)
        assert abs(r["re"] - expected_re) < 1e-10
        assert abs(r["im"] - expected_im) < 1e-10

    def test_ccw_reverses(self, page):
        """CCW flips the perpendicular component."""
        theta = math.pi / 4
        rcw = _call_interp(page, 0, 0, 4, 0, theta, "figure8", ccw=False)
        rccw = _call_interp(page, 0, 0, 4, 0, theta, "figure8", ccw=True)
        # re should be the same (lx is independent of sign)
        assert abs(rcw["re"] - rccw["re"]) < 1e-12
        # im should be opposite
        assert abs(rcw["im"] + rccw["im"]) < 1e-12


# ── degenerate: C ≈ D ────────────────────────────────────────────────

class TestMorphInterpDegenerate:
    """When C ≈ D (len < 1e-15), all paths should return C."""

    def test_circle_degenerate(self, page):
        r = _call_interp(page, 3, 4, 3, 4, math.pi / 2, "circle")
        assert abs(r["re"] - 3) < 1e-12 and abs(r["im"] - 4) < 1e-12

    def test_figure8_degenerate(self, page):
        r = _call_interp(page, 3, 4, 3, 4, math.pi / 2, "figure8")
        assert abs(r["re"] - 3) < 1e-12 and abs(r["im"] - 4) < 1e-12

    def test_line_degenerate(self, page):
        r = _call_interp(page, 3, 4, 3, 4, math.pi / 2, "line")
        assert abs(r["re"] - 3) < 1e-12 and abs(r["im"] - 4) < 1e-12
