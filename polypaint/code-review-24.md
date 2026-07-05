# Code Review 24: The Coeff Program Path — State Machines, Language Design, Missing Features

## Objective

Deep dive on the Coeff Program code path: source text → compiler → wire format → native VM → coefficient vectors. Map every state machine on the path, document the language as it actually exists (verified against source, not docs), and propose language features that would make programs more concise and unlock more translations of existing coefficient functions.

**Method.** Every claim below was verified by reading the implementation (`lambda/coeff_program_source.py`, `lambda/program_source_core.py`, `lambda/coeff_program_chain.py`, `lambda/sweep_cli.c`) or by running code through the real pipeline. The empirical corpus is the poly100 porting batch (2026-07-05, `lambda/port_poly100_programs.py`, commit `1d8d377`): 99 poly100.py functions pushed through analyze → emit → compile → native parity; **25 ported, 74 skipped with machine-classified reasons**. That batch is the best available measurement of what the language can and cannot express, and the feature proposals in §5 are prioritized by its counts.

Reviewed at `1d8d377` (2026-07-05).

---

## 1. The code path, end to end

```
source_text (≤64KB, MAX_COEFF_PROGRAM_SOURCE_BYTES, coeff_program_source.py:64)
  │  split_program_statements          program_source_core.py:291   [SM-1]
  ▼
SourceStatement records (text, line, column)
  │  _CoeffStatementLowerer            coeff_program_source.py:871  [SM-2]
  │    scalar exprs via ExpressionParser  coeff_program_chain.py:756 [SM-3]
  ▼
canonical chip chain  (≤256 entries at save, handler_storage.py:651; ≤32KB JSON)
  │  _expand_macros                    coeff_program_chain.py:1690  [SM-4]
  │  _compile_chain → tokens + scalar_exprs side table
  │  _validate_stack                   coeff_program_chain.py:1854  [SM-5]
  ▼
compiled payload {version, fingerprint, tokens, stack_max, scalar_exprs}
  │  _execution_spec (FROZEN wire text) coeff_program_chain.py:2029
  │  _fingerprint = sha1(version+spec)  coeff_program_chain.py:2168 → artifact cache key
  ▼
S3 saved program (handler_storage._compile_coeff_program_payload:632, re-compiled on load)
or compute request → native runtime
  │  parseCoeffProgram (token cap check sweep_cli.c:3980)
  │  load-time lowering of scalar exprs → CoeffLoweredExprPlan (expr_plans)
  ▼
per-row: evalCoeffProgram              sweep_cli.c:5291             [SM-6]
  │  dispatch switch in COEFF_OP_* enum order  sweep_cli.c:5324
  │  scalar-expr mini-VM in its own arena      [SM-7]
  ▼
poly register → f32 output rows (coeffgen) / solver input (compute)
```

**The length contract** (subtle, load-bearing): the coeffgen path first runs the program once on a probe point with no length check (`sweep_cli.c:8099`, `expectedOutLen=-1`), adopts the probe's output length as `nCoeffsOut` (`:8111`), sizes buffers from it, and then requires every subsequent row to produce exactly that length (`:5375` "output length changed"). So a program **may** change the degree relative to the base function — `poly = fill(36, 0)` on a degree-5 const base is legal — but must do so **consistently across rows**. A program whose output length depends on parameter values fails at the first divergent row.

**Per-row reset invariant** (`sweep_cli.c:5300`): only cursors and lengths reset between rows (`stack_depth`, `stack_head`, `poly_len`, `scratch_len`, `original_len`); the vector planes keep stale bytes, and every live read is preceded by a write. `poly` is seeded by copying the base coefficient function's output. The typed stack must be empty when the program ends (`:5371`) — leftover pushes are a hard row error, which is why generated code never strands values.

**Two scalar-parameter namespaces**: in program expressions, `p1/p2` are the *post-param-program* parameter values and `t1/t2` are the *raw grid coordinates* — the sweep loop passes `(z1, z2, x1, 0, x2, 0)` at `sweep_cli.c:8171`. Programs can mix both; the C coefficient functions only ever saw one.

---

## 2. State machine inventory

Seven distinct machines run on this path. They are individually small and well-guarded; the interesting property is how work is front-loaded so the hot per-row machine (SM-6) stays lean.

