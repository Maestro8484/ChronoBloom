# Focus Reminders Guide

## What is it?
A visual interrupt built into the clock. When enabled, the rings fire a short light animation at set intervals during your chosen hours. A nudge to look up: check in, stretch, hydrate, switch tasks. It's an ask, not an alarm. Some sessions you'll blow right past it, and that's fine. The clock asks, it doesn't demand.

**Good for:** redirecting hyperfocus, pomodoro-style breaks, homework reminders, bedtime cues, leaving-the-house alerts, screen break nudges.

## Quick Start

### Fast path: Quick controls
The **Focus nudges** row in the Quick controls at the top of the page turns nudges on or off and sets the interval. Two taps from landing. Use the full section below when you want the window, days, animation, or colors.

### 1. Open WebUI
- Desktop: `http://esp32c3-v3-8inch.local` (or device IP address)
- Expand the **Motion & nudges** card (it starts collapsed)
- Scroll to the **Focus reminders (nudges)** section

### 2. Configure
- **Enable focus reminders:** Check the box
- **Start hour:** When you want nudges to begin (e.g., 08 for 8 AM)
- **End hour:** When you want them to stop (e.g., 22 for 10 PM). Same hour in both fields means a full 24-hour window (v2.31.0). End earlier than start wraps past midnight (e.g., 22 to 06).
- **Interval (min):** How often in minutes (e.g., 60 for every hour)
- **Presets:** The Pomodoro 25m, Check-in 30m, and Hourly buttons in the reminder section fill the common setups in one click
- **Days of week:** Check which days to enable (Mon-Fri for work, all days for hobby)
- **Reminder animation:** Pick the visual style. Six dedicated gentle nudges (Gentle Pulse, Orbiting Orb, Ripple In, Heartbeat, Slow Bloom, Firefly), or "Use quarter/half-hour/hour animation" to reuse one of your configured clock animations
- **Colors:** The **Reminder colors (nudges)** picker sits under Animation style in the same card. Pick a different palette there so a nudge reads differently from a chime, then click **Save style**.

### 3. Save
- Click the **Save reminder** button (since v2.31.0 the main Save button writes these fields too; Save reminder stays as a convenience)
- Settings save to device memory (persist across power-off/reboot)
- The first nudge fires as soon as the current day and hour match your window, no full-interval wait. After that the interval counts from the last fire or acknowledgment, on device uptime, not wall-clock marks like :00 or :30. A reboot restarts the cadence.

### 4. Test
- Click the preview arrow next to the Reminder animation dropdown to fire the nudge on the device right now
- Or set the clock time to within your window on a selected day; the first nudge fires as soon as the window matches
- Check Serial monitor (115200 baud) for the log: `[FocusReminder] Fired at...`

## Examples

### Pomodoro-Style (Work)
- Enable: YES
- Start: 09 (9 AM)
- End: 17 (5 PM)
- Interval: 25 (every 25 min, Pomodoro timer)
- Days: Mon-Fri
- Animation: Gentle Pulse (soft, frequent-friendly)

### Bedtime Reminder (Daily)
- Enable: YES
- Start: 21 (9 PM)
- End: 22 (10 PM)
- Interval: 10 (every 10 min for 1 hour)
- Days: All
- Animation: Heartbeat (a noticeable two-thump)

### Screen Break (Background)
- Enable: YES
- Start: 08 (8 AM)
- End: 23 (11 PM)
- Interval: 120 (every 2 hours)
- Days: All
- Animation: Slow Bloom (pretty, unhurried)

## Animation Styles

| Name | Feel | Use Case |
|------|------|----------|
| **Gentle Pulse** | A slow breath with a warm hue drift | Frequent reminders (25-60 min) |
| **Orbiting Orb** | Quiet and localized: one orb on the inner ring | Background nudges you don't want to intrude |
| **Ripple In** | Rings swell inward in sequence | Moderate reminders (60-120 min) |
| **Heartbeat** | Two soft thumps | Important cues you shouldn't miss |
| **Slow Bloom** | An unhurried flower opening | Calm, meditative reminders |
| **Firefly** | An asynchronous warm twinkle swarm | Gentle but clear check-ins |

All six are capped at ~80% brightness so they read as a swell, never an alarm, and render in the reminder palette you select (Clock colors, Golden hour, Moonlight, Dawn, or Twilight). You can also pick "Use quarter/half-hour/hour animation" to fire one of your configured clock animations as the reminder instead.

## Known Limitations (v1)

- **Single reminder only:** Only one rule configured. Plan for multi-reminder in future update.
- **No quiet mode:** Reminder fires even during night/sleep hours inside your window (tighten the start/end window, or use the separate night-brightness schedule as a workaround).
- **Day filter needs NTP:** The day-of-week comes from the NTP-synced clock. If the clock has never synced (offline boot, time set by hand), the day filter is skipped and the reminder fires on any selected-hours day.
- **No snooze:** Once fired, interval resets. No way to delay the next fire.
- **Escalation is gentle and new (v2.31.0):** From the second unacknowledged nudge on, the same swell plays a second time about 6 seconds after the first. It asks twice; it never gets brighter. Press either clock button during a nudge, or within about 15 seconds after, to acknowledge: escalation clears and the interval restarts. That press does not adjust the time; the next one does, as usual. Before v2.31.0 every nudge was identical and played once. Still an ask, not an alarm.
- **Brief visual takeover:** While a reminder plays (~2-4.4s) the animation owns the display, but it's non-blocking, so Wi-Fi, the web UI, and time-keeping keep running throughout.

## Troubleshooting

### Animation doesn't fire at expected time
- **Check 1:** Is it enabled? (Motion & nudges card, "Enable focus reminders" checkbox)
- **Check 2:** Is the clock's current time within the start/end window? (the time readout at the top of the page)
- **Check 3:** Is today a selected day? (verify days-of-week checkboxes)
- **Check 4:** Has a full interval passed since the last fire or acknowledgment? (The first nudge after enabling fires as soon as the window matches.)
- **Check 5:** Open Serial monitor (115200 baud), watch for `[FocusReminder] Fired at...` log

### Settings not saving
- Refresh WebUI (Ctrl+F5)
- Check browser console for errors (F12)
- Verify the clock is connected to WiFi (the status line under the live clock preview reads "Wi-Fi on")
- Try **Save brightness** first (quick controls at the top) to verify saves work at all

### Clock time is wrong
- Sync via NTP: **Time & light** card, "Sync to internet" button
- Or set manually: **Time & light** card, enter the time, "Set time" button
- Reminders only fire when time is correct

## Advanced: Debug via Serial

Connect USB to ESP32-C3, open serial monitor (115200 baud):
```
[FocusReminder] Fired at 14:30 (interval=60 min)
```

Decode: Fired at 2:30 PM on a 60-minute interval. Use to verify timing.

## Future Enhancements (Roadmap)

- [ ] Multiple reminder rules (3-5 simultaneous)
- [ ] Quiet/sleep mode exemption
- [ ] Custom labels ("Lunch break", "Leave house", etc.)
- [x] Test-now button (the preview arrow next to the animation dropdown fires the reminder on the device)
- [x] Gentle escalation + button acknowledgment (v2.31.0)
- [ ] Soft audio alert (buzzer) option
- [ ] Snooze/delay next fire
- [ ] Integration with Home Assistant automations
