# Code Review 19 - Root And Solve-Score Programs In The Unified Poly Language

## Scope

This review answers the architectural question: if the project goal is a unified program system, why are `root_transforms` and solve-score still chip chains, and what must change to fold them into the same source/profile/IR model as Param and Coeff?

Reviewed files:

- `lambda/program_profiles.json`
- `lambda/program_v2_translate.py`
- `lambda/root_legacy_registry.json`
- `lambda/root_xforms.h`
- `lambda/solve_score_chain.py`
- `lambda/solve_score.h`
- `lambda/handler_storage.py`
- root/score payload call sites in `lambda/handler_render_plan.py`, `lambda/handler_palette_render_plan.py`, `lambda/handler_raster_mt.py`, `lambda/handler_palette_chunk.py`, `lambda/handler_solve_proximity.py`, `lambda/handler_render_lores_preview.py`, `lambda/handler_bilevel.py`, and `lambda/handler_palette_debug.py`
- UI state in `js/02-preview-solvescore.js`, `js/03-program-modals.js`, `js/07-transform-catalogs.js`, `js/09-render-orchestration.js`
- CR18 design notes in `cr18-implementation-plan.md` and `code-review-18.md`

## Verdict

The user is right. The current implementation is not fully unified. It is approximately:

- Param: source-first program.
- Coeff: source-first program.
- Solve-score: saved chip-chain program with v2 translation and native score opcodes, but no authoritative source language.
- Root: embedded transform chain, not a saved program kind, with partial v2 translation only.

That is not "all pipelines are programs." It is a pragmatic CR18 scope cut. It should now be treated as technical debt.

The correct target is four profile-backed program kinds:

- `param_program`
- `coeff_program`
- `root_program`
- `solve_score_program`

All four should use the shared source-parser core where text exists, the profile symbol table for name legality, v2 execution specs for stable identity, and read-only chip rendering as a visualization. Chips can remain a UI affordance, but they must stop being the authoritative source for root and solve-score.

## Current State

### Root

Root already has partial unified-program scaffolding:

- `program_profiles.json` declares a `root` profile with `program_kind: "root_program"`, symbol `roots`, output contract `in_place_vector`, and `nan_policy: "poison"`.
- `root_legacy_registry.json` assigns stable `fn_index` values to the 9 root transforms.
- `program_v2_translate.translate_root_from_old()` translates old root transform arrays into v2 `native_transform` tokens with `registry: "root"`.
- `root_xforms.h` centralizes native root transform execution and preserves NaN poisoning through `rt_mark_undefined`.

But root is not actually a first-class program yet:

- There is no `root_program_source.py`.
- There are no storage routes: `/list-root-programs`, `/fetch-root-program`, `/save-root-program`, `/delete-root-program`, `/migrate-root-program`.
- The UI stores root transforms in global arrays (`_rtChain`, `_paletteRtChain`) and serializes them directly as `root_transforms`.
- Render/palette/proximity/bilevel/debug handlers pass `root_transforms` through to many binaries as `--root_xforms=<json file>`.
- Native binaries still parse root transforms from the legacy JSON file shape through `parse_root_xform_file`.
- Root transforms are embedded in render/palette artifacts; there is no reusable saved root program object.

This is the bigger inconsistency. Root is already simple enough to become source/profile-backed; keeping it as an embedded chip chain is mostly inertia. Standalone saved root-program CRUD is useful, but it is a second product step, not the first architectural gate.

### Solve-Score

Solve-score is more program-like than root, but still not source-first:

- `handler_storage.py` already has solve-score saved-program routes.
- `solve_score_chain.py` compiles a chain into normalized metrics, `program_tokens`, output channels, fingerprint, and `program_spec`.
- `program_v2_translate.translate_solve_score_from_old()` translates compiled score programs into v2 opcodes in the 64-95 range.
- `solve_score.h` has a real native score VM with numeric op ids such as `SOLVE_SCORE_OP_PUSH_METRIC`, `SOLVE_SCORE_OP_AVG`, `SOLVE_SCORE_OP_EMIT_NORM`, etc.
- `solve_score_eval.py` exists as a Python oracle.

But solve-score still has two non-unified seams:

- The saved object and UI authoring source is still the chip chain.
- The native wire is still a semicolon `program_spec` string like `v2;m0-0;omega_cosine:6;emit_norm`, parsed by `parse_solve_score_program_spec`.

The compile layer is strong enough to reuse. The missing part is an authoritative source parser and a migration of runtime payloads from "chain/spec string first" toward "source -> execution_spec first."

### Code Deep-Dive Findings

These are the implementation facts that matter for folding Root and Solve-Score into the same architecture as Param/Coeff.

1. `program_source_core.py` is already the right shared parser boundary, but it is intentionally small. It owns statement splitting, assignment detection, call parsing, source spans, diagnostics, and profile lookup. It does **not** own expression semantics, locals, metric CSE, root dispatch, or VM lowering. Root and Solve-Score should add profile-specific source modules that use this core, not a second parser core.

2. `pipeline_programs.py` currently covers only Param/Coeff source-vs-chain precedence and legacy transform-to-program translation. Root and Solve-Score must be added there. Do not add a parallel resolver module; render/palette/proximity/bilevel/debug already have enough duplicated score/root handling.

3. `handler_storage.py` already gives Solve-Score saved-program CRUD, but `_compile_solve_score_program_payload()` accepts only `chain`. Param/Coeff storage already use the correct source-wins pattern with `*_source_text_from_payload()`, compile routes, canonical source display, and derived fields ignored from the client. Solve-Score should copy that pattern exactly.

4. Root has no storage routes, but Phase 6A should not start there. The code currently treats root transforms as embedded render/palette/proximity/bilevel artifact state. The lowest-risk fix is to add embedded `root_program_source_text` and compiled root metadata to those payloads first, then decide whether standalone root CRUD is worth the product surface.

5. Root v2 translation already exists: `translate_root_from_old()` emits `MERGED_OP_NATIVE_TRANSFORM` with `registry: "root"`. Root source should lower to that same canonical root chain/token shape. It should not invent root-specific opcodes in v1.

6. `root_xforms.h` has both `parse_root_xform_json()` and `parse_root_xform_file()`. The runtime binaries call the file path today; the file parser delegates to the JSON parser. Short-term source support should continue emitting the same `--root_xforms` file for native compatibility.

7. Root default args are a hidden identity trap. `root_xforms.h` applies defaults when args are omitted, and `root_legacy_registry.json` carries matching defaults, but current v2 translation preserves the provided arg count. CR19 must canonicalize root rows by expanding omitted args to registry defaults before fingerprinting so `pull_unit_circle()` and `pull_unit_circle(0.75, 1.0)` are the same program.

8. Solve-Score v2 translation already exists and is stronger than the source layer needs: it emits `reduce_metric`, `push_metric`, and score op tokens from the compiled chain. Source should lower to the existing chain first, then reuse `compile_solve_score_chain()` and the existing v2 translator.

9. Solve-Score source locals are the new semantic surface. `compile_solve_score_chain()` is RPN stack-only; it has no local variables. CR19 must pin local semantics before implementation. The safe v1 rule is single-assignment local names with expression inlining during source-to-chain lowering, leaving metric CSE to `compile_solve_score_chain()`.

10. Solve-Score old-chain migration cannot be pure expression syntax if exact `program_spec` parity is required. Existing chains can contain `dup` and `flush`; those tokens appear in `program_spec`. Source synthesis must either preserve them as explicit compatibility statements or accept identity churn. CR19 chooses explicit compatibility statements.

11. There is a real cap drift: `solve_score_chain.py` and `solve_score.h` both enforce `MAX_PROGRAM_TOKENS` / `SOLVE_SCORE_MAX_PROGRAM_TOKENS = 32`, but `program_profiles.json` declares `solve_score.value_caps.program_tokens = 256`. Fix that profile drift before using profile caps to validate solve-score source.

12. Render and palette ASL templates still forward `solve_score_chain` and `root_transforms`. That is fine for the short-term native wire, but plan outputs and artifact metadata must also preserve `solve_score_program_source_text`, `root_program_source_text`, compiled fingerprints, and spec-version siblings so populate can restore editable text.

13. JS launch/populate paths are still chip-state-first: `_rtChain`, `_paletteRtChain`, `_renderScoreChain`, and `_paletteScoreChain` are the source of request payloads. Text mode must add source state variables and use the same rule as Param/Coeff: non-blank source text wins over chip rows; old artifacts synthesize canonical source from rows.

14. Deployment packaging currently copies `solve_score_chain.py` broadly but no future `root_program_source.py` / `solve_score_program_source.py`. Every Lambda that parses source must package the new modules plus `program_source_core.py`, `program_profiles.py/json`, `merged_opcodes.py/json` where needed.

15. `program_v2_translate.py` is not yet source-aware for these two profiles. `translate_root_from_old()` currently returns `program_kind: "root_transforms"`, and `translate_solve_score_from_old()` returns no `source_text`. CR19 must update both translators so migrated v2 objects are editable source-backed programs.

