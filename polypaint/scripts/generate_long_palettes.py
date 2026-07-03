#!/usr/bin/env python3
"""
Generate long-palette assets from color_dicts.COLOR_LONG_STRINGS.

Outputs:
- long_palette_catalog_js.js
- lambda/long_palette_lut.generated.h
- lambda/long_palette_names_generated.py
"""

from __future__ import annotations

import colorsys
import json
import math
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

import color_dicts

JS_OUT = ROOT / "long_palette_catalog_js.js"
C_OUT = ROOT / "lambda" / "long_palette_lut.generated.h"
PY_OUT = ROOT / "lambda" / "long_palette_names_generated.py"

RGB = tuple[int, int, int]
_HEX_RE = re.compile(r"^[0-9a-fA-F]{6}$")
_NAME_RE = re.compile(r"^[a-z0-9_]+$")
LONG_STOP_COUNT = 31
RESERVED_PALETTE_NAMES = {
    "inferno", "viridis", "magma", "plasma", "turbo", "cividis",
    "warm", "cool", "bwred", "neon_v", "gilded", "reef", "abyss", "rainbow_d3",
}


def resolve_color_token(token: str) -> RGB:
    s = str(token or "").strip()
    if not s:
        raise ValueError("empty color token")
    lower = s.lower()
    if lower in color_dicts.COLOR_NAME_MAP:
        s = color_dicts.COLOR_NAME_MAP[lower]
    if not _HEX_RE.fullmatch(s):
        raise ValueError(f"unknown color token: {token!r}")
    return (int(s[0:2], 16), int(s[2:4], 16), int(s[4:6], 16))


def parse_long_spec(spec: str) -> tuple[RGB, ...]:
    parts = [p.strip() for p in str(spec or "").split(":") if str(p).strip()]
    if len(parts) < 4:
        raise ValueError(f"long palette must have at least 4 stops, got {spec!r}")
    return tuple(resolve_color_token(part) for part in parts)


def _rgb255_to_hsv01(rgb: RGB) -> tuple[float, float, float]:
    r, g, b = rgb
    return colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)


def interpolate_hue_circle(h0: float, h1: float, t: float) -> float:
    a0 = 2.0 * math.pi * h0
    a1 = 2.0 * math.pi * h1
    z0 = complex(math.cos(a0), math.sin(a0))
    z1 = complex(math.cos(a1), math.sin(a1))
    z = (1.0 - t) * z0 + t * z1
    if abs(z) < 1e-12:
        z = z0
    ang = math.atan2(z.imag, z.real)
    return (ang / (2.0 * math.pi)) % 1.0


def _hsv01_to_rgb255(h: float, s: float, v: float) -> RGB:
    r, g, b = colorsys.hsv_to_rgb(h, s, v)
    return (
        int(round(max(0.0, min(1.0, r)) * 255.0)),
        int(round(max(0.0, min(1.0, g)) * 255.0)),
        int(round(max(0.0, min(1.0, b)) * 255.0)),
    )


def _interp_rgb_hsv(a: RGB, b: RGB, t: float) -> RGB:
    h0, s0, v0 = _rgb255_to_hsv01(a)
    h1, s1, v1 = _rgb255_to_hsv01(b)
    h = interpolate_hue_circle(h0, h1, t)
    s = (1.0 - t) * s0 + t * s1
    v = (1.0 - t) * v0 + t * v1
    return _hsv01_to_rgb255(h, s, v)


def expand_long_palette_hsv(stops: tuple[RGB, ...], n_out: int = LONG_STOP_COUNT) -> list[RGB]:
    if len(stops) < 2:
        raise ValueError("long palette requires at least 2 stops")
    if n_out < 2:
        raise ValueError("long palette output must have at least 2 stops")
    out: list[RGB] = []
    last_seg = len(stops) - 1
    for i in range(n_out):
        pos = (i / (n_out - 1)) * last_seg
        seg = min(int(math.floor(pos)), last_seg - 1)
        t = pos - seg
        out.append(_interp_rgb_hsv(stops[seg], stops[seg + 1], t))
    if out[0] != stops[0] or out[-1] != stops[-1]:
        raise AssertionError("long palette expansion lost exact endpoints")
    return out


def _gradient_css(stops: list[RGB]) -> str:
    def _fmt_pct(value: float) -> str:
        text = f"{value:.4f}".rstrip("0").rstrip(".")
        return text + "%"

    parts = []
    last = len(stops) - 1
    for i, (r, g, b) in enumerate(stops):
        pct = _fmt_pct(i / last * 100.0)
        parts.append(f"rgb({r}, {g}, {b}) {pct}")
    return "linear-gradient(to right, " + ", ".join(parts) + ")"


