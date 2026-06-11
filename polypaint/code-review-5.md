# Code Review 5 — Smell & Aesthetics

**Date:** 2026-06-10 (second pass same day; aesthetics pass executed 2026-06-11)
**Reviewed at:** working tree on `5c0abb0` + the full aesthetics pass
**Bar applied:** "elegant, easy to read for a human, easy to maintain,
self-explanatory, generally not annoying. No unexplained stuff, no dead ends."
**Scope:** `lambda/coeff_program_source.py`, `lambda/coeff_program_chain.py`,
the Coeff Program region of `lambda/sweep_cli.c`, the Coeff Program editor in
`index.html`, `deploy.sh`, `coeff-program-commands.md`, plus the test files
and handlers swept in the second pass.

**Method:** four parallel aesthetics reviewers (one per area) with every major
claim re-verified, a full second pass after the priority items, then a
dedicated execution pass that landed everything below marked ✅. Status
legend: **✅ fixed**, **◻ open** (deliberately left; ride along with the next
touch), **✗ withdrawn** (the finding was wrong — see J3).

---

## Verdict

The bones are good. The registry-driven transform architecture means a plain
new transform is a JSON edit with **zero Python changes**; the typed-stack
validator's diagnostics are excellent; the WHY-comment culture at dangerous
spots is exactly right; there are **no TODOs, no commented-out code, no dead
functions** anywhere in scope.

After the aesthetics pass, the two big deferred refactors are also done:
`CoeffEvalContext` collapsed the C VM's parameter caravans (evalCoeffProgram
is now an 84-line enum-ordered switch over per-op kernels, down from 323
lines), and deploy.sh's create/update branches share one 41-entry spec list
(verified byte-identical to both former lists before the old ones were
deleted). What remains open below is small, deliberate, and labeled.

---

## Cross-cutting themes

### T1. One concept, many names — ✅ fixed

- ✅ A GLOSSARY comment at `legacy_registry()` defines the three terms once:
  *legacy* = old chip form (opcode 9), *native transform* = same registry fn
  via opcode 29, *registry transform* = the umbrella. It also states the
  architecture fact: a plain transform needs only a registry JSON entry plus
  the C case.
- ✅ JS prefix unified: the six `_cp*` stragglers are now `_coeffProgram*`
  (harness pins updated); the feature greps with one prefix.
- ✅ One mode noun: "Chain pipeline selected · " replaced the
  "Legacy selected"/"Chain mode selected" split.
- ✅ `_canonicalCoeffProgramChipName()` replaced the three inline
  const→push_const renames.
- ✅ "lower" now means one thing: chain's `_lower_chip`/`_lower_chain` were
  renamed `_compile_chip`/`_compile_chain` (chips→tokens); "lower" remains
  source→chips only.

### T2. The `[tgt, src]` vs `[src, tgt]` wire-order flip — ✅ documented (structurally permanent)

The dual layout is wire-frozen and stays, but it is no longer implicit:
LAYOUT CONTRACT comment at the JS normalize seam (harness-pinned), named
locals at all three numeric swap sites, per-layout comments on the copy
path's `'poly','poly'` emits, and all three Python flip sites documented.
`_execution_spec`'s docstring now warns its output is fingerprint wire
format, which also covers the SET/AFFINE vs LEGACY print-order asymmetry —
cosmetic, and changeable only with a fingerprint migration.

### T3. "andy" — ✅ fixed

- ✅ JS: `_ctAndyParam` carries `kind: 'andy'`; `_isAndyParam()` everywhere;
  littlewood's clone derives from the base def; `_ctAndyIndex` is
  `findIndex(_isAndyParam)` with the fallback documented; the littlewood
  renderer uses the andy-index lookup instead of a hardcoded `input(3)`.
- ✅ Semantic gloss `out = (1-andy)*transformed + andy*input` now lives at
  `_compile_andy` (chain), at the C token field, and as a
  "Blending (andy)" section in the manual that the table rows reference.