### SM-1: statement splitter (`program_source_core.py:291`)

Character-level FSM with state = `{in_comment} × paren_depth × bracket_depth`. Transitions: `#` enters comment (to EOL); `;` and `\n` flush a statement only at depth 0 (parenthesized statements span lines); `(`/`)`/`[`/`]` adjust depth with immediate underflow errors carrying line/column. Notable: **no string-literal state** — the grammar has no strings, so `#` inside anything is always a comment. This FSM is hand-mirrored in JS (`js/08-chip-editors.js:2433`, `_coeffProgramSourceStatements`) for live statement counting — a three-way grammar surface (Python/C/JS) discussed in §6.

### SM-2: statement lowerer (`coeff_program_source.py:871`)

Dispatch over four statement shapes, in order: **indexed assignment** (`poly[expr] = expr` → index-expr chain + value chain + `_typed_poke_poly`; write targets restricted to `_WRITABLE_LHS_NAMES` = poly), **symbol assignment** (`poly = rhs` where rhs is a call → `_lower_call(target=poly)`, a bare source name → `["set", poly, src]`, a bare identifier → zero-arg call, else typed value lowering), **call statement** (bare `sin(pop)` → `_lower_call(target=push)`), **bare statement** (push-source names, stack aliases `dup/duplicate/swap/drop/flush/emit`; bare `pop`/`peek` rejected with a hint). Selector vocabulary (from `program_profiles.json` via `structural_chips.json`, single source shared with the chip UI): sources `cf|poly|pop|peek`, typed vector sources add `tos`, targets `poly|push`.

### SM-3: scalar expression parser (`coeff_program_chain.py:756`)

Recursive descent (`_expr → _term → _power → _unary → _primary`) over a regex token stream (numbers with `i/j` imaginary suffix, identifiers, `** ( ) [ ] + - * /`). Output is a *postfix token program* (the EXPR_* opcodes), capped at `MAX_SCALAR_EXPR_TOKENS = 32`. Three deliberate rules with teeth:

- `**` requires an **integer literal** exponent, |e| ≤ 32, and unrolls to repeated multiplication *at compile time* (`:877`). Consequence: scalar `**` matches numpy's integer-power semantics bit-for-bit; the vector `power()` op does not (§3.4).
- Static subexpressions constant-fold to a single literal push so text- and chip-authored constants fingerprint identically (`coeff_program_source.py:330`).
- Static division by zero is a compile error, but folding is *wire-preserving only* — emitted tokens stay byte-identical for valid programs.

Identifier namespace: `p1 p2 t1 t2 poly_len`, indexed reads `cf[i] poly[i] tos[i]` (static and dynamic index), constants `pi pi2 pi2i tau tau_i`, unary functions `conj neg real imag abs/mod log sqrt exp sin cos tan sinh cosh tanh angle`.

### SM-4: macro expander (`coeff_program_chain.py:1690`)

Budgeted DFS: depth ≤ 8, cycle detection via an explicit name stack, and a *shared* budget (≤256 chips, ≤256 macro calls cumulative) so nesting cannot amplify before the post-expansion token check. Macros resolve by **name only** — `macro(saved-id)` splices another saved program's chain; there are no parameters (§5 P10).

### SM-5: compile-time abstract stack interpreter (`coeff_program_chain.py:1854`)

The quiet star of the pipeline. It symbolically executes the token stream over the abstract type lattice `{scalar, vector}`: every op's stack effect is modeled (pushes, pops, `swap`, `blend` popping two vectors and pushing one, native transforms popping `stack_arg_count` *scalars* then their vector source), underflow and type mismatches are compile errors, and `stack_max` is computed for the VM's preallocation. This is why the porting tool's mistakes surfaced as compile-time messages ("vector src1 at token 4: top of stack is scalar (need vector)") rather than native crashes. The C VM re-checks everything at runtime (defense in depth), but no valid compile ever trips those.

### SM-6: the native VM (`sweep_cli.c:5291`)

A stack machine over a preallocated ~1.1MB workspace (`sweep_cli.c:3555`):

