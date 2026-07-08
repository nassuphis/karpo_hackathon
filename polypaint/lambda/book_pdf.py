"""Book Maker prepare/compose lambda (book-maker-design.md §5/§6).

One container-image function, two dispatch ops:
- prepare: freeze one entry — cache the ≤3600px JPEG asset + provenance
  snapshot under polypaint/books/{book_id}/assets/. Idempotent.
- compose: render book.tex/cover.tex from the book doc + snapshots, run
  lualatex, upload cover.pdf/content.pdf/source.zip + out/latest.json.
"""
import concurrent.futures
import io
import json
import math
import os
import re
import shutil
import struct
import subprocess
import zipfile
from datetime import datetime, timezone

import boto3
from botocore.config import Config as BotoConfig

import book_tex
from shared import BUCKET, CACHE_IMMUTABLE, parse_body, ok_response, report_status
from spread_pdf import PDF_IMAGE_MAX_PX, PDF_PALETTE_MAX_PX, prepare_pdf_image

# pool sized to the flip-page upload threads (default 10 floods logs with
# discarded-connection warnings under the 16-worker upload)
s3 = boto3.client("s3", config=BotoConfig(max_pool_connections=32))

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


# --- report model: the KV rows the ColorSpread PDF button renders, ported
# here so the book verso matches it (handler_pdf_artifact.build_pdf_report_model
# / spread_pdf._draw_report_summary). Kept dependency-free (no program-source
# modules) since the book verso shows the technical KV grid, not program text.

def _first_text(*values):
    for value in values:
        text = str(value or "").strip()
        if text:
            return text
    return ""


def _solver_label(raw):
    solver = str(raw or "").strip()
    return {"aberth": "AE", "companion_matrix": "CM", "aberth_mt": "AE-MT"}.get(solver, solver)


def _fmt_pct(value):
    if value in ("", None):
        return ""
    try:
        return f"{float(value) * 100:.1f}%"
    except Exception:
        return str(value)


def _color_summary(src_meta):
    color_mode = str(src_meta.get("color_mode") or "").strip()
    palette = str(src_meta.get("palette") or "").strip()
    if color_mode == "solve_score":
        metric = _first_text(src_meta.get("solve_metric"))
        return metric or "score"
    if color_mode == "saved_palette":
        metric = _first_text(src_meta.get("solve_metric"))
        return f"saved palette: {metric}" if metric else "saved palette"
    if color_mode == "proximity":
        return "root proximity"
    if color_mode == "constant":
        color = str(src_meta.get("constant_color") or "").strip()
        return f"constant #{color}" if color else "constant"
    return color_mode or (f"palette {palette}" if palette else "rainbow")


def _fmt_sig2(value):
    """2 significant digits for coordinates (-0.5905770748 -> -0.59)."""
    try:
        return f"{float(value):.2g}"
    except Exception:
        return str(value)


def _viewport_summary(src_meta):
    view_mode = str(src_meta.get("view_mode") or "auto")
    if view_mode == "square":
        extent = str(src_meta.get("square_extent") or "").strip()
        return f"square, extent={_fmt_sig2(extent)}" if extent else "square"
    bounds = {k: src_meta.get(k) for k in ("min_re", "max_re", "min_im", "max_im")}
    if view_mode == "explicit" or all(v not in ("", None) for v in bounds.values()):
        # explicit coordinates: q/shim are irrelevant; 2 significant digits
        return (f"explicit, re [{_fmt_sig2(bounds['min_re'])}, {_fmt_sig2(bounds['max_re'])}], "
                f"im [{_fmt_sig2(bounds['min_im'])}, {_fmt_sig2(bounds['max_im'])}]")
    parts = [view_mode]
    q = _fmt_pct(src_meta.get("quantile"))
    shim = _fmt_pct(src_meta.get("shim"))
    if q:
        parts.append(f"q={q}")
    if shim:
        parts.append(f"shim={shim}")
    return ", ".join(p for p in parts if p)


