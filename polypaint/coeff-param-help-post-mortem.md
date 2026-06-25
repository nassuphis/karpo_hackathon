# coeff-param-help Implementation Post-Mortem

**Implementation commit:** `807b39b` "Refactor param and coeff program help vocab" (24 files, +4193/−288).
**Fixes commits (re-audited):** `3760114` "Harden param coeff help contracts" then `1a0fa61` "Finish param coeff help cleanup" — together resolve the original bugs (BUG-1..6) and careless assumptions (CA-1..5). **Current worktree follow-up:** the Rejected-Forms lookup fragility is fixed, and the pre-M3 chain-oracle/calc-corpus gate now exists. Per-item status in **Resolution Update** below.
**Plan:** `coeff-param-help.md` (Milestones 0–6).
**Date:** 2026-06-25.
**Original test baseline (measured at `807b39b`):** full predeploy gate **passed** — `581 passed, 23 subtests` + all frontend JS checks OK; the 5 changed/new test files passed (`43 passed`). The original suite missed the bugs below. **Current post-fix status:** `3760114` and `1a0fa61` add gates for the resolved failures; see **Resolution Update**.

**Method:** six adversarial reviewers covered the implementation by surface (Param M1, Param M2 generator, Coeff M4, coeff chain/wire, Help/inspector frontend, wiring/gating). Findings were traced to source, and every severity-critical claim was re-run/re-verified independently. Provenance is marked **[verified]** (re-run in this session), **[worktree]** (confirmed against the parent commit `807b39b^`), or **[reviewer-VM]** (established by a reviewer running the frontend JS in a Node VM; mechanism confirmed statically where possible). Nothing here is asserted without a `file:line` trace.

---

## Resolution Update — re-audited at commit `3760114` ("Harden param coeff help contracts")

The triage items were implemented. I re-audited the same way (backend traced/run directly; the Help-layer article model re-verified by running the frontend JS in a Node VM). **Test baseline after the fixes: predeploy gate green (`581 passed` + all frontend checks); the new `tests/test_program_help_forms.py` passes; and `tests/test_frontend_js.sh` now pins the Help-layer invariants (`dup==0`, `andy` resolves to a real article, `cf` has forms, `add` preserves both offsets).**

**Correction to this post-mortem's original CA-3 finding:** the registry *was* enriched — I originally checked per-function keys and missed that the enrichment landed as **top-level `ui` and `compat` blocks**. `gen_param_vocab.py:90-103` now reads `registry["ui"]`/`registry["compat"]`, the moebius/inv hardcodes are gone, and a drift gate (`test_param_program_drift.py:168-175`) pins `vocab == registry.compat == chain side-tables` (I confirmed `compat.variable_arg_counts == chain._VARIABLE_LEGACY_ARG_COUNTS`). CA-3 is resolved, not open.

### Codex current conclusion after re-check

I re-checked the same load-bearing paths against the current worktree. The reviewer is right on the important part: `3760114` fixed the acute Help regressions rather than hiding them. Public Param parsing now reports `empty_expression` for `p1 =`, and `legacy(add, both, both, 1, 2)` preserves both offset args in the parsed chain. The Help article model no longer has the original duplicate-name/andy-shadowing failure class, and those invariants are now gated by frontend tests.

The remaining issues are different in kind. They are not the same high-severity data-loss/regression bugs; the only current user-visible regression I see is the low-severity bare-legacy-token inspector miss. The rest is cleanup and future-risk work: finish consuming the last Param profile fields (`emit_aliases`, `rejected_forms`), remove stale/dead Help helpers and dead generated payload, and do not start an M3 wire-packer rewrite until the legacy-oracle/corpus gate exists. `tests/test_program_help_forms.py` is a useful new contract because it proves concrete Help forms compile, but it is **not** a prose-quality or completeness proof; bad or misleading Help copy can still pass it.

### Status by finding (re-verified)

