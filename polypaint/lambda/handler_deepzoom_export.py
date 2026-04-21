"""
DeepZoom export Lambda — generates OpenSeadragon tile pyramid from an
existing rendered image.

Downloads the exact source image, runs dz_export (libvips dzsave), uploads
.dzi + tile files to S3 under deepzoom/{job_id}/{export_id}/.

Public access via bucket policy on deepzoom/ prefix (no per-object ACL).
"""
import json
import os
import re
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor

import boto3

from color_artifact_meta import load_color_artifact_head
from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
DZ_EXPORT = os.path.join(os.path.dirname(__file__), "dz_export")
VIEWER_TEMPLATE = os.path.join(os.path.dirname(__file__), "deepzoom_viewer_template.html")
_SOURCE_ARTIFACT_KEY_RE = re.compile(r"^renders/([^/]+)/(color|bilevel|coeffs|palettes)/([^/]+)/[^/]+$")
_SOURCE_FAMILY_DIR_MAP = {"color": "color", "bilevel": "bilevel", "coeffs": "coeffs", "palettes": "palette"}


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


def _read_json_key(key):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"].read()
    data = json.loads(body) if body else {}
    if not isinstance(data, dict):
        raise RuntimeError(f"Expected JSON object in {key}")
    return data


def _source_ref_from_key(source_key):
    match = _SOURCE_ARTIFACT_KEY_RE.match(str(source_key or "").strip())
    if not match:
        return {
            "job_id": "",
            "family_dir": "",
            "family": "",
            "artifact_id": "",
        }
    family_dir = match.group(2)
    return {
        "job_id": match.group(1),
        "family_dir": family_dir,
        "family": _SOURCE_FAMILY_DIR_MAP.get(family_dir, ""),
        "artifact_id": match.group(3),
    }


def _coerce_finite_float(value):
    if value in ("", None):
        return None
    try:
        num = float(value)
    except (TypeError, ValueError):
        return None
    if not (num == num and num not in (float("inf"), float("-inf"))):
        return None
    return num


def _load_direct_source_metadata(source_key):
    ref = _source_ref_from_key(source_key)
    meta = {}
    try:
        if ref["family"] == "color" and ref["job_id"] and ref["artifact_id"]:
            head = load_color_artifact_head(s3, BUCKET, ref["job_id"], ref["artifact_id"])
            meta = dict(head.get("metadata", {}) or {})
        elif ref["family"] == "palette" and ref["job_id"] and ref["artifact_id"]:
            meta = _read_json_key(f"renders/{ref['job_id']}/palettes/{ref['artifact_id']}/meta.json")
        elif ref["family"] in ("bilevel", "coeffs"):
            head = s3.head_object(Bucket=BUCKET, Key=source_key)
            meta = dict(head.get("Metadata", {}) or {})
        else:
            head = s3.head_object(Bucket=BUCKET, Key=source_key)
            meta = dict(head.get("Metadata", {}) or {})
    except Exception:
        meta = {}
    return ref, meta


def _manifest_source_fields(source_key):
    ref, meta = _load_direct_source_metadata(source_key)
    manifest = {
        "source_key": source_key,
        "source_artifact_id": str(meta.get("artifact_id") or ref["artifact_id"] or ""),
        "source_family": str(meta.get("family") or ref["family"] or ""),
        "source_rotation": _coerce_finite_float(meta.get("rotation")),
    }
    if manifest["source_rotation"] is None:
        manifest["source_rotation"] = 0.0
    viewport = {
        "viewport_min_re": _coerce_finite_float(meta.get("min_re")),
        "viewport_max_re": _coerce_finite_float(meta.get("max_re")),
        "viewport_min_im": _coerce_finite_float(meta.get("min_im")),
        "viewport_max_im": _coerce_finite_float(meta.get("max_im")),
    }
    if all(value is not None for value in viewport.values()):
        manifest.update(viewport)
    return manifest


def handle_deepzoom_export_request(params, *, require_raw_sidecar=False, task_id="deepzoom_export"):
    job_id = params["job_id"]
    source_key = str(params.get("source_key") or "").strip()
    raw_key = str(params.get("raw_key") or "").strip()
    raw_meta_key = str(params.get("raw_meta_key") or "").strip()
    export_id = params.get("export_id", f"dz_{int(time.time())}")
    source_path = ""

    try:
        report_status(job_id, task_id, "started")

        t0 = time.time()
        if require_raw_sidecar and not (raw_key and raw_meta_key):
            raise RuntimeError("DeepZoom-from-raw requires raw_key and raw_meta_key")
        if not source_key:
            raise RuntimeError("DeepZoom requires source_key")
        source_kind = "image"
        suffix = os.path.splitext(source_key)[1] or ".img"
        source_path = f"/tmp/deepzoom_source{suffix}"
        obj = s3.get_object(Bucket=BUCKET, Key=source_key)
        _read_body_to_path(obj["Body"], source_path)
        dl_ms = int((time.time() - t0) * 1000)

        report_status(job_id, task_id, "generating")

        # Run dz_export
        dz_base = "/tmp/dz/image"
        os.makedirs("/tmp/dz", exist_ok=True)
        t1 = time.time()
        result = subprocess.run(
            [DZ_EXPORT, source_path, dz_base],
            capture_output=True, text=True, timeout=600,
            env=imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"dz_export failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        gen_ms = int((time.time() - t1) * 1000)

        os.remove(source_path)
        source_path = ""

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
        source_manifest = _manifest_source_fields(source_key)

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
            "source_kind": source_kind,
            "dzi_key": f"{s3_prefix}/image.dzi",
            "dzi_url": dzi_url,
            "share_url": share_url,
            "tile_prefix": f"{s3_prefix}/image_files",
            "width": meta["width"],
            "height": meta["height"],
            "tiles_uploaded": uploaded,
        }
        manifest.update(source_manifest)
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
        if source_path:
            try:
                os.remove(source_path)
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
        if source_path:
            try:
                os.remove(source_path)
            except OSError:
                pass
        raise


def handler(event, context):
    params = parse_body(event)
    return handle_deepzoom_export_request(params, require_raw_sidecar=False, task_id="deepzoom_export")
