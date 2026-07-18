# ChronoBloom ESP32-C3 — Feature Catalog

## Current Features (Implemented ✅)

### Time Display
- ✅ **Analog 3-ring clock** — Hours/minutes/seconds shown via LED position
- ✅ **Per-ring color customization** — RGB color pickers for 6 elements
- ✅ **Per-ring brightness control** — Independent intensity sliders (0-255)
- ✅ **Quarter-hour markers** — Every 5th LED on outer ring
- ✅ **Progress fill** — Dim arc showing seconds elapsed
- ✅ **Fading trail** — 4-LED trail behind second hand
- ✅ **Center status pixel** — Breathing idle animation, status indicator

### Brightness Control
- ✅ **Manual brightness** — Day/night sliders
- ✅ **Scheduled brightness** — Auto-switch at configurable hours
- ✅ **VEML7700 auto-brightness** — Lux-based logarithmic curve
- ✅ **Three modes** — Manual / Auto (sensor) / Scheduled (day/night)
- ✅ **Configurable limits** — Min/max brightness clamps
- ✅ **Dark-room display sleep** (v2.5.0) — Auto mode + `darkRoomOff`: 30 s below 0.3 lux blanks all LEDs, wakes above 2.0 lux

### Animations
- ✅ **Hourly chime** — Sweep + center pulse (8 sec)
- ✅ **Status animations** — WiFi connect, button press, NTP sync, settings save
- ✅ **Center idle breathing** — Slow pulse when no status active
- ✅ **Time-interval animations** — Escalating intensity at :15/:30/:00 (v2.4.0 overhaul: 29 legacy modes replaced with 16 palette-aware animations)
  - Quarter-hour (:15/:45, modes 1-3): Slow comet, Dual orbit, Bloom ripple
  - Half-hour (:30, modes 1-3): Unfurl, Three comets, Breathe
  - Top of hour (:00, modes 1-5): Ceremony, Galaxy spin, Supernova, Comet relay, Deep breath
- ✅ **Animation palette system** (simplified v2.27.0) — **one shared 5-option list** applies to both interval chimes and reminders: `Clock colors (default)` (ring-mapped from the configured face colors) + 4 temperature/mood palettes **Golden hour (warm)**, **Moonlight (cool)**, **Dawn (soft-warm)**, **Twilight (muted-cool)**. Each mood is **four explicit per-ring colors** (outer/middle/inner/center) rendered solid per ring — the same model as Clock colors. Source of truth `tools/palettes/palettes.json` (`mood_palettes` → `rings`) → `tools/gen_palettes.py` → `src/anim_palettes.h`. Values are authored **gamma-aware** (route through `gammaColor`, gamma 2.2). The 10 prior flower palettes were removed and archived (`docs/archive/flower_palettes_2026-07-14.md`)
- ✅ **Animation style controls** — Speed (1-5), peak brightness (50-255), trail length (2-12 LEDs)
- ✅ **Reminder palette** — an **independent selection** from the same 5-option list (Clock colors + 4 moods), so a reminder/nudge can read differently from an interval chime. Applies to every reminder-triggered animation incl. nudge modes 0-2, which delegate to the chime animations

### Smart Features
- ✅ **NTP time sync** — Network Time Protocol with timezone/DST support
- ✅ **Runtime time zone** (v2.28.0) — set in the web UI (Time & light → Time zone): 18 common zones plus an "Other" field taking any POSIX TZ string. Stored in NVS (`clock`/`tz`), not in `ClockSettings`, so no settings migration; `NTP_TIMEZONE_TZ` in `platformio.ini` is only the default for a fresh or factory-reset unit. Applies immediately with no reboot and no NTP round trip, since the ESP keeps the system clock in UTC and only `tzset()` has to re-run. Rejected strings return 400 rather than silently meaning UTC
- ✅ **Web UI** — Full-featured control interface with live preview
- ✅ **Browser time sync** — One-click sync from phone/computer clock
- ✅ **EEPROM persistence** — All settings survive power cycles (v8)
- ✅ **WiFi web server** — mDNS hostname (esp32c3-v3-8inch.local)
- ✅ **Live SVG preview** — Clock updates every 90ms in web UI
- ✅ **Animation toggles** — Enable/disable individual effects
- ✅ **Theme selection** — 7 presets: ChronoBloom (clock default), Moonflower, Cherry Blossom, Ember Dahlia, Lotus Pond, Sunflower, Bird of Paradise. Each sets the full face color set plus animation style in one click
- ✅ **Manual time adjustment** — H:M:S entry and +/- minute controls via WebUI
- ✅ **Improv WiFi (v2.29.0)** — Browser-configured WiFi over USB serial at flash time (official improv-wifi/sdk-cpp protocol, `improv/Improv @ 1.2.6`). Bounded 10s window on an unprovisioned device before falling back to the captive portal; serviced only while unprovisioned/AP-fallback, never once connected
- ✅ **WiFi provisioning portal** — Captive portal on first boot; AP fallback at 192.168.4.1 if STA unavailable
- ✅ **OTA firmware updates** — ArduinoOTA (espota) and web UI `/update` page; no USB cable required after first flash
- ✅ **mDNS reconnect** — Hostname re-advertised automatically after WiFi reconnect

