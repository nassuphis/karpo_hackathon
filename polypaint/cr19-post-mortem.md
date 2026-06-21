# CR19 — Implementation Post-Mortem / Code Review

Companion to `code-review-19.md` (the plan). Reviews the **as-built** root + solve-score program-source implementation against that plan.

**Scope reviewed.** The implementation is **uncommitted in the working tree** on top of HEAD `7bdb6af` (which was doc-only). New: `lambda/root_program_source.py` (420), `lambda/solve_score_program_source.py` (528), `tests/test_root_program_source.py`, `tests/test_root_program_storage.py`, `tests/test_solve_score_program_source.py`. Modified: 22 files, ~676 diff lines (`handler_storage.py`, `pipeline_programs.py`, `program_v2_translate.py`, `handler_render_plan.py`, `handler_palette_render_plan.py`, `handler_palette_finalize.py`, `program_profiles.{json,py}`, `deploy.sh`, manifests, two JS, palette ASL, several tests).

**Method.** Per-area reviewers (root, solve-score, completeness) + independent re-verification of every HIGH finding by tracing the code (and one empirical repro). Findings marked **[verified]** (I traced/ran it) or **[reported]** (reviewer-surfaced, spot-checked).

**Gate status.** `95 passed, 2 subtests` (exit 0) on the CR19 pure-Python tests — **but the suite is inadequate**: only hand-written single cases, no negative-param cases, no real-corpus round-trip, no pipeline/native-parity tests. The green run is exactly why the HIGH bugs below shipped (the CR18 "green ≠ correct" lesson, repeated).

## Resolution Update (2026-06-21)

Follow-up implementation pass closed the CR19 blockers that were actionable without live-S3 corpus export:

- **H1-H4 closed.** Solve-score reverse serialization now handles negative params, `pop()`/materialized stack history, full-stack materialization around `dup`/`flush`, and v2 migration fails closed instead of crashing.
- **H5 closed.** Direct root-consuming handlers now route through the root resolver instead of silently ignoring `root_program_source_text`.
- **H6 partially closed.** A local corpus equivalence gate now covers representative saved-program/artifact shapes, program-spec/fingerprint/output-contract equivalence, and authored-source byte-spec parity. The remaining live export of real S3 saved programs/artifacts is still a deployment/release gate, not a unit-test fixture.
- **M1 closed.** Keyword-arg parsing and numeric/percent literal formatting are shared in `program_source_core.py` and covered by `tests/test_program_source_core.py`.
- **M2 closed.** Direct solve-score handlers now use `solve_score_program_for_run`, including render lores preview, solve proximity, raster MT, palette chunk, and palette finalize.
- **M3 closed.** `solve_score_program_source_text` is threaded through render/palette plans, ASL payloads, raw sidecars, artifact metadata, associated-palette metadata, storage inventory, and Populate paths.
- **M4 closed.** Render scratch identity now includes root program fingerprint while preserving the legacy transform fallback.
- **M5 closed.** Authored solve-score source vs equivalent chain now has byte `program_spec`/fingerprint parity tests.
- **L1 closed.** Dead parser/resolver code called out here was removed.
- **L2 closed for the browser path.** Render and Palette now expose Text/Chips source tabs for solve-score and root programs; backend compile routes refresh chip previews; Populate restores authored source when metadata contains it.
- **L3 closed.** Deployment packaging now includes root/solve-score resolver/source modules in the Lambdas that import them, and tests pin those bundles.

Current verified gates for this resolution pass: focused backend/workflow/packaging tests `129 passed, 2 subtests`; route-contract manifest test `9 passed`; frontend split-file harness green. A full-suite attempt reached `1299 passed, 9 skipped, 28 subtests` but cannot be used as a clean local gate in this sandbox: tests that bind `127.0.0.1` fail with `PermissionError`, and Docker-backed native tests fail on the unavailable Docker socket. The live-S3 corpus export/native parity gate remains a release checklist item rather than a completed local test.

---

## Verdict

This is a **partial implementation with shipping blockers**, not a finished Phase 6. The *forward* paths and the hard identity work are genuinely good — root default-arg canonicalization, the solve-score forward compiler, the two-mode output contract, `compile_solve_score_chain` reuse, the Step-0 drift fix, and v2-translate are correct and tested. **But:**

- **6B (solve-score) is not mergeable.** The reverse serializer — the load-bearing piece I flagged last review as "the hard part; run the corpus gate early" — is **not total**: three verified round-trip bugs, and migration **crashes** instead of failing closed. The real-corpus gate that would have caught them **was not built**.
- **6A (root) backend is solid but not wired through.** Four root-consuming handlers bypass the new resolver, so direct source-only requests silently lose their root transforms.
- **Integration / UI / tests are well short of the plan**: 6 of 8 solve-score handlers unrouted; render-path metadata/ASL/raw-sidecar untouched; UI source tabs essentially absent; corpus + pipeline + native-parity gates missing.

---

## Findings by severity

### HIGH

