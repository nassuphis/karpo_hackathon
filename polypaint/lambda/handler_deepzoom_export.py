"""
DeepZoom export Lambda — generates OpenSeadragon tile pyramid from an
existing rendered image.

Downloads the exact source image, runs dz_export (libvips dzsave), uploads
.dzi + tile files to S3 under deepzoom/{job_id}/{export_id}/.

Public access via bucket policy on deepzoom/ prefix (no per-object ACL).
"""
import json
import os
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor

import boto3
from botocore.exceptions import ClientError

from color_artifact_meta import load_color_artifact_head
from shared import (BUCKET, CACHE_IMMUTABLE, parse_body, ok_response, report_status,
                    imgpipe_env, assert_render_source, assert_safe_id, parse_render_key,
                    is_missing_s3_error)

s3 = boto3.client("s3")
DZ_EXPORT = os.path.join(os.path.dirname(__file__), "dz_export")
VIEWER_TEMPLATE = os.path.join(os.path.dirname(__file__), "deepzoom_viewer_template.html")
# dir token (from the structured render-key parser) -> semantic source family.
_SOURCE_FAMILY_DIR_MAP = {"color": "color", "bilevel": "bilevel", "coeffs": "coeffs", "palettes": "palette"}


def _render_viewer(job_id, export_id, created_at):
    """Render standalone viewer HTML from template. Every substitution is
    HTML-escaped (defense in depth on top of the id validation): this HTML is
    published to the public bucket, so an unescaped id would be stored XSS
    (CR28 F9)."""
    import html
    with open(VIEWER_TEMPLATE) as f:
        tmpl = f.read()
    return tmpl.replace("{job_id}", html.escape(str(job_id))) \
               .replace("{export_id}", html.escape(str(export_id))) \
               .replace("{created_at}", html.escape(str(created_at)))


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
    # One structured render-key parser for the whole codebase (code-review-28
    # F12): exact renders/<job>/<family>/<artifact>/<leaf> components, never a
    # substring guess.
    parsed = parse_render_key(str(source_key or "").strip())
    if parsed["variant"] != "canonical":
        return {"job_id": "", "family_dir": "", "family": "", "artifact_id": ""}
    family_dir = parsed["family"]
    return {
        "job_id": parsed["job"],
        "family_dir": family_dir,
        "family": _SOURCE_FAMILY_DIR_MAP.get(family_dir, ""),
        "artifact_id": parsed["artifact_id"],
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


def _export_prefix_exists(job_id, export_id):
    """Create-only export identity (code-review-29 F1): a NEW export must never
    land on an existing prefix — a reused or same-second-defaulted export_id
    would silently mix two exports' objects under one immutable prefix."""
    try:
        s3.head_object(Bucket=BUCKET, Key=f"deepzoom/{job_id}/{export_id}/meta.json")
        return True
    except Exception as exc:
        if is_missing_s3_error(exc):
            return False
        raise


def handle_deepzoom_export_request(params, *, require_raw_sidecar=False, task_id="deepzoom_export"):
    # job_id + export_id become an S3 prefix AND public viewer HTML — validate
    # to a safe charset before any use (CR28 F9): no slashes / HTML / control
    # chars. This blocks both stored-HTML injection and malformed prefixes.
    job_id = assert_safe_id(params["job_id"], "job_id")
    export_id = assert_safe_id(params.get("export_id", f"dz_{int(time.time())}"), "export_id")
    source_key = str(params.get("source_key") or "").strip()
    raw_key = str(params.get("raw_key") or "").strip()
    raw_meta_key = str(params.get("raw_meta_key") or "").strip()
    source_path = ""

    try:
        report_status(job_id, task_id, "started")

        t0 = time.time()
        if require_raw_sidecar and not (raw_key and raw_meta_key):
            raise RuntimeError("DeepZoom-from-raw requires raw_key and raw_meta_key")
        if not source_key:
            raise RuntimeError("DeepZoom requires source_key")
        # code-review-27 F5: pin the source (and raw sidecars) to this job
        # before the download, so the export can't pull another job's bytes
        assert_render_source(source_key, job_id, None, "source_key")
        # Tie the raw sidecars to the SAME artifact as the source, not merely
        # the job (code-review-28 F12). In the from-raw flow the client sends
        # the artifact's own raw_key/raw_meta_key, which live beside its image
        # under renders/<job>/<family>/<artifact>/. Fall back to a job-scope pin
        # only for a legacy/non-canonical source key with no artifact segment.
        _src = parse_render_key(source_key)
        if _src["variant"] == "canonical":
            artifact_prefix = f"renders/{_src['job']}/{_src['family']}/{_src['artifact_id']}/"
            for label, k in (("raw_key", raw_key), ("raw_meta_key", raw_meta_key)):
                if k and not k.startswith(artifact_prefix):
                    raise ValueError(
                        f"{label} {k!r} is not under the source artifact prefix "
                        f"{artifact_prefix} (artifact mismatch)")
        else:
            for label, k in (("raw_key", raw_key), ("raw_meta_key", raw_meta_key)):
                if k and not k.startswith(f"renders/{job_id}/"):
                    raise ValueError(f"{label} {k!r} is not under renders/{job_id}/ (job mismatch)")
        if _export_prefix_exists(job_id, export_id):
            raise RuntimeError(
                f"export prefix deepzoom/{job_id}/{export_id}/ already exists — "
                "each export needs a fresh export_id")
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
        width = int(meta["width"])
        height = int(meta["height"])
        if width != height:
            raise RuntimeError(f"DeepZoom export requires square source image, got {width}x{height}")
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
                    ContentType=content_type,
                    CacheControl=CACHE_IMMUTABLE)
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
            ContentType="text/html; charset=utf-8",
            CacheControl=CACHE_IMMUTABLE,
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
            "pix": width,
            "width": width,
            "height": height,
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


