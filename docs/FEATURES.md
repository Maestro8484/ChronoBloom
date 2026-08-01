# ChronoBloom ESP32-C3 -- Feature Catalog

## Current Features (Implemented ✅)

### Time Display
- ✅ **Analog 3-ring clock** -- Hours/minutes/seconds shown via LED position
- ✅ **Per-ring color customization** -- RGB color pickers for 9 elements (outer marker, outer fill, seconds, minutes, hours, middle face, inner face, inner hour, center)
- ✅ **Per-ring brightness control** -- Independent intensity sliders (0-255)
- ✅ **Quarter-hour markers** -- Every 5th LED on outer ring
- ✅ **Progress fill** -- Dim arc showing seconds elapsed
- ✅ **Fading trail** -- 4-LED trail behind second hand
- ✅ **Petal depth on all three rings** (v2.30.0) -- one 0-100 slider shades every ring's fill with the same crest-to-seam profile, so the face reads as rows of petals instead of flat glow. Crests align into four radial spokes at 12/3/6/9; the outer ring's 5-minute markers are never shaded so the hour marks stay crisp. 0 = flat fill (the old off). Per-theme depths ship in `tools/themes/themes.json`
- ✅ **Center status pixel** -- Breathing idle animation, status indicator

### Brightness Control
- ✅ **Manual brightness** -- Day/night sliders
- ✅ **Scheduled brightness** -- Auto-switch at configurable hours
- ✅ **VEML7700 auto-brightness** -- Lux-based logarithmic curve
- ✅ **Three modes** -- Manual / Auto (sensor) / Scheduled (day/night)
- ✅ **Two-slider auto response: Darkest / Brightest** (v2.30.0) -- the lux curve is span-mapped onto exactly [Darkest..Brightest]: pitch black lands on the Darkest slider, full daylight on the Brightest slider, and neither can be silently defeated. Replaces the retired v18 "Overall dimness" gain, whose label read backwards and whose pre-clamp multiply could pin the face at the floor with the sensor ignored. Migration folds the stored gain into Brightest so no deployed unit's daylight peak jumps
- ✅ **Dark-room display sleep** (v2.5.0) -- Auto mode + `darkRoomOff`: 30 s below 0.3 lux blanks all LEDs, wakes above 2.0 lux

### Animations
- ✅ **Hourly chime** -- Sweep + center pulse (8 sec)
- ✅ **Status animations** -- WiFi connect, button press, NTP sync, settings save
- ✅ **Center idle breathing** -- Slow pulse when no status active
- ✅ **Time-interval animations** -- Escalating intensity at :15/:30/:00 (v2.4.0 overhaul: 29 legacy modes replaced with 16 palette-aware animations)
  - Quarter-hour (:15/:45, modes 1-3): Slow comet, Dual orbit, Bloom ripple
  - Half-hour (:30, modes 1-3): Unfurl, Three comets, Breathe
  - Top of hour (:00, modes 1-5): Ceremony, Galaxy spin, Supernova, Comet relay, Deep breath
- ✅ **Animation palette system** (simplified v2.27.0) -- **one shared 5-option list** applies to both interval chimes and reminders: `Clock colors (default)` (ring-mapped from the configured face colors) + 4 temperature/mood palettes **Golden hour (warm)**, **Moonlight (cool)**, **Dawn (soft-warm)**, **Twilight (muted-cool)**. Each mood is **four explicit per-ring colors** (outer/middle/inner/center) rendered solid per ring -- the same model as Clock colors. Since v2.31.1 each mood **travels hue from ring to ring** (outer rim to center core) while brightening inward; before that they were single-hue lightness ramps, which made every bloom animation read as one flat wash. Keep the hue travel when re-tuning -- it is the point. Source of truth `tools/palettes/palettes.json` (`mood_palettes` -> `rings`) -> `tools/gen_palettes.py` -> `src/anim_palettes.h`. Values are authored **gamma-aware** (route through `gammaColor`, gamma 2.2); the `target` field records the intended on-strip appearance and `rings` is its inverse-gamma. Try candidates on a live clock without flashing via `scripts/tune_palette.py`. The 10 prior flower palettes were removed and archived (`docs/archive/flower_palettes_2026-07-14.md`)
- ✅ **Animation style controls** -- Speed (1-5), peak brightness (50-255), trail length (2-12 LEDs)
- ✅ **Reminder palette** -- an **independent selection** from the same 5-option list (Clock colors + 4 moods), so a reminder/nudge can read differently from an interval chime. Applies to every reminder-triggered animation incl. nudge modes 0-2, which delegate to the chime animations

