#!/usr/bin/env python3
"""Extract a display palette from any image — MIC's artwork algorithm, ported.

This is an exact port of the palette pipeline meditationsincolor.com runs
client-side on its artwork pages (artwork.html, unminified; credit to the
site — the aesthetic weighting is theirs):

  1. Downscale to <= 400 px wide, grid-sample ~12,000 opaque pixels.
  2. k-means++ (k=28) in RGB, 25 Lloyd iterations, count pixels/cluster.
  3. Merge cluster centers closer than 0.04 in Oklab (count-weighted),
     drop clusters under 0.2% of samples.
  4. Score = sqrt(pixel share) x chroma boost x cool boosts x vivid boost
     x substrate penalty (earth 0.58 / neutral 0.82) — an aesthetic
     editor: vivid and cool colors up, canvas browns and greys down.
  5. Select `--colors` winners: best accent per hue family first, a
     guaranteed blue when one holds >= 0.4% of pixels, then score order
     under caps (earth <= 3, neutral <= 2, family <= 3 at target > 8) and
     a minimum Oklab separation (0.055; 0.07 vs earth/neutral), with two
     relaxation passes.

The site seeds with Math.random() — its own palettes differ per reload.
Here --seed makes runs reproducible (tests use it); default is random.

Usage:
  uv run python scripts/extract_image_palette.py IMAGE [--colors 12]
      [--seed N] [--json]
IMAGE is a local file path or an http(s) URL.
"""
from __future__ import annotations

import argparse
import io
import json
import math
import random
import sys
import urllib.request

import numpy as np
from PIL import Image

# ultra rescues tiny-but-compositional details (a small accent region must
# survive the downscale, be sampled, AND beat the cluster cull — all three
# limits move together; raising samples alone fixes only the middle one)
SAMPLING_TIERS = {
    "low":   {"target": 4000,   "max_width": 400, "min_count_fraction": 0.002},
    "med":   {"target": 12000,  "max_width": 400, "min_count_fraction": 0.002},
    "high":  {"target": 40000,  "max_width": 400, "min_count_fraction": 0.002},
    "ultra": {"target": 240000, "max_width": 800, "min_count_fraction": 0.0005},
}
KMEANS_K = 28
KMEANS_ITERS = 25
MERGE_OKLAB = 0.04


def srgb_to_oklab(rgb):
    """rgb: (..., 3) floats 0..255 -> Oklab (..., 3). Vectorized."""
    c = np.asarray(rgb, dtype=np.float64) / 255.0
    lin = np.where(c > 0.04045, ((c + 0.055) / 1.055) ** 2.4, c / 12.92)
    r, g, b = lin[..., 0], lin[..., 1], lin[..., 2]
    l_ = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b
    m_ = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b
    s_ = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b
    l1, m1, s1 = np.cbrt(l_), np.cbrt(m_), np.cbrt(s_)
    return np.stack([
        0.2104542553 * l1 + 0.7936177850 * m1 - 0.0040720468 * s1,
        1.9779984951 * l1 - 2.4285922050 * m1 + 0.4505937099 * s1,
        0.0259040371 * l1 + 0.7827717662 * m1 - 0.8086757660 * s1,
    ], axis=-1)


def _rgb_to_hsl(r, g, b):
    r, g, b = r / 255.0, g / 255.0, b / 255.0
    mx, mn = max(r, g, b), min(r, g, b)
    d = mx - mn
    h = 0.0
    s = 0.0
    l = (mx + mn) / 2.0
    if d != 0:
        s = d / (2 - mx - mn) if l > 0.5 else d / (mx + mn)
        if mx == r:
            h = ((g - b) / d + (6 if g < b else 0)) / 6
        elif mx == g:
            h = ((b - r) / d + 2) / 6
        else:
            h = ((r - g) / d + 4) / 6
    return h * 360.0, s, l


