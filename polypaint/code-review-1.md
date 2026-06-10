# Code Review 1 — Coeff Program stack

> **Status (2026-06-10):** all findings addressed in the follow-up commit.
> F-1..F-20 and the minors are fixed; regression tests added
> (tests/test_coeff_program_drift.py is new, the self-cancelling native test
> was rewritten per-op, and chain/native coverage was extended). Deliberate
> exceptions: n1 (RuntimeError→400 mapping kept — changing it risks the API
> contract for a nit) and m4 (case normalization works as-is). For m11/m16,
> `c_div`'s small-denominator cutoff is kept for param transforms by design;
> the Coeff Program VM now uses full-range scaled division (`c_div_full`).

**Date:** 2026-06-10
**Reviewed at:** commit `d2adb98` (clean working tree)
**Scope:** the Coeff Program text-source / typed-VM system across all five layers:

| Layer | File(s) |
|---|---|
| Source parser | `lambda/coeff_program_source.py` (824 ln) |
| Chain compiler + validator | `lambda/coeff_program_chain.py` (2017 ln) |
| Native VM | `lambda/sweep_cli.c` (coeff program sections) |
| Editor + storage | `index.html` (coeff program editor), `lambda/handler_storage.py` |
| Tests | `tests/test_coeff_program_chain.py`, `tests/test_coeff_program_native.py` |

**Method:** five parallel reviewers (one per layer), followed by independent verification of
every critical/major finding — either by executing the actual parser/compiler
(repro scripts run against `coeff_program_source.py` / `coeff_program_chain.py`) or by direct
read of the flagged C/JS lines. Each finding below carries a verification tag:

- `[executed]` — I reproduced it by running the real code.
- `[code read]` — I read the flagged lines and confirmed the mechanism.
- `[reviewer-traced]` — traced by a reviewer (most executed their repros too); not independently re-verified.

---

## Executive summary

The architecture is sound: the typed-stack validator was traced opcode-by-opcode against the
C dispatch loop and is genuinely consistent with VM semantics (pop orders, broadcast typing,
final-depth rule, limits — with one exception, F-7). Fingerprinting is deterministic
(with one exception, F-9). Storage precedence (`source_text` wins over `chain`) is implemented
correctly in every load/save path.

The real problems cluster at three seams:

1. **The chips editor is broken for several chips** — the JS serializer's default-trimming
   produces chains the backend rejects with 400 (`linear`), and the text synthesizer produces
   source the parser rejects (`argsort`, `exp_affine`) or **silently reinterprets as different
   math** (`pow`, `power`, `linear(poly, 5)`).
2. **Resource caps exist in C but not in Python** — programs that pass validation and get
   saved/fingerprinted can fail at render time (scalar-expr count), and two genuine DoS vectors
   exist on user-supplied Lambda input (macro expansion amplification, `**` token expansion,
   unbounded `power`/`roots` iteration counts).
3. **The per-fn argument-packing rules are triplicated** (Python source layer, Python chain
   layer, C) and have already drifted once (`round(a, b, andy)` works in chain mode, rejected
   from source).

Counts: **2 critical, 14 major, ~20 minor/nit.** Top fixes are listed at the end; most are
small (one- to five-line) changes.

---

## Critical

### F-1. `linear` chip is broken in chips mode — backend 400; text form silently misparses `[executed]`

- `index.html:13496-13499` (serializer default-trim), `index.html:12876-12881` (text synth)
- `lambda/coeff_program_chain.py:1435-1436`, `914-916` (`_compile_affine` requires exactly 4 args)

The `linear` chip is a nativeTransform catalog entry; the serializer trims trailing
default-valued args down to `[name, tgt, src]`:

```js
if (spec.nativeTransform) {
    while (last >= 2 && (values[last] === '' || values[last] === String((pDefs[last] && pDefs[last].def) || ''))) last--;
    return [catalogName, ...values.slice(0, Math.max(last + 1, 2))];
}
```

but the compiler routes `linear` to `_compile_affine`, which requires exactly 4 args.
Verified by execution:

- `[["linear","poly","poly"]]` → `affine chip requires tgt, src, multiplier, offset` → **400 on every save/preview/compute** with a default-state linear chip.
- `[["linear","poly","poly","5"]]` → same 400.
- Text synth `poly = linear(poly, 5)` → **silently misparsed**: `_typed_lower_affine` treats the 2-arg form as `(multiplier, offset)` with source defaulting to `pop`, lowering to

  ```
  [['_typed_push_vector','pop'], ['_typed_push_vector','poly'], ['_typed_binary','multiply'],
   ['_typed_push_scalar','5.0+0.0j'], ['_typed_binary','add'], ['_typed_set_poly']]
  ```

  i.e. it pops the stack and uses `poly` as the *multiplier* — wrong semantics, plus a runtime
  stack underflow when the stack is empty.

Only the full form `linear(poly, 5, 0)` round-trips correctly.

**Fix:** stop default-trimming below the declared arg count for `linear`/`exp_affine` in the
serializer (or make `_compile_affine` default-fill), and make the synthesizer always emit both
multiplier and offset.

### F-2. `poly = range(...)` / `linspace` / `affine` / `macro` / `poke_tos` silently drop the assignment target `[executed]`

- `lambda/coeff_program_source.py:572-580`, `626`, `631-637`; `_lower_range` at 478-485

`_lower_call` is invoked with `target="poly"` from the assignment path (line 681), but the
`range`/`linspace` branches never consult `target` — they return only `[["push_range", ...]]`,
which pushes to the stack and never sets poly. Verified:

```
poly = range(5)
drop
```

compiles cleanly to `[['push_range','5'], ['pop']]` — **poly is never assigned, with no
diagnostic**. (Standalone `poly = range(5)` fails only with the unrelated
`final stack depth is 1; expected 0`.) Same class: `poly = affine(...)`, `poly = macro(name)`,
`poly = poke_tos(i, v)` all silently ignore the lhs. `_lower_const` (473) and `blend` (611)
handle `target` correctly, showing this is an oversight, not design.

**Fix:** in these branches, when `target` is set, append the set-poly step (or reject the
assignment form explicitly).

---

## Major — parser (`lambda/coeff_program_source.py`)

### F-3. Nearly all lowering errors are reported at line 1, column 1 `[executed]`

- `coeff_program_source.py:800`, constructor defaults at 142-145

```python
diagnostics.append(_diagnostic(exc, line=exc.line or stmt.line, column=exc.column or stmt.column))
```

`CoeffProgramSourceError.__init__` defaults `line=1, column=1` — truthy — so `exc.line or
stmt.line` never falls back to the statement's location. Verified:
`poly = sin(poly)\npoly = nosuchfn(poly)` reports `unknown coeff program source function
'nosuchfn'` at **line 1, column 1** instead of line 2. Since the editor uses diagnostics for
highlighting, this defeats the per-statement location plumbing entirely.

**Fix:** default `line=0`/`column=0` in the exception, or pass `stmt.line` unconditionally
when the exception used defaults.

### F-4. Greedy `_ANY_INDEX_RE` rejects valid compound expressions starting with `cf[`/`poly[`/`tos[` `[executed]`

- `coeff_program_source.py:58`, `360-371`, tried first in `_typed_lower_value` at 447

`^(cf|poly|tos)\[(.*)\]$` is greedy, so `cf[0]*cf[1]` matches with the bogus "index"
`0]*cf[1`. Verified: `poly = add(cf[0]*cf[1], poly)` → rejected with the misleading
`unexpected scalar expression token ']'`, while `poly[0] = cf[0]*cf[1]` (direct scalar path)
works fine. Affects every `_typed_lower_value` context: args of `add/sub/mul/div/pow`, unary
names, `scale/shift/linear`, and the `poly = <expr>` fallback. Always an error rather than a
wrong result, but it rejects valid programs with a wrong-location, wrong-content message.

**Fix:** try `_typed_lower_scalar` on the full text first, or fall through to scalar lowering
when the index-form parse fails.

### F-5. Old-form `round(a, b, andy)` works in chain mode, rejected from source text `[executed]`

- `coeff_program_source.py:258-261` (`_native_transform_args_and_andy`),
  `coeff_program_chain.py:1344-1350` (`_max_native_transform_stack_arg_count`: fn 23 → 1)

