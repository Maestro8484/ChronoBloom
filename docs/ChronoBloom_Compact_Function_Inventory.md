# ChronoBloom Compact Function Inventory
Source: `FUNCTION_INVENTORY.md`, `src/main.cpp`, `src/web_html.h`. SHA256 `(regenerate via gen_symmap.py)`. 139 functions as of v2.2.0.
Purpose: print-friendly edit map. Common edit targets only; internals condensed at bottom.

## Page 1 - Common / Reasonably Likely Edit Targets
| Function / Structure | Telegraph meaning | Edit when |
|---|---|---|
| `SettingsStore::defaults` | Factory defaults | Default palette, brightness, feature defaults. |
| `SettingsStore::valid` | EEPROM accept rules | Change when fields/ranges change. |
| `SettingsStore::sanitize` | Clamp user input | Safety bounds for saved settings. |
| `SettingsStore::resetToDefaults` | Wipe EEPROM to defaults | Factory reset path; called on version mismatch. |
| `ClockSettings` | Saved settings schema | Add field -> bump SETTINGS_VERSION. |
| `RingConfig` | Physical LED map | Ring start index, count, direction. |
| `ClockRenderer::render` | Full frame pipeline | Overall draw order/priority. |
| `ClockRenderer::renderFace` | Static ring backdrop | Markers, filler, base colors. |
| `ClockRenderer::renderSeconds` | Second hand logic | Trail/progress/dot behavior. |
| `ClockRenderer::renderMinutes` | Minute dot | Minute placement/color behavior. |
| `ClockRenderer::renderHours` | Hour hand logic | 12h mapping, half-hour advance. |
| `ClockRenderer::effectiveBrightness` | Brightness selector | Manual/lux/scheduled dimming. |
| `ClockRenderer::nightActive` | Night window test | Scheduled dimming hours. |
| `ClockRenderer::setRingPixel` | Logical -> physical LED | Rotation, offset, direction fixes. |
| `ClockRenderer::setCenterPixel` | Center LED writer | Inline vs separate center output. |
| `ClockRenderer::setSacrificialPixelDark` | Keep buffer LED black | Sacrificial first LED handling. |
| `ClockRenderer::renderCenterIdle` | Center breathing pulse | Idle center animation style. |
| `ClockRenderer::renderStatus` | Status spinner/pulse | WiFi/save/error visual feedback. |
| `ClockRenderer::renderHourlyChime` | Top-hour sweep | Hourly chime visual style. |
| `ClockRenderer::tickAnimation` | Animation dispatcher | Routes to per-case anim* method. |
| `ClockRenderer::paletteColor` | Palette-mapped color | 8-palette system; add palette here. |
| `ClockRenderer::triggerAnimDirect` | Fire animation by ID | Used by /previewAnimation endpoint. |
| `ClockRenderer::triggerReminderDirectAnimation` | Fire reminder anim by ID | Reminder palette, warm tones. |
| `triggerQuarterAnimation` / `triggerHalfHourAnimation` / `triggerHourAnimation` | Start selected animation | Selector mapping and no-op rules. |
| `LuxSensor::lux` | Read + smooth lux | Gain thresholds, EMA, settle time. |
| `LuxSensor::autoBrightnessCached` | Lux -> brightness ramp | Curve, min/max feel, ramp speed. |
| `LuxSensor::setLuxOverride` / `clearLuxOverride` | Bypass hardware lux | Used by Demo Mode for brightness sim. |
| `TimeSync::begin` | NTP/TZ startup | Timezone and NTP servers. |
| `TimeSync::syncNow` | Apply valid NTP time | Epoch guard and localtime conversion. |
| `TimeSync::loop` | Periodic resync | Callback + 6-hour refresh behavior. |
| `TimeModel::set` / `addMinutes` / `addHours` | Manual time edits | Browser time buttons and sync. |
| `ButtonInput::poll` | Physical button state machine | Debounce, hold-to-repeat, GPIO pins. |
| `ButtonInput::consumeUp` / `consumeDown` | Button delta consumer | Returns +/- minute delta to loop(). |
| `DemoMode::loop` | Demo sequence driver | Step table, timing, lux override calls. |
| `DemoMode::statusJson` | Demo status API | JSON payload for /demo/status endpoint. |
| `WebUi::begin` | WiFi/web bringup | AP fallback, mDNS, OTA, NTP order. |
| `WebUi::setupRoutes` | HTTP route table | Add/change browser endpoints. |
| `WebUi::settingsJson` | Settings API output | Expose new settings to UI/JS. |
| `WebUi::parseColor` | Hex color parser | Color input validation behavior. |
| `WebUi::clampByte` / `clampWord` | POST bounds guards | Form min/max safety. |
| `FocusReminderScheduler::checkAndFire` | Reminder gatekeeper | Days, hours, interval firing. |
| `FocusReminderScheduler::getDayOfWeek` | Weekday source | Needs pre-NTP epoch guard. |
| `FocusReminderScheduler::triggerReminderAnimation` | Reminder animation map | Map options to effects. |
| `setup` | Boot sequence | Add subsystem init here. |
| `loop` | Main scheduler | Tick, web, render, button, demo. |
| `logRuntimeStatus` | Serial health dump | Debug fields and cadence. |
| `src/web_html.h` | PROGMEM HTML/JS payload | Edit UI layout, labels, JS, CSS. Do NOT edit inline in setupRoutes. |

