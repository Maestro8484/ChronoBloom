# Animation Catalog

> **Current animation set -- verified against source at v2.27.3 (2026-07-17).**
> v2.4.0 replaced the original 29-mode catalog with 16 palette-aware animations.
> The per-mode descriptions below now document that live set (they were legacy /
> historical through v2.9.x and were rewritten to match source in v2.10.2).
> v2.9.7 made the bloom-family animations theme-faithful (each ring blooms in its
> own clock color). v2.27.0 collapsed the 10 flower palettes and the 4 separate
> reminder palettes into ONE shared 5-option list (Clock colors + 4 moods) used
> by both chimes and nudges; the removed flower palettes are archived in
> `docs/archive/flower_palettes_2026-07-14.md`. v2.27.2 added the Firefly nudge
> (mode 11). All reflected below.
> The live set (`AnimPhase` enum + Web UI labels):
>
> | Slot | Modes | Web UI labels |
> |---|---|---|
> | Quarter (:15/:45) | 1-3 | Slow comet, Dual orbit, Bloom ripple |
> | Half-hour (:30) | 1-3 | Unfurl, Three comets, Breathe |
> | Top of hour (:00) | 1-5 | Ceremony, Galaxy spin, Supernova, Comet relay, Deep breath |
> | Reminder | 6-10 (0-5 delegate to the slots above) | Gentle pulse, Orbiting orb, Ripple in, Heartbeat, Slow bloom |
>
> Since v2.5.0, firmware bounds (`sanitize()`, POST clamps, `/previewAnimation`)
> match this set exactly. The Animation Style system (palette/speed/brightness/
> trail) still applies as described in its section below.

## v2.8.0 Feel Pass (2026-07-03)

No modes were added, removed, or renumbered -- this pass reworked the following in
place, purely for visual feel. Full detail in `docs/CHANGELOG.md` [2.8.0]; summary:

- **Reminder nudges (modes 6/8/9/10 -- Gentle Pulse, Ripple In, Heartbeat, Slow Bloom)**:
  every reminder's peak brightness is now capped by a shared `NUDGE_CEIL` (205/255,
  ~80%) so a nudge never blasts full intensity. Gentle Pulse and Ripple In got longer,
  softer envelopes with a subtle warm hue drift. Heartbeat was rebuilt from a hard
  140ms/80ms strobe into two eased 350ms/550ms swells -- same two-thump cadence, now a
  breath instead of a warning flash. Slow Bloom now unfurls its inner-12 stamen ring
  (previously skipped).
- **Galaxy Spin (hour mode 2)**: `spinWave()` replaced by two-arm `galaxyWave()` with a
  true-black floor, applied to every palette (gradient palettes used to render flat
  with no dark lanes). Added slow hue drift, twinkling "star" pixels in the dark lanes,
  and a twinkling core.
- **Comet Relay (hour mode 4)**: each comet head now sweeps the palette as it travels,
  with the trail lagging a few hue-steps behind -- reads as a rainbow spiral instead of
  a flat-colored comet.
- **Bloom Ripple (quarter mode 3) / Unfurl (half-hour mode 1)**: explicit flower-part
  coloring from the center out -- warm gold stamen (center + inner-12, palette-independent)
  -> stigma (middle-24, palette) -> petals (outer-60, palette), via new `stamenColor()`.
  **Superseded:** v2.9.7 made both animations theme-faithful, so the center is now
  `bloomColor(3, 0)` -- its own configured/palette color -- not the palette-independent warm
  gold described above (verified `animQ3` main.cpp:2170, `animH1` main.cpp:2195).
  `stamenColor()` still exists but no longer drives these two.
- **Demo Mode**: hour showcase (step 4) now plays Ceremony -> Galaxy Spin -> Supernova ->
  Comet Relay (was 3, now 4); reminder step (step 5) plays Gentle Pulse -> Ripple In ->
  Heartbeat -> Slow Bloom once each (was Gentle Pulse x2 + Heartbeat x2) to demonstrate
  the full gentled-nudge range. Total demo runtime ~110s (was ~99s). The step table
  lives in `DemoMode::steps[]` in `src/main.cpp`.

## Animation Design Principles

