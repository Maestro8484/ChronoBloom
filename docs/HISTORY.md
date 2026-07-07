# ChronoBloom ESP32-C3 — Project History

## Lineage: Steve Manley → Mike van der Sluis → Maestro8484

---

## Steve Manley (2015-2016)

### Original Vision
**Creator of the 3-ring NeoPixel clock concept**

### Hardware
- **Microcontroller**: Arduino Nano
- **Timekeeping**: DS3234 SPI RTC module with CR2032 backup battery
- **Display**: Adafruit NeoPixel rings (60/24/12 LEDs)
- **Input**: Two push buttons on GPIO7/8 (polled, no interrupts)
- **Enclosure**: 3D printed base and face plates (Google SketchUp), wooden cabinet frame (lathe-turned)
- **Material**: UV glow-in-dark PLA filament (charges from LEDs and ambient light)

### Software
- **Architecture**: Proven render order (face → seconds → minutes → hours) still used today
- **Timekeeping**: Hardware 1Hz interrupt from DS3234 drives clock ticks
- **Features**: MSGEQ7 audio spectrum analyzer integration for music-reactive rainbow modes
- **Button handling**: Loop-wait polling (immune to EMI noise)

### Key Contributions
1. **Concentric ring layout** — Visual metaphor of time as expanding circles
2. **UV glow-in-dark aesthetic** — Frame glows softly in darkness after LED exposure
3. **Stable architecture** — Render order and button polling patterns proven over years
4. **Light bleed mitigation** — 3D print settings and divider designs to prevent LED cross-talk

### Documentation
- YouTube playlist (2016): Build process, lathe-turned wooden cabinet
- Blog posts on Embedded.com: Design decisions and electrical specs
- Collaboration with Max Maxfield (EE Times): Technical discussions on NeoPixel wiring strategies

---

## Mike van der Sluis (2020-2021)

### Cost-Optimization Focus
**Goal**: Make Steve's design accessible with cheaper components

### Hardware Changes
- **NeoPixel rings**: Chinese clones from AliExpress (not Adafruit originals)
- **Microcontroller**: Arduino Nano clone with CH340G USB-serial chip
- **Same RTC**: DS3234 module (kept for reliability)

### Challenges Solved
1. **Dimensional mismatch**: Clone rings had different diameters than Adafruit
2. **STL modifications**: Redesigned 3D printed parts to fit clone ring dimensions
3. **Driver compatibility**: Windows CH340G driver installation guide

### Software Simplification
- **Removed**: MSGEQ7 audio reactive features
- **Kept**: Core clock functionality (time display, buttons, RTC)
- **Result**: Simpler firmware focused purely on timekeeping

### Documentation
- **Instructable** (July 2020): "Neopixel Clock With Three Neopixel Rings"
- **Audience**: Beginners (detailed Arduino IDE setup, library installation)
- **Philosophy**: "An important Dutch habit is always trying to save money ;-)"

### Key Contributions
1. **Hardware accessibility** — Proved the design works with budget components
2. **Modified STL files** — Community can use wider variety of NeoPixel rings
3. **Beginner-friendly docs** — Lowered barrier to entry for new builders

---

## Maestro8484 (2022-2026)

### Evolution Journey
**Two physical builds** → **Four-year production deployment** → **ESP32-C3 modernization**

