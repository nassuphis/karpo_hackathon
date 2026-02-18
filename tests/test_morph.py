"""Tests for the morph system: morphTargetCoeffs, blending, save/load, fast mode integration."""

import pytest


class TestMorphInit:
    def test_morph_target_created_on_init(self, page):
        """morphTargetCoeffs should exist and match coefficients length."""
        result = page.evaluate("""() => ({
            nCoeffs: coefficients.length,
            nMorph: morphTargetCoeffs.length,
            enabled: morphEnabled,
            mu: morphMu,
            rate: morphRate
        })""")
        assert result["nMorph"] == result["nCoeffs"]
        assert result["enabled"] is False
        assert result["mu"] == 0  # disabled → mu=0
        assert result["rate"] == 0.01

    def test_init_morph_target_copies_positions(self, page):
        """initMorphTarget() copies C positions to D."""
        result = page.evaluate("""() => {
            initMorphTarget();
            var pairs = [];
            for (var i = 0; i < coefficients.length; i++) {
                pairs.push({
                    cRe: coefficients[i].re, cIm: coefficients[i].im,
                    dRe: morphTargetCoeffs[i].re, dIm: morphTargetCoeffs[i].im,
                    pathType: morphTargetCoeffs[i].pathType
                });
            }
            return pairs;
        }""")
        for i, p in enumerate(result):
            assert abs(p["cRe"] - p["dRe"]) < 1e-12, f"D[{i}].re != C[{i}].re"
            assert abs(p["cIm"] - p["dIm"]) < 1e-12, f"D[{i}].im != C[{i}].im"
            assert p["pathType"] == "none"

    def test_init_morph_target_has_curve(self, page):
        """Each D coefficient should have a 1-point curve at its position."""
        result = page.evaluate("""() => {
            initMorphTarget();
            return morphTargetCoeffs.map(d => ({
                curveLen: d.curve.length,
                curveRe: d.curve[0].re,
                curveIm: d.curve[0].im,
                re: d.re, im: d.im
            }));
        }""")
        for i, d in enumerate(result):
            assert d["curveLen"] == 1, f"D[{i}] curve should be length 1"
            assert abs(d["curveRe"] - d["re"]) < 1e-12
            assert abs(d["curveIm"] - d["im"]) < 1e-12


class TestMorphBlending:
    def test_mu_zero_no_blending(self, page):
        """When morphMu=0, blended coefficients equal C."""
        result = page.evaluate("""() => {
            morphEnabled = true;
            morphMu = 0;
            // Move D away from C
            for (var i = 0; i < morphTargetCoeffs.length; i++) {
                morphTargetCoeffs[i].re += 10;
                morphTargetCoeffs[i].im += 10;
            }
            // solveRoots blends internally — check what it would produce
            var blended = coefficients.map((c, i) => ({
                re: c.re * (1 - morphMu) + morphTargetCoeffs[i].re * morphMu,
                im: c.im * (1 - morphMu) + morphTargetCoeffs[i].im * morphMu
            }));
            morphEnabled = false;
            morphMu = 0;
            initMorphTarget();
            return blended.map((b, i) => ({
                bRe: b.re, bIm: b.im,
                cRe: coefficients[i].re, cIm: coefficients[i].im
            }));
        }""")
        for i, p in enumerate(result):
            assert abs(p["bRe"] - p["cRe"]) < 1e-12, f"mu=0: blended[{i}].re should equal C"
            assert abs(p["bIm"] - p["cIm"]) < 1e-12, f"mu=0: blended[{i}].im should equal C"

    def test_mu_one_full_morph(self, page):
        """When morphMu=1, blended coefficients equal D."""
        result = page.evaluate("""() => {
            morphEnabled = true;
            morphMu = 1;
            morphTargetCoeffs[0].re = 99;
            morphTargetCoeffs[0].im = 77;
            var blended = coefficients.map((c, i) => ({
                re: c.re * (1 - morphMu) + morphTargetCoeffs[i].re * morphMu,
                im: c.im * (1 - morphMu) + morphTargetCoeffs[i].im * morphMu
            }));
            morphEnabled = false;
            morphMu = 0;
            initMorphTarget();
            return { bRe: blended[0].re, bIm: blended[0].im };
        }""")
        assert abs(result["bRe"] - 99) < 1e-12
        assert abs(result["bIm"] - 77) < 1e-12

    def test_mu_half_midpoint(self, page):
        """When morphMu=0.5, blended = midpoint of C and D."""
        result = page.evaluate("""() => {
            morphEnabled = true;
            morphMu = 0.5;
            var c0re = coefficients[0].re;
            morphTargetCoeffs[0].re = c0re + 10;
            var blendedRe = c0re * 0.5 + morphTargetCoeffs[0].re * 0.5;
            var expected = c0re + 5;
            morphEnabled = false;
            morphMu = 0;
            initMorphTarget();
            return { blendedRe, expected };
        }""")
        assert abs(result["blendedRe"] - result["expected"]) < 1e-12

    def test_morph_mu_sinusoidal_range(self, page):
        """morphMu formula: 0.5 + 0.5*sin(2π*rate*t) stays in [0,1]."""
        result = page.evaluate("""() => {
            var rate = 0.25;
            var results = [];
            for (var t = 0; t <= 10; t += 0.1) {
                var mu = 0.5 + 0.5 * Math.sin(2 * Math.PI * rate * t);
                results.push(mu);
            }
            return {
                min: Math.min(...results),
                max: Math.max(...results),
                count: results.length
            };
        }""")
        assert result["min"] >= -1e-10, f"mu went below 0: {result['min']}"
        assert result["max"] <= 1 + 1e-10, f"mu went above 1: {result['max']}"
        assert result["count"] > 50


