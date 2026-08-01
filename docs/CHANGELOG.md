# ChronoBloom ESP32-C3 — Changelog

> Formerly neopixelClock-esp32c3-v3

## [2.31.5] - 2026-07-31 (No more 10-second pause on every boot of a portal-set-up clock)

Built clean on both envs. **Not yet observed on hardware.** The check that would confirm it: set a
board up through the clock's own setup network, then power it off and on with USB serial open. The
teal ring and the `[Improv] No stored WiFi credentials` line should not appear at all, and the
portal's `*wm:AutoConnect` line should arrive at roughly 0.5 s instead of roughly 9.5 s.

### Fixed
- **A clock set up through its own setup network paused 10 seconds on every power-on.** Before the
  clock does anything else it listens over USB for a browser offering it WiFi, and it skips that
  wait when it already has WiFi. It decided that by looking in one place: the credentials saved by
  the clock's own WiFi page. Credentials entered on the setup network at 192.168.4.1 are not kept
  there - the WiFi chip keeps its own copy - so the clock concluded it had never been set up and
  waited out the full window on every single boot, before it even tried to join the network.

  Since the flasher page recommends the setup network as the reliable route, that was the common
  case for anyone new, and it cost them 10 seconds of dark clock at every power-on.

  The check now asks all three places credentials can live, the same three `setupWiFi()` already
  asks: the WiFi page's own store, the WiFi chip's copy (`WiFiManager::getWiFiIsSaved()`, the same
  call `setupWiFi()` uses, rather than a second hand-written version of it), and any network baked
  in at build time. Anything found means the clock is set up and skips the wait. This can only ever
  make the clock skip the wait more readily, never less, so it cannot lock anyone out of setting up
  a new board; a board holding credentials that no longer work still falls through to the setup
  network, exactly as it does today. Sequencing was checked rather than assumed: the WiFi stack is
  started one line earlier, so it can answer the question by the time it is asked.
  (`src/main.cpp`, `WebUi::begin()`.)

## [2.31.4] - 2026-07-31 (The browser's WiFi box no longer times out mid-typing)

Built clean on both envs and **verified on hardware**, against the published 2.31.3 as a control.
Both binaries were flashed onto a fully erased board over USB and given the same test: answer the
browser handshake, then go completely silent for 35 seconds, the way a person reads the box and
types a password, then send the credentials late.

| | 2.31.3 (published) | 2.31.4 |
|---|---|---|
| Handshake answered | 0.12 s | 0.12 s |
| Setup portal opened | 9.87 s | did not open |
| Credentials sent at | 35.14 s | 35.22 s |
| Device received them | **no, silence** | yes, at 35.33 s |
| Result | timed out, portal fallback needed | provisioned at 37.91 s, connected, mDNS + NTP up |

Confirmed afterwards against the running device: `/diag` reports `firmware_version 2.31.4`.

### Fixed
- **The browser's "Configure WiFi" box timed out on a freshly flashed board.** Reported from the
  first real bare-board flash test (2026-07-30): flashing worked, the box appeared, credentials
  were entered, and it timed out anyway. Fallback to the clock's own setup network worked.

  The clock listened for a browser over USB for a fixed 10 seconds at boot and then stopped, moving
  on to the captive portal - which blocks for its whole lifetime, so the main loop never ran and
  nothing serviced the browser again. Ten seconds is less time than it takes anyone to read the
  box, pick a network and type a password, so the answer arrived at a device that had stopped
  listening. That made it the expected outcome for nearly every user, not bad luck.

  Now the deadline is pushed out as long as a client keeps talking: 10 seconds of silence still
  closes the window, but any complete, checksum-valid Improv frame - including the browser's
  opening handshake, which fires while the user is still reading the dialog - buys another 60
  seconds. A board nobody is talking to behaves exactly as before and opens the portal on the same
  schedule. Only line noise is ignored, since a partial frame never counts as a client.
  (`IMPROV_CLIENT_WINDOW_MS`, `src/main.cpp`.)

### Measured, on the way to that fix
- A client polling twice a second over USB serial gets no reply at all once the clock is connected
  to WiFi, confirming Improv is deliberately deactivated on a running clock and that USB cannot
  silently reconfigure one. Read off a live board on COM12.

## [2.31.3] - 2026-07-29 (Mood palettes, second pass: saturation held)

Data-only, no firmware logic touched. Build-verified on the 8"; **not yet observed on hardware** -
both clocks run 2.31.1.

