# Code Review 17 — Duplicate & Dead Code

Reviewed the working tree at `0c0075a` initially, then re-swept current `HEAD` at `40b9c05` after the CR16 follow-ups and document commits. Scope is deliberately narrow: **duplicate code that can be factored out** and **dead code**. Not correctness, security, or resource limits — those are CR16's beat.

Methodology: parallel read-only sweeps across the four language surfaces (JS parts, Python handlers/modules, native C, deploy/tests), each required to distinguish *statically unreferenced* from *dynamically reachable*. I then verified every dead-code claim and the headline orphan/duplication counts myself, because this repo has bitten reviewers here before (the CR5 "typeof guards are dead" finding was wrong — those guards are load-bearing). Each dead symbol below was traced through JS call sites, HTML `onclick` strings, the `api_manifest.py` source parser, Playwright `page.evaluate`, Python importers, dynamic dispatch tables, the deploy.sh packaging blocks, and `#include` graphs before being classified.

## Verdict

The codebase is in good shape on both axes — the big dedup passes already landed (`_chipPickers`, `_str`/`_pluralize`/`_escapeHtml`, `solve_score.h`/`root_xforms.h` centralization, `pipeline_programs.py`, the deploy.sh libcurl/libvips/spec-list unification). What CR17 finds is the *remainder*: a clean, low-risk set of dead symbols and files that can simply be deleted, plus a ranked list of genuine factor-out opportunities. None of it is a blocker; all of it is optional cleanup. The dead code is free to remove (no behavior change); the duplication is worth attacking top-down by payoff and stopping when the marginal items stop being worth it.

Two themes dominate the duplication: (1) **per-binary C boilerplate** (CLI arg parsing, the sectioned-curl download driver) copied across 14 and 3 translation units respectively — the single highest-LOC target, ~400-550 lines; and (2) **three-way structural triplication** that appears in three different places (the JS palette popups, the deploy.sh packaging blocks, two pairs of near-identical Python handlers).

## Implementation Status

Implemented in the current working tree:

- Removed the 15 verified-dead JS symbols.
- Removed the verified def-only Python helpers.
- Moved root visual scripts and lambda-local manual visual/transpiler tools under `test-visual/`, with path assumptions updated so they can run from their new locations.
- Moved `preview_selection.py` / `ops_xfrm.py` out of `lambda/` into `test-visual/prototypes/`, and updated the `ops_xfrm.md` link.
- Removed the orphaned tracked native/generated files and stale tracked binaries listed in the dead-code section.
- Added `lambda/program_compile_helpers.py` and rewired coeffgen, compute-plan, compute-preview, and param-debug payload/macro helper duplication through it; deploy packaging and packaging tests now include the helper.
- Replaced the remaining hardcoded storage presign `ExpiresIn=3600` with `shared.PRESIGN_EXPIRY`.

Still not implemented here: D1/D2 C helper extraction, D3 palette-popup descriptor refactor, D4 deploy packaging manifest refactor, D5/D8-D15 broader helper extractions. Those are behavior-preserving refactors but large enough to land as separate focused changes.

---

## DEAD / NON-RUNTIME CODE (verified — delete or relocate only as classified)

Each item below was classified by actual reachability. Items marked **dead** were confirmed unreferenced: no static caller, not named in any `onclick`/HTML string, not parsed by `api_manifest.py`, not used by a test or generator, not dynamically dispatched. Items marked **manual** are useful historical/dev tools, but they do not belong in active runtime/package directories or as root-level pytest hazards.

### JS — 15 dead functions/consts in `js/`

All fifteen have exactly one occurrence in tracked frontend/test source (the definition). Verified by token-counting across `js/`, `index.html`, and `tests/`, then with `git grep` for the three public-looking wrapper names. The control symbol `setSolveMetric` correctly came back live via `tests/e2e/render-solve-score.spec.js`, proving the discrimination works.

