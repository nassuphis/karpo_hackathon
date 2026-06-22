# Code Review 20 — Migrate the Render Solve‑Score editor to text‑only

**Status:** implementation plan (deep‑dive complete) · **Date:** 2026‑06‑22

Finish the chips → text migration that param/coeff just completed (commit
`4039058`) by making the **Solve‑Score** program in the Render *and* Palette
tabs text‑authored, removing the editable chip UI, while keeping older saves
fully editable via on‑load reconstruction, and replacing the chip dropdowns'
discoverability with a **generated cheatsheet/inserter** (not a plain help file).

All line numbers below are verified against the tree as of this date; ids are
stable, line numbers may drift after edits — anchor on the ids.

---

## 0. The one correction that reshapes the whole plan

**Solve‑score is NOT structurally analogous to param/coeff.** The param/coeff
migration was cosmetic because the chain was already dead for compute
(`_effectiveParamProgramChainForCompute()` / `_effectiveCoeffProgramChainForCompute()`
`return []` — js/08‑chip‑editors.js:1097‑1117). For solve‑score the **chain is a
live compute input** whenever the text is blank:

- The render/preview payloads send **both** `solve_score_chain` **and**
  `solve_score_program_source_text` (js/09‑render‑orchestration.js:1597‑1598,
  990‑991; js/10‑status‑results.js:515‑516, 641‑642, 812‑813).
- Backend precedence (`solve_score_program_for_run`, lambda/solve_score_pipeline_programs.py:44):
  **source_text (if non‑blank) → `solve_score_program` dict → `solve_score_chain`.**
  Blank source returns `None` from `solve_score_source_text_for_run` (:12) and
  **falls through to the chain.**
- **The editor mode is cosmetic, not gated.** There is no
  `_solveScoreProgramTextModeSelected`. `_effectiveSolveScoreProgramSourceText`
  (js/08:428‑431) just returns the textarea value when non‑blank; the payload and
  modal‑save read the textarea, never `_solveScoreProgramEditorMode`. So we do
  **not** need to pin a mode for correctness (unlike param/coeff) — the textarea
  is authoritative when non‑blank, full stop.

**Consequence:** making solve‑score text‑only is a *behavioral* change, not just
UI removal. The decision that must be made up front: **what does "blank text"
mean once chips are gone?** Today blank text → render from the (now‑hidden)
chain. See §3.

Good news that de‑risks it: the source syntax is ergonomic, the reconstruction
primitive exists and is fail‑closed, **the equivalence test already exists**, and
the backend backward‑compat hook needs **no bundle/manifest changes**. Details
below.

---

## 1. Verified current state

**Two editors, prefixes `render` and `palette`** (index.html):
- Render editor: color‑box ~1751‑1792. Tabs row (`render-ss-tab-chips` /
  `render-ss-tab-text`) + `render-solve-score-program-manage`; chips panel
  `render-ss-chips-panel` (default `active`) containing `#ss-chips` plus the
  add‑popup controls **and three non‑chip controls that must be relocated** (§5);
  text panel `render-ss-text-panel` (**no** `active`) with `#render-ss-source-text`;
  status `render-solve-score-program-status`.
- Palette editor: ~1860‑1893. Tabs row (`palette-ss-tab-chips` /
  `palette-ss-tab-text`) + `palette-solve-score-program-manage`; chips panel
  `palette-ss-chips-panel` (default `active`) with `#palette-ss-chips` + add
  controls + a Stack display (no relocatable controls here); text panel
  `palette-ss-text-panel` (no `active`) with `#palette-ss-source-text`.
- CSS: `.coeff-program-editor-panel { display:none }` /
  `.coeff-program-editor-panel.active { display:block }` (index.html ~865‑870).
  **The chips panels carry `active`; the text panels do not.** Removing the tabs
  therefore requires **adding `active` to the two text panels** or the editor goes
  invisible. (Mirror param/coeff: `pp-text-panel`/`cp-text-panel` are `…active`.)

