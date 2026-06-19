# Code Review 18 — Unifying the Stack DSLs into One Token VM

**Request.** Implement D5 alternative 2 from `architecture-ui-critique.Rnw:315`: *"Unify the two VMs: one token VM with two register files (param scalars, coeff vectors),"* config-customized per use-case (param, coeff, root, solve-score); make text the single source and demote chips to a read-only display layer (D5 alternative 4, `architecture-ui-critique.Rnw:322`); and add a **legacy old→new translator** behind a "Legacy" tab with a migrate feature. This is a **design review only** — no code was changed. It maps the four DSLs exactly, classifies every divergence, specifies the unified VM, and details the migration.

Reviewed at HEAD `32930f4`. Five parallel read-only sweeps (coeff VM, param VM, solve-score, root transforms, JS editor + fingerprint blast-radius); every load-bearing claim re-verified against source by me, file:line throughout.

---

## Verdict

The unification is **conceptually right and mostly already built**: the coeff VM is a near-strict superset of the param VM and already has the typed scalar/vector stack, the named register files, the scalar-expression sub-VM, and the `native_transform` registry bridge the design needs. **The VM merge is the easy 30%.** The hard 70% is exactly what D5 predicted — *"fingerprints are wire format; this is a migration with versioned specs, not a refactor"* (`architecture-ui-critique.Rnw:316-318`). Specifically:

1. **Three true conflicts** force an old→new wire migration: colliding opcode numbers, disjoint legacy `fn_index` namespaces, and incompatible fingerprint canonicalization.
2. **One architecturally-new feature** the existing VMs lack: solve-score's **cross-step lag/windowing**, today hand-rolled in 4 C rasterizers.
3. **Three of the four DSLs have no text parser** (only coeff does), so "chips become display-only" requires *building* param / solve-score / root-transform text grammars first.
4. **The rollback/version surface is the most dangerous and is currently unguarded:** the native parser reads the token stream with **no `version` field** (`sweep_cli.c:3741,6267`), so a renumbered v2 stream would be *silently misexecuted* by rolled-back v1 — worse than any cache miss. A native version gate is the first hard prerequisite (DD2 / §7.4). (The palette collision-`raise` at `handler_palette_render_plan.py:297`, flagged in earlier drafts as the top trap, is mostly a *cache-miss* because the digest is in the S3 slug — see §7.0/§7.4. The fingerprint blast radius is still large — §4.1.)

Recommendation up front: **Phase 0 (add `spec_version` + dual-read) and Phase 1 (finish the registry single-source for program chips) are pure wins — do them regardless.** The full unification (Phases 2–5) is worth funding only because the "Legacy tab migration" is itself the *"real feature need"* D5 said to wait for. Sequenced below so you can stop after any phase with value banked. **Three gates are mandatory before any of Phases 2–5 (§7.10): (1) a native token-stream version gate, (2) a whole-sweep byte-exact old-VM-vs-new-VM oracle, (3) a token-budget corpus test.** The binding design answers to the questions the reviews raised are in **§"Design decisions"** below.

---

## Current state — what is implemented today (verified at HEAD `32930f4`)

A holistic ground-truth picture before the design. Part 1 has the DSL-level detail; this is the system.

**Execution model — two parity targets, not four.** Programs live in exactly two places: **Python** (`coeff_program_chain.py`, `param_program_chain.py`, `solve_score_chain.py`, `coeff_program_source.py`) compiles chips/text → tokens + fingerprint and does **no numeric execution**; **native C** runs them. There is **no WASM and no browser-JS executor** — the browser is a thin client (chip editors + metadata catalogs + `fetch`→Lambda). `solve_score_chain.py` has no evaluator, so there is no Python oracle for scores (DD5).

**The per-pixel pipeline is FOUR binaries, staged through `.bin` files** — "one VM" unifies the *code*, not the pipeline:
1. **param program** — `sweep_cli.c` param VM, per grid cell: grid `(x1,x2)` → complex `p1,p2` (output).
2. **coeff program** — `sweep_cli.c` coeff VM, per row: `cf` + `p1,p2` (input) → coefficient vector `poly`. Output length is **probed once at (0,0) and hard-asserted invariant** for the whole grid (`sweep_cli.c:7841,7948`).
3. **solver** — `sweep_cli.c` Ehrlich-Aberth, **warm-started** from the previous step in **serpentine** order (`:125,:7909`); writes roots `.bin`. The root field is therefore *path-dependent* across the sweep.
4. **root transforms + solve-score** — the raster binaries read roots `.bin`; root transforms run **per sample** as a sub-stage of the solve-score metric prepass on a `float` scratch copy (`root_xforms.h`, `roots2pix_mt.c:202`); solve-score reduces roots → 1 scalar or N channels (`solve_score.h`).

Register handoff carries **direction and type**: param *writes* `p1,p2` with **real-only** `t1,t2` in; coeff *reads* `p1,p2` with **complex** `t1,t2`. A pipeline handoff, not shared registers.

**Fingerprints & caches** — 8 families (§4.1): coeff/param program `sha1`, solve-score `sha256`, palette `palette_variant_fingerprint`, `probe_signature`, `plan_params_digest`, the execution-spec wire string, and the `fn_index` registry numbering. Persisted into `calc.json`, palette/clip/bins/hist meta, the raw sidecar, and DDB; compared with **mismatch-`raise`** at ~8 reuse sites. There is **no `spec_version`** (three independent `*_VERSION = 1` constants).

**Saved programs** — three S3 prefixes (`polypaint/coeff-programs/`, `polypaint/param-programs/`, `polypaint/solve-score-programs/`), **save-by-slug (overwrites in place)**; only coeff stores `source_text`; macros inline by value and the fingerprint hashes the *expanded* chain (§7.3). Root transforms are **not** a saved-program kind.

**Program-consuming binaries — 9 built** (8 distinct sources; `sweep`+`sweep_coeffgen` share `sweep_cli.c`) (§7.8): `sweep`, `sweep_coeffgen` (coeff+param VM), `roots2pix_mt`, `solve_palette_chunk_mt`, `solve_palette_chunk` (single-thread, legacy), `solve_proximity_hist_sectioned`, `solve_proximity_stats`, `bilevel_section_raster`, `solve_palette_debug`. ~16 handler construction sites, ~13 contract tests pin the current wire.

**UI** — chip-based editors for all four DSLs; only coeff has a text source. See **Part 8** for the full current/target UI.

---

## Design decisions — answers to the open questions

The reviews surfaced design questions; these are the binding choices (referenced as DD1–DD6 throughout).

**DD1 — Where legacy `scalar_exprs` are translated (the adapter boundary).** **In the native parser, at load time** — not on the wire, not per-handler. The wire format and fingerprint **keep the compact `scalar_exprs` side-table** (v1 fingerprints byte-stable; no plan-size inflation); the native parser **lowers `scalar_exprs`/`expr_refs` into typed-stack tokens in a separate, generously-sized in-memory arena** (not the 256-token wire cap), under the isolation contract (§2a). The runtime **core then has exactly one VM** — the nested `[op,a,b]` *evaluators* (`sweep_cli.c:3855,6372`) are deleted; the nested *encoding* survives only as a wire form the parser decodes. This is the **only** boundary that handles in-flight ASL-forwarded payloads (`$.plan.pipeline.{coeff,param}_program`, `asl:211,278`) uniformly with freshly-compiled ones. **The parser's lowering path is permanent — not drain-gated** (the compact wire is permanent). The only drain/version concern is *opcode renumbering* (DD2/§4.2), a separate axis from `scalar_exprs`: the runtime nested *evaluators* are deleted at Phase 2 (replaced by parser lowering, which accepts the same compact wire the old evaluator did, so **no `scalar_exprs` drain is needed**). (Supersedes the "never emitted" wording in §2a and §3.2 — the compact form *is* the permanent wire; it is just never *executed* as a nested VM.)

**DD2 — Native version gate (Phase 0; the first hard prerequisite).** Add a `version` field to the token-stream wire and make the native parser (`sweep_cli.c:3741,6267`) **reject unknown/newer versions** with a clear error. "No behavior change *for valid v1 payloads*"; intentionally a change for *unsupported* payloads. Converts the worst failure — silent misexecution of a renumbered v2 stream by rolled-back v1 (§7.4) — into a clean reject. Must land **before any opcode renumbering**.

**DD3 — Migration must never overwrite v1 (hard rule).** Save-by-slug overwrites in place (`handler_storage.py:1199`) and fingerprints hash the *expanded* macro chain (§7.3), so migrating in place would clobber a v1 macro that old artifacts still resolve. **Rule:** migration writes v2 under a **versioned identity** (`<id>@v2` or a `v2/` namespace) via a **non-overwriting migrate route**; v1 objects stay immutable until their artifacts are provably unreferenced. Walk the macro DAG bottom-up (reuse the existing cycle/depth guard).

**DD4 — Token-budget corpus gate (before changing side-table handling).** Caps are concrete: coeff 256, param 64 tokens (`sweep_cli.c:3343,6033`), plan 200 KB (`handler_compute_plan.py:47`). DD1 keeps the wire compact, so the gate is: run a **worst-case saved-program corpus** (programs near the token cap with many dynamic-indexed `cf[i]/poly[i]/tos[i]` expressions) through the parse-time lowering and assert the in-memory arena bound holds.

