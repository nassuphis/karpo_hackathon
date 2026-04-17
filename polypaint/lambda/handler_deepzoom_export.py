"""
DeepZoom export Lambda — generates OpenSeadragon tile pyramid from an
existing rendered image or, when available, a fused greyscale raw sidecar.

Downloads the source image or raw sidecar, materializes a DeepZoom source
image locally, runs dz_export (libvips dzsave), uploads .dzi +
tile files to S3 under deepzoom/{job_id}/{export_id}/.

Public access via bucket policy on deepzoom/ prefix (no per-object ACL).
"""
import json
import os
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor

import boto3

from raw_sidecar import background_color_hex, validate_raw_sidecar
from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
DZ_EXPORT = os.path.join(os.path.dirname(__file__), "dz_export")
VIEWER_TEMPLATE = os.path.join(os.path.dirname(__file__), "deepzoom_viewer_template.html")
PIXEL_BINS_RENDER = os.path.join(os.path.dirname(__file__), "pixel_bins_render")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")
DEFAULT_BACKGROUND_COLOR = "000000"
DEFAULT_PIXEL_BINS_EMPTY = 255


def _render_viewer(job_id, export_id, created_at):
    """Render standalone viewer HTML from template."""
    with open(VIEWER_TEMPLATE) as f:
        tmpl = f.read()
    return tmpl.replace("{job_id}", job_id) \
               .replace("{export_id}", export_id) \
               .replace("{created_at}", created_at)


def _read_body_to_path(obj_body, path):
    with open(path, "wb") as fh:
        if hasattr(obj_body, "iter_chunks"):
            for chunk in obj_body.iter_chunks(chunk_size=1024 * 1024):
                fh.write(chunk)
        else:
            fh.write(obj_body.read())


def _load_json_key(key):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"].read()
    data = json.loads(body) if body else {}
    if not isinstance(data, dict):
        raise RuntimeError(f"Expected JSON object in {key}")
    return data


def _normalize_background_color(value):
    s = str(value or "").strip().lower()
    if s.startswith("#"):
        s = s[1:]
    if len(s) == 6 and all(ch in "0123456789abcdef" for ch in s):
        return s
    return DEFAULT_BACKGROUND_COLOR


def _equalized_byte_to_palette_bin(value):
    if value <= 0:
        return DEFAULT_PIXEL_BINS_EMPTY
    bin_idx = int(((int(value) - 1) * 10) / 255)
    if bin_idx < 0:
        bin_idx = 0
    if bin_idx > 9:
        bin_idx = 9
    return bin_idx


def _write_palette_bins_from_raw(raw_path, bins_path, width, height):
    width = int(width)
    height = int(height)
    expected_size = width * height
    actual_size = os.path.getsize(raw_path)
    if actual_size != expected_size:
        raise RuntimeError(
            f"greyscale raw size mismatch: got {actual_size} bytes, expected {expected_size}"
        )
    with open(raw_path, "rb") as raw_fh, open(bins_path, "wb") as bins_fh:
        for row_idx in range(height):
            row = raw_fh.read(width)
            if len(row) != width:
                raise RuntimeError(
                    f"greyscale raw short read on row {row_idx}: got {len(row)} bytes, expected {width}"
                )
            mapped = bytearray(width)
            for idx, value in enumerate(row):
                mapped[idx] = _equalized_byte_to_palette_bin(value)
            bins_fh.write(mapped)


