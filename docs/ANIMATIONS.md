# Animation Catalog

> **⚠️ Current animation set (v2.4.0+, verified against source 2026-07-04; feel pass v2.8.0):**
> v2.4.0 replaced the 29-mode catalog below with 16 palette-aware animations.
> The per-mode descriptions in this file from "Quarter-Hour Animations" onward
> describe the REMOVED legacy set and are kept for historical reference only.
> v2.8.0 (2026-07-03) did not add/remove/renumber any mode — it reworked several
> in place for feel; see "v2.8.0 Feel Pass" below the Reminder section for what
> changed under each name (Galaxy Spin, Comet Relay, Bloom Ripple, Unfurl, and
> all 4 reminder nudges used in the demo).
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

No modes were added, removed, or renumbered — this pass reworked the following in
place, purely for visual feel. Full detail in `docs/CHANGELOG.md` [2.8.0]; summary:

- **Reminder nudges (modes 6/8/9/10 — Gentle Pulse, Ripple In, Heartbeat, Slow Bloom)**:
  every reminder's peak brightness is now capped by a shared `NUDGE_CEIL` (205/255,
  ~80%) so a nudge never blasts full intensity. Gentle Pulse and Ripple In got longer,
  softer envelopes with a subtle warm hue drift. Heartbeat was rebuilt from a hard
  140ms/80ms strobe into two eased 350ms/550ms swells — same two-thump cadence, now a
  breath instead of a warning flash. Slow Bloom now unfurls its inner-12 stamen ring
  (previously skipped).
- **Galaxy Spin (hour mode 2)**: `spinWave()` replaced by two-arm `galaxyWave()` with a
  true-black floor, applied to every palette (gradient palettes used to render flat
  with no dark lanes). Added slow hue drift, twinkling "star" pixels in the dark lanes,
  and a twinkling core.
- **Comet Relay (hour mode 4)**: each comet head now sweeps the palette as it travels,
  with the trail lagging a few hue-steps behind — reads as a rainbow spiral instead of
  a flat-colored comet.
- **Bloom Ripple (quarter mode 3) / Unfurl (half-hour mode 1)**: explicit flower-part
  coloring from the center out — warm gold stamen (center + inner-12, palette-independent)
  → stigma (middle-24, palette) → petals (outer-60, palette), via new `stamenColor()`.
- **Demo Mode**: hour showcase (step 4) now plays Ceremony → Galaxy Spin → Supernova →
  Comet Relay (was 3, now 4); reminder step (step 5) plays Gentle Pulse → Ripple In →
  Heartbeat → Slow Bloom once each (was Gentle Pulse ×2 + Heartbeat ×2) to demonstrate
  the full gentled-nudge range. Total demo runtime ~110s (was ~99s). See
  `docs/publish/DEMO_MODE.md` for the current step table.

## Animation Design Principles

### Core Philosophy
Animations exist to **acknowledge the passage of time** and **provide visual feedback**, not to obscure the clock display. All animations are time-bound and return to normal clock display.

### Rules
1. **Hands always win** — After animation ends, clock display must be accurate
2. **Escalating intensity** — More frequent events = shorter/subtler animations
3. **Configurable** — User can disable or select specific animations
4. **Time-bound** — All animations have fixed maximum duration
5. **Brightness-aware** — Respect user's brightness settings (day/night/auto)

---

## Animation Triggers

### Time-Interval Animations

**Quarter-Hour** (:15, :30, :45) — **Subtle acknowledgment** (2-3 sec)
- Frequency: 4x per hour
- Intensity: Low
- Purpose: Mark 15-minute intervals without disruption

**Half-Hour** (:00, :30) — **Medium celebration** (4-6 sec)
- Frequency: 2x per hour
- Intensity: Medium
- Purpose: Significant time marker, brief visual interest

**Top of Hour** (:00 only) — **Full spectacle** (8-12 sec)
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

## Quarter-Hour Animations (2-3 sec)

### 1. Sparkle Burst
**Mode**: `quarterAnimation = 1`

