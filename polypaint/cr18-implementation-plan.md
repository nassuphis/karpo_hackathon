# CR18 — Stack-DSL Unification: Implementation Plan (Phases 0–5)

Companion to `code-review-18.md` (the design review). CR18 holds the *why* and the design decisions (DD1–DD6, the three conflicts C1/C2/C3, the verification prerequisites); this doc holds the *how* — per-phase file maps, native structs, schemas, tests, and migration routes, so each phase is codeable without re-deriving current state.

Verified against the `1a1996d` source snapshot; **all later commits touch only docs (CR18 + this plan) and populate-source persistence — no VM/source change — so every file:line below is current** (no specific "current HEAD" hash is pinned here, since it advances with each doc commit). C/Python/JS paths are under `lambda/`, repo root, and `js/` respectively.

**Dependency order & gates.** 0 → 1 → 2A → 2B → 3 → 4 → 5, but two gates from CR18 are hard prerequisites: **DD2 native version gate must precede any opcode renumbering (Phase 4)**; **DD5 whole-sweep byte oracle + a Python solve-score evaluator must exist before Phase 2 ships**; **DD6 benchmarks before the unified dispatch ships (Phase 2)**. Phases 0,1,3 are independently shippable with value; Phases 2,4 are the risk concentration.

---

## Phase 0 — Versioning insurance + native version gate

**Objective.** Two *distinct* version contracts (DD2): (1) `program.version` enforced by the native parser; (2) a `spec_version` metadata sibling beside each fingerprint family for dual-read. No behavior change for valid v1 payloads.

### 0.1 Native `program.version` gate (DD2 #1)
The payload already ships `"version": 1` (`program_compile_helpers.py:19,33`); the native parsers read it never.
| file | fn | insertion point | guard |
|---|---|---|---|
| `sweep_cli.c` | `parseCoeffProgram` (:3741) | after `objEnd` (:3756), before `stack_max` (:3757) | `ver=findKeyIn(objStart,objEnd,"version"); v = ver?parseNum:1; if(v!=1) return -1;` |
| `sweep_cli.c` | `parseParamProgram` (:6267) | after `objEnd` (:6282), before `stack_max` (:6283) | same |

**Rule:** missing ⇒ v1 (old hand-authored payloads + native tests omit it — a hard-fail there is a regression). Reject unknown/newer with a clear stderr message + nonzero return.
**Tests:** new native cases — `version:2` → reject (nonzero exit); `version` absent → runs as v1; `version:1` → unchanged. Add to `test_coeff_program_native.py` / `test_param_program_native.py`.
**Rollback floor (DD2):** once this ships, pre-Phase-0 binaries are no longer valid rollback targets (they ignore the field). Record in the deploy runbook.

### 0.2 `spec_version` sibling at WRITE sites
Add a **family-specific** version int (=1 now) beside each persisted fingerprint — **not a single generic `spec_version`**, because one meta object can carry several families: `handler_palette_finalize.py:533` writes `palette_variant_fingerprint`/`content_fingerprint` *and* `:568` writes `solve_score_chain_fingerprint` in the **same** `meta.json`, so one sibling can't say which it versions. Use `solve_score_spec_version`, `palette_variant_spec_version`, `probe_signature_spec_version` (or a single `spec_versions` map keyed by family). Missing ⇒ 1. **Three families need a new field; two already carry their own `version` int and extend it instead.**

