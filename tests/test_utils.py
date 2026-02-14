"""Tests for utility functions: findPrimeSpeed, subscript, PATH_CATALOG, audio helpers."""

import math
import pytest


class TestSubscript:
    def test_single_digit(self, page):
        result = page.evaluate("() => subscript(0)")
        assert result == "\u2080"

    def test_multi_digit(self, page):
        result = page.evaluate("() => subscript(42)")
        assert result == "\u2084\u2082"

    def test_all_digits(self, page):
        result = page.evaluate("""() => {
            var out = [];
            for (var i = 0; i < 10; i++) out.push(subscript(i));
            return out;
        }""")
        expected = ["\u2080", "\u2081", "\u2082", "\u2083", "\u2084",
                    "\u2085", "\u2086", "\u2087", "\u2088", "\u2089"]
        assert result == expected


class TestSuperscript:
    def test_single_digit(self, page):
        result = page.evaluate("() => superscript(0)")
        assert result == "\u2070"

    def test_multi_digit(self, page):
        result = page.evaluate("() => superscript(12)")
        assert result == "\u00B9\u00B2"

    def test_special_digits(self, page):
        """1, 2, 3 have special Unicode codepoints."""
        result = page.evaluate("""() => [superscript(1), superscript(2), superscript(3)]""")
        assert result == ["\u00B9", "\u00B2", "\u00B3"]


class TestPathCatalog:
    """PATH_CATALOG should be consistent with PATH_PARAMS."""

    def test_all_catalog_values_in_params(self, page):
        """Every path type in PATH_CATALOG should have an entry in PATH_PARAMS."""
        result = page.evaluate("""() => {
            var catalogValues = [];
            for (var entry of PATH_CATALOG) {
                if (entry.value !== undefined) catalogValues.push(entry.value);
                else if (entry.items) {
                    for (var item of entry.items) catalogValues.push(item.value);
                }
            }
            var missing = catalogValues.filter(v => !(v in PATH_PARAMS));
            return {catalogValues: catalogValues, missing: missing, paramKeys: Object.keys(PATH_PARAMS)};
        }""")
        assert result["missing"] == [], f"PATH_CATALOG has types not in PATH_PARAMS: {result['missing']}"

    def test_all_params_in_catalog(self, page):
        """Every PATH_PARAMS key should appear in PATH_CATALOG."""
        result = page.evaluate("""() => {
            var catalogValues = new Set();
            for (var entry of PATH_CATALOG) {
                if (entry.value !== undefined) catalogValues.add(entry.value);
                else if (entry.items) {
                    for (var item of entry.items) catalogValues.add(item.value);
                }
            }
            var paramKeys = Object.keys(PATH_PARAMS);
            var missing = paramKeys.filter(k => !catalogValues.has(k));
            return missing;
        }""")
        assert result == [], f"PATH_PARAMS has types not in PATH_CATALOG: {result}"

    def test_no_duplicate_values(self, page):
        """No duplicate path values in PATH_CATALOG."""
        result = page.evaluate("""() => {
            var values = [];
            for (var entry of PATH_CATALOG) {
                if (entry.value !== undefined) values.push(entry.value);
                else if (entry.items) {
                    for (var item of entry.items) values.push(item.value);
                }
            }
            var seen = new Set();
            var dupes = [];
            for (var v of values) {
                if (seen.has(v)) dupes.push(v);
                seen.add(v);
            }
            return dupes;
        }""")
        assert result == [], f"Duplicate path values: {result}"

    def test_catalog_has_groups(self, page):
        """PATH_CATALOG should have at least 3 groups (Basic, Curves, Space-filling)."""
        result = page.evaluate("""() => {
            return PATH_CATALOG.filter(e => e.group).map(e => e.group);
        }""")
        assert "Basic" in result
        assert "Curves" in result
        assert "Space-filling" in result


