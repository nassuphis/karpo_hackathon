# Code Review 28: Full Adversarial Audit

Reviewed commit: `975e716c5baf27263f17c58b93d9ab3400c101ac`

Review date: 2026-07-10

Scope: native coefficient/parameter execution, authored fables, sparse-fragment assembly, native HTTP readers, wall-pyramid generation and UI lifecycle, saved-program storage, Book/Describe concurrency, dispatch and frontend request handling, DeepZoom export, artifact identity, S3 failure handling, deployment reproducibility, and predeploy coverage.

The worktree changed concurrently while this review was written, so the report is pinned to the commit above. At the final check only the rebuilt `lambda/assemble_greyscale` binary was uncommitted; that binary is excluded. The committed C source and committed fables at `975e716` are the reproducible authority used for compilation and runtime probes.

## Verdict

The current local predeploy gate is green, but the code is not correctness-clean. The most important defects are below the level the existing tests exercise:

- `assemble_greyscale` has real C data races and a partial-thread-start use-after-free path.
- The native program VM accepts finite `double` outputs that overflow to non-finite `float` values at the wire boundary. Several committed fables exercise this at ordinary higher degrees.
- Book's advertised compare-and-swap is neither a unique revision nor an atomic compare-and-swap.
- The generic browser POST helper retries mutations after ambiguous failures and can duplicate dispatches and artifact-producing jobs.
- Dispatch reports rejected Lambda invocations as successfully fired.
- A caller-controlled DeepZoom export id is written into public executable HTML without validation or escaping.
- Saved-program seeding utilities bypass the v1/v2 invalidation rule, and the API implementation can also skip invalidation on a transient S3 HEAD error.
- A clean locked checkout cannot run predeploy because `reportlab` is not declared.

I would not deploy this revision as a fully reliable build. The happy paths are well covered and mostly work; the failures are concurrency, transport-boundary, retry, and ownership failures that happy-path tests cannot disprove.

## Verification

### Green checks

- `bash scripts/predeploy_check.sh`: `1059 passed`, `82 subtests passed`; frontend harness green.
- Focused current-HEAD suite for fables, Book, Describe, wall, and dispatch: `75 passed`.
- VM/oracle suite: `67 passed`, `45 subtests passed`.
- Frontend sequential/runtime harness: all checks passed (`23` scripts).
- `assemble_greyscale.c` and `sweep_cli.c` compile successfully from source.

### Reproductions that the green gate misses

- Running 64 overlapping fragment writers repeatedly produced many different final bytes for the same input. `tests/test_assemble_greyscale.py:167-175` explicitly accepts either winner.
- A loopback server returning HTTP 404 received six requests from `assemble_greyscale`; 404 is being retried as if it were transient.
- This valid Coeff Program returned success and wrote eight `inf` coefficients:

```text
fill(poly_len, 1e100)
poly = pop
emit
```

- At coefficient length 100, fables 7, 11, 12, 13, 17, 25, 28, and 32 emitted non-finite f32 coefficients at the fixed parity points.
- `fable-41` fails natively for every coefficient length below 23 because it writes fixed slots 5, 12, and 22. New fables 43 and 44 also fail at short lengths because their computed triplets use a negative `r1 - 1` index.
- RGB + RGBA input to `wall_dz` fails at `vips_arrayjoin` with mismatched band counts.
- Three Book saves performed in the same second reused the same `saved_at`; a stale third writer passed the check and overwrote the second writer.
- A clean `uv` environment failed all six `tests/test_spread_pdf.py` tests with `ModuleNotFoundError: reportlab`; the clean predeploy run had 29 failures for the same undeclared dependency.

### Full-suite qualification

A broad pytest run reached `1647 passed`, `9 skipped`, and `110 subtests passed`. Its 93 failures were environment-bound loopback/Docker tests blocked by the review sandbox, not accepted as code findings. Relevant loopback and libvips cases were rerun separately where needed. The full run used the pinned `e966cd7` snapshot because HEAD advanced during the review; later fables 40-44 were tested separately on current HEAD, including the multi-length probes above.

