"""Shared palette-name authority for Python handlers."""

BUILTIN_PALETTE_NAMES = {
    "inferno", "viridis", "magma", "plasma", "turbo", "cividis",
    "identity", "identity_hsv", "warm", "cool", "bwred", "neon_v", "gilded", "reef", "abyss", "rainbow_d3",
}

try:
    from tri_palette_names_generated import TRI_PALETTE_NAMES  # type: ignore
except ImportError:  # pragma: no cover - generator should create this before packaging
    TRI_PALETTE_NAMES = set()

try:
    from long_palette_names_generated import LONG_PALETTE_NAMES  # type: ignore
except ImportError:  # pragma: no cover - generator should create this before packaging
    LONG_PALETTE_NAMES = set()

VALID_PALETTE_NAMES = BUILTIN_PALETTE_NAMES | set(TRI_PALETTE_NAMES) | set(LONG_PALETTE_NAMES)

# Custom hex-stop palettes travel as a NAME: "custom:rrggbb-rrggbb-..."
# (lowercase canonical from the UI; accepted case-insensitively here and in
# palette_lut.h). 2..32 equally spaced stops, linear interpolation between —
# the same model paletteRGB applies to every built-in table. The name never
# enters S3 keys (palette_id is digest-based), so no character constraints
# beyond the pattern itself.
import re

CUSTOM_PALETTE_RE = re.compile(r"custom:[0-9a-fA-F]{6}(?:-[0-9a-fA-F]{6}){1,31}\Z")


def is_valid_palette_name(name):
    n = str(name or "")
    return n in VALID_PALETTE_NAMES or bool(CUSTOM_PALETTE_RE.match(n))
