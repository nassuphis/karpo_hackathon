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

## Tabsets

PolyPaint has top-level workflow tabs and smaller in-panel tabsets. Both should
look like tabs, not like unrelated action buttons.

Rules:

- In-panel tabsets must reuse the same visual grammar as the top-level tab bar:
  rectangular tabs, shared bottom border, rounded top corners, muted inactive
  text, and pink-red active text.
- Do not style tab titles as pill buttons or introduce one-off active colors
  such as green, teal, or purple.
- Tab titles are navigation state, not execution actions; do not use
  `.btn-primary` / `.btn-secondary` styling for them.
- Swapping tab panes must not resize the surrounding workflow area. Allocate a
  fixed or explicitly bounded pane height and put overflow inside the active
  pane.
- Plot/debug/result tabs that share one region must have the same allocated
  height. Long debug text belongs in an internal scroll container.

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

### Program Editors

Chip-based program editors should use the Solve Score editor pattern.

Rules:

- Program chip strips must be fixed-height scroll regions, not auto-growing
  blocks.
- Use vertical chip layout with `flex-direction: column` and
  `flex-wrap: nowrap`; otherwise long programs wrap into sideways columns
  instead of scrolling down.
- Enable both vertical and horizontal overflow so long chip formulas remain
  readable without changing the editor height.
- Param Program and Coeff Program editors should visually match the Solve Score
  chip editor unless there is a deliberate workflow reason to differ.

### Chip Input Widths

Chip editors must size text inputs by the kind of value the user is expected to
type, not by whatever compact numeric width happens to fit the row. This applies
to all program editors: Solve Score, Param Program, Coeff Program, and any
future chip pipeline.

Rules:

- Expression/value fields must be wide enough that typical useful inputs are
  visible without horizontal scrolling inside the input.
- Do not give expression/value fields the same width as short selectors.
- Selector fields such as `src`, `tgt`, `mode`, `target`, and `emit` can be
  compact, but must still show the full selected value.
- Program ids, macro names, complex expressions, and formulas must be wide.
- Inputs for chip parameters that accept expressions should normally be at least
  `18ch`, and complex/formula-heavy fields should normally be `28ch` or wider.
- Very short numeric-only fields may be compact only when the value is truly a
  small scalar, for example a mode count, integer limit, or percentage.
- If a chip mixes selectors and expressions, give selectors the small columns
  and reserve the remaining row width for the expression fields.
- If the row cannot fit readable expression fields, allow the chip row to scroll
  horizontally inside the fixed editor region. Do not shrink fields until the
  text becomes unreadable.

Bad:

- `poke_poly` value field with only a few visible characters.
- Param `const` split into narrow `re` / `im` inputs when the user needs to type
  `100j*p1`.
- Coeff `push_const` or `poke_tos` expression inputs narrower than selector
  dropdowns.
- Solve Score chip parameters that hide most of a metric/formula-like value.

Good:

- `src` and `tgt` dropdowns are compact.
- `value`, `expr`, `program id`, `macro`, `a`, `b`, `pow`, and `andy` fields are
  sized according to the expected text length.
- Long expressions remain readable via a wide field or horizontal chip-strip
  scrolling, without changing the editor height.

### Color Inputs

Use a combined color control when a render-visible color is selected:

- native `input[type=color]` for the browser picker / eyedropper path
- adjacent monospace 6-digit hex text input for exact reproducibility
- optional small preset swatches for common values
- a short status readout showing the committed hex value or validation error

Rules:

- keep the color control in a grid/flex row with explicit alignment
- inline buttons in the color row must override global button top margin
- store and send colors as 6-digit hex without `#`
- do not hide a color change in a CSS-only preview; it must be forwarded into
  render payloads and persisted in artifact metadata when it affects output

## Render View Rows

The Render `View` controls are compact mode rows, but they are not plain inline
text. Do not lay them out as one wrapping flex strip.

Rules:

- Use the shared row shape: selector dot column, mode-name column, and a control
  grid column.
- `Auto` uses a two-row label/slider grid so `Quantile` and `Shim` share the
  same label and slider axes.
