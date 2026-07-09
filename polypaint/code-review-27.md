# Code Review 27: Post-CR26 Public Surface And Data-Integrity Pass

Reviewed head: `b55db2a`

Reviewed at: `2026-07-09`

Scope reviewed: deployed HTTP routes, generated API/config manifests, storage mutation routes, dispatch fan-out, directly exposed worker handlers, derived-artifact source-key handling, favorites, vision config, Results/Favorites/Render frontend rendering, Book hardening boundaries, and the tests that currently bless or miss these paths. I did not re-open the VM/compiler bug family except where public worker routes expose those workers.

## Verdict

The previous CR27 draft was too narrow. The bigger problem is not one bad helper route; it is that a deliberately unauthenticated app exposes several admin-grade and worker-grade operations as ordinary browser APIs.

The stack explicitly documents the no-authorizer model in `deploy.sh:125-128`, so "anyone with the URL is admin" is an accepted baseline. Even under that baseline, the public surface still needs blast-radius constraints. Right now a caller with the URL can overwrite global vision API-key configuration, fan out arbitrary Lambda jobs, call worker phases that accept caller-controlled S3 input/output keys, preserve stale debug routes, and inject stored metadata into unescaped UI tables.

For a strictly private single-user URL, this is not automatically a deploy stop. For any shared app URL, F1-F6 are blockers.

## Verification Run

- `uv run python -m pytest tests/test_storage_handler.py tests/test_pdf_artifact_handler.py tests/test_api_route_contracts.py tests/test_dispatch_resilience.py -q`
- Result: `87 passed in 2.85s`
- `uv run python api_manifest.py --check`
- Result: `api_manifest.json: OK`
- `uv run python deploy_manifest.py --check`
- Result: `deploy_manifest.json: OK (42 functions, 70 routes)`
- `bash tests/test_frontend_js.sh`
- Result: frontend fused/help/jobs/mosaic/book/share checks passed.

Important limitation: these green gates do not cover the route-policy questions below. In several places the tests currently bless the unsafe behavior, for example dispatching 200 public jobs in `tests/test_dispatch_resilience.py:307-318`.

## Findings

### F1 - CRITICAL: Public `/save-vision-config` can overwrite or clear global provider API keys

Evidence:

- The storage API publishes `/fetch-vision-config` and `/save-vision-config` in `deploy_manifest.json:152-153`.
- `handle_save_vision_config` accepts `model` and `api_key`, then writes provider keys into DynamoDB in `lambda/handler_storage.py:2057-2096`.
- The key is not returned in full, but a caller can replace it or clear it with `api_key="-"` in `lambda/handler_storage.py:2082-2085`.
- The Book UI calls the same route from the public frontend in `js/14-book.js:713-730`.

Impact:

Anyone with the app URL can break Book Describe for everyone by changing the model or clearing/replacing the stored provider key. This is a global configuration mutation, not a per-job artifact action.

Fix:

Do not expose `/save-vision-config` as a normal unauthenticated app route. Prefer local deployment-time configuration or an admin-only token/header for this route. If the no-auth model is kept, require an explicit admin capability separate from the public app URL and never include the save endpoint in shareable configs.

Tests to add:

- Public storage route policy test: `/save-vision-config` is absent from the browser config or rejects requests without the admin capability.
- `/fetch-vision-config` may remain read-only, but it must only expose `key_set`/hint, never full secrets.

### F2 - HIGH: Public dispatch has unbounded caller-selected fan-out

Evidence:

- `/dispatch-render` is public in `deploy_manifest.json:169-170`.
- `handler_dispatch.FUNCTIONS` exposes many expensive/internal targets in `lambda/handler_dispatch.py:22-42`.
- The handler trusts caller `target` and `jobs`, with no type check or max length in `lambda/handler_dispatch.py:68-87`.
- It invokes all jobs through a 50-thread pool in `lambda/handler_dispatch.py:44` and `lambda/handler_dispatch.py:85-87`.
- `tests/test_dispatch_resilience.py:307-318` explicitly verifies that a 200-job public dispatch fires 200 Lambda invokes.