## Findings

### F1 - HIGH: `assemble_greyscale` has undefined, nondeterministic collision behavior

`process_fragment_bytes` writes directly into one shared output buffer with an unsynchronized `memcpy` at `lambda/assemble_greyscale.c:239-275`. Multiple workers process different fragments concurrently at `lambda/assemble_greyscale.c:299-307`.

Pixel overlap is not hypothetical. `roots2pix_mt` deduplicates through a bitmap inside one raster invocation, but separate section/chunk invocations produce separate fragments and separate bitmaps. Two fragments can therefore contain the same global pixel. The assembler header declares "any arrival wins" (`lambda/assemble_greyscale.c:12`), and the test accepts either result (`tests/test_assemble_greyscale.py:167-175`).

This is still invalid C concurrency:

- Concurrent writes to the same byte are a data race and undefined behavior.
- A multi-channel `memcpy` can theoretically expose a record assembled from different writers.
- The final raw image and its histogram are nondeterministic for identical inputs.
- Output depends on download and scheduler timing, so a retry or worker-count change can alter an artifact.

There is a second race on `AssembleState.failed`: `set_error` writes it under `err_mu` at lines 68-74, while `next_job` reads it under a different mutex at lines 77-84. The flag needs one synchronization discipline or an atomic type.

Fix: define deterministic collision semantics. A sensible implementation is to associate each write with the source-fragment ordinal and atomically claim a winner, then copy the complete record for that winner. Alternatively, build per-worker sparse outputs and merge in source order. Use the same lock/atomic for `failed` reads and writes. Tests must assert one exact output for overlapping scalar and multi-channel fragments across repeated runs and worker counts.

### F2 - HIGH: Partial thread creation failure frees live worker state

The thread-start loop at `lambda/assemble_greyscale.c:481-493` immediately frees `ctxs`, `threads`, `buf`, and every input path when `pthread_create` fails. It does not join threads that were successfully created earlier in the loop. Those live threads continue dereferencing the freed `WorkerCtx`, `AssembleState`, buffer, and path strings.

This is a use-after-free path during thread/resource pressure. A large or caller-influenced worker count makes the failure more plausible.

Fix: track `created`, set a shared stop flag on failure, join all created threads, and only then free state. Cap workers to `min(requested, n_paths, configured_max)`. Add a test seam around thread creation so failure after the first successful start can be exercised under ASan.

### F3 - HIGH: The native VM can write non-finite f32 from a successful finite-double evaluation

`evalCoeffProgram` validates stack shape and vector length but not representability of every final component as f32 (`lambda/sweep_cli.c:5544-5638`). Multiple output paths cast directly:

- coefficient generation: `lambda/sweep_cli.c:8501-8505`, `9634-9640`, and `9853-9857`
- parameter generation: `lambda/sweep_cli.c:8756-8759`
- root output: `lambda/sweep_cli.c:10235-10239` and `10488-10493`

A finite `double` such as `1e100` passes VM checks and becomes f32 infinity. The native process exits successfully and publishes a coefficient file containing `inf`. Companion solving may turn such rows into zero roots and count them as skipped overflow (`lambda/companion_solver.h:27-80`), while other solver paths may fail differently. Either result is a silent semantic change after successful compilation/execution.

Fix: centralize checked f32 packing. Before every transport write, require `isfinite(value)` and `fabs(value) <= FLT_MAX`; return a structured failure naming the step, coefficient/register, and value. Decide explicitly whether subnormal underflow is accepted. Apply one helper to coeff, param, and root outputs. Add direct boundary tests at `FLT_MAX`, just above it, infinity, NaN, and a valid large finite value.

### F4 - HIGH: The fable corpus claims degree independence but validates one length

`lambda/fable_programs.py:14` says one program serves any degree, but `N_PARITY = 36` at line 30 and `tests/test_fable_programs.py` tests only that length.

Current counterexamples:

- `fable-41` writes fixed indices 5, 12, and 22 (`lambda/fable_programs.py:1119-1127`) and fails for every coefficient length below 23.
- `fable-43` uses `r1 - 1` (`lambda/fable_programs.py:1177-1189`) and fails below length 4; `fable-44` repeats the pattern twice (`lambda/fable_programs.py:1209-1223`) and fails below length 5.
- At length 71, fables 17 and 28 emit non-finite values.
- At length 100, fables 7, 11, 12, 13, 17, 25, 28, and 32 emit non-finite values.
- At lengths 128 and 256, the affected set grows further.

The parity helper intentionally excludes rows outside f32 range (`lambda/port_poly100_programs.py:2030-2033`), so a formula can become unusable while the single-N gate remains green. This is not merely a test gap: the product accepts coefficient lengths from 1 up to thousands, and no per-fable degree contract is surfaced.

Fix: either make every fable genuinely degree-agnostic or add declared `min_coeffs`/`max_coeffs` metadata enforced by the selector and compiler. Gate representative lengths, including the minimum, 23/24, 36, 64, 100, and the supported maximum. Require every native output component to be finite, independently of parity filtering.

### F5 - HIGH: Book `expected_saved_at` is not a compare-and-swap

Book revisions use `_utc_now_iso()` with one-second resolution (`lambda/handler_storage.py:269-270`), and every validation generates that value at `lambda/handler_storage.py:1989`. `handle_save_book` then performs a separate read/compare at lines 2104-2125 followed by an unconditional `put_object` at lines 2126-2132.

There are two independent correctness defects:

- Multiple saves in one second share a token, so stale writes pass.
- Even with a unique token, two concurrent writers can both read the same revision, both pass, and both write. The second silently wins.

The regression test says the token advances but does not assert it (`tests/test_book_storage.py:193-206`), which is why same-second reuse remains green.

Fix: use an actual atomic condition. Options are an S3 conditional write against the current ETag/version or a DDB book revision updated with `ConditionExpression`. Return an opaque revision to clients; do not use a display timestamp as ownership. Add same-second and true two-writer barrier tests where exactly one writer succeeds.

### F6 - HIGH: Generic frontend POST retries can duplicate mutations

`lambdaPost` in `js/02-preview-solvescore.js:610-652` retries every POST on network errors, 429, and 503 without knowing whether the operation is idempotent. A lost response is ambiguous: the server may have completed the mutation before the browser retries.

Concrete unsafe callers include:

- `/dispatch-render`, which can invoke the same worker/orchestrator jobs twice
- `/share-mosaic`, which creates a new immutable snapshot and random share id per call
- Book prepare/describe dispatches
- render/palette postprocessing and artifact-producing routes
- saved-program and Book mutations

Duplicate async jobs reuse task ids and output keys, so they can race status and artifact writes. The helper also has no per-attempt timeout, sleeps after the fifth and final 429/503 response, and reports "5 retries" although it made five attempts.

Fix: make retry policy operation-specific. Reads may retry. Mutations must either not retry after ambiguous failure or carry a server-enforced idempotency key whose completed response can be replayed. Add request ids to dispatch/start/share operations and persist dedupe state. Add an AbortController timeout and test response-loss-after-commit.

### F7 - HIGH: Dispatch counts rejected invocations as fired and accepts unbounded malformed jobs

`handler_dispatch.py:91-96` appends a non-202 response to `non_202` and then increments `fired` anyway. Tests explicitly bless a 429 as `fired == 1` (`tests/test_dispatch_resilience.py:295-305`). Most frontend callers check only `fired`, so they proceed to poll jobs Lambda rejected.

The handler also reads `params["jobs"]` without checking that it is an array, that each item is an object, or that the array has a safe maximum (`lambda/handler_dispatch.py:68-87`). A dict is iterated as keys; string jobs can be asynchronously sent to workers and still reported as fired if Invoke returns 202. The public route can request an arbitrary fan-out.