### Smart Features
- ✅ **NTP time sync** -- Network Time Protocol with timezone/DST support
- ✅ **Runtime time zone** (v2.28.0) -- set in the web UI (Time & light -> Time zone): 18 common zones plus an "Other" field taking any POSIX TZ string. Stored in NVS (`clock`/`tz`), not in `ClockSettings`, so no settings migration; `NTP_TIMEZONE_TZ` in `platformio.ini` is only the default for a fresh or factory-reset unit. Applies immediately with no reboot and no NTP round trip, since the ESP keeps the system clock in UTC and only `tzset()` has to re-run. Rejected strings return 400 rather than silently meaning UTC
- ✅ **First-run timezone banner** (v2.31.0) -- a freshly flashed clock prompts for its time zone in the web UI instead of silently running in the build default, so the first thing a new builder sees is the right time
- ✅ **Time-truth cue** (v2.31.0) -- until NTP syncs or the time is set by hand, a short amber inner-ring chase fires every 10 s, so unconfirmed time looks different from set time. Respects the status-blips toggle
- ✅ **Offline DST tick** (v2.31.0) -- local time is re-derived from the epoch once a minute while WiFi is down, so a DST change lands on time offline. Suppressed after a manual time set
- ✅ **Web UI** -- Full-featured control interface with live preview
- ✅ **Unified Save** (v2.31.0) -- the main Save button now writes the reminder and animation-style fields too, so one press covers the whole settings set. The sectional save buttons remain as conveniences. Time zone is the exception: it has its own Save zone button; the main Save does not write it
- ✅ **Browser time sync** -- One-click sync from phone/computer clock
- ✅ **EEPROM persistence** -- All settings survive power cycles (settings schema v19; see Settings Structure below)
- ✅ **WiFi web server** -- mDNS hostname (esp32c3-v3-8inch.local)
- ✅ **Live SVG preview** -- Clock updates every 90ms in web UI
- ✅ **Animation toggles** -- Enable/disable individual effects
- ✅ **Theme selection** -- 7 presets: ChronoBloom (clock default), Moonflower, Cherry Blossom, Ember Dahlia, Lotus Pond, Sunflower, Bird of Paradise. Each sets the full face color set plus animation style in one click
- ✅ **Manual time adjustment** -- H:M:S entry and +/- minute controls via WebUI
- ✅ **Improv WiFi (v2.29.0)** -- Browser-configured WiFi over USB serial at flash time (official improv-wifi/sdk-cpp protocol, `improv/Improv @ 1.2.6`). Window on an unprovisioned device before falling back to the captive portal: 10s of silence closes it, but any complete Improv frame from a browser holds it open another 60s, so the window survives a user reading the dialog and typing a password (v2.31.4). Serviced only while unprovisioned/AP-fallback, never once connected
- ✅ **WiFi provisioning portal** -- Captive portal on first boot; AP fallback at 192.168.4.1 if STA unavailable. Portal windows (v2.31.0): 10 min on a never-provisioned first boot, 2 min on a previously-connected unit, 15 min after factory reset (then falls through to AP mode)
- ✅ **OTA firmware updates** -- ArduinoOTA (espota) and web UI `/update` page; no USB cable required after first flash
- ✅ **mDNS reconnect** -- Hostname re-advertised automatically after WiFi reconnect

### Physical Buttons
- ✅ **Physical UP/DOWN buttons** -- GPIO5(UP) / GPIO9(DOWN), polled (no ISRs), 50ms debounce
  - Short press: +1/-1 minute
  - Hold >500ms: auto-repeat +1/-1 minute every 150ms
  - Hold >2000ms: switches to +60/-60 minutes per fire (hour jump)
  - Release: repeat stops immediately
  - No-WiFi time adjustment without WebUI

