# Code Review 35

Date: 2026-07-20

Reviewed range: `16326fb..b792450`

- Baseline: `16326fb Root pad window control: square of side d centered on 0`
- Reviewed HEAD: `b792450 Recreate giga_1..giga_30: the early wave, 30 programs uploaded`
- Scope: 85 commits, 287 files, 29,762 insertions, 1,936 deletions
- Review mode: adversarial static analysis, targeted executable probes, native test review, frontend runtime tests, and the complete predeploy gate

## Verdict

The branch has substantial useful work and the complete automated gate is green, but the new feature set is not yet production-safe. I found 28 actionable issues: 11 high, 15 medium, and 2 low.

The most serious defects are not cosmetic. The new source "registers" do not have register semantics, tiny programs can cause exponential compiler expansion, constant-index folding gives untrusted arithmetic to Python `eval`, Poly Sheet orchestration can be permanently stranded by a browser reload or an early worker exception, classic CM can exceed its Lambda memory allocation, the API accepts 64-bit solvers with a topology that cannot produce their input, and the newly added Kill path appears to lack a usable `states:StopExecution` permission.

The fact that predeploy passes is important, but it does not contradict these findings. Several tests currently pin the faulty behavior, exercise only happy paths, or contain no assertion capable of detecting the defect.

## Findings Summary

| ID | Severity | Finding |
|---|---|---|
| CR35-F1 | HIGH | Source "registers" are textual aliases, so reading a nondeterministic RHS again changes the program result |
| CR35-F2 | HIGH | Rebinding source locals expands exponentially before any work or expanded-size limit |
| CR35-F3 | HIGH | Constant index folding evaluates attacker-controlled exponentiation without a work budget |
| CR35-F4 | HIGH | Poly Sheet uses the browser as its durable workflow orchestrator |
| CR35-F5 | HIGH | Async sheet and DeepZoom workers can fail before writing status, leaving infinite polls |
| CR35-F6 | HIGH | Poly Sheet's budget estimator is disconnected from the actual degree and work performed |
| CR35-F7 | HIGH | Exact root literals impose multi-second, repeatedly duplicated compile work |
| CR35-F8 | HIGH | Classic `sweep_cm` can exceed the 4096 MB Lambda allocation through whole-file buffering |
| CR35-F9 | HIGH | The backend accepts 64-bit solvers with the incompatible classic workflow topology |
| CR35-F10 | HIGH | The Kill IAM policy grants `StopExecution` against the wrong Step Functions resource type |
| CR35-F11 | HIGH | Stop handlers trust a client-provided execution ARN independently of the job/task being updated |
| CR35-F12 | MEDIUM | CM/JT/Newton row threading is not wired into the classic production workflow |
| CR35-F13 | MEDIUM | Outer solver threads can oversubscribe an independently threaded OpenBLAS |
| CR35-F14 | MEDIUM | Poly Sheet token replacement corrupts overlapping names and counts tokens in comments |
| CR35-F15 | MEDIUM | An explicit zero solver shim is silently converted to 5 percent |
| CR35-F16 | MEDIUM | An omitted scan step executes as 1 but is persisted as 0 |
| CR35-F17 | MEDIUM | Sheet cancellation is permanent, fail-open on S3 errors, and retained in stale frontend state |
| CR35-F18 | MEDIUM | Stitch publishes the final sheet before cleanup, then can report the published sheet as failed |
| CR35-F19 | MEDIUM | Sheet object keys and status have no generation ownership |
| CR35-F20 | MEDIUM | `/list-sheets` scans every temporary frame object under `sheets/` |
| CR35-F21 | MEDIUM | DeepZoom listing failures are treated as cache misses and trigger duplicate exports |
| CR35-F22 | MEDIUM | Poly Sheet drops solver iteration limits and Populate Frame does not restore actual frame bounds |
| CR35-F23 | MEDIUM | Newton accepts iteration limits 51 through 64 but silently executes 50 |
| CR35-F24 | MEDIUM | The expensive-root preview guard omits `roots_ae` and does not inspect the active program in the frontend |
| CR35-F25 | MEDIUM | Preview marquee coordinates are wrong when the preview image is rotated |
| CR35-F26 | MEDIUM | Root-pad interaction rewrites untouched constants at six significant digits and can snap on pointer-down |
| CR35-F27 | LOW | Dismissing a live Jobs-rail item is temporary because the next poll recreates it |
| CR35-F28 | LOW | A new compiler test contains a vacuous self-equality assertion |

