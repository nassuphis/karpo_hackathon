"""
Custom hex-stop palettes: "custom:rrggbb-rrggbb-..." names carry the
definition through every existing palette-name path. This suite pins the
two authorities in lockstep:
  * palette_names.is_valid_palette_name (every Python validation site)
  * palette_lut.h findPalette/paletteRGB (every native renderer)
The C probe builds locally (header-only, no vips); ARM64 lineage is the
deploy build's concern — the parser has no arch-dependent behavior.
"""
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
PROBE_SRC = os.path.join(os.path.dirname(__file__), "palette_custom_probe.c")
PROBE_BIN = "/tmp/polypaint_palette_custom_probe"

sys.path.insert(0, LAMBDA_DIR)

EXAMPLE = "custom:879caa-aaa4a4-859789-91857c-696f69-567183-3b5362-0e3057"


def _ensure_probe():
    deps = [PROBE_SRC, os.path.join(LAMBDA_DIR, "palette_lut.h")]
    if os.path.exists(PROBE_BIN):
        built = os.path.getmtime(PROBE_BIN)
        if all(built >= os.path.getmtime(d) for d in deps):
            return True
    proc = subprocess.run(
        ["cc", "-O2", "-o", PROBE_BIN, PROBE_SRC],
        capture_output=True, text=True,
    )
    return proc.returncode == 0


HAVE_PROBE = _ensure_probe()


def _probe(palette, *ts):
    proc = subprocess.run(
        [PROBE_BIN, palette, *[str(t) for t in ts]],
        capture_output=True, text=True, timeout=30,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:300])
    lines = proc.stdout.strip().splitlines()
    head = dict(part.split("=") for part in lines[0].split())
    rgbs = [tuple(int(v) for v in line.split(",")) for line in lines[1:]]
    return head["name"], int(head["n"]), rgbs


class TestCustomPaletteValidity(unittest.TestCase):
    def test_python_validity_matrix(self):
        from palette_names import is_valid_palette_name

        good = [
            EXAMPLE,
            "custom:879CAA-AAA4A4",                       # case-insensitive
            "custom:" + "-".join(["112233"] * 32),        # max stops
            "inferno",                                    # builtins still pass
        ]
        bad = [
            "custom:879caa",                              # one stop
            "custom:",
            "custom:879caa-zzz4a4",                       # bad hex
            "custom:879caa-aaa4a4-",                      # trailing dash
            "custom:" + "-".join(["112233"] * 33),        # too many stops
            "custom:879caa,aaa4a4",                       # wrong separator
            "#879caa-#aaa4a4",                            # raw user text is UI-side only
            "nonsense",
        ]
        for name in good:
            self.assertTrue(is_valid_palette_name(name), name)
        for name in bad:
            self.assertFalse(is_valid_palette_name(name), name)


@unittest.skipUnless(HAVE_PROBE, "local cc build unavailable")
class TestCustomPaletteNative(unittest.TestCase):
    def test_custom_stops_resolve_and_interpolate(self):
        """The canonical name resolves to its stops: endpoints exact, the
        midpoint of a 2-stop palette is the rounded linear blend — the same
        paletteRGB math every built-in table uses."""
        name, n, rgbs = _probe("custom:000000-ffffff", 0.0, 0.5, 1.0)
        self.assertEqual(name, "custom")
        self.assertEqual(n, 2)
        self.assertEqual(rgbs[0], (0, 0, 0))
        self.assertEqual(rgbs[1], (128, 128, 128))   # 0.5*255 + 0.5 rounds to 128
        self.assertEqual(rgbs[2], (255, 255, 255))

        name, n, rgbs = _probe(EXAMPLE, 0.0, 1.0)
        self.assertEqual(n, 8)
        self.assertEqual(rgbs[0], (0x87, 0x9C, 0xAA))
        self.assertEqual(rgbs[1], (0x0E, 0x30, 0x57))

    def test_uppercase_hex_accepted(self):
        name, n, rgbs = _probe("custom:879CAA-0E3057", 0.0, 1.0)
        self.assertEqual(n, 2)
        self.assertEqual(rgbs[0], (0x87, 0x9C, 0xAA))
        self.assertEqual(rgbs[1], (0x0E, 0x30, 0x57))

    def test_malformed_custom_falls_back_to_inferno(self):
        """Malformed customs behave exactly like unknown names: inferno.
        Matters for deploy ordering — an OLD binary seeing a custom name
        renders inferno rather than crashing."""
        for bad in ("custom:879caa", "custom:879caa-zzz4a4",
                    "custom:879caa-aaa4a4-", "custom:"):
            name, n, _ = _probe(bad, 0.5)
            self.assertEqual(name, "inferno", bad)

    def test_python_and_c_agree_on_validity(self):
        """Lockstep: every name the Python validator accepts must resolve to
        a real custom palette in C (and pass the strict binaries' explicit
        paletteNameIsValid check); every custom-prefixed name Python rejects
        must fall back AND fail the strict check."""
        from palette_names import is_valid_palette_name

        cases = [
            EXAMPLE,
            "custom:879CAA-AAA4A4",
            "custom:" + "-".join(["112233"] * 32),
            "custom:879caa",
            "custom:879caa-aaa4a4-",
            "custom:879caa,aaa4a4",
            "custom:" + "-".join(["112233"] * 33),
            "inferno",
            "nonsense",
        ]
        for name in cases:
            want = is_valid_palette_name(name)
            proc = subprocess.run([PROBE_BIN, "--valid", name],
                                  capture_output=True, text=True, timeout=30)
            self.assertEqual(proc.returncode, 0, proc.stderr[:200])
            self.assertEqual(proc.stdout.strip(), f"valid={1 if want else 0}", name)
            if name.startswith("custom:"):
                resolved, n, _ = _probe(name, 0.5)
                if want:
                    self.assertEqual(resolved, "custom", name)
                    self.assertGreaterEqual(n, 2, name)
                else:
                    self.assertEqual(resolved, "inferno", name)


if __name__ == "__main__":
    unittest.main()