| Finding | Status | Evidence |
|---|---|---|
| **BUG-1** `add` data loss | ✅ **RESOLVED** | editor now consumes `variableArgCounts`; `_paramProgramLegacyTakesNoArgs('add')` is false (`js/08:1746-1755`), so `legacy(add, both, both, 1, 2)` round-trips intact; pinned by `test_frontend_js.sh`. [verified mechanism + reviewer-VM round-trip] |
| **BUG-2** empty-RHS diagnostic | ✅ **RESOLVED** | `lower_statement` override (`param_program_source.py:169-179`) restores `empty_expression`; `parse_param_program_source('p1 =')` → `empty_expression`. [verified] |
| **BUG-3** #7 andy shadowing | ✅ **RESOLVED** | band-aid `_programHelpLookupPriority` **deleted** (0 refs); param tokens no longer registered as lookup keys; dedicated `andy` article added (`js/08:1009-1012`); legacy keys namespaced `legacy:${name}`. `andy` resolves to the shared andy article; pinned. [verified band-aid removal + reviewer-VM] |
| **BUG-4** duplicate/contradictory articles | ✅ **RESOLVED** | cp/pp duplicate top-level names now **0**; `conj` merged into one article carrying both forms; Native-Transform Reference filters names already in `vector_unary`; pinned (`dup==0`). [reviewer-VM] |
| **BUG-5** dead `lower_indexed_assignment` (`p1[0]=x`) | ⚠️ **OPEN (LOW)** | still `unknown_symbol`; the override remains dead. [verified] |
| **BUG-6** `cf`/`poly` inspector | ✅ **RESOLVED** | both resolve to the form-bearing Statement-Forms article; pinned. [reviewer-VM] |
| **CA-1** profile grammar unconsumed | 🟡 **MOSTLY RESOLVED** | js/08 now derives Stack/Arithmetic/Unary from `source.stack_op_aliases`/`binary_ops`/`unary_ops`/`targetable_unary`/`unary_targets` (injecting a new op surfaces it). **Remaining:** `emit_aliases` are still hand-typed (`js/08:926-927`) and `rejected_forms` are not consumed by Help at all — 2 profile fields still outside the generated contract. [verified + reviewer-VM] |
| **CA-2** dead vocab payload | 🟡 **PARTIAL** | `variableArgCounts` now consumed (fixed BUG-1); `uiFunctions`/`categoryMeta` added and consumed. Still emitted-but-unread: `fnIndexByName`, `targetFirst`, `targetLast`, `ditherTargetFirst`, `specs`. [verified] |
| **CA-3** registry not enriched | ✅ **RESOLVED** | top-level `ui` (categories/functions/arg_specs/variable_arg_forms; 69/70) + `compat` (placement + arity) blocks; generator reads them; moebius/inv hardcodes removed; drift-gated vs chain side-tables. *Caveat:* the compiler (`param_program_chain.py`) still keeps its own side-table copies — dual storage, but drift-guarded. [verified] |
| **CA-4** M4 hardcoded fallbacks | ⚠️ **OPEN (LOW)** | `coeff_program_source.py` untouched in `3760114`; vector-op literal fallbacks (`js/07:352-353`, `coeff_program_source.py:131,135`) remain with no drift guard on the fallback path. [verified] |
| **CA-5** `type:"complex"` guardrail | 🟡 **PARTIAL / FUTURE-RISK** | no registry arg currently uses `type:"complex"` (verified), and the current complex source forms for `linear`/`exp_affine`/`round` are already wire-golden-tested in `m3_wire_corpus.json`. Remaining risk is a future registry edit adding a generic complex arg without extending the wire corpus/packer gate. |
| Sloppiness — `_coeffProgramParamDefs` phantom | ⚠️ **OPEN** | still referenced (`js/08:990`), defined nowhere. [verified] |
| Sloppiness — 4 orphaned cheat→help builders | ⚠️ **OPEN** | still dead (definition-only). [verified] |
| Sloppiness — `_programHelpLookupPriority` band-aid | ✅ **RESOLVED** | deleted (0 refs). [verified] |
| Sloppiness — `structural_chips.json` parsed 2× | ⚠️ **OPEN (LOW)** | `coeff_program_source.py` untouched. |
| Process — wire corpus / oracles | 🟡 **PARTIAL** | `tests/fixtures/coeff-wire-corpus/m3_wire_corpus.json` added (5 source cases + 4 chain cases) + wire test expanded; still **no** `coeff_program_chain_legacy.py` / `param_program_source_legacy.py` oracle, not `calc.json`-corpus-driven. [verified] |

### New issue introduced by the rework

- **NEW (LOW) — a bare Param legacy op name no longer resolves in the inspector.** Because legacy articles are re-keyed `legacy:${name}` (`js/08:832`), double-clicking a bare legacy op token (e.g. `moebius` inside `legacy(moebius, …)`) now resolves to nothing ("No generated help"). This is the deliberate trade that stopped legacy names shadowing canonical ops (a net win), and the names still appear in the Help tab's Legacy Transform Reference — but the dblclick-on-token path regressed for legacy ops, and no test pins it. [reviewer-VM; `legacy:` keying confirmed statically] **→ ✅ RESOLVED in `1a0fa61`** (bare-name → `legacy:${name}` alias added and guarded; see Cleanup Update).

