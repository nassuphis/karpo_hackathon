# Sonification

Three independent instrument layers — **B** (Base), **M** (Melody), **V** (Voice) — can be toggled individually via sidebar buttons. Each button opens a config popover with an on/off toggle and tuning sliders that modify the sound in real time. Sound is generated whenever roots update — both during animation playback and during interactive drag of coefficients or roots.

## Audio Graph

```
[osc1: sine 110Hz] ──┐                                                    ┌──► speakers
                      ├──► [gainNode] ──► [lowpass filter] ──► [masterGain]┤
[osc2: triangle ~111Hz] ──┘                                                └──► [mediaDest] ──► recording

[beepOsc: sine] ──► [beepGain] ──► [masterGain]

[arpOsc: triangle] ──► [arpGain] ──► [arpFilter: lowpass] ──► [masterGain]

[lfo: sine 1.5–7.5Hz] ──► [lfoGain] ──► osc1.frequency + osc2.frequency
```

Two main oscillators (sine + slightly detuned triangle at ×1.012) produce a thick, beating theremin tone. They pass through a gain stage, then a lowpass filter whose cutoff tracks the root constellation's spread and kinetic energy, then a master gain node that acts as the definitive gate. A separate beep oscillator, gated by its own gain envelope, handles close encounter events. An arpeggiator oscillator (triangle wave) cycles through roots at 24 notes/sec, mapping each root's angle to a pentatonic pitch and radius to an octave, with pluck-style envelopes gated by per-root velocity. An LFO provides vibrato on both main oscillators, with both rate and depth modulated by the root distribution.

## Instrument Config Popovers

Each instrument button (**B**, **M**, **V**) opens a popover with an on/off toggle and tuning sliders. All parameters take effect immediately — drag a slider during animation and hear the change in real time.

