# flipbook.md — self-hosted page-turning book viewer

Replace the FlowPaper workflow (download content.pdf → run pdf_compress.sh →
upload to their converter) with a native flipbook: the book compile renders
web-sized page images once, server-side, and a standalone viewer on the
public bucket serves the page-turn experience. No third-party service, no
manual compression step, app look and feel.

## 0. Why FlowPaper hurt (and what it actually does)

The compiled book is print-grade — 293×296 mm pages full of q92 JPEGs;
content.pdf runs to hundreds of MB, which FlowPaper's converter only accepts
after `pdf_compress.sh` shrinks it. But FlowPaper's real product is exactly
two things we already know how to do: rasterize PDF pages server-side, and
drive a page-flip UI over the images. We have the rasterizer's home (the
book-pdf Lambda container), a public bucket with immutable-cache discipline,
and an established standalone-viewer pattern (viewer.html,
artifact_mosaic_viewer.html).

`pdf_compress.sh` stays for whatever else it's useful for; the flipbook path
never touches it.

## 1. Design decisions

- **Pre-rendered page JPGs, not client-side PDF.js.** PDF.js would need the
  whole (or a linearized) content.pdf downloaded before first paint —
  exactly the FlowPaper pain again, on a slow line. Pre-rendered pages load
  lazily, a spread at a time.
- **Rasterize inside the existing book compose** (handler_book_pdf compose
  op): the PDF is already on /tmp there, the container is ours, and the
  docker gate covers it. No new Lambda, no new job type.
- **pdftoppm (poppler-utils) as the rasterizer.** The image's libvips is
  built with `-Dpoppler=disabled` (book_pdf.Dockerfile:24) so vips cannot
  load PDFs; `dnf install -y poppler-utils` in the runtime stage adds
  pdftoppm, the standard fast path (fallback if AL2023 ever drops it:
  ghostscript). Gate assertion: pdftoppm renders a page of the fixture book.
- **StPageFlip (page-flip, MIT) vendored into the repo** (js/vendor/
  page-flip.browser.js, git-tracked like fonts/) — no CDN surprises on a
  share link. Single-page mode on cover, two-page spreads after, arrows +
  swipe + keyboard built in.
- **Viewer is a static file** — `flipbook.html` at the repo root, uploaded
  with the site assets, opened as
  `flipbook.html?book={book_id}` (same query-param pattern as the mosaic
  share viewer).

## 2. Data layout (additive)

Compose already writes `books/{book_id}/out/{compile_id}/content.pdf` and
the pointer `books/{book_id}/out/latest.json` (book_pdf.py:307-403). It
additionally writes, per compile:

```
books/{book_id}/out/{compile_id}/flip/p0001.jpg …   (one per PDF page)
books/{book_id}/out/{compile_id}/flip/flip.json     (manifest)
```

- Pages: `pdftoppm -jpeg -r 120 -jpegopt quality=85` → ~1384×1398 px per
  page (293 mm at 120 dpi). A 35-spread book = 1 title + 70 entry pages +
  ≤3 pads ≈ 74 pages ≈ 74 × ~150–350 KB ≈ **15–25 MB total**, fetched
  lazily ~2 pages per turn. Immutable cache headers (compile-id-scoped
  keys, CACHE_IMMUTABLE from shared.py).
- `flip.json` (no-cache is unnecessary — compile-scoped — but the pointer
  below is the mutable hop): `{book_id, compile_id, title, page_count,
  width_px, height_px, pages: ["flip/p0001.jpg", …]}`.
- `latest.json` gains additive fields: `flip_key` (the manifest key) +
  `flip_page_count`. Old latest.json without them → viewer says "no
  flipbook yet, recompile".

## 3. Viewer — flipbook.html

- Reads `?book={id}` → fetches `books/{id}/out/latest.json` (public) →
  fetches flip.json → builds StPageFlip with `showCover: true` (title page
  alone, then verso/recto spreads — matching the print page plan where
  front matter is odd so text faces image per opening).
- Deep-blue app chrome (#1A1A2E page background), title from flip.json,
  page x/y indicator, arrows/keys/swipe, fullscreen button.
- Preloads the next spread on turn; everything else on demand.
- Optional `?compile={compile_id}` pins a specific compile (default:
  latest).

## 4. Book tab UI

Next to Cover PDF / Content PDF / Source Zip (index.html:2106-2108):

- **Flipbook** button — opens `flipbook.html?book={activeId}` in a new tab
  (viewing is the point of a flipbook; this is the one legitimate new-tab
  case). Busy/disabled states per the all-buttons-have-feedback rule;
  "Not compiled yet" flash when latest.json lacks flip_key.
- **Copy Flip Link** — clipboard-copies the public URL (share pattern,
  same feedback as mosaic Share).

## 5. Implementation order

1. **Dockerfile + gate**: poppler-utils in book_pdf.Dockerfile;
   scripts/test-book-docker.sh gains a pdftoppm assertion (render page 1 of
   the fixture book, assert JPEG magic + expected pixel width at -r 120).
2. **Compose op** (handler_book_pdf): after content.pdf upload — pdftoppm
   the pages, parallel-upload flip/*.jpg (CACHE_IMMUTABLE) + flip.json,
   extend latest.json additively. Tests: payload-contract on the flip
   uploads + manifest shape + latest.json fields; failure of the flip step
   must NOT fail the compile (book PDF is the primary artifact — flip_error
   recorded in latest.json instead).
3. **flipbook.html + vendored page-flip** + deploy.sh site-asset list entry.
4. **Book tab buttons** (js/14-book.js + index.html) with busy feedback;
   Playwright covers the not-compiled hint path.
5. Gates per repo law: book docker gate (image + templates changed),
   book/pipeline pytest suites, Playwright (JS), predeploy on every commit.

## 6. Numbers to verify live (render-first discipline)

- pdftoppm timing on the 10-row worst-case fixture in the gate (~74-page
  book expected well under 60 s at -r 120; compose timeout already 600+ s).
- One real book: page JPG sizes vs the 15–25 MB estimate; visual check of a
  text verso at 120 dpi (KV mono text must be crisp — if not, bump to 150
  dpi ≈ +55% bytes before shipping).

## 7. Deferred

- 2× resolution tier for pinch-zoom inside the flipbook (page-flip supports
  swapping sources; only if 120 dpi feels soft on tablets).
- Cover PDF as book jacket wrap-around view.
- Flipbook link QR on the printed colophon page (fun: the printed book
  linking to its digital twin).
