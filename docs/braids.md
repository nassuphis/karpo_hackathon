# Root Braids and Monodromy

When you animate a coefficient along a closed loop, the roots don't just wiggle — they trace out a **braid**. This is a topological phenomenon with deep mathematical roots (pun intended).

The space of degree-*n* polynomials with distinct roots is topologically the [configuration space](https://en.wikipedia.org/wiki/Configuration_space_(mathematics)) of *n* unordered points in **C**. Its fundamental group is the [braid group](https://en.wikipedia.org/wiki/Braid_group) B_n. A closed loop in coefficient space induces a **monodromy permutation** on the roots — after one full cycle, root #3 might now occupy the position that root #7 had before.

This is not a solver artifact; it is a topological invariant of the loop. Different loops around different "holes" in coefficient space produce different permutations. The [cohomology](https://en.wikipedia.org/wiki/Cohomology) of the configuration space (computed by Arnol'd and Cohen) classifies these possibilities.

**What you see in PolyPaint:** the trail patterns are visual braids. Root identity is preserved across frames by reordering each new solver output to best match the previous frame's root positions. This keeps trails tracking the same root continuously. When roots still swap indices (e.g. during fast near-collisions), jump detection (30% of visible range threshold) inserts a path break (SVG `M` move) rather than drawing a false connecting line.

**Root-matching strategies:** The main-thread animation path uses `matchRootOrder`, a greedy nearest-neighbor O(n^2) matcher. For bitmap fast-mode workers, root matching is configurable via `bitmapMatchStrategy`:

- **Hungarian** (`hungarian1`): Kuhn-Munkres O(n^3) optimal assignment, run every step. Most accurate but slowest.
- **Greedy x1** (`assign1`): Greedy nearest-neighbor every step. Same algorithm as `matchRootOrder` but using typed arrays (`matchRoots` in the worker blob).
- **Greedy x4** (`assign4`): Greedy nearest-neighbor every 4th step. Default — balances accuracy and speed since O(n^2) matching is expensive at high frame rates.

These strategies only apply to non-uniform bitmap color modes (Index Rainbow, Derivative, Root Proximity) where root identity matters for coloring.
