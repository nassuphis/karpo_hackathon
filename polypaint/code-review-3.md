# Code Review 3 — CR2 verdicts, dead code/duplication pass, deploy.sh

> **Status (2026-06-10): all findings addressed in the follow-up commit.**
> CR2-1/CR2-2/CR2-3, P-1..P-13, C-1..C-8, J-1..J-6, and D-1..D-10 are fixed;
> regression tests added (alias mirroring, packing parity, affine gates,
> folding, angle/abs parity, forgiving elementwise divide, packaging-test
> bans extended). Deliberate decisions, now documented in code/docs instead
> of changed: C-4 keeps two division policies (scalar expressions error on
> /0 like static folds; elementwise vector division yields 0 — documented in
> the commands doc and both C sites); `c_div`'s small-denominator cutoff
> stays for param transforms/solvers/transpiled functions (coeff+param
> program VMs now use `c_div_full`); the layer-cache key stays script-hash
> only (commented); `c_exp2` keeps its name (transpiler intrinsic, commented);
> the create/update per-function call lists remain two blocks, but the
> drift-prone shared content (dispatch env, workflow templating, async
> invoke-config, render-plan packaging) is now defined once and both helpers
> converge on re-runs (create falls back to update and vice versa).

**Date:** 2026-06-10
**Reviewed at:** commit `7625ca8` (clean working tree)
**Scope:**

1. Verdicts on the three findings in `code-review-2.md` (all re-verified by execution).
2. A fresh pass over the Coeff Program stack focused on dead code, duplication,
   and things that don't make sense (Python compiler/parser, C VM, JS editor).
3. A full review of `deploy.sh` (2,407 lines), the AWS deployment script.

**Method:** four parallel reviewers (Python, C, JS, deploy.sh), with every
major claim re-verified by me — either by executing the actual parser/compiler
or by reading the cited lines. Tags: `[executed]` I reproduced it by running
the real code; `[code read]` I read the flagged lines; `[reviewer-traced]`
traced by a reviewer with quoted grep/line evidence, not independently re-run.

---

## Part 1 — Verdicts on code-review-2.md

### CR2-1 [major] Legacy `power` transform broken in chip/UI paths — **CONFIRMED** `[executed]`

All three repros reproduce exactly as you described:

- `compile_coeff_program_chain([["power","poly","poly","8"]])` →
  `power src2 selector is invalid: '8'` (the vector-binary branch wins at
  `coeff_program_chain.py:1530`, before the registry branch at 1579).
- Source `poly = power(poly, 8)` lowers to typed elementwise `poly ** 8`.
- `["legacy","power","poly","poly","8"]` still compiles fine (fn 25).

And the JS paths are exactly as you suspected — only `exp` is special-cased:

- `_copyCoeffTransformsIntoCoeffProgram` (`index.html:13677`):
  `const programName = normalized.name === 'exp' ? 'exp_affine' : normalized.name;`
  A copied legacy `power` row becomes a chip named `power`, which is the
  **vector-binary** catalog entry (the `_ctCatalog` loop skips it via
  `if (catalog[name]) return;` at 11907). Chips mode then serializes
  `["power","poly","poly","8"]` → backend 400 with the baffling selector
  message. Text mode synthesizes `poly = power(poly, 8)` → **silently
  computes elementwise poly^8** — different math, no error.
- `_normalizeCoeffProgramChain` (`index.html:13531`): an andy-default
  `["legacy","power",...]` row from a saved/uploaded chain is converted to the
  same colliding named chip (only andy-carrying rows stay `legacy` chips), and
  `_validateCoeffProgramUiChain` passes because `power` is a catalog name —
  the corruption is invisible at load.

**Two additions to your finding:**

