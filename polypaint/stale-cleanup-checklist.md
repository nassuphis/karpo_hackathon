# Stale Cleanup Checklist

Purpose: remove permissive legacy behavior from active code paths, stop old terminology from leaking into current UX/logs, and keep backward compatibility explicit instead of accidental.

## Phase 1: Fail Closed On Active Artifact Reads

- [x] `handler_finalize.py`
  - stop falling back from `pix_chunk_*` to legacy `pix_*`
  - keep missing current chunk fragments as "no hits", not "try old layout"
- [x] `handler_bilevel.py`
  - stop falling back from `bits_chunk_*` to legacy `bits_s*`
- [x] `handler_solve_proximity.py`
  - stop falling back from `chunk_*_hist.json` to legacy `stripe_*_hist.json`
- [x] add regression tests proving the legacy key is not read implicitly

## Phase 2: Make Legacy Execution Explicit

- [x] `handler_coeffgen.py`
  - require explicit `phase=legacy_coeffgen` for the legacy stripe path
  - reject missing/unknown `phase` instead of defaulting into legacy mode
- [x] update coeffgen handler tests to pass `legacy_coeffgen` explicitly
- [x] add negative tests for missing/unknown phase

## Phase 3: Scrub Stale Terminology From Current UX

- [ ] remove `stripe` naming from active UI labels, status context, and progress text
- [x] stop surfacing `stripe_idx` as the preferred identity in current dispatch/status paths
- [ ] update active comments/docstrings that still describe chunk-first flows as stripe-based
- [ ] keep legacy names only where they are true compatibility aliases, and label them as such

Targets already identified:
- `index.html`
- `handler_dispatch.py`
- `handler_finalize.py`
- `roots2pix.c`

## Phase 4: Reduce Drift Between Similar Planners

- [x] factor shared chunk/calc compatibility helpers out of:
  - `handler_render_plan.py`
  - `handler_palette_render_plan.py`
- [x] keep one shared contract for chunk items and calc compatibility fields
- [ ] add tests that compare both planners on shared metadata shaping

## Phase 4B: Keep Artifact Metadata Contracts Explicit

- [x] Color artifact readers use the shared merged-metadata loader instead of
  assuming all fields live in S3 image headers
- [x] active palette image writers keep object headers minimal and store bulky
  state in `meta.json`
- [ ] audit the remaining derived-artifact paths for raw `head_object(...).Metadata`
  assumptions and direct large-header uploads

## Phase 5: Review Remaining Public Compatibility Aliases

- [ ] decide whether `solve_proximity` remains a public API name or becomes a compatibility alias only
- [ ] audit storage/inventory code for legacy-only fallbacks that should stay explicit
- [ ] document the remaining supported legacy layouts, instead of keeping them implicit

## Verification Gate

Run after every stale-cleanup batch:

```bash
./.venv/bin/python -m pytest \
  tests/test_pipeline.py \
  tests/test_solve_proximity_handler.py \
  tests/test_bilevel_handler.py -q

./.venv/bin/python -m py_compile \
  lambda/handler_coeffgen.py \
  lambda/handler_finalize.py \
  lambda/handler_bilevel.py \
  lambda/handler_solve_proximity.py

git diff --check
```
