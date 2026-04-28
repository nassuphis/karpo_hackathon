import os
import sys


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from palette_names import VALID_PALETTE_NAMES


def test_identity_palette_is_valid_builtin():
    assert "identity" in VALID_PALETTE_NAMES
    assert "identity_hsv" in VALID_PALETTE_NAMES
