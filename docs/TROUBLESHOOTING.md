# ChronoBloom ESP32-C3 -- Troubleshooting Guide

## Common Issues & Solutions

---

## Brightness Issues

### Display goes completely dark after saving settings

**Symptoms**:
- All LEDs off after a settings save
- Web UI still responds but nothing is visible
- Reloading the page shows brightness value of 0

**Cause**: `dayBrightness=0` (or `nightBrightness=0`) was submitted. In `autoBrightnessMode=0` (manual), `dayBrightness` is passed directly to the LED strip -- zero means off. Pre-v2.1.1 firmware had no floor on this field.

**Fix (v2.1.1+)**: `sanitize()` now floors `dayBrightness < 5` to 44 and `nightBrightness < 1` to 5. Upgrade firmware and re-save any settings to recover.

**Recovery on older firmware**: Use `/diag` to confirm the device is alive, then POST a non-zero brightness:
```
curl -X POST "http://esp32c3-v3-8inch.local/settings" -d "dayBrightness=150"
```
Or hold both buttons on boot for 3 seconds to factory reset EEPROM to defaults.

---

## WebUI / HTTP Issues

### 15inch variant reboots when loading the root page

**Symptoms**:
- Animations become laggy immediately before crash
- Device reboots with `boot_reason: watchdog` or `boot_reason: panic` in `/diag`
- 8inch variant is unaffected

**Root cause**: `htmlPage()` allocated a ~6KB `String` on the heap at request time.
On the 15inch variant (two NeoPixel strips, larger pixel buffer), heap headroom was too
tight to absorb the double-allocation during `server_.send()`.

**Fix**: Resolved in v2.1.0. All large HTML responses now use PROGMEM chunks streamed
via `sendContent_P()`. JSON handlers use `snprintf` into stack buffers. No heap
allocation occurs at page-load time. Confirmed free heap after fix: **236KB**.

**If still crashing after flashing v2.1.0**: Check `/diag` for `free_heap`. If below
~80KB, another allocation source is the culprit -- check for long-running String builds
elsewhere in the loop.

---

## WiFi Connection Problems

### Clock won't connect to WiFi

**Symptoms**:
- Red spinning dot on inner ring (STATUS_WIFI_FAIL animation)
- Serial output shows "Wi-Fi connect failed. Status: X"
- Web UI not accessible

**Diagnostic steps**:
1. Check serial monitor output (`pio device monitor`)
2. Verify WiFi credentials in `platformio.ini`:
   ```ini
   -D WIFI_SSID=\"YourSSID\"
   -D WIFI_PASSWORD=\"YourPassword\"
   ```
3. Check router DHCP table for device (hostname: `esp32c3-v3-8inch`)
4. Verify SSID is 2.4GHz (ESP32-C3 does NOT support 5GHz)

**Common fixes**:
- Special characters in password need escaping: `\"` for quotes, `\\` for backslash
- SSID/password case-sensitive
- Router may be blocking new devices (check MAC filter)
- WiFi signal too weak (try closer to router during first boot)

**Workaround**: Add temporary debug output
```cpp
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
Serial.print("Connecting to: ");
Serial.println(WIFI_SSID);
Serial.print("Password length: ");
Serial.println(strlen(WIFI_PASSWORD));
```

---

### mDNS hostname not resolving

**Symptoms**:
- `http://esp32c3-v3-8inch.local/` times out
- Direct IP access works

**Fixes**:
1. **Windows**: Install Bonjour Print Services (Apple) for mDNS support
2. **Linux**: Ensure `avahi-daemon` is running
3. **macOS**: Should work natively
4. **Alternative**: Use direct IP address from serial output or router DHCP table

**Check mDNS status**:
```cpp
// In setup(), after MDNS.begin()
Serial.print("mDNS started: ");
Serial.println(mdnsEnabled_ ? "YES" : "NO");
```

---

## Button Issues

### Spurious button presses / time jumping backward

> **Resolved v2.0.6**: Buttons moved to GPIO5(UP)/GPIO9(DOWN) with polled reads. Original root cause documented below for reference.

