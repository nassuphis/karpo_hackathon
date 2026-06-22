# CR20 Post‑Mortem — Solve‑Score editor → text‑only (`b689ed2`)

**Date:** 2026‑06‑22 · **Commit reviewed:** `b689ed2` "Implement CR20 solve‑score text editor"
(1615+/250−, 20 files) · **Gate:** `predeploy_check.sh` **green** (exit 0).

Method: four parallel adversarial reviewers (plan‑vs‑implementation + bug hunt)
over the full diff and current tree, then every critical/high claim re‑verified
by tracing the code and **compiling the actual strings against the real backend
compiler** (`compile_solve_score_program_source`).

## Verdict

The hard, risky parts of the migration are **correct** — but the commit ships a
**HIGH‑severity out‑of‑box break**: the default solve‑score source it seeds does
not compile, so a fresh load → render/preview errors. Two medium issues
(cheatsheet snippets that don't compile; chain dropped from the browser payload
with no client‑side blank‑source guard) compound it. The predeploy gate is green
because **no test compiles the default or the cheatsheet snippets** — the test
suite asserts structure (ids present/absent, hydration wiring) but never
exercises the grammar the feature now depends on.

Net: structurally a clean migration; needs a one‑line fix to be usable, plus a
few snippet/test fixes before it's solid.

## What went right (verified)

- **`_renderChips` restructure (js/09:399‑460)** — the load‑bearing fix landed
  correctly: the chip‑HTML body is wrapped in `if (el) { … }` and the early
  return became `else if (which !== 'ss' && which !== 'palette-ss') return;`, so
  the solve‑score post‑render sync block (`_syncSolveScoreLegacyInputs`,
  `_syncScoreNormalizationUi`, `_updateSolveScoreButtons`,
  `_syncPaletteColorInterpretationUi`, …) runs **unconditionally** with the chip
  container gone. Every callee is independently DOM‑guarded. No skipped sync.
