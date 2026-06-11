# Deployment Checklist

> This is not optional. Fukin do it.
>
> If a change touches native binaries, shared libraries, layers, `LD_LIBRARY_PATH`, Docker-built artifacts, or Lambda runtime packaging, the relevant Docker/runtime test is a hard gate. Do not claim the feature is ready until that Docker check passes and actually covers the changed binary/path.
>
> If a change touches source for a deployed native binary, rebuild that deploy
> artifact first. Do not run the Docker/runtime gate against stale binaries in
> `lambda/` and then "discover" they were outdated.

This is the minimum readiness checklist for any new PolyPaint feature.

If a feature changes backend behavior, frontend behavior, packaging, deploy wiring, native binaries, layers, or runtime dependencies, it is **not ready** until the relevant items below are checked.

When claiming a feature is ready, explicitly report:

- which checklist items were checked
- which exact commands/tests were run
- which items were not checked

## 0. Working Discipline

- Before doing substantive work on a feature, read this checklist first.
- Do not treat this as a deploy-only document. Use it to drive implementation,
  verification, and the final readiness call.
- If a command family is already known to need escalation in this environment,
  request escalation immediately instead of wasting a first sandbox attempt.
- In this repo, treat `uv` commands as immediate-escalation commands.
- In this repo, treat Playwright commands as immediate-escalation commands.
  - Example: `npx playwright test ...`
- In this repo, treat `bash scripts/predeploy_check.sh` as an
  immediate-escalation command because it invokes `uv` and hits the same shared
  cache sandbox boundary.
- In this repo, treat `bash scripts/test-docker-runtime.sh` as an
  immediate-escalation command because it needs Docker socket access outside the
  sandbox. Do not run it sandboxed first just to rediscover the Docker socket
  permission denial.
- Do not "rediscover" the same sandbox/cache/web-server failure on `uv` or
  Playwright or `scripts/predeploy_check.sh` or `scripts/test-docker-runtime.sh`.
  Escalate first.
- If a field, flag, mode, or contract branch is removed, remove it end-to-end
  in the same change.
  - Check upstream emitters, planner output, ASL threading, handler reads,
    native CLI/help text, metadata writers, and tests.
  - Do not leave dead compatibility fields or tautological selectors in active
    paths after the feature that needed them is gone.

## 1. Product Wiring

- The feature is reachable from the intended UI surface.
- The visible controls exist in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html).
- The feature is actually wired to a live action, not a dead button.
- Status/log text is specific enough to debug failures.
- Stale-state behavior is defined where relevant.

## 2. Frontend Contract

- Any new frontend service key is present in generated `config.json` from [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh).
- [api_manifest.json](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.json) is updated.
- If `index.html`, `deploy.sh`, `handler_storage.py`, or `handler_dispatch.py`
  changed in a way that affects service keys, storage paths, solver mappings,
  or dispatch targets, regenerate the tracked manifest in the same change:
  - `python3 api_manifest.py --write`
- `python3 api_manifest.py --check` passes.
- If the feature depends on a new config key, stale loaded config is handled safely.
  - Example: auto-reload config once, or fail with an explicit actionable message.

## 3. Backward Compatibility / Stale Clients

- Old browser tabs with stale in-memory `config.json` are handled safely.
- Old checked-in or cached payload shapes are handled safely where relevant.
- Old metadata rows / calc payloads / artifact rows are handled safely where relevant.
- If a new feature depends on new frontend config keys, the stale-client path is tested explicitly.
- If an artifact metadata contract changes, every read path is updated together.
  - Example: moving fields from S3 image headers into sidecar JSON or `meta.json`
    means readers must use the shared merged-metadata loader, not raw
    `head_object(...).Metadata`.
  - Tests must cover both:
    - legacy header-only artifacts
    - current header + sidecar/overlay artifacts

## 4. API / Route Wiring

- The Lambda handler exists.
- The route is published in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh).
- The route is represented in config generation.
- If the feature is exposed through API Gateway, API Gateway has explicit `lambda:add-permission` invoke rights for that Lambda.
- Route, integration, config entry, and API Gateway invoke permission stay in sync.
- The route is covered by contract tests if applicable.