Impact:

A single request can trigger a large async Lambda burst. Reserved concurrency on the dispatch Lambda limits the dispatcher, not the downstream functions it invokes. The caller also chooses targets such as orchestrators, postprocessors, `book_pdf`, `pdf_artifact`, `resize_artifact`, and `deepzoom_export`.

Fix:

Add a dispatch contract per target. Reject non-list `jobs`; cap `len(jobs)` by target; require each target's schema and safe key shapes before invoking; remove internal-only targets from public dispatch or require a server-issued plan token from the relevant planner route.

Tests to add:

- Dispatch rejects missing/non-list/oversized `jobs`.
- Dispatch rejects targets not explicitly public.
- Dispatch target schemas reject caller-supplied keys outside the target's allowed job/artifact scope.

### F3 - HIGH: Direct public worker routes expose unsafe phases with caller S3 read/write keys

Evidence:

- The deployed config exposes direct services including `coeffgen`, `sweep-mt`, `sweep-cm`, `solve_proximity`, `palette-debug`, `deepzoom-export`, `png-export`, and `tiff-compat` in `api_manifest.json`.
- `/coeffgen` is public in `deploy_manifest.json:45-46`; the UI only needs `phase: "degree_probe"` for popup probing in `js/05-render-popups.js:1167-1181`.
- The same handler also exposes `param_gen`, `coeffgen_chunked`, and `legacy_coeffgen` in `lambda/handler_coeffgen.py:129-143`.
- `handle_param_gen` writes caller `params_key` in `lambda/handler_coeffgen.py:157-160` and `lambda/handler_coeffgen.py:243-247`.
- `handle_coeffgen_chunked` reads caller `params_key` and writes caller/default `s3_key` in `lambda/handler_coeffgen.py:351-420` and `lambda/handler_coeffgen.py:468-472`.
- `handle_legacy_coeffgen` writes caller/default `s3_key` in `lambda/handler_coeffgen.py:514-587`.
- `/sweep-mt` and `/sweep-cm` are public in `deploy_manifest.json:26-27` and `deploy_manifest.json:64-65`; they read caller `coeffs_key` and write caller/default `s3_key` in `lambda/handler_sweep_mt.py:35-100` and `lambda/handler_sweep_cm.py:32-101`.
- `/solve-proximity` is public in `deploy_manifest.json:540`; the UI uses a synchronous summary call in `js/10-status-results.js:658`, but the handler also exposes `clip`, `hist`, and `merge` phases in `lambda/handler_solve_proximity.py:564-581`.
- `handle_clip` reads caller `lores_bin_key`/`lores_coeffs_key`/`lores_params_key` and writes caller `out_key` in `lambda/handler_solve_proximity.py:776-958`.
- `handle_hist` reads caller `bin_key`/`coeffs_key`/`params_key`/`clip_key` and writes caller `out_key` in `lambda/handler_solve_proximity.py:980-1013` and `lambda/handler_solve_proximity.py:1490-1495`.
- `/palette-debug` is deployed and in config (`deploy_manifest.json:572`, `api_manifest.json:161`), but I found no frontend caller. Its `persistent` mode reads caller `lores_bin_key` and writes saved palette artifacts under `renders/{job_id}/palettes/...` in `lambda/handler_palette_debug.py:86-123` and `lambda/handler_palette_debug.py:170-276`.

Impact:

These routes make internal worker phases callable as public APIs. Some phases write request-selected S3 keys; others write under request-selected job ids. This is separate from "the user can run the app": these are lower-level worker contracts that assume a trusted planner/orchestrator produced the payload.

Fix:

For public direct routes, expose only the safe frontend phase:

- `/coeffgen`: public API should allow `degree_probe` only. Move `param_gen`, `coeffgen_chunked`, and `legacy_coeffgen` behind dispatch/Step Functions or require an internal invocation marker that API Gateway events cannot set.
- `/solve-proximity`: public API should allow `summary` only. `clip`/`hist`/`merge` should be dispatch-only with planner-generated keys.
- Remove `/palette-debug` from deploy/config if unused. If needed, make it non-persistent only and write only under a debug prefix with TTL/cleanup.
- `/sweep-mt` and `/sweep-cm` should not be direct public routes unless they validate input/output keys against a server-generated compute plan.

Tests to add:

- Public API Gateway events for worker-only phases return 403/400.
- Existing internal dispatch/Step Functions invocations still reach worker phases.
- Generated `api_manifest.json` does not include dead debug services.

### F4 - HIGH: Public storage helpers expose generic S3 introspection and stale mutation primitives

Evidence:

- `deploy_manifest.json:131-146` publishes `/save-metadata`, `/list-prefix`, `/head-keys`, and `/delete-prefix`.
- The generated frontend storage-path list does not include `/save-metadata`, `/list-prefix`, or `/head-keys`; `/delete-prefix` is used only by DeepZoom delete.
- `handle_list_prefix` accepts arbitrary caller prefix/suffix/delimiter and returns matching keys in `lambda/handler_storage.py:4305-4343`.
- `handle_head_keys` accepts arbitrary `keys` and optional `presign` in `lambda/handler_storage.py:4902-4914`; `_head_artifact_keys` can return presigned URLs.
- `handle_delete_prefix` only checks `prefix.startswith("deepzoom/")` before deleting in `lambda/handler_storage.py:5095-5125`.
- `handle_save_metadata` writes caller-supplied metadata to `renders/{job_id}/calc.json` in `lambda/handler_storage.py:4625-4638`.
- The current compute workflow finalizes metadata through `handler_compute_plan`, not storage `/save-metadata`, in `stepfunctions/compute_workflow.asl.json.template:651-667`.
- `tests/test_dispatch_resilience.py:156-258` treats `/head-keys` as a supported public batch HEAD endpoint, so tests currently protect that surface.

Impact:

These are generic bucket tools published through the app API. `/head-keys?presign=true` is broader than the safer `/presign` route, which now restricts keys to `renders/` and `polypaint/books/` in `lambda/handler_storage.py:4654-4667`. `/save-metadata` can forge `calc.json`, poisoning Results and Populate. `/delete-prefix` can wipe `deepzoom/` or an entire job's DeepZoom exports through direct API calls.

Fix:

Remove truly dead routes from `deploy_manifest.json` and the storage dispatcher. For routes still needed, replace `/head-keys` with a render-artifact-only batch HEAD API, remove or narrow `/list-prefix`, make `/delete-prefix` require exactly `deepzoom/<job_id>/<export_id>/`, and remove or internal-gate `/save-metadata`.

Tests to add:

- `/head-keys` rejects `index.html`, `config.json`, `_index` manifests, and any key outside allowed render/book artifacts.
- `/list-prefix` rejects blank/root/internal prefixes.
- `/delete-prefix` rejects `deepzoom/` and `deepzoom/<job_id>/`.
- `/save-metadata` is absent from deployed routes or rejects public API events.

### F5 - HIGH: Derived-artifact workers still trust caller-supplied source keys

Evidence:

- `handler_pdf_artifact.handler` reads `source_image_key`, HEADs it, then downloads it before enforcing `assert_safe_render_image_key` or `assert_render_identity` in `lambda/handler_pdf_artifact.py:416-456`.
- `handler_autolevels.handler` downloads caller `source_image_key` before loading/cross-checking the declared source artifact in `lambda/handler_autolevels.py:201-233`.
- `handler_resize_artifact.handler` accepts and downloads caller `source_image_key` in `lambda/handler_resize_artifact.py:334-394`.
- `handler_deepzoom_export.handle_deepzoom_export_request` downloads caller `source_key` without requiring declared job/family/artifact identity in `lambda/handler_deepzoom_export.py:127-148`.
- `handler_png_export` and `handler_tiff_compat` accept caller `source_key`; if the HEAD fails, they proceed with `{}` metadata and still download the key in `lambda/handler_png_export.py:40-79` and `lambda/handler_tiff_compat.py:40-79`.
- The newer Book prepare path is not vulnerable to this specific issue: it calls `assert_safe_render_image_key` and `assert_render_identity` before download in `lambda/book_pdf.py:376-396`.

