# CR18 — Implementation Post-Mortem / Code Review

Companion to `cr18-implementation-plan.md` (the design) and `code-review-18.md` (the why). This reviews the **as-built** implementation against the plan.

**Scope reviewed.** `git dbbc10f..bb68183` (the plan commit → "Update CR18 implementation status"): **146 files, ~10.8K insertions / ~1.4K deletions, 19 commits**, covering Phases −1 through 5. All planned new artifacts exist (`program_profiles.{json,py}`, `program_source_core.py`, `structural_chips.json`, `solve_score_eval.py`, `merged_opcodes.{h,py,json}`, `param_program_source.py`, `coeff_program_source_legacy.py`, `root_legacy_registry.json`, `program_v2_translate.py`, `gen_{merged_opcodes,program_profiles}.py`).

**Method.** Per-phase review (one reviewer per phase) + independent re-verification of every critical/high finding by tracing the code, + a pure-Python gate run. Findings are marked **[verified]** (I traced the call path) or **[reported]** (reviewer-surfaced, spot-checked).

**Gate status.** Core pure-Python gates green: `86 passed, 15 subtests` (drift ×4, coeff-source equivalence, v2 migration, param source/storage, root registry, solve-score chain). Reviewers also ran, green: the Phase −1 oracle trio (native parity + whole-sweep byte oracle + bench, builds `sweep_cli.c` from source), 13 v2-migration tests, 10 coeff-equivalence tests. Native-binary parity/visual suites were **not** run in this review (require built ARM binaries / Docker).

---

## Verdict

**The implementation is faithful and substantially correct.** The two highest-risk phases — the native VM merge (2A) and the lag centralization (2B) — are **FP-clean and bit-faithful** to the old interpreters (verified: lowered-expr executor is character-identical, signed-zero `atan2` canonicalization preserved, lag warm-up/buffer-separation equivalent). The opcode allocation, `translate_from_old`, root registry, migration routes, the shared solve-score version helper, the frozen coeff equivalence oracle, and the Phase −1 oracle are all real and match the plan.

**But it is not "done-done".** There are **two HIGH-severity correctness regressions** on *legacy-artifact reuse paths* — exactly the paths the green tests don't cover — plus a cluster of MEDIUM gaps where the plan's intent was only half-realized (param parser didn't migrate onto the shared core; equivalence corpus lacks the required saved-program snapshot; macro-DAG isn't transitive; one dual-read site). Fix the two HIGHs before relying on this for old renders.

---

## Findings by severity

### HIGH

**H1 — Solve-score `chain_fingerprint` hash-validation ignores the stored `spec_version` (breaks reuse of pre-v2 artifacts).** [verified]
`_validate_artifact_chain_fingerprint` recomputes `compiled_solve_score_fingerprint(compiled)` with **no version arg** (`handler_solve_proximity.py:403`; called for clips `:412` and sections `:729`). `compiled_solve_score_fingerprint(compiled, version=None)` defaults to `normalize_solve_score_spec_version(None)` → `SOLVE_SCORE_SPEC_VERSION = 2` (`solve_score_chain.py:177-179,719-720`). So it always recomputes the **v2** hash and compares it to the artifact's **stored** hash. An old clip/section artifact (container `version≥2` — the `_validate_clip_artifact:411` guard — written under the v1 scheme, with no `solve_score_spec_version` sibling) therefore mismatches and **raises**. The correct dual-read helper `solve_score_spec_version_from_meta(meta)` (missing⇒1, `:198-203`) exists but is **not used here**, and the *string-compare* path right next to it **does** thread the version (`:419,733`) — an inconsistency. This is precisely the regression §0.3 warned against ("read stored version, missing⇒1, compute both, accept match on stored version").
- **Blast radius:** re-validation/reuse of cached solve-score clip + section artifacts with container `version≥2` (most post-lag artifacts). Fail-closed (raises), so it blocks legit reuse rather than corrupting — but it will break renders/palette reuse against existing artifacts.
- **Fix:** `expected = compiled_solve_score_fingerprint(compiled, version=solve_score_spec_version_from_meta(data))` at both sites, and during transition accept a match on *either* scheme (compute v1 and v2, accept the stored one). Add a regression test that reads a stored **v1** artifact after the v2 bump (none exists — which is why this slipped the green suite).

**H2 — Re-populating a legacy saved render silently drops its coeff/param *program*.** [verified]
`_populateComputeFromDetail` sets the editor to **chips** mode when a saved render has a `*_program_chain` but no `*_program_source_text` (`js/02-preview-solvescore.js:360,363`). But compute is now **source-only**: `_effective{Param,Coeff}ProgramChainForCompute()` return `[]` unconditionally (`js/08-chip-editors.js:931-945`) and `_effective*SourceTextForCompute()` return `null` when the textarea is empty. So loading an old render that used a *program* (not transforms), then hitting Compute/render, sends **empty source + empty chain** — the program is lost while the read-only chips still display it. Pre-Phase-5, chips mode serialized the chain, so this is a new regression and it is **silent** (wrong render, no error).
- **Fix:** on populate with a chain present and no source, synth canonical source via the serializer (`coeff_source_text_from_chain` / `param_source_text_from_chain`) and set mode `'text'`; **or** let `_effective*ChainForCompute` return the populated chain so the existing Lambda-boundary chain→program-v2 translator handles it.

