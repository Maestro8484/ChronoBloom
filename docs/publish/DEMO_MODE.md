# ChronoBloom — Demo Mode Specification

## Purpose
Demo Mode is a non-blocking firmware state machine that sequences through clock
features automatically for video recording. A browser overlay at /demo/overlay
renders live subtitles synced to firmware step state. OBS composites the overlay
over physical clock footage.

## Architecture

### Firmware (DemoMode class)
- Array-driven step table: { duration_ms, subtitle }
- millis() timing, non-blocking
- Runs alongside normal clock operation
- Endpoints: POST /demo/start, POST /demo/stop
- State endpoint: GET /demo/status

### /demo/status response (JSON)
```json
{
  "active": true,
  "step": 2,
  "subtitle": "Focus reminder — ADHD hyperfocus interrupt",
  "elapsed_ms": 4200,
  "step_duration_ms": 18000
}
```

### /demo/overlay
- Full-screen HTML page, no CDN dependencies
- OBS browser source, 1920x1080
- White text, semi-transparent black pill background
- Bottom third position, centered
- Font: system sans-serif, 48px
- Fade: 300ms fade-in on step start, 300ms fade-out on step end
- Polls /demo/status every 500ms

### LuxSensor override (Step 6)
- LuxSensor::setLuxOverride(float lux) — bypasses hardware read
- LuxSensor::clearLuxOverride() — restores hardware read
- DemoMode calls setLuxOverride() to simulate room darkening

### OBS-compatible timestamped captions
`docs/publish/DEMO_CAPTIONS.srt` mirrors the step table below as a standard
SRT file, timestamped to the sequence's firmware timing. Use it as a backup
caption track (VLC source, or burn in during editing) if the live
`/demo/overlay` browser source isn't used for a given take.

## Demo Sequence

Steps 1/2/3/5 are event-paced, not fixed-timer: `DemoMode` waits for
`ClockRenderer::animating() == false` (the current animation reached its own
natural end) before triggering the next one in the sequence, chaining
immediately with no artificial gap. Each animation already fades to
near-black in its own last frames, so the hard cut reads as a clean beat.
An earlier version fired the next animation on a fixed 3s timer regardless of
how long the current one actually took (cutting 5-9s animations off
mid-flight), and a later attempt at fixing that added an explicit pause
between beats — which backfired by giving the main loop's idle-render path a
window to flash the live clock face between every animation. Durations below
are the resulting real-world totals at the clock's configured
`animationSpeed`, not fixed waits.

The sequence follows a "less is more" design (v2.7.1, showcase/reminder sets
widened in v2.8.0): steps 1-3 fire the clock's *configured* chime triggers —
not a hardcoded phase list — twice each, so the demo always mirrors what the
unit actually does at :15/:30/:00 in its own palette, speed, brightness, and
trail settings. The showcase step covers all four big all-ring hour
animations, and the reminder step plays through the full gentled-nudge set
once each — no longer just the two ends of the spectrum — to demonstrate that
every nudge now reads as a soft swell, not an alert flash (v2.8.0 feel pass).
If a chime slot's configured mode is 0 (off), its trigger no-ops and the
step falls through without stalling.

| Step | Duration | Subtitle | Firmware action |
|------|----------|----------|-----------------|
| 0 | 8s | ChronoBloom — ESP32-C3 NeoPixel clock | Idle clock, seconds trail visible |
| 1 | ~4.6s | Quarter-hour chime — twice, exactly as configured | `triggerQuarterAnimation()` ×2, each played to completion |
| 2 | ~10s | Half-hour chime — twice, exactly as configured | `triggerHalfHourAnimation()` ×2, each played to completion |
| 3 | ~16s | Top-of-hour chime — twice, exactly as configured | `triggerHourAnimation()` ×2, each played to completion |
| 4 | ~34s | Hour animation showcase — Ceremony, Galaxy Spin, Supernova, Comet Relay | ANIM_HR1, HR2, HR3, HR4 once each, full duration |
| 5 | ~12.5s | Focus reminder nudges — gentle, subtle, never a flash | Reminder modes 6, 8, 9, 10 once each (Gentle Pulse, Ripple In, Heartbeat, Slow Bloom) |
| 6 | 14s | Auto-brightness — VEML7700 sensor, now 3-4x faster response | Two quick dim/bright lux-override cycles (1.5s ramps) |
| 7 | 10s | Open source — github / printables / hackaday.io | Idle clock, end card subtitle |