def classify(r, g, b, lab):
    hue, sat, light = _rgb_to_hsl(r, g, b)
    chroma = math.hypot(lab[1], lab[2])
    is_cool_muted = (185 <= hue <= 250 and b >= r + 8 and b >= g + 4
                     and (sat >= 0.05 or chroma >= 0.018)
                     and 0.18 < light < 0.82)
    is_neutral = not is_cool_muted and (chroma < 0.022 or sat < 0.10)
    is_earth = (not is_neutral and 15 <= hue <= 78 and sat < 0.45
                and chroma < 0.11 and 0.12 < light < 0.76)
    family = "red"
    if is_earth:
        family = "earth"
    elif is_neutral:
        family = "neutral"
    elif is_cool_muted or (185 <= hue < 265):
        family = "blue"
    elif 150 <= hue < 185:
        family = "teal"
    elif 85 <= hue < 150:
        family = "green"
    elif 55 <= hue < 85:
        family = "yellow"
    elif 25 <= hue < 55:
        family = "orange"
    elif 265 <= hue < 330:
        family = "violet"
    return {
        "chroma": chroma, "family": family,
        "is_earth": is_earth, "is_neutral": is_neutral,
        "is_cool_muted": is_cool_muted,
    }


def sample_image(img: Image.Image, target: int = 12000,
                 max_width: int = 400) -> np.ndarray:
    img = img.convert("RGBA")
    w0, h0 = img.size
    w = min(w0, max_width)
    h = max(1, round(w * (h0 / w0)))
    # the site samples a canvas drawImage downscale; bilinear is the
    # closest Pillow match (exactness is impossible across resamplers)
    img = img.resize((w, h), Image.BILINEAR)
    px = np.asarray(img, dtype=np.float64)
    step = max(1, int(math.floor(math.sqrt((w * h) / target))))
    grid = px[::step, ::step].reshape(-1, 4)
    return grid[grid[:, 3] > 128][:, :3]