| family | representative WRITE sites | action |
|---|---|---|
| solve-score `chain_fingerprint` / `program_spec` | clip `handler_solve_proximity.py:913`; section-hist `:1442`; bins `:1583`; palette meta `handler_palette_finalize.py:534,568`; render plan `handler_palette_render_plan.py:817,1086`; raster `handler_raster_mt.py:587`; extract `handler_extract_palette_from_step_scores.py:545`; recolor `color_recolor_raw.py:419`; saved programs `handler_storage.py:313,428,489` | add **`solve_score_spec_version`** sibling |
| palette `palette_variant_fingerprint` / `content_fingerprint` | `handler_palette_render_plan.py:463,840,1109`; `handler_palette_finalize.py:533` | add **`palette_variant_spec_version`** sibling |
| `probe_signature` | `handler_coeffgen.py:670` (built by `compute_fused.py:34`); stored in plan `handler_compute_plan.py` | **`probe_signature_spec_version`** (recompute under the program's versions) |
| raw sidecar (incl. its embedded `chain_fingerprint`, `raw_sidecar.py:222`) | `raw_sidecar.py:9-11` (`RAW_SIDECAR_VERSION=3`) | **extend the sidecar's own `version`** — *no* `solve_score_spec_version` sibling here (the container version covers its embedded fingerprint; this resolves the apparent raw-sidecar overlap) |
| fragment manifest (incl. embedded `chain_fingerprint`, `handler_finalize_mt.py:523`) | `handler_finalize_mt.py:35` (`FRAGMENT_MANIFEST_VERSION=1`) | **extend existing `version`** |

Do **not** rename existing fingerprint fields (a rename is itself a wire change). Do **not** fold `spec_version` into the hashed payload (`compiled_solve_score_fingerprint`, `solve_score_chain.py:641`) — that re-keys every artifact (the whole point of "adjacent").

### 0.3 Dual-read at the 14 READ/compare sites
Read the stored **family** version (`solve_score_spec_version` / `palette_variant_spec_version` / `probe_signature_spec_version`, or the container's own `version` for raw-sidecar/fragment-manifest; **missing ⇒ 1**) → compute the matching-scheme fingerprint → compare; during transition compute *both* and accept a match on the stored version.

**`program_spec` string compares need a *versioned compare API*, not just a versioned hash (blocker).** The sites at `handler_raster_mt.py:263` / `handler_palette_chunk.py:421` / `handler_solve_proximity.py:415,724` / `handler_finalize_mt.py:282` compare the *canonical string* directly (`canonicalize_solve_score_program_spec(...)`). Versioning the `sha256` does **not** cover these. Add `canonicalize_solve_score_program_spec(spec, version)` and compare `canonicalize(stored, stored_ver) == canonicalize(expected, stored_ver)`. The v2 canonical renderer (Phase 4.3) and this versioned canonicalizer are the **same** change — both must land together.
| sites | type |
|---|---|
| `handler_raster_mt.py:259`, `handler_solve_proximity.py:404`, `handler_palette_finalize.py:316,331`, `handler_finalize_mt.py:106,274`, `handler_palette_chunk.py:417` | `chain_fingerprint` hash compare |
| `handler_raster_mt.py:263`, `handler_solve_proximity.py:415,724`, `handler_finalize_mt.py:282`, `handler_palette_chunk.py:421` | `program_spec` canonical-string compare |
| `handler_compute_plan.py:231-241` | `probe_signature` |
| `handler_palette_render_plan.py:297` | palette collision — **conditional only** (CR18 §4.1 #2: usually a slug-miss → recompute; only guard the slug-stable case) |
| `raw_sidecar.py:197` | `chain_fingerprint` stays *required* (missing field still raises) |

The dual-read field table is in CR18 §4.2. **Centralization opportunity:** all `chain_fingerprint` writes/compares funnel through `compiled_solve_score_fingerprint` (`solve_score_chain.py:641`) — add the v1/v2 scheme selector there once.
**Gates:** new dual-read unit tests per family; full predeploy + native gate; no behavior change for stored v1 artifacts. **Risk:** low for valid v1; the only behavior change is rejecting unsupported `program.version`.

---

## Phase 1 — Structural-chip registry (single source of truth)

**Objective.** A JSON registry for the **program chips** (the `_CHIP_COMPILERS` stack-VM vocabulary) that drives **registry metadata + arg/stack/selector validation + the dispatch map** (which named handler to call) for Python/C/JS, gated by a drift test — **not** data-only compilation: the semantic compiler handlers stay as named functions (see §1.2). **This is new ground:** today the ~25 program chips are hardcoded Python compiler functions with *no* registry, *no* generated JS mirror, and *no* drift gate (only the 28 *transforms* in `coeff_legacy_registry.json` are registry-driven). No runtime VM change.

### 1.1 Registry schema — `structural_chips.json` (new)
Per entry (modelled on the verified per-chip table; tiers from `coeff_program_chain.py:1622` dispatch):
```jsonc
{ "name": "affine", "aliases": ["linear"],
  "opcode": 20,                       // COEFF_OP_AFFINE
  "tier": "B",                        // A=zero-arg, B=_CHIP_COMPILERS, C=name-family, D=registry-fallthrough
  "stack_effect": { "pops": ["src?"], "pushes": ["tgt?"] },   // selector-conditional allowed
  "args": [ {"name":"tgt","type":"selector","required":true},
            {"name":"src","type":"selector","required":true},
            {"name":"multiplier","type":"complex","required":true},
            {"name":"offset","type":"complex","required":true} ],
  "src_selectors": ["cf","poly","pop","peek"],
  "tgt_selectors": ["poly","push"],
  "lower_to": "_compile_affine",      // python compiler fn / token shape
  "synth_only": false }
```
Arg `type ∈ {int, real, complex, selector, enum, registry_ref}`. `synth_only` marks the `_typed_*` ops (`get_scalar/set_poly/poke_poly/fill/blend`) the source layer emits internally. The **full inventory** is the per-chip table the audit produced: 30 `COEFF_OP_*` entries spanning `push_const/push/emit/dup/swap/pop/flush/blend/legacy/poke_poly/poke_tos/vector_binary(+5 fns)/vector_unary(+16 fns)/vector_roll/argsort/littlewood/linspace/range/set/affine/typed_push_scalar/typed_push_vector/typed_binary/typed_unary/typed_get_scalar/typed_set_poly/typed_poke_poly/typed_fill/native_transform(+stack_args)/typed_blend/macro` — plus the Tier-D registry-name fallthrough (`coeff_program_chain.py:1642`). Capture the arg-order quirk: legacy/native rows are **source-first** `[name,src,tgt,…]`, named chips are **target-first** `[tgt,src,…]` (`coeff_program_chain.py:994`, the "LAYOUT CONTRACT").

**Sub-op families need their own namespace table (they are NOT single chips).** `vector_binary` (opcode 12) is a *family* of 5 sub-ops `add/subtract/multiply/divide/power` selected by `fn_index` (`VECTOR_BINARY_OPS`, `coeff_program_chain.py:218`); `vector_unary` (13) is 16 sub-ops (`VECTOR_UNARY_OPS:226`, with `mod`→`abs` canonicalization, `:216`); `vector_roll` (14) is 2; `typed_binary`/`typed_unary` reuse the same sub-op ids via aliases (`sub/mul/div/pow`, `:210`). The schema must model each family with a nested `sub_ops: [{fn_index, name, aliases, source_name, display}]` — pinning every sub-op's `fn_index`, canonical name, **wire/source aliases**, and **display name**, exactly as `coeff_legacy_registry.json` pins transform fn_index/aliases — and the drift gate must pin the sub-op ids too (not just the family opcode).

### 1.2 Generator — extend `gen_coeff_vocab.py`
Today `build_vocab()` emits `window._coeffRegistryVocab = {names, fnIndexByName, aliasToCanonical, sourceAliasByName, chipNameByRegistryName, supportsAndy, ctCatalog, categoryMeta, programParamDefs}` to `coeff_vocab_js.js` (transforms only). Add a parallel `build_structural_chips()` emitting:
- **Python:** a generated table supplying the **metadata + dispatch map** for `_CHIP_COMPILERS` / `_ZERO_ARG_CHIP_OPS` / the name-family tables (`coeff_program_chain.py:1584,1595,1632`). The registry says *which* named handler to call and drives arg/stack/selector validation; it does **not** replace the handler bodies — the semantic functions (e.g. `_compile_affine`, the linear/exp/pow arg-packing special cases) remain **named compiler handlers referenced by `lower_to`**. (A registry can't magically encode every special case; "registry-driven metadata + validation + dispatch," not data-only.)
- **C:** a generated header (chip→`COEFF_OP_*`→stack-effect/arg-shape) the dispatch (`sweep_cli.c:5130`) and parser validate against.
- **JS:** `_coeffProgramCatalog` (today hand-maintained in `js/07-transform-catalogs.js`).
Keep the `--check` byte-equality drift mechanism (`gen_coeff_vocab.py:88`).

### 1.3 Drift gate — extend `tests/test_coeff_program_drift.py`
The existing test pins `COEFF_OP_*` / `COEFF_EXPR_*` / `COEFF_SEL_*` enums + the 28-transform fn_index + aliases + generated-JS byte-equality (`:45,:113,:152,:190`). Add: pin every structural-chip `name → opcode → stack_effect → args → selectors` across registry/Python/C/JS. **Also bring the param VM under the gate** — today `test_coeff_program_drift.py` only scrapes `COEFF_*`; `PARAM_OP_*` / `PARAM_EXPR_*` have *no* drift test (CR18 C2 risk). Add `test_param_program_drift.py` mirroring it. *This is a Phase-1 deliverable because C2 (the fn_index/opcode reconciliation) lands in untested param space otherwise.*

**File map:** new `structural_chips.json`, `param_legacy_registry.json` drift coverage; `gen_coeff_vocab.py` (+ maybe split a `gen_program_chips.py`); `coeff_program_chain.py` (dispatch driven from registry); generated C header; `js/07-transform-catalogs.js` (catalog generated); `tests/test_coeff_program_drift.py` + new `test_param_program_drift.py`. **Risk:** low-medium — bigger than a vocab-gen (the chips carry compiler semantics, §CR18 Phase 1), but no runtime change; the drift gate de-risks divergence.

---

## Phase 2A — Shared runtime (VM merge) behind frozen front-ends

**Objective.** One native interpreter core behind byte-identical per-VM front-ends (DD1 adapter) + the expression-temp arena (DD1 + §2a isolation). **The solve-score lag refactor is split out into Phase 2B** — two unrelated high-risk native changes (VM merge vs lag double-buffer) must be *separate phases behind the same DD5 oracle*, so any numerical drift bisects to one of them. Fingerprints do not move (front-ends frozen). Gated by native parity + DD5 + DD6.

### 2.1 Unified value model + workspace (native structs)
Merge `CoeffProgramWorkspace` (`sweep_cli.c:3504-3536`: planar `stack_re/stack_im[64][256]`, scalar slots, `poly`, scratch/original/aux) and the param state (`ParamProgram`/`ParamProgramToken`/`ParamCx`, `sweep_cli.c:6097-6128`) into one runtime that the config (CR18 §3.3) parametrizes (enabled ops, register layout, caps coeff 256/64/256 vs param 64/16). **Per-profile workspace sizing** (CR18 §3.1, corrected): size to the profile's caps; do not assume a single shared constant (note `MAXDEG` is *inconsistent* across rasterizers — `roots2pix_mt.c:28` 256 vs others 1024 — reconcile, not "size for 1024"; roots can't exceed `MAX_DEGREE 255`).

**Fixed-output-length contract (CR18 §7.5 — must be in the runtime config).** Coeff output length is probed once at (0,0) and **hard-asserted constant** for the whole grid (`sweep_cli.c:7841,7948`), and the warm-start chain depends on it. The unified config needs a `fixed_output_length` flag (true for coeff) that makes the interpreter **reject a per-row length change** — load-bearing once the unified op set includes dynamic indexing (`typed_get_scalar`) and length-changing transforms (`length_policy: may_change`), which could otherwise vary `poly_len` per cell. Other profiles have their own output contracts: param → `p1,p2` scalars; root → in-place, same-length; solve-score → 1 scalar or N channels.

### 2.2 Expression-temp arena (DD1 implementation sketch — the §2a isolation)
Add a **separate** per-thread arena distinct from the user value planes:
- *struct:* `double tempRe[N]/tempIm[N]` (coeff) / `ParamCx temp[N]` (param), `N = MAX_EXPR_NUMS/EXPR_STRIDE` (the existing private-stack sizing at `:3869/:6380`).
- *cap:* the existing `MAX_EXPR_NUMS/STRIDE`; a single expression's lowered sub-sequence must fit it (compile-time check). **Separate from** the user 64/16 cap.
- *frame base / `tos[i]` — ONE base per TOKEN, not per arg (load-bearing):* capture a single `token_frame_base = user_sp` **before evaluating any of the token's dynamic args**; every arg's `tos[i]` reads `user_stack[token_frame_base-1-i]` — the **pre-token** stack. This prevents a multi-dynamic-arg token from letting arg 2 see arg 1's pushed result through `tos0`. Arg intermediates live only in the hidden arena; `user_sp` grows only by the final pushed arg results.
- *`native_transform` args:* each arg expression computes in the arena reading only the pre-token stack (above), then pushes **exactly one** final value; a `stack_arg_count=K` transform is fed by K such args (K user-stack slots, as today), every intermediate off the 64/16 budget. Net per arg: **+1**; the transform pops its K at the end.
- *Mixed literal + dynamic args (load-bearing):* a token can carry some literal args and some `expr_refs` (e.g. `affine(tgt,src, 2.0, p1*cf[0])`). To keep `stack_arg_count` positions stable, **lower ALL args in positional order — literals as constant-pushes, dynamics as lowered expressions** — so the K pushed slots are exactly the token's K args in order. (The alternative, per-arg source metadata, is more complex; prefer lower-all-in-order.)
- *`andy_expr_ref` must be lowered too (this is the bug class we already hit):* `andy` is evaluated by a *separate* call today — `coeffAndyValue` → `coeffEvalScalarExpr` (`sweep_cli.c:4118-4119`, used at `:4619`). Deleting the evaluator (§2.3) without lowering `andy_expr_ref` would dangle it. The lowering must treat `andy_expr_ref` as one more arg expression (into the arena), its result consumed by the transform's blend-back.

### 2.3 Retire the nested scalar-expr evaluators
Delete `coeffEvalScalarExpr` (`sweep_cli.c:3855-4056`) and `paramEvalScalarExpr` (`:6372-6436`) — both are `[op,a,b]`-triple if/else loops over private stacks. Replace with **native load-time lowering** (once, at parse time — *not* in the per-row loop): the compact `scalar_exprs` wire stays (DD1); `parseCoeffProgram`/`parseParamProgram` lower it **once** into cached temp-arena token sequences, and the per-row evaluator only *executes* those pre-lowered tokens (no per-row re-lowering).

**Every `expr_refs` consumer must be migrated — not just `native_transform`.** `coeffArgValue` (`sweep_cli.c:4058`) is read by ~12 ops: affine (`:4682-4683`), exp/pow + legacy arg packing (`:4690-4697`), littlewood (`:4758-4761`), const (`:4808`), set/blend/poke/fill and the typed family (`:4888,4974,4992,5015,5086`), plus `coeffAndyValue` (`:4118`). For **each**, the load-time lowering must materialize its args (literal + dynamic, in order, §2.2) before the op runs — an op whose args aren't lowered would call a deleted evaluator. Treat the consumer set as an explicit checklist; the native-parity gate must exercise each op with a dynamic arg. The merged opcode enum + the param-as-config mapping are **internal only**; the per-VM `execution_spec`/fingerprint stay byte-identical (param still `del scalar_exprs` at `param_program_chain.py:1107`; coeff spec bytes still wire, `coeff_program_chain.py:1838`).

---

## Phase 2B — Centralize the solve-score lag facility

**Objective.** Replace the 5 hand-rolled current/recent lag buffers with one `solve_score.h` facility. **Run after Phase 2A lands clean, behind the same DD5 oracle** — keeping the VM merge and the lag refactor as separate phases means a numerical-drift regression bisects to exactly one of them.

Today 5 call sites across 4 files hand-roll the current/recent double-buffer + step-0 warm-up + per-step memcpy:
| file | fn | buffers | warm-up | memcpy | eval |
|---|---|---|---|---|---|
| `roots2pix_mt.c` | `worker_main:238` | :246-247 | :383-410 | :521-523 | :412 |
| `solve_palette_chunk_mt.c` | `compute_scores_for_roots:317` | :324-325 | :376-403 | :453 | :406 |
| `solve_proximity_hist_sectioned.c` | `hist_section_worker_main:282` | :385-386 | :428-455 | :480-482 | :456 |
| `solve_proximity_stats.c` | `compute_scores_lagged_summary:532` | :579-580 | :619-646 | :653 | :648 |
| `solve_proximity_stats.c` | `compute_program_outputs_summary:670` | :718-719 | :759-790 | :807-809 | :793 |

Add a stateful lagged-stream object to `solve_score.h` (init + per-row `advance(current_sources, prev_row_accessor)` that owns the two buffers + `recentInitialized` warm-up + end-of-step swap), using `solve_score_eval_metric_slots` (`:1593`), `solve_score_eval_lagged_metric_slots` (`:1607`), and the buffer-eval (`:1636`). The **row-addressing differs** per file (sectioned byte offsets vs `idx*stride` vs prelude math) → the object takes a **callback accessor** "give me row N's solve/coeff/param pointers," not the buffers themselves.

**Phase 2A file map:** `sweep_cli.c` (interpreter restructure, delete the 2 scalar-expr evaluators, add the temp arena). **Phase 2B file map:** `solve_score.h` (lag-stream object), the 4 rasterizers (replace hand-rolled lag). **Gates (both phases, behind the *same* DD5 oracle):** native-parity tests bit-identical to *both* old interpreters; the **DD5 whole-sweep byte oracle** (full coeffgen + roots `.bin`, old-VM-vs-new-VM, on warm-start-sensitive programs — single-cell parity is insufficient, CR18 §7.1); **DD6 per-profile benchmarks** (param-only / coeff scalar / coeff vector / solve-score multi-channel / root-raster + per-thread workspace bytes) vs the current tight-`switch` baseline. **Risk:** the highest C changes — running 2A and 2B as separate phases is precisely what keeps FP-drift bisectable.

---

## Phase 3 — Text parser (param); root + solve-score stay chip-primary

**Objective.** Build the **one** missing compute-path text parser — **param** — so coeff+param are text-first authorable. **Root stays a chip-editable flat array** (`[name,...params]`): optional read-only text *export*, **not** a source parser (§3.2 — this refines CR18 §9.3, which over-eagerly listed root as "text-first"; root is text-*able* but not worth a source-editing parser, and it stays chip-editable in Phase 5). **Solve-score stays chip-primary** with read-only text export (CR18 §9.3 — its declarative slot-CSE + lag has no clean text shape).

### 3.1 `param_program_source.py` (new) — mirror `coeff_program_source.py`
Architecture (template at `coeff_program_source.py`): lower source text → existing chip list → existing `compile_param_program_chain`. Four entrypoints mirroring coeff (`:855,889,837,748`): `parse_param_program_source`, `compile_param_program_source`, `param_source_text_from_payload`, `split_param_program_statements`. **Reuse** `param_program_chain`'s `_ExpressionParser` (`:573`), `_expr_to_param_tokens` (`:700`), and target `_lower_chip` (`:950`) — derive all op tables from the chain layer (the coeff parser's "vocabularies cannot drift" pattern, `coeff_program_source.py:14-48`). **Param-specific grammar:** `emit p1`/`emit p2`, `push`/`push t1`, the dual-register unary forms (`square(p1)`-style, `param_program_chain.py:977`), `legacy(name,src,tgt,…)` (the load-bearing bridge — param *cannot* ban it the way coeff text does), `both` target default. Error type `ParamProgramSourceError` (line/col) mirroring `CoeffProgramSourceError` (`:125`).

**Grammar (EBNF sketch):**
```
program   := statement ((";" | "\n") statement)*
statement := assign | call | stackop
assign    := ("p1" | "p2") "=" expr           # → push(expr); emit_p1/p2
call      := name "(" [args] ")"              # emit/push/unary/legacy/macro
stackop   := "push" [reg] | "dup" | "swap" | "pop" | "flush"
expr      := <the existing param ExpressionParser: t1,t2,p1,p2, + - * /, unary exp/real/imag/abs, complex i>
args      := expr ("," expr)* ;  reg := "t1" | "t2" | "p1" | "p2"
```
**Canonical source-from-chain is first-class (the populate path).** The recent "populate emits `_typed_*` garbage" bug means reconstruction must be a real, tested feature: `display_param_program_chain` (`param_program_chain.py:492`) must render clean, **re-parseable** source — never internal synth chips — so a synth-bearing chain renders to its *surface* form (`square(p1)`, not `push;_typed_unary;emit`). Pin `parse_param_program_source(display_param_program_chain(c)) ≡ c` over a corpus including legacy-bridge and expression chips; the Legacy-tab/populate flow depends on it.
**Error recovery:** `strict=True` raises on first error (save/preview); `strict=False` skips the failing statement, records `{line,column,message}` in `diagnostics`, continues (editor live-compile) — exactly coeff's `parse_coeff_program_source(strict=)` (`:855`).
**Examples:**
```
p1 = t1*t2 + p1                          # push expr; emit_p1
p2 = exp(t1)                             # push expr; emit_p2
square(p1)                               # push p1; square; emit_p1  (dual-register sugar)
legacy(moebius, p1, push1, 1, 0, 0, 1)   # the load-bearing bridge
push t1; dup; mul; emit_p1               # explicit stack form
```

### 3.2 Root transforms — stays chip/array (optional read-only export, no source parser)
Root is a flat `[name,...params]` array (`root_xforms.h:43`, `js/07-transform-catalogs.js:993`) with 9 fixed transforms; chips→display works without a parser since the wire is already an array. **Recommendation:** a trivial one-transform-per-line text form (`rotate_roots(0.5)`) is ~30 lines if wanted, but **no compute-path source module is needed** — don't add a root `*_source.py` to compute bundles.

### 3.3 Storage round-trip + route
- `_compile_param_program_payload` (`handler_storage.py:388`) gains a `source_text` arg + `param_source_text_from_payload`, mirroring coeff's injection at `_compile_coeff_program_payload:454` (where `source_text` overrides `chain`).
- Add `/compile-param-program-source` route (mirror `/compile-coeff-program-source` at `handler_storage.py:884`) + handler + frontend `lambdaPost` (`js/08-chip-editors.js:438` is the coeff precedent) — synchronized across the 3 manifest sources (see Phase 4 / 4.5).

### 3.4 Packaging fan-out (CR18 §9.4 — per parser, different bundles)
`param_program_source.py` is compute-path → the same 5 bundles as `coeff_program_source.py`: coeffgen, storage, compute_preview, param_debug, compute_plan (`deploy.sh:964,985,1034,1058,1328`). A missing `cp` is a runtime ImportError caught by `test_deploy_packaging.py`'s AST walk (static imports only). **No root source module** (3.2). **Gates:** `test_param_program_*` (native + storage + source round-trip), the AST packaging gate, api_manifest regen for the new route. **Risk:** low-medium (param is a coeff-parser subset).

---

## Phase 4 — Unified token IR + v2 fingerprint + `translate_from_old` + Legacy tab

**Objective.** The fingerprint version bump: merged opcode IR, per-kind old→new translation, v2 fingerprint scheme + dual-read, migration routes, the Legacy tab, the opcode-renumber drain. **Highest risk.** Prerequisites: DD2 gate (Phase 0) live; DD5 oracle exists.

### 4.1 Merged opcode IR (v2) — resolve C1/C2
One merged opcode enum across param+coeff (C1: today op 8 = param POP vs coeff BLEND; op 10 = param ADD vs coeff POKE_POLY). `native_transform` gains a **registry-namespace discriminator** (C2: `ct_*` 1–28 vs `pt_*` 1–48 collide) **plus per-registry capability metadata** — `andy` and `length_policy` are coeff-only (CR18 §7.6); the param/root registries declare them absent, and the unified op validates capabilities per registry (no `andy` on a root transform; no length change where the profile's fixed-output contract forbids it, §2.1). Bump `program.version` → 2. The native parser (version-gated since Phase 0) keeps a v1 legacy-decode path through the drain (4.6) and adds the v2 decode. The merged enum is the wire only at this phase (Phase 2 kept it internal).

### 4.2 `translate_from_old(kind, payload) → unified_chain`
- **coeff** → near-identity at the *chip* level, **but NOT at the wire level**: v2 inserts the registry-namespace discriminator (`registry='coeff'`) into native/legacy tokens that lack it, renumbers to the merged opcode enum (C1), then re-fingerprints under v2. The chips are unchanged; the token bytes are not.
- **param** → dedicated arithmetic opcodes → typed-op equivalents; `pt_*` legacy → `native_transform[param_registry, fn_index]` (preserve fn_index, C2); `emit_p1/p2` → output projection.
- **root** → a sequence of `native_transform[root_registry, fn_index]`, but root needs its **own value model + execution context** (CR18 §7.0): root transforms run **per-sample at raster time over a `float` root array** (`root_xforms.h:242`), *not* over the coeff VM's `double` complex-vector registers. The root profile must declare: input/output register = the `roots` float vector (in place); a **precision policy** (float vs the VM's double — pin which, since it's observable); the raster-stage invocation context; length-preserving; and the NaN-poison convention (`rt_mark_undefined`). **Unknown names → traced no-op** (CR18 §4.3: drop + diagnostic, never hard-fail or silently vanish — preserves today's `:278` silent-ignore *with a trace*).
- **solve-score** → `reduce_metric` + scalar ops + `emit` — but the hard parts must be specified, not hidden: **(a) metric-slot CSE** (dedup `(metric,source,quantile)` into shared slots, the `program_spec` slot model); **(b) lag references** (`mN-1` reads the prior step — the Phase-2B facility); **(c) implicit scalar output vs explicit `emit`s** (single-score path vs N channels); **(d) per-channel normalization** (`emit_norm`); **(e) the multi-channel output contract** (channel count ↔ `color_interpretation`, CR18 §3.2). Preserve omega/phase exactly (already in `program_spec`). This is why solve-score stays chip-primary — translation is its *only* migration target and the trickiest.
Injection point: the payload compilers already override `chain` from source (`_compile_coeff_program_payload:454`) — `translate_from_old` plugs in there.

### 4.3 v2 fingerprint scheme + dual-read
Bump the family `spec_version` → 2 at the WRITE sites (Phase 0.2); the READ sites (Phase 0.3) already dual-read. Add the v2 canonical renderer in `compiled_solve_score_fingerprint` / the per-VM `execution_spec`. Old artifacts stay v1; new work emits v2.

### 4.4 Migration routes + macro DAG (DD3)
- New routes in `handler_storage.py` dispatch (`:857-931`, the `path.endswith` chain): `/migrate-{coeff,param,solve-score}-program` (or one `/migrate-program`). Version-aware fetch: **prefer the `v2/` namespace object** (e.g. `polypaint/coeff-programs/v2/<id>.json`), else v1. **Non-overwriting save** (today save-by-slug overwrites, `handler_storage.py:1199`,`1114`,`1028`): v2 writes to `v2/`, v1 stays immutable.
- **Macro refs stay by logical `<id>`** (not `<id>@v2`), resolved by a **version-aware fetch with two modes** (this resolves the apparent contradiction): for **v2 compile**, the resolver **requires v2 dependencies** — a v2 program whose macro is still v1-only is an *error* (fail the compile), which forces bottom-up migration; the **v1-fallback** ("prefer `v2/` else v1") is **only** for list/display/migration-discovery, *never* for v2 compile. Migrate the DAG **bottom-up** (leaves first), reusing the existing cycle/depth guard (`handler_storage.py:343,378`).
- **Route 3-source sync** (`api_manifest.py`): a new route must be added to (a) the `handler_storage.py` dispatch (auto-detected by `_extract_storage_routes`), (b) `deploy_manifest.json` `polypaint-storage` `routes[]` (`:97-132`), (c) the frontend `lambdaPost('storage',…,'/X')` call (`js/03-program-modals.js`), then `api_manifest.py --write`; `--check` fails CI otherwise. New migrate/translate handlers also need payload-contract tests (api_manifest pins routes, not payload shapes).

### 4.5 Legacy tab UI (per-kind — CR18 §4.4)
A tab that loads an old saved program and runs `translate_from_old`, **per-kind**: **coeff/param → migrated text form** (Phase 3 param parser; coeff already has one); **root → migrated chain/array form** (+ optional read-only text export — *no* source parser, §3.2); **solve-score → chain + `program_spec` preview** (no text source). Save the v2 copy under the versioned id. **Root needs a new saved-program kind** (it isn't one today — `handler_storage.py` has only coeff/param/solve-score prefixes) — add the prefix + handlers, or store migrated root chains under a render-side home. UI plugs into `js/03-program-modals.js` (the per-kind fetch/list/save/delete call sites).

### 4.6 Opcode-renumber drain (DD2)
Step Functions forwards compiled `coeff_program`/`param_program` straight into native (`compute_workflow.asl.json.template:211,278`). Deploy so no in-flight execution straddles the renumbering (drain window, or translate at the Lambda boundary). The `scalar_exprs` form is *not* part of this drain (permanent wire, DD1) — only the opcode numbering changes.

**Gates:** migration round-trip tests (`translate_from_old(old)` computes/hashes correctly under v2); the **DD5 whole-sweep byte oracle** comparing old-VM v1 vs new-VM-translated v2 output bit-for-bit; dual-read tests across all families; macro-DAG bottom-up tests; the 3-source route sync + new payload-contract tests. **Risk:** highest — this is the migration proper; the palette path is mostly cache-miss (CR18 §4.1 #2), but the solve-score validators (both families) and `probe_signature` are fail-closed if dual-read is incomplete.

---

## Phase 5 — Chips → display-only (coeff/param)

**Objective.** Make text the editable source for coeff/param; chips become read-only visualization. Retire the editable JS + the pre-VM by-name dispatch. **Per-DSL staging:** coeff/param → display-only; **root stays chip-editable** (no text parser shipped, 3.2); **solve-score stays chip-primary**.

### 5.1 Retire (cp/pp editable machinery)
- `_chipPickers` `pp`/`cp` entries + picker engine: `js/08-chip-editors.js:120-186` (and the pp/cp wrappers `:188-253`).
- Mutators' cp/pp branches: `addChip` (`js/07-transform-catalogs.js:1335`), `removeChip` (`:1368`), `moveChip` (`:1387`), `updateChipParam` (`js/08-chip-editors.js:1279`).
- Editable shell arms: `_paramProgramChipShellHtml` editable arm (`js/09-render-orchestration.js:172-177`), `_coeffProgramChipShellHtml` editable arm (`:184-189`), and the `<select>/<input>` emitters with `updateChipParam` onchange (`js/08-chip-editors.js:1326,1339,1350`).
- **Retiring the pre-VM by-name dispatch requires removing or translating Chain mode first.** `lookupCoeffTransform` (`sweep_cli.c:3246`), `parseCtChain` (`:5641`), `parsePtChain` (`:5576`) are the **live Chain-mode** parsers — `pipeline_mode_from_params` (`pipeline_programs.py:35`) still returns `chain` vs `program`, and `coeff_transforms`/`param_transforms` arrays still flow to native (`parsePtChain` called at `sweep_cli.c:7740,7799`). So Phase 5 cannot just delete them: it must **either remove Chain mode entirely** (migrate all saved Chain-mode payloads to Program v2 + drop the `chain` branch in `pipeline_mode_from_params`) **or translate every Chain-mode request to a Program at the Lambda boundary before native**. Pick one and gate the deletion on it — a prerequisite, not a cleanup.

### 5.2 Keep (readonly renderers)
`_renderCoeffProgramChipHtml`/`_renderParamProgramChipHtml` with `{readonly:true}` (`js/09-render-orchestration.js:300,337` — already support it), `_chipReadonlyValueHtml` (`js/08-chip-editors.js:1295`). These become the display layer over parsed text; the readonly call sites already exist (`:388,:395`). Chips-as-display needs the parser to emit **per-chip source spans** (an editor parser, for click-to-locate / error underlining) — build alongside Phase 3's parsers.

### 5.3 Touch points
Dispatch maps to update when retiring cp/pp: `_chainForWhich` (`js/02-preview-solvescore.js:675`), `_catalogForChain` (`:662`), `_renderChips` (`js/09-render-orchestration.js:399`). `rt` editable path (`_renderRtChipHtml`, `_rtCatalog`) stays. **Gate:** the frontend split-load + parts-contract gate (`tests/test_frontend_js.sh`); Playwright for the new text editors. **Risk:** low once Phases 3–4 land.

---

## Cross-phase summary

| Phase | Ships standalone? | Hard gate before it | Primary risk |
|---|---|---|---|
| 0 versioning | yes (pure insurance) | — | regression if missing≠v1 (mitigated) |
| 1 chip registry | yes (kills drift) | — | semantics in the schema; param drift gate |
| 2A shared runtime (VM merge) | yes (one interpreter) | DD5 oracle + DD6 benchmarks | FP determinism, perf |
| 2B lag facility (separate from 2A) | yes | 2A landed + same DD5 oracle | FP drift — kept bisectable by the split |
| 3 text parser (param) | yes (param text-first; root stays chips) | — | low (param ⊂ coeff) |
| 4 v2 migration | no (the bump) | DD2 gate (Phase 0) + DD5 | the migration proper |
| 5 chips display-only | no | Phases 3+4 | low |

If appetite is limited: **0 + 1 + 3** deliver versioning insurance, drift elimination, and text-first **param** (root/solve-score stay chips) **without** the v2 fingerprint migration or the native merge — a coherent stopping point that leaves chips editable and fingerprints untouched.

*No production code was modified writing this plan; all current-state references verified against the `1a1996d` source snapshot.*
