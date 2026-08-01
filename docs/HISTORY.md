# ChronoBloom ESP32-C3 -- Project History

## From Steve Manley's 2015 design to a decade of builds

---

## Steve Manley (2015-2016) -- where it started

### The original 3-ring NeoPixel clock

**Hardware**
- **Microcontroller**: Arduino Nano
- **Timekeeping**: DS3234 SPI RTC module with CR2032 backup battery
- **Display**: NeoPixel rings (60/24/12 LEDs)
- **Input**: Two push buttons (polled, no interrupts)
- **Enclosure**: 3D-printed base and face plates, wooden cabinet frame (lathe-turned)
- **Material**: UV glow-in-dark PLA filament (charges from the LEDs and ambient light)

**Software**
- **Architecture**: Render order (face -> seconds -> minutes -> hours) still used today
- **Timekeeping**: Hardware 1Hz interrupt from the DS3234 drives clock ticks
- **Features**: MSGEQ7 audio spectrum analyzer for music-reactive rainbow modes
- **Button handling**: Loop-wait polling (immune to EMI noise)

### Why this design mattered
1. **Concentric ring layout** -- time as expanding circles
2. **The light-shaping 3D-printed frame** -- the frame reflects and diffuses the LED
   glow into crisp, pointed hands, like a real clock. This is the detail that
   started everything downstream.
3. **UV glow-in-dark aesthetic** -- the frame glows softly in darkness after LED exposure
4. **Stable architecture** -- render order and button polling proven over years

Steve documented the build in a 2016 YouTube playlist and blog posts, and shared
the Arduino sketch (V4.0, 31 Dec 2015) and printable frame. The printed
**reflector** in this project, the part that casts and shapes the light, is a
remix of his design.

---

## Maestro8484 (2022-2026) -- a decade of builds

I was already comfortable with ESP32s and driving addressable LEDs -- that wasn't
the draw. What captivated me was Steve's **3D-printed frame** and the way it
shapes the light. I got a NeoPixel ring kit off Amazon and set out to build one.
What followed was about ten years of iterations.

### First edition -- Arduino Nano
Arduino Nano + RTC module + two hardware buttons. Setting the time through the
buttons was fiddly (interrupt handling made it awkward), but it worked and ran
for **2-3 years**.

### ESP8266 port
Ported the sketch to an ESP8266 (with help from a pre-Codex ChatGPT). No web UI
or WiFi features yet -- just a solid clock. It **ran well for years**.

### 8" build (the replicable one)
A cleanly replicable build, the version this repo targets, sized until the
off-the-shelf WS2812B rings seated precisely. The largest part measures 209 mm
square, about 8 inches, which is where the name comes from. It lives in the house. Around the same time I built a
custom **10" wall clock** using Steve's printed reflector with a glow-in-the-dark
frame and UV LEDs behind it; it ran for about a year.

### 15" build (the showcase)
Scaled the 8" design up to **~198%** so the outer 60-LED ring's pitch would
exactly match the spacing of side-lit WS2812B strip LEDs. This one took real
fabrication work:
- Side-lit WS2812B strip in a perpendicular track around the outer ring's
  perimeter; the same approach for the middle 24-LED ring; a WS2812B PCB ring
  (~80mm, 12 LEDs) from another project for the inner ring.
- Laser-cut **1/8" clear acrylic** face.
- A long diffuser hunt: 3D-printed diffusers in thirds (bed-size limited),
  thickness from 0.8mm down to 0.2mm -- all so-so. **Parchment paper won.**
- Heat-set M3 inserts + 6x M3x10 bolts; two momentary buttons retained for
  hardware time-set.
- Originally ran on an Arduino Nano + RTC.

### ChronoBloom -- ESP32-C3 rewrite (2026)
Mostly from scratch, with Claude Code, I refactored **both** clocks onto the
modern Seeed XIAO ESP32-C3.

