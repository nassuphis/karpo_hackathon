#!/usr/bin/env python3
"""
Generate a simple one-spread PDF preview: text page on the left, image page on the right.

This is intentionally simpler than make_book.py. It produces a single PDF page
representing an open two-page spread so layout can be reviewed quickly.

Usage:
  python make_spread.py snaps/my_image.jpeg
  python make_spread.py my_image.jpeg                  # resolved under snaps/
  python make_spread.py snaps/my_image.jpeg -o out.pdf
"""

import argparse
import random
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / "lambda"))

try:
    from spread_pdf import build_color_spread_pdf
except ModuleNotFoundError as exc:
    raise SystemExit(
        "Missing PDF/image dependency. Use the repo venv, for example:\n"
        "  ../.venv/bin/python make_spread.py snaps/<image>.jpeg"
    ) from exc

SNAPS_DIR = ROOT / "snaps"
OUTPUT_DIR = ROOT / "output" / "pdf"

LOREM_BLOCKS = [
    (
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent "
        "posuere, erat ut mollis luctus, velit sem suscipit sem, a volutpat "
        "velit magna nec erat. Sed in mauris non nibh tincidunt luctus."
    ),
    (
        "Integer sodales, sapien at feugiat consequat, neque massa interdum "
        "odio, sit amet condimentum nibh est in urna. Donec quis pretium "
        "ligula. Curabitur sed magna vitae purus finibus egestas."
    ),
    (
        "Nullam in turpis a nibh feugiat malesuada. Quisque placerat justo "
        "vitae augue feugiat, sit amet pharetra sapien tempor. Aliquam "
        "eleifend tellus sed sem volutpat, vel faucibus mauris dapibus."
    ),
    (
        "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
        "posuere cubilia curae; Aenean faucibus, eros vitae fringilla "
        "convallis, magna purus feugiat libero, nec iaculis massa eros sit amet nisl."
    ),
    (
        "Mauris eu mi ac neque volutpat venenatis. Aenean dictum ultricies "
        "neque, a tincidunt purus interdum non. Phasellus pretium blandit "
        "libero, vitae interdum dui volutpat id."
    ),
]


def _resolve_image_path(raw: str) -> Path:
    p = Path(raw)
    if p.exists():
        return p.resolve()
    candidate = SNAPS_DIR / raw
    if candidate.exists():
        return candidate.resolve()
    raise FileNotFoundError(f"Image not found: {raw!r} (also tried {candidate})")


def _default_output_path(image_path: Path) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    return OUTPUT_DIR / f"{image_path.stem}_spread.pdf"


def _spread_title(image_path: Path) -> str:
    return "PolyPaint Lambda 1.0"


def _lorem_for_image(image_path: Path) -> str:
    rng = random.Random(image_path.stem)
    blocks = LOREM_BLOCKS[:]
    rng.shuffle(blocks)
    return "\n\n".join(blocks[:3])


def generate_spread_pdf(image_path: Path, output_path: Path):
    output_path.parent.mkdir(parents=True, exist_ok=True)
    # Local mode: use lorem placeholder text (no artifact metadata available)
    return build_color_spread_pdf(
        image_path,
        output_path,
        _spread_title(image_path),
        body=_lorem_for_image(image_path),
        filename=image_path.stem,
    )


def main():
    parser = argparse.ArgumentParser(description="Generate a simple text+image spread PDF preview.")
    parser.add_argument("image", help="Path to image, or basename under snaps/")
    parser.add_argument("-o", "--output", help="Output PDF path")
    args = parser.parse_args()

    image_path = _resolve_image_path(args.image)
    output_path = Path(args.output).resolve() if args.output else _default_output_path(image_path)
    out = generate_spread_pdf(image_path, output_path)
    print(out)


if __name__ == "__main__":
    main()
