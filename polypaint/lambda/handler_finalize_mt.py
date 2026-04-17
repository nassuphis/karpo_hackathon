"""
Finalize-MT handler for the fused solve-score color path.

Consumes score-byte sparse pixel-bin fragments emitted by raster-mt,
assembles a canonical greyscale raw, equalizes it, renders RGB tiles,
encodes the final image, uploads the raw sidecar, and writes color
artifact metadata directly.
"""
from __future__ import annotations

import json
import os
import struct
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor

import boto3
from botocore.config import Config
from botocore.exceptions import ClientError

from color_artifact_meta import split_color_artifact_metadata, write_color_artifact_meta_overlay
from raw_sidecar import background_color_hex, build_raw_sidecar
from shared import BUCKET, imgpipe_env, ok_response, parse_body, report_status


s3 = boto3.client("s3")
ASSEMBLE_GREYSCALE = os.path.join(os.path.dirname(__file__), "assemble_greyscale")
PIXEL_BINS_RENDER = os.path.join(os.path.dirname(__file__), "pixel_bins_render")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")
VIPS_THUMBNAIL = "/opt/bin/vipsthumbnail"
DEFAULT_FINALIZE_WORKERS = 16
MAX_FINALIZE_WORKERS = 64
S3_USER_METADATA_LIMIT_BYTES = 2048
FRAGMENT_MANIFEST_VERSION = 1
FRAGMENT_PAIR_ENCODING = "u32le_u8_v1"


def _validate_finalize_workers(value):
    if value in (None, ""):
        value = os.environ.get("FINALIZE_MT_WORKERS", DEFAULT_FINALIZE_WORKERS)
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"finalize_workers must be an integer, got {value!r}")
    if not (1 <= workers <= MAX_FINALIZE_WORKERS):
        raise RuntimeError(f"finalize_workers must be in [1, {MAX_FINALIZE_WORKERS}], got {workers}")
    return workers


def _validate_fragment_manifest(manifest, *, source_item_count, fragment_prefix, expected_chain_fingerprint):
    if not isinstance(manifest, dict):
        raise RuntimeError("FinalizeMT requires fragment_manifest")
    try:
        version = int(manifest.get("version") or 0)
    except (TypeError, ValueError):
        raise RuntimeError(f"fragment_manifest.version must be an integer, got {manifest.get('version')!r}")
    if version != FRAGMENT_MANIFEST_VERSION:
        raise RuntimeError(
            f"fragment_manifest.version must be {FRAGMENT_MANIFEST_VERSION}, got {version}"
        )
    pair_encoding = str(manifest.get("pair_encoding") or "").strip()
    if pair_encoding != FRAGMENT_PAIR_ENCODING:
        raise RuntimeError(
            f"fragment_manifest.pair_encoding must be {FRAGMENT_PAIR_ENCODING!r}, got {pair_encoding!r}"
        )
    try:
        manifest_item_count = int(manifest.get("item_count") or 0)
    except (TypeError, ValueError):
        raise RuntimeError(f"fragment_manifest.item_count must be an integer, got {manifest.get('item_count')!r}")
    if manifest_item_count != int(source_item_count):
        raise RuntimeError(
            f"fragment_manifest.item_count mismatch: expected {source_item_count}, got {manifest_item_count}"
        )
    manifest_prefix = str(manifest.get("fragment_prefix") or "").strip()
    if manifest_prefix != str(fragment_prefix or "").strip():
        raise RuntimeError(
            f"fragment_manifest.fragment_prefix mismatch: expected {fragment_prefix!r}, got {manifest_prefix!r}"
        )
    manifest_fingerprint = str(manifest.get("chain_fingerprint") or "").strip()
    if expected_chain_fingerprint and manifest_fingerprint != expected_chain_fingerprint:
        raise RuntimeError(
            "fragment_manifest.chain_fingerprint mismatch: "
            f"expected {expected_chain_fingerprint}, got {manifest_fingerprint!r}"
        )
    return {
        "version": version,
        "pair_encoding": pair_encoding,
        "item_count": manifest_item_count,
        "fragment_prefix": manifest_prefix,
        "chain_fingerprint": manifest_fingerprint,
    }


def _finalize_s3_client(max_workers):
    pool_size = max(16, int(max_workers) * 2)
    return boto3.client("s3", config=Config(max_pool_connections=pool_size))