| Symbol | Location | Note |
|---|---|---|
| `setPaletteMetric(name)` | `js/04-palette-color.js:916` | dead twin of the live `setSolveMetric` |
| `_palettePreviewCss(name)` | `js/04-palette-color.js:546` | |
| `_renderAllPaletteRows()` | `js/04-palette-color.js:639` | callee `_renderPaletteRow` stays live |
| `_solveScoreOmegaEnabled(prefix)` | `js/04-palette-color.js:648` | omega now edited via chips |
| `_setSolveScoreOmegaEnabled(prefix,enabled)` | `js/04-palette-color.js:658` | "" |
| `_hasParamProgramChain()` | `js/08-chip-editors.js:681` | `_serializeParamProgramChain` stays live |
| `_displayParamProgramEntry(item)` | `js/08-chip-editors.js:844` | |
| `_serializeNamedChain(chain)` | `js/02-preview-solvescore.js:726` | |
| `_normalizePixelBinFragmentMode(value)` | `js/05-render-popups.js:742` | state key `pixel_bin_fragment_mode` appears nowhere — orphan normalizer for a never-wired feature |
| `_normalizeRasterBinGroupSize(value)` | `js/05-render-popups.js:748` | state key `raster_bin_group_size` appears nowhere — same |
| `_fmtStatus` (arrow const) | `js/09-render-orchestration.js:476` | sibling `_fmtMs` has 10 uses |
| `_solveScoreSourceChoices` (array const) | `js/07-transform-catalogs.js:356` | sibling `_solveScoreGenericSourceChoices` is used |
| `openComputeMtPopup()` | `js/05-render-popups.js:1227` | dead compatibility wrapper; `openComputeSolverPopup(...)` is the live entrypoint |
| `runPaletteDebug()` | `js/10-status-results.js:837` | dead alias to `runRenderPaletteArtifact()` |
| `openDeepZoomViewer(dziUrl)` | `js/11-artifacts.js:1406` | dead wrapper; current DeepZoom UI calls other entrypoints |

**Reachability traps confirmed NOT dead (do not remove):** `_solverLoresEndpoint`/`_solverDispatchTarget` (`js/01-core-compute.js:1061,1066`) have no JS caller but `api_manifest.py:142-160` regex-parses their bodies out of the concatenated frontend source — deleting them breaks `tests/test_api_route_contracts.py`. `_ptCatalogEnriched` (`js/07-transform-catalogs.js:91`) is a side-effecting `.every()` that mutates `_ptCatalog` at load — the const binding is vestigial but the expression is load-bearing.

### Python — dead helpers and non-Lambda dev/manual scripts

- **[dead] `shared.s3_get_object()`** — `lambda/shared.py:23`. Public, packaged into nearly every zip via `shared.py`, but a whole-repo search finds only the `def` line. A leftover from the contextual-S3-errors commit whose intended callers were never wired. Safe to delete (or wire up, but nothing uses it).
- **[dead] def-only helper functions** — these have no static callers and no dynamic dispatch path found by `git grep`; delete unless a near-term caller is intentionally added in the same cleanup:
  - `handler_palette_render_plan._artifact_meta_omega_enabled()` — `lambda/handler_palette_render_plan.py:344`; quantile twin is live, omega-enabled twin is not.
  - `handler_pdf_artifact._body_from()` — `lambda/handler_pdf_artifact.py:108`; PDF handler does not call it.
  - `handler_solve_proximity._reject_lagged_unsupported_phase()` — `lambda/handler_solve_proximity.py:398`; lag handling is now inline at the two actual decision sites.
  - `logical_sections._source_segments_from_chunk_items()` — `lambda/logical_sections.py:214`; no caller after CR16 manifest-source refactor.
  - `palette_names.is_valid_palette_name()` — `lambda/palette_names.py:21`; callers import/use `VALID_PALETTE_NAMES` directly.
  - `param_program_chain._finite_complex()` — `lambda/param_program_chain.py:353`; expression migration left it orphaned.
  - `param_program_chain.param_program_chain_id()` — `lambda/param_program_chain.py:1255`; public convenience wrapper, but no caller.
  - `solve_score_chain._quantile_percent_to_fraction()` — `lambda/solve_score_chain.py:327`; no caller.
  - `solve_score_chain.solve_score_uses_lag()` — `lambda/solve_score_chain.py:706`; callers inspect `compiled["uses_lag"]` or use `solve_score_program_spec_uses_lag(...)`.