1. The recommended fix needs one more leg than the review suggests: there is
   **no `power_series` (or `pow_affine`) chip** — neither in the JS catalog
   nor in the chain compiler. Verified: `[["pow_affine","poly","poly"]]` and
   `[["power_series","poly","poly"]]` both fail with `unknown coeff program
   chip` (chain `_LEGACY_NAME_ALIASES` is only `{exp_affine: exp,
   scale100: linear}`). So "map power → power_series in normalize/copy" alone
   would produce chips that don't exist. The fix is either
   (a) keep `power` rows as `legacy` chips in normalize/copy (the mechanism
   the andy-preservation path already uses), or
   (b) add `power_series`/`pow_affine` to chain `_LEGACY_NAME_ALIASES` + a
   catalog chip, making the name first-class everywhere. (b) is the
   complete fix and also resolves CR2-3.
2. The JS alias-map entry `power: 'power_series'` currently fires **only** in
   the legacy-chip synth branch (`index.html:12904`); in the nativeTransform
   branch (12892) only `pow` can ever match, because `power` rows return
   earlier from the vector-binary branch (12860). So today the alias map is
   live for exactly the rows that normalize/copy *don't* produce — the
   andy-carrying legacy ones. `[code read]`

### CR2-2 [concern] `linear(poly, 5)` accepted but means `pop * poly + 5` — **CONFIRMED, and it generalizes** `[executed]`

`cf\npoly = linear(poly, 5)` compiles with zero diagnostics to
`pop * poly + 5`. Your recommended fix (reject 2-arg `linear` when the first
arg is a vector source name) is right, but the footgun is the whole
affine-shorthand family:

- `poly = scale(poly)` → `pop * poly` (poly used as the *multiplier*).
- `poly = shift(poly)` → `pop + poly`.
- `linear(100, 0, 1)` (the natural "multiplier, offset, andy" reading) is
  silently bound as `(src=100, multiplier=0, offset=1)` and compiles clean to
  a **scalar** `100*0+1` pushed to the stack — no diagnostic at all in push
  position. (In `poly =` position it at least errors, but with the misleading
  "poly assignment requires a vector-valued expression".) The system's own
  andy-splitters recognize the 3-arg-with-andy shape on every other path
  (`_native_transform_args_and_andy` fn 14 splits `{3,5}`), so the 3-arg
  shorthand form is the one place the convention inverts.

**Fix shape:** in `_typed_lower_affine`, when the first arg of a 1-arg
`scale`/`shift` or 2-arg `linear` is a vector source name, reject with the
message you proposed; and for 3-arg `linear` where the first arg is *not* a
source name, either reject as ambiguous or treat as `(multiplier, offset,
andy)` via the native route (consistent with the 4-arg form).

### CR2-3 [nit] Alias comment drift — **CONFIRMED** `[executed]`

`coeff_program_source.py:120-122` claims `_NATIVE_TRANSFORM_ALIASES` is
"Mirrored by `_LEGACY_NAME_ALIASES` in coeff_program_chain"; the chain map is
`{'exp_affine': 'exp', 'scale100': 'linear'}` — one of three entries mirrored,
plus an unrelated one. Consequence verified above (CR2-1 addition 1):
`pow_affine`/`power_series` chips are rejected in chain mode while
`exp_affine`/`scale100` compile. Recommend resolving by **adding** the two
entries to `_LEGACY_NAME_ALIASES` (option (b) above) rather than weakening the
comment — symmetric naming across layers is what prevents the next CR2-1.

---

## Part 2 — Fresh pass: dead code, duplication, doesn't-make-sense

### Python (`coeff_program_source.py`, `coeff_program_chain.py`)

**Dead code**

- P-1 [minor] Four public entry points have zero callers repo-wide
  `[executed: grep]`: `compile_coeff_program_diagnostics` (chain:2093),
  `coeff_program_chain_id` (chain:2097),
  `compile_coeff_program_source_diagnostics` (source:871),
  `validate_legacy_registry` (chain:303 — the only test importing that name
  imports it from `param_program_chain`). All callers compose the underlying
  functions directly (e.g. `handler_storage.py:1207-1208`). Delete, or route
  the four handlers through `compile_coeff_program_source` so the wrappers
  earn their keep.
