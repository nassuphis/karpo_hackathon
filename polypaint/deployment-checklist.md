# Deployment Checklist

This is the minimum readiness checklist for any new PolyPaint feature.

If a feature changes backend behavior, frontend behavior, packaging, deploy wiring, native binaries, layers, or runtime dependencies, it is **not ready** until the relevant items below are checked.

When claiming a feature is ready, explicitly report:

- which checklist items were checked
- which exact commands/tests were run
- which items were not checked

## 1. Product Wiring

- The feature is reachable from the intended UI surface.
- The visible controls exist in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html).
- The feature is actually wired to a live action, not a dead button.
- Status/log text is specific enough to debug failures.
- Stale-state behavior is defined where relevant.

## 2. Frontend Contract

- Any new frontend service key is present in generated `config.json` from [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh).
- [api_manifest.json](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.json) is updated.
- `python3 api_manifest.py --check` passes.
- If the feature depends on a new config key, stale loaded config is handled safely.
  - Example: auto-reload config once, or fail with an explicit actionable message.

## 3. Backward Compatibility / Stale Clients

- Old browser tabs with stale in-memory `config.json` are handled safely.
- Old checked-in or cached payload shapes are handled safely where relevant.
- Old metadata rows / calc payloads / artifact rows are handled safely where relevant.
- If a new feature depends on new frontend config keys, the stale-client path is tested explicitly.

## 4. API / Route Wiring

- The Lambda handler exists.
- The route is published in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh).
- The route is represented in config generation.
- If the feature is exposed through API Gateway, API Gateway has explicit `lambda:add-permission` invoke rights for that Lambda.
- Route, integration, config entry, and API Gateway invoke permission stay in sync.
- The route is covered by contract tests if applicable.

## 5. Deploy Packaging

- The handler package includes all required local Python modules.
- The handler package includes all required native binaries.
- Every included native binary has `chmod +x` in deploy packaging.
- Required layers are attached.
- Required environment variables are attached.
- Required `/tmp` size is attached.
- Required IAM permissions are attached if the handler touches AWS resources.

This must be enforced by:

- [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py)

## 6. Resource Budget

- The feature fits Lambda memory.
- The feature fits Lambda `/tmp`.
- The feature fits Lambda timeout.
- The feature fits API Gateway synchronous payload limits if it is sync.
- The feature fits Step Functions state-size limits if it uses Step Functions.
- The feature fits intended concurrency/fan-out without obvious service-pressure bugs.
- Any relevant size estimate is explicit in code or logs instead of being implicit guesswork.

## 7. IAM / Permissions

- Any new AWS API use is matched by explicit IAM permissions in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh).
- If the feature adds DynamoDB actions, S3 actions, Step Functions calls, Lambda invokes, or other IAM-sensitive behavior, those permissions are checked directly.
- If the feature adds an API Gateway route, verify the Lambda is included in the API Gateway invoke-permission grant path, not just route/integration creation.
- The feature is not considered ready if it only works locally or in mocks but the role is missing actions.

## 8. Runtime Dependency Reality

If the feature depends on native binaries, shared libraries, libvips, LAPACK, or other layer/runtime details:

- do not assume packaging implies runtime availability
- add or update a real runtime regression in:
  - [tests/docker_runtime_regression.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/docker_runtime_regression.py)
- run:
  - `bash scripts/test-docker-runtime.sh`

This is required when a feature introduces:

- a new binary/layer combination
- a new libvips path
- a new LAPACK/OpenBLAS path
- a new helper binary dependency

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
  - job / task / chunk / tile where relevant
  - object key / path where relevant
  - size / estimate where relevant
- No bare strings like:
  - `failed`
  - `Internal Server Error`
  - `failed after retries` without context

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

## 18. Deploy Readiness Gate

Before saying “ready”, run the relevant subset of:

```bash
python3 api_manifest.py --check
bash -n deploy.sh
bash tests/test_frontend_js.sh
../.venv/bin/python -m pytest -q <targeted tests>
bash scripts/test-docker-runtime.sh
```

Not every feature needs every command, but any skipped item must be called out explicitly.

## 19. Post-Deploy Reality

If the feature is new and user-visible, the final confidence level should distinguish:

- `locally verified`
- `runtime verified in docker`
- `deployed and live-verified`

Do not collapse those into one statement.

If the feature is user-facing, add:

- one real post-deploy smoke invocation
- confirmation that the expected UI surface works against the deployed backend
- confirmation that the deployed logs/status/error path is actually readable
