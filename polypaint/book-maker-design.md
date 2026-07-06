# Book Maker — Design

**Status: DRAFT for review. Nothing implemented.**

An in-app "Book" tab: collect Color artifacts from anywhere in the app into a named,
ordered book; edit per-entry text; compile server-side into a WhiteWall-compatible
cover + content PDF pair. Replaces the offline `make_polypaint_book.py` flow, whose
two structural problems are that it is program-blind (only understands legacy
`function/param_transforms/coeff_transforms` metas) and offline-only (works on
`snaps/` files, never talks to the backend).

---

## 1. Goal and scope

**V1 (this doc's commitment):**
- Named books: create / list / load / save / delete, mirroring the saved-program modals.
- Book tab: ordered entry list with thumbnails, add/remove/move up/down, per-entry
  title + body text overrides, cover picker, Compile button with live progress.
- "Add to Book" from the Render→color tab and the Favorites tab.
- Server-side compile: per-entry image prep (cached), then a single compose step
  producing `cover.pdf` (629.4×316 mm gross) + `content.pdf` (293×296 mm gross pages,
  multiple of 4), with program-aware provenance text via `build_pdf_report_model`.
- Download via the existing presign flow.

**V2 (explicitly out of V1):** drag-reorder, per-book style options (fonts/colors),
full program-source appendix section, per-spread PDF preview thumbnails, mosaic
context-menu add, Step Functions orchestration, wraparound cover art, multi-user
edit safety beyond last-write-wins.

---

## 2. Verified building blocks (what we reuse)

| Block | Where | What it gives us |
|---|---|---|
| Provenance model | `lambda/handler_pdf_artifact.py:327` `build_pdf_report_model` | Summary rows + programs[] with 3-level fallback (stored source text → chain decompiled → legacy transforms bridged via `pipeline_programs`). Works for program-era AND legacy artifacts. |
| Spread drawing | `lambda/spread_pdf.py` | `prepare_pdf_image` (vipsthumbnail `/opt/bin`, PIL fallback, bomb guard), code-block/KV-grid/section primitives, appendix pagination. |
| Print geometry | `make_polypaint_book.py` | WhiteWall dims (cover 629×316 mm; content 293×296 mm, bleed 3 sides), font registry + embedding, `_emit_black_page` padding, title-page layout. |
| Dispatch fan-out | `lambda/handler_dispatch.py:22-41,48-54` | `{target, jobs[]}` → async Event invoke per job via the `FUNCTIONS` env map; response `{fired, total}`. |
| Progress | `lambda/shared.py:31-46` `report_status` → DDB; `/check-status` (`handler_storage.py:3945`); frontend observer `js/10-status-results.js:1535` | Phase labels, `subtask_prefix`+`expected` sub-progress, `expected` fan-in counting, hard-stale abandon (PDF runs already have it). |
| Named-object CRUD | `handler_storage.py` coeff/root program routes (:1373-1384, :1351-1360) | S3-JSON docs under a prefix, slugified ids, HEAD-metadata cheap list, `errors[]` non-fatal listing, `_XxxNotFound` → 404, saved_at-desc sort. |
| Cross-job references + hydration | favorites (`handler_storage.py:1258-1295`, `js/01-core-compute.js:303-360`) | Reference-not-copy precedent; hydration via `/render-summary` grouped by job; `missing: true` rows instead of silent pruning. |
| Tab pattern | `index.html:1598-1607`, `switchTab` `js/01-core-compute.js:64-74` | Button label lowercased == tab key; panel `id="tab-<key>"`; lazy loader line in `switchTab`. |

Known gaps these blocks do NOT cover (each addressed below): no PDF concatenation
capability anywhere (no pypdf in any layer — `build-pdf-python-layer.sh:20` installs
exactly `reportlab Pillow`); `spread_pdf.py` uses core Helvetica/Courier which are
**not embedded** (WhiteWall requires embedded fonts); ColorSpread geometry is
290 mm-net spreads, not the 293×296 book page; favorites has no ordering.

---

## 3. User flow

1. **Collect.** On the Render tab (color family) an **Add to Book** button sits next
   to Favorite/PDF in the action row (`js/11-artifacts.js:1100` region). The Favorites
   tab toolbar gets the same button for the selected favorite. Both add to the
   **active book** (the one last opened in the Book tab; shown in the button tooltip
   and status line; if none exists, prompt to name one).
2. **Arrange.** The Book tab lists entries in book order: thumbnail, title, source
   job/artifact, ▲/▼ move buttons, Remove. Selecting an entry shows a text editor:
   Title and Body, both defaulting to auto-generated text (placeholder shows the
   auto text; empty override = use auto). Cover picker chooses any entry's image
   (or none → typographic cover).
3. **Verify.** Entries hydrate like favorites (grouped `/render-summary` calls);
   deleted sources show as `missing` rows. Compile refuses while any entry is
   missing, listing the offenders (no silent skips).
4. **Compile.** One button. Phases stream in the status line + jobs rail:
   `prepare k/N` → `compose` → `upload` → `done`. Output row appears with
   **Cover PDF** / **Content PDF** download buttons (presigned).
5. **Iterate.** Text/order edits + recompile are fast: image prep is cached per
   entry, so recompile is compose-only (~1 min, see §7 numbers).

---

## 4. Data model and storage

### Book document — S3 JSON (programs pattern, not favorites-DDB)

Why S3-JSON and not the favorites DDB pattern: a book carries per-entry text
overrides and provenance snapshots — 36 entries × (overrides + a few KB snapshot)
≈ 100–300 KB, uncomfortably near DDB's 400 KB item cap and pointless to shard;
the programs pattern already gives us versioned docs, cheap HEAD listing, and
modal UX for free. Ordering is the array order — reorder = save the doc.

```
polypaint/books/{book_id}.json                      # the document (source of truth)
polypaint/books/{book_id}/assets/{entry_id}.jpg     # prepared image cache (≤3600px, jpeg)
polypaint/books/{book_id}/assets/{entry_id}.provenance.json
polypaint/books/{book_id}/out/{compile_id}/cover.pdf
polypaint/books/{book_id}/out/{compile_id}/content.pdf
polypaint/books/{book_id}/out/latest.json           # pointer written by compose
```

Document schema (v1):

```json
{
  "version": 1,
  "book_kind": "book",
  "id": "my-first-book",            // slug of name, coeff-program slugifier reused
  "name": "My First Book",
  "title": "PolyPaint",             // printed title page / cover
  "subtitle": "",
  "author": "",
  "cover_entry_id": "",             // "" = typographic cover
  "entries": [
    {
      "entry_id": "e_1a2b3c",       // random, stable across reorder; allows same artifact twice
      "job_id": "mmw2ilf7",
      "artifact_id": "color_run_...",
      "image_key": "renders/.../image.jpeg",
      "display_name": "...",
      "added_at": "2026-07-06T12:00:00Z",
      "title_override": "",          // "" = auto
      "body_override": ""            // "" = auto
    }
  ],
  "saved_at": "..."                  // set by backend on save
}
```

S3 object metadata headers for cheap list (mirroring `coeff_program_*`):
`book_name`, `book_entry_count`, `book_saved_at`.

### Reference vs copy — the decision

Entries **reference** render artifacts (favorites precedent, favourites.md:47), but
compile **freezes** what matters:

- **Prepared image** cached at `assets/{entry_id}.jpg` on first prepare. After that,
  the book no longer needs the source image; job deletion breaks only *re*-prepare.
- **Provenance snapshot** (`assets/{entry_id}.provenance.json`) written at the same
  moment: the `calc.json` pipeline block + the color-artifact meta subset that
  `build_pdf_report_model` consumes. Compose builds its report model from the
  snapshot, never from live `renders/…`, so a book compiles identically forever
  once each entry has been prepared once.
- The Book tab surfaces `missing` on hydration exactly like favorites — visible,
  never auto-pruned. An entry that is missing but already prepared still compiles
  (badge it "pinned"); missing AND unprepared blocks compile with a named list.

This is deliberately *not* copy-on-add: sources can be hundreds of MB and add-time
copies would double storage for entries the user may remove; prepare-time caching
copies only the ≤3600 px derivative it actually needs (~3–7 MB each).

### Deletion

`/delete-book` deletes the doc **and** the `polypaint/books/{book_id}/` prefix
(assets + outputs), guarded to that prefix. Render artifacts are never touched
(favorites rule). Removing a single entry from the doc leaves its cached asset;
a compile sweep deletes assets whose `entry_id` is no longer in the doc.

---

## 5. Backend surface

### Storage routes (handler_storage.py, same lambda)

| Route | Payload | Response |
|---|---|---|
| `/list-books` | `{}` | `{books: [{id, name, entry_count, saved_at}], count, order: "saved_at_desc", errors, error_count}` |
| `/fetch-book` | `{id}` | `{book, latest_output}` (`latest_output` from `out/latest.json`, null if never compiled) |
| `/save-book` | `{book}` (id optional → slug from name) | `{book, overwritten}` |
| `/delete-book` | `{id}` | `{id, deleted}` (doc + prefix object count) |

Same contracts as coeff/root programs: `_BookNotFound` → 404, printable
single-line name ≤120 chars, list never fails on one bad object (`errors[]`),
validation rejects unknown `book_kind`, entries missing `job_id/artifact_id/image_key`,
and >200 entries (sanity cap, WhiteWall books are ~30–40 spreads).
Compose/prepare write `out/latest.json` and assets rather than mutating the doc,
so background compiles never race user saves.

### One new lambda: `polypaint-book-pdf`

One function, two dispatch ops (one worker keeps the deploy surface small; both
ops need the same layers):

- **Dispatch target `book_pdf`** (new entry in `handler_dispatch.py FUNCTIONS`,
  env `BOOK_PDF_FUNCTION` added to the dispatch env map in `deploy_manifest.json`).
- **op `prepare`** — payload
  `{op: "prepare", job_id, task_id, book_id, entry_id, source_job_id, source_artifact_id, source_image_key}`.
  Idempotent: if `assets/{entry_id}.jpg` + `.provenance.json` exist, report done
  immediately. Else: download source, `prepare_pdf_image` (jpeg, q92, max 3600 px),
  read `renders/{source_job_id}/calc.json` + color meta, write both asset objects.
  Phases: `load_source → prepare_image → snapshot → done`.
- **op `compose`** — payload
  `{op: "compose", job_id, task_id, book_id, compile_id}`.
  Reads the book doc + all assets, draws cover + content PDFs (§6), uploads to
  `out/{compile_id}/`, writes `out/latest.json`
  (`{compile_id, cover_key, content_key, content_pages, spread_count, compiled_at}`).
  Phases: `load_assets → compose_cover → compose_content k/N → upload → done`
  with `image_key`-style result fields for the observer.

`deploy_manifest.json` entry (initial sizing, rationale in §7):
`memory_mb: 4096`, `tmp: 2048`, layers `["libvips", "pdf_py"]`, no API routes
(async-only worker, reached exclusively through dispatch — pdf_artifact precedent).
The function zip bundles `fonts/` (see §6 fonts).

### Compile orchestration — frontend-chained V1

The frontend Compile button:
1. Dispatches `book_pdf` prepare jobs for every entry lacking a cached asset
   (batch `jobs[]`, task ids `bookprep_{runId}_{entry_id}`), saves an active run
   `{mode: 'book', job_id: book-scoped, run_id, task_id_prefix}` and observes
   `/check-status` with `task_prefix: 'bookprep_' + runId, expected: N` — the
   existing observer already renders fan-in counts from `done/expected`.
2. On `complete`, dispatches the single compose job (`task_id: bookcomp_{runId}`)
   and observes it to `done`.

Known weakness, stated up front: closing the tab between phases stalls the chain
(prepares finish; compose never fires). Acceptable for V1 because every stage is
idempotent — reopening and pressing Compile again skips all cached prepares and
goes straight to compose. V2 moves the chain into a Step Functions workflow
(`stepfunctions/*.template` + orchestrator lambda precedent) if this bites.
Status writes use a synthetic `job_id = "book#{book_id}"` partition so book runs
never collide with render runs (favorites' `favorites#color` precedent).

---

## 6. The compose step (the real new code)

New module `lambda/book_pdf.py`, sharing drawing primitives with `spread_pdf.py`
(extract the shared helpers — `_draw_code_block`, `_draw_kv_grid`, section headers,
wrapping — into a common module rather than duplicating; `spread_pdf.py` keeps its
public API and geometry).

**Geometry: WhiteWall, from `make_polypaint_book.py`.** Content page 290 mm net →
293×296 mm gross (bleed on 3 sides); cover one 629.4×316 mm gross spread
(`make_polypaint_book.py:246-262`). Verified: `spread_pdf.py` already uses the
*identical* per-page geometry (`CONTENT_NET = 290*mm`, same gross math,
spread_pdf.py:49-56) — ColorSpread merely emits two such pages side-by-side as one
double-width PDF page. Compose therefore reuses the existing page math and emits
single pages; only the cover geometry is genuinely new code.

**Page plan (content PDF):**
```
p1  title page (title / subtitle / author)     p2  blank verso
per entry, in order:  verso text page | recto full-bleed image page   (2 pages each)
tail: black pads (_emit_black_page port) to reach a multiple of 4
```
36 entries → 2 + 72 + 2 pad = 76 pages. Verso layout is the book style (black page,
display serif title, body text, mono pipeline + artifact line, optional palette
strip), with text resolved as: override if set, else auto title
(`Study {fn}` style) + auto body from the provenance snapshot's report model —
summary lines plus the program sources condensed (full multi-page appendix is V2;
V1 truncates each program block to what fits the verso, with an explicit
"(truncated)" caption — no silent cuts).

**Cover PDF:** back panel / spine / front panel on one 629×316 mm page; front panel
carries the cover entry's image at the local book's 2/3-above-title layout, or a
typographic cover when `cover_entry_id` is empty. Spine text = title. Note: the
629×316 template is the 28-page A3-square WhiteWall product the local book was
built for; spine width varies with page count on other products — V1 pins this one
template and the doc records `content_pages` so mismatches are visible (open
question §9.3).

**Fonts — must be embedded (WhiteWall hard requirement).** Core Helvetica/Courier
are never embedded by reportlab, so compose uses only bundled TTFs from the repo's
`fonts/` dir (Canela/Tiempos trials for display/body, CourierPrime or JetBrainsMono
for mono — same defaults as `make_polypaint_book.py`), registered à la its
`FONT_REGISTRY` including the Helvetica-shadow trick so reportlab's default font
state resolves to an embedded face. The fonts ship inside the function zip
(~a few MB); glyph-fallback logic (`_font_has_glyph` / mixed runs) ports as-is.
Text sanitation: provenance is already ASCII-sanitized by the handler's
`_stringify_meta` conventions; keep `_safe_pdf_text` as the last line of defense.

**Image format: JPEG q92, not PNG.** Derivation: fractal-noise content compresses
poorly as PNG — a 3600×3600 render is realistically 15–30 MB PNG vs 3–7 MB JPEG q92.
At 36 entries that's ~0.5–1.1 GB of embedded PNG (breaches WhiteWall's 1 GB content
cap) vs ~150–250 MB JPEG (comfortable). 3600 px on a 296 mm page = 309 DPI full-bleed
(296 mm / 25.4 × 300 DPI = 3496 px needed), inside WhiteWall's 180–300 PPI band.
Front-panel cover art at ~296 mm wide (`PANEL_W`, make_polypaint_book.py:258) needs
~3496 px — the same 3600 cap serves
(full-wraparound cover art would need ~7400 px; V2).

---

## 7. Numbers (initial sizing, re-derive before implementation)

| Quantity | Derivation | Result |
|---|---|---|
| Asset size | 3600² JPEG q92, dense content | ~3–7 MB |
| Prepare wall/entry | source download (up to a few hundred MB) + vips shrink-on-load | ~5–20 s, parallel fan-out |
| Compose /tmp peak | 36 assets ×7 MB + content.pdf ~250 MB + cover | ~600 MB → `tmp: 2048` |
| Compose memory | reportlab canvas + one decoded 3600² RGB (~39 MB) at a time; pdf_artifact runs 2048 MB for one image | 4096 MB, headroom |
| Compose wall | 250 MB S3 down + sequential draw of ~76 pages | ≲60–120 s |
| Recompile (all cached) | compose only | ~1–2 min end to end |
| Content PDF | 36×~5 MB + text | ~200 MB ≪ 1 GB cap |
| Book doc | 36 entries × overrides | ~50–300 KB |

Timeout is the global 900 s — compose fits with a wide margin; a fully-cold 40-entry
prepare fan-out is bounded by the slowest single source, not the sum.

---

## 8. Frontend changes

- **New part `js/14-book.js`** — registration is three-touch by contract:
  `<script>` tag in index.html (order after 13), `__ppParts.push('14-book')` trailer,
  and the inline `expected` array at index.html:3273-3284. (`api_manifest.py`
  auto-discovers parts from the script tags.)
- **Tab:** button `Book` in the tab-bar (label lowercased must equal key `book` —
  `switchTab` matches on `textContent.toLowerCase()`), panel `id="tab-book"`
  following the house skeleton: toolbar `btn-book-*`, `book-status`, `book-preview`,
  `book-log`; lazy `if (name === 'book') loadBookTab();` line in `switchTab`.
- **Toolbar:** book selector (name + Load/New/Save/Delete via a modal reusing
  `_bindProgramModalFilterSort` / `_naturalNameCompare`), Compile, Download menu
  (Cover / Content via `/presign`), entry count + missing count info span.
- **Entry list:** rows keyed like favorites (`data-key`, key-based selection
  surviving re-render, ArrowUp/Down in the boot listener), thumbnails from
  hydrated `preview_url`; ▲/▼ mutate the in-memory entries array and mark the
  book dirty (explicit Save button; autosave-on-compile).
- **Hydration:** generalize `_hydrateFavoriteArtifacts` (group refs by job →
  `/render-summary`, mark `missing`) so favorites and books share it.
- **Add buttons:** `btn-render-add-book` in the color action row (same
  enable/labeled-when-present logic as Favorite at `js/11-artifacts.js:1043`);
  `btn-favorites-add-book` in the Favorites toolbar. Payload goes through a
  client-side `_bookAddEntry(ref)` that loads the active book doc, appends, saves.
- **Compile progress:** `_saveActiveRun({mode: 'book', ...})`; the existing
  observer needs a `mode === 'book'` completion branch (dispatch compose, then
  final refresh) — modeled on the PDF branch including its hard-stale abandon.

---

## 9. Decisions and open questions

**Decided in this doc (challenge in review if wrong):**
1. Reference + prepare-time freeze (assets + provenance snapshots), not copy-on-add
   and not live-at-compile. §4.
2. S3-JSON book docs via the programs CRUD pattern; DDB only for run status. §4.
3. One `book_pdf` lambda, two ops, dispatch-only; frontend-chained two-phase
   compile in V1, idempotent stages as the recovery story. §5.
4. Compose-from-assets in one canvas — **no PDF concatenation, no pypdf**. Folios
   and page plan are trivially global in a single canvas; the merge step and a new
   dependency disappear. §6.
5. JPEG q92 / 3600 px assets. §6.
6. WhiteWall geometry from make_polypaint_book, bundled embedded fonts only. §6.

**Genuinely open (user calls):**
1. **Book aesthetic vs ColorSpread aesthetic for the verso pages.** This doc assumes
   the black/Canela book style ported from make_polypaint_book. If the navy
   ColorSpread report style is wanted instead (or as an option), compose absorbs it
   cheaply — but pick one for V1.
2. **Trial-font licensing for print.** The repo's Canela/Tiempos are trial cuts; the
   user already prints locally with them, but embedding them in a Lambda-produced
   commercial print file is the same question in a new place. Swap-in candidates
   exist in `fonts/` (Lyon, Sibila, CourierPrime are non-trial files).
3. **Spine/page-count coupling.** V1 pins the 28-page-template cover dims while
   producing variable page counts. Confirm against WhiteWall's actual product specs
   before the first real order (the local book had the same exposure).
4. **Auto body text quality.** V1 auto text is mechanical (summary + programs). The
   "artsy description" layer of the printed book stays a manual override pass —
   fine, or do we want an assisted-description flow later?

---

## 10. Testing and gates

Per house rules: predeploy runs on every commit; payload contract tests assert
exact dispatched shapes; nothing here deploys — the user deploys.

- `tests/test_book_storage.py` — CRUD, slugify, 404s, list `errors[]`, sort,
  entry validation, delete-prefix guard (model: `test_root_program_storage.py`).
- `tests/test_book_pdf_handler.py` — prepare idempotence (cache hit short-circuit),
  provenance snapshot content, compose phase sequence + `out/latest.json`,
  missing-asset failure (model: `test_pdf_artifact_handler.py`).
- `tests/test_book_pdf.py` — page plan math (2 + 2N + pad ≡ 0 mod 4), geometry
  constants, font embedding assertion (fonts present in the PDF's font resources —
  this is the WhiteWall regression that matters), truncation captions
  (model: `test_spread_pdf.py`).
- Payload contracts — frontend dispatch payloads for `book_pdf` prepare/compose and
  the four storage routes (model: existing `test_api_route_contracts.py` +
  payload-contract suites).
- `api_manifest.py --write` + `deploy_manifest.py --check` after route/function
  edits; parts-contract test covers the new js file.
- Playwright `tests/e2e/book-ui.spec.js` — add→reorder→save→reload round-trip,
  missing-entry compile block, compile happy path with stubbed backend
  (model: `favorites-ui.spec.js`).

---

## 11. Build order

1. Storage routes + book doc validation + tests (pure backend, no deploy deps).
2. `book_pdf` prepare op + snapshot format + tests.
3. `book_pdf` compose op: geometry/fonts port from make_polypaint_book +
   shared-primitives extraction from spread_pdf + tests.
4. Deploy surface: deploy_manifest entry, dispatch target, api_manifest regen.
5. Book tab UI + add buttons + observer branch + Playwright.
6. User deploys; end-to-end run on a real 3–5 entry book; then a full-size book
   against the WhiteWall preflight.
