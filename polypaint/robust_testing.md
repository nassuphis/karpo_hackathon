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

For each frontend service:

- frontend service key
- direct API Gateway route or base URL shape
- backing Lambda
- handler family
- whether it is directly called from frontend

For storage-like services, the manifest must also list subroutes.

For dispatch-driven flows, the manifest must separately list dispatch targets.

This is important because the current app uses both:

- direct service keys, such as:
  - `storage`
  - `preview`
  - `coeffgen`
  - `sweep`
  - `sweep-mt`
  - `sweep-cm`
  - `solve_proximity`
- dispatch targets, such as:
  - `render_orchestrator`
  - `palette_orchestrator`
  - `repalette`
  - `color_repalette`
  - `pdf_artifact`
  - `autolevels`
  - `deepzoom_export`
  - `coeffgen`
  - `sweep`
  - `sweep_mt`
  - `sweep_cm`

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
    "solve_proximity": {
      "base_path": "/solve-proximity"
    },
    "dispatch": {
      "base_path": "/dispatch-render",
      "targets": [
        "render_orchestrator",
        "palette_orchestrator",
        "repalette",
        "color_repalette",
        "pdf_artifact",
        "autolevels",
        "deepzoom_export",
        "coeffgen",
        "sweep",
        "sweep_mt",
        "sweep_cm"
      ]
    }
  }
}
```

### Generate from it

Use the manifest to generate or validate:

- `config.json` contents in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- API Gateway `ensure_route` entries in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- frontend service key usage in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- dispatch target usage in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- dispatch target availability in [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)
- solver endpoint mapping in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html), especially `sweep`, `sweep-mt`, and `sweep-cm`

### Migration rule

Do not switch deploy generation to the manifest first.

The safe order is:

1. scrape current `deploy.sh` / frontend / dispatch state into the manifest
2. validate the current tree against that manifest
3. only then make `deploy.sh` generate from it

Phase 1 is validation-only.

It must not change runtime behavior.
It must not make `deploy.sh` consume the manifest yet.
Its only job is to describe current reality and fail locally when the tree drifts away from it.

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

### Test location

Do not create a second independent frontend harness if it can be avoided.

The first implementation should extend:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Only split into a second JS contract file if the existing shell harness becomes genuinely unmanageable.

The first extension of the current contract tests must cover:

- dispatch targets used from `lambdaPost('dispatch', { target: ... })`
- dynamic solver direct endpoints chosen by `_solverLoresEndpoint(...)`
- dynamic solver dispatch targets chosen by `_solverDispatchTarget(...)`

### What it should assert

For each important action:

- which frontend function is called
- which service key is used
- which path is used
- which dispatch target is used, if any
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

Add an additive API manifest generated from the current tree.

### Step 2

Validate the current tree against the manifest:

- frontend service keys
- frontend storage subroutes
- frontend dispatch targets
- frontend dynamic solver service mapping
- frontend dynamic solver dispatch mapping
- deploy `config.json`
- deploy `ensure_route`
- dispatch target map in [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)

### Step 3

Add `scripts/predeploy_check.sh` and call it from `deploy.sh update`.

### Step 4

Only after validation is stable, make [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) consume the manifest for:

- `ensure_route`
- `config.json`

### Step 5

Extend frontend action contract tests for:

- Favorites
- PDF
- Color RePalette
- Generate-MT

### Step 6

Tighten `lambdaPost()` retry/error policy.

## Definition of Done

This plan is successful when:

1. adding a new frontend route without backend support fails locally
2. adding a backend route without deploy wiring fails locally
3. adding a service key to frontend without `config.json` support fails locally
4. adding a dispatch target to frontend without `handler_dispatch.py` support fails locally
5. adding a solver endpoint/dispatch mapping in frontend without matching deploy/dispatch support fails locally
6. `deploy.sh update` refuses to start AWS work if contract tests fail
7. first-click failures from missing route/config wiring become rare exceptions instead of routine mistakes

## Minimum Required Tests After This Plan

These must be part of the normal predeploy path:

```bash
uv run python -m pytest tests/test_api_route_contracts.py tests/test_deploy_packaging.py -q
bash tests/test_frontend_js.sh
```

And for Phase 1 specifically, the contract suite is not done until it covers:

- direct frontend services
- storage subroutes
- dispatch targets
- solver endpoint mapping
- solver dispatch mapping

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

That attacks the real failure mode directly: contract drift between frontend, handlers, dispatch, and deploy.