| register | size | role |
|---|---|---|
| typed stack | 64 slots × 256 complex + parallel scalar planes + `stack_type[]` tags | operands; scalar slots live *beside* vector planes (push_scalar leaves stale vector bytes) |
| `poly` | 256 complex | the mutable output register, seeded from the base coefficient function |
| `scratch` | 256 complex | per-op working vector, "clobbered by nearly every op — never holds state across ops" |
| `original` | 256 complex | pre-transform snapshot for andy blending AND second operand of vector binary/argsort |
| `aux` | 256 complex | roll/argsort permutation temp |
| expr arena | 96 numbers | scalar-expr evaluation, invisible to the user stack |

Dispatch is a single switch in enum order (`:5324`, 29 opcodes). Broadcast in typed binary ops is symmetric (scalar⊗vector both orders, `:4610`); vector-vector length mismatch is a row error. Exit invariants: empty stack, `poly_len ∈ [1,256]`, row-consistent length.

### SM-7: scalar-expr mini-VM

Separate opcode set `COEFF_EXPR_*` (`sweep_cli.c:3478`, 31 opcodes), hand-duplicated from Python's `EXPR_*` and **drift-pinned** by `tests/test_coeff_program_drift.py`, which regex-parses the C enums and compares constant-by-constant. Expressions ship as token lists in the payload but are lowered *once at program load* into `CoeffLoweredExprPlan` and evaluated per token in the private arena — dynamic chip arguments never touch the user-visible stack.

Adjacent machinery worth naming: the **RNG state machine** (xorshift64, re-seeded per pass at `:8145`, plus a per-row `evalSeed` for littlewood determinism) and the **enum-parity drift guards** — the reason the hand-duplication across Python/C hasn't rotted.

---

## 3. The language as it stands

### 3.1 Statement forms (all verified compiling)

```
poly = <vector expr>              # set: whole-vector assignment
poly[<scalar expr>] = <scalar>    # poke (index may be dynamic)
<call>(...)                       # bare call, result pushed
push_range(a, b[, step])          # arange semantics; negative step supported (sweep_cli.c:5019)
fill(n, v) / const / push_vec     # constant vector (aliases, one op)
linspace(a, b, n)
dup | swap | drop | flush | emit  # stack ops (aliases → duplicate/pop)
legacy(name, src, tgt, args...)   # 28 registry native transforms (rev, shifted-class ops, …)
macro(saved-id)                   # splice another saved program
# comment                         # to EOL, mid-line OK; parens span lines
```

### 3.2 The RPN layer

Bare calls compose through the typed stack: `sin(pop)`, `multiply(pop, pop)`, `poly = pop`. Three rules a program author (or code generator) must know, all verified empirically during the porting batch:

1. **`pop` in an argument slot must pop a vector** (`_typed_push_vector pop` → `coeff_stack_require_vector`). Scalars cannot ride the stack through call arguments; they fold inline.
2. **Arguments lower left-to-right**, so the leftmost `pop` binds top-of-stack: `subtract(pop, pop)` = (last pushed) − (first pushed). Safe generated style: commutative `add(pop, pop)` plus `neg()`.
3. **The call-prefix trap** (`program_source_core.py:283`): an argument that *starts* with `name(` but continues with operators — `log(x) * 1i` — is treated as one call and fails "missing closing parenthesis". Reorder (`1i * log(x)`) or parenthesize.

### 3.3 Numeric dialect — the translation-fidelity contract

This section exists because "translate poly_N faithfully" is exactly where the dialect differences bite. All items verified, several through parity failures:

- **Per-op non-finite clamp**: every vector binary/unary result with non-finite re or im clamps to 0 (`sweep_cli.c:4459`). Consequence: masked garbage (`inf × 0 → nan → 0`) is safe, but numpy-side overflow propagates `inf` where the VM silently zeros. poly100's `try/except → zeros` has no VM analogue.
- **Division asymmetry**: elementwise vector divide is forgiving (0 on zero denominator, `c_div_full`); *scalar-expression* division by zero **errors the row** — deliberately mirroring the compiler's static-fold rejection (comment at `:4448`).
- **Poke asymmetry**: `poke_poly` with a non-finite *value* errors the row (`:5180`) — a scalar fixup of an overflowed expression kills the row where a vector op would have clamped.
- **Two power semantics**: scalar `**` (integer literal) unrolls to repeated multiplication at compile time — matches numpy integer powers exactly, including signed-zero results. Vector `power()` is `c_powc` = `exp(b·log a)` unconditionally (`sweep_cli.c:2603`), principal branch. Measured consequence (poly_67): numpy `z**62` yields an exact `-0.0` real part where c_powc leaves ~1e-14 phase jitter; amplified by |z|⁶² ≈ 4e18 that's ~8e4 absolute on an ill-conditioned slot — invisible at row scale (3.6e-8 row-relative), fatal to naive per-value comparison.
- **Chaos is unportable**: `cos(x)` with |x| ~ 1e22 (poly_6, poly_32: `angle(t)**k` fed into trig) is noise on *both* sides — 1 ulp ≫ 2π — and they disagree. No language feature fixes this; the porting gate correctly refuses.
- **f32 transport**: chunked params arrive as float32 and output rows leave as float32 — saturation above 3.4e38 (poly_1 beyond |p|≈1.4 legitimately produces ~1e49 doubles that become `±inf` on the wire), and ~6e-8 relative floor. Native math is double throughout.
- **`sqrt` special case**: `c_sqrt_c` has an exact branch for negative reals (`:2612`) because the exp/log path left a ~1e-16 residue diverging from compiler static folds — precedent for the P7 proposal.
- **Frozen number formats**: `canonical_number_g17` (coeff v1) vs `canonical_number_repr` (root/solve-score) — both documented as FROZEN wire formats (`program_source_core.py:204`); fingerprints hash formatted numbers.

### 3.4 Limits (all verified at source)

| limit | value | where |
|---|---|---|
| VM tokens (post-macro) | **256** | `MAX_PROGRAM_TOKENS` py:28 == `COEFF_PROGRAM_MAX_TOKENS` C:3378, fixed struct array |
| chain entries at save | 256 | `MAX_COEFF_PROGRAM_STATEMENTS`, handler_storage.py:651 |
| vector length | 256 | `COEFF_PROGRAM_MAX_VECTOR_LEN` C:3380 |
| stack slots | 64 | `COEFF_PROGRAM_MAX_VECTOR_STACK` C:3379 |
| scalar exprs / program | 64 | `COEFF_PROGRAM_MAX_SCALAR_EXPRS` C:3382 |
| tokens / scalar expr | 32 | `MAX_SCALAR_EXPR_TOKENS` py:34 (C arena: 96 nums / stride) |
| args / token | 8 | `COEFF_PROGRAM_MAX_ARGS` C:3381 |
| macro depth / budget | 8 / 256+256 | py:32, :1698 |
| source bytes / chain JSON | 64KB / 32KB | py:64, handler_storage.py:143-152 |

---

## 4. Empirical corpus: what the language expresses today

The porting batch is a 99-function stress test of translation power. Results:

**Ported (25)**: poly_4, 8, 12, 17, 23, 25, 30, 31, 34, 35, 36, 38, 40, 52, 57, 67, 71, 76, 83, 84, 88, 90, 94, 98, 99 — 31 to 249 tokens each, 24 on the strict parity gate (~5e-8 = wire floor), poly_67 on the row-relative gate. Plus hand ports poly_1 v1/v2.

**Skipped (74), by machine-classified reason:**

| count | reason | language gap? |
|---|---|---|
| 13 | recursive: loop reads cf (`cf[k-1] = f(cf[k-2])`) | **yes — no recurrence primitive** (P1) |
| 11 | per-k `if/else` in loop body | **yes — no comparisons/select** (P3) |
| 10 | read-modify slices (`cf[a:b] = f(cf[a:b])`) | **yes — no slice read/write** (P2) |
| ~8 | `np.sum` / `np.prod` tails | **yes — no reductions** (P4) |
| 6 | fancy list indexing (`cf[[1,3,5]] *= x`) | partially (P2 + pokes cover most) |
| 4 | loop writes multiple slots per iteration | mostly order-dependence, some P2 |
| 4 | multiple overlapping segments | order-dependence — correctly refused |
| 3 | `np.arange` with step ≠ 1 in slices | **tool gap, not language** — `push_range` supports steps incl. negative (verified `sweep_cli.c:5027`) |
| 2 | numerically chaotic (`cos` of 1e22) | no — unportable in principle (§3.3) |
| ~13 | misc numpy (array literals, `sph_harm`, ListComp, `np.where`, attribute calls) | assorted |

Workarounds the port tool had to invent — each a conciseness tax a feature would remove:

- **Sign-mask windows**: `(x+|x|)/(2|x|)` on half-integer `range(0.5−a, N+0.5−a)` builds *exact* 0/1 masks (never 0.5 — half-integers are never zero; float division makes exactly 1.0 or 0.0). A two-sided window costs ~27 tokens; poly_52 spends 60+ of its 114 tokens on two windows.
- **Full-range extension**: single loops covering a sub-range evaluate their formula over all slots and let pokes overwrite the rest — safe *only because* vector ops never error a row (clamps, forgiving divide).
- **Dead-poke elimination**: pre-loop pokes on slots the loop covers must be dropped (emission reorders them after the vector); missing this produced a 100% parity failure (poly_23) before the gate caught it.
- **Final-iteration equivalence**: `cf[70] = conj(cf[i-1])` inside a loop = post-loop poke with `k := hi−1`.

---

## 5. Proposed language features

Ordered by unlock count ÷ implementation surface. "Wire impact: additive" means old programs' chains, execution specs, and fingerprints are byte-identical — only new opcodes/enum values are appended, which is the pattern the drift tests already pin.

### P1 — `scan()`: bounded recurrence (unlocks 13 functions)

The single biggest gap. The entire poly_2 class is `cf[0] = init; cf[k] = f(cf[k-1], k)` — inexpressible today except by unrolling (~35 tokens/step × 35 steps ≈ 1,240 ≫ 256).

```
# poly_2 becomes:
poly = fill(36, 0)
poly[0] = p1 + p2
poly = scan(36, 2, poly[0], sin(k*prev) + cos(k*p1) + real(k*p2)*imag(k*prev))
poly = divide(poly, abs(poly))          # v/|v| is elementwise — already vectorizable
poly[17] = p1**2 + real(p1)*p2 - imag(p2**2)
...
```

Design: `scan(len, k0, init_expr, step_expr)` — one new `COEFF_OP_SCAN` token whose two expressions live in the existing `scalar_exprs` side table, plus two new expr opcodes: `EXPR_PREV` (previous element) and `EXPR_K` (current index). The step expression above is 19 postfix tokens — comfortably under the 32-token expr cap. VM: a bounded loop writing `scratch[k] = eval(plan, prev=scratch[k-1], k)`, then push. Crucially this **preserves totality** — no general loops or branches enter the VM; iteration count is a compile-time length. Poly_2 lands at ~30 tokens.

Surface: 1 VM op + 2 expr opcodes (both sides + drift pins), parser support for `prev`/`k` identifiers *only inside* `scan`'s last argument, vocab/help regeneration, stack-validator case (pushes vector). Effort: **M**. Wire impact: additive.

### P2 — slice reads and writes (unlocks 10, de-uglifies ~6 more)

```
poly[2:70] = add(poly[2:70], log(abs(p2 - p1) + 1))    # today: 60-token mask dance
cf[67:71]                                              # slice of the base function output
```

Two ops: `COEFF_OP_SLICE_READ` (src + const bounds in `args` → push window as a vector) and `COEFF_OP_SLICE_WRITE` (pop vector of length b−a, write into poly[a:b)). Grammar is a natural extension of the existing `poly[i]` target and `_slice_bounds`-style parsing already exists in the port tool. Kills the mask workaround for contiguous cases: poly_52 drops from 114 to ~55 tokens and reads like the original. Effort: **M**. Wire impact: additive.

### P3 — comparisons, `rem`, and `select` (unlocks ~11)

The per-k conditional class (`if k % 2 == 0: … else: …`) needs three small pieces:

- `rem(a, b)` — real remainder. **Name matters: `mod` is taken** — it's an alias of `abs` (`canonical_unary_op_name`, chain.py:268), a wire-frozen historical choice that cannot be reassigned.
- `ge(a, b)` / `le` / `gt` / `lt` — elementwise 0/1 masks (binary op family entries).
- `select(c, a, b)` — **pure compile-time sugar**, no VM op: lowers to `add(multiply(c, a), multiply(subtract(1, c), b))`. With c ∈ {0,1} this is exact; the clamp semantics even make `inf`-arm garbage safe when masked, matching the port tool's proven pattern.

```
parity = rem(range(1, 72), 2)
poly = select(parity, <odd-k formula>, <even-k formula>)
```

