# Patterns

## Basic (5)

| Pattern | Description |
|---------|-------------|
| Circle | Evenly spaced on a circle |
| Real axis | Along the real axis |
| Imaginary axis | Along the imaginary axis |
| Grid | Square grid arrangement |
| Random | Uniformly random |

## Coefficient Patterns (8)

| Pattern | Description |
|---------|-------------|
| Spiral | Archimedean spiral |
| Star | Alternating inner/outer radii |
| Figure-8 | Bernoulli lemniscate |
| Conjugate pairs | Pairs straddling the real axis |
| Two clusters | Two separate clusters |
| Geometric decay | Alternating-sign geometric series |
| Rose curve | 3-petal rose r = cos(3θ) |
| Cardioid | Heart-shaped curve |

## Root Shapes (13)

These define the *roots* in a specific shape, then compute the coefficients by expanding the product. Dragging the resulting coefficients perturbs the roots away from the initial shape:

Heart, Circle, Star, Spiral, Cross, Diamond, Chessboard, Smiley, Figure-8, Butterfly, Trefoil, Polygon, Infinity

## Trail Gallery

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T12-25-27.png" width="90%" alt="Degree-24 grid pattern — circle path, one complete loop">
</p>

**Degree 24, grid pattern, circle path (one full loop)** — 17 of 25 coefficients orbiting on a circle (radius 0.75, speed 0.5). Left panel shows the coefficient trails — each selected coefficient traces its own circle, creating an overlapping ring pattern from the grid arrangement. Right panel shows the resulting root braids: small loops and cusps where roots nearly collide, large sweeping arcs where roots respond dramatically to the perturbation. The loop detection fired after one complete cycle, stopping trail accumulation automatically.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T12-29-26.png" width="90%" alt="Degree-24 grid pattern — figure-8 path, one complete loop">
</p>

**Degree 24, grid pattern, figure-8 path** — Same 17 coefficients, now following a figure-8 (lemniscate) at radius 1.5. The coefficient trails on the left form a dense weave of overlapping figure-8s. On the right, every root inherits the double-loop character — small figure-8 knots appear throughout, with some roots tracing tight local loops while others sweep wide arcs. The larger radius amplifies the perturbation, pushing roots further and producing more dramatic braiding than the circle path above.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T12-48-07.png" width="90%" alt="Degree-30 two-clusters pattern — square path">
</p>

**Degree 30, two-clusters pattern, square path** — 11 of 31 coefficients (from one cluster) following a square path at radius 1.5, speed 0.4. The coefficient trails on the left show nested rectangles — each selected coefficient traces its own square, offset by its position within the cluster. The 30 roots on the right arrange in a large ring with emerging trail segments showing the early stages of the braid. The two-cluster pattern concentrates coefficients into two groups, creating an asymmetric perturbation that pushes some roots into tight local orbits while others track the ring's perimeter.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T12-53-02.png" width="90%" alt="Degree-30 chessboard roots — circle path, 5 coefficients">
</p>

**Degree 30, chessboard root shape, circle path** — Only 5 of 31 coefficients selected, orbiting on a large circle (radius 2.0, speed 0.5). The roots were initialized in a chessboard grid pattern (a root shape, so the coefficients were computed from the grid). On the left, the 5 selected coefficients trace circles of varying sizes depending on their magnitude. On the right, most roots stay clustered near their grid positions while one outlier root swings through a wide arc — a striking demonstration of how perturbing a few high-order coefficients can leave most roots nearly fixed while sending one root on a long excursion.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T13-00-17.png" width="90%" alt="Degree-30 diamond roots — circle path, single coefficient">
</p>

**Degree 30, diamond root shape, circle path** — A single coefficient (c₈) orbiting on a large circle (radius 2.0, speed 0.4). The roots were initialized in a diamond arrangement. On the left, the lone selected coefficient traces one clean circle while the remaining coefficients sit near the origin. On the right, the 30 roots maintain their diamond shape but each traces a smooth rounded-square orbit — the diamond's corners soften into curves as the perturbation sweeps around. The minimal input (one coefficient, one circle) produces a surprisingly coherent collective response: every root moves in concert, preserving the diamond's symmetry while the trail reveals the underlying geometry of the perturbation.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T13-37-45.png" width="90%" alt="Degree-5 multi-path — 6 paths, each coefficient on its own circle">
</p>

**Degree 5, circle pattern, 6 simultaneous paths (multi-path demo)** — Every coefficient (c₀ through c₅) assigned to its own animation path, all circles but with different configurations: c₀ on a large circle (radius 1.4, speed 0.6, CCW), c₄ barely moving (radius 0.5, speed 0.1, CCW), and the rest at radius 0.5, speed 1.0 with alternating CW/CCW directions. On the left, six circles of varying size show each coefficient's individual orbit. On the right, the 5 roots trace complex entangled loops — the interference between six independent perturbations at different frequencies and directions creates an intricate braid that no single-path animation could produce. The loop detection fired after one complete cycle, confirming the combined motion is periodic.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T18-01-14.png" width="90%" alt="Degree-29 grid pattern — 10 paths with mixed shapes and angles">
</p>

