# Robust Testing Plan

## Problem

The current failure mode is:

1. code changes look locally correct
2. unit/static tests pass
3. `deploy.sh update` takes several minutes
4. the first real click in the browser fails immediately

That is unacceptable for this app, because:

- deploy is slow
- the UI surface is large
- API contracts are duplicated across frontend, Lambda handlers, and `deploy.sh`
- one missed string literal burns a full deploy cycle

The `Favorites` failure was the exact example:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) used `/list-favorites`, `/add-favorite`, `/delete-favorite`
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py) implemented them
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) did not publish those routes

So the tests were not wrong so much as incomplete. They did not cover the real contract boundary that mattered.

## Goal

Move failures from:

- browser after deploy

to:

- local test failure before deploy starts

The deploy command should become the final publish step, not the first time the system contract is exercised.

## Design Principles

1. One source of truth for contracts.
2. Local contract checks must run before any AWS work.
3. Frontend flows need endpoint-level tests, not just “button exists” checks.
4. Deploy should verify what it published, but deploy verification is the last fence, not the main fence.
5. Any new UI action must add tests in the same change.

## Current Gaps

### 1. Contract duplication

The same API knowledge is split across:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
- [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- generated `config.json`

That means route additions can succeed in one place and silently fail in another.

### 2. Frontend tests are too UI-centric

[tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh) is useful, but it mostly checks:

- behavior inside the JS app
- expected dispatch payloads
- action presence

It does not systematically prove that every endpoint the UI calls is actually wired through the deploy surface.

### 3. Deploy starts too early

`deploy.sh update` currently does a lot of expensive work before the full contract has been validated locally.

### 4. Retry behavior hides contract bugs

[index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) `lambdaPost()` retries network-level failures.

That is good for transient outages.
It is bad for hard contract/config mistakes, because the user waits for a doomed retry loop.

## Target State

The target system has four fences:

1. Source-of-truth manifest
2. Local contract tests
3. Local flow tests
4. Deploy verification

If all four pass, first-click failures should become rare.

## Phase 1: API Manifest

### Goal

Stop hand-maintaining service names and routes in multiple places.

### Add

Create a manifest file, for example:

- [api_manifest.json](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.json)

or

- [lambda/api_manifest.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/api_manifest.py)

### Contents

For each service:

- frontend service key
- API Gateway route
- backing Lambda
- handler type
- whether it is directly called from frontend

Example shape:

```json
{
  "services": {
    "storage": {
      "base_path": "",
      "routes": [
        "/list",
        "/detail",
        "/presign",
        "/list-favorites",
        "/add-favorite",
        "/delete-favorite"
      ]
    },
    "dispatch": {
      "base_path": "/dispatch-render"
    },
    "solve_proximity": {
      "base_path": "/solve-proximity"
    }
  }
}
```

### Generate from it

Use the manifest to generate or validate:

- `config.json` contents in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- API Gateway `ensure_route` entries in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- frontend service key list used by [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### Rule

No new frontend endpoint or service may be added by hand in only one place.

## Phase 2: Hard Local Predeploy Gate

### Goal

Do not touch AWS until local contract checks pass.

### Add

Create a predeploy script, for example:

- [scripts/predeploy_check.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/scripts/predeploy_check.sh)

### It should run

```bash
uv run python -m pytest tests/test_api_route_contracts.py tests/test_deploy_packaging.py -q
bash tests/test_frontend_js.sh
```

Optionally:

```bash
uv run python -m pytest tests/test_pipeline.py -q -k favorites
```

### Integrate into deploy

[deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) should call the predeploy gate before:

- building layers
- compiling binaries
- updating Lambdas
- changing API Gateway

If the gate fails, `deploy.sh update` exits immediately.

### Benefit

This turns:

- 5-minute broken deploy

into:

- 3-second local failure

## Phase 3: Frontend Flow Contract Tests

### Goal

Test user actions as flows, not just as DOM fragments.

### New test file

Add a focused suite, for example:

- [tests/test_frontend_action_contracts.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_action_contracts.js)

run from a small shell wrapper or Node directly.

### What it should assert

For each important action:

- which frontend function is called
- which service key is used
- which path is used
- which required payload keys are present
- what success state is expected in UI state
- what error state is expected in UI state

### First batch

- Favorite add
- Favorite delete
- Favorite refresh
- Color RePalette
- GenerateFromPalette
- PDF ColorSpread
- Render Color Generate
- Render Color Generate-MT

### Why

The bug class is not only “button missing”.
It is also:

- wrong endpoint
- wrong payload
- wrong config service
- wrong state update after success

## Phase 4: Retry Policy Hardening

### Goal

Stop wasting user time on retries for non-transient failures.

### Change

In [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) `lambdaPost()`:

- retry only on:
  - network error where service is known reachable
  - `429`
  - `503`
- fail fast on:
  - `404`
  - service not configured
  - malformed response

### Improve error messages

Show:

- service name
- path
- URL
- HTTP status when available

For example:

```text
Favorite failed: storage /add-favorite HTTP 404
```

instead of:

```text
Favorite failed: Failed to fetch
```

## Phase 5: Post-Deploy Verification

### Goal

If deploy succeeds, confirm the published system matches local expectations.

### Already present

Frontend asset hash verification exists in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh).

### Add

After API Gateway setup, verify that all manifest routes exist remotely.

For example:

- fetch route list from API Gateway
- compare against manifest-required routes
- fail deploy if any are missing

### Important

This is not the primary protection.
It is the last fence after local checks.

## Phase 6: UI Action Coverage Rule

### Process rule

Any new user-visible action must ship with:

1. frontend action test
2. backend contract test if it hits a Lambda route
3. deploy route/config test if it changes API wiring

### Actions covered by rule

- new buttons
- new popup execute flows
- new download menu entries
- new tab refresh flows
- new background async jobs

If the action cannot be expressed in a test, it is not ready to merge.

## Immediate Implementation Order

### Step 1

Add API manifest and move route/service definitions into it.

### Step 2

Make [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) consume the manifest for:

- `ensure_route`
- `config.json`

### Step 3

Add `scripts/predeploy_check.sh` and call it from `deploy.sh update`.

### Step 4

Add frontend action contract tests for:

- Favorites
- PDF
- Color RePalette
- Generate-MT

### Step 5

Tighten `lambdaPost()` retry/error policy.

### Step 6

Add remote API route verification after deploy.

## Definition of Done

This plan is successful when:

1. adding a new frontend route without backend support fails locally
2. adding a backend route without deploy wiring fails locally
3. adding a service key to frontend without `config.json` support fails locally
4. `deploy.sh update` refuses to start AWS work if contract tests fail
5. first-click failures from missing route/config wiring become rare exceptions instead of routine mistakes

## Minimum Required Tests After This Plan

These must be part of the normal predeploy path:

```bash
uv run python -m pytest tests/test_api_route_contracts.py tests/test_deploy_packaging.py -q
bash tests/test_frontend_js.sh
```

For route-heavy or new-action changes, also run:

```bash
uv run python -m pytest tests/test_pipeline.py -q
```

## Recommendation

The highest-value single fix is not “more ad hoc tests”.
It is:

- one manifest
- generated deploy/config wiring
- mandatory local predeploy gate

That attacks the real failure mode directly: contract drift between frontend, handlers, and deploy.
