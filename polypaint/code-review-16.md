# Code Review 16 — In-Depth Backend Audit

**Revision (HEAD `0c0075a`).** This review was first written against `ca3e223`
(palette refactor `ede13b4`). Since then, `536a37b` ("Fix CR15 diagnostics and
palette coverage gaps") closed most of B2 — verified: the e2e test is no longer
inverted (`render-solve-score.spec.js:1344` now asserts RGB-3-output *enabled*
plus a scalar-LUT-rejects-3 sibling), and the palette coverage gaps are filled
(variant-identity stability+discrimination+reuse tests, ASL forwarding
assertions on both chunk and finalize, a multi-channel finalize
no-mirroring ordering test, and native 3-channel packed-byte parity). B2 is
rewritten below to its one remaining item (Stage-0 UI). B1, B3, B4, B5, B6 were
re-checked at HEAD and stand unchanged (`536a37b` touched neither
`handler_compute_plan.py` nor `handler_storage.py`). Top priority is now **B1,
B3, B4**.

Originally reviewed `main` at `ca3e223` (palette multi-channel refactor `ede13b4`
on top of the CR14 base `ba5b67d`). This is a deliberately broad pass into
areas the prior fifteen reviews touched least: native C memory safety, the
security / IAM / data-exposure surface, resource / concurrency / runaway-cost
behavior, the storage-handler breadth and observability, plus the just-shipped
palette multi-channel implementation.

**Method.** Four parallel subsystem reviews (native, security, resource,
storage/shared) plus direct re-verification of the palette commit. Every
blocker- and concern-class finding below was traced to source by me, not
relayed — file:line evidence throughout. Calibrated for a single-user personal
project: I do not inflate the unauthenticated-API posture (it's a conscious,
in-code-documented tradeoff), and I flag separately the few things worth fixing
*regardless* of tenancy.

## Verdict

The mature subsystems are genuinely solid, and that deserves stating plainly:
the native C is defensively written with no memory-safety blocker reachable
from untrusted input; IAM is real least-privilege; secrets hygiene is clean;
there is no shell/JSON injection vector. The drift-prone DSL/contract surfaces
that earlier reviews hardened are holding.

The real action items are few and specific:
1. **One availability blocker** — `handler_compute_plan` lacks the
   Step Functions state-size guard that render and palette already have, so
   large-but-legal jobs die with `States.DataLimitExceeded` instead of a clear
   plan-size error.
2. **The palette work is now correct *and* pinned** (as of `536a37b`) — the
   only remaining item is the skipped Stage-0 UI cleanup.
3. **Storage-handler observability gaps** — 18 of 31 routes have no error
   envelope, and `handle_cleanup` is an unbounded, error-swallowing delete.
4. **A handful of resource sharp edges** — unbounded `pix`, a 500-wide solve
   fan-out whose throttles aren't retried, and non-idempotent compute/render
   launches.

## Findings

### [blocker] B1 — `handler_compute_plan` has no plan-size guard; large jobs overflow the 256 KB Step Functions state limit

`lambda/handler_compute_plan.py:371` returns `chunk_items` (one ~545-byte dict
per chunk) **inline** in the plan object, which then persists across every
workflow state (`compute_workflow.asl.json.template` `ItemsPath:
$.plan.chunk_items` at the ParamGen/Coeffgen/FusedChunk/Solve maps). `n_chunks`
is validated only up to `MAX_CHUNKS = 5000` (`:43,:162`), and the fused path's
`auto_hires_chunks` can independently drive the count from `N` (up to
`MAX_N=50000`) and degree.

Measured: ~536 KB at 1000 chunks, crossing the 256 KB ceiling at **~470
chunks** — reachable by a direct `n_chunks`, by fused auto-chunking at high N,
or by high degree. **`handler_render_plan.py:51` and
`handler_palette_render_plan.py:42` both have a `MAX_PLAN_BYTES = 200 KB` guard
that fails fast with a clear message; compute has none** (verified: the symbol
is absent from the file). The inconsistency is the tell — the team knows the
pattern; one of three plan builders is missing it.

Worst part: on the fused path the system's *own* memory-safety math forces the
high chunk count, so the larger the legal job, the more certain the failure.
Ordering nuance (verified against the ASL — the compute workflow has **no**
`CleanRender` state; that's a render-workflow concept): on the classic path
`BuildPlan` is the first real state, so the guard rejects cleanly before any
fan-out; on the fused path `DegreeProbeTask` runs *before* `BuildFusedPlan`, so
the guard still prevents the state overflow but won't reclaim the degree-probe
work already spent.

**Fix (now):** add the same `MAX_PLAN_BYTES` guard at plan construction
(`BuildPlan` / `BuildFusedPlan`), converting a cryptic `States.DataLimitExceeded`
into a clean fail-fast error. **Fix (real):** stop passing `chunk_items` inline
— write the
chunk list to S3 in the plan Lambda and pass an S3 reference + count; have the
Maps use a Distributed Map `ItemReader`. This also shrinks `calc.json` and the
C1 manifest problem below.

### [concern → mostly resolved] B2 — Palette multi-channel refactor: now correct *and* pinned; only Stage-0 UI remains

The implementation was correct in `ede13b4` (deterministic id closes both the
omega and two-entrypoint traps via one shared `_palette_variant_identity`;
serpentine multi-channel assembly reuses `_copy_pass0_chunk_rows` with
`stride=channels`, no mirrored-rows bug; native path reuses the proven
`solve_score_eval_program_outputs_from_buffers`; the
`color_render_contract.py` packaging gap was fixed). The original concern was
that none of it was test-pinned and one e2e test was inverted. **`536a37b`
closed all of that** — verified at HEAD:

- e2e fixed — `render-solve-score.spec.js:1344` is now "palette family generate
  *accepts* RGB explicit output" (`toBeEnabled()` + empty status), with a new
  "rejects 3-output programs in scalar LUT mode" sibling.
- variant-identity tests added — `test_palette_render_plan.py:35`
  (`_is_stable_and_discriminates_render_contract`), `:72`
  (`_ignores_palette_for_direct_rgb`), `:97`
  (`_reuses_existing_matching_variant_as_done`): stability + discrimination
  (incl. the omega Trap-1 guard) + no-op reuse.
- ASL forwarding pinned — `test_palette_workflow_definition.py:131,134,160`
  assert `score_output_channel_count.$` / `color_interpretation.$` reach both
  the chunk Map and finalize.
- multi-channel ordering pinned —
  `test_palette_finalize_handler.py:334`
  (`test_finalize_assembles_multi_channel_raw_rows_without_mirroring`) — the
  correctness-critical mirror-trap is now guarded.
- native parity pinned — `test_palette_chunk_mt_parity.py:153`
  (`test_explicit_three_channel_emit_norm_writes_packed_bytes_and_scores`).

**Still live (the one remaining item):** Stage-0 UI cleanup. `index.html:1928`
still has `#btn-palette-create` in the stack `.color-row`, labeled "Palette" —
not moved to its own action row and renamed "Generate Artifact" as the plan's
Stage 0 specified, and the interpretation control is `<select id=...>` rather
than the spec'd `name="palette-color-interpretation"`. Cosmetic/UX only; the
backend and the validation contract are complete and pinned.

### [concern] B3 — `handle_cleanup` is an unbounded, error-swallowing delete (fix regardless of auth)

`lambda/handler_storage.py:2249-2271`: deletes whatever `keys` list the caller
supplies, with **no prefix guard** (the docstring says "temp .raw and merge
files," nothing enforces it), wraps `delete_objects` in `except Exception:
pass`, and does `total_deleted += len(batch)` regardless of outcome (with
`Quiet:True` there is no `Deleted` list to count). So it can delete any object
in the bucket — including `index.html`, `js/*`, `config.json` — silently
swallows S3 failures, and returns a fictional success count. This is worth
fixing even single-user: a frontend bug or stale client could wipe the site
assets and report success. **Fix:** prefix allow-list (only `renders/.../*.raw`
/ merge temp keys per its own contract), drop `Quiet:True`, count
`len(resp["Deleted"])`, surface `resp["Errors"]` like `handle_delete` already
does.

### [concern] B4 — 18 of 31 storage routes have no error envelope → bare "Internal Server Error"

`handler_storage.py:844+`: only the 13 program routes go through
`_handle_storage_route` (which maps exceptions to clean 400/404 with context).
`handle_list` and every destructive route (`/delete`, `/delete-prefix`,
`/cleanup`, `/clean-render`, `/delete-task`, `/delete-palette`,
`/delete-render-artifact`, `/delete-favorite`) plus `/check-keys` /
`/check-status` are called raw, and `handler()` has no outer try/except. A
`KeyError` on a missing `job_id` or an S3 `ClientError` becomes an unhandled
Lambda exception → generic 502/"Internal Server Error" — exactly the
contextless string `deployment-checklist.md` §12 prohibits, on the routes most
in need of clear errors. **Fix:** route every branch through
`_handle_storage_route` (or an equivalent wrapper).

### [concern] B5 — Resource sharp edges

- **`pix` has no upper bound** (`handler_render_plan.py` checks only `> 0`; UI
  field has no `max`). `finalize-mt` assembles `pix×pix×channels` raw in /tmp;
  `pix=60000` 3-channel = 10.8 GB → OOM / `/tmp` ENOSPC, *after* the raster
  fan-out is paid for. Add an explicit cap (≤ ~32768) rejected early in the
  plan, plus a UI clamp.
- **Solve Map `MaxConcurrency: 500` × 10 GB workers** (`compute_workflow…:607`,
  `polypaint-sweep-mt` 10240 MB, no reserved concurrency) can saturate account
  concurrency; and `Lambda.TooManyRequestsException` is **not** in any Map's
  `Retry.ErrorEquals`, so a throttle fails the whole run instead of backing
  off. Lower `MaxConcurrency` to a sustainable value and add throttle +
  `States.TaskFailed` to the retry list with backoff.
- **Compute/render launches are not idempotent** — `execution_name` uses a
  fresh client `run_id` per click, so a double-submit runs (and bills) the full
  pipeline twice on the same `renders/{job_id}/` keys. Only palette is
  idempotent (deterministic id). Derive the execution name from a content hash,
  or debounce + reject an already-running execution for the same `job_id`.

### [concern] B6 — `solve_source_manifest` duplicated into every render/palette Map item

`logical_sections.py:313` builds a manifest that is both stored in `$.plan` and
copied verbatim into each Map `ItemSelector` (`render_workflow…:159,317,398`;
`palette_workflow…:159,301`). Measured ~202 KB at 1000 chunks, ~848 KB at 4096.
The render guard catches the plan-level copy but the message blames "pix /
section counts" when the real driver is the upstream compute job's chunk count,
making such jobs confusingly un-renderable. Same fix as B1: pass an S3 key, not
the inline object.

### [nit] B7 — Native hardening (no exploitable bug; defense-in-depth)

`root_xforms.h:94` `malloc(sz+1)` is used (`buf[sz]='\0'`, `fread`) without a
NULL check and ignores the `fread` return; `solve_score.h:509,535,842` have
unchecked `malloc` fallbacks that are currently safe *only* because every
caller caps `degree ≤ 1024` (the malloc branch is dead) — guarded siblings at
603/619/793 show the intended pattern. Several `sweep_cli.c` solve-path mallocs
are degree-bounded but unchecked. Also: `sweep_cli.c`/`sweep_mt.c` silently
truncate stdin JSON > 256 KB rather than erroring (a large `coeff_program`
could mis-parse). All low-probability; worth a tidy pass.

### [nit] B8 — Observability / cleanliness

- `handler_coeffgen.py:365,504` write thin error `result_data` (`{"phase":...}`)
  vs the `handler_bilevel.py` gold standard (`_report_phase_error` preserves
  phase/label/artifact_id and regex-recovers params); the message string does
  carry stderr, so it's a gap not a loss.
- Stale `stripe_` term in `handler_storage.py:1980,2251` docstrings (§12 names
  `stripe_count` as the forbidden example; current term is `chunk_`).
- `handler_storage.py:799` `_put_favorite_entry(allow_existing=...)` parameter
  is dead (both branches return `False`); collapse or make it raise on conflict.

### [info / accepted — not action items unless tenancy changes]

- **Unauthenticated API + fully public bucket.** `deploy.sh:117-120` documents
  this as a conscious single-user tradeoff ("Anyone with the URL can read
  artifacts and delete prefixes"); the URL is in the public `config.json`.
  Fine as stated. The moment the stack is shared: add an API authorizer (at
  least on write/destructive routes) and scope the bucket policy to static-asset
  prefixes, serving renders via the existing read-only 1 h presigns. B3's
  `handle_cleanup` hardening applies regardless.
- **Stale local `solve_palette_chunk_mt` binary** (Jun 15, older than its Jun 18
  source). It's gitignored and deploy cross-compiles fresh, so it won't ship
  stale — but rebuild before any local native test, or you'll exercise the
  pre-refactor scalar binary.

## Confirmed Good

- **Native memory safety.** Degree/length/channel caps (`MAX_COEFFS`,
  `MAX_DEGREE`, `COEFF_PROGRAM_MAX_VECTOR_LEN`, `SOLVE_SCORE_MAX_OUTPUT_CHANNELS`)
  are enforced *before* indexing/copy across the coeff VM, `solve_score.h`,
  `multispan_reader.c` (full gap/overlap/overflow tiling validation),
  `roots2pix_mt.c`, `solve_palette_chunk_mt.c`, and `sweep_mt.c`; worker threads
  write disjoint ranges; the long-URL error-truncation lesson is applied
  everywhere (diagnostics formatted before the URL). No blocker-class bug
  reachable from untrusted input.
- **IAM is real least-privilege** — S3 scoped to the one bucket (no `s3:*`, no
  `Resource:"*"`), `lambda:InvokeFunction` on `polypaint-*`, DynamoDB scoped to
  the one table, `states:StartExecution` on the three named ARNs.
- **Secrets clean** — `gemini_key.txt` gitignored, untracked, never in history,
  never on an upload path; `config.json` carries no secrets; repo-wide secret
  scan clean. No `shell=True`/`os.system`; user data reaches native binaries
  only as JSON-over-stdin with fixed argv; ASL `sed` templating uses deploy-time
  constants only.
- **Presigns** are read-only (`get_object`), 1 h TTL, no write/POST presigns.
- **DynamoDB** TTL correct, on-demand billing, partition-scoped queries (no
  scans), batched deletes with backoff; async-retry deliberately 0 for most
  workers to prevent storms. Input numerics are validated/clamped (the gap is
  array-size-in-state, not missing validation).
- **Palette deterministic id, serpentine ordering, contract packaging, and the
  two-sided RePalette gate** are all correct (B2 is coverage, not correctness).
- **`solve_score_chain` fingerprint** is the single canonical sha256, complete
  (transitively encodes omega/phase/lag/emit modes) and stable; validators are
  uniformly contextual; no mutable default args.

## Verification Run

Directly verified by me at HEAD `0c0075a` (not relayed): B1 (no `MAX_PLAN_BYTES`
in compute vs present in render/palette; `chunk_items` inline at `:371`;
`MAX_CHUNKS=5000`); B3 (`handle_cleanup` arbitrary-key delete + `except: pass`
+ over-count at `:2249`); B4 (`handler()` calls destructive routes raw, no outer
try/except at `:844`); and the B2-resolution claims (`536a37b`: e2e corrected to
assert RGB-enabled, plus the variant-identity, ASL-forwarding, multi-channel
finalize-ordering, and native-parity tests now present at the cited lines).
Full Python suite green at HEAD (**1297 passed, 1 skipped, 33 subtests**) — note
it still does **not** include Playwright or the docker native gate, so the
(now-corrected) e2e test and native byte-parity are pinned in code but only run
under those separate gates.

## Recommended Next Patch (re-ranked against HEAD `0c0075a`)

1. **B1** — add the `MAX_PLAN_BYTES` guard to `handler_compute_plan` now (cheap,
   mirrors render/palette; converts a cryptic `States.DataLimitExceeded` into a
   clean fail-fast error — note the fused path still spends degree-probe work
   before the plan is built/guarded); schedule the `chunk_items`→S3 move as the
   real fix.
2. **B3 / B4** — prefix-guard + honest counting in `handle_cleanup`; wrap the
   unwrapped storage routes in the error envelope.
3. **B5 / B6** — `pix` cap; solve `MaxConcurrency` + throttle retry;
   compute/render idempotency; manifest-to-S3. Next-tier hardening, not
   immediate deploy blockers **unless you expect very large jobs** (the same
   trigger as B1).
4. **B2 (UI only)** — move/rename the palette Generate button (Stage-0). The
   backend and validation are complete and pinned; this is the lone cosmetic
   remnant.
5. **B7 / B8** — native malloc checks, observability nits.

B1 is the one I'd not stack further large changes on without addressing; B3/B4
are cheap and worth doing regardless; B5/B6 scale with job size; B2/B7/B8 land
incrementally.