def _tile_shape(tile_idx, width, height, tile_size, n_tile_cols):
    tile_idx = int(tile_idx)
    width = int(width)
    height = int(height)
    tile_size = int(tile_size)
    n_tile_cols = int(n_tile_cols)
    row = tile_idx // n_tile_cols
    col = tile_idx % n_tile_cols
    tile_w = max(0, min(tile_size, width - col * tile_size))
    tile_h = max(0, min(tile_size, height - row * tile_size))
    return tile_w, tile_h


def _is_missing_s3_error(exc):
    code = getattr(exc, "response", {}).get("Error", {}).get("Code")
    if code in {"NoSuchKey", "404", "NotFound"}:
        return True
    msg = str(exc)
    return "NoSuchKey" in msg or "NotFound" in msg


def _read_body_bytes(body):
    if hasattr(body, "read"):
        return body.read()
    chunks = []
    for chunk in body.iter_chunks(1024 * 1024):
        chunks.append(chunk)
    return b"".join(chunks)


def _load_blob(finalize_s3, key):
    try:
        obj = finalize_s3.get_object(Bucket=BUCKET, Key=key)
        return {"key": key, "data": _read_body_bytes(obj["Body"])}
    except ClientError as exc:
        if not _is_missing_s3_error(exc):
            raise
    return None


def _fragment_key(fragment_prefix, section_idx, tile_idx):
    prefix = str(fragment_prefix or "").strip()
    if prefix:
        return f"{prefix}{int(section_idx):04d}_t{int(tile_idx):04d}.pbx"
    return None


def _tile_origin(tile_idx, tile_size, n_tile_cols):
    row = int(tile_idx) // int(n_tile_cols)
    col = int(tile_idx) % int(n_tile_cols)
    return col * int(tile_size), row * int(tile_size)