def kmeans(samples: np.ndarray, rng: random.Random):
    n = len(samples)
    centers = [samples[rng.randrange(n)].copy()]
    for _ in range(1, KMEANS_K):
        arr = np.asarray(centers)
        d2 = ((samples[:, None, :] - arr[None, :, :]) ** 2).sum(axis=2).min(axis=1)
        total = float(d2.sum())
        r = rng.random() * total
        acc = np.cumsum(d2)
        idx = int(np.searchsorted(acc, r))
        if idx >= n:
            idx = rng.randrange(n)
        centers.append(samples[idx].copy())
    centers = np.asarray(centers)
    for _ in range(KMEANS_ITERS):
        d2 = ((samples[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2)
        assign = d2.argmin(axis=1)
        for c in range(KMEANS_K):
            members = samples[assign == c]
            if len(members):
                centers[c] = members.mean(axis=0)
    d2 = ((samples[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2)
    assign = d2.argmin(axis=1)
    counts = np.bincount(assign, minlength=KMEANS_K)
    return centers, counts


def merge_centers(centers, counts):
    labs = srgb_to_oklab(centers)
    order = np.argsort(-counts, kind="stable")
    used = set()
    merged = []
    for i in order:
        i = int(i)
        if i in used:
            continue
        acc = centers[i] * counts[i]
        n = int(counts[i])
        used.add(i)
        for j in order:
            j = int(j)
            if j in used:
                continue
            if np.linalg.norm(labs[i] - labs[j]) < MERGE_OKLAB:
                acc = acc + centers[j] * counts[j]
                n += int(counts[j])
                used.add(j)
        avg = acc / n
        merged.append({
            "rgb": avg, "lab": srgb_to_oklab(avg), "count": n, "i": i,
        })
    return merged


def build_candidates(merged, total_count):
    out = []
    for m in merged:
        r, g, b = m["rgb"]
        info = classify(r, g, b, m["lab"])
        share = m["count"] / total_count if total_count > 0 else 0.0
        area_score = math.sqrt(share)
        chroma_boost = 1 + min(info["chroma"] / 0.055, 2.25)
        cool_muted_boost = 1.45 if info["is_cool_muted"] else 1.0
        cool_boost = 1.32 if info["family"] == "blue" else (
            1.18 if info["family"] == "teal" else 1.0)
        vivid_boost = 1.0 if (info["is_earth"] or info["is_neutral"]) else 1.08
        substrate = 0.58 if info["is_earth"] else (
            0.82 if info["is_neutral"] else 1.0)
        out.append({
            **m, **info,
            "share": share,
            "score": (area_score * chroma_boost * cool_muted_boost
                      * cool_boost * vivid_boost * substrate),
        })
    return out


def select_candidates(candidates, target):
    target = max(1, target)
    earth_limit = 2 if target <= 8 else 3
    neutral_limit = 1 if target <= 8 else 2
    family_soft_limit = 2 if target <= 8 else 3
    selected = []
    selected_ids = set()
    family_counts = {}
    earth_count = neutral_count = 0

    def too_close(cand):
        for ex in selected:
            thr = 0.07 if (cand["is_earth"] or ex["is_earth"]
                           or cand["is_neutral"] or ex["is_neutral"]) else 0.055
            if np.linalg.norm(cand["lab"] - ex["lab"]) < thr:
                return True
        return False

    def add(cand):
        nonlocal earth_count, neutral_count
        if cand is None or cand["i"] in selected_ids:
            return False
        selected.append(cand)
        selected_ids.add(cand["i"])
        family_counts[cand["family"]] = family_counts.get(cand["family"], 0) + 1
        if cand["is_earth"]:
            earth_count += 1
        if cand["is_neutral"]:
            neutral_count += 1
        return True

    accent_by_family = {}
    for cand in candidates:
        if cand["is_earth"] or cand["is_neutral"]:
            continue
        accent_by_family.setdefault(cand["family"], cand)
    accent_seeds = sorted(accent_by_family.values(),
                          key=lambda c: -c["score"])
    accent_target = min(target, max(3, math.ceil(target / 3)))
    for cand in accent_seeds:
        if len(selected) >= accent_target:
            break
        if too_close(cand):
            continue
        add(cand)

    blue = next((c for c in candidates
                 if c["family"] == "blue" and c["i"] not in selected_ids), None)
    if (blue is not None and len(selected) < target
            and blue["share"] >= 0.004 and not too_close(blue)):
        add(blue)

    for cand in candidates:
        if len(selected) >= target:
            break
        if cand["i"] in selected_ids:
            continue
        if cand["is_earth"] and earth_count >= earth_limit:
            continue
        if cand["is_neutral"] and neutral_count >= neutral_limit:
            continue
        if (not cand["is_earth"] and not cand["is_neutral"]
                and family_counts.get(cand["family"], 0) >= family_soft_limit):
            continue
        if too_close(cand):
            continue
        add(cand)

    for cand in candidates:
        if len(selected) >= target:
            break
        if cand["i"] in selected_ids or too_close(cand):
            continue
        add(cand)

    for cand in candidates:
        if len(selected) >= target:
            break
        if cand["i"] in selected_ids:
            continue
        add(cand)

    return sorted(selected, key=lambda c: -c["score"])


def extract_palette(img: Image.Image, colors: int = 12,
                    seed: int | None = None, sampling: str = "med"):
    """Full pipeline; returns [{hex, share, family, score}...] best-first."""
    tier = SAMPLING_TIERS.get(sampling, SAMPLING_TIERS["med"])
    rng = random.Random(seed)
    samples = sample_image(img, tier["target"], tier["max_width"])
    if len(samples) < 20:
        raise ValueError("image yields too few opaque samples")
    centers, counts = kmeans(samples, rng)
    merged = merge_centers(centers, counts)
    total = sum(m["count"] for m in merged)
    min_count = max(1, total * tier["min_count_fraction"])
    candidates = build_candidates(
        [m for m in merged if m["count"] >= min_count], total)
    candidates.sort(key=lambda c: -c["score"])
    if not candidates:
        raise ValueError("no palette candidates survived")
    chosen = select_candidates(candidates, colors)
    picked_total = sum(c["count"] for c in chosen)
    out = []
    for c in chosen:
        r, g, b = (int(max(0, min(255, round(v)))) for v in c["rgb"])
        out.append({
            "hex": f"{r:02x}{g:02x}{b:02x}",
            "share": (c["count"] / picked_total) if picked_total else 0.0,
            "family": c["family"],
            "score": c["score"],
        })
    return out


def load_image(source: str) -> Image.Image:
    if source.startswith(("http://", "https://")):
        req = urllib.request.Request(
            source, headers={"User-Agent": "PolyPaint-PaletteExtract/1.0"})
        with urllib.request.urlopen(req, timeout=60) as resp:
            return Image.open(io.BytesIO(resp.read()))
    return Image.open(source)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", help="local path or http(s) URL")
    parser.add_argument("--colors", type=int, default=12)
    parser.add_argument("--sampling", choices=sorted(SAMPLING_TIERS), default="med")
    parser.add_argument("--seed", type=int, default=None)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    palette = extract_palette(load_image(args.image), args.colors, args.seed,
                              sampling=args.sampling)
    if args.json:
        print(json.dumps(palette, indent=2))
    else:
        for p in palette:
            print(f"#{p['hex']}  {p['share']*100:5.1f}%  {p['family']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