### Physical Buttons
- ✅ **Physical UP/DOWN buttons** — GPIO5(UP) / GPIO9(DOWN), polled (no ISRs), 50ms debounce
  - Short press: +1/-1 minute
  - Hold >500ms: auto-repeat +1/-1 minute every 150ms
  - Hold >2000ms: switches to +60/-60 minutes per fire (hour jump)
  - Release: repeat stops immediately
  - No-WiFi time adjustment without WebUI

### Focus Reminders
- ✅ **Focus Reminders (ADHD support)** — Visual nudge animations at configurable intervals to interrupt hyperfocus
  - Single configurable rule: interval (1-1440 min), active hours window, days-of-week bitmask
  - Modes 0-2: delegate to existing time-interval animations (quarter/half/hour slots)
  - Modes 6-11: dedicated reminder animations (Gentle pulse, Orbiting orb, Ripple in, Heartbeat, Slow bloom, Firefly)
  - All reminder animations use the reminder palette, capped by a shared `NUDGE_CEIL` (205/255) so a nudge reads as a swell, never an alert flash
  - Enable/disable toggle
  - Configurable per-day schedule (Sun-Sat)
  - Fire timestamp stored in RAM only — not persisted to EEPROM (field `focusReminder_lastFireMs` reserved in struct but unused since v2.1.1)
  - Web UI panel: "Focus Reminders (ADHD)"

### Demo Mode
- ✅ **Demo Mode (Video Recording)** — Non-blocking state machine for feature sequencing during video recording
  - 9-step sequence (`DemoMode::steps[]` in `src/main.cpp`): title card (8.8s), quarter chime (5.6s), half-hour chime (11s), top-of-hour chime (17s), hour animation showcase (36.5s), focus reminder nudges (20s), center LED status (6.8s), auto-brightness cycles (14.85s), open-source end card (10.85s)
  - Total runtime: ~131 seconds. The animation steps are event-paced and depend on which styles are configured, so per-step figures are estimates at `animationSpeed` 3, not exact bounds; each transition includes a ~0.8s dissolve and breather
  - LuxSensor override: simulates two quick room-darkening/brightening cycles to demo the faster (150ms poll) auto-brightness response
  - `docs/publish/DEMO_CAPTIONS.srt`: timestamped SRT caption track mirroring the sequence for OBS/post-production use
  - Web endpoints: `POST /demo/start`, `POST /demo/stop`, `GET /demo/status`, `GET /demo/overlay`
  - `/demo/status` returns: active state, current step, subtitle, elapsed time, step duration
  - `/demo/overlay` HTML: full-screen 1920x1080, OBS-ready browser source with fade transitions
  - Web UI controls: Start/Stop buttons, live status display (step counter + progress bar)
  - Buttons ignored during demo; all web endpoints continue normal operation
  - No settings version change, no EEPROM impact

### Web UI Features
- ✅ **Time controls** — Manual set, browser sync, NTP sync, increment/decrement (WebUI); physical buttons re-added v2.0.6 (GPIO5=UP, GPIO9=DOWN)
- ✅ **Display settings** — Day/night brightness, schedule hours
- ✅ **Ring controls** — 6 separate RGB + brightness: outer marker, outer fill, seconds, minutes, hours, center
- ✅ **Animation controls** — Second trail, progress ring, hourly chime, status animations, interval animations, palette/speed/brightness/trail controls
- ✅ **Animation preview** — Preview button on every animation selector; Animation Style panel with live preview by type
- ✅ **12-hour AM/PM display** — Clock header shows 12h format with AM/PM
- ✅ **Live sensor data** — Current lux reading updates every 2 seconds
- ✅ **Auto-brightness controls** — Mode selection, min/max limits
- ✅ **Network info** — IP address, WiFi SSID, signal strength, NTP sync status
- ✅ **Demo Mode controls** — Start/Stop buttons, live status display for video recording sequences

---

## Planned Features (Priority Order)

