#!/usr/bin/env python3
"""Build data/mic_palette_catalog.json — the Meditations in Color library.

meditationsincolor.com/colorists is a static page that embeds its entire
archive as one inline `const ARTISTS = [...]` JavaScript literal (unquoted
keys — JS, not JSON). Each artist carries a `works` array whose records
hold an ordered hex `palette` plus title/date/source/pageUrl. See
color-meditations.md for the full site investigation.

Modes:
  --fetch [--html PATH] [--url URL]
      Rebuild the catalog. Downloads the page (or reads a saved copy via
      --html), slices the ARTISTS literal, evaluates it in a node `vm`
      sandbox (node is required for --fetch only), and writes the catalog.
      Network + node are deliberately NOT part of any test gate.
  --check
      Offline validation of the committed catalog (schema, hex, name
      lengths, dedup, counts). This is the predeploy-gated mode.

Catalog schema (compact keys — the client fetches this lazily, ~5 MB):
  { schema_version: 1, fetched: "YYYY-MM-DD", origin, credit, count,
    palettes: [ { n: "Artist — Title (date)",   # display name, <= 80 chars
                  a: "Artist", s: "MET",         # artist + source-museum code
                  u: "https://...",              # original record URL (credit)
                  c: ["aabbcc", ...] } ] }       # 1..32 lowercase hex stops

Wire mapping (client side): colors become a `custom:aabbcc-...` spec —
single-color palettes duplicate their stop to satisfy the 2-stop minimum
of CUSTOM_PALETTE_RE — and `n` travels as palette_display_name, so the
render pipeline needs no new palette machinery at all.
"""
from __future__ import annotations

import argparse
import datetime as _dt
import json
import re
import subprocess
import sys
import tempfile
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CATALOG_PATH = ROOT / "data" / "mic_palette_catalog.json"

ORIGIN_URL = "https://meditationsincolor.com/colorists"
CREDIT = "Palettes: Meditations in Color (meditationsincolor.com)"
SCHEMA_VERSION = 1

MAX_NAME_LEN = 80          # lambda/palette_names.py MAX_PALETTE_DISPLAY_NAME_LEN
MAX_STOPS = 32             # custom: wire cap (palette_lut.h / palette_names.py)
MIN_EXPECTED_PALETTES = 20000
HEX_RE = re.compile(r"^[0-9a-f]{6}$")

_NODE_EVAL = """
const fs = require("fs"), vm = require("vm");
const src = fs.readFileSync(process.argv[2], "utf8");
const ctx = {}; vm.createContext(ctx);
vm.runInContext(src + "\\nglobalThis.__out = ARTISTS;", ctx);
process.stdout.write(JSON.stringify(ctx.__out));
"""


def _clean_text(value) -> str:
    text = " ".join(str(value or "").split())
    return "".join(ch for ch in text if ch.isprintable())


def _display_name(artist: str, title: str, date: str) -> str:
    name = f"{artist} — {title}" + (f" ({date})" if date else "")
    if len(name) > MAX_NAME_LEN:
        name = name[: MAX_NAME_LEN - 1].rstrip() + "…"
    return name


def _normalize_hex(value) -> str | None:
    s = str(value or "").strip().lstrip("#").lower()
    if re.fullmatch(r"[0-9a-f]{3}", s):
        s = "".join(c + c for c in s)
    return s if HEX_RE.fullmatch(s) else None


def _slice_artists_literal(html: str) -> str:
    scripts = re.findall(r"<script(?![^>]*src)[^>]*>(.*?)</script>", html, re.S)
    if not scripts:
        raise SystemExit("FATAL: no inline scripts found in the page")
    js = max(scripts, key=len)
    start = js.find("const ARTISTS = [")
    if start < 0:
        raise SystemExit("FATAL: `const ARTISTS = [` not found — site shape changed")
    end_match = re.search(r"^\];", js[start:], re.M)
    if not end_match:
        raise SystemExit("FATAL: ARTISTS literal terminator not found")
    return js[start : start + end_match.end()]


def _eval_artists(slice_js: str) -> list:
    with tempfile.NamedTemporaryFile("w", suffix=".js", delete=False,
                                     encoding="utf-8") as fh:
        fh.write(slice_js)
        slice_path = fh.name
    proc = subprocess.run(
        ["node", "-e", _NODE_EVAL, "node", slice_path],
        capture_output=True, text=True, timeout=120)
    if proc.returncode != 0:
        raise SystemExit(f"FATAL: node evaluation failed: {proc.stderr[:400]}")
    return json.loads(proc.stdout)