- **[dead as Lambda surface] `lambda/preview_selection.py`** — entire file. Zero importers, not packaged by deploy.sh, imports `numba`/`PIL` (no Lambda layer has them) and shells out to a local `./sweep_test`. A dev comparison script living in `lambda/`. Move to `scripts/` or delete.
- **[dead as Lambda surface] `lambda/ops_xfrm.py`** — entire file. Not packaged, no Python importer (only design-doc mentions), imports `numba`. Design prototype. Same disposition.
- **[manual transpiler/visual scripts living under `lambda/`]** — not Lambda surface and not packaged, but useful as manual tools. Move to `test-visual/` or `tools/transpiler/` and update their `lambda/` path assumptions:
  - `lambda/test_poly_accuracy.py` — duplicate of the maintained `tests/test_poly_accuracy.py` path; docs mention the old lambda location, so update docs if moved.
  - `lambda/test_transpiler_all.py`
  - `lambda/test_visual_30.py`
  - `lambda/test_visual_251_275.py`
  - `lambda/test_visual_376_400.py`
  - `lambda/test_visual_401_425.py`
  - `lambda/visual_cmp_301_325.py`
  - `lambda/visual_cmp_351_375.py`
  - `lambda/visual_compare_226_250.py`
  - `lambda/visual_compare_326_350.py`
  - `lambda/visual_compare_476_500.py`

*(`workflow_contracts.py` looks orphaned — not packaged — but is consumed at build time by `workflow_template_render.py` and tests. Live. `lambda/giga.py` and `lambda/ops_poly.py` are also live source material for generated catalogs and visual tests, even though they are not packaged Lambda modules. Do not delete them without moving/updating those generator/test references.)*

### Native C — 2 orphaned `.c`, ~1.48 MB orphaned generated source, stale binaries

- **[orphaned] `lambda/solver_cli.c` and `lambda/lores_viewport.c`** — neither is compiled by deploy.sh (the sole build authority; verified `grep -c` = 0 for both). Both carry stale copies of `solveEA`/`cmpDouble`. The on-disk `solver`/`lores_viewport*` executables are stale artifacts.
- **[orphaned] 4 generated `poly_generated` sources — ~1.48 MB** — `sweep_cli.c` `#include`s only the `_200`/`_400`/`_500`/`_600`/`_700`/`_800`/`_900` variants plus `g_generated.c`/`giga_generated.c`. These four are `#include`d by nothing (verified `includes=0`; control `_500` = 1):
  - `poly_generated.c` (460 KB)
  - `poly_generated_100.c` (397 KB)
  - `poly_generated_300.c` (621 KB) — plus its unreferenced `poly_generated_300_funcs.h`/`_lookups.h`
  - `poly_generated_1000.c` (95 B)

  Biggest single cleanup in the repo by bytes. Confirm none are referenced by a generator before deleting, then remove.
- **[orphaned, keep] `lambda/coeff_program_bench.c`** — `#include`s `sweep_cli.c`; an out-of-build dev benchmark, not dead logic. Leave it (optionally move under a `bench/` dir).
- **[build cruft] stale binaries with no source** — `roots2pix`, `bilevel`, `imgpipe`, `param_gen`, `pixassemble`, `pixbinassemble`, `solver` exist as executables in `lambda/` with no `.c` and no build rule (the live equivalents are `roots2pix_mt`, the `*_bilevel_*`/`bilevel_merge` set, and `sweep`). Safe to delete; they're working-tree clutter, not source dead-code.

### Repo root / tests — orphaned scripts (one is a test-collection hazard)