### MEDIUM

**M1 — Param source parser did NOT migrate onto the shared statement dispatcher.** [reported, spot-checked] `param_program_source.py:100-226` reimplements the orchestration loop + assignment/call/bare split instead of using `ProfileStatementLowerer` + `parse_profile_source` (which **coeff** correctly uses). It reuses only leaf helpers. This leaves exactly the dual-dialect duplication §3.1 set out to kill — coeff is table-driven, param still hand-rolls. Also `param_program_source.py:59-60` hardcodes `_PUSH_SOURCES={"t1","t2"}`/`_EMIT_ALIASES` instead of deriving from `profile_selectors()` like coeff does (`coeff_program_source.py:69-75`), and no drift test pins param-source selectors against the profile. **Fix:** migrate param onto `ProfileStatementLowerer`; derive its selector sets from the profile; add a param-source-vs-profile drift test.

**M2 — Coeff equivalence gate runs on a thin hand-written corpus only.** [reported] §3.6 required **both** a checked-in saved-program snapshot (`tests/fixtures/coeff-program-corpus/saved/`) + an export script **and** the landmine corpus. Only ~4 valid + 3 invalid hand-written programs exist (`tests/test_coeff_source_equivalence.py`); no `saved/` snapshot, no export script. Real saved programs are unexercised by the fingerprint gate. **Fix:** add the export script + snapshot; correct the status claim ("saved + hand-written") to match.

**M3 — The coeff "legacy oracle" only freezes the parser *shell*, not the lowerers.** [reported] `coeff_program_source_legacy.py` imports the **production** semantic lowerers and monkeypatches only split/assignment/call. So the gate proves "old shell + shared lowerers ≡ new shell + shared lowerers", not end-to-end "pre-refactor parser ≡ shared-core parser". Lowerer regressions are invisible to it. Defensible for shell-migration isolation, but narrower than §3.6's stated guarantee — **document the limitation in the plan.**

**M4 — Macro-DAG v2-dependency rule is not transitive.** [reported, spot-checked] `_missing_v2_macros` checks only **direct** macro ids from the top-level `source_chain` (`handler_storage.py:722-729`), and the migrate `translate` resolver reads **v1** source chains (`program_v2_translate.py:412,447`). A v1-only macro-of-a-macro is silently expanded from v1 and never trips the 422. §4.4 requires "v2 compile requires v2 dependencies" bottom-up. Tests cover one level only. **Fix:** walk the DAG (or require v2 reads in the migrate resolver) so transitive v1-only deps 422.

**M5 — `chain_fingerprint` hash-compare has no centralized dual-read selector.** [verified, related to H1] §0.3 said to "add the v1/v2 scheme selector in `compiled_solve_score_fingerprint` once." The `version=` param was added, but the validate sites don't pass it (H1) and `normalize_*`'s default is 2 (write-default) rather than the read-default of 1 — easy to misuse. **Fix:** make the read sites go through one helper that defaults missing⇒1.

**M6 — Phase-1 registry drift-CHECKS but does not DRIVE dispatch (plan §1.2 oversold).** [verified] Neither `coeff_program_chain.py` nor `param_program_chain.py` imports `structural_chips.json`; `_CHIP_COMPILERS` etc. stay hand-maintained and are only byte-compared by the drift gate. The implementation is safe; the plan's "generated table supplying the **dispatch map** … says *which* handler to call" is not what shipped (it ships drift-check + source/frontend-driven). **Fix:** reconcile §1.2 wording to "drift-checked, dispatch unchanged", or wire the dispatch.

**M7 — Phase −1 byte-oracle corpus is too easy.** [verified via reviewer + plan] The whole-sweep oracle runs on 2 trivial 4×4 programs (`poly_1`+rev, `const`/`linear`+rev). §−1(b) explicitly requires **clustered / near-degenerate** roots to exercise the serpentine warm-start cascade — the current corpus won't catch the warm-start regressions it was built for. **Fix:** add clustered/near-degenerate fixtures to `tests/fixtures/oracle/`.

### LOW / NIT