Fix: `fired` must mean accepted HTTP 202 only. Return `attempted`, `accepted`, `rejected`, and `failed`, with complete counts even if details are truncated. Validate per-target job schemas and enforce a configured batch cap before creating the thread pool. Frontend callers should reject any nonzero rejected/failed count.

### F8 - MEDIUM/HIGH: Saved-program v1/v2 lifecycle is still bypassed and error-prone

Fetch prefers migrated v2 objects (`lambda/handler_storage.py:715-716`, `752-753`, `782-783`). API saves try to remove stale v2 objects, but two paths defeat that rule:

1. `lambda/port_poly100_programs.py:2083-2094`, used by the fable uploader, writes the v1 Coeff Program object directly and never deletes or rewrites v2.
2. `scripts/seed_solve_score_programs.py:62-84` does the same for Solve Score programs.

After a program has been migrated, `--force` can therefore report a successful overwrite while fetch continues serving the stale v2 body.

The central API is also brittle. `_drop_stale_program_v2_key` first calls `_key_exists`, but `_key_exists` converts every HEAD exception into `False` (`lambda/handler_storage.py:4260-4266`). A transient 503 on the v2 HEAD skips deletion, the save returns 200, and the stale v2 object shadows the newly saved v1 object. Delete and migration decisions use the same helper.

Fix: expose one storage write/delete primitive and make scripts call it. Deleting an S3 key is idempotent, so stale v2 deletion should not be preceded by a lossy existence probe. More robustly, replace two competing keys with one authoritative generation pointer. Add tests for force-update-after-migration and transient HEAD failure.

### F9 - HIGH: Unvalidated DeepZoom `export_id` creates public stored HTML injection and malformed prefixes

`handle_deepzoom_export_request` accepts `export_id` directly at `lambda/handler_deepzoom_export.py:128-134`. It is used in the S3 prefix at line 183 and substituted raw into HTML by `_render_viewer` at lines 30-36. The template places it in `<title>` and a body `<span>` (`lambda/deepzoom_viewer_template.html:6,26`).

A crafted public `/deepzoom-export` request can therefore publish attacker-controlled HTML/JavaScript under the application's public bucket origin. Slash-containing ids can also create prefix shapes that the normal list/delete UI does not understand.

Fix: call `assert_safe_id(export_id, "export_id")` before status or S3 work, validate `job_id` explicitly, and HTML-escape every template substitution as defense in depth. Add hostile `</span><script>...` and slash-id tests plus a create/list/delete round trip.

### F10 - HIGH: A clean checkout cannot reproduce the green predeploy gate

`pyproject.toml` and `uv.lock` do not declare `reportlab`. Gated PDF tests import `lambda/spread_pdf.py`, which requires it. The local `.venv` contains ReportLab out of band, so current predeploy passes; a fresh `uv` environment fails.

This is release-gate integrity, not a developer convenience. A green result depends on undeclared machine state.

Fix: declare and lock ReportLab in the project or in a locked test/development dependency group used by predeploy. Make predeploy synchronize from the lock in a clean environment in CI. Do not rely on the Dockerfile's separate `pip install`, which does not populate the local test environment.

### F11 - MEDIUM/HIGH: Describe rollback erases pre-existing prose in overwrite mode

After a generated entry fails to save, `book_describe.py:564-572` removes it from `run_prose` and sets its title/body to empty strings. In overwrite mode the entry may have contained old prose before generation. The rollback should restore that exact prior state, not blank it.

A later successful save for another entry can persist the blanked first entry. The run reports the first entry as failed while also destroying its previous text.

Related conflict behavior remains lossy: `_save_book_cas` reapplies generated prose only when the refetched row is blank or already equals the generated text (`lambda/book_describe.py:363-390`). In overwrite mode, an unrelated concurrent edit can leave the selected row equal to its original nonblank base; the generated replacement is then silently dropped.

Fix: capture each entry's base title/body before mutation. On save failure, restore the base. On conflict, apply generated prose only when the latest row still equals that base; if it differs, record an explicit per-entry conflict. Tests need old-prose rollback and unrelated-edit overwrite cases.