## 5. Deploy Packaging

- The handler package includes all required local Python modules.
- If a deployed handler gains a new local import from `lambda/*.py`, update the
  corresponding `cp ... "$DIR/"` packaging block in
  [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) in the
  same change.
- If code is deduplicated into a new shared helper such as `calc_chunks.py` or
  `logical_sections.py`, every deployed handler that imports that helper must be
  repackaged in the same change.
- Do not assume "tests passed locally" means deploy packaging is correct.
  Importing a new helper like `logical_sections.py`, `param_source.py`, or any
  other repo-local module changes the Lambda zip contract and must be reflected
  in `deploy.sh`.
- Check every affected package block explicitly:
  - the handler's own zip
  - any sibling/bench zip that bundles that handler too
  - any duplicated create/update packaging path if present
- The handler package includes all required native binaries.
- Every included native binary has `chmod +x` in deploy packaging.
- Required layers are attached.
- Required environment variables are attached.
- Required `/tmp` size is attached.
- Required IAM permissions are attached if the handler touches AWS resources.
- If the feature changes a Step Functions workflow template, every deploy path that renders that template is checked.
  - In this repo that usually means both the create and update renderings in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh), not just one sed block.
- If workflow/config rendering logic is duplicated, the target fix is to extract one shared helper used by both deploy and tests.
- Parity tests are only a temporary guard while duplication still exists; they are not the desired end state.
- Artifact-rendering tests should call the same helper/path that deploy uses, not a parallel substitution implementation.
- If a planner, worker, or handler contract field is added, removed, renamed, or changes meaning, every relevant Step Functions `Parameters` / `ItemSelector` / JSONPath reference must be updated in the same change.
- Do not assume handler-side runtime validation is sufficient. Check contract drift explicitly across:
  - planner output
  - ASL payload threading
  - worker/handler payload reads
  - workflow-definition tests
- Non-recoverable deploy operations must not be hidden behind `|| true` or blanket `2>/dev/null` suppression.
- Re-runnability should come from specific idempotence handling for expected "already exists" errors, not from swallowing every failure.

This must be enforced by:

- [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py)
- [scripts/predeploy_check.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/scripts/predeploy_check.sh)
- the built-in `Running predeploy contract gate...` step inside [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)

Packaging-specific hard rule:

- if you add or change any local dependency imported by a deployed handler,
  run at minimum:

```bash
../.venv/bin/python -m pytest tests/test_deploy_packaging.py -q
bash scripts/predeploy_check.sh
```

- Do not wait for `./deploy.sh update` to discover the mismatch.
- If the task is being called deploy-ready or you are about to actually deploy,
  the built-in predeploy gate inside `./deploy.sh update` must pass on that same
  tree. Treat a failing predeploy gate as a hard stop, not as post-hoc feedback.
- The feature is not ready if `deploy.sh` has stale file lists, even if the
  handler works in direct local tests.
- For local contract/test commands in this repo, prefer `uv run python` when
  `uv` is available. The repo venv is the fallback, not the default.
- In this environment, `uv` should be treated as an immediate-escalation path
  because the shared cache often sits outside the sandbox.
- If a needed `uv` command matters to the task, request escalation first rather
  than probing in the sandbox.

## 6. Resource Budget

- The feature fits Lambda memory.
- The feature fits Lambda `/tmp`.
- The feature fits Lambda timeout.
- The feature fits S3 user-metadata header limits if it writes object metadata.
  - S3 user metadata is small; bulky fields like `render_execution`,
    solve-score programs, or derived-artifact state belong in sidecar JSON /
    `meta.json`, not image headers.
  - Add a pre-upload size guard when an image/object write depends on metadata.
- The feature fits API Gateway synchronous payload limits if it is sync.
- The feature fits Step Functions state-size limits if it uses Step Functions.
- The feature fits intended concurrency/fan-out without obvious service-pressure bugs.
- Any relevant size estimate is explicit in code or logs instead of being implicit guesswork.

## 7. IAM / Permissions