**Sequence**:
1. Center pixel flashes bright white (100ms)
2. Ripple effect to all quarter-hour markers (positions 0, 15, 30, 45 on outer ring)
3. Quick fade (20ms steps)

**Visual effect**: Starburst from center outward to cardinal points

### 2. Quarter Pulse
**Mode**: `quarterAnimation = 2`

**Sequence**:
1. All quarter-hour markers (every 5th LED on outer ring) brighten 2x
2. Hold for 500ms
3. Dim back to normal
4. Repeat pulse once more

**Visual effect**: Breathing on hour markers (12, 3, 6, 9 o'clock positions)

### 3. Ring Shimmer (Default)
**Mode**: `quarterAnimation = 3`

**Sequence**:
1. Outer ring brightens to 100% for 400ms, returns to normal (200ms)
2. Middle ring brightens to 100% for 400ms, returns to normal (200ms)
3. Inner ring brightens to 100% for 400ms, returns to normal (200ms)

**Visual effect**: Brightness wave cascading inward

### 4. Laser Ping
**Mode**: `quarterAnimation = 4`

Single bright LED sweeps CW around outer ring in ~600ms with decaying trail. Trail fades on completion. Color: palette position 0. Duration: 1.4s.

### 5. DNA Twist
**Mode**: `quarterAnimation = 5`

Two LEDs 30 positions apart sweep CW together in opposite palette colors (positions 0 and 128). Each has a trail. Duration: 2s.

### 6. Tick Spark
**Mode**: `quarterAnimation = 6`

Sequential flashes at clock positions 0, 15, 30, 45 (each 200ms). Colors cycle through palette at positions 0, 85, 170, 255. Duration: 0.8s.

---

## Half-Hour Animations (4-6 sec)

### 1. Rainbow Sweep (Default)
**Mode**: `halfHourAnimation = 1`

**Sequence**:
1. Rainbow color wave sweeps clockwise around outer ring (2 sec)
2. Simultaneously sweep middle and inner rings (2 sec)
3. Brief hold (1 sec)

**Visual effect**: Color wheel rotating through all rings

### 2. Dual Flash
**Mode**: `halfHourAnimation = 2`

**Sequence**:
1. Outer ring splits: left half red, right half cyan
2. Hold 300ms
3. Colors swap (left cyan, right red)
4. Hold 300ms
5. Repeat swap 2 more times (3 total)

**Visual effect**: Hemisphere color battle

### 3. Tidal Pulse
**Mode**: `halfHourAnimation = 3`

**Sequence**:
1. All LEDs dim to 25%
2. Outer ring wave pattern (sine wave brightness)
3. Middle ring counter-rotating wave
4. Inner ring breathing
5. All return to normal brightness together

**Visual effect**: Ocean wave simulation

### 4. Comet Chase
**Mode**: `halfHourAnimation = 4`

Bright head + decaying trail sweeps CW around outer ring for 2 full laps (~3s). Brief full-ring palette flash at end. Duration: 4s.

### 5. Color Explosion
**Mode**: `halfHourAnimation = 5`

All 60 outer ring LEDs light simultaneously with per-LED palette colors. Each fades at a pseudo-random speed seeded by position. Duration: 3.5s.

### 6. Knight Rider
**Mode**: `halfHourAnimation = 6`

Single LED bounces back and forth across outer ring (positions 0-59) with decaying trail. 3 full bounces at ~800ms per pass. Duration: 5s.

### 7. Strobe Party
**Mode**: `halfHourAnimation = 7`

All rings flash at 8Hz for 2s cycling through 4 palette positions (0, 85, 170, 255). Fades to black over 500ms. Duration: 3s.

---

## Top of Hour Animations (8-12 sec)

### 1. Hourly Chime (Original)
**Mode**: `hourAnimation = 1`

**Sequence**:
1. Sweep animation on outer ring
2. Middle ring synchronized pulse
3. Center pixel bright pulse (180ms period, 4 cycles)

**Visual effect**: Classic chime, maintains compatibility with original firmware

**Duration**: 6 seconds (legacy timing)

### 2. Firework Burst
**Mode**: `hourAnimation = 2`

**Sequence**:
1. All LEDs go dark (500ms pause)
2. Center pixel explodes bright white (100ms)
3. Color radiates outward: inner → middle → outer (400ms per ring)
4. Each ring "detonates" with random hue sparkles
5. Random twinkling for 3 seconds
6. Fade back to clock display

**Visual effect**: New Year's Eve firework

**Duration**: 8-9 seconds

### 3. Zenith Cascade
**Mode**: `hourAnimation = 3`

**Sequence**:
1. Golden/yellow color starts at 12 o'clock position (top)
2. Flows clockwise AND counter-clockwise simultaneously
3. Colors meet at 6 o'clock position (bottom) — 1.2 sec
4. Collision creates white flash at bottom (200ms)
5. Color drains upward like closing theater curtain (1.2 sec)
6. Clock display revealed underneath

**Visual effect**: Golden waterfall pooling at bottom, draining away

**Duration**: 6-7 seconds

### 4. Rainbow Spiral (Default)
**Mode**: `hourAnimation = 4`

**Sequence**:
1. Outer ring rainbow chase clockwise (2 laps, 2 sec)
2. Middle ring joins counter-clockwise (2 laps, 2 sec)
3. Inner ring joins clockwise (2 laps, 2 sec)
4. All three spinning simultaneously for 2 sec
5. Slow down together, freeze on current time colors

**Visual effect**: Triple-helix rainbow vortex

**Duration**: 10-11 seconds

### 5. Breathing Mandala
**Mode**: `hourAnimation = 5`

**Sequence**:
1. All rings pulse brightness 50% → 100% → 50% (1.5 sec per breath)
2. Each breath shifts hue slightly around color wheel
3. 6 breaths total (9 sec)
4. Final breath lands on configured hand colors

**Visual effect**: Meditative breathing flower

**Duration**: 9-10 seconds

### 6. Supernova
**Mode**: `hourAnimation = 6`

All rings ramp to white blast (500ms), then split into palette colors per ring. Rainbow rotates outward with per-ring hue offsets (+21845) for 5.5s, then fades to black. Duration: 8.5s.

### 7. Matrix Rain
**Mode**: `hourAnimation = 7`

8 independent green "drops" move CW on outer ring at different speeds, each with decaying trail. Drops loop continuously for 8s. Duration: 8s.

### 8. Galaxy Spin
**Mode**: `hourAnimation = 8`

Three rings spin palette colors at different speeds and directions: outer CW/3s, middle CCW/5s, inner CW/7s. Each ring has a +85-unit palette offset. Fades out over last second. Duration: 10s.

### 9. Color Wipe
**Mode**: `hourAnimation = 9`

Palette gradient sweeps CW ring by ring: outer (3s), then middle (1.5s), then inner (1s). Each LED gets a unique palette position. Duration: 7s. Speed-scaled.

### 10. Thunderstorm
**Mode**: `hourAnimation = 10`

All rings hold deep blue-purple base color with slow sine-wave breathing. 5 lightning strikes at preset intervals flash 2-4 random outer LEDs + center pixel to white for 80ms. Duration: 10.5s.

---

## Reminder Animations (warm palette)

Triggered by Focus Reminder scheduler when `focusReminder_animation` is 6-11. All use `reminderPalette` (warm/urgent colors). Not triggered by clock time intervals.

### ANIM_REM1 — Amber Pulse
**Mode**: `focusReminder_animation = 6`

Inward wave: outer, middle, inner, center each pulse (200ms ramp, 100ms hold, 200ms ramp down) with 350ms stagger between rings. 3 pulse cycles per ring. Duration: 4s.

### ANIM_REM2 — Attention Ring
**Mode**: `focusReminder_animation = 7`

Outer and middle rings dim to 15% face color. Inner ring: single LED sweeps 3 laps (400ms/lap) with 2-LED trail. Duration: 2s.

### ANIM_REM3 — Heartbeat
**Mode**: `focusReminder_animation = 8`

All rings flash in double-beat pattern (beat1: 80ms up/80ms hold/120ms down; beat2: 80ms/80ms/200ms), then dim glow pause (520ms). 4 cycles. Duration: 5s.

### ANIM_REM4 — Sunrise Wake
**Mode**: `focusReminder_animation = 9`

Radial reveal: inner ring glows deep red, expands outward ring by ring. Colors transition red → orange → yellow-white over 6 seconds. Duration: 6s.

### ANIM_REM5 — Campfire Flicker
**Mode**: `focusReminder_animation = 10`

All rings flicker warm amber-orange, each LED slightly different shade. Slow sine-wave brightness breathing (2s period, ±20%). Occasional bright spark on random outer LED. Duration: 5s.

### ANIM_REM6 — Neon Sign
**Mode**: `focusReminder_animation = 11`

Inner ring pulses at 12Hz (on 40ms / off 42ms). Random glitch: full off for ~120ms every ~700ms. Outer ring holds dim amber at 12%. Duration: 4s.

---

## Animation Style System (v11)

All new animations (ANIM_Q4+) use these shared controls:

| Setting | Field | Range | Effect |
|---------|-------|-------|--------|
| Color palette | `animationPalette` | 0-7 | Maps to Rainbow/Fire/Ocean/Forest/Candy/Neon/Monochrome/Clock |
| Reminder palette | `reminderPalette` | 0-3 | Amber/Red/Magenta/Cyan-warm (reminder anims only) |
| Speed | `animationSpeed` | 1-5 | 0.5× / 0.75× / 1× / 1.5× / 2× time multiplier |
| Peak brightness | `animationBrightness` | 50-255 | Max LED brightness during animation |
| Trail length | `trailLength` | 2-12 | LEDs in chase/sweep trail |

The `paletteColor(position, useReminderPalette)` helper maps 0-255 → palette color at `animationBrightness`. The `scaledElapsed(elapsed)` helper applies speed scaling to timing.

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
1. **Design** — Sketch timing and visual effect on paper
2. **Implement** — Add `renderMyAnimation(uint32_t now)` method to `ClockRenderer` class
3. **Integrate** — Add enum value to appropriate trigger (quarter/half/hour)
4. **Web UI** — Add option to dropdown selector
5. **Test** — Verify animation duration, clean return to clock display

### Guidelines
- **Respect brightness** — Use `settings_.get().dayBrightness` as reference
- **Time limit** — Quarter=3s, Half=6s, Hour=12s maximum
- **Clean exit** — Always return to accurate clock display
- **No blocking delays >50ms** — Use state machine for longer animations
- **Test at extremes** — Full brightness and minimum brightness
- **Test all variants** — 8" and 15" clocks may look different

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
- **Sunrise detected** (VEML7700): 5-minute warm color fade (dark blue → orange → yellow)
- **Sunset detected** (VEML7700): 5-minute cool color fade (yellow → orange → deep purple)
- **Storm darkness** (sudden lux drop): Lightning flash effect (random white strobes)
- **Motion detected** (lux spike): Welcome rainbow chase

### Calendar-Triggered
- **New Year (Jan 1 00:00)**: Firework burst + countdown
- **Valentine's Day**: Pink/red heartbeat pulse
- **Halloween**: Orange flicker with random LED dropouts (spooky)
- **Christmas**: Red/green alternating with snowflake sparkles
- **User birthday**: Confetti explosion, extended party mode

### Temperature-Triggered (Future BME280)
- **Heat wave** (>30°C): Red pulsing intensity
- **Cold snap** (<5°C): Blue icy shimmer
- **Comfortable** (18-24°C): Green ambient glow

### Weather-Triggered (Future API)
- **Rain forecast**: Blue droplets falling animation
- **Snow forecast**: White snowflakes drifting down
- **Thunderstorm**: Purple/white lightning flashes
- **Clear sky**: Golden sun rays radiating outward
