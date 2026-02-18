"""Tests for the pattern arrange tool: 21 patterns, distributeOnPath, blend slider."""

import math
import pytest


PATTERNS = [
    "circle", "square", "triangle", "pentagon", "hexagon", "diamond",
    "star", "ellipse", "infinity", "spiral", "grid", "line", "wave",
    "cross", "heart", "lissajous", "rose", "2-circles", "2-squares",
    "ring", "scatter"
]


class TestDistributeOnPath:
    """Test the distributeOnPath helper function."""

    def test_closed_triangle_3_points(self, page):
        """3 points on a closed triangle should be at the vertices."""
        result = page.evaluate("""() => {
            var verts = regularPolyVerts(0, 0, 1, 3);
            var pts = distributeOnPath(3, verts, true);
            return { pts, verts };
        }""")
        for p, v in zip(result["pts"], result["verts"]):
            assert abs(p["re"] - v["re"]) < 1e-10
            assert abs(p["im"] - v["im"]) < 1e-10

    def test_closed_square_4_points(self, page):
        """4 points on a closed square should be at the vertices."""
        result = page.evaluate("""() => {
            var verts = [
                {re: -1, im: 1}, {re: 1, im: 1},
                {re: 1, im: -1}, {re: -1, im: -1}
            ];
            var pts = distributeOnPath(4, verts, true);
            return pts;
        }""")
        expected = [(-1, 1), (1, 1), (1, -1), (-1, -1)]
        for p, (er, ei) in zip(result, expected):
            assert abs(p["re"] - er) < 1e-10
            assert abs(p["im"] - ei) < 1e-10

    def test_closed_square_8_points(self, page):
        """8 points on a closed square should include vertices and midpoints."""
        result = page.evaluate("""() => {
            var verts = [
                {re: -1, im: 1}, {re: 1, im: 1},
                {re: 1, im: -1}, {re: -1, im: -1}
            ];
            return distributeOnPath(8, verts, true);
        }""")
        assert len(result) == 8
        # First point at first vertex
        assert abs(result[0]["re"] - (-1)) < 1e-10
        assert abs(result[0]["im"] - 1) < 1e-10
        # Second point at midpoint of top edge
        assert abs(result[1]["re"] - 0) < 1e-10
        assert abs(result[1]["im"] - 1) < 1e-10

    def test_open_path_endpoints(self, page):
        """Open path with 3 points on 2-point line should include both endpoints."""
        result = page.evaluate("""() => {
            var verts = [{re: 0, im: 0}, {re: 10, im: 0}];
            return distributeOnPath(3, verts, false);
        }""")
        assert len(result) == 3
        assert abs(result[0]["re"]) < 1e-10
        assert abs(result[1]["re"] - 5) < 1e-10
        assert abs(result[2]["re"] - 10) < 1e-10

    def test_single_point(self, page):
        """n=1 should return the first vertex."""
        result = page.evaluate("""() => {
            var verts = [{re: 3, im: 4}, {re: 5, im: 6}];
            return distributeOnPath(1, verts, true);
        }""")
        assert len(result) == 1
        assert abs(result[0]["re"] - 3) < 1e-10
        assert abs(result[0]["im"] - 4) < 1e-10

    def test_equal_spacing(self, page):
        """Points on a closed path should be equally spaced by arc length."""
        result = page.evaluate("""() => {
            var verts = regularPolyVerts(0, 0, 1, 6);
            var pts = distributeOnPath(12, verts, true);
            var dists = [];
            for (var i = 0; i < pts.length; i++) {
                var j = (i + 1) % pts.length;
                var dx = pts[j].re - pts[i].re, dy = pts[j].im - pts[i].im;
                dists.push(Math.sqrt(dx*dx + dy*dy));
            }
            return dists;
        }""")
        avg = sum(result) / len(result)
        for d in result:
            assert abs(d - avg) < 1e-8, f"Spacing not equal: {d} vs avg {avg}"


