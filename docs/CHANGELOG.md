# ChronoBloom ESP32-C3 — Changelog

> Formerly neopixelClock-esp32c3-v3

## [2.8.0] - 2026-07-03 (animation feel pass: gentler nudges, richer galaxy/spiral, stronger bloom contrast)

Coordinated visual overhaul of the reminder, galaxy/spiral, and bloom animations plus the demo script that showcases them — tuned for both the everyday clock and, especially, video capture. No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15); all changes are structural (envelopes, brightness waves, hue drift, per-ring flower separation) so they improve on **every** palette rather than overriding the user's palette choice.

### Changed
- **Focus reminders now read as gentle nudges, not warning flashes** (`ClockRenderer::animRem1/animRem3/animRem4`, `animRem5`):
  - New shared `NUDGE_CEIL` (~80%) caps every reminder's peak brightness so a nudge never blasts full intensity.
  - **Gentle Pulse (Rem1)**: envelope lengthened (was 800/400/1800 → 1200 rise / 500 crest / 1900 fall) and given a subtle warm hue drift across the swell, so it breathes instead of blinking.
  - **Ripple In (Rem3)**: slower onsets (300→350 spacing) and softer 600ms rise / 1100ms fall (was 400/900) so each ring swells in gently.
  - **Heartbeat (Rem4)**: rebuilt from a hard strobe (140ms rise / 80ms fall — effectively a warning flash) into two eased swells (350ms rise / 550ms fall per beat) with a warm hue and brighter stamen core. Still a two-thump cadence, now a breath.
  - **Slow Bloom (Rem5)**: now unfurls the inner-12 "stamen filament" ring (previously skipped) so the bloom completes petals → stigma → stamen → core.
- **Galaxy Spin (`animHr2`) — real light/dark contrast and star twinkle**: the rotating brightness wave (`galaxyWave()`, a two-arm gamma wave with a true-black floor, replacing the single-arm `spinWave()`) now applies to **every** palette, not just the ring-mapped one — gradient palettes used to render flat full-brightness with no dark lanes. Added a slow global hue drift so the arms cycle through the palette over the run, rare white "star" pixels twinkling in the dark lanes (`hash8()`), and a gently twinkling galactic core.
- **Comet Relay / rainbow spiral (`animHr4`)**: each comet's head now sweeps the palette as it travels and its trail lags a few hue-steps behind, turning each flat-colored comet into a rainbow gradient that reads as a spiral. Center finale now uses the warm stamen core.
- **Bloom flower-part contrast** (`animQ3` Bloom Ripple, `animH1` Unfurl): explicit flower coloring from the center out — warm gold **stamen** (center + inner-12, palette-independent) → **stigma** (middle-24, palette) → **petals** (outer-60, palette). New `stamenColor()` helper gives every palette a bright warm heart, the strongest ChronoBloom cue. Unfurl's flat-white center replaced with the warm stamen.
- **Demo script** (`DemoMode`): hour showcase (step 4) now includes Comet Relay alongside Ceremony / Galaxy Spin / Supernova, so both spiral/galaxy pieces are on camera; reminder step (step 5) plays a varied but uniformly gentle set (Gentle Pulse → Ripple In → Heartbeat → Slow Bloom) demonstrating that no nudge flashes. Step duration estimates and subtitles updated; web UI label changed from "96-second" to "~110-second".

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.7.6 → 2.8.0), `src/main.cpp` (new `galaxyWave`/`hash8`/`NUDGE_CEIL`/`stamenColor` helpers replacing `spinWave`; `animHr2`, `animHr4`, `animQ3`, `animH1`, `animRem1`, `animRem3`, `animRem4`, `animRem5`; `DemoMode::stepTick` steps 4–5 and `steps[]` table), `src/web_html.h` (demo showcase label), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.7.6] - 2026-07-02 (demo: no flash between steps; inner ring recolored to contrast)

### Fixed
- **Bright flash between demo animations at step boundaries**: `DemoMode::loop` dispatched on `step_` and called `advanceStep()` when a step's sequence finished, but the *new* step's first trigger only fired on the *next* `loop()` tick (the dispatch happens once per call, top-down by `step_`). In the gap tick, `renderer.animating()` was false and nothing had been triggered yet, so the main loop's render dispatch (`main.cpp` ~3286) fell into its idle-render branch and rendered one frame of the live, fully-lit real-time clock face — a bright flash — between the last frame of the old step and the first frame of the new one. This happened at every step1→2→3→4→5 boundary (four times per demo run). Within a step, back-to-back repeats were already flash-free because the same `advanceXSeq()` call both detects completion and triggers the next repeat in one shot; only the step-level `advanceStep()` hand-off had the gap.
  - Fix: split `loop()`'s per-step logic into `stepTick()`, which returns whether it just advanced to a new step. `loop()` now re-invokes `stepTick()` immediately (bounded to 8 iterations, one per step, as a safety guard) whenever a transition happens, so the new step's first animation triggers in the same tick — `renderer.animating()` never goes false at a step boundary, and the idle-render branch never gets a window.
- **Middle-24 and inner-12 rings visually fusing into one ring during animations**: `ClockRenderer::bandColor()`'s ring-mapped palette 7 case used `innerHourColor` for the inner-12 band, and on the maintainer's clock `innerHourColor` (#DC00B4) is identical to `hoursColor` (#DC00B4, used for the middle-24 band) — a deliberate idle-face design choice (inner hour markers match the middle hand for visual continuity) that becomes a liability in animations, where each band renders as one solid-color ring: the middle and inner rings became indistinguishable, reading as a single wide ring. Inner-12 band now uses `secondsColor` instead — already part of the clock's configured palette, and distinct from outer/middle/center — so it contrasts against both rings outside it. Idle-face rendering (`renderFace`, using `innerHourColor` directly) is untouched; this only affects the animation-only band mapping.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.7.5 → 2.7.6), `src/main.cpp` (`DemoMode::loop`/`stepTick` split, `ClockRenderer::bandColor` band-2 case), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.7.5] - 2026-07-02 (main web UI shows firmware version)

### Added
- Firmware version badge on the main page header (src/web_html.h): a small `v2.7.5`-style label next to "ESP32 Ring Clock", populated by `loadVersion()` fetching the already-existing `/diag` endpoint's `firmware_version` field on page load. Previously the version was only visible on `/update` (added v2.4.3) and `/diag` JSON — the primary web UI itself had no version reference at all.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.7.4 → 2.7.5), `src/web_html.h` (`INDEX_P2` header markup, `loadVersion()`, init call), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.7.4] - 2026-07-02 (galaxy spin: darker trough for a legible rotating band)

### Changed
- `ClockRenderer::spinWave()` (src/main.cpp): with ring-mapped palette 7 (one solid color per ring), Galaxy Spin's motion is conveyed entirely by a brightness wave rotating around each ring — there's no color gradient to show rotation. Confirmed by the user post-v2.7.3 (fps fix) that the wave was readable but its dim side (floor 60/255, ~24%) wasn't dark enough to contrast against the ~100% peak. Replaced the linear-triangle/floor-60 wave with a gamma-shaped wave and a true-black floor (`gamma8(tri)`), concentrating brightness into a narrow comet-like bright arc with a dark trailing gap — the rotation should now read clearly instead of as a subtle ripple.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.7.3 → 2.7.4), `src/main.cpp` (`ClockRenderer::spinWave`), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.7.3] - 2026-07-02 (fix blocking lux read that throttled all animations to ~8fps)

### Fixed
- **The root cause of jerky/laggy/"flashing not spinning" animations**: `LuxSensor::lux()` (src/main.cpp) called `veml_.readLux()`, which defaults to `VEML_LUX_NORMAL` → `readALS(true)` → `readWait()` — and `readWait()` does a **blocking `delay()` of up to 2× the integration time (2×100ms = 200ms)** before reading the ALS register. This runs on the animation render path (`renderAnimFrame` → `effectiveBrightness` → `autoBrightnessCached` → `lux()`, auto-brightness mode 1), and `loop()` captures `now` once per iteration, so a blocked iteration stalls every animation frame with it. Measured from a recorded demo (native-30fps ffmpeg frame extraction): animations rendered at roughly ~8fps instead of the intended 40fps — galaxy spin held then jumped ("flashed" rather than spun), comet relay skipped 8-10 outer-ring LEDs between updates, deep breath stepped its brightness sine in coarse chunks.
- Fix: read via `VEML_LUX_NORMAL_NOWAIT`, which reads the ALS register directly without the integration-wait `delay()`. The sensor integrates continuously every 100ms and `lux()` is throttled to read at most every 120ms, so the register always holds a fresh value — there is nothing to wait for. Lux read drops from up to ~200ms blocking to ~1-3ms of I2C. Also fixes the same stall on the `roomDark()` path when `darkRoomOff` is enabled. No behavior change to the brightness values themselves — only the blocking is removed.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.7.2 → 2.7.3), `src/main.cpp` (`LuxSensor::lux` — NOWAIT read), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.7.2] - 2026-07-02 (dead-tail fix: three animations held pure black for up to 1s before looping)