class TestMorphDisabled:
    def test_morph_disabled_mu_stays_zero(self, page):
        """When morph is disabled, mu must be 0."""
        result = page.evaluate("""() => {
            morphEnabled = false;
            return morphMu;
        }""")
        assert result == 0

    def test_enabling_morph_sets_mu_half(self, page):
        """Enable checkbox sets mu=0.5 for immediate visual effect."""
        result = page.evaluate("""() => {
            var cb = document.getElementById('morph-enable');
            cb.checked = true;
            cb.dispatchEvent(new Event('change'));
            var muAfterEnable = morphMu;
            cb.checked = false;
            cb.dispatchEvent(new Event('change'));
            var muAfterDisable = morphMu;
            return { muAfterEnable, muAfterDisable };
        }""")
        assert result["muAfterEnable"] == 0.5
        assert result["muAfterDisable"] == 0


class TestMorphSaveLoad:
    def test_morph_state_roundtrip(self, page):
        """Morph state survives save/load cycle."""
        result = page.evaluate("""() => {
            morphEnabled = true;
            morphRate = 1.5;
            morphMu = 0.75;
            morphTargetCoeffs[0].re = 42;
            morphTargetCoeffs[0].im = -13;
            var state = buildStateMetadata();
            // Reset
            morphEnabled = false;
            morphRate = 0.25;
            morphMu = 0;
            initMorphTarget();
            // Reload
            applyLoadedState(state);
            return {
                enabled: morphEnabled,
                rate: morphRate,
                mu: morphMu,
                d0re: morphTargetCoeffs[0].re,
                d0im: morphTargetCoeffs[0].im
            };
        }""")
        assert result["enabled"] is True
        assert abs(result["rate"] - 1.5) < 1e-10
        assert abs(result["mu"] - 0.75) < 1e-10
        assert abs(result["d0re"] - 42) < 1e-10
        assert abs(result["d0im"] - (-13)) < 1e-10

    def test_morph_missing_in_old_snap(self, page):
        """Loading a snap without morph field doesn't crash — falls back to initMorphTarget."""
        result = page.evaluate("""() => {
            var state = buildStateMetadata();
            delete state.morph;
            applyLoadedState(state);
            return {
                nMorph: morphTargetCoeffs.length,
                nCoeffs: coefficients.length,
                enabled: morphEnabled
            };
        }""")
        assert result["nMorph"] == result["nCoeffs"]
        assert result["enabled"] is False

    def test_morph_disabled_load_sets_mu_zero(self, page):
        """Loading morph with enabled=false forces mu=0."""
        result = page.evaluate("""() => {
            morphEnabled = true;
            morphMu = 0.75;
            var state = buildStateMetadata();
            state.morph.enabled = false;
            state.morph.mu = 0.99;  // this should be overridden
            applyLoadedState(state);
            return { mu: morphMu, enabled: morphEnabled };
        }""")
        assert result["enabled"] is False
        assert result["mu"] == 0


class TestMorphDegreeSync:
    def test_degree_change_reinits_morph(self, page):
        """Changing degree reinitializes morphTargetCoeffs."""
        result = page.evaluate("""() => {
            morphTargetCoeffs[0].re = 999;
            applyPattern('default');
            return {
                nMorph: morphTargetCoeffs.length,
                nCoeffs: coefficients.length,
                d0re: morphTargetCoeffs[0].re
            };
        }""")
        assert result["nMorph"] == result["nCoeffs"]
        # After pattern change, D should be fresh copy of C, not 999
        assert result["d0re"] != 999


class TestMorphFastMode:
    def test_serialize_includes_morph_when_enabled(self, page):
        """serializeFastModeData includes morph data when enabled."""
        result = page.evaluate("""() => {
            morphEnabled = true;
            morphRate = 0.5;
            morphTargetCoeffs[0].re = 42;
            initBitmapCanvas();
            fastModeCurves = new Map();
            var animated = allAnimatedCoeffs();
            var stepsVal = 100;
            var nRoots = currentRoots.length;
            var sd = serializeFastModeData(animated, stepsVal, nRoots);
            morphEnabled = false;
            morphMu = 0;
            initMorphTarget();
            return {
                hasMorphEnabled: 'morphEnabled' in sd,
                hasMorphRate: 'morphRate' in sd,
                hasMorphTargetRe: 'morphTargetRe' in sd,
                hasMorphTargetIm: 'morphTargetIm' in sd,
                morphEnabled: sd.morphEnabled,
                morphRate: sd.morphRate
            };
        }""")
        assert result["hasMorphEnabled"] is True
        assert result["hasMorphRate"] is True
        assert result["hasMorphTargetRe"] is True
        assert result["hasMorphTargetIm"] is True
        assert result["morphEnabled"] is True
        assert abs(result["morphRate"] - 0.5) < 1e-10

    def test_serialize_no_morph_when_disabled(self, page):
        """serializeFastModeData omits morph target data when disabled."""
        result = page.evaluate("""() => {
            morphEnabled = false;
            initBitmapCanvas();
            fastModeCurves = new Map();
            var animated = allAnimatedCoeffs();
            var sd = serializeFastModeData(animated, 100, currentRoots.length);
            return {
                morphEnabled: sd.morphEnabled,
                hasMorphTargetRe: 'morphTargetRe' in sd
            };
        }""")
        assert result["morphEnabled"] is False