- P-2 [minor] `_LEGACY_UNARY_NAMES` (source:91-106) is fully dead data
  `[executed]`: every member is a registry name, and its only use is
  `if name in _LEGACY_UNARY_NAMES or name in legacy_registry()["by_name"]`
  (source:687) — the left clause can never decide. Worse, it's a drift trap: a
  name added there but not to the registry would route into the native
  transform call and die with the wrong error.
- P-3 [minor] `execution_tokens` result field is the same object as `tokens`
  `[executed]` (chain:2046-2047, also in the strict=False payload). No reader
  anywhere; the storage tests actively assert it is stripped before persisting
  (`test_coeff_program_storage.py:84`). Same vestige in
  `param_program_chain.py:1202`.
- P-4 [nit] Unreachable re-checks in `_legacy_transforms` (chain:1992,
  1997-1998 — `_legacy_fast_path` already excluded those token shapes);
  dead `allow_cf=False` arm in `_lower_vector_unary` (source:545-548);
  vestigial `level` param/filter in `_diagnostic` (source:151, 852).
  `[reviewer-traced]`

**Duplication**

- P-5 [minor] `_legacy_token` vs `_native_transform_token` (chain:1341-1392)
  are verbatim 25-line twins differing only in the op constant and one error
  string. `[code read]`
- P-6 [minor] `_linear_legacy_args` vs `_pow_legacy_args` (chain:1133-1210)
  are near-identical 38-line blocks (defaults and labels differ); the linear
  one mixes its own labels (`legacy(scale100) arg {idx}` inside, "legacy
  (linear) expects…" outside). `[reviewer-traced]`
- P-7 [minor] The andy-split/packing rules for fn 14/16/23/24 still live in
  three places (source `_native_transform_args_and_andy`, chain
  `_legacy_args` family, C `coeffProgramNativeTransformOp`). Currently
  consistent — verified case-by-case — and `native_transform_stack_arg_limit`
  now bridges source→chain, but the **packing rules themselves are not pinned
  by `test_coeff_program_drift.py`** (it pins enums/limits/fn-ids only). A
  parametrized packing-parity test would close the class. `[reviewer-traced]`
- P-8 [nit] mod→abs and sub/mul/div/pow alias canonicalization is re-spelled
  in ~5 places across the two files; inline selector sets re-spell
  `_SOURCE_NAMES` (source:472, 739); `_canonical_expr`'s `mod(`→`abs(`,
  `tau`→`pi2` rewrites duplicate what the chain expression parser already
  supports natively. `[reviewer-traced; tau/mod chain support executed]`

**Doesn't make sense**

- P-9 [minor] `mod` and `abs` produce **different fingerprints for identical
  math** `[executed]`: chip path keeps `mod` (fn 2), typed/source paths
  canonicalize to `abs` (fn 3); C executes both identically. Verified:
  `vector_unary:mod:poly:poly` vs `vector_unary:abs:poly:poly`, fingerprints
  differ → duplicate cache entries for the same program depending on whether
  it came from chips or text. Canonicalize mod→abs in
  `_compile_vector_unary` too (one-line; changes fingerprints of existing
  chip-mod programs once).
- P-10 [minor] Legacy `round` 2-arg back-compat ambiguity `[executed]`:
  `["legacy","round","poly","poly","2","3"]` compiles as multiplier=2,
  **andy=3**. Old persisted chips in the original component form `(a, b)` with
  b≠0 (andy trimmed by the JS serializer, which only trims a single trailing
  default andy — index.html:13582) silently change meaning from `round` by
  complex `2+3j` to multiplier-2-blend-0.3. New UI data is consistent; only
  old data is at risk. Worth a one-time scan of saved programs or a comment
  pinning the decision.
- P-11 [nit] `poly = blend(t)` is rejected ("assign it with emit") while the
  equivalent `poly = add()` works by appending `_typed_set_poly` — the
  asymmetry has no VM grounding (source:648-650). `[reviewer-traced]`
- P-12 [nit] Source-text scalar constants never constant-fold:
  `push_scalar(1-2j)` emits 3 typed tokens where the chip path folds to one
  literal — costs token budget and splits fingerprints between text and chip
  authoring of identical programs. `[reviewer-traced]`
- P-13 [nit] Error-message polish: native-path errors say `legacy(exp)
  expects…` while source text forbids writing `legacy(...)`; `roll(cf, 2)`
  says "expected source selector, got 'cf'" (cf *is* one — it's just not
  allowed here); `emit()`/`dup()` fail with "unknown function" while every
  other zero-arg construct accepts the call form; stale comment
  `# mirrors the clamp in coeffProgramLegacyOp dispatch` (chain:28) names a C
  function that doesn't exist (it's `coeffLegacyIntArg`). `[reviewer-traced;
  comment verified by grep]`