- **Relocation (#1 risk) handled** — `render-solve-score-quantile` (1799),
  `render-score-normalization` (1801), `btn-solve-histogram` (1803) all survive,
  moved into the render toolbar above the text panel. Both text panels carry
  `active` (1805 / 1888), so the editor is visible.
- **Chip removal clean** — all chip tabs/panels/add‑popups/stack ids gone for
  both prefixes; dead helpers (`toggleSolveScorePicker`, etc.) remain only as
  guarded no‑ops. `_setSolveScoreProgramEditorMode` pinned to `'text'`; state
  default flipped to `{render:'text',palette:'text'}`.
- **Backend reconstruction (`_render_artifact_entry`, handler_storage.py:2206‑2265)**
  — gated strictly on a non‑empty list chain (empirically: `''`/`None`/`[]` →
  `""`, no proximity fabrication), stored source wins, `try/except` fail‑silent,
  and both `family=="color"` palette/associated variants reconstructed.
- **`/solve-score-chain-to-source` route** — compiles for
  `fingerprint`/`program_spec`/`output_channels` (the helper returns only a
  string), returns **400** on empty/missing chain (no proximity fabrication),
  fail‑closed `{ok:false,error,code}` with `source_roundtrip_failed` propagated
  (all empirically confirmed). Three‑way frontend↔backend↔deploy wiring
  consistent (`api_manifest.py --check` / `deploy_manifest.py --check` pass).
- **Vocab generator** — `gen_solve_score_vocab.py` imports the constants from
  `solve_score_chain.py` (no re‑listing), has `--check` (green, not stale), and
  the frontend genuinely **hydrates `_ssCatalog` and all metric/source/spec
  tables from `window._solveScoreVocab`** — the hand‑maintained duplicate is
  removed (drift class fixed). Metric set + per‑metric allowed sources verified
  equivalent to the old hand list.
- **Planner→worker chain contract preserved** — `handler_raster_mt.py:592` and
  `handler_palette_chunk.py:220` re‑populate `section_params["solve_score_chain"]`
  from the compiled program after `solve_score_program_for_run`, so workers/ASL
  still consume the canonical chain. The plan's NEVER‑drop rule was respected.

## Findings (severity‑ordered)

### F1 — HIGH · Shipped default solve‑score source does not compile
`_defaultSolveScoreProgramSourceText` (js/08‑chip‑editors.js:440) returns:
```
score = metric(<metric>, <source>, q=0.1%)
emit_norm(score)
```
Compiling that exact string:
> **FAILS: "explicit emit statements cannot be mixed with score assignment"**

The grammar forbids mixing a `score =` assignment with an explicit `emit*`
statement (solve_score_program_source.py). `_ensureSolveScoreSourceDefaults`
(js/08, boot wiring js/12:1064) seeds this into **both** the render and palette
textareas whenever the box is blank — i.e. on a fresh load. Because the chip UI
is gone **and** this commit dropped `solve_score_chain` from the browser payload
(F3), the text is the sole authored input, so dispatch compiles it strict and
raises `SolveScoreSourceCompileError`. **A fresh page → select solve‑score color
mode → render deterministically errors on the shipped default.**

**Fix (one line):** make the default a single consistent form. Both compile:
- `score = metric(<metric>, <source>, q=0.1%)\n` (implicit score, no emit), or
- `emit_norm(metric(<metric>, <source>, q=0.1%))\n` (explicit emit).

### F2 — MEDIUM · Cheatsheet Outputs/Unary/Combine snippets don't compile
The metric buttons and the three "starter" snippets (js/08:561‑563) produce
valid source (verified). But the generated Outputs/Unary/Stack/Combine buttons
emit placeholder identifiers that are not readable expressions (js/08:570,
`const fn = name === 'emit' ? 'emit(score)' : \`${name}(score)\``, and the
combine templates `abs_diff(a, b)` / `ema(a, b, 0.99)`):
> `emit(score)` → **FAILS** "unknown solve‑score expression 'score'"
> `abs_diff(a, b)` → **FAILS** "unknown solve‑score expression 'a'"

`score` is a reserved assignment LHS, not a readable expr; `a`/`b` are nothing.
A user who clicks "abs_diff" then Compile gets an error with no working example.
**Fix:** use real placeholder exprs — `pop()` (a valid expr) or a metric call,
e.g. `abs_diff(metric(proximity, slv, q=0.1%), pop())`, `emit_norm(pop())`.

### F3 — MEDIUM · Browser `solve_score_chain` dropped with no dispatch‑time blank‑source guard
The commit went past the plan's staged Phase 3 ("keep the chain during the
transition") and removed `solve_score_chain` from **all five** browser payload
sites in one step (js/09:990,1596; js/10:512,637,807 — js/09:835 is an artifact
*read*, not a send). With the chain gone, the payload carries only
`solve_score_program_source_text`, which is `''` for a blank textarea
(`_effectiveSolveScoreProgramSourceText`, js/08:428‑431). The backend then fails
closed: `handler_render_plan.py:486` and `handler_raster_mt.py:590` raise
`RuntimeError("fused … requires solve_score_chain or solve_score_program_source_text")`.

There is **no client‑side non‑blank‑source guard** before the five dispatch
sites, and the only mitigation (`_ensureSolveScoreSourceDefaults`) runs **once at
boot** — it does not re‑seed on dispatch and does not cover post‑boot blanking
or the failed‑reconstruction restore path, which deliberately clears the box
(`_restoreSolveScoreSourceFromArtifact`, js/11:288‑300, sets `''` + an error
status). Reproducible: blank the render textarea → render → backend `RuntimeError`.
This is the exact risk the plan §6 Phase 3 and §10 decision #1 flagged.
**Fix:** gate the five fused dispatch entrypoints on non‑blank
`_effectiveSolveScoreProgramSourceText` (surface a friendly validation error), or
re‑seed the default at payload‑build time. (Pairs with F1 — fix the default too,
or the seed itself is invalid.)

### F4 — LOW‑MED · Blank source silently fabricates `proximity` on non‑fused paths
`solve_score_program_for_run` (solve_score_pipeline_programs.py) falls through a
blank source to `compile_solve_score_chain_or_legacy("", …, default_metric="proximity")`,
fabricating a `proximity` program. The fused render/raster guard (F3) raises
before reaching this, so the user‑facing fused behavior is "error," not "silent
wrong program" — but any consumer that calls `solve_score_program_for_run`
without the fused guard would get a silent proximity program, contrary to §3's
"blank = no authored program." Low‑med; worth a backstop in the resolver.

### F5 — LOW · Generated vocab omits the q range
`gen_solve_score_vocab.py` emits metrics/sources/specs but not the quantile
bound `[0.1, 5.0]%` (`solve_score_chain.py` `_validate_quantile_fraction`). Not
load‑bearing today (the inserter hardcodes `q=0.1%`), but the cheatsheet can't
show the valid range. Plan §4 listed it. Add `quantilePercentRange:[0.1,5.0]`.

### F6 — LOW · Allowed‑source ordering changed (`slv,cf` → `cf,slv`)
The generator `sorted()`s allowed sources, so `allowedSources[0]` flipped from
`slv` to `cf`. Defaults are hard‑coded `slv` everywhere that matters; the only
`[0]` fallback (`_defaultSolveScoreChain`, js/02) is reachable only with an
empty/invalid source on a param‑capable metric — would now seed `cf`. Edge case,
not a real regression; note it or sort with `slv` first for parity.

## Test gaps (why the green gate missed F1–F3)

The suite asserts **structure**, never **grammar**. Add:
1. **Compile the default + every cheatsheet snippet** against
   `compile_solve_score_program_source` (would have caught F1 and F2 immediately).
   Best as a backend test that imports the snippet table, or a node check that
   posts each through the compiler.
2. **Route negative paths** for `/solve-score-chain-to-source` (plan §8): empty
   chain → 400 / no proximity; unserializable → `ok:false`; `source_roundtrip_failed`
   code propagation. Only the happy path was added
   (test_solve_score_program_storage.py:253), and it doesn't assert `output_channels`.
3. **Direct `source_roundtrip_failed` unit test** in
   `tests/test_solve_score_program_source.py` (plan §8) — not added; still only
   covered at the migration layer (`test_program_v2_migration.py:196`).
4. **`_renderChips('ss')` runtime sync assert** with `#ss-chips` absent (plan §8)
   — not added; the load‑bearing restructure is unexercised by tests.
5. **Relocation asserts** — `assertIncludes` for `btn-solve-histogram` and
   `render-solve-score-quantile` still present (plan §8/§9) — not added; a future
   cleanup could drop a relocated control undetected.
6. **Palette/associated reconstruction test** (plan §9) — the
   `palette_source_*` / `associated_palette_*` branches are untested.

## Process note

The plan staged Phases 3 (drop browser chain), 4 (frontend load), 5 (remove chip
UI) so the chip+chain **safety net stayed until reconstruction was proven**, with
Phase 5 gated. This commit collapsed 3/4/5 together — removing the safety net in
the same step that shipped an invalid default — which is precisely why F1 has no
fallback to mask it and surfaces as a hard out‑of‑box break. The staging existed
to prevent exactly this; collapsing it traded the guard rail for one commit.

## Recommended fix order

1. **F1** (one line) — make `_defaultSolveScoreProgramSourceText` a single
   compilable form. Unblocks the out‑of‑box flow.
2. **F2** — fix the non‑metric cheatsheet snippets to compile (`pop()` /
   `metric(...)` placeholders).
3. **F3** — add a client‑side non‑blank‑source guard at the five dispatch sites
   (friendly validation instead of a backend `RuntimeError`).
4. **Tests 1–6** — add the grammar/compile test first (guards F1/F2 forever),
   then the route negative‑path + fail‑closed + reconstruction tests.
5. **F4–F6** — backstop the resolver's blank→proximity; emit q range; sort
   sources with `slv` first.

F1+F2+F3+test‑1 are the must‑fix set before this is shippable; the rest are
hardening.
