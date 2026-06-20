# CR18 — Stack-DSL Unification: Implementation Plan (Phases 0–5)

Companion to `code-review-18.md` (the design review). CR18 holds the *why* and the design decisions (DD1–DD6, the three conflicts C1/C2/C3, the verification prerequisites); this doc holds the *how* — per-phase file maps, native structs, schemas, tests, and migration routes, so each phase is codeable without re-deriving current state.

Verified against the `1a1996d` source snapshot; **all later commits touch only docs (CR18 + this plan) and populate-source persistence — no VM/source change — so every file:line below is current** (no specific "current HEAD" hash is pinned here, since it advances with each doc commit). C/Python/JS paths are under `lambda/`, repo root, and `js/` respectively.

**Dependency order & gates** (nominal — *release* gates, not strict serialization). **−1** → 0 → 1 → 2A → 2B → 3 → 4 → 5, with one explicit overlap: **Phase 4 *implementation* may begin before the §3.6 coeff migration completes** (it operates on compiled chains/`execution_spec`, not the parser), but **Phase 4 cannot *release* until §3.6's equivalence gate passes**. **Phase −1 (the verification harness) is a hard prerequisite for Phase 2** — DD5 (a Python solve-score oracle + the whole-sweep byte oracle) and DD6 (benchmarks) must exist before any interpreter merge (see Phase −1). Also: **DD2 native version gate must precede any opcode renumbering (Phase 4)**. Phases −1, 0, 1, 3 are independently shippable with value; Phases 2, 4 are the risk concentration. **Phase 1 now also owns the profile symbol table** — the parser/IR/frontend profiles must be table-driven before the Phase-3 param source parser and the Phase-4 merged IR land, otherwise the new parser repeats the hardwired `t1/t2/p1/p2/cf/poly` split that CR18 is trying to eliminate. The Phase-1 profile table is **not** the native runtime's source of truth in Phase 2A; native caps/layout stay in C and are drift-checked against the profile table (§1.4), keeping the FP-critical path out of JSON.

---

## Phase −1 — Verification harness (DD5 + DD6): build BEFORE Phase 2