### Core Philosophy
Animations exist to **acknowledge the passage of time** and **provide visual feedback**, not to obscure the clock display. All animations are time-bound and return to normal clock display.

### Rules
1. **Hands always win** -- After animation ends, clock display must be accurate
2. **Escalating intensity** -- More frequent events = shorter/subtler animations
3. **Configurable** -- User can disable or select specific animations
4. **Time-bound** -- All animations have fixed maximum duration
5. **Brightness-aware** -- Respect user's brightness settings (day/night/auto)

---

## Animation Triggers

### Time-Interval Animations

**Quarter-Hour** (:15, :30, :45) -- **Subtle acknowledgment** (2-3 sec)
- Frequency: 4x per hour
- Intensity: Low
- Purpose: Mark 15-minute intervals without disruption

**Half-Hour** (:00, :30) -- **Medium celebration** (4-6 sec)
- Frequency: 2x per hour
- Intensity: Medium
- Purpose: Significant time marker, brief visual interest

**Top of Hour** (:00 only) -- **Full spectacle** (8-12 sec)
- Frequency: 1x per hour
- Intensity: High
- Purpose: Hourly celebration, showcase LED capabilities

### Status Animations

**Wi-Fi Connecting** (up to 20 sec)
- Blue spinning dot on inner ring
- Shown during boot WiFi connection attempt
- Transitions to success/fail animation

**Wi-Fi Success** (1.5 sec)
- Green spinning dot on inner ring
- Confirms successful connection

**Wi-Fi Failure** (2 sec)
- Red spinning dot on inner ring
- Indicates connection timeout

**Button Press** (0.7 sec)
- Orange spinning dot on inner ring
- Immediate visual feedback for manual time adjustment

**Time Sync** (1.2-1.5 sec)
- Cyan spinning dot on inner ring
- Shown after NTP sync or browser sync

**Settings Saved** (1.3 sec)
- Purple spinning dot on inner ring
- Confirms EEPROM write success

**OTA Update In Progress** (variable duration)
- Blue spinning dot on inner ring (brighter: 0x0000FF)
- Shown during over-the-air firmware upload
- Duration: until upload completes or fails (typically 5-30 sec)
- No user action needed during update

**OTA Update Success** (2 sec)
- Green spinning dot on inner ring (bright: 0x00FF00)
- Confirms firmware written and verified successfully
- Device reboots automatically after animation

**OTA Update Failed** (2+ sec)
- Red spinning dot on inner ring (bright: 0xFF0000)
- Indicates upload error (connection lost, verification failed, etc.)
- Portal may reappear on next boot

**Center Idle** (continuous)
- Slow breathing pulse on center pixel (1.8 sec period)
- Only active when no other status showing
- Can be disabled via `statusAnimations` setting

---

## Quarter-Hour Animations (modes 1-3, ~2.3-2.8s)

Short, low-intensity acknowledgments fired at :15/:45 (and :30 if no half-hour animation is set).

### 1. Slow Comet
**Mode**: `quarterAnimation = 1` (`animQ1`, ~2.5s)

A single bright pixel travels once clockwise around the outer 60-ring, trailing `trailLength` fading tail LEDs behind it, with an eased brightness attack and release. Color comes from the outer band (`bandColor(0)`): the outer-marker color on **Clock colors**, or that ring's color on a mood palette.

### 2. Dual Orbit
**Mode**: `quarterAnimation = 2` (`animQ2`, ~2.8s)

Two pixels half a ring apart (30 LEDs) orbit the outer 60-ring together, each with its own fading trail. The lead orbiter uses the outer band color and the second uses the center band color, so the two arms read as distinct rather than as a duplicate.

### 3. Bloom Ripple (default)
**Mode**: `quarterAnimation = 3` (`animQ3`, ~2.3s)

A bloom ripples outward from the heart: center, then inner-12, then middle-24, then outer-60 light on staggered onsets and fade back. Since v2.9.7 each ring blooms in its own clock color: center->`centerColor`, inner->`innerHourColor`, middle->`hoursColor`, outer->`outerMarkerColor` via `bloomColor()`. On a mood palette each ring uses that palette's color for the ring instead. The default quarter animation and the signature ChronoBloom effect.

