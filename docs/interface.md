# Interface Guide

Complete control reference for PolyPaint v40. For a quick overview, see the [README](../README.md#interface-overview).

## Layout Overview

![Full app layout](images/iface_layout.png)

The UI is organized around a compact header bar and two side-by-side complex-plane panels (left: coefficients, right: roots) with an operations mid-bar between them.

---

## Header Bar

![Header bar](images/iface_header.png)

| Control | ID | Description |
|---------|----|-------------|
| **PolyPaint v40** | `.app-title` | App title and version label. |
| **Degree** | `#degree-number` | Click the number to open a slider popover (2-30). Reinitializes coefficients on change. |
| **Pattern** | `#pattern` | Initial arrangement of coefficients or roots. 26 patterns in 3 categories: Basic (5), Coefficient shapes (8), Root shapes (13). See [Patterns](patterns.md). |
| **Reset** | `#reset-btn` | Reset to initial state for the current degree and pattern. |
| **Save** | `#save-btn` | Save full application state as a JSON file. |
| **Load** | `#load-btn` | Load a previously saved JSON snapshot. |
| **Export** | `#snap-btn` | Opens a popup with 7 export modes (see below). |
| **T** | `#timing-btn` | Timing stats popup: steps/sec, pass times, composite breakdown. Clears when worker count changes. |
| **Scrub slider** | `#scrub-slider` | Additive scrubber (range 0-5000): drag to add seconds to the current elapsed time. Only works when animation is paused. Resets to zero on release. |
| **Play / Pause / Resume** | `#play-btn` | 3-state cycle: **Play** starts from zero, **Pause** stops mid-animation, **Resume** continues from where it left off. Requires at least one animated C-node or D-node (morph auto-activates when D-nodes exist). |
| **Seconds counter** | `#anim-seconds` | Displays current animation elapsed time (e.g. `3.14s`). Updates during playback and scrubbing. |
| **Home** | `#home-btn` | Return all animated C and D nodes to their start positions (curve[0]) and reset elapsed to 0. |

### Header Layout Order

Left to right: App title | Degree + Pattern | Reset | Save + Load + Export | T | Scrub slider | Play/Pause/Resume | Seconds counter | Home

### Export Popup

![Export popup](images/iface_export_popup.png)

Seven capture modes: Both, Coeffs, Roots, Stats, Sound, Bitmap, Full. Each downloads a PNG screenshot plus a JSON snapshot of the full app state.

---

## Left Panel

The left panel has six tabs: C-Nodes, C-List, D-Nodes, D-List, Jiggle, and Final.

### C-Nodes Tab

![C-Nodes tab with circle paths](images/iface_coeffs_paths.png)

Interactive SVG complex-plane visualization of polynomial coefficients (`#coeff-panel` inside `#coeff-container`).

- **Drag** any coefficient dot to move it; roots update instantly on the right panel.
- **Right-click empty canvas** to add a new coefficient at that position (becomes the new highest-power term).
- **Right-click an existing coefficient** to open a context menu (`#coeff-ctx`) with trajectory settings, live preview, and a Delete button (disabled when only 3 coefficients remain, since minimum degree is 2). Click "Accept" to commit or press Escape to revert. A reposition button cycles the menu through 4 quadrants.
- Assigned trajectory curves are always visible as colored paths on the canvas.

### Trajectory Editor (Animation Bar)

![Trajectory editor / animation bar](images/iface_anim_bar.png)

Located at the top of the C-Nodes tab (`#anim-bar`). Controls the animation path for selected coefficients.

**First row:** Coefficient picker (`#coeff-picker-btn`), Select All (`#select-all-coeffs-btn`) / Deselect (`#deselect-all-btn`), selection label (`#bar-title`), **Update Whole Selection** button (`#sel2path-btn`).

**Second row (`#bar-controls`):** Path type dropdown (`#anim-path`) and path-specific controls (`#bar-dynamic`). Sliders change based on the chosen path type (e.g. R/S/A/CW-CCW/N for circles, S/sigma/N for Gaussian). All path types expose an **N** slider (100--100000, default 200) controlling curve sample point count. Adjusting controls shows a live preview of the proposed paths. Click away or press Escape to revert; click **Update Whole Selection** to commit. Controls dim when no coefficients are selected.

#### Preview / Revert / Commit Pattern

The trajectory editor uses a preview/revert workflow. When you change the path dropdown or adjust a slider, the changes are shown as a live preview on the canvas (backed by `barSnapshots` which stores the original state). You can:

- Click **Update Whole Selection** to commit the previewed changes (`commitBarPreview()`)
- Click outside the editor or press **Escape** to revert to the original state (`revertBarPreview()`)

This replaces the previous immediate-apply behavior, giving you a chance to experiment without permanently changing paths.

#### Path Types (48 for C-nodes, 77 for D-nodes)

Paths are defined in `PATH_CATALOG`. Every path in a group (except Gaussian cloud, Disk sample, and Square sample) automatically gets a dithered variant that adds small noise to each step. Dithered path variants expose two additional parameters: a **sigma** slider controlling dither magnitude and a **Dist** dropdown (`_DIST_PARAM`) to choose between **Normal** (Gaussian) and **Uniform** distribution for the dither noise.

| Group | Base Paths | With Dither |
|-------|-----------|-------------|
| **Top-level** | None | -- |
| **Basic** (9) | Circle, Horizontal, Vertical, O-Spiral, C-Spiral, Gaussian cloud, Disk sample, Square sample, Grid sample | +6 dithered (cloud types random, disk-cloud, sq-cloud excluded) |
| **Curves** (13) | Lissajous, Figure-8, Cardioid, Astroid, Deltoid, Rose (3-petal), Spirograph, Hypotrochoid, Butterfly, Star (pentagram), Square, O-Ellipse, C-Ellipse | +13 dithered |
| **Space-filling** (3) | Hilbert (Moore), Peano, Sierpinski | +3 dithered |
| **Orbit** (D-only, 15) | Follow C, Orb-Circle, Orb-Horizontal, Orb-Vertical, Orb-Figure8, Orb-Lissajous, Orb-Star, Orb-Square, Orb-Cardioid, Orb-Astroid, Orb-Deltoid, Orb-Rose, Orb-Epitrochoid, Orb-Hypotrochoid, Orb-Butterfly | +14 dithered (Follow C excluded) |

Cloud path types (random, disk-cloud, sq-cloud) store curve points as offsets from the anchor position, keeping the C-node visually stationary during animation while the solver uses `c._effRe/_effIm` = anchor + offset. Grid sample (grid-cloud) does get a dithered variant.

Each coefficient stores its own path type, rAbs (radius in absolute world units), speed, angle, and direction independently. The UI exposes rAbs via `buildRabsControl()` -- a mantissa slider plus order-of-magnitude (OOM) buttons. See [Paths](paths.md) for curve formulas, cycle sync, and space-filling curve details.

#### Trajectory Workflow

1. Select one or more coefficients (click, marquee, or Select All)
2. Choose a path type and adjust controls -- a live preview shows the proposed paths
3. Click **Update Whole Selection** to commit, or click away / press Escape to revert
4. Press **Play** (in the header bar) -- all coefficients with a trajectory animate simultaneously

### C-List Tab

![C-List tab](images/iface_list_tab.png)

Tabular view of all coefficients with per-row columns:

| Column | Content |
|--------|---------|
| Checkbox | Selection toggle |
| Color dot | Index-based color |
| Sensitivity dot | Jacobian sensitivity indicator |
| Label | Coefficient index (c0, c1, ...) |
| Power | Polynomial power |
| Path | Animation path type (clickable to open per-coefficient path picker popup) |
| Speed | Animation speed (or dash) |
| rAbs | Path radius in absolute world units (or dash) |
| Curve length | Number of sample points (user-configurable via N parameter, 100--100000) |
| Curve index | Current position along path |
| Coordinates | Complex value (re + im*i) |

**Toolbar (`#coeff-list-toolbar`):** Select All (`#list-select-all-btn`) / Deselect (`#list-deselect-all-btn`), **Same Curve** (`#list-all-curves-btn`) selects all coefficients with the displayed curve type, curve type cycler (prev `#list-curve-prev` / next `#list-curve-next` arrows to cycle through path types present in the polynomial, display label `#list-curve-cycle`), selection count (`#list-count`), Transform dropdown (`#list-transform`), Param1 (`#list-sel-speed`) / Param2 (`#list-sel-param2`) sliders.

#### C-List Curve Editor

![C-List curve editor](images/iface_list_editor.png)

The curve editor (`#list-curve-editor`) below the toolbar lets you edit the path for the selection. It shows a path type dropdown (`#lce-path-sel`) and path-specific controls (`#lce-controls`) based on the first selected coefficient's current path type (tracked by `lceRefIdx`). Choose a path type, adjust parameters, and click **Update Whole Selection** (`#lce-update-sel`) to apply to all selected coefficients. Controls dim when nothing is selected.

#### Transform dropdown (20 bulk operations)

| Transform | Description |
|-----------|-------------|
| PrimeSpeeds | Set all speeds coprime with each other |
| Set All Speeds | Set selected to Param1 speed |
| RandomSpeed | Random speed for each selected |
| RandomAngle | Random starting angle |
| RandomRadius | Random rAbs |
| Lerp Speed | Interpolate speeds from Param1 to Param2 |
| Lerp Radius | Interpolate rAbs values |
| Lerp Angle | Interpolate angles |
| RandomDirection | Random CW/CCW for each |
| FlipAllDirections | Toggle CW/CCW on all selected |
| ShuffleCurves | Randomly reassign path shapes |
| ShufflePositions | Randomly swap home positions |
| CircleLayout | Arrange selected on a circle |
| RotatePositions | Rotate home positions by Param1 degrees |
| ScalePositions | Scale home positions by Param1 factor |
| JitterPositions | Add Gaussian noise to positions |
| Conjugate | Reflect across real axis |
| InvertPositions | Invert through unit circle |
| SortByModulus | Reorder by distance from origin |
| SortByArgument | Reorder by angle from positive real axis |

**Param1 / Param2** sliders (range 1-1000) supply arguments to transforms that need them (e.g. Lerp endpoints, rotation angle). Speed values use thousandths (e.g. Param1=500 means speed 0.500).

### D-Nodes Tab

![D-Nodes tab](images/iface_morph_tab.png)

Interactive SVG complex-plane visualization of morph target D-nodes (`#morph-panel` inside `#morph-container`). See [Morph](morph.md).

**Morph auto-activation:** Morph is always enabled when D-nodes exist. There is no manual enable/disable checkbox -- the morph checkbox was removed in v39. When D-nodes are present, the solver automatically blends between C and D coefficients according to the current mu parameter.

**Morph bar (`#morph-bar`):**

| Control | ID | Description |
|---------|----|-------------|
| **Copy C->D** | `#morph-copy-btn` | Copy current C-coefficients to D-nodes. |
| **Swap C<->D** | `#morph-swap-btn` | Swap C-coefficient and D-node positions. |
| **C-D Path** | `#morph-cdpath-btn` | Open the C-D path popup (see below). |

- D-node dots with drag interaction (same as coefficient panel)
- **Right-click a D-node** to open a context menu (`#dnode-ctx`) with the same trajectory settings, live preview, Accept/Delete/reposition as the C-node context menu
- Selection count shown in the tab header (`#morph-sel-count`)

#### C-D Path Popup

The C-D path popup (`#cdpath-pop`) controls how the blend parameter mu traverses the path between C-node and D-node positions during morph animation. Uses Accept/Revert workflow: click Accept to commit changes, or click outside / press Escape to revert.

| Control | Description |
|---------|-------------|
| **Path type** dropdown | Interpolation path shape: **Line** (default), **Circle**, **Ellipse**, **Figure-8**. |
| **CW / CCW** toggle button | Clockwise or counter-clockwise traversal. Hidden for Line path type. |
| **Minor** slider (10-100%) | Ellipse minor axis as a fraction of the major axis. Only visible when Ellipse is selected. |
| **Rate** slider (0.0000-0.0100 Hz) | Morph oscillation frequency. Controls how fast mu traverses the C-D path. |
| **Start sigma** slider | Dither amount at the C/start position (envelope: max(cos theta, 0)^2). |
| **Mid sigma** slider | Dither amount at the midpoint (envelope: sin^2 theta). |
| **End sigma** slider | Dither amount at the D/end position (envelope: max(-cos theta, 0)^2). |
| **Accept** button | Commit the current settings and close the popup. |

The three sigma sliders add position-dependent noise along the morph path. Each slider's dither is weighted by an envelope function so that noise is strongest at its designated position (start, midpoint, or end) and fades elsewhere.

### D-List Tab

![D-List tab](images/iface_dlist_tab.png)

Identical structure to the C-List tab, but for morph target D-nodes. Assign paths, speeds, and transforms to D-nodes independently from C-coefficients. The toolbar (`#dcoeff-list-toolbar`) has the same controls: Select All (`#dlist-select-all-btn`) / Deselect (`#dlist-deselect-all-btn`), Same Curve (`#dlist-all-curves-btn`), curve type cycler (prev/next arrows, label `#dlist-curve-cycle`), selection count (`#dlist-count`), Transform dropdown (`#dlist-transform`), and Param1 (`#dlist-sel-speed`) / Param2 (`#dlist-sel-param2`) sliders. The curve editor (`#dlist-curve-editor`) uses the same pattern: select D-nodes, adjust parameters via dropdown `#dle-path-sel` and controls `#dle-controls`, and click **Update Whole Selection** (`#dle-update-sel`) to apply. D-nodes have an additional **Follow C** path type that mirrors the corresponding C-node's position (no speed or rAbs displayed). See [D-Node Paths](d-node-paths.md).

### Jiggle Tab

Dedicated tab for coefficient perturbation controls (`#jiggle-content`).

- **Mode** dropdown (12 modes, see below)
- Mode-specific parameter controls (sigma, steps, amplitude, period, etc.)
- **Interval** slider (0.1-100 seconds, step 0.1, between perturbation triggers) with **-0.1** / **+0.1** precision buttons
- **GCD** button: auto-compute interval from the GCD of coefficient speeds

**Jiggle modes** (12):

| Mode | Description | Parameters |
|------|-------------|------------|
| None | No perturbation | -- |
| Random | Gaussian offsets each trigger | sigma (range slider 0-10, **-0.01** / **+0.01** buttons) |
| Rotate | Rotate selected around centroid | steps (range slider 10-5000, **-1** / **+1** / **P** (nearest prime) buttons) |
| Walk | Random walk accumulating offsets | sigma (range slider 0-10, **-0.01** / **+0.01** buttons) |
| Scale (center) | Scale from origin | growth % per trigger |
| Scale (centroid) | Scale from centroid | growth % per trigger |
| Circle | Rotate around origin | steps (range slider 10-5000, **-1** / **+1** / **P** buttons) |
| Spiral (centroid) | Rotate + scale around centroid | steps (10-5000, **-1** / **+1** / **P** buttons), growth % |
| Spiral (center) | Rotate + scale around origin | steps (10-5000, **-1** / **+1** / **P** buttons), growth % |
| Breathe | Sinusoidal scaling from centroid | amplitude, period (cycles) |
| Wobble | Sinusoidal rotation around centroid | steps (10-5000, **-1** / **+1** / **P** buttons), period (cycles) |
| Lissajous | Translate along Lissajous figure | amplitude, steps (range slider 10-5000 for period, **-1** / **+1** / **P** buttons), freqX, freqY |

See [Paths](paths.md) for jiggle formulas.

### Final Tab

![Final tab](images/iface_morph_tab.png)

Shows the actual coefficients sent to the solver (blended C/D positions when morphing is active). Located in `#final-content`.

- **mu** display (`#morph-mu-val`): current blend parameter (0=C, 1=D)
- SVG complex-plane visualization (`#final-panel`) of the final blended coefficients

Morph is always enabled when D-nodes exist (no checkbox). The morph rate and path shape are controlled via the [C-D Path popup](#c-d-path-popup) in the D-Nodes tab.

---

## Mid-bar Operations

![Mid-bar operations](images/iface_midbar.png)

Located between the left and right panels (`.mid-bar`). Buttons brighten when a selection exists.

| Tool | ID | Control | Description |
|------|----|---------|-------------|
| **Scale** | `#scale-tool-btn` | Vertical slider, exponential mapping | Scale uniformly (0.1x - 10x) |
| **Scale Re** | `#scale-re-tool-btn` | Vertical slider | Scale only the real component |
| **Scale Im** | `#scale-im-tool-btn` | Vertical slider | Scale only the imaginary component |
| **Rotate** | `#rotate-tool-btn` | Horizontal slider | Rotate (+/-0.5 turns) |
| **Translate** | `#add-tool-btn` | 2D vector pad | Translate (+/-2 in each axis) |
| **Shape** | `#shape-tool-btn` | Shape chips + slider | Morph selected positions toward a target shape (Box, Tri, Infinity, Pent). Slider controls blend (0-100%). |
| **Pattern** | `#ptrn-tool-btn` | Pattern dropdown + params + Accept | Arrange selected positions on a geometric pattern. 21 patterns: Circle, Square, Triangle, Pentagon, Hexagon, Diamond, Star, Ellipse, Infinity, Spiral, Grid, Line, Wave, Cross, Heart, Lissajous, Rose, 2 Circles, 2 Squares, Ring, Scatter. 13 patterns have adjustable parameters (e.g. Star inner radius, Grid columns, Lissajous frequencies) shown as sliders below the dropdown. Uses **Accept/Revert**: click Accept to commit, or close/Escape to revert. See [Patterns](patterns.md#pattern-arrange-tool-mid-bar). |

Scale, Scale Re, Scale Im, Rotate, and Translate open transient popovers with live drag-to-apply preview. Shape opens a slider popup. Pattern opens an Accept/Revert popup. All close by clicking outside or pressing Escape.

The tools operate on whichever node kind is currently selected (C-coefficients, D-nodes, or roots). A colored indicator label (`#ops-target`) shows the target: **C** (green), **D** (blue), **roots** (red), or **none** (grey). The `#ops-group` div gets a `disabled` class when nothing is selected.

Additional controls:
- **Select All** (`#mid-all-btn`) / **Deselect** (`#mid-none-btn`) -- context-aware: selects all D-nodes if any D-node is selected, all roots if any root is selected, otherwise all coefficients
- **Inv** (`#inverse-btn`) -- reverse coefficient order (reflects roots around the unit circle)

---

## Right Panel

The right panel has five tabs: Roots, Stats, Sound, Bitmap, and BITCFG. Recording controls sit in the tab bar.

### Recording Controls

Located in the right panel tab bar (`span.tab-bar-controls`):

| Control | ID | Description |
|---------|----|-------------|
| **Mode selector** | `#rec-mode` | Roots, Coeffs, Both, Stats, Sound, Bitmap, or Full. "Both" is tab-aware -- renders coefficients plus the active right-side tab. |
| **Record** button | `#rec-roots-btn` | Start WebM video capture. |
| **Stop** button | `#stop-roots-btn` | Stop recording (hidden until recording starts). |

**Auto-stop:** When Trails are enabled, recording auto-stops on loop completion.

### Roots Tab

![Roots tab with toolbar](images/iface_roots_tab.png)

Interactive SVG complex-plane visualization of polynomial roots (`#roots-panel` inside `#roots-container`, with `#domain-canvas` behind it for domain coloring).

**Roots toolbar** (`#roots-toolbar`):

![Roots toolbar](images/iface_roots_toolbar.png)

| Control | ID | Description |
|---------|----|-------------|
| **Trails** | `#trails-btn` | Toggle root trail recording. Loop detection auto-stops after one full cycle. Jump detection breaks trails at root-identity swaps (>30% of visible range). |
| **Color** | `#root-color-btn` | Opens a root coloring popover (`#color-pop`). Controls animation dots only (bitmap coloring is independent). |
| **Domain** | `#domain-toggle-btn` | Toggle domain coloring on the roots canvas background. HSL-mapped: hue = argument, saturation = magnitude. |
| **Fit** | `#roots-fit-btn` | Auto-zoom to fit all roots and trails. |
| **-25%** | `#roots-zoom-in-btn` | Zoom in by 25%. |
| **+25%** | `#roots-zoom-out-btn` | Zoom out by 25%. |
| **Auto-fit** | `#roots-autofit-cb` | Checkbox: auto-zoom roots panel to show all roots after each solve (`rootsAutoFit`). |
| **Select All** | `#select-all-roots-btn` | Select all roots. Behavior depends on `rootSelMode`: selects pinned roots if last selection was pinned, otherwise selects free roots. |
| **Deselect** | `#deselect-all-roots-btn` | Deselect all roots. |
| **Pin** | `#roots-pin-btn` | Pin selected free roots (batch). See [Root Pinning](#root-pinning). |
| **Free** | `#roots-free-btn` | Free selected pinned roots (batch). See [Root Pinning](#root-pinning). |
| **Epsilon toolbar** | `#eps-toolbar` | Shown when pinned roots exist. `pinnedEpsilon` slider with OOM buttons. |

#### Root Color Popup

![Root color popup](images/iface_root_color_pop.png)

**Animation root color modes** (3):

| Mode | Description |
|------|-------------|
| Uniform | Single color from 8-color swatch (White, Red, Orange, Yellow, Green, Cyan, Blue, Purple) |
| Index Rainbow | Color by root index using d3 rainbow spectrum |
| Derivative | Jacobian sensitivity: blue (stable) through white to red (volatile) |

#### Domain Coloring

![Domain coloring](images/iface_domain_coloring.png)

HSL-mapped complex landscape on the roots panel background. Hue represents the argument (phase) of p(z) and brightness represents the magnitude. Roots appear as points where all colors converge.

#### Trails

![Root trails](images/iface_trails.png)

Root trails record positions as colored SVG paths during animation. Loop detection auto-stops after one full cycle. Jump detection breaks trails at root-identity swaps.

- **Drag any root** to move it; coefficients update on the left via inverse reconstruction.
- During root drag, a dashed polyline connects coefficient dots, visualizing the coefficient chain.
- **Marquee select** works the same as on the coefficients panel.
- **Right-click any root** (free or pinned) to open a context menu (`#root-ctx`) with Free/Pinned toggle and Delete button. See [Root Pinning](#root-pinning) below.

#### Root Pinning

Roots can be **pinned** to fix them in place while the remaining free roots are solved. Pinning a root performs synthetic division, removing a linear factor from the polynomial. Freeing a pinned root multiplies the factor back in.

**Toolbar controls** (in `#roots-toolbar`):

| Control | ID | Description |
|---------|----|-------------|
| **Pin** | `#roots-pin-btn` | Pin all selected free roots (batch). Disabled if fewer than 3 coefficients would remain. |
| **Free** | `#roots-free-btn` | Free all selected pinned roots (batch). Multiplies the linear factor back into the polynomial. |
| **Auto-fit checkbox** | `#roots-autofit-cb` | Toggle `rootsAutoFit` -- when checked, the roots panel auto-zooms to show all roots after each solve. |
| **Epsilon toolbar** | `#eps-toolbar` | Shown when pinned roots exist. Contains `pinnedEpsilon` slider with OOM buttons (up/down to shift the divisor by powers of 10). Epsilon adds a coupling term to the expanded polynomial. |

**Root context menu** (`#root-ctx`): Right-click any root to open:

- **Free / Pinned** toggle buttons -- switch the root between free and pinned states. Pinning is disabled when `coefficients.length <= 3` (minimum degree 2).
- **Delete** button -- removes the root entirely. Guarded by `totalDeg = coefficients.length - 1 + pinnedRoots.length` which must remain > 2.
- **Accept** button -- closes the context menu.

Pinned roots appear as separate dots on the SVG canvas. The `rootSelMode` variable (`"free"` | `"pinned"`) tracks which kind of root was last selected, so that **Select All** in the roots toolbar selects the appropriate set.

### Stats Tab

![Stats tab](images/iface_stats_tab.png)

A 4x4 grid of 16 configurable plots. Each plot has a dropdown to select from 32 chart types:

**Time-series (23):**

| Stat | Description |
|------|-------------|
| Force / MinForce / MaxForce | Jacobian sensitivity per root |
| Speed / MinSpeed / MaxSpeed | Root displacement magnitudes |
| MinDist / MaxDist / MeanDist | Pairwise root distances |
| delta-MeanDist / sigma-Dist | Distance rate-of-change and spread |
| Records | Close-approach record-breaking events |
| AngularMom | Signed rotational momentum |
| sigma-Speed / EMASpeed | Speed statistics |
| Odometer / CycleCount | Cumulative distance and cycle detection |
| MedianR / Spread | Median radius, 90th-10th percentile spread |
| EMed / EHi | 50th / 85th percentile speeds |
| Coherence | Angular clustering measure |
| Encounters | Per-root close-approach events |

The last 6 (MedianR through Encounters) are sonification features with EMA smoothing matching the audio pipeline.

**Phase-space (5):** Force vs Speed, MinDist vs MaxDist, MinSpeed vs MaxSpeed, MeanDist vs sigma-Dist, MaxForce vs MaxSpeed

**Spectrum (4):** SpeedSpectrum, OdometerSpectrum, WindingSpectrum, TortuositySpectrum (per-root bar charts with all-time peak dots)

Data is collected every frame into a 4000-frame ring buffer.

### Sound Tab

![Sound tab](images/iface_sound_tab.png)

Three independent audio layers, each with a toggle button and config popover (`#audio-pop`). See [Sonification](sonification.md) for the full algorithm, audio graph, and signal routing matrix.

| Voice | ID | Type | Key Parameters |
|-------|----|------|---------------|
| **B** Base | `#base-toggle-btn` | FM drone | Pitch (55-440 Hz), Range, FM Ratio, FM Depth, Brightness, Volume, Vibrato |
| **M** Melody | `#melody-toggle-btn` | Pentatonic arpeggiator | Rate (2-60 notes/s), Cutoff, Volume, Attack, Decay, Brightness |
| **V** Voice | `#voice-toggle-btn` | Encounter beeps | Cooldown (10-500 ms), Volume, Attack, Decay |

The Sound tab also exposes a **signal routing matrix** (`#config-sliders`): any of 25 computed stats can be patched into any of 14 audio parameters, with per-route normalization (Fixed or adaptive RunMax) and EMA smoothing.

### Bitmap Tab

![Bitmap tab](images/iface_bitmap_tab.png)

Accumulates root (or coefficient) positions as single-pixel stamps on a high-resolution canvas (`#bitmap-canvas` inside `#bitmap-container`).

**Toolbar (`#bitmap-toolbar`):**

![Bitmap toolbar](images/iface_bitmap_toolbar.png)

| Control | ID | Description |
|---------|----|-------------|
| **init** | `#bitmap-start-btn` | Snapshot animation state, create bitmap canvas, reset elapsed to 0. |
| **save** | `#bitmap-save-btn` | Open format popup (`#bitmap-save-pop`): JPEG (with quality slider), PNG, BMP, TIFF. Downloads from CPU buffer at full resolution. |
| **clear** | `#bitmap-clear-btn` | Reset canvas pixels. Elapsed time unchanged. |
| **Resolution** | `#bitmap-res-select` | Canvas size: 1000 / 2000 / 5000 / 8000 / 10000 / 15000 / 25000 px. Above 2000px, display is capped at 2000px while computation runs at full resolution. Auto-restarts fast mode if changed during active rendering. |
| **start / stop** | `#bitmap-fast-btn` | Toggle continuous fast mode (parallel Web Workers). Stop preserves state; start resumes where it left off. |
| **ROOT / COEF** | `#bitmap-coeff-btn` | Toggle between plotting root or coefficient positions. |
| **Steps** | `#bitmap-steps-select` | Solver steps per pass: 10 / 100 / 1K / 5K / 10K / 50K / 100K / 1M. Auto-restarts fast mode if changed during active rendering. |
| **cfg** | `#bitmap-cfg-btn` | Switch to the BITCFG tab (see [BITCFG Tab](#bitcfg-tab) below). |
| **-25%** | `#bitmap-zoom-in-btn` | Zoom in bitmap view by 25%. |
| **+25%** | `#bitmap-zoom-out-btn` | Zoom out bitmap view by 25%. |
| **Zoom label** | `#bitmap-zoom-label` | Current bitmap zoom level (e.g. `1.00x`). |
| **Pass count** | `#bitmap-pass-count` | Zero-padded elapsed-seconds counter shown during computation. |

#### Bitmap Rendering

![Bitmap after rendering](images/iface_bitmap_rendered.png)

After clicking **init** and **start**, parallel Web Workers continuously solve the polynomial and plot root positions as pixels on the bitmap canvas.

#### Bitmap Save Popup

![Bitmap save popup](images/iface_bitmap_save.png)

Four export formats: JPEG (with quality slider), PNG, BMP, TIFF. The image is exported at full compute resolution, not the display-capped resolution.

### BITCFG Tab

The BITCFG tab (`#bitcfg-content`) consolidates all bitmap configuration into a dedicated right-panel tab. Clicking the **cfg** button in the bitmap toolbar switches to this tab. The content is built dynamically by `buildBitmapCfgContent()`.

#### Solver Engine & Workers

| Setting | Options |
|---------|---------|
| **Solver engine** | JavaScript or WebAssembly. See [WASM](wasm_investigation.md). |
| **Workers** | Number of parallel Web Workers: 1, 2, 4, 8, 16. |

#### Background Color

24 preset colors (darks, lights, grays) displayed as a swatch grid.

#### Root Color Modes (7)

Independent from animation root coloring.

| Mode | Description | Palette / Options |
|------|-------------|-------------------|
| Uniform | Single fixed color | 8-color swatch |
| Index Rainbow | Color by root index | d3 rainbow spectrum + matching strategy chips |
| Derivative | Jacobian sensitivity | 16-entry blue-white-red |
| Root Proximity | Min distance to nearest root | Sequential palette from catalog (8 options) |
| Idx x Prox | Index Rainbow hue modulated by proximity brightness | Matching strategy chips + gamma slider (0.1-1.0) |
| Min/Max Ratio | Ratio of min/max root distance | Sequential palette from catalog (8 options) + gamma slider (0.1-1.0) |
| Rel. Proximity | Relative proximity -- normalized distance to nearest root | Sequential palette from catalog (8 options) |

**Proximity palette catalog** (used by Root Proximity, Min/Max Ratio, and Rel. Proximity modes): Inferno (default), Viridis, Magma, Plasma, Turbo, Cividis, Warm, Cool. Each renders as a gradient circle; clicking selects the palette and switches to the corresponding mode.

#### Floor / Ceiling / Frequency Sliders

Each palette-based color mode exposes three additional sliders (appended via `_appendFCF()`):

| Slider | Label | Range | Description |
|--------|-------|-------|-------------|
| **Floor** | F | 0.00--1.00 | Minimum value mapped to the lowest palette color. Values below floor are clamped. |
| **Ceiling** | C | 0.00--1.00 | Maximum value mapped to the highest palette color. Values above ceiling are clamped. Floor and ceiling are linked: floor cannot exceed ceiling and vice versa. |
| **Frequency** | ~ | 0.0--10.0 | Palette cycling frequency. Higher values repeat the palette more times across the value range. |

These sliders are available for all modes that use a sequential palette (Derivative, Root Proximity, Idx x Prox, Min/Max Ratio, Rel. Proximity).

**Root-matching strategies** (used by Index Rainbow and Idx x Prox modes):

| Strategy | Algorithm | Cost | Accuracy |
|----------|-----------|------|----------|
| Hungarian | Kuhn-Munkres optimal assignment | O(n^3) every step | Perfect |
| Greedy x1 | Nearest-neighbor | O(n^2) every step | Good |
| Greedy x4 | Nearest-neighbor every 4th step | O(n^2) / 4 | Slight drift (default) |

---

## Popups Reference

All popups use the `.ops-pop` CSS class, positioned via `getBoundingClientRect()`, toggled via `.open` class.

| Popup | ID | Opened By |
|-------|----|-----------|
| Operations tool | `#ops-pop` | Mid-bar tool buttons (Scale, Rotate, Translate, Shape, Pattern) |
| Degree slider | `#degree-pop` | Clicking degree number |
| Audio config | `#audio-pop` | Sound voice toggle buttons (B, M, V) |
| Root color | `#color-pop` | Root color button in roots toolbar |
| Coefficient picker | `#coeff-pick-pop` | Coefficient picker button in anim bar |
| C-node context menu | `#coeff-ctx` | Right-click on a coefficient dot |
| D-node context menu | `#dnode-ctx` | Right-click on a D-node dot |
| Root context menu | `#root-ctx` | Right-click on a root (free or pinned) |
| Export | `#snap-pop` | Export button in header |
| C-List path picker | `#path-pick-pop` | Clicking path cell in C-List table |
| D-List path picker | `#dpath-pick-pop` | Clicking path cell in D-List table |
| Timing stats | `#timing-pop` | T button in header |
| Bitmap save format | `#bitmap-save-pop` | save button in bitmap toolbar |
| C-D path | `#cdpath-pop` | C-D Path button in morph bar |

---

## Selection Model

![Selection with glow](images/iface_selection.png)

- **Click** any dot to toggle it into the selection.
- **Marquee select:** Click and drag on empty canvas to draw a selection rectangle.
- **Select All / Deselect** buttons in the trajectory editor bar, C-List/D-List toolbars, roots toolbar, and mid-bar.
- Selected nodes pulse with a bright glow.
- **Mutually exclusive selection:** Only one kind of node can be selected at a time. Clicking a C-node clears any D-node and root selections; clicking a D-node clears C-node and root selections; clicking a root clears C-node and D-node selections.
- The **Ops** label in the mid-bar shows a colored indicator (`#ops-target`) reflecting the active selection kind:
  - **C** (green `#5ddb6a`) -- C-coefficients selected
  - **D** (blue `#4ea8de`) -- D-nodes selected
  - **roots** (red `#e94560`) -- roots selected
  - **none** (grey `#888`) -- nothing selected
- The trajectory editor displays the first selected coefficient's settings (by index). Adjusting controls shows a live preview; click **Update Whole Selection** to commit or click away to revert.
- Selection count appears in tab headers: green for coefficients (`#coeff-sel-count` in C-Nodes tab), D-nodes (`#morph-sel-count` in D-Nodes tab), and jiggle targets (`#jiggle-sel-count` in Jiggle tab); red for roots (`#roots-sel-count`).

---

## Recording

| Control | Description |
|---------|-------------|
| **Record** button (`#rec-roots-btn`) | Start WebM video capture. |
| **Stop** button (`#stop-roots-btn`) | Stop recording. |
| **Mode selector** (`#rec-mode`) | Roots, Coeffs, Both, Stats, Sound, Bitmap, or Full. "Both" is tab-aware -- renders coefficients plus the active right-side tab. |
| **Auto-stop** | When Trails are enabled, recording auto-stops on loop completion. |

---

## Keyboard

| Key | Action |
|-----|--------|
| **Escape** | Cascading close: (1) revert animation bar preview, (2) close C-node context menu, (3) close D-node context menu, (4) close root context menu, (5) close C-D path popup (reverts changes), (6) close ops tool popover, (7) close audio popup, (8) close color picker, (9) close coefficient picker, (10) close snap popup, (11) close bitmap cfg, (12) close path picker, (13) deselect all. Stops at the first match. |

---

## Regenerating Screenshots

The screenshots in this guide were captured programmatically using Playwright. To regenerate them:

```bash
python docs/capture_interface.py
```
