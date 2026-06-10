# Code Review 5 — Smell & Aesthetics

**Date:** 2026-06-10 (second pass: same day, after the priority items landed)
**Reviewed at:** working tree on `7625ca8` + the review-3 and review-5 fixes
**Bar applied:** "elegant, easy to read for a human, easy to maintain,
self-explanatory, generally not annoying. No unexplained stuff, no dead ends."
**Scope:** `lambda/coeff_program_source.py`, `lambda/coeff_program_chain.py`,
the Coeff Program region of `lambda/sweep_cli.c`, the Coeff Program editor in
`index.html`, `deploy.sh`, `coeff-program-commands.md` — plus, in the second
pass, the test files and handlers that the first pass skipped.

**Method:** four parallel aesthetics reviewers (one per area) with every major
claim re-verified by me, followed by a full second pass after the priority
fixes: every finding re-checked against the current tree, line anchors
refreshed, counts re-measured, and two areas nobody had reviewed (tests,
handlers) swept. Status legend: **✅ fixed** (this session), **⏳ deferred**
(by decision — do with tests, not casually), **◻ open** (cleanup, ride along
with the next touch).

---

## Verdict

The bones are good. The registry-driven transform architecture means a plain
new transform is a JSON edit with **zero Python changes**; the typed-stack
validator's diagnostics are excellent; the WHY-comment culture at dangerous
spots is exactly right; there are **no TODOs, no commented-out code, no dead
functions** anywhere in scope. After the priority fixes, the two most
error-prone unstated facts (the src/tgt layout flip and andy-as-placeholder)
are now stated in code, and the C VM no longer has silent failure paths.

What remains is fragmented vocabulary and repeated structure — all mechanical,
none behavior-changing, none deploy-blocking.

---

## Cross-cutting themes

### T1. One concept, many names — ◻ open

- **Registry transforms** are called "legacy" (`_legacy_token`,
  `coeff_legacy_registry.json`), "native transform"
  (`_native_transform_token`, `COEFF_OP_NATIVE_TRANSFORM`), and "registry
  transform" (`_registry_transform_token`) — three terms, one JSON file, no
  comment defining the relationship (legacy = old chip form/opcode 9,
  native = same registry fn via opcode 29). "Native" is also overloaded:
  the chain docstring uses it to mean the C VM.
- **JS prefix split** (re-measured, unchanged): 6 distinct `_cp*` names vs
  35 `_coeffProgram*` names, plus verb-first forms and bare onclick
  globals. No rule; the feature cannot be grepped with one prefix.
- **The non-program pipeline mode** still has three user-facing names
  ("Legacy selected", "Chain mode selected", "the Chain pipeline").
- **`const`/`push_const`/`push_vec`** rename is re-implemented inline at
  three JS sites (a `_canonicalCoeffTransformName` analogue exists for the
  ct side).
- **"lower" means two things**: source→chips in `coeff_program_source`
  (per its docstring) vs chips→tokens in `coeff_program_chain`
  (`_lower_chip` at chain:1483 dispatches to `_compile_*`). Renaming C's
  pair to `_compile_chip`/`_compile_chain` (private) makes the pipeline
  vocabulary coherent.

**Fix shape:** one term per concept + a 3-line glossary at
`legacy_registry()`; one JS prefix; one mode noun.

### T2. The `[tgt, src]` vs `[src, tgt]` wire-order flip — ✅ documented / ⏳ structural

The dual layout is wire-frozen and stays, but it is no longer implicit:

- `index.html` now carries a **LAYOUT CONTRACT** comment at the normalize
  seam, and all three numeric swap sites use named locals
  (`legacySrc`/`legacyTgt`, `chipTgt`/`chipSrc`) instead of bare
  `params[2], params[1]`; the copy path's two `'poly','poly'` emits carry
  per-layout comments. The frontend harness pins the contract comment.
- `coeff_program_chain.py` documents all three flip/straight-through sites
  (now at 1516-1521, 1568-1571, 1577-1581).
- Still ◻ open as a nit: the flip leaks into `_execution_spec` output
  (SET/AFFINE print tgt-then-src, LEGACY/NATIVE print src-then-tgt) —
  cosmetic, wire-compatible to change only with a fingerprint migration,
  so realistically permanent; a one-line comment there would close it.

### T3. "andy" — ✅ JS metadata fixed / ◻ glosses still open

- ✅ JS: `_ctAndyParam` now carries `kind: 'andy'` with a comment that
  `ph` is display-only; `_isAndyParam()` replaced all eleven
  `ph === 'andy'` checks (re-verified: zero raw checks remain);
  littlewood's drifted hand-typed clone now derives via
  `{ ..._ctAndyParam, title: ... }`.
