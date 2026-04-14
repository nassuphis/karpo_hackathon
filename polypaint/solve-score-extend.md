# Solve Score Extension Options

Status: design note only.

This document extends the current solve-score chip plan in:

- [chips-for-solve-scores.md](/Users/nicknassuphis/karpo_hackathon/polypaint/chips-for-solve-scores.md)

and the current compiler in:

- [lambda/solve_score_chain.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score_chain.py)

## Objective

There are two immediate extension goals:

1. move quantile into the chip model instead of keeping it as a separate slider
2. support combining multiple solve scores instead of exactly one metric plus optional `omega_cosine`

## Current Constraint

Today the backend contract is still scalar:

- one `solve_metric`
- one `solve_score_quantile`
- one optional final cosine transfer via
  - `solve_score_omega`
  - `solve_score_omega_enabled`

That means:

- putting quantile into the chip model is easy
- combining two scores is **not** just a frontend change

Combination breaks the current compile-to-scalar trick. Once two score maps exist, something backend-side must evaluate them.

## Question 1: Can Quantile Be A Chip Parameter?

Yes.

The cleanest v2 interpretation is:

- metric chips become parameterized
- example:
  - `spread(q=5.0%)`
  - `clusteriness(q=3.0%)`
  - `shelliness(q=1.0%)`

This fits the semantics better than a global slider because quantile really belongs to the metric evaluation.

## Quantile Modeling Approaches

### Approach A: quantile as a metric-chip parameter

Example:

- `spread(q=5.0%)`
- `omega_cosine(3)`

Pros:

- most natural mental model
- easiest UI once metric chips already support params
- future metric-specific params fit the same pattern
- still compiles cleanly to current scalar backend for single-metric chains

Cons:

- requires chip editor param UI for all solve-score metric chips
- migration needed from old `solve_score_quantile` scalar field

My recommendation:

- this should be the default direction

### Approach B: separate quantile chip

Example:

- `spread`
- `quantile(5.0%)`
- `omega_cosine(3)`

Pros:

- quantile is explicit as a pipeline step
- may generalize if later there are multiple normalization/clipping stages

Cons:

- less natural than attaching quantile to the score being clipped
- creates ordering questions
- adds grammar complexity with little immediate benefit

My judgment:

- possible, but worse than metric-owned quantile

### Approach C: keep global quantile outside the chain

Pros:

- smallest code delta
- keeps current backend mapping trivial

Cons:

- inconsistent model once chips are otherwise the source of truth
- awkward if different scores later need different quantiles

My judgment:

- acceptable only as a temporary compatibility layer

## Recommendation For Quantile

Use:

- `metric_name(q=...)`

and keep legacy scalar fields only as compiled compatibility output.

That means the canonical chain becomes something like:

- `spread(q=5.0%)`
- `omega_cosine(3)`

and the compiler emits:

- `solve_metric=spread`
- `solve_score_quantile=0.05`
- `solve_score_omega=3`
- `solve_score_omega_enabled=true`

## Question 2: How To Combine Two Scores?

Yes, this needs some form of expression or structured composition.

But that does **not** mean the first implementation needs a free-form text expression language.

There are several levels of power here.

## Combination Approaches

### Approach 1: dual-score UI with fixed combine operator

Model:

- Score A chip chain
- Score B chip chain
- one combine operator dropdown

Example UI:

- A: `spread(q=5.0%)`
- B: `shelliness(q=3.0%)`
- combine: `weighted_sum`
- weights: `0.7`, `0.3`

Possible combine ops:

- `avg`
- `min`
- `max`
- `mul`
- `weighted_sum`
- `abs_diff`
- `geometric_mean`

Pros:

- simplest usable multi-score system
- no parser
- no ambiguous precedence
- easy to explain and test
- enough for many real use cases

Cons:

- limited to exactly two branches
- becomes awkward if you want nested combinations

Backend implication:

- compute score map A
- compute score map B
- combine them in a new backend step

My judgment:

- strongest option if the immediate goal is "two scores combined"

Note:

- `geometric_mean` is worth including early
- it is often a better default than raw addition when combining differently scaled quality scores

### Approach 2: combine chip over previous two score nodes

Model:

- ordered chip program
- score chips push score maps
- combine chips consume prior maps

