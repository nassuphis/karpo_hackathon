# Coeff / Param Editor Help: Data Sources, Format, and Current Defects

## Objective

The Param Program and Coeff Program editors need real help, not a second copy of the starter cheat sheet.

The Help tab should answer:

- What can I type?
- What does this operation do?
- What forms are valid?
- What parameters exist?
- What are the defaults?
- What are the allowed values?
- What does it consume and produce?
- What is a minimal correct example?

The Starter tab is for insertion snippets. The Help tab is for reference. Mixing those two roles is the root of the current mess.

## The Core Fix: A Generated Grammar Profile (read this first)

This help file has been painful to write for one structural reason, not fifteen cosmetic ones: **Rule 1 ("Parser Wins") has no mechanism behind it.** The parser's grammar lives in Python sets — `param_program_source.py:47-60` holds `_BINARY_OPS`, `_UNARY_OPS`, `_TARGETABLE_UNARY`, `_PUSH_SOURCES`, `_EMIT_ALIASES`, `_STACK_OP_ALIASES` — and nothing in the frontend can see them. So today every Help article re-states those facts **by hand**, and "Parser Wins" is a discipline you personally re-verify per entry. Hand-transcribed grammar is exactly the static-help-that-drifts this whole effort was meant to kill. That is the mess. The cheat-sheet dump is just a symptom.

The repo already has the right plumbing, but it is not carrying enough grammar yet:

- `lambda/program_profiles.json` is the reviewed source of truth for profile metadata: caps, symbols, selectors, outputs, and contracts.
- `lambda/gen_program_profiles.py` emits both `lambda/program_profiles.py` and `program_profiles_js.js`, and `gen_program_profiles.py --check` is a predeploy gate (`scripts/predeploy_check.sh:20`).
- The browser sees this as `window._programProfiles`.
- `param_program_source.py` already reads `program_profile("param")` for symbols/output read-write behavior, but still hardcodes operation grammar in `_BINARY_OPS`, `_UNARY_OPS`, `_TARGETABLE_UNARY`, `_PUSH_SOURCES`, `_EMIT_ALIASES`, and `_STACK_OP_ALIASES`.
- `coeff_program_source.py` is better but not fully solved: it reads symbols/selectors from `program_profile("coeff")`, while operation vocabularies still come from `coeff_program_chain` tables and local alias maps such as `_STACK_ALIASES`, `_VECTOR_BINARY_ALIASES`, `_VECTOR_UNARY_NAMES`, and `_NATIVE_TRANSFORM_ALIASES`.
- `coeff_vocab_js.js` already exposes much of the Coeff registry vocabulary to the frontend, but Help still needs a source-form grammar layer for what users type.

So the problem is not that there is no profile mechanism. The problem is that the profile/generator does not yet carry the source-form operation grammar Help needs. Param is the worst case because the operation sets are completely frontend-invisible. Coeff is partially covered by profiles/registries, but source-form articles still need explicit grammar metadata.

### Fix (preferred): extend `program_profiles.json` with source grammar

1. Extend the existing `"profiles"."param"` block in `lambda/program_profiles.json` with a `source` grammar block carrying exactly what the parser sets carry today:

```json
{
  "profiles": {
    "param": {
      "program_kind": "param_program",
      "value_caps": "... existing fields stay ...",
      "symbols": "... existing fields stay ...",
      "selectors": "... existing fields stay ...",
      "source": {
        "binary_ops": ["add", "subtract", "sub", "mul", "ratio", "div"],
        "unary_ops": ["negate", "conj", "conjugate", "reciprocal", "unit_circle", "square", "cube", "exp"],
        "targetable_unary": ["negate", "conj", "conjugate", "reciprocal", "unit_circle", "square", "cube", "exp"],
        "unary_targets": ["p1", "p2"],
        "push_sources": ["t1", "t2"],
        "stack_op_aliases": {"dup": "duplicate", "duplicate": "duplicate", "swap": "swap", "pop": "pop", "flush": "flush"},
        "emit_aliases": {"emit_p1": "p1", "emit_p2": "p2"},
        "rejected_forms": [
          {"form": "push(both)", "use": "push(t1) / push(t2)"},
          {"form": "emit(p1)", "use": "emit_p1 / emit_p2 / p1 = expr"}
        ]
      }
    }
  }
}
```

2. Change `param_program_source.py` to derive `_BINARY_OPS`, `_UNARY_OPS`, `_TARGETABLE_UNARY`, `_PUSH_SOURCES`, `_EMIT_ALIASES`, and `_STACK_OP_ALIASES` from `program_profile("param")["source"]`. Now the parser and frontend profile cannot disagree. The parser's behavioral logic (arity checks, lowered chain rows, the `emit(p1)` rejection message) stays in Python but is driven by profile vocabulary.

3. **Coeff is different — do not hand-copy its vocab into the profile.** Param closes the loop in step 2: the parser *derives* its sets from the profile, so there is exactly one source of truth. Coeff's operation vocabulary is large and already table-driven — it comes from `coeff_program_chain` plus `_STACK_ALIASES`, `_VECTOR_BINARY_ALIASES`, `_VECTOR_UNARY_NAMES`, and `_NATIVE_TRANSFORM_ALIASES` in `coeff_program_source.py`. If you add a hand-authored `source` block to `profiles.coeff` while the parser keeps reading those tables, you now have two copies of the same truth, hand-synced — exactly the drift this document exists to kill. Close the loop one of two ways:

   - **(a) parity with param:** route the coeff alias maps/vocab through `profiles.coeff.source` and derive them in `coeff_program_source.py`. Cleanest, but a larger parser change because the vocab is bigger and partly comes from `coeff_program_chain`.
   - **(b) recommended for coeff:** generate the coeff grammar mirror *from the parser's existing tables* via the fallback generator below — it reads `_STACK_ALIASES`/`_VECTOR_BINARY_ALIASES`/the `coeff_program_chain` exports, so the truth stays in one place (the parser) and `--check` fails if it changes without regeneration.

   So the split is per editor, not global: **param takes the profile route (step 2); coeff takes the generator route (3b).** The two "fixes" below are not a global either/or.

4. The frontend gets Param grammar through an existing generated asset: it already loads `program_profiles_js.js` as `window._programProfiles`. The Help builder should read `window._programProfiles.profiles.param.source` for Param valid forms/aliases/rejected forms. For Coeff, read `window._programGrammar.coeff` from the generated grammar mirror in step 3b unless you intentionally choose the larger 3a Coeff parser refactor.

### Generated grammar mirror: Param fallback, Coeff recommendation

If routing parser behavior through the profile is too much for v1, add a generator that reads the parser/chain tables and emits a mirror, exactly like `gen_coeff_vocab.py`:

```python
# lambda/gen_program_grammar.py   (--write writes program_grammar_js.js at repo root; --check exits 1 if stale)
import param_program_source as pp   # the parser stays as-is; we read its sets
GRAMMAR = {
  "param": {
    "binary_ops": sorted(pp._BINARY_OPS),
    "unary_ops": sorted(pp._UNARY_OPS),
    "targetable_unary": sorted(pp._TARGETABLE_UNARY),
    "push_sources": sorted(pp._PUSH_SOURCES),
    "stack_op_aliases": pp._STACK_OP_ALIASES,
    "emit_aliases": pp._EMIT_ALIASES,
    "rejected_forms": [{"form": "push(both)", "use": "push(t1) / push(t2)"},
                       {"form": "emit(p1)",  "use": "emit_p1 / emit_p2 / p1 = expr"}],
  },
  "coeff": { ... },   # from coeff_program_source plus coeff_program_chain tables
}
# render `window._programGrammar = {...}` -> program_grammar_js.js; add `gen_program_grammar.py --check` to predeploy.
```

The parsers are untouched; the generator reads their current tables, so the artifact still cannot drift (the `--check` gate fails the build if parser tables change without regeneration). For **param** this is strictly less clean than the profile route (the truth ends up in two shapes), so prefer the profile route there. For **coeff** this *is* the recommended route (3b), not a fallback — coeff's vocab already lives in parser tables, so generating from them is the single-source path, whereas hand-mirroring it into the profile is not.

### Profile notes

- The new `source` aliases (`stack_op_aliases`, `emit_aliases`) are **operation** aliases and are distinct from the param profile's existing `selector_aliases` field, which aliases src/tgt selectors (`p1`/`p2`/`both`). Keep them separate; do not merge the two alias maps.
- `targetable_unary` duplicates `unary_ops` today because the parser treats every unary as targetable (`_TARGETABLE_UNARY = set(_UNARY_OPS)`). Keep it explicit so the profile can narrow it later, but add a test asserting the two stay consistent until they intentionally diverge.
- Param expression constants/functions are not in `program_profiles.json` today. They live in `lambda/param_program_chain.py` (`_EXPR_CONSTANTS` for `pi`/`pi2`/`pi2i`, plus expression functions such as `exp`, `real`, `imag`, `abs`, `mod`). Either add an `expression_namespace` section to `profiles.param.source`, or generate it from `param_program_chain.py`. Do not leave constants/functions as hand-written Help-only facts.
- `program_profiles.json` already carries four profiles — `param`, `coeff`, `root`, `solve_score`. The `source`-grammar convention generalizes to `root`/`solve_score` if those editors ever get Help, so treat it as a profile-wide convention rather than a param/coeff special case.

### Why this is the unlock

