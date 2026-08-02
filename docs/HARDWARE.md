# ChronoBloom ESP32-C3 -- Hardware Specifications

## LED Configuration

### Ring Layout
- **Outer ring**: 60 LEDs (seconds hand + minute hand + quarter-hour markers)
- **Middle ring**: 24 LEDs (hour hand display + ambient fill)
- **Inner ring**: 12 LEDs (hour hand display + ambient fill)
- **Center pixel**: 1 LED (status indicator, breathing animation)

### Variants
**8" Clock (esp32c3_v3_8inch)**:
- Total: 97 LEDs on single strip (60 + 24 + 12 rings, then center pixel)
- Ring pixel offset: 0 (logical ring LED 0 is physical index 0)
- Center pixel: physical index 96 (last in chain)
- Data line: GPIO10 -> 300 ohm resistor (optional) -> DIN of the outer ring

Physical index 0 is a working ring LED. If it dies, everything after it goes dark
too -- a known WS2812B property, because each pixel reads the data and re-sends it
to the next one. The fix is replacing that one pixel, not the whole ring.

> **Note on "sacrificial" first pixels.** Some WS2812B builds put an extra LED
> ahead of the rings, keep it permanently dark, and use it purely to clean up the
> data signal: the ESP32-C3 drives its data pin at 3.3 V, the LEDs want about
> 3.5 V to read a reliable "1" off a 5 V supply, and the first LED re-sends the
> signal at its own full 5 V for everything behind it. ChronoBloom's original 8"
> prototype carried one from early signal troubleshooting. **It was removed on
> 2026-08-01, along with the firmware support for it.** All builds are now the
> plain 97-LED chain.
>
> **Placement is the point.** The sacrificial LED goes right at the board, beside
> the 5 V rail, so the weak 3.3 V signal only travels a couple of centimetres
> before being re-sent as a clean full-swing 5 V one -- and it is that strong
> signal which then makes the long run out to the rings. So it buys data-line
> distance as well as reliability: not by boosting the far end, but by shrinking
> the fragile stretch to almost nothing. Put it at the far end instead and it
> achieves nothing, because the marginal signal has already made the trip.
>
> It is still worth understanding, because it explains a family of "first LED
> flickers", "random sparkle", "works cold, glitches warm" faults -- and one
> genuinely counter-intuitive case where a *better* 5 V supply makes things worse,
> because the LED's threshold scales with its supply voltage. `src/main.cpp` has
> the full plain-English write-up above the LED geometry constants, including the
> symptom list. A 74AHCT125 buffer is the purpose-built version of the same idea,
> fitted in the same place for the same reason, and is what to use on a long run.