Source side strips andy for fn 23 and lowers 2 stack args; chain side caps fn 23 at 1 stack
arg and raises — and because that error is a plain `RuntimeError` during chain compilation
(not a `CoeffProgramSourceError` during lowering), the source layer's full-args fallback is
never taken. Verified: source `poly = round(poly, 1, 2, 0.5)` → `round stack arg count must
be <= 1, got 2`; chain `["round","poly","poly","1","2","0.5"]` → compiles fine. Same program,
two frontends, one wrongly rejected. This is the triplicated fn-packing knowledge (fn 14/16/23/24
special-cased in three places — see F-17) already drifted.

**Fix:** mirror the per-fn stack-arg caps in `_native_transform_args_and_andy` and route
over-cap counts to the full-args `_native_transform` fallback.

---

## Major — compiler (`lambda/coeff_program_chain.py`)

### F-6. Macro expansion is exponentially amplifiable before any size check (DoS) `[executed]`

- `coeff_program_chain.py:1389-1418` (`_expand_macros`); token cap only at 1521, after full lowering

The cycle check is path-local and `MAX_MACRO_DEPTH = 8` bounds depth only — there is no cap
on chips per macro or cumulative expanded size, and the resolver is backed by user-saved S3
programs (`handler_coeffgen.py`). Verified: 8 macros × branching factor 4 expanded to
**32,768 chips before the token cap fired**; the reviewer's run with B=6 materialized 1.68M
chips in 1.5 s. B=1000 → ~10^21 chips, i.e. Lambda memory/CPU exhaustion on validated input.

**Fix:** thread a running expanded-chip counter through `_expand_macros` and abort past
`MAX_PROGRAM_TOKENS`.

### F-7. No Python cap on scalar-expression count — validated, saved programs rejected by the C VM `[executed + code read]`

- `coeff_program_chain.py:815-825` (`_add_arg_expr`, no cap, no dedup), 1516-1525 (only token count checked)
- `sweep_cli.c:3321` (`COEFF_PROGRAM_MAX_SCALAR_EXPRS 64`), 3713-3716 (hard reject)

Python mirrors every other C limit (tokens 256, stack 64, vector len 256, args 8, expr tokens
32≡96/3) but has no constant for expression count. Verified: a 70-token chain with one dynamic
expression each compiles cleanly with `scalar_expr_count: 70`, gets fingerprinted and is
saveable — then every row fails at render time with `coeff_program has too many scalar
expressions`. One `exp` legacy chip contributes up to 4 refs plus andy, so realistic programs
can hit this.

**Fix:** enforce `len(scalar_exprs) <= 64` in `_lower_chain`; ideally dedup identical
flattened expressions.

### F-8. `**` expansion blows up memory before the token-count check (DoS) `[reviewer-traced]`