- `Exact` uses a two-row numeric grid so `Min Re` / `Min Im` and `Max Re` /
  `Max Im` line up vertically.
- Labels inside these rows must use zero margin and explicit line-height; do not
  rely on global `label` baseline behavior.
- If the row needs to collapse on narrow screens, collapse the grid deliberately
  instead of letting flex wrapping decide the order.

## Render Artifact Previews

The Render artifact viewer is a bounded preview pane. It must show the whole
artifact preview, not crop the natural-size `preview.png`.

Rules:

- The preview stage must be constrained to the viewer with explicit width and
  height.
- The image must use `object-fit: contain`, `max-width: 100%`, and
  `max-height: 100%`.
- Drag-selection math must use the displayed image rectangle, not the outer
  padded viewer rectangle, so letterboxed space does not distort viewport
  selection.

## Render Output Preview

The Render `Output` preview is ephemeral, but it is still a viewport-selection
surface.

Rules:

- Wrap the canvas in a positioned preview stage and draw the marquee overlay in
  that stage.
- The preview tabset and plot box should use the full available `Output` box
  width. Do not hardcode the displayed preview to the canvas backing size.
- Keep the displayed preview square with `aspect-ratio: 1 / 1`; the backing
  canvas can remain the requested preview pixel size.
- Use the preview response `viewport` plus the current rotation to compute
  selected exact bounds.
- Selection must update the `Exact` view inputs and switch View mode to `Exact`.
- Pointer math must use the displayed canvas rectangle, not the outer preview
  box.
- Right-click or `Escape` should clear the marquee without changing the current
  exact bounds.

## Checkbox And Radio Rows

Global `label` CSS currently uses block layout and a bottom margin. That is not
safe inside compact radio/checkbox rows.

Rules:

- Any inline checkbox or radio row must override inherited label spacing with
  `margin: 0`.
- The input and its text must be vertically centered by the row class, not by
  browser baseline behavior.
- Use an explicit input column when several radio/checkbox rows are stacked, so
  the controls share one vertical axis.
- Text next to the selector must use a fixed line-height that matches the input
  height closely enough to avoid the label sitting low.

Current Render Color pattern:

- `.color-mode-choice`
  - grid columns: fixed selector column + flexible text column
  - `align-items: center`
  - zero label margin
- `.color-mode-row .color-mode-choice`
  - use this more-specific selector when overriding row label defaults, so a
    generic `.color-mode-row label` rule cannot silently turn the choice back
    into inline-flex alignment
- `.color-mode-choice input[type="radio"]`
  - zero input margin
  - explicit width/height
  - centered in the selector column
- `.color-mode-name`
  - explicit line-height
- `.color-mode-palette-row`
  - shared palette controls for modes that actually consume a palette
  - hidden for direct RGB/HSV modes where palette selection is ignored
  - never duplicate palette swatches inside multiple radio rows

Do not add bare markup like:

```html
<label><input type="radio"> RGB</label>
```

inside matrix, panel, or compact grouped controls. That reintroduces browser
baseline alignment and inherited label-margin bugs.

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
6. Popups must not jump in height when summary or estimate text changes.
   Reserve space on swap-heavy summary/help blocks with a deliberate
   `min-height` instead of letting the dialog reflow on every selection change.

### Selection/Detail Popups

Popups that show a selectable list/table next to a detail pane need stricter
layout rules than simple tuning popups.

Rules:

- The popup shell must have a deliberate height, not only `max-height`.
- The main popup body should consume the remaining shell height with flex/grid
  sizing and `min-height: 0`.
- Action rows must remain inside the fixed popup shell and must never be pushed
  below the visible viewport. For multi-section dialogs, allocate shell rows
  explicitly, for example `auto auto minmax(0, 1fr) auto auto`.
- List/table panes and detail panes must be scroll containers inside that fixed
  body.
- Selection changes may update scrollable content, but must not change the
  popup shell height or the sibling pane sizes.
- `min-height` alone is not enough for swap-heavy detail panes when selected
  content can grow beyond the reserved space.
- Use fixed allocated regions such as `height: 100%`, flex children with
  `min-height: 0`, or grid rows like `minmax(0, 1fr)`.

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