## Detailed Findings

### CR35-F1 - HIGH - Source "registers" are aliases, not evaluated values

`SourceLocals` stores and substitutes source text rather than an evaluated value or VM slot (`lambda/program_source_core.py:314-388`). Its implementation assumes repeated substitution is safe because expressions are deterministic. That assumption is false for newly composable operations such as `littlewood`, whose native result includes the token index in its seed (`lambda/sweep_cli.c:5691-5738`). The same local-substitution mechanism is enabled across Coeff, Param, and Root source paths (`lambda/program_source_core.py:614-632`, `lambda/param_program_source.py:247-286`, `lambda/coeff_program_source.py:1161-1213`, and `lambda/root_program_source.py:421-445`).

I compiled and executed these two Coeff programs at `t1=0.2, t2=0.3`:

```text
r = littlewood(0, 1)
poly = r
emit
```

```text
r = littlewood(0, 1)
poly = r
r
emit
```

The first produced `[[0, 0]]`; the second produced `[[1, 0]]`. Merely reading `r` again lowers another `littlewood` operation and changes the value used by the earlier `poly = r`. That is incompatible with any ordinary meaning of assignment or register storage.

Composability is also operation-dependent. `r = scan(...); poly = add(r, r)` compiles, while `r = littlewood(0, 1); poly = add(r, r)` fails with `unknown scalar expression identifier 'littlewood'`. The syntax therefore looks uniform while its semantics depend on the shape of the textual RHS.

`tests/test_coeff_program_registers.py:1-10` and `:62-92` explicitly test hand-inlining with deterministic expressions. They currently pin macro-expansion semantics rather than register semantics, so they cannot detect this bug.

**Required change:** make the contract explicit. Either rename the feature to textual aliases/macros and restrict RHS expressions to safe, deterministic, composable forms, or implement actual evaluate-once VM local slots. If the UI continues to call these registers, real slots are the correct implementation.

### CR35-F2 - HIGH - Local rebinding causes exponential compiler expansion

`SourceLocals.try_define` recursively substitutes the complete previous expression into every reference in a new definition (`lambda/program_source_core.py:354-387`). The source byte limit is checked only against the original text (`lambda/program_source_core.py:391-399`), not against expanded expressions or the resulting chain.

A roughly 200-byte program beginning with `r = fill(1, 1)` and repeatedly applying `r = add(r, r)` produced this growth locally:

| Rebindings | Expanded chain representation | Compile time |
|---:|---:|---:|
| 8 | 29,445 bytes | 0.008 s |
| 10 | 117,765 bytes | 0.032 s |
| 12 | 471,045 bytes | 0.141 s |
| 14 | 1,884,165 bytes | 0.615 s |
| 16 | 7,536,645 bytes | 2.681 s |

The representation grows by approximately 4x for every two added lines. A tiny accepted request can therefore force large allocations and long compilation before downstream token limits get a chance to reject it. Coeff, Param, and Root share this machinery.

**Required change:** use a DAG or real local slots, and independently enforce expanded-node, expanded-byte, nesting-depth, and compile-time budgets before materializing expansions. The raw source-size cap is not a work limit.

### CR35-F3 - HIGH - Constant index folding performs unbudgeted Python evaluation

`_try_fold_constant_index` accepts a restricted-looking arithmetic string and passes it to Python `eval` with builtins removed (`lambda/coeff_program_source.py:512-538`). It is used by indexed Coeff assignment lowering at `lambda/coeff_program_source.py:1234-1263`.

The regex permits `*`, and therefore permits `**`. Disabling builtins prevents ordinary name-based code execution, but it does not bound integer exponentiation. Bounds checking happens only after Python has evaluated the expression. For example, this path is live and folds the index to 8:

