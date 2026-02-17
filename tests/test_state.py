"""Tests for save/load state roundtrip."""

import json
import pytest


class TestSaveLoadRoundtrip:
    """buildStateMetadata → applyLoadedState → buildStateMetadata should preserve key fields."""

    def test_basic_roundtrip(self, page):
        """Save state, reload, save again — key fields should match."""
        result = page.evaluate("""() => {
            // Save initial state
            var state1 = buildStateMetadata("test-ts");
            // Apply it back
            applyLoadedState(state1);
            // Save again
            var state2 = buildStateMetadata("test-ts");
            return {state1: state1, state2: state2};
        }""")
        s1 = result["state1"]
        s2 = result["state2"]
        assert s1["degree"] == s2["degree"]
        assert len(s1["coefficients"]) == len(s2["coefficients"])
        assert len(s1["roots"]) == len(s2["roots"])
        assert s1["rootColorMode"] == s2["rootColorMode"]
        assert s1["trails"] == s2["trails"]
        assert s1["domainColoring"] == s2["domainColoring"]

    def test_coefficients_preserved(self, page):
        """Coefficient positions and paths survive roundtrip."""
        result = page.evaluate("""() => {
            var state1 = buildStateMetadata();
            applyLoadedState(state1);
            var state2 = buildStateMetadata();
            return {c1: state1.coefficients, c2: state2.coefficients};
        }""")
        c1 = result["c1"]
        c2 = result["c2"]
        assert len(c1) == len(c2)
        for i in range(len(c1)):
            assert abs(c1[i]["home"][0] - c2[i]["home"][0]) < 1e-10, f"Coeff {i} home re mismatch"
            assert abs(c1[i]["home"][1] - c2[i]["home"][1]) < 1e-10, f"Coeff {i} home im mismatch"
            assert c1[i]["pathType"] == c2[i]["pathType"], f"Coeff {i} pathType mismatch"
            assert abs(c1[i]["speed"] - c2[i]["speed"]) < 1e-10, f"Coeff {i} speed mismatch"

    def test_roots_preserved(self, page):
        """Root positions survive roundtrip."""
        result = page.evaluate("""() => {
            var state1 = buildStateMetadata();
            applyLoadedState(state1);
            var state2 = buildStateMetadata();
            return {r1: state1.roots, r2: state2.roots};
        }""")
        r1 = result["r1"]
        r2 = result["r2"]
        assert len(r1) == len(r2)
        for i in range(len(r1)):
            assert abs(r1[i][0] - r2[i][0]) < 1e-6, f"Root {i} re mismatch"
            assert abs(r1[i][1] - r2[i][1]) < 1e-6, f"Root {i} im mismatch"

    def test_jiggle_preserved(self, page):
        """Jiggle settings survive roundtrip."""
        result = page.evaluate("""() => {
            jiggleMode = "rotate";
            jiggleSigma = 15;
            jiggleAngleSteps = 200;
            var state1 = buildStateMetadata();
            applyLoadedState(state1);
            var state2 = buildStateMetadata();
            return {j1: state1.jiggle, j2: state2.jiggle};
        }""")
        j1 = result["j1"]
        j2 = result["j2"]
        assert j1["mode"] == j2["mode"] == "rotate"
        assert j1["sigma"] == j2["sigma"]
        assert j1["angleSteps"] == j2["angleSteps"] == 200

    def test_panel_ranges_preserved(self, page):
        """Panel ranges survive roundtrip."""
        result = page.evaluate("""() => {
            var state1 = buildStateMetadata();
            applyLoadedState(state1);
            var state2 = buildStateMetadata();
            return {p1: state1.panels, p2: state2.panels};
        }""")
        p1 = result["p1"]
        p2 = result["p2"]
        assert abs(p1["coeff"]["range"] - p2["coeff"]["range"]) < 1e-10
        assert abs(p1["roots"]["range"] - p2["roots"]["range"]) < 1e-10

    def test_color_mode_preserved(self, page):
        """Root color mode survives roundtrip."""
        result = page.evaluate("""() => {
            rootColorMode = "proximity";
            var state1 = buildStateMetadata();
            applyLoadedState(state1);
            var state2 = buildStateMetadata();
            return {m1: state1.rootColorMode, m2: state2.rootColorMode};
        }""")
        assert result["m1"] == result["m2"] == "proximity"

    def test_uniform_color_preserved(self, page):
        """Uniform root color RGB preserved."""
        result = page.evaluate("""() => {
            rootColorMode = "uniform";
            uniformRootColor = [100, 200, 50];
            var state1 = buildStateMetadata();
            applyLoadedState(state1);
            var state2 = buildStateMetadata();
            return {c1: state1.uniformRootColor, c2: state2.uniformRootColor};
        }""")
        assert result["c1"] == result["c2"] == [100, 200, 50]


class TestLoadPartialState:
    """applyLoadedState should handle missing fields gracefully."""

    def test_minimal_state(self, page):
        """Loading minimal state (just degree + coefficients) doesn't crash."""
        result = page.evaluate("""() => {
            var minimal = {
                degree: 3,
                coefficients: [
                    {pos: [1, 0], home: [1, 0], pathType: "none", radius: 25, speed: 1, angle: 0, ccw: false, extra: {}},
                    {pos: [0, 0], home: [0, 0], pathType: "none", radius: 25, speed: 1, angle: 0, ccw: false, extra: {}},
                    {pos: [0, 0], home: [0, 0], pathType: "none", radius: 25, speed: 1, angle: 0, ccw: false, extra: {}},
                    {pos: [0, 1], home: [0, 1], pathType: "none", radius: 25, speed: 1, angle: 0, ccw: false, extra: {}}
                ]
            };
            applyLoadedState(minimal);
            return {degree: coefficients.length - 1, nRoots: currentRoots.length};
        }""")
        assert result["degree"] == 3
        assert result["nRoots"] == 3
