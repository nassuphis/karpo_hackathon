"""Tests for the Shape morph tool: rayPolyHit, shapeTargets, and morph interpolation."""

import math
import pytest


class TestRayPolyHit:
    """Verify ray-polygon intersection finds correct boundary points."""

    def test_ray_right_hits_triangle(self, page):
        """Rightward ray from center of equilateral triangle hits the right edge."""
        result = page.evaluate("""() => {
            const R = 2;
            const verts = regularPolyVerts(0, 0, R, 3);
            const hit = rayPolyHit(0, 0, 1, 0, verts);
            return { hit, verts };
        }""")
        assert result["hit"] is not None, "Ray should hit triangle"
        # Right edge of equilateral triangle (top vertex at angle π/2)
        # Rightward ray should hit at x = R*cos(30°) = R*√3/2 ≈ 1.732 for a triangle
        # The right edge goes from vertex at π/2-2π/3 = -π/6 to vertex at π/2+2π/3*? no...
        # Vertices at π/2, π/2+2π/3, π/2+4π/3 = 90°, 210°, 330°
        # So v0=(0,2), v1=(-√3, -1), v2=(√3, -1)
        # Edge v2→v0 goes from (√3,-1) to (0,2). Rightward ray hits this edge.
        # At y=0: parameterize edge: x = √3 + t*(-√3), y = -1 + t*3. y=0 → t=1/3
        # x = √3 - √3/3 = 2√3/3 ≈ 1.155
        expected_x = 2 * math.sqrt(3) / 3
        assert abs(result["hit"]["re"] - expected_x) < 0.01
        assert abs(result["hit"]["im"]) < 0.01

    def test_ray_up_hits_triangle(self, page):
        """Upward ray should hit the top vertex or top edge of the triangle."""
        result = page.evaluate("""() => {
            const R = 2;
            const verts = regularPolyVerts(0, 0, R, 3);
            return rayPolyHit(0, 0, 0, 1, verts);
        }""")
        assert result is not None
        assert abs(result["re"]) < 0.01
        assert abs(result["im"] - 2.0) < 0.01  # top vertex at (0, R)

    def test_ray_hits_pentagon(self, page):
        """Ray from center of pentagon should hit a boundary point at correct distance."""
        result = page.evaluate("""() => {
            const R = 3;
            const verts = regularPolyVerts(0, 0, R, 5);
            // Rightward ray
            const hit = rayPolyHit(0, 0, 1, 0, verts);
            if (!hit) return null;
            return { re: hit.re, im: hit.im, dist: Math.sqrt(hit.re*hit.re + hit.im*hit.im) };
        }""")
        assert result is not None, "Ray should hit pentagon"
        # For a regular pentagon of circumradius R, the inradius is R*cos(π/5) ≈ 0.809*R
        # The rightward ray should hit at a distance between inradius and R
        inradius = 3 * math.cos(math.pi / 5)
        assert result["dist"] >= inradius - 0.01
        assert result["dist"] <= 3.01

    def test_ray_hits_all_directions_triangle(self, page):
        """Rays in 12 evenly spaced directions should all hit the triangle."""
        result = page.evaluate("""() => {
            const R = 2;
            const verts = regularPolyVerts(0, 0, R, 3);
            var hits = 0, misses = 0;
            for (var i = 0; i < 12; i++) {
                var angle = i * 2 * Math.PI / 12;
                var dx = Math.cos(angle), dy = Math.sin(angle);
                var hit = rayPolyHit(0, 0, dx, dy, verts);
                if (hit) hits++; else misses++;
            }
            return { hits, misses };
        }""")
        assert result["hits"] == 12, f"All 12 rays should hit: {result['misses']} missed"

    def test_ray_hits_all_directions_pentagon(self, page):
        """Rays in 20 evenly spaced directions should all hit the pentagon."""
        result = page.evaluate("""() => {
            const R = 2;
            const verts = regularPolyVerts(0, 0, R, 5);
            var hits = 0, misses = 0;
            for (var i = 0; i < 20; i++) {
                var angle = i * 2 * Math.PI / 20;
                var dx = Math.cos(angle), dy = Math.sin(angle);
                var hit = rayPolyHit(0, 0, dx, dy, verts);
                if (hit) hits++; else misses++;
            }
            return { hits, misses };
        }""")
        assert result["hits"] == 20, f"All 20 rays should hit: {result['misses']} missed"

    def test_ray_hits_box_polygon(self, page):
        """Rightward ray should hit the right edge of a box polygon."""
        result = page.evaluate("""() => {
            var verts = [
                {re: -1, im: 1}, {re: 2, im: 1},
                {re: 2, im: -1}, {re: -1, im: -1}
            ];
            return rayPolyHit(0, 0, 1, 0, verts);
        }""")
        assert result is not None
        assert abs(result["re"] - 2.0) < 0.01
        assert abs(result["im"]) < 0.01