Either way, the frontend reads valid forms, aliases, params, and rejected forms from a **generated, gated artifact instead of your memory**. A form is valid iff it is in the grammar. Emit forms are `emit_p1`/`emit_p2` because the grammar says so. `square` gets `square(p1)`/`square(p2)` automatically because the grammar lists `square` in `targetable_unary` and `p1`/`p2` in `unary_targets`. **Mistakes #4, #5, #6, and #13 become structurally impossible**, and the Rule 6 audit collapses to two set comparisons (below). You stop playing whack-a-mole because the holes are gone.

### The Help Article model (one shape, three producers, two consumers)

Everything renders to **one** article object — do not keep `_programHelpItem(name, signature, help)`; it is too flat:

```js
{
  name,        // canonical source name: 'push', 'emit_p1', 'square', 'legacy', 'sort_mod_keep_angle', 'giga_139'
  aliases,     // ['duplicate'] for dup, ['subtract'] for sub — from grammar/registry, shown explicitly
  category,    // human heading: 'Stack', 'Unary transform', 'Coeff native transform', 'Coeff function'
  summary,     // ONE hand-written sentence (authored; never 'Param Program chip.')
  forms,       // every valid source form — FROM the grammar artifact / registry, never invented
  params,      // [{ name, default, choices?, type?, meaning }]
  effect,      // stack effect + what changes — hand-written: '(a b -- a+b)', 'replace p1 with p1*p1'
  examples,    // minimal valid programs
  notes,       // traps — partly FROM grammar.rejected_forms: 'emit(p1) is rejected; use emit_p1.'
}
```

**Three producers, same model:**

