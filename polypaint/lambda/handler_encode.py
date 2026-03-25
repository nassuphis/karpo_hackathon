"""
Encode Lambda handler — converts raw pixel buffer to JPEG/PNG via raw2jpeg.

Single operation: download .raw from S3, encode, upload, return presigned URL.
Supports tile_keys: array of tile .raw keys to concatenate before encoding.
"""
import json
import os
import struct
import subprocess

import boto3

from shared import BUCKET, PRESIGN_EXPIRY, parse_body, ok_response, imgpipe_env, report_status

s3 = boto3.client("s3")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")


def handler(event, context):
    params = parse_body(event)
    mode = params.get("mode", "encode")
    if mode == "preview":
        return handle_preview(params)
    return handle_encode(params)


def handle_preview(params):
    """Generate a 1024px max-dimension preview PNG from any S3 image."""
    source_key = params["source_key"]
    preview_key = params["preview_key"]
    max_size = params.get("max_size", 1024)

    ext = source_key.rsplit('.', 1)[-1].lower()
    in_path = f"/tmp/preview_in.{ext}"
    out_path = "/tmp/preview_out.png"

    try:
        obj = s3.get_object(Bucket=BUCKET, Key=source_key)
        with open(in_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)

        # Use vipsthumbnail from libvips layer (/opt/bin/)
        vipsthumbnail = "/opt/bin/vipsthumbnail"
        env = imgpipe_env()
        env["PATH"] = "/opt/bin:" + env.get("PATH", "")
        result = subprocess.run(
            [vipsthumbnail, in_path, "-s", f"{max_size}x{max_size}",
             "-o", out_path + "[strip]"],
            capture_output=True, text=True, timeout=120,
            env=env,
        )
        if result.returncode != 0:
            raise RuntimeError(f"vipsthumbnail failed: {result.stderr.strip()}")

        file_size = os.path.getsize(out_path)

        # Copy width/height metadata from source if available
        img_meta = {}
        try:
            src_head = s3.head_object(Bucket=BUCKET, Key=source_key)
            src_meta = src_head.get("Metadata", {})
            if "width" in src_meta:
                img_meta["width"] = src_meta["width"]
            if "height" in src_meta:
                img_meta["height"] = src_meta["height"]
        except Exception:
            pass

        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=preview_key,
                          Body=f, ContentType="image/png",
                          Metadata=img_meta)

        url = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": preview_key},
            ExpiresIn=PRESIGN_EXPIRY)

        return ok_response({
            "preview_key": preview_key,
            "file_size": file_size,
            "url": url,
        })
    finally:
        for p in [in_path, out_path]:
            try:
                os.remove(p)
            except OSError:
                pass


def handle_encode(params):
    out_key = params["out_key"]
    fmt = params.get("format", "jpeg")
    quality = params.get("quality", 90)
    job_id = params.get("job_id")
    task_id = "encode"

    try:
        if job_id:
            report_status(job_id, task_id, "started")

        in_path = "/tmp/encode_in.raw"
        ext = "jpeg" if fmt != "png" else "png"
        out_path = f"/tmp/encode_out.{ext}"

        # 2D tile grid stitching — load one row of tiles at a time
        tile_grid = params["tile_grid"]
        n_cols = tile_grid["n_cols"]
        n_rows = tile_grid["n_rows"]
        total_w = params["width"]
        total_h = params["height"]

        if job_id:
            report_status(job_id, task_id, "stitching")

        with open(in_path, "wb") as f:
            f.write(struct.pack("<III", total_w, total_h, 3))
            for tr in range(n_rows):
                # Load one row of tiles
                row_tiles = []
                for tc in range(n_cols):
                    tile_id = tr * n_cols + tc
                    key = tile_grid["tile_keys"][tile_id]
                    data = s3.get_object(Bucket=BUCKET, Key=key)["Body"].read()
                    tw, th, tb = struct.unpack("<III", data[:12])
                    row_tiles.append((tw, th, data[12:]))

                # Write interleaved pixel rows for this tile row
                th = row_tiles[0][1]
                for py in range(th):
                    for (tw, _, pixels_data) in row_tiles:
                        start = py * tw * 3
                        end = start + tw * 3
                        f.write(pixels_data[start:end])
                del row_tiles

        if job_id:
            report_status(job_id, task_id, "encoding")

        # Encode
        bilevel = params.get("bilevel", False)
        encode_args = [RAW2JPEG, in_path, out_path]
        if ext == "jpeg":
            encode_args.append(f"--quality={quality}")
        if bilevel:
            encode_args.append("--bilevel")
        result = subprocess.run(encode_args, capture_output=True, text=True,
                                timeout=300, env=imgpipe_env())
        if result.returncode != 0:
            raise RuntimeError(f"raw2jpeg failed: {result.stderr.strip()}")
        encode_meta = json.loads(result.stdout)

        # Upload with dimensions in S3 metadata
        content_type = "image/jpeg" if ext == "jpeg" else "image/png"
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=out_key,
                          Body=f, ContentType=content_type,
                          Metadata={"width": str(total_w), "height": str(total_h)})

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
            report_status(job_id, task_id, "done")

        return ok_response({
            "out_key": out_key,
            "file_size": encode_meta["file_size"],
            "image_url": image_url,
        })

    except Exception as e:
        if job_id:
            report_status(job_id, task_id, "error", str(e))
        raise
