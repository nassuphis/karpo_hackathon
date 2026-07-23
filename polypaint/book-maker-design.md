# Book Maker — Design

**Status: DRAFT v2 for review. Nothing implemented.**
v2 changes after review: **one spread per entry** (program-appendix spreads dropped as
redundant — the verso carries condensed provenance); **compose is LaTeX, not reportlab**
(user decision: TeX-quality typesetting, WhiteWall-ready with no modifications;
knitr positioned as an optional local authoring path, not the server engine).

An in-app "Book" tab: collect Color artifacts from anywhere in the app into a named,
ordered book; edit per-entry text; compile server-side into a WhiteWall-compatible
cover + content PDF pair — `make_polypaint_book.py`, but in the app, program-aware,
and print-final. The offline script's two structural problems remain the motivation:
it is program-blind (only understands legacy `function/param_transforms/coeff_transforms`
metas) and offline-only (works on `snaps/` files, never talks to the backend).

---

## 1. Goal and scope

**V1 (this doc's commitment):**
- Named books: create / list / load / save / delete, mirroring the saved-program modals.
- Book tab: ordered entry list with thumbnails, add/remove/move up/down, per-entry
  title + body text overrides, cover picker, Compile button with live progress.
- "Add to Book" from three surfaces: the AllCol/AllPal mosaic right-click menu
  (the best whole-collection overview — primary collection surface), the
  Render→color tab action row, and the Favorites tab toolbar.
- Server-side compile: per-entry image prep (cached), then a single LaTeX compose step
  producing a page-count-specific `cover.pdf` (626×316 mm gross for 44 content
  pages) + `content.pdf` (293×296 mm gross pages,
  multiple of 4). **One spread per entry**: verso text page + recto full-bleed image.
  Verso text is program-aware via `build_pdf_report_model` provenance snapshots.
- "Export book source": download the generated `.tex` + assets as a bundle for local
  hand-finishing (this is where a knitr/LaTeX workflow plugs in if wanted).
- Download of the compiled pair via the existing presign flow.

**V2 (explicitly out of V1):** drag-reorder, per-book style/template options,
per-spread PDF preview thumbnails, Step Functions orchestration, wraparound
cover art, assisted "artsy description" generation, multi-user edit safety
beyond last-write-wins.

**Dropped (not deferred):** program-source appendix spreads. One spread per color
item; full program listings belong in the exported source bundle, not the printed book.

---

## 2. Verified building blocks (what we reuse, what's genuinely new)

| Block | Where | What it gives us |
|---|---|---|
| Provenance model | `lambda/handler_pdf_artifact.py:327` `build_pdf_report_model` | Summary rows + programs[] with 3-level fallback (stored source text → chain decompiled → legacy transforms bridged via `pipeline_programs`). Works for program-era AND legacy artifacts. Feeds the verso template. |
| Image prep | `lambda/spread_pdf.py` `prepare_pdf_image` | Shared Book/ColorSpread/local contract: `vipsheader` inspects the source and `vipsthumbnail` normalizes it to a bounded JPEG before any PDF engine sees it. There is no Pillow source-decoding fallback. |
| Print geometry + book layout | WhiteWall `cover_A3square_paper-fujiCrystal-semi-matte_28.idml` and `_44.idml` | Content 290 mm net → 293×296 mm gross (bleed on 3 sides). Each cover panel is 296 mm and outer bleed is 10 mm. Spine width follows content pages: 11 mm at 28 pages and 14 mm at 44 pages, increasing 0.75 mm per four pages. Therefore the 44-page cover is 606×296 mm trim and 626×316 mm gross. The old local script double-counted 3 mm on each panel and is not a geometry authority. |
| Dispatch fan-out | `lambda/handler_dispatch.py:22-41,48-64` (response `{fired, total}` at :101-104) | `{target, jobs[]}` → async Event invoke per job via the `FUNCTIONS` env map; response `{fired, total}`. |
| Progress | `lambda/shared.py:31-46` `report_status` → DDB; `/check-status` (`handler_storage.py:3945`); observer `js/10-status-results.js:1535` | Phase labels, `expected` fan-in counting, hard-stale abandon (PDF runs already have it). |
| Named-object CRUD | `handler_storage.py` coeff/root program routes (:1373-1384, :1351-1360) | S3-JSON docs under a prefix, slugified ids, HEAD-metadata cheap list, `errors[]` non-fatal listing, `_XxxNotFound` → 404, saved_at-desc sort. |
| Cross-job references + hydration | favorites (`handler_storage.py:1258-1295`, `js/01-core-compute.js:303-360`) | Reference-not-copy precedent; hydration via `/render-summary` grouped by job; `missing: true` rows instead of silent pruning. |
| Docker arm64 discipline | `scripts/test-docker-runtime.sh` | The pattern for building and testing the compose container locally before the user deploys. |
| Tab pattern | `index.html:1598-1607`, `switchTab` `js/01-core-compute.js:64-74` | Button label lowercased == tab key; panel `id="tab-<key>"`; lazy loader line in `switchTab`. |

**Genuinely new (no precedent in the repo):** a TeX toolchain in Lambda. No function
today is container-image based (all zip+layers, `deploy.sh:1429-1500`), and no layer
carries TeX (`lambda/build-pdf-python-layer.sh:20` installs exactly `reportlab Pillow` —
which the book path now bypasses entirely). This is the main engineering cost and is
priced in §5/§7.

---

## 3. User flow

1. **Collect.** Three entry points, all adding to the **active book** (the one
   last opened in the Book tab; shown in the button tooltip/menu label and status
   line; if none exists, prompt to name one):
   - **AllCol/AllPal mosaic right-click menu** — the primary surface (user call:
     "the best overview we have"). Mirrors the existing mosaic "Favorite" action
     (`js/13-artifact-mosaics.js:789-799`): an "Add to Book" item, enabled for
     Color tiles and disabled with an "(Color only)" label otherwise — book
     entries are Color artifacts by contract (AllPal tiles are palettes, so
     there the item stays disabled, same as Favorite does today).
   - **Render→color tab** — `btn-render-add-book` next to Favorite/PDF in the
     action row (`js/11-artifacts.js:1100` region).
   - **Favorites tab toolbar** — `btn-favorites-add-book` for the selected
     favorite.
2. **Arrange.** The Book tab lists entries in book order: thumbnail, title, source
   job/artifact, ▲/▼ move buttons, Remove. Selecting an entry shows a text editor:
   Title and Body, both defaulting to auto-generated text (placeholder shows the
   auto text; empty override = use auto). Cover picker chooses any entry's image
   (or none → typographic cover).
3. **Verify.** Entries hydrate like favorites (grouped `/render-summary` calls);
   deleted sources show as `missing` rows. Compile refuses while any entry is
   missing AND unprepared, listing the offenders (no silent skips).
4. **Compile.** One button. Phases stream in the status line + jobs rail:
   `prepare k/N` → `compose (latex)` → `upload` → `done`. Output row appears with
   **Cover PDF** / **Content PDF** / **Export Source** buttons (presigned).
5. **Iterate.** Text/order edits + recompile are fast: image prep is cached per
   entry, so recompile is compose-only (≲2 min, §7).

---

## 4. Data model and storage

### Book document — S3 JSON (programs pattern, not favorites-DDB)

Why S3-JSON and not the favorites DDB pattern: a book carries per-entry text
overrides (long body texts across ~36 entries reach tens of KB, and DDB's 400 KB
item cap turns a growing book into a sharding exercise for zero benefit);
the programs pattern already gives us versioned docs, cheap HEAD listing, and
modal UX for free. Ordering is the array order — reorder = save the doc.

```
polypaint/books/{book_id}.json                      # the document (source of truth)
polypaint/books/{book_id}/assets/{entry_id}.jpg     # prepared image cache (≤5000px, jpeg)
polypaint/books/{book_id}/assets/{entry_id}.provenance.json
polypaint/books/{book_id}/out/{compile_id}/cover.pdf
polypaint/books/{book_id}/out/{compile_id}/content.pdf
polypaint/books/{book_id}/out/{compile_id}/source.zip   # .tex + assets export bundle
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
  "background_color": "1a1a2e",  // whole-book page/jacket background; default deep blue
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
  `build_pdf_report_model` consumes. Compose builds verso text from the snapshot,
  never from live `renders/…`, so a book compiles identically forever once each
  entry has been prepared once.
- The Book tab surfaces `missing` on hydration exactly like favorites — visible,
  never auto-pruned. An entry that is missing but already prepared still compiles
  (badge it "pinned"); missing AND unprepared blocks compile with a named list.

This is deliberately *not* copy-on-add: sources can be hundreds of MB and add-time
copies would double storage for entries the user may remove; prepare-time caching
copies only the ≤5000 px derivative it actually needs (~6–14 MB each).

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

Same contracts as coeff/root programs — with one addition the flat program
prefixes never needed: `polypaint/books/` mixes doc keys with per-book
`{id}/assets/…` and `{id}/out/…` objects (all `.json`-suffixed too), so
`/list-books` MUST skip ids containing `/` (the same guard the program list
already has at `handler_storage.py:1813`). `_BookNotFound` → 404, printable
single-line name ≤120 chars, list never fails on one bad object (`errors[]`),
validation rejects unknown `book_kind`, entries missing `job_id/artifact_id/image_key`,
and >200 entries (sanity cap, WhiteWall books are ~30–40 spreads).
Compose/prepare write `out/latest.json` and assets rather than mutating the doc,
so background compiles never race user saves.

### One new lambda: `polypaint-book-pdf` — the app's first container-image function

One function, two dispatch ops. Container image because the compose op needs a TeX
toolchain, which cannot ship as a zip layer at any reasonable size; prepare rides in
the same image (libvips baked in) to keep one deploy artifact.

- **Image:** AWS Lambda Python 3.12 arm64 base + libvips + a **TinyTeX-style minimal
  TeX Live**: `install-tl -scheme=infraonly` + a pinned `tlmgr install` list
  (`latex, luatex, fontspec, microtype, geometry, graphics, xcolor, eso-pic,
  luaotfload` + their deps) + the repo `fonts/` TTFs baked into the texmf tree.
  The template is one fixed stylized layout, so the package set is closed and known
  at build time. Validated against the user's local TinyTeX (334 MB, 197 packages,
  TeX Live 2024), which already carries every package on the list — expect
  ~250–350 MB of TeX in the image, ~700 MB–1 GB total (§7), far under the 10 GB
  image cap. Engine choice: LuaLaTeX over XeLaTeX/tectonic because full microtype
  (font expansion + protrusion) is LuaTeX-only, and typographic quality is the point
  of this pivot. `\pdfvariable minorversion` pins PDF 1.4+ compat for WhiteWall.
- **Dispatch target `book_pdf`** (new entry in `handler_dispatch.py FUNCTIONS`,
  env `BOOK_PDF_FUNCTION` added to the dispatch env map in `deploy_manifest.json`).
- **op `prepare`** — payload
  `{op: "prepare", job_id, task_id, book_id, entry_id, source_job_id, source_artifact_id, source_image_key}`.
  Idempotent: if `assets/{entry_id}.jpg` + `.provenance.json` exist, report done
  immediately. Else: download source, `prepare_pdf_image` in required-libvips mode
  (jpeg, q92, max 5000 px),
  read `renders/{source_job_id}/calc.json` + color meta, write both asset objects.
  Phases: `load_source → prepare_image → snapshot → done`.
- **op `compose`** — payload
  `{op: "compose", job_id, task_id, book_id, compile_id, expected_saved_at}`.
  Reads the book doc (fails cleanly if its `saved_at` differs from
  `expected_saved_at` — the user saved mid-compile; recompile picks up the new
  state) + all assets (fails listing the entry_ids of any missing assets — an
  entry added after the prepare fan-out has none), **generates `book.tex` + `cover.tex` from Python
  templates** (§6), runs `lualatex` (2 passes) for each, uploads `cover.pdf`,
  `content.pdf`, and `source.zip` to `out/{compile_id}/`, writes `out/latest.json`
  (`{compile_id, cover_key, content_key, source_key, content_pages, spread_count, compiled_at}`).
  Phases: `load_assets → compose_tex → latex_cover → latex_content → upload → done`.
  A LaTeX failure uploads the `.log` tail into the error status — never a silent die.

**Pipeline boundary:** PDF asset preparation and web-flipbook rasterization are
separate operations. The prepare op shrinks each full render artifact to a 5000 px
JPEG before LuaLaTeX builds `content.pdf`. Only after that PDF exists does the
flipbook stage rasterize PDF pages with `pdftoppm` (currently 200 DPI) and encode
the viewer JPEGs. `BOOK_ASSET_MAX_PX` therefore controls print-PDF inputs only; it
must not be reused as a flipbook resolution setting.

`deploy_manifest.json` entry (initial sizing, rationale in §7):
`memory_mb: 4096`, `tmp: 2048`, `package_type: image` (schema extension — see
deploy-surface note), no API routes (async-only worker, reached exclusively through
dispatch — pdf_artifact precedent).

**Deploy-surface cost (honest):** deploy.sh today only creates zip functions
(`create_lambda`/`update_lambda`, deploy.sh:1429-1500). Image functions need: an ECR
repo, `docker buildx --platform linux/arm64` build+push, `create-function
--package-type Image`, and a `deploy_manifest.py` schema extension
(`package_type`/`image` fields + validation). One-time infrastructure, tested
locally via the `test-docker-runtime.sh` pattern before the user deploys.

### Compile orchestration — frontend-chained V1

The frontend Compile button:
1. Dispatches `book_pdf` prepare jobs for ALL entries — prepare is idempotent and
   cached entries no-op instantly, so `expected` is simply the entry count (the
   frontend has no other way to know which assets exist; for the §4 "pinned"
   badge the Book tab batch-HEADs the computed asset keys via the existing
   `/head-keys` route, `handler_storage.py:4441`). Jobs are
   (batch `jobs[]`, task ids `bookprep_{runId}_{entry_id}`), saves an active run
   `{mode: 'book', job_id: book-scoped, run_id, task_id_prefix}` and polls
   `/check-status` with `task_prefix: 'bookprep_' + runId, expected: N`.
   Verified semantics (`handler_storage.py:4039-4048`): `done` counts rows with
   status done, `total = done + errored`, and **`complete` means `total >=
   expected` — all tasks TERMINAL, not all successful**. The compose trigger is
   therefore **`done >= N && errors === 0`**; any `errors > 0` aborts with the
   error details (never compose over a failed prepare). This also requires a
   dedicated `mode === 'book'` observer branch: the existing loop hardcodes
   `expected: 1` (`js/10-status-results.js:1548`) and treats
   `check.complete || results[0].phase === 'done'` as run completion (:1587) —
   with a multi-task prefix that fires on the FIRST finished prepare, so neither
   condition can be reused for the fan-out phase.
2. When `done == N` with zero errors, dispatches the single compose job
   (`task_id: bookcomp_{runId}`) and observes it to `done` (this leg matches the
   existing single-task PDF pattern, including hard-stale abandon).

Known weakness, stated up front: closing the tab between phases stalls the chain
(prepares finish; compose never fires). Acceptable for V1 because every stage is
idempotent — reopening and pressing Compile again skips all cached prepares and
goes straight to compose. V2 moves the chain into a Step Functions workflow
(`stepfunctions/*.template` + orchestrator lambda precedent) if this bites.
Status writes use a synthetic `job_id = "book#{book_id}"` partition so book runs
never collide with render runs (favorites' `favorites#color` precedent).

---

## 6. The compose step — LaTeX templates (the real new code)

New module `lambda/book_tex.py`: Python renders `.tex` from the book doc +
provenance snapshots (plain string templating with a hard escaping layer — every
user/provenance string passes a `_tex_escape` that handles `\ { } $ & # % ^ _ ~`
and strips control chars; program source is set verbatim-style, not escaped-inline).
**Why not knitr as the server engine:** knitr's job is executing R chunks into a
document; at compose time there is nothing left to execute — the provenance is
precomputed in the snapshots, so templating is pure string work Python already does
here. TeX is what buys the typography, and it is fully present. The exported
`source.zip` (tex + assets + a README with the lualatex invocation) is the bridge to
a local knitr/LaTeX workflow for hand-finished editions.

**What LaTeX buys over the reportlab draft (why this pivot is right):** the
Knuth-Plass paragraph breaker with real hyphenation for the verso body text,
microtype optical margins + font expansion, automatic font embedding via fontspec
(the WhiteWall embedded-fonts requirement is satisfied by construction instead of
by the core-font workaround the reportlab path needed), and — decisive for
maintenance — the user can art-direct the template directly.

**Geometry (WhiteWall, verified against the publisher IDML templates):**
content page 290 mm net → **293×296 mm gross** with bleed on 3 sides
(`\geometry{paperwidth=293mm, paperheight=296mm}`, all margins explicit, full-bleed
image pages via `eso-pic`/`tikz` overlay at page edges); cover a single
page-count-specific gross page. Both panels are **296 mm**, with **10 mm** bleed
on every outer edge. For 44 content pages the spine is **14 mm**, so the cover
is `296 + 14 + 296 = 606 mm` trim and **626×316 mm gross**. The 28-page template
has an 11 mm spine and is 623×316 mm gross. The measured progression is
0.75 mm per four content pages.
JPEG assets embed byte-for-byte (DCTDecode pass-through — no recompression, unlike
any raster round-trip).

**Page plan (content PDF):** front matter must be an ODD page count — page 1 is a
recto, and entry text must land on versos (left) with images on rectos (right) of
the same opening. The local script gets this right with exactly one front page
(`make_polypaint_book.py:905-916`: p1 blank recto, then text p2 (L) | image p3 (R));
a 2-page front matter would shift every spread across two openings.
```
p1  title page (recto; blank-vs-title is a template choice, §9.1)
per entry, in order:  verso text page | recto full-bleed image page  (2 pages each)
tail: selected-background pads to reach a multiple of 4   (pad = 3 for even N, 1 for odd N)
```
36 entries → 1 + 72 + 3 pad = 76 pages. **One spread per entry — no appendix
spreads.** Verso layout follows the printed book style (selected page background,
contrast-selected display serif
title, body text, mono pipeline + artifact-id lines, optional palette strip), with
text resolved as: override if set, else auto title + auto body from the provenance
snapshot — summary lines plus the pipeline string, program names/fingerprints, and
a *short* source excerpt where it fits. Full program listings live in `source.zip`,
not on the page.

**Cover PDF:** back / spine / front on one page; front panel carries the cover
entry's image at the local book's 2/3-above-title layout, or a typographic cover
when `cover_entry_id` is empty. The same `background_color` drives the jacket,
content title/report/pad pages, and web flipbook inside covers; deep blue
(`#1A1A2E`) is the default. Spine text = title. The old 609 mm trim width came
from a local-script reconstruction that double-counted panel bleed and is not
used. The generator derives the Fuji Crystal semi-matte spine and gross width
from `content_pages`; `latest.json` records the cover width, height, and spine.

**Fonts:** `polypaint/fonts/` TTFs (git-tracked — 13 files, ~840 KB; only the bulk
font-family download folders stay gitignored for size/filecount) installed in the
image's texmf tree, selected via fontspec — Canela/Tiempos (display/body),
CourierPrime or JetBrainsMono (mono). fontspec embeds them; there is no un-embedded
core-font escape hatch to police. Licensing is settled: this is a strictly
non-commercial project (gifts for friends, zero revenue), within the trial /
free-for-non-commercial terms, and free open-source equivalents exist as swap-ins
if a face ever needs replacing. The Docker build COPYs `fonts/` from the repo and
MUST still fail if the directory is missing or empty — never a font-less image.
Glyph coverage: LuaLaTeX handles fallback per `\setmainfont` feature chains;
provenance text is ASCII-sanitized upstream anyway (`_stringify_meta` conventions).

**Determinism guard:** two lualatex passes, `-interaction=nonstopmode -halt-on-error`,
`SOURCE_DATE_EPOCH` pinned from `compile_id` so recompiles of identical input are
byte-comparable.

---

## 7. Numbers (initial sizing, re-derive before implementation)

| Quantity | Derivation | Result |
|---|---|---|
| Asset size | 5000² JPEG q92, dense content | ~6–14 MB |
| Print resolution | 5000 px / (296 mm / 25.4) | ~429 DPI full-bleed; LuaLaTeX never sees the original 30K-class raster |
| Prepare wall/entry | source download (up to a few hundred MB) + vips shrink-on-load | ~5–20 s, parallel fan-out |
| Container image | lambda python arm64 base (~250 MB) + TinyTeX-style pinned TeX (~250–350 MB, matches user's local 334 MB TinyTeX) + libvips (~30 MB) + fonts | ~0.7–1 GB (cap 10 GB) |
| Cold start | ~1 GB image pull, Lambda-cached after first | +1–3 s vs zip; irrelevant at these run lengths |
| Compose /tmp peak | 36 assets ×14 MB + tex build dir + content.pdf up to ~500 MB + source.zip (~500 MB, JPEGs don't compress) | ~1.5 GB worst case → `tmp: 2048` |
| Compose memory | lualatex on a 76-page image book + vips absent from this op | 4096 MB, headroom |
| Compose wall | 250–500 MB S3 down + 2 lualatex passes (image-heavy but images are includes) | ~1–2 min |
| Content PDF | 36 prepared JPEGs pass-through + text | ~250–500 MB ≪ 1 GB WhiteWall cap |
| Book doc | 36 entries × overrides | ~50–300 KB |

Timeout is the global 900 s — compose fits with margin; a fully-cold 40-entry
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
  (Cover / Content / Source via `/presign`), entry count + missing count info span.
- **Entry list:** rows keyed like favorites (`data-key`, key-based selection
  surviving re-render, ArrowUp/Down in the boot listener), thumbnails from
  hydrated `preview_url`; ▲/▼ mutate the in-memory entries array and mark the
  book dirty (explicit Save button; autosave-on-compile).
- **Hydration:** generalize `_hydrateFavoriteArtifacts` (group refs by job →
  `/render-summary`, mark `missing`) so favorites and books share it.
- **Add buttons:** the AllCol/AllPal mosaic context menu gains an "Add to Book"
  item next to "Favorite" (`js/13-artifact-mosaics.js:634,658,789` pattern:
  enabled for color tiles, "(Color only)" disabled label otherwise, active-book
  name shown in the item label); `btn-render-add-book` in the color action row
  (same enable/labeled-when-present logic as the Favorite button,
  `js/11-artifacts.js:1112` / enable logic :1043); `btn-favorites-add-book` in
  the Favorites toolbar. All three go through a client-side `_bookAddEntry(ref)`
  that loads the active book doc, appends, saves, and reports "(added Nth entry
  to <book>)" in the surface's status line.
- **Compile progress:** `_saveActiveRun({mode: 'book', ...})`; the existing
  observer needs a `mode === 'book'` completion branch (dispatch compose, then
  final refresh) — modeled on the PDF branch including its hard-stale abandon.

---

## 9. Decisions and open questions

**Decided (v2 revision; challenge in review if wrong):**
1. **One spread per entry, no appendix spreads** (user decision). Full sources go to
   `source.zip`. §6.
2. **LaTeX (LuaLaTeX) compose, not reportlab** (user decision); Python templates the
   `.tex` server-side; knitr is the optional *local* authoring path via the exported
   source bundle, not a server dependency. §6.
3. Container-image lambda for `book_pdf` — the unavoidable consequence of 2; priced
   and patterned on the existing Docker arm64 discipline. §5.
4. Reference + prepare-time freeze (assets + provenance snapshots), not copy-on-add
   and not live-at-compile. §4.
5. S3-JSON book docs via the programs CRUD pattern; DDB only for run status. §4.
6. Frontend-chained two-phase compile in V1, idempotent stages as the recovery
   story. §5.
7. JPEG q92 / 5000 px assets, prepared strictly with libvips and embedded pass-through. §6/§7.
8. Fonts are git-tracked in `polypaint/fonts/` and licensing is a non-issue: the
   project is strictly non-commercial (gifts, zero revenue), within trial /
   free-for-non-commercial terms; open-source equivalents exist as swap-ins. §6.
9. **Untrusted-input hardening** (adversarial review round, all shipped): every
   user string reaching HTML is `_escapeHtml`'d (book name, entry title, ids);
   `entry_id` is a strict `[A-Za-z0-9._-]{1,64}` slug and `id` a lowercase slug,
   so neither can break out of an `\includegraphics{}`/`onclick`/attribute;
   `title`/`subtitle`/`author` get the single-line printable guard `name` had;
   `/presign` is prefix-restricted to `renders/` + `polypaint/books/` (it was an
   open arbitrary-object oracle); `book_pdf` re-validates every id it splices into
   a `/tmp` path. The compile poller captures `bookId`/`savedAt` into the run so
   switching books mid-compile can't compose the wrong book.

**Genuinely open (user calls):**
1. **Verso template style.** Default = port of the printed book's black/Canela verso.
   Confirm fonts + layout on the first template proof (a one-spread test book).
2. **Other paper stocks.** The implemented spine rule is specific to Fuji Crystal
   semi-matte. Supporting another paper requires that paper's publisher templates;
   do not reuse this spine rule.
3. **Auto body text quality.** V1 auto text is mechanical (summary + pipeline). The
   "artsy description" layer stays a manual override pass — fine, or do we want an
   assisted-description flow later (V2 candidate)?

---

## 10. Testing and gates

Per house rules: predeploy runs on every commit; payload contract tests assert
exact dispatched shapes; nothing here deploys — the user deploys.

- `tests/test_book_storage.py` — CRUD, slugify, 404s, list `errors[]`, sort,
  entry validation, delete-prefix guard (model: `test_root_program_storage.py`).
- `tests/test_book_tex.py` — template rendering: page plan math (1 + 2N + pad ≡ 0
  mod 4, front matter odd so text stays on versos), `_tex_escape` fuzz (every provenance string class: `$`, `%`, `_`, `\`,
  unicode), geometry constants in the emitted preamble, override-vs-auto resolution.
  Pure Python, runs in the normal predeploy gate without TeX.
- **Docker arm64 TeX gate** — extend the `test-docker-runtime.sh` pattern: build the
  book_pdf image locally, compile a 3-entry fixture book inside it, assert PDF
  properties (page count, MediaBox 293×296 mm / page-count-specific cover,
  including 626×316 mm for 44 pages, fonts embedded —
  this is the WhiteWall regression that matters, checkable by parsing the font
  resource dicts for FontFile streams). Required whenever the image/template changes,
  mirroring the sweep_cli.c rule.
- `tests/test_book_pdf_handler.py` — prepare idempotence (cache hit short-circuit),
  provenance snapshot content, compose phase sequence + `out/latest.json`, LaTeX
  failure surfaces `.log` tail (model: `test_pdf_artifact_handler.py`).
- Payload contracts — frontend dispatch payloads for `book_pdf` prepare/compose and
  the four storage routes.
- `api_manifest.py --write` + `deploy_manifest.py --check` (schema extension) after
  route/function edits; parts-contract test covers the new js file.
- Playwright `tests/e2e/book-ui.spec.js` — add→reorder→save→reload round-trip,
  missing-entry compile block, compile happy path with stubbed backend
  (model: `favorites-ui.spec.js`).

---

## 11. Build order

1. Storage routes + book doc validation + tests (pure backend, no deploy deps).
2. `book_tex.py` templates + escaping + page-plan tests (no TeX needed to test
   template emission).
3. Container image (Dockerfile: base + TeX Live small + lualatex + libvips + fonts)
   + local Docker arm64 compile gate on a fixture book.
4. `book_pdf` handler: prepare op (snapshot format) + compose op (tex → lualatex →
   upload) + handler tests.
5. Deploy surface: deploy_manifest `package_type: image` schema extension, ECR +
   buildx push path in deploy.sh, dispatch target, api_manifest regen.
6. Book tab UI + add buttons + observer branch + Playwright.
7. User deploys; template proof on a 1-spread book (font rendering check, §9.1);
   then a full-size book against the WhiteWall preflight.