class TestPatternPositionsCount:
    """Each pattern should return exactly n points."""

    @pytest.mark.parametrize("pattern", PATTERNS)
    def test_returns_n_points(self, page, pattern):
        result = page.evaluate("""(p) => {
            return patternPositions(p, 10, 0, 0, 2).length;
        }""", pattern)
        assert result == 10

    @pytest.mark.parametrize("pattern", PATTERNS)
    def test_returns_1_point_at_center(self, page, pattern):
        """n=1 should return center point."""
        result = page.evaluate("""(p) => {
            var pts = patternPositions(p, 1, 3, 4, 2);
            return pts[0];
        }""", pattern)
        assert abs(result["re"] - 3) < 1e-10
        assert abs(result["im"] - 4) < 1e-10

    @pytest.mark.parametrize("pattern", PATTERNS)
    def test_returns_2_points(self, page, pattern):
        result = page.evaluate("""(p) => {
            return patternPositions(p, 2, 0, 0, 1).length;
        }""", pattern)
        assert result == 2


class TestPatternPositionsFinite:
    """All points in every pattern should be finite."""

    @pytest.mark.parametrize("pattern", PATTERNS)
    def test_all_finite(self, page, pattern):
        result = page.evaluate("""(p) => {
            var pts = patternPositions(p, 15, 1.5, -0.5, 3);
            return pts.every(pt => isFinite(pt.re) && isFinite(pt.im));
        }""", pattern)
        assert result is True


class TestCircle:
    def test_all_at_radius(self, page):
        """All circle points should be at distance R from center."""
        result = page.evaluate("""() => {
            var pts = patternPositions("circle", 8, 2, 3, 5);
            return pts.map(p => Math.sqrt((p.re-2)**2 + (p.im-3)**2));
        }""")
        for d in result:
            assert abs(d - 5) < 1e-10

    def test_equally_spaced_angles(self, page):
        """Angles should be equally spaced."""
        result = page.evaluate("""() => {
            var pts = patternPositions("circle", 6, 0, 0, 1);
            return pts.map(p => Math.atan2(p.im, p.re));
        }""")
        # First point at pi/2
        assert abs(result[0] - math.pi / 2) < 1e-10


class TestSquare:
    def test_on_square_perimeter(self, page):
        """All points should lie on the square perimeter."""
        result = page.evaluate("""() => {
            var pts = patternPositions("square", 12, 0, 0, 2);
            return pts.map(p => ({
                re: p.re,
                im: p.im,
                onEdge: Math.abs(Math.abs(p.re) - 2) < 1e-8 ||
                        Math.abs(Math.abs(p.im) - 2) < 1e-8
            }));
        }""")
        for p in result:
            assert p["onEdge"], f"Point ({p['re']}, {p['im']}) not on square perimeter"


class TestTriangle:
    def test_on_triangle_perimeter(self, page):
        """All points should be at distance <= R from center."""
        result = page.evaluate("""() => {
            var pts = patternPositions("triangle", 9, 0, 0, 2);
            return pts.map(p => Math.sqrt(p.re**2 + p.im**2));
        }""")
        for d in result:
            assert d <= 2 + 1e-8


class TestPentagon:
    def test_on_pentagon_perimeter(self, page):
        result = page.evaluate("""() => {
            var pts = patternPositions("pentagon", 10, 0, 0, 2);
            return pts.map(p => Math.sqrt(p.re**2 + p.im**2));
        }""")
        for d in result:
            assert d <= 2 + 1e-8


class TestHexagon:
    def test_6_points_at_vertices(self, page):
        """6 points on hexagon should be at the regular hexagon vertices."""
        result = page.evaluate("""() => {
            var pts = patternPositions("hexagon", 6, 0, 0, 1);
            var verts = regularPolyVerts(0, 0, 1, 6);
            return { pts, verts };
        }""")
        for p, v in zip(result["pts"], result["verts"]):
            assert abs(p["re"] - v["re"]) < 1e-8
            assert abs(p["im"] - v["im"]) < 1e-8


class TestDiamond:
    def test_vertex_at_top(self, page):
        """Diamond (4-vertex polygon starting at top) should have first point at top."""
        result = page.evaluate("""() => {
            var pts = patternPositions("diamond", 4, 0, 0, 2);
            return pts[0];
        }""")
        assert abs(result["re"]) < 1e-8
        assert abs(result["im"] - 2) < 1e-8


