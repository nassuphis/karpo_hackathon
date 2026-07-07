"""Composite mosaic-wall pyramid builder (deepzoom-speed.md §7.1).

Runs inside the deepzoom-export lambda (libvips layer + wall_dz binary).
Triggered async by the storage mosaic worker after a manifest refresh:
downloads every wall tile jpg, arrayjoins them into one image in the BAKED
default order, dzsaves a jpg-tile pyramid, uploads it under the
refresh-scoped _index prefix, and flips wall_state on the mosaic status row.
"""

import concurrent.futures
import json
import math
import os
import shutil
import struct
import subprocess
import time
import zlib

import boto3
from botocore.config import Config

from shared import BUCKET, CACHE_IMMUTABLE, imgpipe_env, ok_response

WALL_DZ = os.path.join(os.path.dirname(os.path.abspath(__file__)), "wall_dz")
JOBS_TABLE = os.environ.get("JOBS_TABLE", "polypaint-jobs")
MOSAIC_STATUS_PK = "__allrenders_mosaic__"
STATUS_TASK_BY_KIND = {"color": "color_mosaic_status", "palette": "palette_mosaic_status"}
CELL_PX = 512

# pool sized to the download/upload thread counts (default 10 floods the log
# with discarded-connection warnings and throttles throughput)
s3 = boto3.client("s3", config=Config(max_pool_connections=64))


def _placeholder_png(px=CELL_PX, rgb=(0x12, 0x18, 0x29)):
    """Flat panel-blue cell for a tile whose download fails, so one bad
    artifact cannot sink the whole wall. Pure-stdlib PNG (no PIL here)."""
    def chunk(tag, data):
        payload = tag + data
        return struct.pack(">I", len(data)) + payload + struct.pack(">I", zlib.crc32(payload) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", px, px, 8, 2, 0, 0, 0)
    row = b"\x00" + bytes(rgb) * px
    idat = zlib.compress(row * px, 6)
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr)
            + chunk(b"IDAT", idat) + chunk(b"IEND", b""))


def _desc_str(value):
    # created_at desc while job/artifact tiebreaks stay asc: invert each
    # char code so an ascending tuple sort yields descending timestamps
    return tuple(-ord(c) for c in value)


def _tile_sort_key(tile):
    """The wall's DEFAULT order — must match _mosaicFilteredSortedTiles'
    default branch in js/13-artifact-mosaics.js: created_at desc, job_id asc,
    artifact/palette id asc."""
    aid = str(tile.get("palette_id") or tile.get("artifact_id") or "")
    return (_desc_str(str(tile.get("created_at") or "")),
            str(tile.get("job_id") or ""), aid)


def default_wall_order(tiles):
    return sorted(tiles, key=_tile_sort_key)


def _download_tile(idx, key, work_dir):
    path = os.path.join(work_dir, "%05d" % idx)
    try:
        s3.download_file(BUCKET, key, path + ".jpg")
        return path + ".jpg", False
    except Exception:
        with open(path + ".png", "wb") as fh:
            fh.write(_placeholder_png())
        return path + ".png", True


def _upload_pyramid(prefix, dzi_path, tiles_dir):
    tasks = [(dzi_path, prefix + "wall.dzi", "application/xml")]
    for root, _dirs, files in os.walk(tiles_dir):
        for fname in files:
            local = os.path.join(root, fname)
            rel = os.path.relpath(local, os.path.dirname(tiles_dir))
            ct = "image/jpeg" if fname.endswith((".jpg", ".jpeg")) else "application/octet-stream"
            tasks.append((local, prefix + rel, ct))

    def upload_one(task):
        local, key, ct = task
        with open(local, "rb") as fh:
            s3.put_object(Bucket=BUCKET, Key=key, Body=fh.read(),
                          ContentType=ct, CacheControl=CACHE_IMMUTABLE)
        return 1

    with concurrent.futures.ThreadPoolExecutor(max_workers=50) as pool:
        return sum(pool.map(upload_one, tasks))


