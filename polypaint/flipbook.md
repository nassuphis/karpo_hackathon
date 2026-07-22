# flipbook.md — self-hosted page-turning book viewer

Replace the FlowPaper workflow (download content.pdf → run pdf_compress.sh →
upload to their converter) with a native flipbook: the book compile renders
web-sized page images once, server-side, and a standalone viewer on the
public bucket serves the page-turn experience. No third-party service, no
manual compression step, app look and feel. The web edition is complete:
front cover, blank inside-front cover, every content page, blank inside-back
cover, then the back cover.

Rev 4 adds correct physical cover parity. Every path, budget, and tool below is
verified against the code or tested live; the verification notes say how.

## 0. Why FlowPaper hurt (and what it actually does)

The compiled book is print-grade — 293×296 mm pages full of q92 JPEGs —
and FlowPaper's converter only accepts it after `pdf_compress.sh` shrinks
it. But its real product is two things we already host: rasterize PDF pages
server-side, drive a page-flip UI over the images. We have the rasterizer's
home (the book-pdf container Lambda), a public bucket with immutable-cache
discipline, and the standalone-viewer pattern (viewer.html,
artifact_mosaic_viewer.html). `pdf_compress.sh` stays for other uses; the
flipbook path never touches it.

## 1. Design decisions

- **Pre-rendered page JPGs, not client-side PDF.js** — PDF.js needs the
  whole print-grade PDF (or a linearized one) before first paint: the
  FlowPaper pain again, on a slow line. Pre-rendered pages stream a spread
  at a time.
- **Rasterize inside the existing compose op** — both `book.pdf` (content)
  and `cover.pdf` (one printable `back | spine | front` jacket spread) are
  already in `/tmp/book_build_{compile_id}/` when the upload phase runs.
  No new Lambda, no new job type; the compile's jobs-rail card covers it.
- **Split the cover by physical trim geometry, not by halving pixels.** The
  right 299 mm panel is the front cover and the left panel is the back;
  10 mm outer bleed and the 11 mm spine are excluded. Each panel is
  center-trimmed from 299 mm to the content page's 293 mm width, producing
  exactly the same raster dimensions as every interior page. This is why
  StPageFlip can display the first and final covers without stretching.
- **Represent both inside covers as blank pages.** With `showCover: true`,
  StPageFlip displays page 1 alone, then pairs pages 2|3, 4|5, and so on.
  The inside-front blank therefore places the content PDF's title page on
  the right; every following `report | image` pair stays on one opening.
  The inside-back blank keeps the final back cover as a standalone cover.
- **One Book background selection.** The persisted six-digit
  `background_color` defaults to `1a1a2e` and drives the printable jacket,
  title/report/pad pages, and both generated inside-cover JPEGs. Full-bleed
  artwork pages remain untouched. Typography switches between light and dark
  automatically; QR codes retain fixed dark-on-white contrast.
- **pdftoppm as the rasterizer** — VERIFIED LIVE: `dnf install -y
  poppler-utils` succeeds on `public.ecr.aws/lambda/python:3.12` arm64 and
  installs pdftoppm 24.08.0 (tested 2026-07-08 in docker). It renders PNG
  intermediates at 200 DPI; Pillow then writes q88 4:4:4 JPEGs. Needed because
  the image's libvips is built `-Dpoppler=disabled`
  (book_pdf.Dockerfile:24) and cannot load PDFs.
- **StPageFlip (page-flip, MIT) vendored at `vendor/page-flip.browser.js`**
  — NOT under `js/`: `deployed_asset_key` (deploy.sh:201-208) rewrites
  `js/*` to build-versioned `assets/${BUILD_ID}/…` keys and ONLY index.html
  gets URL-stamping, so a `js/vendor/…` reference from flipbook.html would
  dangle. The `*` case uploads `vendor/…` at its stable key with
  `--cache-control no-cache`, same as artifact_mosaic_viewer.html.
- **flipbook.html is a static root asset** (stable URL, no-cache), opened
  as `flipbook.html?book={book_id}`. Both new files must be added to the
  explicit `frontend_asset_keys` list (deploy.sh:73-84) or deploy never
  uploads them.

## 2. Data layout (additive; real prefix)

`BOOKS_PREFIX = "polypaint/books/"` (book_pdf.py:25 — NOT `books/`).
Compose writes `polypaint/books/{book_id}/out/{compile_id}/content.pdf` +
pointer `polypaint/books/{book_id}/out/latest.json` (book_pdf.py:307-403).
It additionally writes per compile:

```
polypaint/books/{book_id}/out/{compile_id}/flip/p0001.jpg …
polypaint/books/{book_id}/out/{compile_id}/flip/flip.json
```

- Page order: `p0001.jpg` is the front panel cropped from `cover.pdf`;
  `p0002.jpg` is the blank inside-front cover; `p0003.jpg…` are all pages
  from `content.pdf`; the penultimate JPEG is the blank inside-back cover;
  the final JPEG is the back panel cropped from `cover.pdf`. The content PDF
  itself is unchanged.
- Pages: `pdftoppm -png -r 200 ...`, followed by Pillow q88 4:4:4 JPEG
  encoding. Output naming quirk: pdftoppm emits `<prefix>-NN.png` padded to the
  digit count of the LAST page (74 pages → `-01…-74`); the handler globs
  and renames to deterministic `p%04d.jpg` before upload.
- Geometry: the 293×296 mm content page is approximately 2307×2331 px at
  200 DPI. A 35-spread book has 1 title + 70 entry pages + ≤3 pads + 4 cover
  and inside-cover pages. Pages stream approximately two at a time rather than downloading
  either print-grade PDF.