### Focus Reminders
- ✅ **Focus Reminders (ADHD support)** -- Visual nudge animations at configurable intervals to interrupt hyperfocus
  - Single configurable rule: interval (1-1440 min), active hours window, days-of-week bitmask
  - Modes 0-2: delegate to existing time-interval animations (quarter/half/hour slots)
  - Modes 6-11: dedicated reminder animations (Gentle pulse, Orbiting orb, Ripple in, Heartbeat, Slow bloom, Firefly)
  - All reminder animations use the reminder palette, capped by a shared `NUDGE_CEIL` (205/255) so a nudge reads as a swell, never an alert flash -- an ask, not an alarm
  - Enable/disable toggle
  - Configurable per-day schedule (Sun-Sat)
  - **Gentle escalation** (v2.31.0) -- from the second unacknowledged nudge on, the same swell plays a second time ~6 s after the first: it asks twice, at the same brightness, never louder. Before v2.31.0 every nudge was identical and played once
  - **Button acknowledgment** (v2.31.0) -- pressing either physical button during a nudge or within ~15 s after acknowledges it: escalation clears and the interval restarts. The ack press does not adjust the time; later presses do, as usual
  - **Quick nudge control** (v2.31.0) -- a Focus nudges row in the web UI Quick controls: on/off + interval + save, two taps from landing
  - **Preset buttons** (v2.31.0) -- Pomodoro 25m / Check-in 30m / Hourly buttons in the reminder section fill the common setups in one click
  - **24h window** (v2.31.0) -- start hour == end hour now means always-on (previously a zero-width window that never fired)
  - **Factory default days: all 7** (v2.31.0) -- a fresh unit that enables reminders gets nudges immediately instead of a silently empty days mask
  - Fire timestamp stored in RAM only -- not persisted to EEPROM (field `focusReminder_lastFireMs` reserved in struct but unused since v2.1.1)
  - Web UI panel: "Focus Reminders (ADHD)"

### Demo Mode
- ✅ **Demo Mode** -- press **Demo** in the web UI and the clock plays itself: a hands-off tour through the chimes, the hour animations, the nudges and the auto-brightness range, then returns to telling the time. Good for showing someone what it does without waiting for the top of the hour. Non-blocking state machine, so the clock stays responsive throughout
  - 9-step sequence (`DemoMode::steps[]` in `src/main.cpp`): title card (8.8s), quarter chime (5.6s), half-hour chime (11s), top-of-hour chime (17s), hour animation showcase (36.5s), focus reminder nudges (20s), center LED status (6.8s), auto-brightness cycles (14.85s), open-source end card (10.85s)
  - Total runtime: ~131 seconds. The animation steps are event-paced and depend on which styles are configured, so per-step figures are estimates at `animationSpeed` 3, not exact bounds; each transition includes a ~0.8s dissolve and breather
  - LuxSensor override: simulates two quick room-darkening/brightening cycles to demo the faster (150ms poll) auto-brightness response
  - **Pre-roll** (v2.30.0): `POST /demo/start?delay=<seconds>` (0-60) holds the clock fully dark before the demo opens, so a camera can be rolling and settled and frame one is a fade-up out of clean black. Handled on-device, so the countdown survives the browser tab closing; web UI has a Pre-roll seconds box with live countdown
  - Web endpoints: `POST /demo/start`, `POST /demo/stop`, `POST /demo/brightnessCycle` (auto-brightness sweep standalone), `GET /demo/status`, `GET /demo/overlay`
  - `/demo/status` returns: active state, pre-roll state (`preroll` + `preroll_ms`), current step, subtitle, elapsed time, step duration
  - `/demo/overlay` HTML: full-screen 1920x1080, OBS-ready browser source with fade transitions; stays blank during pre-roll so no subtitle lands on the black lead-in
  - Web UI controls: Start/Stop buttons, live status display (step counter + progress bar)
  - Buttons ignored during demo; all web endpoints continue normal operation
  - No settings version change, no EEPROM impact