**`_setSolveScoreProgramEditorMode(prefix, mode)`** (js/08:461‑472): toggles
`${p}-ss` panels/tabs via `_setPanelTabActive` and sets a status string. No
synthesis, no stale‑clear, no modal refresh, no preview‑stale. State var
`_solveScoreProgramEditorMode = { render:'chips', palette:'chips' }` (js/08:402)
is **written but never read by any payload/save** — purely which panel shows.

**Source text plumbing (keep):** `_solveScoreSourceTextarea` →
`#${prefix}-ss-source-text` (js/08:409‑411), `_get/_setSolveScoreProgramSourceText`
(417‑426), `_effectiveSolveScoreProgramSourceText` (428‑431),
`_compileSolveScoreSourceEditor(prefix)` (512‑540, validates against
`/compile-solve-score-program-source`, then renders a chip *preview* into
`#ss-chips`). `_onSolveScoreProgramSourceInput` on the textarea.

**Chips data vs chips UI:** the chip *chain* (`_renderScoreChain` /
`_paletteScoreChain`, synthesized to `solveScoreChain` via
`_resolveSolveScoreState`/`_compileSolveScoreChain`, js/02) is the **empty‑text
compute fallback** and must keep existing. Only the *editable chip rendering and
add/edit controls* are removed. `_renderChips(which)` (js/09:399‑460) already
guards `if (!el) return` on a missing container, so removing `#ss-chips` makes
the chip render a no‑op **and skips the solve‑score post‑render sync block**.
**Fix at the single point, not the callers.** The post-render sync block
(js/09:445-459) does not reference `el`; only the chip-HTML rendering (403-444)
does. So rather than extract a helper and thread it through every caller,
restructure `_renderChips` so the render body is wrapped in `if (el) { ... }`
and the post-render syncs run unconditionally. One change covers every
`_renderChips('ss'|'palette-ss')` caller — chain edits,
`_compileSolveScoreSourceEditor` text-compile preview (js/08:531),
`_setSolveScoreChainFromArtifact`, `_applySolveScoreProgram`, and boot — and is
inert for pp/cp (their syncs sit inside the now-el-guarded branches, exactly as
today's early-return already skips them).

**Shared with the Solve‑Scores MODAL — DO NOT REMOVE:** `_renderRtChipHtml`,
`_solveScoreChipShell` (readonly branch), `_chipInputHtml`/`_chipReadonlyValueHtml`,
`_normalizeSolveScoreChain`, `_compileSolveScoreChain`, `_displaySolveScoreEntry`,
`_ssCatalog`, and `_setSolveScoreProgramStatus`/`openSolveScoreProgramModal`/
`_applySolveScoreProgram`. The modal renders read‑only chip previews of *saved*
programs, independent of the editor tab.

**Modal apply stale‑text trap:** `_applySolveScoreProgram` currently sets the
textarea only when `program.source_text` exists. That is safe while chips are
visible, but wrong for text‑only: loading/importing an old chain‑only JSON can
leave stale source text in the textarea, and that stale text overrides the newly
loaded chain. The migration must either synthesize source before applying the
program, or explicitly clear source text when no source is available.

---

## 2. Preconditions (verified)

1. **Syntax is ergonomic** (round‑trips via `solve_score_source_text_from_chain`):
   `score = metric(proximity, slv, q=0.1%)`;
   `emit_norm(abs_diff(metric(spread, slv, q=0.4%), metric(spread, slv, q=0.9%, lag=1)))`;
   two emits as two lines. Call‑tree only (infix rejected by design), named args,
   `%`, `lag=`. Writable.
2. **Text editor already compiles‑on‑edit** (`_compileSolveScoreSourceEditor` →
   `/compile-solve-score-program-source`).
3. **Backend already prefers source‑or‑chain** (§0) — no backend resolution change.
4. **Reconstruction primitive exists and is fail‑closed.**
   `solve_score_source_text_from_chain` (lambda/solve_score_program_source.py:445)
   re‑compiles its own output and asserts identical `program_spec` **and**
   `fingerprint`; on mismatch it **raises** `SolveScoreProgramSourceError(code="source_roundtrip_failed")`.
   It also raises on un‑serializable rows / stack imbalance. **It does not return a
   degraded string — callers must `try/except`.** (Note: an empty/`None` chain does
   *not* raise — it fabricates a default `proximity` program — so callers must gate
   on a non‑empty list, never call on empty.)
5. **The equivalence test already exists:** `tests/test_solve_score_source_equivalence.py`
   iterates an 11‑case corpus asserting
   `compile(solve_score_source_text_from_chain(chain)).fingerprint == compile(chain).fingerprint`
   (+ program_spec + output channels). We extend the corpus, not write it fresh.

**Phase 5 gate:** chip removal is gated on Phase 1 (cheatsheet/inserter),
Phase 2 reconstruction coverage, and Phase 2b chain→source route coverage. If
reconstruction fails, there is no editable chip fallback after Phase 5. Therefore
the implementation must define a failure UX for un‑serializable old chains before
deleting the chip editor: display a non-editable warning with the raw
chain/program id, keep the chain in the modal read-only preview where possible,
and do not silently show an empty text editor as if the program were empty.

---

## 3. The semantics decision (make this call first)

Once chips are gone, what is authoritative and what happens on blank text?

- **Today:** text wins when non‑blank; blank text → the chain renders.
- **Pinned end state:** **text is the sole authored program.** On load,
  reconstruct text from the saved chain so the editor is never unexpectedly
  blank (Phase 2). During the transition, keep sending `solve_score_chain` so
  behavior is unchanged while reconstruction and text dispatch are proven. After
  Phase 3 drops browser→planner chain fallback, a user-cleared text box means
  "no authored solve-score program" / validation error where the render mode
  requires one. Blank text must **not** fall back to a stale hidden chain.

This is the only genuinely new *behavior*; everything else is mechanical. The
implementation is intentionally two-step: keep chain fallback through Phases 1-2,
then remove browser→planner fallback after source reconstruction and dispatch
tests are green. Do not keep the fallback indefinitely; that would preserve a
hidden editable state after the visible chip editor is removed.

---

## 4. Discoverability — generated cheatsheet/inserter (not a help file)

The chip dropdowns **enumerate** the vocabulary *and* **validate** combinations
(which metrics accept which sources; lag/quantile rules). A help file only does
the first. Build a generated cheatsheet/inserter instead.

**Source of truth (backend, verified):** `lambda/solve_score_chain.py` —
`VALID_SOLVE_SCORE_METRICS` (41 metrics, :22‑64), `VALID_SOLVE_SCORE_SOURCES`
(`slv,cf,pm`, :69), `VALID_SOLVE_SCORE_LAG_DEPTHS` (`{0,1}`, :70),
`PARAM_SOLVE_SCORE_METRICS` / `PARAM_CAPABLE_SOLVE_SCORE_METRICS` /
`_METRIC_ALLOWED_SOURCES` (:71‑96), `UNARY_CHIPS` (:98), `COMBINE_CHIPS` (:109),
`OUTPUT_CHIPS` (:123), `STACK_CHIPS` (:128), `GENERIC_METRIC_PUBLIC_NAME="metric"`.
Grammar facts in `lambda/solve_score_program_source.py`: statements `score = expr`
/ `local = expr` / `push()/dup()/flush()/emit*()`; `metric(name, src, q=..%, lag=0|1)`;
q ∈ [0.1, 5.0]%; infix rejected.

**Vocabulary to surface:** 41 metric names; sources `slv/cf/pm` (+ `-1` lag
variants `slv-1/cf-1/pm-1`); per‑metric allowed sources; unary ops
(`omega_cosine(w,phase)`, `sawtooth(mult)`, `flip`, `const(v)`, `dup`, `flush`,
`clamp`, `sin`, `cos`, `log`, `exp`, `pow(p)`); outputs (`emit`, `emit_norm`,
`emit_none`); combinators (`avg/min/max/mul/add/mult/subtract/ratio/ema(α)/
weighted_sum(a,b)/abs_diff/geometric_mean`); generic chip `metric(name, src, q%)`.

**The frontend currently HAND‑MAINTAINS a duplicate of this** (`_solveScoreMetricNames`
js/07:285‑327, the source‑rule sets js/07:328‑345 / js/02:873‑877, the unary/combine/
output specs js/07:356‑388, assembled into `_ssCatalog` js/07:389‑434). It is kept
in sync only by `tests/test_solve_score_chain.py` drift checks. Generating the vocab
collapses that duplicate.

**Plan (mirror `lambda/gen_coeff_vocab.py`):**
1. New `lambda/gen_solve_score_vocab.py` that `import`s the constants from
   `solve_score_chain.py` (do not re‑list) and emits
   `window._solveScoreVocab = {…}` to repo‑root `solve_score_vocab_js.js`, with a
   "AUTO‑GENERATED … do not edit" header and a `--check` mode (exit 1 on stale),
   exactly like `gen_coeff_vocab.py` (build → render → `--check`).
2. Add `solve_score_vocab_js.js` to `frontend_asset_keys()` in `deploy.sh`, to
   deploy packaging assertions, and to the frontend harness. Load
   `<script src="solve_score_vocab_js.js"></script>` before the bundle, in the
   same slot as `coeff_vocab_js.js`.
3. Add a **cheatsheet/inserter panel** beside `#render-ss-source-text` and
   `#palette-ss-source-text` that reads `window._solveScoreVocab`: list metrics
   (grouped, with allowed sources), emit/combinator/unary forms; clicking a metric
   appends a valid snippet (`metric(name, slv, q=0.1%)`). Start as an inserter +
   collapsible reference; autocomplete is a later nicety.
4. Predeploy: add `"${TEST_PYTHON[@]}" lambda/gen_solve_score_vocab.py --check`
   right after the `gen_coeff_vocab.py --check` line (scripts/predeploy_check.sh).
5. **Required:** refactor js/07:285‑434 to hydrate
   `_solveScoreMetricNames`/`_ssCatalog` from `window._solveScoreVocab`, as
   `_ctCatalog` hydrates from `_coeffRegistryVocab`, removing the duplicate. If
   the generator only powers the cheatsheet while `_ssCatalog` remains
   hand-maintained, the drift class is not fixed.
   The generated object must include metric `allowed_sources`, param-only /
   param-capable classification, lag choices, unary specs, combine specs, output
   specs, stack/control specs, and the generic metric public/chip names. Then
   `_solveScoreMetricAllowedSources`, `_solveScoreGenericMetricNames`,
   `_solveScoreMetricSourceChoices`, and `_ssCatalog` all derive from the same
   generated object instead of preserving a second source-rule table in JS.

**Gate:** do not remove the chips (Phase 5) until the cheatsheet/inserter is live,
the generated vocab drives `_ssCatalog`, and reconstruction coverage is green.

---

## 5. ⚠️ Relocation gotcha (the #1 risk)

Three controls live **inside** `render-ss-chips-panel` but are **not** chip
controls and are referenced elsewhere — they must be **moved under the text
panel, not deleted**:
- `<input type="hidden" id="render-solve-score-quantile" value="0.1">` — written by
  `_syncSolveScoreLegacyInputs` (js/02).
- `<input type="checkbox" id="render-score-normalization">` — read in the render
  payload (js/09:591), js/04‑palette‑color.js:749, js/11‑artifacts.js:582,
  `_syncScoreNormalizationUi` (js/04:717).
- `<button id="btn-solve-histogram" onclick="runSolveScoreHistogramDebug()">` —
  referenced js/10:596, js/04:792.

The **palette** chips panel has no such controls (safe to delete whole).

---

## 6. Phased implementation

Each phase is independently shippable through Phase 4. Phases 1‑3 are
additive/safe; the visible chip removal is Phase 5 and is gated on Phase 1,
Phase 2 reconstruction coverage, and Phase 2b chain→source route coverage.

### Phase 1 — Generated cheatsheet/inserter (additive)
- Add `lambda/gen_solve_score_vocab.py` + generated `solve_score_vocab_js.js`;
  add it to `frontend_asset_keys()` / deploy packaging tests; wire the `<script>`;
  hydrate `_ssCatalog` and related metric/spec arrays from the generated vocab;
  add the cheatsheet/inserter panel beside both text editors; add the `--check`
  to predeploy. (See §4.) No removal yet.

### Phase 2 — Reconstruct source on load (backend, read‑only)
- **Where (verified):** `_render_artifact_entry` (lambda/handler_storage.py:2087),
  the builder behind `/render-summary` → `handle_render_summary` (:3086) that the
  frontend loads (js/01:215, js/10:1754) and that produces the `art` objects
  `populateSelectedRenderArtifact` reads. `solve_score_source_text_from_chain` is
  **already imported** (handler_storage.py:68) and the storage bundle already ships
  `solve_score_program_source.py` + deps — **no deploy.sh / deploy_manifest change.**
- **What:** after the `entry` dict is built (~after the solve‑score fields at
  :2119‑2124), when `entry["solve_score_program_source_text"]` and
  `entry["score_source_text"]` are blank **and**
  `isinstance(entry["solve_score_chain"], list) and entry["solve_score_chain"]`,
  set `entry["solve_score_program_source_text"] = solve_score_source_text_from_chain(entry["solve_score_chain"])`
  inside `try/except Exception: pass`. For `family == "color"`, do the same for the
  palette/associated variants → `palette_source_solve_score_program_source_text`
  (from `palette_source_score_chain`) and `associated_palette_solve_score_program_source_text`
  (from `associated_palette_score_chain`); these feed the frontend's
  `_artifactSolveScoreSourceText` candidate list (js/11:274‑279).
- **Gating:** strictly non‑empty list chains only (never call on `None`/empty —
  avoids the proximity‑fallback fabrication); `try/except` so `source_roundtrip_failed`
  and un‑serializable chains behave exactly as today during Phases 2‑4 (frontend
  keeps chips). Before Phase 5, add an explicit no-source failure UX because the
  editable chip fallback will be gone.
  Mirror the read‑only, stored‑source‑wins, fail‑silent pattern of
  `handle_detail`/`_detail_populate_program_form` (handler_storage.py:2848‑2942).
- After this, `_restoreSolveScoreSourceFromArtifact` (js/11:288) hits its
  text‑present branch for old chain‑only artifacts and loads readable text.

### Phase 2b — Chain→source route for local/imported JSON
- Add a storage route `POST /solve-score-chain-to-source` that accepts
  `{chain}` or `{solve_score_chain}` and returns `{ok, source_text, chain,
  fingerprint, program_spec, output_channel_count, output_channels}` using
  `solve_score_source_text_from_chain` for source reconstruction and
  `compile_solve_score_chain_or_legacy` + `compiled_solve_score_fingerprint` for
  the returned canonical chain/fingerprint/program_spec/output metadata. Empty /
  missing chains must return HTTP 400 instead of fabricating the default
  proximity program.
- Route errors must be fail-closed and structured: invalid/unserializable chains
  return `{ok:false, error, code}` with `code` propagated from
  `SolveScoreProgramSourceError` when available (`source_roundtrip_failed`, etc.).
- Wire the route through `handler_storage.py` dispatch plus
  `deploy_manifest.json` (storage function route list). Regenerate
  `api_manifest.json` with `python3 api_manifest.py --write`; predeploy then
  verifies `deploy_manifest.py --check` and `api_manifest.py --check`. There is
  no per-path deploy.sh route edit for storage subroutes; `deploy.sh` only
  publishes the storage service URL/config. This route is the required fallback
  for local/imported chain-only solve-score JSON in Phase 4; do not rely on
  stale hidden chips for that case.

### Phase 3 — Source becomes the sole authored input
- Keep `solve_score_chain` in the payload during transition (zero behavior change;
  backend ignores it when source is non‑blank). **Before dropping browser→planner
  chain fallback:** grep the **request/planner entrypoints** to confirm the
  browser-originating paths route through `solve_score_program_for_run` /
  `solve_score_source_text_for_run`, and confirm the text‑only editor guarantees
  non‑blank source on dispatch. Do **not** remove the compiled
  `solve_score_chain` contract from plan output, ASL, or worker/runtime handlers:
  workers still consume the planner-produced canonical chain as their execution
  input. Then drop only the five browser payload fallback fields (§1) and set
  blank=empty semantics (§3).

### Phase 4 — Frontend load simplification
- `_restoreSolveScoreSourceFromArtifact` (js/11:288): drop the "clear + chips mode"
  branch; always set the (now‑reconstructed) text and pin text. `_setSolveScoreChainFromArtifact`
  becomes redundant for display. During the Phase 1-2 transition, the chain is
  still sent as a compute fallback; after Phase 3 it is retained only as
  internal/canonical data, not as a hidden browser-authored fallback.
- Populate call sites (js/11 color/palette branches; js/09:897 palette
  orchestration) keep "set chain (fallback) + restore text".
- `_applySolveScoreProgram` (js/03): after `_parseSolveScoreProgram`, if
  `program.source_text` is present set it. Otherwise the program is an imported
  chain-only JSON (fetched programs already carry synthesized source_text), so
  reconstruct source via `POST /solve-score-chain-to-source`. If the route
  rejects the chain, clear the textarea and show an explicit non-editable warning
  that the imported chain cannot be converted to source; do not leave stale
  source text in the editor after loading/importing a chain-only program.

### Phase 5 — Remove the chip editing UI (gated on Phase 1 + Phase 2 + Phase 2b)
- **HTML (index.html):** delete both tabs rows and both chips panels' *editable*
  contents; **relocate** the three render controls (§5) under the render text
  panel; move both `*-solve-score-program-manage` buttons into a toolbar above the
  text panel; **add `active`** to `render-ss-text-panel` and `palette-ss-text-panel`.
  Mirror the param/coeff HTML shape (toolbar row → `…-text-panel …active` → status).
- **JS:** first restructure `_renderChips` (js/09:399-460) so the chip-HTML
  render body is wrapped in `if (el) { ... }` and the post-render solve-score
  sync block runs unconditionally — so the syncs survive `#ss-chips` removal for
  every caller (see §1). Then simplify
  `_setSolveScoreProgramEditorMode(prefix, _mode)` to pin text
  (or remove and replace its callers with direct text‑panel activation), mirroring
  the param/coeff mode‑fn simplification in `4039058`. Change the state default
  (js/08:402) to `'text'`. Leave the chip add/edit helpers as guarded no‑ops
  (their target ids are gone) or remove for tidiness; keep everything the modal
  uses (§1).

---

## 7. Touchpoints (verified)

| Area | File:line | Action |
|---|---|---|
| Render editor markup | index.html ~1751‑1792 | delete tabs+chips; relocate 3 controls; `active` on text panel |
| Palette editor markup | index.html ~1860‑1893 | delete tabs+chips; `active` on text panel |
| Panel CSS | index.html ~865‑870 | none (rely on static `active`) |
| Mode fn | js/08‑chip‑editors.js:461‑472 | pin to text (Phase 5); state default :402 |
| Source plumbing | js/08:409‑431, 512‑540 | keep |
| Load/restore | js/11‑artifacts.js:258‑302, 459‑466 | Phase 4 simplify; reads art fields |
| Populate sites | js/11 (color/palette branches), js/09:897 | keep set‑chain+restore‑text |
| Payload sites | js/09:990‑991, 1597‑1598; js/10:515‑516, 641‑642, 812‑813 | Phase 3 drop browser fallback chain only; keep source text |
| Relocatable controls | index.html `render-solve-score-quantile` / `render-score-normalization` / `btn-solve-histogram` | RELOCATE, do not delete |
| Post-chain sync | js/09-render-orchestration.js:399-460 | guard render body in `if (el)`; run syncs (445-459) unconditionally |
| Backend reconstruct | lambda/handler_storage.py:2087 (`_render_artifact_entry`) | Phase 2 inject |
| Chain→source route | lambda/handler_storage.py + deploy_manifest.json + api_manifest.json | add `POST /solve-score-chain-to-source` for imported chain-only JSON |
| Reconstruct primitive | lambda/solve_score_program_source.py:445 | reuse (imported at handler_storage.py:68) |
| Backend precedence | lambda/solve_score_pipeline_programs.py:12, 44 | no change |
| Vocab source of truth | lambda/solve_score_chain.py:22‑135 | drive the generator |
| Vocab generator (template) | lambda/gen_coeff_vocab.py | clone → gen_solve_score_vocab.py |
| Frontend static assets | deploy.sh `frontend_asset_keys()` | add `solve_score_vocab_js.js` |
| Modal (do not touch) | js/03‑program‑modals.js (`_applySolveScoreProgram`, chip renderers) | keep |

**No bundle changes** for Phase 2 reconstruction itself (storage Lambda already
ships and imports everything). Phase 1 adds a generated frontend asset,
deploy/static asset assertions, generated-vocab drift tests, and a predeploy
`--check`. Phase 2b adds a storage route, so it must update
`deploy_manifest.json`, regenerate `api_manifest.json`, and update route
contract tests.

---

## 8. Testing plan (concrete)

**Frontend harness (tests/test_frontend_js.sh)** — mirror the param/coeff edits in
`4039058` (which inverted the `pp-chips`/`cp-chips` asserts and added text‑panel‑active
asserts). Anchor on ids, not line numbers:
- Invert the `render-ss-tab-text` / `palette-ss-tab-text` `assertIncludes` →
  `assertNotIncludes("render-ss-tab-chips")` / `assertNotIncludes("palette-ss-tab-chips")`,
  and ADD `assertIncludes("id=\"render-ss-text-panel\" class=\"coeff-program-editor-panel active\"")`
  (+ palette).
- ADD `assertNotIncludes("id=\"ss-chips\"")`, `assertNotIncludes("id=\"palette-ss-chips\"")`.
- Invert the `ss-insert-before-btn` / `ss-insert-after-btn` asserts (chip insert UI gone).
- ADD `assertIncludes` for the relocated controls still present
  (`render-solve-score-quantile`, `render-score-normalization`, `btn-solve-histogram`).
- ADD an assert that the cheatsheet/inserter panel + `window._solveScoreVocab` load exists.
- ADD an assert that `_ssCatalog` / metric source choices hydrate from
  `window._solveScoreVocab` rather than a hand-maintained metric list.
- ADD a vm-runtime assertion that `_renderChips('ss')` still runs the solve-score
  syncs (normalization / buttons / legacy inputs) when `#ss-chips` is absent.
- **Rewrite the vm‑runtime assertion** that expects `_solveScoreProgramEditorMode.render === 'chips'`
  after `_restoreSolveScoreSourceFromArtifact` (the recently‑added populate test):
  post‑migration a chip‑based artifact should reconstruct to text and stay `'text'`.
- **KEEP** all `_compileSolveScoreChain` / `_normalizeSolveScoreChain` / catalog /
  modal‑chip / storage‑route asserts (the migration preserves that logic).

**Backend tests:**
- **Extend** `tests/test_solve_score_source_equivalence.py` corpus (it already proves
  fingerprint/program_spec equivalence) with any new representative shapes.
- **Add a direct fail‑closed unit test** to `tests/test_solve_score_program_source.py`:
  monkeypatch `compile_solve_score_program_source` to return a mismatched
  program_spec/fingerprint and assert `solve_score_source_text_from_chain` raises
  `SolveScoreProgramSourceError` with `code == "source_roundtrip_failed"` (the
  machinery is tested only at the migration layer today —
  test_program_v2_migration.py:196‑214).
- **Add a `_render_artifact_entry` reconstruction test** to `tests/test_storage_handler.py`
  (already in the predeploy subset): an artifact with a `solve_score_chain` and no
  source text yields a fingerprint‑equivalent `solve_score_program_source_text`;
  stored source wins; empty/missing chain fabricates nothing; an unserializable
  chain falls back silently (no key added).
- **Add storage route tests** for `/solve-score-chain-to-source`: valid chain
  returns source text whose compile preserves fingerprint/program_spec; empty
  chain returns 400 and does not fabricate proximity; corrupt/unserializable
  chain returns structured failure with `ok:false`; `source_roundtrip_failed`
  propagates its code.
- **Add deploy/route tests**: `tests/test_deploy_packaging.py` must assert
  `solve_score_vocab_js.js` is in `frontend_asset_keys()` and the
  `gen_solve_score_vocab.py --check` predeploy line exists. Route/config
  coverage belongs in `tests/test_api_route_contracts.py` / manifest checks:
  assert `/solve-score-chain-to-source` is represented in
  `deploy_manifest.json` → generated `api_manifest.json`.

**Predeploy subset (scripts/predeploy_check.sh) — GAP TO FIX:** it currently runs
**no** `test_solve_score_*` file (and not `test_program_v2_migration.py`). Add
`tests/test_solve_score_source_equivalence.py`, `tests/test_solve_score_program_source.py`,
and `tests/test_solve_score_chain.py` to the pytest list so the equivalence /
round‑trip / fail‑closed guarantees are gated. `test_frontend_js.sh` and
`test_storage_handler.py` already run; add `gen_solve_score_vocab.py --check`.

---

## 9. Risks & mitigations

- **Blank‑text semantics (the real behavior change).** *Mitigation:* §3 decision;
  reconstruct‑on‑load so text is populated; keep the chain during transition.
- **Discoverability regression.** *Mitigation:* Phase 1 ships and is usable before
  Phase 5 removes chips; generate the vocab so it can't drift.
- **Deleting the relocatable render controls** (`render-solve-score-quantile` /
  `render-score-normalization` / `btn-solve-histogram`). *Mitigation:* §5 — relocate,
  with a harness assert that each still exists.
- **Forgetting `active` on the two text panels** → invisible editor. *Mitigation:*
  harness assert `…-ss-text-panel … active`.
- **Reconstruction not equivalent for some chain** (`source_roundtrip_failed`).
  *Mitigation:* fail‑closed `try/except` keeps chips for that artifact through
  Phases 2‑4; Phase 5 requires an explicit no-source warning UX because editable
  chips are gone; equivalence corpus + the new fail‑closed unit test.
- **Post-render sync lost when chip containers are deleted.** *Mitigation:*
  restructure `_renderChips` to `if (el)`-guard only the render body and run the
  post-render syncs unconditionally; test it with absent chip containers.
- **Palette‑linked / associated solve scores.** *Mitigation:* Phase 2 covers
  `palette_source_*` and `associated_palette_*` chains; harness/back‑end tests
  include a palette‑source case.
- **Dropping the chain prematurely** (Phase 3). *Mitigation:* grep
  browser/request/planner entrypoints for hidden fallback use and enforce the
  non‑blank-source invariant before dropping browser→planner chain fields. Do
  not remove planner/ASL/worker runtime `solve_score_chain` reads.

---

## 10. Pinned decisions

1. **Blank text:** transition keeps chain fallback; final text-only state treats
   blank text as empty/no authored solve-score program. No indefinite hidden-chain
   fallback.
2. **Discoverability:** Phase 1 ships a generated inserter + collapsible
   reference. Autocomplete is deferred.
3. **Vocab authority:** `_ssCatalog`, metric source choices, generic metric
   choices, and the cheatsheet all hydrate from `window._solveScoreVocab`.
   Hand-maintained JS metric/source/spec tables are removed.
4. **Reconstruction:** `_render_artifact_entry` reconstructs artifact populate
   data, and `POST /solve-score-chain-to-source` handles imported chain-only JSON.
   Phase 5 cannot remove the chip editor until both paths are implemented and
   tested.
