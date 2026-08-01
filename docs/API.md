# ChronoBloom ESP32-C3 -- Web API Reference

> Ground truth: `src/main.cpp` (`setupRoutes()`, `setupDemoModeRoutes()`, `struct ClockSettings`).
> If this document and the source disagree, the source wins and this file is stale.
> Schema below matches `SETTINGS_VERSION 19` (firmware v2.30.x/v2.31.0 era).

## HTTP Endpoints

**Base URL**: `http://esp32c3-v3-8inch.local/` (or direct IP address)

Example IPs in this document use the documentation range `192.0.2.x`; your device will have a real LAN address (check `/net` or your router).

---

## GET Endpoints

### `GET /`
**Description**: Serve main web UI page

**Response**: HTML page with embedded JavaScript and CSS

**Features**:
- Live SVG clock preview (updates every 90ms)
- Time controls (manual set, browser sync, NTP sync, +/- minute, time zone)
- Display settings (brightness, schedule, themes)
- Ring color pickers (9 elements: outer marker, outer fill, seconds, minutes, hours, middle face, inner face, inner hour, center)
- Petal depth slider (0-100, all three rings)
- Animation controls (second trail, progress ring, chimes, interval animations, Animation Style panel with preview)
- Auto-brightness controls (mode, Darkest/Brightest sliders, live lux display)
- Focus Reminders panel
- Demo mode controls (start/stop, pre-roll, live status)
- Network info panel

---

### `GET /time`
**Description**: Get current clock time and status

**Response**:
```json
{
  "hour": 14,
  "minute": 32,
  "second": 18,
  "ntpSynced": true,
  "wifi": true,
  "ip": "192.0.2.100"
}
```

**Polling**: Web UI polls this every 1 second

---

### `GET /temperature`
**Description**: Get temperature sensor reading (future)

**Response** (when sensor available):
```json
{
  "available": true,
  "celsius": 22.5
}
```

**Response** (when sensor not available):
```json
{
  "available": false
}
```

**Note**: Currently placeholder. BME280/SHT31 sensor not yet implemented.

---

### `GET /lux`
**Description**: Get ambient light sensor reading

**Response** (when VEML7700 available):
```json
{
  "available": true,
  "lux": 123.4,
  "autoBrightness": 150
}
```

**Response** (when sensor not available):
```json
{
  "available": false
}
```

**Polling**: Web UI polls this every 2 seconds when auto-brightness mode active

**Fields**:
- `lux`: Current light level (0-120,000 lux range, auto-gain)
- `autoBrightness`: Calculated brightness value (0-255) based on logarithmic curve

---

### `GET /lux/override`
**Description**: Diagnostic lux override -- pin the sensor to any value so the brightness response can be swept remotely, or release it back to the real sensor

**Parameters** (query string, one required):
- `lux` (float, >= 0): hold the sensor at this value
- `clear` (any value): release the override

**Response**:
- `{"override":true,"lux":42.0}` when set
- `{"override":false}` when cleared
- `{"available":false}` when no sensor
- `400 {"error":"need ?lux=X or ?clear=1"}` when neither parameter given

---

### `GET /net`
**Description**: Get network information

**Response**:
```json
{
  "hostname": "esp32c3-v3-8inch",
  "ssid": "YOUR_SSID",
  "ip": "192.0.2.100",
  "gateway": "192.0.2.1",
  "subnet": "255.255.255.0",
  "dns": "192.0.2.1",
  "rssi": -45,
  "status": 3
}
```

**Fields**:
- `rssi`: WiFi signal strength in dBm (typical range: -30 to -80)
- `status`: WiFi connection status code (3 = WL_CONNECTED)

---

### `GET /diag`
**Description**: One-call diagnostics: uptime, firmware and settings version, time, NTP state, WiFi state, lux and the full brightness chain, ring levels, power-limiter state, animation state, timezone, save count

**Response fields** (JSON): `uptime_sec`, `firmware_version`, `settings_version`, `time`, `ntp_synced`, `ntp_last_delta_sec`, `wifi_status`, `wifi_ssid`, `wifi_rssi`, `wifi_ip`, `lux`, `brightness_target`, `brightness_ramped`, `effective_brightness`, `outer_marker_level`, `outer_filler_level`, `hours_level`, `middle_hour_level`, `inner_hour_level`, `center_level`, `middle_ambient_scale`, `inner_ambient_scale`, `button_event_count`, `free_heap`, `clock_pixel_count`, `ring_pixel_offset`, `default_outer_ring_offset`, `outer_ring_offset`, `sacrificial_enabled`, `anim_phase`, `last_anim_source`, `last_anim_mode`, `display_sleep`, `reel_active`, `master_fade`, `est_milliamps`, `limiter_brightness`, `max_milliamps`, `timezone`, `settings_save_count`