16. Root row shape is already split in practice. Native `root_xforms.h` accepts both legacy array rows (`["rotate_roots","0.25"]`) and object rows (`{"name":"rotate_roots","fn_index":1,"args":[0.25]}`), but JS populate/edit paths currently understand only array rows. Source-backed root programs may use default-expanded object rows internally, but any compatibility `root_transforms` field sent to current JS/native paths must either stay array-form or every parser/populate path must be upgraded to accept both. Do not silently switch artifact `root_transforms` to object-only rows.

17. Current artifact editability is chain-only. `raw_sidecar.py`, `color_artifact_meta.py`, render metadata, palette metadata, and JS populate restore `solve_score_chain`, `score_program`, and `root_transforms`, not source text. This is exactly the class of bug already hit by Coeff populate returning `_typed_*` internal rows. Phase 6 must treat source text as first-class artifact metadata, not as an optional storage-only field.

18. `program_source_core.parse_call()` lowercases call names, and `ProfileStatementLowerer` lowercases writable LHS symbols. Root/Solve-Score source should explicitly adopt lowercase canonical identifiers. This is not a user-facing restriction on typing case; it is the canonicalization rule. Serializers emit lowercase names, and conflict checks compare lowercase names.

19. V2 execution-spec profile strings are already identity-bearing. `translate_root_from_old()` uses `_execution_spec_v2("root", ...)`; `translate_solve_score_from_old()` uses `_execution_spec_v2("solve-score", ...)`. New `program_kind` values may become `root_program` and `solve_score_program`, but the execution-spec kind strings must stay `"root"` and `"solve-score"` unless a deliberate spec-version bump and cache invalidation are intended.

20. Current solve-score migration tests explicitly assert that migrated solve-score programs have no `source_text`. That is not a harmless missing test; it is a pinned old contract. Phase 6B must invert that test in the same commit that changes the migration response schema.

21. `root_program_source.py` must not rely on native default behavior for canonical identity. Native omission defaults are runtime behavior; source fingerprints and migrated V2 payloads need default-expanded args before hashing, before source regeneration, and before artifact metadata is written.

22. Root transforms are consumed outside the color/palette/proximity happy path. Grep shows `root_xforms.h` wired into `roots2pix_mt.c`, `solve_palette_chunk*.c`, `solve_palette_debug.c`, `solve_proximity_hist_sectioned.c`, `solve_proximity_stats.c`, and `bilevel_section_raster.c`; Python handlers include render plan/lores/raster, palette plan/chunk/debug/finalize, solve proximity, and bilevel. Phase 6A must either route all user-facing root-transform entrypoints through `root_program_for_run()` or explicitly mark a path as legacy-only. Do not leave bilevel as an unplanned exception.

## Architectural Target

The unified poly language should be one parser framework with profile-specific symbols and semantic hooks, not four unrelated parsers.

The right abstraction boundary:

- `program_source_core.py` owns statement splitting, assignment/call parsing, diagnostics, source spans, and profile symbol lookup.
- Profile modules own semantic lowering:
  - `param_program_source.py`
  - `coeff_program_source.py`
  - new `root_program_source.py`
  - new `solve_score_program_source.py`
- `program_profiles.json` declares symbols, output contracts, caps, and legal selector/source namespaces.
- `program_v2_translate.py` becomes compatibility migration only, not the primary source compiler.
- Native execution consumes versioned execution specs or a compact derived wire format generated from execution specs, never browser chip rows.

Important: "one language" does not mean every profile exposes the same symbols. It means one source model:

- assignment
- calls
- expressions where relevant
- profile-driven identifier legality
- profile-specific output contract
- canonical serializer
- structured diagnostics

Root and solve-score can be profile-specific without being chip-chain exceptions.

## Root Program Design

### Source Shape

Root should use simple source syntax. The root stage has one mutable vector: `roots`.

Canonical v1 syntax:

```text
roots = rotate_roots(roots, 0.25)
roots = pull_unit_circle(roots, 0.75, 1.0)
roots = moebius(roots, 1, 0, 0, 1)
```

Allow a mutation shorthand because it matches the current chip workflow:

```text
rotate_roots(0.25)
pull_unit_circle(0.75, 1.0)
moebius(1, 0, 0, 1)
```

The shorthand lowers to `roots = name(roots, args...)`.

`moebius` is intentionally registry-namespaced here. There is also a Param transform named `moebius`, with a different `fn_index` and different arity. Root source must resolve it through the root registry, not through the Param registry. This collision is exactly why the v2 `native_transform` token needs a `registry` discriminator.

Do not make root a coeff-vector program internally. Root transforms run at raster time over float root arrays, not over coeff VM double complex-vector registers. The shared part is the parser/profile/IR system, not the physical workspace.

### Args

Current root args are real literals. Pin v1 to finite real static args only. This matches the existing `root_transforms` payload, the native parser, and all root-consuming binaries.

Later versions may allow static real expressions over constants (`pi`, arithmetic), folded at compile time, but that is not required to make root source-backed. Dynamic expressions over p1/p2/t1/t2 are out of scope until every root-consuming runtime has a well-defined row/sample context.

Do not add p1/p2/t1/t2 access casually. Root transforms execute after solving and are reused in render/palette/proximity/bilevel contexts; those stages do not all carry the same source registers.

### Program Object

Phase 6A does **not** require root saved-program CRUD to ship. The minimum object that must exist is an embedded compiled root program carried by render/palette/proximity/bilevel payloads:

```json
{
  "program_kind": "root_program",
  "version": 2,
  "program_version": 2,
  "spec_version": 2,
  "id": "rotate-quarter",
  "name": "Rotate Quarter",
  "source_text": "roots = rotate_roots(roots, 0.25)\n",
  "chain": [{"name": "rotate_roots", "fn_index": 1, "args": [0.25]}],
  "tokens": [
    {"op": 29, "op_name": "MERGED_OP_NATIVE_TRANSFORM", "registry": "root", "fn_index": 1, "n_args": 1, "args": [0.25], "args_im": [0.0]}
  ],
  "execution_spec": "...",
  "fingerprint": "sha256:..."
}
```

The `chain` field is compatibility/display, not authority.

Standalone saved root programs use the same object shape, but they can ship as Phase 6A.2 after the source/profile boundary is stable.

### Storage Changes

Split storage into two decisions.

Required for Phase 6A:

- artifact/request payload support for `root_program_source_text`
- compiled embedded `root_program` metadata in render/palette plans and artifacts
- `root_program_fingerprint`
- `root_spec_version` sibling fields wherever root program fingerprints or root-program-derived digests are persisted
- `/compile-root-program-source` route for authoritative editor validation

Optional Phase 6A.2 saved-program CRUD:

- `ROOT_PROGRAMS_PREFIX = "polypaint/root-programs/"`
- `_root_program_key(id)`
- `_root_program_v2_key(id)`
- `_compile_root_program_payload(name, chain=None, source_text=None, ...)`
- `_read_root_program_object(id)`
- metadata helpers matching Param/Coeff/Solve-Score
- routes:
  - `/list-root-programs`
  - `/fetch-root-program`
  - `/save-root-program`
  - `/delete-root-program`
  - `/migrate-root-program`

Current CR18 explicitly avoided root as a saved-program kind because root lived embedded in render/palette artifacts. Reverse the embedded-chain authority first; add reusable saved root programs only if the UI needs named/reusable root pipelines.

### Runtime/Payload Changes

Today many handlers take `root_transforms` and write `--root_xforms=<json file>`. This must become a boundary translation:

- request may still include legacy `root_transforms`
- request may include `root_program_source_text`
- request may include `root_program` / `root_program_id`
- plan compiles to `root_program.execution_spec` and canonical root chain for legacy native consumers

Short-term native compatibility:

- Keep writing `--root_xforms` from the compiled root program's canonical chain.
- Do not make every C binary parse full v2 execution specs in the first step.

Long-term native cleanup:

- Replace `--root_xforms` with `--root_program=<execution_spec or compact file>`.
- `root_xforms.h` should parse a versioned root program file with fn_index tokens, not user-facing names.
- `parse_root_xform_file` becomes a legacy decode path only.

### Native Requirements

Root native semantics that must not change:

- NaN poisoning for undefined roots (`rt_mark_undefined`).
- In-place transform order.
- Float root arrays, not double coeff vectors.
- Current unknown-name migration behavior: old unknown transform names can be dropped with a diagnostic during migration, but new source should reject unknown root calls.

### Root Tests

Required tests:

- `test_root_program_source.py`: parse, canonical source, diagnostics, assignment and shorthand lower to same execution spec.
- embedded payload tests: `root_program_source_text` wins over legacy `root_transforms`, and chain-only artifacts synthesize canonical root source.
- optional Phase 6A.2 `test_root_program_storage.py`: save/fetch/list/delete/migrate.
- `test_program_v2_migration.py`: root migration keeps fn_index and unknown names become diagnostics.
- Native parity: root program source -> canonical root chain -> existing `root_xforms.h` output matches old root transforms.
- NaN propagation: `invert_roots` on zero and Cayley poles still poison roots.
- Plan identity: root program fingerprint replaces raw `root_transforms` in render/palette/proximity cache keys, and direct bilevel/debug paths carry the same fingerprint in artifact/status metadata where they persist root state.
- dual-read identity: missing `root_spec_version` reads as v1; current writes include `root_spec_version`.
- Workflow tests: ASL forwards compiled root program fields, not raw editable rows.

