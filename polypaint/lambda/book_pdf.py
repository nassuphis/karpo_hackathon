"""Book Maker prepare/compose lambda (book-maker-design.md §5/§6).

One container-image function, two dispatch ops:
- prepare: freeze one entry — cache the ≤3600px JPEG asset + provenance
  snapshot under polypaint/books/{book_id}/assets/. Idempotent.
- compose: render book.tex/cover.tex from the book doc + snapshots, run
  lualatex, upload cover.pdf/content.pdf/source.zip + out/latest.json.
"""
import io
import json
import os
import shutil
import subprocess
import zipfile
from datetime import datetime, timezone

import boto3

import book_tex
from shared import BUCKET, parse_body, ok_response, report_status
from spread_pdf import PDF_IMAGE_MAX_PX, prepare_pdf_image

s3 = boto3.client("s3")

BOOKS_PREFIX = "polypaint/books/"
FONT_DIR = os.environ.get("BOOK_FONT_DIR", "/opt/book-fonts")
ASSET_MAX_PX = int(os.environ.get("BOOK_ASSET_MAX_PX", PDF_IMAGE_MAX_PX))


def _utc_now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status,
                  result_data={"phase": phase, "phase_label": phase_label, **extra})


def _asset_keys(book_id, entry_id):
    base = f"{BOOKS_PREFIX}{book_id}/assets/{entry_id}"
    return f"{base}.jpg", f"{base}.provenance.json"


def _key_exists(key):
    try:
        s3.head_object(Bucket=BUCKET, Key=key)
        return True
    except Exception:
        return False


def _summary_from_sources(calc, src_meta):
    """The condensed verso fields book_tex reads (design §6): built once at
    prepare time so the book compiles identically forever."""
    pipeline = calc.get("pipeline") if isinstance(calc.get("pipeline"), dict) else {}
    fn = str(pipeline.get("function") or calc.get("function") or "").strip()
    pt = str(pipeline.get("param_transforms_display") or pipeline.get("param_program_display") or "").strip()
    ct = str(pipeline.get("coeff_transforms_display") or pipeline.get("coeff_program_display") or "").strip()
    pipeline_line = f"[ {pt} ]   {fn}   [ {ct} ]" if fn else ""
    scale_parts = []
    degree = str(calc.get("degree") or "").strip()
    if degree:
        scale_parts.append(f"degree {degree}")
    n_value = str(calc.get("N") or calc.get("n1") or "").strip()
    if n_value:
        scale_parts.append(f"{n_value}x{n_value} grid")
    color_mode = str(src_meta.get("color_mode") or "").strip()
    palette = str(src_meta.get("palette") or "").strip()
    coloring = " ".join(p for p in (color_mode, palette) if p)
    solver = str(calc.get("solver") or "").strip()
    return {
        "function": fn,
        "pipeline": pipeline_line,
        "scale": ", ".join(scale_parts),
        "coloring": coloring,
        "solver": f"Solved by {solver}" if solver else "",
    }