- `coeff_program_chain.py:602-622` (`_power` repeats the base's token list per exponent), size guard at 509-514 runs post-parse

Each `**32` multiplies token count ~32× and the forms nest: `(((p1**32)**32)**32)**32` (~30
chars, far under the 64 KiB source cap) expands to ~32⁴ ≈ 1M token dicts during parse before
`MAX_SCALAR_EXPR_TOKENS` (32) is ever consulted. Runs on user-supplied text in Lambda.

**Fix:** enforce a running token budget inside `_power` (and after each `_term`/`_expr` step),
not just at the end.

### F-9. `-0.0` leaks into token args while the fingerprint canonicalizes it — fingerprint-equal programs execute differently `[executed]`

- `coeff_program_chain.py:709-770` (static folding produces -0.0), 818-822 (raw `value.real` into token args), 372-376 (`_format_number` canonicalizes only the fingerprint side)

Verified: `["push_const","4","-(0)"]` and `["push_const","4","0"]` produce **identical
fingerprints** but the first serializes `"args": [4.0, -0.0]` (signbit set), shipped verbatim
to the VM. The C side keeps -0.0 in token args (its `coeffProgramDoubleBits` canonicalization
at `sweep_cli.c:4659` covers only the littlewood seed). A downstream `angle` then gives
`atan2(0,-0) = π` vs `0` — different output under one cache key. The recent -0 work covered
`_format_number` and `_flatten_expr` but not static token args / `args_im` / `andy`.

**Fix:** canonicalize -0 in `_token` (or `_add_arg_expr`).

### F-10. `range`/`linspace` static length bounds check silently bypassed `[executed]`

- `coeff_program_chain.py:878-885`, `892-899`; `_vector_length_arg` at 362-369

`_vector_length_arg` raises `RuntimeError` both for non-numeric expressions (the intended
fallback case) and for numeric out-of-range lengths; the `except RuntimeError` swallows the
latter and re-compiles the literal as a static real arg. Verified: `push_range(500)`,
`push_range(0)`, `push_range(-3)` all compile with static args `[500.0]`/`[0.0]`/`[-3.0]`,
bypassing the `[1, 256]` check. Related (reviewer-traced): static `-1` falls through the same
path and silently aliases `poly_len` via the C sentinel branch (`sweep_cli.c:4000`).

**Fix:** only fall through to the expression path when `float(args[0])` itself fails.

---

## Major — native VM (`lambda/sweep_cli.c`)

### F-11. Static `tosN` scalar-expression read skips the vector type check — reads stale workspace memory `[code read]`

- `sweep_cli.c:3829-3841` (static `COEFF_EXPR_TOS_AT`) vs 3874 (dynamic variant has `coeff_stack_require_vector`)

`coeff_stack_push_scalar` sets `stack_len[slot] = 1` but writes only `stack_scalar_re/_im`,
never `stack_re[slot][]`. When TOS is a scalar, static `tos0` passes the bounds check
(`0 < 1`) and reads `stack_re[slot][0]` — whatever a **previous row's vector** left in that
reused ring slot (the workspace persists across rows; only cursors reset). The Python
validator never inspects scalar-expression references, so `push_scalar(p1); push_const(4, tos0)`
compiles and executes with silently row-order-dependent garbage. In-bounds, so not memory-unsafe,
but real wrong-data — and inconsistent with the DYN path one screen below.

**Fix:** add the same `coeff_stack_require_vector` check, or read `stack_scalar_re/_im` when
the slot is scalar (arguably the intended semantics).

### F-12. Legacy int args (`power`, `invpower`, `roots` k/iters) unbounded — UB cast + unbounded compute loop (DoS) `[code read]`

- `sweep_cli.c:4084-4087` (dispatch casts `(int)args[0]` unclamped), `ct_power` at 2976-2990 clamps only `power < 0`
- `coeff_program_chain.py:1270-1274` — `int(_finite_number(...))` with no min/max; registry carries no range

Verified by read: `ct_power` runs `for (p = 1; p <= power; p++)` over up to 256 coefficients,
per sweep row. `["legacy","power","cf","poly", 1e9]` passes validation → ~1e9 × 256 × rows
iterations → guaranteed Lambda timeout. Via the stack-arg path a runtime-computed `1e300`
reaches `(int)args[0]` — formally UB (ARM64 saturates to `INT_MAX`, same hang).

**Fix:** clamp in C (`k`/`power` ≤ ~4096, `iters` ≤ ~1024) **and** add ranges on the Python
side / registry.

### F-13. `COEFF_OP_BLEND` pops without vector type checks `[code read]`

- `sweep_cli.c:4919-4931`

Unlike EMIT (4860), TYPED_BLEND (4498-4499), and the pop/peek source readers, BLEND never
calls `coeff_stack_require_vector`. Two scalar slots (`stack_len == 1` each) pass the length
check and blend stale `stack_re[][0]` contents. Unreachable from the current compiler (the
validator rejects it), but this C code is the last line of defense and every sibling opcode
has the check. Two-line fix.

---

## Major — JS editor (`index.html`)

### F-14. `pow` / legacy `power` chips collide with typed-binary aliases — silent semantic changes `[executed]`

- `index.html:12876-12892` (synth), `coeff_program_source.py:60-70` (`_VECTOR_BINARY_ALIASES` claims `pow`/`power` first)

Verified:
- Default `pow` chip synthesizes `pow(poly, 1, 1)` → parser error `pow requires no args or src1, src2`.
- After default-trim, `pow(poly, 2)` → **silently** lowered to elementwise `poly ** 2` — not the
  native transform `pow(z*field1, field2)` it represents (which would be `2z`).
- Legacy `power` chip ("(i+1) × geometric series through z^k") synthesizes `power(poly, 8)` →
  silently parsed as elementwise `poly ** 8`. No error, completely different math.

Chain forms compile fine; this is purely a synthesizer/parser round-trip break. Related
(executed, chain side): `["power","poly","poly","8"]` in chain mode is *also* shadowed by the
vector-binary route and rejected (`power src2 selector is invalid: '8'`) — legacy `power` is
reachable only as `["legacy","power",...]`, while `invpower` works by name. (F-6 in the chain
review; same root cause.)

### F-15. `argsort` and default `exp_affine` chips synthesize source the parser always rejects `[executed]`

- `index.html:12842-12897` — no `argsort` branch, so the generic fallback emits the target
  selector as a third arg: `argsort(poly, poly, poly)` → `argsort requires src1, src2` (verified).
- Default `exp_affine` chip trims to 2 selectors → synth `exp_affine(poly)` → rejected
  (`exp_affine requires multiplier, offset, and optional andy…`), while the chain form
  `[["exp_affine","poly","poly"]]` compiles fine (verified).

Every chip chain containing these produces a Text tab that cannot compile.

### F-16. Tab-switch staleness: text only synthesized when the textarea is empty; chain-only load never clears it `[code read]`

- `index.html:12921-12923`, `13543-13557` (`_applyCoeffProgram`)

```js
if (normalized === 'text' && !_getCoeffProgramSourceText().trim() && _cpChain.length) {
    _setCoeffProgramSourceText(_coeffProgramSourceFromChain(_serializeCoeffProgramChain()));
}
```

No dirty tracking. Confirmed failure sequences:
1. Chips → Text (synth) → Chips → edit chips → Text again: textarea non-empty so **not**
   re-synthesized; the stale text is authoritative for save/preview/compute
   (`_effectiveCoeffProgramSourceTextForCompute` keys purely off the active tab). Newer chip
   edits silently ignored.
2. Load text program A, then chain-only program B (`_applyCoeffProgram` doesn't clear the
   textarea in the no-source branch), click Text: **program A's source** is shown and becomes
   authoritative under program B's name. The compute-restore path (line ~4441) clears
   unconditionally; the modal load path should too.

### F-17. fn-index packing rules triplicated and already drifted `[code read + executed]`

fn ids 14/16/23/24 are special-cased in `coeff_program_source.py:248-261`,
`coeff_program_chain.py:1236-1243` + `1344-1350`, and `sweep_cli.c:4538-4588`. F-5 is this
drift manifest. Only the fn-16 stack path is natively tested. No test pins the registry ids.
**Fix:** single shared table (registry-driven max stack-arg counts + andy packing), plus a
drift test (see F-19).

---

## Major — tests

### F-18. The native vector-ops test is self-cancelling — claims ~12 ops, verifies 3 `[executed: read + traced]`

- `tests/test_coeff_program_native.py:482-521`

Confirmed by trace: `["subtract","poly","poly","poly"]` computes `poly − poly = 0`, wiping
every prior result (`argsort`, `roll`, `rolr`, `add`); then `push_const 4,2 / emit` overwrites
poly again; `divide`/`power` on uniform values are degenerate identities; `angle(1)=0` is
overwritten by `push_const 3+4j / emit`. The final assertion (`value.real == 5.0`) depends
only on the last three chips (`push_const 3+4j`, `mod`, `abs`). Swapping the C
roll-left/roll-right dispatch would not fail this test. Same overwrite pattern in
`test_coeff_program_push_const_linspace_and_range_use_poly_len` (593-619): only the last
`push_range` result is asserted.

**Fix:** per-op assertions on non-degenerate inputs.

### F-19. Coverage holes and drift exposure `[reviewer-traced; spot-checked]`

Coverage map highlights (full map in the appendix below):

- `COEFF_OP_TYPED_UNARY=24`: **zero tests** in either file (reachable via bare `abs()`/`sin()`
  after a push — the documented `dup / sin() / swap / cos() / add()` example is untested).
- `COEFF_OP_AFFINE=20`: only a *negative* assertion exists (`assertNotIn`); the explicit
  `affine(tgt, src, m, o)` chip is never positively compiled or executed.
- Static folding silently voids apparent native coverage: `poly[3] = sqrt(-1)` /
  `poly[4] = log(1j)` in the native test are folded to literals by Python, so the C
  `EXPR_SQRT` evaluator is **never executed by any test**; same mechanism leaves
  `EXPR_DIV/CONJ/NEG/TAN/SINH/COSH/TANH/ANGLE/TOS_AT_DYN` and vector-unary 8-16
  (real/imag/exp/cos/tan/sinh/cosh/tanh) with no native execution coverage.
- Opcode/EXPR/fn enums are hand-duplicated in Python and C (currently identical — verified by
  the C reviewer constant-by-constant) with no drift test; drift in any of the untested
  entries above would pass the entire suite.
- Native tests run against the committed `lambda/sweep_test` binary with no freshness check —
  a C edit without recompile makes every "parity" test validate stale code. (They do fail
  loudly when the binary is missing — that part is fine.)

### F-20. Docs claim trig names are callable native transforms; the parser shadows them `[reviewer-traced]`

`coeff-program-commands.md:544-549` lists `cos/sin/tan/cosh/sinh/tanh` as transform-style
commands with optional trailing andy, but `_lower_call` checks `_VECTOR_UNARY_NAMES` first
(`coeff_program_source.py:584-589`), so `cos(poly, 0.5)` raises `cos requires no args or one
source`. Registry fn 17-22 are unreachable from source text; the doc's "shadowed" note lists
only `linear`, `conj`, `pow`/`power`. Chain mode `["sin","poly","poly","0.5"]` *does* compile
to NATIVE_TRANSFORM with andy (verified by the chain reviewer) — so source text cannot express
andy for these at all. The `exp/cos/sin/...` entries in `_LEGACY_UNARY_NAMES` (source.py:91-113)
are dead code.

---

## Minor

**Parser** (`coeff_program_source.py`) `[reviewer-traced unless noted]`
- m1. `_lower_native_transform_call` (547-558) catches `CoeffProgramSourceError` broadly on the
  stack-args fast path, so genuinely malformed args surface the *fallback's* error instead of
  the real one.
- m2. `#` inside parens isn't comment-stripped (739-741) and fails with the opaque
  `invalid scalar expression near '#…'`.
- m3. `_native_transform_args_and_andy` (250-265) silently guesses packing on wrong arg counts
  instead of raising one clear arity error.
- m4. Case normalization is ad hoc (`_INDEX_RE` IGNORECASE vs `lhs.lower()` only on the bare path).

**Compiler** (`coeff_program_chain.py`) `[reviewer-executed]`
- m5. `roll`/`rolr` shift unbounded → `(int)` cast UB in C (`["roll","poly","poly","1e18"]`
  compiles; `sweep_cli.c:4229`).
- m6. `_compile_poke` (945) truncates non-integer indices silently (`poke_tos 1.9` → index 1);
  everywhere else uses `_integer_literal` which rejects.
- m7. `_native_transform_stack_arg_token` (1365) raises bare `ValueError` for non-int count,
  escaping the module's `RuntimeError` convention.
- m8. `-2**2 == +4` (unary minus binds tighter than `**`, opposite of Python) and `2**3**2` is
  rejected rather than right-associative. Deterministic, but worth a doc note.
- m9. Latent `KeyError` on unknown registry enum choices (1267; `_ENUM_ARG_VALUES` knows only
  hi/lo and the registry loader doesn't validate choices).
- m10. Dead code: unused `before = depth()` (1542), unreachable tuple branch in `_chip_args`
  (417), unused `src_name` (1040). Non-strict failure path returns un-canonicalized user input
  as `source_chain` (1992).

**Native VM** (`sweep_cli.c`) `[reviewer-traced]`
- m11. Scalar `EXPR_DIV` (3917-3923): explicit zero check on squared magnitude (`d <= 1e-300`
  ⇒ |b| ≤ ~1.8e-151) while `c_div` (2434-2439) silently zeroes anything |b| < 1e-15 — the
  dynamic `1/p1` at `p1=1e-16` yields 0 while the static fold of the same text yields 1e16.
  The explicit error path is nearly dead code.
- m12. `c_powr` underflow/overflow (2540-2548): `|z| < 1e-30` square-roots to exactly 0
  (Python fold: 1e-20); `|z| > ~1.3e154` makes `m2 = inf` and fails the whole row in the
  scalar VM while the static fold computes ~1e77. Fold-equivalent programs can differ at the
  extremes. Same class for `EXPR_LOG`.
- m13. Unvalidated double→int casts precede their range checks (4229, 4815, 4933, 4949, 3794) —
  formal UB, saturated on ARM64; use `coeffProgramIntegerFromReal` consistently.
- m14. `parseNumArray` (216-229) truncates at maxCount so the `n > MAX` guards (3722-3727,
  3640-3645) are dead code; a hand-crafted 99-number expr truncates to 96 and can evaluate
  silently wrong (not reachable from the Python compiler).
- m15. fn-24 stack-arg fallthrough (4572): counts 3-8 fall into the generic branch with
  different packing instead of erroring like fn 14/16/23 (unreachable today; trap).
- m16. Signed-zero canonicalization verified correct under `gcc -O3` (no `-ffast-math`); one
  documented divergence: C dynamic `sqrt` of `-x − 0j` returns `+i·…` (canonicalized) while
  Python's static `cmath.sqrt` honors the signed zero and returns `−i·…`.

**JS / storage** `[reviewer-executed unless noted]`
- m17. Legacy `linear` andy: synthesizes 4-arg `linear(src, a, b, andy)` → parser error; and
  andy is **silently dropped** on save/load round-trip (catalog filters the andy param,
  `index.html:11908-11910`; same drop for old `['cos', tgt, src, andy]` chains).
- m18. JS statement count splits on lines only; backend counts `;`-separated and joined
  multi-line statements — S3 metadata disagrees with the modal display.
- m19. Uploaded JSON with `"source_text": ""` plus a non-empty `"chain"` loads as an empty
  program (`index.html:13520-13541` keys off key presence, not content).
- m20. The editor never calls `/compile-coeff-program-source` — the diagnostics endpoint built
  for it is dead from the editor's perspective; first feedback on bad text is a failed save.
- m21. `/compile-coeff-program-source` with `strict=False` returns 200 with a silently
  truncated `chain` + fingerprint alongside `ok: false` (`handler_storage.py:1204-1229`) — a
  careless future caller could persist a partial program. Returning `chain: []` on any error
  would be safer.
- m22. `_handle_storage_route` maps `RuntimeError` to 400, so genuine server faults (registry
  load failure, corrupt stored JSON) read as client errors.

**Tests/docs** `[reviewer-traced]`
- m23. Documented-but-untested: `roots`, `roots_cm`, `invpower`, `round`, `safe`, `normalize`,
  `negate_odd`, `max2one`, sort family, `swirler` (source mode); constants `tau`, `tau_i`,
  `pi2`, bare `i`; `swap`, `flush`, scalar `dup`; `#` comments; source-mode `macro`; the `**`
  ±32 boundary. Undocumented-but-implemented: bare `tos` as a vector source.
- m24. `test_coeff_program_littlewood`'s `round(v.real) ∈ {0,1}` passes on all-zeros output.

---

## What checked out clean

Worth recording, since these were explicitly traced:

- **Validator soundness:** every opcode's pop order, operand order, and result typing in
  `_validate_stack` matches the C dispatch loop (typed binary broadcast algebra, GET_SCALAR,
  TYPED_POKE_POLY, TYPED_FILL, TYPED_BLEND, NATIVE_TRANSFORM scalar-then-src pop order,
  EMIT-on-empty no-op, final-depth-0 rule). Limits agree on both sides except F-7.
- **Constant parity today:** all 30 `COEFF_OP_*`, 31 `EXPR_*`, 16 vector-unary, 5 binary, roll
  ops, selectors and scalar sources are numerically identical between Python and C (the risk
  in F-19 is future drift, not current state).
- **Fingerprint determinism** (modulo F-9): sorted-keys JSON, `.17g`, NaN/inf rejected at every
  numeric ingress.
- **Storage precedence:** `source_text` wins over `chain` in all four paths
  (`_compile_coeff_program_payload`, save, fetch, macro resolver); strict parse on save means
  invalid source is never stored; errors propagate as 400s, no 200-with-garbage on save/fetch.
- **Memory safety in the VM hot paths:** vector lengths funnel through
  `coeff_program_check_len` (1..256); ring-buffer arithmetic can't collide at depth ≤ 64;
  legacy `ct_*` fixed arrays are sized to `MAX_COEFFS = 256 = COEFF_PROGRAM_MAX_VECTOR_LEN`;
  `expr_refs`/`andy_expr_ref` range-checked at every use; typed binary/swap stage through
  scratch so in-place aliasing is safe; the cRe/cIm in/out aliasing at the call site is safe.
- **Native tests fail loudly** when the binary is missing (no silent skips), and
  `assert proc.returncode == 0, proc.stderr` surfaces native stderr.

---

## Prioritized fix list

1. **F-1** `linear` chip: serializer trim + `_compile_affine` arity + `linear(poly, m)`
   misparse — the chips editor's default chip 400s today.
2. **F-2** assignment-target drop for `range`/`linspace`/`affine`/`macro`/`poke_tos` — silent
   wrong programs.
3. **F-11** static `tosN` type check (one line in C) and **F-13** BLEND type check (two lines),
   together in one binary rebuild.
4. **F-6/F-8/F-12** the three DoS vectors (macro amplification counter, `**` running budget,
   clamp `power`/`roots` ints in C + Python).
5. **F-7** scalar-expr cap in `_lower_chain` — stops saving programs the VM will refuse.
6. **F-14/F-15** `pow`/`power`/`argsort`/`exp_affine` synth — silent math changes and broken
   Text tabs; **F-16** tab-switch dirty tracking.
7. **F-3** diagnostic locations and **F-4** greedy index regex — biggest editor-UX payoff.
8. **F-9** -0 in token args (cache-correctness), **F-10** range bounds, **F-5/F-17** unify
   fn-packing rules.
9. **F-18/F-19** rewrite the self-cancelling native test; add dynamic-operand native tests for
   the folded-away EXPR ops and vector-unary 8-16; add an enum-drift test (regex the C enums,
   compare to Python constants, pin registry fn ids 14/16/23/24).
10. **F-20** fix the commands doc's transform table (or un-shadow the trig transforms) and pin
    the chosen behavior with a test.

---

## Appendix: native/compile coverage map (condensed)

| Item | Compile test | Native execution | Note |
|---|---|---|---|
| OP 19 SET, 21 PUSH_SCALAR, 23 TYPED_BINARY, 25 GET_SCALAR, 26 SET_POLY, 27 POKE_POLY, 28 FILL, 29 NATIVE_TRANSFORM | yes | yes | covered |
| OP 20 AFFINE | negative only | none | F-19 |
| OP 22 PUSH_VECTOR | yes | exercised | adequate |
| OP 24 TYPED_UNARY | none | none | F-19 |
| OP 30 TYPED_BLEND | negative only | yes (value-asserted) | minor |
| EXPR 1-6, 12-21, 24-26 (lit/params/arith/reads/exp/sin/cos) | yes | yes | covered |
| EXPR 7 DIV, 23 SQRT | folded statically | **never executes in C** | F-19 |
| EXPR 8-11, 22, 27-31 (conj/neg/real/imag/tos-dyn/tan/sinh/cosh/tanh/angle) | partial/compile-only | none | F-19 |
| VEC_UNARY 2-7 (mod/abs/neg/conj/sqrt/log) | yes | yes | covered |
| VEC_UNARY 1 angle, 8-16 (real/imag/exp/sin…tanh) | partial | none/crash-only | F-18, F-19 |
| VEC_BINARY add/sub/mul/div/pow, ROLL, ARGSORT | yes | crash-only (self-cancelling test) | F-18 |
