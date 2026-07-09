# Code Review 25: Adversarial Review of the Recent Feature Stack

Reviewed commit: `f323ecc`

Scope reviewed: Book Maker / Describe / Compose / Flipbook, AllCol/AllPal share and wall-pyramid paths, storage routes touched by those features, and the new frontend glue. This review focused on failure modes that green unit tests often miss: stale writes, route-boundary abuse, secret/config integrity, untrusted key flow, and standalone viewer attack surface.

## Verdict

The feature stack is substantial and much better tested than most of the earlier experimental surfaces. The happy paths are largely coherent: structured Gemini output, per-entry Describe progress, PDF/flipbook build checks, mosaic share snapshots, wall-pyramid fallback, and frontend context-menu staleness guards all have real tests or defensible implementation.

I would not treat this as deployment-clean for a public or semi-public deployment yet. The main blockers are not syntax or normal regression failures; the focused suites pass. The blockers are API-boundary bugs: secret/config rows are reachable through generic storage routes, Describe can overwrite concurrent book edits with a stale full-document save, book `image_key` is trusted too deeply, and the deepzoom-export route exposes an internal wall-pyramid action.

## Verification Run

- `uv run python -m pytest tests/test_book_storage.py tests/test_book_tex.py tests/test_book_pdf_handler.py tests/test_describe_book_entries.py tests/test_wall_pyramid_handler.py tests/test_storage_handler.py tests/test_deploy_packaging.py -q`
- Result: `119 passed in 6.40s`
- `bash tests/test_frontend_js.sh`
- Result: frontend fused checks, generated help checks, lazy preview, jobs rail, artifact mosaic, results popup, coeff modal, and book row checks all passed.

## What Is Solid

- `lambda/book_describe.py` now uses structured Gemini output, skips thought parts, joins multi-part replies, detects early stop reasons, retries transient/model formatting failures once, and records per-entry failures without killing the whole run.
- `lambda/book_pdf.py` checks `expected_saved_at` before compose, verifies prepared assets, surfaces LaTeX log tails, and treats flipbook generation as best-effort rather than making the PDF build brittle.
- `js/13-artifact-mosaics.js` uses per-kind mosaic state, active-tile-source click mapping, context-menu staleness checks, and command helpers instead of blindly driving stale global UI state.
- `handler_storage.py` wraps the palette mosaic route correctly, snapshots share manifests server-side, and validates share `kind`/`size`/`sort`/`cols`.
- `book_tex.tex_escape` is applied to normal title/body/report text, and tests cover special-character escaping in the ordinary prose path.

## Findings

### F1 - HIGH: VisionModel secrets are stored in a generic task table partition that public cleanup routes can delete or mutate

Evidence:

- Vision keys are stored in `JOBS_TABLE` under `job_id="__config__"` and `task_id="vision_model"` in `lambda/handler_storage.py:2004` and `lambda/handler_storage.py:2076`.
- `/save-vision-config` is a storage API route in `lambda/handler_storage.py:1375` and is listed in `api_manifest.json:78` / `deploy_manifest.json:153`.
- `/delete-task` blindly deletes any `{job_id, task_id}` row in `lambda/handler_storage.py:5007`.
- `/clean-render` queries and batch-deletes every DDB row for arbitrary `job_id` in `lambda/handler_storage.py:4476`.

Impact:

Any client that can call storage routes can set, clear, or delete the VisionModel configuration. The direct reproducer is `/delete-task` with `{"job_id":"__config__","task_id":"vision_model"}`. `/clean-render` with `job_id="__config__"` also deletes it. The fetch route does not leak full keys, but mutation/deletion and quota-burning through Describe are enough to make this unsafe for an exposed app.

Fix:

Move Vision config out of the generic jobs/status table, or enforce a reserved-partition guard on every generic route that accepts `job_id`/`task_id` (`delete-task`, `clean-render`, `check-status`, future DDB cleanup tools). At minimum reject `job_id` values beginning with `__` outside dedicated internal handlers, and add tests proving `__config__`, `__allrenders_mosaic__`, and favorites/status sentinels cannot be deleted through generic APIs.

### F2 - HIGH: Describe still saves a stale full book after each entry, so concurrent edits can be lost

Evidence:

- Describe loads the book once and checks `expected_saved_at` only before the loop in `lambda/book_describe.py:412`.
- It mutates that in-memory `doc` and calls `/save-book` with the whole document after every generated entry in `lambda/book_describe.py:457`.
- `handle_save_book` overwrites the whole S3 book object unconditionally and assigns a new `saved_at` in `lambda/handler_storage.py:2088`.
- The existing `test_missing_key_and_saved_at_mismatch` only covers a mismatch at initial load; it does not simulate another save landing between entry 1 and entry 2.

