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
| **Degree** | Click the number to open a slider popover (3-30). Reinitializes coefficients on change. |
| **Pattern** | Initial arrangement of coefficients or roots. 26 patterns in 3 categories: Basic, Coefficient shapes, Root shapes. See [Patterns](patterns.md). |
| **Reset** (&#8634;) | Reset to initial state for the current degree and pattern. |
| **Save** | Save full application state as a JSON file. |
| **Load** | Load a previously saved JSON snapshot. |
| **Export** (&#11015;) | Opens a popup with 7 export modes (see below). |
| **T** | Timing stats: steps/sec, pass times, composite breakdown. Clears when worker count changes. |

### Export Popup

![Export popup](images/iface_export_popup.png)

Seven capture modes: Both, Coeffs, Roots, Stats, Sound, Bitmap, Full. Each downloads a PNG screenshot plus a JSON snapshot of the full app state.

---

## Left Panel

The left panel has four tabs and a shared animation bar.

### Coefficients Tab

![Coefficients tab with circle paths](images/iface_coeffs_paths.png)

Interactive SVG complex-plane visualization of polynomial coefficients.

- **Drag** any coefficient dot to move it; roots update instantly on the right panel.
- **Right-click empty canvas** to add a new coefficient at that position (becomes the new highest-power term).
- **Right-click an existing coefficient** to open a context menu with trajectory settings, live preview, and a Delete button. Click "Accept" to commit or press Escape to revert.
- Assigned trajectory curves are always visible as colored paths on the canvas.

### Trajectory Editor

![Trajectory editor / animation bar](images/iface_anim_bar.png)

Located below the Coefficients tab bar. Controls the animation path for selected coefficients.

**First row:** Coefficient picker, Select All / Deselect, selection label, **Update Sel** button.

**Second row:** Path type dropdown and path-specific controls. Sliders change based on the chosen path type (e.g. R/S/A/CW-CCW for circles, S/sigma for Gaussian). A **PS** (Prime Speed) button appears when the path has a speed parameter -- it sets the speed to the nearest value coprime with all other animated coefficients. Controls dim when no coefficients are selected.

#### Animation Bar

| Control | Description |
|---------|-------------|
| **Scrub slider** | Drag to manually advance C and D coefficients along their paths (0-5 seconds). |
| **Play / Pause** | Start or pause animation of all coefficients with assigned trajectories. |
| **Home** | Return all animated C and D nodes to their start positions (curve[0]). |

#### Path Types (21)

| Group | Paths |
|-------|-------|
| **Basic** | None, Circle, Horizontal, Vertical, Spiral, Random (Gaussian cloud) |
| **Curves** | Lissajous, Figure-8, Cardioid, Astroid, Deltoid, Rose, Epitrochoid, Hypotrochoid, Butterfly, Star, Square, C-Ellipse |
| **Space-filling** | Hilbert (Moore curve), Peano, Sierpinski arrowhead |

Each coefficient stores its own path type, radius, speed, angle, and direction independently. See [Paths](paths.md) for curve formulas, cycle sync, and space-filling curve details.

#### Trajectory Workflow

1. Select one or more coefficients (click, marquee, or Select All)
2. Choose a path type, adjust controls in the trajectory editor
3. Settings apply immediately to all selected coefficients (or click **Update Sel**)
4. Press **Play** -- all coefficients with a trajectory animate simultaneously

### List Tab

![List tab](images/iface_list_tab.png)

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

#### List Curve Editor

![List curve editor](images/iface_list_editor.png)

The curve editor at the top of the List tab lets you edit the path for the selected coefficient. Navigate between coefficients with the arrow buttons, change path type, and adjust parameters.

#### Transform dropdown (21 bulk operations)

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

**Param1 / Param2** sliders supply arguments to transforms that need them (e.g. Lerp endpoints, rotation angle).

### D-List Tab

![D-List tab](images/iface_dlist_tab.png)

Identical structure to the List tab, but for morph target D-nodes. Assign paths, speeds, and transforms to D-nodes independently from C-coefficients. See [D-Node Paths](d-node-paths.md).

### Morph Tab

![Morph tab](images/iface_morph_tab.png)

Interactive morph panel with C/D coefficient visualization and blending controls. See [Morph](morph.md).

- **Copy C to D** / **Swap C and D** buttons
- Blend rate control
- D-node dots with drag interaction

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
| **Resolution** | Canvas size: 1000 / 2000 / 5000 / 8000 / 10000 / 15000 / 25000 px. Above 2000px, display is capped at 2000px while computation runs at full resolution. |
| **start / stop** | Toggle continuous fast mode (parallel Web Workers). Stop preserves state; start resumes where it left off. |
| **ROOT / COEF** | Toggle between plotting root or coefficient positions. |
| **Steps** | Solver steps per pass: 10 / 100 / 1K / 5K / 10K / 50K / 100K / 1M. |
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
| **Background color** | 24 preset colors (darks, lights, grays). |
| **Jiggle** | 10 perturbation modes (see below). Mode-specific parameter sliders, interval control, select-all toggle. |
| **Root color** | 4 bitmap color modes (see below). Independent from animation root coloring. |

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

**Jiggle modes** (10):

| Mode | Description | Parameters |
|------|-------------|------------|
| None | No perturbation | -- |
| Random | Gaussian offsets each trigger | sigma |
| Rotate | Rotate selected around centroid | theta (turns) |
| Walk | Random walk accumulating offsets | sigma |
| Scale | Scale from centroid | % per trigger |
| Circle | Rotate around origin | theta (turns) |
| Spiral | Rotate + scale around centroid | theta, % |
| Breathe | Sinusoidal scaling from centroid | amplitude, period |
| Wobble | Sinusoidal rotation around centroid | amplitude, period |
| Lissajous | Translate along Lissajous figure | amplitude, period, freqX, freqY |

See [Paths](paths.md) for jiggle formulas.

---

## Selection Model

![Selection with glow](images/iface_selection.png)

- **Click** any dot to toggle it into the selection.
- **Marquee select:** Click and drag on empty canvas to draw a selection rectangle.
- **Select All / Deselect** buttons in both the trajectory editor bar and roots toolbar.
- Selected nodes pulse with a bright glow.
- Clicking a coefficient clears any root selection and vice versa.
- The trajectory editor displays the last-selected coefficient's settings. Adjusting any control applies immediately to all selected coefficients.
- Selection count appears in panel headers: green for coefficients, red for roots.

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