- Any new AWS API use is matched by explicit IAM permissions in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh).
- If the feature adds DynamoDB actions, S3 actions, Step Functions calls, Lambda invokes, or other IAM-sensitive behavior, those permissions are checked directly.
- If the feature adds an API Gateway route, verify the Lambda is included in the API Gateway invoke-permission grant path, not just route/integration creation.
- The feature is not considered ready if it only works locally or in mocks but the role is missing actions.
- Preferred pattern: handlers declare required actions explicitly, and deploy tests compare the deployed role policy against the union of those declarations.

## 8. Runtime Dependency Reality

If the feature depends on native binaries, shared libraries, libvips, LAPACK, or other layer/runtime details:

- do not assume packaging implies runtime availability
- rebuild the affected deploy binaries in `lambda/` before running the Docker gate
- add or update a real runtime regression in:
  - [tests/docker_runtime_regression.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/docker_runtime_regression.py)
- run:
  - `bash scripts/test-docker-runtime.sh`

Explicit rule:

- source changed, binary shipped, Docker test needed
- therefore rebuild first, test second
- testing a stale mounted binary does not count as verification

This is required when a feature introduces:

- a new binary/layer combination
- a new libvips path
- a new LAPACK/OpenBLAS path
- a new helper binary dependency
- a new native binary whose `.so` dependencies could drift

Runtime checks should include shared-library resolution, not just binary presence:

- e.g. run `ldd` in the Lambda-like container and assert there are no `not found` lines

Hermetic-linkage rule (from the 2026-06 import outage):

- Never put bundle paths (`/var/task/...`) on a function's `LD_LIBRARY_PATH`.
  Staged libraries (OpenSSL) load into the runtime python itself and break
  `import ssl` at INIT — and only against the *pinned* runtime version, so
  local Lambda-image tests can pass while production dies.
- Native binaries must self-resolve their whole dependency closure via
  `DT_RPATH` (`-Wl,--disable-new-dtags -Wl,-rpath,$ORIGIN/lib`).
  `LD_LIBRARY_PATH` entries may only reference layer paths (`/opt/...`).
- Enforced by `deploy_manifest.py --check` (deploy refuses), the
  `/var/task/lib` ban in tests/test_deploy_packaging.py, and the DT_RPATH
  assertion in tests/docker_runtime_regression.py.
- The same class applies to layers: `/opt/lib` is on `LD_LIBRARY_PATH`, so a
  layer must never ship glibc-family or OpenSSL libraries, and the
  frozen-version shadows python relies on (zlib/bz2/ffi/gcc_s) are pinned in
  the Docker gate. Rebuilding any layer or libcurl binary against current
  Amazon Linux while the runtime stays pinned is the trigger for this whole
  failure family — after any such rebuild, run the Docker gate, redeploy,
  and run the post-deploy INIT sweep before trusting anything.

## 9. Handler Tests

- Add or update handler tests that call the actual Python handler function directly.
- Mock only external boundaries where necessary.
- Validate real response shape, error shape, and cleanup behavior.
- Validate that the handler returns useful contextual errors.

## 10. Frontend Tests

- Add or update [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh) so:
  - the new controls are asserted to exist
  - the new action is actually invoked
  - the expected request shape is verified
  - any critical stale/config/error behavior is covered
- Treat Playwright as an immediate-escalation path in this environment.
- Do not do a sandbox-first attempt for Playwright just to rediscover that the
  local web server cannot bind.

## 11. Native / Syntax Checks

Run the relevant syntax/build checks for touched code:

- Python:
  - `python3 -m py_compile ...`
- Shell:
  - `bash -n deploy.sh`
- C if changed:
  - `cc -fsyntax-only ...` or actual target build if needed

## 12. Failure Surface

- Errors must include enough context to debug:
  - phase
  - function / mode
  - job / task / chunk / section / range where relevant
  - object key / path where relevant
  - size / estimate where relevant
- No bare strings like:
  - `failed`
  - `Internal Server Error`
  - `failed after retries` without context