Impact:

If the user edits the book while Describe is running, the next per-entry Describe save writes the old full document back and can erase the user edit. The comment at `lambda/book_describe.py:454` explicitly states the in-memory doc remains the source of truth for the whole run; that is the unsafe part.

Fix:

Add compare-and-swap semantics to `/save-book` using `expected_saved_at`, and make Describe pass the current expected value on each incremental save. On mismatch, either fail with a retryable status or refetch and merge only the generated fields for the specific `entry_id`. Do not repeatedly PUT the stale whole book without a version check.

### F3 - HIGH: Book `image_key` is accepted as arbitrary text and later used as trusted LaTeX/asset input

Evidence:

- `_validate_book_payload` only checks that `image_key` is present; it does not validate prefix, character set, extension, or render-family shape in `lambda/handler_storage.py:1943`.
- `book_tex._verso_report_page` concatenates `S3_PUBLIC_BASE + image_key` and inserts the result raw into `\qrcode{...}` in `lambda/book_tex.py:181`.
- `book_pdf.handle_prepare` downloads `source_image_key` directly from S3 with no prefix/shape validation in `lambda/book_pdf.py:355`.
- Tests assert the happy QR payload but do not include a malicious key containing `}` / TeX control sequences.

Impact:

The normal UI supplies safe render keys, but `/save-book` is an API route and the server accepts hostile `image_key` values. At compose time, that value becomes a raw LaTeX macro argument. With shell escape off this is probably not OS command execution, but it can still inject or corrupt TeX/PDF/source output. The same unvalidated key path lets prepare attempt to read/process arbitrary bucket keys as images.

Fix:

Validate book entry keys server-side. For book entries, accept only safe render image keys such as `renders/<job>/color/<artifact>/image.(jpeg|jpg|png)` and the known legacy root image forms. Reject braces, backslashes, whitespace control chars, `..`, and unexpected prefixes. Also escape or brace-protect QR payloads as defense in depth. Add tests for malicious `image_key` and for legacy-safe keys.

### F4 - HIGH: The public `/deepzoom-export` route can invoke the internal wall-pyramid builder

Evidence:

- `/deepzoom-export` is an API route in `api_manifest.json:96` and `deploy_manifest.json:342`.
- `lambda/handler_deepzoom_export.py:293` accepts `internal_action="build_wall_pyramid"` from parsed request body or raw event and immediately calls `handle_build_wall_pyramid`.
- `handle_build_wall_pyramid` loads the caller-supplied `manifest_key`, downloads every tile key in it with 32 workers, runs `wall_dz`, and uploads a pyramid under `renders/_index/{kind}_mosaic/{refresh_id}/` in `lambda/handler_wall_pyramid.py:125`.

Impact:

The wall build is intended as a Lambda-to-Lambda async action from storage, but the action is reachable through the public deepzoom-export handler shape. A caller can repeatedly trigger expensive 8GB/libvips work for a ready manifest, or supply malformed/large manifests to burn Lambda time. The status conditional protects the DDB row, but it does not prevent the expensive work from starting or wall artifacts from being uploaded for chosen refresh IDs.

Fix:

Do not expose `internal_action` through the API route. Reject internal actions when the event has API Gateway fields (`path`, `rawPath`, or `requestContext`), or require a signed internal token only storage knows. Validate `manifest_key` against `renders/_index/{kind}_mosaic/<refresh_id>/all.json` and validate `refresh_id` shape before any S3 reads or uploads.

### F5 - MEDIUM: Describe cannot describe some valid book entries because it ignores the stored `image_key`

Evidence:

- Book entries store `image_key` in `js/14-book.js:454`.
- Book hydration uses `/render-summary`, which can resolve legacy and current render entries in `js/14-book.js:58`.
- Describe ignores `entry.image_key` and hardcodes only immutable color previews at `renders/<job>/color/<artifact>/preview.{jpg,png}` in `lambda/book_describe.py:368`.

Impact:

A book entry can be valid enough for the UI and PDF compose, but Describe returns `no preview` if the artifact is legacy/root-shaped or otherwise lacks that exact immutable preview path. That makes Describe more brittle than the rest of the Book pipeline.

Fix:

Store a `preview_key` in book entries when adding them, or have Describe resolve the entry through `/render-summary` the same way the frontend does. At minimum, fall back to `entry.image_key` when no preview is found.

### F6 - MEDIUM: Standalone mosaic viewer trusts arbitrary `manifest=` URLs

Evidence:

- `artifact_mosaic_viewer.html:163` reads `manifest` from the query string.
- It sets that value directly as the visible manifest link href before validation in `artifact_mosaic_viewer.html:168`.
- It fetches the URL and then uses `manifest.base` to build tile, wall, and download URLs in `artifact_mosaic_viewer.html:75` and `artifact_mosaic_viewer.html:198`.

Impact:

The server-generated share URLs are safe, but the standalone viewer is also a general-purpose remote manifest viewer. A malicious URL can point the manifest link at `javascript:` or load attacker-controlled manifests/images. This is weaker than direct DOM XSS because tile values are mostly escaped, but the page should not advertise arbitrary query input as a trusted link.

Fix:

Validate `manifest` before assigning it to `href` or fetching it. For production share links, require HTTPS and the expected bucket/prefix: `renders/_shared_mosaic/<kind>/share_.../manifest.json`. Also validate `manifest.base` before using it for wall/image downloads.

### F7 - MEDIUM: Book source bundles redistribute every tracked TTF, including trial fonts

Evidence:

- The Docker image copies all tracked fonts into `/opt/book-fonts` and then into TeX Live in `lambda/book_pdf.Dockerfile:70`.
- `source.zip` includes every `.ttf` in `FONT_DIR` in `lambda/book_pdf.py:576`.
- The Dockerfile explicitly checks for `TiemposText-Regular-Trial.ttf` in `lambda/book_pdf.Dockerfile:71`.

Impact:

This is not a runtime correctness bug, but it is a packaging/legal risk. Every generated book source archive can include trial fonts, and the Lambda container image also redistributes them. If those trial licenses do not permit this, the feature creates a compliance problem.

Fix:

Bundle only fonts with licenses that allow redistribution in generated archives, or omit fonts from `source.zip` and document local font installation. If trial fonts are needed in Lambda, keep them out of public downloadable source bundles.

### F8 - LOW/MEDIUM: Book PDF container builds are not reproducible

Evidence:

- The Dockerfile downloads TeX Live from `https://mirror.ctan.org/.../install-tl-unx.tar.gz` at build time in `lambda/book_pdf.Dockerfile:47`.
- `tlmgr install ...` installs current package versions with no frozen repository date in `lambda/book_pdf.Dockerfile:55`.

Impact:

A future rebuild can break without a code change, or produce slightly different TeX/package behavior. This is operational risk rather than a current product bug.

Fix:

Pin a TeX Live repository snapshot date, or build from a prebuilt base image whose TeX Live layer is versioned. Keep the package list explicit, but also pin the source.

## Test Gaps To Add

- Describe concurrent edit: save a book, start Describe, make the fake `/save-book` mutate server `saved_at`/content between entries, and assert Describe does not overwrite the external edit.
- Reserved DDB rows: `/delete-task` and `/clean-render` must reject `__config__`, `__allrenders_mosaic__`, favorites, and any future sentinel partitions.
- Book key validation: malicious `image_key` with `}`/`\input`/newlines is rejected by `/save-book`; allowed current and legacy render keys still pass.
- Internal action boundary: API-shaped `/deepzoom-export` event with `internal_action=build_wall_pyramid` is rejected, while direct Lambda event with the internal token/shape is accepted.
- Standalone viewer URL validation: `manifest=javascript:...` and external manifest hosts do not become clickable links and do not load.
- Describe legacy entry: a book entry with `artifact_id=legacy_color` and root `image_key` can still be described, or fails with a clear unsupported-entry message.

## Recommended Fix Order

1. Protect reserved DDB partitions and move/guard VisionModel config.
2. Add `/save-book` CAS and update Describe incremental saves to merge or fail on concurrent edits.
3. Validate book `image_key` / `source_image_key` and escape QR payloads.
4. Lock down `/deepzoom-export` internal wall-pyramid action.
5. Make Describe use stored/resolved preview keys.
6. Harden the standalone mosaic viewer’s manifest/base URL validation.
7. Decide and document the font redistribution policy.
8. Pin TeX Live/container build inputs.

## Deployment Call

For a private, single-user environment, the feature stack is probably usable after accepting the known stale-save risk. For anything exposed beyond a trusted operator, fix F1-F4 first. Those are boundary bugs, not cosmetic polish.