**Degree 29, grid pattern, 10 simultaneous paths with mixed shapes and angles** — 10 of 30 coefficients each on their own path with diverse configurations: circles (c₀ at radius 1.4, c₁₃ at 0.5, c₁₇ at 1.0), a star (c₁₄ at radius 0.15), an astroid (c₁₅ at 0.3), and four horizontal oscillations (c₂₀–c₂₃ at varying speeds 0.2–0.8) plus c₅ horizontal at angle 0.96. Each path uses a different angle rotation, tilting the shapes relative to each other. On the left, the always-visible coefficient paths show the full variety — circles, a star, an astroid, and angled horizontal lines radiating from their coefficients across the grid. On the right, the 29 root trails form a dense braid around a large ring, with tight cusps where roots nearly collide and sweeping arcs from the combined interference of all 10 perturbations at different frequencies and orientations.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T18-08-36.png" width="90%" alt="Degree-29 grid pattern — 11 paths, mostly angled horizontals, maximum chaos">
</p>

**Degree 29, grid pattern, 11 paths with mass horizontal perturbation** — Building on the previous snap, now with nearly every coefficient animated. Two group paths dominate: 12 coefficients (c₁–c₄, c₉–c₁₁, c₁₆–c₂₀) on a single horizontal at speed 0.7 with angle 0.73, and 5 coefficients (c₂₄–c₂₈) on a faster horizontal at speed 1.2 with angle 0.29. The remaining 4 solo paths keep their circles, star, and astroid from before. On the left, the angled horizontal paths form a striking fan of parallel lines across the grid — the angle slider tilts each group's oscillation axis differently. On the right, the root trails explode into tangled loops and whorls: with so many coefficients oscillating at different speeds and angles, the roots are pushed far from equilibrium, producing a chaotic braid where almost every root interacts with its neighbors.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T18-50-52.png" width="90%" alt="Degree-5 circle pattern — Peano space-filling path, single coefficient">
</p>

**Degree 5, circle pattern, Peano space-filling path** — A single coefficient (c₁) tracing a Peano curve at radius 0.75, speed 0.1. On the left, the coefficient's path reveals the Peano curve's characteristic zigzag pattern — dense, space-filling, covering the full square around the coefficient. On the right, each of the 5 roots responds by tracing its own miniature space-filling shape: the Peano structure propagates through the polynomial, producing self-similar fractal-like trails at each root position. The slow speed (0.1) allows the trails to accumulate cleanly, showing the full one-cycle braid. A striking demonstration of how a single space-filling perturbation on one coefficient induces fractal geometry across all roots.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T21-58-35.png" width="90%" alt="Degree-20 two-clusters — dual Hilbert paths, all coefficients animated">
</p>

**Degree 20, two-clusters pattern, dual Hilbert (Moore curve) paths** — All 21 coefficients animated across two Hilbert paths at radius 0.65, speed 0.1, with complementary angles (0.75 and 0.25). One path drives 10 coefficients from one cluster, the other drives 11 from the second cluster. On the left, the overlapping Hilbert curves form a dense, layered maze — the two angle rotations tilt the space-filling grids against each other, creating a moiré-like interference pattern. On the right, the 20 root trails each trace their own miniature Hilbert-like fractal, arranged in a large ring. Every root responds to the combined space-filling motion of both coefficient groups, producing intricate self-similar shapes at each root position — some tightly wound, others more open depending on proximity to the coefficient clusters.

<p align="center">
  <img src="../snaps/polypaint-2026-02-09T07-33-41.png" width="90%" alt="Degree-28 diamond roots — dual circle paths, web-like trail mesh">
</p>

**Degree 28, diamond root shape, dual circle paths at different speeds** — Two coefficients animated on independent circles: c₄ at radius 1.2, speed 0.6 (CW, angle 0.5) and c₀ at radius 1.2, speed 0.1 (CCW, angle 1.0). The roots were initialized in a diamond arrangement. On the left, the two coefficient paths — one large offset circle (c₀ near the bottom-left) and one near the cluster at center — show the asymmetric perturbation. On the right, the 28 roots maintain a large ring with the diamond's characteristic spacing, but the trails weave an intricate web of fine mesh-like filaments connecting neighboring roots. The speed ratio (6:1) between the two paths creates a dense Lissajous-like interference: the fast path drives rapid oscillations while the slow path modulates the overall envelope, producing a cage-like lattice structure around the ring.