Effort: **S–M** (rem/ge are one enum row each; select is lowering only). Wire impact: additive.

### P4 — reductions: `sum()`, `prod()` (unlocks ~8)

`np.sum`/`np.prod` tails (poly_42's `cf[70] = t1*t2 + 1j*sum(log(abs(cf[0:70])+1))`) are the *only* blocker for several otherwise-trivial functions. One op family: vector → scalar, pushed onto the typed stack (which already carries scalars). With P2 slices: `poly[70] = p1*p2 + 1i * sum(log(add(abs(poly[0:70]), 1)))`. Note reductions belong in the *statement/call* layer, not scalar exprs — the expr mini-VM has no vector access beyond indexed reads, and should stay that way. Effort: **S**. Wire impact: additive.

### P5 — `let` bindings: compile-time substitution (elegance, all programs)

The language has no intermediate variables; the porting session leaned on RPN (`multiply(pop, pop)`) and slot-as-scratch tricks. Both work but hurt readability — and the user's instinct ("proper reverse polish… many steps rather than one gigantic nested one") only covers vector-shaped values because of the pop-is-vector rule (§3.2).

```
let L = log(abs(p1 + p2) + 1)               # scalar or vector expression
poly = add(..., multiply(range(1, 37), 1i * L))
```

Cheapest sound design: **parse-time substitution** — `let` bodies inline at each use site, exactly like the port tool's `_InlineTemps` for python temps. Zero VM change, zero wire change (the chain is identical to hand-inlining), plus a diagnostic when a let is referenced more than ~3 times (duplicated computation). A register-file design (named workspace slots) would avoid duplication but adds VM state and wire surface for marginal gain at these program sizes. Effort: **S** (lowering only). Wire impact: none.

### P6 — `step(a)` / `window(a, b)` builtin masks (conciseness; pairs with P3)

Even with P2, non-contiguous masking (alternation, multi-window) recurs. The exact-0/1 arithmetic mask the port tool synthesizes — `(x+|x|)/(2|x|)`, 13 tokens per step — should be one call: lower `window(a, b)` to that construction (sugar, zero VM change), or to `ge`-based masks once P3 lands. Effort: **S**.

### P7 — integer-power numerics: `ipow` (correctness at the edges)

`c_powc`'s exp/log path is measurably noisier than numpy's repeated multiplication for integer exponents (§3.3, poly_67). Adding an *integer fast path inside the existing op* would silently change outputs of already-saved programs whose fingerprints key render-artifact caches — cached artifacts would no longer match recomputation. Two clean options: (a) a **new** `ipow` op/name the compiler emits only for new programs, or (b) fold the fast path into `power` and bump `PROGRAM_VERSION` inside `_fingerprint` (orphans every cache once, honestly). Precedent: `c_sqrt_c`'s exact negative-real branch was added for exactly this class of residue (`sweep_cli.c:2609`). Recommend (a). Effort: **S–M**.

### P8 — parser: call-prefix infix fallback (DX)

`parse_call` (`program_source_core.py:283`) should attempt infix parsing when text starts with `name(` but the matching paren isn't final, instead of erroring. Currently every valid program is unaffected (it's an error today), so the fix is purely additive. Removes the most surprising trap in the grammar. Effort: **S**.

### P9 — scalars through the stack (completes the RPN story)

Allow `pop` to bind a scalar in argument position (the typed stack already tags types; the restriction is in `_typed_lower_vector_source` + `coeff_stack_require_vector`). With it, `push_scalar(<v>)` … `divide(pop, abs(pop))`-style scalar pipelines work and P5 becomes less necessary. Requires relaxing the compile-time validator and the C source-to-scratch path symmetrically. Effort: **M** (touches both machines' type rules — the kind of change SM-5's abstract interpreter makes safe).

### P10 — parameterized macros (library building)

