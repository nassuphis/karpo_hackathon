from __future__ import annotations

import json
import os
import subprocess

from shared import imgpipe_env


SCORE_RAW_RENDER = os.path.join(os.path.dirname(__file__), "score_raw_render")
RAW_HISTOGRAM_CHUNK_BYTES = 8 * 1024 * 1024


def histogram_from_raw_path(raw_path, *, expected_size=None):
    histogram = [0] * 256
    total = 0
    with open(raw_path, "rb") as fh:
        while True:
            chunk = fh.read(RAW_HISTOGRAM_CHUNK_BYTES)
            if not chunk:
                break
            total += len(chunk)
            for value in chunk:
                histogram[value] += 1
    if expected_size is not None and total != int(expected_size):
        raise RuntimeError(
            f"greyscale raw size mismatch: got {total} bytes, expected {int(expected_size)}"
        )
    return histogram


def build_equalization_lut(histogram):
    total = sum(int(v) for v in histogram[1:])
    lut = [0] * 256
    if total <= 0:
        return lut, 0
    cumulative = 0
    for value in range(1, 256):
        cumulative += int(histogram[value])
        equalized = int(round((cumulative * 255.0) / total))
        if equalized < 1:
            equalized = 1
        if equalized > 255:
            equalized = 255
        lut[value] = equalized
    return lut, total


def write_equalization_lut(path, histogram):
    eq_lut, nonzero_pixels = build_equalization_lut(histogram)
    with open(path, "wb") as fh:
        fh.write(bytes(eq_lut))
    return nonzero_pixels


def render_score_raw(
    *,
    raw_path,
    out_path,
    preview_path,
    width,
    height,
    eq_lut_path,
    palette,
    background_color,
    quality,
):
    cmd = [
        SCORE_RAW_RENDER,
        raw_path,
        out_path,
        f"--width={int(width)}",
        f"--height={int(height)}",
        f"--eq_lut={eq_lut_path}",
        f"--palette={str(palette or 'inferno')}",
        f"--background_color={str(background_color or '000000')}",
        f"--quality={int(quality)}",
    ]
    if preview_path:
        cmd.extend([f"--preview={preview_path}", "--preview_max=512"])
    proc = subprocess.run(cmd, capture_output=True, text=True, timeout=600, env=imgpipe_env())
    if proc.returncode != 0:
        raise RuntimeError(f"score_raw_render failed: {proc.stderr.strip() or 'unknown error'}")
    try:
        meta = json.loads(proc.stdout or "{}")
    except Exception as exc:
        raise RuntimeError("score_raw_render returned invalid JSON") from exc
    return {
        "file_size": int(meta.get("file_size") or os.path.getsize(out_path)),
        "preview_file_size": int(
            meta.get("preview_file_size")
            or (os.path.getsize(preview_path) if preview_path and os.path.exists(preview_path) else 0)
        ),
    }
