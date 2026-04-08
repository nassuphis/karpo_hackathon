# PolyPaint UI Style Guide

Status: current-shipped UI guide for [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html).

This document is meant to stop recurring UI regressions in PolyPaint:

- controls dropping out of alignment
- tuning controls crammed into one line
- browser-derived timings shown as if they were backend timings
- new flows inventing a fresh widget instead of reusing an existing pattern
- selection bugs caused by index-based DOM assumptions

PolyPaint is a dense technical workbench, not a marketing site. The UI should
optimize for fast operator comprehension, stable control placement, and visible
runtime state.

## Core Principles

1. Use the existing visual language.
   Current PolyPaint is a dark, compact, technical console:
   - `body`: dark blue-black background
   - monospace inputs/tables/logs
   - muted gray metadata
   - pink-red for primary emphasis
   - blue-gray for secondary actions

2. Prefer consistency over novelty.
   Reuse the existing classes and shapes before inventing a new layout:
   - `.panel`
   - `.anim-row`
   - `.color-box`
   - `.tri-popup`
   - `.render-mt-matrix`
   - `.status`
   - `log()`

3. Keep operator-facing controls explicit.
   If a backend knob is being tuned experimentally or operationally, expose it
   directly instead of hiding it behind code or environment defaults.

4. Timings for backend work must come from AWS-side timestamps or backend timing
   fields.
   Browser wall time is not authoritative for Lambda/Step Functions work.

5. Dense is fine. Ambiguous is not.
   PolyPaint can be compact, but relationships must stay legible:
   - related controls grouped together
   - orthogonal settings arranged in rows/columns
   - logs separate from status text

## Page-Level Structure

Current top-level structure:

- app header
  - title + build id
  - config gear popup
- tab bar
  - `Compute`
  - `Results`
  - `Palette`
  - `Render`
  - `Favorites`
  - `DeepZoom`
- one shared `.panel` wrapping the active tab content

Rules:

- Keep the app header compact.
- Rare operational controls belong in the config popup, not in a full-width
  always-open panel.
- Tabs are for primary workflows, not low-frequency diagnostics.

## Layout Primitives

### `.panel`

Use `.panel` for the outer tab shell or other full workflow sections.

Use when:
- creating a major bounded area
- preserving the existing app chrome

Do not:
- stack nested panel-on-panel sections for minor grouping

### `.anim-row`

Use `.anim-row` for compact inline form rows.

Good for:
- small numeric inputs
- result/job selectors
- refresh rows
- short control strips

Rules:

- labels stay short
- rows may wrap if the control family is naturally linear
- if a row becomes a mini form with several orthogonal settings, stop using a
  flat row and move to a matrix/table layout

### `.color-box`

Use `.color-box` for secondary grouped controls inside a tab, especially in
Render and Palette.

Good for:
- grouped mode settings
- view options
- rotation
- color-family-specific controls

### Matrix/Table Layouts

If the UI is expressing:
- stages as rows
- settings as columns

then use a matrix, not an inline strip.

Canonical example:
- `.render-mt-matrix`
  - rows: `Hist`, `Raster`, `Merge`, `Finalize`
  - columns: `Threads`, `Input`, `Workers`

This is the correct pattern for “same categories across multiple stages”.

Do not:
- place 4-6 tuning inputs in one wrapping line
- rely on wrap order to explain relationships

## Buttons

Global `button` CSS currently includes `margin-top: 8px`.

This is acceptable only for standalone buttons.

### Inline Buttons

Any button inside:
- a flex row
- a compact toolbar
- a filter row
- a matrix cell

must explicitly override the inherited top margin, usually with:

- `style="margin:0; ..."`

or a deliberate horizontal margin like:

- `style="margin:0 0 0 8px; ..."`

Reason:
- this exact inheritance has repeatedly caused visible misalignment

### Button Semantics

Use:

- `.btn-primary`
  - dispatching work
  - preview/run/create/execute actions

- `.btn-secondary`
  - refresh
  - navigation
  - delete/download/config/auxiliary actions

Do not:
- introduce new button color semantics casually
- use primary styling for a non-committing navigation action

## Inputs

Rules:

- Numeric tuning fields are compact.
- Text filters can be flexible width.
- Monospace is preferred for technical identifiers and numeric values.
- Worker/thread inputs must be constrained in width and not allowed to stretch
  the whole layout.

For popup tuning controls:
- use the existing `.autolevel-inline-fields` look
- use explicit labels like `Workers`, `Threads`, `Input`

## Popups

Canonical popup system:
- `.tri-popup-overlay`
- `.tri-popup`
- `*-popup-summary`
- help text
- action row with `Exit` / `Execute`

Use popups for:
- low-frequency tuning
- A/B controls
- stage-specific advanced options
- selection workflows that need filtering

### Popup Rules

1. Overlay click closes.
2. `Escape` closes.
3. `Enter` executes only when that matches the popup’s purpose.
4. The popup summary should explain what is being tuned.
5. Help text should state scope and caveats.

Examples already shipped:
- config popup
- function chooser
- palette choosers
- `Generate`
- `Generate-MT`
- Results refresh

