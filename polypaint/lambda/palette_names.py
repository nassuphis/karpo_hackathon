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


def is_valid_palette_name(name):
    return name in VALID_PALETTE_NAMES