### Net

5 of 6 bugs fixed (BUG-5 low/open), the central CA-1 gap largely closed (binary/unary/stack now profile-derived and dynamically proven), CA-3 fully resolved (registry enriched + drift-gated), and the #7 band-aid removed. Remaining is a low-severity tail: BUG-5, the `emit_aliases`/`rejected_forms` end of CA-1, dead-payload/dead-code cleanup (CA-2 + phantom + orphaned builders), CA-4 fallbacks, CA-5 future-registry guardrails, the new legacy-name inspector regression, and the still-missing M0 chain/Param oracles (required before any M3 packer rewrite).

### Cleanup Update — commit `1a0fa61` ("Finish param coeff help cleanup")

`1a0fa61` closed essentially the entire remaining list (re-verified: backend run directly; Help model re-run in a Node VM; predeploy gate green). Status changes from the table above:

| Item | Now | Evidence |
|---|---|---|
| **CA-1 tail** (`emit_aliases`/`rejected_forms`) | ✅ **RESOLVED** | `emit` article forms now derived from `source.emit_aliases` (`js/08:891-901`); a new "Rejected Forms" Help section is built from `source.rejected_forms` (`js/08:946-957`). Dynamically proven (injecting fake entries surfaces them). No Param grammar field is hand-typed in Help anymore. |
| **Legacy-name inspector regression** | ✅ **RESOLVED** | bare-name → `legacy:${name}` alias added (`js/08:813-817,830`), guarded so the 9 grammar-colliding names (`add`/`conj`/…) keep resolving to the canonical article. `_lookupProgramHelpToken('pp','moebius')` → `legacy:moebius`; `'add'` still → canonical. |
| **BUG-5** (`p1[0]=x`) | ✅ **RESOLVED** | `lower_assignment` now delegates indexed-lhs to the base (`param_program_source.py:182-184`); `p1[0] = 3` → "p1[...] is not valid Param Program source" (the intended message). |
| **CA-2** (dead payload) | ✅ **RESOLVED** | generator stopped emitting the 6 unread fields; `param_vocab_js.js` 2609 → **979** lines. The 7 remaining fields are all consumed. |
| **CA-4** (M4 fallback literals) | ✅ **RESOLVED** | `_coeffStructuralSubOpNames` dropped its literal fallbacks (`js/07:349-356`); the Python helpers now **raise** on a missing/empty `structural_chips.json` (`coeff_program_source.py:93,104`) instead of silently reverting to a hand-list. Vector op names verified set-equal to the old literals. |
| **CA-5** (`type:complex` guardrail) | ✅ **RESOLVED** | guard test `test_coeff_registry_has_no_unpinned_generic_complex_args` (`tests/test_coeff_program_drift.py:370`) fails the build if any registry arg becomes `type:"complex"` without updating the packer/wire corpus. |
| **Phantom `_coeffProgramParamDefs`** + **4 orphaned cheat→help builders** | ✅ **RESOLVED** | all removed (0 repo-wide refs). |

**One new low-severity item surfaced (latent, not a live bug):** the Rejected-Forms entries normalize to the same lookup token as their canonical article (`push(both)`→`push`, `emit(p1)`→`emit`), so the inspector picks the right one only by `_programHelpLookupScore` margin (push 38 vs 9; emit 32 vs 9), not by structure. Correct today; a future form-poor canonical article could be shadowed by its own "rejected" stub. Untested (`js/08:746-779`). **→ ✅ RESOLVED in current worktree:** rejected-form articles are marked `lookup:false`, `_programHelpAddSection` skips them when building the lookup index, and `tests/test_frontend_js.sh` pins that `push(both)`/`emit(p1)` are display-only entries. [verified at `adfa918`: `lookup:false` at `js/08:736/749/954`]