- **[manual visual tools + pytest hazard] 8 root-level visual-comparison scripts** — `compare_131_145.py`, `compare_191_200.py`, `visual_compare.py`, `visual_compare_276_300.py`, `visual_compare_426_450.py`, `test_poly_visual_201_225.py`, `test_visual_161_175.py`, `test_visual_451_475.py`. All unreferenced, all superseded for normal coverage by the consolidated `tests/test_visual_*.py` suite, but they remain useful as historical/manual visual debugging tools. **Requirement:** move all eight out of the repo root into a dedicated `test-visual/` directory, preserving their current runnable behavior and updating any path assumptions needed for the new location. Do **not** leave root-level `test_*.py` files. **Hazard:** the three named `test_*.py` currently sit at the repo *root*; `pyproject.toml` sets no `testpaths` and there's no root `conftest.py`, so `tests/conftest.py:6`'s `collect_ignore_glob` does **not** cover them. A bare `pytest` from the root would try to collect them and run their expensive import-time loops. Moving them to `test-visual/` preserves the manual tools while removing the pytest footgun.
- **[dead] one-off scripts** — `scripts/seed_solve_score_programs.py` (S3 bootstrap, no caller/test), `scripts/run_solve_hist_lambda_bench.py` (manual bench, no caller/test), `scripts/gemini_make_description.py` (5-line alias re-exporting `main` from `gemini_generate_title_description`). Keep-or-cut is a judgment call for the manual ops tools; the alias is trivially redundant.
- **[stale artifacts] tracked PDFs** — `architecture-ui-critique.pdf`, `architecture-ui-critique-2.pdf` are git-tracked build outputs; the current generator writes to `reports/pdf/` (untracked). Committing the output while gitignoring its inputs is inconsistent. Low value.

---

## DUPLICATE CODE (factor-out opportunities, ranked by payoff)

Line estimates are the sweep agents' figures, sanity-checked against the cited spans. "Payoff" = lines removed × drift-risk eliminated.

### Tier 1 — high payoff, low risk