---

## Half-Hour Animations (modes 1-3, ~5s)

Medium-intensity markers fired at :30.

### 1. Unfurl (default)
**Mode**: `halfHourAnimation = 1` (`animH1`, ~5s)

Petals unfurl from the heart outward. The center stamen lights first, then the inner-12, middle-24 and outer-60 rings each fill in progressively, LED by LED, on staggered onsets -- a flower opening. Ring colors via `bloomColor()`.

### 2. Three Comets
**Mode**: `halfHourAnimation = 2` (`animH2`, ~5s)

One comet per ring (outer, middle, inner), all travelling the same direction, each with a fading trail in its ring's band color -- a coordinated three-ring relay rather than three unrelated chasers.

### 3. Breathe
**Mode**: `halfHourAnimation = 3` (`animH3`, ~5s)

All three rings breathe together on phase-offset sine waves (inner leads, outer lags), gamma-weighted so the troughs settle gently into dark instead of stepping through the low PWM levels. Ring colors via `bandColor()`.

---

## Top-of-Hour Animations (modes 1-5, ~8-9s)

The full hourly spectacle, fired at :00.

### 1. Ceremony (default)
**Mode**: `hourAnimation = 1` (`animHr1`, ~9s)

A stately full-clock reveal: the outer-60 fills clockwise, then the middle-24 fills counter-clockwise, then the inner-12 fills, and finally the center stamen eases in and gently pulses. The grandest of the hour animations. Ring colors via `bloomColor()`.

### 2. Galaxy Spin
**Mode**: `hourAnimation = 2` (`animHr2`, ~9-10s)

Two bright spiral arms rotate against dark "space" lanes (`galaxyWave()` with a true-black floor), the hue drifting slowly across the run, with occasional white "star" pixels twinkling in the dark lanes and a twinkling galactic core. Reworked in v2.8.0 to carve dark lanes on every palette.

### 3. Supernova
**Mode**: `hourAnimation = 3` (`animHr3`, ~8s)

A bright core ignites, then expanding shells detonate outward (inner->middle->outer), each easing in, before a global fade over the final second. Ring colors via `bloomColor()`; since v2.9.7 the outer shell lights on every palette (it used to go dark on some).

### 4. Comet Relay
**Mode**: `hourAnimation = 4` (`animHr4`, ~8s)

A relay of comets hands off outer->middle->inner. Each comet's head sweeps the palette as it travels, with its trail lagging a few hue-steps behind, so it reads as a rainbow spiral rather than a flat comet. A warm stamen dot lights at the finish.

### 5. Deep Breath
**Mode**: `hourAnimation = 5` (`animHr5`, ~9s)

The whole flower swells and settles on one long gamma-weighted breath -- every ring and the center rise and fall together. Ring colors via `bandColor()`.

---

## Reminder Animations (modes 6-11, ~2-4.4s)

Triggered by the Focus Reminder scheduler (not by clock time) when `focusReminder_animation` is 6-11. All use the **reminder palette** (`reminderPalette`), selected independently of the chime palette so a nudge can read differently from an interval chime. Since v2.10.3 the reminder palette applies to **every** reminder-triggered animation: nudge modes 0-2, which delegate to the quarter/half-hour/hour chime animations, also render in the reminder palette instead of the chime's animation palette. Every reminder's peak brightness is capped by a shared `NUDGE_CEIL` (205/255, ~80%) so it reads as a swell, never an alert flash (v2.8.0 feel pass).

### ANIM_REM1 -- Gentle Pulse
**Mode**: `focusReminder_animation = 6` (`animRem1`, ~3.6s)

A slow ~1.2s rise / brief crest / ~1.9s fall across all rings -- a breath, not a blink -- with a subtle warm hue drift over the swell.

### ANIM_REM2 -- Orbiting Orb
**Mode**: `focusReminder_animation = 7` (`animRem2`, ~3s)

The outer ring holds a dim wash while a single orb orbits the inner-12 with a short trail. The quietest, most localized nudge.

### ANIM_REM3 -- Ripple In
**Mode**: `focusReminder_animation = 8` (`animRem3`, ~2.8s)

