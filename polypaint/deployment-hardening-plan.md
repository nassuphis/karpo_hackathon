# Deployment Hardening Plan

This document is about making `deploy.sh update` failures rarer, earlier, and easier to diagnose.

It is not another generic checklist. It is a concrete plan based on the failure modes we have already hit in this repo.

## Goal

Before calling a feature "ready", we want a layered deploy-safety system that catches:

- missing route/invoke wiring
- stale or incomplete workflow template substitution
- create/update deploy-path drift
- missing packaged files or binaries
- runtime dependency mismatches inside Lambda
- IAM mismatches
- state-size and `/tmp` budget blowups
- "works in mocks but not after deploy" failures

The standard is:

- local tests catch structural mistakes
- docker/runtime tests catch packaging/runtime mistakes
- one postdeploy smoke catches live AWS integration mistakes

## Failure Modes Already Seen

The plan is driven by real failures, not hypothetical ones.

### 1. Workflow template placeholder leaked into production

Observed failure:

- render associated-palette branch tried to invoke literal `${PaletteChunkFunctionArn}`

Root cause:

- `deploy.sh` rendered the render workflow in multiple places
- the palette substitutions were present in one path but missing in another

What this means:

- testing only one deploy branch is not enough
- workflow template substitution must be validated as an emitted artifact, not inferred from source text

### 2. API Gateway route existed but invoke permission was missing

Observed failure:

- `compute-preview` returned generic HTTP 500 before the handler ran

Root cause:

- integration existed
- Lambda package existed
- API Gateway `lambda:add-permission` grant was missing

What this means:

- route/integration creation is not enough
- deploy tests must check the permission loop too

### 3. Runtime helper/tool existed in tests but not in deployed layer

Observed failure:

- resize path tried to use `/opt/bin/vipsheader`

Root cause:

- mocks and packaging were fine
- actual layer contents did not include that helper

What this means:

- runtime smoke must test the actual tools the feature uses
- packaging alone is not runtime verification

### 4. IAM design was changed but role was not

Observed failure:

- favorites moved to DynamoDB reads/deletes and immediately hit permission failure

Root cause:

- handler logic changed
- role actions were not updated in lockstep

What this means:

- handler/API usage and deploy IAM policy must be checked together

### 5. Step Functions state budget was exceeded after coeffgen

Observed failure:

- `States.DataLimitExceeded` in `PostCoeffgen`

Root cause:

- too much per-chunk state was carried through the workflow

What this means:

- resource-budget tests need to include Step Functions payload size, not just Lambda memory

### 6. Browser-facing feature existed in code but not in user-visible flow

Observed failures:

- missing buttons
- hidden artifacts not visible/downloadable
- stale config/client issues

What this means:

- browser tests must pin important controls and click paths
- VM tests alone are not enough

## Hardening Principles

1. Test the emitted artifacts, not just the source ingredients.
2. Remove duplicated deploy logic; parity tests are at most a short-lived migration aid, not part of the intended steady state.
3. Tests must call the same rendering/helper path that `deploy.sh` uses, not a parallel reimplementation.
4. Treat packaging and runtime as separate concerns.
5. Non-recoverable deploy failures must fail hard; silent masking is itself a bug surface.
6. A feature is not "ready" until at least one real deployed smoke has passed.
7. If a known failure mode happened once, a regression test should be added specifically for it.

## Proposed Additions

## A. Shared Deploy Rendering Helper + Artifact Rendering Test

Extract workflow/config rendering into one shared helper used by both `deploy.sh` and tests.

Then add a test that renders the actual deploy artifacts through that helper and inspects the emitted files.

Artifacts to render:

- generated `config.json`
- rendered `render_workflow.asl.json`
- rendered `compute_workflow.asl.json`
- rendered `palette_workflow.asl.json`

Assertions:

- no `${...}` placeholders remain
- JSON parses
- expected Lambda ARNs are present
- expected routes/service keys are present
- critical workflow states still point at real ARNs, not placeholders

Why this matters:

- this would have caught the `${PaletteChunkFunctionArn}` leak directly

Recommended implementation:

- one shell or Python helper used by both deploy and tests
- do not re-implement substitution in the test
- render through the same code path the deploy uses

This is the most important Phase 1 item because it both:

- catches emitted-artifact bugs
- and creates the seam needed to remove create/update drift

## B. Delete Create/Update Rendering Duplication