### Fixed
- **The v2.31.1 mood palettes still read monochrome on the strip, and the cause was saturation,
  not hue.** v2.31.1 gave every mood real hue travel measured in HSL degrees, but produced its
  "brightens toward the core" by *desaturating* each ring inward. WS2812s render low saturation as
  white, so Moonlight ran 65% -> 46% -> 32% -> 8% HSV saturation: violet at the rim, then white,
  white, white. The hue travel was real in the arithmetic and invisible in the room.
  `tools/palettes/palettes.json` had warned about exactly this in its own header ("WS2812 washes
  pastels to white; keep saturation up") and the first pass ignored it.

  All four moods re-authored to hold saturation high on every ring and take the brightening from
  value instead. The core still reads as the brightest thing on the face because it is one LED
  against sixty; a point light needs no help.

  | Mood | Hue travel | Saturation range | Reads as |
  |---|---|---|---|
  | Golden hour | 38 deg | 84-95% | red-orange rim to saturated gold core |
  | Moonlight | 81 deg | 66-85% | violet rim through blue to cyan core |
  | Dawn | 63 deg | 73-85% | rose rim through coral to amber core |
  | Twilight | 80 deg | 67-81% | indigo rim through blue to teal core |

  The re-authoring pass now **gates** on saturation >= 58%, hue travel >= 35 deg, monotonic hue
  outward-to-inward, and a core brighter than the rim, refusing to write if any gate fails, so this
  class of mistake cannot land silently again.

### Note
- `src/anim_palettes.h` and `tools/gen_palettes.py` here also carry a concurrent session's
  comment-only em-dash cleanup. Those two plus `palettes.json` are one generated set; splitting
  them would leave the header disagreeing with its own source.

## [2.31.2] - 2026-07-29 (The public repo compiles again, and now it cannot stop compiling)

Two halves. First, the publish pipeline was broken and the public repo had been un-buildable since
the v2.29.0 push on 2026-07-18. Second, and the reason this is worth a version: the checks that
would have caught it are now code that runs, not prose in a document nobody executes.

### Fixed (firmware, user-visible)
- **Four em dashes removed from the web UI**, in the footer credit and the two auto-brightness
  labels (`src/web_html.h:59,74,75,77`), plus two in JS comments. `CLAUDE.md` bans em dashes in
  human-facing prose and every shipped `.md` was already clean, but nobody had ever proofread the
  UI strings, so they had been on screen since v2.30.0. This is the only reason `FIRMWARE_VERSION`
  moves: `src/web_html.h` changed.
- **The palette generator was reintroducing them.** `src/anim_palettes.h` carried seven em dashes,
  and since it is generated and headed "DO NOT EDIT", fixing the file would have been undone by the
  next run. Fixed the emitted template in `tools/gen_palettes.py` instead and regenerated. Verified
  the palette data is byte-identical: the diff is 7 comment lines and zero color values.

### Fixed
- **The published repo could not compile.** `src/main.cpp:20` includes `anim_palettes.h`, but that
  header was never added to the export manifest, so `github.com/Maestro8484/ChronoBloom` shipped a
  source file with a missing dependency. Proven by cloning the public repo and building it:
  `fatal error: anim_palettes.h: No such file or directory`. Added `src/anim_palettes.h` to the
  manifest. Both variants now build from the exact published file set (8" 809,530 B, 15" 809,750 B,
  61.8% flash, 0 warnings).
- **A bare `pio run` built the wrong variant.** `default_envs` was `esp32c3_v3_15inch`, the
  experimental 15" build, while every README command passes `-e esp32c3_v3_8inch` explicitly. Anyone
  typing plain `pio run` got 96-pixel firmware with a separate center strip on GPIO20, which on an
  8" build is a scrambled face and no error message. Now defaults to the 8".
- **One uncommitted file could jam the whole export.** The manifest globbed
  `ChronoBloom_8inch/v11/**` from the filesystem, not from git, so an untracked work-in-progress
  `.3mf` sitting in that folder failed the export gate and blocked every publish. Replaced the glob
  with the six explicit tracked filenames, which removes the failure mode by construction rather
  than by remembering to tidy up.

### Removed from the public export
Excluded, not deleted. Every file still exists in the private repo and still works; they just stop
shipping. All are recorded in the manifest's `exclude_assert` list so they cannot drift back in.
- **The demo-reel production rig**, which is maintainer tooling with no value to a builder:
  `demo_reel_designer.html`, `scripts/demo_reel_designer.bat`, `scripts/reel_server.py`, both
  `DEMO_CAPTIONS` tracks, `DEMO_MODE.md`, and all three `tools/video/*.ps1` cutting scripts.
  On-device **Demo Mode stays** — it is a button in the web UI that every user gets, so
  `docs/FEATURES.md` now describes it in user terms instead of as a video-recording state machine.
- **`ChronoBloom_QuickRef.docx`** — opened with "Built from scratch. Coded alone.", which
  contradicts the NOTICE crediting Steve Manley's original and the README's own voice.
- **`ChronoBloom_Compact_Function_Inventory.md` / `.docx`** — stated "v2.2.0, 139 functions" against
  a current 2.31.1 with 233. No generator exists for it, so it is hand-maintained and 29 versions
  stale. `docs/API.md` and `docs/ARCHITECTURE.md` cover the same ground.

### Added
- The annotated web UI screenshot is now actually used. It had been shipping into `docs/images/` at
  727 KB while no document referenced it. New "The web UI" section in `README.md` embeds it.

### Added: three new export gates, because the old failure was a doc, not a bug
The 2026-07-12 audit recommended, verbatim, "clean-clone build passes BOTH variants" as a hard
pre-publish gate. It was written into `docs/publish/qa-deep-2026-07-12-PUBLISH-PATCH.md` marked
"owner applies manually", and never performed. Eleven days of a broken public repo followed. Prose
does not run. `tools/publish/export_public.py` had five gates, all in code, all fail-closed, and all
five did their job. The missing ones are now the sixth, seventh and eighth.

- **Gate 6, em dashes in shipped prose.** Scans everything that ships, source files included,
  because `web_html.h` is UI copy and docs get proofread while UI strings do not. Two files are
  exempt through a new `em_dash_exempt` manifest key with the reasoning recorded next to it: the
  historical changelog (rewriting shipped history to satisfy a later style rule would misrepresent
  what those entries said) and `main.cpp` (about 170, all code comments, none user-visible). The
  suffix list stays broad so a new file is scanned by default; exemption is an explicit entry, never
  a silent default.
- **Gate 7, flasher version match.** The flasher manifest's version must equal `FIRMWARE_VERSION`,
  every binary it references must be in the export set, and no stale binary of another version may
  ride along. This is the gate for the drift that had Pages serving 2.29.0 against a 2.31.1 source
  tree with nothing tying the two numbers together.
- **Gate 8, the export set must compile.** Materializes the export into a throwaway directory and
  builds every `[env:...]` in the shipped `platformio.ini`, the way a stranger's clone would. Env
  list is derived, not hardcoded, so a new variant is covered with no change here. Runs last because
  it is the slow one, and skips if a cheaper gate already failed. `--no-build-check` exists and says
  what skipping it cost last time.
- **Verified by deliberately breaking it:** removing `src/anim_palettes.h` from the manifest and
  re-running produced `EXPORT SET DOES NOT COMPILE` for both variants, exit 1, nothing staged. A
  gate nobody has watched fail is a gate nobody should trust.

### Fixed (tooling, found by testing the new gates)
- **A gate failure could exit looking like success.** Gate messages quote file content, and printing
  an arrow character out of `docs/CHANGELOG.md` raised `UnicodeEncodeError` on the default Windows
  console codepage. That killed the process before `sys.exit(1)` ran, so a real failure surfaced as
  a traceback with a misleading exit code. `stdout`/`stderr` are now reconfigured to UTF-8 with
  replacement. Pre-existing; only surfaced because the new gate generated enough output to hit it.

### Files changed
`src/web_html.h`, `src/anim_palettes.h` (regenerated), `tools/gen_palettes.py`,
`platformio.ini` (`FIRMWARE_VERSION` 2.31.1 -> 2.31.2, `default_envs` 15inch -> 8inch),
`tools/publish/export_public.py`, `tools/publish/export_manifest.json`, `README.md`,
`docs/FEATURES.md`, `docs/ANIMATIONS.md`, `docs/CHANGELOG.md`

## [2.31.1] - 2026-07-28 (Mood palettes get radial hue travel)

Data-only change: `tools/palettes/palettes.json` re-authored, `src/anim_palettes.h` regenerated by
`tools/gen_palettes.py`. No firmware logic touched. Build-verified; **not yet observed on hardware**
— the on-strip look is what decides these values, so expect a live-tuning pass.

### Changed
- **All four mood palettes now travel hue from ring to ring.** Every mood was previously a single
  hue that only got lighter toward the center (Golden hour was amber on amber on amber), so a bloom
  animation on any mood read as one flat wash — most visibly on nudges, where Golden hour is the
  factory default and Bloom Ripple came out one red-orange no matter what. Each palette now steps
  hue outward-to-inward while still brightening toward the core:
  - Golden hour: red-orange rim → amber → gold → pale gold core (38° of hue travel)
  - Moonlight: violet rim → periwinkle → ice blue → cool white core (77°)
  - Dawn: rose rim → coral → peach → cream core (57°)
  - Twilight: indigo rim → blue → sky → pale cyan core (55°)

  Authored gamma-aware as before (`rings` = inverse-gamma of `target`); the `target` field in
  `palettes.json` now records the intended on-strip appearance for every ring.

## [2.31.0] - 2026-07-28 (Launch-audit fixes: the nudge story made true, first-run un-broken)

Overnight implementation of the ultra launch audit's confirmed findings. Every change below is
build-verified on both variants and the web UI was exercised against a mock device in a browser;
none of it has been observed on hardware yet — flash and check before shipping it to a stranger.

### Added
- **Nudge escalation.** The second and later unacknowledged reminders play a second swell ~6s
  after the first, so a nudge that went unnoticed asks twice. RAM-only counter; nothing persists.
- **Button acknowledgment.** Pressing either clock button during a nudge (or within 15s after)
  now reads as "seen it": it clears the escalation counter and restarts the interval. It no
  longer adjusts the time — the old behavior corrupted the clock as the only physical response
  to a reminder. Time adjustment works exactly as before outside the ack window.
- **First-run time zone banner.** When no zone has ever been saved (new `tzConfigured` flag in
  `/settings` JSON, backed by NVS), the web UI shows a banner with the browser's detected zone
  and a one-tap apply. Dismissable; never returns once a zone is saved.
- **Quick nudge control.** On/off + interval + save, in Quick controls, two taps from landing.
  Preset buttons (Pomodoro 25m / Check-in 30m / Hourly) in the reminder section.
- **Time-truth cue.** Until NTP lands or the user sets the time by hand, a short amber inner-ring
  chase fires every 10s — "plausible but unconfirmed" is now visibly different from "set".
  Respects the status-blips toggle. New `STATUS_TIME_UNSET` status color.
- **Offline DST tick.** Local time is re-derived from the epoch once a minute while WiFi is down,
  so a DST flip lands on time offline. Suppressed after a manual time set — offline, the user's
  hands are the better truth.

### Changed
- **Factory reminder days: none → all seven.** daysMask 0 meant enable+save silently never fired —
  the audit's top story-killer. Reminders still ship disabled; nothing fires unasked. The UI also
  guards the same trap: enabling with zero days checked auto-selects all seven and says so.
- **Start hour == end hour now means a 24-hour window** (was: silently never fires).
- **Save saves everything.** The main Save button now collects reminder and animation-style
  fields too; it used to silently revert unsaved edits in those sections on completion. The
  sectional save buttons remain.
- **Save/error feedback.** Every save path now shows a toast ("Saved" / "Could not reach the
  clock"); fetch failures no longer fail silently.
- **Portal timeouts.** Fresh-flash first boot: 10-minute provisioning window (was 2, shorter than
  a stranger's join-the-AP fumble). Factory reset: 15 minutes (was forever — a reset clock hung
  dead until provisioned; it now falls through to AP mode and an offline clock).
- **Mobile usability.** 16px inputs (stops iOS focus auto-zoom), 42px touch targets, savebar wraps.
- **Labels de-jargoned.** "Outer sec/min" → "Second/Minute hand", "Status" → "Wi-Fi status blips",
  "Hourly bloom" → "Hour-top center bloom", GW/RSSI → gateway/signal. Page and title now say
  ChronoBloom, not "ESP32 Ring Clock". POSIX TZ field hidden unless "Other" is picked.
- **Reminder preview fix.** Previewing "Use quarter/half/hour animation" (modes 0-2) works; the
  0-is-Off guard now applies only to the interval selects.

### Hardened (same session, after a 7-agent adversarial review of the diff)
- **Ack window redesigned.** The first cut re-armed the window on every acknowledgment, so
  holding a button to adjust time got eaten as endless acks. The window now opens only at fire
  time and closes on the first ack; the next press adjusts time normally. Signed-delta compare
  makes a stale window harmless across millis() wrap.
- **Escalation moved inside the day/hour window** (an edge-straddling repeat is dropped, never
  played into quiet hours) and all reminder RAM state clears on disable, so nothing can strand
  across a disable/re-enable.
- **Portal classification fixed:** captive-portal/Improv-provisioned units (credentials only in
  the WiFi-stack NVS) were misread as never-provisioned and would have sat 10 minutes in the
  portal on every boot-before-router power blip; `getWiFiIsSaved()` now counts.
- **Save refuses to run from a page that never finished loading** (would have posted browser
  defaults over the deployed config). The daysMask auto-repair on the main Save is now visible
  (checks the boxes, toasts) instead of silent. Quick nudge confirmation names the active hour
  window, so an evening enable outside it doesn't read as broken. Timezone banner only
  dismisses when the save landed; error toasts distinguish "clock rejected it" (shows the
  reason) from "unreachable". Brisbane zone option added; delegated-chime previews explain
  themselves when the chime is Off; checkboxes exempted from the 42px mobile rule.
- Web NTP "Sync to internet" now lifts the manual-time override inside syncNow() itself, so
  the offline DST tick resumes after any accepted network sync.

### Verified
- Both variants compile clean (8" 61.1% flash). Web UI reconstructed from PROGMEM: tags balance,
  JS parses under node, and every new behavior (banner apply, quick save, 0-days guard, unified
  Save payload, presets, toasts, the loaded-page save gate, window text, chime-off hint)
  exercised against a mock device server in a real browser.
- NOT yet verified on hardware: escalation timing, ack window feel, portal timeout fallthrough,
  offline DST tick, amber time-unset cue.

## [2.30.1] - 2026-07-27 (On-wall verdict fixes: ChronoBloom hand contrast + Sunflower export honored)

The operator flashed 2.30.0 to both units and judged it on the wall — the real gate. Two failures,
both fixed the same evening:

### Changed
- **ChronoBloom inner hour hand: magenta → periwinkle `#6eb9ff`.** Both hour-hand segments had been
  the identical magenta since v2.4.8 ("unified for visual continuity"), so the middle/inner pair had
  ZERO hue contrast — and the 2.22.1 gamma pipeline shifted that magenta's emit toward hot pink,
  which is why the operator remembers the hands carrying "a tone of blue" that is now gone. The
  periwinkle echoes the outer marks (theme identity), reads azure `(40,126,255)` against the warm
  inner face (164 deg separation, 8.9x luminance pop), and sits 102 deg from the magenta middle
  hand. Emit-verified through the exact pipeline. `defaults()` updated to match (defaults ARE the
  ChronoBloom theme).
- **ChronoBloom face scales 55/55 → 39/39** — the same D/255 lift rebalance already applied to
  Ember/Lotus, restoring the lifted crest to the canonical 55-equivalent luminance so the middle
  hand keeps its authored 4.7x pop instead of the 3.3x the 2.30.0 lift left it.
- **Sunflower petalDepth 40 → 0.** The theme chip was forcing petal texture onto the operator's
  hardware-tuned export (whose exported petalMode was OFF): depth 40's lift pushed the huge face
  scales (181/244) to near-max and drowned the dark-red silhouette hands — observed live on the 15"
  wall clock as "contrast between hour hands is poorer." The export is now honored verbatim; the
  depth slider remains for anyone who wants texture on it.

### Deployed
- OTA to both units the same evening (15" wall clock first — it is the display the verdict came
  from — then the 8"), plus a one-time `petalDepth=0` settings restore on the 15" to undo the value
  the 2.30.0 chip-apply had already persisted into its EEPROM.

## [2.30.0] - 2026-07-27 (Petal depth that actually reads + two honest brightness sliders + capture pre-roll)

What the 8" demo-reel shoot needed. "Scalloped" is gone as a term: it reads as a cooking technique,
not an amount of shading. The feature is **Petal depth** everywhere now -- firmware field, web UI
label, theme key, narration. And the three-slider auto-brightness stack ("Darkest / Brightest /
Overall dimness") collapses to the two sliders that mean something.

### Changed
- **Petal shading applies to all three rings, from one shared profile.** Before, the outer 60 had
  none at all and the two inner rings used different profiles -- middle 24 a `[70%,100%,70%]` triangle,
  inner 12 a `[100%,60%]` every-other-LED alternation that reads as dither rather than petals. That is
  the "not all rings convey it equally" complaint, and it was literally true. Now every ring runs the
  same crest-to-seam profile off `PETAL_SHADE_*` + `petalFactor()`, so at any depth all three sit at the
  identical seam/crest ratio (verified by simulation at depths 0/30/45/60/70/100).
  - outer 60: 12 petals x 5 LEDs, crest **on** the 5-minute marker. **Markers are never shaded**, so the
    hour marks stay crisp and gain contrast against the shaded filler either side of them.
  - middle 24: 8 petals x 3 LEDs, crest at `i%3==1` (the v17 phase, unchanged).
  - inner 12: **4 petals x 3 LEDs** (was 6 x 2). Same cell as the middle ring -- identical profile is what
    makes them read equally -- and phased so its crests land on the same radial spokes as the middle
    ring's (middle `2i+1` faces inner `i`). Result: four strong spokes at 12/3/6/9 with finer petals
    filling in between.
- **`petalMode` (0/1) → `petalDepth` (0-100), SETTINGS_VERSION 18→19.** Same byte, same offset, no
  struct growth. Depth 0 makes `petalPixel()` collapse to the plain flat fill, so the old solid-fill
  branch is gone entirely -- one render path instead of two to keep in sync. `PETAL_SHADE_MAX = 170`
  caps a full-depth seam cut: deep, never black.
- **The contrast math got a real fix, not just a knob** (`petalPixel()`, replacing the first-pass
  post-gamma multiply). Measured on ChronoBloom's middle face at day brightness 44, the v17-style cut
  produced a crest of (6,0,4) against a seam of (4,0,2) -- quantization noise, not a petal, which is
  why the default theme "really has no contrasting petals." Two changes, both simulated end-to-end
  through the exact emit pipeline before landing:
  - **The seam shades the COLOR before gamma** -- a linear-light cut reads at roughly half its size
    perceptually; pre-gamma it reads at face value. The level multiply itself stays post-gamma, so
    this does NOT reintroduce the v2.22.0 pre-gamma-level crush (only the deliberate petal shading
    term moves).
  - **The crest is lifted by 255/D** (D = deepest seam factor), so depth adds contrast instead of only
    ever dimming the face: the crest gains what the seam loses.
  - Net effect on ChronoBloom's middle face, perceptual (sRGB) channel delta crest-to-seam: 11→18 at
    global brightness 44, 12→29 at 128, 17→39 at 255. Roughly 2.3x, and it scales with depth.

- **Auto-brightness is two sliders now: Darkest and Brightest.** The third slider ("Overall dimness",
  the v18 gain) is retired -- its label read backwards (lower = dimmer), and mathematically it
  multiplied the lux curve BEFORE the min/max clamp, so it could shove the whole response under the
  Darkest floor and pin the face there with the sensor effectively ignored. The new response
  (`autoSpanMap()`, one shared definition used by the renderer, `/diag`, and the serial status log)
  stretches the sensor curve's native 15..255 output across exactly
  [Darkest..Brightest]: pitch black lands ON the Darkest slider, full daylight ON the Brightest
  slider, and neither slider can ever be silently defeated. "Dim the whole clock" is now simply
  "pull Brightest down."
  - **Migration folds the stored gain into Brightest** so no deployed unit's daylight peak jumps:
    old peak = clamp(255 x gain%, min, max) becomes the new max exactly. Simulated against every
    real config (factory 60%, operator 51%, edge 0%/100%) across the full curve: endpoints exact,
    worst mid-curve deviation 5 counts of 255. Factory default Brightest becomes 153 -- the same
    daylight peak the old defaults (max 255 x gain 60%) actually produced.
  - The gain byte stays in the struct as `reservedGain` (zeroed) so the EEPROM layout is untouched;
    `autoBrightnessGain` is gone from the web UI, `POST /settings`, `GET /settings`, and `/diag`.

### Added
- **"Petal depth" slider (0-100) in Ring colors**, replacing the "Petal texture" checkbox, and moved
  below the last ring row since it is no longer a rings-1-&-2 setting. Live WYSIWYG preview mirrors
  `petalPixel()` -- pre-gamma tint, truncating lift, same integer math -- and was verified to emit
  **byte-identical hex** to a Python simulation of the firmware pipeline at depths 0/45/100 on all
  three rings, crest and seam and the outer ring's edge shade alike.
- **Per-theme petal depth**, replacing the on/off flag: ChronoBloom 45, Moonflower 60, Cherry Blossom
  60, Ember Dahlia 70, Lotus Pond 65, Sunflower 40, Bird of Paradise 50. Chosen against each theme's
  own fill brightness -- bright/pale fills carry more shading before the seam muddies; dark fills need
  less. ChronoBloom and Sunflower were previously OFF and now carry a deliberately gentle amount.
- **Full-roster beauty audit through the emit pipeline** (all 7 themes x 12 emitted swatches x 2
  brightness regimes, scored against the project's own design rules: hand/face separation, 45-60
  face band, saturation floor, center-warmest, harmony template, channel-quantization health).
  Findings: the roster is sound under its own templates -- the analogous themes' sub-60-degree
  hand/face hue seps are their declared designs carried by luminance hierarchy, Lotus's soft aqua
  marks are the 2.24.0 author's deliberate choice, and Sunflower's inverted hierarchy is an
  operator hardware-tuned export (untouched on principle). One real defect found and fixed: **the
  B+C crest lift broke the "dim petal wash, bright stamen" hierarchy on three rings** -- Ember
  Dahlia's inner hand fell to 0.6x its lifted face crest (the red hand drowned in the gold corona),
  Ember middle to 1.7x, Lotus middle to 1.2x. Fixed by rebalancing those face scales so the LIFTED
  crest lands exactly at the 2.24.0-authored luminance (`scale_new = scale_old x D/255`): Ember
  middleFaceScale 55→29, innerFaceScale 60→32; Lotus middleFaceScale 50→28. Verified restored to
  the authored ratios to one decimal (3.2/1.2 vs 3.1/1.2; 2.2 vs 2.1). The themes.json design-rule
  comment now states the 45-60 face band applies to the lifted crest (`scale x 255/D`), so a stored
  sub-45 scale under high depth is legitimate. Known and accepted: at day brightness 44, seam
  bottoms quantize small channels to 1-2 counts (hue purifies toward the dominant channel) -- an
  inherent 8-bit floor, invisible on the diffuser at those counts and gone at daylight peak.
  Proof sheet with every emitted swatch delivered to the operator; on-wall eye check remains the
  final gate per FLORAL_COLOR_DESIGN's own guardrail.
- **Demo-reel capture pre-roll.** `POST /demo/start?delay=<seconds>` (0-60) holds the clock **fully
  dark** before the reel opens, so a camera can be rolling and settled and frame one is a fade-up out
  of clean black. Web UI: a "Pre-roll" seconds box next to the Demo button (default 5, remembered in
  localStorage), with a live countdown in the status line. `/demo/status` gains `preroll` +
  `preroll_ms`; `/demo/overlay` treats pre-roll as "not running" and stays blank, so no subtitle is
  ever composited onto the black lead-in. Handled on-device rather than by a browser timer so the
  countdown survives the tab being closed or slept.

### Fixed
- **`tools/themes/themes.json` had silently drifted from what ships.** It declares itself the single
  source of truth and `tools/gen_themes.py` stamps it into both `src/web_html.h` and
  `docs/publish/demo_reel_designer.html` -- but it still held the **pre-2.24.0** colors for Moonflower,
  Cherry Blossom, Ember Dahlia and Lotus Pond, and never carried `petalMode` at all. Running the
  generator would have silently reverted the shipped botanical palettes (e.g. cherry ring-3 back from
  burgundy `#662d3f` to `#64143c`). Rebuilt from the shipped values, verified field-by-field that no
  colour or level changed.
- **Sunflower and Bird of Paradise are real theme entries again.** They were hand-appended JS lines in
  `web_html.h`, so they existed in the clock's own UI but were **absent from the demo reel designer's
  roster entirely**. Both now live in `themes.json`; the two hand-appended lines are gone and all seven
  themes come from one generated line in each consumer.

### Migration
`SETTINGS_VERSION` 18→19 changes no field offsets and adds no bytes -- two bytes change MEANING.
The petal byte: a stored `1` read back as a v19 depth would mean "1% depth", i.e. silently off, so
`widenPetalFlag()` maps the old flag onto the default depth in the v17 and v18 branches; units at v15
and v16 never had the feature and are seeded flat (0). The gain byte: folded into `maxAutoBrightness`
in the v18 branch (see above) and zeroed; v15-v17 units never had a gain, so their max is already
their true peak and only the byte is zeroed. No settings wipe on any deployed unit.

### Files changed
- `src/main.cpp` -- `ClockSettings.petalMode` → `petalDepth`, `autoBrightnessGain` → `reservedGain`
  (retired); new `PETAL_SHADE_MAX` / `PETAL_DEPTH_DEFAULT` / `PETAL_SHADE_5` / `PETAL_SHADE_3` /
  `petalFactor()` / `petalPixel()` / `widenPetalFlag()` / `autoSpanMap()`; `SETTINGS_VERSION` 18→19;
  `SettingsStore::begin()` (v15/v16/v17 branches updated, new v18 branch with the gain fold),
  `defaults()` (Brightest 153), `sanitize()`; `ClockRenderer::renderFace()` (one precomputed-pixel
  shaded path across all three rings), `effectiveBrightness()` (span map); `/diag` and
  `logRuntimeStatus()` share `autoSpanMap()` and drop the gain field; `WebUi` settings POST parser +
  `settingsJson()`; `DemoMode::start()` / `stop()` / `loop()` / `rendersFaceNow()` / `statusJson()` +
  `preRollEndMs_`; `setupDemoModeRoutes()` (`?delay=`).
- `src/web_html.h` -- "Petal texture" checkbox → "Petal depth" slider (moved below the last ring row);
  `PETAL_S5` / `PETAL_S3` / `petalFactorJS()` / `petalTintJS()`; `draw()` B+C shading on all three
  rings; "Overall dimness" label + gain slider removed, auto helper text rewritten; `bindLive()`,
  `collectFaceParams()`, `saveBrightness()`, `loadSettings()`, `applyTheme()`; demo Pre-roll input,
  `startDemo()`, `updateDemoStatus()`; `OVERLAY_HTML` stays blank during pre-roll; regenerated
  `THEMES` line.
- `tools/themes/themes.json` -- rebuilt from shipped values, `petalMode` → per-theme `petalDepth`,
  sunflower + birdofparadise folded in, `_comment` palette names refreshed to the v2.27.0 set.
- `docs/publish/demo_reel_designer.html` -- regenerated `THEMES` line (now carries all seven themes).
- `platformio.ini` -- `FIRMWARE_VERSION` 2.29.0 → 2.30.0.
- `scripts/webui_mock_server.py`, `scripts/walkthrough/local_serve.py` -- mock `/settings` carries `petalDepth`.
- `scripts/walkthrough/callouts.json`, `narration.json`, `SCRIPT_DRAFT.md` -- `#petalMode` selector and
  "scalloped" wording replaced (the stale selector would have made the walkthrough miss the control).
- `docs/publish/DEMO_MODE.md` -- `?delay=` pre-roll, `preroll` / `preroll_ms` status fields.
- `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md` -- regenerated.

### Verified / NOT verified
- **Verified:** clean warning-free builds, BOTH envs (`esp32c3_v3_8inch` + `esp32c3_v3_15inch`,
  61.1% flash). Petal math simulated end-to-end through the exact emit pipeline (truncating integer
  math, gamma LUT, sRGB re-encode): profile equality across rings, marker exemption, spoke alignment,
  depth-0 collapse to flat fill, and the perceptual-contrast numbers quoted above. Web UI driven live
  in a browser against `scripts/webui_mock_server.py`: zero console errors; preview emits
  **byte-identical hex** to the firmware simulation at depths 0/45/100 (mid/inner crest+seam, outer
  seam and edge shade); gain slider and label gone; `collectFaceParams()` / `saveBrightness()` /
  `loadSettings()` all exercised without throwing after the element removal (the first patch left a
  `qs('autoBrightnessGain').value` reference in `collectFaceParams` that WOULD have broken face saves
  -- caught by running the page, not by reading it). Gain-fold migration simulated across the full
  curve for factory 60% / operator 51% / 0% / 100%: endpoints exact, worst deviation 5/255. Pre-roll
  countdown + status lines render; `?delay=` clamps 0-60 both sides. Theme round-trip vs
  `git show HEAD`: no pre-existing value changed. `tools/theme_emit_check.py` PASS.
- **NOT verified on hardware.** Nothing in this release has been flashed or seen on LEDs. The
  per-theme depth values and the B+C look are authored judgements; the operator's eye on the 8" is
  the real check, and the sliders exist precisely so both can be retuned live. The auto-brightness
  span map has not been watched against a real lux sweep.

## [Docs] - 2026-07-17 (Launch item 6: README becomes a replication document. No firmware change; FIRMWARE_VERSION stays 2.29.0)

### Changed
- **README** is now a build-it-from-scratch guide, not a feature list. New "Build it, step by step"
  with six numbered steps, each carrying an observable verify gate, keyed to a new "If it does not
  work" troubleshooting table. BOM filled from `docs/publish/PROJECT_FACTS.md` §5: real links for the
  WESIRI 241-LED ring kit (~$26) and the VEML7700; four rows keep marked placeholder links; added the
  missing M3 heat-set inserts + bolts row. Diffuser set to 0.6mm printed PLA throughout (operator's
  2026-07-17 decision), replacing parchment/0.3mm language.
- **`docs/publish/PROJECT_FACTS.md`** §6 diffuser corrected 0.3mm → 0.6mm to match.
- **build_help issue template** now asks which numbered build step failed.

### Added
- **`docs/PRINTING.md`**: 8" print settings (monotonic top/bottom fill, full-coverage shells, speed
  caps) and the 0.6mm diffuser spec; ring outer diameters flagged as caliper-pending.
- **`.github/FUNDING.yml`**: Buy Me a Coffee sponsor-button scaffold, line commented until the owner
  supplies a username (a placeholder would render a 404 button). README Support carries a matching
  commented line. Not on the flasher page (someone mid-flash should not get a donation ask, and
  GitHub Pages prohibits payment processing anyway).
- Both new shippable files added to the export manifest; gate PASSES at 49 files.

### Outstanding operator data (item 6 is structurally complete; these are fill-ins)
- Four BOM purchase links (XIAO, buttons, PSU, resistor), the Buy Me a Coffee username, a wiring
  photo, and caliper-verified ring dimensions. The Amazon Associates ID `maestro8484-20` exists in
  PROJECT_FACTS; whether to append affiliate tags (with the required disclosure) is an owner decision,
  left untouched.

## [2.29.0] - 2026-07-17 (Improv Wi-Fi: configure Wi-Fi from the browser at flash time)

Pairs with the new browser flasher (`docs/flasher/`): ESP Web Tools can now also hand the freshly
flashed device its Wi-Fi credentials over the same USB-serial connection, no captive-portal detour
required for the common case.

### Added
- **Improv Wi-Fi serial support.** On a device with no saved Wi-Fi yet (fresh flash or post
  factory-reset), the clock listens for the [Improv](https://www.improv-wifi.com/) protocol over
  USB-serial for a bounded 10-second window before falling back to the existing WiFiManager
  captive portal. A browser-side Improv client (ESP Web Tools' own "Configure Wi-Fi" dialog) can
  supply SSID/password directly; the clock connects, saves the credentials to the same `wifi` NVS
  namespace the `/wifi` web page already uses, and reports success with the device's
  `http://<hostname>.local/` URL.
  - Protocol parsing/framing: official `improv-wifi/sdk-cpp`, pinned as `improv/Improv @ 1.2.6` in
    `platformio.ini`. That SDK ships packet parsing + frame building only (no Arduino serial glue
    is published upstream); the byte-framing in `ImprovSerialHandler::sendFrame()` (`src/main.cpp`)
    is ported from ESPHome's `improv_serial` component, the reference production implementation
    built on the same SDK.
  - Serviced non-blocking and byte-at-a-time inside `loop()`, only while the device is
    unprovisioned or in AP fallback — once connected, USB-serial can no longer silently
    reconfigure a running clock's Wi-Fi. The pre-flash window itself sits before `setupWiFi()` in
    `WebUi::begin()` and only runs when no credentials are already saved, so a normally configured
    clock's boot time is unaffected.
  - WiFiManager's captive portal (`esp32c3-clock-setup`, 192.168.4.1) is unchanged and remains the
    fallback for clients without Improv support.

### Unchanged
- `SETTINGS_VERSION` stays 18 — `ClockSettings` was not touched. Wi-Fi credentials, like the
  timezone string (2.28.0), live in NVS (Preferences), not in the fixed-size settings blob.

### Deferred (not verified this session)
- Button-hold factory-reset re-test against the new pre-window (should be unaffected — it forces
  the portal directly and never enters the Improv gate — but not exercised on hardware).
- End-to-end browser Improv dialog test against a bare device (no serial monitor "type the SSID by
  hand" substitute exists; needs ESP Web Tools' actual dialog).

## [2.28.0] - 2026-07-17 (Runtime timezone: the clock is no longer locked to the maintainer's zone)

The last setting that required a compiler. Until now `NTP_TIMEZONE_TZ` was a build flag, so every
binary (and every future web-flashed image) displayed US Mountain time and a builder in another zone
had to install a toolchain and recompile to fix it. That blocked the browser-flasher path from being
worth shipping. Now it is a runtime setting.

### Added
- **Time zone picker** in the web UI (Time & light → Time zone): 18 common zones plus an "Other"
  escape hatch that accepts any POSIX TZ string. Every shipped option is verified to pass the
  firmware's own validator, so the UI cannot offer a zone the device would reject.
- `tzone` namespace in `src/main.cpp`: `valid()` / `get()` / `apply()` / `save()` / `clear()`.
- `TimeSync::setTimezone()`. Changing zone re-derives local time from the UTC epoch already on the
  chip, so it takes effect **immediately, with no reboot and no NTP round trip** (the ESP keeps the
  system clock in UTC; only `tzset()` has to re-run).
- `timezone` exposed in `GET /settings` and `GET /diag`; accepted by `POST /settings`.

### Changed
- `NTP_TIMEZONE_TZ` in `platformio.ini` is now the compile-time **default** only: what a fresh or
  factory-reset unit starts on. Behavior for anyone who never touches the setting is unchanged.
- Factory reset clears the `clock` NVS namespace, so a reset unit returns to the build-time default,
  matching what the `wifi` namespace already does.

### Design note
The TZ string lives in **NVS (Preferences), not `ClockSettings`** — hence **`SETTINGS_VERSION` stays
18 and no unit migrates or loses settings**. Prior art decided this: the firmware already keeps
user-set strings in NVS (the WiFi credentials, `wifi` namespace), while `ClockSettings` is a fixed
84-byte POD blob behind a hard 128-byte EEPROM ceiling. A ~40-byte string would have consumed most of
the remaining 44 bytes of headroom and forced a migration on every deployed unit, buying nothing.

### Validation
`tzone::valid()` is a sanity check, not a POSIX parser: an invalid TZ string does not fail loudly in
the C library, it silently resolves to UTC, reaching the user as "the clock is just wrong by N hours"
with no diagnostic. It requires a plausible charset, a zone name, and at least one digit (every real
TZ string carries an offset). That rejects the two realistic failures: `GARBAGE`-class typos, and
IANA names like `America/Denver`, which users will try and which would otherwise silently mean UTC.
It also rejects quotes and backslashes, which keeps the unescaped `"timezone":"%s"` in
`settingsJson()` from being able to emit invalid JSON. `POST /settings` answers **400** on a rejected
string rather than storing it.

### Files changed
- `src/main.cpp` (tzone namespace, TimeSync::setTimezone, /settings POST, settingsJson, /diag, factory reset)
- `src/web_html.h` (Time zone row, `tzPicked`/`tzSync`/`saveTz`, loadSettings)
- `platformio.ini` (FIRMWARE_VERSION 2.27.3 → 2.28.0; NTP_TIMEZONE_TZ documented as a default)
- `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md` (regenerated)

### Verified / NOT verified
Both variants build clean (8" and 15"). Validator logic checked against 14 real POSIX TZ strings
(including the `<+0330>` Iran and Lord Howe quoted forms) and 9 hostile inputs: all pass/reject as
intended. Web UI reconstructed from the PROGMEM chunks: tags balance (94/94 div, 14/14 select), the
30 KB of index JS parses under node, and all 18 zone options pass `tzone::valid()`.
**NOT verified on hardware** — no flash performed. The operator flashes OTA; confirm via `/diag`
that `timezone` reports the set zone and that the displayed time moves immediately without a reboot.

## [Docs] - 2026-07-17 (Launch audit session 2 part 1: export gate unjammed, docs fact-refreshed, two new ship gates. No firmware behavior change; FIRMWARE_VERSION stays 2.27.3)

### Fixed
- **Export gate was failing, blocking any re-publish.** Reworded the LAN IP out of the `LED_GAMMA`
  comment (`src/main.cpp`) and the v2.22.x gamma note (this file). Gate now passes at 44 files.
- **Shipped config carried maintainer machine defaults.** `upload_port = COM6/COM9` and the
  `upload_protocol` duplicates removed from both envs in `platformio.ini`; both shipped
  `scripts/upload_*.bat` echoed the same ports in their USB-fallback hint. Ports moved to the
  gitignored `platformio.local.ini` and documented in `platformio.local.ini.example`. PlatformIO
  auto-detects when `upload_port` is absent. Verified: maintainer ports still resolve, build clean.
- **Shipped docs contradicted shipped firmware.** `docs/ANIMATIONS.md`: palette table still listed
  the flower palettes removed in v2.27.0; reminder heading said modes 6-10 when Firefly is 11;
  Firefly had no entry despite being in the UI. `docs/FEATURES.md`: demo sequence claimed 8 steps /
  96 s / "8 color palettes" when `DemoMode::steps[]` is 9 steps / ~131 s with no palette step;
  theme list said "Classic, Aqua, Magenta" when the real list is 7 presets; reminder modes corrected.
- **Four dead links** in shipped docs pointed at `REVIEW.md` / `WORKFLOW.md`, which never ship.

### Added
- **Untracked-file gate** in `tools/publish/export_public.py`. The include globs walk the filesystem,
  not git, so any file dropped into a globbed directory shipped silently (this is how two scratch
  test-print 3MFs and one unreviewed STL entered the set). Checks source paths, not destination keys,
  which differ under the manifest's `relocate` map.
- **Relative-link gate**: every markdown link in a shipped doc must resolve inside the export set.
- Both gates were verified by injecting the exact failure they exist to catch.

### Changed
- Untracked 3D scratch files moved out of the ship path to `docs/3d_testprints/` (preserved, not
  deleted). One of them, `chronobloom-8inch-base-cutout (1).stl`, is NOT a duplicate of the tracked
  cutout and needs an operator decision on whether it is a real revision.
- `docs/LAUNCH_GAPS.md`: G2 (Windows clone failure) **refuted**. Session 1's clone failed only
  because the scratchpad it cloned into was ~140 characters deep; a clone to a normal path succeeds
  with long-path support off. The audit had measured its own environment, not the repo.

## [Docs] - 2026-07-16 (Launch audit session 1: gap manifest + README pass. No firmware change; FIRMWARE_VERSION stays 2.27.3)

### Added
- `LAUNCH_AUDIT_TEMPLATE.md` (audit doctrine, razor table, session plan) and `docs/LAUNCH_GAPS.md`
  (classified gap manifest: hard blockers file:line, stranger-walk verdicts G1-G19, session-2 build
  list, session-3 clean-room test plan).

### Changed
- `README.md`: fact-fix ("8 color palettes" was stale since the v2.27.0 palette simplification;
  now describes per-ring controls + theme presets + mood palettes) plus a dash/style pass with
  computed verification (digit/URL/link/code multisets identical, fenced blocks byte-identical).

### Found, not fixed (session 2 executes; see docs/LAUNCH_GAPS.md)
- Export gate currently FAILS on a LAN IP in `src/main.cpp:221` + `docs/CHANGELOG.md:277`
  (this file, the v2.22.x gamma note); public repo is 19 minor versions stale at v2.8.0.

## [2.27.3] - 2026-07-14 (Motion & nudges consolidation — de-dup preview/save, co-locate anim style)

Web-UI reorganization (web_html.h only; no firmware-logic or settings change). Addresses the R2-R4/R6
findings in `docs/UI_REDUNDANCY_2026-07-14.md`.

### Changed
- **All animation-style controls now live together** under Motion & nudges → "Animation style":
  Animation colors (chimes) · Speed · Brightness · Trail length · Reminder colors (nudges), with one
  Preview and one Save. Previously colors+speed were in Quick controls and brightness+trail+reminder-colors
  were in Motion — you had to touch two cards to tune one thing (R4).
- **Quick controls** slims to what you actually touch often: Theme chips + Brightness (with its own
  **Save brightness** button). Theme chips already set the whole animation style in one click.
- Second/minute-effects **"Trail length" → "Second-hand trail length"** to disambiguate it from the
  animation trail length (R6).

### Removed
- **Redundant "Preview slot" selector + its Preview button** (R2). The per-row ▶ buttons next to each
  Quarter/Half-hour/Hour/Reminder animation already preview that slot with the current unsaved style, so
  the slot-picker was a third, weaker way to do the same thing. The single "Preview" in Animation style
  previews the configured interval chime (hour → half-hour → quarter) in the selected colors.
- **Redundant second Save button** (R3). Quick "Save" and Motion "Save style" both persisted the same five
  animation fields. Now: "Save style" (Motion) saves the animation style; "Save brightness" (Quick) saves
  the brightness settings — one non-overlapping save per concern.

## [2.27.2] - 2026-07-14 (Firefly nudge — fix double-dimming, denser swarm)

### Fixed
- **Firefly reminder was ~`animBr`× too dim (root cause).** `animRem6` multiplied brightness by
  `animBr()` explicitly *after* `bandColor()` had already rendered at `animBr()` — a double application of
  animation-brightness that no other nudge does. At the default brightness that left Firefly rendering at
  ~0.6² instead of ~0.6 of a ring color, i.e. roughly `animBr×` weaker than its siblings. Removed the
  redundant factor so Firefly's peak matches the other nudges (still capped at `NUDGE_CEIL`).
- **Saved "Firefly" reminder reset to "quarter animation" on every reboot.** `sanitize()` clamped
  `focusReminder_animation > 10` to 0, but Firefly is value **11** (and POST already accepts 0-11), so a
  stored Firefly was wiped on the next boot. Changed the bound to `> 11`. (Firefly is the only option that
  was affected; the "(alt)" 3-5 removed in 2.27.1 were never > 10.)

### Changed
- **Denser Firefly swarm** so it reads as a clear cue: 9 → 14 twinkle points, hit rate ~43% → ~55%,
  period 1.6s → 1.4s; outer points placed on filler (non-marker) LEDs so a firefly isn't hidden under a
  bright hour marker. Warmer breathing core (amplitude 40±40 → 60±50).
- Still bounded — peak capped at `NUDGE_CEIL` (unchanged) and it repaints the idle face underneath, so it
  stays a gentle overlay, not an alert. **Perceptual result needs confirming on the 15" — tune the point
  count / hit rate / `NUDGE_CEIL` if it now overshoots.**

## [2.27.1] - 2026-07-14 (Web UI: smart Preview, scope labels, drop dead reminder options)

Low-risk web-UI clarity pass (web_html.h only; no firmware-logic or settings change).

### Changed
- **Quick-controls Preview is now honest.** It previewed a *hardcoded* "Galaxy spin" (hour mode 2)
  regardless of configuration. It now previews your **configured interval chime** — Hour, else Half-hour,
  else Quarter (first not "Off") — in the selected animation colors. Falls back to Galaxy spin only if all
  three interval slots are Off.
- **Scope labels** on the two color pickers so their purpose is explicit: `Animation colors (interval
  chimes)` (Quick controls) and `Reminder colors (nudges)` (Motion & nudges), each with a tooltip pointing
  at the other. They share one palette list but are independent selections (per v2.27.0).

### Removed
- **Dead "(alt)" reminder-animation options.** The `Reminder animation` dropdown listed *Use quarter/
  half-hour/hour animation (alt)* (modes 3-5), which call the identical firmware path as modes 0-2 (see
  `triggerReminderDirectAnimation`, main.cpp) — no behavioral difference. Dropped the three duplicates
  (12→9 options). Safe/non-destructive: a unit with a stored value of 3-5 is normalized on load to its
  identical 0-2 option (no empty dropdown), and firmware still renders 3-5 the same as 0-2.

## [2.27.0] - 2026-07-14 (Animation-palette simplification — one shared mood list)

Occam pass on the animation colors (operator directive: *"too many options is the same as poor
options"*). Theme-specific animation palettes made no sense — you already picked a theme; the
animation should follow it. The two separate palette systems collapse onto **one shared 5-option
list** that applies to both interval chimes and reminders.

### Changed
- **One shared palette list** replaces the 10 flower palettes + 4 separate reminder palettes:
  `Clock colors (default)` + 4 temperature/mood palettes — **Golden hour (warm)**, **Moonlight (cool)**,
  **Dawn (soft-warm)**, **Twilight (muted-cool)**. Both the **Animation colors** dropdown (chimes) and the
  **Reminder colors** dropdown (nudges) now offer the *same* five options, so a nudge can still be set to
  read differently from a chime (design option b).
- **Reminders can now use Clock colors** (index 7) — previously reminders were locked to the mood set;
  the `pal==7` face-mapping now resolves in the reminder render path too.
- **Renamed** the cool mood `Moonflower`→`Moonlight` (it collided with the *Moonflower* clock theme name,
  a real source of confusion); re-authored `blush`→`Dawn (soft-warm)` and `calm`→`Twilight (muted-cool)`
  with new values.
- **Gamma-aware palette values.** The mood palettes are now authored through the gamma pipeline
  (`gammaColor`, gamma 2.2) — seeds are inverse-gamma of the intended on-strip appearance, replacing the
  pre-gamma (v2.21.0-vintage) values that had been rendering too dark since gamma landed in v2.22.1. The
  generator/header no longer claim "no gamma". **Seeds need live confirmation on the 15" — they will
  render brighter than the current (gamma-darkened) moods.**
- **Palette handling unified.** New `sanitizePaletteValue()` is the single definition of a valid palette
  ({0-3 moods, 7 Clock colors}); used by `sanitize()`, POST `/settings`, and the preview path. Any
  unknown/removed/out-of-range value → 7 (Clock colors), uniformly (fixes the stale-client-POST and
  theme-import edge cases where an out-of-range index could land on the wrong palette or an empty dropdown).
- **`tools/palettes/palettes.json`** → single `mood_palettes` array; **`tools/gen_palettes.py`** emits one
  `MOOD_PALETTE_RINGS` + `MOOD_PALETTE_COUNT`. `main.cpp` renderer resolves both palettes through a shared
  `curPalette()`/`anchorColor()`.

### Removed
- The 10 flower animation palettes (`Ember, Coral, Lagoon, Dusk, Nocturne, Tropic, Prism, Thunderstorm,
  Sunny, Blue Paradise`) and the 4 old reminder palette definitions. The `Sunflower` and
  `Bird of Paradise` clock themes now point their `animationPalette` at Clock colors (7).

### Notes
- **No `SETTINGS_VERSION` change** (stays 18): the `ClockSettings` struct layout is unchanged —
  `animationPalette`/`reminderPalette` are the same bytes, only their value *meaning* changed. Deployed
  units keep every setting; stored values self-correct via `sanitizePaletteValue()` (removed flower
  indices → Clock colors; old reminder 0-3 → same-index new mood).
- **Reversible:** all removed palette data (json, generated C, swatches, dropdown labels) archived verbatim
  in `docs/archive/flower_palettes_2026-07-14.md` with a re-import checklist.

## [2.26.1] - 2026-07-14 (Remove "Go Pack" theme + "Green & Gold" animation palette)

Removed the NFL "Go Pack" clock theme and its paired "Green & Gold" animation palette (index 11) for
aesthetic consistency with the clock's botanical identity. Non-destructive and reversible.

### Removed
- **`THEMES.gopack`** preset (web UI theme chips are generated from `THEMES`, so the chip disappears
  automatically) and its `animationPalette` dropdown option + swatch.
- **"Green & Gold" animation palette** (idx 11) from `tools/palettes/palettes.json`; regenerated
  `src/anim_palettes.h` (`ANIM_PALETTE_COUNT` 12→11).

### Notes
- **Non-destructive to deployed units:** no `SETTINGS_VERSION` change (palette count is compile-time, not
  struct layout). A unit currently set to `animationPalette 11` falls back to 7 (Clock colors) via
  `sanitize()`; stored face colors are untouched.
- **Fully reversible:** every value (theme preset, palette RGB, swatch, dropdown) is archived verbatim in
  `docs/archive/gopack_theme_2026-07-14.md` with a re-import checklist.
- Also refreshed two stale `animationPalette` enumeration comments in `main.cpp` to match the real palette
  names (they still listed long-renamed palettes like "Snowstorm"/"Wildflower").

## [2.26.0] - 2026-07-14 (Auto-brightness gain — dimmer & hue-safe, lux keeps directing brightness)

Acts on the 2.25.1 diagnosis. A live lux sweep on the 15" confirmed the mechanism was **Hypothesis C**
(the `maxAutoBrightness` clamp pinning the top of the response flat — `effective` stuck at 108 from
300→10000 lux), **not** the current limiter (est. draw peaked ~261 mA vs the 1800 budget — B ruled out).
The operator's unit had `maxAutoBrightness=108`, i.e. it was already using the clamp as a dimmer, which is
exactly what flattens the high end. This release replaces that with a proper downward scale.

### Added
- **`autoBrightnessGain` setting (SETTINGS_VERSION 17→18,** appended byte, in-place v17→v18 migration —
  deployed units keep every setting, no wipe; seeded to **60%**). A 0–100% multiplier applied to the
  lux-derived auto-brightness curve *before* the min/max clamp. Because it scales the whole curve down, it
  (a) dims the display overall and (b) pulls the top of the curve back under `maxAutoBrightness`, so lux
  keeps directing brightness across the full range instead of clamping flat. Multiplicative → the response
  *shape* (relative intensity between rings/elements) is preserved.
- **Web UI:** "Overall dimness" slider in Brightness → Auto, wired through preview/save/quick-save and
  reflected in `/diag` (`auto_brightness_gain`) and the serial runtime log.

> Note: as with every `SETTINGS_VERSION` bump, a **saved user-defaults** block from v17 is not carried
> across (the user-defaults slot is version-gated and has no migration). If you'd used "Save as my
> defaults", re-save it after flashing so `/settings/reset` restores your profile instead of factory.

### Changed
- **The global auto-brightness dim is now hue-safe.** The face `render()` path draws at full brightness,
  then dims via a new `scaleStripBufferVideo()` (WLED `color_fade`-style: any originally-nonzero channel
  stays ≥1) instead of baking the dim in through Adafruit `setBrightness()`, whose per-channel truncation
  drops small color channels to 0 first and walks saturated colors toward their primary as the clock dims —
  the reported "lux shifts hue, not brightness" artifact. `masterFade` keeps the exact scaler (a dissolve
  genuinely wants true black at f=0). Animation frames are unchanged (they manage their own envelopes).

## [2.25.1] - 2026-07-13 (Lux-override diagnostics — instrument the brightness/lux path)

Instrumentation-only release to disambiguate the operator's lux→brightness report ("lux changes
affect hue more than brightness; response varies by ring"). No change to the rendering or color
pipeline. Enables a full-range remote lux sweep + makes the current limiter observable.

### Fixed
- **`POST /demo/brightnessCycle` self-cancelled and drove no brightness change** (observed live on
  2.25.0). `DemoMode::loop()` computed `se = now - standaloneBrightnessStartMs_` with `now`
  snapshotted at loop-top, but `startBrightnessCycle()` sets the start stamp *later in the same
  iteration* (inside `webUi.loop()`'s POST handler), so `now < startMs` and the unsigned subtraction
  underflowed to ~4.29e9 — tripping the done-branch and disabling the cycle before it ever called
  `setLuxOverride()`. Now clamped to 0 on that first iteration. (Also un-breaks the reel designer's
  Live Run brightness segment, which shares this path.)

### Added
- **`GET /lux/override?lux=X` / `?clear=1`** — diagnostic route to pin the sensor to any lux value
  (full 0.1..10000 range; the built-in demo sweep only covers 1..220) so brightness response can be
  swept remotely and held steady for photos. Reuses the existing `setLuxOverride`/`clearLuxOverride`.
- **`/diag` limiter fields: `est_milliamps`, `limiter_brightness`, `max_milliamps`.** The current
  limiter (`ledShowBudgeted`) rescales brightness at `show()` time and was invisible to `/diag`;
  when `limiter_brightness` drops below `effective_brightness`, the limiter is actively fighting the
  lux-driven brightness (Hypothesis B). `est_milliamps` vs `max_milliamps` shows the headroom.

## [2.25.0] - 2026-07-13 (Petal texture — scalloped ring fills like real petal whorls)

New user-facing feature (ships in the same flash as the 2.24.0 color work below).

### Added
- **`petalMode` setting (SETTINGS_VERSION 16→17,** appended byte, v16→v17 in-place migration in
  `SettingsStore::begin()` — deployed units keep every setting, no wipe). 0 = solid fills (default).
- **Firmware:** `renderFace()` petal branch — middle 24 = 8 petals of 3 LEDs `[70%,100%,70%]`, inner 12 =
  6 petals of 2 LEDs `[100%,60%]`, phase-offset so the whorls interleave like real petal layers. Factors are
  brightness ops on the already-corrected ambient (single-gamma invariant preserved).
- **Web UI:** "Petal texture" checkbox (Ring colors), full wiring (`petalMode` POST arg, `/settings` JSON,
  live WYSIWYG preview renders the scallop). Themes: ON for Moonflower / Cherry Blossom / Ember Dahlia /
  Lotus Pond / Bird of Paradise; OFF for ChronoBloom and the operator-tuned Sunflower / Go Pack exports.
- Note: `Reset` with pre-v17 user-defaults falls back to factory defaults (stored block is version-checked).

## [2.24.0] - 2026-07-13 (WYSIWYG web preview + theme remap: operator exports + crushed-fill lifts)

Gamma-2.2-on-color/linear-brightness pipeline (2.22.1/2.23.0) verified byte-identical to WLED source
(`docs/WLED_PARITY_2026-07-13.md`) and untouched. This release fixes the two broken halves of the color
CONTRACT (`docs/RETRO_COLOR_2026-07-13.md`): stale theme data and a preview that lied.

### Changed
- **Web-UI clock preview is now WYSIWYG.** `setLed()` renders each LED through the exact firmware pipeline
  (`ledDisp()`: round-half-up gamma-2.2 → integer-truncated level scale → sRGB re-encode for the monitor);
  `draw()` folds `outerRingBrightness` into ring-3 marker/filler exactly as `renderFace()` does. The on-screen
  clock now shows what the LEDs emit — dark-crushed fills look dark on screen too.
- **Sunflower + Go Pack re-authored from the operator's hardware-tuned Export-Theme json (verbatim).**
  Sunflower is now a light-face/dark-hands design (bright gold fills, deep red hands, dim markers).
- **Moonflower / Cherry Blossom / Ember Dahlia / Lotus Pond / Bird of Paradise re-authored from RESEARCH-SOURCED
  botanical palettes** (color-hex.com, SchemeColor photo-extractions, etc. — cited in the session log). Each
  sourced display color was inverted through the exact firmware transform (gamma → level → ring-brightness) so
  the LED emits the intended hue. Notable: cherry ring-3 is now burgundy `#662d3f` (was bright red), Bird ring-2
  is a slate blue-gray bract (operator spec; no botanical hex exists) and ring-1 deep foliage green.
- **New ship-gate: `tools/theme_emit_check.py`** — byte-exact emit simulation of every THEMES field; fails any
  field whose dominant emitted channel is below 10 at day brightness. Current tree: PASS (all 8 themes).

**NOT yet hardware-verified** — operator flash + eye is the check. ChronoBloom + Ember/Cherry hands & several
markers are original (not yet reviewed on the corrected pipeline); Sunflower + Go Pack are operator exports.

## [2.23.0] - 2026-07-13 (Unified color pipeline: one correction chokepoint for every color — precision first)

Guiding philosophy going forward: **precision (consistency + reproducibility) before accuracy (closeness to
truth).** Every raw sRGB *design* color in the firmware now routes through ONE chokepoint — `gammaColor(r,g,b)` —
so correction is applied exactly once and uniformly across the entire display. Previously face/hands/markers/
animation palettes were corrected (via `ringColor`) while 6 paths wrote raw `strip_.Color()` and bypassed
correction entirely (found by MAD/Codex): status indicators, hourly chime, bloom stamen core, Galaxy-Spin star
tints, WiFi-portal fill, factory-reset fills. That inconsistency is the "front falling off." Now unified.

WHAT the correction is (gamma value, future white balance) lives solely in `ledGamma()`/`ledGammaInit()` — change
it there and every color follows. That is the accuracy knob, deliberately separated from the plumbing.

Face/theme/animation colors are **mathematically unchanged** from 2.22.1 (`scale(gammaColor(r,g,b),level)` ≡ the
old inline `ledGamma`-then-×level). Only the 6 formerly-bypassed paths change appearance (now gamma-corrected).

**NOT yet hardware-verified.** Known accuracy follow-up: some low-value diagnostic colors (e.g. the idle status
head `40,40,40`) dim substantially under gamma and may need their raw values bumped for visibility — an accuracy
tweak, made in one place, not a precision regression.

### Changed
- New `gammaColor(uint8_t,uint8_t,uint8_t)` — the single color-correction chokepoint (packs 0x00RRGGBB via
  `ledGamma` per channel). Documented invariant: compositors (`blendMax`/`blendOver`) and brightness ops
  (`scale`/`dim`/`pulse`) receive already-corrected colors and must NOT re-route through it.
- `ringColor()` → `scale(gammaColor(r,g,b), level)` (identical result). `renderCenterIdle()` → same idiom.
- Routed through `gammaColor`: `renderStatus`, `renderHourlyChime`, `stamenColor`, Galaxy-Spin tints, WiFi-portal
  fill, factory-reset fills. `ledCorrect()` remains a no-op.
- `FIRMWARE_VERSION` 2.22.1 → 2.23.0.

## [2.22.1] - 2026-07-13 (Fix: gamma applied to color BEFORE brightness, not on output buffer — dark inner/middle rings)

2.22.0 applied the gamma-2.2 LUT to the finished output buffer (`ledCorrect`), which runs AFTER `renderFace`
pre-dims the ambient fills via `scale(ringColor(color,255), middleFaceScale≈55)`. Gamma then hit `0.22·color`
and crushed the scale factor itself (`0.22^2.2≈0.03`, ~6× extra dimming) → inner/middle ring fills emitted
near-black on hardware (observed on the 15" at 2.22.0; hour hands + center use level 255 so were unaffected).
Same failure class as the reverted gamma-2.8 attempt. Fix: gamma now applied to the pure COLOR before the
per-ring level/scale multiply — WLED's documented "gamma on color, brightness after" ordering (its own code
warns "applying gamma after brightness has too much color loss"). **NOT yet hardware-verified — needs operator
flash + visual check on both variants.**

### Changed
- `ringColor()` (`src/main.cpp`): gamma-correct r,g,b via `ledGamma()` before the `×level` multiply. Covers
  face, hands, markers, and all animation colors (which route through `bandColor`/`bloomColor`/`paletteColor`).
- `renderCenterIdle()`: center bypasses `ringColor`, so gamma its color before the `amount` scale too.
- `ledCorrect()`: now a no-op (was the buffer-gamma). Correction lives at the color stage; no double-gamma.
- New `ledGamma(uint8_t)` helper (LUT unchanged). Hardcoded status/chime diagnostic colors remain raw.
- `FIRMWARE_VERSION` 2.22.0 → 2.22.1.

## [2.22.0] - 2026-07-13 (WS2812B output pipeline → WLED reference: gamma 2.2, white-balance scaling removed)

Replaced the output color correction to match the operator's verified WLED "Color & White" reference config
(screenshot from a WS2812B WLED 16.x reference rig; gamma implementation verified against
github.com/wled/WLED `wled00/colors.cpp`). The former linear white-balance scaling (green×176/255, blue×240/255)
fought warm mid-tones (gold/amber skew); it is removed. In its place, a 256-entry output gamma LUT
`gammaT[i]=pow(i/255, 2.2)*255` is applied identically to R, G, B on the finished GRB buffer right before show(),
exactly as WLED does. Distinct from the animation-shaping `gamma8()` (unchanged). Version 2.22.0 (skips the
poisoned 2.21.2 stuck on the 15" unit). SETTINGS_VERSION unchanged (16); no migration. **NOT yet hardware-verified
— needs operator flash + visual check.** Theme hexes were hand-tuned against the old pipeline and may need
re-tuning after this lands (the downstream theme task).

### Changed
- `ledCorrect()` (`src/main.cpp`): linear G×0.69 / B×0.94 white-balance scaling → gamma-2.2 LUT on all channels.
- New build flag `LED_GAMMA` (default 2.2). `LED_CORRECT_ENABLED 0` still bypasses correction entirely.
- `FIRMWARE_VERSION` 2.21.1 → 2.22.0.

## [2.21.1] - 2026-07-13 (Theme fills redone as radial ring gradients — Bird of Paradise + Go Pack)

The three ring-fill fields (outerFiller → middleFace → innerFace) were painting near-identical hues, wasting the
3-ring + center radial structure. First two themes redone so the four fills form an intentional radial gradient
with visible ring-to-ring boundaries, tuned in a WS2812-honest mock (per-field level scaling → FastLED-style
warm correction G×176/255, B×240/255 → gamma-encoded to match the strip's linear output). Remaining 6 themes
pending hardware validation of this color model. FIRMWARE_VERSION 2.21.0→2.21.1; SETTINGS_VERSION unchanged (16);
theme recolor is wire-safe (no migration).

### Changed
- **Bird of Paradise** fills → real cobalt→azure→teal→fiery-orange sweep. Was 3 near-identical cobalt blues
  (`#10309C`/`#12369C`/`#0C2680`, mid-face scales 55/50). Now outerFiller `#0E28DC` (cobalt, level 120→135),
  middleFace `#1488D2` (azure, scale 55→95), innerFace `#10B48C` (teal, scale 50→90), center `#FF7A1E`→`#FF5A00`
  (fierier, level 190→205). Orange markers/hands unchanged in hue (marker level 230→225, hours `#FF6410`→`#FF7014`).
- **Go Pack** greens now brighten inward + golds deepened (were washing pale on hardware). Fills: outerFiller
  `#0E2A16`→`#08301A` (deep pine, level 120→130), middleFace `#16401F`→`#14702E` (forest, scale 55→90),
  innerFace `#0E2A16`→`#24B445` (emerald, scale 50→82, no longer muddy-dark), center `#E0A814`→`#D48806`
  (level 190→200). Golds deepened: outerMarker `#D4A017`→`#A86A00` (level 235→205), hours/minutes `#D4A017`/
  `#C8960C`→`#B47A06` (hours level 255→235), innerHour `#C8960C`→`#A86400` (level 255→235).

## [2.21.0] - 2026-07-12 (Go Pack gets true green/gold animations — new "Green & Gold" palette)

Follow-up to 2.20.0: Go Pack was animating gold-only (the "Clock colors" palette maps the gold hands/marks,
not the green fill). Added a dedicated green/gold animation palette so Go Pack's animations read green + gold.

### Added
- **"Green & Gold" animation palette** (index 11): forest-green rings + gold throat/core, no sparkle. Go Pack
  now uses it (animationPalette 7→11). `ANIM_PALETTE_COUNT` 11→12; palette VALUES unchanged for 0–10;
  SETTINGS_VERSION unchanged (16).

## [2.20.0] - 2026-07-12 (Sunflower + Bird of Paradise themes; drop Snowstorm/GreenNGold; UI polish)

Palette + theme roster tuned toward more vibrant, high-contrast looks; theme UI tightened.

### Added
- **Sunflower** clock-theme (gold ticks/hands, rustic-orange accents, deep-mahogany/burgundy face) + its
  **Sunny** animation palette (index 9): golden-yellow → rustic orange → mahogany → dark-burgundy core.
- **Bird of Paradise** clock-theme (fiery-orange ticks/hands, ivory second hand, cobalt-blue face) + its
  **Blue Paradise** animation palette (index 10): complementary cobalt blue vs fiery orange, ivory accent.

### Removed / Changed
- **Snowstorm** animation palette removed (too close to Thunderstorm) — index 9 reused for **Sunny**.
- **GreenNGold** animation palette removed and its **gold-shimmer overlay reverted** (`applyGoldShimmer`
  deleted from `renderAnimFrame`) — index 10 reused for **Blue Paradise**. **Go Pack** now animates in its own
  green/gold via "Clock colors" (animationPalette 7) instead of a dedicated palette.
- **Go Pack** recolored deeper: dark green `#0E2A16`/`#16401F`, dark gold `#C8960C`/`#D4A017`.
- Palette VALUES still 0–10, SETTINGS_VERSION unchanged (16); a saved `animationPalette` of 9/10 now maps to
  the new palettes.

### UI
- **Motion & nudges** drawer collapsed by default.
- **Theme chips** pack tighter (narrower min width, smaller gap/padding); their color dot uses
  marker→face→center so Go Pack + the new themes read as green/blue gradients, not flat gold.

### Note
Existing flower palettes (Ember/Coral/Lagoon/Dusk/Nocturne/Tropic/Prism) still lean pastel — not retuned this
pass. New-theme/palette hexes + the layout tightening are best judged on hardware / the operator's screen.

## [2.19.0] - 2026-07-12 (palette names decoupled from themes + custom named themes (P3, browser-stored))

Two things. (1) The animation palettes no longer share names with the theme chips — flower names belong to
the themes (true-to-life flower colors); the palettes get their own descriptive names. (2) You can now save
the clock's current settings as a named theme and recall it.

### Changed
- **Animation palettes 0–6 renamed** (colors unchanged), decoupling them from the flower theme names:
  Dahlia→**Ember**, Cherry Blossom→**Coral**, Water Lily→**Lagoon**, Iris→**Dusk**, Moonflower→**Nocturne**,
  Bird of Paradise→**Tropic**, Wildflower→**Prism**. (`palettes.json` + regenerated `anim_palettes.h` +
  dropdown labels.) With GreenNGold (10), no name now appears in both the palette dropdown and the theme
  chips. Palette VALUES unchanged — no wire/EEPROM change.

### Added
- **Custom named themes (P3, browser-stored).** A "＋ Save as theme…" button in Quick controls snapshots the
  clock's current settings (all-inclusive) under a name you choose; saved themes appear as chips beside the
  built-ins, each with a delete ✕, and clicking one applies the full snapshot. Stored in browser localStorage
  (the "both" plan — on-device NVS is the later half). Built-in "ChronoBloom" stays a first-class chip.

## [2.18.0] - 2026-07-12 (GreenNGold animation + gold sparkle shimmer + Go Pack clock-theme)

Reclassifies the Packers colors correctly: "Green Bay Packers" was an animation palette (wrong — it's a
whole-clock look). Now split into two distinct things: **Go Pack** (a full clock-theme) and **GreenNGold**
(its animation palette, with a live gold sparkle overlay).

### Added
- **Go Pack clock-theme** — a built-in theme chip: dark-green rings, gold hour ticks + hands, white second
  hand, gold breathing core. Sets the GreenNGold animation palette. Hardcoded like ChronoBloom, so it needs
  no custom-theme storage (that UI is still P3).
- **Gold sparkle shimmer** for GreenNGold: `applyGoldShimmer()` overlays sparse bright-/throwback-gold
  twinkles over whatever animation is playing while palette 10 is active — rides the existing twinkle engine
  (Galaxy Spin stars / Firefly nudge), max-blended so it only ADDS light, hooked once in `renderAnimFrame`
  after `tickAnimation`, gated out during reminder animations. **NOT yet observed on hardware** — the mock
  can't render firmware animations; the shimmer needs a flashed-device eyeball.

### Changed
- **Animation palette 10 renamed "Green Bay Packers" → "GreenNGold"** and recolored: dark forest-green rings
  (#183C1E / #22582E), deep throwback-gold throat (#8A6A16), shiny gold core (#FFC42A). Dropdown label +
  swatch updated. The `animationPalette` VALUE stays 10 — no wire/EEPROM change, no migration.

### Naming (in progress)
Flower names now belong to the themes (true-to-life flower colors); animation palettes move to their own
descriptive names (Go Pack theme → GreenNGold palette is the first). Renaming the remaining flower palettes
(0–6) to distinct descriptive names is proposed, not yet applied.

## [2.17.0] - 2026-07-12 (web UI consistency P2: "Preview on clock" — flash-safe transient settings overlay)

Adds a real hardware preview for the clock-face theme (and any settings tweak): see a change on the LEDs
before committing it. Reuses the existing non-persistent-override pattern, generalized from animation style
to the whole settings block. RAM only — no EEPROM writes, no settings migration (SETTINGS_VERSION stays 16).

### Added
- **▶ Preview in the save bar** (`previewOnClock()`): posts the current unsaved face settings to the new
  **`POST /settings/preview`** endpoint, which renders them on the clock for ~10 s then auto-reverts to the
  saved face. No flash write, so it is safe to repeat and safe during OTA.
- **`SettingsStore` preview overlay:** `setPreview()/clearPreview()/tickPreview()`; `get()` transparently
  returns the overlay while active so the renderer needs zero changes; `getPersisted()` gives the JSON API
  and `/diag` the saved state regardless of an in-flight preview. `loop()` expires the overlay and forces one
  re-render back to the saved face. `POST /settings/preview/clear` reverts on demand; a real Save/reset
  clears any active preview.

### Changed
- **DRY:** the ~50-line `/settings` POST arg parser is now a shared `applyPostedSettings()` helper called by
  both `/settings` and `/settings/preview`, so the two parsers cannot drift.

## [2.16.0] - 2026-07-12 (web UI consistency P1: theme naming + Quick-controls Preview/Save)

First pass of the web-UI ↔ firmware consistency work found during hardware testing of 2.15.0. Vocabulary
unified toward "theme"; every quick control now has a visible Preview and Save. Labels only — no JSON
key / POST arg / EEPROM changes, so existing backups, exported themes, and the demo-reel designer keep
working, and no settings migration (SETTINGS_VERSION stays 16).

### Fixed
- **Animation-theme preview now covers palettes 8/9/10.** `/previewAnimation` clamped the preview palette
  to 0–7, so Thunderstorm/Snowstorm/Green Bay Packers could never be previewed (silently fell back to 7).
  Widened to `0..ANIM_PALETTE_COUNT-1` (`src/main.cpp`).
- **The animation theme is no longer a silent no-op.** The Quick-controls animation dropdown had no way to
  be *seen* on the clock (a palette only colors trigger animations, invisible while the face is idle) — it
  now has a **▶ Preview** button that plays a sample animation on the clock in the selected theme.

### Added
- **Quick controls: ▶ Preview + Save.** Preview plays a sample hour animation in the selected animation
  theme as a non-persistent override (self-reverting, no flash write). Save persists the quick controls
  (animation theme, speed, animation brightness, trail, reminder theme, brightness mode + levels) in one
  request, with the on-device "settings saved" confirmation.

### Changed
- **"Palette" → "Theme" in the UI.** "Animation palette" → **Animation theme**, "Reminder palette" →
  **Reminder theme**. Internally these remain the `animationPalette` / `reminderPalette` fields — a theme is
  a bundle that *contains* a palette; users pick the theme, the palette stays an implementation detail.
- **"Chime" → "interval animation"** wording (the device has no speaker; "chime" implied sound). "Animation
  brightness (chimes & nudges)" → "(interval animations & nudges)". The distinct hourly visual-sweep
  accessory `hourlyChime` is relabeled **"Hourly bloom"** (kept as its own feature; id/wire key unchanged).
- **No auto-save on theme change.** Selecting an animation theme / speed no longer writes to the clock on
  change; it updates the UI, and you Preview and/or Save explicitly. Matches the preview-then-commit model
  and removes the old asymmetry (animation palette auto-saved, reminder palette did not).

## [2.15.0] - 2026-07-12 (dashboard web UI redesign + 3 new animation palettes)

Full `web_html.h` redesign to the "dashboard" layout the operator chose (sticky preview + visual quick
row + four category drawers), plus three new animation palettes. Verified in the mock-endpoint browser
harness (every control id present once, live-preview firing, no console errors); both variants build clean.

### Added
- **3 new animation palettes** (`tools/palettes/palettes.json` → `anim_palettes.h`): **8 = Thunderstorm**
  (storm slate + electric-blue + lightning core), **9 = Snowstorm** (cold blue → white), **10 = Green Bay
  Packers** (team green rings + gold accent). `ANIM_PALETTE_COUNT` 7→11; index 7 stays the "Clock colors"
  firmware sentinel via a placeholder entry, so **deployed clocks storing 7 are unaffected — no migration,
  no palette flip.** Input clamps widened (`animationPalette` accepts 0–10).
- **Mode-adaptive Brightness** in the quick row: single `dayBrightness` slider in Manual/Scheduled (and
  when no sensor is detected, auto-detected via `/lux`), or a min↔max dim-range (`minAutoBrightness`/
  `maxAutoBrightness`) in Auto (sensor) mode. Separate **"Animation brightness (chimes & nudges)"** slider
  (`animationBrightness`) in the Motion drawer.
- **Theme swatch chips** (replaces the theme dropdown) and a **persistent Save / Set-default / Reset bar**
  in the preview card (no longer buried in a collapsed card).

### Changed
- **Dashboard layout.** Sticky preview (clock +25%, network line, save bar) beside a control column of a
  visual **Quick controls** card (theme · palette · speed · brightness) and four collapsible category
  drawers: **Motion & nudges** (default-open, accented) · **Colors & rings** (default-collapsed, set-once)
  · **Time & light** · **System**. Emphasis shifted to animations/nudges; the static color/ring tuning
  tucks away.
- Palette picker gains the 3 new palettes; Time & light labels made more descriptive.

### Performance
- Confirmed the richer brightness UI adds **no** load to the lux poll or WS2812 render path: it only sets
  bytes consumed by `effectiveBrightness()`'s `constrain()`. Lux is polled in `loop()` (never an ISR),
  throttled ≥120 ms with a ~1–3 ms non-blocking NOWAIT read.

## [2.14.2] - 2026-07-12 (web UI: back to one scannable page — tabs removed, Ring colors collapsible)

Reverts the tabbed layout (2.14.0/2.14.1) back toward the original single scannable panel after
feedback that too many tabs/cards made it hard to grasp. Web-UI-only; verified in a browser.

### Changed
- **Tabs removed.** The 5-tab bar and tab panels are gone; every card is visible again in one
  scannable column beside the sticky preview (the layout kind that was liked originally), just
  without the one thing that made it too long.
- **Ring colors is now collapsible** (native `<details>`, default collapsed) — the 9-row block that
  was the only real space hog tucks away, so the page reads short and scannable but the manual
  per-ring tuning is one click away. No animation, no per-card accordion (that was the disliked
  2.13.3 approach); just this one oversized block.
- **Removed the "ADHD" references** from the UI: the Focus Reminders card is now titled "Focus
  Reminders" with softer copy ("A gentle nudge to look up and shift focus…"), and the Demo Mode
  card says "focus nudges" instead of "ADHD nudges".

### Notes
- Pure markup/CSS/JS-init reorg — no control ids added, removed, or duplicated; the `showTab`/
  `initTabs` tab JS was removed and every existing binding (loadSettings/bindLive/saveSettings/draw/
  applyTheme) is untouched. Card order preserved: Presets, Ring colors, Ring setup, Effects, Color
  Theme, Animation Style, Time Animations, Focus Reminders, Time, Display, Auto-Brightness, Network,
  Admin, Full backup, Demo.

## [2.14.1] - 2026-07-12 (web UI: Face tab regroup — Presets up top, "Ring options" split)

Refinement of the 2.14.0 Face tab after feedback that Theme and Contrast preset were buried
and "Ring options" was a weak catch-all header. Web-UI-only; verified in a browser.

### Changed
- **Theme + Contrast preset promoted to a new "Presets" group at the top of the Face tab** — the
  fastest way to change the whole look is now the first thing you see, not a mid-panel dropdown.
- **"Ring options" split into two properly-named groups:** "Ring setup" (outer ring brightness,
  center LED, ring rotation offset) and "Effects" (second trail / progress / hourly chime / status
  toggles + trail and progress controls, plus Save / Save-as-default / Reset).
- No control ids added, removed, or duplicated — Theme/Contrast were moved, not copied; all wiring
  (applyTheme/applyContrastPreset/saveSettings/live-preview) is unchanged.

## [2.14.0] - 2026-07-12 (web UI: preview pane + five tabs — dark-only redesign)

Full reorganization of the settings page after the grouped-column and collapsible-card
attempts read as cluttered and over-compartmentalized. Grounded in how comparable control
panels are built (WLED tabs, OBS fixed preview, Hyperion-style consolidation). Web-UI-only;
**NOT on-LED verified** — needs an OTA reflash to see on the clocks.

### Changed
- **Two-pane layout: fixed preview + tabbed controls.** The clock preview keeps its own
  sticky grid column (`.stage`); the right column (`.ctrl`) is a 5-tab bar with one panel
  visible at a time — so you navigate instead of scrolling a long stack, and the preview can
  never overlap the controls (the 2.13.4 sticky-over-flow bug).
- **13 groups consolidated into 5 tabs:** **Face** (ring colors + ring options, no split) ·
  **Style** (color theme + animation style) · **Motion** (time animations + focus reminders) ·
  **Time & light** (set time, display, auto-brightness) · **System** (network, admin, backup,
  demo). Active tab persists in `localStorage` (`cbTab`).
- **Removed** the collapsible-card mechanism entirely (CSS + the six `*Card*` JS functions) and
  the 3-column masonry; panel chrome is lightened to hairline-separated groups instead of boxed
  cards, for a calmer, more compact feel.

### Notes
- Pure markup/CSS/JS-init reorganization — no control ids added, removed, or duplicated, and the
  existing wiring (`loadSettings`/`bindLive`/`saveSettings`/`draw`/`applyTheme` and every
  live-preview binding) is untouched. Dark-only (unchanged; the root already declares
  `color-scheme:dark`).

## [2.13.4] - 2026-07-11 (web UI: grouped 3-column layout + Rings split)

Reorganizes the settings column into purpose-grouped columns so the page reads as an
all-in-one glance instead of one long stack. Web-UI-only; **NOT on-LED verified** — needs an
OTA reflash to see on the clocks.

### Changed
- **Right settings area is now a 3-column grid** (`.cols`/`.col`) grouped by purpose:
  *Look* (Ring colors, Ring options, Color theme) · *Motion & nudges* (Animation style, Time
  animations, Focus reminders) · *Time · light* (Time, Display, Auto-brightness, Demo). Collapses
  to 2 columns ≤1200px and 1 column ≤820px.
- **Network, Admin, and Full settings backup moved under the clock preview** (new `.leftcol`
  wraps the sticky preview + these three state cards), where they fit tidily once the tall cards
  are collapsed.
- **The Rings card is split in two:** `Ring colors` (the 9 ring color+level rows + a Save button)
  and `Ring options` (outer-ring brightness, contrast preset, theme, center source, toggles,
  trail/progress, rotation offset, Save/Save-as-default/Reset). This removes the one oversized card
  that no arrangement could fit at a glance.

### Notes
- Pure markup/CSS reorganization — no control ids added, removed, or duplicated (`resetBtn` stays
  unique in Ring options), so `saveSettings`/`bindLive`/`applyTheme` and every live-preview binding
  are unchanged. Collapsible-card behavior (2.13.3) applies to all cards including the two new ones.

## [2.13.3] - 2026-07-11 (web UI: collapsible setting cards)

Every settings card in the web UI is now collapsible, so the ~12-card page reads as a tidy
column of headers you expand on demand instead of one long scroll. Built to also give a clean
web-UI shot for the launch video (a compact column of headers shows the breadth of
customization; expanding one shows the depth). **NOT hardware-verified on the LEDs** — this is
a web-UI-only change; verified in a browser against the served page (toggle, keyboard, state
persistence, expand/collapse-all, and that live-preview controls still fire). Needs an OTA
reflash for the operator to see it on the clocks.

### Added
- **Collapsible cards.** Each `.panel` header (`<h2>`) is now a real button (role/tabindex/
  `aria-expanded`, toggles on click and Enter/Space) with a rotating chevron. Card open/closed
  state persists per-card in `localStorage` (keyed by title) across reloads.
- **Expand all / Collapse all** toolbar at the top of the settings column.
- First-visit default is **all cards collapsed** (`DEFAULT_CARDS_COLLAPSED` in the page script —
  flip to `false` for expanded-by-default); after that, each card remembers its own state.

### Implementation notes
- Card bodies are wrapped and the header/chevron injected **in JavaScript at load**
  (`initCollapsibleCards()`), so the existing PROGMEM markup and every `oninput`/`onchange`
  live-preview binding are left untouched — no per-panel HTML surgery, no risk of desyncing the
  concatenated string literals.
- Smooth height animation uses the CSS grid `grid-template-rows: 1fr ↔ 0fr` transition (no JS
  height math, no `max-height` phantom-space lag). A `.no-anim` class suppresses the animation
  during initial state restore so cards don't animate on page load.

## [2.13.2] - 2026-07-11 (auto-brightness demo: snappy ramp during the lux-override sweep)

The `/demo/brightnessCycle` auto-dimming showcase visibly dimmed on 2.13.1 but the
dim/bright pulses read as sluggish — the exponential brightness ramp (tau 220ms) eases
in near the target, so the last third of each transition "crept" and looked mushy. This
was confirmed on the live 15" clock (idle, auto-brightness on, range 10–255): the sweep
worked but lagged the 1.5s lux segments. **Hardware-verified:** flashed to the clock and the
operator confirmed the auto-brightness demo dimming now reads correctly — snappy dim/bright pulses.

### Changed
- **`LuxSensor::autoBrightnessCached()` ramp time-constant is now conditional on the lux
  override.** `kRampTauMs` drops to **70 ms while the brightness-demo override is active**
  (crisp tracking of the dim/bright sweep) and stays **220 ms for real ambient changes**
  (so the display never visibly jumps when room light shifts). Normal auto-brightness feel
  is unchanged; only the demo pulses get snappier.
- Remaining speed lever if still too slow: the 1.5s per-segment sweep in
  `DemoMode::brightnessCycleLux()` — shortening those segments makes the pulses crisper.

## [2.13.1] - 2026-07-11 (reel dissolve: eased, slightly longer — softer transitions)

Polish pass on the reel cross-dissolve after on-hardware feedback that transitions
(especially into the reminder/nudge section) read as "abrupt." NOT LED-verified —
built clean, handed to the operator to flash and judge the feel.

### Changed
- **`ClockRenderer::tickReelMode()` reel dissolve is now eased, not linear.** The
  `masterFade_` ramp runs through `ease8()` (smoothstep) instead of a constant-rate
  linear slope, so every dissolve starts and ends gently. An eased curve also masks
  frame-timing unevenness better than a linear one (may soften a perceived stutter,
  though a single-core render hitch at the HTTP trigger — the leading suspect for the
  "flicker on nudge start" — is not addressed by this).
- **`REEL_FADE_MS` 500 → 600 ms** — a touch more time for the dissolve to breathe.

### Diagnosis notes (verified this session, no bug found)
- The reminder-step "flash/stutter" is NOT a gross firmware bug: the reel dissolve
  ramps smoothly (measured `master_fade` 0→81→180→255), the nudge animations start
  from black with soft eased attacks, `scaledElapsed` at speed 2 is monotonic, the
  catId→animation mapping is correct (stale header comments at line ~866 misled;
  the real animRem1/3/4 comments confirm Gentle Pulse / Ripple In / Heartbeat), and
  animations don't overlap their slots. `/diag` telemetry can't expose per-frame
  rendering, so a sub-100ms flicker can't be reproduced remotely.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.13.0 → 2.13.1), `src/main.cpp`,
  `docs/CHANGELOG.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.13.0] - 2026-07-10 (demo reel Live Run: latched reel mode — the face can no longer flash between animations)

The v2.12.2/2.12.3 reel transition was **face-suppression via per-animation timed safety windows**, driven from the browser one `/reel/hold` call per animation. Whenever the browser's timing slipped, an animation overran its predicted slot, or a step hung, the window expired and the main loop fell through to the face-render branch — so the live clock face flashed back in between animations. That is the reported defect, and it is structural to a timed-window design, not a tuning problem.

This replaces it with the model **`DemoMode` already uses** and has always been robust: **latched reel mode + on-device auto-dissolve.** Face-suppression is now a *latch*, not a timer; the cross-dissolves are generated *on the device* off each animation's start/end edges. The browser only says "begin reel / fire trigger / end reel" — it never micro-manages fade phases or races an expiring window.

### Changed
- **`ClockRenderer`: `armReelTransition()`/`tickReelTransition()`/`reelTransitionActive()`/`clearReelTransition()` → `beginReel()`/`endReel()`/`tickReelMode()`/`reelModeActive()`/`reelRendersFaceNow()`/`noteReelActivity()`/`clearReel()`.** While reel mode is latched (`reelActive_`), the loop draws the live face **only** while dissolving the face itself out (reel open) or back in (reel close); every other idle beat between animations is pure black (`renderTransitionBlack()`). An animation ending snaps `masterFade_` to 0 (it already self-faded via its own release envelope); the next animation starting fades `masterFade_` 0→255 over `REEL_FADE_MS` (500 ms) — a uniform, gentle bloom-from-black on every animation. Same `masterFade_`/`scaleStripBuffer` channel as before.
- **Crash-guard is now idle-only.** `reelSafetyDeadlineMs_` is continuously refreshed while an animation is actually on screen and on each `/previewAnimation` (`noteReelActivity()`), so it only counts down during genuine idle gaps. If the browser stops driving for `REEL_SAFETY_MS` (8 s) with nothing animating, the clock dissolves back to its face on its own — a closed/crashed page can never strand it black, and an overrunning animation can no longer trip the guard mid-reel (the 2.12.3 flash trigger).
- **Endpoints: `POST /reel/hold` → `POST /reel/begin` + `POST /reel/end`.** `begin` latches reel mode (optional `ms` = idle crash-guard, clamped 2000–20000, default 8000); `end` dissolves back to the face. No per-animation fade params anymore.
- **`demo_reel_designer.html` Live Run** now calls `beginReel()` **once** (on the first animation, and again whenever animations resume after a face-based step) and `endReel()` for face steps (brightness / idle / end card) and at run end/abort. The per-animation `armReel()` calls and the `REEL_OPEN`/`REEL_BETWEEN_STEPS`/`REEL_WITHIN_STEP` profiles are gone. On open it waits `REEL_OPEN_SETTLE_MS` (650 ms) so the live face dissolves out before the first animation blooms in.
- **`demo_reel_designer.html` Live Run pacing — removed the `/anim/status` event-pacing poll (`waitForAnimIdle()` / `animStatusAvailable()`) → robust timer pacing. THIS is the reliability regression the reel had been suffering.** The poll was added ~v2.11 to stop slow animations being cut off, but a single timed-out request (device busy / WiFi jitter / **the browser tab backgrounded to start a screen recording**) blocked the run for seconds with nothing on screen, the firmware's reel idle-guard then lapsed, and the reel dropped to the live clock face and never recovered — the intermittent "reel shits out around the reminder step" report. Timer pacing sleeps the speed-corrected catalog duration + `ANIM_GAP_S` and never reads device state mid-run; catalog durations overestimate real runtime so nothing is cut off (the original reason for adding the poll), and the firmware refreshes its reel guard while animating + on each trigger so the latch can't lapse between beats. Verified: a full reel driven from the page ran start-to-finish on the 15" (`/diag` showed `reel_active:true` held through every step incl. the reminder step, then a clean finish).
- **`demo_reel_designer.html` sequence editor — the "+ add animation…" pulldown no longer removes an animation once it is in the step.** The same animation can now be added multiple times to one step (separate chips, each with its own repeat count and order, e.g. A, B, A).
- **`demo_reel_designer.html` Live Run — silent Web-Audio keepalive so a hidden/minimized tab can't stall the reel.** Browsers throttle `setTimeout` in a backgrounded tab (down to ~1/min), which stretched the timer pacing past the firmware's idle-guard and dropped the reel to the clock face — the "it fucks off when I switch away to record" failure. `startKeepAlive()`/`stopKeepAlive()` run an inaudible (gain 0.0008) 30 Hz Web-Audio source for the duration of a Live Run; an audio-producing tab is exempt from timer throttling, so pacing stays at full rate while hidden. **Verified on the 15" with the tab deliberately hidden: `reel_active` held `true` through every animation step (incl. Supernova→Comet and the whole reminder step) with tight pacing, then a clean "Done" — vs. the same hidden-tab run *without* the keepalive, which dropped to the face at ~t=40s with 39-second dead stretches.**
- **NEW: standalone native reel player (`scripts/play_reel.py` + `scripts/play_reel.bat`) — the browser-free "nuclear option" for reliable playback.** Plays an exported reel `.json` DIRECTLY against a clock over HTTP from a native Python process, using the same firmware endpoints (`/settings`, `/reel/begin`, `/previewAnimation`, `/demo/brightnessCycle`, `/reel/end`). Because it is not a browser tab, it is structurally immune to all three browser failure modes (hidden-tab throttling, stale cache, poll stalls). Timing is native `time.sleep()` paced to the same speed-corrected catalog durations as the designer, so it matches the exported `.srt`. Drag a reel `.json` onto `play_reel.bat` (or `python play_reel.py reel.json --host … --preroll 5`). **Verified on the 15": played a real exported reel start-to-finish, `reel_active` held `true` through every animation step (Supernova + the whole reminder step included), timeline matched the `.srt` to the second, clean finish.** No third-party deps (urllib only). The in-page Live Run remains for quick checks; this is the recommended path for recording.
- **Launcher now force-serves the current designer HTML (kills the stale-cache trap).** `scripts/demo_reel_designer.bat` and `.claude/launch.json` no longer use `python -m http.server` (which sends no cache headers, so a browser reused a *stale cached* copy of the designer — the reason a fixed Live Run kept "still failing"). New `scripts/reel_server.py` sends `Cache-Control: no-store` and strips `If-Modified-Since`/`If-None-Match` so it can never answer 304 with a stale body; the `.bat` also appends a `?v=<random>` cache-buster to the URL. Added a visible **"Live Run driver: timer-paced · build 2026-07-11"** stamp on the Export tab so it's obvious at a glance whether a fresh page is loaded (if the line is missing, the browser is on a cached copy).
- **`/diag`** gains `reel_active` and `master_fade` so reel mode's latch/dissolve state is observable over LAN (this is what made the above verifiable remotely, without eyes on the LEDs).
- **`DemoMode::start()`** now `clearReel()`s first, so a latched reel can never linger under the on-device Start Demo showcase.

### Verification status
- **Firmware (2.13.0) flashed to the 15" and verified on the device.** Both variants build clean. Driving the reel endpoints by hand against the live clock showed the latch working exactly as designed: `/reel/begin` → `reel_active:true, master_fade:0`; firing an animation → `master_fade` ramps 0→255 (blooms from black); the **gap between animations holds `reel_active:true, master_fade:0` — the clock face is never drawn**; `/reel/end` → back to the face.
- **Full reel driven from the designer page ran start-to-finish on the 15", observed via `/diag`.** `reel_active` stayed `true` through Q3 → H1 → the whole hour showcase (Hr1–Hr4) → **the reminder step (Rem1/6/5/4), where it used to die** → clean handoff to the brightness/end-card face steps → `liveStatus: "Done — sequence complete."` No revert to the clock face at any animation boundary.
- **Not yet visually confirmed on the LEDs by a human** — the dissolve *smoothness/gentleness* is the operator's call. But "plays as one uninterrupted reel, never reverting to the clock face" is confirmed by device telemetry, not a sim.
- **The designer fix needs NO reflash** — reload `demo_reel_designer.html` (the `.bat` serves the current file) and the current flashed 2.13.0 firmware works with it.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.12.3 → 2.13.0; SETTINGS_VERSION unchanged at 16), `src/main.cpp`, `docs/CHANGELOG.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/publish/demo_reel_designer.html`, `scripts/reel_server.py` (new, no-cache static server), `scripts/demo_reel_designer.bat` (use reel_server.py + `?v=` cache-buster), `.claude/launch.json` (demo-reel-designer → reel_server.py)

## [2.12.3] - 2026-07-10 (reel Live Run: real dissolve transitions, not a hard cut to black)

v2.12.2's `/reel/hold` was a binary switch — black or normal, no fade — so a Live Run still read as a hard cut (into black, then a pop to the next animation), just without the live face in between. This replaces it with an actual fade-out → hold → fade-in timeline on the same `masterFade_`/`scaleStripBuffer` channel already proven true-black-capable, so reel transitions genuinely dissolve.

### Changed
- **`ClockRenderer::holdReelBlank()`/`reelHoldActive()` → `armReelTransition()`/`tickReelTransition()`/`reelTransitionActive()`.** Mirrors `DemoMode`'s `armBreather()`/`updateFade()` model (same channel, same guarantees) but every phase length is caller-supplied via the endpoint, and it's ticked every `loop()` iteration (gated on `!demoMode.active()` so the two features never fight over `masterFade_`).
- **`POST /reel/hold`** gains `fadeOutMs` / `holdMs` / `fadeInMs` params (all optional, sane defaults; `ms` remains the auto-expiring safety ceiling, widened automatically if the phases exceed it). `ms=0` still means "release immediately."
- **`demo_reel_designer.html`** Live Run (`holdReel()` → `armReel()`/`releaseReel()`) now arms a distinct profile per boundary: **`REEL_OPEN`** (400/250/300ms) leaving the live face at reel start, **`REEL_BETWEEN_STEPS`** (0/450/400ms) moving to a new sequence step — the "smooth" transition — and **`REEL_WITHIN_STEP`** (0/150/200ms) between repeats/animations inside the same step — "briefer but still gradual." Fade-out is 0 (snap) except at `REEL_OPEN`: every other boundary starts from an animation that already faded itself near-black via its own attack/release envelope, so there's normally nothing bright left to dissolve away.

### Verified
- Headless port of both the firmware controller and the designer's profile-selection logic across a representative reel: **0/720** idle-gap samples show the live face (unchanged guarantee from 2.12.2); every armed transition's `masterFade` ramps through 21–41 distinct intermediate values (not a single jump) — confirms `WITHIN_STEP` is measurably briefer than `BETWEEN_STEPS`/`OPEN` while still being a real ramp, not a snap. **DEPLOYED + VERIFIED on the 15" via OTA** — `/diag` reports 2.12.3 and the operator confirmed the reel transitions on hardware. 8" flash pending (offline at flash time).

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.12.2 → 2.12.3; SETTINGS_VERSION unchanged at 16), `src/main.cpp`, `docs/CHANGELOG.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/publish/demo_reel_designer.html`

## [2.12.2] - 2026-07-10 (fix: demo reel Live Run showed the clock face between animations)

The v2.12.1 fixes were all in `DemoMode` (the on-device `POST /demo/start` showcase). But the **demo reel designer's Live Run** never uses `DemoMode` — it fires `POST /previewAnimation` per animation and event-paces via `/anim/status`. Between previewed animations the device was simply idle, so it rendered the live clock face in every gap. This is the "designer still demos the active clock face between animations" report.

### Added
- **`POST /reel/hold?ms=N`** — holds the display black (via `renderTransitionBlack()`) while no animation is playing, for `N` ms from now (clamped to 30 s, auto-expiring). `ms=0` clears it. `ClockRenderer::holdReelBlank()` / `reelHoldActive()`; a new branch in the main loop owns the display while the hold is active (so it can't fall through to the face-render branches). The auto-expiry means a closed/crashed browser can never leave the clock stuck black.

### Changed
- **`docs/publish/demo_reel_designer.html` Live Run** now calls `/reel/hold` before every animation (covering the animation + its trailing gap + 1.5 s margin, refreshed each animation) and clears it (`ms=0`) for steps that should show the real face (idle / brightness / end card) and when the run ends/aborts. Verified with a headless coverage sim: the hold covers 100% of the animation-step idle gaps (0 uncovered samples).

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.12.1 → 2.12.2; SETTINGS_VERSION unchanged at 16), `src/main.cpp`, `docs/CHANGELOG.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/publish/demo_reel_designer.html`

## [2.12.1] - 2026-07-10 (fix: demo still flashed the clock face between animations)

The v2.12.0 demo dissolve still cut briefly to the live clock face between animations. Two root causes, both fixed:

### Fixed
- **`applyMasterFade()` used `Adafruit_NeoPixel::setBrightness()`** to dim toward black. That rescale floors at brightness level 1 (never truly off) and quantizes hard at the low end, so a "masterFade = 0" hold still emitted a visible ghost of whatever was drawn. Replaced with `scaleStripBuffer()` — an in-place per-channel multiply of the pixel buffer (like `ledCorrect()`), so `masterFade = 0` is exact, true black, and the fade is smooth.
- **During breathers the demo rendered `render(timeModel.get())` — the live clock face — and relied on the fade to hide it.** Any imperfect/mistimed frame leaked the real face. Now structural: new `ClockRenderer::renderTransitionBlack()` (pure black frame) + `DemoMode::rendersFaceNow()`; the global loop draws the face **only** during genuine face-content steps (intro / center / brightness / end card) and their fade-out, and draws black during every animation-step breather. The clock face can no longer appear between animations, regardless of fade timing. The demo also no longer falls through to the `consumeDirty()`/`needs*` face-render branches while active.

### Added
- **`docs/publish/demo_transition_sim.html`** + `scripts/demo_transition_sim.bat` — a faithful JS port of the `DemoMode` transition state machine rendered on the web-UI-style clock, to watch/verify the dissolve→breather→fade-in transitions (with a "Legacy (pre-fix)" toggle to compare). A headless port also verified the invariant: the OLD logic selected the live face in 15 animation-step gap windows; the NEW logic selects it in 0.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.12.0 → 2.12.1; SETTINGS_VERSION unchanged at 16), `src/main.cpp`, `docs/CHANGELOG.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/publish/demo_transition_sim.html`, `scripts/demo_transition_sim.bat`

## [2.12.0] - 2026-07-10 (second-hand accessory tuning + tint blending, gentle demo dissolves)

The clock-face second trail and progress ring were drawn with a plain `setRingPixel` overwrite, so their fading tails punched dark holes into the face and faded to black instead of blending with the outer marks/fill/minute colors underneath — the exact failure `blendRingPixelMax()` was written to avoid. They now alpha-composite ("tint") over the face via a new `blendOver()` / `blendRingPixelOver()`, and gain four tuning knobs. The stock demo hard-cut between every animation; it now cross-dissolves through a calm breather. **SETTINGS_VERSION 15 → 16** with a prefix-preserving migration in `SettingsStore::begin()` — deployed units keep all their colors/config across the upgrade (no settings wipe).

### Added
- **Second-hand trail tuning** — `secondTrailLength` (2–12 LEDs, was a hardcoded 4) and `secondTrailStyle` (Classic geometric / Linear / Smooth comet falloff, via new `trailAlpha()`).
- **Progress ring tuning** — `progressLevel` (0–255 tint strength, was a hardcoded ~8%) and `progressStyle` (Uniform arc / Comet gradient that brightens toward the second hand). Progress still follows the seconds color.
- **`blendOver()` / `blendRingPixelOver()`** — standard "over" alpha-composite so an overlay crossfades into the pixel underneath rather than overwriting it to black.
- **Master-dimmer cross-dissolves in Demo Mode** — `ClockRenderer` gained a `masterFade_` (applied to every face/animation frame via `applyMasterFade()`); `DemoMode` drives it to fade out → hold a ~0.4s breather → fade the next piece in at every animation/step boundary, in line with the clock's subtle/gentle philosophy.

### Changed
- `renderSeconds()` now tints the trail and progress arc over the face instead of overwriting it.
- Demo steps re-timed (~+0.8s per transition for the dissolve+breather); web UI now says "~130-second" showcase.
- Web UI Display panel: new Trail length / Trail fade / Progress intensity / Progress style controls under the Second trail / Progress ring toggles; SVG preview reflects them.

### Migration
- v15 → v16: appended 4 bytes; `begin()` seeds them (length 4, style Smooth, progress 90, Uniform) and re-stamps the version, preserving every existing field. Stale v15 user-defaults blocks are treated as absent until re-saved (version-gated), so no garbage restore.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.11.1 → 2.12.0, SETTINGS_VERSION 15 → 16), `src/main.cpp`, `src/web_html.h`, `docs/CHANGELOG.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/publish/demo_reel_designer.html`

## [2.11.1] - 2026-07-09 (shared ring choreography: mirrored, quarter-turn staggered multi-ring effects)

Every multi-ring effect used to invent its own stagger (or none). One convention now governs all of them, in `choreoPos()` / `choreoTrail()`: **ring N starts a quarter turn (90°) ahead of ring N-1, and adjacent rings counter-rotate.** Phases land at 0° / 90° / 180° (outer / middle / inner); outer and inner run clockwise, the middle ring runs counter. No `ClockSettings` change (SETTINGS_VERSION unchanged at 15).

### Changed
- **Dual Orbit** (`animQ2`) was a single pair of orbiters on the outer ring. All three rings now carry the pair, mirrored and quarter-turn staggered — two interleaved spirals instead of one lap.
- **Three Comets** (`animH2`): counter-rotation reinstated. It had been removed once because a lone reversed middle ring read as "chunks of LEDs moving against each other"; with *every* ring mirrored against its neighbour and phase-staggered, the same motion reads as a weave rather than a broken relay. **This is a knowing reversal of that earlier decision** — revert this hunk if it doesn't hold up on camera.
- **Comet Relay** (`animHr4`): the three legs were three near-identical hand-rolled blocks. Collapsed into one loop over a `Leg` table driving the shared choreography, so the baton hands off across a mirrored stagger.
- **Galaxy Spin** (`animHr2`): base arm phases normalized from arbitrary `86`/`171` to true quarter turns (`64` = 90°, `128` = 180°), matching the other animations. Middle arm still counter-rotates.
- **Orbiting Orb** (`animRem2`): inner-ring orb now rides `choreoPos()` instead of a bare modulo.
- **Ceremony / Unfurl / Slow Bloom** (`animHr1`, `animH1`, `animRem5`): their per-ring *fill* sweeps also follow the convention, so a bloom unfurls mirrored and staggered. Ceremony's hand-rolled `(24 - i) % 24` middle-ring reversal is now just `choreoPos()`'s mirror.

`choreoPos()` is a permutation for a full fill (verified for all three ring sizes), and `choreoTrail()` places the tail behind the head in both directions.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.11.0 → 2.11.1), `src/main.cpp`, `docs/CHANGELOG.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.11.0] - 2026-07-09 (tranquility pass: nudge/animation abruptness fixes, Firefly nudge, demo reel Live Run timing fix)

Triggered by reel take C006 review: Slow Bloom cut off mid-swell, Galaxy Spin's stars read as a strobe, sharp transitions across the reminder set. Root cause of the cuts was a Live Run timing bug in `docs/publish/demo_reel_designer.html`, not the animations — see below. New user-facing endpoints + new reminder animation (minor bump). No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15).

### Added
- **Firefly reminder nudge** (`focusReminder_animation` mode 11 / `ANIM_REM6`, `animRem6()`) — 9 fixed points across the three rings, each breathing on its own staggered ~1.6s cycle via the new shared `twinkleLevel()` engine (also driving Galaxy Spin's stars). Unlike every other animation it repaints the idle clock face beneath itself and max-blends the fireflies over it: `renderAnimFrame()` hands each animation a cleared strip, and a nudge that lights only a handful of pixels would otherwise blank the clock for four seconds — which reads as the display switching off, not a gentle cue. Wired into the web UI dropdown, `/previewAnimation` (mode range now 0-11), stock DemoMode's step 5 showcase, and the demo reel designer's factory sequence.
- **`GET /anim/status`** — CORS-open (only endpoint in this API that is) `{"animating":bool,"phase":string}` status, so an external sequencer can event-pace animation triggers instead of guessing wall-clock durations. Used by the demo reel designer's Live Run.
- **`POST /demo/brightnessCycle`** (optional `ms` param, default 14000) — the auto-brightness dim/bright showcase, extracted from stock DemoMode step 7 into a standalone-triggerable routine. Previously this segment only existed inside the fixed stock demo sequence, so the reel designer's Live Run had no way to drive it and could only sleep through a static, unchanging clock face — exactly what happened in the C006 take (last 6.5s is a motionless idle clock, not a brightness cycle).

### Fixed
- **Demo reel designer's Live Run was beheading every reminder nudge mid-animation.** At subtlety > 0 the designer sends a lower `speed` value to the firmware, which runs animations at a slower rate (e.g. speed 1 = 0.5x = double the wall-clock duration) — but Live Run slept only the *nominal* catalog duration before firing the next trigger, cutting off whatever was still playing. Compounding bug: `subtletyEffective()`'s duration formula was `baseDuration * lerp(1.0, 0.6, f)`, claiming higher subtlety makes nudges *shorter* while the speed value it computes in the same function makes them run *longer* — backwards. Fixed by (a) deriving duration from the same `SPEED_FACTOR` table the firmware actually uses, and (b) event-pacing Live Run via `/anim/status` polling instead of sleeping a guessed duration at all — this generalizes to quarter/half/hour steps too, which were subject to the same class of bug if the global Animation Style speed was ever set away from 3.
- **Exported SRT / shot list carried nominal timings while the clock ran the real (slower) ones — the direct cause of caption desync.** `subtletyEffective()` fixed only the on-screen readout; `suggestDuration()`, which is what actually populates `step.duration` and therefore every export, still summed raw catalog values and consulted neither subtlety nor the global speed. It now derives real wall-clock length per animation. At the factory reminder settings (subtlety 65) the step's true cost is 19.4s, not the 13.7s previously exported. Saved reels in `localStorage` are migrated on load (`durSchema`), so an existing reel can't silently re-export the old wrong timings.
- **`CATALOG` durations were stale for the quarter and hour rows**, skewing every export: Dual Orbit 2.5→2.8s, Ceremony 7.0→9.0s, Galaxy Spin 10.0→9.0s, Supernova 8.5→8.0s, Comet Relay 8.5→8.0s, Deep Breath 9.5→9.0s. All rows now verified against the `dur` constants in `main.cpp`.
- **Live Run's inter-animation gap was unaccounted for.** Event-pacing adds a settle delay plus poll granularity per trigger (~0.35s × ~12 animations ≈ 5s of drift the SRT knew nothing about). The gap is now an explicit `ANIM_GAP_S` counted by both the exports and the run, and Live Run pads each animation out to its predicted slot so the recorded timeline matches the captions.
- **Live Run would silently stall on firmware older than 2.11.0.** `fetch` does not reject on a 404, so a missing `/anim/status` left every animation spinning out its full 3x safety timeout. Live Run now probes the endpoint once and falls back to timed pacing with a visible warning.
- **Galaxy Spin's "stars" were a synchronized full-white strobe, not a twinkle.** `hash8(se/130)` gated every star pixel on the same global 130ms clock, so all qualifying stars popped to pure `(255,255,255)` on the same frame — on camera that reads as a flash, not starlight. New shared `twinkleLevel()` engine gives each pixel its own staggered ~1.4s breathing cycle (eased fade in/out, ~22% of cycles host a star), tinted warm/cool white and capped at ~78% instead of pure white. Stars additionally fade with lane depth (`starLit()`) and are max-blended over the arm colour, so the rotating spiral arm dissolves a star instead of clipping it off in one frame — a temporal fade alone still left a spatial pop. Galaxy Spin's core also replaced a random 90ms jitter with a slow ~2.4s sine breath.
- **Comet Relay's three ring handoffs each vanished in one frame** when their time window ended, regardless of where the comet was mid-travel. Each ring's comet + trail now fades over the final 400ms of its window.
- **Heartbeat dropped to full black between its two beats** (~300ms gap, doubled at slower animation speed) — read as the clock switching off mid-nudge. Added a ~12% floor glow so the flower dims but never fully disappears between thumps. The floor is itself gated by an attack/release envelope: a constant floor would snap on at the first frame and off at the last, trading the mid-nudge gap for two new pops at the boundaries.
- **Orbiting Orb and Slow Bloom were the only two reminder nudges that could still hit full `animBr()`** — every other nudge already respected `NUDGE_CEIL` (nudge, not alert). Both now capped consistently with the rest of the set.

### Changed
- Reminder step caption rewritten to state the nudge philosophy instead of just listing animation names: "a subtle, clear cue to shift gears out of hyperfocus. An ask, not an alarm."
- End-card caption no longer claims Printables/Hackaday.io are live (they weren't) — now "Free & open source: GitHub, with more platforms coming soon."

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.10.9 → 2.11.0), `src/main.cpp`, `src/web_html.h`, `docs/publish/demo_reel_designer.html`, `docs/publish/TRANQUILITY_PASS_PROPOSAL.md` (new), `docs/CHANGELOG.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`

## [2.10.9] - 2026-07-08 (palette model rebuilt to per-ring colors; hour-hand triangle; WS2812 color correction)

Supersedes the v2.10.3 OKLab-LUT color approach. Reached through dev iterations 2.10.4–2.10.8, flashed to the 15" for on-wall testing rather than released separately. No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15). **Retrospective:** most of the 2.10.4–2.10.9 churn was self-inflicted — the fix was a per-ring color model the codebase *already had* (palette 7) plus a standard FastLED correction; see CLAUDE.md "Engineering Principles — Prior Art & Simplicity First."

### Changed
- **Flower/reminder palettes rebuilt from 64-entry OKLab gradient LUTs to four explicit per-ring colors** `[outer, middle, inner, center]`, rendered SOLID per ring — the same path palette 7 ("Clock colors") already used for configured face colors. Root cause of "all palettes wash to blue-white / Iris gold throat missing": the flower palettes swept the whole gradient across every ring, so a palette that is ~80% blue (Iris) rendered every ring mostly-blue and the gold throat was one drowned pixel. Assigning each ring its structural color fixes it by construction. Deleted the OKLab bake, the `min_l` luminance floor, the v2.10.3 gamma bake, and the interim saturation floor — none survived. `tools/palettes/palettes.json` now carries a `rings` array per palette; `tools/gen_palettes.py` emits `ANIM_PALETTE_RINGS[7][4][3]` / `REM_PALETTE_RINGS[4][4][3]`; `ClockRenderer` gained `anchorColor()` and routes `paletteColor()`/`bandColor()`/`bloomColor()` through it.

### Fixed
- **Hour hand renders the intended outward-pointing triangle.** The rings are radially aligned but the middle-24 is wired one slot ahead of the inner-12 (middle index `2h+1` lines up with inner index `h`). `renderHours()` advances by thirds: :00–:19 spoke `inner{h}`/`middle{2h+1}`; :20–:39 triangle `inner{h,h+1}`/`middle{2h+2}` (apex centered one ring out); :40–:59 spoke at the next hour. Previously the inner ring lit only one LED and the middle was mis-offset, giving a skewed/inward "scalene" shape.
- **WS2812 color correction added at the output stage** — `ledCorrect()` in `ledShowBudgeted()` and before each center-strip `show()`. Green ×176/255, blue ×240/255, red full (FastLED `TypicalLEDStrip` = `0xFFB0F0`), ported rather than adopting the whole library. Compensates the LEDs' native green/blue bias so warm hues (gold, amber) render true and blue stops dominating; applied once to the finished buffer so face + palettes + animations are corrected uniformly. Compile toggle `LED_CORRECT_ENABLED`; tune `LED_CORRECT_G` / `LED_CORRECT_B`.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.10.3 → 2.10.9), `tools/palettes/palettes.json`, `tools/gen_palettes.py`, `src/anim_palettes.h` (regenerated), `src/main.cpp`, `CLAUDE.md`, `docs/CHANGELOG.md`, `docs/FEATURES.md`, `docs/ANIMATIONS.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/SESSIONS.md`

## [2.10.3] - 2026-07-08 (fix palette wash-out on LEDs + reminder palette not applied to nudge modes 0-5)

Two on-hardware color defects reported against the v2.10.x flower palettes. No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15).

### Fixed
- **All palettes rendered as one of three washed-out looks on the LEDs** (whitish-blue / whitish-pink-purple / amber-gold) despite well-differentiated design colors. Root cause: the OKLab-baked LUTs stored sRGB (gamma-encoded) values, but WS2812 output is linear in the 8-bit duty value — writing sRGB directly compresses channel contrast, so saturation collapses (pastels → near-white, warm ramps → generic amber). `tools/gen_palettes.py` now bakes LED gamma (sRGB → linear, piecewise sRGB curve ≈ 2.2/2.4) into every LUT entry; `src/anim_palettes.h` regenerated. LUT entries are now linear LED values — intentionally darker/more saturated as raw numbers than the design hexes in `palettes.json`. Per-channel floor of 1 preserves the never-black invariant. Zero runtime cost; no render-loop change. Web-UI swatches keep the design sRGB colors (screens want sRGB — only the LEDs need linear).
- **Selected reminder palette had no effect on nudge animations for modes 0-5 — including mode 0, the default.** `focusReminder_animation` 0-5 reuse the quarter/half-hour/hour chime animations, which sampled the *animation* palette (or clock colors); only the dedicated nudges (modes 6-10) passed `useReminderPalette`. New `ClockRenderer::reminderPaletteActive_` flag: set by `triggerReminderDirectAnimation()` for every mode, cleared by the chime/preview triggers and when the animation returns to idle. `paletteColor()`/`bandColor()`/`bloomColor()` force reminder-palette sampling while it's set, so any animation fired as a focus reminder now renders in the configured reminder palette. Deliberate palette-independent accents (golden stamen core, Galaxy Spin star twinkles) unchanged. Also fixed the stale `focusReminder_animation` struct comment (claimed range 0-5; actual 0-10).

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.10.2 → 2.10.3), `tools/gen_palettes.py`, `src/anim_palettes.h` (regenerated), `src/main.cpp`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.10.2] - 2026-07-08 (rename the "Sakura" clock-face theme for naming consistency)

Closes the gap flagged in v2.10.1: the named clock-face **Theme** selector (`tools/themes/themes.json`, shipped in v2.9.1) still had a theme called "Sakura (cherry blossom)" while the animation/reminder palettes had already been renamed to "Cherry Blossom." No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15) — theme selection is client-side only (not persisted to EEPROM), so the key rename carries no migration risk.

### Changed
- **Renamed the "Sakura (cherry blossom)" clock-face theme → "Cherry Blossom (spring bloom)"**, matching the `Name (descriptor)` pattern used by the other themes (Moonflower "night bloom", Ember Dahlia "fire bloom", Lotus Pond "water lily"). JSON key `sakura` → `cherryblossom` in `tools/themes/themes.json`; restamped into `src/web_html.h` and `docs/publish/demo_reel_designer.html` via `tools/gen_themes.py`. Its `reminderPalette` mapping (index 2) is unchanged — and now happens to also be named "Cherry Blossom," so applying this theme picks a nudge with the matching name.
- `docs/THEME_SYSTEM_DESIGN.md` and `docs/FLORAL_COLOR_DESIGN.md` theme-roster references updated to match.
- **Demo Reel Designer sync**: `docs/publish/demo_reel_designer.html` still listed the pre-v2.10.1 reminder-palette names (`REMINDER_PALETTES` / `REM_PALETTE_HEX` = Amber/Red/Magenta/Cyan-warm) — now mirrors the firmware exactly (Golden Hour/Moonflower/Cherry Blossom/Lavender with representative LUT swatch colors). Its animation catalog, animation-palette list, and theme roster were already in sync. The `scripts/demo_reel_designer.bat` launcher holds no palette/animation data (static-server launch only) — nothing to change. Also synced the reminder-palette option lists in `docs/ANIMATIONS.md` and `docs/FEATURES.md`.
- **Documentation accuracy pass** — `docs/ANIMATIONS.md`'s per-mode catalog still described the pre-v2.4.0 29-mode *legacy* animation set (Sparkle Burst, Rainbow Sweep, Knight Rider, Thunderstorm, Neon Sign, …) under a banner admitting it was historical. Rewrote all four tiers to document the actual live 16 animations (Slow Comet/Dual Orbit/Bloom Ripple, Unfurl/Three Comets/Breathe, Ceremony/Galaxy Spin/Supernova/Comet Relay/Deep Breath, Gentle Pulse/Orbiting Orb/Ripple In/Heartbeat/Slow Bloom) with accurate behavior, color sources, and v2.9.7/v2.10.0 notes. Fixed the same stale animation names in `docs/FOCUS_REMINDERS.md` (incl. an incorrect "uses `delay()`" claim — reminders are non-blocking), `docs/API.md`, and `docs/ARCHITECTURE.md`, and the stale animation-name comment in `SettingsStore::defaults()` (comment only — no behavior change, no `FIRMWARE_VERSION` bump).

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.10.1 → 2.10.2), `tools/themes/themes.json`, `src/web_html.h`, `docs/publish/demo_reel_designer.html` (both restamped), `src/main.cpp` (defaults comment only), `docs/THEME_SYSTEM_DESIGN.md`, `docs/FLORAL_COLOR_DESIGN.md`, `docs/ANIMATIONS.md`, `docs/FOCUS_REMINDERS.md`, `docs/API.md`, `docs/ARCHITECTURE.md`, `docs/FEATURES.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.10.1] - 2026-07-08 (palette naming pass + color swatches in the web UI)

Follow-up to v2.10.0. No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15); palette indices/values unchanged — display names and web UI only.

### Changed
- **Renamed "Sakura" → "Cherry Blossom"** (both the animation and reminder palette). "Sakura" is a Japanese loanword; "Cherry Blossom" is its plain English name and was already used in the palette's own description text.
- **Simplified the remaining flower-palette names to plain, single-word-where-possible English**, dropping the paired poetic constructions: Ember Dahlia → **Dahlia**, Lotus Pond → **Water Lily**, Blue Iris → **Iris**, Wildflower Meadow → **Wildflower**, Lavender Dusk → **Lavender**. Moonflower, Bird of Paradise, Golden Hour, and Cherry Blossom were already plain and unchanged. Updated everywhere: `tools/palettes/palettes.json` (source of truth, regenerates `src/anim_palettes.h`), `src/main.cpp` struct comments, `src/web_html.h` dropdowns, `docs/API.md`, `docs/ANIMATIONS.md`, `docs/FEATURES.md`, `docs/publish/demo_reel_designer.html`.

### Added
- **Color swatches on both palette dropdowns.** A small chip next to Color palette / Reminder palette now shows the palette's actual color — previously the name was the only cue, so "what does Iris even look like" required picking it and watching the LEDs. Native `<option>` background-color styling is unreliable across browsers, so the swatch is a `<span>` driven by JS (`updatePaletteSwatches()`), hooked into the existing `draw()` loop (already running every 90ms) — zero new polling, negligible cost (two property reads + a style write). The "Clock colors" (7) swatch is a live 3-stop gradient of the user's own configured outer-marker/hours/center colors rather than a fixed swatch, since that palette has no fixed color.

### Not changed (flagged, not silently touched)
- **The named clock-face Theme selector still has a theme called "Sakura (cherry blossom)"** (`tools/themes/themes.json`, shipped in v2.9.1/413eec1). That's a separate feature (whole-face themes) from the animation/reminder palettes touched here, uses "sakura" as a JSON key, and predates this session's scope — left alone pending a decision on whether to rename it too.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.10.0 → 2.10.1), `tools/palettes/palettes.json`, `src/anim_palettes.h` (regenerated), `src/main.cpp` (comments only), `src/web_html.h` (dropdown text, swatch markup/CSS/JS), `docs/API.md`, `docs/ANIMATIONS.md`, `docs/FEATURES.md`, `docs/publish/demo_reel_designer.html`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.10.0] - 2026-07-08 (flower palette system — OKLab LUTs replace the legacy gradient palettes)

The animation color system is now flower-first, matching the ChronoBloom identity end to end. No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15) — palette indices reuse the same EEPROM bytes and value ranges (0-7 / 0-3).

### Added
- **`tools/gen_palettes.py` + `tools/palettes/palettes.json` → `src/anim_palettes.h`** — a palette pipeline mirroring the theme pipeline. Palettes are defined as hex keyframes and interpolated **offline in OKLab** (perceptually uniform — no muddy sRGB midpoints), then baked into 64-entry cyclic LUTs (~2.1 KB flash). The render loop is one array index + 8-bit lerp (`lutColor()`), no float math.
- **Seven flower animation palettes** (replace Rainbow/Fire/Ocean/Forest/Candy/Neon/Monochrome at the same indices): 0 Ember Dahlia (gold→crimson, warm analogous), 1 Sakura (blush→rose, monochromatic), 2 Lotus Pond (teal water→jade pads→lotus pink), 3 Blue Iris (periwinkle→violet + golden throat, split-complementary), 4 Moonflower (moonlight→night violet, the gentlest), 5 Bird of Paradise (orange/green/blue triadic — the event palette), 6 Wildflower Meadow (OKLab-equalized hue wheel — the designed rainbow). Palette 7 **Clock colors stays the default and is unchanged** (ring-mapped face colors).
- **Four flower reminder palettes** (replace Amber/Red/Magenta/Cyan-warm): 0 Golden Hour (default warm, successor to Amber), 1 Moonflower, 2 Sakura, 3 Lavender Dusk. All low-chroma/analogous so `NUDGE_CEIL` swells read gentle, never alert-red.
- **Theme→nudge unification**: each named theme in `tools/themes/themes.json` now selects its matching flower nudge (Moonflower theme→Moonflower nudge, Sakura→Sakura, Lotus Pond→Sakura, ChronoBloom/Ember Dahlia→Golden Hour). Applying a theme sets face colors + animation style + reminder palette in one click (the `applyTheme()` plumbing already existed).

### Changed
- **The position-0-black invariant is now structural.** Every LUT entry has a min-luminance floor baked in (`min_l` in palettes.json, verified by the generator's "darkest channel-max" audit) — no palette position is black, so solid ring fills are safe at any position. Dark lanes/fades come from animation envelopes (`galaxyWave`, `animEnv`), never the palette. The v2.9.7 hand-written invariant comment is replaced by this guarantee.
- Web UI dropdowns renamed to the flower vocabulary (with gradient hints, e.g. "Ember dahlia — gold → crimson"); demo reel designer palette list/colors updated to match.
- Docs updated: API.md, ANIMATIONS.md, FEATURES.md palette tables; FLORAL_COLOR_DESIGN.md marked implemented.

### Migration note
- Users who had a legacy gradient palette selected (0-6) get the flower palette at the same index — a deliberate replacement, not a bug. The default (7, Clock colors) and all reminder default behavior (0, warm) carry over with the same spirit.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.9.7 → 2.10.0), `src/anim_palettes.h` (new, generated), `src/main.cpp` (`lutColor()`, `paletteColor()` rewrite, comment updates), `src/web_html.h` (dropdowns + theme restamp), `tools/gen_palettes.py` + `tools/palettes/palettes.json` (new), `tools/themes/themes.json` (per-theme reminder palettes), `docs/publish/demo_reel_designer.html`, `docs/API.md`, `docs/ANIMATIONS.md`, `docs/FEATURES.md`, `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.9.7] - 2026-07-08 (bloom animations follow the clock theme + outer-ring blackout fix)

Color-fidelity + latent-bug fix for the bloom-family animations. No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15).

### Fixed
- **Bloom Ripple (and Unfurl, Ceremony, Supernova) now bloom in the clock's own colors instead of a hardcoded warm-gold/white core.** The center and inner-12 ring were painted with `stamenColor()` / `strip_.Color(255,255,255)` regardless of theme, so on any theme the bloom read as *white → middle-hand color → outer-marker color* rather than the configured face. New `bloomColor(band, frac)` maps each ring to its configured color, center-outward: center → `centerColor`, inner-12 → `innerHourColor`, middle-24 → `hoursColor`, outer-60 → `outerMarkerColor`. Unlike `bandColor()` (which maps the inner band to `secondsColor` so scrolling comets stay distinct), `bloomColor()` uses `innerHourColor` so the concentric rings intentionally fuse into one flower.
- **The 60-LED outer ring no longer stays dark during Bloom Ripple / Supernova on non-default palettes.** Both painted every outer-ring pixel at gradient **position 0**, which is *pure black* on Fire, Forest, and Monochrome — so the ring never lit. The bloom animations now spread `frac` across each ring (like Unfurl/Deep Breath already did), so gradient palettes sweep the ring and always include lit pixels; the ring-mapped "Clock colors" palette is unaffected (it ignores `frac`).

### Prevention
- Documented the position-0-is-black **invariant** at `paletteColor()`: a ring filled as one solid color must never sample a single fixed gradient position, and themed fills must route through `bandColor()`/`bloomColor()` rather than a hardcoded `strip_.Color()`. This is the trap that caused both the recurring "doesn't match the theme" reports and the dark-outer-ring bug.

### Not changed (by design)
- **Slow Bloom** (`animRem5`) is a *reminder* animation on the separate `reminderPalette` (Amber/Red/Magenta/Green); it already has no hardcoded core and no blackout, and a focus nudge is intentionally palette-independent of the clock theme, so it was left alone.
- **Comet Relay** and **Deep Breath** keep their existing behavior (Comet Relay's warm end-dot via `stamenColor()`; Deep Breath already themed via `bandColor()`), so `stamenColor()` remains in use.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.9.6 → 2.9.7), `src/main.cpp` (new `bloomColor()`; `animQ3`/`animH1`/`animHr1`/`animHr3` route through it and spread `frac`; `paletteColor()` invariant comment), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.9.6] - 2026-07-08 (stuck-button hold ceiling)

Hardening audit finding **M5** (`docs/HARDENING_AUDIT.md`). No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15).

### Fixed
- **A stuck/shorted UP or DOWN switch can no longer wind the clock away indefinitely.** `ButtonInput::poll()` previously fired hold-repeats for as long as a button read LOW; a failed switch (moisture, cold-solder bridge, shorted wiring) reads as a permanent hold and would race the time forward/back at ±60 min per hour-phase repeat and spam `STATUS_BUTTON` animations forever — looking like a firmware bug. A hold exceeding **30 s** (`kStuckHoldMs`) is now treated as stuck: repeats are suppressed until a real release edge is seen (`upStuck_`/`downStuck_` latch, cleared on release). Normal presses and legitimate long holds (minute/hour repeat) are unaffected.
- The **GPIO9 (DOWN) = BOOT strapping pin** caveat that motivates this — a DOWN button asserted at the reset instant drops the board into USB download mode — is already documented for builders in `docs/HARDWARE.md` (pin table + factory-reset note); the DOWN branch comment now cross-references it.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.9.5 → 2.9.6), `src/main.cpp` (`ButtonInput::poll()` stuck-hold guard; `kStuckHoldMs`, `upStuck_`/`downStuck_` members), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.9.5] - 2026-07-08 (JSON-escape SSID — odd network names no longer blank the UI)

Hardening audit finding **M2** (`docs/HARDENING_AUDIT.md`). No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15).

### Fixed
- **An SSID containing `"`, `\`, or a control character no longer breaks the web UI.** `/net` and `/diag` interpolated `WiFi.SSID()` straight into their JSON responses; such an SSID produced invalid JSON, the client's `JSON.parse()` threw, and the whole UI went blank — looking bricked to a non-technical user though the device was fine. New `WebUi::jsonEscape()` escapes quote/backslash/newline/tab and control bytes (`\uXXXX`) before interpolation; both endpoints now escape the SSID.
- **Truncated JSON is no longer sent as a 200.** `/diag` previously logged `snprintf` truncation but still returned the malformed (partial) buffer; `/net` had no check at all. Both now return **HTTP 500** with a small JSON error body on truncation instead of a body the client can't parse.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.9.4 → 2.9.5), `src/main.cpp` (`WebUi::jsonEscape()`; `/net` and `/diag` escape SSID + 500-on-truncation), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.9.4] - 2026-07-08 (I²C fault backoff — no more render stutter on a dropped sensor)

Hardening audit finding **M1** (`docs/HARDENING_AUDIT.md`). No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15).

### Fixed
- **A VEML7700 that drops off the bus at runtime no longer stutters animations forever.** `LuxSensor::lux()` is on the animation render hot path; a runtime I²C fault (ESD, loose wire, stuck SDA) previously made every read hit the core's ~50 ms Wire timeout and retry indefinitely (~8 fps stutter for the life of the fault, even though the error return was "handled"). Two changes bound the worst case:
  - `Wire.setTimeOut(10)` in `setup()` caps each transaction at 10 ms (ample for a VEML7700 register read) instead of the ~50 ms default.
  - `lux()` now counts consecutive read failures; after 10 (~1.2 s) it sets `available_ = false` and falls back to fixed brightness, then **slowly re-probes every 5 s** (full `veml_.begin()` re-init so a reconnected sensor recovers gain/integration/enable on its own). A healthy read clears the streak.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.9.3 → 2.9.4), `src/main.cpp` (`Wire.setTimeOut(10)` in `setup()`; `LuxSensor::lux()` error-streak + slow re-probe; `consecutiveErrors_`/`lastReprobeMs_`/`kLuxMaxConsecErrors`/`kLuxReprobeMs` members), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.9.3] - 2026-07-08 (OTA rollback mark-valid — wired, bootloader-gated)

Hardening audit finding **H2** (`docs/HARDENING_AUDIT.md`). No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15).

### Added
- **OTA "boot confirmed good" mark-valid.** New `markBootValid()` calls `esp_ota_mark_app_valid_cancel_rollback()` once, ~10 s into a boot that has rendered at least one frame — the point where a fresh OTA image has proven it can run. This is the app-side half of ESP-IDF app-rollback: with a rollback-capable bootloader, an image that boot-loops before reaching this point auto-reverts to the previous good partition.

### Known limitation (flagged, not guessed)
- **Auto-rollback is NOT active on the current build.** It requires the *bootloader* to be built with `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y`. PlatformIO's `framework = arduino` ships a **precompiled** bootloader that does not set it, and it cannot be toggled with a build flag — enabling it means rebuilding the bootloader from a custom sdkconfig (e.g. Arduino-as-IDF-component), a larger change deferred out of this patch. Until then `markBootValid()` is a deliberate no-op: the running partition reports `VALID` (never `PENDING_VERIFY`), so the mark-valid call is correctly skipped. The code is wired so adopting a rollback-capable bootloader later needs no firmware change. Note that `Update.end(true)` already validates the incoming image's embedded SHA/magic before switching the boot partition, so a *truncated* upload is still rejected today — the gap rollback closes is specifically a valid-but-boot-looping image.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.9.2 → 2.9.3), `src/main.cpp` (`#include "esp_ota_ops.h"`, `markBootValid()` + one-shot call in `loop()`), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.9.2] - 2026-07-08 (LED current budget — brownout guard)

Hardening audit finding **H1** (`docs/HARDENING_AUDIT.md`), root cause of the 2026-07-08 15" field failure (froze mid-preview → all LEDs off → recovered only on a 4 A powered hub). No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15).

### Added
- **Global LED current limiter.** 96× WS2812B at full white draws ~5.76 A — far past a typical 0.5–2 A USB supply — so a bright preview/animation frame could sag the rail and trip the brownout detector. Adafruit_NeoPixel has no current limiter, so a new `ledShowBudgeted()` estimates commanded current from the pixel buffer before every `show()` and, if it exceeds the budget, scales that frame's brightness down. It is a per-frame ceiling only: dimmer frames pass through untouched and the next frame re-sets its intended brightness. Applied to both render paths (`ClockRenderer::render`, `renderAnimFrame`) and the high-current boot displays (WiFi-portal blue fill, factory-reset red/white fills).
- **Per-variant `MAX_LED_MILLIAMPS` build flag** (both `esp32c3_v3_8inch` and `esp32c3_v3_15inch`, default 1800 mA = usable LED share of a 5 V / 2 A supply). Builders on a beefier PSU can raise it; a build without the flag falls back to 1800 mA in `main.cpp`.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.9.1 → 2.9.2; `MAX_LED_MILLIAMPS` on both envs), `src/main.cpp` (`ledShowBudgeted()` free function + `MAX_LED_MILLIAMPS`/`LED_MA_PER_CHANNEL_FULL` fallbacks; 5 `show()` call sites routed through it), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.9.1] - 2026-07-08 (OTA flash-collision guard)