```text
poly = fill(10, 0)
poly
poly[2**3] = tos[0]
drop
emit
```

Replacing the index with a compact exponent tower can consume disproportionate CPU and memory before the compiler rejects the eventual index.

**Required change:** do not use Python `eval`. Reuse a bounded static-expression parser, or support a deliberately small integer grammar with explicit operator count, nesting, exponent, magnitude, and evaluation-time limits.

### CR35-F4 - HIGH - The browser is the durable Poly Sheet orchestrator

The frontend dispatches sheet workers, polls each worker group, and then dispatches stitch (`js/16-poly-sheets.js:94-192`). The active run exists only in `_activeSheetRun` (`js/16-poly-sheets.js:8-9`), and running jobs are intentionally excluded from persistent browser state (`js/10-status-results.js:1876-1881`). Poll loops at `js/16-poly-sheets.js:195-232` have no persisted continuation, transient-error retry policy, missing-status terminal condition, or resume path.

Consequences:

- Closing or reloading the page after worker dispatch prevents stitch from ever being dispatched.
- A single `/check-status` network error exits the client workflow even if every Lambda continues successfully.
- A partial dispatch can leave accepted workers running after the UI reports dispatch failure (`js/16-poly-sheets.js:154-160`).
- A stitch-dispatch failure strands all completed temporary tiles.

This is not merely weak UX; the browser is the only coordinator for a distributed asynchronous workflow.

**Required change:** move orchestration to a durable backend workflow, preferably Step Functions. A smaller alternative is a persisted run descriptor with idempotent resume, worker-count reconciliation, stitch ownership, retry deadlines, and explicit cleanup. The tab must be able to reconstruct and continue an in-progress run after reload.

### CR35-F5 - HIGH - Workers can fail before writing their first status row

Poly Sheet worker validation and substitution occur before the first `report_status` call (`lambda/handler_poly_sheet.py:588-604`). The top-level run path has the same ordering at `lambda/handler_poly_sheet.py:720-734`. Sheet DeepZoom validates input before entering its protected/status-reporting region (`lambda/handler_deepzoom_export.py:336-349`). Both async Event targets are configured with zero retries (`deploy.sh:488-497`).

If any pre-status operation raises, the Event invocation disappears without creating a terminal row. `/check-status` treats an empty result set as incomplete rather than terminal (`lambda/handler_storage.py:5879-5994`), so the frontend can poll forever.

**Required change:** synchronously validate before accepting the async request, or create an owned `accepted/computing` status row before invocation. The worker must wrap every operation after acceptance and always publish a terminal error. Pollers also need a stale/missing-row deadline so an absent status cannot mean "still working" indefinitely.

### CR35-F6 - HIGH - Poly Sheet's budget estimator does not estimate the requested job

`_enforce_worker_budget` hardcodes `degree = 40` and `fused_threads = 2` (`lambda/handler_poly_sheet.py:472-481`). The actual source can produce degrees up to 255, exact root-literal expansion is omitted, and rendering plus PNG encoding are not represented. `_render_frame_tile` recompiles the substituted source for every frame (`lambda/handler_poly_sheet.py:484-490`).

This is a false safety boundary: a request can pass the estimate while doing radically more compile and solve work than estimated. The exact-root measurements in CR35-F7 demonstrate that compile cost alone can consume several seconds per frame before solving starts.

**Required change:** compile and probe a representative frame before fan-out, derive degree and solver topology from the resulting execution spec, and budget conservatively from measured compile, solve, render, and encode costs. Persist the estimate inputs and actual timings so production data can recalibrate the model.

### CR35-F7 - HIGH - Exact root literals repeatedly perform expensive rational expansion

Exact literals use an O(n^2) `Fraction`-based coefficient expansion (`lambda/coeff_program_chain.py:1985-2009`). `_compile_expanded_roots` performs this expansion before `_intern_vector_constant` (`lambda/coeff_program_chain.py:2023-2032`), so two identical literal calls repeat all expansion work even though the final vector pool deduplicates them.