### C VM (`sweep_cli.c`)

- C-1 [major, drift risk] The vector binary/unary math is implemented twice,
  entry-for-entry: `coeffProgramBinaryVectorOp`/`coeffProgramUnaryVectorOp`
  (4209-4275) vs `coeffProgramApplyTypedBinary`/`ApplyTypedUnary` (4337-4402),
  including identical `isfinite` clamping. The review-1 `c_div_full` patch had
  to be applied at **both** 4220 and 4348 — proof the tables drift-couple.
  Both opcode families are live (Python emits ops 12-15 and 23-24). The legacy
  loops should call the typed apply helpers per element. `[code read — I
  patched both sites myself in review 1]`
- C-2 [minor] `c_abs` (2553), vector MOD/ABS (4242), and typed unary abs
  (4368) still compute `sqrt(r*r+i*i)` — the exact underflow/overflow class
  the review-1 hypot patch fixed for `c_log`/`c_powr`. `abs(z)` of
  |z|≈1e-200 underflows to 0 while Python's static fold is exact. Same
  one-line hypot treatment applies (note `c_abs` is also a transpiler
  intrinsic used by generated coefficient functions — changing it affects
  those too, which is fine but should be deliberate). `[executed: grep; sites
  confirmed]`
- C-3 [minor] Signed-zero canonicalization covers 2 of 5 `atan2` consumers:
  `c_log`/`c_powr` canonicalize; scalar `EXPR_ANGLE` (3940), vector/typed
  ANGLE (4240/4366), and `c_arg` (2554) take raw signed zeros. So
  `imag(log(z))` and `angle(z)` can disagree on the branch for the same
  `z = (-x, -0)`. Pick one convention and apply it to all five (or to none
  and document). `[executed: grep of atan2 sites]`
- C-4 [minor] Two division policies in one VM: scalar-expr `1/0` errors the
  row; typed-stack `divide` by exact zero returns 0 and continues
  (`c_div_full` zero branch + isfinite clamp). Same input text, different
  outcome depending on which lowering path it took. `[reviewer-traced]`
- C-5 [minor] The roll comment claims the UB fix is complete, but
  `fmod(±inf/NaN, n)` → NaN and `(int)NaN` is still UB (4282-4284); compiler
  prevents it, but the sibling poke paths use `coeffProgramIntegerFromReal` —
  finish the job with an `isfinite` check. Related: static `CF_AT/POLY_AT/
  TOS_AT` index (3840) and `VECTOR_BINARY`/`ARGSORT` src2 (5034/5050) still
  raw-cast where siblings validate. `[code read — my own patch]`
- C-6 [minor] Coeff token parser silently lets `"args"` overwrite a
  conflicting explicit `"n_args"` (3640-3646) while the param-program parser
  errors on the same mismatch (6141-6175). Port the cross-check. `[code read]`
- C-7 [minor] fn16 static-args branch (4658-4669) is a byte-identical clone of
  the generic else (4678-4689): `legacyArgCount` is already initialized to
  `tok->n_args` (4579) and Python always emits fn16 as 4 real components.
  Delete the branch (keep the better error string if wanted). `[code read]`