`deploy.sh` currently has separate create/update renderings for some generated files and state machines.

The correct fix is:

- reduce duplication in `deploy.sh`
- move repeated rendering into one shell helper or one Python helper
- have both create and update paths call that helper

Why this matters:

- parity tests only detect the smell
- one rendering helper removes the smell and the bug source

Short-term note:

- if duplication still exists during the transition, keep a parity-style test only as a temporary guard
- once the shared helper lands, that parity test should be deleted

This should be treated as Phase 1 work, not deferred cleanup.

## C. Silent-Failure Audit in `deploy.sh`

Audit every `|| true`, `2>/dev/null`, and similar suppression in `deploy.sh`.

Split them into two categories:

- genuinely optional/idempotent operations that can fail softly
- non-recoverable operations that must fail the deploy

The goal is not "less suppression." The goal is specific idempotence.

Correct pattern:

- check first, or
- catch only the specific expected error such as:
  - `ResourceConflictException`
  - `ResourceAlreadyExistsException`
  - `ResourceInUseException`
  - existing permission statement / already-attached policy cases

Incorrect pattern:

- blanket `2>/dev/null || true`
- swallowing any failure just because one expected failure mode exists

Priority targets:

- `lambda add-permission`
- IAM policy attachment
- route/integration creation follow-ups
- table creation / wait paths
- state machine updates

Why this matters:

- the deploy script currently hides some failures that should stop the release immediately
- tests outside the script cannot fully compensate for the script swallowing the error

Required outcome:

- a failed invoke-permission or role update must crash `deploy.sh`
- suppression remains only where the failure is truly expected and harmless
- re-runnability comes from specific "already exists" handling, not from pretending every failure is fine

## D. Packaged Lambda Import Smoke

After packaging a Lambda zip, test that the packaged Python code can actually import the handler module.

For each packaged handler:

- unpack to a temp dir
- set `PYTHONPATH` to the unpacked dir
- import the handler module
- assert import does not fail on missing local modules

This catches:

- missing sidecar Python files
- missing generated helper modules
- packaging drift not visible to `py_compile`

This is lighter than a full runtime test and should run fast.

## E. Packaged Runtime Smoke in Docker

Extend the docker/runtime coverage to test packaged Lambda behavior, not only source-tree binaries.

For runtime-sensitive features, verify:

- packaged binary exists where the handler expects it
- packaged helper exists
- shared libraries resolve inside the Lambda-like environment
- one minimal happy-path invocation works

Add an explicit shared-library check:

- run `ldd` on the packaged binary inside the Lambda-like container
- assert there are no `not found` lines

Priority targets:

- compute-preview package
- resize-artifact package
- solve-proximity package
- palette-finalize package
- any new package using LAPACK/libvips/helper binaries

This catches:

- "binary exists locally but not in zip"
- "zip is fine but runtime layer is wrong"
- "tool name assumed by handler is not actually shipped"
- "binary exists but a required `.so` is missing at runtime"

## F. IAM/Permission Contract Tests

Expand permission tests beyond API Gateway invoke rights.

Add tests that check:

- handlers using new DynamoDB actions have matching IAM policy actions in `deploy.sh`
- handlers using new S3 operations have matching IAM coverage
- Step Functions/Lambda invoke relationships are represented where needed

Preferred design:

- each handler declares:
  - `REQUIRED_IAM_ACTIONS = [...]`
- deploy tests assert the role policy in `deploy.sh` is a superset of the union of those declarations

This is better than grep heuristics because the source of truth lives with the handler that needs the action.

Important limitation:

- some cross-service resource wiring still needs deploy-specific checks
- this does not replace API Gateway invoke-permission or workflow-integrator tests

Priority targets:

- favorites/storage paths
- any new artifact handlers
- workflow starter Lambdas

## G. Workflow Field-Completeness Tests

We already added some field-forwarding tests. Extend that approach systematically.

For each workflow:

- plan fields used downstream must be explicitly forwarded
- dangerous default-masking fields should be pinned
- omission of fields should fail tests before deploy

Priority fields:

- solve-score settings
- root transforms
- times
- retries
- worker counts
- source palette / associated palette metadata
- output keys

Goal:

- stop "field missing, handler default masked it" bugs earlier

## H. Resource-Budget Tests

Add local budget tests for:

- Step Functions state size
- estimated Lambda `/tmp` usage
- large sync payload size
- large bundle size where relevant