**Hardware**
- Seeed XIAO ESP32-C3 (WiFi built in)
- VEML7700 ambient light sensor (I2C). An earlier ambient sensor didn't work
  out; the VEML7700 eventually did -- after sorting out **polling interference
  between the lux sensor reads and the WS2812B output** that was throttling the
  animations.

**Software**
- Migrated from Arduino IDE to PlatformIO; dual 8"/15" variants via compile-time config
- Full web UI with live preview; fully web-configurable with the two physical
  buttons kept as a **hardware fallback**
- NTP time sync with timezone + DST (no RTC chip)
- EEPROM settings persistence with a user-saved defaults slot
- Per-ring color/brightness, 8 palettes, 16 palette-aware animations
- VEML7700 auto-brightness + dark-room display sleep
- WiFi provisioning captive portal, mDNS, OTA firmware updates
- **Focus Reminders** -- gentle visual nudges to interrupt ADHD hyperfocus

---

## Technical evolution

| Aspect           | Steve Manley (2015) | ChronoBloom (current) |
|------------------|---------------------|------------------------|
| Microcontroller  | Arduino Nano        | XIAO ESP32-C3          |
| Timekeeping      | DS3234 RTC          | NTP (no RTC chip)      |
| Button input     | Polled              | Polled GPIO5/9*        |
| WiFi             | None                | Built-in               |
| Web UI           | None                | Full-featured          |
| Sensors          | MSGEQ7 audio        | VEML7700 lux           |
| Settings storage | Hardcoded           | EEPROM (versioned)     |
| Animations       | Audio-reactive      | 16 palette-aware       |
| Platform         | Arduino IDE         | PlatformIO             |

\* *An earlier ESP32-C3 revision tried interrupt-driven buttons on GPIO3/4, which
are JTAG-strapped pins and fired spuriously -- reverted to polled GPIO5/9, the same
reliable polled approach Steve used from the start.*

---

## What stayed, what changed

**Stayed**
- The 3-ring analog metaphor: position = time, color = information
- Steve's render order: face -> seconds -> minutes -> hours
- The light-shaping printed frame, remixed at multiple scales
- Build-it-yourself, open-source ethos

**Changed**
- Standalone -> connected (WiFi, NTP)
- Static -> adaptive (lux sensor auto-brightness)
- Simple -> smart (web UI, animations, persistent settings)
- Single-scale -> parametric (8" and 15" from one codebase)

**Learned along the way**
1. Button polling beats ISRs here -- the interrupt-driven button revision fired
   spuriously; polling (Steve's original choice) is immune to the noise.
2. Parchment paper is the best diffuser I found -- better than frosted acrylic or
   thin printed panels.
3. UV glow-in-dark PLA enhances the bloom in darkness.
4. On the ESP32-C3, the lux sensor and the WS2812B output must not fight over
   timing -- that interference was the hardest bug of the rewrite.

---

## Attribution

**Steve Manley** -- for the original vision and the design that started all of
this. The render order, the UV-glow aesthetic, and above all the light-shaping
3D-printed frame are his. The printed reflector here is a remix of his part. Original
Arduino sketch (V4.0, 31 Dec 2015) shared under the MIT License; see [NOTICE](../NOTICE).

**Maestro8484** -- a decade of builds (Nano -> ESP8266 -> 8"/10"/15" frames -> the
ESP32-C3 rewrite): WiFi, NTP, web UI, sensors, OTA, and Focus Reminders.

**Claude (Anthropic)** -- Claude Code drove much of the 2026 ESP32-C3 refactor
(implementation, builds, debugging).

---

## What's next

**Ideas** (not commitments): holiday date-based animations, a BME280 temp sensor
on the free I2C bus, Home Assistant / MQTT, a sunrise-fade alarm, theme
preset save/load, multi-clock sync.

**Never**: games, text scrolling, pixel art -- **this stays a clock.**