**Symptoms (historical -- ISR-based buttons on GPIO3/GPIO4)**:
- Time jumps backward by 5 minutes without physical button press
- Most common when USB cable connected
- Less common when powered via 5V-only supply

**Root cause**: GPIO3/GPIO4 are JTAG TCK/TDI pins on ESP32-C3. USB data connection can trigger JTAG subsystem, causing FALLING edge interrupts.

**Diagnostic test**:
1. Power clock via 5V-only supply (no USB data pins connected)
2. Observe if spurious presses stop
3. If yes, confirms JTAG interference

**Fixes** (in priority order):

**Option 1: Revert to polled buttons** (recommended)
```cpp
// Replace ISR-based ButtonInput with polled version
void poll(uint32_t now) {
  bool upNow = (digitalRead(BUTTON_UP_PIN) == LOW);
  bool downNow = (digitalRead(BUTTON_DOWN_PIN) == LOW);
  
  if (upNow && !lastUp_ && now - lastUpMs_ > debounceMs_) {
    upPressed_ = true;
    lastUpMs_ = now;
  }
  if (downNow && !lastDown_ && now - lastDownMs_ > debounceMs_) {
    downPressed_ = true;
    lastDownMs_ = now;
  }
  
  lastUp_ = upNow;
  lastDown_ = downNow;
}

// Call in loop():
buttons.poll(millis());
```

**Option 2: Move buttons to safe pins** (requires rewiring)
- Use GPIO6, GPIO7, GPIO8, or GPIO9
- Note: GPIO6/7 currently used for I2C (VEML7700)
- Viable if sensor not installed

**Option 3: Add stable-LOW guard** (software-only, partial fix)
```cpp
// In main loop, when consuming button flag:
if (buttonUpPressed_) {
  if (digitalRead(BUTTON_UP_PIN) == LOW) {  // Verify still pressed
    timeModel.addMinutes(1);
  }
  buttonUpPressed_ = false;
}
```

---

### Buttons not responding at all

**Symptoms**:
- Physical button press does nothing
- No animation, no time change

**Checks**:
1. Verify pull-up resistors: buttons should connect pin to GND when pressed
2. Check `INPUT_PULLUP` mode set in `pinMode()`
3. Test with multimeter: pin should read 3.3V when not pressed, 0V when pressed
4. Verify button wiring: Common = GND, NO (normally open) = GPIO pin

**Debug code**:
```cpp
// In loop():
Serial.print("UP: ");
Serial.print(digitalRead(GPIO3));
Serial.print(" DOWN: ");
Serial.println(digitalRead(GPIO4));
delay(500);
```
Should print `1 1` when not pressed, `0 1` or `1 0` when single button pressed.

---

## LED Display Problems

### All LEDs off / no display

**Checks**:
1. **Power supply**: Verify 5V present at LED strip VCC/GND
2. **Data connection**: Check 300 ohm resistor inline with GPIO10 -> LED DIN
3. **Strip polarity**: Ensure VCC/GND not swapped (can damage LEDs)
4. **Index 0**: On both variants, physical index 0 is the first active ring LED (`RING_PIXEL_OFFSET=0`). If upgrading the 15" from a pre-v2.0.2 build, rewire GPIO10 data line directly to DIN of first ring LED.
5. **First LED misbehaving specifically** (flickers, wrong colour, or sparkle that clusters at the start of the chain while the rest is fine): that pattern points at logic levels, not a dead LED. The C3 drives data at 3.3 V and WS2812Bs want about 3.5 V off a 5 V supply, so the first LED is reading a marginal signal. Watch for the counter-intuitive version: a *better*, truer 5 V supply can break a chain that worked on a sagging one, because the threshold scales with supply voltage. Keep the run from the board to the first LED short, keep a ground alongside the data wire, and if it persists fit a 74AHCT125 buffer. `src/main.cpp` carries the full explanation above the LED geometry constants.

**Serial output check**:
```
ChronoBloom v3 booting...
Build target: esp32c3-v3-8inch
LED count: 97
Ring pixel offset: 0
Center pixel: enabled at physical index 96
```