### Fixed
- **Saving settings during an OTA flash could corrupt the update and brick the unit.** On the single-core ESP32-C3, a `/settings` EEPROM commit and the OTA partition write are both blocking flash operations; if they interleave (e.g. the user hits Save in the web UI while a `curl … /update` or ArduinoOTA push is streaming), the half-written OTA image can be corrupted, leaving the device in a boot-fail / multi-power-cycle recovery state (observed 2026-07-08 on the 15" unit). The three flash-writing endpoints — `POST /settings`, `/settings/reset`, `/settings/saveDefault` — now reject with HTTP 503 "Firmware update in progress — settings locked" while `Update.isRunning()` is true. Preview is unaffected (it uses non-persistent renderer overrides and never writes flash).

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.9.0 → 2.9.1), `src/main.cpp` (`Update.isRunning()` guard on the three settings-write routes in `WebUi::setupRoutes`), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

## [2.9.0] - 2026-07-08 (named theme system, center-LED source, animation contrast fidelity)

Design doc: `docs/THEME_SYSTEM_DESIGN.md`. No `ClockSettings` layout change (SETTINGS_VERSION unchanged at 15 — the retired `colorTheme` byte was renamed in place), so **no settings wipe on upgrade**.

### Added
- **Named theme system**: 5 curated bloom themes — ChronoBloom (clock default, locked to the maintainer's canonical export), Moonflower, Sakura, Ember Dahlia, Lotus Pond — selectable from a new **Theme** dropdown in the web UI (replacing the dead Classic/Aqua/Magenta selector). Each theme is a complete, valid theme-export (version 3): 9 ring colors, 9 levels, outer-ring brightness, and animation style, with `animationPalette` pinned to 7 so animations always inherit the theme. Single source of truth `tools/themes/themes.json`; `tools/gen_themes.py` stamps the roster into both `src/web_html.h` and `docs/publish/demo_reel_designer.html` (run it after any roster edit).
- **Center LED source selector** (`centerSource`, new "Center LED shows" dropdown): 0 = Wi-Fi/status + bloom pulse (default, today's behavior), 1 = bloom pulse only (status stays on the inner ring), 2 = status only, 3 = temperature (wired but falls back to 0 until a sensor exists), 4 = off. Reuses the EEPROM byte of the retired `colorTheme` field — same offset/size, so existing units upgrade with behavior unchanged.
- **Demo reel center-LED beat**: new 6s step (now 9 steps, ~115s) forcing a green `STATUS_WIFI_OK` pulse with the subtitle "Center LED — live Wi-Fi / status indicator, sensor-ready". `/demo/status` JSON now reports `steps` so the web UI progress line no longer hardcodes the count.

### Changed
- **"Clock colors" animations now preserve per-ring contrast** (`ClockRenderer::bandColor` palette-7 path): each band is scaled by its ring's configured level relative to the brightest configured band (outer→`outerMarkerLevel`, middle→`hoursLevel`, inner→`secondsLevel`, center→`centerLevel`). Previously every band rendered at one flat `animationBrightness`, erasing the tuned face/hand hierarchy during animations; now the brightest ring hits the Peak Brightness setting exactly and the rest hold their configured ratios.
- **Factory defaults aligned to the canonical ChronoBloom theme**: `outerMarkerLevel` 210→225, `animationBrightness` 200→157, `trailLength` 4→6, `outerRingBrightness` 90→77. A factory-fresh flash now boots into exactly the theme the dropdown calls "ChronoBloom (clock default)".
- **Demo reel designer**: dead Color theme dropdown replaced with the Center LED selector; new "Clock theme (ring colors)" picker (default = clock's saved theme) that pushes the theme's full ring color/level set with the settings and seeds the style controls; factory reel style now defaults to palette 7 / brightness 157 (was Rainbow / 220); palette list labels 7 as "Clock theme (default)"; localStorage saves migrate automatically.

### Removed
- Dead `colorTheme` code path: the Classic/Aqua/Magenta dropdown (web UI + designer) and its only consumer `ClockRenderer::secondColor()`, which was never called — the setting had no visible effect anywhere.

### Files changed
- `platformio.ini` (FIRMWARE_VERSION 2.8.0 → 2.9.0), `src/main.cpp` (`ClockSettings.centerSource`, `sanitize`, `defaults`, `bandColor` weighted bands, `centerSourceEffective`/`centerShowsStatus`/`centerIdleActive`, `render`/`renderStatus` center gating, `secondColor` removed, `DemoMode` step table/`stepTick`/`statusJson`), `src/web_html.h` (Theme + Center LED dropdowns, `THEMES` const + `applyTheme`/`updateThemePreset`/`fillThemeSelect`, `centerSource` in load/save, demo step count), `docs/publish/demo_reel_designer.html`, `tools/themes/themes.json` (new), `tools/gen_themes.py` (new), `docs/THEME_SYSTEM_DESIGN.md` (new), `docs/symmap.json`, `docs/FUNCTION_INVENTORY.md`, `docs/CHANGELOG.md`

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
- Lineage/attribution references normalized to the full `Maestro8484` handle across NOTICE, README.md, docs/HISTORY.md, AGENTS.md, docs/CHANGELOG.md, docs/SESSIONS.md.
- Planned MQTT topic placeholder in docs/API.md renamed from `iris-clock` to `chronobloom-clock` (leftover unrelated-project codename).
- Historical hardcoded OTA target IP generalized to a `192.168.1.x` placeholder across docs/API.md, docs/AUDIT_2026-05-13.md, docs/CHANGELOG.md, docs/SESSIONS.md, docs/TROUBLESHOOTING.md.
- Hardcoded local repo path (maintainer's real Windows username) replaced with `C:\path\to\chronobloom-esp32c3` across internal workflow/session docs not shipped in the public repo.

### Added
- `docs/publish/PROVENANCE_AUDIT.md` — pre-publish provenance/secrets audit ahead of the public release repo split.
- `docs/publish/RELEASE_MANIFEST.md` — exact include/exclude file list for the curated public repo, with user-decision items and pre-publish gates (Session 37B).
- Launch copy drafts in `docs/publish/`: `REDDIT_ESP32.md`, `REDDIT_3DPRINTING.md`, `REDDIT_ADHD.md`, `HACKADAY.md`, `PRINTABLES.md`, `YOUTUBE_DESC.md` (Session 37B; internal drafts, not shipped in public repo).
- `.github/ISSUE_TEMPLATE/bug_report.md` and `.github/ISSUE_TEMPLATE/build_help.md` — public-repo issue templates with solo-dev best-effort support framing (Session 37B).

### Changed (docs, Session 37B)
- `README.md` rewritten build-first for strangers: hero image + YouTube placeholders, BOM table with placeholder purchase links, 8" build as recommended path / 15" as showcase, flash + OTA quickstart, WiFi provisioning walkthrough, solo-dev support note, lineage/credits. Removed links to internal WORKFLOW.md/REVIEW.md.
- Internal workflow docs (not shipped in the public repo): residual real-username mention scrubbed to "the maintainer's dev PC" (completes the username scrub from the pre-publish provenance audit).

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
- NOTICE file with attribution to Steve Manley's original NeoPixel Ring Clock design
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