`effective_brightness` is computed through the same `autoSpanMap()` the renderer uses, so `/diag` can never disagree with what the LEDs are doing.

---

### `GET /settings`
**Description**: Get all current settings

**Response** (all keys, values are examples):
```json
{
  "dayBrightness": 44,
  "nightBrightness": 5,
  "nightStartHour": 22,
  "nightEndHour": 7,
  "centerSource": 0,
  "secondTrail": 0,
  "progressSeconds": 0,
  "hourlyChime": 1,
  "statusAnimations": 1,
  "outerMarkerColor": "#6EB9FF",
  "outerMarkerLevel": 225,
  "outerFillerColor": "#0008C8",
  "outerFillerLevel": 145,
  "secondsColor": "#64FFB4",
  "secondsLevel": 230,
  "minutesColor": "#FF6400",
  "minutesLevel": 220,
  "hoursColor": "#DC00B4",
  "hoursLevel": 255,
  "middleFaceColor": "#DC00B4",
  "innerFaceColor": "#FF3C00",
  "innerHourColor": "#6EB9FF",
  "innerHourLevel": 255,
  "centerColor": "#FF3C00",
  "centerLevel": 180,
  "autoBrightnessMode": 1,
  "minAutoBrightness": 10,
  "maxAutoBrightness": 153,
  "quarterAnimation": 3,
  "halfHourAnimation": 1,
  "hourAnimation": 4,
  "intervalAnimationsEnabled": 1,
  "focusReminder_enabled": 0,
  "focusReminder_startHour": 8,
  "focusReminder_endHour": 22,
  "focusReminder_intervalMinutes": 60,
  "focusReminder_daysMask": 0,
  "focusReminder_animation": 0,
  "focusReminder_durationSeconds": 60,
  "outerRingOffset": 0,
  "animationPalette": 7,
  "animationSpeed": 3,
  "animationBrightness": 157,
  "trailLength": 6,
  "reminderPalette": 0,
  "outerRingBrightness": 77,
  "middleFaceScale": 39,
  "innerFaceScale": 39,
  "darkRoomOff": 0,
  "secondTrailLength": 4,
  "secondTrailStyle": 2,
  "progressLevel": 90,
  "progressStyle": 0,
  "petalDepth": 45,
  "timezone": "MST7MDT,M3.2.0,M11.1.0",
  "tzConfigured": true,
  "hasUserDefaults": false
}
```

**Color format**: Hex RGB (`#RRGGBB`)
**Level values**: 0-255 (brightness intensity for each element)
**Mode enums**: See Settings Structure section below
**`tzConfigured`** (v2.31.0): read-only. `true` once a time zone has been saved to NVS; drives the first-run timezone banner in the web UI. `POST /settings` does not accept it -- strip it before re-POSTing a saved settings backup.

---