- Headers: `flip/*.jpg` + `flip.json` are compile-id-scoped = immutable →
  `CacheControl=CACHE_IMMUTABLE` (shared.py, already COPY'd into the book
  image). **latest.json is the mutable pointer and today ships with NO
  cache headers (book_pdf.py:403) — the public viewer would see stale
  pointers after a recompile. Fix in the same commit: add
  `CacheControl="no-cache, max-age=0"` to that put.** (The Book tab never
  noticed because it reads latest via the storage route, not HTTP cache.)
- `flip.json`: `{book_id, compile_id, title, page_count,
  content_page_count, front_cover_page:1, inside_front_cover_page:2,
  content_first_page:3, content_last_page, inside_back_cover_page,
  back_cover_page, width_px, height_px, pages:["p0001.jpg", …]}` (names
  relative to the flip/ prefix).
  `latest.json` gains additive `flip_key` + `flip_page_count` —
  and on rasterization failure, `flip_error` instead (see §5.2).

## 3. Budgets (derived from live config)

Function `polypaint-book-pdf` (deploy_manifest.json): 4096 MB memory,
**/tmp = 2048 MB** (deploy.sh:396), timeout $TIMEOUT = 900 s.

- Disk at peak (inside the existing build_dir lifecycle, cleaned in
  `finally`): entry assets ~100–250 MB + book.pdf + cover.pdf ~100–350 MB
  + flip pages and one temporary cover-spread PNG remain below the 2048 MB
  allocation for the supported Book sizes.
- Time: real compiles run ~2–3 min of the 900 s budget. pdftoppm at ~1–2
  s/page serial ≈ 75–150 s for 74 pages; run **4 parallel workers over
  `-f/-l` page ranges** (4096 MB comfortably holds 4 × ~200–400 MB
  renderers) → ~20–40 s added. Gate measures the fixture's actual time.
- Upload: ~76 small objects through the existing pattern, seconds.

## 4. Viewer — flipbook.html

- Query params: `?book={id}` (+ optional `?compile={compile_id}` to pin;
  default latest). Client-side sanitize with the server's own rule,
  `/^[A-Za-z0-9._#-]{1,80}$/` (mirrors book_pdf._safe_id, book_pdf.py:164)
  — S3 keys don't path-traverse, but garbage should fail fast and loud.
- Flow: fetch `polypaint/books/{id}/out/latest.json` (public) → if no
  `flip_key`: show "No flipbook for this compile yet — recompile the book"
  (+ surface `flip_error` if present) → else fetch flip.json → StPageFlip
  with `showCover: true`: p1 is the front cover and stands alone; the first
  opening is `blank inside-front | title`; every item is then
  `report/palette | image`; the last opening ends with a blank inside-back,
  and the final back cover stands alone. The inside-cover blanks preserve
  the interior page pairing guaranteed by `book_tex.page_plan`.
- Deep-blue chrome (#1A1A2E), title from flip.json, "page x / y", arrows +
  keys + swipe (library-native), fullscreen button. Preload next spread on
  turn.

## 5. Implementation order

1. **Dockerfile + gate**: `dnf install -y poppler-utils` in the runtime
   stage (book_pdf.Dockerfile:33 block); scripts/test-book-docker.sh
   renders page 1 of the fixture book at `-r 120` and asserts JPEG magic +
   width 1384±1 px, and times the full-book rasterization.
2. **Compose op** (book_pdf.py, upload phase): rasterize content, rasterize
   and split the jacket cover, insert both blank inside covers, preserve the
   front/blank/content/blank/back order, then rename →
   parallel-upload flip/*.jpg + flip.json (immutable) → extend latest
   dict additively → **latest.json put gains no-cache** (§2). Failure
   isolation: the flip block is try/except; on error latest.json carries
   `flip_error` and the compile still succeeds (the PDF is the primary
   artifact). Tests: payload contracts on flip uploads/manifest/latest
   fields + the failure-isolation path, in tests/test_book_pdf_handler.py.
3. **flipbook.html + vendor/page-flip.browser.js** + both added to
   `frontend_asset_keys` (deploy.sh:73-84).
4. **Book tab** (index.html:2106-2108 toolbar + js/14-book.js):
   - **Flipbook** button — enabled off `_bookState.latestOutput.flip_key`;
     `latest_output` already flows verbatim through the storage fetch
     (handler_storage.handle_fetch_book, :2015-2029 → js/14-book.js:49),
     so zero storage changes. Opens the stable viewer URL in a new tab
     (the one legitimate new-tab case — it IS a viewer); flashes
     "Recompile first" on the button when absent (feedback rule).
   - **Copy Flip Link** — `_copyTextToClipboard` (global from js/13) with
     copied-state feedback.
5. Gates: book docker gate (image changed — mandatory), book pytest
   suites, Playwright (JS), predeploy every commit.

## 6. Render-first checks before shipping

- Gate prints fixture rasterization time; abort the 4-worker design if a
  page exceeds ~3 s (would threaten the 900 s budget on huge books).
- One real book end-to-end: page sizes vs the 15–25 MB estimate, and a
  visual read of a text verso at 120 dpi — the mono KV text must be crisp
  on a laptop screen; if soft, bump to 150 dpi (≈ +55% bytes) before
  shipping, not after.

## 7. Deferred

- 2× zoom tier for tablets (page-flip supports source swapping).
- Separate jacket wrap-around viewer for inspecting the unsplit cover PDF.
- QR of the flip link on a printed colophon page (the printed book linking
  to its digital twin).
