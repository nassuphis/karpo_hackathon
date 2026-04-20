# Solve Score Modal Plan

Status: proposed feature plan for replacing local solve-score preset/import/export controls with an S3-backed modal workflow.

## Goal

Replace the current solve-score preset dropdown and JSON import/export strip in the Render and Palette tabs with one shared `Solve Scores...` popup backed by S3.

This should let the operator:

- browse saved solve-score programs
- inspect the current live program
- load a saved program into the active tab
- save the current live program to S3
- delete a saved program
- download the current live program as JSON
- upload a local JSON program into the active tab

Scope:

- Render tab: in scope
- Palette tab: in scope
- Compute tab: explicitly out of scope for this feature

## UI Requirements

The popup should follow the current-shipped UI rules in [ui_docs/style_guide.md](/Users/nicknassuphis/karpo_hackathon/polypaint/ui_docs/style_guide.md):

- use the existing popup system
  - `.tri-popup-overlay`
  - `.tri-popup`
  - `_bindPopupShell(...)`
- use a dense but explicit table-and-detail layout
- do not use fake disabled inputs
- keep operator-facing controls explicit

## UI Changes

### Replace Existing Controls

In both Render and Palette:

- remove the built-in preset dropdown
- remove `Load preset`
- remove `Load JSON`
- remove `Save JSON`
- replace all of the above with a single `Solve Scores...` button

### Shared Popup

One shared popup should be used for both tabs.

Opening context:

- opening from Render acts on the Render solve-score chain
- opening from Palette acts on the Palette solve-score chain

### Popup Layout

Left pane:

- scrollable table of saved solve-score programs

Table columns:

- `Name`
- `Statements`
- `Saved`

Right pane:

- `Program name`
  - editable text input
  - defaults from the current live program
  - updates when the user selects a saved row only if the input is currently empty or unchanged since the last row selection
  - if the operator has typed a custom unsaved name, row selection must not clobber it
- `Current program`
  - read-only display block
  - shows the active tab's current live solve-score program
  - not editable in this popup
- `Selected saved program`
  - read-only display block
  - shows the currently selected saved program from the table
  - empty until a row is selected
  - lets the operator compare before pressing `Load`

Bottom action row:

- `Load`
- `Save`
- `Delete`
- `Download`
- `Upload`

## Popup Behavior

### Selection Model

- clicking a table row selects a saved program
- the selected row enables `Load` and `Delete`
- the selected row updates the right-side name input

### Action Semantics

`Load`

- fetch the selected saved program from S3
- validate it client-side
- if validation fails, do not mutate the live solve-score chain
- load it into the active tab's live solve-score chain

`Save`

- serialize the active tab's current live solve-score program
- send the live program payload from the webapp to the backend using the current right-side name input
- this is save-as semantics for the current live chain
- global namespace rules apply; see Backend section

`Delete`

- delete the selected saved program from S3

`Download`

- download the active tab's current live solve-score program as JSON
- output JSON shape must round-trip cleanly through `Upload`
- output shape should be the current shipped portable program shape:

```json
{
  "version": 1,
  "name": "Proximity q=0.1%",
  "chain": [["proximity", "0.1"]],
  "metric": "proximity",
  "display": "proximity(slv,q=0.1%)",
  "program_spec": "m0"
}
```

`Upload`

- import a local JSON file
- validate it
- apply it to the active tab's current live solve-score chain
- accepted input shape must tolerate both:
  - current shipped portable program JSON from `Download`
  - server-written saved objects fetched from S3, including derived fields like `id`, `statement_count`, and `saved_at`
- derived fields present in uploaded JSON must be ignored during apply

### Table States

The left table must have explicit states:

- loading
- empty
- error
- loaded

Required behavior:

- opening the popup starts with a loading state until S3 results arrive
- if there are no saved programs, show an explicit empty-state row or message
- if list fetch fails, show an explicit error state and keep `Load` and `Delete` disabled
- selection state must survive table refresh when the selected `id` is still present

## Backend

Add this feature to the existing storage CRUD handler in [handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py).

This matches current repo convention:

- favorites are storage routes
- palettes are storage routes
- render/deepzoom cleanup is storage routes

Do not add a separate Lambda for this feature unless there is a later, explicit reason to split it.

### Routes