The gates Phase 2 depends on are a standalone deliverable; Phase 2A cannot start without them. (Independent of Phases 0/1/3, which don't need it.)

**Implementation status:** landed as the Phase −1 harness in `lambda/solve_score_eval.py`, `tests/test_solve_score_native_parity.py`, `tests/oracle_runner.py`, `tests/test_whole_sweep_oracle.py`, `tests/fixtures/oracle/`, `scripts/oracle_baseline.sh`, and `tests/bench_vm.py`. The harness compiles native test binaries from source; it does not trust checked-in native binaries.

**(a) Python solve-score evaluator — the DD5 oracle for solve-score.** Today `solve_score_chain.py` only *compiles* — no numeric eval, so solve-score has no cross-language oracle (CR18 §7.1). Add **`lambda/solve_score_eval.py`**: a pure-Python mirror of `solve_score.h` — the metric prepass (`compute_solve_metric_score`), the postfix scalar VM, lag (`mN-1`), and emit — taking roots/coeff/param inputs → scalar or N channels. Pin it with **`tests/test_solve_score_native_parity.py`**: feed the same roots `.bin` + program to both `solve_score_eval.py` and the C binary, assert agreement (epsilon for floats; exact for SHA-addressed bytes).

**(b) Whole-sweep byte oracle — DD5 for the VM merge.** **`tests/test_whole_sweep_oracle.py`**: for a corpus of warm-start-sensitive programs (clustered / near-degenerate roots), run the FULL coeffgen + solve and compare the complete coeffs `.bin` + roots `.bin` **byte-for-byte** between the pre-merge binary and the post-merge binary — single-cell parity is insufficient under the serpentine warm-start cascade (CR18 §7.1). Snapshot the old-binary outputs as a baseline (**`scripts/oracle_baseline.sh`**) *before* Phase 2A touches the interpreter. **Artifact locations (pin, don't drift):** the warm-start **corpus** is **checked in** as `tests/fixtures/oracle/programs/<name>.json` (one program + its inputs per file) with an index `tests/fixtures/oracle/corpus.json`; the byte baselines are **checked-in sha256 manifests** `tests/fixtures/oracle/baseline/<name>.sha256` (small — not the raw `.bin`), and the test regenerates the `.bin` under its own tmp dir and compares to the manifest (deterministic, not "works on my machine"); **bench results** go to `reports/` (gitignored — added to `.gitignore` in Phase −1) and gate on a **threshold**, not absolute numbers. **Baseline build discipline:** generate the baseline from a **clean checkout at a named/tagged commit with freshly rebuilt binaries**, never from binaries dirty in the local worktree, so the oracle pins a reproducible reference, not local build drift.

**(c) Per-profile benchmarks — DD6.** **`tests/bench_vm.py`** (or `scripts/bench_vm.sh`) timing the 5 profiles — param-only, coeff scalar-heavy, coeff vector-heavy, solve-score multi-channel, root-raster — plus per-thread workspace bytes, vs the current tight-`switch` baseline.

**Commands:** `uv run python -m pytest tests/test_solve_score_native_parity.py tests/test_whole_sweep_oracle.py`; `uv run python tests/bench_vm.py --against-baseline tests/fixtures/oracle/bench_baseline.json`. **Files:** `lambda/solve_score_eval.py`, `tests/test_solve_score_native_parity.py`, `tests/test_whole_sweep_oracle.py`, `tests/bench_vm.py`, `scripts/oracle_baseline.sh` + the warm-start corpus under `tests/fixtures/oracle/`. **Gate:** parity green + bench within the configured threshold (default 10%; tighten only after host variance is measured) before any interpreter merge in Phase 2A/2B.

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
**Rollback floor (DD2):** Phase 0 itself is **rollback-safe** — v1 payloads decode identically on pre-Phase-0 binaries (the version field is inert for v1). The floor rises only at **Phase 4**: once `program.version=2` payloads/artifacts exist, pre-Phase-0 binaries (which ignore the field) would mis-decode them as v1, so they stop being valid rollback targets *from Phase 4 onward*. Record the **Phase-4** floor in the deploy runbook.

### 0.2 `spec_version` sibling at WRITE sites
Add a **family-specific** version int (=1 now) beside each persisted fingerprint — **not a single generic `spec_version`**, because one meta object can carry several families: `handler_palette_finalize.py:533` writes `palette_variant_fingerprint`/`content_fingerprint` *and* `:568` writes `solve_score_chain_fingerprint` in the **same** `meta.json`, so one sibling can't say which it versions. Use `solve_score_spec_version`, `palette_variant_spec_version`, `probe_signature_spec_version` — **explicit per-family sibling fields** (not a generic `spec_version`, and not a single combined map). Missing ⇒ 1. (The solve-score *family* itself spans three fingerprint **scopes** — base / `palette_source` / `associated_palette`, `solve_score_chain.py:146,154` — so it takes three explicit scope siblings, §0.2 table.) **All these families take a new sibling field — including the solve-score fingerprint *embedded* in the fragment manifest **`RENDER_FINALIZE_MT_FRAGMENT_MANIFEST` (`workflow_contracts.py:61-69`, which forwards `chain_fingerprint` but no sibling today)** and raw-sidecar, whose `solve_score_spec_version` sibling lives *inside* the container as a **new key** (NOT the container's own `version:1`, which is fail-closed — see table); Phase 0 adds that key to the manifest **and the rendered ASL that fills it**. Only the coeff/param *program* fingerprint takes no sibling (it's versioned by the in-hash `PROGRAM_VERSION`).**

| family | representative WRITE sites | action |
|---|---|---|
| solve-score `chain_fingerprint` / `program_spec` | clip `handler_solve_proximity.py:913`; section-hist `:1442`; bins `:1583`; palette meta `handler_palette_finalize.py:534,568`; render plan `handler_palette_render_plan.py:817,1086`; raster `handler_raster_mt.py:587`; extract `handler_extract_palette_from_step_scores.py:545`; recolor `color_recolor_raw.py:419`; saved programs `handler_storage.py:313,428,489` | add **per-scope** siblings: `solve_score_spec_version` (base) + **`palette_source_spec_version`** + **`associated_palette_spec_version`** — one meta.json can carry all three (`solve_score_chain.py:146,154`), so one sibling is ambiguous |
| palette `palette_variant_fingerprint` / `content_fingerprint` | `handler_palette_render_plan.py:463,840,1109`; `handler_palette_finalize.py:533` | add **`palette_variant_spec_version`** sibling |
| `probe_signature` | `handler_coeffgen.py:670` (built by `compute_fused.py:34`); stored in plan `handler_compute_plan.py` | **`probe_signature_spec_version`** (recompute under the program's versions) |
| raw sidecar (embeds `chain_fingerprint`, `raw_sidecar.py:222`) | container `version` (`raw_sidecar.py:9-11`) | **add `solve_score_spec_version` *inside* the sidecar — do NOT bump the container `version`** for a fingerprint-scheme change: `validate_raw_sidecar` (`raw_sidecar.py:268`) *rejects* unknown container versions, so a bump is **fail-closed on old readers** (rollback breaks). Reserve the container `version` for actual sidecar-layout changes. |
| fragment manifest (embeds `chain_fingerprint`, `handler_finalize_mt.py:523`) | container `version` (`handler_finalize_mt.py:35`) | **add `solve_score_spec_version` *inside* the manifest** — same reason: `handler_finalize_mt.py:69` rejects unknown `version` (fail-closed). |
| coeff/param **program** fingerprint | `handler_compute_plan.py:279,286`; folded into `probe_signature` (`compute_fused.py:34`) | **no sibling needed** — versioned by the **in-hash `PROGRAM_VERSION`** (`coeff_program_chain.py:1977`: `sha1({version, execution_spec})`, = DD2's `program.version`). A v2 bump re-keys it automatically; `probe_signature` (which folds it) carries `probe_signature_spec_version`. |

Do **not** rename existing fingerprint fields (a rename is itself a wire change). Do **not** fold `spec_version` into the hashed payload (`compiled_solve_score_fingerprint`, `solve_score_chain.py:641`) — that re-keys every artifact (the whole point of "adjacent").

### 0.3 Dual-read at the 14 READ/compare sites
Read the stored **family** version (`solve_score_spec_version` / `palette_variant_spec_version` / `probe_signature_spec_version` — and for raw-sidecar/fragment-manifest the `solve_score_spec_version` sibling **inside** the container, *not* the container's own `version`; **missing ⇒ 1**) → compute the matching-scheme fingerprint → compare; during transition compute *both* and accept a match on the stored version.

**`program_spec` string compares need a *versioned compare API*, not just a versioned hash (blocker).** The sites at `handler_raster_mt.py:263` / `handler_palette_chunk.py:421` / `handler_solve_proximity.py:415,724` / `handler_finalize_mt.py:282` compare the *canonical string* directly (`canonicalize_solve_score_program_spec(...)`). Versioning the `sha256` does **not** cover these. Add `canonicalize_solve_score_program_spec(spec, version)` and compare `canonicalize(stored, stored_ver) == canonicalize(expected, stored_ver)`. **Phase 0 ships the *signature* with v1-only behavior** (`version=1` is the only implemented scheme — no behavior change); **Phase 4.3 adds the v2 renderer branch.** So Phase 0 lands the versioned API + dual-read plumbing now, without the v2 scheme — they do **not** have to land together.
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

## Phase 1 — Structural-chip registry + profile symbol table (single source of truth)

**Objective.** A JSON registry for the **program chips** (the `_CHIP_COMPILERS` stack-VM vocabulary) plus a **profile symbol table** for param/coeff/root/solve-score. The chip registry drives **registry metadata + arg/stack/selector validation + the dispatch map** (which named handler to call) for Python/C/JS, gated by a drift test — **not** data-only compilation: the semantic compiler handlers stay as named functions (see §1.2). The symbol table makes identifiers (`t1`, `p1`, `cf`, `poly`, `poly_len`, metric slots, roots) profile configuration, not parser magic. **This is new ground:** today the ~25 program chips are hardcoded Python compiler functions with *no* registry, *no* generated JS mirror, and *no* drift gate (only the 28 *transforms* in `coeff_legacy_registry.json` are registry-driven), and parser/lowerer code directly knows each program kind's register names. No runtime VM change: Phase 1 feeds Python parser/validation/frontend metadata, while C keeps its existing constants and receives a drift test.

**Implementation status (2026-06-20): landed.** `structural_chips.json` and `program_profiles.json` are checked in; `gen_coeff_vocab.py` emits the structural-chip/profile metadata into `coeff_vocab_js.js`; `gen_program_profiles.py` emits `lambda/program_profiles.py` plus `program_profiles_js.js` and both are byte-checked by predeploy. `program_source_core.py` imports the generated Python profile mirror; frontend Coeff Program selector choices derive from the generated profile mirror instead of hardcoded arrays. Drift coverage pins Coeff op/expr/selector/sub-op/structural-chip/profile metadata, Param op/expr/selector/profile metadata, generated JS/Python mirrors, and deploy packaging.

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
Arg `type ∈ {int, real, complex, selector, enum, registry_ref}`. `synth_only` marks the `_typed_*` ops (`get_scalar/set_poly/poke_poly/fill/blend`) the source layer emits internally. The **full inventory** is the per-chip table the audit produced: 30 `COEFF_OP_*` entries spanning `push_const/push/emit/dup/swap/pop/flush/blend/legacy/poke_poly/poke_tos/vector_binary(+5 fns)/vector_unary(+16 fns)/vector_roll/argsort/littlewood/linspace/range/set/affine/typed_push_scalar/typed_push_vector/typed_binary/typed_unary/typed_get_scalar/typed_set_poly/typed_poke_poly/typed_fill/native_transform(+stack_args)/typed_blend/macro` — plus the Tier-D registry-name fallthrough (`coeff_program_chain.py:1642`). Capture the arg-order quirk: legacy/native rows are **source-first** `[name,src,tgt,…]`, named chips are **target-first** `[tgt,src,…]` (`coeff_program_chain.py:1570,1636`; the named "LAYOUT CONTRACT" comment lives in `js/08-chip-editors.js:998`).

**Sub-op families need their own namespace table (they are NOT single chips).** `vector_binary` (opcode 12) is a *family* of 5 sub-ops `add/subtract/multiply/divide/power` selected by `fn_index` (`VECTOR_BINARY_OPS`, `coeff_program_chain.py:218`); `vector_unary` (13) is 16 sub-ops (`VECTOR_UNARY_OPS:226`, with `mod`→`abs` canonicalization, `:216`); `vector_roll` (14) is 2; `typed_binary`/`typed_unary` reuse the same sub-op ids via aliases (`sub/mul/div/pow`, `:210`). The schema must model each family with a nested `sub_ops: [{fn_index, name, aliases, source_name, display}]` — pinning every sub-op's `fn_index`, canonical name, **wire/source aliases**, and **display name**, exactly as `coeff_legacy_registry.json` pins transform fn_index/aliases — and the drift gate must pin the sub-op ids too (not just the family opcode).

### 1.2 Generator — extend `gen_coeff_vocab.py`
Today `build_vocab()` emits `window._coeffRegistryVocab = {names, fnIndexByName, aliasToCanonical, sourceAliasByName, chipNameByRegistryName, supportsAndy, ctCatalog, categoryMeta, programParamDefs}` to `coeff_vocab_js.js` (transforms only). Add a parallel `build_structural_chips()` emitting:
- **Python:** a generated table supplying the **metadata + dispatch map** for `_CHIP_COMPILERS` / `_ZERO_ARG_CHIP_OPS` / the name-family tables (`coeff_program_chain.py:1584,1595,1632`). The registry says *which* named handler to call and drives arg/stack/selector validation; it does **not** replace the handler bodies — the semantic functions (e.g. `_compile_affine`, the linear/exp/pow arg-packing special cases) remain **named compiler handlers referenced by `lower_to`**. (A registry can't magically encode every special case; "registry-driven metadata + validation + dispatch," not data-only.)
- **C:** a generated header (chip→`COEFF_OP_*`→stack-effect/arg-shape) the dispatch (`sweep_cli.c:5130`) and parser validate against.
- **JS:** `_coeffProgramCatalog` (today hand-maintained in `js/07-transform-catalogs.js`).
Keep the `--check` byte-equality drift mechanism (`gen_coeff_vocab.py:88`).

### 1.3 Drift gate — extend `tests/test_coeff_program_drift.py`
The existing test pins `COEFF_OP_*` / `COEFF_EXPR_*` / `COEFF_SEL_*` enums + the 28-transform fn_index + aliases + generated-JS byte-equality (`:45,:113,:152,:190`). Add: pin every structural-chip `name → opcode → stack_effect → args → selectors` across registry/Python/C/JS. **Also bring the param VM under the gate** — today `test_coeff_program_drift.py` only scrapes `COEFF_*`; `PARAM_OP_*` / `PARAM_EXPR_*` have *no* drift test (CR18 C2 risk). Add `test_param_program_drift.py` mirroring it. *This is a Phase-1 deliverable because C2 (the fn_index/opcode reconciliation) lands in untested param space otherwise.*

### 1.4 Profile symbol table — `program_profiles.json` (new)
Add a profile registry that the source parsers, validation layer, generated JS metadata, and drift tests consume. The C runtime does **not** consume this JSON in Phase 2A; it keeps its reviewed constants/layout/init code and is checked against the profile table. This is the simplifying architecture where it is useful: **same parser core, same typed IR vocabulary, different profile symbol tables + small lowering hooks**. The parser should ask "what is this identifier in this profile?" instead of hardcoding `p1` for Param and `poly` for Coeff in separate branches.

Schema sketch:
```jsonc
{
  "profile": "param",
  "value_caps": {
    "program_tokens": 64,
    "total_stack": 16,
    "vector_len": 0,
    "vector_capable_stack": 0
  },
  "symbols": {
    "t1": { "type": "scalar_complex", "access": "read", "role": "input", "contexts": ["expr", "selector"] },
    "t2": { "type": "scalar_complex", "access": "read", "role": "input", "contexts": ["expr", "selector"] },
    "p1": { "type": "scalar_complex", "access": "read_write", "role": "output", "initial": "t1", "contexts": ["expr", "lhs", "selector"] },
    "p2": { "type": "scalar_complex", "access": "read_write", "role": "output", "initial": "t2", "contexts": ["expr", "lhs", "selector"] }
  },
  "selectors": {
    "legacy_src": ["p1", "p2", "both", "pop1", "pop2"],
    "legacy_tgt": ["p1", "p2", "both", "push1", "push2"]
  },
  "outputs": ["p1", "p2"],
  "output_contract": { "kind": "scalar_registers", "default_initialization": true }
}
```
The same table covers Coeff:
```jsonc
{
  "profile": "coeff",
  "value_caps": {
    "program_tokens": 256,
    "total_stack": 64,
    "vector_len": 256,
    "vector_capable_stack": 64,
    "max_args": 8,
    "scalar_exprs": 64,
    "expr_tokens": 32
  },
  "symbols": {
    "t1": { "type": "scalar_complex", "access": "read", "role": "input", "contexts": ["expr"] },
    "t2": { "type": "scalar_complex", "access": "read", "role": "input", "contexts": ["expr"] },
    "p1": { "type": "scalar_complex", "access": "read", "role": "input", "contexts": ["expr"] },
    "p2": { "type": "scalar_complex", "access": "read", "role": "input", "contexts": ["expr"] },
    "cf": { "type": "vector_complex", "access": "read", "role": "input", "contexts": ["expr_index_base", "selector", "rhs"] },
    "poly": { "type": "vector_complex", "access": "read_write", "role": "output", "initial": "cf", "contexts": ["expr_index_base", "selector", "lhs", "rhs"] },
    "poly_len": { "type": "int", "access": "read", "role": "derived", "from": "poly.length", "contexts": ["expr", "length_arg"] },
    "tos": { "type": "stack_pseudo", "access": "read", "role": "stack_top", "contexts": ["expr_index_base"] }
  },
  "selectors": {
    "src": ["cf", "poly", "pop", "peek"],
    "tgt": ["poly", "push"]
  },
  "outputs": ["poly"],
  "output_contract": { "kind": "fixed_vector", "initial": "cf", "fixed_output_length": true }
}
```
Root and solve-score also get profiles even if they stay chip-primary in v1:
- **root:** `roots` is a `vector_float` read/write input-output symbol, in-place, length-preserving, NaN-poison policy, raster-stage execution context.
- **solve-score:** `m0..m15` is a generated `scalar_real` metric-slot **namespace** with optional lag reads (`mN-1`), not a static binding — the actual metric/source/quantile bindings still arrive at runtime through `--score_metrics` / `--score_sources` / clips (§4.1a). Output is implicit terminal scalar or explicit `emit*` channels; stack is scalar-only.

**Caps semantics:** `program_tokens` mirrors the native token cap (`PARAM_PROGRAM_MAX_TOKENS=64`, `COEFF_PROGRAM_MAX_TOKENS=256`). `total_stack` is the total VM stack depth. `vector_capable_stack` is the subset of stack slots that may hold vectors (`coeff: 64/64`, all stack slots vector-capable; `param: 16/0`, no vector stack). `vector_len` is max vector length. `max_args` / `scalar_exprs` / `expr_tokens` are included only where that profile owns those wire caps; if Phase 2A keeps the coeff expression arena C-only, the drift test still asserts these profile values equal the C constants rather than using JSON at runtime.

**Selector authority:** selector verbs live in the **profile**, not scattered across chip schemas. `structural_chips.json` declares selector *slots* (`src`, `tgt`, `legacy_src`, `legacy_tgt`), while `program_profiles.json` declares which selector tokens are legal for each slot in each profile. The drift gate checks the join: every chip's selector slot must resolve to profile selectors, and every selector accepted by Python/C/JS must appear in the profile. This prevents a new split-brain between `src_selectors` in structural chips and `contexts:["selector"]` on value symbols.

**Symbol fields:** `type ∈ {scalar_complex, scalar_real, int, vector_complex, vector_float, stack_pseudo, metric_slot}`; `access ∈ {read, write, read_write}`; `role ∈ {input, output, derived, scratch, stack_top, metric}`; `initial` is a symbolic initializer (`p1=t1`, `p2=t2`, `poly=cf`); `contexts` gates where the name is legal (`expr`, `lhs`, `selector`, `rhs`, `expr_index_base`, `length_arg`, `emit`). Invalid cases become uniform diagnostics: `cf[i] = ...` fails because `cf.access=read`; `p1 = ...` is legal in Param and illegal in Coeff because Coeff `p1.access=read`; `poly_len = ...` fails because derived symbols are not writable.

**Generated outputs:** `program_profiles.py` / JS mirror. The Python parser uses the table for identifier resolution and source diagnostics; the Python lowerers use it to select profile-specific register opcodes; the frontend uses it for editor help, autocomplete, and disabling illegal assignments before save. C gets a generated **drift-check header/report**, not a runtime dependency in Phase 2A: native constants/layout/initializers stay in C for FP determinism, and `test_program_profiles_drift.py` asserts the JSON profile matches C (`PROGRAM_MAX_TOKENS`, stack caps, vector caps, output contract flags, selector enums). If a later phase chooses to generate C config from the profile table, that must explicitly enter the DD5 byte-oracle blast radius.

**Important boundary:** the symbol table is not a replacement for semantic hooks. It solves name lookup, access control, type/kind checks, initialization, and output contracts. Profile-specific lowering still exists for solve-score metric-slot CSE/lag, coeff fixed-vector enforcement, root float/NaN behavior, and legacy transform packing. This keeps the parser/config readable instead of turning JSON into a hidden programming language.

**File map:** new `structural_chips.json`, new `program_profiles.json`, generated `program_profiles.py` + JS mirror + C drift-report/header, `param_legacy_registry.json` drift coverage; `gen_coeff_vocab.py` (split to `gen_program_metadata.py` if useful); `coeff_program_chain.py` / `param_program_chain.py` (dispatch + identifier validation driven from registries); generated C drift header/report; `js/07-transform-catalogs.js` (catalog generated); `tests/test_coeff_program_drift.py` + new `test_param_program_drift.py` + new `test_program_profiles_drift.py`. **Risk:** low-medium — bigger than a vocab-gen (the chips carry compiler semantics, §CR18 Phase 1), with expanded validation/frontend blast radius but **no runtime behavior change**; the drift gate de-risks divergence.

---

## Phase 2A — Shared runtime (VM merge) behind frozen front-ends

**Implementation status (2026-06-20): landed.** `sweep_cli.c` now lowers coeff `scalar_exprs` once at parse/load into cached expression plans, evaluates them in a hidden workspace temp arena, resolves all token args/`andy` before dispatch, and enforces the probed fixed-output length inside `evalCoeffProgram` for row paths. `paramEvalScalarExpr` intentionally remains as the legacy nested-expr compatibility path until the Phase-4 drain. Gates run: coeff native/compiler tests, DD5 whole-sweep oracle, full pytest, predeploy, DD6 benchmark.

**Objective.** One native interpreter core behind byte-identical per-VM front-ends (DD1 adapter) + the expression-temp arena (DD1 + §2a isolation). **The solve-score lag refactor is split out into Phase 2B** — two unrelated high-risk native changes (VM merge vs lag double-buffer) must be *separate phases behind the same DD5 oracle*, so any numerical drift bisects to one of them. Fingerprints do not move (front-ends frozen). Gated by native parity + DD5 + DD6.

### 2.1 Unified value model + workspace (native structs)
Merge `CoeffProgramWorkspace` (`sweep_cli.c:3504-3536`: planar `stack_re/stack_im[64][256]`, scalar slots, `poly`, scratch/original/aux) and the param state (`ParamProgram`/`ParamProgramToken`/`ParamCx`, `sweep_cli.c:6097-6128`) into one runtime with explicit per-profile C config (enabled ops, register layout, initialization rules, output contract, caps coeff 256/64/256 vs param 64/16). The generated profile table (§1.4) is a **drift-checked mirror**, not a runtime dependency, so a JSON typo cannot perturb the FP-critical path. **Per-profile workspace sizing** (CR18 §3.1, corrected): size to the profile's caps; do not assume a single shared constant (note `MAXDEG` is *inconsistent* across rasterizers — `roots2pix_mt.c:28` 256 vs others 1024 — reconcile, not "size for 1024"; roots can't exceed `MAX_DEGREE 255`).

**Fixed-output-length contract (CR18 §7.5 — must be in the runtime config).** Coeff output length is probed once at (0,0) and **hard-asserted constant** for the whole grid (`sweep_cli.c:7841,7948`), and the warm-start chain depends on it. The unified config needs a `fixed_output_length` flag (true for coeff) that makes the interpreter **reject a per-row length change** — load-bearing once the unified op set includes dynamic indexing (`typed_get_scalar`) and length-changing transforms (`length_policy: may_change`), which could otherwise vary `poly_len` per cell. Other profiles have their own output contracts: param → `p1,p2` scalars; root → in-place, same-length; solve-score → 1 scalar or N channels. **Where the expected length lives (a boolean isn't enough):** the (0,0) degree probe (`sweep_cli.c:7841`) computes it once; pass it into the per-row eval as `expected_out_len`, and the interpreter rejects any row whose `poly_len != expected_out_len` — surfaced by the **row loop** (retaining today's outer assertion at `:7948`), never a silent truncation. So `fixed_output_length` means "assert `poly_len == expected_out_len`, where `expected_out_len` is the probed length passed into eval."

### 2.2 Expression-temp arena (DD1 implementation sketch — the §2a isolation)
Add a **separate** per-thread arena distinct from the user value planes (**coeff-only** — this whole section concerns the coeff expr-plan lowering; param builds no arena in Phase 2A, §2.3):
- *struct:* `double tempRe[N]/tempIm[N]`, `N = COEFF_PROGRAM_MAX_EXPR_NUMS/COEFF_PROGRAM_EXPR_STRIDE` (the existing private-stack sizing at `:3869`).
- *cap:* the existing `MAX_EXPR_NUMS/STRIDE`; a single expression's lowered sub-sequence must fit it (compile-time check). **Separate from** the user 64/16 cap.
- *frame base / `tos[i]` — ONE base per TOKEN, not per arg (load-bearing):* capture a single `token_frame_base = user_sp` **before evaluating any of the token's dynamic args**; every arg's `tos[i]` reads `user_stack[token_frame_base-1-i]` — the **pre-token** stack. This prevents a multi-dynamic-arg token from letting arg 2 see arg 1's pushed result through `tos0`. Arg intermediates live only in the hidden arena; `user_sp` grows only by the final pushed arg results.
- *Two arg-delivery mechanisms (full ABI in §2.2a):* (i) **resolved-arg ops** — the ~12 ops that read `coeffArgValue` today (affine/const/littlewood/poke/set/blend/typed/…) take args from a per-token **resolved-arg array**, *not* the user stack; (ii) **stack-arg `native_transform`** (the `stack_arg_count` form only) pops K args off the user stack (each a net **+1** push, intermediates in the arena). Most ops are (i); (ii) is the minority path.
- *Mixed literal + dynamic args:* a token can carry some literal args and some `expr_refs` (e.g. `affine(tgt,src, 2.0, p1*cf[0])`). Resolve **all** args in positional order (literal → copy `token.args[i]`; dynamic → run the lowered plan into the arena → slot `i`); resolved-arg ops read them from the array, the stack-arg form pushes them in order. See §2.2a.
- *`andy_expr_ref` must be lowered too (this is the bug class we already hit):* `andy` is evaluated by a *separate* call today — `coeffAndyValue` → `coeffEvalScalarExpr` (`sweep_cli.c:4118-4119`, used at `:4619`). Deleting the evaluator (§2.3) without lowering `andy_expr_ref` would dangle it. The lowering must treat `andy_expr_ref` as one more arg expression (into the arena), its result consumed by the transform's blend-back.

### 2.2a Arg-resolution ABI + lowered-plan storage (the Phase-2A native contract)
`coeffArgValue` (`sweep_cli.c:4058`) — today literal-or-`coeffEvalScalarExpr` — is replaced by a pre-resolved array filled before each op dispatches. The concrete native contract (so implementers don't invent incompatible mechanics):
```c
// Built ONCE at parse/load (load-time lowering, §2.3), per program:
typedef struct { uint16_t op; uint16_t fn_index; int16_t index; double a, b; } LoweredExprToken;        // a typed-stack token
typedef struct { LoweredExprToken toks[COEFF_PROGRAM_MAX_EXPR_NUMS/COEFF_PROGRAM_EXPR_STRIDE]; uint16_t n; } LoweredExprPlan;
LoweredExprPlan expr_plans[COEFF_PROGRAM_MAX_SCALAR_EXPRS];           // indexed by the OLD expr_ref id; token.expr_refs[i] still indexes it

// Per token, BEFORE op dispatch (replaces the scattered coeffArgValue calls):
typedef struct { double re[COEFF_PROGRAM_MAX_ARGS], im[COEFF_PROGRAM_MAX_ARGS]; int n;
                 double andy_re, andy_im; int has_andy; } ResolvedArgs;
// resolve_token_args(tok, ctx, arena) -> ResolvedArgs:
//   for i in 0..tok->n_args:
//     rargs.re/im[i] = (tok->expr_refs[i] >= 0) ? run expr_plans[tok->expr_refs[i]] in the arena   // §2.2 frame-base rules
//                                               : (tok->args[i], tok->args_im[i]);                  // literal
//   andy: tok->andy_expr_ref>=0 ? run that plan -> andy : (tok->andy ? literal : none)
```
**Op classification (implementer checklist):**
- **resolved-arg ops** (read `rargs.re/im[i]`): const, affine/linear, littlewood, poke_poly/poke_tos, set, blend, the typed family, and the *non-stack* `native_transform`/legacy arg path — i.e. every `coeffArgValue` site at `sweep_cli.c:4682–5086`.
- **stack-arg op**: `native_transform` **`stack_arg_count` form only** — pops K from the user stack (§2.2 (ii)).
- **andy**: always a resolved scalar (`rargs.andy`), consumed by the transform's blend-back.
The `expr_plans` live in the per-program workspace (`MAX_SCALAR_EXPRS × MAX_EXPR_NUMS/STRIDE`), built once; row eval calls `resolve_token_args` then the op reads `rargs`. This is the shared ABI Phase-2A must pin before touching C. **`LoweredExprToken` fields are op-discriminated:** a `literal` uses `a`/`b` (re/im); a value-ref uses `index` (which `cf[i]`/`tos[i]`/coord, with `op` encoding the source — cf vs tos vs p1/p2); `typed_binary`/`typed_unary` use `fn_index` (the `VECTOR_BINARY_OPS`/`VECTOR_UNARY_OPS` sub-op id, `coeff_program_chain.py:218,226`). `a`/`b`/`fn_index`/`index` unused by an op stay zero.

### 2.3 Retire the nested scalar-expr evaluators
**Phase 2A deletes/replaces only the COEFF evaluator** `coeffEvalScalarExpr` (`sweep_cli.c:3855-4056`). **`paramEvalScalarExpr` (`:6372-6436`) stays as a compatibility path through Phase 2A** — it serves legacy nested-expr param payloads and can only be removed *after* the Phase-4 opcode-renumber drain (§4.6) proves none can still arrive (its new output already carries no `expr_refs`, so it's idle for v2 programs). Replace the coeff evaluator with **native load-time lowering** (once, at parse time — *not* in the per-row loop): the compact `scalar_exprs` wire stays (DD1); **`parseCoeffProgram` (coeff only) lowers it once** into cached temp-arena token sequences, and the per-row evaluator only *executes* those pre-lowered tokens (no per-row re-lowering). **`parseParamProgram` gets no load-time lowering in Phase 2A** — it stays legacy-compatible (legacy nested-expr payloads still run through `paramEvalScalarExpr`) until the Phase-4 drain (§2.3, §4.6); new param programs already arrive pre-lowered (`scalar_exprs=[]`, §2.2a), so there is nothing for it to lower.

**Every `expr_refs` consumer must be migrated — not just `native_transform`.** `coeffArgValue` (`sweep_cli.c:4058`) is read by ~12 ops: affine (`:4682-4683`), exp/pow + legacy arg packing (`:4690-4697`), littlewood (`:4758-4761`), const (`:4808`), set/blend/poke/fill and the typed family (`:4888,4974,4992,5015,5086`), plus `coeffAndyValue` (`:4118`). For **each**, the load-time lowering must materialize its args (literal + dynamic, in order, §2.2) before the op runs — an op whose args aren't lowered would call a deleted evaluator. Treat the consumer set as an explicit checklist; the native-parity gate must exercise each op with a dynamic arg. The merged opcode enum + the param-as-config mapping are **internal only**; the per-VM `execution_spec`/fingerprint stay byte-identical (param still `del scalar_exprs` at `param_program_chain.py:1107`; coeff spec bytes still wire, `coeff_program_chain.py:1838`).

**Param needs no resolved-arg ABI — only a drain-gate.** The new param compiler lowers *every* expression into main tokens (`_expr_to_param_tokens`; `_lower_chain` returns `scalar_exprs=[]`, `param_program_chain.py:1005`), so new param tokens carry **no `expr_refs`**, and `paramArgValue`/`paramEvalScalarExpr` (`sweep_cli.c:6438`) are reached **only by legacy nested-expr payloads** the C still accepts (`test_param_program_native.py`). So §2.2a's `ResolvedArgs`/`expr_plans` are **coeff-only** (coeff alone keeps the side-table, DD1); for param, just **drain-gate the deletion** — remove `paramEvalScalarExpr` once no legacy nested-expr payload can still arrive (the same opcode-renumber drain, §4.6). No param arg-plan to build.

---

## Phase 2B — Centralize the solve-score lag facility

**Implementation status (2026-06-20): landed.** `solve_score.h` owns `SolveScoreLagStream` (current/recent metric buffers, first-row lag warm-up, and row advance). The five previous hand-rolled lag loops in `roots2pix_mt.c`, `solve_palette_chunk_mt.c`, `solve_proximity_hist_sectioned.c`, and both `solve_proximity_stats.c` summary paths now use per-file previous-row accessors plus the shared stream. Gates run: targeted solve/palette/proximity tests, DD5 whole-sweep oracle, full pytest, predeploy, DD6 benchmark.

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

**Phase 2A file map:** `sweep_cli.c` (interpreter restructure, **delete the COEFF scalar-expr evaluator `coeffEvalScalarExpr` only** — `paramEvalScalarExpr` stays a compat path until the Phase-4 drain, §2.3 — add the **coeff** expr-plan arena; param builds none in 2A). **Phase 2B file map:** `solve_score.h` (lag-stream object), the 4 rasterizers (replace hand-rolled lag). **Gates (both phases, behind the *same* DD5 oracle):** native-parity tests bit-identical to *both* old interpreters; the **DD5 whole-sweep byte oracle** (full coeffgen + roots `.bin`, old-VM-vs-new-VM, on warm-start-sensitive programs — single-cell parity is insufficient, CR18 §7.1); **DD6 per-profile benchmarks** (param-only / coeff scalar / coeff vector / solve-score multi-channel / root-raster + per-thread workspace bytes) vs the current tight-`switch` baseline. **Risk:** the highest C changes — running 2A and 2B as separate phases is precisely what keeps FP-drift bisectable.

---

## Phase 3 — Shared source parser core + Param first, Coeff migrated before v2; root + solve-score stay chip-primary

**Objective.** Build the shared source parser core and converge Param + Coeff on **one source language**: one lexer/parser/AST/expression parser/source diagnostic engine, with profile tables deciding which symbols, assignments, selectors, and operations are legal. **Param ports first only because it is the smaller missing text profile; Coeff must migrate to the same core before Phase 4/v2 IR is considered complete.** **Root stays a chip-editable flat array** (`[name,...params]`): optional read-only text *export*, **not** a source parser (§3.2 — this refines CR18 §9.3, which over-eagerly listed root as "text-first"; root is text-*able* but not worth a source-editing parser, and it stays chip-editable in Phase 5). **Solve-score stays chip-primary** with read-only text export (CR18 §9.3 — its declarative slot-CSE + lag has no clean text shape). The architectural rule is: **one broad grammar, one diagnostic engine, profile symbol tables for identifier legality, and profile hooks for the semantics that are not data-only**. Syntax should be shared where possible; profile errors should be legality diagnostics, not separate parser dialects.

**Implementation status (2026-06-20):** the Param-source increment and Coeff production flip are implemented and gate-passing. Landed: `program_source_core.py` (shared statement/call/assignment/profile utilities and parse/lower/diagnostic orchestration), `param_program_source.py` (Param source parser + canonical `param_source_text_from_chain`), `/compile-param-program-source`, saved-program source precedence, compute-plan/preview/degree-probe/param-debug source plumbing, ASL forwarding, deploy packaging, API manifest regeneration, Param text-editor UI tab, result-populate source restoration, and regression tests. The compute-path helper treats **blank source text as absent** because orchestrators inject blank fields for ASL JSONPath stability; save/fetch still use the stricter saved-payload source rule. Coeff source now runs through the shared core/profile hook boundary described in §3.6; the frozen old Coeff parser remains test-only until the v2 release gate has passed.

### 3.1 `program_source_core.py` + Param profile first, Coeff profile second
Architecture: add **`lambda/program_source_core.py`** containing the shared lexer, statement splitter, expression parser, source-span diagnostics, assignment parsing, call parsing, and profile-symbol resolution. `param_program_source.py` is then a thin profile wrapper: source text → shared parser with `profile="param"` → existing chip list → existing `compile_param_program_chain`. Four entrypoints mirroring coeff (`:855,889,837,748`): `parse_param_program_source`, `compile_param_program_source`, `param_source_text_from_payload`, `split_param_program_statements`. **Do not clone `coeff_program_source.py` wholesale**: move reusable pieces into `program_source_core.py`, and then migrate `coeff_program_source.py` onto the same core once the Param wrapper is green. The shared core must be designed as the **coeff-capable superset** from day one (indexed lvalues/rvalues, source/target selector slots, vector/scalar values, source-first vs target-first call layouts), not as a scalar-only Param parser that Coeff later cannot reuse.

**Mandatory Coeff migration gate (a release gate, not a start gate):** Phase 3 can ship Param text support as a standalone increment, and **Phase 4 may *start*** the v2 IR/translation work — `translate_from_old` operates on compiled chains + `execution_spec`, not on the parser, so the coeff parser doesn't block v2 groundwork. But **Phase 4 cannot *ship*: v2 is not complete until Coeff also uses the shared parser core** and passes byte-equivalence. Because that equivalence makes the old and shared-core parsers interchangeable by construction, the *ordering* of the coeff port vs the v2 work is free; what is not free is shipping v2 with two coeff parser dialects still live. (Full coeff-migration breakdown: §3.6.) The Coeff port is gated by byte-equivalence over a source corpus: old coeff parser → chain/fingerprint must match shared-core coeff parser → chain/fingerprint, including macro expansion, canonical source regeneration, typed-stack expressions, selectors, indexed reads/writes, `poly_len`, `tosN`, and source-first vs target-first layouts. This keeps the user-facing language unified without pretending the Coeff migration is low-risk.

Reuse `param_program_chain`'s expression lowering (`_expr_to_param_tokens`, `:700`) and target `_lower_chip` (`:950`) for the final chip-list compile; derive op tables from the chain layer plus `program_profiles.json` so vocabularies cannot drift. **Param profile grammar:** the canonical emit is assignment to writable output symbols (`p1 = expr` / `p2 = expr`, discovered from the profile table, **not** hardcoded parser magic and **not** `emit p1`); the stack ops `push` (= both) / `push(t1)` / `push(t2)` + `emit_p1` / `emit_p2` / `dup` / `swap` / `pop` / `flush`; the targetable-unary sugar `square(p1)` (which means `p1 = square(t1)` by the Param profile's "matching grid coord" hook, not a universal parser rule); `legacy(name,src,tgt,…)` (the load-bearing bridge — param *cannot* ban it the way coeff text does; its name+selectors are a distinct arg category from expr args), `both` target default. Error type `ParamProgramSourceError` (line/col) mirroring `CoeffProgramSourceError` (`:125`).

**Grammar (EBNF sketch — one broad shared-core grammar; the productions above are the Param profile *subset*, the block below is what the Coeff profile adds):**
```
program       := statement ((";" | "\n") statement)*
statement     := assign | coeff_assign | unary_stmt | legacy_call | registry_call | selector_call | macro_call | stackop | stmt_chip
assign        := output_symbol "=" expr                       # CANONICAL emit (Param: p1/p2; profile table decides)
unary_stmt    := unary_name "(" writable_symbol ")"           # Param sugar: square(p1) -> p1 = square(t1); see note
legacy_call   := "legacy" "(" name "," sel "," sel ("," expr)* ")"   # bridge: name + 2 selectors, then EXPR args
registry_call := name "(" [ expr ("," expr)* ] ")"           # bare pt_* transform: EXPR args only
macro_call    := "macro" "(" id ")"
stackop       := "push" | "push" "(" stack_push_symbol ")" | "emit_p1" | "emit_p2" | "dup" | "swap" | "pop" | "flush"
                                                              # push bare = both; t1/t2 only — push(p1)/push(p2) rejected (param_program_chain.py:790)
expr          := <shared expression parser; identifiers resolved by the active profile symbol table>
sel           := <legacy selector symbol from the active profile>             # Param: p1,p2,both,pop1,pop2,push1,push2
output_symbol := <symbol with role=output and write access>                   # Param: p1,p2
writable_symbol := <symbol with write access>                                 # Param: p1,p2
stack_push_symbol := <profile-allowed explicit push source>                   # Param: t1,t2
unary_name    := "square" | "cube" | "conj" | "negate" | "reciprocal" | "unit_circle" | "exp"
# --- Coeff profile adds (Param above is a subset; authority = coeff_program_source.py:689 _lower_statement) ---
coeff_assign  := coeff_lvalue "=" rhs                        # Coeff: poly = … / poly[i] = …
coeff_lvalue  := "poly" | "poly" "[" expr "]"               # whole-vector or indexed lvalue (Param has no indexed lvalue)
rhs           := source_sel | call | bare_chip | scalar_expr | vector_expr   # NOT just expr: poly = sin, poly = sin(poly), poly = cf, poly = <expr>
bare_chip     := name                                        # zero-arg chip as RHS, e.g. poly = sin
source_sel    := "cf" | "poly" | "pop" | "peek"            # bare source on the RHS
stmt_chip     := name [ "(" expr ("," expr)* ")" ]          # statement-level chips: emit, drop, push_vec, push_scalar, arange, …
indexed_read  := index_base "[" expr "]"                    # Coeff: cf[i] / poly[i] / tos[i] inside expr
index_base    := <symbol: expr_index_base context>          # Coeff: cf, poly, tos
selector_call := name "(" sel_slot ("," sel_slot)* ("," expr)* ")"  # Coeff: src/tgt selector slots, then expr args
sel_slot      := <profile selector token for the slot>      # Coeff src: cf,poly,pop,peek ; tgt: poly,push
# poly_len + scalar refs t1,t2,p1,p2 are profile symbols resolved in expr (Coeff: all read-only)
# LAYOUT CONTRACT: legacy/native calls source-first [name,src,tgt,…]; named chips target-first [tgt,src,…] (coeff_program_chain.py:1570,1636; named comment js/08-chip-editors.js:998)
```
**Selector args ≠ expression args** (resolves the `args:=expr` bug): `legacy(name, src, tgt, …)` takes an identifier `name` + two `sel` selectors, *then* expr args — `expr` applies only to the trailing args, never to the name/selectors. **One canonical form per construct:** emit is assignment to a profile-writable output symbol (`p1 = expr`, `p2 = expr` in Param), not `emit p1` / `emit(p1)`; `emit_p1`/`emit_p2` are the explicit zero-arg stack-pop ops. **`unary_name(reg)` is documented Param-profile sugar** (resolves the `square(p1)` ambiguity): it applies the op to the *matching grid coordinate* — `square(p1) ≡ p1 = square(t1)`, `square(p2) ≡ p2 = square(t2)` — **not** "square the current p1". The source-from-chain renderer emits exactly this form for targetable-unary chips.
**Canonical source-from-chain is first-class (the populate path) — and needs a NEW serializer.** `display_param_program_chain` (`param_program_chain.py:492`) is a *display formatter* (`"; ".join` of `name(args)` chips) — it emits non-canonical forms like `emit(p1)`, which the grammar above rejects (canonical emit is `p1 = expr`). So **add a real `param_source_text_from_chain(chain)`** that renders the *canonical* surface (`p1 = expr`, `push(t1)`, `square(p1)`, `legacy(...)`) — never `emit(p1)`, never internal synth chips (`push;_typed_unary;emit`) — mirroring coeff's source serializer. The populate path and the round-trip gate use **this** serializer. Pin **compiled equivalence**, not raw chain equality (aliases/canonicalization can legitimately reshape surface rows): `compile_param_program_chain(parse_param_program_source(param_source_text_from_chain(c))).execution_spec == compile_param_program_chain(c).execution_spec` **and equal `fingerprint`**, over a corpus including legacy-bridge and expression chips. `display_param_program_chain` stays the chip *display* formatter (Phase 5 readonly view). The Legacy-tab/populate flow depends on the new serializer.
**Error recovery:** `strict=True` raises on first error (save/preview); `strict=False` skips the failing statement, records `{line,column,message}` in `diagnostics`, continues (editor live-compile) — exactly coeff's `parse_coeff_program_source(strict=)` (`:855`).
**Examples:**
```
p1 = t1*t2 + p1                          # push expr; emit_p1
p2 = exp(t1)                             # push expr; emit_p2
square(p1)                               # sugar: p1 = square(t1) — input is the matching grid coord, NOT current p1
legacy(asp, p1, push1, 0, 0.1)           # bridge: name + 2 selectors (p1, push1) + 2 expr args (asp takes a,b)
push(t1); dup; mul; emit_p1              # explicit stack form
```

### 3.2 Root transforms — stays chip/array (optional read-only export, no source parser)
Root is a flat `[name,...params]` array (`root_xforms.h:43`, `js/07-transform-catalogs.js:993`) with 9 fixed transforms; chips→display works without a parser since the wire is already an array. **Recommendation:** a trivial one-transform-per-line text form (`rotate_roots(0.5)`) is ~30 lines if wanted, but **no compute-path source module is needed** — don't add a root `*_source.py` to compute bundles.

### 3.3 Storage round-trip + route
- `_compile_param_program_payload` (`handler_storage.py:388`) gains a `source_text` arg + `param_source_text_from_payload`, mirroring coeff's injection at `_compile_coeff_program_payload:454` (where `source_text` overrides `chain`).
- Add `/compile-param-program-source` route (mirror `/compile-coeff-program-source` at `handler_storage.py:884`) + handler + frontend `lambdaPost` (`js/08-chip-editors.js:438` is the coeff precedent) — synchronized across the 3 manifest sources (see Phase 4 / 4.5).

### 3.3a Compute-path source plumbing — mirror coeff (blocker)
A `/compile-param-program-source` route is **not sufficient**: `coeff_program_source_text` threads through the *whole compute path*, and param must mirror every site (today `param_program_source_text` exists **nowhere** in `lambda/`, `js/`, or `stepfunctions/`). The coeff precedent — 7 literal carriers + 2 helper consumers:
- **JS payloads:** `js/08-chip-editors.js`, `js/02-preview-solvescore.js` — add `param_program_source_text` to the dispatched payloads.
- **`pipeline_programs.py`:** add `param_source_text_for_run` + `parse_param_source_for_run` (mirror `coeff_source_text_for_run`/`parse_coeff_source_for_run`); add `param_program_source_text` to the `pipeline_mode_from_params` program-inference set (`:39`).
- **`handler_compute_orchestrator.py`, `handler_compute_plan.py`, `handler_compute_preview.py`:** thread `param_program_source_text` exactly as coeff.
- **`handler_coeffgen.py`:** add `_resolve_param_program` using `param_source_text_for_run` (mirror `_resolve_coeff_program:58`, which calls `coeff_source_text_for_run` at `:62` — the degree-probe path consumes source text via the helper, which is *why* coeffgen is in scope despite having no literal `coeff_program_source_text`).
- **`handler_param_debug.py`:** consume param source text (today it reads only `param_program_chain`, `:136`).
- **ASL:** add the `param_program_source_text` forward in `compute_workflow.asl.json.template` (coeff is already forwarded there).
**Gate:** a contract test asserting `param_program_source_text` reaches each carrier (mirror the coeff source-text tests); packaging (§3.4); api_manifest for the new route.

### 3.4 Packaging fan-out (CR18 §9.4 — per parser, different bundles)
`param_program_source.py` and `program_source_core.py` are compute-path → the same 5 bundles as `coeff_program_source.py`: coeffgen, storage, compute_preview, param_debug, compute_plan (`deploy.sh:964,985,1034,1058,1328`). `program_profiles.py/json` must ride with every bundle that imports either source parser. Once Coeff is migrated to `program_source_core.py`, the existing `coeff_program_source.py` entrypoints remain the public module names but delegate to the core, so downstream imports and payload contracts do not churn. A missing `cp` is a runtime ImportError caught by `test_deploy_packaging.py`'s AST walk (static imports only). **No root source module** (3.2). **Gates:** `test_param_program_*` (native + storage + source round-trip), shared parser-core unit tests, profile-symbol diagnostics tests, the AST packaging gate, api_manifest regen for the new route, and a Coeff shared-core equivalence suite as a **Phase 4 release gate** — must pass before v2 ships, not before Phase 4 starts (`source → old coeff parser → chain/fingerprint` equals `source → shared core coeff profile → chain/fingerprint` over saved programs + hand-written corpus; full breakdown §3.6). **Risk:** low-medium for the Param increment; medium for the mandatory Coeff migration because it touches a live, fingerprint-critical parser.

### 3.5 Param text-editor UI file map (concrete)
Mirror coeff's text-editor wiring (Phase 5 §5.2/5.3 lists the coeff/readonly sites):
- **Text editor + source/chip tab:** `js/08-chip-editors.js` (the coeff `lambdaPost` precedent `:438`, picker/editor engine `:120-186`) + the chip-shell tab arms in `js/09-render-orchestration.js` (`_paramProgramChipShellHtml` `:172`, readonly renderers `:300,337`).
- **Populate (source-from-chain) button:** `js/02-preview-solvescore.js` + `js/07-transform-catalogs.js` (the populate/`addChip` path) — must round-trip through the new `param_source_text_from_chain` (§3.1 compiled-equivalence gate; **not** the `display_param_program_chain` display formatter, which emits non-canonical `emit(p1)`), never emit synth chips.
- **Storage modal (save/fetch/list):** `js/03-program-modals.js` (the per-kind call sites) gains the param source-text field + the `/compile-param-program-source` call.
**Gate:** Playwright for the param text editor + populate; the frontend split-load / parts-contract gate (`tests/test_frontend_js.sh`).

### 3.6 Coeff migration to the shared core (mandatory; Phase 4 release gate)
Coeff already authors text today (`coeff_program_source.py`), so this delivers **no new user-facing capability** — its value is internal: one parser/grammar/diagnostic engine instead of two dialects that can drift. Because it touches a **live, fingerprint-critical** parser, it is its own tracked deliverable, scheduled in the Phase-3→4 window and gated at **v2 release** (not Phase-4 start).

**Implementation status (2026-06-20): production flip complete; oracle retained.** `coeff_program_source.py` now delegates statement splitting, top-level call parsing, top-level assignment detection, source-span records, diagnostic-code formatting, selector/symbol vocabulary lookup, the top-level statement-dispatch grammar (`assignment` vs `call` vs `bare`), and the parse/lower/diagnostic orchestration to `program_source_core.py` + `program_profiles.json`. The production Coeff path is a profile-backed `ProfileStatementLowerer` hook class; `coeff_program_source_legacy.py` keeps the frozen old Coeff parser/statement lowerer as a test-only oracle, never as a production import. Strict-mode Coeff source failures raise a `RuntimeError` subclass carrying structured `{line,column,level,code}` diagnostics. `tests/fixtures/coeff-program-corpus/` plus `test_coeff_source_equivalence.py` compare legacy-shell/legacy-statement-lowering vs shared-core/profile-hook output over a checked-in corpus, including compiled `execution_spec` + fingerprint equality. The canonical source serializer decompiles source-generated internal `_typed_*` chains (including dynamic indexing, `tos[...]`, typed set/poke/fill, and native stack-arg transforms such as `exp_affine`) back to editable text and the oracle requires regenerated source to preserve `execution_spec` + fingerprint. Coeff chip/value-specific lowering remains in the Coeff profile hook module by design; the parser/diagnostic/orchestration engine is shared. The old oracle is deleted only after the v2 release gate passes.

- **What moves into `program_source_core.py`:** the lexer, statement splitter, expression parser, source-span diagnostics, assignment/call parsing, and profile-symbol resolution that coeff and param share (today these live in `coeff_program_source.py`). Coeff-specific surface — indexed lvalues/rvalues (`poly[i]`, `cf[i]`, `tos[i]`), `src/tgt` selector slots, vector/scalar values, source-first vs target-first call layouts, `poly_len`/`tosN` — is expressed through the **Coeff profile + profile hooks**, not a coeff-only parser.
- **`coeff_program_source.py` stays the public shim:** its four entrypoints (`parse_coeff_program_source`, `compile_coeff_program_source`, `coeff_source_text_from_payload`, `split_coeff_program_statements` — `:855,889,837,748`) keep their names/signatures and **delegate** to `program_source_core.py` with `profile="coeff"`, so no downstream import or payload contract churns (Lambda bundles, `pipeline_programs.py`, the compute handlers).
- **Two steps — flip, then delete (the equivalence gate needs *both* parsers live):** Step 1 flips production to the shared-core coeff profile but **keeps the old coeff parser as a frozen test-only adapter** — `lambda/coeff_program_source_legacy.py`, a frozen copy imported **only** by the equivalence test, never by production code — so CI can keep comparing old-vs-shared-core (and the refactor can't accidentally delete the oracle). Step 2 — a **follow-up cleanup** — deletes the old parser **only after** equivalence is green and the v2 release gate has passed. Production runs exactly one parser (shared-core) throughout, so there's no production divergence; the old parser survives solely as the test oracle until cleanup. (Deleting it in the flip change would leave the equivalence gate nothing to compare against.)
- **Corpus:** every saved coeff program from a **checked-in fixture snapshot** of the existing **unversioned** store (`polypaint/coeff-programs/<id>.json` — `COEFF_PROGRAMS_PREFIX`, `handler_storage.py:75`; *not* a `v1/` prefix, with v2 to live under `polypaint/coeff-programs/v2/<id>.json`) **plus** a hand-written landmine corpus covering the §3.1 hard cases — macro expansion, typed-stack expressions, every selector slot, indexed reads/writes, `poly_len`, `tosN`, both call layouts, and the strict/non-strict error-recovery paths. CI must never depend on live S3: add an export script (e.g. `scripts/export_coeff_program_corpus.py`) that snapshots S3 programs into `tests/fixtures/coeff-program-corpus/saved/<id>.json`, and an index `tests/fixtures/coeff-program-corpus/corpus.json`; tests read only those checked-in fixtures.
- **Equivalence asserted per corpus item:** old coeff parser vs shared-core coeff profile must agree on **every field of the compile output** — `chain` (parsed), `source_chain`, `expanded_chain` (macro-expanded), `tokens`, `scalar_exprs`, `execution_spec`, `legacy_coeff_transforms`, and the `fingerprint` (the `compile_coeff_program_chain` keys, `coeff_program_chain.py:2092-2106`) — plus diagnostics and canonical source regeneration:
  - **Diagnostics** compared on `{line, column, level, code}`, **never raw message text** (brittle). This is now implemented for the parser-shell equivalence gate: non-strict diagnostics include a stable `code`, and strict source failures raise `CoeffProgramSourceCompileError` carrying the structured diagnostics list instead of collapsing to an unstructured `RuntimeError`.
  - **Canonical source regeneration** is implemented by backend **`coeff_source_text_from_chain(chain)`** — canonical, re-parseable, mirroring param's `param_source_text_from_chain` (§3.1) — distinct from `display_coeff_program_chain` (`coeff_program_chain.py:877`, human-readable) and the UI-local `_coeffProgramSourceFromRows` (`js/08-chip-editors.js:310`, display-only). The round-trip `parse(coeff_source_text_from_chain(c))` must yield an equal `execution_spec`/`fingerprint`; the oracle now enforces this over the checked-in corpus, including source-generated internal `_typed_*` chains.
- **Phase 4 release gate:** this suite must be green before v2 ships. Phase-4 groundwork (v2 IR, `translate_from_old`, dual-read) proceeds in parallel — it operates on compiled chains/`execution_spec`, not the parser — but v2 is not "complete" while two coeff parser dialects are live.

---

## Phase 4 — Unified token IR + v2 fingerprint + `translate_from_old` + Legacy tab

**Objective.** The fingerprint version bump: merged opcode IR, per-kind old→new translation, v2 fingerprint scheme + dual-read, migration routes, the Legacy tab, the opcode-renumber drain. **Highest risk.** Prerequisites (to *start*): DD2 gate (Phase 0) live; DD5 oracle exists. **Release gate (before v2 *ships*, not before Phase 4 starts):** the §3.6 Coeff shared-core equivalence suite must pass — v2 cannot ship with two coeff parser dialects live.

**Implementation status (2026-06-20):** Phase 4 has started with the storage/native-wire groundwork. Landed: `program_v2_translate.py`, v2 fingerprints (`sha256:` over `{program_kind,spec_version,payload}`), per-kind migration routes `/migrate-{coeff,param,solve-score}-program`, dry-run previews, non-overwriting writes under `polypaint/{kind}-programs/v2/<id>.json`, explicit `409` conflict and `422` missing-macro responses, backend `coeff_source_text_from_chain(chain)`, deploy/API manifest sync, migration-route tests, generated `merged_opcodes.{py,h}` plus JS mirror, and native v2 decode for the currently executable Param/Coeff subset with `native_transform.registry` validation. This slice **does not** switch live compute paths to write v2 payloads by default, does not implement solve-score/root v2 execution, and does not add the Legacy tab UI. Existing v1 compute/read paths remain unchanged; the §3.6 Coeff shared-core release gate has landed and remains a v2-ship gate.

### 4.1 Merged opcode IR (v2) — resolve C1/C2
One merged opcode enum across param+coeff (C1: today op 8 = param POP vs coeff BLEND; op 10 = param ADD vs coeff POKE_POLY). `native_transform` gains a **registry-namespace discriminator** (C2: `ct_*` 1–28 vs `pt_*` 1–48 collide) **plus per-registry capability metadata** — `andy` and `length_policy` are coeff-only (CR18 §7.6); the param/root registries declare them absent, and the unified op validates capabilities per registry (no `andy` on a root transform; no length change where the profile's fixed-output contract forbids it, §2.1). Bump `program.version` → 2. The native parser (version-gated since Phase 0) keeps a v1 legacy-decode path through the drain (4.6) and adds the v2 decode. The merged enum is the wire only at this phase (Phase 2 kept it internal).

**Merged opcode-ID allocation — full exact table.** Single source of truth: a generated `lambda/merged_opcodes.{h,py}` + a JS mirror, checked by the Phase-1 drift gate. Ranges: **1–30** coeff (reference), **31–47** param-specific, **48–63** reserved, **64–95** solve-score.

*Coeff (1–30, unchanged — `sweep_cli.c:3356-3385`):* the 30 `COEFF_OP_*` keep their ids; `native_transform`=29 gains the `registry` discriminator. Reference range, not re-listed.

*Param-specific (31–47) — ops with no **safe** coeff fold (verified against the coeff sub-op tables):*
| id | param op | id | param op |
|---|---|---|---|
| 31 | `push_t1` — param input | 37 | `ratio` — zero→0 divide |
| 32 | `push_t2` — param input | 38 | `divide` — **errors** on zero denom (`sweep_cli.c:6649`); coeff divide is forgiving (zero→0, `:4276`) — cannot fold |
| 33 | `push_p1` — legacy-bridge read | 39 | `reciprocal` — absent from coeff `VECTOR_UNARY_OPS` |
| 34 | `push_p2` — legacy-bridge read | 40 | `unit_circle` — absent from coeff `VECTOR_UNARY_OPS` |
| 35 | `emit_p1` — output projection | 41 | `square` — absent from coeff `VECTOR_UNARY_OPS` |
| 36 | `emit_p2` — output projection | 42 | `cube` — absent from coeff `VECTOR_UNARY_OPS` |

(`43–47` reserved.) Param ops that **safely fold** (verified equivalent; each parity-gated in Phase 2A before it lands): `const→typed_push_scalar(21)` — **not op 1**, which is `COEFF_OP_CONST`, the *vector* push-const with a length arg (`sweep_cli.c:4803`); `duplicate→4`, `swap→5`, `pop→6`, `flush→7`; `add→typed_binary(add)`, `subtract→typed_binary(subtract)`, `mul→typed_binary(multiply)` (the coeff `VECTOR_BINARY_OPS` ids, `coeff_program_chain.py:218`); `negate/conj/exp/real/imag/abs→typed_unary(neg/conj/exp/real/imag/abs)` (the coeff `VECTOR_UNARY_OPS` ids, `:226`); `legacy→native_transform(29, registry=param)`. **Anything not in this fold list stays param-specific** (the table above) — the four missing unaries + `divide` have no semantics-preserving coeff target.

*Solve-score (64–95) — own range (separate score-VM register file + metric/lag/emit context), enum order with `reduce_metric` prepended:*
| id | op | id | op |
|---|---|---|---|
| 64 | `reduce_metric` *(new — prepass-as-opcode)* | 79 | `dup` (`DUP=15`) |
| 65 | `push_metric` (`PUSH_METRIC=1`) | 80 | `add` (`ADD=16`) |
| 66 | `avg` (`AVG=2`) | 81 | `mult` (`MULT=17`) |
| 67 | `min` (`MIN=3`) | 82 | `subtract` (`SUBTRACT=18`) |
| 68 | `max` (`MAX=4`) | 83 | `ratio` (`RATIO=19`) |
| 69 | `mul` (`MUL=5`) | 84 | `clamp` (`CLAMP=20`) |
| 70 | `weighted_sum` (`WEIGHTED_SUM=6`) | 85 | `ema` (`EMA=21`) |
| 71 | `abs_diff` (`ABS_DIFF=7`) | 86 | `sin` (`SIN=22`) |
| 72 | `geometric_mean` (`GEOMETRIC_MEAN=8`) | 87 | `cos` (`COS=23`) |
| 73 | `omega_cosine` (`OMEGA_COSINE=9`) | 88 | `log` (`LOG=24`) |
| 74 | `sawtooth` (`SAWTOOTH=10`) | 89 | `exp` (`EXP=25`) |
| 75 | `flip` (`FLIP=11`) | 90 | `pow` (`POW=26`) |
| 76 | `emit` (`EMIT=12`) | 91 | `emit_none` (`EMIT_NONE=27`) |
| 77 | `emit_norm` (`EMIT_NORM=13`) | 92 | `flush` (`FLUSH=28`) |
| 78 | `const` (`CONST=14`) | 93–95 | reserved |

(Exact integers are the generated header's to lock; this table is the **content** contract the drift test enforces across Python/C/JS — `solve_score.h:927-954`. **Decided: solve-score ops keep their own ids in 64–95 — no aliasing to coeff ids.** The score VM is a separate register file + metric/lag/emit context, so a distinct, self-contained range is the single unambiguous mapping the generated header enforces.)

### 4.1a Solve-score v2 wire format (the CLI contract — was unspecified)
Today every solve-score program reaches native as a **semicolon string** in `--score_program=<spec>`, parsed by `parse_solve_score_program_spec` (`solve_score.h:1160`) in **5 binaries**: `roots2pix_mt.c:657`, `solve_palette_chunk.c:86`, `solve_palette_chunk_mt.c:796`, `solve_proximity_stats.c:984`, `solve_proximity_hist_sectioned.c:742`. **Decision (DD1-consistent): v2 keeps the same `--score_program=<string>` wire** — *not* a new numeric-token arg, JSON arg, or dual-format field. The string carries an explicit version (a leading `v2;` sentinel, mirroring how `program.version` rides the coeff/param payload); `parse_solve_score_program_spec` gains a version branch that **lowers the v2 spec to the merged numeric opcodes (64–95) internally** — exactly as coeff/param keep the compact wire and lower natively (DD1). So **all 5 binaries are unchanged at the arg surface** (still read `--score_program`); only the parser's internal lowering + the version check change. Missing sentinel ⇒ v1 (back-compat). Uniform across the three DSLs: compact text in, native lowering to merged opcodes, no new transport. **The version marker is handled by ONE shared helper, not ad-hoc:** add `strip_solve_score_version(spec) → (version, body)` used by *both* the native parser *and* every Python consumer that tokenizes the string today — `canonicalize_solve_score_program_spec` (`solve_score_chain.py:540`), `solve_score_program_spec_uses_lag` (`:554`), `color_recolor_raw.py:142`, `handler_extract_palette_from_step_scores.py:153` — so no consumer ever `split(";")`s a raw versioned string (else the leading `v2;` surfaces as a bogus token in all four). **Metric definitions stay out-of-band:** the 5 binaries still receive metric defs via the separate `--score_metrics`/`--score_sources`/`--score_clip_los`/`--score_clip_his` args; the program references metric **slots** by index (`m{slot}-{lag}`, `solve_score_chain.py:537`). v2 does **not** embed metric defs in `score_program` — `reduce_metric`'s `metric/source/quantile` are compiler-IR annotations only, dropped at the wire (which carries the slot index).

### 4.2 `translate_from_old(kind, payload) → unified_chain`
- **coeff** → near-identity at the *chip* level, **but NOT at the wire level**: v2 inserts the registry-namespace discriminator (`registry='coeff'`) into native/legacy tokens that lack it, renumbers to the merged opcode enum (C1), then re-fingerprints under v2. The chips are unchanged; the token bytes are not.
- **param** → dedicated arithmetic opcodes → typed-op equivalents; `pt_*` legacy → `native_transform[param_registry, fn_index]` (preserve fn_index, C2); `emit_p1/p2` → output projection.
- **root** → a sequence of `native_transform[root_registry, fn_index]` — but **there is no root registry today**: `apply_root_xforms` dispatches by `strcmp(e->name, …)` (`root_xforms.h:242`) and no `root_legacy_registry.json` exists (only coeff/param do). **Phase 4 must add `root_legacy_registry.json`** assigning a stable `fn_index` to each of the 9 root transforms (`rotate_roots`/`pull_unit_circle`/`roots_toline`/`line_to_unit_circle`/`invert_roots`/`add_complex`/`mul_complex`/…), pinned by a `test_registry_fn_indices_are_pinned`-style test (the coeff/param precedent), and give `apply_root_xforms` a numeric-dispatch path (or resolve name→`fn_index` via the registry at translate time). Root also needs its **own value model + execution context** (CR18 §7.0): root transforms run **per-sample at raster time over a `float` root array** (`root_xforms.h:242`), *not* over the coeff VM's `double` complex-vector registers. The root profile must declare: input/output register = the `roots` float vector (in place); a **precision policy** (float vs the VM's double — pin which, since it's observable); the raster-stage invocation context; length-preserving; and the NaN-poison convention (`rt_mark_undefined`). **Unknown names → traced no-op** (CR18 §4.3: drop + diagnostic, never hard-fail or silently vanish — preserves today's `:278` silent-ignore *with a trace*).
- **solve-score** → `reduce_metric` + scalar ops + `emit` — but the hard parts must be specified, not hidden: **(a) metric-slot CSE** (dedup `(metric,source,quantile)` into shared slots, the `program_spec` slot model); **(b) lag references** (`mN-1` reads the prior step — the Phase-2B facility); **(c) implicit scalar output vs explicit `emit`s** (single-score path vs N channels); **(d) per-channel normalization** (`emit_norm`); **(e) the multi-channel output contract** (channel count ↔ `color_interpretation`, CR18 §3.2). Preserve omega/phase exactly (already in `program_spec`). This is why solve-score stays chip-primary — translation is its *only* migration target and the trickiest.
Injection point: the payload compilers already override `chain` from source (`_compile_coeff_program_payload:454`) — `translate_from_old` plugs in there.

**v2 token shapes (so Phase 4 is codeable, not prose).** These are the **compiler IR** (`translate_from_old` output) — op names are strings for readability; the **native wire lowers every op to a stable numeric id in the merged opcode enum** (`native_transform` → 29; `reduce_metric`/`push_metric`/`emit*` get numeric opcodes too). One canonical representation: **IR = string op-names at the compiler layer; wire = numeric opcodes**. The IR token extends today's coeff token (`op, fn_index, src, tgt, n_args, args[], args_im[], expr_refs[], andy`) with a `registry` discriminator on `native_transform`, and adds the solve-score ops; the `program_spec` fields (`op, metricSlot, lagDepth, a, b`, `solve_score.h:982`) carry over:
```jsonc
// root transform — registry-namespaced native_transform (IR op-name; wire: numeric 29):
{ "op": "native_transform", "registry": "root", "fn_index": 8, "n_args": 4, "args": [1,0,0,1], "args_im": [0,0,0,0] }
// coeff/param legacy stay native_transform with registry "coeff"/"param":
{ "op": "native_transform", "registry": "coeff", "fn_index": 27, "src": 1, "tgt": 1, "andy": 0.0 }
// solve-score metric reduction (slot-CSE'd: metric+source+quantile → normalized slot):
{ "op": "reduce_metric", "metric": "proximity", "source": "slv", "quantile": 0.001, "slot": 0 }  // metric/source/quantile = IR-only (CSE/display); WIRE carries only slot — defs stay in --score_metrics/... args (§4.1a)
{ "op": "push_metric", "slot": 0, "lag": 1 }      // lag read of slot 0 at the PRIOR step (mN-1)
{ "op": "weighted_sum", "a": 0.5, "b": 0.5 }      { "op": "omega_cosine", "a": 1.0, "b": 0.0 }
{ "op": "emit" }          // explicit channel, raw
{ "op": "emit_norm" }     // explicit channel, range-normalized
{ "op": "emit_none" }     // discard a stack value, still explicit-output mode
// implicit scalar output  = NO emit op + terminal stack depth 1 (the single score)
// multi-channel output     = N emit/emit_norm ops, terminal depth 0, channel_count = #emits (≤8) ↔ color_interpretation (CR18 §3.2)
```
v2 only *adds* `registry` to `native_transform` and renumbers the merged opcode enum; the reduce/lag/emit shapes mirror the current `program_spec` tokens. The implicit-vs-explicit and channel-count↔`color_interpretation` contracts are unchanged.

### 4.3 v2 fingerprint scheme + dual-read
Bump the family `spec_version` → 2 at the WRITE sites (Phase 0.2); the READ sites (Phase 0.3) already dual-read. Add the v2 canonical renderer in `compiled_solve_score_fingerprint` / the per-VM `execution_spec`. Old artifacts stay v1; new work emits v2.

### 4.4 Migration routes + macro DAG (DD3)
- New routes in `handler_storage.py` dispatch (`:857-931`, the `path.endswith` chain): **`/migrate-{coeff,param,solve-score}-program` — per-kind (decided)**, matching the existing per-kind storage-route convention (save/fetch/list/delete are all per-kind; the Legacy tab lists per-kind, so per-kind migrate is the least-surprising shape for api_manifest/JS/tests). No `/migrate-program` umbrella; no root migrate route (root isn't a saved-program kind — §4.5). Version-aware fetch (compile mode): require v2 deps (§macro rule below); the `v2/`-else-v1 fallback is list/display/discovery only. **Non-overwriting save** (today save-by-slug overwrites, `handler_storage.py:1199`,`1114`,`1028`): v2 writes to the `v2/` namespace, v1 stays immutable.
- **Macros are coeff/param-only** — resolvers `_param_program_macro_resolver`/`_coeff_program_macro_resolver` (`handler_storage.py:343,378`); **solve-score has no macro mechanism** (none in `solve_score_chain.py`/`solve_score.h`), so its migration has no macro-DAG step and never returns the 422 below. For coeff/param: **Macro refs stay by logical `<id>`** (not `<id>@v2`), resolved by a **version-aware fetch with two modes** (this resolves the apparent contradiction): for **v2 compile**, the resolver **requires v2 dependencies** — a v2 program whose macro is still v1-only is an *error* (fail the compile), which forces bottom-up migration; the **v1-fallback** ("prefer `v2/` else v1") is **only** for list/display/migration-discovery, *never* for v2 compile. Migrate the DAG **bottom-up** (leaves first), reusing the existing cycle/depth guard (`handler_storage.py:343,378`).
- **Route 3-source sync** (`api_manifest.py`): a new route must be added to (a) the `handler_storage.py` dispatch (auto-detected by `_extract_storage_routes`), (b) `deploy_manifest.json` `polypaint-storage` `routes[]` (`:97-132`), (c) the frontend `lambdaPost('storage',…,'/X')` call (`js/03-program-modals.js`), then `api_manifest.py --write`; `--check` fails CI otherwise. New migrate/translate handlers also need payload-contract tests (api_manifest pins routes, not payload shapes).
- **Route contracts (request/response — pin before JS/tests):** each `/migrate-{kind}-program` (POST via the `storage` dispatcher):
```jsonc
// request
{ "id": "<program id>", "from_version": 1, "dry_run": true }
// response (dry_run=true → preview only, no write):
{ "id": "...", "kind": "coeff|param|solve-score",
  // migrated is PER-KIND: coeff/param carry source_text; solve-score has NO source text (chip-primary) — it returns program_spec instead:
  "migrated": {  // coeff/param: { "source_text": "...", "chain": [...], "fingerprint": "sha1:...", "spec_version": 2, "program_version": 2 }
                 // solve-score: { "program_spec": "...", "chain": [...], "fingerprint": "sha256:...", "spec_version": 2 }  (no source_text)
  },
  "v1": { "fingerprint": "...", "spec_version": 1 },   // for the Legacy-tab diff
  "wrote": false }                                      // true when dry_run=false (writes v2/<id>.json)
// conflict:      v2/<id>.json already exists with a different fingerprint → 409 { "error": "v2 exists", "existing_fingerprint": "..." }
// macro-missing: a referenced macro has no v2 yet → 422 { "error": "macro not migrated", "missing": ["<id>", ...] }  (coeff/param only — solve-score has no macros; bottom-up, §macro rule)
```
`dry_run=true` powers the Legacy-tab diff; `dry_run=false` does the non-overwriting `v2/` save. Add this shape to the payload-contract tests.

### 4.5 Legacy tab UI (per-kind — CR18 §4.4)
A tab that loads an old saved program and runs `translate_from_old`, **per-kind for the saved-program kinds only**: **coeff/param → migrated text form** (Phase 3 param parser; coeff already has one); **solve-score → chain + `program_spec` preview** (no text source). **Root is not a saved-program kind and is not listed in the tab** — its `root_transforms` are embedded in render/palette artifacts and migrate **in place** (next sentence); `translate_from_old` still handles the root case (§4.2), but only for that embedded path, not a tab entry. Save the v2 copy under the versioned id. **Decided: root is NOT promoted to a saved-program kind** — its `root_transforms` live embedded in render/palette artifacts and migrate **in place** under the v2 palette identity (no new `polypaint/root-programs/` prefix, no root migrate route, no root entry in the Legacy tab's program list). So the Legacy tab lists/migrates only coeff/param/solve-score. UI plugs into `js/03-program-modals.js` (the per-kind fetch/list/save/delete call sites).

### 4.6 Opcode-renumber drain (DD2)
Step Functions forwards compiled `coeff_program`/`param_program` straight into native (`compute_workflow.asl.json.template:211,278`). Deploy so no in-flight execution straddles the renumbering (drain window, or translate at the Lambda boundary). The `scalar_exprs` form is *not* part of this drain (permanent wire, DD1) — only the opcode numbering changes.

**Gates:** migration round-trip tests (`translate_from_old(old)` computes/hashes correctly under v2); the **DD5 whole-sweep byte oracle** comparing old-VM v1 vs new-VM-translated v2 output bit-for-bit; dual-read tests across all families; macro-DAG bottom-up tests; the 3-source route sync + new payload-contract tests. **Risk:** highest — this is the migration proper; the palette path is mostly cache-miss (CR18 §4.1 #2), but the solve-score validators (both families) and `probe_signature` are fail-closed if dual-read is incomplete.

---

## Phase 5 — Chips → display-only (coeff/param)

**Objective.** Make text the editable source for coeff/param; chips become read-only visualization. Retire the editable JS + the pre-VM by-name dispatch. **Per-DSL staging:** coeff/param → display-only; **root stays chip-editable** (no text parser shipped, 3.2); **solve-score stays chip-primary**.

**Implementation status (2026-06-20): boundary-translation slice landed.** Compute plan, compute preview, and coeffgen now translate nonempty `param_transforms` / `coeff_transforms` into compiled Param/Coeff Program payloads before building native CLI specs, including the old Program-mode `legacy_transforms` / `legacy_coeff_transforms` fast-path cases. The native specs carry inert empty transform arrays plus `param_program` / `coeff_program`; the original transform rows are preserved only as `*_transforms_display` metadata for logs, preview-context messages, and UI display. This uses the current executable program payloads (v1 wire today; v2 once Phase 4 execution is live). Native `parsePtChain` / `parseCtChain`, editable chip UI, ASL transform-field forwarding, and direct native-chain regression tests remain in place until the next Phase 5 slices delete them behind explicit gates.

### 5.1 Retire (cp/pp editable machinery)
- `_chipPickers` `pp`/`cp` entries + picker engine: `js/08-chip-editors.js:120-186` (and the pp/cp wrappers `:188-253`).
- Mutators' cp/pp branches: `addChip` (`js/07-transform-catalogs.js:1335`), `removeChip` (`:1368`), `moveChip` (`:1387`), `updateChipParam` (`js/08-chip-editors.js:1279`).
- Editable shell arms: `_paramProgramChipShellHtml` editable arm (`js/09-render-orchestration.js:172-177`), `_coeffProgramChipShellHtml` editable arm (`:184-189`), and the `<select>/<input>` emitters with `updateChipParam` onchange (`js/08-chip-editors.js:1326,1339,1350`).
- **Retiring the pre-VM by-name dispatch requires removing or translating Chain mode first.** `lookupCoeffTransform` (`sweep_cli.c:3246`), `parseCtChain` (`:5641`), `parsePtChain` (`:5576`) are the **live Chain-mode** parsers — `pipeline_mode_from_params` (`pipeline_programs.py:35`) still returns `chain` vs `program`, and old payloads can still contain `coeff_transforms`/`param_transforms`. So Phase 5 cannot just delete them. **Decided: translate Chain-mode requests to program payloads at the Lambda boundary** (keep the UI's Chain/Program mode *during* migration), then **delete native `parseCtChain`/`parsePtChain` only after a test proves nonempty Chain arrays no longer reach native**. Removing Chain mode from the UI is a later, separate step. This is a gating prerequisite, not a cleanup. **Boundary file map:** `pipeline_mode_from_params:35` is a pure **classifier** (returns `"chain"`/`"program"` only) — it does **not** transform. The shared translation helpers live in `pipeline_programs.py` (`param_transforms_to_program_chain`, `coeff_transforms_to_program_chain`), mirroring the existing `coeff_source_text_for_run`/`parse_coeff_source_for_run` helper shape. The three compute handlers that used to emit nonempty transform arrays to native — `handler_compute_plan.py`, `handler_compute_preview.py`, `handler_coeffgen.py` — now translate before the native payload is built. **Program mode reaches the same chain parsers too — not just Chain mode:** when a compiled program carries `legacy_transforms`/`legacy_coeff_transforms`, old handler code fell back to setting `param_transforms`/`coeff_transforms`; Phase 5 disables those fast-path fallbacks and always emits compiled program payloads. **Two layers — don't conflate them:** the ASL still forwards `param_transforms`/`coeff_transforms` through *state I/O* at 8 sites (`compute_workflow.asl.json.template:104,107,211,278,394,446,552,554`); the **gate is on the native CLI payload**, not the ASL state. During migration the arrays keep riding ASL state **inertly** (no ASL change in this slice — they're simply unused once the handler builds the native command from `coeff_program`/`param_program`). **Gating test:** assert no nonempty `coeff_transforms`/`param_transforms` in any dispatched *compute-handler native CLI payload*; the native-parser deletion is blocked on it passing. **Final ASL cleanup** — deleting the `.$` forwards — is a **later, separate step** tied to retiring Chain mode from the UI, and must land **simultaneously** with the handlers ceasing to emit those plan fields (a Step Functions `.$` reference to a now-missing path is a runtime error).

### 5.2 Keep (readonly renderers)
`_renderCoeffProgramChipHtml`/`_renderParamProgramChipHtml` with `{readonly:true}` (`js/09-render-orchestration.js:300,337` — already support it), `_chipReadonlyValueHtml` (`js/08-chip-editors.js:1295`). These become the display layer over parsed text; the readonly call sites already exist (`:388,:395`). Chips-as-display needs the parser to emit **per-chip source spans** (an editor parser, for click-to-locate / error underlining) — build alongside Phase 3's parsers.

### 5.3 Touch points
Dispatch maps to update when retiring cp/pp: `_chainForWhich` (`js/02-preview-solvescore.js:675`), `_catalogForChain` (`:662`), `_renderChips` (`js/09-render-orchestration.js:399`). `rt` editable path (`_renderRtChipHtml`, `_rtCatalog`) stays. **Gate:** the frontend split-load + parts-contract gate (`tests/test_frontend_js.sh`); Playwright for the new text editors. **Risk:** low once Phases 3–4 land.

---

## Cross-phase summary

| Phase | Ships standalone? | Hard gate before it | Primary risk |
|---|---|---|---|
| −1 verification harness | yes (oracle + benches) | — | none — it's the gate everything else leans on |
| 0 versioning | yes (pure insurance) | — | regression if missing≠v1 (mitigated) |
| 1 chip registry + symbol table | yes (kills drift + hardwired-name duplication) | — | semantics in the schema; param/profile drift gate |
| 2A shared runtime (VM merge) | yes (one interpreter) | DD5 oracle + DD6 benchmarks | FP determinism, perf |
| 2B lag facility (separate from 2A) | yes | 2A landed + same DD5 oracle | FP drift — kept bisectable by the split |
| 3 shared source parser (param first, coeff before v2) | yes for the Param increment; complete only after Coeff migration | — | low-medium for Param; medium for Coeff because its grammar is the superset and fingerprint-critical |
| 4 v2 migration | no (the bump) | **start:** DD2 gate (Phase 0) + DD5 · **ship:** + Coeff shared-core equivalence gate (§3.6) | the migration proper |
| 5 chips display-only | no | Phases 3+4 | low |

If appetite is limited: **0 + 1 + 3** deliver versioning insurance, drift elimination, and text-first Param/Coeff authoring (root/solve-score stay chips) without requiring the full v2 fingerprint migration or native merge — a coherent stopping point that leaves chips editable and fingerprints untouched. The remaining full shared-language work is the Phase-4/5 migration surface: v2 execution rollout, solve-score/root translation, the Legacy tab, and eventually making chips display-only.

*No production code was modified writing this plan; all current-state references verified against the `1a1996d` source snapshot.*