- ◻ Open (small): the one-line semantic gloss is still missing next to
  `_compile_andy` (chain:875 area) and at the `tok->andy` struct field in
  C; the manual still defines andy only in the littlewood section while
  ten table rows say "optional `andy`" — a "Blending (`andy`)" subsection
  with the formula would close all of them.
- ◻ New in second pass: `_ctAndyIndex` (index.html) still returns
  `Math.max(0, pDefs.length - 1)` — "andy is always last" by assumption.
  Now that `_isAndyParam` exists, the honest one-liner is
  `findIndex(_isAndyParam)`.

### T4. Hand-mirrored tables and magic registry ids — ◻ open

- The unary-op name list still exists **four times** (chain
  `_VECTOR_UNARY_OPS`, source `_VECTOR_UNARY_NAMES`, and twice inside the
  expression parser — name set + name→op dict). Source could derive its
  copies; it already imports 30+ names from chain.
- Registry ids 14/16/23/24 remain bare ints at seven sites across the two
  Python files (decoder-ring comment at source:245 only). `FN_LINEAR,
  FN_EXP, FN_ROUND, FN_POW` constants in chain — or a packing field in the
  registry JSON — remove the least-discoverable sync trap. (The behavior
  itself is pinned by `test_native_transform_packing_parity_*` since
  review 3, so drift now fails loudly; this is readability.)
- Source still imports three underscore-privates from chain
  (`_ExpressionParser`, `_expr_value_if_static`,
  `_canonical_unary_op_name`); the codebase's own precedent
  (`native_transform_stack_arg_limit`, promoted with a docstring) is the
  model.

### T5. Error/diagnostic voice — ✅ silent paths fixed / ◻ voice still mixed

- ✅ **Zero silent failure paths remain in the coeff VM** (second-pass
  re-scan): the nine flagged sites now print (`coeff_program scalar
  expression stack overflow/underflow at unary/binary op`, non-real
  dynamic index, arg/length index range, andy NULL-out, roll-empty), and
  the conflated message was reworded to "did not produce a single finite
  result". The only two bare `return 1;` lines left in the region
  (dispatchCt at 3314/3321) are **propagating** returns — verified that
  `ct_pow_args`/`ct_arg_pad_lo` print before failing.
- ◻ The C log-prefix split persists and is now exactly **46
  `"Coeff Program ..."` vs 46 `"coeff_program ..."`** plus 4 unprefixed
  (my new messages used the snake prefix, evening the score). Grepping
  Lambda logs still takes multiple patterns; a mechanical sweep to one
  prefix remains worthwhile.
- ◻ Python message voice still mixes four templates ("requires" /
  "expects" / `{v!r}` vs bare / two selector-error patterns), and
  littlewood is "value1/value2" in source errors but "field1/field2" in
  chain errors.
- ◻ JS: the chips/text authority rules still live only in status strings;
  a 4-line comment above the `_effective*` pair is missing.

### T6. Ad-hoc normalization — ◻ open

`str(x or "").strip().lower()` ×28 in the Python pair (including the
double-expression alias lookup pasted at source:236 and :593);
`String(v == null ? '' : v)` ×33 and `${n === 1 ? '' : 's'}` ×18 in JS with
`_ctParamValue` re-inlined at six sites; the andy-strip stanza pasted ×4 in
chain (a `_split_andy` helper collapses it).

---

## Per-file highlights (beyond the themes)

### `lambda/sweep_cli.c` (coeff region) — anchors refreshed

- **⏳ The 12-value caravan** (unchanged: 7 functions at 15–17 params, the
  `p1r..ws` caravan at **34 call sites**). `CoeffEvalContext` deletes
  ~50–60 lines and is the highest-payoff refactor — deferred by decision
  to a calm moment with tests, not pre-deploy.
- **⏳ `evalCoeffProgram`**: still 323 lines, 30 arms, historical order
  (LINSPACE/RANGE wedged between CONST and PUSH). Half the arms already
  delegate; finishing the per-op-function pattern + enum-ordered `switch`
  rides along with the context refactor.
- ◻ `CoeffProgramWorkspace` (3494) still has zero field docs; `original_*`
  doubles as andy snapshot and binary-op operand2; `scratch` is also the
  SWAP temp while `aux` is the roll temp. Six lines of struct comments.
- ◻ `coeffProgramApplyTypedBinary/Unary` are no longer "typed" (shared by
  the untyped ops 12/13); the comment patches the name.
- ◻ Unexplained magics: `1e-12` imag tolerance ×9 (no named constant; same
  literal means a ceil-fudge elsewhere), `-700` log sentinel, six `350.0`
  trig thresholds, the `-1.0` poly_len sentinel contract, `1e-9` integer
  snap. (The `c_div` cutoff and -0 comments show the house standard these
  deviate from.)
