# ChronoBloom ESP32-C3 — Hardware Specifications

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
- Data line: GPIO10 → 300Ω resistor → DIN of the outer ring

> **Sacrificial pixel (vestigial, disabled by default).** The firmware supports
> an optional extra WS2812B ahead of the rings that stays dark and re-drives
> the 3.3V data line at 5V logic (`SACRIFICIAL_PIXEL_ENABLED`, plus a 98-LED /
> offset-1 geometry). The original prototype has one spliced in as a leftover
> from early signal troubleshooting; it proved unnecessary and remains only
> because removing it wasn't worth the soldering. Replicable builds should
> leave it disabled. If you genuinely need level shifting on a long data run,
> use a 74AHCT125 instead. Enable via a `[led_chain]` override in
> `platformio.local.ini` (see `platformio.local.ini.example`).

**15" Clock (esp32c3_v3_15inch)**:
- Main strip: 96 LEDs (rings only, physical indexes 0–95)
- Separate center strip: 1 LED on GPIO20 (CENTER_PIXEL_SEPARATE_OUTPUT=1)
- Ring offset: 0 — physical index 0 is the first ring LED (12 o'clock, outer ring)

---

## Pin Assignments (XIAO ESP32-C3)

### NeoPixel Data
| Pin    | GPIO   | Function                  | Notes                          |
|--------|--------|---------------------------|--------------------------------|
| D10    | GPIO10 | NeoPixel rings (DIN)      | 300Ω resistor inline to strip |
| D7     | GPIO20 | Center pixel (DIN)        | 15" variant only               |

### User Input
| Pin    | GPIO   | Function     | Notes                                                |
|--------|--------|--------------|------------------------------------------------------|
| D3     | GPIO5  | Button UP    | INPUT_PULLUP, polled (no ISR), 50ms debounce         |
| D9     | GPIO9  | Button DOWN  | INPUT_PULLUP, polled (no ISR), 50ms debounce; also XIAO BOOT pin — do not hold at power-on reset |
| (n/c)  | GPIO3  | Unused       | JTAG TCK — do NOT use for ISR inputs                 |
| (n/c)  | GPIO4  | Unused       | JTAG TDI — do NOT use for ISR inputs                 |

> **Button hold-to-repeat** (v2.0.7): Short press ±1 min. Hold >500ms: repeat ±1 min every 150ms.
> Hold >2s: repeat ±60 min per fire. Release stops immediately.
>
> **Factory reset** (v2.0.8): Hold UP (GPIO5) at power-on → red LEDs appear →
> add DOWN (GPIO9) and hold both 3s → white flash → reboot into WiFi portal.
> GPIO9 must NOT be held at the power-on instant (enters download mode).

### I2C Bus (Sensors)
| Pin    | GPIO   | Function  | Wire Color | Connected Devices   |
|--------|--------|-----------|------------|---------------------|
| D4     | GPIO6  | I2C SDA   | Green      | VEML7700 (0x10)     |
| D5     | GPIO7  | I2C SCL   | Yellow     | Future: BME280/SHT31|

### Power
| Pin       | Function                | Notes                              |
|-----------|-------------------------|------------------------------------|
| 5V / VIN  | LED power rail          | External 5V supply (2-5A)          |
| GND       | Common ground           | ESP32 + LED supply must connect    |
| 3V3       | Logic power only        | **Do NOT power LEDs from 3.3V**    |

### Avoid These Pins
**GPIO2, GPIO8, GPIO9** — ESP32-C3 boot strapping pins. Do not use for any peripherals.

---

## Sensors

### VEML7700 Ambient Light Sensor
- **I2C Address**: 0x10 (default, fixed)
- **Range**: 0-120,000 lux with auto-gain
- **Resolution**: 16-bit
- **Power**: 3.3V (< 1mA typical)
- **Purpose**: Auto-brightness, sunrise/sunset detection
- **Library**: Adafruit_VEML7700

**Wiring**:
```
VEML7700 VCC → XIAO 3.3V
VEML7700 GND → XIAO GND
VEML7700 SDA → GPIO6 (green wire)
VEML7700 SCL → GPIO7 (yellow wire)
```

### Future: Temperature/Humidity Sensor
**BME280** (recommended):
- I2C Address: 0x76 or 0x77
- Temp + Humidity + Pressure
- Can coexist with VEML7700 on same I2C bus

**SHT31** (alternative):
- I2C Address: 0x44
- Temp + Humidity (higher accuracy than BME280)
- Can coexist with VEML7700

---

## Physical Construction

### Materials
**Frame**:
- UV glow-in-dark PLA (3D printed)
- Charges from LEDs and ambient room light
- Glows softly in darkness

**Diffuser**:
- Parchment paper (laser-safe, optimal light distribution)
- Creates soft petal-like bloom effect visible in photos

**Face**:
- Laser-cut acrylic (clear or frosted)
- 600mm x 410mm CO2 laser bed used for 15" variant

### Scale Variants
**8.5" Clock**:
- 85% scale (0.85x original)
- Maximizes Bambu Labs P1S print bed (256mm x 256mm)
- Single-piece parts where possible

**15" Clock**:
- 200% scale (2x original)
- Parts split into thirds for assembly
- Requires laser cutter for acrylic face (600mm x 410mm bed)
- Modular assembly with alignment pins

### Assembly Notes
- Frame prints in thirds, joined with alignment pins
- Diffuser sandwiched between frame and acrylic face
- LED strips mounted to back of frame with cable ties or hot glue
- Wire management: route all wires through center, exit at bottom

---

## Power Requirements

### LED Power Consumption
- **Per LED**: ~60mA at full white (all channels 100%)
- **Typical usage**: ~20mA per LED (colored, not full brightness)
- **Total worst-case** (97 LEDs, 8" variant): 5.8A @ 5V (rare, only during full-white test)
- **Typical operation**: 1.5-2A @ 5V (clock display mode)

### Recommended Power Supply
- **Minimum**: 5V 2A (wall adapter)
- **Recommended**: 5V 3A (handles animations + brightness spikes)
- **Best**: 5V 5A (headroom for future expansion)

### ESP32-C3 Power
- **Active WiFi**: ~120mA @ 3.3V
- **Deep sleep**: ~10μA (not used in this project)
- **Powered via**: USB-C (XIAO board) or VIN pin

---

## Environmental Specifications

### Operating Conditions
- **Temperature**: 0°C to 50°C (indoor use)
- **Humidity**: 20-80% RH (non-condensing)
- **Light sensitivity**: VEML7700 auto-adjusts gain (0.0036 - 120,000 lux range)

### Mounting
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
// Both variants default to RING_PIXEL_OFFSET = 0.
// Vestigial sacrificial-pixel override (local build only): offset = 1.
constexpr RingConfig RING_OUTER_60  = {60, RING_PIXEL_OFFSET,      true};  // 60 LEDs
constexpr RingConfig RING_MIDDLE_24 = {24, RING_PIXEL_OFFSET + 60, true};  // 24 LEDs
constexpr RingConfig RING_INNER_12  = {12, RING_PIXEL_OFFSET + 84, true};  // 12 LEDs
```

### LED Indexing (8" variant, default geometry)
- **Physical strip indexes 0–59**: Outer ring (seconds/minutes), logical 0–59
- **Physical strip indexes 60–83**: Middle ring (hours), logical 0–23
- **Physical strip indexes 84–95**: Inner ring (hours), logical 0–11
- **Physical strip index 96**: Center pixel

> With the vestigial sacrificial-pixel override enabled (98-LED chain), the
> dark sacrificial WS2812B sits at physical index 0 and everything above
> shifts up by one (rings 1–96, center 97, `RING_PIXEL_OFFSET=1`).

### LED Indexing (15" variant)
- **Physical strip indexes 0–59**: Outer ring, logical 0–59
- **Physical strip indexes 60–83**: Middle ring, logical 0–23
- **Physical strip indexes 84–95**: Inner ring, logical 0–11
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
