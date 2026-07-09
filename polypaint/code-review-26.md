# Code Review 26: Post-CR25 Adversarial Review

Reviewed head: `d7bb945`

Scope reviewed: the fixes after `code-review-25.md`, the Book/Describe/PDF paths those fixes touched, the standalone AllCol/AllPal share viewer, the wall-pyramid internal action path, the storage route boundary, the calculation-VM/native oracle gates, and the new `lambda/fable_programs.py` seed corpus.

## Verdict

Most CR25 fixes are real. The focused Book, storage, wall-pyramid, DeepZoom-export, frontend, and fable happy-path checks are green. The remaining problems are not broad syntax/test failures; they are edge-case consistency bugs and boundary gaps in the same family as CR25: one field is validated while adjacent fields remain trusted, conflict recovery replays more state than it should, and seed/packaging utilities are not gated tightly enough.

I would not call this deployment-clean for anything beyond the explicitly documented single-user/public-stack assumption. The highest-risk remaining issues are:

- VisionModel config and stored API keys remain mutable and usable through unauthenticated public routes.
- Describe CAS recovery can still overwrite a human edit to an entry already generated earlier in the same Describe run.
- Book/PDF provenance can be forged by mixing `image_key`, `job_id`, and `artifact_id` from different artifacts.
- Generic render deletion can wipe the internal mosaic index/share prefixes because only DDB sentinels were reserved.
- The wall-pyramid path depends on the post-migration "all wall preview tiles are 512px" invariant but does not assert it.

## Verification Run

- `.venv/bin/python -m pytest tests/test_book_storage.py tests/test_book_pdf_handler.py tests/test_describe_book_entries.py tests/test_deepzoom_export_handler.py tests/test_wall_pyramid_handler.py tests/test_storage_handler.py -q`
- Result: `95 passed in 22.61s`
- `bash tests/test_frontend_js.sh`
- Result: all frontend fused/help/jobs/mosaic/book/share checks passed.
- `.venv/bin/python lambda/fable_programs.py --dry-run`
- Result: all 12 fables passed parity, worst relative error roughly `4.10e-08` to `5.72e-08`.
- `.venv/bin/python lambda/fable_programs.py --only does_not_exist --dry-run`
- Result: exits 0 with `all 0 passed parity`, which is a real script bug.
- `git diff --name-only f323ecc..HEAD | rg 'sweep_cli|program_chain|program_source|root_xforms|solve_score|coeff|param|fable|raw_score|score_raw|vm|registry|opcodes|\.c$|\.h$'`
- Result: only `polypaint/lambda/fable_programs.py`; the core C VM, program compilers, registries, and native headers are not changed in this feature range.
- `.venv/bin/python -m pytest tests/test_coeff_vm_property_fuzz.py tests/test_coeff_program_native.py tests/test_param_program_native.py tests/test_solve_score_native_parity.py tests/test_whole_sweep_oracle.py tests/test_program_m3_oracles.py tests/test_coeff_wire_fingerprints.py -q`
- Result: `67 passed, 45 subtests passed in 10.11s`

VM/runtime boundary:

The calculation VM is not showing a new regression in this review range. The native/property/oracle gates are present in `scripts/predeploy_check.sh`, and `tests/test_predeploy_gate_completeness.py` requires the most important VM gates: whole-sweep oracle, solve-score native parity, M3 oracle, and coeff wire fingerprints. The remaining fable issue is different: the new authored fable corpus is only a script today, so it is not automatically exercising those VM/compiler paths in predeploy.

## CR25 Fix Status

Fixed:

- Generic DDB mutation routes now reject reserved partitions with `_assert_mutable_job_partition` in `lambda/handler_storage.py:4511`, used by `handle_clean_render` and `handle_delete_task`.
- `/save-book` now supports `expected_saved_at`, and Describe uses a CAS/refetch path in `lambda/book_describe.py:356`.
- Book `image_key` and PDF `source_image_key` now go through `assert_safe_render_image_key` in `lambda/handler_storage.py:1961` and `lambda/book_pdf.py:383`.
- API-shaped `/deepzoom-export` events can no longer invoke `internal_action=build_wall_pyramid`; the direct-invoke path remains available.
- `handler_wall_pyramid.py` validates `kind`, `refresh_id`, and the canonical manifest key shape before loading a manifest.
- The standalone mosaic viewer now validates `manifest` and `manifest.base` against the expected S3 host before fetch/link/tile use.
- Book `source.zip` font bundling now excludes trial/demo fonts and only includes referenced TTFs.
- TeX Live installs from a fixed TL2025 snapshot instead of a moving CTAN mirror.

