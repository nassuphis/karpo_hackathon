# Testing Refactor Plan

This document is about fixing the current testing sprawl without creating a second layer of stale bureaucracy.

The repo has a lot of tests. The problem is not lack of tests. The problem is lack of structure:

- there is no single authoritative answer to "what tests exist?"
- there is no single authoritative answer to "what runs on deploy?"
- there is no single authoritative answer to "what must run for this change?"
- automatic, manual, docker, browser, and live-smoke checks are mixed together in conversation and docs
- some tests live in `tests/`, some in `lambda/`, and the split is not clearly explained
- deploy runs a subset, but that subset is not presented as one named group

The goal of this refactor is not "more tests". The goal is:

- one source of truth
- one standard runner
- one explicit deploy gate
- one explicit distinction between local, docker/runtime, browser, and live verification

## Core Design

The right design is a hybrid:

- Python tests should use `pytest` marks for grouping.
- Non-pytest tests should live in a small machine-readable catalog.

This avoids duplicating metadata that `pytest` already knows how to express.

That means:

- do **not** put every Python test into a giant JSON catalog if `pytest.mark` can already describe it
- **do** catalog the things `pytest` does not naturally know about:
  - `tests/test_frontend_js.sh`
  - Playwright specs
  - docker wrapper scripts
  - live smoke scripts
  - any other non-pytest test entrypoints

So the source of truth is:

1. `pytest` marks for Python tests
2. a small catalog for non-pytest tests

Not:

1. a giant hand-maintained JSON file describing everything twice

## Current Problems

The current testing system is split across:

- [docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md)
- [deployment-checklist.md](/Users/nicknassuphis/karpo_hackathon/polypaint/deployment-checklist.md)
- [scripts/predeploy_check.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/scripts/predeploy_check.sh)
- [scripts/test-docker-runtime.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/scripts/test-docker-runtime.sh)
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- targeted commands run ad hoc during implementation

There is also a location split that needs to be acknowledged up front:

- main test corpus in [tests](/Users/nicknassuphis/karpo_hackathon/polypaint/tests)
- older or specialized test files in [lambda](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda)

That creates several failure modes:

- a test exists, but nobody knows when it is required
- a test runs during deploy, but the docs do not make that obvious
- a feature is described as "tested" even though only the local subset ran
- a hand-maintained inventory would rot unless it is enforced
- per-file inventory is too coarse for grab-bag files like [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)

## Principles

1. Do not duplicate metadata that a tool already supports.
2. Any machine-readable inventory must be enforced or it will rot.
3. Phase 1 must be small enough to land.
4. File moves are not Phase 1.
5. Change-type mapping helps, but does not remove human judgment for cross-cutting changes.

That last point matters. The system can reduce the work of deciding what to run, but it cannot fully automate semantic reasoning.

Example:

- touching `handler_storage.py` might look like a Python handler change
- but if the change adds new DynamoDB actions, IAM tests must also run

So the correct standard is:

- the system narrows the required set substantially
- the developer still has to think about cross-cutting effects

## Deliverables

## 1. Pytest Marks for Python Tests

Add marks to Python tests so they self-describe their groups.

Examples:

- `@pytest.mark.predeploy`
- `@pytest.mark.fast`
- `@pytest.mark.docker`
- `@pytest.mark.workflow`
- `@pytest.mark.poly_parity`
- `@pytest.mark.slow`

This gives us:

- named groups
- native pytest filtering
- metadata living next to the tests

This should be the primary grouping mechanism for Python tests.

## 2. Minimal Catalog for Non-Pytest Tests

Create:

- [tests/catalog.json](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/catalog.json)

But keep it small.

Phase 1 catalog scope:

- `tests/test_frontend_js.sh`
- `tests/e2e/*.spec.js`
- `scripts/test-docker-runtime.sh`
- future live smoke scripts
- any other test entrypoint that is not a pytest test file

Use the minimum viable schema:

- `id`
- `path`
- `kind`
- `auto_groups`
- `change_types`

Example:

```json
{
  "id": "frontend_vm",
  "path": "tests/test_frontend_js.sh",
  "kind": "frontend_vm",
  "auto_groups": ["predeploy", "full"],
  "change_types": ["frontend_ui", "frontend_contract"]
}
```