### Web UI Features
- ✅ **Time controls** -- Manual set, browser sync, NTP sync, increment/decrement (WebUI); physical buttons re-added v2.0.6 (GPIO5=UP, GPIO9=DOWN)
- ✅ **Display settings** -- Day/night brightness, schedule hours
- ✅ **Ring controls** -- 9 color elements: outer marker, outer fill, seconds, minutes, hours, middle face, inner face, inner hour, center (levels where applicable), plus the petal depth slider
- ✅ **Animation controls** -- Second trail, progress ring, hourly chime, status animations, interval animations, palette/speed/brightness/trail controls
- ✅ **Animation preview** -- Preview button on every animation selector; Animation Style panel with live preview by type
- ✅ **12-hour AM/PM display** -- Clock header shows 12h format with AM/PM
- ✅ **Live sensor data** -- Current lux reading updates every 2 seconds
- ✅ **Auto-brightness controls** -- Mode selection, min/max limits
- ✅ **Network info** -- IP address, WiFi SSID, signal strength, NTP sync status
- ✅ **Demo Mode controls** -- Start/Stop buttons, live status display for video recording sequences

---

## Planned Features (Priority Order)

### High Priority
- [ ] **Sunrise/sunset detection** -- VEML7700 detects daylight transitions, triggers warm fade animations
- [ ] **Holiday auto-animations** -- Date-triggered effects (Christmas, Halloween, New Year, Easter, user birthday)
- [ ] **Multiple Focus Reminder rules** -- 3-5 concurrent reminder rules

### Medium Priority
- [ ] **Theme presets** -- Save/load entire color schemes to EEPROM slots (named presets)
- [ ] **BME280 temp/humidity sensor** -- Color-coded center pixel (blue=cold, red=hot), web UI display
- [ ] **MQTT for Home Assistant** -- Publish state, subscribe to commands
- [ ] **Sunrise alarm** -- 5-minute gradual brightness increase simulating dawn
- [ ] **Pomodoro timer mode** -- 25/5 work/break visual countdown on outer ring

### Low Priority
- [ ] **Lux history graph** -- 60-minute trend chart in web UI (canvas or SVG)
- [ ] **Circadian rhythm color shift** -- Warm orange evening, cool blue morning (auto-adjusts based on lux)
- [ ] **Motion detection proxy** -- Sudden lux spike = person approaching, trigger welcome animation
- [x] **Power saving mode** -- Done v2.5.0 as dark-room display sleep (`darkRoomOff`)
- [ ] **Multi-clock network sync** -- Master/slave mode for synchronized animations across multiple clocks
- [ ] **LED mapping test mode** -- `/ledtest?pixel=N` and `/ledwalk` endpoints for physical verification
- [ ] **Animation playlist** -- Auto-rotate effects every N minutes in demo mode
- [ ] **Custom schedules** -- Different animations at different times/days
- [ ] **Voice control** -- Alexa/Google Assistant via MQTT bridge

---

## Removed Features (Was Implemented, Now Removed)

**Physical buttons (GPIO3/GPIO4)** -- removed v2.0.4, re-added v2.0.6 on safe pins
- Originally GPIO3/GPIO4 (JTAG TCK/TDI): spurious ISR fires with USB connected -> removed v2.0.4
- Re-added v2.0.6 on GPIO5(UP)/GPIO9(DOWN) using polled reads, no ISRs
- v2.0.7: GPIO swap to GPIO5=UP, GPIO9=DOWN; hold-to-repeat added (500ms->1min/150ms, 2s->60min/fire)
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

## Settings Structure (EEPROM v19)

> The struct sketch below is a summary; `src/main.cpp` (`struct ClockSettings`,
> `SETTINGS_VERSION`) is authoritative -- `docs/API.md` carries the full
> regenerated field list. Additions since v11:
> `middleFace*`/`innerFace*`/`innerHour*` colors, `outerRingBrightness`,
> `middleFaceScale`, `innerFaceScale` (v12-v14), `darkRoomOff` (v15),
> `secondTrailLength`/`secondTrailStyle`/`progressLevel`/`progressStyle` (v16),
> petal flag (v17), auto-brightness gain (v18, retired), petal depth 0-100 +
> gain fold (v19). A second EEPROM slot at offset 128 stores user-saved
> defaults (v2.4.5+, magic 0xD2); compile-time static_asserts guard both
> layout boundaries.

### Stored Configuration
**Total size**: 256 bytes (main slot at 0, user-defaults slot at 128)  
**Magic byte**: 0xC1 (main), 0xD2 (user defaults)  
**Version**: 19

