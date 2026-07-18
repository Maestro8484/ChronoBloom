# ChronoBloom

A 3-ring NeoPixel wall clock on the Seeed XIAO ESP32-C3. Seconds sweep the outer 60-LED ring, minutes on the 24, hours on the 12. Web UI, NTP sync, OTA updates, ambient-light auto dimming, and Focus Reminder nudges for ADHD hyperfocus. The diffused rings bloom like a flower, hence the name.

<!-- TODO: hero image coming soon. Finished 8" clock, lights on, slightly dark room. -->
<!-- ![ChronoBloom 8-inch build](docs/images/hero.jpg) -->

<!-- TODO: demo video coming soon. Insert YouTube video ID after upload. -->
<!--
[![ChronoBloom Demo](https://img.youtube.com/vi/VIDEO_ID/maxresdefault.jpg)](https://www.youtube.com/watch?v=VIDEO_ID)
-->

## What it does

- Time from NTP over WiFi, with timezone and DST rules. No RTC chip.
- Web UI on your LAN for colors, brightness, animations, and per-ring settings
- Per-ring color and intensity controls, theme presets, and warm-to-cool mood palettes for animations
- Quarter-hour, half-hour, and top-of-hour chime animations (3/3/5 styles)
- VEML7700 lux sensor drives auto-brightness. Pitch black room, LEDs sleep. Light returns, clock wakes.
- Focus Reminders: visual nudge animations at intervals you pick, built for interrupting hyperfocus
- WiFi setup in the browser at flash time, or via captive portal on first boot. No hardcoding credentials.
- OTA firmware updates from a browser after the first USB flash
- Settings persist in EEPROM, with a user-saved defaults slot

## Bill of materials

Prices swing. Shop around.

| Part | Qty | Notes | Where |
|---|---|---|---|
| Seeed XIAO ESP32-C3 | 1 | The brain. USB-C, WiFi, tiny. | [Amazon](#) <!-- TODO: link --> / [eBay](#) <!-- TODO: link --> |
| WS2812B 241-LED ring set | 1 | Sold as a 9-ring kit. You use the 60, 24, and 12 rings plus 1 loose pixel for the center. Rest goes in the parts bin. | [Amazon](#) <!-- TODO: link --> / [eBay](#) <!-- TODO: link --> |
| VEML7700 lux sensor module | 1 | Optional but worth it. Auto-brightness and dark-room sleep. | [Amazon](#) <!-- TODO: link --> / [eBay](#) <!-- TODO: link --> |
| Momentary push buttons | 2 | Time up/down, factory reset combo | [Amazon](#) <!-- TODO: link --> / [eBay](#) <!-- TODO: link --> |
| 5V power supply, 3A | 1 | 2A works, 3A gives headroom for bright animations | [Amazon](#) <!-- TODO: link --> / [eBay](#) <!-- TODO: link --> |
| 300Ω resistor | 1 | Inline on the LED data line | [Amazon](#) <!-- TODO: link --> / [eBay](#) <!-- TODO: link --> |

Plus a 3D printer for the frame, parchment paper for the diffuser, wire, and hot glue or cable ties.

## Two builds

### 8" build (start here)

The recommended path. 85% scale, fits a 256mm print bed (Bambu P1S class). Single NeoPixel chain of 97 LEDs: the three rings, then the center pixel. Parchment paper diffuser. Desktop kickstand or wall hang. Weighs about 500g.

Build environment: `esp32c3_v3_8inch`

Historical footnote: the firmware supports an optional "sacrificial" extra first pixel that stays dark and re-drives the data line at 5V logic. The original prototype has one spliced in, left over from troubleshooting. It turned out to be unnecessary and ships disabled. You will almost certainly never need it. See the `[led_chain]` note in `platformio.ini` if you are curious.

### 15" build (showcase)

200% scale. Frame prints in thirds with alignment pins. The face diffuser needs one of:

- Laser-cut acrylic (600mm x 410mm bed) with parchment paper behind it
- 0.5mm white PLA face printed on an XXL-bed printer

Separate center pixel strip on GPIO20. This is the wall piece. Build the 8" first, then decide if you want to go big.

Build environment: `esp32c3_v3_15inch`

### 3D print files

STL/3MF files for both builds: [docs/publish/ChronoBloom_3D_Files/](docs/publish/ChronoBloom_3D_Files/). CC BY 4.0, credit Steve Manley (see [NOTICE](NOTICE)).

<!-- TODO: ring OD dimensions (outer 60-LED ~175mm, middle 24-LED ~88mm, inner 12-LED ~50mm) are not yet caliper-verified against a printed part. Verify before treating these as exact. -->

## Wiring

```
XIAO Pin    GPIO    Function                Notes
─────────────────────────────────────────────────────────────
D10         GPIO10  NeoPixel data (rings)   Through 300Ω resistor
D7          GPIO20  NeoPixel data (center)  15" build only
D3          GPIO5   Button UP               INPUT_PULLUP, polled
D9          GPIO9   Button DOWN             INPUT_PULLUP, polled
D4          GPIO6   I2C SDA                 VEML7700
D5          GPIO7   I2C SCL                 VEML7700
5V/VIN      -       LED power rail          External 5V supply
GND         -       Common ground           ESP32 + LED supply
3V3         -       VEML7700 power only     Do NOT power LEDs
─────────────────────────────────────────────────────────────
AVOID: GPIO2, GPIO8 (boot strapping); GPIO3/GPIO4 (JTAG)
```

Do not hold the DOWN button (GPIO9) at power-on. It is the ESP32-C3 boot pin.

Full pin maps, LED indexing, power math, and assembly notes: [docs/HARDWARE.md](docs/HARDWARE.md)

## Flash it

Easiest way: [maestro8484.github.io/ChronoBloom/flasher](https://maestro8484.github.io/ChronoBloom/flasher/). Chrome or Edge on a desktop, plug in the XIAO over USB-C, click the button. No install.

If you would rather build from source, that path stays fully open too. Needs [PlatformIO](https://platformio.org/) (VS Code extension or CLI).

First flash over USB:

```
pio run -e esp32c3_v3_8inch -t upload --upload-protocol esptool --upload-port COMx
```

Swap `COMx` for your port (`/dev/ttyACM0` on Linux). Serial monitor runs at 115200.

Every flash after that is OTA from a browser:

1. Build: `pio run -e esp32c3_v3_8inch`
2. Open `http://esp32c3-v3-8inch.local/update`
3. Upload `.pio/build/esp32c3_v3_8inch/firmware.bin`

Inner ring shows blue during update, green on success, red on failure. Device reboots itself.

## WiFi setup

No credentials in code. On first boot the clock listens over the same USB cable for a browser-based WiFi setup (the flasher page above may offer this right after flashing, using the Improv standard). Type your network name and password there and you are done.

If that does not show up, or you skip it, the clock opens its own captive portal instead:

1. Join the WiFi network `esp32c3-clock-setup` from your phone (no password)
2. A setup page opens (or go to `http://192.168.4.1`)
3. Pick your network, enter the password
4. Clock saves it to EEPROM and connects

After that it auto-connects on boot. If the password changes or the network disappears, the portal comes back.

Then open the web UI: `http://esp32c3-v3-8inch.local/` (or the IP from your router's device list).

## Docs

- [docs/HARDWARE.md](docs/HARDWARE.md): pins, LED mapping, power, construction
- [docs/FEATURES.md](docs/FEATURES.md): every feature, current and planned
- [docs/ANIMATIONS.md](docs/ANIMATIONS.md): animation catalog and triggers
- [docs/API.md](docs/API.md): web endpoints, settings JSON, client examples
- [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md): when it does not work
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md): codebase structure, for contributors
- [docs/FOCUS_REMINDERS.md](docs/FOCUS_REMINDERS.md): the ADHD-nudge feature in depth
- [docs/HISTORY.md](docs/HISTORY.md): the full decade-long design story
- [docs/CHANGELOG.md](docs/CHANGELOG.md): version history

## Support

One person built and maintains this as a hobby project. Support is best effort. Open an issue with the bug report or build help template and I will get to it when I can. No SLA, no promises, but I do read everything.

## Origin story and credits

**Steve Manley** built the original NeoPixel Ring Clock in 2015 (Arduino Nano, DS3234 RTC, 3D printed frame). The electronics weren't the draw (ESP32s and addressable LEDs were already familiar territory). It was his 3D-printed design: the way the frame shapes and diffuses the LED glow into crisp pointed hands, like a real clock. The printed core piece in this repo is a remix of his design.

From there, a decade of builds by **Maestro8484**:

- **First edition**: Arduino Nano + RTC + two buttons. Setting time through interrupt-driven buttons was fiddly, but it ran for 2-3 years.
- **ESP8266 port**: same clock, new brain. Ran fine for years; never needed WiFi features.
- **8" build**: the replicable 85%-scale version this repo targets. Alongside it, a custom 10" glow-in-dark wall clock built around Steve's core piece with UV LEDs behind the frame.
- **15" build**: the 8" design scaled ~198% so the outer ring pitch exactly matches side-lit WS2812B strip LED spacing. Much print-and-diffuser trial and error; parchment paper beat every 3D-printed diffuser from 0.8mm down to 0.2mm.
- **ChronoBloom (2026)**: ground-up rewrite on the XIAO ESP32-C3: web UI, NTP, OTA, lux-sensor auto-brightness, Focus Reminders. The two buttons stay as a hardware fallback.

Thanks to Steve for the design that started all of this. Attribution: [NOTICE](NOTICE). Full story: [docs/HISTORY.md](docs/HISTORY.md).

## License

Firmware: [Apache 2.0](LICENSE). Hardware/STL files: [CC BY 4.0](LICENSE-HARDWARE).