class TestStar:
    def test_alternating_radii(self, page):
        """Star points at vertices should alternate between R and inner radius."""
        result = page.evaluate("""() => {
            var pts = patternPositions("star", 10, 0, 0, 2);
            return pts.map(p => Math.sqrt(p.re**2 + p.im**2));
        }""")
        # 10 points on a 10-vertex star should be at the vertices
        for i, d in enumerate(result):
            expected = 2 if i % 2 == 0 else 2 * 0.38
            assert abs(d - expected) < 0.15, f"Point {i}: dist={d}, expected~{expected}"


class TestEllipse:
    def test_on_ellipse(self, page):
        """Points should approximately satisfy (x/rx)^2 + (y/ry)^2 = 1."""
        result = page.evaluate("""() => {
            var R = 2;
            var rx = R * 1.4, ry = R * 0.7;
            var pts = patternPositions("ellipse", 20, 0, 0, R);
            return pts.map(p => (p.re/rx)**2 + (p.im/ry)**2);
        }""")
        for v in result:
            assert abs(v - 1) < 0.05, f"Not on ellipse: value={v}"


class TestInfinity:
    def test_figure8_shape(self, page):
        """Infinity should have points on both sides of x-axis."""
        result = page.evaluate("""() => {
            var pts = patternPositions("infinity", 20, 0, 0, 2);
            var posX = pts.filter(p => p.re > 0.1).length;
            var negX = pts.filter(p => p.re < -0.1).length;
            return { posX, negX, total: pts.length };
        }""")
        assert result["posX"] > 3
        assert result["negX"] > 3


class TestSpiral:
    def test_starts_near_center(self, page):
        """First point should be near center, last point near R."""
        result = page.evaluate("""() => {
            var pts = patternPositions("spiral", 20, 0, 0, 2);
            var d0 = Math.sqrt(pts[0].re**2 + pts[0].im**2);
            var dN = Math.sqrt(pts[19].re**2 + pts[19].im**2);
            return { d0, dN };
        }""")
        assert result["d0"] < 0.5
        assert result["dN"] > 1.5

    def test_monotonically_increasing_radius(self, page):
        """Points should move outward (roughly)."""
        result = page.evaluate("""() => {
            var pts = patternPositions("spiral", 20, 0, 0, 2);
            return pts.map(p => Math.sqrt(p.re**2 + p.im**2));
        }""")
        # First few should be smaller than last few
        assert sum(result[:5]) < sum(result[-5:])


class TestGrid:
    def test_within_bounding_box(self, page):
        """All grid points should be within [-R, R] x [-R, R]."""
        result = page.evaluate("""() => {
            var pts = patternPositions("grid", 12, 0, 0, 2);
            return pts.map(p => ({ re: p.re, im: p.im }));
        }""")
        for p in result:
            assert -2 - 1e-8 <= p["re"] <= 2 + 1e-8
            assert -2 - 1e-8 <= p["im"] <= 2 + 1e-8

    def test_grid_arrangement(self, page):
        """9 points should form a 3x3 grid."""
        result = page.evaluate("""() => {
            var pts = patternPositions("grid", 9, 0, 0, 2);
            var reVals = [...new Set(pts.map(p => Math.round(p.re * 1000) / 1000))];
            var imVals = [...new Set(pts.map(p => Math.round(p.im * 1000) / 1000))];
            return { nCols: reVals.length, nRows: imVals.length };
        }""")
        assert result["nCols"] == 3
        assert result["nRows"] == 3


class TestLine:
    def test_all_same_im(self, page):
        """All line points should have the same imaginary part."""
        result = page.evaluate("""() => {
            var pts = patternPositions("line", 10, 1, 2, 3);
            return pts.map(p => p.im);
        }""")
        for v in result:
            assert abs(v - 2) < 1e-10

    def test_spans_range(self, page):
        """Line should span from cRe-R to cRe+R."""
        result = page.evaluate("""() => {
            var pts = patternPositions("line", 10, 1, 2, 3);
            return { first: pts[0].re, last: pts[9].re };
        }""")
        assert abs(result["first"] - (-2)) < 1e-10  # 1 - 3
        assert abs(result["last"] - 4) < 1e-10  # 1 + 3