### Phase 1: First Build (2022) — 8.5" Clock
- **Scale**: 85% (0.85x) to maximize Bambu Labs P1S print bed (256mm x 256mm)
- **Microcontroller**: Started with Arduino Nano (following Mike's design)
- **Brief ESP8266 experiment**: Abandoned due to RAM/flash constraints
- **Material innovation**: UV glow-in-dark PLA (following Steve's aesthetic)
- **Diffuser discovery**: Parchment paper creates optimal light bloom effect

### Phase 2: Large Build (2022-2023) — 15" Wall Clock
- **Scale**: 200% (2x) requiring part segmentation
- **Fabrication**: Hybrid approach (FDM 3D printing + CO2 laser cutting)
- **Laser bed**: 600mm x 410mm allowed single-piece acrylic face
- **Assembly**: Parametric thirds design with alignment pins
- **Deployment**: Wall-mounted in home, ran continuously for 4 years

### Phase 3: ChronoBloom / ESP32-C3 Modernization (March–May 2026)
**Trigger**: Claude Code availability enabled ambitious refactor

#### Hardware Upgrade
- **Microcontroller**: Seeed Studio XIAO ESP32-C3
- **Sensor addition**: VEML7700 ambient light sensor (I2C)
- **Future-proofing**: I2C bus ready for BME280/SHT31
- **Pin conflict discovery**: GPIO3/4 JTAG issue identified during development

#### Software Rewrite
- **Platform**: Migrated from Arduino IDE to PlatformIO
- **Architecture**: Single-file monolith (main.cpp, ~2000 lines)
- **Build system**: Dual variants (8" and 15") with compile-time configuration
- **Features added (v1.x)**:
  - Full web UI with live SVG preview
  - NTP timezone sync with DST support (Mountain time)
  - EEPROM settings persistence (v7)
  - Per-ring RGB color customization
  - VEML7700 auto-brightness (3 modes)
  - Time-interval animations (quarter/half/hour escalation)
  - Browser time sync, mDNS hostname
  - WiFi provisioning portal (captive portal, AP fallback)
  - OTA firmware updates (ArduinoOTA, port 3232)
- **Features added (v2.0)**:
  - Focus Reminders — ADHD hyperfocus interrupt system (EEPROM v8)
  - Day-of-week scheduling via NTP weekday
- **Features added (v2.0.2)**:
  - Sacrificial pixel removed (hardware rework + firmware fix)
  - mDNS reconnect handler
  - WiFi AP fallback when STA unavailable

#### Collaboration Period (March-May 2026)
- **Claude Chat**: Planning, architecture, feature design
- **Claude Code**: Implementation, builds, flashing, debugging
- **Documentation**: Workflow rules established, REVIEW.md technical audit
- **Token efficiency**: Snapshot system to minimize context re-processing

### Key Contributions
1. **Multi-scale parametric design** — Proven at 85% and 200% scales
2. **Hybrid fabrication workflow** — Combined FDM + laser cutting advantages
3. **Parchment paper diffuser** — Optimal light distribution discovery
4. **ESP32-C3 smart features** — Web UI, NTP, sensors, animations, OTA, Focus Reminders
5. **Dual build variants** — Compile-time configuration for 8" vs 15"
6. **4-year production validation** — Real-world reliability testing
7. **Comprehensive documentation** — WORKFLOW.md, FEATURES.md, ANIMATIONS.md, API.md
8. **Public rebranding** — Project renamed ChronoBloom ESP32-C3 (May 2026)

---

## Technical Evolution Summary

| Aspect              | Steve (2015)        | Mike (2020)         | ChronoBloom (current) |
|---------------------|---------------------|---------------------|------------------------|
| Microcontroller     | Arduino Nano        | Nano clone (CH340G) | XIAO ESP32-C3          |
| Timekeeping         | DS3234 RTC          | DS3234 RTC          | NTP (no RTC chip)      |
| Button input        | Polled GPIO7/8      | Polled GPIO7/8      | Polled GPIO5/9*        |
| WiFi                | None                | None                | Built-in               |
| Web UI              | None                | None                | Full-featured          |
| Sensors             | MSGEQ7 audio        | None                | VEML7700 lux           |
| Settings storage    | Hardcoded           | Hardcoded           | EEPROM (versioned)     |
| Animations          | Audio-reactive      | Basic chime         | 16 palette-aware       |
| Platform            | Arduino IDE         | Arduino IDE         | PlatformIO             |
| Power               | USB / barrel jack   | USB / barrel jack   | USB-C / VIN pin        |

\* *An earlier ESP32-C3 revision tried interrupt-driven buttons on GPIO3/4, which are JTAG-strapped pins and fired spuriously — reverted to polled GPIO5/9, the same reliable approach Steve used from the start.*

---

## Philosophy Continuity

### What Stayed the Same
- **3-ring analog metaphor** — Position = time, color = information
- **UV glow-in-dark aesthetic** — Frame charges from LEDs, glows in darkness
- **Proven render order** — Face → seconds → minutes → hours (Steve's original)
- **Build-it-yourself ethos** — Open source, community-improvable

### What Changed
- **From standalone to connected** — WiFi enables remote control, NTP sync
- **From static to adaptive** — Sensors enable auto-brightness, environmental response
- **From simple to smart** — Web UI, animations, persistent settings
- **From single-scale to parametric** — Multiple size variants from same codebase

### What Was Learned
1. **Hardware RTC is reliable** — Steve's DS3234 choice was correct (battery backup invaluable)
2. **Button polling > ISRs** — Steve's polled approach immune to GPIO noise (Maestro8484's ISR regression)
3. **Cheap clones work** — Mike proved cost reduction doesn't sacrifice functionality
4. **UV PLA is magical** — Glow-in-dark effect enhances the light bloom aesthetic
5. **Parchment paper wins** — Better than frosted acrylic or spray-painted glass
6. **ESP32 is overkill (in a good way)** — WiFi + sensors + OTA + future expansion justify complexity

---

## Community Impact

### Derivative Builds
Multiple makers have built variants inspired by Steve's original:
- Hagyma's Thingiverse remix (2017): Modified for Chinese LED rings
- Peter Neufeld's kinetic variant (2020): Added rotating outer ring with magnets
- Various WordClock projects adopted the concentric ring aesthetic

### Educational Value
- Arduino beginners learn: NeoPixel control, RTC interfacing, button debouncing
- Intermediate builders learn: 3D design iteration, laser cutting, hybrid fabrication
- Advanced users learn: ESP32 web servers, EEPROM management, I2C sensors

---

## Thanks & Attribution

**Steve Manley**: For creating the original vision and proving the concept. The render order, UV glow aesthetic, and careful light management are his legacy.

**Mike van der Sluis**: For making it accessible to budget-conscious makers. The modified STLs and beginner-friendly documentation expanded the builder community.

**Maestro8484**: For modernizing the platform without losing the soul. Web UI, sensors, OTA, and Focus Reminders enhance but don't replace the core analog timekeeping experience. Public rebranding to ChronoBloom ESP32-C3 completed May 2026.

**Claude AI (Anthropic)**: For enabling the March-May 2026 refactor through Claude Chat (planning) and Claude Code (implementation). The collaboration workflow proved essential for managing complexity while maintaining code quality.

---

## What's Next?

### Immediate
- Holiday animations (auto-triggered date-based effects)

*(OTA firmware updates and the WiFi provisioning portal, both listed here as upcoming in earlier drafts of this page, have since shipped — see README.md.)*

### Medium-term (2026-2027)
- BME280 temperature sensor integration
- Home Assistant MQTT support
- Sunrise alarm with 5-minute fade
- Theme preset save/load

### Long-term Vision
- Multi-clock network sync (master/slave animations)
- Community animation library (share custom effects)
- Mobile app for advanced configuration
- Solar-powered variant with battery backup

### Never
- Games, text scrolling, pixel art — **this stays a clock**

---

**ChronoBloom ESP32-C3 is now 10+ years old as a concept (2015-2026) and stronger than ever. Each iteration has honored the original vision while adapting to modern capabilities.**
