# PolyPaint

Interactive polynomial root visualizer. Drag coefficients in the complex plane and watch the roots move in real time.

A degree-*n* polynomial has *n*+1 complex coefficients and *n* complex roots. PolyPaint gives you two side-by-side complex-plane panels: the left shows the coefficients as draggable dots, the right shows the roots computed on the fly. As you drag any coefficient, the roots flow continuously across the plane, leaving colored trails behind them.

## Quick Start

```bash
uv run uvicorn server:app --reload --port 8000
```

Open [http://localhost:8000](http://localhost:8000).

> `uv run` automatically creates a virtual environment, installs all dependencies from `pyproject.toml`, and runs the command. No manual setup required.

## Architecture

```
Browser (d3.js v7)  ←— WebSocket (JSON) —→  FastAPI (Python)
                                                  ↓
                                             numpy.roots()
```

| Component | Technology | Why |
|-----------|-----------|-----|
| Frontend | Single HTML file, d3.js v7 (CDN) | No build step, immediate rendering |
| Transport | WebSocket | Persistent connection avoids per-request overhead at 60Hz drag rate |
| Backend | FastAPI + uvicorn | Async WebSocket support, minimal boilerplate |
| Root solver | `numpy.roots()` | Eigenvalue decomposition of companion matrix, sub-millisecond even at degree 30 |

### Data Flow

1. User drags a coefficient dot in the left panel
2. `mousemove` fires, updating the coefficient's complex value
3. `requestAnimationFrame` throttles WebSocket sends to ~60 msg/sec
4. Server receives `{"coefficients": [[re, im], ...]}` (descending degree order)
5. Server strips leading near-zero coefficients, calls `numpy.roots()`, filters NaN/Inf
6. Server returns `{"roots": [[re, im], ...], "error": null}`
7. Client matches incoming roots to previous frame (greedy nearest-neighbor) for trail coherence
8. Client appends new positions to trail history, redraws SVG paths and dots

## Controls

| Control | Description |
|---------|-------------|
| **Degree slider** (3–30) | Number of polynomial roots. Changing it reinitializes coefficients and clears trails. |
| **Pattern dropdown** | Initial arrangement of coefficients (or roots). See [Patterns](#patterns) below. |
| **Spread slider** (0.2–2.5) | Scales the initial pattern size. |
| **Domain coloring** checkbox | Toggles the domain coloring background on the roots panel. See [Domain Coloring](#domain-coloring) below. |
| **Reset Trails** button | Clears all trail paths and resets the roots panel zoom. Does not move coefficients. |

## Panels

### Left Panel — Coefficients

- Each coefficient c₀, c₁, ..., cₙ is a colored circle at its position in the complex plane
- **Drag any dot** to change that coefficient's value
- Colors follow a rainbow gradient: `d3.interpolateRainbow(i/n)`
- Labels (c₀, c₁, ...) appear next to each dot
- **Auto-scaling**: the viewport grows when you drag a coefficient toward the edge, and shrinks when coefficients return closer to the origin
- Hover tooltip shows exact complex value (e.g. `0.500 + 0.866i`)

### Right Panel — Roots

- Non-draggable dots showing the polynomial's roots, recomputed on every coefficient change
- Colors use `d3.interpolateSinebow` to distinguish root identities
- **Trails**: each root leaves a colored path behind it as it moves, showing the root locus
- **Auto-scaling (zoom-out only)**: the viewport expands to fit roots that venture far from the origin but **never zooms back in** — this preserves visibility of the full trail history. Zoom resets only when trails are cleared.

### Shared Visual Elements

- Complex-plane grid with integer gridlines
- Real (Re) and Imaginary (Im) axis labels
- Dashed unit circle for reference
- Dark theme: background `#1a1a2e`, panels `#16213e`

## Domain Coloring

When enabled (on by default), the roots panel background is painted using [domain coloring](https://en.wikipedia.org/wiki/Domain_coloring) — a standard technique for visualizing complex functions. For each pixel at position z in the complex plane, the polynomial p(z) is evaluated and mapped to a color:

- **Hue** = arg(p(z)) — the phase/argument of the polynomial's value. A full rotation through the color wheel (red → yellow → green → cyan → blue → magenta → red) represents a full 2π cycle of the argument. Roots of the polynomial appear as points where all colors converge, since arg(p(z)) cycles through all values as you orbit a zero.
- **Lightness** = modulated by log|p(z)| using the formula `0.5 + 0.4 * cos(2π * frac(log₂|p(z)|))`. This creates **contour lines** at powers of 2 in the modulus, making the magnitude structure visible. Zeros appear as dark points.
- **Saturation** = fixed at 0.8.

The polynomial is evaluated using [Horner's method](https://en.wikipedia.org/wiki/Horner%27s_method) for numerical stability and speed. The canvas renders at half resolution (~62k complex polynomial evaluations per frame) and is CSS-scaled to full size, keeping the visualization smooth at 60fps even at degree 30.

The domain coloring updates in real time as you drag coefficients, giving an immediate visual sense of how the polynomial's complex landscape shifts.

## Patterns

### Basic (5)

Initialize coefficients in simple geometric arrangements:

| Pattern | Description |
|---------|-------------|
| **Circle** | Evenly spaced on a circle of radius `spread` |
| **Real axis** | Evenly spaced along the real axis |
| **Imaginary axis** | Evenly spaced along the imaginary axis |
| **Grid** | Square grid arrangement |
| **Random** | Uniformly random in `[-spread, spread]²` |

### Coefficient Patterns (8)

More complex coefficient arrangements that produce interesting root behaviors:

| Pattern | Description |
|---------|-------------|
| **Spiral** | Archimedean spiral — roots trace beautiful curves when perturbed |
| **Star** | Alternating inner/outer radii — gives spiky root patterns |
| **Figure-8** | Bernoulli lemniscate — rich root sensitivity |
| **Conjugate pairs** | Pairs straddling the real axis — roots bifurcate dramatically |
| **Two clusters** | Two separate clusters — roots jump between them |
| **Geometric decay** | Alternating-sign geometric series — classic ill-conditioned setup |
| **Rose curve** | 3-petal rose `r = cos(3θ)` — symmetry-breaking effects |
| **Cardioid** | Heart-shaped curve — roots respond dramatically near the cusp |

### Root Shapes (12)

These define the *roots* in a specific shape, then compute the coefficients by expanding the product (z − r₀)(z − r₁)···(z − rₙ₋₁). Dragging the resulting coefficients perturbs the roots away from the initial shape:

| Pattern | Description |
|---------|-------------|
| **Heart** | Parametric heart curve (sin³ form) |
| **Circle** | Roots of unity (evenly on unit circle) |
| **Star** | Alternating inner/outer radii |
| **Spiral** | Archimedean spiral with increasing radius |
| **Cross** | Plus (+) shape along both axes |
| **Diamond** | L¹ unit ball: \|x\| + \|y\| = spread |
| **Chessboard** | Only "black squares" of a grid |
| **Smiley** | Two eyes (small circles) + smile arc |
| **Figure-8** | Bernoulli lemniscate |
| **Butterfly** | Butterfly polar curve |
| **Trefoil** | 3-leaf clover `r = cos(3θ)` |
| **Polygon** | Roots distributed along edges of a regular polygon (3–8 sides) |
| **Infinity** | Lemniscate of Bernoulli (figure-eight / infinity symbol) |

## Trail Coherence

`numpy.roots()` returns roots in arbitrary order — there is no guaranteed correspondence between the i-th root across consecutive calls. Without correction, trails would jump chaotically between unrelated roots.

PolyPaint solves this with **greedy nearest-neighbor matching**: for each root from the previous frame, the closest unmatched root in the new frame is assigned to the same trail. This is O(n²) per frame but negligible for n ≤ 30.

## File Structure

```
polypaint/
├── pyproject.toml        # Dependencies: fastapi, uvicorn, websockets, numpy
├── server.py             # FastAPI backend (~40 lines)
├── static/
│   └── index.html        # Entire frontend (~850 lines): d3.js, CSS, JS all inline
├── uv.lock               # Locked dependency versions
└── README.md
```

## Dependencies

| Package | Version | Purpose |
|---------|---------|---------|
| fastapi | ≥0.115 | Web framework with WebSocket support |
| uvicorn[standard] | ≥0.30 | ASGI server |
| websockets | ≥12.0 | WebSocket protocol implementation |
| numpy | ≥1.26 | Polynomial root finding via companion matrix eigenvalues |

Python ≥ 3.10 required.

## Technical Details

### Root Finding

`numpy.roots(coeffs)` works by constructing the [companion matrix](https://en.wikipedia.org/wiki/Companion_matrix) of the polynomial and computing its eigenvalues. This is numerically stable and runs in O(n³) time (dominated by the eigenvalue decomposition), which is sub-millisecond for degree ≤ 30.

### Edge Cases Handled

- **Leading coefficient at origin**: stripping leading near-zero coefficients before calling `numpy.roots` avoids degenerate companion matrices
- **NaN/Inf roots**: filtered out server-side before sending to client
- **WebSocket disconnect**: auto-reconnect after 1 second
- **Window resize**: panels dynamically resize to use all available screen space
- **Degree change**: coefficients reinitialized, trails cleared, both panels reset

### Performance

- WebSocket sends are throttled via `requestAnimationFrame` (~60 fps cap)
- No d3 transitions on root dots — positions update instantly to avoid animation conflicts during rapid drag
- Single SVG per panel with layered groups (grid → trails → dots)
- Trail paths use `d3.line()` generator, re-rendered each frame from cached point arrays