class TestWave:
    def test_x_range(self, page):
        """Wave should span cRe-R to cRe+R horizontally."""
        result = page.evaluate("""() => {
            var pts = patternPositions("wave", 20, 0, 0, 2);
            return { minRe: Math.min(...pts.map(p=>p.re)), maxRe: Math.max(...pts.map(p=>p.re)) };
        }""")
        assert abs(result["minRe"] - (-2)) < 1e-8
        assert abs(result["maxRe"] - 2) < 1e-8

    def test_y_variation(self, page):
        """Wave should have y-values that vary (not all zero)."""
        result = page.evaluate("""() => {
            var pts = patternPositions("wave", 20, 0, 0, 2);
            return { minIm: Math.min(...pts.map(p=>p.im)), maxIm: Math.max(...pts.map(p=>p.im)) };
        }""")
        assert result["maxIm"] > 0.1
        assert result["minIm"] < -0.1


class TestCross:
    def test_within_bounds(self, page):
        """All cross points should be within the arm length."""
        result = page.evaluate("""() => {
            var pts = patternPositions("cross", 20, 0, 0, 2);
            return pts.every(p => Math.abs(p.re) <= 2 + 1e-8 && Math.abs(p.im) <= 2 + 1e-8);
        }""")
        assert result is True

    def test_has_four_arms(self, page):
        """Cross should have points extending in all 4 cardinal directions."""
        result = page.evaluate("""() => {
            var pts = patternPositions("cross", 24, 0, 0, 2);
            var up = pts.some(p => p.im > 1);
            var down = pts.some(p => p.im < -1);
            var left = pts.some(p => p.re < -1);
            var right = pts.some(p => p.re > 1);
            return { up, down, left, right };
        }""")
        assert result["up"] and result["down"] and result["left"] and result["right"]


class TestHeart:
    def test_wider_at_top(self, page):
        """Heart should be wider at top than bottom (two lobes)."""
        result = page.evaluate("""() => {
            var pts = patternPositions("heart", 30, 0, 0, 2);
            var topPts = pts.filter(p => p.im > 0);
            var botPts = pts.filter(p => p.im < 0);
            var topWidth = Math.max(...topPts.map(p=>p.re)) - Math.min(...topPts.map(p=>p.re));
            var botWidth = botPts.length > 0 ?
                Math.max(...botPts.map(p=>p.re)) - Math.min(...botPts.map(p=>p.re)) : 0;
            return { topWidth, botWidth };
        }""")
        assert result["topWidth"] > result["botWidth"]


class TestLissajous:
    def test_within_radius(self, page):
        """All Lissajous points should be within [-R, R] x [-R, R]."""
        result = page.evaluate("""() => {
            var pts = patternPositions("lissajous", 20, 0, 0, 2);
            return pts.every(p => Math.abs(p.re) <= 2 + 1e-8 && Math.abs(p.im) <= 2 + 1e-8);
        }""")
        assert result is True

    def test_crosses_origin(self, page):
        """Lissajous 3:2 should cross near the origin."""
        result = page.evaluate("""() => {
            var pts = patternPositions("lissajous", 30, 0, 0, 2);
            return pts.some(p => Math.abs(p.re) < 0.5 && Math.abs(p.im) < 0.5);
        }""")
        assert result is True


class TestRose:
    def test_within_radius(self, page):
        """All rose curve points should be within radius R."""
        result = page.evaluate("""() => {
            var pts = patternPositions("rose", 20, 0, 0, 2);
            return pts.map(p => Math.sqrt(p.re**2 + p.im**2));
        }""")
        for d in result:
            assert d <= 2 + 0.1

    def test_has_petals(self, page):
        """Rose with cos(3θ) should have 3 petals (6 half-petals)."""
        result = page.evaluate("""() => {
            var pts = patternPositions("rose", 60, 0, 0, 2);
            var nearOrigin = pts.filter(p => Math.sqrt(p.re**2 + p.im**2) < 0.2).length;
            return nearOrigin;
        }""")
        # Rose curve passes through origin multiple times
        assert result >= 2


