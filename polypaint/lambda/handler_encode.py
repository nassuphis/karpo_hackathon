"""
Encode Lambda handler — converts raw pixel buffer to JPEG/PNG via raw2jpeg.

Single operation: download .raw from S3, encode, upload, return presigned URL.
Supports either explicit tile_keys or a tile_prefix used to derive tile paths.
"""
import json
import os
import struct
import subprocess
import time

import boto3

from color_artifact_meta import split_color_artifact_metadata, write_color_artifact_meta_overlay
from shared import BUCKET, PRESIGN_EXPIRY, parse_body, ok_response, imgpipe_env, report_status

s3 = boto3.client("s3")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")
S3_USER_METADATA_LIMIT_BYTES = 2048


def _metadata_size_bytes(meta):
    total = 0
    for key, value in (meta or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


def _infer_color_artifact_ref(out_key):
    parts = str(out_key or "").strip("/").split("/")
    if len(parts) == 5 and parts[0] == "renders" and parts[2] == "color" and parts[4].startswith("image."):
        return parts[1], parts[3]
    if len(parts) == 3 and parts[0] == "renders" and parts[2].startswith("image."):
        return parts[1], "legacy_color"
    return None, None


def handler(event, context):
    params = parse_body(event)
    out_key = params["out_key"]
    fmt = params.get("format", "jpeg")
    quality = params.get("quality", 90)
    job_id = params.get("job_id")
    task_id = params.get("task_id", "encode")

    try:
        progress = {"phase": "encode", "format": fmt}
        if job_id:
            report_status(job_id, task_id, "started", result_data=progress)

        in_path = "/tmp/encode_in.raw"
        ext = "jpeg" if fmt != "png" else "png"
        out_path = f"/tmp/encode_out.{ext}"

        # 2D tile grid stitching — load one row of tiles at a time
        tile_grid = params["tile_grid"]
        n_cols = int(tile_grid["n_cols"])
        n_rows = int(tile_grid["n_rows"])
        if n_cols <= 0 or n_rows <= 0:
            raise RuntimeError("tile_grid requires positive n_cols and n_rows")
        tile_keys = tile_grid.get("tile_keys") or []
        tile_prefix = str(tile_grid.get("tile_prefix") or "").strip()
        if params.get("width") not in ("", None) or params.get("height") not in ("", None):
            raise RuntimeError("Encode no longer accepts width/height; pass pix for square output")
        pix = int(params["pix"])
        if pix <= 0:
            raise RuntimeError("Encode requires pix > 0")
        total_w = pix
        total_h = pix

        if job_id:
            report_status(job_id, task_id, "stitching", result_data=progress)

        t_stitch = time.time()
        with open(in_path, "wb") as f:
            f.write(struct.pack("<III", total_w, total_h, 3))
            rows_written = 0
            for tr in range(n_rows):
                # Load one row of tiles
                row_tiles = []
                for tc in range(n_cols):
                    tile_id = tr * n_cols + tc
                    if tile_keys:
                        key = tile_keys[tile_id]
                    elif tile_prefix:
                        key = f"{tile_prefix}{tile_id:04d}.raw"
                    else:
                        raise RuntimeError("tile_grid requires tile_keys or tile_prefix")
                    data = s3.get_object(Bucket=BUCKET, Key=key)["Body"].read()
                    tw, th, tb = struct.unpack("<III", data[:12])
                    row_tiles.append((tw, th, tb, data[12:]))

                # Write interleaved pixel rows for this tile row
                th = row_tiles[0][1]
                row_width = sum(int(tile[0]) for tile in row_tiles)
                if row_width != total_w:
                    raise RuntimeError(
                        f"tile row {tr} width mismatch: got {row_width}, expected {total_w}"
                    )
                if any(int(tile[1]) != th for tile in row_tiles):
                    raise RuntimeError(f"tile row {tr} has inconsistent tile heights")
                if any(int(tile[2]) != 3 for tile in row_tiles):
                    raise RuntimeError(f"tile row {tr} has non-RGB tile")
                for py in range(th):
                    for (tw, _, _, pixels_data) in row_tiles:
                        start = py * tw * 3
                        end = start + tw * 3
                        f.write(pixels_data[start:end])
                rows_written += int(th)
                del row_tiles
            if rows_written != total_h:
                raise RuntimeError(
                    f"tile grid height mismatch: got {rows_written}, expected {total_h}"
                )
        progress["stitch_ms"] = int((time.time() - t_stitch) * 1000)

        if job_id:
            report_status(job_id, task_id, "encoding", result_data=progress)

        # Encode
        bilevel = params.get("bilevel", False)
        encode_args = [RAW2JPEG, in_path, out_path]
        if ext == "jpeg":
            encode_args.append(f"--quality={quality}")
        if bilevel:
            encode_args.append("--bilevel")
        t_encode = time.time()
        result = subprocess.run(encode_args, capture_output=True, text=True,
                                timeout=300, env=imgpipe_env())
        progress["encode_ms"] = int((time.time() - t_encode) * 1000)
        if result.returncode != 0:
            raise RuntimeError(f"raw2jpeg failed: {result.stderr.strip()}")
        encode_meta = json.loads(result.stdout)

        # Upload with dimensions plus caller-supplied artifact metadata
        content_type = "image/jpeg" if ext == "jpeg" else "image/png"
        raw_meta = dict(params.get("metadata") or {})
        extra_meta = {}
        for k, v in raw_meta.items():
            if v is None:
                continue
            extra_meta[str(k)] = str(v)
        image_meta = dict(extra_meta)
        overlay_meta = {}
        if str(raw_meta.get("family") or "").strip() == "color":
            job_id_for_overlay, artifact_id_for_overlay = _infer_color_artifact_ref(out_key)
            if not job_id_for_overlay or not artifact_id_for_overlay:
                raise RuntimeError(f"Cannot derive Color artifact path from out_key {out_key!r}")
            image_meta, overlay_meta = split_color_artifact_metadata(raw_meta)
        final_metadata = {"pix": str(pix), "width": str(total_w), "height": str(total_h), **image_meta}
        metadata_size = _metadata_size_bytes(final_metadata)
        if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
            raise RuntimeError(
                f"Image metadata too large before upload: {metadata_size} bytes > "
                f"{S3_USER_METADATA_LIMIT_BYTES} limit"
            )
        t_upload = time.time()
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=out_key,
                          Body=f, ContentType=content_type,
                          Metadata=final_metadata)
        if overlay_meta:
            write_color_artifact_meta_overlay(
                s3,
                BUCKET,
                job_id_for_overlay,
                artifact_id_for_overlay,
                overlay_meta,
            )
        progress["upload_ms"] = int((time.time() - t_upload) * 1000)
        progress["file_size"] = encode_meta["file_size"]

        image_url = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": out_key},
            ExpiresIn=PRESIGN_EXPIRY)

        for p in [in_path, out_path]:
            try:
                os.remove(p)
            except OSError:
                pass

        if job_id:
            report_status(job_id, task_id, "done", result_data=progress)

        return ok_response({
            "out_key": out_key,
            "file_size": encode_meta["file_size"],
            "image_url": image_url,
        })

    except Exception as e:
        if job_id:
            progress = locals().get("progress", {"phase": "encode"})
            progress["error"] = str(e)
            report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