def _tile_local_pairs_to_global_fragment_bytes(*, pbx_bytes, tile_idx, tile_w, tile_h, tile_size, n_tile_cols, width):
    if len(pbx_bytes) % 8 != 0:
        raise RuntimeError(
            f"tile-local pbx fragment for tile {tile_idx} has invalid length {len(pbx_bytes)} (expected multiple of 8)"
        )
    tile_x, tile_y = _tile_origin(tile_idx, tile_size, n_tile_cols)
    out = bytearray((len(pbx_bytes) // 8) * 5)
    write_off = 0
    for off in range(0, len(pbx_bytes), 8):
        pix_idx, score = struct.unpack_from("<II", pbx_bytes, off)
        if pix_idx >= tile_w * tile_h:
            raise RuntimeError(
                f"tile-local pbx fragment for tile {tile_idx} has pix_idx {pix_idx} out of bounds for {tile_w}x{tile_h}"
            )
        if not (1 <= score <= 255):
            raise RuntimeError(
                f"tile-local pbx fragment for tile {tile_idx} has invalid score byte {score}; expected 1..255"
            )
        local_y = pix_idx // tile_w
        local_x = pix_idx % tile_w
        global_x = tile_x + local_x
        global_y = tile_y + local_y
        global_idx = global_y * int(width) + global_x
        struct.pack_into("<I", out, write_off, int(global_idx))
        out[write_off + 4] = int(score)
        write_off += 5
    return bytes(out)


def _ordered_prefetch(n_items, workers, load_fn):
    if n_items <= 0:
        return
    bounded_workers = max(1, min(int(workers), int(n_items)))
    with ThreadPoolExecutor(max_workers=bounded_workers) as executor:
        pending = {}
        next_submit = 0
        next_emit = 0
        while next_emit < n_items:
            while next_submit < n_items and len(pending) < bounded_workers:
                idx = next_submit
                pending[idx] = executor.submit(load_fn, idx)
                next_submit += 1
            loaded = pending.pop(next_emit).result()
            yield next_emit, loaded
            next_emit += 1


def _materialize_global_fragment_files(
    *,
    finalize_s3,
    job_id,
    source_item_count,
    width,
    height,
    tile_size,
    n_tile_cols,
    n_tile_rows,
    workers,
    fragment_prefix,
    path_prefix,
):
    fragment_paths = []
    files_seen = 0
    files_loaded = 0
    bytes_loaded = 0
    for tile_idx in range(int(n_tile_cols) * int(n_tile_rows)):
        tile_w, tile_h = _tile_shape(tile_idx, width, height, tile_size, n_tile_cols)
        if tile_w <= 0 or tile_h <= 0:
            continue
        out_path = f"/tmp/{path_prefix}_tile_{tile_idx:04d}.frag"
        wrote_any = False
        with open(out_path, "wb") as out_fh:
            for section_idx, loaded in _ordered_prefetch(
                source_item_count,
                workers,
                lambda idx: _load_blob(
                    finalize_s3,
                    _fragment_key(fragment_prefix, idx, tile_idx) or f"renders/{job_id}/pixbin_chunk_{idx:04d}_t{tile_idx:04d}.pbx",
                ),
            ):
                files_seen = max(files_seen, section_idx + 1)
                if loaded is None:
                    continue
                converted = _tile_local_pairs_to_global_fragment_bytes(
                    pbx_bytes=loaded["data"],
                    tile_idx=tile_idx,
                    tile_w=tile_w,
                    tile_h=tile_h,
                    tile_size=tile_size,
                    n_tile_cols=n_tile_cols,
                    width=width,
                )
                out_fh.write(converted)
                wrote_any = True
                files_loaded += 1
                bytes_loaded += len(loaded["data"])
        if wrote_any:
            fragment_paths.append(out_path)
        else:
            try:
                os.remove(out_path)
            except OSError:
                pass
    return {
        "fragment_paths": fragment_paths,
        "files_seen": files_seen,
        "files_loaded": files_loaded,
        "bytes_loaded": bytes_loaded,
    }


def _assemble_greyscale_raw(*, width, height, raw_path, hist_path, workers, fragment_paths):
    cmd = [
        ASSEMBLE_GREYSCALE,
        f"--width={int(width)}",
        f"--height={int(height)}",
        f"--output={raw_path}",
        f"--hist-output={hist_path}",
        f"--workers={int(workers)}",
        *fragment_paths,
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
    if proc.returncode != 0:
        raise RuntimeError(f"assemble_greyscale failed: {proc.stderr.strip() or 'unknown error'}")
    with open(hist_path, "r", encoding="utf-8") as fh:
        hist = json.load(fh)
    histogram = hist.get("histogram")
    if not isinstance(histogram, list) or len(histogram) != 256:
        raise RuntimeError("assemble_greyscale histogram sidecar missing 256-bin histogram")
    return {
        "histogram": [int(v) for v in histogram],
        "background_pixels": int(hist.get("background_pixels") or 0),
        "nonzero_pixels": int(hist.get("nonzero_pixels") or 0),
    }


def _build_equalization_lut(histogram):
    total = sum(histogram[1:])
    lut = [0] * 256
    if total <= 0:
        return lut, 0
    cumulative = 0
    for value in range(1, 256):
        cumulative += histogram[value]
        equalized = int(round((cumulative * 255.0) / total))
        if equalized < 1:
            equalized = 1
        if equalized > 255:
            equalized = 255
        lut[value] = equalized
    return lut, total


def _equalized_byte_to_palette_bin(value):
    if value <= 0:
        return 255
    bin_idx = int(((value - 1) * 10) / 255)
    if bin_idx < 0:
        bin_idx = 0
    if bin_idx > 9:
        bin_idx = 9
    return bin_idx


def _write_render_bin_tile_from_raw(*, raw_path, out_path, width, tile_x, tile_y, tile_w, tile_h, lut):
    with open(raw_path, "rb") as raw_fh, open(out_path, "wb") as out_fh:
        for row in range(tile_h):
            raw_fh.seek((tile_y + row) * int(width) + tile_x)
            row_bytes = raw_fh.read(tile_w)
            if len(row_bytes) != tile_w:
                raise RuntimeError(
                    f"greyscale raw short read at row {tile_y + row}: got {len(row_bytes)} bytes, expected {tile_w}"
                )
            mapped = bytearray(tile_w)
            for idx, value in enumerate(row_bytes):
                mapped[idx] = _equalized_byte_to_palette_bin(lut[value])
            out_fh.write(mapped)


def _load_clip_slots(finalize_s3, clip_key, expected_chain_fingerprint):
    clip_key = str(clip_key or "").strip()
    if not clip_key:
        raise RuntimeError("FinalizeMT requires solve_score_clip_key to build the raw sidecar schema")
    obj = finalize_s3.get_object(Bucket=BUCKET, Key=clip_key)
    clip_data = json.loads(_read_body_bytes(obj["Body"]) or b"{}")
    actual_fp = str(clip_data.get("chain_fingerprint") or "").strip()
    if not actual_fp:
        raise RuntimeError("solve-score clip artifact missing chain_fingerprint")
    if expected_chain_fingerprint and actual_fp != expected_chain_fingerprint:
        raise RuntimeError(
            f"solve-score clip artifact fingerprint mismatch: expected {expected_chain_fingerprint}, got {actual_fp!r}"
        )
    metrics = clip_data.get("metrics")
    if not isinstance(metrics, list) or not metrics:
        raise RuntimeError("solve-score clip artifact missing metrics")
    clip_slots = []
    for idx, row in enumerate(metrics):
        clip_slots.append(
            {
                "slot": int(row.get("slot", idx)),
                "metric": str(row.get("metric") or "").strip(),
                "source": str(row.get("source", "slv") or "slv").strip(),
                "clip_lo": float(row["clip_lo"]),
                "clip_hi": float(row["clip_hi"]),
            }
        )
    return {
        "clip_slots": clip_slots,
        "program": str(clip_data.get("program") or "").strip(),
        "chain_fingerprint": actual_fp,
    }


def _render_rgb_tiles(*, tile_infos, palette, background_color):
    rendered_tiles = []
    for tile in tile_infos:
        tile_idx = tile["tile_idx"]
        bin_path = f"/tmp/fused_bins_tile_{tile_idx:04d}.u8"
        raw_path = f"/tmp/fused_rgb_tile_{tile_idx:04d}.raw"
        with open(bin_path, "wb") as out:
            out.write(tile["render_bins"])
        render = subprocess.run(
            [
                PIXEL_BINS_RENDER,
                bin_path,
                raw_path,
                f"--tile_w={tile['tile_w']}",
                f"--tile_h={tile['tile_h']}",
                f"--palette={palette}",
                f"--background_color={background_color}",
                "--empty=255",
            ],
            capture_output=True,
            text=True,
            timeout=300,
        )
        if render.returncode != 0:
            raise RuntimeError(f"pixel_bins_render failed for tile {tile_idx}: {render.stderr.strip() or 'unknown error'}")
        rendered_tiles.append({
            "tile_idx": tile_idx,
            "tile_w": tile["tile_w"],
            "tile_h": tile["tile_h"],
            "path": raw_path,
        })
    return rendered_tiles


def _stitch_tile_grid_to_rgb_raw(*, out_path, width, height, n_tile_rows, n_tile_cols, rendered_tiles):
    with open(out_path, "wb") as out:
        out.write(struct.pack("<III", width, height, 3))
        for tile_row in range(n_tile_rows):
            row_tiles = rendered_tiles[tile_row * n_tile_cols:(tile_row + 1) * n_tile_cols]
            if not row_tiles:
                continue
            row_payloads = []
            for tile in row_tiles:
                data = open(tile["path"], "rb").read()
                tile_w, tile_h, bands = struct.unpack("<III", data[:12])
                if bands != 3:
                    raise RuntimeError(f"tile {tile['tile_idx']} raw bands mismatch: expected 3, got {bands}")
                row_payloads.append((tile_w, tile_h, data[12:]))
            tile_h = row_payloads[0][1]
            for py in range(tile_h):
                for tile_w, _, pixels in row_payloads:
                    start = py * tile_w * 3
                    end = start + tile_w * 3
                    out.write(pixels[start:end])


def _metadata_size_bytes(meta):
    total = 0
    for key, value in (meta or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _encode_preview(source_path, preview_path):
    prev = subprocess.run(
        [VIPS_THUMBNAIL, source_path, "-s", "512x512", "-o", preview_path + "[strip]"],
        capture_output=True,
        text=True,
        timeout=120,
        env=imgpipe_env(),
    )
    if prev.returncode != 0:
        raise RuntimeError(f"Preview generation failed: {prev.stderr.strip() or 'unknown error'}")


def _finalize_associated_palette(
    *,
    finalize_s3,
    job_id,
    run_id,
    workers,
    source_item_count,
    tile_size,
    associated_palette,
    associated_palette_grid_n,
    associated_palette_times,
    associated_palette_degree,
    plan_params_digest,
    render_execution,
    metadata,
    clip_slots,
    chain_fingerprint,
    score_program,
):
    grid_n = int(associated_palette_grid_n or 0)
    if grid_n <= 0:
        raise RuntimeError("associated palette finalize requires associated_palette_grid_n > 0")
    n_tile_cols = (grid_n + tile_size - 1) // tile_size
    n_tile_rows = (grid_n + tile_size - 1) // tile_size
    fragment_prefix = str(associated_palette.get("fragment_prefix") or "").strip()
    if not fragment_prefix:
        raise RuntimeError("associated palette finalize requires fragment_prefix")

    raw_path = "/tmp/assoc_palette_greyscale.raw"
    hist_path = "/tmp/assoc_palette_greyscale.hist.json"
    fragment_info = _materialize_global_fragment_files(
        finalize_s3=finalize_s3,
        job_id=job_id,
        source_item_count=source_item_count,
        width=grid_n,
        height=grid_n,
        tile_size=tile_size,
        n_tile_cols=n_tile_cols,
        n_tile_rows=n_tile_rows,
        workers=workers,
        fragment_prefix=fragment_prefix,
        path_prefix="assoc_palette",
    )
    hist_meta = _assemble_greyscale_raw(
        width=grid_n,
        height=grid_n,
        raw_path=raw_path,
        hist_path=hist_path,
        workers=workers,
        fragment_paths=fragment_info["fragment_paths"],
    )
    histogram = hist_meta["histogram"]
    lut, nonzero_pixels = _build_equalization_lut(histogram)
    render_tiles = []
    for tile_idx in range(n_tile_cols * n_tile_rows):
        tile_w, tile_h = _tile_shape(tile_idx, grid_n, grid_n, tile_size, n_tile_cols)
        tile_x, tile_y = _tile_origin(tile_idx, tile_size, n_tile_cols)
        bin_path = f"/tmp/assoc_palette_bins_tile_{tile_idx:04d}.u8"
        _write_render_bin_tile_from_raw(
            raw_path=raw_path,
            out_path=bin_path,
            width=grid_n,
            tile_x=tile_x,
            tile_y=tile_y,
            tile_w=tile_w,
            tile_h=tile_h,
            lut=lut,
        )
        render_tiles.append({
            "tile_idx": tile_idx,
            "tile_w": tile_w,
            "tile_h": tile_h,
            "render_bins_path": bin_path,
        })

    rendered_tiles = _render_rgb_tiles(
        tile_infos=[{
            "tile_idx": tile["tile_idx"],
            "tile_w": tile["tile_w"],
            "tile_h": tile["tile_h"],
            "render_bins": open(tile["render_bins_path"], "rb").read(),
        } for tile in render_tiles],
        palette=str(associated_palette.get("palette") or metadata.get("palette") or "inferno"),
        background_color=background_color_hex(metadata.get("background_color", [0, 0, 0])),
    )
    encode_in_path = "/tmp/assoc_palette_encode_in.raw"
    encode_out_path = "/tmp/assoc_palette.jpeg"
    _stitch_tile_grid_to_rgb_raw(
        out_path=encode_in_path,
        width=grid_n,
        height=grid_n,
        n_tile_rows=n_tile_rows,
        n_tile_cols=n_tile_cols,
        rendered_tiles=rendered_tiles,
    )
    encode = subprocess.run(
        [RAW2JPEG, encode_in_path, encode_out_path, "--quality=90"],
        capture_output=True,
        text=True,
        timeout=600,
        env=imgpipe_env(),
    )
    if encode.returncode != 0:
        raise RuntimeError(f"associated palette raw2jpeg failed: {encode.stderr.strip() or 'unknown error'}")
    encode_meta = json.loads(encode.stdout or "{}")

    preview_path = "/tmp/assoc_palette_preview.png"
    _encode_preview(encode_out_path, preview_path)

    raw_key = str(associated_palette["raw_key"])
    raw_meta_key = str(associated_palette["raw_meta_key"])
    image_key = str(associated_palette["image_key"])
    preview_key = str(associated_palette["preview_key"])
    meta_key = str(associated_palette["meta_key"])
    palette_id = str(associated_palette.get("palette_id") or "")
    created_at = _utc_now_iso()

    sidecar = build_raw_sidecar(
        job_id=job_id,
        run_id=run_id,
        artifact_family="palette",
        artifact_id=palette_id,
        width=grid_n,
        height=grid_n,
        chain_fingerprint=chain_fingerprint,
        score_chain=associated_palette.get("score_chain", metadata.get("solve_score_chain", "")),
        score_program=score_program,
        clip_slots=clip_slots,
        background_color=metadata.get("background_color", [0, 0, 0]),
        plan_params_digest=plan_params_digest,
        render_execution=render_execution,
        raw_key=raw_key,
        image_key=image_key,
        preview_key=preview_key,
        meta_key=meta_key,
        created_at=created_at,
    )

    with open(raw_path, "rb") as raw_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=raw_key,
            Body=raw_fh,
            ContentType="application/octet-stream",
        )
    finalize_s3.put_object(
        Bucket=BUCKET,
        Key=raw_meta_key,
        Body=json.dumps(sidecar, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )

    image_metadata = {
        "width": str(grid_n),
        "height": str(grid_n),
        "palette": str(associated_palette.get("palette") or metadata.get("palette") or ""),
        "full_n": str(grid_n),
        "times": str(int(associated_palette_times or 1)),
        "using_pass": "0",
    }
    metadata_size = _metadata_size_bytes(image_metadata)
    if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
        raise RuntimeError(
            f"associated palette image metadata too large before upload: {metadata_size} bytes > {S3_USER_METADATA_LIMIT_BYTES} limit"
        )
    with open(encode_out_path, "rb") as out_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=image_key,
            Body=out_fh,
            ContentType="image/jpeg",
            Metadata=image_metadata,
        )
    with open(preview_path, "rb") as preview_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=preview_key,
            Body=preview_fh,
            ContentType="image/png",
        )

    meta_body = {
        "job_id": job_id,
        "palette_id": palette_id,
        "created_at": created_at,
        "display_name": str(associated_palette.get("display_name") or palette_id),
        "palette": str(associated_palette.get("palette") or metadata.get("palette") or ""),
        "degree": int(associated_palette_degree or 0),
        "N": grid_n,
        "times": int(associated_palette_times or 1),
        "using_pass": 0,
        "image_pass": 0,
        "base_grid_solves": grid_n * grid_n,
        "total_solves": grid_n * grid_n,
        "pass_count": int(associated_palette_times or 1),
        "data_layout": "fused_pass0_raw_v1",
        "render_reusable": False,
        "file_size": int(encode_meta.get("file_size", os.path.getsize(encode_out_path))),
        "image_key": image_key,
        "preview_key": preview_key,
        "raw_key": raw_key,
        "raw_meta_key": raw_meta_key,
        "metric": str(associated_palette.get("metric") or metadata.get("solve_metric") or ""),
        "solve_score_chain": associated_palette.get("score_chain", metadata.get("solve_score_chain", "")),
        "chain_fingerprint": chain_fingerprint,
        "derived_from_color_artifact_id": str(associated_palette.get("source_color_artifact_id") or ""),
        "derivation_kind": "extract_palette",
    }
    finalize_s3.put_object(
        Bucket=BUCKET,
        Key=meta_key,
        Body=json.dumps(meta_body, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )
    return {
        "palette_id": palette_id,
        "image_key": image_key,
        "preview_key": preview_key,
        "raw_key": raw_key,
        "raw_meta_key": raw_meta_key,
        "file_size": meta_body["file_size"],
    }


def handler(event, context):
    params = parse_body(event)
    phase = str(params.get("phase") or "").strip()
    if phase != "finalize_mt":
        raise RuntimeError(f"FinalizeMT requires phase='finalize_mt', got {phase!r}")
    job_id = params["job_id"]
    run_id = str(params.get("run_id") or "")
    task_id = params.get("task_id", "finalize_mt")
    mode = str(params.get("mode") or "").strip()
    width = int(params["width"])
    height = int(params["height"])
    tile_size = int(params["tile_size"])
    n_tile_cols = int(params["n_tile_cols"])
    n_tile_rows = int(params["n_tile_rows"])
    source_item_count = int(params.get("source_item_count", params.get("raster_item_count", 0)) or 0)
    if source_item_count <= 0:
        raise RuntimeError("FinalizeMT requires source_item_count > 0")
    fmt = str(params.get("format", "jpeg") or "jpeg").lower()
    quality = int(params.get("quality", 90) or 90)
    palette = str(params.get("palette", "inferno") or "inferno")
    background_color = str(params.get("background_color", "000000") or "000000")
    image_key = str(params["image_key"])
    preview_key = str(params.get("preview_key") or "")
    meta_key = str(params.get("meta_key") or "")
    raw_key = str(params["raw_key"])
    raw_meta_key = str(params["raw_meta_key"])
    plan_params_digest = str(params.get("plan_params_digest") or "").strip()
    solve_score_clip_key = str(params.get("solve_score_clip_key") or "").strip()
    fragment_prefix = str(params.get("fragment_prefix") or "").strip()
    fragment_manifest = dict(params.get("fragment_manifest") or {})
    associated_palette = dict(params.get("associated_palette") or {})
    associated_palette_grid_n = int(params.get("associated_palette_grid_n") or 0)
    associated_palette_times = int(params.get("associated_palette_times") or 0)
    associated_palette_degree = int(params.get("associated_palette_degree") or 0)
    render_execution = dict(params.get("render_execution") or {})
    metadata = dict(params.get("metadata") or {})
    workers = _validate_finalize_workers(params.get("finalize_workers", DEFAULT_FINALIZE_WORKERS))
    if str(render_execution.get("raster_engine") or "") != "mt":
        raise RuntimeError(
            f"FinalizeMT requires render_execution.raster_engine='mt', got {render_execution.get('raster_engine')!r}"
        )
    expected_chain_fingerprint = str(metadata.get("solve_score_chain_fingerprint") or metadata.get("chain_fingerprint") or "").strip()
    _validate_fragment_manifest(
        fragment_manifest,
        source_item_count=source_item_count,
        fragment_prefix=fragment_prefix,
        expected_chain_fingerprint=expected_chain_fingerprint,
    )
    if str(metadata.get("color_mode") or "") != "solve_score":
        raise RuntimeError("FinalizeMT currently supports only solve_score color artifacts")

    finalize_s3 = _finalize_s3_client(workers)
    progress = {
        "phase": "finalize_mt",
        "source_item_count": source_item_count,
        "width": width,
        "height": height,
        "workers": workers,
    }
    report_status(job_id, task_id, "started", result_data=progress)
    clip_info = _load_clip_slots(finalize_s3, solve_score_clip_key, expected_chain_fingerprint)
    if clip_info["program"] and metadata.get("score_program") and clip_info["program"] != str(metadata.get("score_program")):
        raise RuntimeError(
            f"FinalizeMT score_program mismatch: expected {metadata.get('score_program')!r}, got {clip_info['program']!r}"
        )

    histogram = [0] * 256
    t_assemble = time.time()
    raw_path = "/tmp/greyscale.raw"
    hist_path = "/tmp/greyscale.hist.json"
    fragment_info = _materialize_global_fragment_files(
        finalize_s3=finalize_s3,
        job_id=job_id,
        source_item_count=source_item_count,
        width=width,
        height=height,
        tile_size=tile_size,
        n_tile_cols=n_tile_cols,
        n_tile_rows=n_tile_rows,
        workers=workers,
        fragment_prefix=fragment_prefix,
        path_prefix="main",
    )
    hist_meta = _assemble_greyscale_raw(
        width=width,
        height=height,
        raw_path=raw_path,
        hist_path=hist_path,
        workers=workers,
        fragment_paths=fragment_info["fragment_paths"],
    )
    histogram = hist_meta["histogram"]
    progress["assemble_tiles_ms"] = int((time.time() - t_assemble) * 1000)
    report_status(job_id, task_id, "assembled_score_tiles", result_data=progress)

    t_raw = time.time()
    progress["write_raw_ms"] = int((time.time() - t_raw) * 1000)
    progress["raw_size"] = os.path.getsize(raw_path)
    report_status(job_id, task_id, "wrote_greyscale_raw", result_data=progress)

    lut, nonzero_pixels = _build_equalization_lut(histogram)
    progress["nonzero_pixels"] = hist_meta["nonzero_pixels"]
    progress["background_pixels"] = hist_meta["background_pixels"]

    t_render = time.time()
    render_inputs = []
    for tile_idx in range(n_tile_cols * n_tile_rows):
        tile_w, tile_h = _tile_shape(tile_idx, width, height, tile_size, n_tile_cols)
        tile_x, tile_y = _tile_origin(tile_idx, tile_size, n_tile_cols)
        bin_path = f"/tmp/fused_bins_tile_{tile_idx:04d}.u8"
        _write_render_bin_tile_from_raw(
            raw_path=raw_path,
            out_path=bin_path,
            width=width,
            tile_x=tile_x,
            tile_y=tile_y,
            tile_w=tile_w,
            tile_h=tile_h,
            lut=lut,
        )
        with open(bin_path, "rb") as fh:
            render_inputs.append({
                "tile_idx": tile_idx,
                "tile_w": tile_w,
                "tile_h": tile_h,
                "render_bins": fh.read(),
            })
    rendered_tiles = _render_rgb_tiles(tile_infos=render_inputs, palette=palette, background_color=background_color)
    progress["render_tiles_ms"] = int((time.time() - t_render) * 1000)
    report_status(job_id, task_id, "rendered_rgb_tiles", result_data=progress)

    encode_in_path = "/tmp/finalize_mt_encode_in.raw"
    ext = "png" if fmt == "png" else "jpeg"
    encode_out_path = f"/tmp/finalize_mt_out.{ext}"
    t_encode = time.time()
    _stitch_tile_grid_to_rgb_raw(
        out_path=encode_in_path,
        width=width,
        height=height,
        n_tile_rows=n_tile_rows,
        n_tile_cols=n_tile_cols,
        rendered_tiles=rendered_tiles,
    )
    encode_args = [RAW2JPEG, encode_in_path, encode_out_path]
    if ext == "jpeg":
        encode_args.append(f"--quality={quality}")
    encode = subprocess.run(encode_args, capture_output=True, text=True, timeout=600, env=imgpipe_env())
    if encode.returncode != 0:
        raise RuntimeError(f"raw2jpeg failed: {encode.stderr.strip() or 'unknown error'}")
    encode_meta = json.loads(encode.stdout or "{}")
    progress["encode_ms"] = int((time.time() - t_encode) * 1000)
    progress["file_size"] = int(encode_meta.get("file_size", os.path.getsize(encode_out_path)))
    report_status(job_id, task_id, "encoded", result_data=progress)

    sidecar = build_raw_sidecar(
        job_id=job_id,
        run_id=run_id,
        artifact_family=str(metadata.get("family") or mode or "color"),
        artifact_id=str(metadata.get("artifact_id") or ""),
        width=width,
        height=height,
        chain_fingerprint=clip_info["chain_fingerprint"],
        score_chain=metadata.get("solve_score_chain", ""),
        score_program=str(metadata.get("score_program") or clip_info["program"]),
        clip_slots=clip_info["clip_slots"],
        background_color=background_color,
        plan_params_digest=plan_params_digest,
        render_execution=render_execution,
        raw_key=raw_key,
        image_key=image_key,
        preview_key=preview_key,
        meta_key=meta_key,
        created_at=str(metadata.get("created_at") or _utc_now_iso()),
    )

    t_upload = time.time()
    with open(raw_path, "rb") as raw_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=raw_key,
            Body=raw_fh,
            ContentType="application/octet-stream",
        )
    finalize_s3.put_object(
        Bucket=BUCKET,
        Key=raw_meta_key,
        Body=json.dumps(sidecar, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )

    final_metadata = dict(metadata)
    final_metadata["render_execution"] = render_execution
    final_metadata["raw_key"] = raw_key
    final_metadata["raw_meta_key"] = raw_meta_key
    final_metadata["repalette_capable"] = False
    final_metadata["pixel_bins_prefix"] = ""
    final_metadata["pixel_bins_empty"] = ""
    final_metadata["pixel_bins_layout"] = ""
    image_meta, overlay_meta = split_color_artifact_metadata(final_metadata)
    final_headers = {"width": str(width), "height": str(height), **image_meta}
    metadata_size = _metadata_size_bytes(final_headers)
    if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
        raise RuntimeError(
            f"image metadata too large before upload: {metadata_size} bytes > {S3_USER_METADATA_LIMIT_BYTES} limit"
        )
    content_type = "image/png" if ext == "png" else "image/jpeg"
    with open(encode_out_path, "rb") as out_fh:
        finalize_s3.put_object(
            Bucket=BUCKET,
            Key=image_key,
            Body=out_fh,
            ContentType=content_type,
            Metadata=final_headers,
        )
    artifact_id = str(final_metadata.get("artifact_id") or "").strip()
    if artifact_id:
        write_color_artifact_meta_overlay(finalize_s3, BUCKET, job_id, artifact_id, overlay_meta)
    associated_palette_result = None
    if str(associated_palette.get("mode") or "") == "generated":
        associated_palette_result = _finalize_associated_palette(
            finalize_s3=finalize_s3,
            job_id=job_id,
            run_id=run_id,
            workers=workers,
            source_item_count=source_item_count,
            tile_size=tile_size,
            associated_palette=associated_palette,
            associated_palette_grid_n=associated_palette_grid_n,
            associated_palette_times=associated_palette_times,
            associated_palette_degree=associated_palette_degree,
            plan_params_digest=plan_params_digest,
            render_execution=render_execution,
            metadata=metadata,
            clip_slots=clip_info["clip_slots"],
            chain_fingerprint=clip_info["chain_fingerprint"],
            score_program=str(metadata.get("score_program") or clip_info["program"]),
        )
    progress["upload_ms"] = int((time.time() - t_upload) * 1000)
    progress["image_key"] = image_key
    progress["raw_key"] = raw_key
    if associated_palette_result:
        progress["associated_palette_image_key"] = associated_palette_result["image_key"]
    report_status(job_id, task_id, "done", result_data=progress)

    result = {
        "image_key": image_key,
        "raw_key": raw_key,
        "raw_meta_key": raw_meta_key,
        "file_size": progress["file_size"],
        "timings": {
            "assemble_tiles_ms": progress["assemble_tiles_ms"],
            "write_raw_ms": progress["write_raw_ms"],
            "render_tiles_ms": progress["render_tiles_ms"],
            "encode_ms": progress["encode_ms"],
            "upload_ms": progress["upload_ms"],
        },
    }
    if associated_palette_result:
        result["associated_palette"] = associated_palette_result
    return ok_response(result)