**DD5 — Verification: build the oracle (gates Phase 2).** Native tests run `--score_program` but compare the binary against *itself* (span readers), not a reference. Build (a) a **Python solve-score evaluator** as a cross-language oracle (none exists), and (b) an **old-VM-vs-new-VM whole-sweep byte-exact gate** (full coeffgen + roots `.bin`) on warm-start-sensitive programs — single-cell parity is insufficient under §7.1's cascade.

**DD6 — Benchmark gates (before unified dispatch ships).** Per-profile, vs the current `switch`+direct-register baseline: param-only, coeff scalar-heavy, coeff vector-heavy, solve-score multi-channel, root-transform raster path; plus per-thread workspace bytes (§7.7).

---

## Part 1 — The four DSLs as they exist today

| | **Coeff program** | **Param program** | **Solve-score** | **Root transforms** |
|---|---|---|---|---|
| Model | typed stack VM | untyped stack VM | metric prepass → scalar stack VM → emit | **flat sequential list** (no VM) |
| Value type | complex `vector` + complex `scalar` (tagged) | complex scalar only | `double` scalar slots (from roots-vector reduction) | `float` root vector, in-place |
| Stack | 64 slots × 256-len vectors | 16 scalars | 32 doubles | none |
| Opcodes | 30 (`COEFF_OP_*`) | 27 (`PARAM_OP_*`) | ~25 chips → tokens | 9 hand-coded `strcmp` arms |
| Registers | `cf`,`poly` vec + `p1/p2/t1/t2/poly_len` scalar | `t1/t2` (in) `p1/p2` (out) | inputs `slv`/`cf`/`pm`; out: 1 scalar or N channels | the root array |
| Scalar-expr sub-VM | yes (27 `EXPR_*` ops defined, max enum 31, dynamic indexing) | yes (10 `EXPR_*` ops defined, max enum 14) | no | no |
| Text source | **yes, full round-trip** | **no** | **no** | **no** |
| Fingerprint | `sha1({v, rendered-string})` | `sha1({v, raw-token-JSON})` | `sha256({program_spec, metrics})` | none (folded into palette identity) |
| C location | `sweep_cli.c:3340-5183` | `sweep_cli.c:6030-6783` | `solve_score.h` (1861 lines) | `root_xforms.h` (282 lines) |
| Python | `coeff_program_chain.py` (2158), `coeff_program_source.py` (905) | `param_program_chain.py` (1242) | `solve_score_chain.py` (1110) | `color_artifact_meta.py:124` parse only |
| Run context | coeff-gen time, per row | param-gen time, per grid cell | per solve step (with prepass) | **raster time, per sample** |

**Coeff VM = the reference.** Two value types on one stack — `COEFF_STACK_VECTOR=1`, `COEFF_STACK_SCALAR=2` (`sweep_cli.c:3388-3391`), with the "scalar slots beside vector planes" layout (`sweep_cli.c:3504-3536`, the stale-bytes invariant documented at `:3509-3511`). 30 opcodes (`sweep_cli.c:3355-3386`) including typed scalar/vector binary/unary with **broadcast** (`coeffProgramTypedBinaryOp:4438`), **dynamic indexing** (`coeffProgramTypedGetScalar:4519`), vector generators (linspace/range/const), and the `NATIVE_TRANSFORM` registry bridge (`:4609` → `coeffLegacyApply:4152` over `coeff_legacy_registry.json`, 28 `ct_*` fns). Full text→chips→tokens pipeline: `parse_coeff_program_source` (`coeff_program_source.py:855`) → `compile_coeff_program_chain` (`coeff_program_chain.py:2113`) with macro expansion (`:1499`), stack validation (`:1663`), `execution_spec` (`:1838`), `sha1` fingerprint (`:1977`). Caps at `coeff_program_chain.py:20-28` / `sweep_cli.c:3343-3349`.

**Param VM = a strict subset** in capability but an **incompatible sibling** in encoding. `ParamCx{r,i}` scalar stack only (`sweep_cli.c:6097-6100`), depth 16; produces `(p1,p2)` complex scalars that *feed the coeff VM as inputs* (`CoeffEvalContext`, `sweep_cli.c:3542-3550`, read at `:3891-3898`) — so the coeff scalar model already contains the entire param value model. Crucially: arithmetic is **dedicated top-level opcodes** (`PARAM_OP_ADD/MUL/SQUARE/EXP/...`, `sweep_cli.c:6050-6067`) where coeff folds the same operations into `fn_index`-dispatched typed binary/unary. Chips-only — **no `param_program_source.py` exists** (the similarly-named `param_source.py` is the params.bin record layout, unrelated). Two fully separate C interpreters share zero execution code; only ~14 scalar-expr ops and the token struct layout are duplicated (copy-paste, not reuse).

**Solve-score = a hybrid**, not a pure stack VM. Phase A: a fixed **metric prepass** reduces the per-step root vector to normalized scalar slots — `solve_score_eval_metric_slots` (`solve_score.h:1593`) → `compute_solve_metric_score` (`:428`) → clip-to-[0,1] (`:1588`). Phase B: a postfix `double stack[32]` VM over those slots — `solve_score_eval_program_outputs_from_buffers` (`solve_score.h:1636`), ops push_metric/const/dup, the combine family (avg/min/max/mul/add/weighted_sum/ema/abs_diff/geometric_mean, `:1675-1758`), the unary family (omega_cosine/sawtooth/flip/clamp/sin/cos/log/exp/pow, `:1759-1798`), and emit/emit_norm/emit_none. Output is 1 implicit scalar or N explicit channels (cap 8, `SOLVE_SCORE_MAX_OUTPUT_CHANNELS`), bound to RGB/HSV/LUT externally by `color_render_contract.py:73`. Compiles chain → `program_spec` string (`solve_score_chain.py:489`) → C re-parses (`solve_score.h:1160`).

**Root transforms = not a VM at all.** A flat `for` loop applying `{name,args}` entries in place to a `float` root array — `apply_root_xforms` (`root_xforms.h:242-280`), 9 hand-coded transforms (`rotate_roots`, `moebius`, `invert_roots`, …), no stack, no expressions, "unknown transforms silently ignored" (`root_xforms.h:278`). Wire form is the same `[name, ...params]` array the program chains use (`js/09-render-orchestration.js:607`). Runs **per-sample at rasterization time** on `float` roots — a different execution context and element type than the coeff VM's coeff-gen-time `double` vectors.

---

## Part 2 — Commonality and divergence (the unification surface)

### 2a. The shared core (what the unified VM keeps as its spine)
Typed value stack (scalar + vector complex); stack management (dup/swap/pop/flush/const); typed binary/unary kernels with broadcast; **scalar arithmetic expressed as typed-stack tokens** (`push_scalar` / `typed_binary` / `typed_unary` / `typed_get_scalar` — all already present, `sweep_cli.c:3376,3378,3380`); register push/emit; the `native_transform` registry bridge (with `stack_arg_count` so per-row arguments arrive as stack values, `sweep_cli.c:4620`); macros (compile-time chip inlining, no runtime footprint — `coeff_program_chain.py:1499`); and the `sha1({version, execution_spec})` fingerprint shape. **The coeff VM already implements all of this.**

**Genuinely "one VM" means retiring the nested scalar-expression sub-VM, not keeping it.** Today there are two `[op,a,b]` expression evaluators living *inside* the token VMs — `coeffEvalScalarExpr` (`sweep_cli.c:3855`) and `paramEvalScalarExpr` (`sweep_cli.c:6372`) — that compute per-row token arguments. Keeping them would mean the "unified VM" still contains a second VM, contradicting the goal. The right move is to **lower scalar expressions into typed-stack tokens** (push the computed scalar; feed transforms via `stack_arg_count` stack args), so the unified compiler emits *only* stack tokens. The compact `scalar_exprs`/`expr_refs` form **stays the permanent wire encoding** (DD1 — keeping it compact avoids the token-budget blow-up of §7.2); what is removed is the runtime nested *evaluator*. The native parser lowers the compact form into typed-stack tokens at load, so *execution* sees only one VM. The evaluators are deleted at Phase 2 — parser-lowering replaces them and accepts the same wire — so this needs **no `scalar_exprs` drain**; only opcode renumbering needs the §4.2 drain.

**Isolation contract (specify before implementing the lowering).** Today `coeffEvalScalarExpr` runs on a *private* scalar stack (`sweep_cli.c:3870`) and resolves `tos[i]` against the user stack's depth *at expression start* (`sweep_cli.c:3921`) — so expression temps are invisible to the program and `tos[i]` always means the *pre-expression* top. Lowering onto the shared typed stack (where `_typed_push_scalar` pushes real slots, `coeff_program_source.py:284`, and `native_transform` pops them, `sweep_cli.c:4620`) loses both properties unless the lowering guarantees **(a) net-zero stack effect** — the lowered expression consumes its own temps and leaves exactly one result, so user-visible depth changes only by that single push — and **(b) frame-based `tos[i]`** — `tos[i]` resolves against a frame base captured *before* the expression's temps, never against a temp the expression itself pushed. Without both, stack-depth validation and `tos[i]`/peek reads silently change meaning. Pin the mechanism (a hidden expression frame / staging slots) before writing the lowering.