**Test code**:
```cpp
// In setup(), after strip_.begin():
strip_.fill(strip_.Color(255, 0, 0), 0, CLOCK_PIXEL_COUNT);  // All red
strip_.show();
delay(2000);
```

---

### Center pixel not lighting (15" variant)

**Symptoms**:
- All rings work
- Center pixel remains dark

**Checks**:
1. Verify `CENTER_PIXEL_SEPARATE_OUTPUT=1` in build flags
2. Check `CENTER_PIXEL_PIN=20` wired correctly
3. Confirm GPIO20 not shorted to GND
4. Test center strip separately:
```cpp
centerStrip_->setPixelColor(0, centerStrip_->Color(255, 255, 255));
centerStrip_->show();
```

---

### Wrong colors / colors shifted

**Symptoms**:
- Red appears green, green appears blue, etc.

**Cause**: NeoPixel color order mismatch

**Fix**: Change `NEO_GRB` to `NEO_RGB` (or vice versa) in NeoPixel initialization:
```cpp
Adafruit_NeoPixel ledStrip(CLOCK_PIXEL_COUNT, LED_DATA_PIN, NEO_RGB + NEO_KHZ800);
```

Common orders: `NEO_GRB`, `NEO_RGB`, `NEO_GRBW` (if RGBW strip)

---

### Flickering during web requests

**Symptoms**:
- LEDs flicker or dim briefly when accessing web UI
- Most noticeable during long HTTP responses

**Cause**: WiFi interrupt preemption delays `strip_.show()` timing

**Mitigations**:
1. Reduce web UI polling frequency (increase `setInterval` from 1000ms to 2000ms)
2. Use longer delays between animation frames
3. Disable WiFi during critical animations:
```cpp
WiFi.mode(WIFI_OFF);
renderAnimation();
WiFi.mode(WIFI_STA);
```

---

## Sensor Issues

### VEML7700 not detected

**Symptoms**:
- Serial output: "VEML7700 not found on I2C"
- `/lux` endpoint returns `{"available":false}`

**Diagnostic I2C scan**:
```cpp
Wire.begin(6, 7);
Serial.println("Scanning I2C bus...");
for (uint8_t addr = 1; addr < 127; addr++) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    Serial.print("Found device at 0x");
    Serial.println(addr, HEX);
  }
}
```
Expected output: `Found device at 0x10`

**Common fixes**:
- Check SDA/SCL not swapped (green=SDA/GPIO6, yellow=SCL/GPIO7)
- Verify 3.3V and GND connected
- Try different I2C address if using non-standard VEML7700 breakout
- Check solder joints on sensor breakout board

---

### Auto-brightness not working

**Symptoms**:
- Lux reading shown in web UI
- Brightness doesn't change with room lighting

**Checks**:
1. Verify `autoBrightnessMode = 1` (auto) in web UI, not 0 (manual) or 2 (scheduled)
2. Check min/max brightness limits (if both set to same value, no change visible)
3. Confirm lux reading actually changing (cover sensor, shine flashlight)

**Debug output**:
```cpp
Serial.print("Lux: ");
Serial.print(luxSensor.lux());
Serial.print(" -> Brightness: ");
Serial.println(luxSensor.autoBrightness());
```

**Calibration**: If brightness changes too slowly/quickly, adjust logarithmic curve in `LuxSensor::autoBrightness()`.

---

## Settings & EEPROM

### Settings not persisting after reboot

**Symptoms**:
- Change settings in web UI, reboot, settings reset to defaults

**Checks**:
1. Verify "Save display" button clicked (not just changing values)
2. Check serial output for "Settings saved" confirmation
3. Verify `EEPROM.commit()` called after `EEPROM.put()`

**Force EEPROM reset** (if corrupted):
```cpp
// In setup(), before settingsStore.begin():
EEPROM.begin(256);
for (int i = 0; i < 256; i++) {
  EEPROM.write(i, 0x00);
}
EEPROM.commit();
Serial.println("EEPROM cleared");
```

---

### Settings reset unexpectedly

**Cause**: Settings version mismatch (firmware update changed `ClockSettings` struct)