**Base** (6 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Pitch | 55–440 Hz | 110 Hz | Center frequency of the drone |
| Range | 0.5–4.0 oct | 2.0 | How many octaves the pitch swings with root spread |
| Detune | 1.000–1.050 | 1.012 | Frequency ratio between the two oscillators (beating) |
| Bright | 50–1000 Hz | 250 Hz | Filter cutoff floor (higher = brighter at rest) |
| Volume | 0.05–0.50 | 0.22 | Gain swing driven by kinetic energy |
| Vibrato | 0–25 Hz | 10 Hz | LFO depth driven by angular coherence |

**Melody** (5 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Rate | 2–60 /s | 24 /s | Arpeggiator step speed (notes per second) |
| Volume | 0.02–0.30 | 0.12 | Peak note gain |
| Attack | 1–20 ms | 4 ms | Pluck attack time |
| Decay | 10–200 ms | 64 ms | Pluck decay time |
| Bright | 200–4000 Hz | 1200 Hz | Filter cutoff floor |

**Voice** (5 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Memory | 1.0001–1.010 | 1.001 | Record decay rate (higher = records expire faster, more beeps) |
| Cooldown | 10–500 ms | 80 ms | Minimum gap between beeps |
| Volume | 0.02–0.30 | 0.12 | Beep peak gain |
| Attack | 1–20 ms | 5 ms | Beep attack time |
| Decay | 10–300 ms | 80 ms | Beep ring-down time |

## Feature Extraction

Six features are extracted from `currentRoots` each frame, using distribution statistics rather than simple averages. This ensures the sound responds to the *shape* of the root constellation, not just its center of mass.

| Feature | Formula | Maps to |
|---------|---------|---------|
| **Median radius** (`r50`) | 50th percentile of distances from centroid | Oscillator pitch |
| **Spread** (`r90 − r10`) | 90th minus 10th percentile of radii | Filter cutoff (brightness) |
| **Energy med** (`E_med`) | 50th percentile of per-root velocities | Filter cutoff boost, LFO speed |
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

## Sound Mapping

**Pitch (theremin):**
```
frequency = 110 × 2^((r50_norm − 0.5) × 2.0)
```
where `r50_norm = clamp(r50 / panel_range, 0, 1)`. The median radius maps to ±1 octave around A2 (110 Hz). When the root constellation expands, pitch rises; when it contracts, pitch falls. This is more stable than centroid-based pitch because the median radius resists centroid drift. The second oscillator tracks at `frequency × 1.012` for a slow beating effect.

**Filter cutoff (brightness):**
```
cutoff = 250 + 3500 × spread_norm + 1500 × E_med_norm
```
Clamped to 150–8000 Hz, Q = 2. The constellation's spread opens the filter (wider cloud = brighter tone), and median kinetic energy adds further brightness. When roots are tightly clustered and still, only the fundamental comes through; when they're spread out and active, upper harmonics emerge.

**Gain (loudness):**
```
gain = 0.03 + 0.22 × E_hi_smoothed
```
where `E_hi_norm = clamp(E_hi / (range × 0.05), 0, 1)`. The 85th-percentile energy drives loudness — when even a few roots are moving fast, you hear it. The small floor (0.03) ensures the drone doesn't completely vanish during slow structural rearrangements. The smoothing filter provides a natural fade-out when motion stops, and the watchdog timer (see Silence Management) handles the final fade to true silence.

**Vibrato (coherence):**
```
lfo_depth = 2 + 10 × R_smoothed  Hz
lfo_rate  = 1.5 + 6.0 × E_med_norm  Hz
```
The LFO modulates both oscillators' frequencies. When roots cluster angularly (high R), vibrato depth increases up to 12 Hz — the sound "wobbles" as the clump moves. When roots form a balanced ring (low R), vibrato settles to a gentle 2 Hz baseline. Additionally, the LFO *rate* itself increases with median energy: calm scenes get slow vibrato (1.5 Hz), active scenes get faster pulsing (up to 7.5 Hz).

**Close encounter beeps (novelty-based):**

Instead of a fixed or adaptive threshold, each root tracks its own **top 3 closest distances** ever observed to any neighbor. A beep fires only when a root **beats one of its own records** — a genuinely unusual close approach for that specific root.

Each root's encounter table is seeded with current distances on the first frame (no startup burst). On subsequent frames:
1. Each root finds its 3 closest neighbors via partial sort
2. If any distance is smaller than the root's worst record, the record is replaced
3. The root with the most dramatic improvement fires a beep at its own **pentatonic pitch** (root 0 = C4, root 1 = D4, ..., ascending through octaves via the scale [C, D, E, G, A])
4. All records slowly **decay** (`×1.001` per frame, ~6%/sec at 60fps), so old records gradually become beatable again

```
pitch = midiToHz(60 + pentatonic[i % 5] + 12 × floor(i / 5))
peak  = 0.05 + 0.12 × clamp(improvement / (range × 0.05), 0, 1)
envelope: 0.0001 → peak (5ms attack) → 0.0001 (80ms decay)
cooldown: 80ms between beeps
```

This approach is inherently adaptive: tight configurations set low records early, so only truly exceptional approaches trigger; loose configurations keep records high, so moderate approaches still register. No threshold constants to tune.

## Silence Management

A **watchdog timer** runs via `setInterval` (100ms) while sound is enabled. If `updateAudio()` has not been called for 150ms (i.e., no root updates from animation or drag), the watchdog fades `masterGain` to zero over ~150ms. Using `setInterval` instead of `requestAnimationFrame` ensures the watchdog keeps running even when the browser throttles animation in background tabs.

A **visibilitychange** listener immediately ramps `masterGain` to zero (20ms time constant) when the page becomes hidden, preventing orphaned audio when the user switches tabs.

Additionally, `resetAudioState()` is called on: animation stop, Home button, pattern change, degree change, and sound toggle off. It zeroes all smoothing accumulators and ramps `masterGain` to zero, ensuring clean silence in all state transitions.