- **L1** `sweep_cli.c:3578-3585,4218` — `CoeffResolvedArgs.has_andy` is set but never read (dead field; `coeffAndyValue` reads `andy_re/im` directly). [verified-ish]
- **L2** `param_program_source.py:194-219` — param strict mode raises a **plain `RuntimeError`** (drops computed diagnostics), unlike coeff's structured `CoeffProgramSourceCompileError`. Parity gap. [reported]
- **L3** `solve_score.h` — the `v2;` marker + opcode renumber (65–92) rode along in the **Phase-2B lag commit**; it's internally consistent but bundling muddies the "drift bisects to one phase" intent. Scope-hygiene only. [reported]
- **L4** `structural_chips.json` — ships `op/op_symbol/selector_slots/internal`, not the plan's `stack_effect/tier/lower_to/synth_only`. Consequently the drift gate does **not** pin each chip's pop/push `stack_effect` (§1.3 wanted it). Update §1.1/§1.3 or add `stack_effect` + gate it. [reported]
- **L5** JS editable machinery is **disabled but not deleted** (`_chipPickers.pp/.cp` engine `js/08-chip-editors.js:120-143`; the now-unreachable editable arms in `js/09-render-orchestration.js:172-189`). §5.1 said "retire"; this is the follow-up cleanup left undone (dead code). [reported]
- **L6** `pipeline_programs.py:80-169` — the Chain→Program target-arg→selector translation duplicates logic already in `compile_param_program_chain`; and the `exp/round` real/imag→complex repack (`:151-169`) has no dedicated numeric-parity test for the Chain→Program path. Maintenance smell. [reported]
- **L7** `program_source_core.py:65` — `load_program_profiles` falls back JSON-on-`except Exception`, masking a real import error as "module missing". [reported]
- **L8** `js/program_profiles_gen.js` (a standalone JS profile mirror, §1.4) does not exist; profile data reaches JS via the catalog generator instead. Likely fine (browser is a thin client, doesn't execute opcodes), but the plan implies a standalone mirror — note or drop. [verified absent]

---

## Per-phase verdict

| Phase | Verdict | Headline |
|---|---|---|
| −1 verification harness | **Ships clean** | Faithful Python oracle + real parity/byte tests, green — but weak warm-start corpus (M7). |
| 0 versioning + gate | **One HIGH bug** | Native gate + WRITE siblings + string-compare dual-read correct; **hash-compare dual-read missing (H1)**. |
| 1 registry + profiles | **Landed, two oversells** | Caps accurate, selector-authority split + JOIN gate, C drift-checked, param under gate; dispatch not driven (M6), param-source bypasses profile (M1). |
| 2A VM merge | **Faithful & correct** | FP-clean; arena + arg ABI + load-time lowering as specced; param compat path kept. Only L1. |
| 2B lag facility | **Approve** | Single lag-stream object, all 5 sites converted, numerically equivalent. Only L3 scope nit. |
| 3 shared parser + coeff migration | **Ship-with-followups** | Coeff really delegates to the core; frozen oracle isolated; canonical serializer + structured diagnostics real. But M1/M2/M3. |
| 4 v2 IR + migration | **Ship-ready, one MEDIUM** | Opcodes/translate/root-registry/routes/shared score-version helper all correct; macro DAG not transitive (M4). |
| 5 chips display-only | **Substantially complete, one HIGH** | Native boundary genuinely closed (hard reject + deleted parsers + per-handler gates); **populate regression (H2)** + dead-code cleanup (L5). |

---

## What's genuinely strong

- **FP determinism held across the riskiest changes.** 2A's lowered-expr executor is character-identical to the old inline loop; 2B's lag refactor is byte-faithful (warm-up, lag depth, buffer separation all verified). The whole-sweep byte oracle exists and is green. This was the project's central risk and it was managed well.
- **The native boundary is actually closed.** `parsePtChain`/`parseCtChain` are gone; `rejectLegacyTransformChain` hard-rejects non-empty transform arrays in all four native modes; the Program-mode legacy fast-paths were all converted (no transform array leaks to the CLI). The "two layers" ASL discipline held (no dangling `.$`).
- **The drift/versioning scaffolding is real**, not decorative: family + per-scope `spec_version` siblings, the in-container sibling without bumping the fail-closed container version, the shared `strip_solve_score_version` helper used by native + all 4 Python tokenizers, and generators byte-gated in predeploy.
- **The hard Phase-3 pieces shipped**: coeff genuinely delegates to the shared core, the frozen test-only oracle is correctly isolated (only the equivalence test imports it), and the canonical serializer round-trips `execution_spec`/`fingerprint`.

---

## Recommended before calling CR18 "done"

1. **Fix H1** (thread `solve_score_spec_version_from_meta(data)` into the two hash-validate sites + accept either scheme in transition) and add the missing **read-old-v1-artifact** regression test.
2. **Fix H2** (populate must synth canonical source, or route the chain through the boundary translator).
3. Close the §3.6 release-gate gaps: **M2** (saved-program corpus + export script) and document **M3** (shell-only oracle).
4. **M4** transitive macro-DAG enforcement.
5. **M1** migrate param onto the shared dispatcher (the anti-duplication this phase exists for) + param-source profile drift test.
6. Reconcile plan vs reality: **M6** (§1.2 dispatch wording), **L4** (`stack_effect` scope), **M7** (warm-start corpus), **L8** (JS profile mirror).
7. Cleanup: **L5** (delete dead editable JS), **L2** (param structured exception), **L1/L7** nits.

*Items 1–2 are correctness regressions on legacy-artifact paths and should land first; 3–7 are completeness/maintainability and can follow.*
