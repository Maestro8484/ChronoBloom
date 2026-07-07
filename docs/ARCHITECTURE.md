# Architecture: Focus Reminder System (v2.0+)

## Overview
The Focus Reminder system is a non-blocking scheduler that fires attention-grabbing NeoPixel animations at user-configured intervals during active hours on selected days. Designed for ADHD hyperfocus interruption and task switching cues.

## Components

### FocusReminderScheduler Class
- **Location:** `src/main.cpp` (new in v2.0)
- **Scope:** Application-level scheduler, independent of WebUI
- **Loop cycle:** Called once per main loop iteration (every ~1ms)
- **State stored in:** `ClockSettings.focusReminder_*` fields (EEPROM-backed)

### Scheduling Logic
```
checkAndFire(now) {
  1. Check master enable flag
  2. Get current day-of-week (TODO: from NTP sync)
  3. Validate day matches bitmask
  4. Validate current hour in window [startHour, endHour)
  5. Validate interval elapsed: (now - lastFireMs) >= (intervalMinutes * 60000ms)
  6. If all pass: trigger animation + update lastFireMs
}
```

### Animation Reuse
Reminders invoke existing `ClockRenderer` methods:
- `triggerQuarterAnimation(now)` — Sparkle burst or pulse
- `triggerHalfHourAnimation(now)` — Rainbow sweep or flash
- `triggerHourAnimation(now)` — Chime, firework, spiral, or mandala

No new animations added in v1; all are blocking `delay()`-based. TODO: async animation queue in v2.1.

### EEPROM Schema
```cpp
struct ClockSettings {
  // ... existing 180 bytes ...
  
  // Focus Reminders (16 bytes total)
  uint8_t focusReminder_enabled;          // 1 byte: 0=off, 1=on
  uint8_t focusReminder_startHour;        // 1 byte: 0-23
  uint8_t focusReminder_endHour;          // 1 byte: 0-23
  uint16_t focusReminder_intervalMinutes; // 2 bytes: 1-1440
  uint8_t focusReminder_daysMask;         // 1 byte: bitmask Sun(0)-Sat(6)
  uint8_t focusReminder_animation;        // 1 byte: 0-5 (animation enum)
  uint8_t focusReminder_durationSeconds;  // 1 byte: reserved for v2 (currently unused)
  uint32_t focusReminder_lastFireMs;      // 4 bytes: millis() timestamp of last fire
  // 3 bytes reserved for future expansion
};
```

**Validation:** `SettingsStore::valid()` checks all fields are in legal range.
**Defaults:** Reminder disabled, 08:00-22:00 window, 60-min interval, all days, Quarter Pulse animation.
**Persistence:** Auto-saved to EEPROM on WebUI "Save reminder" button.

### WebUI Integration
- **Panel:** "Focus Reminders (ADHD)" (new section in main settings page)
- **Controls:**
  - Enable/disable toggle
  - Start/end hour (number inputs, 0-23)
  - Interval (number input, 1-1440 minutes)
  - Days-of-week selector (7 checkboxes, Sun-Sat, stored as bitmask)
  - Animation dropdown (6 options: reuse existing animations)
  - Save button (POSTs to `/settings` endpoint)
- **Real-time sync:** Reminder config loaded at page load, updates reflected after save

### Time Validation
- Window is half-open: `[startHour, endHour)` (e.g., 08:00-22:00 fires 08:00-21:59)
- Wrapped windows supported: if `startHour > endHour`, window crosses midnight (e.g., 22:00-08:00)
- Day-of-week: 0=Sunday, 6=Saturday (bitmask, e.g., 0x7F = all days)

### Interval Timing
- Last fire timestamp stored as `millis()` at time of animation trigger
- Interval calculated as: `uint32_t(intervalMinutes) * 60000UL` milliseconds
- Clock wraps at ~49 days; interval logic handles correctly via unsigned arithmetic
- Fire occurs when: `(now - lastFireMs) >= intervalMs`

### Serial Logging
Each fire logs:
```
[FocusReminder] Fired at HH:MM (interval=N min, dow=D)
```
Enable via Serial monitor at 115200 baud.

### No Blocking on Reminder Fire
- Scheduler call is non-blocking (< 1ms)
- Animation trigger calls `renderer_.triggerQuarterAnimation(now)` etc.
- Those methods ARE internally blocking (use `delay()`)
- This is acceptable for MVP but should be queued in v2.1

### Day-of-Week Calculation (TODO)
Current: hardcoded to Sunday (0).
Planned: compute from NTP-synced system `struct tm` after `configTzTime()` + `localtime_r()`.

---

## Testing Checklist

- [ ] Firmware compiles (pio build)
- [ ] Device boots, WebUI loads, new "Focus Reminders (ADHD)" panel visible
- [ ] Enable toggle works (setting persists after reboot)
- [ ] Start/end hour inputs work (0-23 range enforced)
- [ ] Interval input works (1-1440 range, clamped)
- [ ] Days-of-week toggles set bitmask correctly
- [ ] Animation dropdown selects animation type
- [ ] Save button POSTs to `/settings` and triggers reload
- [ ] Disable reminder: animation does not fire (confirmed via console log)
- [ ] Set time to within window + matching day, wait interval: animation fires
- [ ] Interval fires multiple times: every 60 min (if set to 60)
- [ ] Outside time window: no fire
- [ ] Non-matching day: no fire (once day calc is fixed)
