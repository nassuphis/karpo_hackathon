# coeff-param-help Implementation Post-Mortem

**Implementation commit:** `807b39b` "Refactor param and coeff program help vocab" (24 files, +4193/−288).
**Plan:** `coeff-param-help.md` (Milestones 0–6).
**Date:** 2026-06-25.
**Test baseline (measured):** full predeploy gate **passes** — `581 passed, 23 subtests` + all frontend JS checks OK; the 5 changed/new test files pass (`43 passed`). **Every bug in this document is uncaught by that suite.**

**Method:** six adversarial reviewers covered the implementation by surface (Param M1, Param M2 generator, Coeff M4, coeff chain/wire, Help/inspector frontend, wiring/gating). Findings were traced to source, and every severity-critical claim was re-run/re-verified independently. Provenance is marked **[verified]** (re-run in this session), **[worktree]** (confirmed against the parent commit `807b39b^`), or **[reviewer-VM]** (established by a reviewer running the frontend JS in a Node VM; mechanism confirmed statically where possible). Nothing here is asserted without a `file:line` trace.

---

## Codex follow-up reconciliation

I re-checked the load-bearing claims after reading this post-mortem. The core
verdict is correct: the backend/profile/generator work is materially better
than the old state, but the Help UI still violates the architecture's main rule:
**valid source forms must be consumed from generated grammar/registry data, not
retyped in Help code.**

Additional verification performed:

- `p1 =` currently returns diagnostic code `source_error`; the pre-refactor
  contract was `empty_expression`. The post-mortem's BUG-2 is real.
- `param_vocab_js.js` emits `variableArgCounts.add=[0,1,2]`, but the JS editor
  does not consume `variableArgCounts`; `add` has no `argSpecs` entry. BUG-1 is
  real and is the highest-priority fix because it can silently drop user input.
- A direct Node VM inspection of `_programHelpRegistry` confirms duplicate
  article names: `cp` duplicates `cf`, `poly`, `conj`, `sin`, `cos`, `tan`,
  `sinh`, `cosh`, `tanh`; `pp` duplicates `add`, `swap`, `square`, `cube`,
  `exp`, `negate`, `reciprocal`, `unit_circle`.
- The same VM inspection confirms the inspector lookup bug: `cp` lookup for
  `andy` resolves to the `linear` article, and `cf` resolves to the form-less
  Core Symbols article. BUG-3/BUG-6 are real.
- `_programHelpLookupPriority` is indeed a band-aid, not architecture. It is
  only accidentally useful for Param legacy rows and does not fix the Coeff
  `andy`/duplicate article model.

My conclusion: do **not** proceed to the Coeff M3 wire-codec rewrite until this
Help/registry consumption layer is repaired. The M3 rewrite is fingerprint-risk
work; this post-mortem shows the current problem is not lack of codec
abstraction, it is that generated metadata is not treated as executable UI
contract. Fix the contract first.

Implementation triage:

1. Fix `add` immediately by either emitting/consuming a real `argSpecs.add`
   entry or making the editor consume `variableArgCounts` generically. Add a
   test that serializing `legacy(add, ..., re, im)` preserves both args.
2. Replace hand-typed Param Help op lists with `profiles.param.source` consumers
   for stack ops, binary ops, unary ops, emit aliases, and rejected forms.
3. Rebuild the Help registry as one article per semantic operation. Aliases and
   parameter tokens should point to that operation; they should not create
   competing articles.
4. Give `andy` one shared article and have each transform parameter link to it
   without letting `andy` resolve to an arbitrary transform.
5. Restore or explicitly approve the `empty_expression` diagnostic change.
6. Remove fallback literals/dead payloads only after tests prove every consumer
   is reading the generated source of truth.

---

## Verdict