**15" Clock (esp32c3_v3_15inch)**:
- Main strip: 96 LEDs (rings only, physical indexes 0-95)
- Separate center strip: 1 LED on GPIO20 (CENTER_PIXEL_SEPARATE_OUTPUT=1)
- Ring offset: 0 -- physical index 0 is the first ring LED (12 o'clock, outer ring)

---

## Pin Assignments (XIAO ESP32-C3)

### NeoPixel Data
| Pin    | GPIO   | Function                  | Notes                          |
|--------|--------|---------------------------|--------------------------------|
| D10    | GPIO10 | NeoPixel rings (DIN)      | 300 ohm resistor inline, optional |
| D7     | GPIO20 | Center pixel (DIN)        | 15" variant only               |

### User Input
| Pin    | GPIO   | Function     | Notes                                                |
|--------|--------|--------------|------------------------------------------------------|
| D3     | GPIO5  | Button UP    | INPUT_PULLUP, polled (no ISR), 50ms debounce         |
| D9     | GPIO9  | Button DOWN  | INPUT_PULLUP, polled (no ISR), 50ms debounce; also XIAO BOOT pin -- do not hold at power-on reset |
| (n/c)  | GPIO3  | Unused       | JTAG TCK -- do NOT use for ISR inputs                 |
| (n/c)  | GPIO4  | Unused       | JTAG TDI -- do NOT use for ISR inputs                 |

> **Button hold-to-repeat**: Short press plus or minus 1 min. Hold >500ms: repeat plus or minus 1 min every 150ms.
> Hold >2s: repeat plus or minus 60 min per fire. Release stops immediately.
>
> **Factory reset**: Hold UP (GPIO5) at power-on -> red LEDs appear ->
> add DOWN (GPIO9) and hold both 3s -> white flash -> reboot into WiFi portal.
> GPIO9 must NOT be held at the power-on instant (enters download mode).

### I2C Bus (Sensors)
| Pin    | GPIO   | Function  | Wire Color | Connected Devices   |
|--------|--------|-----------|------------|---------------------|
| D4     | GPIO6  | I2C SDA   | Green      | VEML7700 (0x10)     |
| D5     | GPIO7  | I2C SCL   | Yellow     | VEML7700            |

### Power
| Pin       | Function                | Notes                              |
|-----------|-------------------------|------------------------------------|
| 5V / VIN  | Board input AND LED power rail | This build feeds the whole board through this pin, not through the USB-C socket. The 5V pin and the USB-C VBUS line are tied to the same internal net, so power put on either one reaches both. The rings tap this same pin; there is no separate LED supply |
| GND       | Common ground           | Rings share ground with the XIAO   |
| 3V3       | Logic power only        | **Do NOT power LEDs from 3.3V**    |

**Never power the board from the 5V pin and the USB-C cable at the same time.**
Because the two are the same net, doing both back-feeds one source into the
other with nothing in between to stop it. That is only safe with a blocking
diode (an "OR-ing" or ideal-diode circuit) between the two inputs. This board
has no such diode fitted. Pick one power source and unplug the other before
connecting the second.

A 5V 2A supply wired directly to the 5V pin powers the whole clock. The
firmware sums each frame's LED draw and dims any frame that would cross the
1.8A budget, so the supply is never overdrawn. Details under Power
Requirements below.

### Avoid These Pins
- **GPIO2, GPIO8** -- ESP32-C3 boot strapping pins. Never use for peripherals.
- **GPIO9** -- also a strapping pin (BOOT). It works for exactly one job: the polled
  pull-up Button DOWN, as wired above. Nothing else, and never held low at the
  power-on instant (that puts the chip in download mode).

---

## Sensors

### VEML7700 Ambient Light Sensor
- **I2C Address**: 0x10 (default, fixed)
- **Range**: 0-120,000 lux with auto-gain
- **Power**: 3.3V
- **Purpose**: auto-brightness and dark-room display sleep
- **Library**: Adafruit_VEML7700

**Wiring**:
```
VEML7700 VCC -> XIAO 3.3V
VEML7700 GND -> XIAO GND
VEML7700 SDA -> GPIO6 (green wire)
VEML7700 SCL -> GPIO7 (yellow wire)
```

---

## Physical Construction

### Materials
**Frame**:
- Any PLA, 3D printed. UV glow-in-the-dark PLA is a nice touch: it charges off the LEDs and glows softly after the display sleeps.

**Diffuser**:
- 8": 0.6mm printed white PLA over the face (see [PRINTING.md](PRINTING.md))
- 15": laser-cut acrylic with parchment paper behind it, or a 0.5mm printed PLA face

### Scale Variants
**8" Clock**:
- Sized so the 241-LED kit's rings seat precisely. Largest part measures 209 mm square, about 8 inches, so a 220 mm bed (Ender 3 class) is enough
- Single-piece parts where possible

**15" Clock**:
- 200% scale
- Frame prints in thirds, joined with alignment pins
- Acrylic face needs a 600mm x 410mm laser bed

### Assembly Notes
- LED rings mounted with cable ties or hot glue
- Wire management: route all wires through center, exit at bottom

---

## Power Requirements

### LED Power Consumption
- **Per LED**: ~60mA at full white (all channels 100%)
- **Typical usage**: ~20mA per LED (colored, not full brightness)
- **Total worst-case** (97 LEDs, 8" variant): 5.8A @ 5V (rare, only during full-white test)
- **Typical operation**: 1.5-2A @ 5V (clock display mode)

### Recommended Power Supply
A 5V 2A supply wired to the 5V/VIN pin is enough. The firmware caps total LED draw at 1800mA (`MAX_LED_MILLIAMPS`): before every frame it adds up what the pixels are about to draw and dims that frame if it would cross the budget. So the worst-case 5.8A can't actually happen.

### ESP32-C3 Power
- **Active WiFi**: ~120mA @ 3.3V
- **Powered via**: 5V/VIN pin on this build. The USB-C socket is an alternate input on the same net, not a second simultaneous source -- see the warning under Pin Assignments > Power.

---

## Mounting
- **Wall-mounted**: Keyhole slots in back frame (15" variant)
- **Desktop stand**: 3D printed kickstand (8" variant)
- **Weight**: ~500g (8"), ~1.2kg (15")

---

## Firmware Pin Mapping (Code Reference)

### Build Flags (platformio.ini)
```ini
-D LED_DATA_PIN=10
-D CENTER_PIXEL_PIN=20           # 15" variant only
-D CENTER_PIXEL_SEPARATE_OUTPUT=1 # 15" variant only
-D LUX_SENSOR_I2C_SDA=6
-D LUX_SENSOR_I2C_SCL=7
```

### Ring Config (main.cpp)
```cpp
// Both variants use RING_PIXEL_OFFSET = 0.
// Raise it only if your chain has unused LEDs before the outer ring.
constexpr RingConfig RING_OUTER_60  = {60, RING_PIXEL_OFFSET,      true};  // 60 LEDs
constexpr RingConfig RING_MIDDLE_24 = {24, RING_PIXEL_OFFSET + 60, true};  // 24 LEDs
constexpr RingConfig RING_INNER_12  = {12, RING_PIXEL_OFFSET + 84, true};  // 12 LEDs
```

### LED Indexing (8" variant, default geometry)
- **Physical strip indexes 0-59**: Outer ring (seconds/minutes), logical 0-59
- **Physical strip indexes 60-83**: Middle ring (hours), logical 0-23
- **Physical strip indexes 84-95**: Inner ring (hours), logical 0-11
- **Physical strip index 96**: Center pixel

> If a chain ever carries unused LEDs ahead of the outer ring, raise
> `RING_PIXEL_OFFSET` and `CLOCK_PIXEL_COUNT` to match, via a `[led_chain]`
> override in `platformio.local.ini`. Firmware geometry and real wiring must
> always agree: a mismatch rotates the whole clock face by one LED.

### LED Indexing (15" variant)
- **Physical strip indexes 0-59**: Outer ring, logical 0-59
- **Physical strip indexes 60-83**: Middle ring, logical 0-23
- **Physical strip indexes 84-95**: Inner ring, logical 0-11
- **Separate strip index 0**: Center pixel (GPIO20)

---

## Testing & Calibration

### I2C Bus Scan (Check Sensor Connection)
```cpp
Wire.begin(6, 7);
for (uint8_t addr = 1; addr < 127; addr++) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    Serial.print("Found I2C device at 0x");
    Serial.println(addr, HEX);
  }
}
// Expected: 0x10 (VEML7700)
```

### LED Mapping Test
Add to `setup()` for visual verification:
```cpp
// Light each ring in sequence
for (int i = 0; i < 60; i++) {
  strip.setPixelColor(i + RING_PIXEL_OFFSET, strip.Color(255, 0, 0));
  strip.show();
  delay(50);
}
```

### Brightness Calibration
- **Dark room** (0-1 lux): Target brightness ~15
- **Evening light** (10 lux): Target brightness ~60
- **Indoor day** (100 lux): Target brightness ~120
- **Bright window** (1000 lux): Target brightness ~200
- **Direct sun** (10,000+ lux): Target brightness 255

Adjust logarithmic curve in `LuxSensor::autoBrightness()` if needed.