**Pre-M3 oracle/corpus gate:** no longer deferred in current worktree. `lambda/coeff_program_chain_legacy.py`, `lambda/param_program_chain_legacy.py`, `lambda/program_source_core_legacy.py`, and `lambda/param_program_source_legacy.py` are frozen, first-line-marked test oracles; `lambda/coeff_program_source_legacy.py` now compiles through the frozen chain oracle. `tests/fixtures/program-m3-oracle/` contains calc-shaped source/chain fixtures, and `tests/test_program_m3_oracles.py` compares current vs frozen parsed chains, execution specs, fingerprints, token/scalar counts, and Param chain→source regeneration. The test is wired into `scripts/predeploy_check.sh` and asserted by `tests/test_deploy_packaging.py`. This is now the hard prerequisite gate before any M3 wire-packer rewrite.

**Latest code-review correction:** the prior wording overclaimed oracle purity. Re-checking the imports shows:

- `coeff_program_chain_legacy.py` is the frozen Coeff chain/wire oracle. That is the load-bearing reference for an M3 wire-packer rewrite.
- `param_program_source_legacy.py` uses frozen Param chain and frozen source-core copies, so Param source/chain comparison is genuinely isolated from current Param source/chain edits.
- `coeff_program_source_legacy.py` intentionally imports live `coeff_program_source as _current` for semantic lowering and only freezes the old source-shell plus frozen chain compile. That is enough to compare old shell/new shell and to catch chain/wire divergence, but it is **not** a fully frozen Coeff source-semantic oracle. If a future change rewrites Coeff source semantic lowering, freeze those lowerers too before claiming old-vs-new source equivalence.

With that correction, the gate is still useful and correctly predeploy-gated, but its safe claim is narrower: it protects the M3 chain/wire-packer seam and Param source migration; it does not by itself prove arbitrary future Coeff source-lowering rewrites.

**Net after `1a0fa61` plus current follow-up:** every bug (BUG-1..6) is fixed, every careless assumption (CA-1..5) is resolved, all flagged dead code is gone, and the prior cycle's legacy-name regression is fixed. The implementation now matches the plan's intent: Help is generated-data-driven, gated, and free of the drift class the project set out to kill. The rejected-form lookup collision is structurally removed, and the pre-M3 equivalence oracle/corpus gate is present and predeploy-gated.

### Codex final re-check after `1a0fa61`

I agree with the new review's main conclusion. I re-checked the load-bearing code paths against current `main`:

- Param Help consumes generated `emit_aliases` and `rejected_forms` rather than hand-maintaining those forms (`js/08:892-957`).
- Bare legacy lookup is fixed without stealing grammar names: legacy aliases are added only when they do not collide with Param grammar names (`js/08:815-830`).
- Param indexed assignment now reaches the intended indexed-lhs branch (`param_program_source.py:181-189`).
- Dead Param vocab payload is gone; `param_vocab_js.js` now carries only consumed fields (`argSpecs`, `targetArgIndexes`, `variableArgCounts`, `independentTargets`, `categoryMeta`, `uiFunctions`, `names`), and `test_param_program_drift.py` asserts the removed fields stay removed.
- Coeff structural fallback lists are gone; the browser reads generated structural chips, and the backend parser loads `structural_chips.json` once and fails loudly on a missing family. The deployment packaging consequence is covered: every bundle with `coeff_program_source.py` now ships `structural_chips.json`, and `test_deploy_packaging.py` asserts that contract.

My only disagreement is with reading "resolved" as "nothing else to do." It means the original implementation defects are fixed. It does **not** mean M0 is complete: there is still no frozen legacy parser/chain oracle and no saved-`calc.json` equivalence corpus. That is acceptable only because M3 was not attempted. If the next work is the wire-packer rewrite, the oracle/corpus becomes a hard gate, not documentation cleanup.

The Rejected-Forms token-collision item was real but low risk. It is fixed in the current worktree by marking rejected stubs as display-only (`lookup:false`) and pinning that behavior in the frontend JS harness.

### Current follow-up after requested items 1 and 2

Implemented:

- Rejected-form Help rows are display-only: they still render in the Help tab, but they no longer register lookup keys that can compete with canonical `push`/`emit` articles.
- Frozen M3 oracles exist for the risky parser/packer seam: `coeff_program_chain_legacy.py`, `program_source_core_legacy.py`, `param_program_chain_legacy.py`, and `param_program_source_legacy.py` are marked test-only legacy oracles. `coeff_program_source_legacy.py` is a frozen shell oracle that still uses live Coeff semantic lowerers by design; do not treat it as a complete old Coeff source implementation.
- The M3 corpus is calc-shaped and checked in under `tests/fixtures/program-m3-oracle/`; it includes source-form and persisted-chain cases covering Param source, Param chain regeneration, Coeff source native transforms, indexed Coeff source, and legacy component chains.
- `tests/test_program_m3_oracles.py` compares current and frozen outputs over the corpus and is included in predeploy. This is now the required gate before touching any Coeff M3 wire-packer rewrite.