The param VM is otherwise the same architecture at smaller scale; root transforms are a degenerate "list of native_transform calls"; solve-score is the scalar engine plus a metric-reduction prepass.

### 2b. The three TRUE conflicts (these force the wire migration)

**C1 — Opcode numbers collide with different meanings.** `PARAM_OP` and `COEFF_OP` are separate namespaces that diverge after #1: op 8 = `POP` (param) vs `BLEND` (coeff); op 10 = `ADD` (param) vs `POKE_POLY` (coeff) (`sweep_cli.c:6040-6068` vs `:3355-3386`). One merged enum is mandatory, and it **renumbers every persisted token stream**.

**C2 — Legacy `fn_index` namespaces are disjoint and both 1-based.** `param_legacy_registry.json` indexes `pt_*` 1–48; `coeff_legacy_registry.json` indexes `ct_*` 1–28. Same integers, different functions, dispatched by `paramLegacyApply` (`sweep_cli.c:6464`) vs `coeffLegacyApply` (`:4152`). A unified `NATIVE_TRANSFORM` needs a **registry-namespace discriminator** (registry id, or disjoint ranges). The `fn_index` values are pinned wire format (`tests/test_coeff_program_drift.py`; JS "never renumber" at `js/07-transform-catalogs.js:453`), so they must be **preserved, not reassigned**.

**C3 — Fingerprint canonical forms are incompatible.** Verified directly:
- Param: `_execution_spec` = `json.dumps({"tokens": tokens})` — raw integer-opcode token dicts, and it **`del scalar_exprs`** (`param_program_chain.py:1107-1113`), i.e. param scalar-expression content is *omitted from the fingerprint entirely* — a latent collision bug the unification would fix but must reproduce for old fingerprints.
- Coeff: `_execution_spec` = a rendered canonical DSL string (op names, selectors, `.17g` numbers), with the explicit warning *"its exact byte layout is wire format: formatting changes here invalidate every cached artifact"* (`coeff_program_chain.py:1838-1843`).
- Both wrap in `sha1({"version": PROGRAM_VERSION, "execution_spec": spec})` (`coeff:1977`, `param:1116`). Unifying forces ONE canonical renderer; whichever wins, the other VM's every cached artifact re-hashes.

Plus a minor semantic conflict: param has **two divide ops** — `ratio` (zero→0, `sweep_cli.c:6156`) and `divide` (zero→hard error, `:6646`). Both must survive as distinct opcodes, not a flag, or saved param programs change behavior.

### 2c. The differences that are only config / additive (the reassuring majority)
| Dimension | Resolution |
|---|---|
| typed stack vs scalar-only | **config**: param mode masks the vector type. Coeff's typed model is the superset. |
| register file | **config**: coeff already carries `p1/p2/t1/t2`; param mode hides `poly`/`cf`. |
| dedicated arithmetic opcodes (param) vs `fn_index` dispatch (coeff) | **additive**: ensure each param op (incl. `unit_circle`/`square`/`cube`, real-returning `real/imag/abs`) has a typed-op equivalent. |
| emit/push as distinct opcodes (param) vs selector operand (coeff) | **config**: represent param register targets as selector values. |
| scalar-expr op set | **config**: coeff's is a near-superset; param mode forbids poly/cf-referencing expr ops. **Both** C evaluators (`paramEvalScalarExpr`/`coeffEvalScalarExpr`) are **deleted** — expressions lower to typed-stack tokens (§2a, under the isolation contract); they do not merge into a shared evaluator. |
| output contract (param→`p1,p2`; coeff→`poly`) | **config**: a declared output projection per use-case. |
| capacity caps | **config**: per-use-case `MAX_*` table. |

### 2d. The architecturally-new feature: solve-score cross-step lag
None of the existing VMs have cross-invocation memory — they are pure functions of one input. Solve-score's lag (`m{slot}-1`, depths {0,1}) reads a **previous step's** metric buffer (`solve_score.h:1653-1665`), implemented by a hand-rolled **current/recent double-buffer + per-step `memcpy` + step-0 warm-up prelude** replicated across **all four** rasterizers (verified: 25 `recentMetricBuffer` refs in `roots2pix_mt.c`, `solve_palette_chunk_mt.c`, `solve_proximity_hist_sectioned.c`, `solve_proximity_stats.c`; prelude e.g. `solve_proximity_stats.c:759-790`). The unified VM core must add a **history/windowing facility** (lagged registers + host-managed ring buffer + declared window depth). This is genuinely new VM machinery — and unifying it is also the chance to delete the scattered double-buffering.

### 2e. Execution-context differences (real work, not config)
The unified interpreter must be invokable from three different host contexts: coeff/param-gen (per row/cell, `double`), rasterization (per sample, `float` roots — root transforms), and per-solve-step with a metric prepass (solve-score). The `float` vs `double` root binding is observable precision, and the per-sample raster invocation is a hot loop the coeff VM never runs in. Plan for a thin, allocation-free interpreter entry that all three call.

---

## Part 3 — The unified VM design

### 3.1 Value & register model
**Values:** `{ scalar: complex f64, vector: complex f64[] }` — exactly the coeff VM's typed model (`sweep_cli.c:3388-3391`). The float-root case (root transforms) is handled by a host-side bind that promotes `float→double` on entry and narrows on exit (preserve today's precision by documenting the cast points, `root_xforms.h:121-122`).

**Per-profile workspace sizing (not a shared constant).** The coeff workspace is fixed at 64 stack slots × `COEFF_PROGRAM_MAX_VECTOR_LEN = 256` (`sweep_cli.c:3345`, ~274 KB), but the root/solve raster paths operate on degree vectors up to `MAXDEG = 1024` (`solve_palette_chunk_mt.c:44`, `solve_proximity_stats.c:40`). The unified VM must allocate **per-profile** vector/stack storage sized to that profile's caps; reusing the coeff 64×256 workspace for root/solve vectors would overflow on a 1024-element vector. Treat workspace dimensions as part of the use-case config (§3.3), allocated per profile — not a single shared constant.

**On "two register files."** The framing (param scalars / coeff vectors) is the right *mental model* and is **already realized inside the coeff VM** (scalar slots beside vector planes). But honestly it is a small set of **named, typed, role-tagged registers selected by config**, because solve-score and root transforms need bindings the two-file model doesn't name:
- **Scalar inputs:** `p1,p2,t1,t2,poly_len` (param + coeff already share these).
- **Vector inputs (read-only, host-supplied per invocation):** `cf` (coeffs), `roots` (solved roots — new, for solve-score & root transforms), `pm` (param row — solve-score).
- **Vector output register:** `poly` — written in place by **coeff** only. **Root transforms write the `roots` register in place, *not* `poly`** (the table in §3.3 has this right); do not implement root transforms through a `poly` alias.
- **Scalar/channel output:** `p1,p2` (param) or the **emit channel vector** (solve-score, ≤8).
- **History:** a depth-D ring of the metric/slot buffer (solve-score lag).

So: keep the two-register-file vocabulary in the UI/docs, implement it as the above typed register table gated by the use-case config.

### 3.2 Opcode set (the union, deduplicated)
- **Stack core:** const/push/emit/dup/swap/pop/flush.
- **Typed arithmetic:** `typed_binary[fn]`, `typed_unary[fn]` with broadcast (absorbs param's dedicated arithmetic and coeff's vector ops). Keep both `ratio` and `divide` (C3 minor).
- **Scalar arithmetic (no sub-VM):** lowered to typed-stack tokens — `push_scalar`, `typed_binary[fn]`, `typed_unary[fn]`, and `typed_get_scalar` for dynamic `cf[i]/poly[i]/tos[i]` indexing (all already exist). The compact `[op,a,b]` `scalar_exprs`/`expr_refs` form **stays the permanent wire encoding** (DD1); the native parser lowers it to these typed-stack tokens at load, so the runtime nested *evaluators* (`sweep_cli.c:3855,6372`) are deleted — execution sees only typed-stack tokens. No `scalar_exprs` drain (old evaluator and new parser-lowering accept the same compact wire); only opcode renumbering needs the §4.2 drain. Param mode masks the poly/cf-referencing forms.
- **Vector generators:** const-vector / linspace / range (coeff-only today; config-gated).
- **Registry bridge:** `native_transform[registry_id, fn_index, src, tgt, andy, args]` — absorbs `ct_*`, `pt_*`, **and the 9 root transforms** (onboard them into a registry; §4). The `registry_id` resolves C2.
- **Solve-score additions:** `reduce_metric[metric, source, lag, clip_lo, clip_hi]` (vector-input → normalized scalar; this is Phase-A made into opcodes, but keep the prepass/slot-dedup as an optimization — see 3.5); `omega_cosine[ω,φ]` (one more unary); `emit[mode]` draining the scalar stack to output channels with the implicit-1/explicit-0 terminal contract.