### `GET /anim/status`
**Description**: Whether an animation is currently playing, and which phase. Lets an external sequencer (the demo reel designer's Live Run) event-pace triggers instead of guessing wall-clock durations.

**Response**: `{"animating":true,"phase":"HR2"}`

**CORS**: This is the one CORS-open endpoint (`Access-Control-Allow-Origin: *`) -- read-only status, no side effects, and the designer tool runs from a different origin.

---

### `GET /wifi`
**Description**: WiFi settings page (HTML). Shows the saved SSID and current connection status; form posts to `POST /wifi`.

---

### `GET /update`
**Description**: Firmware update page (HTML). Shows the running firmware version and accepts a `.bin` upload via `POST /update`.

---

### `GET /demo/status`
**Description**: Demo mode state for the web UI and overlay

**Response** (running): `active`, `preroll` (bool), `preroll_ms` (remaining pre-roll, when pre-rolling), `step`, `steps`, `subtitle`, `elapsed_ms`, `step_duration_ms`
**Response** (idle): `{"active":false}`

---

### `GET /demo/overlay`
**Description**: Full-screen 1920x1080 OBS-ready browser source (HTML) with fade transitions; mirrors `/demo/status` subtitles. Stays blank during pre-roll so no subtitle is composited onto the black lead-in.

---

## POST Endpoints

### `POST /set`
**Description**: Manually set clock time

**Parameters** (application/x-www-form-urlencoded):
- `hour` (int, 0-23, required)
- `minute` (int, 0-59, required)
- `second` (int, 0-59, required)

**Example**:
```
POST /set
Content-Type: application/x-www-form-urlencoded

hour=14&minute=30&second=0
```

**Response**: `200 OK "ok"` or `400 Bad Request "missing hour/minute/second"`

**Side effects**: Triggers STATUS_TIME_SYNC animation (1.2 sec)

---

### `POST /syncBrowser`
**Description**: Sync clock to browser's local time

**Parameters** (application/x-www-form-urlencoded):
- `hour` (int, 0-23, required)
- `minute` (int, 0-59, required)
- `second` (int, 0-59, required)

**Example**:
```javascript
const d = new Date();
fetch('/syncBrowser', {
  method: 'POST',
  headers: {'Content-Type': 'application/x-www-form-urlencoded'},
  body: `hour=${d.getHours()}&minute=${d.getMinutes()}&second=${d.getSeconds()}`
});
```

**Response**: `200 OK "ok"` or `400 Bad Request "missing hour/minute/second"`

**Side effects**: Triggers STATUS_TIME_SYNC animation (1.5 sec)

---

### `POST /syncNtp`
**Description**: Force immediate NTP time sync

**Parameters**: None

**Response**: `200 OK "ok"` or `503 Service Unavailable "ntp unavailable"`

**Side effects**:
- Triggers STATUS_TIME_SYNC animation (1.5 sec) on success
- Triggers STATUS_WIFI_FAIL animation (1.5 sec) on failure

**Failure conditions**:
- WiFi not connected
- NTP server unreachable
- Time received is before cutoff (< Jan 2024)

---

### `POST /addMinute`
**Description**: Increment time by 1 minute

**Parameters**: None

**Response**: `200 OK "ok"`

**Side effects**: Triggers STATUS_BUTTON animation (0.7 sec)

---

### `POST /subMinute`
**Description**: Decrement time by 1 minute

**Parameters**: None

**Response**: `200 OK "ok"`

**Side effects**: Triggers STATUS_BUTTON animation (0.7 sec)

---

### `POST /previewAnimation`
**Description**: Fire an animation immediately for preview -- does not modify saved settings

**Parameters** (application/x-www-form-urlencoded):
- `type` (string, required): `quarter` | `halfhour` | `hour` | `reminder`
- `mode` (int, required): animation index within the type
  - `quarter`: 1-3
  - `halfhour`: 1-3
  - `hour`: 1-5
  - `reminder`: 0-11

**Optional style overrides** (non-persistent, apply to this preview only, auto-cleared when the animation ends):
- `palette` (0-3 moods, 7 = clock colors)
- `speed` (1-5)
- `brightness` (50-255)
- `trail` (2-12)
- `reminderPalette` (0-3 moods, 7 = clock colors)

**Example**:
```
POST /previewAnimation
Content-Type: application/x-www-form-urlencoded

type=hour&mode=4
```

**Response**: `200 OK "ok"` | `400 Bad Request "<error>"`

**Side effects**: Triggers the specified animation on the LED rings immediately. No settings are written -- the old flow persisted the style via `POST /settings` before every preview, which cost an EEPROM flash commit per click.

---

### `POST /settings`
**Description**: Update display settings. Only keys present in the request are touched; everything else keeps its saved value.

**Parameters** (application/x-www-form-urlencoded, all optional):

**Brightness**:
- `dayBrightness` (int, 0-255)
- `nightBrightness` (int, 0-255)
- `nightStartHour` (int, 0-23)
- `nightEndHour` (int, 0-23)

**Display**:
- `centerSource` (int, 0-4: 0=status + bloom breathing, 1=bloom only, 2=status only, 3=temperature when a sensor exists, 4=off)
- `secondTrail` (int, 0-1)
- `progressSeconds` (int, 0-1)
- `hourlyChime` (int, 0-1)
- `statusAnimations` (int, 0-1)

**Ring Colors** (hex format `#RRGGBB`):
- `outerMarkerColor`, `outerFillerColor`, `secondsColor`, `minutesColor`, `hoursColor`, `middleFaceColor`, `innerFaceColor`, `innerHourColor`, `centerColor`

**Ring Levels** (int, 0-255):
- `outerMarkerLevel`, `outerFillerLevel`, `secondsLevel`, `minutesLevel`, `hoursLevel`, `innerHourLevel`, `centerLevel`

**Auto-Brightness**:
- `autoBrightnessMode` (int, 0-2: 0=manual, 1=auto, 2=scheduled)
- `minAutoBrightness` (int, 5-255) -- the Darkest slider
- `maxAutoBrightness` (int, 5-255) -- the Brightest slider

**Time-Interval Animations**:
- `quarterAnimation` (int, 0-3: 0=off, 1=Slow comet, 2=Dual orbit, 3=Bloom ripple)
- `halfHourAnimation` (int, 0-3: 0=off, 1=Unfurl, 2=Three comets, 3=Breathe)
- `hourAnimation` (int, 0-5: 0=off, 1=Ceremony, 2=Galaxy spin, 3=Supernova, 4=Comet relay, 5=Deep breath)
- `intervalAnimationsEnabled` (int, 0-1)

**Animation Style**:
- `animationPalette` (int; shared 5-option list, v2.27.0: `0`=Golden hour (warm), `1`=Moonlight (cool), `2`=Dawn (soft-warm), `3`=Twilight (muted-cool), `7`=Clock colors -- default. Any other value is coerced to `7`. Moods live in `tools/palettes/palettes.json` `mood_palettes`; `7` is ring-mapped from the face colors in firmware)
- `animationSpeed` (int, 1-5: 1=slow, 3=normal, 5=fast)
- `animationBrightness` (int, 50-255)
- `trailLength` (int, 2-12)
- `reminderPalette` (int; **same** 5-option list as `animationPalette`: `0-3` moods, `7`=Clock colors. Independent of `animationPalette` so a nudge can read differently from a chime)

**Focus Reminders**:
- `focusReminder_enabled` (int, 0-1)
- `focusReminder_startHour` (int, 0-23)
- `focusReminder_endHour` (int, 0-23) -- start == end is treated as an always-on 24h window (v2.31.0)
- `focusReminder_intervalMinutes` (int, 1-1440)
- `focusReminder_daysMask` (int, 0-127: bitmask Sun=bit0 .. Sat=bit6)
- `focusReminder_animation` (int, 0-11: 0-5 delegate to the chime animation slots, 6-11 dedicated nudges)
- `focusReminder_durationSeconds` (int, 1-60) -- **reserved**: accepted and stored, not yet used by the firmware

**Face / ring tuning**:
- `outerRingOffset` (int, 0-59: clockwise LED rotation applied to all rings at render time)
- `outerRingBrightness` (int, 0-100: percent multiplier on outer ring colors)
- `middleFaceScale` (int, 0-255: middle ring ambient level)
- `innerFaceScale` (int, 0-255: inner ring ambient level)
- `darkRoomOff` (int, 0-1: blank all LEDs in a pitch-black room, auto mode only)
- `petalDepth` (int, 0-100: how deeply ring fills are shaded between petals; 0 = flat fill). Legacy `petalMode` (0/1) is still accepted as an alias when `petalDepth` is absent, so custom themes saved before v2.30.0 keep their texture.

**Second-hand accessories**:
- `secondTrailLength` (int, 2-12)
- `secondTrailStyle` (int, 0-2: 0=classic geometric, 1=linear, 2=smooth gamma comet)
- `progressLevel` (int, 0-255: tint strength of the progress arc)
- `progressStyle` (int, 0-1: 0=uniform arc, 1=comet gradient)

**Timezone**:
- `timezone` (string, POSIX TZ, e.g. `MST7MDT,M3.2.0,M11.1.0`). Not a `ClockSettings` field -- it lives in NVS. An invalid string returns `400` and nothing is saved.

**Misc**:
- `silent` (any value): suppress the settings-saved status animation

**Example**:
```
POST /settings
Content-Type: application/x-www-form-urlencoded

dayBrightness=200&nightBrightness=20&autoBrightnessMode=1&secondsColor=#FF0000
```

**Response**: `200 OK "ok"` | `400 Bad Request` (invalid timezone) | `503 Service Unavailable` (firmware update in progress -- settings are locked so a flash write cannot collide with the OTA partition write)

**Side effects**:
- Settings saved to EEPROM
- Triggers STATUS_SETTINGS_SAVED animation (1.3 sec) unless `silent`
- Clock display updates immediately with new colors/brightness

---

### `POST /settings/preview`
**Description**: Non-persistent "Preview on clock": render the posted settings for a short TTL with no EEPROM write, then auto-revert to the saved face. Safe during OTA. Same parameter set as `POST /settings`.

**Extra parameter**: `ttl` (int, ms, optional; default 10000, clamped 1000-30000)

**Response**: `200 OK "ok"`

---

### `POST /settings/preview/clear`
**Description**: Cancel an in-flight preview and return to the saved face immediately

**Response**: `200 OK "ok"`

---

### `POST /settings/reset`
**Description**: Reset settings to defaults (user-saved defaults if present, else factory)

**Response**: `200 OK "ok"` | `503` during a firmware update

---

### `POST /settings/saveDefault`
**Description**: Save the current settings as the user defaults (second EEPROM slot at offset 128, magic 0xD2). `POST /settings/reset` restores this set from then on.

**Response**: `200 OK "ok"` | `503` during a firmware update

---

### `POST /reel/begin`
**Description**: Latch demo-reel Live Run mode: the live clock face is not drawn between previewed animations (black instead) and the device generates gentle cross-dissolves itself off each animation's start/end edges

**Parameters**: `ms` (int, optional; default 8000, clamped 2000-20000) -- idle crash-guard: if nothing animates for this long the clock returns to its face on its own, so a closed or crashed browser can never strand it black

**Response**: `200 OK "ok"`

---

### `POST /reel/end`
**Description**: End reel mode: gently dissolve back to the live clock face

**Response**: `200 OK "ok"`

---

### `POST /wifi`
**Description**: Save WiFi credentials from the `/wifi` page and reconnect

**Parameters**: `ssid` (string, 1-32 chars, required), `pass` (string)

**Response**: `200 OK "Saved. Reconnecting to <ssid>..."` | `400` on bad SSID length

---

### `POST /update`
**Description**: Upload a firmware `.bin` (multipart form upload from the `/update` page). Streams chunks to the OTA partition, verifies, reboots on success.

**Response**: `200` "Update successful, rebooting..." | `400` "No update in progress" | `413` "File too large" | `500` on begin/write/end failure

---

### `POST /demo/start`
**Description**: Start the demo reel (video recording sequence)

**Parameters**: `delay` (int, seconds, optional, clamped 0-60) -- capture pre-roll: holds the clock fully dark before the reel opens, so a camera can be rolling and settled and frame one is a fade-up out of clean black. Handled on-device, so the countdown survives the browser tab closing.

**Response**: `200 {"status":"started"}`

---

### `POST /demo/stop`
**Description**: Stop the demo reel immediately

**Response**: `200 {"status":"stopped"}`

---

### `POST /demo/brightnessCycle`
**Description**: Run only the auto-brightness demo cycle (lux override sweep), standalone

**Parameters**: `ms` (int, optional; default 14000) -- cycle duration

**Response**: `200 {"status":"started"}`

---

## Settings Structure (EEPROM, SETTINGS_VERSION 19)

### ClockSettings Struct

Regenerated from `src/main.cpp` (`struct ClockSettings`, `SETTINGS_VERSION = 19`). Field order below is the EEPROM layout order.

```cpp
struct ClockSettings {
  uint8_t magic;                    // 0xC1 (validation byte)
  uint8_t version;                  // 19 (SETTINGS_VERSION)

  // Brightness
  uint8_t dayBrightness;            // 0-255, default 44
  uint8_t nightBrightness;          // 0-255, default 5
  uint8_t nightStartHour;           // 0-23, default 22
  uint8_t nightEndHour;             // 0-23, default 7

  // Display
  uint8_t centerSource;             // 0-4: what the center LED shows when idle
                                    // (0=status+bloom, 1=bloom, 2=status, 3=temperature
                                    // when a sensor exists, 4=off). Reuses the EEPROM
                                    // byte of the retired colorTheme field.
  uint8_t secondTrail;              // 0-1 (tints over the face since v16)
  uint8_t progressSeconds;          // 0-1 (tints over the face since v16)
  uint8_t hourlyChime;              // 0-1, default 1
  uint8_t statusAnimations;         // 0-1, default 1

  // Ring colors. Note middleFace and innerFace are RGB only (their levels are
  // the *FaceScale fields further down); the rest are RGB + level.
  uint8_t outerMarkerRed, outerMarkerGreen, outerMarkerBlue, outerMarkerLevel;
  uint8_t outerFillerRed, outerFillerGreen, outerFillerBlue, outerFillerLevel;
  uint8_t secondsRed, secondsGreen, secondsBlue, secondsLevel;
  uint8_t minutesRed, minutesGreen, minutesBlue, minutesLevel;
  uint8_t hoursRed, hoursGreen, hoursBlue, hoursLevel;          // middle 24h hand
  uint8_t middleFaceRed, middleFaceGreen, middleFaceBlue;       // middle ring ambient
  uint8_t innerFaceRed, innerFaceGreen, innerFaceBlue;          // inner ring ambient
  uint8_t innerHourRed, innerHourGreen, innerHourBlue, innerHourLevel; // inner 12h hand
  uint8_t centerRed, centerGreen, centerBlue, centerLevel;

  // Auto-brightness
  uint8_t autoBrightnessMode;       // 0=manual, 1=auto, 2=scheduled; default 1
  uint8_t minAutoBrightness;        // 5-255 (Darkest), default 10
  uint8_t maxAutoBrightness;        // 5-255 (Brightest), default 153

  // Time-interval animations
  uint8_t quarterAnimation;         // 0-3, default 3 (Bloom Ripple)
  uint8_t halfHourAnimation;        // 0-3, default 1 (Unfurl)
  uint8_t hourAnimation;            // 0-5, default 4 (Comet Relay)
  uint8_t intervalAnimationsEnabled; // 0-1, default 1

  // Focus Reminders (added v8)
  uint8_t  focusReminder_enabled;         // 0-1, default 0
  uint8_t  focusReminder_startHour;       // 0-23, default 8
  uint8_t  focusReminder_endHour;         // 0-23, default 22
  uint16_t focusReminder_intervalMinutes; // 1-1440, default 60
  uint8_t  focusReminder_daysMask;        // bitmask Sun(bit0)..Sat(bit6)
                                          // (factory default becomes all days in v2.31.0)
  uint8_t  focusReminder_animation;       // 0-11: 0-5 reuse the chime anims,
                                          // 6-11 dedicated nudges
  uint8_t  focusReminder_durationSeconds; // 1-60. RESERVED: accepted via POST
                                          // /settings and stored, but not yet
                                          // used by the firmware.
  uint32_t focusReminder_lastFireMs;      // INTERNAL: RAM-only bookkeeping,
                                          // ignore it. Never meaningfully
                                          // persisted; not settable.

  // Ring rotation (added v10)
  uint8_t outerRingOffset;          // 0-59: clockwise LED rotation, all rings

  // Animation customization (added v11)
  uint8_t animationPalette;         // 0-3 moods, 7=Clock colors (default); others coerce to 7
  uint8_t animationSpeed;           // 1-5, default 3
  uint8_t animationBrightness;      // 50-255, default 157
  uint8_t trailLength;              // 2-12, default 6
  uint8_t reminderPalette;          // same 5-option list, independent of animationPalette

  // Face tuning (added v12-v14)
  uint8_t outerRingBrightness;      // 0-100 percent, default 77
  uint8_t middleFaceScale;          // 0-255, default 39
  uint8_t innerFaceScale;           // 0-255, default 39

  uint8_t darkRoomOff;              // 0-1 (added v15), default 0

  // Second-hand accessory tuning (added v16)
  uint8_t secondTrailLength;        // 2-12, default 4
  uint8_t secondTrailStyle;         // 0-2, default 2 (smooth)
  uint8_t progressLevel;            // 0-255, default 90
  uint8_t progressStyle;            // 0-1, default 0 (uniform)

  // Petal depth (added v17 as an on/off flag, widened to 0-100 in v19)
  uint8_t petalDepth;               // 0-100, default 45. 0 = flat fill.

  // RETIRED (v19): held autoBrightnessGain (v18). Folded into
  // maxAutoBrightness on migration; byte kept zeroed to preserve the layout.
  uint8_t reservedGain;
};
```

**EEPROM window**: 256 bytes total. Main slot at offset 0 (magic 0xC1); user-saved defaults slot at offset 128 (magic 0xD2, written by `POST /settings/saveDefault`). Compile-time static_asserts guard both boundaries.

**Not in the struct**: the timezone (NVS, `clock`/`tz`) and WiFi credentials (NVS, `wifi` namespace). Neither is touched by settings migrations or factory reset of the struct.

### Default Values

`SettingsStore::defaults()` in `src/main.cpp` is authoritative -- the values below were read from it at settings v19 and are the ChronoBloom theme:

**Brightness**:
- Day: 44 (~17%), Night: 5 (~2%), night schedule 22:00-07:00
- Auto: mode 1 (sensor), Darkest 10, Brightest 153 (the same daylight peak the pre-v19 defaults produced)

**Colors** (RGB @ level):
- Outer marker: periwinkle blue (110, 185, 255) @ 225
- Outer filler: deep royal blue (0, 8, 200) @ 145
- Seconds: mint green (100, 255, 180) @ 230
- Minutes: orange (255, 100, 0) @ 220
- Hours (middle hand): hot pink/magenta (220, 0, 180) @ 255
- Middle face: hot pink (220, 0, 180), scale 39
- Inner face: warm orange (255, 60, 0), scale 39
- Inner hour hand: periwinkle (110, 185, 255) @ 255 (v2.30.1 -- was magenta, zero hue contrast against the middle hand)
- Center: warm orange-red (255, 60, 0) @ 180

**Animations** (factory defaults):
- Quarter-hour: Bloom Ripple (mode 3), Half-hour: Unfurl (mode 1), Top of hour: Comet Relay (mode 4), interval animations enabled
- Style: palette 7 (Clock colors), speed 3, brightness 157, trail 6; reminder palette 0 (Golden hour)
- Petal depth: 45

---

## Error Responses

### `400 Bad Request`
**Causes**:
- Missing required parameters
- Invalid parameter format
- Out-of-range values
- Invalid POSIX timezone string on `POST /settings`

**Example**: `POST /set` without `hour` parameter returns:
```
400 Bad Request
missing hour/minute/second
```

### `503 Service Unavailable`
**Causes**:
- NTP sync requested but WiFi disconnected, NTP unreachable, or time invalid (`POST /syncNtp`)
- Settings write requested while a firmware update is running (`POST /settings`, `/settings/reset`, `/settings/saveDefault` all answer `Firmware update in progress -- settings locked`; a flash commit mid-OTA can corrupt the half-written image)

---

## CORS & Security

**CORS**: `GET /anim/status` is CORS-open (`Access-Control-Allow-Origin: *`; read-only, no side effects). Every other endpoint is same-origin only.

**Authentication**: None. Anyone on the local network can access all endpoints.

**Rate limiting**: None. Endpoints can be called unlimited times.

**Future considerations**:
- Basic authentication for `/settings` POST
- Rate limiting for NTP sync (max 1/minute)
- API key for external integrations

---

## WebSocket Support

**Status**: Not implemented

**Future**: Live time updates could use WebSocket instead of 1-second polling to reduce HTTP overhead.

---

## MQTT Topics (Future)

**When MQTT support added**:

> Note: topic prefix `chronobloom-clock` below is the planned firmware identifier. Final topic names will be confirmed when MQTT is implemented.

**Subscribe** (commands from Home Assistant):
- `chronobloom-clock/command/brightness` -- Set brightness (0-255)
- `chronobloom-clock/command/mode` -- Set display mode
- `chronobloom-clock/command/animation` -- Trigger specific animation

**Publish** (state to Home Assistant):
- `chronobloom-clock/state` -- Current time, brightness, mode (JSON)
- `chronobloom-clock/sensor/lux` -- Ambient light level
- `chronobloom-clock/sensor/temperature` -- Temperature (when sensor added)

---

## Client Libraries

### JavaScript (Web UI)

**Fetch time**:
```javascript
const response = await fetch('/time');
const data = await response.json();
console.log(`Current time: ${data.hour}:${data.minute}:${data.second}`);
```

**Save settings**:
```javascript
const params = new URLSearchParams();
params.set('dayBrightness', 200);
params.set('secondsColor', '#FF0000');

await fetch('/settings', {
  method: 'POST',
  headers: {'Content-Type': 'application/x-www-form-urlencoded'},
  body: params.toString()
});
```

### Python (Home Automation)

**Get current time**:
```python
import requests

response = requests.get('http://esp32c3-v3-8inch.local/time')
data = response.json()
print(f"Current time: {data['hour']}:{data['minute']}:{data['second']}")
```

**Set brightness**:
```python
requests.post('http://esp32c3-v3-8inch.local/settings', data={
  'dayBrightness': 200,
  'nightBrightness': 10
})
```

### Curl (Command Line)

**Sync to NTP**:
```bash
curl -X POST http://esp32c3-v3-8inch.local/syncNtp
```

**Get diagnostics**:
```bash
curl http://esp32c3-v3-8inch.local/diag
```

**Set time**:
```bash
curl -X POST http://esp32c3-v3-8inch.local/set \
  -d "hour=14&minute=30&second=0"
```

---

## WiFi Provisioning

### First-Boot Portal

**Behavior**:
- On first boot, if no saved WiFi credentials exist, the device first listens for Improv WiFi over USB serial (bounded window, used by the browser flasher), then opens a WiFi access point
- SSID: `esp32c3-clock-setup` (no password required)
- IP address: `192.168.4.1`
- Portal window (v2.31.0): 10 minutes on a never-provisioned first boot, 2 minutes on a previously-connected unit, 15 minutes after factory reset (then falls through to AP mode)

### WiFi AP Fallback

If all STA connection attempts fail (wrong password, network unavailable), the device starts a software AP and runs the full web server at `192.168.4.1`. Clock display, web UI, and settings are all functional in AP mode. NTP, mDNS, and OTA are skipped until a STA connection is established. While nobody is connected to the setup AP, the device retries the known credentials every 5 minutes and reboots into normal mode on success.

**Portal page**:
1. Displays list of available WiFi networks (SSIDs)
2. User selects their SSID and enters password
3. Device stores credentials (WiFiManager library manages storage)
4. Device connects to saved network
5. Portal closes automatically

**Credential Storage**:
- WiFi SSID and password stored in NVS (separate from clock settings)
- Persists across reboots
- Can also be changed any time via the `/wifi` page

**Error Handling**:
- If the portal window closes without credentials, the portal reappears on next boot
- Invalid credentials (password changed in router): portal reappears on next boot
- WiFi network unavailable: portal reappears on next boot

---

## OTA (Over-The-Air) Firmware Updates

### Overview

After initial USB flash, firmware updates can be deployed over WiFi using ArduinoOTA protocol on port 3232, or by uploading a `.bin` on the `/update` page.

### OTA Protocol Details

**Port**: 3232 (TCP)
**Authentication**: None. Current firmware calls `ArduinoOTA.begin()` without a password; OTA is open to the local network.
**Protocol**: ArduinoOTA binary protocol (not HTTP/HTTPS)

### Update Command

```powershell
# Build new firmware
pio run -e esp32c3_v3_8inch

# Upload via OTA to device at mDNS hostname
pio run -e esp32c3_v3_8inch -t upload --upload-port esp32c3-v3-8inch.local:3232

# Or upload to device by IP address
pio run -e esp32c3_v3_8inch -t upload --upload-port 192.0.2.100:3232
```

### Update Flow

1. **Build phase**: PlatformIO compiles firmware (.bin file)
2. **Connection phase**: PlatformIO connects to device on port 3232
3. **Upload phase**: Binary streamed to device (shows progress % in serial monitor if connected)
4. **Flash phase**: Device writes new firmware to flash memory
5. **Verify phase**: Device verifies flash integrity
6. **Reboot phase**: Device automatically reboots with new firmware
7. **Status animations**:
   - During upload: inner ring shows blue
   - On success: inner ring shows green
   - On failure: inner ring shows red

### Status Indicators

**Serial output during OTA**:
```
[OTA] Update starting...
[OTA] Progress: 45000/703962 (6.4%)
[OTA] Progress: 90000/703962 (12.8%)
...
[OTA] Update complete, rebooting...
```

### Troubleshooting OTA

**Port 3232 not responding**:
- Device must be on WiFi (test with `ping esp32c3-v3-8inch.local`)
- Check firewall: port 3232 may be blocked
- Restart device and try again (OTA server initializes ~30-60s after boot)
- Verify mDNS is working: ping should resolve hostname to IP

**Network interruption during upload**:
- Blue animation stops, device may reboot
- Try again with stronger WiFi signal (move closer to router)
- Check router is not dropping connection on specific devices

**Authentication failure**:
- Current firmware sets no OTA password, so an auth failure means the device is still running an older build that did
- Reflash once over USB (or the `/update` page) to bring it to the current firmware