### Fixed
- Diagnosed via frame-by-frame analysis of a recorded demo (ffmpeg contact sheets, ~64 frames across the first 8s at 8fps): the alternating bright-ring/full-black pattern was not a camera artifact. Three animations have onset/fade tables whose visual content reaches zero brightness well before their coded `dur` elapses, so the animation stays "active" (holding pure black) for the remainder of `dur` before `ClockRenderer::animating()` goes false and the immediate-chain sequencer (v2.6.0) triggers the next one — reading as flash → black hold → flash rather than a clean loop.
  - `animQ3` (Bloom Ripple): last band reaches zero at se=2300, `dur` was 2500 → 200ms dead tail. Trimmed to 2300.
  - `animRem3` (Ripple In): last band reaches zero at se=2500, `dur` was 3500 → **1000ms** dead tail. Trimmed to 2500.
  - `animRem4` (Heartbeat): second beat's decay finishes at se≈2040, `dur` was 3000 → **~960ms** dead tail. Trimmed to 2100. This was the worst offender in the current demo sequence: Heartbeat plays twice in step 5, so ~1.9s of every ~10.2s reminder step was pure black before this fix.
- The other 13 animations were audited against the same failure mode (visual completion time vs. coded `dur`) and confirmed clean — their tails are either a coordinated global brightness fade that reaches zero exactly at `dur`, or continuous motion/breathing with no early dead zone.

### Changed
- `DemoMode::steps[]` progress-bar duration estimates updated for the two shortened animations (step 1: 5500→4600ms, step 5: 12500→10200ms). `docs/publish/DEMO_CAPTIONS.srt`/`DEMO_MODE.md` resynced; total ~99s (was ~102s).

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.7.1 → 2.7.2), `src/main.cpp` (`animQ3`, `animRem3`, `animRem4` dur constants; `DemoMode::steps[]`), `docs/publish/DEMO_MODE.md`, `docs/publish/DEMO_CAPTIONS.srt`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.7.1] - 2026-07-02 (demo redesign: configured chimes, less-is-more sequence)

### Changed
- `DemoMode` sequence redesigned around the operator's exported configuration (via the v2.6.0 Full Settings Backup) on the "less is more" principle: steps 1-3 now fire the clock's **configured** chime triggers (`triggerQuarterAnimation`/`triggerHalfHourAnimation`/`triggerHourAnimation`) twice each via new `advanceConfiguredSeq()` — the demo always mirrors what the unit actually does at :15/:30/:00 in its own palette/speed/brightness/trail, instead of racing through a hardcoded catalog of every style. If a slot's mode is 0 (off) the trigger no-ops and the step falls through without stalling.
- Step 4 is now a 3-animation showcase (Ceremony, Galaxy Spin, Supernova — the big all-ring hour styles) instead of all 5 hour styles; the 8-palette rotation step is dropped entirely (lowest payoff per second, and the clock's own colors are now the demo's whole point).
- Step 5 reminders: `advanceReminderSeq()` takes an explicit mode list — Gentle Pulse ×2 then Heartbeat ×2, contrasting the subtle and attention-getting ends of the nudge spectrum instead of racing through all five styles once each.
- Total runtime ~102s (was ~134s). `docs/publish/DEMO_CAPTIONS.srt` and `docs/publish/DEMO_MODE.md` resynced.

### Fixed
- Web UI demo status line said "Step X/6" — there are 8 steps (src/web_html.h).

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.7.0 → 2.7.1), `src/main.cpp` (`DemoMode::loop`, `advanceConfiguredSeq` added, `advanceReminderSeq` re-signatured, `advancePaletteSeq` removed, `steps[]` table), `src/web_html.h` (step count), `docs/publish/DEMO_MODE.md`, `docs/publish/DEMO_CAPTIONS.srt`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.7.0] - 2026-07-02 (animation rework: ring-true colors, eased fades, preview without EEPROM writes)

### Fixed
- **Palette 7 quadrant striping** (src/main.cpp): every animation sampled the palette by LED position around the ring (e.g. `paletteColor(i * 4)` spans positions 0-236 on the outer ring), but ring-mapped palette 7 assigns colors by palette *quadrant* (0-63 = outer marker color, 64-127 = minutes, 128-191 = hours, 192+ = center). Result: with palette 7 selected — the palette meant to show the clock's own colors — each ring was striped into four different quadrant colors (1-3 o'clock differed from 6-9 o'clock). New `ClockRenderer::bandColor(band, frac)` samples by ring band (0=outer, 1=middle, 2=inner, 3=center): palette 7 returns each ring's actual configured color; gradient palettes 0-6 keep their around-the-ring gradients unchanged. All 16 animations rewired.
- **Three Comets counter-rotation** (`animH2`): the middle-ring comet ran in the opposite direction from the outer/inner comets by design, which read as chunks of LEDs moving against each other. All three comets now travel the same direction.
- **Preview-induced lag/unresponsiveness**: every "Preview" click in the web UI POSTed `/settings` first, and `SettingsStore::update()` commits to EEPROM flash on every call — a blocking, interrupt-masking flash write per click, stalling rendering mid-animation and wearing flash. `/previewAnimation` now accepts optional `palette`/`speed`/`brightness`/`trail`/`reminderPalette` params applied as non-persistent renderer overrides (extending the v2.5.4 palette-override mechanism to the full style set; auto-cleared when the animation finishes so scheduled chimes use saved settings). `previewAnim()`/`previewStyleAnim()` in src/web_html.h no longer write settings at all — "Save style" remains the only persist path. (Reported `free_heap` ~234 KB was healthy; the lag was flash commits, not a memory leak.)
- **Halting/steppy fades**: linear brightness ramps read as visible steps on WS2812s, especially near the dim end. Added `ease8()` (quadratic ease-in-out) applied to every animation's attack/release envelope via new `animEnv()`, `gamma8()` perceptual weighting on trail falloff (`trailLevel()`) and on the Breathe/Deep Breath sine troughs, so fades settle smoothly into dark instead of stepping through the last PWM levels.
- **Galaxy Spin invisible with ring-mapped palette**: one solid color per ring means rotation had nothing to show; a rotating brightness wave (`spinWave()`, floor 60) keeps the motion legible in the clock's own colors. Gradient palettes keep the original rainbow spin.

### Changed
- Animation color palette dropdown (src/web_html.h): "Clock colors" is now listed first and labeled "Clock colors (default)"; factory default `animationPalette` changed 0 → 7 in `SettingsStore::defaults()` so new/reset clocks animate in their own configured ring colors out of the box. No `ClockSettings` layout change — `SETTINGS_VERSION` stays 15; existing clocks keep their saved palette.
- Dual Orbit (`animQ2`) second orbiter uses the center band color — for gradient palettes numerically identical to before (position 128); for palette 7 it contrasts with the first orbiter instead of duplicating it.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.6.0 → 2.7.0), `src/main.cpp` (`bandColor`/`ease8`/`gamma8`/`spinWave`/`animEnv`/`trailLevel`/`animBr`/`animTl`/`setStyleOverride`/`clearStyleOverrides`, all 16 `anim*` functions, `paletteColor`, `scaledElapsed`, `tickAnimation`, `SettingsStore::defaults`, `/previewAnimation` handler), `src/web_html.h` (palette dropdown, `styleParams`/`previewAnim`/`previewStyleAnim`), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.6.0] - 2026-07-02 (demo mode: seamless animation chaining; full settings backup)

### Fixed
- `DemoMode` sequencer (v2.5.4) still looked halting after the event-paced rewrite: the 450ms gap added between animations let `ClockRenderer::animating()` go false, which let the main `loop()`'s idle-render branch (`timeModel.consumeDirty()` → `renderer.render()`) fire once and flash the live, fully-lit real-time clock face for the length of the gap before the next demo animation started — a jarring flash-cut on every single beat. Removed `DemoMode::kGapMs`/`gapUntilMs_` entirely; `advanceAnimPhaseSeq()`, `advanceReminderSeq()`, and `advancePaletteSeq()` (src/main.cpp) now chain straight into the next animation the instant the current one finishes, in the same `loop()` tick, so `animating()` never goes false mid-sequence and the idle-render branch never gets a window to fire. Each animation already fades to near-black in its own last frames, so the immediate cut reads as a clean beat rather than a jump.

