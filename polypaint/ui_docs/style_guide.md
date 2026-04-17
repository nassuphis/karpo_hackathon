# PolyPaint UI Style Guide

Status: current-shipped UI guide for [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html).

This document is meant to stop recurring UI regressions in PolyPaint:

- controls dropping out of alignment
- tuning controls crammed into one line
- browser-derived timings shown as if they were backend timings
- new flows inventing a fresh widget instead of reusing an existing pattern
- selection bugs caused by index-based DOM assumptions
- misleading disabled inputs that look interactive but do nothing

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
- Input labels must name the operational unit when ambiguity is possible:
  - `Threads`
  - `Workers`
  - `Retries`
  - `Chunks`
  - `Sections`
- Input labels must also name the scope when there are parallel variants:
  - `Hires`
  - `Lores`
  - `Hist`
  - `Raster`
  - `Palette`

Bad:

- `Param gen`
- `Coeffgen`
- `Solve`

Better:

- `Param-gen threads`
- `Coeffgen threads`
- `Solve workers`

Required when parallel scopes exist:

- `Hires param-gen threads`
- `Lores param-gen threads`
- `Hist sections`
- `Raster retries`

Rule:

- if the user cannot tell what is being counted or which stage it applies to,
  the label is incomplete

For popup tuning controls:
- use the existing `.autolevel-inline-fields` look
- use explicit labels like `Workers`, `Threads`, `Input`

## False Affordance

Do not use disabled inputs to display non-editable information.

If the user cannot change a value, render it as:
- plain text
- helper text
- a status label

Disabled form controls create false affordance:
- they look editable
- they often still show input/cursor styling
- they suggest interaction but do nothing

Example:
- a row like `Solve | solver default` should be rendered as text, not as a
  disabled text box

Do not restate an already-chosen action path as a fake control.

Example:
- entering `Calculate-AE-MT` already fixes the solver path to AE-MT
- do not render a disabled `Solve | solver default` row inside that popup
- either omit the row entirely or explain the constraint in plain text if it
  matters operationally

Practical rule:
- before adding any form control, ask whether the user can change it here
- if the answer is no, do not render an input

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
