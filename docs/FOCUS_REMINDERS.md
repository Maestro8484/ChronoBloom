# Focus Reminders: ADHD Hyperfocus Killer Guide

## What is it?
A visual interrupt system built into your NeoPixel ring clock. When enabled, the clock fires attention-grabbing light animations at regular intervals during your work hours—a gentle nudge to check in, stretch, hydrate, or switch tasks.

**Designed for:** ADHD hyperfocus, pomodoro-style breaks, homework reminders, bedtime cues, leaving-the-house alerts, screen break nudges.

## Quick Start

### 1. Open WebUI
- Desktop: `http://esp32c3-v3-8inch.local` (or device IP address)
- Find the **"Focus Reminders (ADHD)"** panel

### 2. Configure
- **Enable:** Check the box
- **Start hour:** When you want reminders to begin (e.g., 08 for 8 AM)
- **End hour:** When you want them to stop (e.g., 22 for 10 PM)
- **Interval:** How often in minutes (e.g., 60 for every hour)
- **Days:** Check which days of the week to enable (Mon-Fri for work, all days for hobby)
- **Animation:** Pick the visual style (Quarter Pulse, Rainbow Sweep, Hour Chime)

### 3. Save
- Click **"Save reminder"** button
- Settings save to device memory (persist across power-off/reboot)

### 4. Test
- Manually set clock time to within your window + matching day
- Wait for next interval
- Clock will flash the animation you selected
- Check Serial monitor (115200 baud) for log: `[FocusReminder] Fired at...`

## Examples

### Pomodoro-Style (Work)
- Enable: YES
- Start: 09 (9 AM)
- End: 17 (5 PM)
- Interval: 25 (every 25 min, Pomodoro timer)
- Days: Mon-Fri
- Animation: Quarter Pulse (gentle)

### Bedtime Reminder (Daily)
- Enable: YES
- Start: 21 (9 PM)
- End: 22 (10 PM)
- Interval: 10 (every 10 min for 1 hour)
- Days: All
- Animation: Hour Chime (noticeable)

### Screen Break (Background)
- Enable: YES
- Start: 08 (8 AM)
- End: 23 (11 PM)
- Interval: 120 (every 2 hours)
- Days: All
- Animation: Rainbow Sweep (pretty)

## Animation Styles

| Name | Feel | Use Case |
|------|------|----------|
| **Quarter Pulse** | Gentle, subtle | Frequent reminders (25-60 min) |
| **Half-Hour Sweep** | Flowing, gradual | Moderate reminders (60-120 min) |
| **Hour Chime** | Dramatic, attention-grabbing | Important cues (break, bedtime, leave) |

## Known Limitations (v1)

- **Single reminder only:** Only one rule configured. Plan for multi-reminder in future update.
- **No quiet mode:** Reminder fires even during night/sleep (use separate night-brightness schedule as workaround).
- **Day-of-week auto-detection:** Currently must set to "all days" or manually verify clock day is correct.
- **No snooze:** Once fired, interval resets. No way to delay next fire.
- **Animation blocks briefly:** Animation uses `delay()` internally; clock display briefly pauses during fire (< 5 seconds).

## Troubleshooting

### Animation doesn't fire at expected time
- **Check 1:** Is reminder enabled? (toggle visible on WebUI)
- **Check 2:** Is current clock time within start/end window? (check Time panel)
- **Check 3:** Is today a selected day? (verify days-of-week checkboxes)
- **Check 4:** Has enough time passed since last fire? (interval in minutes)
- **Check 5:** Open Serial monitor (115200 baud), watch for `[FocusReminder] Fired at...` log

### Settings not saving
- Refresh WebUI (Ctrl+F5)
- Check browser console for errors (F12)
- Verify clock is connected to WiFi (check Status line at bottom)
- Try "Save display" first (other settings panel) to verify POST works

### Clock time is wrong
- Sync via NTP: Time panel → "NTP sync" button
- Or set manually: Time panel → set hour/minute/second → "Set" button
- Reminders only fire when time is correct

## Advanced: Debug via Serial

Connect USB to ESP32-C3, open serial monitor (115200 baud):
```
[FocusReminder] Fired at 14:30 (interval=60 min, dow=2)
```

Decode: Fired at 2:30 PM, 60-min interval, Wednesday (day 2). Use to verify timing.

## Future Enhancements (Roadmap)

- [ ] Multiple reminder rules (3-5 simultaneous)
- [ ] Quiet/sleep mode exemption
- [ ] Custom labels ("Lunch break", "Leave house", etc.)
- [ ] Test-now button
- [ ] Soft audio alert (buzzer) option
- [ ] Snooze/delay next fire
- [ ] Integration with Home Assistant automations