- ◻ `stack_head` ring-buffer costume (always derivable from
  `stack_depth`; the modular incantation ×5, twice inlined); dead
  `!ws ||` NULL guards for a calling mode that doesn't exist; `EMIT`'s
  silent empty-stack no-op is documented Python-side but uncommented in
  the C arm; fn14/fn16 stack-packing twins; BINARY/ARGSORT prologues
  re-encode selector bounds as raw doubles (`>= 1.0 && <= 5.0`).
- ◻ Nits: snake_case stack primitives vs camelCase everything else; two
  -0-canonicalization idioms; one banner for 1,750 lines (sub-banners at
  parse / arg-plumbing / kernels / evaluator).

### `lambda/coeff_program_source.py` + `coeff_program_chain.py`

- **◻ `_lower_call` (source:630, now ~125 lines after the stack-op call
  forms) and `_lower_chip` (chain:1483, ~95 lines)** remain long name
  dispatchers; most branches are mechanical `name → compile fn` pairs.
  Dispatch tables, keeping only the irregular cases as named helpers —
  deferred with the other table conversions.
- **◻ `_execution_spec` (chain:1771)**: the identical LINSPACE/RANGE
  branches are now at 1783/1788; the 150-char nested ternary is pasted in
  both; the `_SELECTOR_NAMES.get(int(...) or 0, ...)` idiom ×17. Merge +
  two tiny locals shrink it by a third.
- **◻ `compile_coeff_program_chain` (chain:2019)** still has no docstring
  and writes its 19-key result dict twice (success + error literals).
  Public docstrings missing on the same five entry points as before.
- ◻ Mixed return shapes in `_typed_lower_*` (bare chain vs `(chain, type)`
  vs `| None`); 5-tuple positional returns from the legacy-args family.
- ◻ `compile_coeff_program_source` is an **unused public abstraction**
  (not unreachable code — tests use it): production handlers compose
  `parse_coeff_program_source` + `compile_coeff_program_chain` manually —
  second pass corrected the count to **four** handlers
  (`handler_coeffgen`, `handler_compute_plan`, `handler_compute_preview`,
  `handler_storage`), and `handle_compile_coeff_program_source`
  re-implements the body inline (handler_storage.py:1207). Route them
  through it or mark it test-only.
- ◻ Smaller: `_lower_range` is the one sibling not taking `target=`;
  scale/shift blocks are structural clones; the push-order surprise at
  source:568 lacks its one-line why; the `_typed_*` underscore chip-name
  convention is unstated; strict mode discards structured line/column via
  bare `RuntimeError(str(exc))`; wire-locked limit constants
  inconsistently annotated; three complex→text serializers with no
  "which to use when".

### `index.html` (Coeff Program editor)