- C-8 [nit] `ws->aux_len` is write-only (3516, 4288, 4328, 4802 — never read);
  `shift %= n` after `fmod` is a no-op (4285); dead `: 100.0`/`: 1.0` ternary
  arms inside blocks gated on `stack_arg_count > 0` (4602-4633); scalar-expr
  DIV inlines `c_div_full`'s body verbatim (3953-3965) instead of calling it;
  blend lerp duplicated between op 8 and typed blend (4979-4995 vs 4549-4564);
  expr-ref range check triplicated with three different messages; `c_exp2`
  computes e^z, not 2^x; the `+1` slack in the push_range span check (4874)
  buys nothing; param-program division still has the dead 1e-300 guards over
  `c_div`'s 1e-30 cutoff (6090-6094, 6351-6357, 6583-6589) — the c_div_full
  fix was deliberately not propagated there, but the dead guards remain.
  `[reviewer-traced; aux_len and fmod no-op code read]`

### JS editor (`index.html`)

- J-1 [major] = CR2-1 (the `power` collision), verdict above.
- J-2 [major] **Copy path silently drops non-default andy on every
  transform** `[code read]`: `_copyCoeffTransformsIntoCoeffProgram`
  (13671-13678) appends andy into named-chip params, but the named chips'
  pDefs exclude andy (filtered at 11901/11908-11910) and
  `_serializeCoeffProgramChain` maps values over pDefs only — so ct
  `['sin','0.5']` copies to a chip that serializes as `['sin','poly','poly']`.
  This contradicts the andy-preserving normalize logic added in review 1
  (13519-13530), whose own comment explains exactly this hazard. Fix: copy
  should emit `legacy` chips for andy-carrying rows, like normalize does.
- J-3 [minor] Vestigial `sourceName` in `_coeffProgramSourceFromChain`
  `[reviewer-traced]`: the only caller passes serializer output (12944), which
  can never emit an `exp` row with >2 params (catalog exp is the 2-param
  unary; every ingest converts exp-with-extras to `exp_affine`). So
  `sourceName === name` always: the guard at 12866 is always true, and the
  whole `sourceName !== name` branch (12897-12901) is unreachable pre-alias-map
  leftover. Delete `sourceName`; keep the alias map.
- J-4 [minor] Emptying the chain chip-by-chip leaves stale auto-synthesized
  text authoritative `[code read]`: re-synthesis (12942) is gated on
  `_cpChain.length`, and `removeChip` (12530) only splices — synthesize text,
  delete all chips, switch to Text: the deleted program is still there and
  authoritative. Clear auto-synthed text when the chain empties.
- J-5 [minor] Copy doesn't clear pre-existing user text while load does
  (13653-13658 vs 13665-13686) — after "Copy legacy transforms", switching to
  the Text tab makes the *pre-copy* text authoritative. Same one-line clear as
  `_applyCoeffProgram`. `[reviewer-traced]`
- J-6 [nit] exp↔exp_affine aliasing is re-encoded at ~7 independent JS sites
  (11727-11729, 11893-11906, 12845, 12852, 13531, 13540, 13677, 14078) — CR2-1
  is the direct cost of this pattern; a single registry→chip-name map used by
  normalize/copy/synth would have prevented it. Also: dead `item.name ===
  'exp'` half of the condition at 14157 (only `exp_affine` can reach it);
  `_coeffProgramSourceDisplay` ignores `;` separators that the statement
  counter now counts; the tab-switch status overstates authority when the
  pipeline is in chain mode. `[reviewer-traced]`

---

## Part 3 — deploy.sh review

Overall: an unusually disciplined hand-rolled deploy for its size —
explicit-list packaging (nothing ships by accident), content-hash layer
dedupe, real in-Docker runtime smoke tests (libvips op probes, dzsave/pngsave
round-trips, sweep_cm known-roots check), a contract-test gate
(`scripts/predeploy_check.sh`) that runs before anything mutates, `show-build`
drift auditing, frontend uploaded last with byte-for-byte post-upload
verification. The packaging contract suite (`test_deploy_packaging.py`, 20/20
passing) pins build ordering, handler dependency closures, and route↔
permission parity. Two systemic problems and one large maintenance tax:

- D-1 [major] **Zips are appended to, never cleaned** `[executed: grep]`:
  every packaging block `rm -rf`s its staging dir but `zip -r9`s into the
  existing `/tmp/polypaint-*.zip` — zero `rm -f`/`zip -FS` hits in the whole
  script. `zip` add/updates entries but never removes ones absent from the
  staging tree, so on any machine that has deployed before, a file *removed*
  from a package list (deleted binary, renamed module) **ships forever**, and
  a stale `.py` can shadow a renamed module at import time. It also poisons
  `show-build`, which builds its comparison zip fresh and will report
  MISMATCH against a polluted deployed zip. Fix: `rm -f "$ZIP"` before each
  `zip` (or `zip -FS`). Cheap, mechanical, 40 sites.
- D-2 [major] **`>/dev/null 2>&1` without `|| true` under `set -euo pipefail`
  (deploy.sh:13)** `[executed: sites read]`: create-stage (1439-1441),
  `ensure_route`'s update/create-route (1491-1496), and all four
  event-invoke-config blocks (2006-2027, 2313-2333). Any transient AWS error
  (throttle, eventual consistency) kills the deploy **with no output at all** —
  the idiom reads as "ignore errors" but means "die silently". Contrast the
  correct `add-permission ... || true` at 1536-1541. Each site should pick a
  side: `|| true` if best-effort, or drop the redirects if must-succeed.
- D-3 [major] **Guarded lib copies can ship loader-broken zips**
  `[executed: lines read]`: `cp lambda/roots2pix_mt_lib/* ... 2>/dev/null ||
  true` (995, also 1201, 1213, 1258) vs the hard-fail copies for finalize/
  bilevel/lores (1006, 1038, 1092). If a `*_lib/` dir is empty (interrupted
  Docker build), raster-mt / solve-proximity / palette-chunk deploy fine and
  fail at runtime with a dynamic-loader error. The team already recognized
  this — `test_deploy_packaging.py` bans the `|| true` variant for three
  bundles — the ban just wasn't propagated to the other four. Extend the test
  and drop the guards.
- D-4 [major, maintenance] **The entire per-function config is duplicated
  between the create and update branches** (1754-2027 vs 2060-2333), including
  the 19-variable dispatch env line — verified character-identical today and
  present exactly twice `[executed: grep count = 2]`. The packaging tests pin
  only a handful of pairs, so most can drift unnoticed. A single
  `deploy_one_function name handler zip mem layers env tmp` table consumed by
  both branches would delete ~500 lines and the hazard. Related: the
  render-plan zip file list exists both in `package_render_plan_zip()`
  (293-306) and inline (1289-1301).
- D-5 [minor] No `Cache-Control` on any uploaded asset (`grep cache-control` →
  nothing): browsers heuristically cache `index.html`/`config.json`, so users
  can see a stale frontend after deploy. `--cache-control no-cache` for
  index.html/config.json is the minimal fix. `[reviewer-traced]`
- D-6 [minor] `"frontend_sha256"` in config.json is computed with bare
  `shasum` — i.e. **SHA-1** (214-218, 245-246) `[executed: grep]`. Comparisons
  are like-for-like so nothing breaks; the field name lies. `shasum -a 256`
  or rename.
- D-7 [minor] `update-function-code` failures are misdiagnosed: any error
  (`2>/dev/null`) falls through to "doesn't exist yet, creating...", and the
  subsequent create dies with a confusing "already exists" while the real
  error was discarded (1385-1394). Also no `wait function-updated` *before*
  code update — a still-in-progress prior update throws ResourceConflict into
  the same misdiagnosis. `[reviewer-traced]`
- D-8 [minor] `create` is not re-runnable (no exists-fallback in
  `create_lambda`, 1358-1379): a partial create must be finished with
  `update`, which does converge — workable but undocumented. `[reviewer-traced]`