Example:

- `spread(q=5.0%)`
- `shelliness(q=3.0%)`
- `weighted_sum(a=0.7,b=0.3)`
- `omega_cosine(3)`

This is essentially a stack-machine or postfix program.

Pros:

- stays chip-native
- no text parser
- naturally extends beyond two metrics
- internal representation is simple and structured

Cons:

- less obvious to users than two named branches
- requires stack validation
- harder to present clearly in the UI

Backend implication:

- backend must evaluate a score-expression stack, not a scalar metric

My judgment:

- good long-term internal model
- maybe too abstract as the first user-facing multi-score UI

### Approach 3: explicit branch groups plus combine chip

Model:

- chips can contain grouped subchains

Example:

- `combine(weighted_sum, a=0.7, b=0.3)`
- branch A: `spread(q=5.0%)`
- branch B: `shelliness(q=3.0%)`
- final: `omega_cosine(3)`

Pros:

- clear tree structure
- easier to reason about than postfix
- maps well to an AST

Cons:

- much more UI work
- chip editor becomes nested

My judgment:

- architecturally clean
- too heavy for the first extension

### Approach 4: small text expression language

Example:

- `0.7*spread(q=5.0%) + 0.3*shelliness(q=3.0%)`
- `max(spread(q=5.0%), shelliness(q=3.0%))`
- `omega_cosine(3, avg(spread(q=5.0%), area(q=2.0%)))`

Pros:

- very expressive
- compact for power users
- easy to serialize as text

Cons:

- parser, validation, error messages, precedence, and editor UX all become real work
- harder to keep safe and constrained
- not aligned with the chip-first UI direction

My judgment:

- do not start here
- maybe add later as an advanced mode that compiles to the same AST

### Approach 5: JSON AST as canonical backend format, chips as editor

Model:

- frontend still presents chips
- canonical saved representation is a structured tree

Example:

```json
{
  "kind": "weighted_sum",
  "weights": [0.7, 0.3],
  "inputs": [
    {"kind": "metric", "name": "spread", "quantile": 0.05},
    {"kind": "metric", "name": "shelliness", "quantile": 0.03}
  ],
  "final": {"kind": "omega_cosine", "omega": 3.0}
}
```

Pros:

- explicit
- versionable
- excellent for backend validation
- compatible with both chip UI and future text-expression UI

Cons:

- more implementation work up front than the current flat chain
- needs migration path from current v1 array-like chain

My judgment:

- best canonical representation once multi-score composition is real

## Does This Require A Real Expression Language?

Not immediately.

If the real near-term ask is:

- "take score A"
- "take score B"
- "combine them with one operator"

then a structured dual-branch model is enough.

A real expression language becomes necessary only when you want things like:

- three or more metrics
- nested operators
- reuse of subexpressions
- arbitrary precedence

So the progression can be:

1. parameterized metric chips
2. two-branch combine model
3. optional internal AST only if RPN becomes limiting
4. optional text expression syntax later

## Backend Execution Approaches

Once multiple scores are supported, the backend has to evaluate more than one score map.

There are several ways to do that.

### Backend A: compute two full score maps, then combine

Flow:

1. compute map A
2. compute map B
3. combine arrays
4. apply final transfer

Pros:

- simple
- easy to validate
- easiest first implementation

Cons:

- more IO and temp data
- repeated passes over the same solve data

My judgment:

- best first backend for two-score support

Practical note:

- the extra IO here is likely acceptable for the first version
- the solve itself dominates the cost
- do not optimize to single-pass native evaluation before real usage patterns justify it

### Backend B: extend native solve-score engine to evaluate the composed AST in one pass

Pros:

- fastest long term
- avoids extra intermediate files

Cons:

- much larger native change
- more coupled rollout

My judgment:

- phase 2 or 3 work, not first extension

### Backend C: hybrid

Flow:

- backend evaluates multiple scalar metrics one at a time
- Python orchestration combines them
- later move the combine logic into native code if needed

Pros:

- good delivery path
- keeps risk down early

Cons:

- transitional duplication

My judgment:

- pragmatic

## Suggested User-Facing Syntax Options

If the UI remains chip-based, these are the best concrete options.

### Option 1: single chain with parameterized metric chip