### F12 - MEDIUM/HIGH: Render identity validation uses substring coincidence, not a parsed identity

`assert_render_identity` verifies only a job prefix and the presence of `/<artifact_id>/` anywhere in the key (`lambda/shared.py:307-320`). It does not parse `renders/<job>/<family>/<artifact>/<leaf>`.

Verified false accepts include:

- artifact id `color` for `renders/jobA/color/real-art/image.jpeg`
- artifact id `jobA` for the same key
- artifact id `palettes` for `renders/jobA/palettes/real-pal/image.jpeg`

The associated-palette check repeats the same substring pattern at `lambda/handler_attach_palette_to_color.py:73-85`; declaring `palette_id="palettes"` can satisfy the check for an unrelated palette path. DeepZoom raw sidecars are tied only to the job prefix at `lambda/handler_deepzoom_export.py:144-149`, not to the source artifact.

Fix: implement one structured render-key parser returning job, family, artifact id, and leaf. Compare exact components and permitted leaves; represent legacy root-shaped keys as explicit variants. Reuse it in Book/PDF, derived workers, associated palettes, favorites, and DeepZoom sidecars.

### F13 - MEDIUM: Canonical S3 existence/read helpers still turn transient failure into absence

The codebase has a recurring failure-taxonomy bug:

- `_key_exists` catches every exception and returns false (`lambda/handler_storage.py:4260-4266`).
- `_head_artifact_keys` catches every exception and emits `exists: false` (`lambda/handler_storage.py:4862-4899`).
- `_load_color_artifact_overlay` returns `None` for every GET error (`lambda/handler_storage.py:2829-2840`).
- `load_color_artifact_head` treats every candidate HEAD error as "try the next key" (`lambda/color_artifact_meta.py:94-120`).

A throttled HEAD, AccessDenied, malformed metadata conversion, and NoSuchKey become indistinguishable. Render summaries can omit real artifacts, mosaic refresh can publish an incomplete manifest, and program lifecycle operations can skip required cleanup.

Fix: use one shared `is_missing_s3_error` policy. Only 404/NoSuchKey means absent. Retry or propagate 429/5xx/transport failures; treat AccessDenied as a hard configuration error. Where fail-soft behavior is intentional, surface per-reason error counts instead of silently relabeling errors as missing.

### F14 - MEDIUM: Wall refresh can publish an all-placeholder wall as ready

`_download_tile` catches every exception and returns a placeholder (`lambda/handler_wall_pyramid.py:70-78`). The builder counts placeholders but always runs the compositor, writes `wall.json`, and sets `wall_state="ready"` (`lambda/handler_wall_pyramid.py:166-225`).

If credentials, bucket access, key layout, or networking fails systemically, a wall containing no real artifact can be advertised as ready. The frontend's ready message does not show the placeholder count.

Fix: classify missing vs transient/systemic errors, retry transient downloads, and abort when every tile fails or a configured failure ratio is exceeded. Store reason counts in wall status and display them. Test all-fail, AccessDenied, throttling, and one-missing-tile behavior.

### F15 - MEDIUM: `wall_dz` does not actually normalize RGBA to RGB

At `lambda/wall_dz.c:75-86`, any image whose band count is not three is passed through `vips_colourspace(..., sRGB)`. Libvips preserves alpha, so RGBA remains four bands. `vips_arrayjoin` then fails when mixed with RGB images.

The current migrated preview inventory is 512px and normally uses JPEG, so this is not a claim that current previews are mixed-size or currently RGBA. It is a real failure in the fallback/normalization contract the code advertises for odd or legacy inputs. The Docker gate covers 500px grayscale JPEG, not RGBA.

Fix: flatten alpha against an explicitly chosen background, then convert/extract exactly three bands. Add mixed RGB, grayscale, RGBA, and CMYK cases to the real libvips gate.

### F16 - MEDIUM: Native retry predicates retry permanent HTTP errors and have no deadlines