**H1 — Solve-score source rejects negative call params (can't author *or* round-trip them).** [verified, repro'd]
`_parse_expr` (`solve_score_program_source.py:190`) runs `re.search(r"(?<![eE])-", raw[1:])` over the whole expression *before* parsing the call, so any expression text containing a `-` past position 0 is rejected as "infix." Confirmed empirically: `score = sawtooth(metric(proximity, slv, q=0.1%), -2)` → `SolveScoreProgramSourceCompileError: ... infix arithmetic is not supported`. This hits negative `sawtooth`/`omega_cosine` phase, `pow` exponent, `weighted_sum` weights, and negative `const`. Those compile fine as *chains*, so they're real existing programs — they just can't be authored in source or survive `chain→source` round-trip. **Fix:** scope the infix guard to operator position (don't treat a leading `-` inside a parsed numeric arg as infix); parse call args first, then validate.

**H2 — Reverse serializer emits `pop()`, which the forward parser can't reparse.** [verified]
The materialized/dup path appends `score = pop()` (`:506`), `name()` + `name(pop())` (`:470-471`), and `push(...)`/`name()`/`name(pop())` (`:490-492`). But `_parse_expr` only handles bare `pop` (`:196`), not `pop()` with parens → reparse of any dup-bearing chain fails. **Fix:** make the parser accept `pop()` (or have the serializer emit the form the parser accepts), and add a dup round-trip test.

**H3 — `dup` over an unmaterialized lower stack drops metric slots.** [reported, consistent with code]
The materialize path only pushes the *current op's* unmaterialized args; a lower stack item left unmaterialized when a `dup`/`flush` sequence runs is never `push()`-ed, so a chain like `[spread, crowding, dup, flush, proximity, emit_norm]` loses the `spread` slot and the `program_spec` drifts. (Also dead code: `while len(stack) < 0: pass` at `:486` — clearly not the intended materialize-the-lower-stack loop.) **Fix:** materialize the entire residual stack before a stack-only op, and pin it with a multi-slot dup/flush test.

**H4 — Migration is not fail-closed; it crashes.** [verified]
`translate_solve_score_from_old` calls `solve_score_source_text_from_chain(canonical_chain)` with **no try/except** (`program_v2_translate.py:465`) — unlike root's, which is guarded at `:510`. So any valid chain that trips H1/H2/H3 raises and **crashes the whole migration**, instead of the algorithm's own step-12 behavior (return no `source_text`, keep the program chip-only with a `source_roundtrip_failed` diagnostic). **Fix:** wrap it; on `source_roundtrip_failed`, migrate chip-only and record the diagnostic.

**H5 — Root resolver is wired into only 2 of the root-consuming handlers.** [verified]
`root_program_for_run` is imported only by `handler_render_plan.py:44` and `handler_palette_render_plan.py:24`. `handler_bilevel`, `handler_solve_proximity`, `handler_render_lores_preview`, and `palette_debug` still read `params["root_transforms"]` raw and write `--root_xforms` directly. A direct request to those paths carrying `root_program_source_text` or an embedded `root_program` (and no legacy rows) **runs with no root transforms, silently** — the exact "unplanned exception" the plan (Step 3 / deep-dive #22) forbids. **Fix:** route those four through `root_program_for_run` (or mark them legacy-only in the plan).

**H6 — The real-corpus equivalence gate (the 6B go/no-go) and the pipeline/native-parity tests are missing.** [verified]
`test_solve_score_program_source.py` has 5 hand-written single-case tests; there is **no** iteration over saved programs / render-palette-proximity artifacts / raw-sidecar `score_chain` (plan Step 11 + the §6B completion rule), **no** `test_root_program_pipeline.py` / `test_solve_score_program_pipeline.py`, and **no** `solve_score_eval.py` native-parity test. This gate is precisely what would have caught H1–H3. It is the empirical proof of the §6B "every valid chain round-trips" rule, and without it that rule is unverified. **Fix:** build and run it before merge.

### MEDIUM

**M1 — Step 1 deviation: kwargs/percent parsing lives in the profile module, not the shared core.** [verified] `_parse_kwargs` (`solve_score_program_source.py:138`) and `_fmt_q_percent` (`:87`) are reimplemented here; `program_source_core.py` was not touched and has no `parse_call_args`/`parse_percent_literal`. The plan's Step 1 explicitly put these in the shared core (so the grammar extension is one place, profile-safe, and source-core-tested). This repeats the param-bypass smell from the CR18 post-mortem. **Fix:** relocate to `program_source_core.py` with the Step-1 tests.

**M2 — Step 7 routing is 2/8.** [verified] Still calling `compile_solve_score_chain` directly (source/embedded program ignored): `handler_raster_mt.py:589`, `handler_palette_chunk.py:211`, `handler_render_lores_preview.py:333`, `handler_solve_proximity.py:390`, `handler_palette_finalize.py:184`; `handler_extract_palette_from_step_scores.py` has no solve-score routing. Only `handler_render_plan`/`handler_palette_render_plan` use the resolver. **Fix:** route the rest through `solve_score_program_for_run` (plan handlers compile; chunk/finalize consume plan-provided fields).

**M3 — Step 8 metadata threading is incomplete → source won't survive render artifacts.** [verified] `render_workflow.asl.json.template`, `workflow_contracts.py`, `raw_sidecar.py`, `color_artifact_meta.py` are untouched; palette ASL lacks `solve_score_program_source_text`. So a rendered/palette artifact won't carry the source text, and **populate can't restore editable source for renders** — the Risk-4 / CR18-H2 class the plan called out as must-fix-from-the-start. **Fix:** thread the source/fingerprint/spec-version fields through the render path + metadata writers.

**M4 — Root cache-identity split.** [reported] `_plan_params_digest` switched to `root_program_fingerprint` but `_solve_score_scratch_key` (`handler_render_plan.py:174`) still hashes raw `root_transforms`; the plan listed it among the identity helpers to update. Two cache keys now key off different root identities. **Fix:** pass the fingerprint into the scratch key; keep a dual-read fallback for old artifacts.

**M5 — No independent `program_spec` byte-identity test.** [verified] The plan calls this "mandatory" (several validators compare the string). It's only asserted inside the round-trip serializer test; there's no "authored source vs hand-written equivalent chain → identical `program_spec`/fingerprint" test. **Fix:** add it.

### LOW / NIT

- **L1** [verified] Dead code: `while len(stack) < 0: pass` (`solve_score_program_source.py:486`); dead ternary `strict=False if strict else False` (`pipeline_programs.py:170,235`).
- **L2** [reported] Step 9 UI essentially absent — only object-form populate touches in `js/09:854`, `js/11:400`; no Text/Chips source tabs, no source state, no `/compile-*-program-source` calls in JS. The plan's closing rule ("do not call Phase 6 done until populate restores editable root/score source text") is unmet for the new flows. (Last step; deferrable — but the user-facing goal isn't reached yet.)
- **L3** [verified] Step 10 packaging partial: the new source modules aren't bundled into ~8 Lambdas that would touch source once M2 lands (render_lores_preview, solve_proximity, palette_chunk, finalize_mt, raster_mt, palette_debug, bilevel, palette_finalize), and `api_manifest.json` `frontend.storage_paths` isn't synced with the two new compile routes (so `api_manifest --check` isn't fully green). Low *now* (those paths don't resolve source yet); becomes ImportError-class once M2 is done.

---

## What's genuinely strong

- **Root default-arg canonicalization (the #7 identity trap) is correctly solved + tested.** `_canonical_row` expands omitted args to registry defaults before fingerprinting, so `pull_unit_circle()` ≡ `pull_unit_circle(0.75,1.0)`.
- **Solve-score forward compiler + two-mode output contract are correct and tested**, and it reuses `compile_solve_score_chain` (no reimplemented semantics); metric CSE is preserved by delegation.
- **The reverse serializer is a real symbolic-stack algorithm** with the reparse assertion + `source_roundtrip_failed` fail-closed code — it just isn't total yet (H1–H3).
- **Step 0** drift fix (`program_tokens` 256→32 + drift-test assertions), **v2 translate** (root→`root_program` + source_text; solve-score migration adds source_text; inverted migration test), and **moebius registry namespacing** are all done correctly.

---

## Per-area verdict

| Area | Verdict | Headline |
|---|---|---|
| Root source module (6A core) | **Correct** | Parser, default-arg canonicalization, serializer, v2-translate all right + tested. |
| Root integration (6A) | **Incomplete** | H5: 4 root handlers bypass the resolver → silent transform loss; M4 scratch-key. |
| Solve-score forward (6B) | **Correct** | Compiler, output contract, storage, reuse — good. |
| Solve-score reverse serializer (6B) | **Not mergeable** | H1 negative params, H2 `pop()`, H3 dup-drops-slot — not total. |
| Solve-score migration (6B) | **Not mergeable** | H4 crashes instead of failing closed. |
| Tests / corpus gate | **Inadequate** | H6: green but no corpus/negative/pipeline/native-parity. |
| Step 7 handler routing | **Partial (2/8)** | M2. |
| Step 8 metadata/ASL/sidecar | **Partial** | M3 — source won't survive render artifacts. |
| Step 9 UI | **Absent** | L2. |
| Step 10 packaging | **Partial** | L3. |

---

## Recommended before merge (ordered)

1. **Fix the 6B reverse serializer:** H1 (negative-param guard), H2 (`pop()` reparse), H3 (materialize the full residual stack) — and **H4** wrap migration so it fails closed.
2. **Build the real-corpus equivalence gate (H6) and run it** over live saved programs + render/palette/proximity/raw-sidecar chains; add the pipeline + `solve_score_eval.py` native-parity tests. This is the proof — and it would have caught H1–H3.
3. **Wire the resolvers through:** H5 (4 root handlers) + M2 (6 solve-score handlers).
4. **Thread source metadata through the render path** (M3) so populate can restore source for renders.
5. M1 (relocate kwargs/percent to the shared core), M4 (scratch-key identity), M5 (byte-identity test), L1 cleanups.
6. **Then** Step 9 UI + Step 10 packaging/manifest sync.

6A is close — H5 + M4 and it's solid. 6B has a good spine but is blocked by H1–H4 + the missing H6 gate; treat the corpus gate as the go/no-go before calling 6B done.