def handle_prepare(params):
    job_id = params["job_id"]
    task_id = params["task_id"]
    book_id = params["book_id"]
    entry_id = params["entry_id"]
    source_job_id = params["source_job_id"]
    source_image_key = params["source_image_key"]
    asset_key, prov_key = _asset_keys(book_id, entry_id)
    progress = {"family": "book", "book_id": book_id, "entry_id": entry_id, "op": "prepare"}

    if _key_exists(asset_key) and _key_exists(prov_key):
        _phase(job_id, task_id, "done", "done", "Cached", **progress, cached=True)
        return ok_response({"book_id": book_id, "entry_id": entry_id, "cached": True})

    _phase(job_id, task_id, "started", "load_source", "Load source", **progress)
    source_ext = source_image_key.rsplit(".", 1)[-1].lower()
    source_local = f"/tmp/book_src_{entry_id}.{source_ext}"
    prepared_local = f"/tmp/book_asset_{entry_id}.jpg"
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=source_image_key)
        with open(source_local, "wb") as fh:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                fh.write(chunk)

        _phase(job_id, task_id, "processing", "prepare_image", "Prepare image", **progress)
        info = prepare_pdf_image(source_local, prepared_local,
                                 max_px=ASSET_MAX_PX, quality=92, image_format="jpeg")

        _phase(job_id, task_id, "processing", "snapshot", "Provenance snapshot", **progress)
        try:
            calc_obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{source_job_id}/calc.json")
            calc = json.loads(calc_obj["Body"].read())
        except Exception:
            calc = {}
        src_meta = {}
        try:
            head = s3.head_object(Bucket=BUCKET, Key=source_image_key)
            src_meta = dict(head.get("Metadata") or {})
        except Exception:
            pass
        snapshot = {
            "version": 1,
            "entry_id": entry_id,
            "source_job_id": source_job_id,
            "source_artifact_id": params.get("source_artifact_id", ""),
            "prepared": {k: info[k] for k in
                         ("source_width", "source_height", "prepared_width", "prepared_height")},
            "summary": _summary_from_sources(calc, src_meta),
            "created_at": _utc_now_iso(),
        }

        with open(prepared_local, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, asset_key,
                              ExtraArgs={"ContentType": "image/jpeg"})
        s3.put_object(Bucket=BUCKET, Key=prov_key,
                      Body=json.dumps(snapshot).encode("utf-8"),
                      ContentType="application/json")
        _phase(job_id, task_id, "done", "done", "Done", **progress,
               prepared_width=info["prepared_width"], prepared_height=info["prepared_height"])
        return ok_response({"book_id": book_id, "entry_id": entry_id, "cached": False})
    finally:
        for path in (source_local, prepared_local):
            try:
                os.remove(path)
            except OSError:
                pass


def _run_lualatex(build_dir, name):
    """Two passes, halt-on-error; surfaces the log tail on failure."""
    for _ in range(2):
        run = subprocess.run(
            ["lualatex", "-interaction=nonstopmode", "-halt-on-error", f"{name}.tex"],
            cwd=build_dir, capture_output=True, text=True, timeout=600)
        if run.returncode != 0:
            tail = ""
            log_path = os.path.join(build_dir, f"{name}.log")
            if os.path.exists(log_path):
                tail = open(log_path, errors="replace").read()[-2000:]
            raise RuntimeError(f"lualatex failed on {name}.tex: {tail or run.stderr[-500:]}")