def build_catalog(artists: list, fetched: str) -> dict:
    rows = []
    seen = set()
    skipped_hex = skipped_empty = skipped_dup = 0
    for artist in artists:
        artist_name = _clean_text(artist.get("name"))
        for work in artist.get("works") or []:
            colors = [_normalize_hex(h) for h in (work.get("palette") or [])]
            if not colors:
                skipped_empty += 1
                continue
            if any(c is None for c in colors) or len(colors) > MAX_STOPS:
                skipped_hex += 1
                continue
            url = _clean_text(work.get("pageUrl"))
            title = _clean_text(work.get("title"))
            date = _clean_text(work.get("date"))
            key = url or f"{artist.get('id')}|{title}|{date}"
            if key in seen:
                skipped_dup += 1
                continue
            seen.add(key)
            rows.append({
                "n": _display_name(artist_name, title, date),
                "a": artist_name,
                "s": _clean_text(work.get("source")),
                "u": url,
                "c": colors,
            })
    rows.sort(key=lambda r: (r["a"].lower(), r["n"].lower()))
    print(f"kept {len(rows)} palettes "
          f"(skipped: {skipped_dup} duplicate, {skipped_hex} bad-hex, "
          f"{skipped_empty} empty)")
    return {
        "schema_version": SCHEMA_VERSION,
        "fetched": fetched,
        "origin": ORIGIN_URL,
        "credit": CREDIT,
        "count": len(rows),
        "palettes": rows,
    }


def check_catalog() -> int:
    if not CATALOG_PATH.exists():
        print(f"FATAL: {CATALOG_PATH} missing — run --fetch")
        return 1
    doc = json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
    problems = []
    if doc.get("schema_version") != SCHEMA_VERSION:
        problems.append("schema_version mismatch")
    for field in ("fetched", "origin", "credit"):
        if not str(doc.get(field) or "").strip():
            problems.append(f"missing {field}")
    rows = doc.get("palettes")
    if not isinstance(rows, list) or len(rows) < MIN_EXPECTED_PALETTES:
        problems.append(f"expected >= {MIN_EXPECTED_PALETTES} palettes, "
                        f"got {len(rows) if isinstance(rows, list) else 'none'}")
        rows = rows if isinstance(rows, list) else []
    if doc.get("count") != len(rows):
        problems.append("count field disagrees with palettes length")
    seen = set()
    for i, row in enumerate(rows):
        name = str(row.get("n") or "")
        colors = row.get("c")
        if not name or len(name) > MAX_NAME_LEN:
            problems.append(f"row {i}: bad name length")
        if any(not ch.isprintable() for ch in name):
            problems.append(f"row {i}: non-printable name")
        if (not isinstance(colors, list) or not (1 <= len(colors) <= MAX_STOPS)
                or not all(isinstance(c, str) and HEX_RE.fullmatch(c)
                           for c in colors)):
            problems.append(f"row {i}: invalid colors")
        key = row.get("u") or name
        if key in seen:
            problems.append(f"row {i}: duplicate key {key!r}")
        seen.add(key)
        if problems and len(problems) > 20:
            break
    if problems:
        for p in problems[:20]:
            print(f"FATAL: {p}")
        return 1
    print(f"OK {CATALOG_PATH} ({len(rows)} palettes, fetched {doc['fetched']})")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--fetch", action="store_true")
    group.add_argument("--check", action="store_true")
    parser.add_argument("--url", default=ORIGIN_URL)
    parser.add_argument("--html", help="parse a saved copy instead of fetching")
    args = parser.parse_args()

    if args.check:
        return check_catalog()

    if args.html:
        html = Path(args.html).read_text(encoding="utf-8")
    else:
        req = urllib.request.Request(
            args.url, headers={"User-Agent": "PolyPaint-PaletteResearch/1.0"})
        with urllib.request.urlopen(req, timeout=60) as resp:
            html = resp.read().decode("utf-8")
        print(f"fetched {args.url}: {len(html):,} chars")

    artists = _eval_artists(_slice_artists_literal(html))
    print(f"parsed {len(artists)} artists")
    catalog = build_catalog(artists, _dt.date.today().isoformat())
    CATALOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    CATALOG_PATH.write_text(
        json.dumps(catalog, ensure_ascii=False, separators=(",", ":")) + "\n",
        encoding="utf-8")
    print(f"wrote {CATALOG_PATH} ({CATALOG_PATH.stat().st_size:,} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
