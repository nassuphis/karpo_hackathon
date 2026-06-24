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

4. The frontend gets Param grammar through an existing generated asset: it already loads `program_profiles_js.js` as `window._programProfiles`. The Help builder should read `window._programProfiles.profiles.param.source` for Param valid forms/aliases/rejected forms. For Coeff, read `window.__programGrammar.coeff` from the generated grammar mirror in step 3b unless you intentionally choose the larger 3a Coeff parser refactor.

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
# render `window.__programGrammar = {...}` -> program_grammar_js.js; add `gen_program_grammar.py --check` to predeploy.
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
2. **Transforms** (param legacy / coeff native): generated from `_paramProgramLegacyArgSpecs` + `_ptCatalog`/`_ptInfo`, and `_ctCatalog` + `_coeffProgramParamDefs(name)` (andy included automatically — every registry transform already carries it).
3. **Coeff functions**: generated from `window._coeffFuncCatalog`. Render these **lookup-only** (search box + the double-click inspector), never as a 1000-row list — fixes #9 by removing the surface that needs navigation, not by navigating it.

**Two consumers, same articles:** the Help tab renders the ordered list of articles per section; the inspector resolves a token (and its aliases) to the **same** article object. There is no separate sparse lookup list — that separate list was the source of the `sort_mod_keep_angle`-hides-`andy` shadowing (#7).

**The seam test (Rule 6, now trivial):** per editor, assert `article.forms ⊆ grammar.forms(name)` (no article documents a parser-rejected form) and `grammar.commands ⊆ articles` (every parser command has an article). Do this on normalized form objects, not raw display strings, so formatting changes do not break the test. Example form objects: `{kind:"call", name:"push", args:["t1"]}`, `{kind:"bare", name:"emit_p1"}`, `{kind:"assignment", target:"p1", expr:"expr"}`. Both sides derive from the generated grammar artifact, so this is a set comparison, not a subjective content audit.

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

- `_coeffProgramLegacyNames`
- `_ctCatalog`
- `_coeffProgramParamDefs(name)`

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

### Step 0: Land the generated grammar artifacts first (the prerequisite)

Before writing any article, make source grammar machine-readable to the frontend (see "The Core Fix" above).

Param path:

- Extend the existing `profiles.param` entry in `lambda/program_profiles.json` with `source` grammar metadata.
- Derive Param parser operation sets from `program_profile("param")["source"]`.
- Include the Param expression namespace (`pi`, `pi2`, `pi2i`, `exp`, `real`, `imag`, `abs`, `mod`) in `profiles.param.source`, either by moving it into the profile or by generating that subsection from `lambda/param_program_chain.py`.
- Regenerate `lambda/program_profiles.py` and `program_profiles_js.js`.
- Confirm `lambda/gen_program_profiles.py --check` stays green.
- In the frontend, read `window._programProfiles.profiles.param.source`.

Coeff path:

- Add `lambda/gen_program_grammar.py --write/--check`.
- Emit `program_grammar_js.js` from `lambda/coeff_program_source.py` and `lambda/coeff_program_chain.py` tables, including `_STACK_ALIASES`, `_VECTOR_BINARY_ALIASES`, `_VECTOR_UNARY_NAMES`, `_NATIVE_TRANSFORM_ALIASES`, `VECTOR_BINARY_OPS`, `VECTOR_UNARY_OPS`, `SCALAR_UNARY_EXPR_OPS`, `TYPED_BINARY_NAME_ALIASES`, and `TEXT_NAME_ALIASES`.
- Register the `--check` in `scripts/predeploy_check.sh`.
- In the frontend, read `window.__programGrammar.coeff`.

Optional larger Coeff path:

- Instead of `program_grammar_js.js`, route Coeff parser operation vocab through `profiles.coeff.source` and derive the parser tables from the profile. Do this only if you are willing to refactor the Coeff parser/chain table boundary. Do not hand-copy Coeff vocab into `profiles.coeff.source` while the parser keeps reading separate tables.

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

Create grammar articles for Coeff source statements and core symbols from `window.__programGrammar.coeff` plus existing Coeff registries.

Do not rely on starter snippets or `_coeffProgramCheatSections` for Help. They are examples only.

### Step 6: Add Coeff Native Transform Articles

Generate one article per native transform from:

- `_coeffProgramLegacyNames`
- `_ctCatalog`
- `_coeffProgramParamDefs(name)`

Every article must show `andy`.

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
- Article validity is checked by normalized form objects, not fragile string matching.
- All Param legacy transforms have `src`, `tgt`, and transform args.
- All Coeff native transforms have `andy`.
- All coefficient functions with params expose those params.
- No duplicate primary article names within a section.

## Bottom Line

The current Help implementation is mechanically closer than before, but conceptually wrong.

It answers "what buttons can I insert?" more than "what language can I write?"

The correct fix is not more patching around lookup priority. It is also not "try harder to keep the prose in sync with the parser by hand" — that is the trap you are in. The correct fix is to make the parser grammar a generated artifact the frontend reads, then rebuild Help as a generated reference manual on top of it:

- **Generated grammar artifacts** define valid source forms, aliases, expression namespaces, and rejected forms, and are `--check`-gated so they cannot drift. Param uses `profiles.param.source` in `program_profiles.json`; Coeff uses `program_grammar_js.js` generated from parser/chain tables unless you intentionally refactor Coeff to read `profiles.coeff.source`.
- Registries define names, params, defaults, choices, descriptions.
- Help articles use one model (`{name, aliases, category, summary, forms, params, effect, examples, notes}`): the grammar artifact/registry supplies the skeleton, you hand-author only the prose (summary, stack effect, notes), and a set-comparison test enforces forms ⊆ grammar and grammar ⊆ articles.
- One article object feeds both the Help tab and the inspector; coeff functions are lookup-only, not a rendered list.
- Starter remains separate.

Build the grammar artifacts first (Step 0). Until valid-form is something the code looks up instead of something you remember, every article you write is another mole to whack.