def _rgb_hex(rgb: RGB) -> str:
    return "#%02x%02x%02x" % rgb


def build_catalog_records() -> list[dict]:
    grouped: dict[tuple[RGB, ...], list[str]] = {}
    for name, spec in color_dicts.COLOR_LONG_STRINGS.items():
        stops = parse_long_spec(spec)
        grouped.setdefault(stops, []).append(name)

    records = []
    seen_ids = set()
    for stops_in, names in grouped.items():
        names = sorted(names)
        canonical = sorted(names, key=lambda n: (-len(n), n))[0]
        if not _NAME_RE.fullmatch(canonical):
            raise ValueError(f"unsupported canonical name: {canonical!r}")
        palette_id = f"long_{canonical}"
        if palette_id in seen_ids or palette_id in RESERVED_PALETTE_NAMES:
            raise ValueError(f"palette id collision: {palette_id}")
        seen_ids.add(palette_id)
        aliases = [n for n in names if n != canonical]
        stops = expand_long_palette_hsv(stops_in)
        records.append({
            "name": canonical,
            "palette_id": palette_id,
            "aliases": aliases,
            "n_input_stops": len(stops_in),
            "stops": [_rgb_hex(rgb) for rgb in stops],
            "gradient_css": _gradient_css(stops),
            "search_text": " ".join([canonical, *aliases]).lower(),
        })
    records.sort(key=lambda rec: rec["name"])
    return records


def generate_js(records: list[dict]) -> str:
    return (
        "window._longPaletteCatalog = "
        + json.dumps(records, indent=2, separators=(",", ": "))
        + ";\n"
    )


def generate_c_header(records: list[dict]) -> str:
    lines = [
        "/* Auto-generated by scripts/generate_long_palettes.py. Do not edit manually. */",
        "#ifndef LONG_PALETTE_LUT_GENERATED_H",
        "#define LONG_PALETTE_LUT_GENERATED_H",
        "",
    ]
    entry_lines = []
    for rec in records:
        arr_name = "PAL_LONG_" + rec["name"].upper()
        lines.append(f"static const RGB {arr_name}[{LONG_STOP_COUNT}] = {{")
        for stop in rec["stops"]:
            r = int(stop[1:3], 16)
            g = int(stop[3:5], 16)
            b = int(stop[5:7], 16)
            lines.append(f"    {{{r},{g},{b}}},")
        lines.append("};")
        lines.append("")
        entry_lines.append(f'    {{"{rec["palette_id"]}", {arr_name}, {LONG_STOP_COUNT}}},')
    lines.append("#define LONG_PALETTE_ENTRIES \\")
    if entry_lines:
        for idx, entry in enumerate(entry_lines):
            suffix = " \\" if idx < len(entry_lines) - 1 else ""
            lines.append(entry + suffix)
    else:
        lines.append("    /* no long palettes generated */")
    lines.append("")
    lines.append("#endif /* LONG_PALETTE_LUT_GENERATED_H */")
    return "\n".join(lines) + "\n"


def generate_py(records: list[dict]) -> str:
    names = "\n".join(f'    "{rec["palette_id"]}",' for rec in records)
    return (
        '"""Auto-generated long palette names. Do not edit manually."""\n\n'
        "LONG_PALETTE_NAMES = {\n"
        f"{names}\n"
        "}\n"
    )


def _rendered_outputs() -> list[tuple[Path, str]]:
    records = build_catalog_records()
    return [
        (JS_OUT, generate_js(records)),
        (C_OUT, generate_c_header(records)),
        (PY_OUT, generate_py(records)),
    ]


def write_outputs() -> None:
    records = build_catalog_records()
    outputs = [
        (JS_OUT, generate_js(records)),
        (C_OUT, generate_c_header(records)),
        (PY_OUT, generate_py(records)),
    ]
    for path, text in outputs:
        path.write_text(text, encoding="utf-8")
    print(f"generated {len(records)} canonical long palettes")


def check_outputs() -> int:
    stale = []
    for path, text in _rendered_outputs():
        try:
            on_disk = path.read_text(encoding="utf-8")
        except FileNotFoundError:
            on_disk = None
        if on_disk != text:
            stale.append(str(path.relative_to(ROOT)))
    if stale:
        print(f"long palette outputs stale: {', '.join(stale)}; run scripts/generate_long_palettes.py")
        return 1
    print(f"long palette outputs: OK")
    return 0


if __name__ == "__main__":
    if "--check" in sys.argv[1:]:
        raise SystemExit(check_outputs())
    write_outputs()