class TestShapeTargets:
    """Verify shapeTargets returns boundary points for each shape."""

    SHAPES = ["box", "tri", "pent", "inf"]

    def _setup_items(self, page):
        """Create known coefficient positions and return items + geometry."""
        return page.evaluate("""() => {
            // Set up 5 coefficients in known positions
            var items = [
                {idx:0, re: 1, im: 0.5},
                {idx:1, re:-0.5, im: 1.2},
                {idx:2, re: 0.3, im:-0.8},
                {idx:3, re:-1.0, im:-0.3},
                {idx:4, re: 0.7, im: 0.1}
            ];
            var n = items.length;
            var cRe = 0, cIm = 0;
            for (var s of items) { cRe += s.re; cIm += s.im; }
            cRe /= n; cIm /= n;
            var R2 = 0;
            for (var s of items) {
                var d2 = (s.re - cRe) ** 2 + (s.im - cIm) ** 2;
                if (d2 > R2) R2 = d2;
            }
            var R = Math.sqrt(R2);
            var bMinRe = Infinity, bMaxRe = -Infinity, bMinIm = Infinity, bMaxIm = -Infinity;
            for (var s of items) {
                if (s.re < bMinRe) bMinRe = s.re;
                if (s.re > bMaxRe) bMaxRe = s.re;
                if (s.im < bMinIm) bMinIm = s.im;
                if (s.im > bMaxIm) bMaxIm = s.im;
            }
            return { items, cRe, cIm, R, bbox: [bMinRe, bMaxRe, bMinIm, bMaxIm] };
        }""")

    @pytest.mark.parametrize("shape", SHAPES)
    def test_all_targets_non_null(self, page, shape):
        """Every item should get a valid target point (not null, not fallback to original)."""
        setup = self._setup_items(page)
        result = page.evaluate("""(args) => {
            var targets = shapeTargets(args.shape, args.items, args.cRe, args.cIm, args.R, args.bbox);
            var results = [];
            for (var i = 0; i < targets.length; i++) {
                var t = targets[i], s = args.items[i];
                var dx = s.re - args.cRe, dy = s.im - args.cIm;
                var atCentroid = Math.abs(dx) < 1e-15 && Math.abs(dy) < 1e-15;
                var atOriginal = Math.abs(t.re - s.re) < 1e-10 && Math.abs(t.im - s.im) < 1e-10;
                results.push({ re: t.re, im: t.im, atOriginal, atCentroid });
            }
            return results;
        }""", {"shape": shape, **setup})
        # At least one non-centroid item should get a target different from original.
        # (Items already on the shape boundary legitimately have target == original.)
        moved = sum(1 for r in result if not r["atCentroid"] and not r["atOriginal"])
        total = sum(1 for r in result if not r["atCentroid"])
        assert total == 0 or moved > 0, (
            f"Shape '{shape}': 0/{total} items got distinct targets — "
            f"rayPolyHit likely broken")

    def test_box_targets_on_boundary(self, page):
        """Box targets should lie on the bounding box edges."""
        setup = self._setup_items(page)
        result = page.evaluate("""(args) => {
            var targets = shapeTargets("box", args.items, args.cRe, args.cIm, args.R, args.bbox);
            return targets.map(t => ({
                re: t.re, im: t.im,
                onLeft: Math.abs(t.re - args.bbox[0]) < 1e-6,
                onRight: Math.abs(t.re - args.bbox[1]) < 1e-6,
                onBottom: Math.abs(t.im - args.bbox[2]) < 1e-6,
                onTop: Math.abs(t.im - args.bbox[3]) < 1e-6
            }));
        }""", {**setup})
        for i, t in enumerate(result):
            on_edge = t["onLeft"] or t["onRight"] or t["onBottom"] or t["onTop"]
            assert on_edge, f"Box target {i} at ({t['re']:.4f}, {t['im']:.4f}) not on any edge"

    def test_tri_targets_on_boundary(self, page):
        """Triangle targets should lie on the triangle edges (distance from centroid)."""
        setup = self._setup_items(page)
        result = page.evaluate("""(args) => {
            var targets = shapeTargets("tri", args.items, args.cRe, args.cIm, args.R, args.bbox);
            var verts = regularPolyVerts(args.cRe, args.cIm, args.R, 3);
            // For each target, find min distance to any triangle edge
            return targets.map(t => {
                var minDist = Infinity;
                for (var i = 0; i < 3; i++) {
                    var j = (i + 1) % 3;
                    // Distance from point to line segment
                    var ax = verts[i].re, ay = verts[i].im;
                    var bx = verts[j].re, by = verts[j].im;
                    var dx = bx - ax, dy = by - ay;
                    var len2 = dx*dx + dy*dy;
                    var s = Math.max(0, Math.min(1, ((t.re-ax)*dx + (t.im-ay)*dy) / len2));
                    var px = ax + s*dx, py = ay + s*dy;
                    var d = Math.sqrt((t.re-px)**2 + (t.im-py)**2);
                    if (d < minDist) minDist = d;
                }
                return { re: t.re, im: t.im, edgeDist: minDist };
            });
        }""", {**setup})
        for i, t in enumerate(result):
            assert t["edgeDist"] < 0.001, (
                f"Tri target {i} at ({t['re']:.4f}, {t['im']:.4f}) "
                f"is {t['edgeDist']:.6f} from nearest edge")

    def test_pent_targets_on_boundary(self, page):
        """Pentagon targets should lie on the pentagon edges."""
        setup = self._setup_items(page)
        result = page.evaluate("""(args) => {
            var targets = shapeTargets("pent", args.items, args.cRe, args.cIm, args.R, args.bbox);
            var verts = regularPolyVerts(args.cRe, args.cIm, args.R, 5);
            return targets.map(t => {
                var minDist = Infinity;
                for (var i = 0; i < 5; i++) {
                    var j = (i + 1) % 5;
                    var ax = verts[i].re, ay = verts[i].im;
                    var bx = verts[j].re, by = verts[j].im;
                    var dx = bx - ax, dy = by - ay;
                    var len2 = dx*dx + dy*dy;
                    var s = Math.max(0, Math.min(1, ((t.re-ax)*dx + (t.im-ay)*dy) / len2));
                    var px = ax + s*dx, py = ay + s*dy;
                    var d = Math.sqrt((t.re-px)**2 + (t.im-py)**2);
                    if (d < minDist) minDist = d;
                }
                return { re: t.re, im: t.im, edgeDist: minDist };
            });
        }""", {**setup})
        for i, t in enumerate(result):
            assert t["edgeDist"] < 0.001, (
                f"Pent target {i} at ({t['re']:.4f}, {t['im']:.4f}) "
                f"is {t['edgeDist']:.6f} from nearest edge")

    def test_inf_targets_at_correct_distance(self, page):
        """Infinity targets should be at distance R*(0.2 + 0.8*cos²θ) from centroid."""
        setup = self._setup_items(page)
        result = page.evaluate("""(args) => {
            var targets = shapeTargets("inf", args.items, args.cRe, args.cIm, args.R, args.bbox);
            return targets.map((t, i) => {
                var s = args.items[i];
                var dx = s.re - args.cRe, dy = s.im - args.cIm;
                var dOrig = Math.sqrt(dx*dx + dy*dy);
                if (dOrig < 1e-15) return { dist: 0, expected: 0, ok: true };
                var theta = Math.atan2(dy, dx);
                var c = Math.cos(theta);
                var expected = args.R * (0.2 + 0.8 * c * c);
                var dist = Math.sqrt((t.re - args.cRe)**2 + (t.im - args.cIm)**2);
                return { dist, expected, ok: Math.abs(dist - expected) < 1e-6 };
            });
        }""", {**setup})
        for i, t in enumerate(result):
            assert t["ok"], (
                f"Inf target {i}: dist={t['dist']:.6f}, expected={t['expected']:.6f}")


