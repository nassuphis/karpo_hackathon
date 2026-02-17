# Interface Guide

Complete control reference for PolyPaint. For a quick overview, see the [README](../README.md#interface-overview).

## Layout Overview

![Full app layout](images/iface_layout.png)

The UI is organized around a compact header bar and two side-by-side complex-plane panels (left: coefficients, right: roots) with an operations mid-bar between them.

---

## Header Bar

![Header bar](images/iface_header.png)

| Control | Description |
|---------|-------------|
| **Degree** | Click the number to open a slider popover (2-30). Reinitializes coefficients on change. |
| **Pattern** | Initial arrangement of coefficients or roots. 26 patterns in 3 categories: Basic, Coefficient shapes, Root shapes. See [Patterns](patterns.md). |
| **Reset** (&#8634;) | Reset to initial state for the current degree and pattern. |
| **Save** | Save full application state as a JSON file. |
| **Load** | Load a previously saved JSON snapshot. |
| **Export** (&#11015;) | Opens a popup with 7 export modes (see below). |
| **T** | Timing stats: steps/sec, pass times, composite breakdown. Clears when worker count changes. |
| **Scrub slider** | Additive scrubber: drag to add seconds to the current elapsed time (only works when animation is paused). Resets to zero on release. |
| **Play / Pause / Resume** | 3-state cycle: Play starts from zero, Pause stops mid-animation, Resume continues from where it left off. |
| **Seconds counter** | Displays current animation elapsed time (e.g. `3.14s`). Updates during playback and scrubbing. |
| **Home** | Return all animated C and D nodes to their start positions (curve[0]) and reset elapsed to 0. |

### Export Popup

![Export popup](images/iface_export_popup.png)

Seven capture modes: Both, Coeffs, Roots, Stats, Sound, Bitmap, Full. Each downloads a PNG screenshot plus a JSON snapshot of the full app state.

---

## Left Panel

The left panel has six tabs: C-Nodes, C-List, D-Nodes, D-List, Jiggle, and Final.

### C-Nodes Tab

![C-Nodes tab with circle paths](images/iface_coeffs_paths.png)

Interactive SVG complex-plane visualization of polynomial coefficients.

- **Drag** any coefficient dot to move it; roots update instantly on the right panel.
- **Right-click empty canvas** to add a new coefficient at that position (becomes the new highest-power term).
- **Right-click an existing coefficient** to open a context menu with trajectory settings, live preview, and a Delete button (disabled when only 3 coefficients remain, since minimum degree is 2). Click "Accept" to commit or press Escape to revert.
- Assigned trajectory curves are always visible as colored paths on the canvas.

### Trajectory Editor

![Trajectory editor / animation bar](images/iface_anim_bar.png)

Located at the top of the C-Nodes tab. Controls the animation path for selected coefficients.

**First row:** Coefficient picker, Select All / Deselect, selection label, **Update Whole Selection** button.

**Second row:** Path type dropdown and path-specific controls. Sliders change based on the chosen path type (e.g. R/S/A/CW-CCW for circles, S/sigma for Gaussian). Adjusting controls shows a live preview of the proposed paths. Click away or press Escape to revert; click **Update Whole Selection** to commit. Controls dim when no coefficients are selected.

#### Preview / Revert / Commit Pattern

The trajectory editor uses a preview/revert workflow. When you change the path dropdown or adjust a slider, the changes are shown as a live preview on the canvas (backed by `barSnapshots` which stores the original state). You can:

- Click **Update Whole Selection** to commit the previewed changes (`commitBarPreview()`)
- Click outside the editor or press **Escape** to revert to the original state (`revertBarPreview()`)

This replaces the previous immediate-apply behavior, giving you a chance to experiment without permanently changing paths.

#### Path Types (21 for C-nodes, 22 for D-nodes)

| Group | Paths |
|-------|-------|
| **Basic** | None, Circle, Horizontal, Vertical, Spiral, Random (Gaussian cloud) |
| **Curves** | Lissajous, Figure-8, Cardioid, Astroid, Deltoid, Rose, Epitrochoid, Hypotrochoid, Butterfly, Star, Square, C-Ellipse |
| **Space-filling** | Hilbert (Moore curve), Peano, Sierpinski arrowhead |
| **D-only** | Follow C (mirrors the corresponding C-node position; no speed/radius parameters) |

Each coefficient stores its own path type, radius, speed, angle, and direction independently. See [Paths](paths.md) for curve formulas, cycle sync, and space-filling curve details.

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
| Path | Animation path type (or dash if none) |
| Speed | Animation speed (or dash) |
| Radius | Path radius (or dash) |
| Curve length | Number of sample points |
| Curve index | Current position along path |
| Coordinates | Complex value (re + im*i) |

**Toolbar:** Select All / Deselect, **Same Curve** (select all coefficients with the displayed curve type), curve type cycler (prev/next arrows to cycle through path types present in the polynomial), selection count, Transform dropdown, Param1/Param2 sliders.

#### C-List Curve Editor

![C-List curve editor](images/iface_list_editor.png)

The curve editor below the toolbar lets you edit the path for the selection. It shows controls based on the first selected coefficient's current path type. Choose a path type, adjust parameters, and click **Update Whole Selection** to apply to all selected coefficients. Controls dim when nothing is selected.

#### Transform dropdown (20 bulk operations)

| Transform | Description |
|-----------|-------------|
| PrimeSpeeds | Set all speeds coprime with each other |
| Set All Speeds | Set selected to Param1 speed |
| RandomSpeed | Random speed for each selected |
| RandomAngle | Random starting angle |
| RandomRadius | Random radius |
| Lerp Speed | Interpolate speeds from Param1 to Param2 |
| Lerp Radius | Interpolate radii |
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

Interactive SVG complex-plane visualization of morph target D-nodes. See [Morph](morph.md).

- **Copy C->D** / **Swap C<->D** buttons in the toolbar
- D-node dots with drag interaction (same as coefficient panel)
- Selection count shown in the tab header

### D-List Tab

![D-List tab](images/iface_dlist_tab.png)

Identical structure to the C-List tab, but for morph target D-nodes. Assign paths, speeds, and transforms to D-nodes independently from C-coefficients. The toolbar has the same controls: Select All / Deselect, Same Curve, curve type cycler, Transform dropdown, and Param1/Param2 sliders. The curve editor uses the same pattern: select D-nodes, adjust parameters, and click **Update Whole Selection** to apply. D-nodes have an additional **Follow C** path type that mirrors the corresponding C-node's position (no speed or radius displayed). See [D-Node Paths](d-node-paths.md).

### Jiggle Tab

Dedicated tab for coefficient perturbation controls (previously embedded in the bitmap cfg popup).

- **Mode** dropdown (12 modes, see below)
- Mode-specific parameter controls (sigma, steps, amplitude, period, etc.)
- **Interval** slider (0.1-100 seconds, step 0.1, between perturbation triggers) with +/-0.1s precision buttons
- **GCD** button: auto-compute interval from the GCD of coefficient speeds

**Jiggle modes** (12):

| Mode | Description | Parameters |
|------|-------------|------------|
| None | No perturbation | -- |
| Random | Gaussian offsets each trigger | sigma (range slider 0-10, +/-0.01 buttons) |
| Rotate | Rotate selected around centroid | steps (range slider 10-5000) |
| Walk | Random walk accumulating offsets | sigma (range slider 0-10, +/-0.01 buttons) |
| Scale (center) | Scale from origin | growth % per trigger |
| Scale (centroid) | Scale from centroid | growth % per trigger |
| Circle | Rotate around origin | steps (range slider 10-5000) |
| Spiral (centroid) | Rotate + scale around centroid | steps (10-5000), growth % |
| Spiral (center) | Rotate + scale around origin | steps (10-5000), growth % |
| Breathe | Sinusoidal scaling from centroid | amplitude, period (cycles) |
| Wobble | Sinusoidal rotation around centroid | steps (10-5000), period (cycles) |
| Lissajous | Translate along Lissajous figure | amplitude, steps (range slider 10-5000), freqX, freqY |

See [Paths](paths.md) for jiggle formulas.

### Final Tab

![Final tab](images/iface_morph_tab.png)

Shows the actual coefficients sent to the solver (blended C/D positions when morphing is enabled).

- **Morph** checkbox: enable/disable morph blending
- **Rate** slider (0.01-2.00 Hz): morph oscillation frequency
- **mu** display: current blend parameter (0=C, 1=D)
- SVG complex-plane visualization of the final blended coefficients

---

## Mid-bar Operations

![Mid-bar operations](images/iface_midbar.png)

Located between the left and right panels. Buttons brighten when a selection exists.

| Tool | Control | Range |
|------|---------|-------|
| **Scale** (&#8661;) | Vertical slider, exponential mapping | 0.1x - 10x |
| **Rotate** (&#10226;) | Horizontal slider | +/-0.5 turns |
| **Translate** (&#10011;) | 2D vector pad | +/-2 in each axis |

Each tool opens a transient popover with live preview. Drag to apply; changes are immediate. Close by clicking outside or pressing Escape.

A colored label below shows **coeffs** (green) or **roots** (red) to indicate the target.

Additional controls:
- **Select All / Deselect** -- selects roots if any root is selected, otherwise coefficients
- **Inv** -- reverse coefficient order (reflects roots around the unit circle)

---

## Right Panel

### Roots Tab

![Roots tab with toolbar](images/iface_roots_tab.png)

Interactive SVG complex-plane visualization of polynomial roots.

**Roots toolbar** (overlay on canvas):

![Roots toolbar](images/iface_roots_toolbar.png)

| Control | Description |
|---------|-------------|
| **Trails** | Toggle root trail recording. Loop detection auto-stops after one full cycle. Jump detection breaks trails at root-identity swaps (>30% of visible range). |
| **Color** | Opens a root coloring popover. Controls animation dots only (bitmap coloring is independent). |
| **Domain** (&#9680;) | Toggle domain coloring on the roots canvas background. HSL-mapped: hue = argument, saturation = magnitude. |
| **Fit** | Auto-zoom to fit all roots and trails. |
| **+25%** | Zoom out by 25%. |
| **Select All / Deselect** | Select or deselect all roots. |

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

Three independent audio layers, each with a toggle button and config popover. See [Sonification](sonification.md) for the full algorithm, audio graph, and signal routing matrix.

| Voice | Type | Key Parameters |
|-------|------|---------------|
| **B** Base | FM drone | Pitch (55-440 Hz), Range, FM Ratio, FM Depth, Brightness, Volume, Vibrato |
| **M** Melody | Pentatonic arpeggiator | Rate (2-60 notes/s), Cutoff, Volume, Attack, Decay, Brightness |
| **V** Voice | Encounter beeps | Cooldown (10-500 ms), Volume, Attack, Decay |

The Sound tab also exposes a **signal routing matrix**: any of 25 computed stats can be patched into any of 14 audio parameters, with per-route normalization (Fixed or adaptive RunMax) and EMA smoothing.

### Bitmap Tab

![Bitmap tab](images/iface_bitmap_tab.png)

Accumulates root (or coefficient) positions as single-pixel stamps on a high-resolution canvas.

**Toolbar:**

![Bitmap toolbar](images/iface_bitmap_toolbar.png)

| Control | Description |
|---------|-------------|
| **init** | Snapshot animation state, create bitmap canvas, reset elapsed to 0. |
| **save** | Open format popup: JPEG (with quality slider), PNG, BMP, TIFF. Downloads from CPU buffer at full resolution. |
| **clear** | Reset canvas pixels. Elapsed time unchanged. |
| **Resolution** | Canvas size: 1000 / 2000 / 5000 / 8000 / 10000 / 15000 / 25000 px. Above 2000px, display is capped at 2000px while computation runs at full resolution. Auto-restarts fast mode if changed during active rendering. |
| **start / stop** | Toggle continuous fast mode (parallel Web Workers). Stop preserves state; start resumes where it left off. |
| **ROOT / COEF** | Toggle between plotting root or coefficient positions. |
| **Steps** | Solver steps per pass: 10 / 100 / 1K / 5K / 10K / 50K / 100K / 1M. Auto-restarts fast mode if changed during active rendering. |
| **cfg** | Open configuration popup (see below). |

A zero-padded elapsed-seconds counter appears during computation.

#### Bitmap Rendering

![Bitmap after rendering](images/iface_bitmap_rendered.png)

After clicking **init** and **start**, parallel Web Workers continuously solve the polynomial and plot root positions as pixels on the bitmap canvas.

#### Bitmap Save Popup

![Bitmap save popup](images/iface_bitmap_save.png)

Four export formats: JPEG (with quality slider), PNG, BMP, TIFF. The image is exported at full compute resolution, not the display-capped resolution.

#### Configuration Popup (cfg)

![Bitmap configuration popup](images/iface_bitmap_cfg.png)

| Setting | Options |
|---------|---------|
| **Solver engine** | JavaScript or WebAssembly. See [WASM](wasm_investigation.md). |
| **Workers** | Number of parallel Web Workers: 1, 2, 4, 8, 16. |
| **Background color** | 24 preset colors (darks, lights, grays). |
| **Root color** | 4 bitmap color modes (see below). Independent from animation root coloring. |

Jiggle controls have moved to the dedicated [Jiggle tab](#jiggle-tab) in the left panel.

**Bitmap color modes** (4):

| Mode | Description | Palette |
|------|-------------|---------|
| Uniform | Single fixed color | 8-color swatch |
| Index Rainbow | Color by root index | d3 rainbow spectrum |
| Derivative | Jacobian sensitivity | 16-entry blue-white-red |
| Root Proximity | Min distance to nearest root | 16-entry sequential palette from catalog (8 options) |

**Proximity palette catalog** (Root Proximity mode): Inferno (default), Viridis, Magma, Plasma, Turbo, Cividis, Warm, Cool. Each renders as a gradient circle in the cfg popup; clicking selects the palette and switches to Proximity mode.

**Root-matching strategies** (Index Rainbow mode only):

| Strategy | Algorithm | Cost | Accuracy |
|----------|-----------|------|----------|
| Hungarian | Kuhn-Munkres optimal assignment | O(n^3) every step | Perfect |
| Greedy x1 | Nearest-neighbor | O(n^2) every step | Good |
| Greedy x4 | Nearest-neighbor every 4th step | O(n^2) / 4 | Slight drift (default) |

---

## Selection Model

![Selection with glow](images/iface_selection.png)

- **Click** any dot to toggle it into the selection.
- **Marquee select:** Click and drag on empty canvas to draw a selection rectangle.
- **Select All / Deselect** buttons in both the trajectory editor bar and roots toolbar.
- Selected nodes pulse with a bright glow.
- Clicking a coefficient clears any root selection and vice versa.
- The trajectory editor displays the first selected coefficient's settings (by index). Adjusting controls shows a live preview; click **Update Whole Selection** to commit or click away to revert.
- Selection count appears in tab headers: green for coefficients (C-Nodes tab), D-nodes (D-Nodes tab), and jiggle targets (Jiggle tab); red for roots.

---

## Recording

| Control | Description |
|---------|-------------|
| **Record** button | Start WebM video capture. |
| **Mode selector** | Roots, Coeffs, Both, Stats, Sound, Bitmap, or Full. "Both" is tab-aware -- renders coefficients plus the active right-side tab. |
| **Auto-stop** | When Trails are enabled, recording auto-stops on loop completion. |

---

## Keyboard

| Key | Action |
|-----|--------|
| **Escape** | Close any open popup/popover. If none open, deselect all. Cascading: context menu, tool popover, audio popup, color picker, coefficient picker, snap popup, bitmap cfg, path picker. |

---

## Regenerating Screenshots

The screenshots in this guide were captured programmatically using Playwright. To regenerate them:

```bash
python docs/capture_interface.py
```