Macros splice by name only. With textual parameter substitution (`macro(step-mask, a=10)` resolving before SM-4's expansion), the mask/window/scan idioms become a stdlib of saved helper programs instead of tool-generated boilerplate. Budget/cycle machinery already exists. Effort: **M**.

### Explicitly not proposed

- **Raising the 256-token cap** — ruled out by project decision (2026-07-05); every proposal above fits inside it, and P1/P2 exist precisely to compress what unrolling inflates.
- **General loops/branches in the VM** — the VM is currently trivially total (no back-edges); every program halts in ≤256 dispatches. `scan` delivers recurrence with a compile-time bound; keep it that way.

### Suggested phasing

1. **P1 + P2 + P4** — 31 of the 74 skips become portable (13 recursive + 10 slices + 8 reductions), and the port tool's mask/extension machinery mostly retires. Re-run `port_poly100_programs.py` as the acceptance test: the ported count should roughly double.
2. **P5 + P8 + P6** — authoring quality: lets, no parse trap, one-call masks. These change no wire bytes.
3. **P3 + P9** — conditional class + full RPN.
4. **P7 + P10** — numerics edge + library layer.

Every phase keeps the drift-test discipline: new enum values appended on both sides, `test_coeff_program_drift.py` extended in the same commit, vocab/help regenerated (`gen_coeff_vocab.py --check` already gates staleness).

---

## 6. Risks and invariants to protect

1. **The wire freeze is the load-bearing constraint.** `_execution_spec`'s byte layout and both number-formatting policies are documented FROZEN; fingerprints key persisted render artifacts. All proposals except P7 are additive under this rule; P7 is called out because it changes *values*, not bytes.
2. **Three-way grammar duplication.** The statement splitter exists in Python (`program_source_core.py`), the token semantics in C, and the splitter again in JS (`js/08-chip-editors.js:2433`). Python↔C is drift-pinned by enum-parsing tests; Python↔JS is pinned only by harness string checks. Any grammar change (P5's `let`, P8) must touch all three or the JS statement counter miscounts. Worth a comment in both splitters naming the mirror explicitly (the Python side has it; the JS side should point back).
3. **Totality.** No proposal introduces unbounded iteration. `scan`'s bound is a compile-time constant ≤ 256; the stack validator extends naturally (scan pushes one vector).
4. **The parity harness is the acceptance gate.** The porting batch demonstrated it catches real bugs (dead-poke ordering: 100% divergence; snapshot semantics; conditioning cliffs). Any new op should land with a `port_poly100_programs.py` re-run and a native `sweep_test` case in `tests/test_coeff_program_native.py` — the same pattern the existing ops follow.
5. **Save-path limits are enforced at load too** (`_read_coeff_program_object` recompiles): a feature that lets programs compile larger must never produce saved payloads the deployed reader rejects — this is why the 256 cap was respected rather than worked around.

---

## Appendix A: opcode inventory (verified against both enum tables)

VM ops (30): `const push emit duplicate swap pop flush blend legacy poke_poly poke_tos vector_binary vector_unary vector_roll vector_argsort littlewood linspace range set affine typed_push_scalar typed_push_vector typed_binary typed_unary typed_get_scalar typed_set_poly typed_poke_poly typed_fill native_transform typed_blend` — plus proposal slots for `scan`, `slice_read`, `slice_write`, `reduce`.

Scalar-expr ops (31): literal, `p1 p2 t1 t2 poly_len`, `cf/poly/tos` static+dynamic indexed reads, `+ − × ÷`, `conj neg real imag abs log sqrt exp sin cos tan sinh cosh tanh angle` — plus proposal slots for `prev`, `k`, `rem`, comparisons.

Vector binary (5): `add subtract multiply divide power`. Vector unary (16): `angle mod/abs neg conj sqrt log real imag exp sin cos tan sinh cosh tanh`. Roll (2): `roll rolr`. Native transforms: 28 registry functions.

## Appendix B: porting-batch trap list (all reproduced, all now encoded in the tool)

1. `pop` args must be vectors; scalars fold inline.
2. Leftmost `pop` = top of stack; use `neg` + commutative `add(pop, pop)`.
3. `name(...) <op> ...` argument mis-parses as a call — parenthesize or reorder (P8 fixes).
4. Scalar `**` integer-literal-only, |e| ≤ 32; use `power()` otherwise.
5. Pre-loop pokes on loop-covered slots are dead — drop, don't reorder.
6. Loop-range extension is safe only because vector ops clamp instead of erroring; poke values must be finite or the row dies.
7. Masks: `(x+|x|)/(2|x|)` on half-integer ranges is exact 0/1 (P6 makes it one call).
8. f32 transport saturates at 3.4e38 and floors parity at ~6e-8; compare row-relative when integer-power phase jitter meets catastrophic cancellation (poly_67).