Partially fixed:

- Describe now falls back from immutable preview keys to `entry.image_key`, which fixes missing legacy previews, but it may send a full-size render image to the Vision API.
- Book key syntax is now safe, but the server still does not enforce that `job_id`, `artifact_id`, and `image_key` describe the same artifact.
- VisionModel config can no longer be wiped through `/delete-task` or `/clean-render`, but it is still directly public-mutable through `/save-vision-config`.

## Findings

### F1 - HIGH: VisionModel config is still public-mutable and publicly usable

Evidence:

- The deployment explicitly has public-read bucket and unauthenticated API routes in `deploy.sh:125-128`.
- `/fetch-vision-config` and `/save-vision-config` are public storage routes in `lambda/handler_storage.py:1376-1379`.
- Vision config is still stored under `job_id="__config__"` / `task_id="vision_model"` in `lambda/handler_storage.py:2010-2011`.
- `handle_save_vision_config` accepts any caller-supplied model and `api_key="-"` clear operation in `lambda/handler_storage.py:2052-2075`.

Impact:

The CR25 reserved-partition fix closed accidental deletion through generic routes, but direct mutation remains. A caller with the app URL can change the model, clear a provider key, or choose a model/provider that causes Describe to use an existing stored key. Since Describe uses the configured key in `lambda/book_describe.py:450-460`, this is a quota/cost and operational-control problem even though `/fetch-vision-config` only exposes hints, not full secrets.

Fix:

Add a write/auth boundary for VisionModel config and any route that can spend stored provider keys. Reasonable minimum: require an operator token for `/save-vision-config` and Book Describe, store the token outside public S3, and reject config mutation without it. Stronger: keep API keys as Lambda environment or Secrets Manager values and let the UI select only a model/provider, not write secrets.

### F2 - HIGH: Describe CAS recovery still overwrites human edits to entries generated earlier in the same run

Evidence:

- `_save_book_cas` refetches the latest book on 409, then reapplies every generated entry in `run_prose` to the fresh document in `lambda/book_describe.py:367-373`.
- `run_prose` accumulates every successful generation in the run in `lambda/book_describe.py:480` and `lambda/book_describe.py:511`.
- The existing conflict test covers a human edit to an untouched future row, not a human edit to an already-generated row.

Reproducer shape:

- Describe generates entry `e1`, saves it, and records `run_prose["e1"]`.
- A user edits `e1` manually while Describe continues.
- A later save for `e2` hits a CAS conflict.
- `_save_book_cas` refetches the user's edited book, then blindly writes `run_prose["e1"]` back over the user's edit.

I verified the helper behavior directly: a refetched `e1` with `title_override="HUMAN"` is changed back to `title_override="GEN"` by `_save_book_cas`.

Related edge:

If an entry generation succeeds but its save fails, the generated prose remains in both `doc` and `run_prose` while `described` is decremented in `lambda/book_describe.py:520-525`. A later successful save for another entry can persist the earlier "failed" prose anyway, so status and persisted content diverge.

Fix:

Do not replay the whole run indiscriminately after a CAS conflict. Track per-entry base state at the time the prose was generated and only reapply if the refetched entry still matches that base or is blank. If the refetched entry has human-edited `title_override`/`body_override`, skip that entry and record a conflict. On save failure, remove that entry from `run_prose` or refetch/reset `doc` before continuing.

Tests to add:

- Human edits an already-generated row after its first save; a later CAS conflict must preserve the human edit.
- A save failure for entry `e1` followed by a successful `e2` save must not persist `e1` while reporting `e1` as failed.

### F3 - MEDIUM: Book/PDF provenance can mix an image from one artifact with metadata from another

Evidence:

- `_validate_book_payload` stores `job_id`, `artifact_id`, and `image_key` independently in `lambda/handler_storage.py:1946-1950`.
- It only validates that `image_key` is syntactically a render image key in `lambda/handler_storage.py:1956-1961`.
- I verified `_validate_book_payload` accepts `job_id="jobA"`, `artifact_id="artA"`, and `image_key="renders/jobB/color/artB/image.jpeg"`.
- `book_pdf.handle_prepare` downloads the image from `source_image_key` in `lambda/book_pdf.py:402`, but fetches calc/provenance from `source_job_id` in `lambda/book_pdf.py:413` and overlay meta from `source_job_id/source_artifact_id` in `lambda/book_pdf.py:427-435`.