## Solve-Score Program Design

### Source Shape

Solve-score needs source text, but it should not copy Coeff syntax blindly. Its natural model is scalar score expressions plus metric reduction leaves. The canonical form is **named-variable expression source**. Do not add a second stack-statement source language.

Canonical v1 syntax:

```text
p = metric(proximity, slv, q=0.1%)
s = metric(spread, cf, q=0.5%)
combined = avg(p, s)
emit_norm(combined)
```

For lag:

```text
now = metric(proximity, slv, q=0.1%)
prev = metric(proximity, slv, q=0.1%, lag=1)
emit_norm(abs_diff(now, prev))
```

For multi-channel:

```text
r = metric(spread, slv, q=0.1%)
g = metric(crowding, slv, q=0.1%)
b = metric(anisotropy, slv, q=0.1%)
emit_norm(r)
emit_norm(g)
emit_norm(b)
```

For simple scalar LUT:

```text
score = omega_cosine(metric(proximity, slv, q=0.1%), 6)
```

No bare trailing expression in v1. The normal authoring form is named-variable/expression source.

Compatibility exception: existing solve-score chains can contain stack-only tokens such as `dup` and `flush`, and those tokens are preserved in `program_spec`. To synthesize editable source from old chains without changing identity, the parser must accept explicit compatibility statements `dup()` and `flush()`. The canonical serializer should avoid them when an expression form can preserve the same `program_spec`, but it may emit them when exact old-chain round-trip requires them.

### Grammar Delta

Solve-score source requires two grammar features not currently used by Param/Coeff source:

- keyword args, at least for `metric(..., q=..., lag=...)`
- percent literals for quantiles, e.g. `0.1%`
- call-tree expressions for solve-score values; v1 does not need general infix arithmetic because existing solve-score operations are named calls such as `add(a,b)`, `mul(a,b)`, `ratio(a,b)`, and `pow(x,e)`

This is not free. Phase 6B must explicitly extend `program_source_core.py` to parse keyword args and percent literals, or solve-score must use a positional-only syntax. The recommended choice is to extend the shared core, because `metric(proximity, slv, q=0.1%, lag=1)` is substantially more readable than `metric(proximity, slv, 0.001, 1)`, and readability is the reason to add solve-score source text.

The grammar extension must be profile-safe:

- Param/Coeff do not automatically accept keyword args unless their profile hooks opt in.
- Percent literals are parsed as real scalar literals divided by 100 only in contexts that request `percent`.
- Diagnostics identify unsupported keyword args and duplicate args by line/column.

### Local Variable Contract

Solve-score locals are source-level aliases only in v1. They do not add native local slots and they do not change `compile_solve_score_chain()`.

Pin the rules:

- Local identifiers are single-assignment within one source program.
- Use before assignment is a compile error.
- Reassigning a local is a compile error.
- `score` is reserved and cannot be read as a normal local.
- `metric`, `emit`, `emit_norm`, `emit_none`, metric names, metric sources, and operation names are reserved.
- A local reference inlines the local expression into the RPN chain lowering.
- The same metric expression used through two locals still deduplicates by the existing metric-slot CSE in `compile_solve_score_chain()`.
- Cycles are impossible under single-assignment/use-after-definition rules; no fixed-point evaluation is needed.
- Source locals are not required to preserve old stack op tokens. If old-chain source synthesis needs exact `dup` or `flush` tokens, it uses the explicit compatibility statements instead of inventing local-variable semantics.

Example lowering:

```text
p = metric(proximity, slv, q=0.1%)
q = metric(proximity, slv, q=0.1%)
emit_norm(avg(p, q))
```

lowers to the existing chain shape:

```json
[["proximity", "slv", "0.1"], ["proximity", "slv", "0.1"], ["avg"], ["emit_norm"]]
```

Then `compile_solve_score_chain()` collapses the duplicate metric into one metric slot, exactly as it does for chip chains today.

### Why This Is Better Than Chips

The old objection was that solve-score has metric-slot CSE, lag, and multi-channel output. That is true, but it argues for a compiler, not for permanent chip authority.

Named source variables solve the readability problem:

- current chain: `metric(proximity,slv,q=0.1) metric(proximity,slv-1,q=0.1) abs_diff emit_norm`
- source: `prev = metric(proximity, slv, q=0.1%, lag=1); emit_norm(abs_diff(now, prev))`

The compiler can still erase names into positional metric slots to preserve the current efficient native model.

### Output Contract

Solve-score source must map to the existing two-mode CR18 output contract.

**`score` is a reserved output identifier**, not an ordinary intermediate variable. It names the implicit output channel — the compiler already emits a channel literally named `score` (`solve_score_chain.py:1009`). Use other names (`p`, `s`, `combined`, `r`/`g`/`b`) for intermediates; assigning `score` is what selects implicit-scalar mode and is mutually exclusive with `emit*`.

Implicit scalar mode:

- exactly one assignment to the reserved `score` output (intermediate variables such as `p`/`s` may also be assigned)
- no `emit`, `emit_norm`, or `emit_none`
- lowers to the current no-explicit-output program: terminal stack depth 1, native output op defaults to `emit`

Explicit channel mode:

- one or more `emit(expr)`, `emit_norm(expr)`, or `emit_none(expr)` statements
- no required `score = ...`
- lowers to the current explicit-output program: every emit consumes one expression, final stack depth 0, output channel count equals the non-`none` emits

Invalid mixed mode:

- `score = ...` plus explicit emits is rejected unless a later version defines it.
- bare expression statements are rejected in v1.
- channel-count validation remains tied to `color_interpretation`.

### Semantic Lowering

Do not rewrite solve-score semantics from scratch. Reuse `compile_solve_score_chain` as the first semantic backend:

1. Parse source to a high-level AST.
2. Lower expressions to the existing solve-score chain shape.
3. Run `compile_solve_score_chain`.
4. Generate:
   - canonical chain
   - `program_tokens`
   - metric slots
   - output channels
   - v2 execution spec
   - fingerprint
   - current `program_spec` compatibility string

This keeps metric CSE, source validation, lag rules, stack validation, and output-channel logic in one place initially.

The equivalence gate must assert both identity surfaces:

- source-generated fingerprint equals the equivalent chain-generated fingerprint
- source-generated canonical `program_spec` string equals the equivalent chain-generated canonical `program_spec`

The second assertion is mandatory because several current validators compare `program_spec` strings, not just hashes.

Later, invert the dependency:

- source lowers directly to typed solve-score IR
- chain compiler becomes legacy compatibility

### Source Profile

Expand the `solve_score` profile in `program_profiles.json`.

Current profile only has a synthetic `metrics` namespace. It needs explicit source-language symbols:

- scalar variables: local read/write identifiers, probably limited to real score values
- metric source enums: `slv`, `cf`, `pm`
- output functions: `emit`, `emit_norm`, `emit_none`
- metric call: compiler special form, not an ordinary arithmetic function
- allowed unary/binary functions from `solve_score_chain.py`: `avg`, `min`, `max`, `mul`, `mult`, `add`, `subtract`, `ratio`, `ema`, `weighted_sum`, `abs_diff`, `geometric_mean`, `omega_cosine`, `sawtooth`, `flip`, `clamp`, `sin`, `cos`, `log`, `exp`, `pow`

The profile must also declare:

- max metric slots = 16
- max program tokens = 32 today, matching `solve_score_chain.py` and `solve_score.h`; fix the current `program_profiles.json` drift before using the profile as source metadata
- max output channels = 8
- lag depths = `{0, 1}`
- source capability matrix per metric (`_METRIC_ALLOWED_SOURCES`)
- arg schema for the `metric` special form: two positional args (`metric_name`, `source`), then optional `q=` (percent literal) and `lag=` (`{0, 1}`) keyword args

The metric source capability matrix currently lives only in Python code. It should be data or generated metadata so JS/source/compiler/native cannot drift.

### Program Object

Add `source_text` to saved solve-score programs.

New canonical object:

```json
{
  "program_kind": "solve_score_program",
  "version": 2,
  "program_version": 2,
  "spec_version": 2,
  "id": "rgb-shape",
  "name": "RGB Shape",
  "source_text": "r = metric(spread, slv, q=0.1%)\nemit_norm(r)\n",
  "chain": [["spread", "slv", "0.1"], ["emit_norm"]],
  "metrics": [...],
  "program_tokens": [...],
  "tokens": [...],
  "program_spec": "v2;m0-0;emit_norm",
  "execution_spec": "...",
  "fingerprint": "sha256:...",
  "output_channel_count": 1,
  "output_channels": [...]
}
```

The key rule should match Param/Coeff:

- `source_text` wins on save/compute.
- `chain` is compatibility/display.
- fetch may synthesize source from chain for old saved programs.