def handle_sheet_deepzoom(params, task_id="sheet_deepzoom"):
    """DeepZoom export for a poly-sheet mosaic. The source key is
    constructed SERVER-SIDE from the validated sheet_id — the caller
    never supplies a key, so the renders/ source pinning does not
    apply. Non-square sources are allowed (sheet mosaics are
    cols x rows grids; OpenSeadragon handles any aspect)."""
    sheet_id = assert_safe_id(params.get("sheet_id"), "sheet_id")
    job_id = assert_safe_id(str(params.get("job_id") or sheet_id), "job_id")
    export_id = assert_safe_id(
        params.get("export_id", f"dz_{int(time.time())}"), "export_id")
    # round-3 finding 2: follow the publication pointer to the current
    # generation's immutable PNG (fall back to the legacy fixed key)
    # round-5 finding 6: fall back to the legacy fixed key ONLY on a
    # confirmed-missing run.json — a transient S3 error must not silently
    # point the export at a key that may not exist.
    source_key = f"sheets/{sheet_id}/sheet.png"
    try:
        run = json.loads(s3.get_object(
            Bucket=BUCKET, Key=f"sheets/{sheet_id}/run.json")["Body"].read())
        if isinstance(run, dict) and run.get("published_png_key"):
            source_key = run["published_png_key"]
    except ClientError as exc:
        if not is_missing_s3_error(exc):
            raise
    except (ValueError, TypeError):
        pass
    source_path = ""
    try:
        report_status(job_id, task_id, "started")
        t0 = time.time()
        if _export_prefix_exists(job_id, export_id):
            raise RuntimeError(
                f"export prefix deepzoom/{job_id}/{export_id}/ already exists — "
                "each export needs a fresh export_id")
        source_path = "/tmp/deepzoom_source.png"
        obj = s3.get_object(Bucket=BUCKET, Key=source_key)
        _read_body_to_path(obj["Body"], source_path)
        dl_ms = int((time.time() - t0) * 1000)

        report_status(job_id, task_id, "generating")
        dz_base = "/tmp/dz/image"
        os.makedirs("/tmp/dz", exist_ok=True)
        t1 = time.time()
        result = subprocess.run(
            [DZ_EXPORT, source_path, dz_base, "--bilevel"],
            capture_output=True, text=True, timeout=600,
            env=imgpipe_env())
        if result.returncode != 0:
            raise RuntimeError(f"dz_export failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        width = int(meta["width"])
        height = int(meta["height"])
        if int(meta.get("bitdepth") or 0) != 1:
            raise RuntimeError("sheet dz_export did not produce bilevel tiles")
        gen_ms = int((time.time() - t1) * 1000)
        os.remove(source_path)
        source_path = ""

        report_status(job_id, task_id, "uploading")
        s3_prefix = f"deepzoom/{job_id}/{export_id}"
        dzi_path = dz_base + ".dzi"
        tiles_dir = dz_base + "_files"
        upload_tasks = []
        if os.path.exists(dzi_path):
            upload_tasks.append((dzi_path, f"{s3_prefix}/image.dzi", "application/xml"))
        if os.path.isdir(tiles_dir):
            for root, dirs, files in os.walk(tiles_dir):
                for fname in files:
                    local = os.path.join(root, fname)
                    rel = os.path.relpath(local, os.path.dirname(tiles_dir))
                    ct = "image/png" if fname.endswith(".png") else "application/octet-stream"
                    upload_tasks.append((local, f"{s3_prefix}/{rel}", ct))

        def upload_one(task):
            local_path, s3_key, content_type = task
            with open(local_path, "rb") as fh:
                s3.put_object(Bucket=BUCKET, Key=s3_key, Body=fh.read(),
                              ContentType=content_type,
                              CacheControl=CACHE_IMMUTABLE)
            return 1

        t2 = time.time()
        with ThreadPoolExecutor(max_workers=50) as pool:
            uploaded = sum(pool.map(upload_one, upload_tasks))
        upload_ms = int((time.time() - t2) * 1000)

        region = os.environ.get("AWS_REGION", "us-east-1")
        dzi_url = f"https://{BUCKET}.s3.{region}.amazonaws.com/{s3_prefix}/image.dzi"
        share_url = f"https://{BUCKET}.s3.{region}.amazonaws.com/{s3_prefix}/viewer.html"
        created_at = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
        viewer_html = _render_viewer(job_id, export_id, created_at)
        s3.put_object(Bucket=BUCKET, Key=f"{s3_prefix}/viewer.html",
                      Body=viewer_html, ContentType="text/html; charset=utf-8",
                      CacheControl=CACHE_IMMUTABLE)
        manifest = {
            "job_id": job_id,
            "export_id": export_id,
            "created_at": created_at,
            "source_kind": "sheet",
            "sheet_id": sheet_id,
            "source_key": source_key,
            "dzi_key": f"{s3_prefix}/image.dzi",
            "dzi_url": dzi_url,
            "share_url": share_url,
            "tile_prefix": f"{s3_prefix}/image_files",
            "pix": max(width, height),
            "width": width,
            "height": height,
            "tile_bitdepth": 1,
            "tiles_uploaded": uploaded,
        }
        s3.put_object(Bucket=BUCKET, Key=f"{s3_prefix}/meta.json",
                      Body=json.dumps(manifest),
                      ContentType="application/json")
        import shutil
        shutil.rmtree("/tmp/dz", ignore_errors=True)
        report_status(job_id, task_id, "done", result_data={
            "phase": "done", "phase_label": "DeepZoom ready",
            "sheet_id": sheet_id, "export_id": export_id,
            "share_url": share_url, "dzi_url": dzi_url,
        })
        return ok_response({
            "export_id": export_id,
            "dzi_url": dzi_url,
            "share_url": share_url,
            "tiles_uploaded": uploaded,
            "dl_ms": dl_ms,
            "gen_ms": gen_ms,
            "upload_ms": upload_ms,
            "source_kind": "sheet",
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


def _looks_like_api_gateway(event):
    """True when the event arrived over the public HTTP API rather than a
    direct Lambda-to-Lambda invoke. The wall-pyramid build is storage's
    internal async fan-out only; it must never be reachable from a request."""
    return isinstance(event, dict) and any(
        k in event for k in ("requestContext", "rawPath", "httpMethod",
                             "routeKey", "headers", "queryStringParameters"))


def handler(event, context):
    params = parse_body(event)
    wants_wall = (params.get("internal_action") == "build_wall_pyramid"
                  or event.get("internal_action") == "build_wall_pyramid")
    if wants_wall:
        # Reject the internal action when the event carries API Gateway
        # fields: a public caller must not trigger the expensive libvips
        # wall build with a chosen manifest (code-review-25 F4).
        if _looks_like_api_gateway(event):
            return {
                "statusCode": 403,
                "headers": {"Content-Type": "application/json",
                            "Access-Control-Allow-Origin": "*"},
                "body": json.dumps({"error": "internal_action is not exposed over the API"}),
            }
        from handler_wall_pyramid import handle_build_wall_pyramid
        return handle_build_wall_pyramid(params if params.get("internal_action") else event)
    # The caller may thread its own operation-unique task_id (code-review-29 F1)
    # so overlapping exports for one job never share a status row.
    if str(params.get("action") or "").strip().lower() == "sheet":
        task_id = assert_safe_id(str(params.get("task_id") or "sheet_deepzoom"), "task_id")
        return handle_sheet_deepzoom(params, task_id=task_id)
    task_id = assert_safe_id(str(params.get("task_id") or "deepzoom_export"), "task_id")
    return handle_deepzoom_export_request(params, require_raw_sidecar=False, task_id=task_id)