### T4. Hand-mirrored tables and magic registry ids — ✅ fixed

- ✅ Source derives its alias/unary tables from chain's promoted publics
  (`VECTOR_BINARY_OPS`, `VECTOR_UNARY_OPS`, `SCALAR_UNARY_EXPR_OPS`,
  `canonical_unary_op_name`, …) — the four hand-mirrored copies are gone,
  and the former underscore-private imports are public with docstrings.
- ✅ `FN_LINEAR=14, FN_EXP=16, FN_ROUND=23, FN_POW=24` constants replaced the
  bare ints at every packing site in both Python files and are mirrored in
  the drift tests (which also pin all 28 registry ids).

### T5. Error/diagnostic voice — ✅ fixed

- ✅ Zero silent failure paths in the coeff VM (re-verified); the two bare
  `return 1;` lines are propagating returns whose callees print.
- ✅ One C log prefix: all 46 `"Coeff Program ..."` messages and the 4
  unprefixed ones now use `coeff_program ...` — Lambda logs grep with one
  pattern.
- ✅ Python message voice normalized ("X requires shape" forms); littlewood
  says value1/value2 in source errors, chain errors, *and* the JS chip
  labels/tooltips (the manual already used value1/value2).
- ✅ The chips/text authority rules now have a 4-line comment above the
  `_effective*` family (chain mode sends transforms, program mode sends
  programs; exactly one side non-empty, so the backend needs no mode flag).

### T6. Ad-hoc normalization — ✅ JS fixed / ◻ Python idiom kept

- ✅ JS: `_str()` (20 conversions), `_pluralize()` (11 template conversions),
  `_paramValueOrDefault()` at the three default-fallback sites,
  `_chainDisplayString()` for the two display dedupes; the andy-strip stanza
  in chain is one `_split_trailing_andy(spec, raw_args, *, arities)` used by
  all four packers.
- ◻ Python's `str(x or "").strip().lower()` idiom (~28 sites) was left as
  is: it is the house normalization spelling, each site is one short line,
  and a helper saves nothing a reader needs.

---

## Per-file highlights (beyond the themes)

### `lambda/sweep_cli.c` (coeff region) — ✅ the big one landed

- ✅ **`CoeffEvalContext`** (program + p1/p2/t1/t2 + cf + workspace, built
  once per row) replaced the 12-value caravan at all ~30 call sites; the
  seven 15–17-param functions now take `(ctx, …)`.
- ✅ **`evalCoeffProgram` is 84 lines** (was 323): 19 per-op static kernels
  (`coeffProgramConstOp`, `…LinspaceOp`, `…VectorBinaryDispatch`, …) under a
  `switch (tok->op)` in COEFF_OP enum order.
- ✅ `CoeffProgramWorkspace` has field docs (scratch = result/SWAP temp,
  original = andy snapshot *and* binary operand2, aux = roll temp);
  `coeffProgramApplyBinaryFn/UnaryFn` renamed (they serve untyped ops too);
  `coeff_stack_top_slot()` replaced the five modular incantations; dead
  `!ws ||` guards removed; EMIT's empty-stack no-op commented; fn14/fn16
  stack-packer twins merged; selector bounds use the enum
  (`(double)COEFF_SEL_CF..`); named `COEFF_PROGRAM_IMAG_TOL 1e-12` (9 sites)
  plus sentinel comments for −700, 350.0, 1e-9, and −1.0 poly_len.
- ✅ Four section banners: JSON parse layer / scalar expressions and arg
  plumbing / per-op kernels / evaluator.
- ◻ snake_case stack primitives vs camelCase elsewhere — left; renaming is
  churn without a readability win.
- All 40 WASM-robustness behaviors and the native parity suite stayed green;
  sweep_test, ARM sweep, and sweep_coeffgen rebuilt from the refactored
  source.