Impact:

A worker can be pointed at bytes from a different job/artifact while writing provenance that says it derived from the selected artifact. In the worst cases this becomes a same-bucket read/transform/publish primitive. In less severe cases it corrupts provenance in PDFs, DeepZoom exports, and postprocessed render artifacts.

Fix:

Centralize artifact-source validation and call it before every S3 `head_object`/`get_object` on a user-supplied source key. For color-derived actions, derive the image key server-side from `{job_id, artifact_id}` using `load_color_artifact_head`, or require exact equality with that canonical key. Add family-specific validators for bilevel, palette, coeff, PDF, and legacy root render shapes rather than using substring checks.

Tests to add:

- PDF, Autolevels, Resize, DeepZoom, PNG export, and TIFF compat reject a source key from a different job before any S3 GET.
- Missing S3 metadata must not downgrade validation to "assume OK".
- Existing legitimate legacy-color flows still pass with explicit legacy shapes.

### F6 - HIGH: Stored S3/DDB metadata can XSS Results, Favorites, and Render artifact tables

Evidence:

- Results rows use `tr.innerHTML` with unescaped `r.job_id` and `r.function` from `calc.json` in `js/01-core-compute.js:820-834`.
- Favorites build table/viewer HTML from stored favorite fields without escaping in `js/01-core-compute.js:527-553`.
- `/add-favorite` stores caller `display_name`, `image_key`, and `preview_key` directly in DynamoDB in `lambda/handler_storage.py:1469-1489`.
- Render artifact rows insert `_renderArtifactSummary(art)` unescaped in `js/11-artifacts.js:1068-1080`; that summary includes stored metadata such as palette names, artifact ids, source color ids, and solve displays from `js/11-artifacts.js:346-375`.
- I checked the broad `innerHTML` grep: saved-program modals, Book rows, artifact mosaics, DeepZoom inventory, and jobs rail mostly use `_escapeHtml` or mosaic-specific escaping. This finding is not a blanket claim against every `innerHTML`.

Impact:

Stored metadata can become executable HTML when the app renders Results, Favorites, or Render artifacts. With the current public metadata/favorite routes, this is a real stored-XSS path for anyone who can write or poison S3/DDB metadata.

Fix:

Replace these tables with DOM construction and `textContent`, or escape every interpolated value with `_escapeHtml`. Avoid inline `onclick` strings; attach listeners after row creation. Also remove `/save-metadata` and canonicalize favorites so arbitrary caller strings cannot become trusted stored fields.

Tests to add:

- Results render `<img onerror=...>` in `function` as text.
- Favorites render hostile `display_name`, `image_key`, and `missing_reason` as text.
- Render artifact summaries escape palette/artifact metadata.

### F7 - MEDIUM/HIGH: Favorites are references to Color artifacts, but the server stores caller-provided keys

Evidence:

- `handle_add_favorite` accepts `job_id`, `artifact_id`, `display_name`, `image_key`, and `preview_key`, then stores them directly in `lambda/handler_storage.py:1469-1489`.
- The frontend sends those values from local artifact/tile state in `js/01-core-compute.js:363-377`.
- `_hydrateFavoriteArtifacts` falls back to stored favorite keys if `/render-summary` cannot find the artifact, preserving `image_key` and `preview_key` on a missing favorite in `js/01-core-compute.js:324-356`.
- Favorites downloads use `art.image_key` through `/presign` in `js/01-core-compute.js:658-719`.

Impact:

Favorites can become dangling or misleading records. They also feed F6 because display/key fields are stored and later rendered. The route should store "favorite this canonical Color artifact", not "store these caller-provided keys".

Fix:

On `/add-favorite`, call `load_color_artifact_head(job_id, artifact_id)`, require the artifact to exist, and store canonical server-derived `image_key`, `preview_key`, display metadata, dimensions, and size. Reject mismatched caller hints or ignore them entirely.

Tests to add:

- `/add-favorite` rejects missing artifacts.
- `/add-favorite` rejects or ignores mismatched caller image/preview keys.
- `/list-favorites` returns canonical keys from real Color artifacts.

### F8 - MEDIUM: Associated palette metadata is safe-key checked but not identity checked

Evidence:

- `handler_attach_palette_to_color` checks `associated_palette_image_key` and `associated_palette_preview_key` with `assert_safe_render_image_key` in `lambda/handler_attach_palette_to_color.py:64-72`.
- It does not require those keys to match the declared `associated_palette_id`, `job_id`, or palette prefix before writing the color overlay in `lambda/handler_attach_palette_to_color.py:87-116`.
- `handler_pdf_artifact` downloads `associated_palette_image_key` from color metadata in `lambda/handler_pdf_artifact.py:458-470`.
- `book_pdf.handle_prepare` re-checks associated palette key syntax before embedding a swatch, but syntax is not identity in `lambda/book_pdf.py:451-475`.

Impact:

The arbitrary-key problem is mostly blocked, but provenance can still be wrong. A Color artifact can claim palette id X while embedding a swatch from job/artifact Y. That flows into ColorSpread PDF and Book PDF output.

Fix:

Add a palette identity validator. For normal same-job palettes, require `renders/<job_id>/palettes/<associated_palette_id>/image.jpeg` and `renders/<job_id>/palettes/<associated_palette_id>/preview.png`. If cross-job/dependency palettes are intentionally allowed, store and display that dependency identity explicitly.

Tests to add:

- Attach rejects palette keys from another job.
- Attach rejects keys whose path id differs from `associated_palette_id`.
- PDF/Book prepare drop or reject malformed existing overlays rather than embedding wrong swatches.

### F9 - MEDIUM: Orchestrator run ids are used raw in Step Functions names and task ids

Evidence:

- `handler_compute_orchestrator.handler` builds `task_id = f"compute_run_{solver_mode}_{run_id}"` and `execution_name = f"compute_{solver_mode}_{run_id}"` from caller `run_id` in `lambda/handler_compute_orchestrator.py:79-126`.
- `handler_render_orchestrator.handler` builds `task_id = f"render_run_{mode}_{run_id}"` and `execution_name = f"render_{mode}_{run_id}"` from caller `run_id` in `lambda/handler_render_orchestrator.py:81-119`.
- `handler_palette_orchestrator.handler` starts Step Functions with `name=f"palette_{run_id}"` and writes task ids from caller/given `run_id`/`task_id` in `lambda/handler_palette_orchestrator.py:23-63`.

Impact:

Bad `run_id` values can produce Step Functions `InvalidName` errors, inconsistent DDB task ids, or path-like strings passed into workflow outputs. This is mostly reliability and diagnostics, but the public dispatch route can invoke these orchestrators directly.

Fix:

Add a shared validator for `run_id`, `task_id`, `job_id`, `artifact_id`, and export ids wherever they feed Step Functions names, DDB partition/sort keys, or S3 prefixes. Use a narrow pattern such as `[A-Za-z0-9_-]{1,64}` for run ids and task ids. Preserve existing `compute_*` job ids but reject slash/whitespace/control characters.

Tests to add:

- Compute/render/palette orchestrators reject `run_id` containing `/`, whitespace, `:`, control chars, or excessive length before `start_execution`.
- Valid frontend-generated ids still pass.

### F10 - MEDIUM: API manifest checks contract shape, not route exposure policy

Evidence:

- `api_manifest.py` records frontend literal services, frontend storage paths, backend storage routes, dispatch targets, deploy config services, and deploy routes in `api_manifest.py:212-245`.
- `api_manifest.json` correctly shows that some deployed/config services are not frontend literal calls, for example `palette-debug`, `sweep-mt`, and `sweep-cm`.
- The manifest check passes even though dead/admin/debug surfaces remain in deploy/config.

Impact:

The manifest gate prevents accidental drift, but it does not answer "should this route be public?" A route can be unused by the frontend and still be deployed forever. This is how `/palette-debug`, `/save-metadata`, `/head-keys`, and broad worker phases survive with green checks.

Fix:

Extend the manifest model with route classes:

- `public_frontend`: directly used by the browser and safe for public API events.
- `public_readonly`: share/viewer read-only APIs.
- `internal_worker`: invokable only through dispatch/Step Functions or direct Lambda-to-Lambda events.
- `admin`: not emitted in public config without an explicit admin capability.
- `retired`: must not be routed.

Then add a predeploy assertion that every deployed route has a class and that browser config does not expose `internal_worker`, `admin`, or `retired` endpoints.

### F11 - LOW/MEDIUM: DeepZoom delete accepts broader prefixes than the UI intends

Evidence:

- The UI sends `deepzoom/${ex.job_id}/${ex.export_id}/` in `js/12-deepzoom-boot.js:208-223`.
- `handle_delete_prefix` accepts any prefix under `deepzoom/` in `lambda/handler_storage.py:5095-5125`.

Impact:

A direct API caller can delete all DeepZoom exports (`deepzoom/`) or all exports for a job (`deepzoom/<job>/`). The deploy comment says destructive routes are public by design, but this backend route should still enforce the narrower UI operation it implements.

Fix:

Require exactly `deepzoom/<job_id>/<export_id>/` with safe components and reject broader prefixes.

Tests to add:

- Reject `deepzoom/`, `deepzoom/<job>/`, and malformed components.
- Accept a valid exact export prefix.

## Corrections From The Hasty Draft

- Book storage and `book_pdf.prepare` are not examples of the source-key bug. They already validate safe render keys and identity before download.
- Most saved-program modal, Book, mosaic, DeepZoom inventory, and jobs-rail `innerHTML` paths are escaped. The live XSS finding is specific to Results, Favorites, and Render artifact tables/viewers.
- The no-authorizer model is deliberate. The bug is not "there is no auth" in the abstract; the bug is that admin/global/internal-worker operations are not separately constrained under that model.
- `/palette-debug` should be called out explicitly: it is a deployed/configured debug service with persistent writes and no current frontend caller found.

## Recommended Fix Order

1. Remove or admin-gate `/save-vision-config`; do not expose global provider-key writes through the ordinary public app config.
2. Add dispatch caps and target schemas; remove internal-only targets from public dispatch.
3. Restrict direct public worker routes to safe frontend phases (`coeffgen=degree_probe`, `solve_proximity=summary`) and retire `/palette-debug` if unused.
4. Remove or narrow raw storage helper routes: `/save-metadata`, `/head-keys`, `/list-prefix`, `/delete-prefix`.
5. Add centralized source-key identity validation for PDF, Autolevels, Resize, DeepZoom, PNG export, and TIFF compat before any S3 read.
6. Replace unsafe Results/Favorites/Render table rendering with DOM/textContent or comprehensive escaping.
7. Canonicalize `/add-favorite` server-side from real Color artifact metadata.
8. Add associated-palette identity checks.
9. Normalize run/task/job/artifact ids at orchestrator and worker boundaries.
10. Extend `api_manifest`/predeploy with route exposure classes so dead/admin/internal routes cannot remain public by accident.

## Deployment Call

Private single-user URL: deployable if the URL is treated as an admin secret, but F1 is still dangerous because it affects stored provider keys.

Shared app URL: not deployable as-is. F1-F6 expose global config mutation, worker fan-out, caller-key worker phases, generic storage primitives, provenance corruption, and stored XSS.