def _materialize_image_from_raw(raw_key, raw_meta_key, source_key):
    raw_meta = validate_raw_sidecar(
        _load_json_key(raw_meta_key),
        expected_raw_key=raw_key,
        expected_artifact_family="color",
    )
    width = int(raw_meta["width"])
    height = int(raw_meta["height"])
    palette = "inferno"
    if source_key:
        try:
            head = s3.head_object(Bucket=BUCKET, Key=source_key)
            palette = str((head.get("Metadata") or {}).get("palette") or palette)
        except Exception:
            palette = "inferno"
    background_color = background_color_hex(raw_meta.get("background_color"))
    raw_path = "/tmp/source.greyscale.raw"
    bins_path = "/tmp/source.deepzoom.bins"
    rgb_raw_path = "/tmp/source.deepzoom.rgb.raw"
    image_path = "/tmp/source.deepzoom.png"

    raw_obj = s3.get_object(Bucket=BUCKET, Key=raw_key)
    _read_body_to_path(raw_obj["Body"], raw_path)
    _write_palette_bins_from_raw(raw_path, bins_path, width, height)

    render = subprocess.run(
        [
            PIXEL_BINS_RENDER,
            bins_path,
            rgb_raw_path,
            f"--tile_w={width}",
            f"--tile_h={height}",
            f"--palette={palette}",
            f"--background_color={background_color}",
            f"--empty={DEFAULT_PIXEL_BINS_EMPTY}",
        ],
        capture_output=True,
        text=True,
        timeout=600,
    )
    if render.returncode != 0:
        raise RuntimeError(f"pixel_bins_render failed: {render.stderr.strip() or 'unknown error'}")

    encode = subprocess.run(
        [RAW2JPEG, rgb_raw_path, image_path],
        capture_output=True,
        text=True,
        timeout=600,
        env=imgpipe_env(),
    )
    if encode.returncode != 0:
        raise RuntimeError(f"raw2jpeg failed: {encode.stderr.strip() or 'unknown error'}")

    return image_path, {
        "width": width,
        "height": height,
        "raw_key": raw_key,
        "raw_meta_key": raw_meta_key,
        "palette": palette,
        "background_color": background_color,
    }