Boundary: this is a representative checked-in calc corpus, not an automated export of every live S3 `calc.json`. If the M3 rewrite starts and we want production-wide assurance, the next step is to add a separate corpus-harvest script that snapshots real saved calc files into this fixture format before rewriting the packer.

Oracle boundary: `program_source_core_legacy.py` still reads the current generated `program_profiles` artifact. That is deliberate: profile drift is covered by the generated-profile `--check` gates and profile-drift tests, while this oracle pins the source/chain lowering and compiled wire/fingerprint seam. If M3 ever mutates profile semantics in the same change, freeze a `program_profiles_legacy.py/json` copy and repoint the oracle before comparing old vs new. Separately, if Coeff source semantic lowering is rewritten, freeze the Coeff lowerer code now imported through `_current` before treating `coeff_program_source_legacy.py` as a full old-source oracle.

---

## Codex follow-up reconciliation

*(All six triage items below were implemented in `3760114`; see the Resolution Update above for per-item verification. This section is preserved as the pre-fix record.)*

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

The **data-generation layer was solid from the start**; the **Help-consumption layer was the weak point — and commit `3760114` repaired most of it.** As originally found (`807b39b`), the Help layer had a new data-loss bug (`add`), an unfixed #7 andy-shadowing, duplicate/contradictory articles, and a profile grammar that was ~90% ignored. **After `3760114` (see Resolution Update above): `add` is fixed, the #7 band-aid is gone and `andy` resolves to a real shared article, duplicates are eliminated, and Param Help now derives stack/binary/unary forms from `profiles.param.source` — the central thesis is now substantially met.** What remains is a low-severity tail: `emit_aliases` still hand-typed, `rejected_forms` still unconsumed by Help, dead payload/dead code not yet removed, the M4 fallback literals still present, future `type:"complex"` registry additions still needing a wire-oracle guard, one new low-severity legacy-name inspector regression, and the M0 equivalence oracles still unbuilt (needed before an M3 packer rewrite). The backend (M1/M2/M4 + generators + gating + wire fingerprints) was and remains correct and well-gated.

> The sections below are the **original as-found analysis against `807b39b`**. Current per-item status is in the **Resolution Update** above.

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

> **Status of each item is in the Resolution Update above** — BUG-1/2/3/4/6 are ✅ RESOLVED in `3760114`; BUG-5 remains OPEN (low). The text below is the original as-found analysis.

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

> **Status in the Resolution Update above** — CA-3 is ✅ RESOLVED (registry enriched via top-level `ui`/`compat` blocks, drift-gated); CA-1 is 🟡 mostly resolved (stack/binary/unary now profile-derived; `emit_aliases`/`rejected_forms` remain); CA-2 🟡 partial; CA-4/CA-5 OPEN (low). Text below is the original as-found analysis.

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

## Remaining work — none open

All findings across the four audit cycles are resolved (re-verified each cycle: backend run directly, Help model via Node VM, oracle integrity by construction):

- **`3760114`** — BUG-1/2/3/4/6, CA-3, core of CA-1.
- **`1a0fa61`** — CA-1 tail, BUG-5, CA-2, CA-4, CA-5, the legacy-name inspector regression, all dead code.
- **`adfa918`** — the Rejected-Forms lookup collision (structural: `lookup:false`) and the pre-M3 frozen-oracle + `calc.json`-corpus equivalence gate.

Nothing on the original bug / careless-assumption / sloppiness list remains open. The predeploy gate is green and now includes the oracle gate (`tests/test_program_m3_oracles.py`).

**Forward status after the M3 follow-up:** the first wire-compatible M3 packer rewrite has now landed for the forward Coeff packer. `linear`/`exp`/`round`/`pow` declare `compat_signatures` in `coeff_legacy_registry.json`, `coeff_program_chain.py` uses one signature interpreter instead of the old per-function forward helpers, and `tests/test_coeff_program_drift.py` pins that architecture. The frozen oracle and golden fingerprint tests are green, so the checked corpus keeps byte-identical `execution_spec`/`fingerprint`. Remaining boundary: the corpus is still representative rather than production-wide; before calling M3 ship-complete, run `scripts/harvest_program_m3_oracle_corpus.py` against saved `calc.json` payloads and include the harvested cases or a curated production subset.