def _build_report(calc, src_meta, source_job_id, source_artifact_id):
    """Mirror build_pdf_report_model's summary rows (empty values dropped for
    a clean book page)."""
    pipeline = calc.get("pipeline") if isinstance(calc.get("pipeline"), dict) else {}
    fn = _first_text(pipeline.get("function"), calc.get("function"), "?")
    cfpv = _first_text(pipeline.get("cfpv_display"))
    function_display = f"{fn}({cfpv})" if cfpv else fn
    color_summary = _color_summary(src_meta)
    color_mode = _first_text(src_meta.get("color_mode"), color_summary)
    rows = [
        ("Function", function_display),
        ("Degree", _first_text(calc.get("degree"), calc.get("probe_degree"))),
        ("N", _first_text(calc.get("N"), calc.get("n1"))),
        ("Times", _first_text(calc.get("times"))),
        ("Solver", _solver_label(calc.get("solver"))),
        ("Interpretation", _first_text(src_meta.get("color_interpretation"),
                                       src_meta.get("score_output_interpretation"))),
        ("Palette", _first_text(src_meta.get("palette"))),
        ("Output channels", _first_text(src_meta.get("score_output_channel_count"),
                                        src_meta.get("raw_channels"))),
        ("Viewport", _viewport_summary(src_meta)),
    ]
    if color_mode != "solve_score":
        rows.insert(5, ("Color mode", color_summary or color_mode))
    palette_label = _first_text(
        src_meta.get("associated_palette_id"), src_meta.get("palette_artifact_id"),
        src_meta.get("palette_id"), src_meta.get("palette"))
    return {
        "compute_id": source_job_id,
        "artifact_id": source_artifact_id,
        "summary_rows": [[label, value] for label, value in rows if str(value).strip()],
        "palette_label": palette_label,
    }


# Flipbook page rasterization (flipbook.md §2-3): pdftoppm renders PNG
# intermediates (its jpeg encoder is locked to 4:2:0 chroma subsampling,
# which washes out saturated art) and Pillow re-encodes at q88 with 4:4:4.
# 200 dpi -> ~2307x2331px pages: true 1:1 pixels on 2x retina desktops
# (150 dpi was soft there and marginal on 3x phones). Gate-measured
# fractions of a second per page leave the 900s budget untouched.
FLIP_DPI = 200
FLIP_QUALITY = 88
FLIP_WORKERS = 4


def _jpeg_dimensions(path):
    """Width/height from JPEG SOF markers (no imaging deps in this image
    beyond vips, which can't be reached from Python here)."""
    with open(path, "rb") as fh:
        data = fh.read(65536)
    if data[:2] != b"\xff\xd8":
        raise RuntimeError(f"not a JPEG: {path}")
    i = 2
    while i + 9 < len(data):
        if data[i] != 0xFF:
            i += 1
            continue
        marker = data[i + 1]
        if 0xC0 <= marker <= 0xCF and marker not in (0xC4, 0xC8, 0xCC):
            height, width = struct.unpack(">HH", data[i + 5:i + 9])
            return width, height
        length = struct.unpack(">H", data[i + 2:i + 4])[0]
        i += 2 + length
    raise RuntimeError(f"no SOF marker found: {path}")