def _set_wall_state(kind, refresh_id, state, *, wall_json_key="", error=""):
    ddb = boto3.client("dynamodb", region_name=os.environ.get("AWS_REGION", "us-east-1"))
    try:
        ddb.update_item(
            TableName=JOBS_TABLE,
            Key={"job_id": {"S": MOSAIC_STATUS_PK},
                 "task_id": {"S": STATUS_TASK_BY_KIND[kind]}},
            UpdateExpression=(
                "SET wall_state = :ws, wall_refresh_id = :rid, "
                "wall_json_key = :wk, wall_error = :we, wall_updated_at = :ua"
            ),
            ConditionExpression="refresh_id = :rid",
            ExpressionAttributeValues={
                ":ws": {"S": state},
                ":rid": {"S": refresh_id},
                ":wk": {"S": wall_json_key},
                ":we": {"S": str(error or "")[:512]},
                ":ua": {"S": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())},
            },
        )
    except ddb.exceptions.ConditionalCheckFailedException:
        # a newer refresh owns the status row; this wall is already obsolete
        pass


def handle_build_wall_pyramid(params):
    kind = str(params.get("kind") or "color")
    refresh_id = str(params.get("refresh_id") or "")
    manifest_key = str(params.get("manifest_key") or "")
    if kind not in STATUS_TASK_BY_KIND or not refresh_id or not manifest_key:
        return ok_response({"error": "kind, refresh_id, manifest_key required"})

    work_dir = "/tmp/wall_tiles"
    out_dir = "/tmp/wall_out"
    for d in (work_dir, out_dir):
        shutil.rmtree(d, ignore_errors=True)
        os.makedirs(d, exist_ok=True)

    try:
        manifest = json.loads(s3.get_object(Bucket=BUCKET, Key=manifest_key)["Body"].read())
        tiles = default_wall_order(list(manifest.get("tiles") or []))
        if not tiles:
            _set_wall_state(kind, refresh_id, "error", error="manifest has no tiles")
            return ok_response({"error": "manifest has no tiles"})

        t0 = time.time()
        with concurrent.futures.ThreadPoolExecutor(max_workers=32) as pool:
            results = list(pool.map(
                lambda item: _download_tile(item[0], item[1].get("key") or "", work_dir),
                enumerate(tiles)))
        paths = [p for p, _ in results]
        placeholders = sum(1 for _, ph in results if ph)
        dl_ms = int((time.time() - t0) * 1000)

        cols = max(1, math.ceil(math.sqrt(len(tiles))))
        list_path = os.path.join(out_dir, "tiles.txt")
        with open(list_path, "w") as fh:
            fh.write("\n".join(paths) + "\n")

        t1 = time.time()
        run = subprocess.run(
            [WALL_DZ, list_path, str(cols), os.path.join(out_dir, "wall")],
            capture_output=True, text=True, timeout=780, env=imgpipe_env())
        if run.returncode != 0:
            raise RuntimeError("wall_dz failed: %s" % run.stderr.strip()[:400])
        dims = json.loads(run.stdout)
        dz_ms = int((time.time() - t1) * 1000)
        shutil.rmtree(work_dir, ignore_errors=True)

        prefix = "renders/_index/%s_mosaic/%s/" % (kind, refresh_id)
        t2 = time.time()
        uploaded = _upload_pyramid(prefix, os.path.join(out_dir, "wall.dzi"),
                                   os.path.join(out_dir, "wall_files"))
        wall = {
            "manifest_type": "artifact_wall_pyramid",
            "kind": kind,
            "refresh_id": refresh_id,
            "dzi_key": prefix + "wall.dzi",
            "width": int(dims["width"]),
            "height": int(dims["height"]),
            "cols": cols,
            "rows": math.ceil(len(tiles) / cols),
            "cell_px": CELL_PX,
            "count": len(tiles),
            "placeholders": placeholders,
            "sort": "created",
            "tiles": tiles,
            "download_ms": dl_ms,
            "dzsave_ms": dz_ms,
            "upload_ms": int((time.time() - t2) * 1000),
            "tiles_uploaded": uploaded,
        }
        wall_json_key = prefix + "wall.json"
        s3.put_object(Bucket=BUCKET, Key=wall_json_key,
                      Body=json.dumps(wall, separators=(",", ":")).encode("utf-8"),
                      ContentType="application/json",
                      CacheControl="no-cache, max-age=0")
        _set_wall_state(kind, refresh_id, "ready", wall_json_key=wall_json_key)
        return ok_response({"wall_json_key": wall_json_key, "tiles_uploaded": uploaded,
                            "placeholders": placeholders})
    except Exception as exc:  # noqa: BLE001 — wall is best-effort; manifest already ready
        _set_wall_state(kind, refresh_id, "error", error=str(exc))
        return ok_response({"error": str(exc)})
    finally:
        shutil.rmtree(work_dir, ignore_errors=True)
        shutil.rmtree(out_dir, ignore_errors=True)