Total: ~110 seconds (1:50) at animationSpeed 3 with the maintainer's
configuration (Bloom Ripple / Unfurl / Comet Relay). Step 1-5 durations are
event-paced estimates, not fixed waits — each animation runs to its natural
end before the next is triggered.

History: the original sequence raced through every animation style on a
fixed 3s timer, cutting 5-9s animations off mid-flight, and its
palette-rotation step called `SettingsStore::update()` every 2s — writing
EEPROM 8 times per run and permanently overwriting the saved
`animationPalette` without restoring it, while never triggering a visible
animation. The v2.5.4 fix added event pacing but with a 450ms gap that let
the idle clock face flash between beats; v2.6.0 removed the gap; v2.7.1
replaced the everything-catalog with the configured-chimes design above and
dropped the palette step entirely.

v2.7.2 found (via frame-by-frame analysis of a recorded demo — see Session
42 in SESSIONS.md) that three animations had a "dead tail": their coded
`dur` ran hundreds of milliseconds to a full second past when their visual
content actually faded to black, so the immediate-chain sequencer (v2.6.0)
was holding on pure black for that whole stretch before the next animation
popped on — reading as a flash/black-gap/flash stutter rather than a loop.
Bloom Ripple (`animQ3`, dur 2500→2300), Ripple In (`animRem3`, dur
3500→2500), and Heartbeat (`animRem4`, dur 3000→2100) were trimmed to end
exactly when their last visible pixel goes dark. Heartbeat was the worst
offender — played twice in step 5, ~1.9s of its ~10.2s runtime was dead
black before this fix.

v2.8.0 (animation feel pass) widened both the showcase and reminder steps
alongside the visual rework they were built to demonstrate: step 4 gained a
fourth animation (Comet Relay, now paired with its rainbow-spiral rework) and
step 5 replaced "two ends of the spectrum" (Gentle Pulse ×2 + Heartbeat ×2)
with one pass through all four nudges used in the reminder scheduler (Gentle
Pulse, Ripple In, Heartbeat, Slow Bloom) so the demo actually shows the
`NUDGE_CEIL` brightness cap and eased envelopes working across the whole set,
not just the one animation (Heartbeat) that used to carry the point alone.
Total runtime grew from ~99s to ~110s as a direct result.

### Why Step 6 changed
The lux-derived brightness target used to refresh every 500ms; it now
refreshes every 150ms (~3-4x faster), so large lux swings start moving
toward the new target almost immediately instead of lagging for a
noticeable beat. The old demo used one slow 25s ramp-down/hold/ramp-up,
which was tuned to show off a sensor that took time to react. That's no
longer representative — two fast 1.5s dim/bright cycles now demonstrate
the snappier response instead of masking it.

## Button Behavior During Demo
Buttons ignored. Normal web endpoints continue to function.

## EEPROM / Settings
No changes. No SETTINGS_VERSION bump. No new build flags.

## OBS Setup
Scene 1 (main): camera input + browser source at http://esp32c3-v3-8inch.local/demo/overlay
Scene 2 (web UI clip): browser source at http://esp32c3-v3-8inch.local/ fullscreen
Post: DaVinci Resolve free — trim, title card, end URL card only.

## Recording Notes
- Subtitles captured live from overlay during recording, no post-production subtitle work
- Web UI and button interaction filmed as separate supplemental clips
- Demo triggered via POST /demo/start from browser or curl before hitting record