### 3.3 Per-use-case config (the customization the user asked for)
One capability descriptor per use-case selects: **enabled opcodes**, **register layout & I/O contract** (which inputs bind, which register(s) project to the result), **type set** (scalar-only vs +vector), **caps** (`MAX_TOKENS/STACK/VECTOR_LEN`), **enabled transform registries**, and **window depth** (lag). Sketch:

| use-case | inputs | output projection | types | vectors? | transforms | lag |
|---|---|---|---|---|---|---|
| **param** | t1,t2 | p1,p2 (scalars) | scalar | no | `pt_*` registry | 0 |
| **coeff** | cf, p1,p2,t1,t2 | poly (vector) | scalar+vector | yes | `ct_*` registry | 0 |
| **root** | roots (in place) | roots (vector) | scalar+vector | yes | root registry | 0 |
| **solve-score** | roots/cf/pm | 1 scalar or N channels | scalar (+vec inputs) | reduce-only | — | 0–1 |

### 3.4 Compile pipeline unification
One `text → chips` parser (per-use-case grammar sharing one lexer), one `chips → tokens` compiler (config-parametrized `_compile_chip` + `_validate_stack`), one `execution_spec` renderer, one `sha1`/`sha256` fingerprint. Macros carry over unchanged. The chip-list stays as the intermediate IR (it's the natural migration seam and keeps macros/legacy round-tripping working).

### 3.5 The C side
One interpreter in a shared header (replacing the duplicated param/coeff interpreters and the `root_xforms.h` list-applier), dispatching on the merged opcode enum, gated by a runtime config struct. Delete the two runtime scalar-expr *evaluators* (`sweep_cli.c:3855`, `:6372`) at Phase 2, replacing them with parser-side lowering of the (permanent, DD1) compact `scalar_exprs` wire into typed-stack tokens — there is no nested evaluator to dedup. **This needs no `scalar_exprs` drain:** both the old evaluator and the new parser-lowering accept the same compact wire and must be bit-identical (DD5); the §4.2 drain applies only to opcode renumbering. Centralize the lag ring-buffer + warm-up (deleting the 4 hand-rolled copies in §2d). **Keep the solve-score metric prepass as an optimization** — `reduce_metric` opcodes should still be hoisted/slot-deduped out of the per-output loop (today `solve_score_eval_metric_slots` runs once per step, the cheap VM runs per output, `solve_score.h:1593` vs `:1636`); don't regress that into per-output re-reduction.

---

## Part 4 — The legacy migration (the "translate from old" function + Legacy tab)

This is the real cost, exactly as D5 warned. Two halves: a wire/fingerprint version scheme, and a per-DSL translator.

### 4.1 Fingerprint/wire-format blast radius (what breaks if token bytes change)
There are **eight** persisted fingerprint/wire families; the five that gate reuse/dedup:

1. **coeff/param program fingerprints** → baked into the plan and `calc.json` (`handler_compute_plan.py:279,286,557,564`), folded into **`probe_signature`** which is *compared with a mismatch-raise* for fused-compute reuse (`handler_compute_plan.py:231-241`).
2. **`palette_variant_fingerprint` / `content_fingerprint`** (`sha256` over the solve-score fingerprint + chain + interpretation + root_transforms, `handler_palette_render_plan.py:175-213`) — **the fail-closed trap**: `_load_existing_palette_for_identity` **`raise`s "Palette artifact id collision"** when the stored fingerprint ≠ expected (`handler_palette_render_plan.py:297-300`, verified). Because the `palette_id` slug embeds `digest[:12]`, a silent VM change on the same logical artifact throws rather than recomputes. **This must be made version-aware (dual-read → recompute) before any token change ships, or palette reuse bricks.**
3. **solve-score `chain_fingerprint`** (`sha256`, `solve_score_chain.py:641`) — persisted into solve-score/clip/bins/hist metadata, **required** in the raw sidecar (`raw_sidecar.py:197-198`, raises if missing), and validated with mismatch-raises across **7 raise sites in 5 handler files**: `handler_raster_mt.py:259`, `handler_solve_proximity.py:404`, `handler_palette_finalize.py:316,331`, `handler_finalize_mt.py:106,274`, and **`handler_palette_chunk.py:417` (bins fingerprint) plus `:421`** — the latter a `program_spec` **string** compare, a *second wire surface* the hash-only view misses (Part 7 §7-second-dive). (`handler_render_lores_preview.py:559` only *assigns* the fingerprint — it is **not** a validator; earlier drafts wrongly listed it.)
4. **Saved programs** (`polypaint/{coeff,param,solve-score}-programs/<id>.json`) store a `fingerprint`; **only coeff stores `source_text`** — param/solve-score can only be re-migrated from their stored `chain` (no text fallback).
5. **`fn_index`** wire numbering in both legacy registries (pinned by `test_coeff_program_drift.py`, surfaced to JS via generated `coeff_vocab_js.js`).

There is **no `spec_version` anywhere today** (verified: 0 hits) — only three independent `*_VERSION = 1` constants (`coeff_program_chain.py:19`, `param_program_chain.py:26`, `handler_storage.py:59`).

### 4.2 Migration strategy
1. **Introduce a unified `spec_version` stored adjacent to (not inside) the hashed payload** so existing v1 fingerprints stay byte-identical (D6's "cheap insurance," `architecture-ui-critique.Rnw:349`; folding it into the hash would re-key everything back at Phase 0 — see Phase 0). The v2 canonical form is hashed and emitted **only at this migration point**. Old artifacts stay valid under v1; the unified VM emits v2.
2. **Dual-read everywhere reuse is decided** — the probe-signature guard, the palette collision-raise (#2 above — change to "version mismatch ⇒ recompute, not raise"), and the six solve-score `chain_fingerprint` validators must accept `{v1 old-scheme, v2 new-scheme}` during transition.
3. **`translate_from_old(kind, payload) → unified_chain`** maps each old format to the new token IR:
   - **coeff** → essentially identity (it's the reference); re-fingerprint under v2.
   - **param chain** → unified tokens: dedicated arithmetic opcodes → typed-op equivalents; `pt_*` legacy → `native_transform[param_registry, fn_index]` (preserve fn_index); emit_p1/p2 → output projection.
   - **root `[name,...params]` list** → a sequence of `native_transform[root_registry, fn_index]` tokens (trivial once the 9 transforms are registry-onboarded; §4.3).
   - **solve-score chain** → `reduce_metric` + scalar ops + `emit`; preserve omega/phase/lag/emit exactly (they're already in `program_spec`).
4. **Preserve `fn_index` numbering** (C2) — never renumber the legacy registries.
5. **Drain in-flight payloads before the opcode-renumbered v2 stream goes live.** Step Functions forwards already-compiled `coeff_program`/`param_program` objects straight into native tasks (`compute_workflow.asl.json.template:211,278`). The hazard is the **opcode renumbering** (C1): a binary handed a token stream numbered for the *other* version misexecutes. The DD2 version gate turns that into a clean reject; deploy so no in-flight execution straddles the renumbering (a drain window, or translate at the Lambda boundary). **Note (corrected):** the compact `scalar_exprs` form is *not* part of this drain — it is the permanent wire (DD1), accepted by both old and new binaries; only the opcode *numbering* changes across versions, so the `scalar_exprs` evaluators are deleted at Phase 2 without a drain.

### 4.3 Prerequisites the design exposes
- **Three text parsers must be built** before chips can become display-only: param, solve-score, and root transforms have **no text round-trip** (only coeff does, via `coeff_program_source.py`). Each needs a grammar + parser + `source_text` round-trip mirroring coeff. This is a substantial, independent chunk.
- **Root transforms must be onboarded into the registry** (`coeff_legacy_registry.json`-style): they're 9 hand-coded `strcmp` arms (`root_xforms.h:246-277`) with a hand-maintained JS mirror (`_rtCatalog`, `js/07-transform-catalogs.js:273`), *not* in the generated-vocab pipeline. Give each a `fn_index` + registry entry (`length_policy: preserve`, root-vector binding) so they flow through `native_transform` and `gen_coeff_vocab.py`. The migration needs an explicit **root-transform ABI** section pinning two behaviors that differ from the coeff registry:
  - **NaN-on-undefined must be preserved.** `rt_mark_undefined` writes `NAN` for poles/undefined outputs (`root_xforms.h:108-112`) and downstream rasterizers clip on NaN; the unified `native_transform` path must keep this exact propagation (not turn it into an error).
  - **Unknown-transform behavior is a deliberate change.** Today unknown names are *silently ignored* (`root_xforms.h:278`); a registry/`fn_index` VM naturally *rejects* an unknown index. Rejecting is the better default, but it **is** a behavior change — state it explicitly, and ensure `translate_from_old` never emits an unknown/aliased root name so existing saved chains keep working.
  - **Root canonicalization touches more caches than palette identity.** Beyond the fail-closed palette `palette_variant_fingerprint` (§4.1 #2), `root_transforms` also feed the render-side `_plan_params_digest` (`handler_render_plan.py:57`) and the `_solve_score_scratch_key` 8-char rt-hash (`handler_render_plan.py:135`). A v2 root canonical form shifts those too — mostly cache-miss (recompute), not fail-closed, but Phase 0's reuse inventory must enumerate them.
- **The pre-VM by-name legacy dispatch paths** can be retired *after* migration: `lookupCoeffTransform` strcmp ladder (`sweep_cli.c:3246`), `parseCtChain` (`:5640`), the param by-name path (`:2749`,`:5575`). The `pt_*`/`ct_*` are already ~90% migrated (registry + `fn_index` VM dispatch live); migration is mostly *deletion*. (Earlier drafts cited `sweep_cli.c:8939` as a "`legacy_transforms` round-trip shim" — **incorrect**: that line is a benchmark `printf`, not a shim, and no such helper exists. Legacy chips must be lowered through `translate_from_old`.)

### 4.4 The Legacy tab / migrate feature (per the request)
A UI that loads an old saved program (any of the four kinds), runs `translate_from_old`, shows the **new text form** (now possible because Phase 3 built the parsers), and lets the user save the migrated v2 version. Old artifacts keep resolving under v1 (dual-read); new work is v2. This *is* the "real feature need" D5 said to queue behind (`architecture-ui-critique.Rnw:318`) — so doing the migration to enable it is coherent rather than speculative.

### 4.5 Byte-exact behaviors the migration must preserve (each a documented past bug)
`-0.0` canonicalization across static-fold and dynamic eval incl. log/sqrt/angle principal-branch (`coeff_program_chain.py:784-791` + C mirror); the scalar-slot-beside-vector-plane stale-bytes invariant (`sweep_cli.c:3509-3511`, the `:3931` bug); registry arg-packing quirks (linear/exp/pow pack complex pairs, `sweep_cli.c:4634-4677`); the range `ceil(span-1e-12)` fudge (`:4863`); and the param **scalar_exprs-dropped-from-fingerprint** quirk (§2b C3) for old v1 fingerprints. Pin all with native-parity tests (the existing `test_coeff_program_drift.py` / `test_*_program_native.py` discipline).

---

## Part 5 — Phasing (ordered; each phase ships standalone value)

- **Phase 0 — versioning insurance (no behavior change *for valid v1 payloads*).** Store a `spec_version` **adjacent to** each fingerprint (a sibling field in the meta/payload), **not inside the hashed bytes** — folding it into the hashed payload would change every fingerprint immediately, the opposite of no-change. (Equivalently: carry dual `fingerprint`/`fingerprint_v2` fields.) Keep emitting only the v1 hash here; add dual-read plumbing at every reuse decision — first **inventory** them (the palette identity, the probe signature, the six solve-score `chain_fingerprint` validators, and the render-side root-transform digests/scratch keys in §4.3) — including making the palette collision-`raise` *recompute* on a version/scheme mismatch instead of throwing (`handler_palette_render_plan.py:297`). **Also add the native token-stream version gate here (DD2):** the parser reads a `version` and rejects unknown/newer — intentionally a behavior change for *unsupported* payloads, and it kills the silent-misexec failure (§7.4). The v2 hash is emitted only at Phase 4. Independently valuable (D6). *Low risk for valid v1 payloads.*
- **Phase 1 — extend the registry to the structural program chips (more than vocab).** The ~20 structural chips (push/set/emit/littlewood/macro…) are not just UI labels — they are executable **compiler functions** with stack effects, arg typing, selector rules, and token lowering (`_CHIP_COMPILERS`, `coeff_program_chain.py:1595`). A registry that only emits JS labels will *not* kill drift. To actually remove it, the registry must carry the compiler-relevant metadata (stack effect, arg types, selector rules, dispatch target) and *drive* compilation, with drift tests pinning Python/C/JS agreement — the same D4-1 move that landed for transforms (`architecture-ui-critique.Rnw:332`), but a larger surface. No *runtime* VM change. *Low-to-medium risk — bigger than a vocab-gen.*
- **Phase 2 — shared runtime behind frozen front-ends (adapter layer).** This is only possible with an explicit adapter boundary: the per-VM **front-ends stay byte-identical** — each VM keeps its own parser, token normalization, `execution_spec` renderer, and fingerprint exactly as today (param still `del scalar_exprs`, `param_program_chain.py:1107`; coeff still treats spec bytes as wire format, `coeff_program_chain.py:1838`) — and only the **runtime interpreter internals** are shared behind them. The merged opcode enum, the expression-to-token lowering, and the centralized lag ring-buffer (delete the 4 hand-rolled copies) are all **internal** and must not reach the wire or the spec string until Phase 4. Re-express the param VM as a config of the shared runtime (coeff is the superset). Pin with native-parity tests so the shared runtime is bit-identical to both old interpreters. *Largest C change; medium risk — the adapter discipline is exactly what keeps fingerprints from moving.*
- **Phase 3 — build the three missing text parsers** (param, solve-score, root) with `source_text` round-trips. Enables text-first authoring. *Medium effort, low risk (additive).*
- **Phase 4 — unified token IR + single fingerprint scheme (v2) + `translate_from_old` + Legacy tab.** This is the fingerprint version bump. Dual-read v1/v2; onboard root transforms into the registry. *Highest risk — the migration proper.*
- **Phase 5 — chips → display-only.** Switch the existing readonly chip renderers (`_renderCoeffProgramChipHtml`, etc., already support `{readonly:true}`) as the visualization; retire the picker/edit JS and the pre-VM by-name dispatch paths. *Low risk once Phases 3–4 land.*

---

## Part 6 — Risks & recommendation

**Top risks, in order (revised by Part 7 — the earlier palette-first ordering was wrong):** (1) **native silent-misexecution on rollback** — no version gate on the token stream (§7.4, DD2); (2) **the migration can't be shown safe** — warm-start path-dependence + no oracle (§7.1, DD5); (3) **token-budget blow-up** from lowering (§7.2, DD4); (4) byte-exact fingerprint/behavior preservation across the ~6 mismatch-raising solve-score validators + the required raw-sidecar fingerprint; (5) macro-DAG transitivity + overwrite-by-slug (§7.3, DD3); (6) param/solve-score/root have no `source_text`, so text-first needs three new parsers. The palette collision-`raise` (earlier flagged #1) is mostly a cache-miss — §7.0.

**Effort:** the VM merge proper is moderate (coeff is already the superset). The bulk is the migration (Phase 4), the three text parsers (Phase 3), and the lag facility (Phase 2). This touches Python compilers, both C interpreters, the JS editor, every fingerprint/persistence site, and Step Functions payloads — large but well-bounded by the existing cross-language drift tests.

**Recommendation:** Do **Phase 0 and Phase 1 now** regardless of the larger plan — they're pure wins and the doc already endorses both. Fund **Phases 2–5** as one program *because* the Legacy-tab migration is the feature that justifies the fingerprint bump D5 deferred. If appetite is limited, Phases 0–3 deliver most of the developer-experience win (one C interpreter, text-first authoring) **without** the v2 fingerprint migration — stop there and leave chips editable until the migration is worth it.

---

*No production code was modified by this review.*

---

# Part 7 — Deep dive: previously-unexamined obstacles

Parts 1–6 (and three review rounds) hardened the VM *internals* — value model, wire conflicts, fingerprints, lowering, deploy drain, workspace sizing. This pass examined the surfaces none of those rounds touched: whether other implementations exist, the performance/hot-loop budget, floating-point determinism vs the content-addressed cache, the full native-binary CLI surface, how you'd *prove* the migration safe, and cross-stage/feature interactions. Findings ranked by blast radius; every claim re-verified against source at HEAD `32930f4`. Several **correct earlier parts** — collected in §7.0.

## 7.0 Corrections to earlier parts (these supersede the cited text)

- **Register handoff (§3.1 is wrong).** §3.1 says param + coeff "already share" `p1/p2/t1/t2`. They don't share them as peers: param *writes* `p1/p2` as its **output** (`sweep_cli.c:6767`) and coeff *reads* them as **inputs** (a role flip), and param's `t1/t2` are **real-only** (`t1 = param_cx(t1r, 0.0)`, `sweep_cli.c:6566`) while coeff's are **full complex** (`evalCoeffProgram(... t1r, t1i ...)`, `sweep_cli.c:5101`). A flat "shared register table" loses both the direction and the type. The unified config must model register **role** (in/out) and **element type** per stage.
- **Degree/workspace overflow (§3.1 refine).** §3.1 warns a "1024-element root vector would overflow the coeff 64×256 workspace." Roots **cannot** reach 1024: the solver caps at `MAX_DEGREE 255`/`MAX_COEFFS 256` (`sweep_cli.c:25-26`). The real defect is a **cap inconsistency across sibling rasterizers**: `roots2pix_mt.c:28` uses `MAXDEG 256` but `solve_palette_chunk_mt.c:44` and `solve_proximity_stats.c:40` use `MAXDEG 1024`. Per-profile sizing must *reconcile* these, not just "size for 1024."
- **The palette collision-`raise` is NOT the scariest rollback spot (§4.1/§6 reframe).** Because `palette_id` embeds `digest[:12]` in the S3 slug (`handler_palette_render_plan.py:212`), a v2 fingerprint usually yields a *different* slug → key-miss → recompute (a cache miss), and after rollback v1's slug still points at the surviving v1 artifact. The `raise` (`:297`) only fires if a future scheme keeps the slug stable while changing the hashed payload. The **actual** rollback poison is §7.4.
- **Pipeline shape (§3.3 understates it).** "One VM" is one *codebase* invoked as staged configs across **four separate executables** with `.bin` files as the inter-stage wire — not one program (§7.5). And **root transforms are not a peer use-case**: they run as a per-sample sub-stage *inside* the solve-score prepass, on a `float` scratch copy of the roots (`roots2pix_mt.c:202`, `solve_palette_chunk_mt.c:226`).
- **Good news (assumption confirmed).** There is **no third/fourth VM**: no WASM and no browser-JS executor (0 wasm/step_loop tracked; 0 `Worker`/`WebAssembly` in `js/`; `solve_score_chain.py` has no numeric evaluator). The parity-target count is exactly Python (compile) + native C (run), as Parts 1–6 assume. (Note: prior project memory citing a "WASM build" and "Web Workers" was stale — those were removed in the JS-split restructure.)

## 7.1 [highest] You cannot currently *prove* the migration safe — warm-start makes the whole sweep path-dependent, and there is no oracle

This is the obstacle the plan most needs and never names. The content-addressed cache reuses bytes when the **program** fingerprint matches, but the fingerprint covers neither output bytes nor the *evaluation path*. Two compounding facts make single-cell parity insufficient:

1. **The root field is path-dependent across steps.** The solver warm-starts each step's roots from the previous step's (`warmStartNeedsReseed`, `sweep_cli.c:125`; in-place EA iteration from preserved `rootRe/rootIm`), and the sweep runs in **serpentine** order to keep steps adjacent (`(i1 & 1) ? (n2-1-j) : j`, `sweep_cli.c:7909` + 6 sites). So a **1-ULP** change to any cell's coefficients (e.g. from the §2a lowering touching `-0.0`/principal-branch handling differently in one of {Python static fold, C dynamic eval}) can flip *which* root each downstream step converges to — a cascade across the entire chunk. The program fingerprint is unchanged, so the system **silently reuses poisoned/mismatched artifacts**. Blast radius = the entire render family.
2. **There is no cross-language numerical oracle for the riskiest DSL.** `solve_score_chain.py` only compiles/validates — it computes **no score** (0 evaluators). Solve-score parity is enforced only C-vs-C (single-span vs multi-span readers of the *same* binary). Coeff parity is checked at **epsilon `1e-6`** against hand-written expected lists (`test_coeff_program_native.py:119`), not bit-exact — a drift that still changes the SHA-addressed output bytes downstream **passes green**. Param is byte-exact but C-vs-C only.

**Consequence:** "the VM is bit-identical on one cell" does not imply cache safety. The migration needs a **whole-sweep, byte-exact golden gate** (full coeffgen + roots `.bin`, old VM vs new VM, on warm-start-sensitive programs) plus a **new Python solve-score evaluator** to act as an oracle — neither exists. This gate is a prerequisite, not a nice-to-have.

## 7.2 [major] Lowering scalar-expressions fights the token budget — it can make valid programs uncompilable and blow the plan size

The §2a/§3.2 decision to lower expressions into typed-stack tokens has a cost Parts 1–6 missed. Today a scalar expression of *any* size is **one** `expr_ref` pointing into a separate side-table (`scalar_exprs[64]`, each ≤32 ops; `sweep_cli.c:3347,3501`) — it costs **zero** main-stream tokens. Lowering moves the ops into the main stream: ~**1 token per op**, and **3 tokens per dynamic index** (`cf[i]`/`poly[i]`/`tos[i]`, `coeff_program_source.py:292`). So:

- **Token-cap overflow.** A program near `COEFF_PROGRAM_MAX_TOKENS = 256` (`sweep_cli.c:3343`; param's cap is **64**) with a few dynamic-indexed expressions can balloon past the cap after lowering — a previously-valid, currently-saved program becomes **uncompilable** under the unified VM.
- **Plan-size overflow.** The compiled `{tokens, scalar_exprs}` rides **inside the Step Functions plan** (`handler_compute_plan.py:284`), guarded at `MAX_PLAN_BYTES = 200 KB` (`:47`). Lowering replaces a compact `scalar_exprs` float table with a long `tokens` array of opcode dicts — the same program can trip the 200 KB guard and **fail the whole compute plan**.

This is a genuine tension with the "genuinely one VM" goal: purity (no nested expr VM) costs token budget. **Resolved by DD1:** keep `scalar_exprs` as the compact *wire + fingerprint* encoding (so v1 fingerprints are byte-stable and the plan does not inflate), and have the native parser lower them into typed-stack tokens in a **separate, generously-sized in-memory arena** (not the 256-token wire cap). The runtime core then has one VM. This avoids the wire/plan blow-up entirely; only the in-memory arena grows, and DD4's corpus test bounds it.

## 7.3 [major] The migration is transitive over the macro DAG

Macros are inlined **by value at compile time**, and the fingerprint hashes the **expanded** chain (`coeff_program_chain.py:2125→2137`; pinned by `test_coeff_program_chain.py:581`, `test_param_program_storage.py:149`). The saved object stores the *un-expanded* chain, so re-fingerprinting program B **re-resolves macro A from S3**. Therefore migrating B to v2 forces **resolving and re-fingerprinting every macro'd A under the v2 renderer** — you cannot migrate B in isolation; migration is transitive over B's macro DAG. Worse, "save migrated" is **destructive-by-slug** (`handler_storage.py:1207` overwrites in place), so migrating A in place can **overwrite the v1 A that dual-read still depends on** for old-artifact resolution. There is no rule, route, or test for a mixed v1/v2 macro graph. (Macros are kind-locked — coeff↔coeff, param↔param; solve-score has none — so at least the DAG doesn't cross kinds.)

## 7.4 [major] Rollback is unsafe on two distinct axes

If v2 artifacts are written during a rollout and the deploy is rolled back to v1:

1. **Native silently misexecutes a renumbered token stream.** `parseCoeffProgram`/`parseParamProgram` read the `tokens` array and **never read a `version` field** (verified: no version parse in the parser). Because the unified VM renumbers opcodes (C1: op 8 = coeff `BLEND` vs param `POP`; op 10 = coeff `POKE_POLY` vs param `ADD`), a v1 binary handed an in-flight v2 token stream runs the **wrong opcodes with no error**. This is far worse than the `scalar_exprs` drain (§4.2) — it's silent wrong math. A version gate on the token stream (reject unknown version) is mandatory *before* any renumbering ships.
2. **In-flight multi-stage renders fail closed.** The solve-score `chain_fingerprint` has **no version field in its hashed payload** (`solve_score_chain.py:642`) and is compared with mismatch-`raise` in 6 handlers (`handler_raster_mt.py:259`, `handler_palette_finalize.py:316,331`, `handler_finalize_mt.py:106,274`, `handler_solve_proximity.py:404`; required by `raw_sidecar.py:197`). A render whose *clip* stage ran under v2 and whose *finalize* runs under rolled-back v1 mismatches → the render aborts.

## 7.5 [major] The coeff program's output length is contractually input-invariant — a unified VM that varies it breaks the solver

The coeff degree is **probed once** at cell (0,0) by running the full param+coeff pipeline (`sweep_cli.c:7841-7870`), then **hard-asserted invariant for every grid cell** (`if (nCoeffs != nCoeffsOut) { … return 1; }`, `sweep_cli.c:7948`), and the warm-start chain `memcpy`s a fixed-size `degree` buffer per step (`:9516`). So the coeff program's output length must be **input-independent across the whole grid**. A unified VM that permits a data-dependent `poly` length (dynamic `typed_get_scalar`/`poly_len`, or a `length_policy: may_change` transform whose result varies per cell) would trip the per-row assertion and abort the chunk. The config model must encode a **fixed-output-length contract** for the coeff profile. The same program fingerprint also gates **degree-probe reuse** (`build_probe_signature` folds both program fingerprints, compared with mismatch-`raise` at `handler_compute_plan.py:240`) — so the C3 fingerprint change invalidates the fused degree-probe too, not just program reuse.

## 7.6 [concern] `native_transform` does not generalize — `andy`/`length_policy` are coeff-only

The two registries have **structurally different schemas**: `param_legacy_registry.json` has **no** `supports_andy` and **no** `length_policy` (0 occurrences) — it uses `kind` over scalar sources; `coeff_legacy_registry.json` uses `supports_andy`/`length_policy` over vector sources. `andy` (blend-with-original) is vector-element-wise and presumes output aligned with the input vector — meaningful for coeff, **nonsensical for a solve-score reduction** (output is scalar channels) and for **root transforms** (`float`, in place, NaN-poison, silent-ignore-unknown per `root_xforms.h:278`). A unified `native_transform` therefore needs **per-registry capability gating**, not one signature — otherwise the config admits nonsensical combinations (andy on a reduction, length-change on a root transform).

## 7.7 [concern] Performance & memory — the generality tax was never costed

The native C is the throughput path; Parts 1–6 never costed the refactor against it.
- **Dispatch tax.** Both VMs dispatch via a tight `switch` on `tok->op` (jump table; `sweep_cli.c:5130`, `:6575`) with registers as direct struct fields (`ctx->p1r`, `ws->poly_re`). A config-driven VM adds, **per token in per-pixel/per-row loops**: an enabled-op mask check, register-binding indirection (fixed fields → indexed lookups), and a more general value model. Folding param's *direct* arithmetic opcodes into coeff's `fn_index`-dispatched `typed_binary` turns one inlined op into a two-level dispatch — a measurable param-path regression §2c frames as merely "additive."
- **Workspace × threads.** `sizeof(CoeffProgramWorkspace) ≈ 273 KB` today, allocated **per worker thread**. A per-profile degree-1024 workspace is ~1 MB; at the 16-thread cap that's ~17 MB of VM scratch vs ~4.4 MB today, before the lag ring buffer. A real OOM vector on constrained Lambda memory.
- **`reduce_metric`-as-opcode risks per-channel re-reduction.** Solve-score today *physically separates* the O(degree²) metric prepass (run once per step, slot-deduped, `solve_score.h:1593`) from the cheap per-output stack VM (`:1636`). §3.2's "`reduce_metric` opcode" would, if dispatched inline, re-run the O(degree²) reduction **per output channel** (up to 8×). The unified compiler must **hoist and slot-dedup** reduce tokens into a prepass — which fights the "one interpreter, one token stream" framing.

**Required benchmark gates (DD6) — measurements, not prose.** Before the unified dispatch ships, each profile must show no regression beyond an agreed threshold vs the current `switch`-dispatch + direct-register-field baseline, benchmarked on its own hot loop: **param-only** (per grid cell), **coeff scalar-heavy**, **coeff vector-heavy**, **solve-score multi-channel** (per pixel × per step × per channel), and the **root-transform raster path** (per sample). Capture per-thread workspace bytes in the same harness (the 273 KB→~1 MB × threads concern above).

## 7.8 [concern] The CLI/test blast radius is undercounted

Parts 1–6 name 5 sources; the live program-consuming surface is larger:
- **3 more binaries** consume programs/transforms and must be migrated: `solve_palette_chunk.c` (single-thread, live via `use_legacy_binary`, takes `--score_program`/`--root_xforms`), `bilevel_section_raster.c` (`--root_xforms`, `:98`), `solve_palette_debug.c` (`--root_xforms`, `:133`).
- **The `--score_output_clip_los/_his/_normalize` family** and the file-based source flags (`--score_coeffs_file`/`--score_params_file`) are additional score-wire surface.
- **No param-side `fn_index` drift test exists** — only `ct_*` (coeff) is pinned by `test_coeff_program_drift.py`; the `pt_*` 1–48 namespace (C2) is materially under-protected, exactly where renumbering risk lives.
- Net: **~8 program-consuming binaries, ~16 handler construction sites, ~13 contract tests** pin the current per-DSL wire format.

## 7.9 Verification infrastructure that must be BUILT (not extended)

§4.5 says "pin with native-parity tests" as if the harness exists. It does not, for this job:
- **No test asserts fingerprint/wire equivalence** — all parity tests compare numeric output, never that `translate_from_old(old)` hashes or computes identically.
- **No independent solve-score oracle.** (Earlier drafts said "no native parity test at all" — overstated.) Native tests *do* run `--score_program`/`program_spec` (`test_raster_mt_parity.py`, `test_palette_chunk_mt_parity.py`, `test_solve_proximity_hist_sectioned.py`, `docker_runtime_regression.py`) — but they compare the binary **against itself** (single-span vs multi-span readers), not against an independent reference. The real gap (DD5): no Python solve-score evaluator and no old-VM-vs-new-VM whole-sweep byte oracle — the only things that would catch a migration drift, given §7.1's warm-start cascade.
- **No param/solve-score drift tests** (C1/C2 renumbering lands in untested enum space).
- **The list route can't drive a batch migration** (HEAD summaries only, no fingerprint/chain) → migration is N full GET+recompiles, each re-resolving macros (§7.3).
- **No `translate`/versioned-fetch/dual-kind/non-overwriting-save routes**, and **root transforms aren't a saved-program kind** — the Legacy tab's "four kinds" has no storage home for migrated root chains.

## 7.10 Revised recommendation

The deep dive does not change the *direction* (coeff VM as reference; migration is the cost) but it moves three things to **gating prerequisites** that Parts 1–6 treated as later detail:

1. **Add the native token-stream version gate first (§7.4, DD2) — before any opcode renumbering.** Silent misexecution of a v2 stream by a rolled-back v1 binary is the worst failure in the project, and the gate is cheap (a `version` field + reject). This is the one prerequisite that must land even before experimentation.
2. **Build the verification harness before Phase 2 (§7.1, §7.9, DD5).** A whole-sweep byte-exact old-VM-vs-new-VM gate + a Python solve-score oracle + param/solve-score drift tests — otherwise the migration cannot be shown safe and the warm-start cascade ships silently. Highest blast radius; largest effort.
3. **Resolve the token budget before changing side-table handling (§7.2, DD4).** DD1 keeps the wire/fingerprint compact (lower at parse time), so the work is bounding the in-memory arena with a worst-case corpus test.

Then the macro-DAG transitivity (§7.3), the fixed-output-length contract (§7.5), per-registry capability gating (§7.6), and the per-profile/perf budget (§7.7) are design constraints to encode, and §7.8 is the corrected blast-radius checklist. Phase 0 (versioning-adjacent-to-hash + the DD2 version gate) and Phase 1 (registry-driven structural chips) remain the safe, independently-valuable starting points.

---

# Part 8 — UI changes

The refactor has a real UI dimension (D5 alternative 4: "make text the only source and render chips as a read-only visualization") plus the new Legacy/migrate tab. This is its own workstream, gated behind the three text parsers (Phase 3).

## 8.1 Current UI

Chip-based editors for all four DSLs (`js/07-transform-catalogs.js`, `js/08-chip-editors.js`, `js/02-preview-solvescore.js`, `js/03-program-modals.js`): per-DSL chip chains (`_coeffProgramChain`, `_ppChain`/`_ptChain`, `_renderScoreChain`/`_paletteScoreChain`, `_rtChain`/`_paletteRtChain`), add/remove/move/edit-param controls, and the `_chipPickers` catalog-discovery popups. **Only the coeff program has a text source** — the textarea + `/compile-coeff-program-source` round-trip with chips↔text sync (`js/08-chip-editors.js:282-449`). Param, solve-score, and root transforms are **chips-only**. Chip rows currently serialize *wire layout* (the src/tgt-order "LAYOUT CONTRACT", `js/08-chip-editors.js:994`) — the D5 "chips serialize wire rows rather than user intent" leak.

## 8.2 Target UI

- **Text-first, but NOT uniformly — the goal is per-DSL (corrected; see §9.3).** "Text as single source for all four" over-generalizes from coeff. Accurate target: **coeff/param/root transforms → text-first** (chips become a read-only visualization via the existing `_render*ChipHtml{readonly:true}` renderers); **solve-score → chips stay primary, text is a read-only export.**
- **Why solve-score is the exception.** Its model is a *declarative metric prepass* (metric + source `slv/cf/pm` + quantile → a CSE-deduped normalized slot) plus a postfix combine program with *cross-step lag* (`mN-1`) and multi-channel emit. A readable text form (`s0 = spread(slv, q=0.5%)` … `avg(s0, s1)`) requires **inventing a named-slot binding sub-language the VM does not have**, then erasing it back to positional slots to preserve `program_spec`/fingerprint — and lag references are cryptic under implicit positional slots. The 44-metric enum with per-metric source gating is exactly what a pickable catalog is *for*. So solve-score keeps editable chips.
- **The easy two.** **Param** is a genuine subset of coeff's parser (its `ExpressionParser` already exists, `param_program_chain.py:573`); the only net-new surface is the `emit_p1/p2`/`push_t1` register keywords and transcribing the `legacy(...)` validation the chip compiler already encodes — text-first is clean. **Root transforms** are a flat one-transform-per-line list (`rotate_roots(0.5)`) — a ~30-line parser, no expression grammar at all. So §4.3's "three equivalent substantial parser chunks" is wrong: root is trivial, param is lexer-reuse, only coeff-class effort is solve-score (which we're *not* doing).
- **Chips-as-display needs a text→chip span map** (for click-to-locate / error underlining), i.e. an editor parser, not just the compile parser.
- **The Legacy tab** — unchanged from §4.4: list old programs → `translate_from_old` → diff preview → save under a versioned id (DD3, never overwriting v1). Needs the new storage routes (§9.4) that don't exist today.
- **Chips-as-display needs more than the compile parser:** to highlight/round-trip and underline errors, the parser must emit **per-chip source spans** (an editor parser, not just the compile parser).
- **The Legacy tab.** A new tab to *list* old saved programs (all kinds), *translate* each to its v2 text form via `translate_from_old`, *preview the diff*, and *save the migrated copy* under a **versioned id** (DD3 — never overwriting v1). Needs new/changed storage routes (a `translate`/`migrate` endpoint, version-aware fetch, non-overwriting save) that **do not exist today** (§7.9); and because root transforms aren't a saved-program kind, the tab needs a storage home for migrated root chains.

## 8.3 UX risks

1. **Discoverability regression.** Removing the chip *pickers* loses the catalog-browse affordance power users rely on. Mitigation: a read-only "insert from catalog" palette that emits *text*, or a command-palette over the generated vocab — discoverability without chips being the editable model.
2. **Transitional inconsistency.** The per-DSL staging means a period where some editors are text and others chips — confusing unless clearly labeled.
3. **Greenfield error UX.** The three new parsers need diagnostics on par with coeff's line/column (CR15 set the bar); chips currently surface errors inline, text must match.
4. **Trust in the migration preview.** The Legacy-tab diff must convey *semantic* equivalence (or differences), which depends on DD5's oracle existing — otherwise "migrated" is an unverified claim.

---

# Part 9 — Second deep-dive: audit corrections + operational blast radius

A full re-read of the (now large, heavily-edited) doc: re-verify every claim against HEAD, find internal contradictions, and probe surfaces the first deep dive never touched (operational/deploy/ecosystem, and whether the text grammars are even tractable). Direction and the Part-7 prerequisites are unchanged; this fixes coherence and adds the operational layer.

## 9.1 Corrections applied to this document (verified against source)

- **Resolved the central self-contradiction (the `scalar_exprs`/drain story).** DD1 says the wire *permanently* keeps the compact `scalar_exprs` side-table, but §2a/§3.2/§3.5/§4.2-step-5 said "never emitted / delete the evaluators after the drain" — incompatible, and the "drain" conflated two hazards. Now coherent across all five spots: **the compact `scalar_exprs` form is the permanent wire encoding; the native parser lowers it to typed-stack tokens at load (permanently); the runtime nested *evaluators* are deleted at Phase 2 with no `scalar_exprs` drain** (old-evaluator and new-parser binaries both accept the same wire and must be bit-identical, DD5); **the drain/version-gate is purely about opcode renumbering.**
- **Two factually-wrong citations fixed.** `sweep_cli.c:8939` is a benchmark `printf`, *not* a "legacy_transforms round-trip shim" (no such helper exists) — §4.3 corrected. `handler_render_lores_preview.py:559` only *assigns* the fingerprint, it is *not* a mismatch-raise validator — removed from the §4.1 list.
- **EXPR-op counts fixed** (Part 1 table): coeff scalar-expr is **27 ops defined** (max enum value 31), param **10 defined** (max 14) — earlier text used the max enum value as the count.
- **Binary count + prefixes fixed** (Current state): **9 binaries built** (8 distinct sources — `sweep`+`sweep_coeffgen` share `sweep_cli.c`); storage prefixes are under `polypaint/` (`polypaint/coeff-programs/` etc.).
- **Part 8 grammar goal corrected** — see §9.3.
- **Minor citation drift (logged, not load-bearing, not chased in-line):** `program_compile_helpers.py:26`→27 (the `scalar_exprs` key), `parseCtChain :5640`→5641, param by-name `:5575`→5578, the `:3931` stale-bytes bug line is `:3932-3934`, `ParamCx :6097`→6099. Workspace size: §3.1 cites the source comment ("~274 KiB", `sweep_cli.c:3552`) while §7.7 cites the computed 273.2 KB — both correct, just different sources. "8 fingerprint families" is 7 distinct names (8 only if coeff+param `sha1` are counted separately). None of these change a conclusion. Everything else in the doc's explicit citation set (token caps, degree probe/assert, warm-start/serpentine, the parsers' missing version field, the palette collision-raise, ASL forwarding, MAXDEG misalignment, the 4 native tests, registry counts/schemas, `spec_version`/WASM absence) **re-verified CONFIRMED exact**.

## 9.2 New fail-closed surface: a 7th solve-score validator (add to the Phase 0 inventory)

The "≥6 handlers" enumeration (§4.1 #3) missed **`handler_palette_chunk.py`** — the GenerateFromPalette / bins-reuse path. It has *two* relevant raises: `:417` ("Bins fingerprint mismatch", a `chain_fingerprint` hash compare) **and** `:421` ("Bins program mismatch", a `program_spec` **string** compare). The string compare is a **second wire surface** the hash-only blast-radius view misses: a v2 canonical-`program_spec` change breaks `:421` even if the hash were dual-read. Both sites must be in the Phase 0 dual-read inventory. (Folded into §4.1 #3.)

## 9.3 Text-grammar verdict (per DSL) — corrects "text-first for all four"

| DSL | Text grammar difficulty | Verdict |
|---|---|---|
| **coeff** | full statement language + RD expression parser + RPN lowering (shipped, `coeff_program_source.py`) | the bar — done |
| **param** | a genuine *subset* of coeff's parser; its `ExpressionParser` already exists; net-new = `emit/push` register keywords + transcribing `legacy(...)` validation already in the chip compiler | **(a) clean — text-first** |
| **root transforms** | flat `name(scalars)` one-per-line; ~30-line parser; no value language at all | **(a) trivially clean — text-first** |
| **solve-score** | declarative metric prepass + slot-CSE + cross-step lag (`mN-1`) + multi-channel emit; a readable text form needs an *invented named-slot binding sub-language the VM lacks*, then erased back to positional slots to preserve the fingerprint; lag refs cryptic under implicit slots | **(c) text is *worse* than chips → keep chip-primary, text as read-only export** |

So Part 8's "text-first for all four" was wrong: do it for coeff/param/root; **leave solve-score chip-primary.** This also right-sizes §4.3's "three substantial parser chunks" — root is a day, param is lexer-reuse, and the coeff-class effort (solve-score) is the one we should *not* undertake.

## 9.4 Operational blast radius (unexamined until now)

- **Packaging fan-out.** The new parser modules (a `param_program_source.py`, plus a `root` source helper — and *not* a solve-score one, per §9.3) must be `cp`'d into the **same 5 Lambda bundles that carry `coeff_program_source.py` today** — coeffgen, storage, compute_preview, param_debug, compute_plan (`deploy.sh:964,985,1034,1058,1328`). A missing `cp` is a runtime `ImportError`, **caught at predeploy** by `tests/test_deploy_packaging.py`'s recursive AST import walk — *but only for static imports*; a dynamic/string import would slip through to `postdeploy_init_check.sh`.
- **api_manifest is a 4-source cross-check.** A new `migrate`/`translate`/non-overwriting-save route requires *synchronized* edits in **handler_storage.py dispatch + deploy_manifest.json routes + the frontend `lambdaPost` call + a regenerated `api_manifest.json`** (predeploy `--check` is exact-equality), or a contract test fails. **Payload-shape changes are invisible to api_manifest** — they're pinned only by the payload-contract pytests, so the migrate/translate handlers need *new* contract tests added to the predeploy gate.
- **Docker ARM64 gate covers token-IR, not the parsers.** The gate exercises the binaries' token execution (so opcode/stack-typing changes are covered) but the **new text parsers run Python-side and are never exercised by the Docker gate** — they need Python unit tests. If the unified VM *renames* a binary, the gate's hardcoded binary list (`test-docker-runtime.sh:14`) and `docker_runtime_regression.py`'s `__main__` need manual extension.
- **IAM is a non-issue.** The shared storage role's S3 grant is **bucket-wide** (`deploy.sh:1715-1728`), so new routes reading+writing the program prefixes work without policy changes.

## 9.5 Downstream / ecosystem consumers — safe by construction (except §9.2)

The frontend holds **zero** fingerprints (every reuse/dedup decision is server-side). Favorites, book generation (`make_polypaint_book.py`), DeepZoom, "snaps", and recolor-from-raw all address artifacts by **random run-id / timestamp slugs** (`color_{run_id}`, `dz_{timestamp}`) and treat any embedded fingerprint as **opaque passthrough** — so a v2 fingerprint change cannot dangle them. The results catalog surfaces fingerprints it never compares (display passthrough). And the **animation / morph / sonification execution surfaces** named in older project memory **do not exist** in the current tree (removed with the WASM/worker era) — there is no per-frame or client-side program execution. The only downstream surprise is the §9.2 validator.

## 9.6 Net of the second dive

The refactor's direction (coeff VM as reference; migration is the cost) and Part 7's three gating prerequisites (version gate → verification harness → token budget) stand unchanged. The second dive (a) removed the doc's one real self-contradiction, (b) fixed two wrong and several imprecise citations, (c) corrected the text-first goal to per-DSL (solve-score stays chip-primary), (d) added a 7th fingerprint validator + a `program_spec`-string wire surface to the Phase 0 inventory, and (e) added the operational layer — packaging fan-out, the 4-source manifest sync, and the Docker-gate parser gap (IAM is fine). No new *gating* prerequisite emerged.

*No production code was modified by this review.*
