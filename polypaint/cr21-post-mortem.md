# CR21 Post-Mortem — PDF artifact: downsample + multi-page provenance report (`45f4997`)

**Date:** 2026-06-23 · **Commit:** `45f4997` "Refresh PDF artifact generation" (935+/143−, 8 files) · **Tests:** `test_spread_pdf.py` + `test_pdf_artifact_handler.py` = 5 passed; frontend harness OK.

Method: four parallel adversarial reviewers over the full diff + current files, then every HIGH/correctness finding re-verified here by tracing the code and exercising it (built real PDFs, decompressed streams, grepped the gate).

## Verdict

**The root-cause fix is correct and well-built.** Downsampling-before-ReportLab (F1) is implemented end-to-end, the multi-page report renderer follows the format spec (trim geometry, palette, type scale, summary + appendix), and the report model is robust against thin `calc.json`. The problems are **test integrity, not runtime correctness** — and they are the *same class as CR20*: a green gate that doesn't actually exercise the new behavior. Plus two MEDIUM user-visible polish bugs. Nothing blocks the stuck-job fix; the must-fix set is the tests + two small UI/render bugs.

## What went right (verified)

- **F1 end-to-end (the actual fix).** `prepare_pdf_image` (spread_pdf.py:302-356) opens lazily, caps the long edge via `thumbnail(LANCZOS)`, returns full dims; the handler prepares **both** main + palette (handler:346-358) and passes the **prepared** paths into `build_color_spread_pdf` (handler:387-393), which no longer decodes the full raster. Verified: 8000×8000 → 3600×3600. The 10k raster is never embedded.
- **Decompression-bomb guard:** `MAX_IMAGE_PIXELS` raised + explicit `PDF_MAX_SOURCE_PIXELS` preflight (spread_pdf.py:320-324) + `DecompressionBombError → ValueError` (341-345). Verified both paths fire.
- **Format spec compliance:** trim-based geometry (`ORIGIN_X = BLEED_3 + MARGIN_L`, `CONTENT_W = CONTENT_NET − margins = 242 mm`, **not** `PAGE_W`); palette tokens, type scale, and all four helpers present; **real multi-page** summary + appendix via `c.showPage()`; Unicode sanitized via `_safe_pdf_text` (latin-1 replace); temp files cleaned in `finally` (handler:474); no exceptions escape compose.
- **Report model robust:** `build_pdf_report_model` (handler:194-273) reads param/coeff source (with display/chain fallback), root, solve-score (`solve_score_program_source_text` or `score_source_text`), function name; all via `.get`/guards — no KeyError on an old/partial `calc.json`.
- **Cleanup (F7):** dead `parse_boolish`, `pipeline_parts`, obsolete `filename`, and old `_build_spread_meta` removed.
- **Frontend abandon (F2/F6):** `_showPdfHardStaleAbandon` is mode-gated (`run.mode === 'pdf'`, js/10:1674-1676), manual (a button), clears the **persisted** lock via `_clearActiveRun()` → `localStorage.removeItem` (js/09:1475), and the wording is PDF-accurate ("PDF compose has not updated for 5+ min" / "stalled … 15+ min"). Docs updated (popup is the live entry point; Lambda+Pillow+ReportLab; not Rnw/LaTeX).
- **Packaging:** no new module — everything is in already-bundled `spread_pdf.py`/`handler_pdf_artifact.py` (deploy.sh:1214); no deploy/manifest change needed (correct).

## Findings (severity-ordered)

### F1 — HIGH · The PDF smoke test is hollow (the new feature is effectively untested)
`test_spread_pdf.py:85-86` asserts `b"compute_demo"` / `b"color_demo"` are in the PDF bytes. **Verified:** those match *only* the uncompressed `/Title` docinfo (`c.setTitle`). The rendered report content is in FlateDecode streams — a known program line (`UNIQ_PROG_LINE_42`, `rev(cf)`) does **not** appear in raw bytes. Reviewer D proved the test still passes with `_draw_report_summary`/`_draw_appendix_pages` neutered to draw nothing. So the central new feature — provenance report content — has **no** regression guard, and the plan's explicit requirement ("assert param/coeff source text appears in the generated PDF text stream", code-review-21.md step 7) is **not met**.
**Fix:** decompress the content streams (pypdf/pikepdf `extract_text()`, or zlib-inflate each `/Contents`) and assert a KV value (e.g. `AE-MT`), a program source line, and the appendix header are actually rendered.

### F2 — HIGH · Predeploy doesn't gate the new PDF tests (recurring CR20 miss)
**Verified:** neither `test_spread_pdf.py` nor `test_pdf_artifact_handler.py` is in `scripts/predeploy_check.sh`. All new backend PDF coverage is ungated — a regression in `spread_pdf.py`/`handler_pdf_artifact.py` passes predeploy. (`test_frontend_js.sh` is gated.)
**Fix:** add both files to the predeploy pytest subset, and add the matching membership `assertIn(...)` to `test_deploy_packaging.py`.