**Fields**:
```cpp
struct ClockSettings {
  uint8_t magic;                    // 0xC1 (validation)
  uint8_t version;                  // 19 (SETTINGS_VERSION)
  
  // Brightness
  uint8_t dayBrightness;            // 0-255
  uint8_t nightBrightness;          // 0-255
  uint8_t nightStartHour;           // 0-23
  uint8_t nightEndHour;             // 0-23
  
  // Display
  uint8_t centerSource;             // 0-4: idle center LED role (reuses the retired colorTheme byte)
  uint8_t secondTrail;              // 0=off, 1=on (tints over the face, v16)
  uint8_t progressSeconds;          // 0=off, 1=on (tints over the face, v16)
  uint8_t hourlyChime;              // 0=off, 1=on
  uint8_t statusAnimations;         // 0=off, 1=on
  
  // Ring colors (9 elements; middleFace/innerFace are RGB only -- their levels
  // are the *FaceScale fields below. Full layout in docs/API.md)
  uint8_t outerMarkerRed, outerMarkerGreen, outerMarkerBlue, outerMarkerLevel;
  uint8_t outerFillerRed, outerFillerGreen, outerFillerBlue, outerFillerLevel;
  uint8_t secondsRed, secondsGreen, secondsBlue, secondsLevel;
  uint8_t minutesRed, minutesGreen, minutesBlue, minutesLevel;
  uint8_t hoursRed, hoursGreen, hoursBlue, hoursLevel;         // middle 24h hand
  uint8_t middleFaceRed, middleFaceGreen, middleFaceBlue;      // v14
  uint8_t innerFaceRed, innerFaceGreen, innerFaceBlue;         // v14
  uint8_t innerHourRed, innerHourGreen, innerHourBlue, innerHourLevel; // v14
  uint8_t centerRed, centerGreen, centerBlue, centerLevel;
  
  // Auto-brightness (VEML7700)
  uint8_t autoBrightnessMode;       // 0=manual, 1=auto, 2=scheduled
  uint8_t minAutoBrightness;        // 5-255
  uint8_t maxAutoBrightness;        // 5-255
  
  // Time-interval animations (v2.4.0 set; higher legacy modes are sanitized away)
  uint8_t quarterAnimation;         // 0=off, 1=Slow comet, 2=Dual orbit, 3=Bloom ripple
  uint8_t halfHourAnimation;        // 0=off, 1=Unfurl, 2=Three comets, 3=Breathe
  uint8_t hourAnimation;            // 0=off, 1=Ceremony, 2=Galaxy spin, 3=Supernova, 4=Comet relay, 5=Deep breath
  uint8_t intervalAnimationsEnabled; // 0=off, 1=on

  // Focus Reminders (added v8)
  uint8_t focusReminder_enabled;             // 0=off, 1=on
  uint8_t focusReminder_startHour;           // 0-23 (active window start)
  uint8_t focusReminder_endHour;             // 0-23 (active window end)
  uint16_t focusReminder_intervalMinutes;    // 1-1440 (minutes between nudges)
  uint8_t focusReminder_daysMask;            // bitmask Sun(bit0)..Sat(bit6); factory default all days since v2.31.0
  uint8_t focusReminder_animation;           // 0-5=delegate to time anims; 6-11=dedicated reminder anims
  uint8_t focusReminder_durationSeconds;     // 1-60 (added v9). RESERVED: accepted+stored, not yet used
  uint32_t focusReminder_lastFireMs;         // INTERNAL: RAM-only bookkeeping, ignore

  // Ring rotation (added v10)
  uint8_t outerRingOffset;          // 0-59: clockwise LED rotation applied to all rings at render time

  // Animation customization (added v11)
  uint8_t animationPalette;         // Shared 5-option list (v2.27.0): 0=Golden hour(warm),1=Moonlight(cool),2=Dawn(soft-warm),3=Twilight(muted-cool),7=Clock colors (default). Other values -> 7.
  uint8_t animationSpeed;           // 1-5 (1=slow, 3=normal, 5=fast)
  uint8_t animationBrightness;      // 50-255 peak brightness during animations
  uint8_t trailLength;              // 2-12 LEDs (chase/sweep trail length; interval anims)
  uint8_t reminderPalette;          // Same 5-option list as animationPalette (v2.27.0): 0-3 moods, 7=Clock colors

  // ...outerRingBrightness, middleFaceScale, innerFaceScale (v12-14), darkRoomOff (v15)...

  // Second-hand accessory tuning (added v16)
  uint8_t secondTrailLength;        // 2-12 LEDs (clock-face second trail length)
  uint8_t secondTrailStyle;         // 0=classic geometric, 1=linear, 2=smooth (gamma comet)
  uint8_t progressLevel;            // 0-255 tint strength (alpha) of the progress arc over the face
  uint8_t progressStyle;            // 0=uniform arc, 1=comet gradient (brighter toward the second hand)

  // Petal depth (added v17 as an on/off flag, widened to 0-100 in v19)
  uint8_t petalDepth;               // 0-100 seam shading across all three rings; 0=flat fill

  // Retired v19: held autoBrightnessGain (v18); folded into maxAutoBrightness
  // on migration, byte kept zeroed to preserve the layout
  uint8_t reservedGain;
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
- **v12**: Added per-ring face brightness (`outerRingBrightness`, `middleFaceScale`, `innerFaceScale`) -- v2.4.2
- **v13**: v2.4.4/2.4.5 era (user-saved defaults slot added alongside)
- **v14**: Split inner-ring palettes (`middleFace*`, `innerFace*`, `innerHour*` color/level fields) -- v2.4.7
- **v15**: Added `darkRoomOff` (dark-room display sleep) -- v2.5.0
- **v16**: Added `secondTrailLength`, `secondTrailStyle`, `progressLevel`, `progressStyle` (second-hand accessory tuning; trail/progress now tint over the face) -- v2.12.0. **First version bump with a prefix-preserving migration** -- deployed units keep all their settings (no wipe).
- **v17**: Added `petalMode` (petal texture on/off flag) -- v2.25.0
- **v18**: Added `autoBrightnessGain` (retired one version later; its pre-clamp multiply could pin the face at the floor) -- v2.26.0
- **v19**: `petalMode` widened to `petalDepth` (0-100, all three rings, one shared profile); gain retired to `reservedGain` (zeroed, folded into `maxAutoBrightness` on migration so no unit's daylight peak jumps). Same offsets, no struct growth, no wipe -- v2.30.0

### Bumping Settings Version
When adding fields (append at the end -- never reorder existing ones):
1. Update `ClockSettings` struct (append new fields)
2. Update `SettingsStore::defaults()` with new defaults
3. Increment `SETTINGS_VERSION` by 1
4. Add a migration branch in `SettingsStore::begin()` (see the v15->v16 case): keep the old struct prefix, seed the new fields, re-stamp the version. This preserves every existing setting -- **no wipe**. (Omitting the migration falls back to a full reset to `defaults()` on next boot, the pre-v16 behavior.)

---

## Stability / OTA Infrastructure (Maturation Track)

Target state: device is flashable, debuggable, and verifiable entirely over WiFi with
no USB cable or serial monitor required.

### Done
- ✅ **Task 2** -- `WiFi.setAutoReconnect(true)` confirmed in `setupWiFi()`
- ✅ **Task 3** -- `ArduinoOTA.onError()` calls `ESP.restart()` on stall
- ✅ **Task 4** -- Software watchdog: `esp_task_wdt` 10s window in `loop()`
- ✅ **Web UI firmware update** -- `/update` page accepts `.bin` upload; fixed FormData + `UPDATE_SIZE_UNKNOWN`

### Planned
- ✅ **Task 1** -- `/diag` endpoint: uptime, firmware version, boot reason, free heap, WiFi stats, NTP sync status, NTP last delta, button event count (2026-05-13)
- ✅ **Task 6** -- Physical buttons re-added: GPIO9(UP)/GPIO5(DOWN), polled, 50ms debounce, `ButtonInput` class (2026-05-13)
- ✅ **Task 5** -- Button-hold factory reset on boot (2026-05-13): hold UP at power-on -> red LEDs; add DOWN, hold both 3s -> clears EEPROM, blue LEDs while portal broadcasts `esp32c3-clock-setup`