def handle_compose(params, latex_runner=_run_lualatex):
    job_id = params["job_id"]
    task_id = params["task_id"]
    book_id = params["book_id"]
    compile_id = params["compile_id"]
    expected_saved_at = str(params.get("expected_saved_at") or "")
    progress = {"family": "book", "book_id": book_id, "compile_id": compile_id, "op": "compose"}
    out_prefix = f"{BOOKS_PREFIX}{book_id}/out/{compile_id}/"

    _phase(job_id, task_id, "started", "load_assets", "Load book + assets", **progress)
    obj = s3.get_object(Bucket=BUCKET, Key=f"{BOOKS_PREFIX}{book_id}.json")
    book = json.loads(obj["Body"].read())
    if expected_saved_at and str(book.get("saved_at") or "") != expected_saved_at:
        raise RuntimeError(
            f"book {book_id} was saved mid-compile (saved_at {book.get('saved_at')!r} != "
            f"{expected_saved_at!r}); recompile to pick up the new state")
    entries = list(book.get("entries") or [])
    if not entries:
        raise RuntimeError(f"book {book_id} has no entries")

    build_dir = f"/tmp/book_build_{compile_id}"
    shutil.rmtree(build_dir, ignore_errors=True)
    os.makedirs(os.path.join(build_dir, book_tex.ASSET_DIR))
    try:
        provenance = {}
        missing = []
        for entry in entries:
            entry_id = entry.get("entry_id") or ""
            asset_key, prov_key = _asset_keys(book_id, entry_id)
            try:
                s3.download_file(BUCKET, asset_key,
                                 os.path.join(build_dir, book_tex.ASSET_DIR, f"{entry_id}.jpg"))
                pobj = s3.get_object(Bucket=BUCKET, Key=prov_key)
                provenance[entry_id] = json.loads(pobj["Body"].read())
            except Exception:
                missing.append(entry_id)
        if missing:
            raise RuntimeError(f"book {book_id} is missing prepared assets for entries: "
                               f"{', '.join(missing[:10])} — run prepare first")
        if os.path.isdir(FONT_DIR):
            for fname in os.listdir(FONT_DIR):
                shutil.copy(os.path.join(FONT_DIR, fname), build_dir)

        _phase(job_id, task_id, "processing", "compose_tex", "Render tex", **progress)
        content_tex, content_pages = book_tex.render_content_tex(book, provenance)
        cover_rel = None
        cover_id = str(book.get("cover_entry_id") or "")
        if cover_id and cover_id in provenance:
            cover_rel = f"{book_tex.ASSET_DIR}/{cover_id}.jpg"
        cover_tex = book_tex.render_cover_tex(book, cover_rel)
        with open(os.path.join(build_dir, "book.tex"), "w") as fh:
            fh.write(content_tex)
        with open(os.path.join(build_dir, "cover.tex"), "w") as fh:
            fh.write(cover_tex)

        _phase(job_id, task_id, "processing", "latex_content", "LaTeX content", **progress)
        latex_runner(build_dir, "book")
        _phase(job_id, task_id, "processing", "latex_cover", "LaTeX cover", **progress)
        latex_runner(build_dir, "cover")

        _phase(job_id, task_id, "uploading", "upload", "Upload outputs", **progress)
        s3.upload_file(os.path.join(build_dir, "book.pdf"), BUCKET,
                       out_prefix + "content.pdf",
                       ExtraArgs={"ContentType": "application/pdf"})
        s3.upload_file(os.path.join(build_dir, "cover.pdf"), BUCKET,
                       out_prefix + "cover.pdf",
                       ExtraArgs={"ContentType": "application/pdf"})

        zip_buf = io.BytesIO()
        with zipfile.ZipFile(zip_buf, "w", zipfile.ZIP_DEFLATED) as zf:
            zf.writestr("book.tex", content_tex)
            zf.writestr("cover.tex", cover_tex)
            zf.writestr("README.txt",
                        "PolyPaint book source. Compile: lualatex book.tex (twice); "
                        "assets/ + fonts must sit beside the tex files.\n")
            for entry_id in provenance:
                zf.write(os.path.join(build_dir, book_tex.ASSET_DIR, f"{entry_id}.jpg"),
                         f"{book_tex.ASSET_DIR}/{entry_id}.jpg")
        zip_buf.seek(0)
        s3.upload_fileobj(zip_buf, BUCKET, out_prefix + "source.zip",
                          ExtraArgs={"ContentType": "application/zip"})

        latest = {
            "compile_id": compile_id,
            "cover_key": out_prefix + "cover.pdf",
            "content_key": out_prefix + "content.pdf",
            "source_key": out_prefix + "source.zip",
            "content_pages": content_pages,
            "spread_count": len(entries),
            "compiled_at": _utc_now_iso(),
        }
        s3.put_object(Bucket=BUCKET, Key=f"{BOOKS_PREFIX}{book_id}/out/latest.json",
                      Body=json.dumps(latest).encode("utf-8"),
                      ContentType="application/json")
        _phase(job_id, task_id, "done", "done", "Done", **progress,
               content_pages=content_pages, **{k: latest[k] for k in
                                               ("cover_key", "content_key", "source_key")})
        return ok_response({"book_id": book_id, **latest})
    finally:
        shutil.rmtree(build_dir, ignore_errors=True)


def handler(event, context):
    params = parse_body(event)
    op = str(params.get("op") or "").strip()
    job_id = params.get("job_id", "")
    task_id = params.get("task_id", "")
    try:
        if op == "prepare":
            return handle_prepare(params)
        if op == "compose":
            return handle_compose(params)
        raise RuntimeError(f"book_pdf unknown op {op!r}")
    except Exception as e:
        report_status(job_id, task_id, "error", str(e),
                      result_data={"family": "book", "op": op})
        raise