`ag_retryable_failure` first recognizes selected transient statuses, then returns true for `CURLE_HTTP_RETURNED_ERROR` (`lambda/assemble_greyscale.c:125-135`). With `CURLOPT_FAILONERROR`, any HTTP 4xx/5xx uses that curl code, so a permanent 403/404 is retried. A 404 reproduction made all six attempts.

The copied predicate appears in:

- `lambda/multispan_reader.c:405-414`
- `lambda/solve_palette_chunk_mt.c:110-119`
- `lambda/solve_proximity_hist_sectioned.c:297-306`

None of these downloaders configures connect, total-transfer, or low-speed timeouts. Retry count therefore does not bound wall time. `assemble_greyscale` also reports the constant maximum attempt count even when a non-retryable failure exits earlier (`lambda/assemble_greyscale.c:184-191`).

Fix: when an HTTP status is present, base retry solely on the explicit status allowlist. Use curl-code retry rules only for status-zero transport failures. Add connect/transfer/low-speed deadlines, jittered backoff, and actual-attempt diagnostics. Gate 403/404 single-attempt and timeout cases for all shared implementations.

### F17 - MEDIUM: AllPal "Add to Book" invents a source artifact id from the palette id

`js/13-artifact-mosaics.js:976-989` assumes a palette id is `pal_<color_artifact_id>` and strips the prefix. Current generated ids are `pal_<metric>_<interpretation>_<digest>` (`lambda/handler_palette_render_plan.py:238-243`), so the inferred color id is usually nonexistent.

Palette metadata already stores the authoritative `derived_from_color_artifact_id` (`lambda/handler_palette_finalize.py:612-614`). `_palette_entry_from_prefix` preserves arbitrary metadata, but the mosaic tile projection omits this field at `lambda/handler_storage.py:3744-3755`.

Fix: include `derived_from_color_artifact_id` in the AllPal manifest and consume that exact field. Do not infer ownership from display ids. Test extracted, associated, standalone, repalette, and missing-source palettes.

### F18 - MEDIUM: A transient ready-wall fetch leaves the UI permanently on the slow grid

`_maybeLoadMosaicWall` catches a `wall.json` fetch error and only sets `state.wall = null` (`js/13-artifact-mosaics.js:503-516`). `_scheduleMosaicWallPoll` runs only while `wall_state == "computing"` and stops after 40 eight-second polls (`js/13-artifact-mosaics.js:611-635`).

If status is already `ready` but `wall.json` has one transient fetch failure, no retry is scheduled. The UI logs that the wall is ready while retaining the per-tile grid until tab re-entry. The 320-second poll horizon is also shorter than the wall Lambda's 900-second timeout.

Fix: retry ready-status metadata fetches while the wall is missing, with bounded backoff and visible diagnostics. Align the overall deadline with the worker timeout or poll until a terminal state. Test a first-fetch failure followed by success and a build lasting longer than 320 seconds.

### F19 - LOW/MEDIUM: Optional flat JPEG makes the entire wall fail above about 16,129 tiles

`wall_dz` writes `wall.jpg` before generating the DZI and treats JPEG failure as fatal (`lambda/wall_dz.c:121-143`). With 512px cells and `ceil(sqrt(count))` columns, 16,130 tiles require 128 columns, or 65,536px, beyond the JPEG dimension limit cited by the code. The DZI itself is intended for images larger than that, but it is never generated because the optional flat export aborts first.

Fix: enforce a documented count limit, omit/replace the flat JPEG above the limit, or generate a bounded-resolution contact-sheet download separately. DZI generation must not depend on an optional flat artifact. Add a geometry-only threshold test without allocating the full wall.

### F20 - LOW/MEDIUM: Gate completeness is bookkeeping, not load-bearing coverage

There are 153 Python test files: 83 are predeploy-gated and 70 are explicitly allowed to remain ungated. Two currently load-bearing examples are still in `ALLOWED_UNGATED`:

- `test_assemble_greyscale.py`, which is the only test of the new native retry implementation
- `test_attach_palette_to_color_handler.py`, which is the only direct gate for the palette identity hardening