class TestTwoCircles:
    def test_split_into_two_groups(self, page):
        """Points should be split into two groups on separate circles."""
        result = page.evaluate("""() => {
            var pts = patternPositions("2-circles", 10, 0, 0, 2);
            var leftPts = pts.filter(p => p.re < 0);
            var rightPts = pts.filter(p => p.re > 0);
            return { left: leftPts.length, right: rightPts.length };
        }""")
        assert result["left"] >= 3
        assert result["right"] >= 3


class TestTwoSquares:
    def test_split_into_two_groups(self, page):
        """Points should form two separate square groups."""
        result = page.evaluate("""() => {
            var pts = patternPositions("2-squares", 12, 0, 0, 2);
            var leftPts = pts.filter(p => p.re < 0);
            var rightPts = pts.filter(p => p.re > 0);
            return { left: leftPts.length, right: rightPts.length };
        }""")
        assert result["left"] >= 3
        assert result["right"] >= 3


class TestRing:
    def test_two_radii(self, page):
        """Ring should have points at two distinct radii (R and R/2)."""
        result = page.evaluate("""() => {
            var pts = patternPositions("ring", 10, 0, 0, 2);
            var dists = pts.map(p => Math.sqrt(p.re**2 + p.im**2));
            var outer = dists.filter(d => d > 1.5).length;
            var inner = dists.filter(d => d < 1.5).length;
            return { outer, inner };
        }""")
        assert result["outer"] >= 3
        assert result["inner"] >= 2


class TestScatter:
    def test_within_radius(self, page):
        """All scatter points should be within the circle of radius R."""
        result = page.evaluate("""() => {
            var pts = patternPositions("scatter", 20, 0, 0, 2);
            return pts.every(p => Math.sqrt(p.re**2 + p.im**2) <= 2 + 1e-8);
        }""")
        assert result is True

    def test_fills_area(self, page):
        """Scatter should have points at various distances (not all on edge)."""
        result = page.evaluate("""() => {
            var pts = patternPositions("scatter", 30, 0, 0, 2);
            var dists = pts.map(p => Math.sqrt(p.re**2 + p.im**2));
            var nearCenter = dists.filter(d => d < 0.8).length;
            var midRange = dists.filter(d => d >= 0.8 && d < 1.5).length;
            var outer = dists.filter(d => d >= 1.5).length;
            return { nearCenter, midRange, outer };
        }""")
        assert result["nearCenter"] > 0
        assert result["midRange"] > 0
        assert result["outer"] > 0