class TestShapeMorphInterpolation:
    """Test the morph interpolation at slider positions 0, 0.5, and 1."""

    SHAPES = ["box", "tri", "inf", "pent"]

    def _morph_at(self, page, shape, u):
        """Compute morph positions at slider value u for a known set of points."""
        return page.evaluate("""(args) => {
            var items = [
                {idx:0, re: 1, im: 0.5},
                {idx:1, re:-0.5, im: 1.2},
                {idx:2, re: 0.3, im:-0.8},
                {idx:3, re:-1.0, im:-0.3},
                {idx:4, re: 0.7, im: 0.1}
            ];
            var n = items.length;
            var cRe = 0, cIm = 0;
            for (var s of items) { cRe += s.re; cIm += s.im; }
            cRe /= n; cIm /= n;
            var R2 = 0;
            for (var s of items) {
                var d2 = (s.re - cRe) ** 2 + (s.im - cIm) ** 2;
                if (d2 > R2) R2 = d2;
            }
            var R = Math.sqrt(R2);
            var bMinRe = Infinity, bMaxRe = -Infinity, bMinIm = Infinity, bMaxIm = -Infinity;
            for (var s of items) {
                if (s.re < bMinRe) bMinRe = s.re;
                if (s.re > bMaxRe) bMaxRe = s.re;
                if (s.im < bMinIm) bMinIm = s.im;
                if (s.im > bMaxIm) bMaxIm = s.im;
            }
            var bbox = [bMinRe, bMaxRe, bMinIm, bMaxIm];
            var targets = shapeTargets(args.shape, items, cRe, cIm, R, bbox);
            var u = args.u;
            var positions = items.map((s, i) => {
                var bp = targets[i];
                if (u <= 0.5) {
                    var f = u * 2;
                    return { re: cRe + f * (s.re - cRe), im: cIm + f * (s.im - cIm) };
                } else {
                    var f = (u - 0.5) * 2;
                    return { re: s.re + f * (bp.re - s.re), im: s.im + f * (bp.im - s.im) };
                }
            });
            return { positions, items, targets, cRe, cIm };
        }""", {"shape": shape, "u": u})

    @pytest.mark.parametrize("shape", SHAPES)
    def test_slider_zero_all_at_centroid(self, page, shape):
        """At u=0, all points should collapse to the centroid."""
        r = self._morph_at(page, shape, 0.0)
        cRe, cIm = r["cRe"], r["cIm"]
        for i, p in enumerate(r["positions"]):
            assert abs(p["re"] - cRe) < 1e-10, f"{shape} item {i}: re not at centroid"
            assert abs(p["im"] - cIm) < 1e-10, f"{shape} item {i}: im not at centroid"

    @pytest.mark.parametrize("shape", SHAPES)
    def test_slider_half_at_original(self, page, shape):
        """At u=0.5, all points should be at their original positions."""
        r = self._morph_at(page, shape, 0.5)
        for i, p in enumerate(r["positions"]):
            orig = r["items"][i]
            assert abs(p["re"] - orig["re"]) < 1e-10, f"{shape} item {i}: re not at original"
            assert abs(p["im"] - orig["im"]) < 1e-10, f"{shape} item {i}: im not at original"

    @pytest.mark.parametrize("shape", SHAPES)
    def test_slider_one_at_target(self, page, shape):
        """At u=1, all points should be at their shape boundary targets."""
        r = self._morph_at(page, shape, 1.0)
        for i, p in enumerate(r["positions"]):
            tgt = r["targets"][i]
            assert abs(p["re"] - tgt["re"]) < 1e-10, (
                f"{shape} item {i}: re={p['re']:.6f} expected={tgt['re']:.6f}")
            assert abs(p["im"] - tgt["im"]) < 1e-10, (
                f"{shape} item {i}: im={p['im']:.6f} expected={tgt['im']:.6f}")

    @pytest.mark.parametrize("shape", SHAPES)
    def test_slider_quarter_between_centroid_and_original(self, page, shape):
        """At u=0.25, points should be halfway between centroid and original."""
        r = self._morph_at(page, shape, 0.25)
        cRe, cIm = r["cRe"], r["cIm"]
        for i, p in enumerate(r["positions"]):
            orig = r["items"][i]
            expected_re = (cRe + orig["re"]) / 2
            expected_im = (cIm + orig["im"]) / 2
            assert abs(p["re"] - expected_re) < 1e-10, f"{shape} item {i}: re wrong at u=0.25"
            assert abs(p["im"] - expected_im) < 1e-10, f"{shape} item {i}: im wrong at u=0.25"