### F3 — MEDIUM · Appendix back-references point to wrong / non-existent pages
Summary excerpts say `… full source on appendix p.N` where `N` comes from `_assign_appendix_pages`'s **single-column** `ceil(lines/88)` estimate (spread_pdf.py:476-485), but the appendix actually renders in a **two-column** flow (`_draw_appendix_pages`:562-608) and banners pages "Source Appendix 1,2,3…" counting from 1, while the excerpt page numbers count from 2 (summary = p.1). Reviewer A reproduced a concrete mismatch (excerpts → p.2/p.3/**p.7** while only 4 appendix spreads exist). Two compounding errors: divergent page model + different numbering base.
**Fix:** one source of truth — render the appendix first and record each program's real landing page (or share one pagination model + base) before drawing the excerpt captions.

### F4 — MEDIUM · "Abandon" clears the lock but leaves the action bar disabled
**Verified:** `_clearActiveRun()` (js/09:1471-1476) does not call `_updateRenderActionButtons()`, and js/10 never calls it. Render-tab buttons are all gated on `|| !!_activeRenderRun` (js/11:685-720). The abandon handler clears the lock and stops polling, so nothing re-evaluates the buttons — they stay visibly disabled until an unrelated re-render (artifact select / family switch). The normal completion path only works because it follows the clear with `refreshRenderArtifacts(...)`; abandon skips it. (The palette analog already calls `_updateRenderActionButtons()` in its save/clear — js/09:1031/1040.)
**Fix:** call `_updateRenderActionButtons()` at the end of the abandon onclick, or inside `_clearActiveRun()` to match the palette pattern. (A reload already unblocks because the persisted lock is removed — this is in-session only.)

### F5 — MEDIUM (plan-acknowledged) · PNG sources still take a full decode before downsample
In the resized path, `work = img.copy()` (spread_pdf.py:331) runs before `thumbnail`. `draft()` shrinks the decode for **JPEG only**; for **PNG** (and the handler writes prepared as PNG; sources are commonly PNG) `draft()` is a no-op, so `img.copy()` forces a full-resolution decode (~300 MB for 10k²) before shrinking. OOM is *reduced* (bounded by the 150 M-pixel guard ≈ 450 MB < 2048 MB) not *eliminated* — exactly the limitation the plan scoped. The `img.copy()` is also unnecessary (`thumbnail` mutates in place); dropping it halves transient peak for large PNGs.
**Fix (optional, tightening):** `thumbnail` on `img` directly inside the `with`, no copy.

### F6 — LOW
- **`_wrap_monospace_line` infinite-loops when `max_chars ≤ 1`** (spread_pdf.py:408-421): the 2-space hanging indent keeps `len(prefix)+len(rest) > 1` forever. Not reachable internally (callers floor width at 20), but it's a public-named helper — guard it (`max(1, max_chars - len(prefix))` + break on empty `rest`).
- **CLI back-compat:** `build_color_spread_pdf` now always returns `{"path","page_count"}` (was a bare `Path`), so `make_spread.py` prints the dict instead of the path; `_build_color_spread_pdf_legacy` is now dead code.
- **DPI-derived cap** `pdf_image_max_px_for_box` (spread_pdf.py:292-294) is defined but never called — the flat `PDF_IMAGE_MAX_PX=3600` is used (fine; the function is dead).
- **Mis-scoped diff:** the index.html +5 lines are an unrelated `.coeff-program-source-text` sizing CSS rule, not PDF code — it rode along in this commit.
- **Doc residue:** `pdf_artifact.md`'s retained "Historical Design Notes" still describe the old centered-text left page and `586 × 296 mm` spread, contradicting the new design (shielded only by the "current contract wins" precedence clause).

## Test gaps (beyond F1/F2)
- **Multi-page overflow unproven:** `page_count >= 2` (test_spread_pdf.py:81) is satisfied by a single appendix page even for a 1-line program; the column/spread overflow path (`next_column`/`showPage`) is never forced. Add a program that exceeds one column and assert `page_count >= 3`.
- **Report-source extraction unverified:** the handler test's mocked `calc.json` has **no `pipeline`**, so `build_pdf_report_model` emits only the synthetic Coefficient-Function entry — param/coeff/root/score source extraction is never exercised. Feed a `pipeline` with `*_program_source_text` and assert they land in `report["programs"]`.
- **Bomb path untested:** the `PDF_MAX_SOURCE_PIXELS` / `DecompressionBombError` guards have no test.

## Recommended fix order
1. **F1** — make the smoke test decompress streams and assert real rendered content (KV value + a program line + appendix). This single test also catches F3 if it asserts the back-reference resolves.
2. **F2** — gate both PDF tests in predeploy + packaging membership.
3. **F4** — one-line `_updateRenderActionButtons()` in the abandon path.
4. **F3** — unify appendix page numbering (render-then-reference).
5. Test gaps (overflow, report-source, bomb), then **F5** (drop `img.copy()`), then the LOW cleanups (wrap guard, CLI return shape, dead `pdf_image_max_px_for_box`/`_build_color_spread_pdf_legacy`, doc residue, stray CSS).

F1 + F2 + F4 + the report-source test are the must-fix set before this is trustworthy; the runtime fix itself (downsample) is sound and shippable.