Example:

- `spread(q=5.0%)`
- `omega_cosine(3)`

This is the recommended next step even without multi-score support.

### Option 2: two score groups + combine panel

Example:

- group A: `spread(q=5.0%)`
- group B: `shelliness(q=3.0%)`
- combine: `weighted_sum(0.7,0.3)`
- final: `omega_cosine(3)`

This is the recommended first multi-score UI.

### Option 3: postfix chip chain

Example:

- `spread(q=5.0%)`
- `shelliness(q=3.0%)`
- `weighted_sum(0.7,0.3)`
- `omega_cosine(3)`

This is the recommended internal model if the system outgrows the two-group UI.

### Option 4: advanced expression text box

Example:

- `weighted_sum(0.7, spread(q=5.0%), 0.3, shelliness(q=3.0%))`

This should be optional and advanced, not the first design.

## Recommended Rollout

### Phase 1: explicit RPN chip UI for testing

Make phase 1 a literal postfix/RPN chip editor.

Meaning:

- metric chips push one score map
- binary combine chips pop two score maps and push one
- unary final-transfer chips pop one score map and push one
- quantile lives on the metric chip itself

Example:

- `spread(q=5.0%)`
- `shelliness(q=3.0%)`
- `weighted_sum(0.7,0.3)`
- `omega_cosine(3)`

This should be treated as the canonical execution model, not just a temporary hack.

Phase 1 UI should include:

- visible ordered chip list
- live stack-depth indicator
- visual stack-state display
  - example: a tiny vertical column of colored blocks that grows and shrinks with stack depth
  - stack underflow should be visible without requiring the user to reason about integer deltas
- per-chip arity / effect hint
  - examples:
    - `metric: +1`
    - `weighted_sum: -1`
    - `omega_cosine: 0`
- immediate invalid-stack rejection
- final validity rule:
  - the program must end with stack depth exactly `1`

Why do this first:

- it proves the semantics with the least frontend work
- it gives a direct way to test multi-score composition early
- it avoids premature work on a friendlier translator UI before the execution model is stable

### Phase 2: quantileized metric chips on the scalar-compatible single-score path

Do:

- change metric chips from `spread` to `spread(q=5.0%)`
- keep `omega_cosine(omega)` as final transfer
- preserve scalar compile output for backend compatibility

Do not:

- add multi-score combination yet

This is the compatibility bridge for places still using the old scalar backend contract.

Practical sequencing note:

- phase 1 and phase 2 are small and independent enough that they can ship together
- that would give:
  - immediate RPN testing
  - immediate removal of the separate quantile slider

### Phase 3: add structured two-score composition UI on top of RPN

Do:

- add Score A and Score B groups
- add one combine operator and optional weights
- compile that friendlier UI down to the same RPN program underneath

Do not:

- add arbitrary nested expressions yet

This means the user-facing UI can improve later without changing:

- execution semantics
- validation semantics
- storage format

### Phase 4: optional AST layer, only if RPN starts to hurt

Do:

- store a tree/AST format in metadata
- let the chip UI edit that AST
- keep scalar legacy fields only where needed for old readers

This phase is optional. RPN already serializes expression structure well enough for the likely near-term needs.

Revisit only if one of these becomes real:

- named subexpressions
- conditionals
- lazy evaluation
- tooling that genuinely wants tree editing more than sequence editing

### Phase 5: optional advanced expression syntax

Do only if needed:

- add text formula input for power users
- compile it to the same RPN program or AST

Do not do this just because it sounds elegant.

This should stay deferred until there is repeated real demand for:

- manual typed formulas
- nested expressions that are painful in chip form
- copy/paste of score programs as text

## Recommendation Summary

My recommendation is:

1. yes, move quantile into the metric chip
   - `spread(q=5.0%)`
2. yes, make phase 1 a real RPN chip UI for power-user testing
3. do **not** start with a free-form expression language
4. if you want a friendlier UI later, add:
   - Score A
   - Score B
   - combine operator
   - optional weights
   and compile that to the same RPN program
5. consider AST later only if RPN storage becomes limiting

That gives:

- a better chip model immediately
- a practical early testbed for combining two scores
- a clean separation between execution model and UX model
- no premature parser work
- a clean long-term architecture