The wall Python test mocks the subprocess, and `scripts/test-wall-dz-docker.sh` is manual. The meta-test proves every file is listed in one of two sets; it cannot tell when a historically excluded file becomes release-critical.

Fix: remove newly load-bearing suites from the allowlist and add their fast portions to predeploy. Split environment-heavy tests from fast contract tests instead of excluding whole files. Add the real mixed-format wall gate to deploy/CI.

### F21 - LOW: AllCol still offers an obsolete fixed 1024 filter

The preview migration is complete; current previews are normalized to 512. This review therefore does not repeat the old mixed-size-preview finding. The remaining issue is stale UI configuration: `ARTIFACT_MOSAICS.color.fixedSizes` is still `['512', '1024']` (`js/13-artifact-mosaics.js:7-14`), and fixed sizes override manifest-observed sizes at lines 335-345.

Selecting 1024 after a fresh manifest produces an empty view for no useful reason.

Fix: derive Color size options from `manifest.sizes`, or set the fixed list to 512 while retaining backend tolerance for old objects.

## Known Accepted Risks Still Open

These are not rediscovered defects, but they remain material deployment constraints documented in `code-review-27.md`:

- The S3 bucket is public-read and the API has no authorizer; `deploy.sh:125-128` explicitly accepts destructive public routes for a single-user art stack.
- `/save-vision-config` can mutate global provider/model configuration and stored keys.
- `/dispatch-render` exposes caller-selected fan-out, and several internal worker phases remain routable publicly.
- Generic storage helpers such as `/save-metadata`, `/list-prefix`, and `/head-keys` remain public. `/head-keys` also lacks a key-array schema and batch cap.
- Favorites still persist caller-provided image/preview references rather than canonical server-resolved artifact identity.

Do not share this stack URL with untrusted users until those assumptions change. API-manifest drift checks verify that routes match the manifest; they do not enforce an exposure/authentication policy.

## Non-Findings And Corrections

- Preview-size migration: current previews are 512px. There is no current "mixed 512/1024 previews break the wall" finding. `wall_dz` normalization is defense in depth; F15 concerns band count, not dimensions.
- Current predeploy is genuinely green in the existing machine environment. F10 is specifically clean-environment reproducibility.
- The VM wire/opcode/oracle gates pass. F3 is a missing finite-f32 transport invariant that those gates do not currently assert.
- Mosaic refresh ownership, refresh-scoped keys, and current/previous manifest pruning remain structurally sound in the reviewed paths.
- The frontend mosaic controller keeps Color and Palette state/viewers separate and its left/right-click tile mapping uses stored wall geometry correctly.
- Program v2 fingerprints no longer include raw source spelling; the earlier source-spelling cache split is fixed.

## Recommended Fix Order

1. Fix F1/F2 together: deterministic assembly, one synchronization discipline, and safe partial thread startup.
2. Fix F3/F4: checked f32 transport, supported-degree contracts, and multi-length fable gates.
3. Replace Book's timestamp/read-then-write pseudo-CAS (F5), then repair overwrite rollback/merge (F11).
4. Stop unsafe mutation retries and make dispatch truthful/idempotent (F6/F7).
5. Validate and escape DeepZoom export identity before another public export (F9).
6. Restore clean-checkout predeploy reproducibility (F10).
7. Centralize saved-program writes and missing-vs-error S3 handling (F8/F13).
8. Harden wall failure classification, image normalization, polling, and large-wall behavior (F14-F16, F18-F19).
9. Replace substring identity checks with one parser and fix AllPal provenance (F12/F17).
10. Tighten load-bearing gates and remove stale UI state (F20/F21).

## Deployment Call

No for a reliability-sensitive deployment. The current gate proves a large happy-path surface, but it does not catch the reproducible nondeterministic assembler, f32 overflow, false CAS, duplicate mutation, rejected-dispatch, or public HTML-injection paths. The documented single-user/public-stack security risk is a separate accepted constraint; it does not make the correctness findings acceptable.