## Logs and Status

PolyPaint uses two different communication surfaces:

- `status`
  - short current state
- scrolling log
  - history of actions and progress

### Rule

Any tab that launches work, refreshes inventory, or performs multi-step actions
should have a scrolling log.

Current log targets:
- `compute-log`
- `results-log`
- `render-log`
- `favorites-log`
- `palette-log`

DeepZoom currently has status/inventory controls but is still more status-driven.

### Logging Rules

Use the shared `log(msg, cls, target)` helper.

Properties of the current log system:
- newest entries are prepended
- entries are timestamped in the browser for display ordering
- log color comes from `cls`
  - default
  - `ok`
  - `err`

Do not:
- create ad hoc logging widgets when an existing `*-log` area is appropriate
- hide important lifecycle messages in only a status line

## Timing Rules

This is a hard rule.

### Backend timings

For backend operations:
- render phases
- results refresh
- pipeline stage timing

displayed timings must come from:
- backend timing fields
- DynamoDB timestamps
- server-emitted run timestamps

Examples:
- render phase wall time from AWS-side timestamps
- Results refresh time from `/list` response fields like:
  - `list_us`
  - `prefix_list_us`
  - `calc_fetch_us`
  - `sort_us`

### Browser timings

Browser timing is acceptable only for:
- purely local UI behavior
- client-only helper operations
- as a fallback when no authoritative backend timing exists

Do not:
- present browser elapsed time as if it were AWS elapsed time
- mix client wall time with server timing without making that explicit

## Tables and Inventory Views

Current inventory/table pattern:
- compact monospace rows
- sticky header
- sortable columns where useful
- row selection highlight
- stable row identity through `data-*` attributes

### Rule: Selection must follow stable IDs, not array indices

When a table can be:
- sorted
- filtered
- partially refreshed

selection/highlighting must map by stable row key such as `job_id` or
`artifact_id`, not by the unsorted cache index.

This is a known regression class and must not be reintroduced.

### Results Table

Current expected shape:
- left: table + filters + results log
- right: preview/details/actions

Rules:
- keep the table fast and list-oriented
- heavier detail stays in row selection/detail fetch
- filtering belongs above the table
- operational refresh tuning belongs in a popup, not inline with the filter row

## Tunable Backend Controls

If a user needs to benchmark or tune backend concurrency, use explicit UI
controls rather than silent hidden defaults.

### Current shipped tuning controls

Render:
- `Generate`
  - solve-score hist input mode

Render MT:
- `Hist`
  - threads
  - input
- `Raster`
  - threads
  - input
- `Merge`
  - workers
- `Finalize`
  - workers

Results:
- refresh popup
  - workers

### Design Rule

When the same conceptual tuning exists across stages:
- use rows for stages
- use columns for tuning dimensions

When only one tuning dimension exists:
- use a small popup with one field and a short explanation

## Naming and DOM Conventions

Prefer predictable ids:

- tab ids: `tab-*`
- status ids: `*-status`
- log ids: `*-log`
- popup overlay ids: `*-popup-overlay`
- popup action ids:
  - `*-popup-close`
  - `*-popup-cancel`
  - `*-popup-run`

This matters because the current JS relies heavily on direct DOM id lookups.

## Current Failure Patterns To Avoid

### 1. Inline button misalignment

Cause:
- inherited global `button` margin in compact rows

Fix:
- explicit `margin:0` on inline buttons

### 2. One-line control soup

Cause:
- too many related controls in a flex row

Fix:
- move to matrix/table layout

### 3. Browser clock pretending to be backend truth

Cause:
- using `Date.now()` / tab-visible elapsed for AWS work

Fix:
- only show server-side timing fields for backend work

### 4. Wrong selected row after sorting

Cause:
- index-based highlight against unsorted cache

Fix:
- stable row id in DOM

### 5. Hidden tuning

Cause:
- performance knobs only in code/env

Fix:
- expose tunable backend knobs when they are part of active measurement

## Review Checklist For Any UI Change

Before shipping a UI change, check:

1. Does it reuse an existing layout primitive?
2. If it is inline, are button margins explicitly corrected?
3. If it has more than 3 orthogonal controls, should it be a matrix or popup?
4. If it displays timing, is the source backend-authored?
5. If it refreshes or runs work, does it log to the correct `*-log` target?
6. If it changes sorting/filtering/selection, is selection keyed by stable id?
7. If it introduces a backend tuning knob, is it visible and named clearly?
8. Does it preserve the dense-but-readable technical workbench feel?

## Testing Expectations

UI changes should usually update [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh) when they affect:

- required DOM ids
- popup existence/behavior
- button layout invariants
- payload wiring
- logging text
- selection behavior

Backend payload changes should also update the corresponding Python tests.

## Bottom Line

PolyPaint UI should feel like:
- a fast operator console
- explicit about state
- explicit about tuning
- visually compact
- structurally disciplined

When in doubt:
- reuse the shipped popup pattern
- reuse the shipped matrix pattern
- log visibly
- trust AWS timings, not browser elapsed time
