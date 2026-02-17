# Sonification

<video controls loop playsinline width="720" src="https://github.com/user-attachments/assets/f2141904-eef3-433f-81fa-7ac0a1a61bb5"></video>

Three independent instrument layers — **B** (Base), **M** (Melody), **V** (Voice) — can be toggled individually via sidebar buttons. Each button opens a config popover with an on/off toggle and tuning sliders that modify the sound in real time. Sound is generated whenever roots update — both during animation playback and during interactive drag of coefficients or roots.

## Audio Graph

```
[modulator: sine] ──► [modGain] ──► carrier.frequency
                                                                           ┌──► speakers
[carrier: sine 85Hz] ──► [gainNode] ──► [lowpass filter] ──► [masterGain]┤
                                                                           └──► [mediaDest] ──► recording

[beepOsc: sine] ──► [beepGain] ──► [masterGain]

[arpOsc: triangle] ──► [arpGain] ──► [arpFilter: lowpass] ──► [masterGain]

[lfo: sine 1.5–7.5Hz] ──► [lfoGain] ──► carrier.frequency
```

The Base layer uses **FM synthesis**: a sine carrier whose frequency is modulated by a second sine oscillator. The modulator's frequency tracks the carrier at a configurable ratio (default ×2), and its depth (via `modGain`) is driven by a routable signal (default: EHi — 85th-percentile kinetic energy) — at rest the tone is a pure sine, and as roots move the timbre blooms into increasingly rich harmonics. The carrier passes through a gain stage, then a lowpass filter whose cutoff is driven by a routable signal (default: Spread), then a master gain node. A separate beep oscillator, gated by its own gain envelope, handles close encounter events. An arpeggiator oscillator (triangle wave) cycles through the top-N fastest roots, mapping each root's angle to a pentatonic pitch and radius to an octave, with pluck-style envelopes scaled by velocity rank. An LFO provides vibrato on the carrier, with both rate and depth driven by routable signals (defaults: EMed and Coherence respectively).

## Instrument Config Popovers

Each instrument button (**B**, **M**, **V**) opens a popover with an on/off toggle and tuning sliders. All parameters take effect immediately — drag a slider during animation and hear the change in real time.

