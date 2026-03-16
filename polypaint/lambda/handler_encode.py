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

        tile_grid = params.get("tile_grid")
        if tile_grid:
            # 2D tile grid stitching — load one row of tiles at a time
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

        elif (tile_keys := params.get("tile_keys")):
            # Concatenate tile .raw files into one full .raw
            total_height = 0
            with open(in_path, "wb") as f:
                f.write(b'\x00' * 12)  # placeholder header
                for tk in tile_keys:
                    data = s3.get_object(Bucket=BUCKET, Key=tk)["Body"].read()
                    w, h, b = struct.unpack("<III", data[:12])
                    if total_height == 0:
                        width, bands = w, b
                    f.write(data[12:])  # pixel data only
                    total_height += h
                # Write correct header
                f.seek(0)
                f.write(struct.pack("<III", width, total_height, bands))
        else:
            # Single raw file
            raw_key = params["raw_key"]
            obj = s3.get_object(Bucket=BUCKET, Key=raw_key)
            with open(in_path, "wb") as f:
                f.write(obj["Body"].read())

        if job_id:
            report_status(job_id, task_id, "encoding")

        # Encode
        encode_args = [RAW2JPEG, in_path, out_path]
        if ext == "jpeg":
            encode_args.append(f"--quality={quality}")
        result = subprocess.run(encode_args, capture_output=True, text=True,
                                timeout=300, env=imgpipe_env())
        if result.returncode != 0:
            raise RuntimeError(f"raw2jpeg failed: {result.stderr.strip()}")
        encode_meta = json.loads(result.stdout)

        # Upload
        content_type = "image/jpeg" if ext == "jpeg" else "image/png"
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=out_key,
                          Body=f, ContentType=content_type)

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