**Expected behavior**: When `SETTINGS_VERSION` increments, all settings reset to defaults. This is by design to ensure clean state.

**Check version**:
```cpp
Serial.print("Stored settings version: ");
Serial.println(storedSettings.version);
Serial.print("Current firmware version: ");
Serial.println(SETTINGS_VERSION);
```

---

## Web UI Issues

### Web UI loads but controls don't work

**Symptoms**:
- Can access web page
- Buttons/sliders do nothing
- No errors in browser console

**Checks**:
1. Hard refresh browser (Ctrl+Shift+R) to clear cached JavaScript
2. Check browser console for JavaScript errors (F12 -> Console tab)
3. Verify `/settings` POST endpoint responding (Network tab in F12)

**Test endpoint directly**:
```
POST http://192.168.1.XXX/settings
Content-Type: application/x-www-form-urlencoded

dayBrightness=200&nightBrightness=20
```

---

### Live clock preview not updating

**Symptoms**:
- SVG clock visible but frozen
- Time doesn't advance

**Checks**:
1. Verify `/time` endpoint responding (check Network tab, should poll every 1 sec)
2. Check JavaScript console for fetch errors
3. Confirm `setInterval(refresh, 1000)` running

**Test `/time` endpoint**:
```
GET http://192.168.1.XXX/time
```
Should return JSON: `{"hour":14,"minute":30,"second":45,...}`

---

## Build & Flash Issues

### Upload fails: "Port not found"

**Symptoms**:
- `pio run -t upload` fails
- Error: "Please specify `upload_port`"

**Fixes**:
1. Check USB cable (data-capable, not charge-only)
2. Verify XIAO board drivers installed (CH340 or CP210x)
3. Manually specify port:
```powershell
pio run -e esp32c3_v3_8inch -t upload --upload-port COM3
```
4. Windows: Check Device Manager -> Ports (COM & LPT)
5. Linux: Check `ls /dev/ttyUSB*` or `ls /dev/ttyACM*`

---

### Compilation errors after updating

**Common errors**:

**"LuxSensor was not declared"**:
- Missing `#include` or class definition
- Check `LUX_SENSOR_ENABLED=1` in build flags

**"Adafruit_VEML7700.h: No such file"**:
- Library not installed
- Add to `platformio.ini` lib_deps:
```ini
adafruit/Adafruit VEML7700 Library @ ^2.1.6
```
- Run `pio lib install`

**"Multiple definition of..."**:
- Function/variable defined in header without `inline` or `static`
- Move implementation to `.cpp` or add `inline` keyword

---

## Animation Issues

### Animations not triggering at expected times

**Symptoms**:
- Top of hour passes, no animation
- Quarter-hour passes, nothing happens

**Checks**:
1. Verify `intervalAnimationsEnabled = 1` in web UI
2. Confirm specific animation not set to "Off" (mode 0)
3. Check time is actually advancing (NTP sync working)

**Debug trigger logic**:
```cpp
if (time.minute != lastMinute && time.second == 0) {
  Serial.print("Minute changed: ");
  Serial.print(time.minute);
  Serial.print(" Animation enabled: ");
  Serial.println(settingsStore.get().intervalAnimationsEnabled);
}
```

---

### Animation runs but clock display wrong after

**Symptoms**:
- Animation plays correctly
- Afterward, clock shows wrong time or frozen

**Cause**: Animation didn't call `timeModel.markDirty()` at end

**Fix**: Add to end of animation function:
```cpp
void renderMyAnimation(uint32_t now) {
  // ... animation code ...
  
  timeModel.markDirty();  // Force full redraw
}
```

---

## Demo Mode

### Demo Mode won't start

**Symptoms**:
- Click "Start" button in web UI, nothing happens
- Status shows "Idle" and doesn't advance

**Checks**:
1. Verify device is connected and web UI loading normally
2. Check browser console (F12) for JavaScript errors on `startDemo()` call
3. Confirm `/demo/start` endpoint is reachable: `curl -X POST http://esp32c3-v3-8inch.local/demo/start`
4. Check `/demo/status` response (should return `{"active":true,...}` after starting)