def handle_deepzoom_export_request(params, *, require_raw_sidecar=False, task_id="deepzoom_export"):
    job_id = params["job_id"]
    source_key = params["source_key"]
    raw_key = str(params.get("raw_key") or "").strip()
    raw_meta_key = str(params.get("raw_meta_key") or "").strip()
    export_id = params.get("export_id", f"dz_{int(time.time())}")

    try:
        report_status(job_id, task_id, "started")

        t0 = time.time()
        source_kind = "image"
        source_meta = {}
        if require_raw_sidecar and not (raw_key and raw_meta_key):
            raise RuntimeError("DeepZoom-from-raw requires raw_key and raw_meta_key")
        if raw_key and raw_meta_key:
            in_path, source_meta = _materialize_image_from_raw(raw_key, raw_meta_key, source_key)
            source_kind = "raw"
        else:
            in_path = "/tmp/source.tif"
            obj = s3.get_object(Bucket=BUCKET, Key=source_key)
            _read_body_to_path(obj["Body"], in_path)
        dl_ms = int((time.time() - t0) * 1000)

        report_status(job_id, task_id, "generating")

        # Run dz_export
        dz_base = "/tmp/dz/image"
        os.makedirs("/tmp/dz", exist_ok=True)
        t1 = time.time()
        result = subprocess.run(
            [DZ_EXPORT, in_path, dz_base],
            capture_output=True, text=True, timeout=600,
            env=imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"dz_export failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        gen_ms = int((time.time() - t1) * 1000)

        os.remove(in_path)

        report_status(job_id, task_id, "uploading")

        # Upload .dzi + tile files to S3
        s3_prefix = f"deepzoom/{job_id}/{export_id}"
        dzi_path = dz_base + ".dzi"
        tiles_dir = dz_base + "_files"
        uploaded = 0

        # Collect all files to upload
        upload_tasks = []
        if os.path.exists(dzi_path):
            upload_tasks.append((dzi_path, f"{s3_prefix}/image.dzi", "application/xml"))

        if os.path.isdir(tiles_dir):
            for root, dirs, files in os.walk(tiles_dir):
                for fname in files:
                    local = os.path.join(root, fname)
                    rel = os.path.relpath(local, os.path.dirname(tiles_dir))
                    s3_key = f"{s3_prefix}/{rel}"
                    ct = "image/png" if fname.endswith(".png") else "application/octet-stream"
                    upload_tasks.append((local, s3_key, ct))

        # Parallel upload (public access via bucket policy on deepzoom/ prefix)
        def upload_one(task):
            local_path, s3_key, content_type = task
            with open(local_path, "rb") as fh:
                s3.put_object(
                    Bucket=BUCKET, Key=s3_key, Body=fh.read(),
                    ContentType=content_type)
            return 1

        t2 = time.time()
        with ThreadPoolExecutor(max_workers=50) as pool:
            results = list(pool.map(upload_one, upload_tasks))
        uploaded = sum(results)
        upload_ms = int((time.time() - t2) * 1000)

        # Build manifest and viewer before writing anything
        region = os.environ.get("AWS_REGION", "us-east-1")
        dzi_url = f"https://{BUCKET}.s3.{region}.amazonaws.com/{s3_prefix}/image.dzi"
        share_url = f"https://{BUCKET}.s3.{region}.amazonaws.com/{s3_prefix}/viewer.html"
        created_at = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())

        # Upload viewer.html first — meta.json advertises share_url,
        # so the viewer must exist before meta is written.
        viewer_html = _render_viewer(job_id, export_id, created_at)
        s3.put_object(
            Bucket=BUCKET,
            Key=f"{s3_prefix}/viewer.html",
            Body=viewer_html,
            ContentType="text/html; charset=utf-8"
        )

        # Write meta.json (references share_url which now exists)
        manifest = {
            "job_id": job_id,
            "export_id": export_id,
            "created_at": created_at,
            "source_key": source_key,
            "source_kind": source_kind,
            "dzi_key": f"{s3_prefix}/image.dzi",
            "dzi_url": dzi_url,
            "share_url": share_url,
            "tile_prefix": f"{s3_prefix}/image_files",
            "width": meta["width"],
            "height": meta["height"],
            "tiles_uploaded": uploaded,
        }
        if source_kind == "raw":
            manifest["raw_key"] = source_meta.get("raw_key", raw_key)
            manifest["raw_meta_key"] = source_meta.get("raw_meta_key", raw_meta_key)
            manifest["palette"] = source_meta.get("palette", "")
        manifest_json = json.dumps(manifest)
        s3.put_object(
            Bucket=BUCKET,
            Key=f"{s3_prefix}/meta.json",
            Body=manifest_json,
            ContentType="application/json"
        )
        # Also write canonical pointer for fast Render-tab refresh
        s3.put_object(
            Bucket=BUCKET,
            Key=f"renders/{job_id}/deepzoom_latest.json",
            Body=manifest_json,
            ContentType="application/json"
        )

        # Cleanup /tmp
        import shutil
        shutil.rmtree("/tmp/dz", ignore_errors=True)
        for path in (
            "/tmp/source.tif",
            "/tmp/source.greyscale.raw",
            "/tmp/source.deepzoom.bins",
            "/tmp/source.deepzoom.rgb.raw",
            "/tmp/source.deepzoom.png",
        ):
            try:
                os.remove(path)
            except OSError:
                pass

        report_status(job_id, task_id, "done")
        return ok_response({
            "export_id": export_id,
            "dzi_url": dzi_url,
            "share_url": share_url,
            "tiles_uploaded": uploaded,
            "dl_ms": dl_ms,
            "gen_ms": gen_ms,
            "upload_ms": upload_ms,
            "source_kind": source_kind,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        import shutil
        shutil.rmtree("/tmp/dz", ignore_errors=True)
        for path in (
            "/tmp/source.tif",
            "/tmp/source.greyscale.raw",
            "/tmp/source.deepzoom.bins",
            "/tmp/source.deepzoom.rgb.raw",
            "/tmp/source.deepzoom.png",
        ):
            try:
                os.remove(path)
            except OSError:
                pass
        raise


def handler(event, context):
    params = parse_body(event)
    return handle_deepzoom_export_request(params, require_raw_sidecar=False, task_id="deepzoom_export")