def _render_flipbook_pages(build_dir, out_prefix, book, content_pages, progress_cb=None):
    """Rasterize book.pdf into flip/p%04d.jpg + flip.json under out_prefix.
    Returns the additive latest.json fields. Raises on failure — the caller
    isolates errors so the compile still succeeds without a flipbook."""
    flip_dir = os.path.join(build_dir, "flip")
    os.makedirs(flip_dir, exist_ok=True)
    pdf_path = os.path.join(build_dir, "book.pdf")

    per = max(1, math.ceil(content_pages / FLIP_WORKERS))
    ranges = [(first, min(first + per - 1, content_pages))
              for first in range(1, content_pages + 1, per)]

    from PIL import Image, ImageFile
    import threading
    tolerant_lock = threading.Lock()
    progress_lock = threading.Lock()
    progress_done = [0]

    def _tick_progress():
        if not progress_cb:
            return
        with progress_lock:
            progress_done[0] += 1
            done = progress_done[0]
        # every page on small books, ~20 updates on large ones
        if done == content_pages or done % max(1, content_pages // 20) == 0:
            progress_cb(done, content_pages)

    def convert_png(src, dest, tolerant=False):
        # tolerant decode is a PIL GLOBAL: serialize those attempts
        if tolerant:
            with tolerant_lock:
                ImageFile.LOAD_TRUNCATED_IMAGES = True
                try:
                    return convert_png(src, dest)
                finally:
                    ImageFile.LOAD_TRUNCATED_IMAGES = False
        img = Image.open(src)
        img.load()
        if img.mode != "RGB":
            img = img.convert("RGB")
        img.save(dest, format="JPEG",
                 quality=FLIP_QUALITY, subsampling=0, optimize=True)

    def render_and_convert(rng):
        """Render->convert->delete one page at a time: at 200dpi a noisy-art
        PNG intermediate runs ~10MB, and letting all of them coexist (as a
        whole-range render would) can blow the 2048MB /tmp on a large book.
        Peak residency this way = FLIP_WORKERS pages. Output names pad the
        page number to the digits of the DOCUMENT's page count (poppler
        uses numberOfDigits(getNumPages()) regardless of -f/-l — learned in
        prod when a 16-page book wrote page-01.png, not page-1.png)."""
        first, last = rng
        done = []

        def run_page(number):
            run = subprocess.run(
                ["pdftoppm", "-png", "-r", str(FLIP_DPI),
                 "-f", str(number), "-l", str(number),
                 pdf_path, os.path.join(flip_dir, "page")],
                capture_output=True, text=True, timeout=120)
            if run.returncode != 0:
                raise RuntimeError(
                    f"pdftoppm page {number} failed: {run.stderr.strip()[:300]}")

        for number in range(first, last + 1):
            run_page(number)
            _tick_progress()
            pad = len(str(content_pages))
            src = os.path.join(flip_dir, "page-%0*d.png" % (pad, number))
            if not os.path.exists(src):
                # poppler-version belt and braces: scan for any padding
                matches = [f for f in os.listdir(flip_dir)
                           if re.fullmatch(r"page-0*%d\.png" % number, f)]
                if not matches:
                    raise RuntimeError(f"pdftoppm page {number}: output missing")
                src = os.path.join(flip_dir, matches[0])
            canonical = "p%04d.jpg" % number
            try:
                convert_png(src, os.path.join(flip_dir, canonical))
            except Exception as exc:  # noqa: BLE001
                # seen live once ("broken data stream when writing image
                # file") with no page context and no reproduction in-container
                # (5/5 clean): retry with a fresh render + tolerant decode,
                # and if it still fails, say exactly what/where/how big.
                png_size = os.path.getsize(src) if os.path.exists(src) else -1
                print(f"flipbook: page {number} convert failed ({exc}); "
                      f"png={png_size}B — re-rendering")
                try:
                    os.remove(src)
                except OSError:
                    pass
                run_page(number)
                if not os.path.exists(src):
                    raise RuntimeError(f"pdftoppm page {number}: output missing on retry")
                try:
                    convert_png(src, os.path.join(flip_dir, canonical), tolerant=True)
                except Exception as exc2:  # noqa: BLE001
                    free_mb = shutil.disk_usage("/tmp").free // 1048576
                    raise RuntimeError(
                        f"page {number} convert failed after re-render: {exc2} "
                        f"(png={os.path.getsize(src) if os.path.exists(src) else -1}B, "
                        f"first: {exc}, /tmp free={free_mb}MB)") from exc2
                print(f"flipbook: page {number} recovered on retry")
            os.remove(src)
            done.append(canonical)
        return done

    with concurrent.futures.ThreadPoolExecutor(max_workers=FLIP_WORKERS) as pool:
        pages = [name for chunk in pool.map(render_and_convert, ranges) for name in chunk]
    if len(pages) != content_pages:
        raise RuntimeError(f"rendered {len(pages)} pages, expected {content_pages}")
    width_px, height_px = _jpeg_dimensions(os.path.join(flip_dir, pages[0]))

    def upload_page(name):
        with open(os.path.join(flip_dir, name), "rb") as fh:
            s3.put_object(Bucket=BUCKET, Key=f"{out_prefix}flip/{name}",
                          Body=fh.read(), ContentType="image/jpeg",
                          CacheControl=CACHE_IMMUTABLE)

    with concurrent.futures.ThreadPoolExecutor(max_workers=16) as pool:
        list(pool.map(upload_page, pages))

    flip_manifest = {
        "book_id": book.get("id") or book.get("name") or "",
        "compile_id": out_prefix.rstrip("/").split("/")[-1],
        "title": book.get("title") or book.get("name") or "PolyPaint",
        "page_count": content_pages,
        "width_px": width_px,
        "height_px": height_px,
        "pages": pages,
    }
    flip_key = f"{out_prefix}flip/flip.json"
    s3.put_object(Bucket=BUCKET, Key=flip_key,
                  Body=json.dumps(flip_manifest, separators=(",", ":")).encode("utf-8"),
                  ContentType="application/json",
                  CacheControl=CACHE_IMMUTABLE)
    shutil.rmtree(flip_dir, ignore_errors=True)
    return {"flip_key": flip_key, "flip_page_count": content_pages}


_ID_RE = None


def _safe_id(value, label):
    import re
    if not re.fullmatch(r"[A-Za-z0-9._#-]{1,80}", str(value or "")):
        raise ValueError(f"book_pdf {label} has an unsafe value: {value!r}")
    return str(value)


def handle_prepare(params):
    job_id = params["job_id"]
    task_id = params["task_id"]
    book_id = _safe_id(params["book_id"], "book_id")
    entry_id = _safe_id(params["entry_id"], "entry_id")
    source_job_id = params["source_job_id"]
    source_image_key = params["source_image_key"]
    asset_key, prov_key = _asset_keys(book_id, entry_id)
    palette_key = f"{BOOKS_PREFIX}{book_id}/assets/{entry_id}.palette.jpg"
    progress = {"family": "book", "book_id": book_id, "entry_id": entry_id, "op": "prepare"}

    # Compile always re-prepares (force=true) — hitting Compile should produce
    # a fresh book, not silently reuse stale assets. The cache short-circuit
    # only applies to non-forced calls (e.g. a future "pinned" existence probe).
    if not params.get("force") and _key_exists(asset_key) and _key_exists(prov_key):
        _phase(job_id, task_id, "done", "done", "Cached", **progress, cached=True)
        return ok_response({"book_id": book_id, "entry_id": entry_id, "cached": True})

    _phase(job_id, task_id, "started", "load_source", "Load source", **progress)
    source_ext = source_image_key.rsplit(".", 1)[-1].lower()
    source_local = f"/tmp/book_src_{entry_id}.{source_ext}"
    prepared_local = f"/tmp/book_asset_{entry_id}.jpg"
    palette_local = None
    prepared_palette_local = None
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
        # The S3 object Metadata is a limited subset — associated_palette_* (and
        # the full color meta) live in the color-artifact overlay meta.json, the
        # SAME source the ColorSpread PDF button reads via load_color_artifact_head.
        # Missing this is why the palette never appeared.
        source_artifact_id = params.get("source_artifact_id", "")
        overlay_key = (f"renders/{source_job_id}/meta.json" if source_artifact_id == "legacy_color"
                       else f"renders/{source_job_id}/color/{source_artifact_id}/meta.json")
        try:
            ov = s3.get_object(Bucket=BUCKET, Key=overlay_key)
            overlay = json.loads(ov["Body"].read())
            if isinstance(overlay, dict):
                src_meta.update(overlay)   # overlay wins, matching load_color_artifact_head
        except Exception:
            pass

        # associated palette swatch (same source the ColorSpread PDF uses)
        has_palette = False
        palette_image_key = str(src_meta.get("associated_palette_image_key") or "").strip()
        if palette_image_key:
            try:
                palette_ext = palette_image_key.rsplit(".", 1)[-1].lower()
                palette_local = f"/tmp/book_pal_{entry_id}.{palette_ext}"
                pal_obj = s3.get_object(Bucket=BUCKET, Key=palette_image_key)
                with open(palette_local, "wb") as pf:
                    for chunk in pal_obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                        pf.write(chunk)
                prepared_palette_local = f"/tmp/book_pal_prep_{entry_id}.jpg"
                # print target: the verso palette square is ~110mm; 1600px
                # ~= 370 DPI there (sources are 4000px, so this is cheap)
                prepare_pdf_image(palette_local, prepared_palette_local,
                                  max_px=1600, quality=92, image_format="jpeg")
                has_palette = True
            except Exception:
                has_palette = False

        report = _build_report(calc, src_meta, source_job_id,
                               params.get("source_artifact_id", ""))
        report["has_palette"] = has_palette
        snapshot = {
            "version": 2,
            "entry_id": entry_id,
            "source_job_id": source_job_id,
            "source_artifact_id": params.get("source_artifact_id", ""),
            "prepared": {k: info[k] for k in
                         ("source_width", "source_height", "prepared_width", "prepared_height")},
            "report": report,
            "created_at": _utc_now_iso(),
        }

        with open(prepared_local, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, asset_key,
                              ExtraArgs={"ContentType": "image/jpeg"})
        if has_palette and prepared_palette_local:
            with open(prepared_palette_local, "rb") as pf:
                s3.upload_fileobj(pf, BUCKET, palette_key,
                                  ExtraArgs={"ContentType": "image/jpeg"})
        s3.put_object(Bucket=BUCKET, Key=prov_key,
                      Body=json.dumps(snapshot).encode("utf-8"),
                      ContentType="application/json")
        _phase(job_id, task_id, "done", "done", "Done", **progress,
               prepared_width=info["prepared_width"], prepared_height=info["prepared_height"])
        return ok_response({"book_id": book_id, "entry_id": entry_id, "cached": False})
    finally:
        for path in (source_local, prepared_local, palette_local, prepared_palette_local):
            try:
                if path:
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
    book_id = _safe_id(params["book_id"], "book_id")
    compile_id = _safe_id(params["compile_id"], "compile_id")
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
        for asset_idx, entry in enumerate(entries, start=1):
            _phase(job_id, task_id, "processing", "load_assets",
                   f"Load assets {asset_idx}/{len(entries)}", **progress)
            entry_id = entry.get("entry_id") or ""
            asset_key, prov_key = _asset_keys(book_id, entry_id)
            try:
                s3.download_file(BUCKET, asset_key,
                                 os.path.join(build_dir, book_tex.ASSET_DIR, f"{entry_id}.jpg"))
                pobj = s3.get_object(Bucket=BUCKET, Key=prov_key)
                provenance[entry_id] = json.loads(pobj["Body"].read())
            except Exception:
                missing.append(entry_id)
                continue
            # optional palette swatch (best-effort; absence just omits it)
            if ((provenance[entry_id].get("report") or {}).get("has_palette")):
                try:
                    s3.download_file(
                        BUCKET, f"{BOOKS_PREFIX}{book_id}/assets/{entry_id}.palette.jpg",
                        os.path.join(build_dir, book_tex.ASSET_DIR, f"{entry_id}.palette.jpg"))
                except Exception:
                    provenance[entry_id]["report"]["has_palette"] = False
        if missing:
            raise RuntimeError(f"book {book_id} is missing prepared assets for entries: "
                               f"{', '.join(missing[:10])} — run prepare first")
        # fonts are installed in the image's texmf tree (stable path -> stable
        # luaotfload cache); no per-build-dir copy needed for the compile.

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
                        "assets/ + the bundled .ttf fonts must sit beside the tex files.\n")
            for entry_id in provenance:
                zf.write(os.path.join(build_dir, book_tex.ASSET_DIR, f"{entry_id}.jpg"),
                         f"{book_tex.ASSET_DIR}/{entry_id}.jpg")
            if os.path.isdir(FONT_DIR):
                for fname in sorted(os.listdir(FONT_DIR)):
                    if fname.lower().endswith(".ttf"):
                        zf.write(os.path.join(FONT_DIR, fname), fname)
        zip_buf.seek(0)
        s3.upload_fileobj(zip_buf, BUCKET, out_prefix + "source.zip",
                          ExtraArgs={"ContentType": "application/zip"})

        # flipbook pages (flipbook.md §5.2): best-effort — the PDF is the
        # primary artifact, so a rasterization failure records flip_error
        # in latest.json instead of failing the compile
        flip_fields = {}
        try:
            _phase(job_id, task_id, "processing", "flipbook",
                   f"Flipbook pages 0/{content_pages}", **progress)
            flip_fields = _render_flipbook_pages(
                build_dir, out_prefix, book, content_pages,
                progress_cb=lambda done, total: _phase(
                    job_id, task_id, "processing", "flipbook",
                    f"Flipbook pages {done}/{total}", **progress))
        except Exception as exc:  # noqa: BLE001
            flip_fields = {"flip_error": str(exc)[:300]}
            print(f"flipbook failed for {book_id}/{compile_id}: {exc}")

        latest = {
            "compile_id": compile_id,
            "cover_key": out_prefix + "cover.pdf",
            "content_key": out_prefix + "content.pdf",
            "source_key": out_prefix + "source.zip",
            "content_pages": content_pages,
            "spread_count": len(entries),
            "compiled_at": _utc_now_iso(),
            **flip_fields,
        }
        # latest.json is the mutable pointer the public flipbook viewer
        # fetches over HTTP: without no-cache a recompile serves stale
        # pointers from browser heuristic caching (flipbook.md §2)
        s3.put_object(Bucket=BUCKET, Key=f"{BOOKS_PREFIX}{book_id}/out/latest.json",
                      Body=json.dumps(latest).encode("utf-8"),
                      ContentType="application/json",
                      CacheControl="no-cache, max-age=0")
        _phase(job_id, task_id, "done", "done", "Done", **progress,
               content_pages=content_pages,
               flip_page_count=latest.get("flip_page_count", 0),
               flip_error=latest.get("flip_error", ""),
               **{k: latest[k] for k in ("cover_key", "content_key", "source_key")})
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