**Test response**:
```
GET http://esp32c3-v3-8inch.local/demo/status
```
Should return JSON with `"active": true` and current `"step"` number.

---

### Demo Mode stops abruptly

**Symptoms**:
- Demo starts (status updates)
- Stops mid-sequence
- Clock returns to normal display

**Checks**:
1. Verify WiFi still connected (demo stops if OTA or other network event interrupts)
2. Check serial output for watchdog resets or exceptions
3. Confirm device free heap > 80KB (large allocations during demo can cause issues)
4. Verify step duration timings not being overridden by user interaction

**Debug output**:
```cpp
// In DemoMode::loop():
Serial.print("Demo step: ");
Serial.print(currentStep_);
Serial.print(" Elapsed: ");
Serial.println(elapsedMs_);
```

---

### /demo/overlay not displaying correctly in OBS

**Symptoms**:
- OBS browser source shows blank or wrong content
- Text not fading in/out smoothly
- Subtitles lag behind demo sequence

**Checks**:
1. Verify OBS browser source URL: `http://esp32c3-v3-8inch.local/demo/overlay`
2. Confirm OBS browser source dimensions: 1920x1080 (as per spec)
3. Check OBS browser source cache disabled (may cause stale HTML load)
4. Verify `/demo/status` polling responding (overlay polls every 500ms)
5. Confirm browser zoom set to 100% in OBS properties

**Test `/demo/overlay` in browser**:
- Open directly: `http://esp32c3-v3-8inch.local/demo/overlay`
- Should display full-screen white text on black pill background
- Text should fade in/out as demo progresses

---

### Lux override not working during demo

**Symptoms**:
- Auto-brightness demo step (step 3) runs but brightness doesn't change
- Lux reading in web UI shows real sensor value, not override

**Cause**: `LuxSensor::setLuxOverride()` called during demo should temporarily bypass hardware read.

**Checks**:
1. Verify auto-brightness mode enabled in web UI (`autoBrightnessMode = 1`)
2. Confirm lux limits allow room to change (min/max brightness differ)
3. Check `/lux` endpoint during demo: should return override value, not hardware value

**Debug output**:
```cpp
// In LuxSensor::lux():
if (luxOverrideActive_) {
  Serial.print("Override active: ");
  Serial.println(luxOverrideValue_);
}
```

---

### Buttons not responsive during demo

**Expected behavior**: Physical buttons are ignored during demo playback.

**If buttons still control time**:
1. Verify demo is actually running (`/demo/status` shows `"active": true`)
2. Check `ButtonInput::poll()` respects demo state -- should consume button events without acting on them
3. Confirm buttons weren't pressed before demo started (state machine may have pending press)

---

## Performance Issues

### Sluggish web UI / slow response

**Symptoms**:
- Web page takes seconds to load
- Button clicks delayed

**Causes**:
- WiFi signal weak (clock too far from router)
- Too many concurrent connections
- ESP32 busy with animation rendering

**Mitigations**:
1. Move clock closer to router
2. Reduce SVG preview refresh rate (2000ms instead of 1000ms)
3. Disable animations during web UI usage
4. Increase web server timeout in code

---

### Clock loses time / drifts

**Symptoms**:
- Clock slow by minutes/hours
- No NTP sync happening

**Checks**:
1. Verify NTP sync enabled: `ENABLE_NTP=1` in build flags
2. Check WiFi connected (NTP requires network)
3. Monitor serial for "NTP sync" messages
4. Verify timezone string correct: `MST7MDT,M3.2.0,M11.1.0` for Mountain time

**Force NTP sync** via web UI or POST:
```
POST http://192.168.1.XXX/syncNtp
```

---

## Hardware Damage Prevention

### Precautions
1. **NEVER power LEDs from 3.3V** -- Will damage ESP32 voltage regulator
2. **Check polarity before connecting** -- Reversed VCC/GND can destroy LED strip
3. **Use 300 ohm resistor on data line** -- Prevents voltage spikes from damaging first LED
4. **Limit initial brightness** -- Test with low brightness before full power
5. **Proper power supply sizing** -- 5V 2A is enough: firmware caps total LED draw at 1800mA (`MAX_LED_MILLIAMPS`) by dimming any frame that would exceed it