Do not add rich fields unless the runner or docs actually use them.

## 3. Catalog/Filesystem Parity Meta-Test

This is mandatory in Phase 1.

If the catalog is not enforced, it will rot.

Add a meta-test that checks both directions:

- every catalog entry points to a file that exists
- every non-pytest test file that is meant to be catalogued is either:
  - in `tests/catalog.json`, or
  - explicitly listed in an exclusion file

Suggested companions:

- [tests/test_test_catalog.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_test_catalog.py)
- [tests/catalog_excluded.json](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/catalog_excluded.json)

This meta-test should be in the deploy gate.

Phase 1 scope rule:

- this parity meta-test scans `tests/` for catalogued non-pytest entrypoints
- it does **not** scan `lambda/`

Reason:

- `lambda/` currently contains Python test files and legacy/specialized test code
- those are part of the broader test-surface problem, but not Phase 1 catalog scope
- Phase 1 should not pretend that `lambda/` is already normalized

Without this, the catalog is just a second stale doc.

## 4. One Standard Runner

Create:

- [scripts/all-tests.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/scripts/all-tests.sh)

Phase 1 modes should stay small:

- `predeploy`
- `docker`
- `full`

Recommended behavior:

- `predeploy`
  - `pytest -m predeploy`
  - plus catalogued non-pytest tests in the `predeploy` group
- `docker`
  - docker runtime checks only
- `full`
  - broad local suite, excluding live smoke unless explicitly requested

Important:

- for Python tests, the runner should use `pytest -m ...`
- for non-pytest tests, the runner should read `tests/catalog.json`
- any non-zero from any sub-step must make the runner exit non-zero

Do not make the runner a second hardcoded inventory unrelated to either source of truth.

## 5. One Explicit Deploy Gate Group

Define one named group that means:

- "this is what runs before deploy is considered structurally sane"

Suggested name:

- `deploy_gate`

This group should include:

- predeploy contract checks
- deploy packaging checks
- frontend VM test
- docker runtime test
- catalog parity meta-test

And it should explicitly **not** try to be "all important tests".

In particular, the initial `deploy_gate` group does not need to include broad handler unit suites.
Those belong in larger local/full groups unless there is a specific reason to promote one into the deploy gate.

Then:

- `scripts/predeploy_check.sh` should become a thin wrapper around that group
- `deploy.sh` should call the runner/group, not hand-list test commands forever

## 6. Clarify Automatic vs Manual vs Live

This is one of the highest-value outcomes.

Every test or group must explicitly state whether it is:

- automatic on deploy
- manual local
- manual docker/runtime
- manual browser
- manual live AWS

This needs to be objective.

Right now the confusion usually comes from people saying "tested" when they mean only one of those categories.

## 7. Split or Acknowledge Coarse Test Files

The catalog and marks are less useful if one file covers too many unrelated surfaces.

The immediate offender is:

- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)

It is too coarse to cleanly describe as one thing.

Phase 1 recommendation:

- explicitly acknowledge this file as a coarse legacy bucket
- add a visible pytest mark such as `legacy_grab_bag` so the coarse granularity is declared rather than hidden

Phase 2 recommendation:

- split it into per-handler or per-surface files where practical

This is not required before any progress can be made, but it should be stated explicitly so the system does not pretend the granularity is better than it is.

Also document the dual-location reality:

- [tests](/Users/nicknassuphis/karpo_hackathon/polypaint/tests)
- [lambda](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda)

and decide whether:

- the `lambda/` tests are temporary legacy tests
- or part of the supported test surface

That ambiguity should not survive the refactor.

## 8. Live Smoke Needs Real Files

"Live smoke" should stop being aspirational prose.

Give it a concrete home and naming convention.

Suggested location:

- [tests/smoke](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/smoke)

Suggested naming:

- `tests/smoke/compute.sh`
- `tests/smoke/render-color.sh`
- `tests/smoke/render-bilevel.sh`
- `tests/smoke/palette.sh`
- `tests/smoke/compute-preview.sh`

These can start as minimal stubs if needed, but they should exist as named entrypoints.

Without that, "live-verified" remains too fuzzy.

## 9. Rework docs/testing.md

[docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md) should be rewritten around:

- pytest marks
- non-pytest catalog entries
- named groups
- runner commands
- automatic vs manual vs live distinctions

It should stop trying to be:

- the full test inventory
- the full runner spec
- the change-type map
- the readiness checklist

That is too many jobs for one file.

## 10. Keep deployment-checklist.md Focused

[deployment-checklist.md](/Users/nicknassuphis/karpo_hackathon/polypaint/deployment-checklist.md) should stay a readiness checklist.

It should reference:

- named groups
- the standard runner
- live smoke rules

But it should not become the canonical list of test files.

## 11. Simplify the Future Directory Structure

Do not over-design the final directory tree.

If test directories are reorganized later, keep it modest.

A likely sensible target is something closer to:

- `tests/handlers/`
- `tests/frontend/`
- `tests/runtime/`
- `tests/e2e/`

Leave the rest top-level until a stronger structure is actually needed.

This is Phase 2 or Phase 3 work, not Phase 1.

## 12. Clarify Poly/Parity Coverage

The polynomial tests need a short explicit map in the docs.

They are currently a mix of:

- targeted hand parity
- low-agreement repair coverage
- catalog consistency
- parity result generation
- visual overlap tests
- individual behavior/smoke tests

This should be explained in terms of purpose, not just file names.

But this is documentation and grouping work, not the first thing to automate.

## Suggested Phases

## Phase 1A: Minimum Viable Control

This phase should be small and enforceable.

Deliver:

1. Add pytest marks to Python tests.
2. Create a minimal `tests/catalog.json` for non-pytest tests only.
3. Add the catalog/filesystem parity meta-test.
4. Create `scripts/all-tests.sh` with:
   - `predeploy`
   - `docker`
   - `full`
5. Define one named deploy gate group.
6. Document the `tests/` vs `lambda/` split explicitly.

Definition of done:

- there is one answer for Python test grouping
- there is one answer for non-pytest test inventory
- the catalog cannot silently rot
- there is one standard runner
- there is one named deploy gate

## Phase 1B: Deploy Gate Wiring

Once Phase 1A is stable:

1. Repoint [scripts/predeploy_check.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/scripts/predeploy_check.sh) to the standard runner/group.
2. Repoint [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) to the standard runner/group where practical.

Definition of done:

- deploy is using the new named gate rather than a hand-maintained command list
- rollback of the wiring is easy if the runner integration is wrong

## Phase 2: Documentation Cleanup

Deliver:

- rewrite `docs/testing.md`
- reference the runner and groups from `deployment-checklist.md`
- add a short map for poly/parity coverage
- document the automatic/manual/docker/live distinctions cleanly

Definition of done:

- a contributor can tell what to run without reading source

## Phase 3: Structural Cleanup

Deliver:

- split coarse files like `tests/test_pipeline.py` where worthwhile
- decide what to do with `lambda/` tests
- optionally reorganize test directories modestly

Definition of done:

- test granularity is less misleading
- test locations are easier to scan

## Phase 4: Smarter Automation

Deliver:

- optional changed-path-based test suggestions
- optional generated docs from the marks/catalog
- optional CI/reporting improvements

Definition of done:

- "what should have run?" can be answered mechanically most of the time

## Non-Goals

This refactor is not about:

- forcing every test into the deploy gate
- pretending change-type mapping removes human judgment
- deleting existing targeted tests
- over-designing the filesystem before the grouping system exists

## Acceptance Criteria

The refactor is successful when:

1. Python tests are grouped by pytest marks rather than by undocumented convention.
2. Non-pytest tests are catalogued in one small enforced inventory.
3. There is a meta-test preventing catalog drift.
4. There is one standard runner with named modes.
5. There is one explicit deploy gate group.
6. The answer to "what runs automatically?" is objective.
7. The answer to "was this locally tested, docker-tested, or live-tested?" is objective.

## Immediate Next Steps

1. Decide and document the initial pytest marks.
2. Create `tests/catalog.json` for non-pytest tests only.
3. Create `tests/catalog_excluded.json`.
4. Add `tests/test_test_catalog.py`.
5. Add the initial coarse-file mark for [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py).
6. Add `scripts/all-tests.sh`.
7. Define the initial `deploy_gate` group.
8. Add a short note in `docs/testing.md` about the `tests/` and `lambda/` split.