The **data-generation layer is solid**; the **Help-consumption layer is where the project fell short.** Backend M1/M2/M4 + the generators + gating/packaging are correctly implemented, fingerprint-preserving, and well-gated — that work is shippable. But the frontend Help layer (M5) did the easy 80% (move the *names* and *vector-op* lists to generated vocab) and skipped the part that was the actual point: **wiring Help to read valid forms from the generated grammar.** The plan's central thesis — "a form is valid iff the generated artifact says so; Mistakes #4/#5/#6 become structurally impossible" — is **not achieved for Param** (the profile grammar it generates is 90% ignored), the named #7 andy-shadowing bug is **not fixed** (only made silent), and a **new data-loss regression** (`add`) was introduced. Net: the refactor improved the backend but left the user-facing Help with the same class of drift it set out to kill, plus one fresh bug.

---

## Scope: what was actually implemented

| Milestone | Status |
|---|---|
| **M1** Param grammar profile + shared lowerer | **Done** — full migration to `ProfileStatementLowerer`; op sets derived from `profiles.param.source`; Coeff-safe (Param-local override, no shared-base mutation) |
| **M2** Param vocab generator | **Done** — `gen_param_vocab.py` → `param_vocab_js.js`; all 70 names exposed; old JS hand-lists deleted (one new bug, BUG-1) |
| **M4** Coeff structural vocab | **Done** — vector-op membership reseeded from `structuralChips`; real/imag bug fixed; `_lower_call` ladder intentionally kept |
| **M5** Help article model + inspector | **Partial / weakest** — names moved to vocab, but grammar forms still hand-typed; #7 not fixed; duplicate/contradictory articles |
| **M6** dead-code deletion + gating | **Mostly done** — handler routed through public API; new generator gated/packaged/load-ordered correctly; some dead code left |
| **M3** Coeff andy/complex wire codec | **Correctly NOT attempted** — packers untouched; only additive registry metadata landed |
| **M0** equivalence harness | **Partial** — a real golden-fingerprint test landed (coeff, 6 forms); the chain/Param legacy oracles and saved-`calc.json` corpus were **not** built |

---

## Bugs (severity-ranked)

### BUG-1 — MED · Data loss: `legacy(add, …)` silently drops its offset arguments  *(NEW regression)* **[verified]**
The `add` registry transform accepts 0/1/2 args server-side (`param_program_chain.py:908-918`), but the frontend now serializes it to zero args, dropping any offset.

Root cause: `gen_param_vocab.py:_special_arg_specs` injects hardcoded arg specs for `moebius` (`:65`) and `inv_t_plus_2` (`:72`) — whose registry `args` is `[]` — **but forgot `add`** (also `args:[]`). Confirmed in the generated artifact: `argSpecs` contains `moebius`/`inv_t_plus_2` but **not `add`**. So `_paramProgramLegacyTakesNoArgs('add')` returns true (`js/08-chip-editors.js:1723`) → `_paramProgramLegacyArgsFromInput` returns `[]` (`:1809`) → the serializer emits `['legacy','add',src,tgt]` with no offsets.

The data needed to fix it is *already emitted* — `variableArgCounts.add=[0,1,2]` is in `param_vocab_js.js` — but **`variableArgCounts` has zero consumers across all 13 JS files** (verified), so nothing reads it. This is precisely the "Help that misrepresents the grammar" the project set out to kill, freshly introduced. Among the 22 newly-exposed transforms, `add` is the only one with arg-bearing semantics and no `argSpecs`.

*Blast radius:* reachable only via an explicit `legacy(add, …, re, im)` chip; the bare `add` chip resolves to the native `a+b` stack op (`_ppCatalog`, `js/07:137`), not the registry transform. Narrow, but a real frontend↔backend contract mismatch with silent data loss.

**Fix:** add `add` to `_special_arg_specs` (or have the editor consume `variableArgCounts`), and add a drift-test assertion pinning `add`'s editor representation.

### BUG-2 — MED · Diagnostic-contract regression: empty-RHS assignment code changed `empty_expression` → `source_error`  *(unapproved)* **[verified]**
Trigger: compile `p1 =` (empty/whitespace RHS). Old parser raised `code="empty_expression"`; new parser returns `['source_error']` (verified by running it). The base `ProfileStatementLowerer.lower_statement` (`program_source_core.py:347-352`) intercepts empty RHS *before* dispatching to Param's override, so Param's `_canonical_expr` empty-guard never runs.