- `POST /list-solve-score-programs`
- `POST /fetch-solve-score-program`
- `POST /save-solve-score-program`
- `POST /delete-solve-score-program`

Frontend calls should continue using:

- `lambdaPost('storage', body, '/<path>')`

### Route Schemas

`POST /list-solve-score-programs`

Request body:

```json
{}
```

Success response:

```json
{
  "programs": [
    {
      "id": "proximity-q01",
      "name": "Proximity q=0.1%",
      "statement_count": 1,
      "saved_at": "2026-04-20T12:34:56Z"
    }
  ],
  "count": 1,
  "order": "saved_at_desc"
}
```

`POST /fetch-solve-score-program`

Request body:

```json
{
  "id": "proximity-q01"
}
```

Success response:

```json
{
  "program": {
    "version": 1,
    "id": "proximity-q01",
    "name": "Proximity q=0.1%",
    "chain": [["proximity", "0.1"]],
    "metric": "proximity",
    "display": "proximity(slv,q=0.1%)",
    "program_spec": "m0",
    "statement_count": 1,
    "saved_at": "2026-04-20T12:34:56Z"
  }
}
```

`POST /save-solve-score-program`

Request body:

```json
{
  "name": "Proximity q=0.1%",
  "chain": [["proximity", "0.1"]]
}
```

Success response:

```json
{
  "program": {
    "version": 1,
    "id": "proximity-q01",
    "name": "Proximity q=0.1%",
    "chain": [["proximity", "0.1"]],
    "metric": "proximity",
    "display": "proximity(slv,q=0.1%)",
    "program_spec": "m0",
    "statement_count": 1,
    "saved_at": "2026-04-20T12:34:56Z"
  },
  "overwritten": false
}
```

`POST /delete-solve-score-program`

Request body:

```json
{
  "id": "proximity-q01"
}
```

Success response:

```json
{
  "id": "proximity-q01",
  "deleted": 1
}
```

Error responses:

- invalid request or validation failure returns a `400`-class error payload with a concise `error` string
- missing fetch/delete target returns a not-found error payload with a concise `error` string

### S3 Prefix

Store programs under:

- `polypaint/solve-score-programs/<slug>.json`

This feature is globally scoped.

- there is no per-user namespace
- there is no per-job namespace
- there is no per-tab namespace

### ID / Slug Rules

The server derives `id` from `name` using a stable slug rule:

- lowercase ASCII
- replace runs of non-`[a-z0-9]` with `-`
- trim leading and trailing `-`
- if the result is empty, use `solve-score-program`
- truncate to 64 characters

Collision rule:

- names that normalize to the same slug intentionally map to the same saved program key
- punctuation and capitalization differences do not create separate records

### Saved Object Shape

Client save request body:

```json
{
  "name": "Proximity q=0.1%",
  "chain": [["proximity", "0.1"]]
}
```

The chain sent in `/save-solve-score-program` comes from the active webapp state.

It is not an S3 reference and it is not looked up from an existing saved program.

Server-written saved object:

```json
{
  "version": 1,
  "id": "proximity-q01",
  "name": "Proximity q=0.1%",
  "chain": [["proximity", "0.1"]],
  "metric": "proximity",
  "display": "proximity(slv,q=0.1%)",
  "program_spec": "m0",
  "statement_count": 1,
  "saved_at": "2026-04-20T12:34:56Z"
}
```

Future schema note:

- fetch and list paths must continue to tolerate `version: 1`
- if a future `version: 2` is introduced, readers must remain backward-compatible with `version: 1` objects

Field ownership:

- client supplies:
  - `name`
  - `chain`
- server generates:
  - `version`
  - `id`
  - `metric`
  - `display`
  - `program_spec`
  - `statement_count`
  - `saved_at`

### Server-Side Rules

- validate `chain` on save using the existing solve-score compiler
- reject invalid or empty chains
- reject empty or unreasonable `name`
- normalize or generate `id` from `name`
- compute `statement_count` as the number of statements in the normalized chain
- compute and store `saved_at`
- `list` should return summary rows only
- `fetch` should return the full saved JSON
- `delete` should delete by `id`
- ignore any client-supplied `id`, `metric`, `display`, `program_spec`, `statement_count`, `saved_at`, or `version`

### Save Payload Validation

Because `/save-solve-score-program` accepts the live chain directly from the webapp, the handler must validate aggressively before writing anything to S3.