**D1. [duplicate] C `getArg*` CLI-parser family — 14 binaries (~250-300 lines).** Identical `getArg`/`getArgInt`/`getArgDouble`/`getArgStr`/`getArgLongLong` in 14 translation units (`solve_palette_chunk_mt.c:46`, `roots2pix_mt.c:106`, `solve_proximity_hist_sectioned.c:32`, `solve_proximity_stats.c:44`, `solve_palette_chunk.c:33`, `solve_palette_debug.c:35`, `bilevel_merge.c:21`, `bilevel_section_raster.c:26`, `coeffs_bilevel_raster.c:29`, `assemble_greyscale.c:52`, `palette_bins_render.c:26`, `raw2jpeg.c:27`, `score_raw_render.c:30`, `raw_to_bilevel.c:18`). `autolevels_render.c:92` has the same logic under different names. **Fix:** header-only `cli_args.h` with `static inline` (keeps every consumer dep-free — they're static/standalone). Fold `option_matches`/`reject_unknown_options` (4 files) into the same header.

**D2. [duplicate] C sectioned-range curl download driver — 3 binaries (~150-250 lines).** The richest cluster and the highest *quality* win (it kills 3-way drift in retry/backoff policy). `DownloadBuffer`, `write_section_cb`, `sleep_ms`, `retryable_range_failure`, the 10-line `curl_easy_setopt` block, and the dual-section retry loop with its error strings are copied across `multispan_reader.c`, `solve_palette_chunk_mt.c`, and `solve_proximity_hist_sectioned.c`. The two consumers already `#include "multispan_reader.h"` and link `multispan_reader.c`. **Fix:** promote the helpers into `multispan_reader.h`/`.c` and reuse. Fold `clamp_threads` (3 files) in too.

**D3. [duplicate] JS palette-popup triplication — `js/04-palette-color.js` (~250-300 lines).** Even after `_bindPopupShell` was extracted, five function clusters remain fully triplicated across builtin/tri/long, differing only by an id prefix, a state object, and a few accessor names: `_render{Tri,Long,Builtin}PalettePopup` (984/1075/1166), `_open*` (1055/1146/1231), `_apply*Filter` (1069/1160/1244), `_close*` (951/962/973), `_init*` (1285/1316/1347). **Fix:** a per-type descriptor table `{prefix, state, visibleCatalog, catalogAvailable, activeNameFor, setFor, title, emptyMsg, showAliases, btnId}` driving generic `_renderPalettePopup(desc)` etc. — exactly the `_chipPickers` pattern already used in `js/08`. All three families confirmed at full parity. Highest-payoff JS target.

**D4. [duplicate] deploy.sh Lambda-packaging blocks — ~30 blocks (~250-300 of 409 lines).** `deploy.sh:949-1357` repeats the same 6-line shape (`rm -rf`/`mkdir`/`cp .py`/`cp bins`/`chmod`/`zip -FS`/`du`) ~30 times (40 `zip -FS` lines total). Only with/without `/lib` and with/without binaries vary. The precedent `package_render_plan_zip` (`deploy.sh:292`) already exists for the one render-plan case. **Fix:** a `package_lambda <dir> <zip> <label> --py "…" --bin "…" [--libdir …]` helper, or — cleaner — add `pys`/`bins`/`libdir` fields to `deploy_manifest.json` and emit these blocks from `deploy_manifest.py` the way the spec list already is. (Note: `deploy_manifest.json` currently carries no packaging file lists — verified.)

**D5. [duplicate] Python `handler_png_export.py` ⇄ `handler_tiff_compat.py` — ~170 lines.** `diff` shows the two diverge only in binary name, extension/keys, `format` value, label strings, and two `bigtiff` fields; the entire head-validate→download→subprocess→preview→meta→upload→presign pipeline is identical. **Fix:** a shared `_run_bilevel_postprocess(binary, ext, content_type, labels, …)` in a small helper module.

**D6. [duplicate] Python compute-program compile helpers — 4 handlers (~50 lines + drift risk).** `_compiled_coeff_program_payload` and `_compiled_param_program_payload` are repeated in `handler_coeffgen.py:47/61`, `handler_compute_plan.py:49/63`, `handler_compute_preview.py:99/113`, and `handler_param_debug.py:30` (param only). The macro-resolver closures are also triplicated in `handler_coeffgen.py:112/120`, `handler_compute_plan.py:131/139`, and `handler_compute_preview.py:164/172`. This is the same precedence/macro-resolution surface that already caused bugs. **Fix:** extend `pipeline_programs.py` or add `program_compile_helpers.py` with shared `compiled_param_program_payload(...)`, `compiled_coeff_program_payload(...)`, and macro-resolver factories.

### Tier 2 — medium payoff

**D7. [duplicate] JS single-job dispatch executors — `js/05-render-popups.js` (~90-110 lines).** `_executeRepalette` (133), color_repalette (242), color_to_bilevel (336), pdf_color_spread (1349) share the identical btn-disable → `lambdaPost('dispatch',{target,jobs:[job]})` → fired-check → `_saveActiveRun` → status → close → `startActiveRenderObserver` scaffold; only the guard, per-job payload, label, target, mode, and close-fn vary. **Fix:** `_runSingleJobDispatch({btnId, label, mode, guard, buildJob, onClose})`.

**D8. [duplicate] Python status-writer cluster — `handler_compute_status.py` ⇄ `handler_render_status.py` (~90 lines).** `_put_row` (172/165), `_extract_error_message` (109/129, identical but comments), and the `queued`/`phase`/`done`/`error` dispatch are byte-identical. **Fix:** a shared `status_writer.py`.

**D9. [duplicate] Python small-helper redundancy — consolidate into `shared.py` (~80 lines total).**
- `_utc_now_iso()` defined in **13** files (two impls, byte-identical output).
- `_phase(job_id, task_id, status, phase, phase_label, **extra)` copied in **7** files — identical.
- `_get_ddb()` reimplemented in 3 files that *already* import `shared` (where `shared._get_ddb` exists) — pure redundancy.
- JSON/error response helper (`_json_response`/`error_response`/`_error_response`) in **5+** sites — generalize `shared.ok_response` into `shared.json_response(status_code, body)`.

**D10. [duplicate] C JSON mini-parser — `sweep_cli.c` ⇄ `sweep_mt.c` (~50 lines).** `skip`/`parseNum`/`findKey`/`parseBool`/`parseString` byte-identical across the two static binaries. **Fix:** header-only `json_mini.h`. (Leave `sweep_cli.c`'s extra `findKeyIn`; `parseStringArray` is currently dead.)

**D11. [duplicate] C `solveEA` kernel — 2 live binaries (~70 lines live).** Identical kernel + `seedEAInitialGuess` + `MAX_ITER`/`TOL2` macros in `sweep_cli.c:50` and `sweep_mt.c:37` (and in the two orphaned `.c` from the dead-code section — deleting those removes two more copies for free). **Fix:** header-only `solver_ea.h`. *This is the EA solver, not the param/coeff VMs — those stay separate.*

**D12. [duplicate] JS `pt` picker never migrated to `_chipPickers` — `js/08-chip-editors.js:44-99` (~35-45 lines).** `_renderParamTransformAddPopup` is byte-identical to the generic `_renderChipPickerPopup` modulo hardcoded strings and `selectParamTransformChip` vs `picker.selectFnName`. `addChip('pt', …)` already supports it. **Fix:** add a `pt:` entry to `_chipPickers` and reduce these to thin wrappers — keeping the public `toggleParamTransformPicker`/`selectParamTransformChip` names (live via HTML).

**D13. [duplicate] Python render/raster helper logic.** `_viewport_bounds` is identical in `handler_bilevel.py:132` and `handler_raster_mt.py:94`; `_download_range` is identical in `handler_palette_chunk.py:66` and `handler_solve_proximity.py:87`; `_sectioned_input_size_limit` is identical in `handler_palette_chunk.py:160` and `handler_solve_proximity.py:651`; `_write_url_manifest` is identical in `handler_bilevel.py:289` and `handler_finalize_mt.py:141`. **Fix:** move these into `render_geometry.py`, `range_download.py`, or `logical_sections.py` depending on ownership.

**D14. [duplicate] palette-associated-artifact helpers.** `_associated_palette_display_name` appears in `color_recolor_raw.py:154`, `handler_extract_palette_from_step_scores.py:200`, and `handler_render_plan.py:111`; `_associated_palette_ref` / `_associated_palette_exists` are duplicated between `handler_extract_palette_from_step_scores.py:114/127` and `handler_palette_render_plan.py:311/324`. **Fix:** a small `associated_palette.py` helper module used by render-plan, extract, and recolor paths.

**D15. [duplicate] tests boilerplate.** `sys.path.insert(0, …/lambda)` repeated in ~50 files → move to `tests/conftest.py` (~50 lines). `fake_run` subprocess stub redefined **27** times across 10 handler tests → a `tests/_fakebin.py` factory absorbs the signature + binary-dispatch scaffold (per-binary output bytes stay test-local). The AST sweep also found repeated visual-test helpers (`load_poly_funcs`, `run_c`, `rast`, `run_py`) across `tests/test_visual_*.py`, repeated `_run_sweep`/`_read_params`/`_read_coeffs` in the hand-poly tests, and repeated HTTP-range test servers in parity tests. Clean, low-risk.

### Tier 3 — low payoff (fold opportunistically, don't make a project of it)

- **C micro-dups:** `parseCuts` ×2 (→ `solve_score.h`), `cmp_double` ×4, the serpentine index expression `(i1&1)?(n2-1-j):j` inlined 7× in `sweep_cli.c` + its inverse in `solve_palette_debug.c` (a `static inline serpentine_i2` would de-risk the shuffle/deshuffle contract more than it saves lines), inconsistent `round(clamp01*255)` formulas (3 variants — a *consistency* fix, not a LOC one), and def-only wrappers/helpers `parseStringArray` (`sweep_cli.c:543`), `runParamGenThreaded` (`sweep_cli.c:8479`), `parse_solve_score_program_args` / `solve_score_program_from_legacy` (`solve_score.h:1471/1479`). Treat `c_acos` (`sweep_cli.c:2605`) more carefully: current C does not call it, but `transpile_poly.py` can emit it, so it is generator-compatibility surface unless the transpiler is changed.
- **Python micro-dups:** orchestrator starter template ×3, `_active_execution_for_job` duplicated between compute/render orchestrators, `_stringify_meta` ×3 (+ the per-dict twin in `color_artifact_meta.py`), `_metadata_size_bytes` ×6, `_load_json_key` ×3, `_download_key_to_path` ×2, `_normalize_background_color` ×3, `_fragment_key` ×2 (→ `logical_sections.py`), and the `n_steps` prelude + ENOSPC `except` shared by `handler_sweep_mt.py`/`handler_sweep_cm.py`/`handler_coeffgen.py`.
- **Python drift nits:** hardcoded `ExpiresIn=3600` at `handler_storage.py:2421` should use `shared.PRESIGN_EXPIRY`; the magic `ExpiresIn=900` fragment-URL TTL is repeated ~10× with only `handler_bilevel.py:49` naming it (`FRAGMENT_URL_EXPIRES_S`) — no single source of truth.
- **`handler_compute_orchestrator.py:98-105`** inlines the pipeline-mode rule that `pipeline_programs.pipeline_mode_from_params` centralizes — and the inlined copy omits the helper's legacy aliases / `param_program` / `coeff_program` probes. A genuine second copy of the rule; low-stakes today, but exactly the drift class CR14 tried to close.

### Marginal — flagged but NOT recommended

- **JS per-palette-type getters/setters** (`js/04:397-519`, `_{builtin,tri,long}PaletteForMode` etc., ~80-120 lines): factorable in principle, but they read/write top-level `let` globals, so a data table needs a wrapper-object indirection that touches load order. Lower confidence than D3; do it only if D3's descriptor object can absorb them cleanly.
- **JS `_aggregate*Perf` ×12** (`js/09`/`js/10`/`js/12`): share a `filter → reduce` skeleton but each has a genuinely different field schema (sum / max / first-non-empty / weighted). A spec-driven reducer is possible but adds indirection for divergent shapes. Only worth it if a per-field `{op}` table reads cleanly.

---

## Considered and intentional — leave alone

These look like duplication/dead code but are deliberate; do **not** "fix" them:

- **`_chipPickers` (pp/cp) and its thin public wrappers** (`js/08:120-266`) — already correctly factored; the wrappers (`toggleParamProgramPicker`, `selectCoeffProgramChip`) exist *for* `onclick` reachability. `_coeffProgramCategoryGroups` vs `_paramProgramCategoryGroups` and the two `_sync*ProgramAddOptions` legitimately diverge (cp filters `spec.hidden`, different fallbacks, no hidden `<select>`).
- **The param-program VM vs the coeff-program VM** (Python `param_program_chain.py` vs `coeff_program_chain.py`; the C param VM vs coeff VM in `sweep_cli.c`) — deliberately separate. The sweep agents were instructed not to propose merging them, and didn't.
- **`handler_color_repalette.py` ⇄ `handler_recolor_from_raw.py`** — identical bodies, but intentional: two distinct dispatch-map entries both delegating to the shared `color_recolor_raw.handle_…`. The logic is already deduped into the module; these are the correct thin-entrypoint pattern.
- **Wire-format/fingerprint serializers** — not dedup targets where it would change emitted bytes.
- **The live `solve_palette_chunk.c`** (non-MT) — the memory note said "retired but compiled"; current state is **still reachable**: `handler_palette_chunk.py:507` selects the non-MT binary when `use_legacy_binary` (threads==1, tmpfile, no lag, no raw output). Its `getArg*`/`parseCuts` are covered by D1/D-tier-3, but the file is not dead.

## Confirmed genuinely clean (checked, no action)

- Shared singletons hold: `_escapeHtml` (98 uses), `_pluralize` (11), `_str`, `_fmtMs`, `lambdaPost`/`lambdaUrl` (only 5 raw `fetch` in the whole frontend). No dynamic dispatch (`window[…]`/`eval`/`new Function`) anywhere — the static analysis is complete.
- `solve_score_chain.py` / `pipeline_programs.py` are single sources of truth and are *imported*, not reimplemented, by the handlers that need them. The suspected render/palette/storage copies of solve-score precedence do **not** exist.
- C shared headers (`solve_score.h`, `root_xforms.h`, `palette_lut.h`, `companion_solver.h`) are properly shared — no inline re-definitions of the metric/clip/xform/palette logic outside them.
- All 40 `handler()` Lambda entrypoints, the storage route table, and `sweep_mt`/`sweep_cm` are live and packaged.
- Generated catalogs (`coeff_vocab_js.js`, `tri/long_palette_catalog_js.js`, the `*_palette_lut.generated.h`, `coeff_func_lookup.h`) regenerate byte-identical to what's committed — not stale, and drift-guarded by tests.
- The prior deploy.sh dedup (libcurl heredoc, libvips flag vars, manifest spec list) holds.

---

## Recommended order

Dead code first (free, no behavior change), then duplication top-down by payoff, stopping when it stops being worth it:

1. **Delete or move dead/manual code** — delete the 15 JS symbols, `shared.s3_get_object`, and the verified def-only Python helpers; move the 8 root visual-comparison scripts into `test-visual/` so they remain manual tools without being root pytest hazards; move the lambda-local manual transpiler/visual scripts out of `lambda/`; delete the 4 orphaned `poly_generated` files (~1.48 MB) + 2 orphaned `.c` + stale binaries; and move `preview_selection.py`/`ops_xfrm.py` out of `lambda/`. All verified; each is a clean removal or relocation.
2. **D1 + D2** (C `cli_args.h` and the `multispan_reader.h` download driver) — the single largest LOC win and it eliminates real 3-way drift risk in retry policy.
3. **D3 + D4 + D6** (JS palette-popup descriptor table; deploy.sh packaging via manifest; compute-program compile helper centralization) — three structural simplifications that remove real drift surfaces.
4. **D5 + D8-D10 + D13-D14** (the Python handler pairs and shared helper consolidation) — medium payoff, low risk.
5. **D11-D15** opportunistically; Tier 3 only when you're already touching the file.

For a single-user project this is all elective. If you only do step 1 and D1/D2/D3, you've captured the overwhelming majority of the value.

## Verification

I verified myself (not just relayed from the sweeps):
- All 15 JS dead symbols are def-only (reference count = 1 across `js/`+`index.html`+`tests/`, and the three wrapper checks also covered all tracked HTML including the deepzoom viewer template); the control `setSolveMetric` is live, confirming the count discriminates live from dead.
- `shared.s3_get_object` is def-only.
- The listed Python helper functions are def-only by AST/token sweep and `git grep`.
- The 4 `poly_generated` orphans have `#include` count 0 (control `_500` = 1); sizes 460 KB / 397 KB / 621 KB / 95 B.
- `solver_cli.c`/`lores_viewport.c` absent from deploy.sh (`grep -c` = 0).
- deploy.sh has 40 `zip -FS` packaging lines.
- The three root-level `test_*.py` exist and fall outside `tests/conftest.py:6`'s `collect_ignore_glob` (no `testpaths` in `pyproject.toml`).
- Python AST duplicate-function sweep added the compile-helper, associated-palette, viewport/range-download, and visual-test helper clusters.

This document was updated after the first CR17 implementation batch above.