- Do not mention obsolete controls or terms in active errors/logs.
  - Example: if the current UI exposes `chunks`, active errors must not talk
    about `stripe_count`.

See also:

- [report-all-errors-with-context.md](/Users/nicknassuphis/karpo_hackathon/polypaint/report-all-errors-with-context.md)

## 13. Observability

- It is clear where progress appears:
  - browser log
  - DynamoDB status rows
  - CloudWatch logs
  - Step Functions execution history
- The feature has enough timing/size fields to debug slowness, not just success/failure.
- The active code path is visible in logs.
- A live production failure can be debugged without reading source first.

## 14. Persistence Semantics

- If the feature is supposed to be ephemeral, verify it does not leak into:
  - S3 durable artifacts
  - DynamoDB task rows
  - Results / Favorites / Render inventories
- If the feature is supposed to be durable, verify metadata and cleanup paths exist.

## 15. Migration / Cleanup

- If the feature creates new durable artifacts, metadata, indexes, or rows, the migration story is explicit.
- Old durable objects are either:
  - still supported
  - explicitly rejected with a clear error
  - or migrated/backfilled
- Cleanup of stale/partial outputs is defined.

## 16. Performance Acceptance

- There is an explicit statement of what “fast enough” means for the feature.
- There is at least one concrete command/test/log that demonstrates the expected performance envelope.
- Do not treat “works once” as equivalent to performance acceptance.

## 17. Rollback / Disable Path

- There is a fast recovery path if the feature breaks in production.
- This can be:
  - hide/disable the UI control
  - revert a route
  - lower concurrency
  - remove a config key
  - redeploy a prior known-good version
- The rollback path should be known before the feature is called ready.
- If deploy mutates multiple AWS resources, `deploy.sh` should record which step failed so rollback starts from facts instead of guesswork.

## 18. Deploy Readiness Gate

Before saying “ready”, run the relevant subset of:

```bash
python3 api_manifest.py --write
python3 api_manifest.py --check
bash -n deploy.sh
../.venv/bin/python -m pytest tests/test_deploy_packaging.py -q
bash scripts/predeploy_check.sh
bash tests/test_frontend_js.sh
../.venv/bin/python -m pytest -q <targeted tests>
bash scripts/test-docker-runtime.sh
```

Not every feature needs every command, but any skipped item must be called out explicitly.

When the touched code produces a deploy binary, insert one step before the
Docker gate:

```bash
# rebuild affected deploy binary/binaries first
...
bash scripts/test-docker-runtime.sh
```

## 19. Post-Deploy Reality Check

After `./deploy.sh update`, do not assume the deployed behavior matches the
current tree just because AWS accepted the update.

Run:

```bash
bash scripts/postdeploy_init_check.sh
./deploy.sh show-build
```

The INIT sweep invokes every deployed function once with a meaningless probe
payload and fails on `Runtime.ImportModuleError` (and friends): an
import-dead Lambda 500s on every call but is invisible until someone clicks
the feature. A handler-level error on the probe payload is a pass — only
"cannot start" fails. Run it before any manual smoke testing.

Treat `show-build` as the source of truth for:

- deployed frontend/config vs local source
- deployed critical Lambda bundle hash vs local packaged bundle
- deployed Step Functions definition vs local rendered definition

If the app still shows stale behavior:

- if `show-build` says `MATCH`, debug the runtime path
- if `show-build` says `MISMATCH`, stop guessing and fix deploy drift first

The Docker gate is only valid if it exercised freshly rebuilt artifacts.

For new user-visible features, the final confidence level must distinguish:

- `locally verified`
- `runtime verified in docker`
- `deployed and live-verified`

Do not collapse those into one statement.

For user-facing features, add:

- one real post-deploy smoke invocation
- confirmation that the expected UI surface works against the deployed backend
- confirmation that the deployed logs/status/error path is actually readable

Minimum default smoke floor for major user-facing releases:

- one compute flow
- one render color flow
- one render bilevel flow
- one palette flow
- one compute-preview flow

When routes are added or changed, also run at least one deployed payload-shape
smoke against the real API route and assert it returns structured JSON rather
than a permission or gateway failure.