### `lambda/coeff_program_source.py` + `coeff_program_chain.py` — ✅ fixed

- ✅ `_compile_chip` is dispatch-table driven (`_ZERO_ARG_CHIP_OPS`,
  `_CHIP_COMPILERS`); `_typed_lower_scalar`'s 16-arm elif is ~22 lines of
  table dispatch over derived tables; only the genuinely irregular families
  remain explicit branches.
- ✅ `_execution_spec`: LINSPACE/RANGE twins merged; `sel()`/`sel_value()`
  closures replaced the 17 `_SELECTOR_NAMES.get(...)` repetitions; verified
  **byte-identical fingerprints and specs against HEAD** over 19
  representative programs before landing.
- ✅ `_result_payload()` writes the 19-key result dict once;
  `compile_coeff_program_chain` and the five public entry points have
  docstrings; `_lower_range` takes `target=` like its siblings;
  `_AFFINE_SHORTHANDS` collapsed the scale/shift twin blocks; the push-order
  surprise has its why-comment; "fill requires fill(value) or
  fill(length, value)" message shape.
- ◻ Mixed `_typed_lower_*` return shapes and the 5-tuple legacy-args returns
  — left; unifying them is churn in correct code.
- ◻ `compile_coeff_program_source` remains an unused public abstraction
  (tests use it; four handlers compose parse+compile inline,
  shape-consistently). Routing handlers through it touches four Lambdas for
  zero behavior change — do it when a handler is next edited for real.

### `index.html` (Coeff Program editor) — ✅ fixed

- ✅ Picker copy-paste parameterized: `_chipPickers = {pp, cp}` descriptors +
  four shared functions; public onclick names kept as thin wrappers because
  generated HTML references them.
- ✅ `updateChipParam` split into `_validateChipParamValue` (pure: value or
  error per param kind) + a 15-line apply step.
- ✅ `_coeffProgramSourceFromChain` → `_coeffProgramSourceFromRows` (it takes
  serialized rows); twin target-choices constants merged; truncation caps
  commented; `_syncCoeffProgramAddOptions` has its reason comment; hidden
  chips say why they're hidden; an 8-line "adding a transform" checklist
  sits above `_ctCatalog`; a which-code glossary sits at `_chainForWhich`.
- ✗ **Withdrawn: the "defensive `typeof` guards" finding was wrong.** They
  are load-bearing: the frontend harness executes extracted slices where
  later `let` globals don't exist (and same-file TDZ applies before init).
  Removing them broke the harness. They stay, now with comments explaining
  exactly this.
- ◻ `_renderChips`' twin pp/cp status blocks — left; the strings differ for
  real reasons and the mode noun is now consistent.

### `deploy.sh` — ✅ fixed

- ✅ **One spec list.** `deploy_lambda` (create-or-update by `$ACTION`) +
  `deploy_all_lambdas` (38 specs) + `deploy_orchestrator_lambdas` (3 specs,
  post-state-machine). The 41 unified specs were verified **byte-identical**
  to both former branch lists before deletion; the helper-signature trap
  (create's arg 5 = ROLE vs update's arg 5 = LAYERS) is hidden inside the
  wrapper. `tests/test_deploy_packaging.py` now pins the structure: exactly
  one spec line per function, both branches calling the shared functions,
  and only the wrapper + the two converge fallbacks calling the raw helpers.
- ✅ Step Functions: `ensure_sfn_execution_role` + `ensure_state_machine` +
  `grant_sfn_start_policy` replaced the six near-identical blocks — and the
  SM "ARN" vars no longer transiently hold an `updateDate` (the ARN is
  always constructed from the name; the helper documents why output isn't
  captured).