The plan's M1 work item says "Preserve Param's current diagnostic contract unless a test explicitly approves a change." No test approves this; the new `test_profile_backed_rejected_forms_keep_existing_diagnostics` locks the five *flagged* codes but not empty-RHS. The five named codes (`read_only_symbol`, `unknown_symbol`, `noncanonical_emit`, `bad_selector`, `empty_source`) **all survive** [verified] — but this sixth, unflagged code silently regressed.

### BUG-3 — MED · The #7 andy-shadowing bug is not fixed — the symptom became silent **[reviewer-VM + mechanism verified]**
The plan's named bug (#7: `sort_mod_keep_angle`'s per-param `andy` lookup key collides with the standalone `andy` article). The fix shipped was to **delete the standalone `andy` article** rather than give `andy` one shared resolvable article (the plan's Rule 4). Result (reviewer ran it in a VM): double-clicking **`andy`** in the Coeff inspector now opens the **`linear`** transform's article — a *silent wrong resolution* replacing a *visible collision*.

The band-aid that was supposed to help — and that the plan said to **delete** — was kept and is **dead for Coeff** [verified]: `_programHelpLookupPriority` (`js/08:773`) guards on `category.includes('native transform')`, but coeff native transforms get `category = spec.category || 'native'` (`js/08:881`) → the literal is `'native'`/`'accumulation'`/`'elementwise'`, never `'native transform'`, so the guard matches **0 of 28** coeff items. It fires only for Param legacy items (`'legacy transform…'`, `:843`) by prefix coincidence. So the priority hack is dead-for-coeff, works-by-luck-for-param, and should have been removed.

### BUG-4 — MED · Duplicate and contradictory Help articles (violates plan Rules 5/6) **[reviewer-VM]**
Live registry inspection found **9 duplicated top-level article names in `cp`** (`cf`, `poly`, `conj`, `sin`, `cos`, `tan`, `sinh`, `cosh`, `tanh`) and **8 in `pp`**. Worse than duplication: they **contradict** — `conj` appears once as a Vector-Op article (`poly = conj(poly)`, no andy) and once as a Native-Transform article (`poly = conj(poly, 0)`, with andy). Cause: the same op is emitted into two Help sections (vector sub-ops *and* native transforms; Core-Symbols *and* Statement-Forms). This is the "two sources feeding Help that disagree" the plan's article model ("one operation, one article") was meant to prevent — reintroduced by the new producers.

### BUG-5 — LOW · Dead `lower_indexed_assignment` override; `p1[0]=x` gets the wrong error **[verified by reviewer]**
`ParamStatementLowerer.lower_assignment` (`param_program_source.py:168`) doesn't call `super()`, so the base's indexed-lhs path (the only caller of the override at `:172`) never runs. `p1[0] = 3` returns `unknown_symbol` ("unknown … symbol: p1[0]") instead of the override's intended message. Still rejected, just with a worse diagnostic; the override is dead code.

### BUG-6 — LOW · `cf`/`poly` inspector resolves to the form-less article **[reviewer-VM]**
Double-clicking `cf`/`poly` in `cp` returns the "Core Symbols" entry (`forms: []`) rather than the richer "Statement Forms" entry (`forms: ['cf']`). The dual-registration tiebreak picked the worse article.

---

## Careless assumptions

### CA-1 — "Generating the data" was treated as equivalent to "Help consuming it." It isn't. **[verified]**
The plan's core unlock was Help reading valid forms/aliases/rejected-forms from `profiles.param.source`. The profile **carries** `binary_ops`, `unary_ops`, `stack_op_aliases`, `emit_aliases`, and `rejected_forms` (the generator emits them). But js/08 reads **only** `source.push_sources` (`:924`) and `source.unary_targets` (`:964`) — the other grammar is still **hand-typed in js/08:947-968**. So "Mistakes #4/#5/#6 structurally impossible" is **not achieved for Param**: add a unary/binary op in `param_program_source.py` and the Param Help list silently won't show it — the exact drift class the project exists to kill, reintroduced in narrower form. This is the single most important gap: the refactor produced the artifact but didn't wire the consumer to it.

### CA-2 — Emitting metadata was assumed to be consuming it; 2/3 of the generated file is dead payload. **[verified]**
Of 11 fields in `window._paramRegistryVocab`, only **4 are read by any JS** (`names`, `argSpecs`, `targetArgIndexes`, `independentTargets`). **7 are emitted but never consumed**: `fnIndexByName`, `specs`, `variableArgCounts`, `targetFirst`, `targetLast`, `ditherTargetFirst`, `programProfiles` (the last duplicates `program_profiles_js.js`, already shipped separately). This is most of why `param_vocab_js.js` is 2609 lines — and the dead `variableArgCounts` is the direct enabler of BUG-1 (the fix data is present but unread).

### CA-3 — "Single source of truth" was only partly realized; the registry was not enriched. **[verified]**
The plan (revised) said: enrich `param_legacy_registry.json` with `ui`/`category`/`desc`/`target_placement`/variable-arity so the generator is single-source. That enrichment did **not** happen. Instead:
- `moebius`/`inv_t_plus_2` arg specs are **hardcoded in the generator** (`gen_param_vocab.py:64-77`) because the registry still says `args:[]` — a Python-side duplicate of intent that can drift from the VM independently (the drift test pins moebius's count but not inv_t_plus_2's, and not add's — BUG-1).
- Target-placement/variable-arity metadata is read from the **chain side-tables** (`param_program_chain.py`), and category/desc prose from **JS `_ptInfo`/`_ppCatalog`**. So arg-shape/placement/prose are sourced from three different non-registry homes; the generator collates them rather than the registry being authoritative.

### CA-4 — The M4 "no hardcoded arrays remain" gate was met in spirit, not in letter. **[reviewer]**
The vector-op literals were demoted to **fallbacks**, not deleted: `js/07:352-353` still contains the full 16-name unary / 5-name binary literals as arguments to `_coeffStructuralSubOpNames(...)`, and `coeff_program_source.py:131,135` keep chain-constant fallbacks. The live path is metadata-driven and tested, but the fallback literal has **no drift guard** — if `structural_chips.json` fails to load, the code silently reverts to a hand-list that could be stale, the exact failure mode the milestone targeted. The gate text ("No frontend hardcoded vector-op arrays remain") is literally false.

### CA-5 — The widened arg-type set removed a guardrail with no replacement. **[worktree]**
`coeff_program_chain.py:319` now accepts `type:"complex"` in the loader, but the M3 codec that would pack it doesn't exist. A `complex` arg flows through the generic `_legacy_args` packer into `spec["args"]` and onto the wire. Harmless **today** (no registry arg uses it; registry unchanged) — but a future registry edit adding `type:"complex"` would silently move fingerprints with no gate. There is no test asserting "no registry arg is `type:"complex"` until the codec lands."

---

## General sloppiness

- **`_coeffProgramParamDefs` phantom still referenced** at `js/08:984` (`typeof === 'function'` guard), **defined nowhere** [verified]. The plan explicitly called this dead reference out; the refactor moved the surrounding code into `_coeffTransformParams` but left the phantom.
- **Four orphaned cheat→help builder functions** left after the refactor removed their callers: `_programHelpItemsFromCheatSection`, `_paramProgramChipHelpItem`, `_programHelpParamItemsFromCheatSection`, `_programHelpCoeffItemsFromCheatSection` (js/08:788/800/852/889) [reviewer].
- **`structural_chips.json` is fully parsed twice at module import** (`coeff_program_source.py:90` and `:107`), plus again in the chain layer — three parses of one file, the same "re-read in N places" the plan complains about, at smaller scale [reviewer].
- **`programProfiles` re-embedded** in `param_vocab_js.js` despite already shipping as `program_profiles_js.js` [verified].
- **Dead `_programHelpLookupPriority` band-aid kept** (BUG-3) though the plan said to delete it.

---

## What was done right (for fairness — most of the backend is solid)

- **Wire fingerprints are preserved.** [worktree] The packers (`_affine_pair_legacy_args`/`_linear`/`_pow`/`_exp`/`_round_legacy_args`, `_token`, `_execution_spec`, `_fingerprint`) are textually untouched; the `coeff_program_chain.py` +16 is **additive metadata only** (`effective_args`, `compat_signatures`, accept `complex` type), write-only on the wire path (only `gen_coeff_vocab.py` reads it). Rebuilding at `807b39b^` reproduced all 8 golden fingerprints + execution-spec hashes exactly.
- **A real golden-fingerprint gate landed.** `tests/test_coeff_wire_fingerprints.py` pins **hardcoded golden hex** for `fingerprint` and `execution_spec` SHA-1 (not relative `A==B`) — exactly the "golden snapshots" the plan said were missing — and the goldens are anchored to pre-refactor behavior [worktree]. Coverage is partial (6 forms: old four-real linear/pow, exp, round both forms, affine-with-imaginary, littlewood; **omits** the new 2-complex linear/pow forms and assignment-form affine, and isn't the saved-`calc.json` corpus).
- **AP-2 genuinely fixed.** All 70 registry names reach the frontend (the 22 previously-dropped transforms verified present); the old hand-typed 48-name list and arg-spec tables were deleted.
- **The real/imag live bug is fixed and pinned.** Coeff vector-unary now derives from `structuralChips` (16 ops incl. `real`/`imag`); locked by `test_generated_coeff_vocab_exposes_all_vector_unary_subops` asserting set-equality with `chain.VECTOR_UNARY_OPS`.
- **M1 is Coeff-safe and fingerprint-preserving.** No mutation of the shared base class; Param supplies a local `lower_assignment` override; 5/6 diagnostic codes survive and are test-locked; a 13-program old-vs-new battery matched fingerprints exactly.
- **Gating/packaging/load-order/handler wiring are all correct.** `gen_param_vocab.py --check` is in predeploy; `param_vocab_js.js` loads (index.html:3194) before its consumers (js/07 :3206, js/08 :3207); it's in `deploy.sh` packaging and asserted by `test_deploy_packaging.py`; `handler_storage.py` routes Param compile through the public `compile_param_program_source` (M6). The full gate is green.

---

## Process gaps (plan gates that are unmet as written)

- **No Param fingerprint-equivalence corpus.** The M1 gate "Param chain/fingerprint equivalence holds for the corpus" / "round-trips for corpus chains" has no implementing oracle (`param_program_source_legacy.py` doesn't exist) and no saved-program corpus. Equivalence currently rests on a reviewer's ad-hoc 13-program battery, not a committed gate.
- **The wire gate is hand-written, not corpus-backed**, and omits the new clean-complex `linear`/`pow` forms — the highest-risk forms for the *future* M3 packer rewrite.

---

## Prioritized fixes

1. **BUG-1 (`add` data loss):** add `add` to `gen_param_vocab.py:_special_arg_specs` (or wire the editor to consume `variableArgCounts`); pin it in `test_param_program_drift.py`. *Real user-visible data loss.*
2. **CA-1 (Help ignores the profile grammar):** make js/08 read `source.binary_ops`/`unary_ops`/`stack_op_aliases`/`emit_aliases`/`rejected_forms` instead of the hand-typed lists at `js/08:947-968` — otherwise the project's central goal is unmet for Param.
3. **BUG-3 (#7 not fixed):** give `andy` one shared resolvable article (plan Rule 4) and **delete** the dead `_programHelpLookupPriority` band-aid; assert in `test_frontend_js.sh` what `andy` resolves to.
4. **BUG-4 (dup/contradictory articles):** dedupe the two Help producers so each op yields one article; add a Rule-6 audit (no duplicate top-level names; andy-bearing ops documented one way).
5. **BUG-2 (empty-RHS diagnostic):** restore `empty_expression` for `p1 =` (override the base empty-RHS branch) or explicitly approve the change with a test.
6. **Cleanup:** remove the `_coeffProgramParamDefs` phantom (js/08:984), the 4 orphaned cheat builders, and the 7 dead vocab fields; cache the `structural_chips.json` parse; add a test asserting no registry arg is `type:"complex"` until the M3 codec exists (CA-5).
7. **Process:** if M3 (packer rewrite) is next, first build `coeff_program_chain_legacy.py` + the saved-`calc.json` corpus and extend the wire test to the new complex forms — the plan's own prerequisite.