### Runtime/Wire Changes

There are two levels.

Short-term:

- Source compiles to existing chain.
- Existing `solve_score_program_cli_payload` still emits `score_metrics`, `score_sources`, `score_clip_los`, `score_clip_his`, and `score_program`.
- Native `parse_solve_score_program_spec` remains the runtime parser.
- All render/palette/proximity/bilevel/debug paths get source support without native changes.

Long-term:

- Native should accept v2 `execution_spec` or compact numeric token payload.
- `score_program` semicolon string becomes derived compatibility, not the authoritative runtime wire.
- The 5 native consumers should all share the same score-program loader:
  - `roots2pix_mt.c`
  - `solve_palette_chunk.c`
  - `solve_palette_chunk_mt.c`
  - `solve_proximity_stats.c`
  - `solve_proximity_hist_sectioned.c`
  - plus debug/single-thread helpers where present

Do not inline metric reduction as ordinary scalar op execution per channel. Metric reduction is a prepass with CSE and lag handling. The unified program model should represent this explicitly:

- `reduce_metric` declarations / metric slots
- score-expression tokens that push those slots
- output emit ops

### Handler Changes

Every handler currently compiling `solve_score_chain` needs a shared resolver:

```python
solve_score_program_for_run(params, scope="solve")
```

It should apply precedence:

1. `solve_score_program_source_text`
2. `solve_score_program` / `solve_score_program_id`
3. legacy `solve_score_chain`
4. legacy scalar fields where still accepted

Call sites to update include:

- `handler_render_plan.py`
- `handler_palette_render_plan.py`
- `handler_raster_mt.py`
- `handler_palette_chunk.py`
- `handler_palette_finalize.py`
- `handler_solve_proximity.py`
- `handler_render_lores_preview.py`
- `handler_finalize_mt.py`
- `color_recolor_raw.py`
- `handler_extract_palette_from_step_scores.py`

Avoid another copy-paste resolver. CR14 already showed this duplication class is dangerous.

This resolver should live in `lambda/pipeline_programs.py` next to the existing param/coeff request-path precedence helpers. Do not create a parallel `program_resolvers.py` or `pipeline_score_programs.py`; that would recreate the exact resolver-drift class this change is supposed to remove.

### UI Changes

Render and palette currently use `_renderScoreChain` and `_paletteScoreChain`. The UI needs a source tab just like Param/Coeff:

- Solve Score Text tab in Render.
- Solve Score Text tab in Palette.
- Saved solve-score modal shows source text first.
- Chips are read-only visualization of compiled source.
- Populate from artifact must restore `solve_score_program_source_text` if present, else synthesize from chain.
- Multi-channel output mode must read the compiled program output contract, not infer from stale chip rows.

The JS parser should remain advisory only. The backend parser is authoritative.

### Solve-Score Tests

Required tests:

- `test_solve_score_program_source.py`: parse source, canonical source, diagnostics.
- Chain parity: source -> chain -> compiled fingerprint equals equivalent chip chain.
- Metric CSE parity: two source vars referencing the same metric/source/q share one slot.
- Lag parity: `lag=1` source equals current `slv-1` chain.
- Explicit output parity: three `emit_norm(...)` outputs produce same `output_channels`.
- Storage: save/fetch/list/delete with source precedence.
- Populate: old chain-only artifacts synthesize source.
- Render plan and palette plan: source text forwarded and compiled once.
- Native parity through `solve_score_eval.py`: source program and old chain program produce identical scores.
- `program_spec` compatibility: source-generated compiled program still matches old validators until native wire is migrated.

## Unified Poly Language Shape

The broad language can be one grammar with profile hooks:

```text
statement :=
    assignment
  | call_statement
  | expression_statement

assignment :=
    symbol "=" expression
  | symbol "[" expression "]" "=" expression

call_statement :=
    ident "(" args? ")"

expression :=
    literals
  | identifiers
  | function calls
  | indexing
  | infix arithmetic
```

Profile differences:

- Param allows `p1`, `p2` assignment and scalar complex expressions.
- Coeff allows `poly`, `poly[i]`, vector/scalar operations, and fixed vector output.
- Root allows `roots` mutation and root-native transform calls over float vectors.
- Solve-score allows local real score variables, `metric(...)` leaves, lag, and emits.

This gives the user one mental model:

- named inputs
- named output
- assignments
- calls
- source text is saved
- chips visualize the compiled program

It does not require one physical runtime workspace.

## Concrete File Map

### New Files

- `lambda/root_program_source.py`
- `lambda/solve_score_program_source.py`
- `tests/test_root_program_source.py`
- optional Phase 6A.2: `tests/test_root_program_storage.py`
- `tests/test_solve_score_program_source.py`
- `tests/test_solve_score_program_source_native_parity.py`

### Modify

- `lambda/program_profiles.json`: expand root and solve-score profile metadata for source language.
- `lambda/gen_program_profiles.py`: regenerate mirrors.
- `lambda/gen_coeff_vocab.py`: expose root/score program source metadata to JS catalog entries consumed by the editor.
- `lambda/pipeline_programs.py`: add root and solve-score source-vs-chain resolvers; this is the single request-path precedence module.
- `lambda/program_v2_translate.py`: keep old chain migration but add source serializers for root/score.
- `lambda/handler_storage.py`: add solve-score `source_text` save/fetch precedence; add root saved-program routes only in optional Phase 6A.2.
- `lambda/solve_score_chain.py`: keep as the semantic chain compiler; export/reuse its op tables and constants from `solve_score_program_source.py` rather than moving source parsing into this file.
- `lambda/root_xforms.h`: eventually parse versioned root program tokens, not just old JSON names.
- `lambda/solve_score.h`: eventually parse versioned numeric score program tokens, not semicolon strings.
- render/palette/proximity/bilevel/debug handlers: replace direct `root_transforms` and `solve_score_chain` reads with shared compiled program resolvers where those program families are accepted.
- `workflow_contracts.py` and ASL templates: forward source/program fields where needed.
- `js/02-preview-solvescore.js`: add source text state and populate behavior.
- `js/03-program-modals.js`: source-first solve-score modal; new root-program modal or unified program modal only if Phase 6A.2 saved root CRUD ships.
- `js/07-transform-catalogs.js` and `js/09-render-orchestration.js`: root/score chips become visualization and compatibility input, not authority.
- `api_manifest.py` / `deploy_manifest.json`: source compile routes and optional Phase 6A.2 root CRUD routes if added.
- `deploy.sh`: package new source modules into storage/render/palette/proximity/bilevel/debug bundles where source is parsed.

## Migration Order

### Phase A - Root Source/Profile First

Root is the easiest and gives immediate architectural consistency.

1. Add `root_program_source.py`.
2. Add source serializer from legacy root transforms.
3. Add render/palette/proximity/bilevel/debug payload resolver in `pipeline_programs.py`: `root_program_source_text` wins, else embedded `root_program`, else legacy `root_transforms`.
4. Store embedded compiled root program metadata, `root_program_fingerprint`, and `root_spec_version` on render/palette/proximity/bilevel artifacts.
5. Continue emitting `--root_xforms` from the compiled root program for native compatibility.
6. Update UI populate behavior so artifacts restore root source text if present, else synthesize it from legacy rows.
7. Add native parity, dual-read, and cache identity tests.

This should be low to medium risk.

### Phase A.2 - Optional Saved Root CRUD

Standalone saved root programs are a product feature, not a prerequisite for source-backed root pipelines. Add `/list-root-programs`, `/fetch-root-program`, `/save-root-program`, `/delete-root-program`, and `/migrate-root-program` only after embedded source/profile support is stable or if actual reuse demand appears.

### Phase B - Solve-Score Source Layer

1. Add `solve_score_program_source.py`.
2. Define metric expression syntax with named variables and emit statements.
3. Lower source to current solve-score chain and reuse `compile_solve_score_chain`.
4. Save/fetch source_text in solve-score saved programs.
5. Add source fields to render/palette plans and artifact metadata.
6. Add text tab in render/palette score editors.
7. Add populate-source restoration.
8. Keep `score_program` semicolon wire for now.

This is medium risk because solve-score touches render, palette, proximity, raw-sidecar, and program-spec validators.

### Phase C - Native Wire Cleanup

1. Teach native root consumers to read versioned root tokens.
2. Teach native solve-score consumers to read versioned score tokens or a shared execution-spec payload.
3. Keep old wire decoders only as migration compatibility.
4. Move cache/fingerprint identity to execution specs, not semicolon strings or raw transform arrays.

This is high risk and should be gated by the existing Python solve-score oracle and whole-sweep byte oracle.

## Main Risks

### Risk 1 - Solve-score metric prepass accidentally becomes per-output work

Do not lower `metric(...)` into ordinary scalar runtime calls inside each output expression. Metrics are expensive reductions and must remain slot-CSE'd.

### Risk 2 - Root programs get incorrectly run in the coeff VM