### Added
- Full Settings Backup panel (src/web_html.h, Web UI): `exportBackup()`/`importBackup()` round-trip the *entire* `GET /settings` JSON (ring colors/levels, animation palette/speed/peak-brightness/trail-length, auto-brightness, focus reminders, chime style selections, everything) through the existing `POST /settings` handler, which already accepts every one of those fields by name. No firmware/endpoint changes needed — this is a JS-only addition alongside the existing colors-only "Export theme" feature. Lets the operator capture and hand off their exact tuned configuration (previously only ring colors were exportable via "theme"; animation settings were explicitly excluded).
- "Export theme"/"Import theme" (src/web_html.h) now also round-trips `animationPalette`, `animationSpeed`, `animationBrightness`, `trailLength`, `reminderPalette` — the exact field set of the Animation Style panel. Theme JSON format bumped `version: 2` → `3` (importer still tolerates missing fields for old theme files). Panel description text updated to match; previously read "Themes do not include brightness or animation settings," which was true and is no longer accurate.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.5.4 → 2.6.0), `src/main.cpp` (`DemoMode` sequencer — removed gap mechanism), `src/web_html.h` (Full Settings Backup panel + JS, Color Theme panel description updated), `docs/publish/DEMO_MODE.md`, `docs/publish/DEMO_CAPTIONS.srt` (step timings resynced to event-paced durations), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.5.4] - 2026-07-02 (demo mode: event-paced sequencer, no more EEPROM churn from palette showcase)

