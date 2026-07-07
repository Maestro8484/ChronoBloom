---
name: Bug report
about: Something in the firmware or web UI is broken
title: "[BUG] "
labels: bug
assignees: ''
---

**Heads up:** this is a one-person hobby project. Support is best effort. Complete reports get fixed a lot faster than "it doesn't work."

## What happened

<!-- What did you see? What did you expect instead? -->

## Firmware version

<!-- Shown on the web UI footer and in the serial boot banner, e.g. 2.5.2 -->

## Build variant

- [ ] 8" (`esp32c3_v3_8inch`)
- [ ] 15" (`esp32c3_v3_15inch`)
- [ ] Custom (describe below)

## Steps to reproduce

1.
2.
3.

## Serial output

<!-- Connect USB, run `pio device monitor` (115200 baud), paste relevant lines.
     The boot banner plus anything printed when the bug happens is gold. -->

```
paste here
```

## Environment

- How flashed: USB / OTA
- Sensor connected: VEML7700 yes/no
- Anything nonstandard in your build (different pins, LED counts, PSU):