Root transforms are float, raster-stage, in-place, NaN-poisoning operations. They are not coeff vector ops. Unification should happen at source/profile/IR boundaries, not by forcing root into the coeff workspace.

### Risk 3 - Another duplicated resolver layer

Do not add independent source-vs-chain precedence logic to every render/palette/proximity/bilevel/debug handler. Add one resolver per program family and test it.

### Risk 4 - Old artifacts lose editable source on populate

We just fixed this class for Param/Coeff. Root/Solve-score must get the same rule from the start: if an artifact has only chain rows, synthesize canonical source before displaying/editing.

### Risk 5 - Fingerprint churn without dual-read

Root transforms feed render plan digests, scratch keys, palette identities, and artifact metadata. Solve-score feeds both chain fingerprints and program-spec string compares. Any new source/fingerprint scheme needs dual-read and explicit spec-version siblings, not silent hash changes.

## CR18 Provenance And CR19 Supersession

Do not rewrite CR18 as if it was wrong at the time. CR18 is shipped history and its chip-primary root/solve-score stance was a deliberate scope cut. CR19 supersedes that stance as the forward Phase 6 plan.

The forward record should say:

- CR18 temporarily left root and solve-score chip-primary to control scope.
- The target architecture is four first-class program profiles.
- Root source/profile migration is Phase 6A.
- Standalone saved root CRUD is optional Phase 6A.2.
- Solve-score source migration is Phase 6B.
- Native wire cleanup is Phase 6C.
- If CR18 is edited at all, add a pointer to this CR19 plan rather than rewriting old decisions in place.

## Bottom Line

The current root/solve-score state made sense as a short-term CR18 scope cut, but it is not the final architecture.

Root should become a source/profile-backed embedded `root_program` next. It is small, already has a registry, and the profile exists. Standalone saved root-program CRUD can follow only if reuse demand justifies it.

Solve-score should then become source-backed using named score variables and `metric(...)` calls, lowering initially to the existing chain compiler. It is more work, but the existing compiler already contains the hard semantic rules, so the source layer is feasible.

Once those two land, the project finally has the intended shape: four profiles, one source/diagnostic architecture, versioned execution specs, and chips as visualization rather than authority.

## Implementation Section - Exact Steps

This section is the implementation ticket and the **authoritative** specification — the "Root Program Design" and "Solve-Score Program Design" sections above are rationale/background, and **on any conflict these Steps win**. Do them in order; later steps depend on earlier source/identity contracts.

**Step → Phase map.** Steps 0–11 deliver **Phase 6A** (root source/profile) + **Phase 6B** (solve-score source), both on the short-term native wire. **Phase 6C (native-wire cleanup) has no Step here** — it is a separate later ticket gated by the solve-score oracle and the whole-sweep byte oracle.

| Step | Phase |
|---|---|
| 0 Fix shared metadata drift | prerequisite (both) |
| 1 Extend `program_source_core.py` | 6A + 6B foundation |
| 2 Root source module | 6A |
| 3 Root resolver + embedded payload | 6A |
| 4 Solve-score source module | 6B |
| 5 Update v2 translation | 6A + 6B |
| 6 Solve-score storage | 6B |
| 7 Solve-score resolver | 6B |
| 8 Workflow + artifact metadata | 6A + 6B |
| 9 UI source tabs + populate | 6A + 6B |
| 10 Deployment packaging | 6A + 6B |
| 11 Required test gates | all |

Root (6A) is shippable through Steps 0–3 plus the root slices of 5/8/9/10; solve-score (6B) adds Steps 4/6/7 plus the score slices.

**6B completion rule.** Solve-score source migration is not allowed to be "best effort" for valid programs. The existing solve-score chain language is finite and fully expressible in the source grammar once `dup()`, `flush()`, and `emit_none()` are included. Therefore every chain that currently compiles with `compile_solve_score_chain_or_legacy()` must round-trip through source. A round-trip failure on a valid chain is an implementation bug, not an accepted limitation. Only malformed/corrupt historical payloads that already fail the current chain compiler may remain legacy/unmigrated.

**6B implementation order.** Do solve-score in this exact order so failures are localized:

1. Step 0/1 foundation: fix profile caps, add kwargs/percent parsing, and add source-core tests.
2. Step 4 forward compiler: implement `solve_score_program_source.py` source -> chain -> existing compiler, with unit tests for every operation class in the operation coverage table.
3. Step 4 reverse serializer: implement total `solve_score_source_text_from_chain()` over valid canonical chains, including `dup()`, `flush()`, and `emit_none()` compatibility, with round-trip assertions in tests.
4. Step 6 storage: add source-aware save/fetch/list/compile route. Chain-only valid saved programs must synthesize source on fetch.
5. Step 7 pipeline resolver: route render/palette/lores/proximity/chunk/finalize/extract paths through `solve_score_program_for_run()`.
6. Step 8/9 metadata and UI: persist source text on artifacts/raw sidecars and make populate/text tabs prefer source.
7. Step 5 migration: update V2 translation only after the reverse serializer and storage fetch path pass the real-corpus gate.
8. Step 10/11 packaging and gates: package modules, update manifests, run corpus/native/UI/deploy gates.

There are no remaining solve-score design choices after this section: if an implementation discovers a valid canonical chain that cannot be represented, the source grammar or serializer is incomplete and must be extended in the same phase.

### Step 0 - Fix Shared Metadata Drift

1. Update `lambda/program_profiles.json`:
   - Set `profiles.solve_score.value_caps.program_tokens` to `32`.
   - Add `max_metric_slots: 16` or equivalent explicit cap under `solve_score.value_caps`; do not rely only on the `metrics.namespace.count` field.
   - Add solve-score source metadata: local scalar namespace, reserved output symbol `score`, metric sources `slv/cf/pm`, output calls `emit/emit_norm/emit_none`, and keyword schema for `metric(metric_name, source, q=percent, lag=0|1)`.
   - Add root source metadata: root transform registry name, writable `roots`, and finite real static args.