### Fixed
- `DemoMode::loop` (src/main.cpp) rewrote the animation sequencer from a fixed 3s-spacing timer to an event-driven one that waits for `ClockRenderer::animating()` to go false (plus a 450ms breathing gap) before triggering the next animation. Previously the half-hour (~5s) and hour (8-9s) chime animations were forcibly interrupted every 3s, producing the rapid/halting look reported in REVIEW.md-adjacent demo feedback.
- Palette-showcase step (step 4) called `SettingsStore::update()` every 2s, which writes to EEPROM and permanently overwrote the clock's real saved `animationPalette` — never restored afterward, and the step didn't actually trigger any animation (16s of silent EEPROM writes with the palette change invisible on the idle face). Replaced with a new non-persistent `ClockRenderer::setPaletteOverride()`/`clearPaletteOverride()` preview mechanism (never touches SettingsStore/EEPROM) and the step now plays Bloom Ripple (ANIM_Q3) once per palette so all 8 are actually visible.
- Demo mode's default palette/speed/brightness/trail-length were already read live from `settings_.get()`, so the showcase now stays honest to the unit's real configured palette (ring-mapped palette 7 on the 15" clock: `outerMarkerColor`/`minutesColor`/`hoursColor`/`centerColor`) for every step except the explicit palette-rotation showcase.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.5.3 → 2.5.4), `src/main.cpp` (`ClockRenderer::paletteColor`/new palette-override accessors, `DemoMode` sequencer rewrite + step-duration table), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.5.3] - 2026-07-02 (sacrificial pixel disabled by default, no settings version change)

### Changed
- 8" build default geometry changed from 98 LEDs (sacrificial pixel at physical index 0, `RING_PIXEL_OFFSET=1`) to 97 LEDs (no sacrificial pixel, `RING_PIXEL_OFFSET=0`, center at index 96). The sacrificial pixel is a vestigial troubleshooting remnant present only in the maintainer's original prototype; replicable public builds never needed it.
- `platformio.ini`: 8" LED-chain geometry factored into a new `[led_chain]` section (defaults to the replicable 97-LED config) with an annotation documenting the sacrificial pixel's original purpose and why it ships disabled. FIRMWARE_VERSION 2.5.2 → 2.5.3.
- Maintainer's 98-LED hardware geometry moved to a `[led_chain]` override in the gitignored `platformio.local.ini` (same mechanism as WiFi credentials); commented template added to `platformio.local.ini.example`.
- `src/main.cpp`: explanatory comment added above the `SACRIFICIAL_PIXEL_ENABLED` default (comment-only; sacrificial machinery itself unchanged and still build-flag gated).
- README.md (8" build section, BOM), docs/HARDWARE.md (variants, LED indexing, ring config, power worst-case), and launch drafts (REDDIT_ESP32.md, HACKADAY.md, PRINTABLES.md) updated to describe the 97-LED default with the sacrificial pixel as a disabled vestigial option.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.5.2 → 2.5.3, `[led_chain]` section), `platformio.local.ini.example`, `src/main.cpp` (comment only), `README.md`, `docs/HARDWARE.md`, `docs/publish/REDDIT_ESP32.md`, `docs/publish/HACKADAY.md`, `docs/publish/PRINTABLES.md`, `docs/publish/RELEASE_MANIFEST.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.5.2] - 2026-07-02 (pre-publish provenance cleanup, no settings version change)

### Changed
- Boot banner and `#error` guard in `src/main.cpp` renamed from "NeoPixelClock v3" to "ChronoBloom v3" to match the current product name (visible on serial monitor at boot).
- `docs/TROUBLESHOOTING.md` expected serial output updated to match the new boot banner text.
- Lineage/attribution references to "Maestro" replaced with "Maestro8484" across NOTICE, README.md, docs/HISTORY.md, AGENTS.md, docs/CHANGELOG.md, docs/SESSIONS.md.
- Planned MQTT topic placeholder in docs/API.md renamed from `iris-clock` to `chronobloom-clock` (leftover unrelated-project codename).
- Historical OTA target IP `192.168.1.110` generalized to `192.168.1.x` across docs/API.md, docs/AUDIT_2026-05-13.md, docs/CHANGELOG.md, docs/SESSIONS.md, docs/TROUBLESHOOTING.md.
- Hardcoded local repo path (maintainer's real Windows username) replaced with `C:\path\to\chronobloom-esp32c3` across internal workflow/session docs not shipped in this repo.

### Added
- `docs/publish/PROVENANCE_AUDIT.md` — pre-publish provenance/secrets audit ahead of the public release repo split.
- `docs/publish/RELEASE_MANIFEST.md` — exact include/exclude file list for the curated public repo, with user-decision items and pre-publish gates (Session 37B).
- Launch copy drafts in `docs/publish/`: `REDDIT_ESP32.md`, `REDDIT_3DPRINTING.md`, `REDDIT_ADHD.md`, `HACKADAY.md`, `PRINTABLES.md`, `YOUTUBE_DESC.md` (Session 37B; internal drafts, not shipped in public repo).
- `.github/ISSUE_TEMPLATE/bug_report.md` and `.github/ISSUE_TEMPLATE/build_help.md` — public-repo issue templates with solo-dev best-effort support framing (Session 37B).

### Changed (docs, Session 37B)
- `README.md` rewritten build-first for strangers: hero image + YouTube placeholders, BOM table with placeholder purchase links, 8" build as recommended path / 15" as showcase, flash + OTA quickstart, WiFi provisioning walkthrough, solo-dev support note, lineage/credits. Removed links to internal WORKFLOW.md/REVIEW.md.
- Internal workflow docs (not shipped in this repo): residual real-username mention scrubbed to "the maintainer's dev PC" (completes the username scrub from the pre-publish provenance audit).

## [2.5.1] - 2026-07-01 (demo mode expansion, no settings version change)

### Changed
- `DemoMode` sequence expanded from 6 steps (~93s) to 8 steps (96s): quarter/half-hour/hour chimes now cycle through all available animation styles (3/3/5) instead of firing one each; palette step now cycles all 8 palettes instead of 4; focus-reminder step now cycles all 5 ADHD-friendly nudge styles instead of firing one.
- Auto-brightness demo step (Step 6) reworked from one slow 25s ramp-down/hold/ramp-up into two quick 1.5s dim/bright lux-override cycles, to showcase the recently-sped-up auto-brightness response (lux-poll refresh went from 500ms to 150ms, ~3-4x faster to react to light changes).
- `docs/publish/DEMO_MODE.md` step table and `src/web_html.h` demo-mode panel blurb updated to match.

### Added
- `docs/publish/DEMO_CAPTIONS.srt` — timestamped SRT caption track mirroring the new demo sequence, for OBS/post-production use as a backup to the live `/demo/overlay` browser source.

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini` (FIRMWARE_VERSION 2.5.0 → 2.5.1), `docs/publish/DEMO_MODE.md`, `docs/publish/DEMO_CAPTIONS.srt` (new), `docs/FEATURES.md`

---

## [2.5.0] - 2026-06-30 (repo hygiene, no firmware change)

Session 36 (public release repo hygiene). No `FIRMWARE_VERSION`/`SETTINGS_VERSION` change.

### Changed
- WiFi credentials moved out of committed `platformio.ini` into gitignored `platformio.local.ini`, pulled in via an overridable `[wifi_secrets]` section. `platformio.local.ini.example` committed as the onboarding template.
- Consolidated root `CHANGELOG.md` (stale fork) into this file; root is now a one-line pointer.
- `docs/FEATURES.md` animation catalog and `docs/publish/DEMO_MODE.md`/`PUBLISH.md` updated to match the current implementation (see docs/SESSIONS.md Session 36 for detail).
- `legalcode.txt` renamed to `LICENSE-HARDWARE`.

### Files changed
- `platformio.ini`, `platformio.local.ini.example`, `.gitignore`, `CHANGELOG.md`, `docs/CHANGELOG.md`, `docs/FEATURES.md`, `docs/publish/DEMO_MODE.md`, `docs/publish/PUBLISH.md`, `legalcode.txt` → `LICENSE-HARDWARE`, `docs/SESSIONS.md`

---

## [2.5.0] - 2026-06-10

Session 34 (Fable review session). Full review report: `docs/FABLE_REVIEW_2026-06-10.md`.

### Added
- **Dark-room display sleep** (`darkRoomOff`, default off): in auto-brightness mode, 30 s below 0.3 lux blanks all LEDs; wakes immediately above 2.0 lux. Checkbox in the Auto-Brightness panel; `/diag` reports `display_sleep`. Implements the planned "Power saving mode".
- **AP-fallback self-recovery**: while stranded in AP mode with no portal clients, the device retries saved STA credentials every 5 minutes (AP+STA) and reboots into normal mode on success. Fixes the boots-before-router-after-power-outage stranding.
- Compile-time `static_assert` guards for the EEPROM layout (main slot vs user-defaults slot vs window size).
- `/diag` and `settingsJson` snprintf truncation detection (serial warning instead of a silently broken web UI).

### Fixed
- **Animation mode bounds**: `valid()`/`sanitize()`/POST clamps/`/previewAnimation` still accepted the legacy 2.1.0 maxima (quarter ≤6, half ≤7, hour ≤10). Quarter 4-6 misfired into half-hour animations, half 4-7 into hour animations, hour 6-10 played nothing. Bounds now match the v2.4.0 set (3/3/5); dead `AnimPhase` values removed.
- **Settings load no longer nukes on a single bad byte**: magic+version match now routes through `sanitize()` for field-level repair instead of a full factory reset.
- **Focus reminder day filter on unsynced clocks**: day-of-week comes from the NTP epoch; offline boots (1970 epoch) now skip the day filter instead of silently never firing. `daysMask == 0` still means never.
- **mDNS after WiFi reconnect**: clean `end()`/`begin()` and the HTTP service record is re-added (was lost on every reconnect).

### Changed
- `/lux` and `/temperature` build JSON via `snprintf` (was heap `String` concat; `/lux` is polled every 2 s).
- `/update` upload serial logging rate-limited to 500 ms (was per-chunk, slowing every web OTA); byte count corrected.
- Removed dead `ClockRenderer::renderCenterAnimationFrame()` (no callers since v2.4.8).
- `SETTINGS_VERSION` 14 → 15 (all clocks reset to defaults on first boot; user-defaults slot invalidates). `FIRMWARE_VERSION` 2.4.9 → 2.5.0.

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, docs.

---

## [2.4.9] - 2026-05-24

### Fixed
- Center LED pulse jitter root cause: `renderCenterIdle()` rewrote to single-multiply in raw-pixel space. Old pipeline (`ringColor` → `scale` → `show`) applied three sequential integer truncations, leaving only ~19 distinct brightness levels (~0.5 R units/frame). New approach sweeps `amount` 12 %–100 % of `centerLevel` with one multiply, giving ~61 levels (~3.4 R units/frame). Visually smooth at all settings.
- `FIRMWARE_VERSION` 2.4.8 → 2.4.9.

---

## [2.4.8] - 2026-05-24

### Fixed
- Center LED pulse render interval 80 ms → 50 ms (12.5 → 20 fps); reduces visible jitter when WebUI is polling.
- Factory defaults restored to pre-v2.4.7 appearance: hot pink/magenta hands (#DC00B4), original face glow colors, middleFaceScale/innerFaceScale back to 55.
- All 11 contrast presets now use identical `hoursColor` and `innerHourColor` so the hour hand reads as one continuous indicator across both inner rings.
- `FIRMWARE_VERSION` 2.4.7 → 2.4.8.

---

## [2.4.7] - 2026-05-24

### Added
- Independent middle/inner hour-ring palette controls in the Rings Web UI:
  `middleFaceColor` + `middleFaceScale`, `hoursColor` + `hoursLevel`,
  `innerFaceColor` + `innerFaceScale`, and `innerHourColor` + `innerHourLevel`.
- Natural Contrast preset and updated theme export/import schema including the new face/hour fields.
- New `ClockSettings` fields for middle face color, inner face color, and inner hour hand color/level.
- Five new contrast presets: Solstice, Deep Space, Lava, Frostbite, Neon Garden — each using a distinct hue-contrast or value-contrast pairing optimized for NeoPixel LEDs.

### Changed
- `SETTINGS_VERSION` 13 → 14 and `FIRMWARE_VERSION` 2.4.6 → 2.4.7.
- Factory defaults now use higher-contrast natural colors for the two inner rings: deep teal middle face, warm amber middle hour hand, muted violet inner face, and soft magenta inner hour hand.
- `ClockRenderer::renderFace()` renders middle/inner ring faces from independent face colors.
- `ClockRenderer::renderHours()` renders middle and inner hour indicators with separate hand colors/levels.
- `WebUi::setupRoutes()` parses the new `/settings` color/level fields.
- `WebUi::settingsJson()` emits the new palette fields.
- `src/web_html.h` Web UI JS (`draw`, `loadSettings`, `bindLive`, `saveSettings`, contrast presets, theme import/export) now handles the new controls.

### Fixed
- `SettingsStore::resetToDefaults()` now ignores stale user-default EEPROM blocks saved under older settings versions, avoiding uninitialized palette bytes after the v14 struct expansion.
- `SettingsStore::hasUserDefaults()` now checks the version byte so the WebUI reset button label stays accurate after a firmware upgrade.
- `bindLive()` now clears the contrast-preset selector for all preset-owned inputs (colors, `hoursLevel`, `innerHourLevel`), not just the scale sliders.

### Files changed
- `src/main.cpp`
- `src/web_html.h`
- `platformio.ini`
- `docs/symmap.json`
- `docs/FUNCTION_INVENTORY.md`
- `docs/SESSIONS.md`
- `docs/CHANGELOG.md`
- `CHANGELOG.md`

---

## [2.4.6] - 2026-05-22

### Fixed
- **Critical: `sendClose()` infinite recursion crashed the device on every HTTP request.** The `replace_all` that converted `server_.send(` calls to `sendClose(` also mutated the `server_.send(` call inside `sendClose()` itself, creating a self-calling function — every incoming request (page loads, `/diag`, `/time`, etc.) triggered a stack overflow within ~5s and a WDT reset. Fixed by restoring the direct `server_.send(code, contentType, body)` call inside `sendClose()`.

### Files changed
- `src/main.cpp`, `platformio.ini` (`FIRMWARE_VERSION` 2.4.5 → 2.4.6)

---

## [2.4.5] - 2026-05-21

### Added
- **User-saved defaults** (`POST /settings/saveDefault`, second EEPROM slot at offset 128): `SettingsStore::hasUserDefaults()` / `saveAsUserDefaults()`; `resetToDefaults()` now prefers user defaults over factory defaults. Web UI "Save as default" button and dynamic reset-button label.

### Changed
- `Connection: close` header on all HTTP responses (`WebUi::sendClose()`) forces immediate TCP release instead of leaving connections open for the 2s `HC_WAIT_CLOSE` window.
- Animation render cadence capped to a 25 ms minimum interval (~40 fps) so `server_.handleClient()` isn't starved of CPU during animations.

### Note
- Shipped with the critical infinite-recursion bug in `sendClose()` fixed in 2.4.6; was never stable.

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini` (`FIRMWARE_VERSION` 2.4.4 → 2.4.5)

---

## [2.4.4] - 2026-05-21

### Fixed
- **Critical: settings wiped on reboot when animations exceeded stale limits.** `SettingsStore::valid()` accepted animation indices up to 3/3/5 but the POST handler allowed 0–6/0–7/0–10 — any saved animation above the stale `valid()` limits caused all settings to silently reset to factory defaults on next reboot. Root cause of "intermittent" color/contrast resets. Fixed by aligning `valid()`/`sanitize()` bounds to the POST handler ranges.
- Web UI preview didn't reflect `middleFaceScale`/`innerFaceScale` (hardcoded to 50 in `draw()`); now reads live DOM values.

### Changed
- Contrast presets redesigned to exploit the bloom gradient: Defaults (90/50/50), Subtle Bloom (65/38/58), Deep Bloom (35/75/115), Crisp (80/10/15), Vivid (95/88/110). Old preset keys (soft/clear/high/balanced) replaced with (subtle/bloom/crisp/vivid).

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini` (`FIRMWARE_VERSION` 2.4.3 → 2.4.4)

---

## [2.4.3] - 2026-05-21

### Changed
- **Firmware version displayed on `/update` page**: info-box now shows `FIRMWARE_VERSION` build flag instead of hardcoded "Latest".

### Files changed
- `src/web_html.h` — `UPDATE_P1` split into `UPDATE_P1` + `UPDATE_P1B`.
- `src/main.cpp` — `/update` GET handler injects version string between PROGMEM chunks.
- `platformio.ini` — `FIRMWARE_VERSION` 2.4.2 → 2.4.3.

---

## [2.4.2] - 2026-05-18 (backfill — committed as 152edc4, session undocumented at time of commit)

### Added
- Per-ring face brightness controls: `middleFaceScale` / `innerFaceScale` EEPROM fields (0–255).
- Contrast preset dropdown in web UI Rings panel (Soft / Clear / High / Balanced Glow).
- `SETTINGS_VERSION` 12 → 13.

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`.

---

## [2.4.1] - 2026-05-18

### Fixed
- 8-inch factory defaults now use the same software ring rotation that the physical clock requires: `DEFAULT_OUTER_RING_OFFSET=1`.
- Reset-to-defaults now restores `outerRingOffset` from the build-time default instead of hardcoding `0`.
- 8-inch LED geometry remains explicit and valid: physical pixel 0 is sacrificial, ring pixels are 1-96, center pixel is 97, total strip count is 98.

### Changed
- `/diag` now reports `default_outer_ring_offset` beside the live saved `outer_ring_offset`.
- Serial LED status now prints `defaultRot` beside the live software rotation.
- Project docs now use the ChronoBloom anchor phrase and no longer reference the retired project nickname.
- `FIRMWARE_VERSION` bumped `2.3.6` -> `2.4.1`.

### Functions modified
- `SettingsStore::defaults()` - uses `DEFAULT_OUTER_RING_OFFSET` for factory `outerRingOffset`.
- `SettingsStore::sanitize()` - falls back to `DEFAULT_OUTER_RING_OFFSET` for invalid persisted ring rotations.
- `WebUi::setupRoutes()` - adds `default_outer_ring_offset` to `/diag`.
- `logRuntimeStatus()` - prints default software rotation in the serial status line.

### Files changed
- `platformio.ini` - firmware version, 8-inch/15-inch default outer ring rotation build flags.
- `src/main.cpp` - build-time default rotation macro, validation, defaults, diagnostics.
- `AGENTS.md`, `CLAUDE.md`, `WORKFLOW.md` - ChronoBloom wording cleanup.
- `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md` - regenerated.
- `docs/CHANGELOG.md`, `docs/SESSIONS.md`, `docs/SESSION_CLOSURE.md` - closure documentation.

---

## [2.4.0] - 2026-05-18

### Changed
- Replaced the previous 29 animation modes with a smaller set of 16 smoother, palette-aware animations: quarter x3, half-hour x3, hour x5, reminder x5.
- Tightened animation validation and sanitization limits to match the reduced mode counts.
- Updated reminder routing and animation phase names so removed animation modes are no longer exposed internally.
- Updated Web UI animation and focus reminder dropdown labels to match the new animation set.

### Functions modified
- `SettingsStore::valid()` - clamps animation mode maxima to the new supported mode counts.
- `SettingsStore::sanitize()` - resets removed animation modes to `0`.
- `ClockRenderer::triggerReminderDirectAnimation()` - maps reminder modes to the five remaining dedicated reminder animations.
- `ClockRenderer::animPhaseName()` - removes labels for deleted animation phases.
- `ClockRenderer::tickAnimation()` - dispatches only the retained animation functions.
- `ClockRenderer::animQ1()` through `ClockRenderer::animRem5()` - replaced with the new animation implementations.

### Files changed
- `src/main.cpp` - animation implementation overhaul and mode bounds.
- `src/web_html.h` - Web UI animation option labels and removed modes.
- `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md` - regenerated for shifted function ranges and reduced function count.
- `docs/SESSIONS.md` - session closure entry.

---

## [2.3.6] - 2026-05-17

### Fixed
- **`renderAnimFrame` uint32 underflow** caused every web preview animation to render exactly one frame then stop. `loop()` captures `now = millis()` before `webUi.loop()` runs; `/previewAnimation` sets `animStartMs_` to a timestamp newer than the captured `now`, so `elapsed = now - animStartMs_` underflowed to ~4.3 billion and immediately exceeded the termination threshold. Present since v2.1.0. Fixed by passing a fresh `millis()` into `renderAnimFrame()`.

### Files changed
- `src/main.cpp` (`loop()`), `platformio.ini` (`FIRMWARE_VERSION` 2.3.5 → 2.3.6)

---

## [2.3.5] - 2026-05-17

### Added
- `/diag` fields: `anim_phase` (current animation phase string), `last_anim_source` (quarter/halfhour/hour/reminder/preview), `last_anim_mode`, `settings_save_count`.

### Files changed
- `src/main.cpp`, `platformio.ini` (`FIRMWARE_VERSION` 2.3.4 → 2.3.5), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

---

## [2.3.4] - 2026-05-17

### Fixed
- **Browser cache**: the `/` GET handler had no cache headers, so browsers served a stale cached page and silently ignored all JS changes in 2.3.1–2.3.3. Added `Cache-Control: no-cache, no-store, must-revalidate`. Requires a hard-refresh after flashing to discard the old cached page.

### Files changed
- `src/main.cpp` (`WebUi::setupRoutes` `/` GET handler), `platformio.ini` (`FIRMWARE_VERSION` 2.3.3 → 2.3.4)

---

## [2.3.3] - 2026-05-17

### Fixed
- `animQ3` ignored the Animation Palette setting (drew configured clock colors instead); now fills outer/middle rings with palette-sampled colors in a bright/dim cycle.
- `saveAnimStyle()` triggered the purple "settings saved" LED flash on every style save; converted to async fetch with `silent=1`.

### Files changed
- `src/main.cpp` (`animQ3`), `src/web_html.h` (`saveAnimStyle`), `platformio.ini` (`FIRMWARE_VERSION` 2.3.2 → 2.3.3), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

---

## [2.3.2] - 2026-05-17

### Fixed
- `previewAnim()` was synchronous and ignored unsaved style slider values — preview always ran with the last-saved style. Now async; silently POSTs current style fields before triggering preview.
- `previewStyleAnim()` triggered the purple settings-saved flash and previewed hardcoded demo animations instead of the user's actual selected slot; now saves silently and previews the real slot/mode.
- `stylePreviewType` selector replaced 11 hardcoded demo options with 4 slot options (Quarter/Half-hour/Hour/Reminder "current selection").
- `/settings` POST gained a `silent` param to suppress the save-confirmation animation without skipping the actual save.
- `animQ3` and `animH1`/`animHr4` now respond to Palette and Speed sliders (`scaledElapsed()` and `paletteColor()` applied).

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini` (`FIRMWARE_VERSION` 2.3.1 → 2.3.2), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

---

## [2.3.1] - 2026-05-17

### Fixed
- Animation brightness: `animQ1`–`animQ3`, `animH1`–`animH3`, `animHr1`–`animHr5` all had hardcoded or missing `setBrightness()` calls; all now scale to `settings.animationBrightness`.
- `previewStyleAnim()` was synchronous and triggered the preview before style settings were saved; now awaits the `/settings` POST first.
- "Preview with" dropdown replaced with a direct `type:mode` picker listing 11 palette-capable animations, so preview always shows a meaningful demonstration regardless of assigned slot mode.

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini` (`FIRMWARE_VERSION` 2.3.0 → 2.3.1)

---

## [2.3.0] - 2026-05-17

### Added
- `outerRingBrightness` EEPROM field (0–100%): per-ring brightness multiplier for the outer 60-LED ring.
- Color Theme export/import panel: serializes all ring colors/levels + `outerRingBrightness` to a downloadable JSON theme file and re-applies it on import.

### Changed
- `SETTINGS_VERSION` 11 → 12. New factory defaults for outer marker/filler/seconds/minutes colors.

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini` (`FIRMWARE_VERSION` 2.2.2 → 2.3.0)

---

## [2.2.3] - 2026-05-17

### Fixed
- Web UI preview `draw()` ambient levels (22/24) didn't match the firmware's `renderFace()` ambient value (50/50) introduced in 2.2.2; corrected to match.

### Files changed
- `src/web_html.h`

---

## [2.2.2] - 2026-05-17

### Fixed
- `renderFace()` ambient scale: the `hoursLevel/6` and `centerLevel/6` ratio formulas from 2.2.1 produced different ambient levels per ring because the two fields have different defaults (255 vs 180). Replaced with a flat 50/255 (~20%) constant for both middle and inner rings.

### Files changed
- `src/main.cpp` (`renderFace`), `platformio.ini` (`FIRMWARE_VERSION` 2.2.1 → 2.2.2)

---

## [2.2.1] - 2026-05-17

### Added
- `POST /settings/reset` endpoint + "Reset all to defaults" button (Display panel, confirm-guarded).
- `/diag` extended with 7 fields: `effective_brightness`, `outer_marker_level`, `outer_filler_level`, `hours_level`, `center_level`, `middle_ambient_scale`, `inner_ambient_scale`.

### Fixed
- `renderFace()` ambient scale for middle/inner rings was hardcoded at ~8.6%/9.4%, far dimmer than the outer ring; scaled to `hoursLevel/6` and `centerLevel/6` (superseded in 2.2.2).

### Files changed
- `src/main.cpp`, `src/web_html.h`, `platformio.ini` (`FIRMWARE_VERSION` 2.2.0 → 2.2.1)

---

## [2.2.0] - 2026-05-16

### Added
- **Demo Mode**: `DemoMode` class, non-blocking state machine for video-recording sequences (idle clock, chime animations, palette cycling, focus reminder, auto-brightness ramp, end card; ~93s total). `LuxSensor::setLuxOverride()`/`clearLuxOverride()` for simulating darkness. Endpoints `POST /demo/start`, `POST /demo/stop`, `GET /demo/status`, `GET /demo/overlay` (OBS-ready HTML). No EEPROM/SETTINGS_VERSION changes.

### Changed (refactor — no behavior change)
- WebUI HTML/JS extracted from `WebUi::setupRoutes()` (742 → 313 lines) into PROGMEM headers in new file `src/web_html.h`.
- `ClockRenderer::tickAnimation` decomposed from a 650-line switch into a 35-line dispatcher plus 30 per-case private methods (`animQ1`–`Q6`, `animH1`–`H7`, `animHr1`–`Hr10`, `animRem1`–`Rem6`).

### Files changed
- `src/main.cpp`, `src/web_html.h` (new), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/HARDWARE.md`, `WORKFLOW.md`

---

## [2.1.1] - 2026-05-16

### Added
- `/diag` endpoint expanded: uptime, firmware/settings versions, time, NTP sync state, WiFi status/SSID/RSSI/IP, lux, brightness chain, button event counter, free heap, LED pixel config.

### Fixed
- `setRingPixel` rotation rounding: middle/inner rings previously moved in quantized steps of 2.5/5 LEDs, so most single-step `outerRingOffset` changes produced no visible movement on those rings.
- Brightness floor in `sanitize()`: `dayBrightness < 5` floors to 44, `nightBrightness < 1` floors to 5, preventing WebUI lockout from an accidental zero-brightness input.

### Changed
- `FocusReminderScheduler::lastFireMs` moved to RAM (no longer persisted to EEPROM on every fire) — eliminates ~20k EEPROM writes/year for typical schedules.
- `DemoMode::statusJson` migrated to `snprintf`.

### Files changed
- `src/main.cpp`, `platformio.ini` (`FIRMWARE_VERSION` 2.1.0 → 2.1.1)

---

## [2.1.0] — 2026-05-16 (docs)

### Documentation
- LICENSE (Apache 2.0) and LICENSE-HARDWARE (CC BY 4.0) added to repo root
- NOTICE file with attribution chain (Steve Manley → Mike van der Sluis → Maestro8484)
- docs/publish/PUBLISH.md: publishing tracker, platform targets, STL notes
- docs/publish/DEMO_MODE.md: Demo Mode firmware spec — step table, /demo/status, /demo/overlay, LuxSensor override API

---

## [2.1.0] — 2026-05-15

### Added
- Animation palette system: 8 color palettes (Rainbow, Fire, Ocean, Forest, Candy, Neon, Monochrome, Clock)
- Reminder palette system: 4 warm palettes (Amber, Red, Magenta, Cyan-warm) for focus reminder animations
- Animation speed control: 5-step scale (Dreamy slow → Hyperactive)
- Animation peak brightness control: 50-255 range, independent of clock brightness
- Trail length control: 2-12 LEDs for chase/sweep animations
- 3 new quarter-hour animations: Laser ping, DNA twist, Tick spark
- 4 new half-hour animations: Comet chase, Color explosion, Knight Rider, Strobe party
- 5 new hour animations: Supernova, Matrix rain, Galaxy spin, Color wipe, Thunderstorm
- 6 dedicated focus reminder animations: Amber pulse, Attention ring, Heartbeat, Sunrise wake, Campfire flicker, Neon sign
- Web UI: Preview button for each animation selector
- Web UI: Animation Style panel (palette, speed, brightness, trail length)
- Web UI: 12-hour time format with AM/PM in clock display
- `/previewAnimation` POST endpoint
- `/diag`: `fw` field showing firmware version string (2.1.0)

### Changed
- SETTINGS_VERSION bumped 10 → 11 (settings reset to defaults on first boot after update)
- `focusReminder_animation` modes 0-5 now labeled as animation delegates; modes 6-11 are dedicated reminder animations
- `triggerReminderAnimation()` routing consolidated into `ClockRenderer::triggerReminderDirectAnimation()`

---

## [2.1.0] - 2026-05-13

### Fixed
- **WebUI crash on 15inch variant** — `htmlPage()` (~6KB String on heap) replaced with three `PROGMEM const char[]` chunks streamed via `server_.setContentLength(CONTENT_LENGTH_UNKNOWN)` + `sendContent_P()`. No heap allocation for the page payload.
- **`/wifi` GET handler** — large inline HTML with `.replace()` converted to PROGMEM chunks + two small `sendContent()` calls for dynamic SSID/status values.
- **`/update` GET handler** — large inline HTML converted to two PROGMEM chunks.
- **`settingsJson()`** — ~30 `String +` concatenations (repeated heap reallocs) replaced with `snprintf` into `char buf[900]` + inline color hex formatting. Zero heap allocs except final `String(buf)` return.
- **`/time`, `/net`, `/diag` JSON handlers** — `String +` concatenation chains replaced with `snprintf` into stack `char buf[]` (128/256 bytes). Payload passed directly to `server_.send()` with no heap `String` object.

### Files changed
- `src/main.cpp` — `WebUi::setupRoutes()`, `WebUi::settingsJson()`, `WebUi::htmlPage()`
- `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md` — regenerated

---

## [2.0.9] - 2026-05-13

### Added
- **`GET /wifi` — WiFi settings page** (`WebUi::setupRoutes`)
  - Dark-themed standalone page matching `/update` visual style
  - Shows current saved SSID and live WiFi status; password never echoed
  - SSID/password form POSTs to `/wifi`; JS handles response and reconnect feedback
- **`POST /wifi` — credential save + reconnect** (`WebUi::setupRoutes`)
  - Validates SSID length (1–32 chars); saves `ssid`/`pass` to Preferences `"wifi"` namespace
  - Responds 200, then `WiFi.disconnect()` + `WiFi.begin()` with new credentials
- **Preferences `"wifi"` tier in `setupWiFi()`** — priority 1 before build-time SSID
  - On forcePortal (factory reset): `"wifi"` namespace cleared before launching portal
- **30s non-blocking STA reconnect poll** in `WebUi::loop()` — skipped if `apMode_` is true
- **`WebUi::apMode() const`** accessor
- **WiFi Settings link** added to Admin panel in `htmlPage()`

### Files changed
- `src/main.cpp` — `setupWiFi()`, `WebUi::loop()`, `WebUi::apMode()`, `WebUi::setupRoutes()`, `WebUi::htmlPage()`
- `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md` — regenerated (98 functions)

---

## [2.0.8] - 2026-05-13

### Added
- **Button-hold factory reset on boot** (`src/main.cpp`, `platformio.ini`)
  - Hold UP (GPIO5) at power-on → LEDs turn red; add DOWN (GPIO9) and hold both 3s → factory reset
  - GPIO5 used for initial detection (GPIO9 is XIAO BOOT pin — holding it at reset instant enters download mode)
  - `SettingsStore::resetToDefaults()`: writes zero to EEPROM magic byte, forces defaults on next boot
  - `Preferences "factory/portal"` flag persists across reboot; consumed in `setupWiFi()` to force WiFiManager portal
  - `setupWiFi()`: `WiFi.disconnect(false, true)` + `wm.resetSettings()` + `wm.startConfigPortal()` bypasses hardcoded credentials
  - Portal LED feedback: all LEDs turn blue when `esp32c3-clock-setup` SSID is broadcasting
  - Portal stays open indefinitely (`setConfigPortalTimeout(0)`) until credentials saved
  - `wm.setConnectRetries(1)` — reduces pre-portal wait from ~60s to ~8s on credential failure
  - `#include <Preferences.h>` added; forward declaration `extern Adafruit_NeoPixel ledStrip` added before `setupWiFi()`

### Changed
- **`platformio.ini`**: both envs now use `upload_protocol = esptool` / `upload_port = COM6`; removed espota from 8inch env
- **`platformio.ini`**: `RING_PIXEL_OFFSET` remains 1 — confirmed correct; offset 0 is invalid when `SACRIFICIAL_PIXEL_ENABLED=1`

### Files changed
- `src/main.cpp` — `SettingsStore::resetToDefaults()`, factory reset block in `setup()`, `setupWiFi()` portal logic + LED callback
- `platformio.ini` — upload protocol/port both envs; ring offset correction; sacrificial pixel confirmed

---

## [2.0.7] - 2026-05-13

### Added
- **`ButtonInput` hold-to-repeat** (`src/main.cpp`)
  - Short press: +1/-1 minute (unchanged)
  - Hold >500ms: auto-repeat at +1/-1 minute every 150ms
  - Hold >2000ms: switches to +60/-60 minute per fire (hour jump)
  - Release: stops repeat, resets all hold state
  - New state per button: `pressedAt`, `lastRepeatAt`, `holdPhase` (enum `IDLE`/`REPEAT_MIN`/`REPEAT_HOUR`)
  - New `consumeUp()`/`consumeDown()` return int delta; `loop()` passes delta to `addMinutes()`
  - No ISRs, no mode changes, no LED feedback changes

### Changed
- **GPIO assignments swapped** (`platformio.ini`, `src/main.cpp` fallback defines)
  - `BUTTON_UP_PIN`: 9 → 5 (GPIO5 / D3)
  - `BUTTON_DOWN_PIN`: 5 → 9 (GPIO9 / D9)

### Files changed
- `src/main.cpp` — `ButtonInput` class (hold-to-repeat state machine, `consumeUp`/`consumeDown`), fallback `#define` for `BUTTON_DOWN_PIN` fixed (was 5, now 9)
- `platformio.ini` — `BUTTON_UP_PIN`/`BUTTON_DOWN_PIN` values swapped

---

## [2.0.6] - 2026-05-13

### Added
- **`GET /diag` diagnostic endpoint** (`src/main.cpp`)
  - Returns JSON with 10 fields: `uptime`, `firmware_version`, `boot_reason`, `free_heap`, `wifi_ssid`, `wifi_rssi`, `wifi_ip`, `ntp_synced`, `ntp_last_delta`, `button_events`
  - `TimeSync::lastDeltaSec_`: tracks seconds delta between model time and NTP time at each sync
  - `g_buttonEventCount`: global counter incremented on every consumed button press (up or down)
- **`tools/gen_symmap.py`** — canonical script to regenerate `docs/symmap.json` and `docs/FUNCTION_INVENTORY.md` from `src/main.cpp`

### Changed
- **`docs/symmap.json` + `docs/FUNCTION_INVENTORY.md`** regenerated: 93 functions (was 87); ButtonInput class (4 methods) + 2 constructors newly tracked; all line numbers updated for S5 button re-add shift

### Files changed
- `src/main.cpp` — `/diag` route, `TimeSync::lastDeltaSec_`, `g_buttonEventCount`
- `tools/gen_symmap.py` — new file
- `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md` — regenerated

---

## [2.0.5] - 2026-05-11

### Added
- **Software watchdog** (`src/main.cpp`)
  - `esp_task_wdt_init(10, true)` + `esp_task_wdt_add(NULL)` in `setup()` — 10-second window, reboot on timeout
  - `esp_task_wdt_reset()` at top of `loop()`, in `ArduinoOTA.onProgress`, and in `UPLOAD_FILE_WRITE` handler
  - Ensures device recovers automatically from OTA hang, I2C block, or WiFi manager stall
- **Web UI firmware update (`/update` page)** (`src/main.cpp`)
  - Fixed raw-binary XHR upload OOM crash at ~40% (switched to `FormData` for streaming multipart)
  - Fixed `Update.begin(0)` failure with FormData by using `UPDATE_SIZE_UNKNOWN`
  - Guard: skip too-large check when `upload.totalSize == 0` (FormData reports 0 at `UPLOAD_FILE_START`)

### Changed
- **OTA password removed** — `ArduinoOTA.setPassword("iris_ota_2026")` removed; no auth required
- **`platformio.ini`** — consolidated `esp32c3_v3_8inch_ota` env into `esp32c3_v3_8inch`; `upload_protocol = espota` and `upload_port = 192.168.1.x` now default for 8" variant

### Files changed
- `src/main.cpp` — watchdog init/feed, FormData fix, UPDATE_SIZE_UNKNOWN, OTA password removed
- `platformio.ini` — espota as default protocol, removed separate OTA env
- `docs/PRIMER.md`, `docs/SESSIONS.md`, `docs/TROUBLESHOOTING.md`, `README.md` — OTA command references updated

---

## [2.0.4] - 2026-05-10

### Removed
- **Physical buttons removed (both variants)**
  - GPIO3 (Button UP) and GPIO4 (Button DOWN) are no longer wired or used
  - Removed from hardware on both 8" and 15" variants
  - All manual time adjustment now handled exclusively via WebUI

### Reason for removal
  - GPIO3 and GPIO4 are JTAG TCK/TDI pins on the XIAO ESP32-C3
  - USB data connection caused spurious ISR fires on both pins, triggering
    unintended addMinutes(-1) calls and backward minute-hand jumps
  - Buttons were not essential given full WebUI time control
  - Removal eliminates the highest-confidence known input-corruption path
    documented in REVIEW.md

### Re-addition note
  - Buttons may be reintroduced in a future version
  - If re-added, implementation MUST use polled reads (not ISRs) and MUST
    avoid GPIO3/GPIO4 -- use GPIO6, GPIO7, GPIO8, or GPIO9 instead
  - See REVIEW.md Section 1 for full technical rationale and the polling
    implementation template

### Files changed
- `platformio.ini` -- removed BUTTON_UP_PIN and BUTTON_DOWN_PIN build flags
- `src/main.cpp` -- removed ButtonInput class, button poll/consume calls, button status animation
- `docs/HARDWARE.md` -- removed button pin rows from pin table, added removal note
- `docs/FEATURES.md` -- moved physical buttons to Removed Features section
- `README.md` -- removed button references from hardware notes and pin table

## [2.0.3] - 2026-05-09

### Changed
- **`renderHours`: thirds-based hour ring progression** (`src/main.cpp`)
  - Middle ring (24 LED) now advances in three steps across each hour:
    - `:00–:19` → single pixel at exact hour position (clean top-of-hour anchor)
    - `:20–:39` → two-pixel straddle (mid-transit between positions)
    - `:40–:59` → single pixel at advanced position (approaching next hour)
  - Previously used a binary half-hour offset (`minute >= 30 ? +1 : 0`), which left `:15` visually ambiguous (straddling when it should have been a clean anchor)
  - Inner ring (12 LED) unchanged in behaviour but clarified: single pixel at `:00–:29`, two-pixel straddle at `:30–:59`

### Files changed
- `src/main.cpp` — `ClockRenderer::renderHours` only

---

## [2.0.2] - 2026-05-08

### Fixed
- **Sacrificial LED removed (both variants)**
  - `RING_PIXEL_OFFSET` set to 0 on both 8" and 15" (rings now start at physical index 0)
  - 8": `CLOCK_PIXEL_COUNT` 98→97, `CENTER_PIXEL_INDEX` 97→96
  - 15": `CLOCK_PIXEL_COUNT` 97→96
  - `SACRIFICIAL_PIXEL_ENABLED=0` on both variants; `SACRIFICIAL_PIXEL_INDEX` removed
  - **Hardware required:** rewire GPIO10 data line directly to DIN of first ring LED

- **15" ring LED (GPIO10) failure**
  - Root cause: dead sacrificial WS2812B at chain position 0 blocked all signal to rings
  - Resolved by above pixel offset/count changes + hardware rework

- **WiFi: AP fallback when STA unavailable**
  - Added `wm.setConfigPortalTimeout(120)` — portal releases after 2 min instead of blocking forever
  - If all STA attempts fail, device starts `WiFi.softAP(DEVICE_HOSTNAME)` and runs web server at `192.168.4.1`
  - Clock, web UI, and settings all functional in AP mode; NTP/mDNS/OTA skipped until STA available

- **mDNS hostname lost after WiFi reconnect**
  - Added `WiFi.onEvent(ARDUINO_EVENT_WIFI_STA_GOT_IP)` handler to re-run `MDNS.begin()` on reconnect

- **FocusReminder day-of-week always fired as Sunday**
  - `getDayOfWeek()` now uses `time(nullptr)` + `localtime_r()` for actual NTP weekday
  - Graceful fallback to Sunday (0) before first NTP sync

### Files changed
- `platformio.ini` — pixel counts, offsets, sacrificial flags
- `src/main.cpp` — WiFi AP fallback, portal timeout, mDNS reconnect handler, DOW fix

---

## [2.0.1] - 2026-05-08

### Changed
- `platformio.ini`
  - Changed `default_envs = esp32c3_v3_8inch` to `default_envs = esp32c3_v3_15inch` so plain `pio run` / VS Code default tasks target the installed 15-inch clock variant.
  - Added `-D ARDUINO_USB_MODE=1` to the shared ESP32-C3 build flags so the native USB interface is configured for USB CDC operation.
  - Added `-D ARDUINO_USB_CDC_ON_BOOT=1` to the shared ESP32-C3 build flags so `Serial` output appears on the Windows COM port after boot.
- `src/main.cpp`
  - Replaced WiFiManager-only `setupWiFi()` behavior with a build-time credential first path.
  - Added a guard that skips build-time credential mode only when `WIFI_SSID` is still the placeholder `clock-ssid`.
  - Added serial logging before the build-time connection attempt: `[WiFi] Trying build-time SSID: ...`.
  - Added `WiFi.begin(WIFI_SSID, WIFI_PASSWORD)` so credentials from `platformio.ini` are actually used.
  - Added a timed connection loop using `WIFI_CONNECT_TIMEOUT_MS`.
  - Added progress dot output every 500 ms during the build-time Wi-Fi attempt.
  - Added a newline after the Wi-Fi connection attempt finishes.
  - Added success handling that logs `[WiFi] Connected with build-time credentials` and returns `true`.
  - Added failure handling that logs the Wi-Fi status and then falls back to the existing `esp32c3-clock-setup` WiFiManager portal.
  - Added `WiFi.disconnect(false)` and a short delay before starting the fallback portal.
  - Kept the original `wm.autoConnect("esp32c3-clock-setup", "")` path as fallback behavior.
  - Added global `lastStatusLogMs` state for periodic runtime serial logging.
  - Added `logRuntimeStatus(uint32_t now)`, which prints uptime, Wi-Fi status code, hostname, connected SSID, IP address, RSSI, and free heap every 10 seconds.
  - Added `logRuntimeStatus(now)` to the main loop after `timeSync.loop()` and `webUi.loop()` so serial status is available even if the monitor attaches after boot.
  - Removed trailing blank lines at the end of `src/main.cpp`.

### Verified
- Flashed `esp32c3_v3_15inch` over USB on `COM9`.
- Confirmed the device connects to configured SSID from `platformio.ini`.
- Confirmed mDNS resolves `esp32c3-v3-15inch.local` to device IP.
- Confirmed `http://esp32c3-v3-15inch.local/` returns HTTP 200.
- Confirmed `/net` reports correct SSID, `status=3`, and expected RSSI range.
- Confirmed serial monitor on `COM9` at `115200` prints runtime status lines such as:
  ```text
  [Status] uptime=10s wifi=3 host=esp32c3-v3-15inch ssid=<YOUR_SSID> ip=192.168.1.x rssi=-73 heap=242972
  ```

### Restore-Back Instructions

Use one of these restore paths depending on whether the changes have been committed.

#### Restore with Git Before Commit
From the repo root:

```powershell
git -c safe.directory=C:/path/to/chronobloom-esp32c3 checkout -- platformio.ini src/main.cpp docs/CHANGELOG.md
```

This removes the 15-inch default env change, USB CDC serial flags, build-time Wi-Fi credential path, periodic status heartbeat, and this changelog entry.

#### Restore Manually
1. In `platformio.ini`, change:
   ```ini
   default_envs = esp32c3_v3_15inch
   ```
   back to:
   ```ini
   default_envs = esp32c3_v3_8inch
   ```
2. In `platformio.ini`, remove these two lines from `[common] build_flags`:
   ```ini
   -D ARDUINO_USB_MODE=1
   -D ARDUINO_USB_CDC_ON_BOOT=1
   ```
3. In `src/main.cpp`, replace the expanded `setupWiFi()` body with the previous WiFiManager-only implementation:
   ```cpp
   bool setupWiFi() {
     WiFiManager wm;
     wm.setConnectRetries(3);
     bool connected = wm.autoConnect("esp32c3-clock-setup", "");
     return connected;
   }
   ```
4. In `src/main.cpp`, remove:
   ```cpp
   uint32_t lastStatusLogMs = 0;
   ```
5. In `src/main.cpp`, remove the whole `logRuntimeStatus(uint32_t now)` function.
6. In `src/main.cpp`, remove this call from `loop()`:
   ```cpp
   logRuntimeStatus(now);
   ```
7. In `docs/CHANGELOG.md`, remove this entire `2.0.1` section.
8. Rebuild the desired variant:
   ```powershell
   pio run -e esp32c3_v3_8inch
   pio run -e esp32c3_v3_15inch
   ```

#### Restore After Commit
If these changes have already been committed, revert that commit instead of editing files by hand:

```powershell
git -c safe.directory=C:/path/to/chronobloom-esp32c3 revert <commit-sha>
```

Then rebuild and flash the desired environment.

## [2.0.0] - 2026-05-06

### Added
- **Focus Reminders (ADHD MVP)** - Visual nudge system for hyperfocus interruption
  - Single configurable reminder rule with enable/disable toggle
  - Configurable active hours window (start/end hour)
  - Repeat interval in minutes (1-1440 min)
  - Days-of-week selector (Sun-Sat bitmask)
  - Reuses existing animation system (Quarter Pulse, Half-Hour Sweep, Hour Chime)
  - Persistent storage via EEPROM (16 bytes added to ClockSettings)
  - WebUI panel: "Focus Reminders (ADHD)" with form controls
  - Serial logging of reminder fires for debugging

### Changed
- **SETTINGS_VERSION bumped 7 → 8**
  - Old v7 settings auto-reset to v8 defaults on first boot (backward compatible)
  - `ClockSettings` struct extended by 16 bytes (13 used + 3 reserved for v2)
  - Added `FocusReminderScheduler` class to firmware main loop

### Technical Details
- **EEPROM footprint:** +16 bytes (total 256 bytes, ~60 bytes headroom remaining)
- **Code footprint:** ~300 lines added (FocusReminderScheduler class, WebUI panel, helpers)
- **Loop performance:** Reminder check runs in < 1ms, non-blocking
- **Files modified:** `src/main.cpp` (only file changed)

### Known Limitations (v1)
- Day-of-week calculation hardcoded to Sunday (0) - placeholder pending NTP weekday integration
- No test-now button (manual time adjustment required to validate)
- No animation queueing (reminder can overlap with status/chime animations)
- No quiet-mode or sleep-mode exemption logic
- Single reminder rule only (multi-reminder planned for v2.1)

### Future Roadmap (v2.1+)
- Multiple reminder rules (3-5 concurrent reminders)
- Day-of-week auto-calculation from NTP system time
- Test-now button on WebUI
- Animation queue to prevent overlap
- Quiet/sleep mode exemption flag per reminder
- Custom reminder labels/names
- Optional soft-sound/buzzer integration (for non-visual users)

### Migration Guide (v1.x → v2.0)
1. Upload new firmware (v2.0 binary)
2. Device boots, EEPROM auto-resets to v8 defaults
3. Old display/animation settings preserved
4. New "Focus Reminders (ADHD)" panel appears on WebUI
5. Configure reminder as desired; click "Save reminder"
6. Test by setting time to window + interval, verify animation fires