Impact:

The UI sends consistent triples, but the API does not enforce consistency. A bad or hand-crafted book entry can produce a PDF where the page image comes from artifact B while the report, coefficient function, metrics, and QR/provenance say artifact A. That is misleading output, not TeX injection, but it damages the trustworthiness of the Book feature.

Fix:

Parse safe render image keys into a structured identity and validate it against the sibling fields. For immutable color keys, require `image_key == renders/<job_id>/color/<artifact_id>/image.*`. For legacy root color keys, require the explicit legacy artifact id convention. Alternatively, derive `job_id` and `artifact_id` from the validated key server-side and stop trusting the separate request fields.

Tests to add:

- `/save-book` rejects mismatched `job_id`/`artifact_id`/`image_key`.
- `book_pdf.handle_prepare` rejects mismatched `source_job_id`/`source_artifact_id`/`source_image_key`.

### F4 - MEDIUM: Describe's legacy fallback can send full-resolution renders to Vision

Evidence:

- `_entry_preview_bytes` tries `preview.jpg` / `preview.png`, then appends `entry.image_key` as fallback in `lambda/book_describe.py:411-425`.
- `entry.image_key` is the full render image key for Book/PDF use, not necessarily a thumbnail.
- `describe_image` sends those bytes directly to the provider in `lambda/book_describe.py:305-321`.

Impact:

This fixed "no preview" for legacy/root-shaped artifacts, but the fallback can be much larger than a 512 preview. That can increase latency, provider cost, request size, and failure rate during Describe. It also means legacy entries behave differently from current immutable color entries.

Fix:

Prefer a stored `preview_key` in book entries. If fallback to `image_key` is necessary, downscale locally before calling Vision. The PDF path already has image preparation utilities; Describe should not send arbitrarily large full-resolution renders when it only needs a thumbnail-level visual.

### F5 - MEDIUM: `lambda/fable_programs.py` has no automated gate

Evidence:

- `lambda/fable_programs.py --dry-run` passes all 12 parity checks today.
- There is no `tests/test_fable_programs.py` and no predeploy reference to the fable corpus.
- `scripts/predeploy_check.sh` has a completeness gate for `tests/test_*.py`, but that cannot catch a parity corpus that is only a script.

Impact:

The fables are exactly the kind of authored coefficient programs that will rot when the coefficient language, VM, or source compiler changes. Manual dry-run parity is useful, but it is not a deployment gate. A future compiler refactor can break a fable and still pass predeploy.

Fix:

Add `tests/test_fable_programs.py` with at least compile-only checks for all 12 sources, and preferably the same native parity checks currently run by `lambda/fable_programs.py --dry-run` if they are fast and deterministic on the build host. Add the new test file to `scripts/predeploy_check.sh`.

### F6 - LOW/MEDIUM: `--only` silently accepts unknown fable names and exits success with zero checks

Evidence:

- `lambda/fable_programs.py:398` filters `FABLES` with `if not args.only or n in args.only`.
- Running `.venv/bin/python lambda/fable_programs.py --only does_not_exist --dry-run` exits 0 and prints `all 0 passed parity`.

Impact:

A typo in a targeted dry-run or upload command looks like success while checking/uploading nothing. This matters because the fable corpus is currently script-gated manually rather than pytest-gated.

Fix:

Validate `--only` against `{name for name, _, _ in FABLES}`. If any requested names are unknown, exit nonzero and print the allowed names. Also reject an empty `picks` set.

### F7 - LOW/MEDIUM: Source zip font redistribution is fixed, but the Lambda image still packages trial fonts

Evidence:

- `source.zip` uses `_redistributable_source_fonts` and excludes `trial` / `demo` faces in `lambda/book_pdf.py:356-372` and `lambda/book_pdf.py:598-605`.
- The Docker image still copies every tracked TTF into `/opt/book-fonts` and TeX Live in `lambda/book_pdf.Dockerfile:79-83`.
- The Dockerfile explicitly requires `TiemposText-Regular-Trial.ttf` in `lambda/book_pdf.Dockerfile:80`.

Impact:

If the only concern was downloadable `source.zip`, this is fixed. If the trial license also restricts server/container redistribution or embedding in deployed infrastructure, the Lambda image still carries the same licensing risk.

Fix:

Decide the licensing policy explicitly. If trial fonts are allowed only for local/private use, keep them out of the deployed image and use redistributable substitutes there. If Lambda deployment is permitted by the font license, document that assumption next to the Dockerfile.

### F8 - LOW: Book PDF builds are more reproducible, but still not fully pinned

Evidence:

- TeX Live is now pinned to `TL_SNAPSHOT` in `lambda/book_pdf.Dockerfile:53-64`.
- Python dependencies remain moving ranges in `lambda/book_pdf.Dockerfile:86`: `boto3>=1.34`, `Pillow>=10,<12`, `reportlab>=4,<5`.
- The TeX installer tarball is downloaded from the snapshot URL without a checksum in `lambda/book_pdf.Dockerfile:54-60`.

Impact:

The biggest source of TeX drift is fixed. Rebuilding the image can still pick different Python package patch versions, and there is no integrity check on the downloaded installer tarball.

Fix:

Pin Python dependency versions or install from a locked requirements file. Add a checksum for the TeX installer tarball if you want supply-chain reproducibility rather than just behavior stability.

### F9 - LOW: Banned-word enforcement is one-pass only

Evidence:

- `describe_image` finds banned words after the first response in `lambda/book_describe.py:315`.
- If offenders exist, it asks for one rewrite in `lambda/book_describe.py:317-321`.
- It does not re-run `find_banned` on the rewritten title/body before returning.

Impact:

The "never use these words" rule is advisory rather than enforced. A rewritten response can still contain banned wording and be persisted.

Fix:

After the rewrite, run `find_banned` again. If offenders remain, either raise an entry-level failure so the next Describe run retries it, or do one final deterministic local rejection rather than saving the text.

### F10 - LOW/MEDIUM: Wall-pyramid mode depends on the 512px preview invariant but does not assert it

Evidence:

- Color manifests include each tile's actual `preview_width` / `preview_height` in `lambda/handler_storage.py:3503-3515`.
- The manifest can carry observed sizes through `sizes` / `size_counts` in `lambda/handler_storage.py:3600-3603` and `lambda/handler_storage.py:3647-3648`.
- Despite that, the same manifest still declares `"tile_size": 512` in `lambda/handler_storage.py:3646`.
- The wall builder hardcodes `CELL_PX = 512` in `lambda/handler_wall_pyramid.py:30`.
- `wall_dz.c` explicitly assumes "uniform 512px preview jpgs" in `lambda/wall_dz.c:13-15`, but `handler_wall_pyramid.py` downloads whatever `tile.key` points at in `lambda/handler_wall_pyramid.py:155-158`.
- The wall metadata reports `"cell_px": CELL_PX` in `lambda/handler_wall_pyramid.py:196`, and both the in-app and standalone viewers use that cell size for click mapping in `js/13-artifact-mosaics.js:650` and `artifact_mosaic_viewer.html:242`.
- The current wall test pins `wall["cell_px"] == 512` in `tests/test_wall_pyramid_handler.py:143`, so the bug is codified rather than covered.

Impact:

This is not a current mixed-size product bug if the preview migration completed and every wall tile now points at a normalized 512px preview. The earlier HIGH finding was overstated on that point. The remaining issue is an unasserted data invariant: the builder and viewer rely on 512px cells, but nothing in `handle_build_wall_pyramid` proves the manifest it received satisfies that contract. A future writer, bad repair, or stale object could reintroduce a non-512 tile and the wall path would silently produce wrong geometry/click mapping.

Fix:

Keep the 512px wall design, but make the invariant explicit. Before downloading tiles, verify every tile intended for the wall has `preview_width == preview_height == 512` or comes from a known normalized preview-JPG path. If any tile violates that, mark the wall as unsupported/error and let the UI fall back to the per-tile grid. This keeps the current fast wall path while preventing a silent geometry regression.

Tests to add:

- A wall manifest containing any non-512 tile must not produce `wall.json`; it should mark `wall_state` as unsupported/error and fall back to the grid.
- A normal post-migration manifest with all 512 previews still builds a wall with `cell_px=512`.
- The migration/repair path should have a regression test proving color wall tile keys/dimensions are normalized to 512.

### F11 - MEDIUM/HIGH: Generic `/delete` can wipe internal mosaic S3 state

Evidence:

- `handle_delete` deletes every object under `renders/{job_id}/` with no reserved-prefix guard in `lambda/handler_storage.py:4261-4292`.
- Mosaic manifests and wall pyramids live under `renders/_index/...`.
- Mosaic share snapshots live under `renders/_shared_mosaic/...`.
- `_list_mosaic_job_ids` explicitly treats underscore-prefixed render prefixes as internal/non-job prefixes in `lambda/handler_storage.py:3450-3452`, but the delete route does not use the same rule.
- CR25 fixed the DDB reserved partitions with `_assert_mutable_job_partition`, but that guard only protects job/status rows, not S3 pseudo-jobs.

Impact:

A call to `/delete` with `{"job_id":"_index"}` deletes every AllCol/AllPal manifest, wall pyramid, and wall image under `renders/_index/`. A call with `{"job_id":"_shared_mosaic"}` deletes standalone share snapshots. This is still within the app's documented unauthenticated/destructive API model, but it is not an ordinary render job delete; it destroys shared internal feature state.

Fix:

Add a render-prefix reservation guard for generic S3 deletion routes. At minimum reject job ids that are empty, start with `_`, or equal known internal render pseudo-jobs (`_index`, `_shared_mosaic`). Apply it to `handle_delete`, and consider applying it to any route that builds `renders/{job_id}/...` from caller input and deletes or overwrites objects.

Tests to add:

- `/delete` rejects `_index` and `_shared_mosaic`.
- `/delete` still accepts a normal compute id such as `compute_abc123`.

### F12 - MEDIUM: Book frontend continues after failed saves and mutates local state before save success

Evidence:

- `bookSave()` catches errors, reports status, and does not rethrow or return a success value in `js/14-book.js:327-345`.
- `bookCompile()` does `if (_bookState.dirty) await bookSave();` and then dispatches prepare jobs regardless of whether the save succeeded in `js/14-book.js:535-552`.
- `_bookDescribeRun()` has the same pattern before dispatching Describe in `js/14-book.js:742-759`.
- `bookEditSave()` expects `bookSave()` to throw, then closes the editor and reports "Entry text saved" after `await bookSave()` in `js/14-book.js:200-206`; the catch is ineffective because `bookSave()` swallowed the error.
- `_bookAddEntryImpl()` pushes a new entry into the local `doc.entries` before calling `/save-book` in `js/14-book.js:452-459`. If the save fails, the catch reports failure but does not roll back the pushed entry.

Impact:

The UI can dispatch Compile or Describe for a book state that was never persisted, or claim entry text was saved when it was not. A failed Add-to-Book can also leave a phantom local entry in memory. The next successful save/compile can accidentally publish that phantom, or the server can compile/describe the old S3 book while the UI believes it is working on the new local one.

Fix:

Make `bookSave()` return `true` on success and `false` on failure, or let it throw. Callers that require persisted state must abort if the save did not succeed or if `_bookState.dirty` remains true. For `_bookAddEntryImpl()`, build a candidate document first or push/rollback around the save failure.

Tests to add:

- If `/save-book` fails, `bookCompile()` and `_bookDescribeRun()` must not dispatch jobs.
- If `_bookAddEntryImpl()` save fails, the local document entry count must remain unchanged.
- `bookEditSave()` must not close the editor or show "saved" if `/save-book` failed.

### F13 - MEDIUM: Associated palette image keys are still trusted without render-key validation

Evidence:

- `book_pdf.handle_prepare` validates the main `source_image_key` in `lambda/book_pdf.py:383`.
- It then merges color overlay metadata into `src_meta` in `lambda/book_pdf.py:430-435`.
- It reads `associated_palette_image_key` from that metadata and downloads it directly with `s3.get_object` in `lambda/book_pdf.py:440-445`.
- The attach-palette worker accepts `associated_palette_image_key` from job parameters and writes it into the overlay metadata without using `assert_safe_render_image_key` in `lambda/handler_attach_palette_to_color.py:40-85`.

Impact:

The main Book image path is now pinned to render images, but the associated palette path is not. In normal generated flows this key is server-produced and safe. If a malformed overlay is created, Book PDF can download and process an arbitrary in-bucket key as the palette swatch. This is the same "one key validated, sibling key trusted" pattern as F3.

Fix:

Validate `associated_palette_image_key` when the overlay is written and again before Book PDF downloads it. It should be a render image key under the same job's palette/artifact namespace, or it should be omitted. If validation fails, skip the palette and record a clear provenance warning instead of silently swallowing it.

Tests to add:

- `handler_attach_palette_to_color` rejects an unsafe `associated_palette_image_key`.
- `book_pdf.handle_prepare` ignores or rejects unsafe associated palette keys in overlay metadata.

### F14 - LOW/MEDIUM: Share snapshot manifests keep the source `manifest_key`, not the snapshot key

Evidence:

- `handle_share_mosaic` copies the ready manifest with `snapshot_manifest = dict(manifest)` in `lambda/handler_storage.py:4136`.
- It adds `shared_at`, `share_id`, `share_kind`, and `source_manifest_key` in `lambda/handler_storage.py:4137-4142`.
- It does not replace `snapshot_manifest["manifest_key"]` with the actual snapshot key `renders/_shared_mosaic/<kind>/<share_id>/manifest.json`.
- The response returns `"manifest_key": snapshot_key` in `lambda/handler_storage.py:4163-4166`, so the response and the JSON object disagree.

Impact:

The current standalone viewer does not use `manifest_key`, so this is not a visible bug today. It is still a stale-pointer trap: any future tool that reads the snapshot manifest and follows `manifest_key` will jump back to the canonical `_index` manifest instead of staying on the immutable share snapshot.

Fix:

Set `snapshot_manifest["manifest_key"] = snapshot_key` and keep the original under `source_manifest_key`. Add a test asserting the stored JSON's `manifest_key` equals `share_key`.

### F15 - LOW: `/presign` injects raw caller-supplied filename into Content-Disposition

Evidence:

- `handle_presign` accepts `filename` from the request in `lambda/handler_storage.py:4640`.
- It writes it directly into `ResponseContentDisposition` as `attachment; filename="{filename}"` in `lambda/handler_storage.py:4641-4642`.

Impact:

Normal callers pass safe app-generated names. The public route accepts arbitrary strings, though, so quotes, semicolons, or control characters can create malformed `Content-Disposition` output in the presigned response. This is not S3 key exfiltration, but it is avoidable header/UX brittleness.

Fix:

Sanitize download filenames to a small safe charset, strip control characters, and use an RFC 5987 `filename*=` parameter for non-ASCII if needed. Add a unit test for quotes/newlines in `filename`.

## Non-Findings / Confirmed Good

- The CR25 F4 internal wall-pyramid route boundary is fixed for API Gateway events, and the wall builder validates the canonical manifest path before work starts.
- The new `_mosaic_refresh_id()` format matches the wall builder regex; the `mosaic_x` strings in tests only exercise payload formation, not the builder validation path.
- The standalone mosaic viewer no longer accepts `javascript:` or external-host manifest URLs; frontend checks cover this.
- `bash tests/test_frontend_js.sh` includes runtime checks for AllCol/AllPal share buttons and viewer URL validation.
- Predeploy now includes the Book storage/PDF/Describe, storage handler, DeepZoom export, and wall-pyramid suites reviewed here.
- `handler_dispatch.py` does not expose `attach_palette_to_color` as a direct dispatch target; the associated-palette key issue is still worth fixing, but it is not an arbitrary public attach route.

## Recommended Fix Order

1. Fix Describe CAS replay: preserve human edits to already-generated entries and remove failed entries from `run_prose` before continuing.
2. Enforce Book/PDF artifact identity consistency between `job_id`, `artifact_id`, and `image_key`.
3. Protect internal S3 pseudo-job prefixes (`_index`, `_shared_mosaic`) from generic render deletion.
4. Make Book save failure propagation explicit and abort Compile/Describe on failed saves.
5. Add an auth/operator boundary for VisionModel config writes and stored-key-consuming Describe operations.
6. Downscale Describe's full-image fallback or store/use a true `preview_key`.
7. Validate associated palette image keys on write and use.
8. Add and gate `tests/test_fable_programs.py`; fix `--only` typo handling at the same time.
9. Add a wall-pyramid invariant test that rejects non-512 manifest tiles, even though current migrated data should be 512.
10. Decide the Docker-image font licensing policy and document or adjust the image.
11. Re-run the focused suites plus full predeploy before deployment.

## Deployment Call

Private single-operator use is probably acceptable if you understand the remaining VisionModel route exposure. For any shared/public use, fix F1-F3 and F11-F12 first. Those are not cosmetic; they affect secret/config control, human edit preservation, provenance correctness, internal-state deletion, and whether Book compile/describe operate on saved data. F10 is now a guardrail, not a current deployment blocker, assuming the 512-preview migration is complete.