1. **Grammar core** (`push`/`emit`/`square`/`add`/`dup`/…): `forms`/`aliases`/`params`/validity come from the generated grammar artifact; `summary`/`effect`/`notes` are **hand-authored prose keyed by grammar command names**. This is the *only* hand-written part, it is small (~25 ops per editor), and it is where the real value lives — stack effects (#11), operand order, and "use `emit_p1`, not `emit(p1)`" notes that no registry will ever generate. Generate the skeleton, author the meaning.
2. **Transforms** (param legacy / coeff native): generated from `_paramProgramLegacyArgSpecs` + `_ptCatalog`/`_ptInfo`, and from normalized coeff registry entries (`effective_args`, aliases, categories, descriptions). After the migration, `andy` comes from `effective_args`, not from a JS-only append.
3. **Coeff functions**: generated from `window._coeffFuncCatalog`. Render these **lookup-only** (search box + the double-click inspector), never as a 1000-row list — fixes #9 by removing the surface that needs navigation, not by navigating it.

**Two consumers, same articles:** the Help tab renders the ordered list of articles per section; the inspector resolves a token (and its aliases) to the **same** article object. There is no separate sparse lookup list — that separate list was the source of the `sort_mod_keep_angle`-hides-`andy` shadowing (#7).

**The seam test (Rule 6, now trivial):** per editor, assert `article.forms ⊆ grammar.forms(name)` (no article documents a parser-rejected form) and `grammar.commands ⊆ articles` (every parser command has an article). Do this on normalized form objects, not raw display strings, so formatting changes do not break the test. Example form objects: `{kind:"call", name:"push", args:["t1"]}`, `{kind:"bare", name:"emit_p1"}`, `{kind:"assignment", target:"p1", expr:"expr"}`. Both sides derive from the generated grammar artifact, so this is a set comparison, not a subjective content audit.

## Deep Dive: How The Parsers Actually Work (verified findings)

I read `param_program_source.py`, `coeff_program_source.py`, `coeff_program_chain.py`, `program_source_core.py`, `program_profiles.json`, and the three `gen_*` generators in full. The picture is better than the "Core Fix" framing assumed in one way (coeff is mostly data already) and worse in another (the same grammar is hand-copied in up to three languages). The findings below are all cited to `file:line`.

### A. The shared parsing core already exists — and param doesn't use it

`program_source_core.py` ships `ProfileStatementLowerer` (`:305-394`) and `parse_profile_source` (`:397-461`), explicitly built to centralize the assignment/call/bare grammar split and writability checks "that must not drift between Param and Coeff."

- **Coeff uses it:** `_CoeffStatementLowerer(ProfileStatementLowerer)` (`coeff_program_source.py:755`), driven by `parse_profile_source` (`:864`).
- **Param does not:** it hand-rolls `_lower_assignment`/`_lower_call`/`_lower_bare` plus its own statement loop (`param_program_source.py:113/123/178/195-221`), re-implementing the split/find-assignment/parse-call orchestration the core already provides.

So the shared infrastructure is half-adopted. **Migrating param onto `ProfileStatementLowerer` + `parse_profile_source` deletes the duplicated grammar split and diagnostic plumbing** and puts both editors on one dispatch — a prerequisite for one Help generator covering both.

### B. Where the grammar truth actually lives (data vs control flow)

| Grammar fact | Param | Coeff |
|---|---|---|
| Symbols + selectors (cf/poly/p1/p2/t1/t2/tos/poly_len, writability) | **profile** (`program_profiles.json`) | **profile** |
| Native/legacy transforms (names, aliases, args, andy-support, categories, descriptions) | **`param_program_chain.py` registry** (70 ops, 32 w/ arg schemas) | **`coeff_legacy_registry.json`** via `legacy_registry()`, already consumed |
| Structural / stack / vector / constructor vocab | hardcoded sets `param_program_source.py:40-60` | **fully described in `structural_chips.json`** — but the parser ignores it (§C) |
| Scalar-expression ops/constants | n/a | `coeff_program_chain.py` literals (`SCALAR_UNARY_EXPR_OPS`, `_EXPR_CONSTANTS`) |
| **Statement grammar** (command keywords, arities, arg-kinds, call-vs-bare, assignment, rejected forms) | **control flow only** | **control flow only** |

Bottom line: **coeff vocabulary is ~80% already data** (registry JSON + `structural_chips.json` + profile), and `gen_coeff_vocab.py` already emits it to `window._coeffRegistryVocab`. Param vocabulary is sets-in-a-`.py`. The *statement grammar* of **both** is control flow — that's the part no table currently captures.

### C. The redundancy map (the "less redundant" target)

The same fact is encoded in up to three places that must be hand-synced:

- **Coeff structural vocab → 3 homes:** `structural_chips.json` (documentation/frontend metadata, *not executed at runtime*), the parser's hardcoded `_lower_call` ladder (`coeff_program_source.py:572-687`) + its mirror decompiler `coeff_source_text_from_chain` (`:1044-1181`), and the chain compiler's `_CHIP_COMPILERS`/`_ZERO_ARG_CHIP_OPS`. Only a `_typed_*` subset has a drift test.
- **Coeff vector-op names → 3 languages:** chain Python (`VECTOR_BINARY_OPS`/`VECTOR_UNARY_OPS`), the parser's derived sets, **and re-hardcoded in JS** — `js/07-transform-catalogs.js:445-446` literally lists `['add','subtract','multiply','divide','power']` and the 14 unary names. Verified.
- **Param `[t1,t2]`/`[p1,p2]` → 3 forms:** profile `selectors.push`/`emit`, the parser's `_PUSH_SOURCES`/`_EMIT_ALIASES` (`param_program_source.py:59-60`), and `_INPUT_SYMBOLS`/`_OUTPUT_SYMBOLS` (`:30-39`). The parser re-hardcodes what the profile already declares.
- **andy-arity logic duplicated** between source (`_split_native_transform_andy`, `coeff_program_source.py:205-226`) and chain (`_split_trailing_andy` + four `_*_legacy_args`), both hardcoding `{3,5}/{3}/{2,3}` by `fn_index`. This belongs in declarative `effective_args` + accepted `compat_signatures`, not in per-`fn_index` parser/compiler branches (only a `supports_andy` boolean exists in JSON today).
- **Forward and reverse grammars are hand-kept copies:** `param_source_text_from_chain` / `coeff_source_text_from_chain` re-enumerate the op categories independently of the lowerers — the round-trip is not table-driven.
- **Dead duplicates:** coeff `_legacy_lower_statement` (`:690-752`) is superseded by the class; param's `_INPUT_SYMBOLS` (`:35`) is unused; the public `compile_param_program_source` (`:241`) is unused (handler_storage.py:1683 inlines its body).

### D. Auto-extraction surface — readable today vs locked in control flow

- **Coeff vocabulary is generatable today.** `gen_coeff_vocab.py` already reads `coeff_legacy_registry.json` + `structural_chips.json` + `program_profiles.json`. `structural_chips.json` is already a machine-readable grammar for the structural/stack/vector layer (chips with `name`/`aliases`/`source_aliases`/arg types/selector slots/sub-ops). So coeff Help **needs no new extraction for vocabulary** — read the existing tables.
- **Param: op vocab dumps, statement grammar doesn't.** The sets (`:40-60`) dump cleanly; but `push`/`const`/`legacy`/`macro` (statement keywords — only `if name == "…"` literals), per-command arities, arg-kinds (selector vs expression vs target vs name), the call-vs-bare matrix, assignment syntax, and rejected forms are control flow. A set-dump misses ~70% of the grammar.
- **Locked in both:** the call-shape grammar (which forms each command accepts), arities, and rejection rules. No table encodes them.

### E. The unifying lever: a command-descriptor table

The single change that makes both editors data-driven, auto-documentable, **and** less redundant: a declarative **command descriptor** per source command — `{name, forms, arity, arg_kinds, bare_ok, targetable, emits, aliases, reject?}`.

- For **param**, put it in `profiles.param.source` (free on the frontend — §F). It then drives, from one source: (1) the lowerer (`_lower_call`/`_lower_bare`), (2) the reverse serializer (`param_source_text_from_chain`), and (3) the generated Help reference. Three hand-kept copies collapse to one.
- For **coeff**, the table largely **already exists** as `structural_chips.json`. The refactor is "have `_lower_call` consult it" instead of the hardcoded ladder — which also deletes `_STACK_ALIASES`/`_VECTOR_FILL_NAMES` and the roll/argsort/blend hardcodes.

This is also why the earlier param-profile / coeff-generator split is right: param's vocab belongs in the profile (it's small and absent there); coeff's vocab is already in `structural_chips.json`/registry, so the generator/parser should read *those*, not a re-typed profile copy.

### F. The generator path is already paved (no new infrastructure)

- `gen_program_profiles.py` is a **generic JSON→mirror**: `render_py`/`render_js` serialize the *entire* payload (`:30-31`, `:40-41`), so any new key under `profiles.param.source` flows verbatim into `program_profiles.py` **and** `program_profiles_js.js` (`window._programProfiles`). The frontend already reads arbitrary profile fields live (`js/07-transform-catalogs.js:447-451`). **Adding grammar to the profile is free on the frontend.**
- **Precedent: `root` and `solve_score` profiles already carry a `source` sub-block** (`program_profiles.json`). The `source`-block convention is established — param/coeff are catching up, not inventing.
- **Missing accessor:** there is no `profile_source(profile)` in `program_source_core.py` (it has `profile_selectors`/`profile_symbol`/`profile_symbols_with_context`). Add one before param/coeff grow `source` blocks.
- **Gating:** register any generator in `scripts/predeploy_check.sh:20-23` and add a drift test mirroring `tests/test_program_profiles_drift.py`. The pattern is established for all three existing generators.

### G. The correctness gate: execute the parser

Both parsers expose `compile_param_program_source` (`param_program_source.py:241`) and `compile_coeff_program_source` (`coeff_program_source.py:875`). So the audit (Rule 6) should **run the parser** on every article's `forms` (assert accept) and every `rejected_forms[].form` (assert reject, with the expected `code`). That covers exactly the statement-level grammar no table can encode (§D), and makes "Parser Wins" enforced by execution rather than by mirroring. Set-comparison against the profile/`structural_chips.json` is the fast frontend check; parser execution is the authoritative gate.

### H. Schema inconsistencies to fix in passing

The param and coeff profile blocks have drifted, which a uniform Help generator trips over:

- param has `selector_aliases` (`program_profiles.json:28-31`); coeff does not.
- `value_caps` keys diverge — param has `lowered_expr_tokens`, coeff has `legacy_int_arg` (`:14` vs `:45`); no documented "every profile must carry" contract.
- coeff symbols use extra keys (`from`) and context tokens (`expr_index_base`/`length_arg`/`rhs`) absent from param, with no schema validation — so a generator cannot assume a uniform symbol shape. Add a profile schema check alongside the `source`-block work.

### Net recommendation from the deep dive

1. **Migrate param onto `ProfileStatementLowerer`/`parse_profile_source`** (§A) — removes the param/coeff dispatch asymmetry and the hand-rolled statement loop.
2. **Make vocab single-source:** param → `profiles.param.source` (and derive the parser's `_BINARY_OPS`/etc. from it); coeff → have the parser *consume* `structural_chips.json` instead of re-hardcoding it; delete the JS re-hardcoded op lists (`js/07-transform-catalogs.js:445-446`) in favor of `window._coeffRegistryVocab`.
3. **Introduce the command-descriptor table** (§E) so the lowerer, the reverse serializer, and Help all read one source.
4. **Add `profile_source()` + a profile schema check** (§F/§H), register the generator in predeploy, add a drift test.
5. **Make the audit execute the parser** (§G).

Items 1–2 are the high-leverage refactors; they are *also* the prerequisites that turn the Help reference from "hand-transcribe the grammar" into "render the data." The rest of this document (article model, format, sections) is what you render *on top of* that data.

## Anti-Pattern Catalog: Self-Cancelling Complexity (the real bug class)

The `andy` mess is one instance of a class: **the code does something in one place, then adds more code on top to undo it.** Something is modelled badly — a flag where data belongs, an append where a declaration belongs, a forward transform with a hand-written inverse — and the rest of the system grows compensating special cases. Each entry below is a deletion target, not a feature. The refactor's success metric is how much code these deletions remove.

### Hard constraint governing AP-1 and AP-3: the wire layer is fingerprinted and permanent

Before touching `andy` or complex args, know this (verified): the compiled chain's byte layout **is** the fingerprint, and the fingerprint **is** the render-artifact cache key — `coeff_program_chain.py:912` ("Refs are fingerprinted wire format — this layer is permanent"), `:1841` ("the fingerprint hashes this string, so its exact byte layout is wire"), `:2118` (fingerprint = artifact cache key). So how `andy` is positioned and how complex args are packed are not just "storage" — they are cache-key bytes. **Re-encoding them changes the fingerprint of every existing program using those transforms, orphaning its cached render artifact.**

This forces a fork; this plan chooses (A):

- **(A) wire-compatible cleanup — chosen:** clean the *source syntax, parser, schema, frontend, and Help* freely, but the compiler emits **byte-identical wire** for existing programs. The fingerprint-equivalence corpus test (over real saved `calc.json` programs) is then **the** gate — not a checklist line, the thing that decides shippability.
- **(B) wire-breaking cleanup — explicitly out of scope:** re-encode the wire, accept fingerprint changes, ship an artifact migration / mass re-render. Much larger blast radius; do not drift into this accidentally.

Two consequences of (A), applied in AP-1/AP-3 below:

1. **There are two "canonical"s.** Canonical *source text* (`linear(2+3j, 0)`) — yes, change it. Canonical *wire* — no, it stays. `compat_signatures` is therefore not merely "old forms to read"; for fingerprinted functions it is **the declared permanent wire layout**.
2. **Packing is generalized, not deleted.** Something must still produce the existing wire from clean source; the win is collapsing the per-`fn_index` packers/unpackers/JS copies into **one data-driven codec** driven by that declared wire layout, not removing packing entirely.

### AP-1 · `andy` is a flag pretending not to be an argument

**State (verified):** `coeff_legacy_registry.json` has **28 functions, 0 with `andy` in `args[]`, all 28 with `supports_andy: true`.** `andy` is a boolean flag. Because it isn't a declared arg, three readers synthesize it and then strip it:

- JS appends a shared `_ctAndyParam` to every transform's params (`js/07-transform-catalogs.js:357,396,408`), with `kind:'andy'` as a marker other code filters/serializes/validates on.
- The source parser strips a trailing andy via per-`fn_index` arity sets `{3,5}/{3}/{2,3}/{declared+1}` (`coeff_program_source.py:205-226`, `_split_native_transform_andy`).
- The chain compiler strips it again via `_split_trailing_andy` + four `_*_legacy_args` helpers, each re-hardcoding the same arity sets (`coeff_program_chain.py:942,1264-1358`).

A blend amount — a parameter no different from `q` or `omega` — is appended in one layer and stripped in three, with its arity table copied three times.

**Fix:** `andy` is an **optional trailing argument**, declared by the registry argument schema like any other source-visible argument. The robust shape is not "keep `supports_andy` and append in JS"; it is:

- Add a normalized `effective_args` view at registry-load time.
- New registry data declares optional args directly (`{"name":"andy","type":"real","default":0,"optional":true,"role":"andy"}`). `andy` is a normal **trailing positional** optional arg — it already is one positionally (`_split_trailing_andy` strips `raw_args[-1]`), so it needs **no special `wire` field**; giving it one re-special-cases the thing being deleted. Its compiled position is unchanged (wire constraint above).
- Existing `supports_andy:true` is accepted only as a migration shim that materializes the same `effective_args` row, then becomes deprecated.
- The parser, chain compiler, serializer, frontend catalog, and Help all read `effective_args`; none of them special-case `supports_andy`.

Then delete `_ctAndyParam`-append, `_split_native_transform_andy`, and the four chain andy-splitters — the ordinary argument machinery handles it. For the functions whose with/without-andy arities are genuinely ambiguous (`linear`/`pow` `{3,5}`, `exp` `{3}`, `round` `{2,3}`), put the **accepted source signatures in the registry as data**, deleting the triplicated Python tables. Keep `role:"andy"` only as a UI/help label; it must not be load-bearing for parse/compile arity logic. Subject to the wire constraint: this is a schema/source/frontend cleanup; andy's compiled byte position stays, so fingerprints don't move. **Net: one declaration replaces a flag + an append + three strippers + three arity tables — without re-encoding the wire.**

### Two root causes underneath almost all of these

Before the list: nearly every entry below traces to **the registry/type model being too weak to say what the language actually means**, so code compensates:

- **The arg-type system can't express "optional" or "complex".** → `andy` becomes a flag (AP-1); complex args get packed into 4 reals (AP-3). Add `optional` and `complex` to the arg schema now, as part of this migration, and the compensating code deletes itself.
- **Param has no vocabulary generator.** Coeff and Solve-Score generate their JS from Python (`gen_coeff_vocab.py`, `gen_solve_score_vocab.py`); Param never got one, so its vocabulary is hand-typed in JS and has already drifted (AP-2, a live bug). Add `gen_param_vocab.py` and the hand-mirror + drift vanish.

### AP-2 · Param JS hand-mirrors the registry — and has silently dropped 22 of 70 functions (LIVE BUG)

**Verified:** `param_legacy_registry.json` has **70** functions; `js/07-transform-catalogs.js:96-145` `_paramProgramLegacyNames` lists **48**. The 22 missing — `add, adth, cadd, crdth, ddith, iadd, iscale, ldth, ndith, radd, rscale, scale, scdth, sdith, t1iadd, t1radd, t2iadd, t2radd, zz1, zz2, zz3, zzold` — **are accepted by the parser** (`param_program_source.py:173,189` accept any name in `legacy_registry()["by_name"]`; `scale`/`cadd`/`zz1` confirmed present) but the `legacy(...)` dropdown and Legacy-Transform help can't offer them. The whole param legacy vocab is re-typed in JS (`_paramProgramLegacyNames`, `_paramProgramLegacyArgSpecs` :176-202, `_paramProgramLegacyTargetArgIndexes` :158-174 — a copy of `param_program_chain.py:_LEGACY_TARGET_ARG_INDEXES`), with nothing keeping it in sync.

**Why:** Param is the only one of the three program languages with **no generator**. **Fix:** add `gen_param_vocab.py` (clone `gen_coeff_vocab.py`) emitting `window._paramRegistryVocab` from `param_legacy_registry.json` + `program_profiles.json`; hydrate the JS lists from it; add `test_param_program_drift.py` (mirror the coeff drift test). **Deletes ~110 lines of hand-typed JS and fixes the 22-function bug permanently.**

### AP-3 · Complex args packed into 4 reals because the type system can't say `complex`

**Verified:** the registry loader rejects anything but real/int/enum — `coeff_program_chain.py:319` *"v1 legacy bridge supports real, int, and enum args only."* So `linear/exp/pow/round`, which take complex `(multiplier, offset)`, store them as **four real components** and special-case the packing per `fn_index` in three places: forward packers `_affine_pair_legacy_args`/`_linear_legacy_args`/`_pow_legacy_args`/`_exp_legacy_args`/`_round_legacy_args` (`coeff_program_chain.py:1223-1342`); the inverse un-packers re-special-case the same fn_indices (`:2018/2034/2047`); and JS folds 4 reals → 2 complex a third time (`js/07-transform-catalogs.js:707-722`).

**Fix:** add a `complex` arg type to the loader and make complexness a property of the source argument, not a JS-only `complexWide` display hint. The existing compiled token representation already has the right lanes: `args`, `args_im`, and `expr_refs` are emitted by `_token()` and serialized by `_execution_spec()`, so they are fingerprinted wire. Complex values also use the scalar-expression token format when dynamic values are needed, with complex literals represented as `{op: EXPR_LITERAL, "a": re, "b": im}` (`coeff_program_chain.py:721,735`). Route complex *source* args through this existing representation. The old four-real component layout is **the declared permanent wire** for these fingerprinted functions (a `compat_signature`), not throwaway legacy — see the wire constraint above:

```json
{
  "name": "linear",
  "args": [
    {"name": "multiplier", "type": "complex", "default": "100"},
    {"name": "offset", "type": "complex", "default": "0"},
    {"name": "andy", "type": "real", "default": "0", "optional": true, "role": "andy"}
  ],
  "compat_signatures": [
    {"args": ["multiplier_re", "multiplier_im", "offset_re", "offset_im", "andy?"], "legacy": true}
  ]
}
```

Under wire-compatible cleanup (A): the **source** parser/serializer use the canonical complex form (`linear(2+3j, 0)`); the **compiler still emits the existing four-real wire** for these fingerprinted functions, driven by the `compat_signature` as the declared wire layout. So this **collapses** the five hardcoded packers + three inverse cases + two JS copies into **one data-driven complex↔wire codec** — it does not remove packing (the wire is permanent). Net is still a large deletion (~150+ lines of per-`fn_index` ladders → one codec), framed as "make packing data-driven and singular," gated by **byte-identical fingerprints over the saved-program corpus**.

Decision: the **source parser accepts both canonical complex forms and old packed real-component forms** through the same `compat_signatures` table. That is the least surprising migration: no saved `coeff_program_source_text` breaks, canonical serialization still emits the clean complex form, and Help presents the packed forms only as legacy-accepted compatibility notes, not as primary syntax. This keeps compatibility in one data-driven source instead of preserving ad hoc parser branches.

### AP-4 · Forward lowering and a hand-written inverse decompiler re-enumerate the same vocabulary

**Verified:** source→chain lowering and chain→source serialization are two independent hand-written tables over the same ops, in **both** languages. Coeff: `_lower_call` (`coeff_program_source.py:572-687`, ~37 dispatch arms) vs `coeff_source_text_from_chain` (`:975-1183`, ~27 arms) — each alias group (`{const,push_const,push_vec,fill}`, `{push_range,range,arange}`, …) spelled out on both sides. Param: `_lower_call` (`param_program_source.py:123-175`) vs `param_source_text_from_chain` (`:273-302`) — e.g. the `const`+`emit`→`p1 = expr` fusion is written forward at `:113-120` and inverted at `:281-284`. Add/rename an op and you must edit both or the round-trip drifts. **Fix:** drive both directions from one per-op descriptor (the command-descriptor table; for coeff, `structural_chips.json` already has the rows). Round-trip `source→chain→source` becomes the invariant test. **Deletes the smaller ladder each side (~120 lines coeff).**

### AP-5 · The JS client hand-ports the Python bridge canonicalizer it no longer needs

**Verified:** `js/08-chip-editors.js:1612-1799` is a line-for-line reimplementation of `param_program_chain.py`'s bridge canonicalization — `_migrateParamProgramLegacyTargetArgs` (`:1634`) ≡ `_migrate_legacy_target_arg` (`chain.py:466`); `_normalizeLegacyBridgeParams`/`_paramProgramMoebiusArgsForUi`/`_formatParamProgramComplexArg` ≡ their Python originals. But Param is now **text-only** (`_setParamProgramEditorMode` pinned to `'text'`, `js/08:349-354`) and the server already round-trips via `/compile-param-program-source` — so the client never needs to canonicalize a chain; it sends `source_text` and the server is the sole canonicalizer. **Fix:** delete the JS migrator/bridge stack; `param_source_text_from_chain` (server) is the only chain→text path. **Deletes ~180 lines.**

### AP-6 · Param re-lists the chain's op vocab, and `_lower_bare` duplicates `_lower_call`

**Verified (byte-identical):** `param_program_source.py:40-58` re-declares `_BINARY_OPS`/`_UNARY_OPS`/`_STACK_OP_ALIASES`/`_TARGETABLE_UNARY` with the same key sets the chain layer already owns in `param_program_chain.py:155-192` — purely to validate arity. And `_lower_bare` (`:178-191`) re-checks every bare-capable op (`push`, emit aliases, stack, binary, unary, legacy) that `_lower_call` already handles with empty args — it exists only because the core's `parse_call` returns `None` for a no-paren token. **Fix:** chain exposes one `OP_TABLE` (name → {opcode, arity, targetable, category}) that source reads; normalize bare idents to zero-arg calls so `_lower_bare` collapses into `_lower_call`. **Deletes the duplicated op sets + the whole bare ladder (~40 lines).**

### AP-7 · One selector list, five copies, guarded by a test that exists only because of the duplication

**Verified:** `['p1','p2','both','pop1','pop2']`/`['…','push1','push2']` lives in `program_profiles.json:23-24`, `param_program_chain.py:125-139` (`_SOURCE_SELECTORS`/`_TARGET_SELECTORS`), `js/07:265-266` (`_ppCatalog.legacy`), and `js/08:803-804` (`_paramProgramLegacyCallParams`) — and `test_param_program_drift.py:105-106` literally asserts `selectors == list(chain._SOURCE_SELECTORS)`, i.e. a test that exists *because* the same list is written twice in Python. The JS copies have **no** drift guard. **Fix:** derive `_SOURCE_SELECTORS`/`_TARGET_SELECTORS` from the profile (`profile_selectors(...)`), delete the second Python copy and its assertion; the JS copies come from AP-2's generator. **Deletes ~20 lines + a whole drift assertion.**

### AP-8 · `chip_name` is a third encoding of the "registry name is shadowed" fact

**Verified:** when a registry name collides with a typed-expr builtin (`exp`, `power`), the surface name (`exp_affine`, `power_series`) is encoded three ways that must agree: `aliases:[...]` (drives `TEXT_NAME_ALIASES`), a separate `chip_name` field (`coeff_legacy_registry.json:360,695`), and a hand-written ladder `_source_native_name` (`coeff_program_source.py:916-929`). **Fix:** drop `chip_name`; read the first `aliases` entry; `_source_native_name`'s branches become one `TEXT_NAME_ALIASES.get(name, name)`. **Deletes ~14 lines + a 2-field-must-agree JSON invariant.**

### Minor (real do-then-undo, small deletions)

- **`const(re,im)` → `"(re)+(im)*1j"` string → re-parsed** back to a complex literal it already had as two numbers (`param_program_chain.py:451-452,1037-1039` build the string; `_compile_expr` re-parses it). Build the literal directly. (~4 lines, one needless parse per legacy const.)
- **`peek`↔`tos` converted and un-converted inside one function** — `source_vector_from_selector` (`coeff_program_source.py:1019-1021`) maps `tos→peek` then `peek→tos` in the same body (comment admits the no-op). Pick one spelling. (~6 lines.)
- **`hi`/`lo` enum → `{0.0,1.0}` floats → re-detected** with a *hardcoded* inverse (`coeff_program_chain.py:1374` encodes via `_ENUM_ARG_VALUES`; `:2078` decodes via literal `"lo" if value==1 else "hi"`, not the inverse map). Invert `_ENUM_ARG_VALUES` once. (correctness hardening.)
- **`_chip_args`/`_chip_name_and_args` duplicated** across `param_program_chain.py:418` and `param_program_source.py:261`. Move one to `program_source_core.py`. (~10 lines.)

### Not findings — verified *good*, do not "fix" these

The sweep also confirmed patterns that look similar but are correct, so the refactor doesn't accidentally break them:

- **`-0`/whitespace canonicalization** (`coeff_program_chain.py:476,882`) — both sides converge to the same value *so they don't diverge*; there is no undo. Correct.
- **Derived alias maps** — `TYPED_BINARY_NAME_ALIASES`, `_VECTOR_BINARY_ALIASES`, `_NATIVE_TRANSFORM_ALIASES` are *derived* from the chain/registry (`coeff_program_source.py:87-90,132`) specifically to avoid drift. This is the target pattern, not a smell.
- **`ProfileStatementLowerer`/`parse_profile_source`** is the un-duplicated shared core; coeff uses it. Param joining it is the fix (AP-6), not a problem with the core.
- **The `legacy(...)` bridge existing at all** is a deliberate, pinned compatibility path (`param_program_chain.py:9-13`) — don't delete it.

## Current Source Files

### Frontend Help Builder

The current Help UI is built in:

- `js/08-chip-editors.js`

Important current functions:

- `_programHelpBuildParamRegistry()`
- `_programHelpBuildCoeffRegistry()`
- `_programHelpItemHtml()`
- `_renderProgramSourceHelp()`
- `_lookupProgramHelpToken()`
- `_onProgramSourceDblClick()`

These functions currently build both the Help tab and the double-click inspector.

### Param Data Sources

Param source grammar is authoritative in:

- `lambda/param_program_source.py`

Important grammar facts from that file:

- Assignments: `p1 = expr`, `p2 = expr`
- Push: `push()`, `push(t1)`, `push(t2)`
- Emit aliases: `emit_p1`, `emit_p2`
- `emit(p1)` is explicitly rejected as non-canonical.
- Stack ops: `dup`, `duplicate`, `swap`, `pop`, `flush`
- Binary ops: `add`, `subtract`, `sub`, `mul`, `ratio`, `div`
- Unary ops: `negate`, `conj`, `conjugate`, `reciprocal`, `unit_circle`, `square`, `cube`, `exp`
- Targeted unary form is valid: `square(p1)` and `square(p2)` both lower to a direct target mutation.
- Legacy call form: `legacy(name, src, tgt, ...)`
- Macro form: `macro(name)`

Param frontend catalogs are currently in:

- `js/07-transform-catalogs.js`

Important Param metadata:

- `_ppCatalog`: Param Program chip-ish metadata.
- `_paramProgramLegacyNames`: legacy Param transform names.
- `_paramProgramLegacyArgSpecs`: per-legacy-transform argument specs.
- `_ptCatalog`: legacy Param transform labels/categories/descriptions.
- `_ptInfo`: additional legacy transform descriptions merged into `_ptCatalog`.

Problem: `_ppCatalog` is not fully authoritative for source text. It currently claims some forms that contradict `lambda/param_program_source.py`. Help must not blindly trust `_ppCatalog` when the parser says otherwise.

### Coeff Data Sources

Coeff source grammar is authoritative in:

- `lambda/coeff_program_source.py`
- `lambda/program_source_core.py`

Coeff frontend catalogs are currently in:

- `coeff_vocab_js.js`
- `coeff_func_catalog_js.js`
- `js/07-transform-catalogs.js`
- `js/08-chip-editors.js`

Important Coeff metadata:

- `window._coeffRegistryVocab` from `coeff_vocab_js.js`
- `_coeffRegistryVocab.structuralChips.chips`
- `_coeffRegistryVocab.sourceAliasByName`
- `_coeffRegistryVocab.chipNameByRegistryName`
- `_coeffProgramLegacyNames`
- `_ctCatalog`
- `_ctAndyParam`
- `window._coeffFuncCatalog` from `coeff_func_catalog_js.js`

Important current convention:

- `_ctCatalog` appends `andy` to all registry transforms through `_ctAndyParam`.
- `_coeffProgramParamDefs(name)` is the correct accessor for transform params when available.
- Coeff function params come from `coeff_func_catalog_js.js`.

## Mechanics: How Help Data Should Be Collected

### Rule 1: Parser / Compiler Wins

If parser behavior conflicts with frontend catalog text, the parser wins.

Example: `_ppCatalog.push` says `choices: ['t1', 't2', 'both']`, but `lambda/param_program_source.py` accepts only `push()`, `push(t1)`, and `push(t2)`. Therefore Help must not document `push(both)` as valid.

Example: `_ppCatalog.emit` describes `emit(target)`, but `lambda/param_program_source.py` rejects `emit(p1)` and requires `emit_p1`, `emit_p2`, or assignment syntax. Therefore Help must not present `emit(target=p1)` as the canonical source form.

### Rule 2: Registries Provide Parameter Metadata, Not Final Help Text

Catalogs provide:

- Names
- Aliases
- Categories
- Parameter names
- Defaults
- Allowed choices
- Short descriptions

They do not automatically provide a useful help article. A help article still needs structure: signature, forms, parameters, stack effect, examples, constraints.

### Rule 3: Starter Snippets Are Not Help Entries

Starter snippets are examples. They are not reference documentation.

This is invalid Help behavior:

```text
square
Unary
Square the top stack value.
Insert
square(p1)
Unary
Targeted unary form: mutate p1 directly.
Insert
```

The correct Help entry is one logical reference entry:

```text
square
Forms:
  square
  square(p1)
  square(p2)

Effect:
  square        pop z, push z*z
  square(p1)   replace p1 with p1*p1
  square(p2)   replace p2 with p2*p2

Parameters:
  target optional, one of p1, p2

Examples:
  push(t1)
  square
  emit_p1

  square(p2)
```

### Rule 4: Lookup Entries and Help Entries Are Different

The double-click inspector needs a lookup map from token to help article.

The Help tab needs an ordered list of help articles.

These should share article objects, but they are not the same operation:

- Lookup: many tokens can resolve to one article.
- Help tab: one operation should appear once in the correct section.

Current code incorrectly lets cheat-sheet entries compete with reference entries. That is how sparse or misleading entries appeared before richer generated entries.

### Rule 5: One Operation, One Primary Help Article

If an operation has multiple forms, show them inside one article.

Do not render separate top-level entries for:

- `square`
- `square(p1)`
- `square(p2)`

Render one `square` article with all valid forms.

Do not render separate top-level entries for:

- `push()`
- `push(t1)`
- `push(t2)`

Render one `push` article with all valid forms.

### Rule 6: Generated Help Must Be Auditable

There must be an audit function or test that checks:

- Every Param source command known to the parser has a Help article.
- Every Param legacy transform has a Help article with `src`, `tgt`, and transform-specific args.
- Every Coeff source command known to the parser has a Help article.
- Every Coeff native transform has a Help article with `andy`.
- Every coefficient function with params has a Help article with all params.
- No Help article documents a form rejected by the parser.
- No Help article appears twice as separate contradictory entries.

The current tests check partial metadata matching, but they do not check quality, correctness against parser grammar, or duplicate/misleading presentation.

**Make the audit execute the parser.** A set-comparison against the grammar artifact catches op-vocabulary drift, but the statement-level grammar (command keywords, arities, arg-kinds, assignment, rejected forms) is control flow that no artifact fully encodes (Deep Dive §D). Both parsers expose `compile_param_program_source` (`param_program_source.py:241`) and `compile_coeff_program_source` (`coeff_program_source.py:875`), so the authoritative audit is a Python test that **runs the parser** on every article's `forms` (assert it compiles) and on every `rejected_forms[].form` (assert it raises, ideally with the expected `code`). That is "Parser Wins" enforced by execution rather than by mirroring, and it covers exactly the forms a set-dump misses.

## Correct Help Format

Each Help article should use the same shape.

### Required Fields

Every operation article should have:

- Name
- Category
- Summary
- Valid forms
- Parameters
- Effect
- Examples
- Notes / constraints
- Aliases, if any

### Field Details

#### Name

The canonical source-text name.

Examples:

- `push`
- `emit_p1`
- `square`
- `legacy`
- `sort_mod_keep_angle`
- `giga_139`

#### Category

Human-oriented category, not just raw registry category.

Good categories:

- Core symbols
- Assignment
- Input / output
- Stack
- Arithmetic
- Unary transform
- Legacy Param transform
- Coeff vector source
- Coeff scalar expression
- Coeff vector construction
- Coeff stack
- Coeff native transform
- Coeff function

Bad category output:

```text
Starters
Starters
Insert
```

#### Summary

One direct sentence. No generic filler.

Good:

```text
Squares the top stack value, or directly squares p1/p2 in targeted form.
```

Bad:

```text
Param Program chip.
```

#### Valid Forms

List every valid syntax form. This is the most important section.

For `push`:

```text
push()
push(t1)
push(t2)
```

Do not list `push(both)` unless the parser accepts it.

For `emit`:

```text
emit_p1
emit_p2
p1 = expr
p2 = expr
```

Do not list `emit(p1)` as valid source because it is rejected.

For `square`:

```text
square
square(p1)
square(p2)
```

For Param legacy:

```text
legacy(moebius, src, tgt, a, b, c, d)
```

For Coeff native:

```text
sort_mod_keep_angle(poly, andy=0)
```

#### Parameters

Each parameter row should show:

- Name
- Default
- Allowed values, if enumerable
- Type / expected expression kind
- Meaning

Example:

```text
src
  default: both
  choices: p1, p2, both, pop1, pop2
  meaning: source selector passed to the legacy transform

tgt
  default: both
  choices: p1, p2, both, push1, push2
  meaning: target selector receiving the legacy transform result

a
  default: 1
  type: complex expression
```

For Coeff native transforms:

```text
andy
  default: 0
  meaning: blend transformed output with original poly
```

#### Effect

Say what changes.

Examples:

```text
push(t1)
  Pushes input t1 onto the Param stack.

add
  Pops a and b; pushes a+b. The top of stack is b.

square(p2)
  Replaces p2 with p2*p2 without using the stack.

poly = rev(poly)
  Replaces the working coefficient vector with its reversed order.

emit
  Commits the top coefficient vector as output poly.
```

#### Examples

Examples should be minimal and valid.

They should not be the only source of syntax information.

Good:

```text
push(t1)
square
emit_p1
```

```text
square(p2)
```

```text
legacy(moebius, both, both, 1, 0, 0, 1)
```

Bad:

```text
square(p1)
```

if that is the only example for targeted unary, because it hides `square(p2)`.

#### Notes / Constraints

Use this for important traps.

Examples:

```text
emit(p1) is rejected. Use emit_p1 or p1 = expr.
```

```text
push(both) is not valid Param source text.
```

```text
andy is available on every registry-backed Coeff native transform.
```

```text
Coeff function parameters come from coeff_func_catalog_js.js and are function-specific.
```

## Correct Param Help Structure

Param Help should not display the starter cheat sheet. It should display these reference sections:

### 1. Param Source Overview

Explain:

- Param Program source transforms `t1`, `t2` into `p1`, `p2`.
- `p1` and `p2` are writable outputs.
- `t1` and `t2` are read-only inputs.
- Expressions can use constants/functions supported by the parser.
- Statements can be separated by newline or semicolon.

### 2. Core Symbols

Entries:

- `t1`
- `t2`
- `p1`
- `p2`
- `pi`
- `pi2`
- `pi2i`

Each symbol should say read/write status and where it can be used.

### 3. Assignment Forms

Entries:

- `p1 = expr`
- `p2 = expr`

Explain that assignment lowers to `const(expr)` then `emit_p1` or `emit_p2`.

### 4. Input / Output

Entries:

- `push`
- `const`
- `emit_p1`
- `emit_p2`

`push` must list only accepted forms:

- `push()`
- `push(t1)`
- `push(t2)`

`emit_p1` and `emit_p2` must be separate canonical entries or one combined `emit` article that clearly says the source forms are `emit_p1` and `emit_p2`.

Do not present `emit(target=p1)` as source syntax.

### 5. Stack Ops

Entries:

- `dup` / `duplicate`
- `swap`
- `pop`
- `flush`

Each needs stack effect.

### 6. Arithmetic

Entries:

- `add`
- `subtract` / `sub`
- `mul`
- `ratio` / `div`

Each needs operand order.

### 7. Unary Ops

Entries:

- `negate`
- `conj` / `conjugate`
- `reciprocal`
- `unit_circle`
- `square`
- `cube`
- `exp`

Each must show:

- Stack form: `square`
- Targeted form when valid: `square(p1)`, `square(p2)`

Do not show only `square(p1)`.

### 8. Macro

Entry:

- `macro(name)`

Explain what namespace `name` comes from.

### 9. Legacy Param Transforms

One article per legacy transform.

Every article must show:

- `legacy(name, src, tgt, ...)`
- `src` choices
- `tgt` choices
- transform-specific args from `_paramProgramLegacyArgSpecs`
- defaults
- short transform description from `_ptCatalog` / `_ptInfo`

Example:

```text
legacy(moebius, src=both, tgt=both, a=1, b=0, c=0, d=1)
```

## Correct Coeff Help Structure

Coeff Help should not display the starter cheat sheet. It should display these reference sections:

### 1. Coeff Source Overview

Explain:

- Coeff Program source transforms an input coefficient vector `cf` into output `poly`.
- `cf` is immutable input.
- `poly` is the writable/current coefficient vector.
- `poly_len` is available in expressions.
- `t1`, `t2`, `p1`, `p2` are scalar expression symbols from the Param stage.
- Statements can manipulate vectors, stack slots, and indexed coefficients.

### 2. Core Symbols

Entries:

- `cf`
- `poly`
- `poly_len`
- `tos[i]`
- `t1`
- `t2`
- `p1`
- `p2`

### 3. Coeff Statement Forms

Entries should cover real source grammar:

- `cf`
- `poly`
- `emit`
- `poly = expr`
- `poly[i] = expr`
- `push_vec(...)`
- `push_scalar(...)`
- `poke_poly(...)`
- `poke_tos(...)`
- `macro(name)`

### 4. Coeff Stack Ops

Entries:

- `dup`
- `swap`
- `drop`
- `flush`
- `pop`
- `peek`

Each needs vector/scalar stack effect where applicable.

### 5. Vector Constructors

Entries:

- `fill`
- `arange`
- `linspace`
- `littlewood`

Each needs argument names, defaults, and length behavior.

### 6. Vector Arithmetic / Structural Ops

Entries should be generated from:

- `_coeffRegistryVocab.structuralChips.chips`
- `_coeffFamilySubOps(...)`
- `_coeffRegistrySourceName(...)`

Each structural op needs:

- Canonical source name
- Aliases
- Valid source forms
- Parameters
- Effect

### 7. Coeff Native Transforms

Entries should be generated from:

- normalized coeff registry entries after the migration: names/aliases/categories/descriptions plus `effective_args`
- `_coeffProgramLegacyNames`, `_ctCatalog`, and `_coeffProgramParamDefs(name)` only as current pre-migration accessors

Each entry must include `andy` because every registry-backed native transform supports it.

Example:

```text
sort_mod_keep_angle(poly, andy=0)
```

### 8. Coefficient Functions

Entries should be generated from:

- `window._coeffFuncCatalog`

Each entry must include:

- Function name
- Parameter names/defaults
- Degree if relevant
- Kind/source
- Any available description

Example:

```text
giga_139(int1=251, int2=37, int3=619, deg=11)
```

The current function catalog has many entries, so this section needs filtering/search or collapsible grouping. Dumping 1100 entries as an undifferentiated scroll is technically complete but not good help.

## Inspector Behavior

Double-click should open the same article shown in Help.

Rules:

- Use current text selection first.
- Normalize `poly[0]` to `poly`.
- Normalize `sort_mod_keep_angle(...)` to `sort_mod_keep_angle`.
- Normalize `legacy(moebius, ...)` enough to resolve both `legacy` and `moebius`.
- Resolve aliases to the same article.
- Show missing-help explicitly when a token is unknown.

The inspector must not show a sparse starter entry if a generated reference article exists.

## How To Examine The Current Help

### In The Browser

1. Open the Compute tab.
2. Switch Param Program side panel to Help.
3. Search visually for `square`, `push`, `emit`, and `moebius`.
4. Switch Coeff Program side panel to Help.
5. Search visually for `sort_mod_keep_angle`, `rev`, and a coefficient function such as `giga_139`.
6. Double-click tokens inside the editor and compare inspector output against the Help tab.

### With Targeted Node Checks

Use the same frontend files the app loads and inspect generated registries.

The important checks are:

- Does `push` resolve to a valid Param source form?
- Does `emit` avoid documenting rejected `emit(p1)` syntax?
- Does `square` mention both `square(p1)` and `square(p2)`?
- Does every Param legacy transform include `src` and `tgt`?
- Does every Coeff native transform include `andy`?
- Does every coefficient function include its catalog params?

### With Tests

Current relevant test file:

- `tests/test_frontend_js.sh`

Useful future tests:

- Assert Help tab does not include starter section titles such as `Starters`.
- Assert Help tab does not render `Insert` buttons.
- Assert `push(both)` is not shown as valid Param source.
- Assert `emit(target=p1)` / `emit(p1)` is not shown as valid Param source.
- Assert `square` article includes `square`, `square(p1)`, and `square(p2)`.
- Assert every native Coeff transform article includes `andy`.
- Assert one operation appears once as a primary article, not as duplicate sparse/rich entries.
- Assert old packed Coeff source forms for `linear`, `pow`, `exp`, and `round` still compile through `compat_signatures`.
- Assert canonical Coeff source serialization for those transforms emits complex source forms, not packed real-component forms.
- Assert compiled wire/fingerprint equality before and after the optional/complex migration over a real saved-program corpus.

## Current Mistakes In The Help I Wrote

### 1. The Help Tab Is Still A Cheat Sheet Dump

The current Help builder adds `_paramProgramCheatSections` into `_programHelpBuildParamRegistry()`.

That is why Help displays:

```text
Starters
identity
Insert
mix
Insert
...
```

This is wrong. Starter snippets belong in Starter, not Help.

### 2. Help Entries Repeat Section Names As Metadata

Current entries show things like:

```text
square
Unary
Square the top stack value.
Insert
```

The repeated `Unary` label is not useful. It is layout noise. Category should be a heading or subtle tag, not a second line on every tiny card.

### 3. Insert Buttons In Help Are Mostly Wrong

Help is reference. Starter is insertion.

Some reference articles can have a small "Insert example" affordance, but every Help row should not end with `Insert`. That makes the Help tab feel like a duplicate button wall.

### 4. `square` Is Misleading

Current Help shows:

```text
square
Square the top stack value.
square(p1)
Targeted unary form: mutate p1 directly.
```

That is incomplete and misleading.

The parser accepts:

```text
square
square(p1)
square(p2)
```

If the Help shows only `square(p1)`, it implies `square(p2)` may not work. That is false.

### 5. Param Help Documents Catalog Params Without Checking Parser Validity

`_ppCatalog.push` currently says:

```js
choices: ['t1', 't2', 'both']
```

But the parser accepts only:

```text
push()
push(t1)
push(t2)
```

It rejects `push(both)`.

So the Help must not blindly render `_ppCatalog.push` as authoritative.

### 6. Param Help Risks Documenting Rejected Emit Syntax

`_ppCatalog.emit` describes:

```text
emit(target=p1)
```

But `lambda/param_program_source.py` explicitly rejects:

```text
emit(p1)
```

The canonical source syntax is:

```text
emit_p1
emit_p2
p1 = expr
p2 = expr
```

Any Help entry that implies `emit(p1)` is valid is wrong.

### 7. Cheat Snippets And Reference Entries Compete In The Same Lookup

The current implementation builds a lookup map from all Help items. When both sparse snippets and generated reference entries exist, one can shadow the other.

This already caused `sort_mod_keep_angle` to hide `andy`.

The code was patched to prefer richer entries in lookup, but that is not the right architecture. The right architecture is: do not put starter snippets into the Help reference list at all.

### 8. Coeff Help Is Metadata-Complete But Presentation-Poor

Coeff native transforms now resolve params correctly, including `andy`, but the Help tab still contains generated entries inside cheat-derived sections plus a separate native reference section.

That is redundant.

The user should see one coherent Coeff native transform section, not sparse/rich duplicates.

### 9. Coefficient Function Help Is Too Large Without Navigation

The function catalog has over 1000 entries.

Showing all of them in a flat list is not useful. It needs:

- Search
- Filtering by kind/source
- Collapsible groups
- Maybe top matches for the current token

Completeness without navigability is not help.

### 10. Descriptions Are Too Sparse

Many entries fall back to text like:

```text
Param Program chip.
Native coefficient transform.
```

That is not useful. If metadata lacks a real description, the Help generator should still synthesize a practical explanation from grammar/category/effect.

### 11. Stack Effects Are Missing

Stack languages need stack effects.

For example:

```text
add: (a b -- a+b)
square: (z -- z*z)
emit_p1: (z -- ), writes p1
```

Without stack effects, the Help is incomplete even if every parameter is listed.

### 12. Allowed Values Are Not Displayed Clearly

Parameters with `choices` should show those choices inline or in a parameter table.

Examples:

```text
target
  choices: p1, p2
```

```text
src
  choices: p1, p2, both, pop1, pop2
```

Current Help tends to show only `name=default`, which is not enough.

### 13. Aliases Are Not Presented Clearly

Aliases like these need explicit display:

- `dup` / `duplicate`
- `sub` / `subtract`
- `div` / `ratio`
- `conj` / `conjugate`

Current Help may resolve them, but it does not present them as aliases in a clear way.

### 14. Help Does Not Distinguish Source Forms From Lowered Chain Forms

Help must document what the user types, not the internal lowered chip chain.

This is the same principle as the earlier Populate/PDF bugs: user-facing text should be source text when source text exists.

### 15. The UI Format Is Not Scannable

The current Help output is a long vertical stream of tiny cards.

A useful help file needs hierarchy:

- Overview
- Section heading
- Operation article
- Signature
- Forms
- Params
- Effect
- Example

The current output is visually repetitive and hard to scan.

## Implementation Direction

### Current State Snapshot (verified against the code, 2026-06-24)

Where the code actually is today — full detail + `file:line` in the Deep Dive above:

- **Param parser bypasses the shared engine.** It does **not** use `ProfileStatementLowerer`/`parse_profile_source`; it hand-rolls `_lower_assignment`/`_lower_call`/`_lower_bare` + its own statement loop. Op vocab is hardcoded sets (`param_program_source.py:40-60`); it reads the profile only for symbols, not ops. No generated grammar artifact for Param exists.
- **Coeff parser already uses the shared engine** (`_CoeffStatementLowerer(ProfileStatementLowerer)` + `parse_profile_source`). Its vocabulary already lives in data — `coeff_legacy_registry.json` + `structural_chips.json` + the profile — and `gen_coeff_vocab.py` already emits it to `window._coeffRegistryVocab`. But `_lower_call` re-hardcodes that vocab instead of reading `structural_chips.json`, and the op-name lists are re-typed a **third** time in JS (`js/07-transform-catalogs.js:445-446`).
- **Both** parsers' *statement grammar* (command keywords, arities, arg-kinds, call-vs-bare, assignment, rejected forms) is control flow, not data — only the op-vocabulary is dumpable.
- **Infra is ready.** `gen_program_profiles.py` is a generic JSON→mirror, so any new `profiles.param.source` key is free on the frontend; `root`/`solve_score` profiles already carry `source` blocks (precedent); `compile_param_program_source`/`compile_coeff_program_source` exist (so the audit can run the parser). There is **no** `profile_source()` accessor yet, and the param/coeff profile schemas have drifted (param has `selector_aliases`, coeff doesn't; `value_caps` keys differ).
- **Frontend Help today** uses the flat `_programHelpItem(name, signature, help)` model (`js/08-chip-editors.js:714`) and folds the starter cheat sections into Help — the exact thing to undo (Mistakes #1, #7).

### Sequencing & Priority (refactor the language definition first)

The Help work exposed a real language-design problem: Param and Coeff source grammar is duplicated across parser control flow, frontend catalogs, starter snippets, reverse serializers, and tests. If Help is built on top of that as-is, it will just document the drift. So this ticket should include the refactor that makes the source languages data-backed and testable.

Scope boundary: make **vocabulary, valid forms, aliases, parameter metadata, optional/complex argument shape, and rejected forms** data-backed. Do **not** try to move semantic lowering into JSON. Semantics such as affine sugar, cf/poly staging, assignability checks, and the actual numeric meaning of `andy` remain code; whether `andy` exists, where it appears, and how it is omitted/defaulted are registry facts. **The compiled wire/chain bytes are out of scope for re-encoding**: they are the fingerprint and the artifact cache key (see "Hard constraint" in the Anti-Pattern Catalog), so this is a source/schema/frontend/Help cleanup that emits **byte-identical wire** for existing programs — enforced by a fingerprint-equivalence corpus test, not a wire migration.

1. **Param refactor first.** Extend `profiles.param.source`, add `profile_source()`, migrate Param onto `ProfileStatementLowerer`/`parse_profile_source`, and derive `_BINARY_OPS`/`_UNARY_OPS`/`_TARGETABLE_UNARY`/`_PUSH_SOURCES`/`_EMIT_ALIASES`/`_STACK_OP_ALIASES` from the profile. This removes the hand-rolled Param statement loop and makes Param grammar frontend-visible through `window._programProfiles`.
2. **Coeff arg-schema migration second.** Extend `coeff_legacy_registry.json` / `legacy_registry()` to support `optional:true`, `type:"complex"`, normalized `effective_args`, and explicit `compat_signatures`. This is the required cleanup that deletes the `andy` append/strip pattern and the per-function complex-arg pack/unpack tables. Keep accepting old packed source forms through `compat_signatures`; emit canonical complex/optional source forms; compile to byte-identical existing wire.
3. **Coeff vocabulary refactor third.** Make Coeff source vocabulary come from existing data (`structural_chips.json`, `coeff_legacy_registry.json`, `program_profiles.json`) instead of duplicated tables/lists. Delete the JS hardcoded vector op lists in favor of `window._coeffRegistryVocab`. Make `_lower_call` consult structural metadata for vocabulary/aliases/arity where possible, while keeping semantic lowering logic in Python.
4. **Coeff registry cleanup in the same pass.** Deprecate `supports_andy` after load-time migration to `effective_args`; add declarative accepted signatures where current source/chain logic duplicates fn-index arity rules. The parser and compiler must no longer branch on `fn_index` just to decide whether a trailing value is `andy`.
5. **Dead-code cleanup as part of the refactor.** Delete `_legacy_lower_statement` once the class lowerer remains the only path, remove unused Param `_INPUT_SYMBOLS`, and route `handler_storage.py` compile handling through the public `compile_param_program_source` wrapper instead of inlining its body.
6. **Then build Help on the new data.** The Help tab and inspector consume generated article objects. Starter remains separate. No Help article is hand-authoritative for valid syntax.

Regression gates are non-negotiable: parser round-trip tests, chain/fingerprint equivalence where applicable, source serializer tests, parser-execution Help audits, generator drift checks, and frontend runtime tests must all pass before the Help UI is considered shippable.

### Step 0: Land the data-backed grammar refactor

Before writing any article, make source grammar machine-readable to the frontend and consumed by the parsers where that is the single-source path (see "The Core Fix" above).

Param path:

- Extend the existing `profiles.param` entry in `lambda/program_profiles.json` with a `source` grammar block, and **derive the parser's `_BINARY_OPS`/`_UNARY_OPS`/`_TARGETABLE_UNARY`/`_PUSH_SOURCES`/`_EMIT_ALIASES`/`_STACK_OP_ALIASES` from it** so the parser and profile cannot disagree. This is the Param half of the symmetry refactor; do it together with the `ProfileStatementLowerer` migration.
- Include the Param expression namespace (`pi`, `pi2`, `pi2i`, `exp`, `real`, `imag`, `abs`, `mod`) in `profiles.param.source`, either by moving it into the profile or generating that subsection from `lambda/param_program_chain.py`.
- Add a `profile_source(profile)` accessor in `program_source_core.py` (it has `profile_selectors`/`profile_symbol` but not this).
- Migrate Param to `ProfileStatementLowerer`/`parse_profile_source`; keep lowering semantics equivalent and compare compiled chains/fingerprints on a corpus before deleting the old hand-rolled path.
- Regenerate `lambda/program_profiles.py` and `program_profiles_js.js`; confirm `lambda/gen_program_profiles.py --check` stays green; add a profile-schema check covering the new block.
- In the frontend, read `window._programProfiles.profiles.param.source`.

Coeff path:

- Treat `window._coeffRegistryVocab` as the existing generated vocabulary artifact. It already comes from `structural_chips.json`, `coeff_legacy_registry.json`, and `program_profiles.json`.
- Extend the coeff registry argument schema before building Help on native transforms:
  - accept `type:"complex"` in `legacy_registry()` and compile it through the existing token lanes (`args[i]`, `args_im[i]`, `expr_refs[i]`);
  - accept `optional:true` with defaults and trailing omission;
  - produce normalized `effective_args` for parser/frontend/help consumption;
  - materialize legacy `supports_andy:true` into an `effective_args` entry only as a migration shim, then stop using the flag in parser/compiler/frontend logic;
  - add `compat_signatures` for old packed real-component forms (`linear`/`pow` four-real, `exp`/`round` old component forms), so existing source payloads still compile while canonical source uses complex args;
  - declare the fingerprinted wire layout for each compatibility signature so the generic codec can emit the same execution spec bytes as the old per-function helpers.
- Replace `_split_native_transform_andy`, `_split_trailing_andy`, `_affine_pair_legacy_args`, `_linear_legacy_args`, `_pow_legacy_args`, `_exp_legacy_args`, `_round_legacy_args`, and the matching serializer/JS special cases with one generic optional/complex codec that is explicitly byte-compatible with the current wire. If any of those helpers cannot be deleted in the first pass, the doc/test must name the remaining compatibility reason explicitly.
- Refactor Coeff source lowering so vocabulary/aliases/arity are read from structural/registry metadata where possible. Keep semantic lowering logic in Python.
- Delete frontend hardcoded vector op lists in favor of `window._coeffRegistryVocab.structuralChips`.
- If Help still needs a normalized source-form layer not present in `_coeffRegistryVocab`, add `lambda/gen_program_grammar.py --write/--check` emitting `window._programGrammar.coeff` from structural/registry/parser metadata, register the `--check` in `scripts/predeploy_check.sh:20-23`, and add a drift test mirroring `tests/test_program_profiles_drift.py`. Use the single-underscore global convention (`_programGrammar`, **not** `__programGrammar`).
- Do **not** hand-copy coeff vocab into `profiles.coeff.source` while the parser keeps reading separate tables — that re-creates drift.

Do not start Step 3 until a valid form is something the code can *look up*, not something you assert. Everything below assumes this generated grammar artifact exists.

### Step 1: Separate Starter From Help Completely

Remove `_paramProgramCheatSections` and `_coeffProgramCheatSections` from Help generation.

Starter tab:

- Snippets
- Insert buttons
- Small examples

Help tab:

- Reference articles
- No mass Insert buttons
- Maybe one "Insert example" per article later, but not v1

### Step 2: Build Explicit Source Help Specs

Create a generated/intermediate help article model:

```js
{
  name,
  aliases,
  category,
  summary,
  forms,
  params,
  effect,
  examples,
  notes,
}
```

Do not use `_programHelpItem(name, signature, help)` as the only model. It is too flat.

### Step 3: Add Param Grammar-Derived Articles

Generate Param grammar-core articles from the generated grammar artifact:

- assignments
- push
- const
- emit aliases
- stack ops
- binary ops
- unary ops
- macro
- legacy call shape

The generated Param grammar supplies names, aliases, valid forms, targets, choices, expression namespace, and rejected forms. The hand-authored layer supplies only summary/effect/examples/notes keyed by grammar command name. Do not encode valid forms in prose.

### Step 4: Add Param Legacy Transform Articles

Generate one article per legacy transform from:

- `_paramProgramLegacyNames`
- `_paramProgramLegacyArgSpecs`
- `_ptCatalog`
- `_ptInfo`

Wrap each transform in the real source call shape:

```text
legacy(name, src, tgt, ...)
```

### Step 5: Add Coeff Grammar-Derived Articles

Create grammar articles for Coeff source statements and core symbols from `window._coeffRegistryVocab`, plus `window._programGrammar.coeff` if the normalized source-form layer is added.

Do not rely on starter snippets or `_coeffProgramCheatSections` for Help. They are examples only.

### Step 6: Add Coeff Native Transform Articles

Generate one article per native transform from:

- normalized coeff registry entries (`effective_args`, aliases, categories, descriptions)
- `_coeffProgramLegacyNames` / `_ctCatalog` / `_coeffProgramParamDefs(name)` only as the current pre-migration accessors

Every article must show `andy`.

Source of truth: this comes from normalized registry `effective_args`, not from a Help-only append. If `andy` is missing from an article after the migration, the registry/generator is wrong, not the article template.

### Step 7: Add Coeff Function Articles With Search

Generate coefficient function articles from:

- `window._coeffFuncCatalog`

But add search/filtering before exposing the full list. Otherwise the Help tab becomes an unusable wall.

### Step 8: Rebuild The Inspector On Top Of Articles

The inspector should resolve to the same article object used by Help.

No separate sparse lookup entries.

### Step 9: Add Quality Tests

Tests should check:

- Help does not contain `Starters`.
- Help does not contain bulk `Insert` buttons.
- `push(both)` is not documented.
- `emit(p1)` is not documented as valid.
- `square` documents `square`, `square(p1)`, and `square(p2)`.
- **Every documented `form` compiles and every `rejected_forms[].form` raises**, verified by executing `compile_param_program_source`/`compile_coeff_program_source` (the authoritative gate — Rule 6). Set-comparison against the grammar artifact is the fast secondary check.
- Article validity uses normalized form objects, not fragile string matching.
- All Param legacy transforms have `src`, `tgt`, and transform args.
- All Coeff native transforms have `andy` through normalized `effective_args`.
- Coeff registry schema accepts and round-trips `optional:true` args and `type:"complex"` args.
- Canonical Coeff source serialization emits complex args as complex arguments, not old real-component packs, when a transform has a complex schema.
- Old packed real-component compatibility signatures still compile for `linear`, `pow`, `exp`, and `round`.
- Old packed source forms are accepted only through declared `compat_signatures`; there must be no per-function parser branches for those cases.
- **The compiled wire/chain and its fingerprint are byte-identical before and after the migration for every program in the saved `calc.json` corpus.** This is the load-bearing gate — a fingerprint change orphans cached render artifacts (the wire is the artifact cache key). "Canonical serialization emits complex args" above means canonical **source text**, never canonical wire.
- No parser/compiler/frontend path branches on `supports_andy` except the load-time migration shim that creates `effective_args`.
- All coefficient functions with params expose those params.
- No duplicate primary article names within a section.

## Bottom Line

The current Help implementation is mechanically closer than before, but conceptually wrong.

It answers "what buttons can I insert?" more than "what language can I write?"

The correct fix is not more patching around lookup priority. It is also not "try harder to keep the prose in sync with the parser by hand" — that is the trap you are in. The correct fix is to make the parser grammar a generated artifact the frontend reads, then rebuild Help as a generated reference manual on top of it:

- **Generated grammar artifacts** define valid source forms, aliases, expression namespaces, and rejected forms, `--check`-gated so they cannot drift. Param adds `profiles.param.source` and consumes it in the parser. Coeff consumes existing structural/registry/profile metadata instead of duplicating vocab, with a thin generated source-form layer if `_coeffRegistryVocab` is not enough.
- Registries define names, params, defaults, choices, descriptions, optional args, complex args, and compatibility signatures. `andy` is not a separate frontend append or parser stripper; it is a normal optional argument in normalized registry data.
- Help articles use one model (`{name, aliases, category, summary, forms, params, effect, examples, notes}`): the grammar artifact/registry supplies the skeleton, you hand-author only the prose (summary, stack effect, notes), and a set-comparison test enforces forms ⊆ grammar and grammar ⊆ articles.
- One article object feeds both the Help tab and the inspector; coeff functions are lookup-only, not a rendered list.
- Starter remains separate.

Build the data-backed grammar refactor first (Step 0), and make the audit *run the parser* (Rule 6). Until valid-form is something the code looks up — and the parser executes in tests — instead of something you remember, every article you write is another mole to whack.

Sequencing reminder (from the Deep Dive): this is no longer just a Help UI patch. The Help tab is the forcing function to make Param and Coeff source languages less flaky: Param joins the shared lowerer/profile-driven grammar path; Coeff stops retyping structural vocabulary in parser/frontend code. Help ships after those grammar sources and gates are in place.