Rings swell inward in sequence -- outer, middle, inner, center -- each with a soft ~600ms rise / ~1.1s fall.

### ANIM_REM4 -- Heartbeat
**Mode**: `focusReminder_animation = 9` (`animRem4`, ~2.1s)

Two eased swells in a two-thump cadence (350ms rise / 550ms fall), the second beat quieter -- a heartbeat that breathes rather than strobes (rebuilt from a hard strobe in v2.8.0).

### ANIM_REM5 -- Slow Bloom
**Mode**: `focusReminder_animation = 10` (`animRem5`, ~4s)

A slow bloom opens outer->middle->inner->center over ~4s, completing petals-inward to the stamen.

### ANIM_REM6 -- Firefly
**Mode**: `focusReminder_animation = 11` (`animRem6`, ~4s)

Fourteen fixed points scattered across the three rings breathe on their own staggered cycles (~1.4s period, roughly 55% of cycles hosting a light), so the swarm drifts rather than pulses in unison. The whole nudge fades in over ~0.7s and out over ~0.9s.

Unlike every other nudge, Firefly repaints the idle face first and drifts the swarm across it, rather than painting onto the cleared buffer `renderAnimFrame()` hands it. Lighting only a handful of pixels on a cleared strip would blank the clock for four seconds, which reads as the display switching off. The face keeps its own `effectiveBrightness` and the swarm is the overlay.

---

## Animation Style System

All palette-aware animations use these shared controls:

| Setting | Field | Range | Effect |
|---------|-------|-------|--------|
| Color palette (chimes) | `animationPalette` | 0-3, 7 | One shared 5-option list since v2.27.0: 7 = Clock colors (default, ring-mapped face colors), 0 = Golden hour (warm), 1 = Moonlight (cool), 2 = Dawn (soft-warm), 3 = Twilight (muted-cool). Each mood is four explicit per-ring colors (outer/middle/inner/center) rendered solid per ring. Any other value sanitizes to 7 (`sanitizePaletteValue`) |
| Reminder palette (nudges) | `reminderPalette` | 0-3, 7 | The same 5-option list as `animationPalette`, selected independently so a nudge can read differently from a chime. Applies to every reminder-triggered animation, including delegated nudge modes 0-2 |
| Speed | `animationSpeed` | 1-5 | 0.5x / 0.75x / 1x / 1.5x / 2x time multiplier |
| Peak brightness | `animationBrightness` | 50-255 | Max LED brightness during animation |
| Trail length | `trailLength` | 2-12 | LEDs in chase/sweep trail |

The `paletteColor(position, useReminderPalette)` helper maps 0-255 -> palette color at `animationBrightness`. The `scaledElapsed(elapsed)` helper applies speed scaling to timing.

### Palette 7 renders the face, including its sameness (know this before filming)

`Clock colors` (7) is the default and faithfully reproduces the configured face -- the rings
fusing into one flower is its documented intent (`bloomColor()`, main.cpp:1845-1847). The
consequence: **if the face theme is a tight color family, every bloom animation reads as one
hue.** Checked against both live units 2026-07-28 -- the 15" has `hoursColor #B30000` and
`innerHourColor #B40404` (the same red to the eye), and the 8" has outer/middle/center all one
orange. Bloom Ripple on palette 7 therefore looks monochrome on both, which is correct behavior,
not a bug.

For a camera, select a **mood palette (0-3)** instead. Since v2.31.1 the moods travel hue from
ring to ring (rim to core) while brightening inward, which is what makes a bloom read as radial
on video. Try any candidate ramp on a live clock without flashing:
`python scripts/tune_palette.py --mood 0 --anim quarter:3`.

---

## Implementation Reference

### Trigger Logic (loop)
```cpp
const ClockTime time = timeModel.get();
static uint8_t lastMinute = 255;

if (settingsStore.get().intervalAnimationsEnabled && time.minute != lastMinute && time.second == 0) {
  lastMinute = time.minute;
  
  if (time.minute == 0) {
    // Top of hour
    renderer.triggerHourAnimation(millis());
  } else if (time.minute == 30) {
    // Half-hour
    renderer.triggerHalfHourAnimation(millis());
  } else if (time.minute % 15 == 0) {
    // Quarter-hour
    renderer.triggerQuarterAnimation(millis());
  }
}
```