### High Priority
- [ ] **Sunrise/sunset detection** — VEML7700 detects daylight transitions, triggers warm fade animations
- [ ] **Holiday auto-animations** — Date-triggered effects (Christmas, Halloween, New Year, Easter, user birthday)
- [ ] **Multiple Focus Reminder rules** — 3-5 concurrent reminder rules (v2.1 roadmap)
- [ ] **Multiple Focus Reminder rules** — 3-5 concurrent reminder rules

### Medium Priority
- [ ] **Theme presets** — Save/load entire color schemes to EEPROM slots (named presets)
- [ ] **BME280 temp/humidity sensor** — Color-coded center pixel (blue=cold, red=hot), web UI display
- [ ] **MQTT for Home Assistant** — Publish state, subscribe to commands
- [ ] **Sunrise alarm** — 5-minute gradual brightness increase simulating dawn
- [ ] **Pomodoro timer mode** — 25/5 work/break visual countdown on outer ring

### Low Priority
- [ ] **Lux history graph** — 60-minute trend chart in web UI (canvas or SVG)
- [ ] **Circadian rhythm color shift** — Warm orange evening, cool blue morning (auto-adjusts based on lux)
- [ ] **Motion detection proxy** — Sudden lux spike = person approaching, trigger welcome animation
- [x] **Power saving mode** — Done v2.5.0 as dark-room display sleep (`darkRoomOff`)
- [ ] **Multi-clock network sync** — Master/slave mode for synchronized animations across multiple clocks
- [ ] **LED mapping test mode** — `/ledtest?pixel=N` and `/ledwalk` endpoints for physical verification
- [ ] **Animation playlist** — Auto-rotate effects every N minutes in demo mode
- [ ] **Custom schedules** — Different animations at different times/days
- [ ] **Voice control** — Alexa/Google Assistant via MQTT bridge

---

## Removed Features (Was Implemented, Now Removed)

**Physical buttons (GPIO3/GPIO4)** — removed v2.0.4, re-added v2.0.6 on safe pins
- Originally GPIO3/GPIO4 (JTAG TCK/TDI): spurious ISR fires with USB connected → removed v2.0.4
- Re-added v2.0.6 on GPIO5(UP)/GPIO9(DOWN) using polled reads, no ISRs
- v2.0.7: GPIO swap to GPIO5=UP, GPIO9=DOWN; hold-to-repeat added (500ms→1min/150ms, 2s→60min/fire)
- See CHANGELOG [2.0.4], [2.0.6], [2.0.7] for full context

---

## Rejected Features (Do Not Implement)

### Why These Don't Fit
The clock's purpose is **elegant analog timekeeping through light and color**. The 3-ring LED topology (60/24/12) is optimized for circular time display, not general-purpose pixel matrix.

### Specific Rejections

❌ **Game modes** (Tetris/Snake/Pong)
- Requires rectangular pixel grid
- Obscures time display entirely
- Wrong use case for a clock

❌ **Text scrolling / message display**
- Insufficient pixel density (96 LEDs total)
- Ring topology makes text unreadable
- Better suited for 8x8 or larger matrices

❌ **Pixel art upload / painting interface**
- 96 LEDs in concentric circles ≠ rectangular canvas
- Can't map bitmap images to ring topology meaningfully

❌ **7-segment digital display overlay**
- Conflicts with analog aesthetic
- Would require hiding ring LEDs = wasted hardware
- If you want digital, buy a different clock

❌ **Music visualization / audio reactive mode**
- Would obscure time display during playback
- I2S mic + FFT would constantly override clock
- Dedicated LED strip projects exist for this purpose

❌ **Social media counters / stock tickers**
- This is not an information dashboard
- No way to display numbers/text clearly
- Use Tidbyt or LaMetric for this

❌ **Audio speaker mode / Bluetooth speaker**
- Feature creep beyond timekeeping
- Adds speaker hardware complexity
- Not what this project is about

❌ **E-ink companion display**
- Unnecessary complexity
- If you need date/temp text, add a separate e-ink module elsewhere

❌ **Video playback / GIF animation upload**
- 96 LEDs total, circular topology
- Cannot render video meaningfully
- Wrong hardware entirely

---

## Feature Request Guidelines

### Before Proposing a New Feature, Ask:

1. **Does it enhance timekeeping?** (Better readability, more accurate, more informative about time?)
2. **Does it use the ring topology effectively?** (Radial symmetry, position = time metaphor?)
3. **Is it visually compatible?** (Light bloom aesthetic, flower-like petal effect?)
4. **Does it stay true to analog display?** (No trying to make digital displays from circular LEDs)
5. **Is the complexity justified?** (Simple is better, don't add features just because you can)

### Good Feature Examples:
- ✅ Moon phase indicator (inner ring changes color based on lunar cycle)
- ✅ Tide indicator (for coastal users, outer ring shows high/low tide position)
- ✅ Countdown to event (outer ring fills as event approaches)
- ✅ Weather-based color themes (cloudy = gray, sunny = yellow, rainy = blue)
- ✅ Air quality indicator (center pixel color = AQI level)

### Bad Feature Examples:
- ❌ Display email notifications as scrolling text
- ❌ Play YouTube videos on the LED rings
- ❌ Cryptocurrency price charts
- ❌ Turn it into a smart speaker
- ❌ Instagram follower counter

---

## Settings Structure (EEPROM v16)

> The struct sketch below is the v11 layout plus later additions; `src/main.cpp`
> (`struct ClockSettings`) is authoritative. v12+ added:
> `middleFace*`/`innerFace*`/`innerHour*` colors, `outerRingBrightness`,
> `middleFaceScale`, `innerFaceScale` (v12-v14), `darkRoomOff` (v15),
> `secondTrailLength`/`secondTrailStyle`/`progressLevel`/`progressStyle` (v16).
> A second EEPROM slot at offset 128 stores user-saved defaults (v2.4.5+,
> magic 0xD2); compile-time static_asserts guard both layout boundaries.

### Stored Configuration
**Total size**: 256 bytes (main slot at 0, user-defaults slot at 128)  
**Magic byte**: 0xC1 (main), 0xD2 (user defaults)  
**Version**: 16

**Fields**:
```cpp
struct ClockSettings {
  uint8_t magic;                    // 0xC1 (validation)
  uint8_t version;                  // 11 (current)
  
  // Brightness
  uint8_t dayBrightness;            // 0-255
  uint8_t nightBrightness;          // 0-255
  uint8_t nightStartHour;           // 0-23
  uint8_t nightEndHour;             // 0-23
  
  // Display
  uint8_t colorTheme;               // 0=Classic, 1=Aqua, 2=Magenta
  uint8_t secondTrail;              // 0=off, 1=on (tints over the face, v16)
  uint8_t progressSeconds;          // 0=off, 1=on (tints over the face, v16)
  uint8_t hourlyChime;              // 0=off, 1=on
  uint8_t statusAnimations;         // 0=off, 1=on
  
  // Ring colors (RGB + level for 6 elements)
  uint8_t outerMarkerRed, outerMarkerGreen, outerMarkerBlue, outerMarkerLevel;
  uint8_t outerFillerRed, outerFillerGreen, outerFillerBlue, outerFillerLevel;
  uint8_t secondsRed, secondsGreen, secondsBlue, secondsLevel;
  uint8_t minutesRed, minutesGreen, minutesBlue, minutesLevel;
  uint8_t hoursRed, hoursGreen, hoursBlue, hoursLevel;
  uint8_t centerRed, centerGreen, centerBlue, centerLevel;
  
  // Auto-brightness (VEML7700)
  uint8_t autoBrightnessMode;       // 0=manual, 1=auto, 2=scheduled
  uint8_t minAutoBrightness;        // 5-255
  uint8_t maxAutoBrightness;        // 5-255
  
  // Time-interval animations
  uint8_t quarterAnimation;         // 0=off, 1-3=original, 4=laser ping, 5=DNA twist, 6=tick spark
  uint8_t halfHourAnimation;        // 0=off, 1-3=original, 4=comet, 5=explosion, 6=KnightRider, 7=strobe
  uint8_t hourAnimation;            // 0=off, 1-5=original, 6=supernova, 7=matrix, 8=galaxy, 9=wipe, 10=thunder
  uint8_t intervalAnimationsEnabled; // 0=off, 1=on

  // Focus Reminders (added v8)
  uint8_t focusReminder_enabled;             // 0=off, 1=on
  uint8_t focusReminder_startHour;           // 0-23 (active window start)
  uint8_t focusReminder_endHour;             // 0-23 (active window end)
  uint16_t focusReminder_intervalMinutes;    // 1-1440 (minutes between nudges)
  uint8_t focusReminder_daysMask;            // bitmask Sun(bit0)…Sat(bit6)
  uint8_t focusReminder_animation;           // 0-5=delegate to time anims; 6-11=dedicated reminder anims
  uint8_t focusReminder_durationSeconds;     // 1-60 (animation duration, added v9)
  uint32_t focusReminder_lastFireMs;         // millis() timestamp of last fire

  // Ring rotation (added v10)
  uint8_t outerRingOffset;          // 0-59: clockwise LED rotation applied to all rings at render time

  // Animation customization (added v11)
  uint8_t animationPalette;         // Shared 5-option list (v2.27.0): 0=Golden hour(warm),1=Moonlight(cool),2=Dawn(soft-warm),3=Twilight(muted-cool),7=Clock colors (default). Other values → 7.
  uint8_t animationSpeed;           // 1-5 (1=slow, 3=normal, 5=fast)
  uint8_t animationBrightness;      // 50-255 peak brightness during animations
  uint8_t trailLength;              // 2-12 LEDs (chase/sweep trail length — interval anims)
  uint8_t reminderPalette;          // Same 5-option list as animationPalette (v2.27.0): 0-3 moods, 7=Clock colors

  // ...outerRingBrightness, middleFaceScale, innerFaceScale (v12-14), darkRoomOff (v15)...

  // Second-hand accessory tuning (added v16)
  uint8_t secondTrailLength;        // 2-12 LEDs (clock-face second trail length)
  uint8_t secondTrailStyle;         // 0=classic geometric, 1=linear, 2=smooth (gamma comet)
  uint8_t progressLevel;            // 0-255 tint strength (alpha) of the progress arc over the face
  uint8_t progressStyle;            // 0=uniform arc, 1=comet gradient (brighter toward the second hand)
};
```

### Version History
- **v1-5**: Original ESP8266 versions (deprecated)
- **v6**: Initial ESP32-C3 release (March 2026)
- **v7**: Added VEML7700 auto-brightness + time-interval animations (May 2026)
- **v8**: Added Focus Reminders scheduler (May 2026); v7 settings auto-reset to v8 defaults on first boot
- **v9**: Added `focusReminder_durationSeconds` (animation duration 1-60s; repurposed from reserved block)
- **v10**: Added `outerRingOffset` (software ring rotation 0-59 LEDs; web UI "Ring rotation offset" control)
- **v11**: Added `animationPalette`, `animationSpeed`, `animationBrightness`, `trailLength`, `reminderPalette` (animation customization system)
- **v12**: Added per-ring face brightness (`outerRingBrightness`, `middleFaceScale`, `innerFaceScale`) — v2.4.2
- **v13**: v2.4.4/2.4.5 era (user-saved defaults slot added alongside)
- **v14**: Split inner-ring palettes (`middleFace*`, `innerFace*`, `innerHour*` color/level fields) — v2.4.7
- **v15**: Added `darkRoomOff` (dark-room display sleep) — v2.5.0
- **v16**: Added `secondTrailLength`, `secondTrailStyle`, `progressLevel`, `progressStyle` (second-hand accessory tuning; trail/progress now tint over the face) — v2.12.0. **First version bump with a prefix-preserving migration** — deployed units keep all their settings (no wipe).

### Bumping Settings Version
When adding fields (append at the end — never reorder existing ones):
1. Update `ClockSettings` struct (append new fields)
2. Update `SettingsStore::defaults()` with new defaults
3. Increment `SETTINGS_VERSION` by 1
4. Add a migration branch in `SettingsStore::begin()` (see the v15→v16 case): keep the old struct prefix, seed the new fields, re-stamp the version. This preserves every existing setting — **no wipe**. (Omitting the migration falls back to a full reset to `defaults()` on next boot, the pre-v16 behavior.)

---

## Stability / OTA Infrastructure (Maturation Track)

Target state: device is flashable, debuggable, and verifiable entirely over WiFi with
no USB cable or serial monitor required.

### Done
- ✅ **Task 2** — `WiFi.setAutoReconnect(true)` confirmed in `setupWiFi()`
- ✅ **Task 3** — `ArduinoOTA.onError()` calls `ESP.restart()` on stall
- ✅ **Task 4** — Software watchdog: `esp_task_wdt` 10s window in `loop()`
- ✅ **Web UI firmware update** — `/update` page accepts `.bin` upload; fixed FormData + `UPDATE_SIZE_UNKNOWN`

### Planned
- ✅ **Task 1** -- `/diag` endpoint: uptime, firmware version, boot reason, free heap, WiFi stats, NTP sync status, NTP last delta, button event count (2026-05-13)
- ✅ **Task 6** -- Physical buttons re-added: GPIO9(UP)/GPIO5(DOWN), polled, 50ms debounce, `ButtonInput` class (2026-05-13)
- ✅ **Task 5** -- Button-hold factory reset on boot (2026-05-13): hold UP at power-on → red LEDs; add DOWN, hold both 3s → clears EEPROM, blue LEDs while portal broadcasts `esp32c3-clock-setup`
