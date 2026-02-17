# Tutorial: Capturing Root Trajectories as PNG

This step-by-step guide walks you through creating an animated polynomial, recording its root trajectories, and saving the result as a high-resolution PNG image.

**Time:** ~5 minutes | **Difficulty:** Beginner | **Prerequisites:** A modern browser (Chrome, Firefox, Edge)

---

## 1. Open the App

Open [index.html](../index.html) in your browser, or visit the [live demo](https://nassuphis.github.io/karpo_hackathon/). You'll see the default view: a degree-5 polynomial with coefficients on the left panel and roots on the right.

![Initial app state](images/01_initial.png)

The **left panel** shows the polynomial's coefficients as colored dots in the complex plane. Each dot can be dragged. The **right panel** shows the computed roots. Between them, the **mid-bar** has transform tools (scale, rotate, translate).

---

## 2. Choose a Starting Pattern

Click the **Pattern** dropdown in the header bar and select **Spiral**. This arranges the coefficients in a spiral layout, which produces more interesting root dynamics than the default circle.

![Spiral pattern selected](images/02_spiral_pattern.png)

The coefficients reposition themselves and the roots on the right update instantly. PolyPaint includes 26 patterns across three categories (Basic, Coefficient shapes, Root shapes) -- feel free to explore them later.

---

## 3. Select All Coefficients

Click the **All** button in the trajectory editor bar (just below the tab row, on the left). All 6 coefficient dots light up with a selection glow, and the label shows **6 coeffs**.

![All coefficients selected](images/03_select_all.png)

You can also click individual dots to toggle them, or drag on empty canvas to marquee-select a group. For this tutorial, we want all of them animated.

---

## 4. Assign Circle Paths

With all coefficients selected, open the **path type dropdown** (bottom-left, currently showing "Circle" or "None") and select **Circle**. Colored circle paths appear around each coefficient, showing the trajectory it will follow during animation.

![Circle paths assigned](images/04_circle_path.png)

Each coefficient now has its own circular orbit. The sliders to the right of the dropdown control **S** (speed, 1–1000), **R** (radius), and **A** (starting angle). You can adjust these per-coefficient or for the whole selection at once.

---

## 5. Set Prime Speeds

To make the animation more interesting, give each coefficient a different speed so the roots trace complex, non-repeating patterns.

1. Click the **C-List** tab to switch to the table view
2. Click **All** in the list header
3. Open the **Transform** dropdown and choose **PrimeSpeeds**

This sets each coefficient's speed to a value coprime with all others, ensuring their orbits never synchronize and the root trajectories fill more of the plane.

Switch back to the **C-Nodes** tab to see the coefficient paths.

![Prime speeds applied](images/05_prime_speeds.png)

---

## 6. Enable Trails

Click the **Trails** button on the roots toolbar (top of the right panel). It highlights when active. This tells PolyPaint to record root positions as SVG paths as the animation plays.

![Trails enabled](images/06_trails_on.png)

Nothing visible changes yet -- trails appear once the animation starts.

---

## 7. Play the Animation

Click the **Play** button in the header bar (right side, next to the scrub slider). The coefficients begin orbiting along their circle paths, and the roots respond by tracing colored trails on the right panel. The seconds counter next to the Play button shows elapsed animation time.

![Trails after 3 seconds](images/07_trails_running.png)

Let it run for several seconds. The trails grow into intricate braid-like patterns as roots chase each other around the plane. The pattern depends on the degree, path types, speeds, and radii.

![Trails after 6 seconds](images/08_trails_complete.png)

Click **Pause** when you're happy with the trail pattern. You can click **Resume** to continue from where you paused, or **Home** to reset. Trails collect up to the maximum trail point limit.

---

## 8. Export as PNG (Quick Method)

For a quick screenshot of the current view:

1. Click the **Export** button (down arrow) in the header bar
2. A popup appears with 7 capture modes

![Export popup](images/09_export_popup.png)

| Mode | What it captures |
|------|-----------------|
| **Both** | Coefficients + active right tab side-by-side |
| **Coeffs** | Left panel only |
| **Roots** | Right panel only (includes trails) |
| **Stats** | Stats dashboard |
| **Sound** | Sound panel |
| **Bitmap** | Bitmap canvas |
| **Full** | Entire app window |

Click **Roots** to save just the root trails as a PNG. The file downloads automatically along with a JSON snapshot of the full app state (so you can recreate this exact setup later).

---

## 9. High-Resolution Bitmap Render

The Export method above captures the screen at display resolution. For publication-quality images at up to 25,000 x 25,000 pixels, use the **Bitmap** tab.

### Initialize

1. Click the **Bitmap** tab on the right panel
2. Click **init** to snapshot the current animation state and create the bitmap canvas

![Bitmap tab](images/10_bitmap_tab.png)

![Bitmap initialized](images/11_bitmap_init.png)

The bitmap canvas appears as a black rectangle. The toolbar shows resolution (default 2000px), step count, and start/pause controls. You can change the resolution dropdown before or after init -- options range from 1,000 to 25,000 pixels. Changing the resolution or step count while fast mode is running will automatically restart rendering with the new settings.

### Render

Click **start** to begin fast-mode rendering. This launches parallel Web Workers that continuously solve the polynomial with the coefficients advancing along their paths, plotting each root position as a pixel on the bitmap.

![Bitmap rendering in progress](images/12_bitmap_rendering.png)

The elapsed counter ticks up as pixels accumulate. Let it run for 5-30 seconds depending on the resolution and level of detail you want. Higher step counts (configurable via the **steps** dropdown) mean more solver iterations per worker pass.

Click **pause** to halt rendering. The accumulated image is preserved. You can click **cont** (continue) to resume accumulating more pixels.

![Bitmap render complete](images/13_bitmap_done.png)

### Save

Click **save** to open the format popup:

![Bitmap save popup](images/14_bitmap_save.png)

| Format | Best for |
|--------|---------|
| **PNG** | Lossless, often smallest for sparse bitmaps |
| **JPEG** | Lossy with quality slider (1-100) |
| **BMP** | Uncompressed, universal compatibility |
| **TIFF** | Lossless, print workflows |

Select your format and click **Download**. The image is exported at full compute resolution, not the display-capped resolution.

---

## 10. Configure Bitmap Colors and Solver

Click **cfg** on the bitmap toolbar to open the configuration popup. This controls the solver engine, background color, jiggle perturbation, and root coloring.

![Bitmap configuration popup](images/15_bitmap_cfg.png)

### Color Modes

| Mode | Effect |
|------|--------|
| **Uniform** | All roots plot in a single color (pick from 8 swatches) |
| **Index Rainbow** | Each root gets a distinct rainbow color by index |
| **Derivative** | Colors by Jacobian sensitivity: blue (stable) to red (volatile) |
| **Root Proximity** | Colors by distance to nearest other root, using a selectable palette |

For Index Rainbow mode, you can also choose a **matching strategy** (Hungarian, Greedy x1, Greedy x4) that determines how root identities are tracked between solver steps.

### Jiggle

Jiggle adds small perturbations to coefficients between animation cycles, causing the root trajectories to explore nearby parameter space. This fills in sparse areas and creates richer textures. Try **Random** or **Spiral** jiggle with a small sigma. Key parameters: **sigma** (0–10, perturbation magnitude as a fraction of coefficient extent divided by 10), **interval** (0.1–100 seconds between perturbation cycles), and **angle/circle steps** (10–5000, controlling rotational resolution for directional jiggle modes).

---

## Bonus: Domain Coloring

Switch back to the **Roots** tab and click the **domain coloring** button (half-circle icon) to overlay an HSL-mapped visualization of the polynomial landscape on the roots panel background.

![Domain coloring enabled](images/16_domain_coloring.png)

Hue represents the argument (phase) of p(z) and brightness represents the magnitude. Roots appear as points where all colors converge. This provides geometric intuition for why roots move the way they do when coefficients change.

---

## Tips

- **Higher degree = richer patterns.** Try degree 10-15 with the Spiral pattern and prime speeds for elaborate braids.
- **Mix path types.** Not every coefficient needs a circle -- try setting some to Lissajous, Figure-8, or Hilbert for more complex dynamics.
- **Use D-nodes for morphing.** The D-List tab defines morph-target coefficients. D-nodes support a **Follow C** path type that mirrors the corresponding C-node's position, making it easy to set up morph targets that track the main coefficients.
- **Use the scrub slider** (in the header bar) to preview the animation before committing to a long bitmap render. The scrub is additive -- it adds seconds to the current elapsed time when paused.
- **PNG is usually smallest** for bitmap exports of sparse root trajectories (a 15K render can be 4.5 MB as PNG vs 35 MB as JPEG).
- **Save your state** (floppy disk icon) before experimenting -- the JSON snapshot lets you return to any setup instantly.

---

## Regenerating Screenshots

The screenshots in this tutorial were captured programmatically using Playwright. To regenerate them:

```bash
python docs/capture_tutorial.py
```

This runs a headless Chromium instance through all the tutorial steps and saves updated PNGs to `docs/images/`.