### Animation Method Signature
```cpp
void renderAnimationName(uint32_t now) {
  // Animation code here
  // Must restore clock display at end
}
```

### Key Helper Functions
```cpp
uint32_t scale(uint32_t color, uint8_t amount);        // Scale RGB by 0-255
uint32_t pulse(uint32_t color, uint32_t now, ...);     // Breathing effect
void setRingPixel(const RingConfig &ring, uint8_t idx, uint32_t color);
void setCenterPixel(uint32_t color);
```

---

## Adding New Animations

### Process
1. **Design** -- Sketch timing and visual effect on paper
2. **Implement** -- Add `renderMyAnimation(uint32_t now)` method to `ClockRenderer` class
3. **Integrate** -- Add enum value to appropriate trigger (quarter/half/hour)
4. **Web UI** -- Add option to dropdown selector
5. **Test** -- Verify animation duration, clean return to clock display

### Guidelines
- **Respect brightness** -- Use `settings_.get().dayBrightness` as reference
- **Time limit** -- Quarter=3s, Half=6s, Hour=12s maximum
- **Clean exit** -- Always return to accurate clock display
- **No blocking delays >50ms** -- Use state machine for longer animations
- **Test at extremes** -- Full brightness and minimum brightness
- **Test all variants** -- 8" and 15" clocks may look different

### Example Template
```cpp
void renderMyAnimation(uint32_t now) {
  const ClockSettings &settings = settings_.get();
  strip_.clear();
  
  // Animation sequence here
  // Use strip_.show() to update display
  // Use delay() sparingly (non-blocking preferred)
  
  // Restore normal display at end
  timeModel.markDirty();  // Force full redraw
}
```

---

## Animation Timing Chart

| Trigger          | Frequency | Duration | Intensity | User Configurable |
|------------------|-----------|----------|-----------|-------------------|
| Quarter-hour     | 4x/hour   | 2-3 sec  | Low       | ✅ 3 variants + off |
| Half-hour        | 2x/hour   | 4-6 sec  | Medium    | ✅ 3 variants + off |
| Top of hour      | 1x/hour   | 8-12 sec | High      | ✅ 5 variants + off |
| Wi-Fi connecting | Boot only | <20 sec  | Low       | ❌ Always on       |
| Wi-Fi success    | Boot only | 1.5 sec  | Low       | ✅ statusAnimations|
| Wi-Fi failure    | Boot only | 2 sec    | Low       | ✅ statusAnimations|
| Button press     | User      | 0.7 sec  | Low       | ✅ statusAnimations|
| Time sync        | Variable  | 1.2 sec  | Low       | ✅ statusAnimations|
| Settings saved   | User      | 1.3 sec  | Low       | ✅ statusAnimations|
| Center idle      | Continuous| N/A      | Minimal   | ✅ statusAnimations|

---

## Future Animation Ideas

### Sensor-Triggered
- **Sunrise detected** (VEML7700): 5-minute warm color fade (dark blue -> orange -> yellow)
- **Sunset detected** (VEML7700): 5-minute cool color fade (yellow -> orange -> deep purple)
- **Storm darkness** (sudden lux drop): Lightning flash effect (random white strobes)
- **Motion detected** (lux spike): Welcome rainbow chase

### Calendar-Triggered
- **New Year (Jan 1 00:00)**: Firework burst + countdown
- **Valentine's Day**: Pink/red heartbeat pulse
- **Halloween**: Orange flicker with random LED dropouts (spooky)
- **Christmas**: Red/green alternating with snowflake sparkles
- **User birthday**: Confetti explosion, extended party mode

### Temperature-Triggered (Future BME280)
- **Heat wave** (>30 degreesC): Red pulsing intensity
- **Cold snap** (<5 degreesC): Blue icy shimmer
- **Comfortable** (18-24 degreesC): Green ambient glow

### Weather-Triggered (Future API)
- **Rain forecast**: Blue droplets falling animation
- **Snow forecast**: White snowflakes drifting down
- **Thunderstorm**: Purple/white lightning flashes
- **Clear sky**: Golden sun rays radiating outward