## Rules That Prevent Pain
| Rule | Why it matters |
|---|---|
| EEPROM rule | Changing ClockSettings layout requires SETTINGS_VERSION bump or stale EEPROM will bite. |
| Rotation rule | outerRingOffset is one outer-ring step; if one step worse, reverse direction. |
| Sacrificial LED rule | Do not remove setSacrificialPixelDark; buffer LED must stay black. |
| UI setting rule | New setting touches ClockSettings, defaults, valid, sanitize, settingsJson, setupRoutes, web_html.h, version bump. |
| Animation rule | Visual look lives in anim* methods; dispatcher is tickAnimation; selector start is trigger*Animation. |
| HTML rule | All web UI HTML/JS lives in src/web_html.h. Page content changes go there, not to setupRoutes. |
| Demo rule | Demo step table and timing live in DemoMode::loop. Lux simulation uses setLuxOverride/clearLuxOverride. |
| Button rule | ButtonInput uses polled reads (no ISRs). GPIO5=UP, GPIO9=DOWN. Hold >500ms repeats; >2s jumps 60min. |

## Page 2 - Rarely Edited / Support Internals
These are real functions, but normally not the first place to edit unless debugging plumbing, boot, or timing.

- `configureWiFiHostname` - hostname plumbing before DHCP
- `writeStatusLed` - GPIO LED helper; no-op when STATUS_LED_PIN < 0
- `setupWiFi` - usually edit credentials/build flags, not logic
- `setupOTA` - OTA handlers/password/log text
- `setupDemoModeRoutes` - registers /demo/* routes at runtime; avoids incomplete-type issues
- `LuxSensor::begin` - VEML7700 init only
- `LuxSensor::autoBrightness` / `autoBrightnessTarget` - wrapper and diagnostics only
- `SettingsStore::begin` / `update` / `save` - EEPROM load/save wrappers
- `TimeModel::get` / `tickOneSecond` / `consumeDirty` / `incrementSecondNoLock` / `markDirty` - clock plumbing
- `ClockRenderer::ClockRenderer` - constructor
- `ClockRenderer::begin` - strip init/clear/show
- `ClockRenderer::setLuxSensor` / `setStatus` / `animating` - wiring setters and state flag
- `ClockRenderer::renderAnimFrame` / `renderCenterAnimationFrame` - animation render wrappers
- `ClockRenderer::needsFullAnimationFrame` / `needsCenterAnimationFrame` - 80ms render gates
- `ClockRenderer::dim` / `scale` / `pulse` / `scaledElapsed` / `secondColor` / `ringColor` - color/timing math helpers
- `ClockRenderer::chimeActive` / `statusActive` / `centerIdleActive` - small boolean gates
- `ClockRenderer::logShow` - serial LED timing diagnostics
- `ClockRenderer::animQ1-Q6` - 6 quarter-hour animation implementations
- `ClockRenderer::animH1-H7` - 7 half-hour animation implementations
- `ClockRenderer::animHr1-Hr10` - 10 top-of-hour animation implementations
- `ClockRenderer::animRem1-Rem6` - 6 focus reminder animation implementations (edit specific method for that effect)
- `ButtonInput::begin` / `consumeUpPress` / `consumeDownPress` - button init and raw press consumers
- `DemoMode::DemoMode` / `start` / `stop` - demo lifecycle; step table is in loop()
- `WebUi::loop` - handleClient + ArduinoOTA pump
- `WebUi::boolJson` / `colorHex` / `hexNibble` / `wifiStatusText` / `setLuxSensor` / `enabled` / `apMode` - output helpers
- `TimeSync::TimeSync` / `synced` / `lastDeltaSec` - constructor and state accessors
- `TemperatureInput` - disabled stub in current builds
- `ClockTime` - h/m/s POD copy
- `AnimPhase` / `StatusMode` - enum IDs for animations/status

## Fast Edit Path Cheat Sheet
- **Color/default look:** `SettingsStore::defaults`, `renderFace`, hand renderers.
- **Clock physically rotated:** `setRingPixel`, `RingConfig`, `outerRingOffset`.
- **Sacrificial WS2812B:** `setSacrificialPixelDark`, ring offsets/config, pixel count.
- **Web UI page content (HTML/CSS/JS):** `src/web_html.h` -- not setupRoutes.
- **Web UI endpoints/logic:** `setupRoutes`, `settingsJson`, clamps/sanitize.
- **New saved option:** `ClockSettings`, defaults, valid, sanitize, settingsJson, POST handler, web_html.h, version bump.
- **Brightness feels wrong:** `effectiveBrightness`, `LuxSensor::lux`, `autoBrightnessCached`.
- **Reminder weirdness:** `checkAndFire`, `getDayOfWeek`, `triggerReminderAnimation`.
- **Physical buttons:** `ButtonInput::poll`, `consumeUp`/`consumeDown`. Pins: GPIO5=UP, GPIO9=DOWN.
- **Demo mode sequence or timing:** `DemoMode::loop` (step table), `DemoMode::statusJson`.
- **Specific animation look:** `ClockRenderer::animQ/H/Hr/Rem*` -- find the matching method.