Local compile timings were:

| Literal | Compile time |
|---|---:|
| `grid5` | 0.0027 s |
| `grid9` | 0.019 s |
| `grid12` | 0.755 s |
| `grid15` | 3.072 s |
| `ring255` | 5.487 s |
| `chess22` | 4.542 s |

Two identical `ring255` calls took 11.013 s even though only one vector constant remained in the compiled pool. Poly Sheet recompilation multiplies this cost across frames.

**Required change:** memoize by canonical literal before expansion, use a balanced product tree or otherwise more efficient coefficient construction, and impose an explicit compile-work budget based on root count and exact arithmetic size. Persisted constant vectors should bypass expansion entirely when possible.

### CR35-F8 - HIGH - Classic `sweep_cm` can exceed Lambda memory

The classic handler retains the complete S3 coefficient object in Python while launching the native solver (`lambda/handler_sweep_cm.py:55-64`). The C process then allocates its own complete coefficient input and complete root output (`lambda/sweep_cm.c:206-240`). The function has 4096 MB (`deploy_manifest.json:50-63`).

The classic planner allows `N` up to 50,000, total points up to 2.5 billion, and `chunks=1` without deriving a memory floor (`lambda/handler_compute_plan.py:50-56`, `:113-145`, and `:244`). A legal `degree=70, N=1600, chunks=1` plan requires approximately:

- Python S3 body: 1,454,080,000 bytes
- C coefficient input: 1,454,080,000 bytes
- C root output: 1,433,600,000 bytes
- Total before Python/runtime/BLAS overhead: 4,341,760,000 bytes, about 4,141 MiB

That already exceeds the configured memory before allocator fragmentation, native libraries, stacks, or process overhead.

**Required change:** stream S3 directly to a file rather than retaining a Python bytes copy, use `mmap`/`pread` and `pwrite` in the solver, and make the planner derive a minimum classic chunk count from coefficient, root, runtime, and safety-margin memory.

### CR35-F9 - HIGH - 64-bit solvers are accepted with an impossible classic plan

`_validate_solver_mode` accepts `jt64`, `cm64`, and `ae64` independently of execution method (`lambda/handler_compute_plan.py:689-695`), while the default execution method is classic. The solver helpers then select `polypaint-sweep-mt` with a `fused_*` mode (`lambda/handler_compute_plan.py:726-745`).

In the classic state machine, `CoeffgenMap` only writes coefficients (`stepfunctions/compute_workflow.asl.json:343-398`), then `SolveMap` invokes `sweep_mt` (`:829-844`). `handler_sweep_mt` interprets any `fused_*` mode as already having generated roots and immediately checks for the output object (`lambda/handler_sweep_mt.py:46-79`). The classic path never created that root object.

The resulting combinations are currently constructible:

```text
jt64 -> classic_chunk_pipeline -> polypaint-sweep-mt -> fused_jt64
cm64 -> classic_chunk_pipeline -> polypaint-sweep-mt -> fused_cm64
ae64 -> classic_chunk_pipeline -> polypaint-sweep-mt -> fused_ae64
```

`tests/test_compute_plan.py:55-90` builds these modes without specifying an execution method, so it currently pins the invalid topology instead of rejecting it. The frontend happens to request fused execution (`js/05-render-popups.js:1180`), but the backend contract remains broken for direct or stale clients.

**Required change:** reject 64-bit solvers unless the execution method is fused, or implement a valid classic topology for them. `handler_sweep_mt` should also use an explicit mode allowlist rather than treating every `fused_*` string as equivalent.

### CR35-F10 - HIGH - Kill is granted against the wrong IAM resource