Examples:

- `PostCoeffgen`-style payload size checks
- solve temp-file estimates
- compute-preview response size guardrails

These should be explicit and numeric, not just "seems fine."

## I. Browser Coverage Policy

The current UI browser coverage is much better, but the policy should be explicit:

- if a new user-facing control is added, at least one Playwright test must click it
- if a popup is added, Playwright must assert it opens
- if a control dispatches work, at least one test must inspect the request shape

This is already mostly true now for the major tabs, but the policy should be enforced socially and documented in [docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md).

## J. Rollback Recorder and Recovery Notes

Add a trap-based deploy recorder in `deploy.sh` that records:

- which deploy step failed
- which artifact or AWS operation was in progress
- which generated files were being updated

Also document a minimal rollback story for each risky deploy area:

- API Gateway route/integration changes
- Lambda package updates
- IAM policy changes
- Step Functions definition changes

Why this matters:

- `set -euo pipefail` is not enough if the operator cannot tell what partially applied
- faster failure diagnosis lowers recovery time when deploy does go wrong

## K. Postdeploy Smoke Script

Add a small documented postdeploy smoke suite, ideally scriptable.

Define a hard minimum floor for "deploy verified":

- compute
- render color
- render bilevel
- palette artifact
- compute preview

Extended flows to include when touched:

- resize artifact
- associated-palette render path
- deep zoom export

This does not have to be huge. The point is one real run per top-level system.

Required outputs:

- success/failure
- job id / run id
- where the logs/status are

The important part is discipline:

- feature is not called fully verified until this passes on deployed infra

## L. Deployed Manifest/Route Payload Smoke

Add a small postdeploy script that:

- reads manifest/example payloads
- POSTs them to the deployed routes
- asserts the response is structured JSON and not:
  - API Gateway 502
  - Lambda permission failure
  - generic Lambda error envelope

This closes the loop between:

- static manifest contract
- deployed route wiring
- deployed handler importability
- API Gateway invoke permission

## Priority Order

### Phase 1: Immediate hardening

1. Collapse duplicated workflow/config rendering into one shared helper
2. Deploy artifact rendering test that calls that helper
3. Replace blanket deploy suppression with specific idempotence handling for expected "already exists" cases
4. Add a trap-based rollback recorder to `deploy.sh`
5. Continue adding specific regressions for every deploy bug actually seen

This phase gives the best return quickly.

### Phase 2: Packaging/runtime hardening

1. Packaged Lambda import smoke
2. Packaged runtime smoke in docker for runtime-sensitive features
3. Explicit shared-library resolution checks
4. Expand docker coverage for new binary/layer combinations

### Phase 3: Integration hardening

1. IAM/permission contract tests with handler-declared required actions
2. Broader workflow field-forwarding tests
3. Resource-budget tests

### Phase 4: Production confidence

1. Postdeploy smoke script/checklist with hard minimum flows
2. Manifest/route payload smoke against deployed APIs
3. Consistent reporting of:
   - locally verified
   - docker verified
   - live verified

## Concrete Next Steps

### Step 1

Extract shared workflow/config rendering into one helper and have both `deploy.sh` and tests call it.

### Step 2

Implement a deploy test that renders all generated workflow/config artifacts through that helper and asserts:

- no placeholders remain
- JSON parses
- required routes/services/ARNs are present

### Step 3

Audit `deploy.sh` for masked failures and replace blanket suppression with specific idempotence handlers for expected "already exists" cases.

### Step 4

Add packaged import smoke for Lambda zips.

### Step 5

Add one packaged docker/runtime smoke for a known runtime-sensitive handler path, including shared-library resolution.

### Step 6

Define and document a minimal postdeploy smoke for the top-level user flows and a small manifest-driven deployed-route payload smoke.

## Definition of Done

Deployment hardening is materially improved when:

- generated deploy artifacts are tested directly through the same helper deploy uses
- create/update deploy drift is removed by refactor rather than merely detected
- masked non-recoverable deploy failures are eliminated
- re-runnable deploy behavior comes from specific idempotence handlers, not blanket suppression
- runtime-sensitive handlers have packaged runtime smoke
- shared-library resolution is tested for packaged native binaries
- permission regressions are harder to introduce silently
- at least one hard-minimum postdeploy smoke is expected for new user-visible features

Until then, `deploy.sh update` will continue to be more brittle than it should be.