2. Run `lambda/gen_program_profiles.py` after editing the JSON.
3. Update `tests/test_program_profiles_drift.py`:
   - Assert solve-score profile `program_tokens == solve_score_chain.MAX_PROGRAM_TOKENS`.
   - Assert solve-score profile metric slot cap equals `solve_score_chain.MAX_METRIC_SLOTS`.
   - Assert solve-score profile output channel cap equals `solve_score_chain.MAX_OUTPUT_CHANNELS`.
   - Assert root profile references `root_legacy_registry.json` and the registry names match `root_xforms.h`. (Root's `value_caps.program_tokens`/stack fields are **nominal** — root is not a token/stack VM — so do not assert them against a native constant; for root only the registry-name match matters.)

### Step 1 - Extend `program_source_core.py`

1. Add a shared call-argument parser:
   - `parse_call_args(args)` or equivalent returning positional args and keyword args.
   - Preserve line/column enough for duplicate keyword and unsupported keyword diagnostics.
   - Reject positional args after keyword args.
2. Add a percent-literal helper:
   - `parse_percent_literal("0.1%") -> 0.1` as percent units for solve-score chain compatibility, not fraction units.
   - Only profile hooks decide where percent literals are accepted.
3. Keep existing Param/Coeff behavior unchanged:
   - `parse_call()` can continue returning raw arg strings for old callers.
   - New helpers must be opt-in from Root/Solve-Score modules.
4. Pin identifier canonicalization:
   - Function names are canonical lowercase, matching current `parse_call()`.
   - Writable profile symbols are canonical lowercase, matching `ProfileStatementLowerer`.
   - Root transform names, solve-score chip names, metric names, metric sources, and source locals compare lowercase.
   - Serializers emit lowercase names.
   - Local variables that differ only by case are a duplicate-name diagnostic.
5. Add unit tests in a new or existing source-core test:
   - top-level split with `metric(..., q=0.1%)`
   - duplicate keyword diagnostic
   - positional-after-keyword diagnostic
   - percent literal rejected by profiles that do not opt in
   - case-normalized call names and local-name collision diagnostics

### Step 2 - Add Root Source Module

Create `lambda/root_program_source.py`.

Required API:

- `RootProgramSourceError`
- `RootProgramSourceCompileError`
- `parse_root_program_source(source_text, strict=True)`
- `compile_root_program_source(source_text, strict=True)`
- `root_source_text_from_payload(payload)`
- `root_source_text_from_chain(chain)`
- `display_root_program_chain(chain)`
- `serialize_root_program_chain(chain)`
- `compile_root_program_chain(chain, strict=True)`
- `root_transforms_from_program_chain(chain)`
- `root_program_chain_from_transforms(root_transforms)`

Lowering rules:

- `roots = rotate_roots(roots, 0.25)` lowers to one root transform row.
- `rotate_roots(0.25)` is accepted shorthand and lowers to the same row.
- Assignment RHS must be a root transform call whose first argument is `roots`; arbitrary root-vector expressions are not v1.
- Shorthand calls omit the `roots` vector argument and mutate roots in place.
- Only `roots` is writable.
- `roots = roots` is a no-op and should be rejected as useless unless a later source feature needs it.
- Root args are finite real static values in v1.
- Each root transform accepts between 0 and its registry arity args; omitted args are filled from `root_legacy_registry.json` defaults during canonicalization.
- Too many args are a source error.
- Unknown root transform names are errors in source mode.
- Old migration may still drop unknown root transform names with warnings.
- `moebius` resolves through `root_legacy_registry.json`, not through Param.
- Transform names are canonical lowercase.

Canonical chain shape is default-expanded:

```json
[{"name": "rotate_roots", "fn_index": 1, "args": [0.25]}]
```

For example, `pull_unit_circle()` and `pull_unit_circle(0.75, 1.0)` both canonicalize to:

```json
[{"name": "pull_unit_circle", "fn_index": 2, "args": [0.75, 1.0]}]
```

Canonical source regeneration:

- `root_source_text_from_chain([["rotate_roots","0.25"]])` emits `rotate_roots(0.25)`.
- Object-form root rows emit the registry name plus args.
- Unknown old rows emit a diagnostic in migration paths; source serializer should not invent source for unknown transforms.

Compilation:

- Use `MERGED_OP_NATIVE_TRANSFORM` with `registry: "root"` and stable `fn_index`.
- Include `args_im` as zeros for shape consistency.
- Fingerprint over the canonical root execution spec, not raw text.
- Default-expanded source, default-expanded legacy rows, and already-explicit rows must have the same fingerprint.
- Return both canonical program chain and compatibility `root_transforms` rows.
- The canonical program chain may be object-form. The compatibility `root_transforms` rows must remain accepted by current native/JS paths. In v1, prefer legacy array rows for `root_transforms` unless every consumer listed in Step 3 is updated to accept object rows.

Storage/API route:

- Add `handle_compile_root_program_source()` to `lambda/handler_storage.py`.
- Add `/compile-root-program-source` to the storage router and deployment manifest.
- Response shape matches Param/Coeff compile routes: `{ok, chain, display, statement_count, fingerprint, diagnostics, program}`.
- This route is required even if saved root CRUD is deferred.

### Step 3 - Add Root Resolver And Embedded Payload Support

Modify `lambda/pipeline_programs.py`.

Add:

```python
root_program_for_run(params, *, strict=True)
root_source_text_for_run(params)
root_transforms_to_program_chain(root_transforms)
```

Precedence:

1. non-blank `root_program_source_text`
2. embedded `root_program.source_text`
3. embedded `root_program.chain`
4. legacy `root_transforms`

Return a normalized object with:

- `source_text`
- `chain`
- `tokens`
- `execution_spec`
- `fingerprint`
- `spec_version`
- `root_transforms` compatibility rows for `--root_xforms`
- `diagnostics`

Modify root consumer handlers:

- `lambda/handler_render_plan.py`
- `lambda/handler_palette_render_plan.py`
- `lambda/handler_render_lores_preview.py`
- `lambda/handler_solve_proximity.py`
- `lambda/handler_bilevel.py`
- `lambda/handler_palette_debug.py`
- `lambda/handler_storage.py` for `/compile-root-program-source`
- any helper path that currently reads `params["root_transforms"]`

Exact identity helpers to update:

- `handler_render_plan._plan_params_digest()`
- `handler_render_plan._solve_score_scratch_key()`
- `handler_palette_render_plan._solve_score_scratch_keys()`
- `handler_palette_render_plan._scratch_matches()`
- `handler_palette_render_plan._palette_identity_payload()`
- `color_artifact_meta.parse_root_transforms()`
- `handler_storage._parse_root_transforms()`
- JS `_setRenderRootTransforms()` and `_setPaletteRootTransformsFromArtifact()`

Rules:

- Plan-time compilation happens once.
- Downstream tasks still receive `root_transforms` compatibility rows until Phase C.
- Direct API handlers that do not have a separate plan stage still call `root_program_for_run()` before writing `--root_xforms`.
- Plan outputs/artifact metadata also include `root_program`, `root_program_source_text`, `root_program_fingerprint`, and `root_spec_version`.
- Cache keys that currently hash raw `root_transforms` hash `root_program_fingerprint` instead, with a dual-read fallback for old artifacts missing `root_spec_version`.
- Scratch reuse checks compare root fingerprints first; only old metadata without `root_spec_version` falls back to comparing parsed raw `root_transforms`.
- Artifact metadata keeps `root_transforms` for old consumers and adds `root_program_source_text` / `root_program` for editability.
- Populate prefers `root_program_source_text`. If missing, it synthesizes source from `root_program.chain` or legacy `root_transforms`.
- Any parser/populate helper that receives `root_transforms` must accept both array rows and object rows, because native already accepts both and migration will produce object-form program chains.

Tests:

- Compile route returns `{ok, chain, display, fingerprint, diagnostics, program}` for root source.
- Source wins over legacy `root_transforms`.
- Embedded `root_program.chain` wins over legacy `root_transforms`.
- Bilevel and palette-debug root source requests produce the same native `--root_xforms` rows as old `root_transforms`.
- Missing `root_spec_version` reads as v1.
- Same legacy rows and equivalent source produce the same root fingerprint.
- Omitted default args and explicit default args produce the same root fingerprint.
- Unknown source transform is an error; unknown legacy migration row is a warning.
- Object-form root rows and array-form root rows both populate editable source.
- Artifact `root_transforms` remains consumable by existing native `parse_root_xform_file()` until Phase C.

### Step 4 - Add Solve-Score Source Module

Create `lambda/solve_score_program_source.py`.

Required API:

- `SolveScoreProgramSourceError`
- `SolveScoreProgramSourceCompileError`
- `parse_solve_score_program_source(source_text, strict=True)`
- `compile_solve_score_program_source(source_text, strict=True)`
- `solve_score_source_text_from_payload(payload)`
- `solve_score_source_text_from_chain(chain)`

Grammar:

- Assignment: `name = expression`
- Reserved implicit output assignment: `score = expression`
- Explicit emits: `emit(expr)`, `emit_norm(expr)`, `emit_none(expr)`
- Metric leaf: `metric(metric_name, source, q=0.1%, lag=0|1)`. This is the canonical emitted source form for every metric. Metric-specific chain rows such as `["proximity", "slv", "0.1"]` serialize back to `metric(proximity, slv, q=0.1%)`, not `proximity(...)`.
- Function calls: current non-metric solve-score op set from `solve_score_chain.py`; metric names appear only as the first argument of `metric(...)` in source v1.
- Expressions are call-tree expressions: metric calls, numeric literals, `const(value)`, local names, and named solve-score functions. **Infix arithmetic is rejected with a diagnostic** in the solve-score profile — the shared core may parse `a+b` for Param/Coeff, but the solve-score profile must reject it (call-tree only), never silently lower it.
- No bare expression statements in v1.
- Compatibility statements `dup()` and `flush()` are valid only to preserve old stack-chain identity; generated source may use them when needed. **These make the parser stack-aware, not purely expression-based**: it must track stack depth so a `dup()`/`flush()` against an invalid stack state is a diagnostic. Implement this as a small stack-depth check alongside the expression lowering — it is the only non-expression state the solve-score parser carries.

Operation coverage is total over the current `solve_score_chain.py` language:

- Metric leaves:
  - Generic public metric chip: `["metric", metric_name, source, q_percent]`.
  - Metric-specific rows: any `name in VALID_SOLVE_SCORE_METRICS` with canonical params `[source_or_source-1, q_percent]`.
  - Canonical source always emits `metric(metric_name, source, q=q_percent%, lag=0|1)`.
- Stack leaves/statements:
  - `const(value)` -> `["const", value]`
  - `dup()` -> `["dup"]`
  - `flush()` -> `["flush"]`
- Unary calls:
  - `omega_cosine(x, omega[, phase])`
  - `sawtooth(x, mult)`
  - `flip(x)`, `clamp(x)`, `sin(x)`, `cos(x)`, `log(x)`, `exp(x)`
  - `pow(x, exponent)`
- Binary calls:
  - `avg(a,b)`, `min(a,b)`, `max(a,b)`, `mul(a,b)`, `add(a,b)`, `mult(a,b)`, `subtract(a,b)`, `ratio(a,b)`, `abs_diff(a,b)`, `geometric_mean(a,b)`
  - `ema(a,b,alpha)`
  - `weighted_sum(a,b,wa,wb)`
- Output calls:
  - `emit(expr)`, `emit_norm(expr)`, `emit_none(expr)`

If `solve_score_chain.py` gains a new chip, Phase 6B is not complete until this table, the parser, the reverse serializer, and the corpus gate are updated.

Local semantics:

- Single-assignment source locals.
- Use-before-definition error.
- Reassignment error.
- `score` is reserved output, not a readable local.
- Local references inline into RPN lowering.
- Local names are canonical lowercase; `a = ...` and `A = ...` collide.
- Local names may not collide with solve-score chip names, output calls, metric names, metric sources, or reserved words (`score`, `emit`, `emit_norm`, `emit_none`, `metric`, `const`, `dup`, `flush`).

Expression-to-chain lowering:

- `metric(proximity, slv, q=0.1%)` -> `["proximity", "slv", "0.1"]`
- `metric(proximity, slv, q=0.1%, lag=1)` -> `["proximity", "slv-1", "0.1"]`
- `avg(a, b)` lowers by emitting chain for `a`, chain for `b`, then `["avg"]`.
- Numeric literal leaves lower to `["const", value]`.
- `const(value)` lowers to `["const", value]`.
- Unary calls emit expression chain then the unary chip.
- Parameterized unary calls:
  - `omega_cosine(x, omega)` -> expr chain, `["omega_cosine", omega]`
  - `omega_cosine(x, omega, phase)` -> expr chain, `["omega_cosine", omega, phase]`
  - `sawtooth(x, mult)` -> expr chain, `["sawtooth", mult]`
  - `pow(x, exponent)` -> expr chain, `["pow", exponent]`
- Binary parameterized calls:
  - `ema(a, b, alpha)` -> `a`, `b`, `["ema", alpha]`
  - `weighted_sum(a, b, wa, wb)` -> `a`, `b`, `["weighted_sum", wa, wb]`
- Constants use existing `["const", value]`.
- `dup()` lowers to `["dup"]`.
- `flush()` lowers to `["flush"]`.
- Explicit emit consumes the expression chain and appends `["emit_norm"]`, `["emit"]`, or `["emit_none"]`.
- Implicit scalar mode appends no emit; the chain must end with stack depth 1 after lowering `score = expr`.
- `emit_none(expr)` is an explicit output statement that consumes its expression but contributes zero color channels. It must preserve the old `program_spec` behavior for chains that used `emit_none` as a sink.

Canonical source regeneration:

`solve_score_source_text_from_chain()` is the hard part of Phase 6B and must be implemented with an explicit reverse-lowering algorithm, not a best-effort display renderer.

Algorithm:

1. Compile/canonicalize the input chain first with `compile_solve_score_chain_or_legacy()` and `serialize_solve_score_chain()`. The serializer works from canonical public rows, not raw user rows.
2. Walk the canonical chain once with a symbolic stack. Each stack item is an expression node containing:
   - `source`: source text for the expression.
   - `chain`: canonical chain rows that reproduce it.
   - `program_spec`: optional rendered sub-spec fragment for diagnostics/debug.
3. Metric rows create leaf nodes:
   - `["metric", "proximity", "slv", "0.1"]` -> `metric(proximity, slv, q=0.1%)`
   - `["proximity", "slv", "0.1"]` -> `metric(proximity, slv, q=0.1%)`
   - `["proximity", "slv-1", "0.1"]` -> `metric(proximity, slv, q=0.1%, lag=1)`
   - `source-1` maps to `lag=1`; any other source suffix is invalid because the current compiler rejects it.
4. `["const", value]` creates `const(value)` unless a bare numeric literal is required for exact round-trip readability. The serializer may emit either form only if reparsing produces the identical canonical chain and `program_spec`.
5. Pure unary/binary/parameterized chips pop expression nodes and produce call-tree nodes using the same function names as Step 4 lowering: `avg(a,b)`, `abs(x)`, `omega_cosine(x,omega)`, `ema(a,b,alpha)`, etc.
6. `["dup"]` duplicates the top symbolic stack item and records that the generated source must include an explicit `dup()` compatibility statement unless the final reparse can remove it without changing `program_spec`.
7. `["flush"]` clears the symbolic stack and records an explicit `flush()` compatibility statement.
8. `["emit"]`, `["emit_norm"]`, and `["emit_none"]` pop one expression node and emit the corresponding source statement. `emit_none(expr)` emits a statement but contributes zero output channels.
9. At end of chain:
   - If there were explicit emits, final symbolic stack depth must be zero after any required compatibility statements.
   - If there were no emits, final stack depth must be one and source emits `score = <expr>`.
10. Synthetic local names are optional. Use them only to keep generated source readable when an expression node would otherwise be duplicated or very long. Generated locals use deterministic lowercase names (`v0`, `v1`, ...), are single-assignment, and are emitted before first use.
11. After generating source, reparse it with `compile_solve_score_program_source()` and assert all of the following match the canonical input:
   - `serialize_solve_score_chain(chain)`
   - `compiled_solve_score_fingerprint`
   - rendered `program_spec`
   - `output_channel_count`
   - `output_channels`
12. If the generated source cannot satisfy the exact round-trip assertions for a chain that compiles today, the implementation must raise a test failure. For malformed/corrupt historical payloads that already fail the current compiler, the serializer fails closed with a structured diagnostic (`source_roundtrip_failed`) and returns no authoritative `source_text`. Storage/fetch/populate may then display the old chip chain, but migration must not claim the corrupt payload is source-backed.

Rules:

- The serializer must never emit bare trailing expressions.
- The serializer must never emit lowered/internal token names.
- The serializer may emit `dup()` or `flush()` compatibility statements only when required for exact old-chain identity.
- Exact `program_spec` parity is a release gate, not a nice-to-have.
- For every valid canonical chain, `solve_score_source_text_from_chain()` is total. There is no expected-fail list for valid chains.

Compilation:

- Source lowers to chain.
- Run `compile_solve_score_chain()` / `compile_solve_score_chain_or_legacy()`.
- Return source text, source display, statement count, canonical chain, compiled metrics, output channels, fingerprint, and `program_spec`.

### Step 5 - Update V2 Translation

Before storage/handler rollout, update v2 migration so old programs become source-backed.

Modify `lambda/program_v2_translate.py`:

- `translate_root_from_old()` calls the root canonicalizer/compiler.
- It returns `program_kind: "root_program"`, not `"root_transforms"`.
- It includes default-expanded `chain`, `source_text`, `source_display`, `tokens`, `execution_spec`, `fingerprint`, diagnostics, and counts.
- It keeps `_execution_spec_v2("root", ...)` unless the V2 spec version is deliberately bumped.
- It keeps warning diagnostics for unknown old root transform rows.
- `translate_solve_score_from_old()` calls `solve_score_source_text_from_chain()` after compiling/canonicalizing the old chain.
- It includes `source_text`, `source_display`, `chain`, `program_spec`, `tokens`, `execution_spec`, `fingerprint`, output contract fields, and counts.
- It keeps `_execution_spec_v2("solve-score", ...)` unless the V2 spec version is deliberately bumped.
- Root and solve-score v2 fingerprints must be computed from canonical compiled payloads, not from raw old rows.

Tests:

- Root migration emits `program_kind: "root_program"`.
- Root migration default-expands omitted args before fingerprinting.
- Solve-score migration now includes reparseable `source_text`.
- Existing `test_migrate_solve_score_program_dry_run_has_no_source_text` is inverted/renamed to require source text. **This deliberately changes the CR18 §4.4 per-kind migrate-response contract** — solve-score previously returned `program_spec` with *no* `source_text`; update the migrate-route response schema and its payload-contract test together so the change reads as intended rather than as a regression.
- Migrated source reparses to the same canonical serialized/public chain, fingerprint, and `program_spec`.
- Execution-spec kind strings remain `"root"` and `"solve-score"` for equivalent old/new migrated programs.
- Every valid solve-score v1/v2 chain accepted by `compile_solve_score_chain_or_legacy()` migrates with `source_text`. Migration may omit `source_text` only for payloads that fail the existing compiler, and those responses must carry diagnostics explaining that the old payload is invalid/corrupt.

### Step 6 - Update Solve-Score Storage

Modify `lambda/handler_storage.py`.

Changes:

- `_compile_solve_score_program_payload()` accepts `source_text=None`.
- If `source_text` is present and authoritative, parse source and replace `chain` with parsed chain.
- Store `source_text`, `source_display`, `source_statement_count`, `fingerprint`, `output_channel_count`, and `output_channels`.
- Keep `chain` as compatibility/display.
- Ignore client-supplied derived fields (`program_spec`, `fingerprint`, `metrics`, `output_channels`) exactly as current storage ignores other derived fields.
- Add `/compile-solve-score-program-source`.
- Fetch old chain-only saved programs with synthesized `source_text` from `solve_score_source_text_from_chain()`.
- List summaries include `has_source_text` and output contract metadata.

Tests:

- Save with source text round-trips source and canonical chain.
- Source wins over conflicting chain.
- Blank source with non-empty chain falls back to chain.
- Compile route returns `{ok, chain, display, fingerprint, diagnostics, program}`.
- Chain-only saved program fetch synthesizes reparseable source.
- `program_spec` string parity with equivalent old chain.
- If a chain-only saved program is valid under the current compiler, fetch/list must synthesize source and set `has_source_text`.
- If a chain-only saved program is invalid/corrupt under the current compiler, fetch/list surfaces it as legacy/chip-only with diagnostics and does not set `has_source_text`.

### Step 7 - Add Solve-Score Resolver To `pipeline_programs.py`

Add:

```python
solve_score_program_for_run(params, *, scope="solve", strict=True)
solve_score_source_text_for_run(params, *, scope="solve")
```

Precedence:

1. non-blank `solve_score_program_source_text`
2. saved/embedded `solve_score_program.source_text`
3. saved/embedded `solve_score_program.chain`
4. legacy `solve_score_chain`
5. legacy scalar fields: `metric`, `solve_score_quantile`, `solve_score_omega`, `solve_score_omega_enabled`

Return a compiled object matching `compile_solve_score_chain_or_legacy()` plus:

- `source_text`
- `source_display`
- `chain_public`
- `fingerprint`
- `spec_version`
- `program_spec`
- `output_channel_count`
- `output_channels`
- `has_explicit_outputs`

Modify all direct compile call sites:

- `handler_render_plan.py`
- `handler_palette_render_plan.py`
- `handler_render_lores_preview.py`
- `handler_raster_mt.py`
- `handler_palette_chunk.py`
- `handler_palette_finalize.py`
- `handler_solve_proximity.py`
- `handler_extract_palette_from_step_scores.py`

Rules:

- Plan handlers compile from source and forward canonical chain plus source metadata.
- Chunk/finalize handlers primarily consume plan-provided canonical chain/program fields; they should not re-resolve UI source unless they are direct API entrypoints.
- Keep `score_program` semicolon wire until Phase C.
- `program_spec` compatibility string must be byte-identical for source and equivalent chain.
- Missing `solve_score_spec_version` reads as legacy/current default exactly as today; source support must not break old artifacts.

Tests:

- Render plan source-vs-chain precedence.
- Palette plan source-vs-chain precedence.
- Lores preview source support.
- Proximity source support.
- Raster/chunk/finalize still reject mismatched `score_program` as before.
- Source and old chain produce identical native scores through `solve_score_eval.py`.

### Step 8 - Workflow And Artifact Metadata

Modify:

- `lambda/workflow_contracts.py`
- `stepfunctions/render_workflow.asl.json.template`
- `stepfunctions/palette_workflow.asl.json.template`
- plan/artifact metadata writers in render/palette/proximity/bilevel/debug handlers
- bilevel and palette-debug metadata writers
- `lambda/raw_sidecar.py`
- `lambda/color_artifact_meta.py`
- `lambda/handler_extract_palette_from_step_scores.py`
- attach/repalette/recolor handlers that copy score/root metadata

Add fields:

- `root_program_source_text`
- `root_program`
- `root_program_fingerprint`
- `root_spec_version`
- `solve_score_program_source_text`
- `solve_score_program`
- `solve_score_source_display`

Keep fields:

- `root_transforms`
- `solve_score_chain`
- `score_program`
- `solve_score_spec_version`

Rules:

- Source fields are metadata for editability and identity; native tasks still use compatibility rows/spec strings.
- Artifact populate must prefer stored source text.
- Old artifacts without source text synthesize source from rows.
- Cache identity uses fingerprints/spec versions with missing-version fallback for old artifacts.
- Raw sidecars add source fields without removing `score_chain` or `score_program`.
- Color artifact overlays add source fields without removing `root_transforms`, `solve_score_chain`, or `score_program`.
- Metadata copy helpers (`attach`, `repalette`, `recolor`, associated-palette inheritance) must preserve source fields together with the existing compatibility fields.
- `score_program` remains the semicolon native wire and validation string; `solve_score_program_source_text` is editability metadata.
- `root_transforms` remains the native wire; `root_program_source_text` is editability metadata.

Tests:

- ASL definition tests assert new fields are forwarded where needed.
- Artifact metadata tests assert source text survives render/palette/proximity/bilevel flows.
- Populate tests assert text restoration from new artifacts and source synthesis from old chain-only artifacts.
- Raw sidecar validation round-trips `solve_score_program_source_text` while preserving `score_program`.
- Color artifact inventory exposes root and solve-score source fields without breaking old artifacts.

### Step 9 - UI Source Tabs And Populate

Modify JS:

- `js/02-preview-solvescore.js`
- `js/03-program-modals.js`
- `js/07-transform-catalogs.js`
- `js/08-chip-editors.js`
- `js/09-render-orchestration.js`
- `js/10-status-results.js`
- `js/11-artifacts.js`

Root UI:

- Add source state for render root and palette root.
- Add Text/Chips tabset for root transforms where root chains are edited.
- Non-blank root source wins when launching render/palette/proximity/bilevel/debug.
- Chips become visualization/compatibility input.
- Populate restores `root_program_source_text` if present, else synthesizes from `root_transforms`.
- Root chip population must accept both legacy array rows and canonical object rows.
- Text edits must not leave stale chip rows authoritative; launch payload includes source text whenever the text tab has non-blank source.

Solve-score UI:

- Add source state for render solve-score and palette solve-score.
- Add Text/Chips tabset in Render and Palette score editors.
- Non-blank solve-score source wins when launching preview/render/palette/proximity/debug.
- Saved solve-score modal shows source first and saves source text.
- Populate restores `solve_score_program_source_text` if present, else synthesizes from `solve_score_chain`.
- Multi-channel mode uses compiled output contract from backend, not stale chip count.
- Text-mode source is what saved-program modals save. Chip rows are compatibility visualization/input only when source text is blank.
- Populate must never display lowered/internal implementation rows (`_typed_*`, v2 op rows, or raw token dictionaries) as source text.

Backend authority:

- JS may tokenize/highlight, but `/compile-*-program-source` routes are authoritative.
- UI must not save a source program if backend compile route returns `ok: false`.
- Add root/solve-score compile-source calls in the same style as `js/08-chip-editors.js` uses for Param/Coeff today.

### Step 10 - Deployment Packaging

Update `deploy.sh`, `deploy_manifest.json`, and `tests/test_deploy_packaging.py`.

Add storage routes to `deploy_manifest.json`:

- `/compile-root-program-source`
- `/compile-solve-score-program-source`

Package `root_program_source.py` into zips that parse or resolve root source:

- storage for `/compile-root-program-source`
- render plan
- palette render plan
- render lores preview
- solve proximity
- bilevel
- palette debug if that Lambda remains a direct root-transform entrypoint

Package `solve_score_program_source.py` into zips that parse or resolve score source:

- storage
- render plan
- palette render plan
- render lores preview
- raster/chunk/finalize only if they accept direct source entrypoints
- solve proximity
- extract palette from step scores if it synthesizes/stores source

Package shared dependencies wherever new source modules are packaged:

- `program_source_core.py`
- `program_profiles.py`
- `program_profiles.json`
- `gen_program_profiles.py` is not packaged, but its generated output must be committed before packaging.
- `merged_opcodes.py`
- `merged_opcodes.json`
- `root_legacy_registry.json`
- `solve_score_chain.py`

Add packaging tests for every new file in every affected zip.

### Step 11 - Required Test Gates

Minimum Python/unit gates:

- `tests/test_root_program_source.py`
- `tests/test_root_program_pipeline.py`
- `tests/test_solve_score_program_source.py`
- `tests/test_solve_score_program_storage.py`
- `tests/test_solve_score_program_pipeline.py`
- `tests/test_program_profiles_drift.py`
- `tests/test_program_v2_migration.py`
- render/palette/proximity/bilevel/debug plan/direct-handler tests
- bilevel and palette-debug root-source tests
- workflow definition tests
- deploy packaging tests
- `api_manifest.py --check` — the two new `/compile-{root,solve-score}-program-source` routes synced across the handler dispatch, `deploy_manifest.json`, and the frontend `lambdaPost`
- frontend grep/harness tests for source tabs and populate
- frontend populate tests that old artifacts synthesize readable source and new artifacts restore stored source
- root default-arg canonicalization tests (`pull_unit_circle()` equals explicit defaults)
- solve-score compatibility synthesis tests for old chains containing `dup` and `flush`
- solve-score compatibility synthesis tests for `emit_none`
- profile cap drift tests for solve-score token/metric/output caps
- root array-row and object-row compatibility tests through storage inventory and JS populate serializers
- V2 execution-spec kind stability tests for `"root"` and `"solve-score"`
- tests that no source serializer emits internal lowered row names as user-editable text
- solve-score real-corpus equivalence test. Build a fixture corpus from:
  - every saved solve-score program JSON currently in storage fixtures or exported from S3,
  - every render/palette/proximity artifact metadata fixture containing `solve_score_chain`,
  - representative associated-palette metadata and raw-sidecar metadata containing `score_chain`.
  For each corpus chain that compiles today, assert `chain -> source -> chain` preserves canonical serialized chain, fingerprint, `program_spec`, output contract fields, and native score parity. There is no expected-fail list for valid chains. Invalid/corrupt corpus payloads must be separated into a corrupt-fixture list and must already fail the current compiler before they are allowed to remain non-source-backed.

Minimum native/parity gates:

- root source -> root transforms -> `root_xforms.h` native output equals old rows
- solve-score source -> old chain -> `solve_score_eval.py` output equals old rows
- solve-score source and equivalent chain have identical `program_spec`
- solve-score source and equivalent chain have identical fingerprint
- multi-channel explicit emits preserve output channel count and interpretation
- lagged metrics from source preserve prelude requirements

Do not call Phase 6 done until populate from a newly rendered artifact restores editable root/score source text, not just internal lowered chip rows.