`deploy.sh:1763-1774` places `states:StartExecution` and `states:StopExecution` in one policy statement whose resources are state-machine ARNs. AWS defines `StartExecution` against a `stateMachine*` resource and `StopExecution` against an `execution*` resource. See the [AWS Step Functions service authorization reference](https://docs.aws.amazon.com/service-authorization/latest/reference/list_stepfunctions.html).

The deployed stop call should therefore receive `AccessDenied` even though unit tests pass, because `tests/test_compute_orchestrator.py:48-98` replaces boto3 and never evaluates IAM.

**Required change:** split the IAM statement. Grant `StartExecution` on the intended state-machine ARNs and `StopExecution` on tightly scoped execution ARNs such as `arn:aws:states:${REGION}:${ACCOUNT}:execution:${STATE_MACHINE_NAME}:*`. Add a deployment-policy contract test and a post-deploy stop smoke test.

### CR35-F11 - HIGH - Stop handlers do not bind an execution to the supplied job/task

The compute, render, and palette stop handlers accept an execution ARN from the request, perform only a broad `arn:aws:states:` prefix check, stop that execution, and separately update the caller-supplied job/task status (`lambda/handler_compute_orchestrator.py:154-175`, `lambda/handler_render_orchestrator.py:147-168`, and `lambda/handler_palette_orchestrator.py:75-97`).

Once CR35-F10 is fixed, a stale or mismatched client payload can stop execution A while marking job B as stopped. The handlers also do not constrain account, region, state-machine name, or stored ownership.

**Required change:** consistently read the authoritative status row, require an exact match with its stored execution ARN, validate account/region/state-machine identity, then conditionally write the terminal status only if the same execution still owns the task.

### CR35-F12 - MEDIUM - New native row threading is not used by classic production solves

`sweep_cm` defaults to `n_threads=1` (`lambda/sweep_cm.c:198-202`), and `handler_sweep_cm` passes a thread count only if one is already present in input (`lambda/handler_sweep_cm.py:67-78`). The classic state-machine lores and SolveMap payloads omit it (`stepfunctions/compute_workflow.asl.json:652-667` and `:829-844`). The plan records coefficient and fused thread fields, but no classic solve thread field (`lambda/handler_compute_plan.py:326-358`).

The row-threading feature therefore works in local tests, preview, and sheet paths while the expensive classic production CM/JT/Newton path remains serial.

**Required change:** make `solve_threads` an explicit planned value, carry it through ASL and metadata, and A/B it on the deployed architecture rather than relying on a handler default.

### CR35-F13 - MEDIUM - Solver threads can oversubscribe OpenBLAS

The deployed `sweep_cm` links OpenBLAS (`scripts/build-deploy-binaries.sh:109-112` and `deploy.sh:1091-1094`). No deployment or handler code pins `OPENBLAS_NUM_THREADS`, `OMP_NUM_THREADS`, `GOTO_NUM_THREADS`, or equivalent settings. Each outer row worker calls LAPACK, so several pthread workers may each start a BLAS team on a Lambda with roughly two vCPUs.

The local threading tests use Accelerate rather than the deployed OpenBLAS build (`tests/test_sweep_cm_threads.py:14-35`), so a green local speedup does not establish production scaling.

**Required change:** choose one owner of parallelism. If row workers own it, pin BLAS to one thread. If BLAS owns it, keep outer threads at one. Benchmark both on the deployed Graviton/Lambda configuration and record wall time, CPU time, billed duration, and numerical parity.

### CR35-F14 - MEDIUM - Sheet token substitution is not lexical

The backend uses repeated raw string replacement (`lambda/handler_poly_sheet.py:156-170` and `:456-460`), and Populate repeats the same split/join behavior in JavaScript (`js/16-poly-sheets.js:661-670`).

With tokens `$T` and `$T2`, replacing `$T` first modifies the prefix of `$T2`; the next replacement then reports `$T2` missing. A token appearing only in a comment also satisfies the presence check and generates identical frames rather than reporting that the program does not vary.

**Required change:** perform one lexical substitution pass, match complete token names with longest-match or explicit boundaries, ignore comments, and share frontend/backend fixtures for overlapping names, strings, comments, and missing-token diagnostics.

### CR35-F15 - MEDIUM - A zero solver shim becomes 5 percent

Several paths use truthiness instead of presence:

- `js/02-preview-solvescore.js:123-124` and `:369-370`
- `js/16-poly-sheets.js:64-65`, `:488-490`, and `:678-680`
- `lambda/handler_poly_sheet.py:382-383`

The UI permits zero (`index.html:1803`), and preview validation explicitly accepts it (`lambda/handler_compute_preview.py:238-246`). Nevertheless, `0 || 0.05` and its Python equivalent turn a valid zero into 0.05. I reproduced the backend parser returning 0.05 for an explicit zero.

**Required change:** distinguish `null`/missing/blank from numeric zero in every transport and persistence path. Add a zero-value round-trip test from UI payload through sheet manifest and Populate.

### CR35-F16 - MEDIUM - Scan step execution and persistence disagree

`scan_values` uses a default step of 1 (`lambda/handler_poly_sheet.py:118-132`). `_parse_scan_axis` executes with `spec.get("step")`, but serializes the field as `float(spec.get("step") or 0.0)` (`lambda/handler_poly_sheet.py:426-443`).

For an omitted step, I obtained values `[2, 3, 4]` while the returned axis metadata stored `step: 0.0`. Populate then sends the stored zero on the next run, where it is rejected. A successful run thus serializes a configuration that cannot reproduce itself.

**Required change:** resolve one effective step value and use it for validation, execution, manifest storage, display, and Populate.

### CR35-F17 - MEDIUM - Cancellation has no lifecycle and fails open

Cancellation writes a persistent S3 marker that is never consumed or deleted (`lambda/handler_poly_sheet.py:93-115`). Any `ClientError`, including throttling, AccessDenied, or a 5xx, is interpreted as "not cancelled" rather than only treating `NoSuchKey` as absent. In the browser, `_activeSheetRun` is assigned at `js/16-poly-sheets.js:160` and is not cleared on all success/error exits, so Cancel can later target a stale completed run.

The same `sheet_id`/generation cannot be safely retried because its old marker remains effective, while a transient S3 failure can allow work the user cancelled to continue.

**Required change:** make cancellation generation-scoped, distinguish missing from operational S3 failures, clean or expire markers, and clear active frontend state in a guaranteed `finally` path.

### CR35-F18 - MEDIUM - Cleanup can turn a committed sheet into an error

The stitch path uploads the final PNG and manifest, then deletes temporary objects, then reports done (`lambda/handler_poly_sheet.py:693-717`). If cleanup raises, the outer handler reports an error even though the public artifact has already been committed. The delete call also does not inspect per-key `Errors` in a successful response.

**Required change:** define publication as the commit point. Mark the generation ready after both final objects are durably written; perform cleanup as best-effort garbage collection with separately reported cleanup status. A retry must recognize an already committed generation rather than rebuild from potentially missing tiles.

### CR35-F19 - MEDIUM - Sheet runs have no generation ownership

Temporary frame keys, final keys, cancellation keys, and status identity are all derived from `sheet_id` without a unique run generation. Concurrent, replayed, or retried requests for one ID can mix frame tiles and configuration; a stale worker can overwrite a newer run's object or status. Random frontend IDs lower collision probability but do not establish a backend invariant.

**Required change:** assign a server-generated run/generation ID, include it in every temporary key and status row, persist it in the manifest, and use compare-and-set ownership for worker progress, stitch, cancellation, and final publication.

### CR35-F20 - MEDIUM - Sheet listing scales with temporary debris

`/list-sheets` paginates the entire flat `sheets/` prefix without a delimiter (`lambda/handler_storage.py:6630-6652`). A failed or stranded sheet can leave up to 512 temporary frame objects. Refresh latency and S3 listing cost therefore grow with every abandoned run, not with the number of completed sheets.

**Required change:** list from a compact DDB/S3 index, or use per-sheet prefixes plus delimiter and manifest checks. Add lifecycle expiration for temporary tiles and cancellation objects regardless of normal cleanup success.

### CR35-F21 - MEDIUM - DeepZoom list failures trigger duplicate work

The sheet DeepZoom lookup catches every list failure and returns `null` (`js/16-poly-sheets.js:276-286`). `_sheetViewDeepZoom` interprets `null` as absence and starts a new export (`js/16-poly-sheets.js:417-429`). A timeout, 500, or temporary network loss can therefore launch duplicate expensive work.

**Required change:** distinguish a successful empty result from a failed request. Retry transient listing failures with bounded backoff; generate only after a successful authoritative miss.

### CR35-F22 - MEDIUM - Sheet fidelity omits iteration limits and actual frame bounds

`_sheetInheritedFrame` copies solver mode but not `compute-preview-iters` (`js/16-poly-sheets.js:51-82`). `_solve_frame` does not pass a max-iteration value (`lambda/handler_poly_sheet.py:183-230`), and the manifest omits it (`:542-575`). A preview using a capped AE/Newton solve can therefore differ from the sheet generated from that preview.

Each frame record contains exact bounds (`lambda/handler_poly_sheet.py:521-524`), but `_sheetPopulateFrame` restores scan values and shim rather than the selected frame's actual bounds/mode (`js/16-poly-sheets.js:650-687`). Populate Frame is consequently not a reliable reconstruction of frozen or explicitly bounded frames.

**Required change:** persist and pass solver iteration limits, and restore `rec.bounds` into the compute preview's explicit/marquee state. Add a Preview -> Sheet -> Populate -> Preview identity test.

### CR35-F23 - MEDIUM - Newton iteration limits above 50 are silently rewritten

The UI allows 64 (`index.html:1800`), and backend planning/preview/native parsing accept values through 64 (`lambda/handler_compute_plan.py:698-711`, `lambda/handler_compute_preview.py:547-551`, and `lambda/sweep_cm.c:191-196`). The Newton implementation caps at 50 and resets any larger value to 50 (`lambda/newton_solver.h:27-28` and `:77-80`). I ran a native request with 51 and observed metadata reporting 50.

**Required change:** expose per-solver limits and reject Newton values above 50 before dispatch, or intentionally clamp once and return/store/display the effective value everywhere. Silent cross-layer clamping is not acceptable.

### CR35-F24 - MEDIUM - Expensive-root preview protection excludes `roots_ae`

The frontend guard recognizes only legacy `roots_cm` and `roots_jt` text (`js/02-preview-solvescore.js:378-385`). The backend does inspect compiled Coeff operations, but still checks only CM/JT and emits a CM-specific message (`lambda/handler_compute_preview.py:152-169`). `roots_ae` performs a 64-iteration AE solve for every row (`lambda/sweep_cli.c:3402-3438`) and bypasses both protections.

The frontend string check also misses indirect operations in loaded programs/macros and can false-match comments.

**Required change:** derive cost from the compiled execution spec, include all embedded root solvers, and name the actual expensive operation in the diagnostic. The frontend may provide an early hint, but the backend compiled-program guard must be authoritative.

### CR35-F25 - MEDIUM - Marquee coordinate mapping ignores preview rotation

The preview image is visually rotated with CSS (`js/02-preview-solvescore.js:40-45`), while `relFrac` and bounds conversion treat pointer coordinates as if the image were unrotated (`js/02-preview-solvescore.js:67-105`). The UI title acknowledges the limitation (`index.html:1805`) but still exposes the incompatible control.

**Required change:** inverse-transform pointer coordinates for 90/180/270 degree rotations before mapping to complex bounds, or disable marquee interaction while rotation is nonzero. Add corner-mapping tests for all four rotations.

### CR35-F26 - MEDIUM - Root-pad selection mutates and rounds data

`_scrubFormatNumber` uses `toPrecision(6)` (`js/08-chip-editors.js:1691-1697`). `_rootPadFormatCall` rewrites every root in the call (`js/08-chip-editors.js:1642-1646`), and `_rootPadWrite` commits that rewritten call (`:2195-2199`). Pointer-down immediately invokes drag handling (`:2214-2237`); with the default 0.5 snap, merely selecting an unsnapped root can move it before the user drags.

Existing tests use snap-aligned values and do not assert preservation of untouched token precision (`tests/test_frontend_js.sh:2420-2479`).

**Required change:** preserve original source tokens for unchanged roots, format changed numbers with round-trip precision, and require a movement threshold before mutation. A click/select action must be observational only.

### CR35-F27 - LOW - Dismissed live jobs immediately return

`_jobsRailDismiss` removes a DOM/state item but records no dismissal tombstone (`js/10-status-results.js:1932-1938`). Active sheet and workflow polls continue calling the upsert path, so the item reappears on the next interval.

**Required change:** retain a session-scoped dismissed-ID set until the job reaches terminal state or a new run with the same logical ID begins. Add a runtime test that advances a poll after dismissal.

### CR35-F28 - LOW - A test assertion can never fail

`tests/test_source_locals_and_infix.py:271` contains:

```python
self.assertEqual(compiled["scalar_expr_count"], compiled["scalar_expr_count"])
```

This compares a value to itself and provides no coverage. It likely intended to compare the transformed program with a baseline or a known expected count.

**Required change:** replace it with the intended exact invariant. Add nondeterministic/evaluate-once and expansion-budget cases so CR35-F1/F2 cannot be accidentally blessed by deterministic equivalence tests.

## Areas That Held Up

The review also found several well-implemented areas that should be preserved:

- Custom palette encoding is consistent across the JavaScript canonical wire builder (`js/04-palette-color.js:325-349`), Python validation (`lambda/palette_names.py:20-32`), and the C parser (`lambda/palette_lut.h:208-281`).
- Solve-score `asin`, `acos`, and `atan` handling is aligned between Python and C, including clamp behavior (`lambda/solve_score_eval.py:507-521` and `lambda/solve_score.h:2475-2508`).
- The new CM/JT/Newton native workers use per-thread solver workspace. The native race gate and ThreadSanitizer checks passed; I did not find a row-output data race.
- The new `giga_1..giga_30` data and helper tests passed. Their declared approximations are explicit rather than silently represented as exact historical reconstruction.
- All 59 changed `tests/test_*` files in the reviewed range are present in `scripts/predeploy_check.sh`. Gate registration is materially better than in earlier review cycles.

## Verification

The following checks passed at `b792450`:

- Focused compiler, sheet, solver, and orchestration tests: 121 passed, 37 subtests passed.
- New giga program suites: 16 passed, 207 subtests passed.
- Complete predeploy Python gate: 1,594 passed, 2 skipped, 3 warnings, 2,502 subtests passed in 58.94 s.
- Frontend sequential-load and runtime harness: passed.
- Playwright end-to-end suite: 47 passed in 14.1 s.
- Native gate and ThreadSanitizer checks: passed. One `/dev/full` error-path check was skipped because that device was unavailable in the test environment.
- Final result: `Predeploy contract gate passed.`

No production AWS workflow was launched during this review. Performance timings in CR35-F2 and CR35-F7 are local compiler measurements; CR35-F13 therefore remains a production configuration risk requiring Lambda A/B telemetry rather than a claimed measured slowdown.

## Recommended Fix Order

1. Fix the language safety boundary first: CR35-F1, F2, F3, and F7. Do not expose "registers" as value storage until their semantics are real, and enforce compiler work budgets before expansion/evaluation.
2. Make Poly Sheet durable: CR35-F4, F5, F6, and F14 through F22. A server-owned generation and orchestration state should precede further UI polish.
3. Repair compute execution invariants: CR35-F8 through F13, F23, and F24. In particular, reject impossible solver/topology combinations and correct StopExecution IAM before relying on Kill.
4. Fix editor data-preservation defects: CR35-F25 and F26.
5. Tighten the gate with regression tests for every reproduced case, including the currently vacuous CR35-F28 assertion. The full suite being green today demonstrates that new tests must target failure semantics and lifecycle transitions, not just successful compilation and dispatch.

## Release Assessment

The existing green gate supports continued development, but it is not sufficient evidence to deploy the new feature set as production-ready. CR35-F1 through F11 should be treated as release blockers for the affected features. The safest sequence is to disable or hide unsafe new entry points while fixing them, rather than relying on frontend validation to protect backend/compiler invariants.