- ✅ The four identical Docker libcurl heredocs are one
  `build_libcurl_binary` helper (binary name + extra sources via env vars,
  inner script verbatim); the libvips stanza's nine repeated flag blocks
  use `$VIPS_CFLAGS`/`$VIPS_LIBS`/`$VIPS_TIFF_LIBS` defined once. The
  refactored build path was executed end-to-end locally: all 14 binaries
  compiled, lib staging produced libcurl.so.4 next to each dynamic binary,
  and the in-container smoke tests passed (libvips op probes, dz/png/tiff
  exports, sweep_cm roots 1,2,3).
- ✅ Staging DIR names match their `*_NAME` variables
  (`SOLVE_PROXIMITY_DIR`, `PALETTE_CHUNK_DIR`, `COMPUTE_PLAN_DIR`,
  `SWEEP_CM_DIR`, …); 15-line header phase map (and `show-build` described
  as deploy-nothing); `REMOVED_SWEEP_NAME` carries its retirement note.
- ✅ config.json's positional `%s` printf is **deliberate** and now says so:
  api_manifest.py parses those placeholders as the service contract.
- ◻ Echo label column alignment and the constants-block comment regimes —
  left; pure cosmetics in a file whose every functional duplication is gone.
- Net: deploy.sh shrank 2,398 → 2,144 lines while gaining six helpers and
  the header map; the spec list exists once.

### `coeff-program-commands.md` — ✅ fixed

- ✅ Table of contents; "Blending (andy)" section with the formula (the
  table rows and littlewood reference it); shadowing rules consolidated
  into one "Shadowed Names" subsection (the vector-binary note points
  there); real one-clause definitions for the circular rows pulled from the
  C (`safe` = zero non-finite parts, `max2one` = largest-magnitude
  coefficient set to 1+0j, `cummax` = running magnitude maximum,
  `swirler` = the exp(a⁴+b⁴+2πi·ab) swirl, `normalize` = divide by c[0],
  `deriv` = leading-first derivative, `round` = round re/im of src*a);
  linspace's count-and-stop wording fixed with an example; Limits says
  `push_vec`.
- ◻ Numeric caps stated in both table rows and Limits — left; both
  placements serve a reader.

### Tests — ✅ boilerplate fixed / ◻ regrouping deferred

- ✅ tests/test_coeff_program_native.py: the eight pasted params-tempfile
  blocks are one `_run_chunked_with_params()` runner (the existing helpers
  now delegate to it); every test reads as program/params/expect.
- ◻ test_coeff_program_chain.py's review-era class names
  (`TestCodeReview3Fixes`, …) — regroup by feature on the next substantive
  touch, as planned; renaming classes alone has no payoff.

---

## Genuinely well done — preserve these

1. **Decision-grade WHY comments at the dangerous spots**, now including the
   LAYOUT CONTRACT, `kind: 'andy'`, the typeof-guard harness explanation,
   and deploy's printf-is-wire-format note: the line/column default
   rationale, the greedy-regex postmortem, `_canonical_zero`'s fingerprint
   explanation, the macro-budget rationale, round's pinned 2-arg decision,
   C's division-policy comments, `c_exp2`'s "transpiler intrinsic",
   deploy's SECURITY MODEL / layer-cache / error-routing notes.
2. **The registry-driven transform architecture** — now documented as
   procedure, not tribal knowledge: the GLOSSARY at `legacy_registry()` and
   the "adding a transform" checklist above `_ctCatalog`, with packing
   rules pinned by parity tests and registry ids pinned by drift tests.
3. **Mirror comments, async hygiene, keyword-only flags**; the frontend
   harness pinning load-bearing source lines (it pins *exact text* — edits
   to pinned lines update the harness in the same change, which this pass
   did several times).

---

## What deliberately remains open

- Python's one-line normalization idiom (T6) and the mixed `_typed_lower_*`
  return shapes — working idiom, no reader cost.
- `compile_coeff_program_source` as the unused-but-tested public entry —
  route handlers through it on the next real handler edit.
- test_coeff_program_chain.py class regrouping — next substantive touch.
- deploy.sh echo-column alignment; `_renderChips` twin status blocks —
  cosmetics below the effort line.