class TestFindPrimeSpeed:
    """findPrimeSpeed finds nearest coprime integer speed."""

    def test_coprime_with_empty(self, page):
        """No other animated coefficients → return current speed."""
        result = page.evaluate("""() => {
            // Set up coefficients with no animated paths
            var saved = coefficients;
            coefficients = [{pathType: "none", speed: 1}];
            var r = findPrimeSpeed(5, new Set([0]));
            coefficients = saved;
            return r;
        }""")
        assert result == 5

    def test_avoids_common_factor(self, page):
        """Speed should be coprime with other animated speeds."""
        result = page.evaluate("""() => {
            var saved = coefficients;
            coefficients = [
                {pathType: "circle", speed: 1, re: 0, im: 0},  // speed*100 = 100
                {pathType: "none", speed: 1, re: 0, im: 0}     // target (excluded)
            ];
            // Exclude index 1, other speed is 100
            var r = findPrimeSpeed(50, new Set([1]));  // 50 shares factor 50 with 100
            coefficients = saved;
            // GCD(50*100, 100*100) = GCD(5000, 10000) > 1, so 50 is invalid
            // Should find nearby coprime
            return r;
        }""")
        # Result should be coprime with 100. 50 shares factor 2 and 5 with 100.
        # 51 = 3*17, GCD(51*100, 100*100) → we need GCD(5100, 10000)...
        # Actually the function works with speed*100 as integers
        # Speed 100 maps to int 10000. We want GCD(result*100, 10000) == 1
        # result*100 must be coprime with 10000. 10000 = 2^4 * 5^4
        # Need result*100 coprime with 10000. 100 = 2^2 * 5^2. So result*100 = result * 2^2 * 5^2
        # For coprimality with 10000 = 2^4 * 5^4, need result to have no factors of 2 or 5.
        # 51 = 3*17 → 51*100 = 5100, GCD(5100, 10000) = 100 > 1. Not coprime.
        # Actually let me re-read: the function rounds speeds to speed*100 (integer cents).
        # It checks GCD(candidate*100, otherSpeed*100) > 1.
        # Wait no - it does: Math.round(coefficients[i].speed * 100) for others.
        # Then isValid(s) checks GCD(s, o) > 1 for each o.
        # currentIntSpeed is already in the *100 space? No - looking at the code:
        # findPrimeSpeed(currentIntSpeed, excludeSet)
        # otherSpeeds.push(Math.round(coefficients[i].speed * 100))
        # So otherSpeeds contains speed*100 values. currentIntSpeed is also in that space.
        # isValid(s) checks: s !== o && GCD(s, o) > 1
        # So if other is 100 (speed=1), and we try s=50: GCD(50, 100) = 50 > 1 → invalid.
        # s=51: GCD(51, 100) = 1 → valid!
        assert result != 50  # should have moved away from 50

    def test_returns_integer(self, page):
        """Result should be an integer."""
        result = page.evaluate("""() => {
            var saved = coefficients;
            coefficients = [{pathType: "circle", speed: 0.5, re: 0, im: 0}];
            var r = findPrimeSpeed(100, new Set());
            coefficients = saved;
            return r;
        }""")
        assert result == int(result)


class TestAudioHelpers:
    """Test small audio utility functions."""

    def test_clamp_within(self, page):
        assert page.evaluate("() => audioClamp(5, 0, 10)") == 5

    def test_clamp_below(self, page):
        assert page.evaluate("() => audioClamp(-1, 0, 10)") == 0

    def test_clamp_above(self, page):
        assert page.evaluate("() => audioClamp(15, 0, 10)") == 10

    def test_frac01_positive(self, page):
        result = page.evaluate("() => frac01(2.75)")
        assert abs(result - 0.75) < 1e-10

    def test_frac01_negative(self, page):
        result = page.evaluate("() => frac01(-0.3)")
        assert abs(result - 0.7) < 1e-10

    def test_frac01_zero(self, page):
        assert page.evaluate("() => frac01(0)") == 0

    def test_midi_to_hz_a4(self, page):
        """MIDI 69 = A4 = 440 Hz."""
        result = page.evaluate("() => midiToHz(69)")
        assert abs(result - 440.0) < 1e-10

    def test_midi_to_hz_a5(self, page):
        """MIDI 81 = A5 = 880 Hz."""
        result = page.evaluate("() => midiToHz(81)")
        assert abs(result - 880.0) < 1e-6

    def test_midi_to_hz_c4(self, page):
        """MIDI 60 = C4 ≈ 261.63 Hz."""
        result = page.evaluate("() => midiToHz(60)")
        assert abs(result - 261.6256) < 0.01


class TestQuantizeToScale:
    """quantizeToScale snaps semitone to nearest pentatonic scale degree."""

    def test_on_scale(self, page):
        """Scale degrees [0, 2, 4, 7, 9] should map to themselves."""
        result = page.evaluate("""() => {
            return [0, 2, 4, 7, 9].map(s => quantizeToScale(s));
        }""")
        assert result == [0, 2, 4, 7, 9]

    def test_off_scale(self, page):
        """1 (between 0 and 2) → nearest is 0 or 2."""
        result = page.evaluate("() => quantizeToScale(1)")
        assert result in [0, 2]

    def test_octave_wrapping(self, page):
        """12 + 2 = 14 → should map to 12 + 2 = 14."""
        result = page.evaluate("() => quantizeToScale(14)")
        assert result == 14  # 14 = 12 + 2, and 2 is in scale


class TestComputeRange:
    """computeRange computes appropriate range from point array."""

    def test_basic(self, page):
        result = page.evaluate("""() => computeRange([{re: 1, im: 0}, {re: 0, im: 1}])""")
        # maxAbs = max(MIN_RANGE=1.5, 1) = 1.5, raw = 1.5 * 1.3 * 2 = 3.9, ceil = 4, /2 = 2.0
        assert result == 2.0

    def test_array_format(self, page):
        """computeRange also accepts [re, im] arrays."""
        result = page.evaluate("""() => computeRange([[2, 0], [0, -3]])""")
        # maxAbs = 3, raw = 3 * 1.3 * 2 = 7.8, ceil = 8, /2 = 4.0
        assert result == 4.0

    def test_non_finite_ignored(self, page):
        """Non-finite values should be ignored."""
        result = page.evaluate("""() => computeRange([{re: Infinity, im: 0}, {re: 1, im: NaN}])""")
        # Only finite is re=1, maxAbs=1.5 (MIN_RANGE), raw=1.5*1.3*2=3.9, ceil=4, /2=2.0
        assert result >= 1.5  # at least MIN_RANGE after padding

    def test_empty(self, page):
        """Empty array → MIN_RANGE padded."""
        result = page.evaluate("""() => computeRange([])""")
        assert result >= 1.5  # MIN_RANGE