Required checks:

- `name` must be non-empty
- `name` max length: 120 characters
- `name` must be bounded in length
- `chain` must be present and non-empty
- `chain` must be a JSON array of statements in the same shape the current webapp emits
- `chain` max statement count: 256
- serialized `chain` max JSON size: 16 KiB
- each string token inside the chain max length: 128 characters
- `chain` must compile through the authoritative solve-score compiler
- malformed or extra client fields must not be trusted or copied through blindly

The compiler should remain the authoritative validator for chain semantics.

### Overwrite and Concurrency

Default save behavior may use last-writer-wins unless explicit overwrite protection is added later.

Required behavior:

- save checks whether the target key already exists
- if it does not exist, save succeeds
- if it does exist, save overwrites the existing object unless a later optimistic-concurrency scheme is added

Initial implementation note:

- for this feature, last-writer-wins is acceptable
- if optimistic concurrency is added later, it should be based on previously fetched server metadata rather than trusting the client blindly

## UI State Rules

### Button Semantics

`Load`

- source of truth: selected saved row only
- never loads from the current live chain display

`Save`

- source of truth: current right-side `Program name` input plus current live chain
- never saves the selected saved program body unless that same body is also the current live chain

`Delete`

- source of truth: selected saved row only

`Download`

- source of truth: current live chain only
- does not download the selected saved program unless that same program has already been loaded into the live chain

`Upload`

- source of truth: local JSON file
- applies to the live chain only
- does not persist to S3 until `Save` is pressed

### Enable / Disable Rules

`Load`

- enabled only when:
  - table state is `loaded`
  - a saved row is selected
  - no popup action is currently in flight

`Save`

- enabled only when:
  - current name input is non-empty after trim
  - current live chain compiles successfully
  - no popup action is currently in flight

`Delete`

- enabled only when:
  - table state is `loaded`
  - a saved row is selected
  - no popup action is currently in flight

`Download`

- enabled only when:
  - current live chain compiles successfully
  - no popup action is currently in flight

`Upload`

- enabled whenever no popup action is currently in flight

### Delete Behavior

- deleting requires an explicit confirmation prompt
- after delete succeeds:
  - remove the row from the table
  - clear selected-row state
  - clear the `Selected saved program` pane
  - keep the current live chain unchanged
  - keep the current name input unchanged unless it was explicitly mirroring the deleted row and product behavior later decides otherwise

### Display Format

`Current program` block should show:

- `Metric`
- `Statements`
- `Program spec`
- canonical pretty-printed `chain` JSON

`Selected saved program` block should show:

- `Name`
- `Saved`
- `Metric`
- `Statements`
- `Program spec`
- canonical pretty-printed `chain` JSON

The pretty-printed `chain` JSON should use two-space indentation and a monospace read-only display block.

### List Ordering

Saved-program table order:

- primary sort: `saved_at` descending
- tie-breaker: `id` ascending

## Existing Frontend Integration

The current local-file solve-score program flow is implemented in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) around:

- `_ensureSolveScoreProgramCatalogLoaded(...)`
- `loadSolveScoreProgramPreset(...)`
- `triggerSolveScoreProgramImport(...)`
- `saveSolveScoreProgram(...)`

The new modal should replace that local preset flow with S3-backed CRUD while reusing the existing solve-score chain compile/serialize path already used by:

- `_resolveSolveScoreState(...)`
- `_compileSolveScoreChain(...)`
- `_serializeSolveScoreChain(...)`

Load and upload failure behavior must preserve current safety semantics:

- validation must happen before the chain is mutated
- failed load/upload must leave the live chain unchanged

## API / Deploy Contract

This feature must be wired through the current API contract checks:

- [api_manifest.py](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.py)
- [api_manifest.json](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.json)
- [tests/test_api_route_contracts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_api_route_contracts.py)

For this feature, the chosen service shape is `storage`.

Required updates:

- add the four new storage paths to frontend calls
- add the four new routes to [handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
- add the four API routes in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- regenerate or update [api_manifest.json](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.json)

No new Lambda packaging or config service is required if the feature stays on `storage`.

## Recommended Implementation Order

1. Add `handler_storage.py` routes for `list`, `fetch`, `save`, and `delete`.
2. Add deploy routes and API manifest updates.
3. Seed existing wired solve-score presets into S3 once.
4. Add the shared `Solve Scores...` popup shell.
5. Replace the old Render and Palette preset/import/export controls.
6. Wire popup actions to the backend and current live solve-score state.
7. Add explicit loading / empty / error table states.
8. Return `overwritten=true|false` from save and surface that result in the popup status.
9. Remove obsolete local preset dropdown logic that is no longer used.
10. Add frontend, backend, and contract tests.

## Migration Scope

Migrate these existing wired presets into S3:

- [solve-score-programs/index.json](/Users/nicknassuphis/karpo_hackathon/polypaint/solve-score-programs/index.json)
- files referenced from that catalog under [solve-score-programs](/Users/nicknassuphis/karpo_hackathon/polypaint/solve-score-programs)

Do not automatically migrate repo-root [scores](/Users/nicknassuphis/karpo_hackathon/polypaint/scores) in the initial cut.

Reason:

- `solve-score-programs/` is what the current UI actually uses
- `scores/` appears to contain broader operator/export artifacts and is not currently the wired preset catalog

### Migration Mechanics

Use a one-off seed script, not a permanent deploy-time sync.

Suggested path:

- add a small script under [scripts](/Users/nicknassuphis/karpo_hackathon/polypaint/scripts)
- script reads [solve-score-programs/index.json](/Users/nicknassuphis/karpo_hackathon/polypaint/solve-score-programs/index.json)
- script loads each referenced JSON file
- script writes normalized saved objects to `polypaint/solve-score-programs/`

Seed script safety requirements:

- default mode is idempotent
- if a target key already exists, the script skips it by default
- script provides a dry-run mode
- script may provide an explicit `--force` mode to overwrite existing saved objects
- rerunning the seed script without `--force` must not clobber newer user-saved programs

The seed script is an operator/admin step, not part of normal app runtime.

## Implementation Map

Frontend:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - replace old preset/import/export controls
  - add shared modal markup
  - add popup state, list/fetch/save/delete/download/upload handlers
  - add loading/empty/error/selection logic

Backend:

- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
  - add 4 new routes and S3 CRUD logic

Deploy / API contract:

- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
  - publish the 4 new storage routes
- [api_manifest.json](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.json)
  - reflect the new storage paths
- [api_manifest.py](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.py)
  - no parser-shape change expected if routes stay on `storage`

Tests:

- add backend coverage near the storage handler tests or create a focused solve-score-programs storage test module
- [tests/test_api_route_contracts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_api_route_contracts.py)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- add or extend Playwright coverage, likely in:
  - [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)
  - [tests/e2e/palette-ui.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/palette-ui.spec.js)

## Tests

### Backend

Add handler coverage for:

- list
- fetch
- save
- delete
- invalid chain rejection
- empty-name rejection
- client-supplied `id` ignored on save
- client-supplied derived fields ignored on save
- chain compile error returned in save response and nothing written to S3
- duplicate-name overwrite behavior
- slug collision is observable:
  - save `My Prog!`
  - save `my prog?`
  - second save returns `overwritten: true`
  - stored object body is the second save

### Frontend

Add tests for:

- popup opens from Render
- popup opens from Palette
- table lists saved programs
- selecting a row enables `Load` and `Delete`
- `Load` updates the active tab
- `Save` persists the current live program
- `Delete` removes a saved program
- `Download` exports the current live program
- `Upload` imports a local JSON program
- load failure preserves the current live chain
- overwrite result is reported correctly when an existing slug is replaced
- loading / empty / error table states render correctly

## Acceptance Criteria

The feature is ready when all of the following are true:

- Render and Palette both use `Solve Scores...` instead of the old preset/import/export strip
- the popup lists globally saved programs from S3
- `Load`, `Save`, `Delete`, `Download`, and `Upload` all behave according to the rules above
- invalid load/upload/save requests never mutate the current live chain
- saved rows sort by newest first
- route wiring passes manifest and route-contract checks
- source checks and frontend browser coverage pass for both Render and Palette entry points

### Contract

Run:

- `python3 api_manifest.py --check`
- API route contract tests
- frontend source checks

## Non-Goals

Not in scope for this feature:

- editing the solve-score program body inside the popup
- per-user permissions or multi-tenant access control
- Compute-tab solve-score program management
- version history for saved solve-score programs

The right-side program block is display-only. If inline editing is desired later, that should be a separate feature.