**Base** (7 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Pitch | 55–440 Hz | 85 Hz | Carrier center frequency |
| Range | 0.5–4.0 oct | 1.8 | How many octaves the pitch swings with root spread |
| FM Ratio | ×0.5–×8.0 | ×2.0 | Modulator frequency relative to carrier (integer = harmonic, non-integer = metallic) |
| FM Depth | 0–800 Hz | 300 Hz | Maximum FM modulation depth, scaled by energy (0 = pure sine) |
| Bright | 50–1000 Hz | 250 Hz | Filter cutoff floor (higher = brighter at rest) |
| Volume | 0.05–0.50 | 0.22 | Gain swing driven by kinetic energy |
| Vibrato | 0–25 Hz | 10 Hz | LFO depth driven by angular coherence |

**Melody** (6 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Rate | 2–60 /s | 5 /s | Arpeggiator step speed (notes per second) |
| Cutoff | 2–degree | 5 | Top-N fastest roots to arpeggio (lower = fewer, more focused notes) |
| Volume | 0.02–0.30 | 0.12 | Peak note gain (scaled by velocity rank: fastest = loudest) |
| Attack | 1–20 ms | 4 ms | Pluck attack time |
| Decay | 10–200 ms | 64 ms | Pluck decay time |
| Bright | 200–4000 Hz | 1200 Hz | Filter cutoff floor |

**Voice** (4 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Cooldown | 10–500 ms | 10 ms | Minimum gap between beeps |
| Volume | 0.02–0.30 | 0.12 | Beep peak gain |
| Attack | 1–20 ms | 5 ms | Beep attack time |
| Decay | 10–300 ms | 80 ms | Beep ring-down time |

Memory and Novelty sliders have moved to the **Config** tab under "Encounters / Records":

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Memory | 1.0–1.02 | 1.001 | Record decay rate per frame (1.0 = records permanent, higher = expire faster, more beeps) |
| Novelty | 0.30–1.00 | 0.85 | Record hysteresis — stored distance is multiplied by this (lower = next approach must be much closer to trigger) |

## Feature Extraction

Six features are extracted from `currentRoots` each frame, using distribution statistics rather than simple averages. This ensures the sound responds to the *shape* of the root constellation, not just its center of mass.

| Feature | Formula | Maps to |
|---------|---------|---------|
| **Median radius** (`r50`) | 50th percentile of distances from centroid | Oscillator pitch |
| **Spread** (`r90 − r10`) | 90th minus 10th percentile of radii | Filter cutoff (brightness) |
| **Energy med** (`E_med`) | 50th percentile of per-root velocities | LFO speed (vibrato rate) |
| **Energy hi** (`E_hi`) | 85th percentile of per-root velocities | Gain (loudness) |
| **Angular coherence** (`R`) | circular mean resultant length of root angles | LFO depth (vibrato) |
| **Close encounters** | per-root top-3 closest distances ever seen | Beep on record-breaking approach |

**Radius distribution:** Each root's distance from the centroid is computed, sorted, and sampled at the 10th, 50th, and 90th percentiles using linear interpolation. The spread `r90 − r10` captures how "inflated" or "collapsed" the constellation is, independent of where its center sits.

**Dual energy:** Per-root velocities (Euclidean frame-to-frame displacement) are sorted and sampled at two percentiles. The 50th percentile (`E_med`) provides a stable, noise-resistant measure; the 85th percentile (`E_hi`) captures when *some* roots go wild even if the majority are calm. This split prevents the median from flattening dynamics.

**Angular coherence:** Instead of mean angular velocity (which cancels when roots rotate in opposite directions), we compute the [circular mean resultant length](https://en.wikipedia.org/wiki/Directional_statistics#Mean_resultant_length): `R = |mean(e^{iθ})|`. R = 1 when all roots cluster at the same angle (a "clump"); R ≈ 0 when angles are uniformly distributed (a ring). This gives structural information even when roots aren't moving.

All features are smoothed with a one-pole exponential filter before mapping to audio parameters:

```
smoothed += α × (raw − smoothed),  α = 0.1
```

This prevents audible discontinuities from frame-to-frame noise while remaining responsive enough to track real motion.

All six features are also available as **Stats dashboard** time-series plots (MedianR, Spread, EMed, EHi, Coherence, Encounters) with matching EMA smoothing, so you can visually track exactly what drives the sound.

## Sound Mapping

**Pitch (FM carrier):**
```
carrier_freq = freq × 2^((pitchRoute.smoothed − 0.5) × octaves)
mod_freq     = carrier_freq × modRatio
mod_depth    = modDepth × fmRoute.smoothed
```
where `freq` defaults to 85 Hz and `octaves` defaults to 1.8. The pitch route's smoothed value (default source: MedianR) maps to ±0.9 octaves around the center frequency. When the root constellation expands, pitch rises; when it contracts, pitch falls. The FM modulator tracks the carrier at the configured ratio (default ×2.0), and its depth scales with the FM route's smoothed value (default source: EHi) — at rest the output is a pure sine, and as roots move the modulation index increases, producing progressively richer harmonics (bells at low depth, brass/metallic at high depth).

**Filter cutoff (brightness):**
```
cutoff = filterLo + filterHi × filterRoute.smoothed
```
Clamped to 150–8000 Hz, Q = 2. `filterLo` defaults to 250 Hz, `filterHi` defaults to 3500 Hz, and the filter route defaults to Spread. When roots are tightly clustered, only the fundamental comes through; when they're spread out, upper harmonics emerge.

**Gain (loudness):**
```
gain = gainFloor + gainRange × gainRoute.smoothed
```
where `gainFloor` defaults to 0.03 and `gainRange` defaults to 0.22. The gain route defaults to EHi (85th-percentile energy), so loudness rises when even a few roots are moving fast. The small floor ensures the drone doesn't completely vanish during slow structural rearrangements. The smoothing filter provides a natural fade-out when motion stops, and the watchdog timer (see Silence Management) handles the final fade to true silence.

**Vibrato (coherence):**
```
lfo_depth = 2 + vibDepth × vibDepthRoute.smoothed  Hz
lfo_rate  = 1.5 + vibRate × vibRateRoute.smoothed  Hz
```
where `vibDepth` defaults to 10 Hz and `vibRate` defaults to 6.0 Hz. The depth route defaults to Coherence and the rate route defaults to EMed. The LFO modulates the carrier's frequency. When roots cluster angularly (high coherence), vibrato depth increases up to 12 Hz — the sound "wobbles" as the clump moves. When roots form a balanced ring (low coherence), vibrato settles to a gentle 2 Hz baseline. Additionally, the LFO *rate* itself increases with median energy: calm scenes get slow vibrato (1.5 Hz), active scenes get faster pulsing (up to 7.5 Hz).

**Close encounter beeps (novelty-based):**

Instead of a fixed or adaptive threshold, each root tracks its own **top 3 closest distances** ever observed to any neighbor. A beep fires only when a root **beats one of its own records** — a genuinely unusual close approach for that specific root.

Each root's encounter table is seeded with current distances on the first frame (no startup burst). On subsequent frames:
1. Each root finds its 3 closest neighbors via partial sort
2. If any distance is smaller than the root's worst record, the record is replaced with `distance × novelty` — when novelty < 1, the stored record is tighter than the actual approach, requiring the next trigger to be proportionally closer (e.g., novelty = 0.85 means the next approach must be ~15% closer)
3. The root with the most dramatic improvement fires a beep at its own **pentatonic pitch** (root 0 = C4, root 1 = D4, ..., ascending through octaves via the scale [C, D, E, G, A])
4. All records slowly **decay** (`×memory` per frame, default 1.001 ≈ 6%/sec at 60fps), so old records gradually become beatable again — set memory to 1.0 for permanent records (beeps trail off completely)

```
pitch = midiToHz(60 + pentatonic[i % 5] + 12 × floor(i / 5))
peak  = clamp((0.05 + voiceConfig.peak × improvNorm) × volScale, 0.0001, 0.5)
envelope: 0.0001 → peak (5ms attack) → 0.0001 (ringdown decay)
cooldown: voiceConfig.cooldown × voiceRoutes[3].smoothed × 2
```

The cooldown, volume, ringdown, and pitch bias are all modulated by their respective Voice routes (see Signal Routing). When routes are disconnected (default), the ×2 scaling with neutral smoothed = 0.5 gives a ×1.0 multiplier, preserving the slider value as-is.

This approach is inherently adaptive: tight configurations set low records early, so only truly exceptional approaches trigger; loose configurations keep records high, so moderate approaches still register. The novelty and memory sliders (in the Config tab) provide direct control over beep density without changing the fundamental algorithm.

**Melody cutoff (velocity ranking):**

The arpeggiator sorts all roots by velocity (frame-to-frame displacement) each frame and only cycles through the top N fastest. The `cutoff` slider controls N — at maximum (= degree) all roots participate as before; at minimum (= 2) only the two fastest-moving roots get arpeggiated. Volume scales linearly with rank: the fastest root plays at full volume, the Nth plays near-silent. This focuses the melody on the most active parts of the constellation.

## Silence Management

A **watchdog timer** runs via `setInterval` (100ms) while sound is enabled. If `updateAudio()` has not been called for 150ms (i.e., no root updates from animation or drag), the watchdog fades `masterGain` to zero over ~150ms. Using `setInterval` instead of `requestAnimationFrame` ensures the watchdog keeps running even when the browser throttles animation in background tabs.

A **visibilitychange** listener immediately ramps `masterGain` to zero (20ms time constant) when the page becomes hidden, preventing orphaned audio when the user switches tabs.

Additionally, `resetAudioState()` is called on: animation stop, Home button, pattern change, degree change, and sound toggle off. It zeroes all smoothing accumulators and ramps `masterGain` to zero, ensuring clean silence in all state transitions.

## UI Feedback Sounds

Three lightweight feedback sounds run independently of the sonification audio graph, sharing the same `AudioContext` but bypassing `masterGain`:

| Function | Sound | Purpose |
|----------|-------|---------|
| `uiPing(freq, dur)` | Sine blip at `freq` (default 880 Hz), 0.08s decay | Positive confirmation — successful save, commit, fit-to-view |
| `uiBuzz()` | Sawtooth sweep 340 → 160 Hz over 0.15s | Negative feedback — "nothing to do" or precondition not met |
| `uiClick()` | Sine tick at 1600 Hz, 0.025s decay | Subtle default for any button/swatch/chip click |

A **global click listener** (`document.addEventListener("click", ...)`) fires `uiClick()` on the next animation frame for any `<button>`, `.swatch`, `.cpick-label`, or `.chip` element. If the button's own handler already played a `uiPing` or `uiBuzz` (tracked via the `_uiSoundActive` flag), the global listener skips the click to avoid doubling.

## Sound Tab — Signal Routing

The roots panel's **Sound** tab exposes a modular routing matrix that connects any computed statistic to any audio parameter. Each of the three instruments (Base, Melody, Voice) has its own routing table with independently configurable signal chains.

### Architecture

Each route is a single signal chain:

```
[input signal] → [normalization] → [EMA smoothing] → [audio parameter]
```

Routes are evaluated every frame inside `updateAudio()`, even during interactive drag.

### Input Signals (25 sources)

Any of the 23 stats computed by the stats pipeline can drive any audio parameter, plus two constants for testing:

| Group | Signals |
|-------|---------|
| **Sonification features** | MedianR, Spread, EMed, EHi, Coherence |
| **Speed stats** | Speed, MinSpeed, MaxSpeed, σSpeed, EMASpeed |
| **Distance stats** | MeanDist, MinDist, MaxDist, σDist, ΔMeanDist |
| **Dynamics** | AngularMom, Force, MinForce, MaxForce |
| **Events** | Records, Encounters, Odometer, CycleCount |
| **Constants** | Const0 (always 0), Const1 (always 1) |

Setting a route's input to **—** disconnects it; Voice and Melody routes default to a neutral `smoothed = 0.5` (which maps to ×1.0 scaling), preserving unmodified behavior.

### Normalization

Two modes are available per route (default: **RunMax**):

| Mode | Algorithm |
|------|-----------|
| **Fixed** | Pre-computed bounds based on panel range and degree — `raw = clamp(value / reference, 0, 1)` |
| **RunMax** | Tracks a running maximum with exponential decay (0.9995/frame ≈ 3%/sec at 60fps) — `runMax = max(runMax × 0.9995, |value|)`, then `raw = value / runMax`. Adapts automatically to the dynamic range of the current animation. |

Bipolar sources (AngularMom) map to [0, 1] via `value / runMax × 0.5 + 0.5` so that zero maps to 0.5 (neutral center).

### EMA Smoothing

Each route has an independent **α** slider (0.01–1.0, log scale) controlling its one-pole exponential moving average:

```
smoothed += α × (raw − smoothed)
```

Low α (0.01) = slow, heavily smoothed response. High α (1.0) = near-instant tracking.

### Routable Targets (14 parameters)

**Base** (6 targets — default wiring shown):

| Target | Default source | Effect |
|--------|---------------|--------|
| Carrier pitch | MedianR | Octave scaling around center frequency: `freq × 2^((smoothed − 0.5) × octaves)` |
| FM mod depth | EHi | Modulation index: `modDepth × smoothed` (0 = pure sine, 1 = full FM) |
| Filter cutoff | Spread | Brightness: `filterLo + filterHi × smoothed`, clamped 150–8000 Hz |
| Gain | EHi | Loudness: `gainFloor + gainRange × smoothed` |
| Vibrato depth | Coherence | LFO amplitude: `2 + vibDepth × smoothed` Hz |
| Vibrato rate | EMed | LFO speed: `1.5 + vibRate × smoothed` Hz |

**Voice** (4 targets — all disconnected by default):

| Target | Effect |
|--------|--------|
| Pitch bias | ±12 semitone shift: `(smoothed − 0.5) × 24` added to MIDI note |
| Volume | Beep peak gain: `peak × smoothed × 2` (neutral at 0.5 → ×1.0) |
| Ringdown | Beep decay time: `ringdown × smoothed × 2` |
| Cooldown | Minimum beep interval: `cooldown × smoothed × 2` |

**Melody** (4 targets — all disconnected by default):

| Target | Effect |
|--------|--------|
| Rate | Arpeggiator speed: `rate × smoothed × 2` (neutral at 0.5 → ×1.0) |
| Brightness | Filter cutoff: `cutoff × smoothed × 2` |
| Volume | Note peak gain: `peak × smoothed × 2` |
| Decay | Pluck decay time: `decay × smoothed × 2` |

Voice and Melody routes use a **×2 scaling** convention: disconnected routes sit at smoothed = 0.5, giving a ×1.0 multiplier (no change). Connecting a source that swings 0–1 gives a 0×–2× modulation range.