- **◻ ~130 lines of pp/cp picker copy-paste across eight function pairs**
  — the drift it caused (littlewood's andy) is ✅ fixed at the data level,
  but the structural duplication remains; the `which` mechanism is the
  natural parameterization.
- ◻ `updateChipParam` (92 lines, six near-identical alert branches),
  `_renderChips` (73 lines, twin pp/cp status blocks), the catalog IIFE
  (~230 lines with the exp/power shadow surgery 1,100 lines from the
  alias maps it mirrors).
- ◻ `_coeffProgramSourceFromChain` takes serialized rows, not the chain
  its name implies; rename or document the object⇄row contract at the
  serialize/normalize pair.
- ◻ Nits: preview truncation constants disagree (6 vs 4 lines);
  `_syncCoeffProgramAddOptions` is a symmetry-only trivial wrapper;
  hidden chips lack "why hidden" comments; twin target-choices constants
  with identical contents; defensive `typeof` guards against same-file
  globals.

### `deploy.sh` — anchors refreshed after the ARN deletion

- **⏳ The create/update spec lists are still typed twice** (now
  1838–2065 vs 2097–2330), and the helper signatures still differ
  (1419: arg 5 = ROLE; 1454: arg 5 = LAYERS) — the copy-paste trap. One
  `deploy_lambda` table used by both branches removes ~250 lines.
  Deferred by decision (do with tests).
- ✅ The 28 dead Step Function ARN assignments are deleted (verified:
  zero matches remain; `bash -n` + 20/20 packaging tests pass).
- ◻ Packaging-block surface drift (echo label columns 13→12→15, three
  cp-wrapping styles, header comments restating drifted file manifests);
  DIR-name abbreviation drift (`SP_DIR`, `PD_DIR`, `CM_DIR`, `PAL_*`);
  four identical Docker libcurl heredocs + nine identical gcc flag
  stanzas; zero banners in the update branch; helpers interleaved with
  phases; the 11-line header documents 6 of ~38 lambdas and not the
  create⇄update convergence; constants block in three regimes with
  "1769 = 1 vCPU" explained for 3 of 8 occurrences.
- ◻ Nits: SM "ARN" vars briefly hold an `updateDate` and are echoed as
  ARNs (×3); config.json's one snake_case key + 15-`%s` positional
  printf; `REMOVED_SWEEP_NAME` tombstones with no retirement note;
  DEPLOYED/UPDATED summaries list an unexplained subset.

### `coeff-program-commands.md`

- ◻ Still no table of contents (675 lines, 17 sections).
- ◻ The pow/power/exp shadowing rules stated three times; numeric caps
  stated twice (body + Limits).
- ◻ Circular table rows remain ("`swirler` — Legacy swirler transform",
  "`safe` — Legacy safe transform") — pull one mathematical clause each
  from the C.
- ◻ andy defined only under littlewood (see T3); presentation drift
  (heading depths, `Example:` vs `Examples:`, linspace's
  `length`/`count`, `push_const` used after `push_vec` is declared
  preferred); three sentences needing two reads (rewrites provided in
  review history); unexplained cross-references ("Param Program or Param
  Chain", "Chain mode"); `emit` defined twice.

### Second-pass additions (areas the first pass skipped)

- **◻ tests/test_coeff_program_native.py (1,037 lines, 35 tests): the
  params-tempfile boilerplate is pasted 8 times** even though helpers
  exist and are used 18× elsewhere (`_run_source_with_params`,
  `_run_chain_values`) — the older tests predate the helpers. Migrating
  them deletes ~80 lines and makes every test read as
  setup-program/params/expect.
- **◻ tests/test_coeff_program_chain.py (983 lines)** has accreted three
  review-era classes (`TestCoeffProgramChain`,
  `TestCoeffProgramReviewFixes`, `TestCodeReview3Fixes`) whose names
  describe *when* tests were written, not *what* they cover. Fine
  historically; on the next touch, regroup by feature (parsing /
  validation / transforms / folding) so a failure's location says what
  broke.
- **Handlers**: the four inline parse+compile compositions are
  shape-consistent with each other (no drift found) — the smell is only
  the unused abstraction noted above, not divergence.
- **Verified-clean loose ends**: the two remaining bare `return 1;` lines
  in the C region are propagating (callees print); the `zip -FS`
  conversion covers all 41 zip sites including the render-plan helper;
  the new `.claude/settings.json` allowlist contains no
  arbitrary-execution wildcards.

---

## Genuinely well done — preserve these

1. **Decision-grade WHY comments at the dangerous spots**, now including
   the new LAYOUT CONTRACT and `kind: 'andy'` notes: the line/column
   default rationale, the greedy-regex postmortem, `_canonical_zero`'s
   fingerprint explanation, the macro-budget rationale, round's pinned
   2-arg decision, C's division-policy comments at both sites, the
   regression-guard comments, `c_exp2`'s "transpiler intrinsic", deploy's
   SECURITY MODEL / layer-cache / error-routing notes.
2. **The registry-driven transform architecture** (a plain new transform =
   one JSON entry, zero Python) and the JS chip catalog (a non-colliding
   chip = one entry) — now with the packing rules pinned by parity tests.
   Still undocumented as a fact: one sentence in the chain docstring and
   an 8-line "adding a transform" checklist above `_ctCatalog` would
   convert tribal knowledge into procedure.
3. **Mirror comments, async hygiene, keyword-only flags**: JS functions
   naming their exact backend counterparts; sequence-token staleness
   guards and try/finally busy-state restoration in the modal layer; no
   boolean traps anywhere in the Python pair; clean module ordering; the
   frontend harness pinning load-bearing source lines (note: it pins
   *exact text* — editing a pinned line means updating the harness, as
   the `_ctAndyParam` marker showed).

---

## Remaining priorities (post-fix)

1. **⏳ `CoeffEvalContext` + per-opcode `switch`** in the C VM — the big
   one, with tests, not casually.
2. **⏳ deploy.sh spec-list unification** (and the helper-signature trap
   dies with it).
3. **◻ Vocabulary pass**: registry-transform glossary + one term, one C
   log prefix, FN_* constants, promote the three underscore imports,
   derive source's mirrored tables.
4. **◻ Docs pass**: ToC, "Blending (andy)" section, real definitions for
   the circular rows, the two discoverability sentences.
5. **◻ Ride-along cleanups** (next time each file is touched): the
   workspace struct comment, named tolerance constants, `_split_andy`,
   `_execution_spec` merge, native-test boilerplate migration, picker
   parameterization.