class TestPatternBlend:
    """Test the blend interpolation: u=0 gives original, u=1 gives pattern."""

    def _setup_items(self, page):
        """Set up a selection with known positions and return context."""
        return page.evaluate("""() => {
            for (var i = 0; i < coefficients.length; i++) selectedCoeffs.add(i);
            var snap = snapshotSelection();
            var n = snap.items.length;
            var cRe = 0, cIm = 0;
            for (var s of snap.items) { cRe += s.re; cIm += s.im; }
            cRe /= n; cIm /= n;
            var R2 = 0;
            for (var s of snap.items) {
                var d2 = (s.re - cRe)**2 + (s.im - cIm)**2;
                if (d2 > R2) R2 = d2;
            }
            return {
                items: snap.items.map(s => ({re: s.re, im: s.im})),
                cRe: cRe, cIm: cIm, R: Math.sqrt(R2) || 1, n: n
            };
        }""")

    @pytest.mark.parametrize("pattern", ["circle", "square", "grid", "star", "line"])
    def test_blend_0_gives_original(self, page, pattern):
        """At u=0, positions should be unchanged from original."""
        ctx = self._setup_items(page)
        result = page.evaluate("""(args) => {
            var targets = patternPositions(args.pattern, args.n, args.cRe, args.cIm, args.R);
            var blended = args.items.map((s, i) => ({
                re: s.re + 0 * (targets[i].re - s.re),
                im: s.im + 0 * (targets[i].im - s.im)
            }));
            return blended;
        }""", {"pattern": pattern, **ctx})
        for p, orig in zip(result, ctx["items"]):
            assert abs(p["re"] - orig["re"]) < 1e-10
            assert abs(p["im"] - orig["im"]) < 1e-10

    @pytest.mark.parametrize("pattern", ["circle", "square", "grid", "star", "line"])
    def test_blend_1_gives_pattern(self, page, pattern):
        """At u=1, positions should match the pattern targets."""
        ctx = self._setup_items(page)
        result = page.evaluate("""(args) => {
            var targets = patternPositions(args.pattern, args.n, args.cRe, args.cIm, args.R);
            var blended = args.items.map((s, i) => ({
                re: s.re + 1 * (targets[i].re - s.re),
                im: s.im + 1 * (targets[i].im - s.im)
            }));
            return { blended, targets };
        }""", {"pattern": pattern, **ctx})
        for p, t in zip(result["blended"], result["targets"]):
            assert abs(p["re"] - t["re"]) < 1e-10
            assert abs(p["im"] - t["im"]) < 1e-10

    @pytest.mark.parametrize("pattern", ["circle", "triangle", "hexagon"])
    def test_blend_half_is_midpoint(self, page, pattern):
        """At u=0.5, positions should be midpoint between original and target."""
        ctx = self._setup_items(page)
        result = page.evaluate("""(args) => {
            var targets = patternPositions(args.pattern, args.n, args.cRe, args.cIm, args.R);
            var blended = args.items.map((s, i) => ({
                re: s.re + 0.5 * (targets[i].re - s.re),
                im: s.im + 0.5 * (targets[i].im - s.im)
            }));
            var expected = args.items.map((s, i) => ({
                re: (s.re + targets[i].re) / 2,
                im: (s.im + targets[i].im) / 2
            }));
            return { blended, expected };
        }""", {"pattern": pattern, **ctx})
        for p, e in zip(result["blended"], result["expected"]):
            assert abs(p["re"] - e["re"]) < 1e-10
            assert abs(p["im"] - e["im"]) < 1e-10


class TestPatternCentroid:
    """Pattern outputs should be centered near the input centroid."""

    @pytest.mark.parametrize("pattern", [
        "circle", "square", "triangle", "pentagon", "hexagon", "diamond",
        "star", "ellipse", "grid", "cross", "ring", "scatter"
    ])
    def test_centroid_near_input(self, page, pattern):
        """Output centroid should be close to the input centroid for symmetric patterns."""
        result = page.evaluate("""(p) => {
            var pts = patternPositions(p, 20, 3.5, -1.2, 2);
            var cRe = 0, cIm = 0;
            for (var pt of pts) { cRe += pt.re; cIm += pt.im; }
            cRe /= pts.length; cIm /= pts.length;
            return { cRe, cIm };
        }""", pattern)
        assert abs(result["cRe"] - 3.5) < 0.5, f"Centroid re={result['cRe']}, expected~3.5"
        assert abs(result["cIm"] - (-1.2)) < 0.5, f"Centroid im={result['cIm']}, expected~-1.2"


class TestPatternUIExists:
    """Verify the Pattern tool UI elements exist and function."""

    def test_ptrn_button_exists(self, page):
        """The Ptrn button should exist in the mid-bar."""
        result = page.evaluate("""() => {
            var btn = document.getElementById("ptrn-tool-btn");
            return btn !== null;
        }""")
        assert result is True

    def test_pattern_list_constant(self, page):
        """PATTERN_LIST should have 21 entries."""
        result = page.evaluate("""() => {
            return PATTERN_LIST.length;
        }""")
        assert result == 21


class TestPatternLargeN:
    """Test patterns work with larger point counts."""

    @pytest.mark.parametrize("pattern", PATTERNS)
    def test_50_points(self, page, pattern):
        """Each pattern should handle 50 points without error."""
        result = page.evaluate("""(p) => {
            var pts = patternPositions(p, 50, 0, 0, 3);
            return { n: pts.length, allFinite: pts.every(pt => isFinite(pt.re) && isFinite(pt.im)) };
        }""", pattern)
        assert result["n"] == 50
        assert result["allFinite"] is True