### If LEDs won't light after assembly
1. Check LED strip not damaged during fabrication (measure resistance VCC-GND, should be >1k ohm)
2. Verify data direction (DIN -> DOUT, chain must be correct direction)
3. Test with single LED first (disconnect strip, connect one LED directly)

---

## Getting Help

### Before asking for help, collect:
1. Serial monitor output (full boot sequence)
2. Web UI screenshot showing current settings
3. Build target (`esp32c3_v3_8inch` or `esp32c3_v3_15inch`)
4. Firmware version (check serial: "SETTINGS_VERSION")
5. Symptoms and steps to reproduce

### Useful diagnostic commands
```powershell
# Full serial output
pio device monitor -e esp32c3_v3_8inch

# Clean build (remove cached files)
pio run -e esp32c3_v3_8inch -t clean
pio run -e esp32c3_v3_8inch

# List connected devices
pio device list
```

---

## WiFi AP Fallback

### Device starts its own access point instead of connecting

**When this happens**:
- All STA connection attempts fail (wrong credentials, network unavailable)
- Portal timeout (120 seconds) expires without a successful connection
- Device starts `WiFi.softAP(DEVICE_HOSTNAME)` and serves web UI at `192.168.4.1`

**In AP fallback mode**:
- Clock continues to display time (using last-known or browser-synced time)
- Web UI is accessible at `http://192.168.4.1/`
- NTP, mDNS, and OTA are unavailable until STA connects
- All settings can be changed and saved via the web UI

**To restore STA connection**:
1. Connect to the device AP (`esp32c3-v3-8inch` or similar SSID)
2. Navigate to `http://192.168.4.1/`
3. Use web UI or force a portal reset (see WiFi Provisioning Portal section below)

---

## WiFi Provisioning Portal

### Portal doesn't appear on first boot

**Symptoms**:
- Expected `esp32c3-clock-setup` WiFi network doesn't appear
- Device tries to connect to hardcoded SSID instead

**Fixes**:
1. Check if WiFiManager has saved credentials from previous session -- portal only appears on true first boot
2. Force reset: uncomment `wm.resetSettings();` in `setupWiFi()` function, rebuild, and flash
3. Verify ENABLE_WIFI_UI is defined in build flags

---

## OTA (Over-The-Air) Updates

OTA is the normal update path -- no USB cable required after initial flash.

**Preferred method -- Web UI**:
1. Build in VS Code / PlatformIO
2. Open `http://esp32c3-v3-8inch.local/update`
3. Select `.pio/build/esp32c3_v3_8inch/firmware.bin` and upload
4. Device reboots automatically on success

**Alternative -- PlatformIO espota** (default upload protocol):
```powershell
pio run -e esp32c3_v3_8inch -t upload
```
Targets IP `192.168.1.x` (hardcoded in `platformio.ini`). Update `upload_port` if device IP changes.

**USB fallback** (first flash or recovery only):
```powershell
pio run -e esp32c3_v3_8inch -t upload --upload-protocol esptool --upload-port COMx
```

### OTA not responding

**Symptoms**: espota times out with "No response from ESP" or web UI shows connection error

**Fixes**:
1. Verify device is on WiFi: `ping 192.168.1.x`
2. Verify OTA initialized: serial monitor should show `[OTA] Ready` after boot
3. Power-cycle device to clear any stuck OTA state
4. If device IP changed: update `upload_port` in `platformio.ini`

### Device reboots during OTA

**Symptoms**: upload starts (blue animation), then device reboots before completion

**Possible causes**:
- WiFi instability -- move device closer to router
- Software watchdog fired (10s window) -- should auto-recover on reboot

---

### Check documentation
- [ARCHITECTURE.md](ARCHITECTURE.md) -- Codebase structure
- [HARDWARE.md](HARDWARE.md) -- Pin mappings and specs
- [FEATURES.md](FEATURES.md) -- Current feature list
- [API.md](API.md) -- Web endpoints and settings structure