- D-9 [minor] Security posture, presumably accepted: the whole bucket is
  public-read (177-197) and the HTTP API has CORS `*` with **no authorizer on
  destructive routes** (`/delete`, `/delete-prefix`, `/cleanup`, …,
  1604-1623). Anyone can read every artifact and delete prefixes. Fine for a
  hackathon project; worth stating as a decision. (Checked the obvious
  hazard: `gemini_key.txt` is gitignored, untracked, unreferenced by
  deploy.sh, and not in the upload set `[executed: git ls-files + grep]` —
  keep it that way, never add a root `s3 sync`.)
- D-10 [nit] Dead `$? -ne 0` checks under `set -e` make the friendly error
  messages unreachable (451-458, 505-509); the index.html JS extraction
  assumes exactly one `<script>` block (currently true); `REMOVED_SWEEP_NAME`
  tombstones run forever (pinned by tests, retire eventually) and
  `lambda/handler_sweep.py` is now pure cruft; `$BILEVEL_NAME`/
  `$REPALETTE_NAME` get API-GW permissions with no routes; layer rebuilds key
  only on build-script hash (never picks up upstream package updates, and old
  layer versions accrue forever); `step_scores_to_palette_raw` is the one
  static build without `-lm` (harmless under musl); palette/catalog
  generation uses system `python3` while tests use uv — two pythons in one
  script. `[reviewer-traced]`

**Repo drift: clean.** Everything deploy.sh references exists; all 40 zips are
deployed in both branches; 40 of 41 `handler_*.py` are packaged and the one
exception (`handler_sweep.py`) is the deliberately decommissioned sweep
Lambda whose absence the tests assert. Two things that looked vestigial are
live and correctly ordered: the host `sweep_test` build (used by
`gen_catalog.py` degree probing) and `stepfunctions/render_workflow.asl.json.template`
(consumed via `workflow_template_render.py`). The binary→layer→
`LD_LIBRARY_PATH` matrix is consistent across all 41 functions.
`[reviewer-traced, with ls/grep evidence; packaging tests re-run: 20/20]`

---

## Priorities

1. **CR2-1 / J-1 + CR2-3**: make `power`/`pow` first-class — add
   `power_series`/`pow_affine` to chain `_LEGACY_NAME_ALIASES`, special-case
   them (or use legacy chips) in `_normalizeCoeffProgramChain` and
   `_copyCoeffTransformsIntoCoeffProgram`, pin with round-trip tests.
2. **J-2**: copy path must preserve andy (emit legacy chips like normalize).
3. **D-1, D-2, D-3**: the three mechanical deploy.sh fixes (`rm -f` before
   zip; pick a side on every silent redirect; un-guard the four lib copies and
   extend the packaging-test ban).
4. **CR2-2**: gate the affine-shorthand family against source-name first args
   (and decide the 3-arg `linear` andy form).
5. **C-1** (merge the duplicated C op tables) and **C-2** (hypot for
   `c_abs`/MOD/ABS) — the two C items with real numeric/drift consequences.
6. **P-9** (mod→abs fingerprint unification) and **P-7** (a packing-parity
   test for fn 14/16/23/24) close the remaining cross-layer drift gaps.
7. Sweep the dead code in one pass: P-1..P-4, C-7..C-8, J-3, and the
   deploy.sh nits — none urgent, all cheap.

## Verification

```bash
uv run python -m pytest tests/test_coeff_program_chain.py tests/test_coeff_program_native.py tests/test_coeff_program_drift.py -q   # 100 passed
uv run python -m pytest tests/test_deploy_packaging.py -q                                                                            # 20 passed
bash tests/test_frontend_js.sh                                                                                                       # OK
```

Repro transcripts for every `[executed]` claim are reproducible from the
snippets above; the key ones: the `power` chain/source repros, the
`pow_affine`/`power_series` unknown-chip rejections, `linear(poly,5)` /
`scale(poly)` / `shift(poly)` / `linear(100,0,1)` lowerings, mod-vs-abs
fingerprint inequality, legacy-round `(2,3)` → multiplier+andy, the
`execution_tokens is tokens` identity, and the deploy.sh grep counts (zero zip
cleanups; dispatch env line exactly twice; `set -euo pipefail` at line 13).
