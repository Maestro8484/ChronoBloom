# ChronoBloom ESP32-C3 — Web API Reference

## HTTP Endpoints

**Base URL**: `http://esp32c3-v3-8inch.local/` (or direct IP address)

---

## GET Endpoints

### `GET /`
**Description**: Serve main web UI page

**Response**: HTML page with embedded JavaScript and CSS

**Features**:
- Live SVG clock preview (updates every 90ms)
- Time controls (manual set, browser sync, NTP sync, +/- minute)
- Display settings (brightness, schedule, theme)
- Ring color pickers (6 elements: outer marker, outer fill, seconds, minutes, hours, center)
- Animation controls (second trail, progress ring, chimes, interval animations)
- Auto-brightness controls (mode, min/max, live lux display)
- Network info panel

---

### `GET /time`
**Description**: Get current clock time and status

**Response**:
```json
{
  "hour": 14,
  "minute": 32,
  "second": 18,
  "ntpSynced": true,
  "wifi": true,
  "ip": "192.168.1.100"
}
```

**Polling**: Web UI polls this every 1 second

---

### `GET /temperature`
**Description**: Get temperature sensor reading (future)

**Response** (when sensor available):
```json
{
  "available": true,
  "celsius": 22.5
}
```

**Response** (when sensor not available):
```json
{
  "available": false
}
```

**Note**: Currently placeholder. BME280/SHT31 sensor not yet implemented.

---

### `GET /lux`
**Description**: Get ambient light sensor reading

**Response** (when VEML7700 available):
```json
{
  "available": true,
  "lux": 123.4,
  "autoBrightness": 150
}
```

**Response** (when sensor not available):
```json
{
  "available": false
}
```

**Polling**: Web UI polls this every 2 seconds when auto-brightness mode active

**Fields**:
- `lux`: Current light level (0-120,000 lux range, auto-gain)
- `autoBrightness`: Calculated brightness value (0-255) based on logarithmic curve

---

### `GET /net`
**Description**: Get network information

**Response**:
```json
{
  "hostname": "esp32c3-v3-8inch",
  "ssid": "YOUR_SSID",
  "ip": "192.168.1.100",
  "gateway": "192.168.1.1",
  "subnet": "255.255.255.0",
  "dns": "192.168.1.1",
  "rssi": -45,
  "status": 3
}
```

**Fields**:
- `rssi`: WiFi signal strength in dBm (typical range: -30 to -80)
- `status`: WiFi connection status code (3 = WL_CONNECTED)

---

### `GET /settings`
**Description**: Get all current settings

**Response**:
```json
{
  "dayBrightness": 44,
  "nightBrightness": 5,
  "nightStartHour": 22,
  "nightEndHour": 7,
  "colorTheme": 0,
  "secondTrail": 0,
  "progressSeconds": 0,
  "hourlyChime": 1,
  "statusAnimations": 1,
  "outerMarkerColor": "#4696FF",
  "outerMarkerLevel": 220,
  "outerFillerColor": "#0005C8",
  "outerFillerLevel": 130,
  "secondsColor": "#B4DCFF",
  "secondsLevel": 255,
  "minutesColor": "#FF5000",
  "minutesLevel": 245,
  "hoursColor": "#DC00B4",
  "hoursLevel": 255,
  "centerColor": "#FF3C00",
  "centerLevel": 180,
  "autoBrightnessMode": 1,
  "minAutoBrightness": 10,
  "maxAutoBrightness": 255,
  "quarterAnimation": 3,
  "halfHourAnimation": 1,
  "hourAnimation": 4,
  "intervalAnimationsEnabled": 1
}
```

**Color format**: Hex RGB (`#RRGGBB`)  
**Level values**: 0-255 (brightness intensity for each element)  
**Mode enums**: See Settings Structure section below

---

## POST Endpoints

### `POST /set`
**Description**: Manually set clock time

**Parameters** (application/x-www-form-urlencoded):
- `hour` (int, 0-23, required)
- `minute` (int, 0-59, required)
- `second` (int, 0-59, required)

**Example**:
```
POST /set
Content-Type: application/x-www-form-urlencoded

hour=14&minute=30&second=0
```

**Response**: `200 OK "ok"` or `400 Bad Request "missing hour/minute/second"`

**Side effects**: Triggers STATUS_TIME_SYNC animation (1.2 sec)

---

### `POST /syncBrowser`
**Description**: Sync clock to browser's local time

**Parameters** (application/x-www-form-urlencoded):
- `hour` (int, 0-23, required)
- `minute` (int, 0-59, required)
- `second` (int, 0-59, required)

**Example**:
```javascript
const d = new Date();
fetch('/syncBrowser', {
  method: 'POST',
  headers: {'Content-Type': 'application/x-www-form-urlencoded'},
  body: `hour=${d.getHours()}&minute=${d.getMinutes()}&second=${d.getSeconds()}`
});
```

**Response**: `200 OK "ok"` or `400 Bad Request "missing hour/minute/second"`

**Side effects**: Triggers STATUS_TIME_SYNC animation (1.5 sec)

---

### `POST /syncNtp`
**Description**: Force immediate NTP time sync

**Parameters**: None

**Response**: `200 OK "ok"` or `503 Service Unavailable "ntp unavailable"`

**Side effects**: 
- Triggers STATUS_TIME_SYNC animation (1.5 sec) on success
- Triggers STATUS_WIFI_FAIL animation (1.5 sec) on failure

**Failure conditions**:
- WiFi not connected
- NTP server unreachable
- Time received is before cutoff (< Jan 2024)

---

### `POST /addMinute`
**Description**: Increment time by 1 minute

**Parameters**: None

**Response**: `200 OK "ok"`

**Side effects**: Triggers STATUS_BUTTON animation (0.7 sec)

---

### `POST /subMinute`
**Description**: Decrement time by 1 minute

**Parameters**: None

**Response**: `200 OK "ok"`

**Side effects**: Triggers STATUS_BUTTON animation (0.7 sec)

---

### `POST /previewAnimation`
**Description**: Fire an animation immediately for preview — does not modify saved settings

**Parameters** (application/x-www-form-urlencoded):
- `type` (string, required): `quarter` | `halfhour` | `hour` | `reminder`
- `mode` (int, required): animation index within the type
  - `quarter`: 1-6
  - `halfhour`: 1-7
  - `hour`: 1-10
  - `reminder`: 0-11

**Example**:
```
POST /previewAnimation
Content-Type: application/x-www-form-urlencoded

type=hour&mode=6
```

**Response**: `200 OK "ok"` | `400 Bad Request "<error>"`

**Side effects**: Triggers the specified animation on the LED rings immediately. No settings are written.

---

### `POST /settings`
**Description**: Update display settings

**Parameters** (application/x-www-form-urlencoded, all optional):

**Brightness**:
- `dayBrightness` (int, 0-255)
- `nightBrightness` (int, 0-255)
- `nightStartHour` (int, 0-23)
- `nightEndHour` (int, 0-23)

**Display**:
- `colorTheme` (int, 0-2: 0=Classic, 1=Aqua, 2=Magenta)
- `secondTrail` (int, 0-1: 0=off, 1=on)
- `progressSeconds` (int, 0-1: 0=off, 1=on)
- `hourlyChime` (int, 0-1: 0=off, 1=on)
- `statusAnimations` (int, 0-1: 0=off, 1=on)

**Ring Colors** (hex format `#RRGGBB`):
- `outerMarkerColor`
- `outerFillerColor`
- `secondsColor`
- `minutesColor`
- `hoursColor`
- `centerColor`

**Ring Levels** (int, 0-255):
- `outerMarkerLevel`
- `outerFillerLevel`
- `secondsLevel`
- `minutesLevel`
- `hoursLevel`
- `centerLevel`

**Auto-Brightness**:
- `autoBrightnessMode` (int, 0-2: 0=manual, 1=auto, 2=scheduled)
- `minAutoBrightness` (int, 5-255)
- `maxAutoBrightness` (int, 5-255)

**Time-Interval Animations**:
- `quarterAnimation` (int, 0-6: 0=off, 1=sparkle, 2=pulse, 3=shimmer, 4=laser ping, 5=DNA twist, 6=tick spark)
- `halfHourAnimation` (int, 0-7: 0=off, 1=sweep, 2=flash, 3=tidal, 4=comet, 5=explosion, 6=knight rider, 7=strobe)
- `hourAnimation` (int, 0-10: 0=off, 1-5=original, 6=supernova, 7=matrix, 8=galaxy, 9=wipe, 10=thunder)
- `intervalAnimationsEnabled` (int, 0-1: 0=off, 1=on)

**Animation Style** (added v11):
- `animationPalette` (int; shared 5-option list, v2.27.0: `0`=Golden hour (warm), `1`=Moonlight (cool), `2`=Dawn (soft-warm), `3`=Twilight (muted-cool), `7`=Clock colors — default. Any other value is coerced to `7`. Moods live in `tools/palettes/palettes.json` `mood_palettes`; `7` is ring-mapped from the face colors in firmware)
- `animationSpeed` (int, 1-5: 1=slow, 3=normal, 5=fast)
- `animationBrightness` (int, 50-255)
- `trailLength` (int, 2-12)
- `reminderPalette` (int; **same** 5-option list as `animationPalette`: `0-3` moods, `7`=Clock colors. Independent of `animationPalette` so a nudge can read differently from a chime)

**Example**:
```
POST /settings
Content-Type: application/x-www-form-urlencoded

dayBrightness=200&nightBrightness=20&autoBrightnessMode=1&secondsColor=#FF0000
```

**Response**: `200 OK "ok"`

**Side effects**: 
- Settings saved to EEPROM
- Triggers STATUS_SETTINGS_SAVED animation (1.3 sec)
- Clock display updates immediately with new colors/brightness

---

## Settings Structure (EEPROM v8)

### ClockSettings Struct

```cpp
struct ClockSettings {
  uint8_t magic;                    // 0xC1 (validation byte)
  uint8_t version;                  // 8 (current version)
  
  // Brightness (2 bytes)
  uint8_t dayBrightness;            // 0-255, default 44
  uint8_t nightBrightness;          // 0-255, default 5
  uint8_t nightStartHour;           // 0-23, default 22
  uint8_t nightEndHour;             // 0-23, default 7
  
  // Display (5 bytes)
  uint8_t colorTheme;               // 0-2, default 0
  uint8_t secondTrail;              // 0-1, default 0
  uint8_t progressSeconds;          // 0-1, default 0
  uint8_t hourlyChime;              // 0-1, default 1
  uint8_t statusAnimations;         // 0-1, default 1
  
  // Ring colors (24 bytes: 6 elements × 4 bytes each)
  uint8_t outerMarkerRed, outerMarkerGreen, outerMarkerBlue, outerMarkerLevel;
  uint8_t outerFillerRed, outerFillerGreen, outerFillerBlue, outerFillerLevel;
  uint8_t secondsRed, secondsGreen, secondsBlue, secondsLevel;
  uint8_t minutesRed, minutesGreen, minutesBlue, minutesLevel;
  uint8_t hoursRed, hoursGreen, hoursBlue, hoursLevel;
  uint8_t centerRed, centerGreen, centerBlue, centerLevel;
  
  // Auto-brightness (3 bytes)
  uint8_t autoBrightnessMode;       // 0-2, default 1
  uint8_t minAutoBrightness;        // 5-255, default 10
  uint8_t maxAutoBrightness;        // 5-255, default 255
  
  // Time-interval animations (4 bytes)
  uint8_t quarterAnimation;         // 0-3, default 3 (shimmer)
  uint8_t halfHourAnimation;        // 0-3, default 1 (sweep)
  uint8_t hourAnimation;            // 0-5, default 4 (spiral)
  uint8_t intervalAnimationsEnabled; // 0-1, default 1

  // Focus Reminders (16 bytes, added v8)
  uint8_t focusEnabled;             // 0-1, default 0
  uint8_t focusIntervalMinutes;     // 1-255, default 25
  uint8_t focusStartHour;           // 0-23, default 9
  uint8_t focusEndHour;             // 0-23, default 17
  uint8_t focusDaysMask;            // bitmask Sun(0)…Sat(6), default 0x3E (Mon-Fri)
  uint8_t focusAnimationMode;       // animation to play, default 2 (quarter pulse)
  uint8_t focusReserved[3];         // reserved
};
```

**Total size**: 43 bytes (of 256 byte EEPROM allocation)

### Default Values

**Brightness**:
- Day: 44 (~17%)
- Night: 5 (~2%)
- Night schedule: 22:00 - 07:00

**Colors** (RGB):
- Outer marker: Bright blue-white (70, 150, 255) @ level 220
- Outer filler: Deep royal blue (0, 5, 200) @ level 130
- Seconds: Ice blue (180, 220, 255) @ level 255
- Minutes: Orange (255, 80, 0) @ level 245
- Hours: Hot pink/magenta (220, 0, 180) @ level 255
- Center: Warm orange-red (255, 60, 0) @ level 180

**Animations** (factory defaults):
- Quarter-hour: Bloom Ripple (mode 3)
- Half-hour: Unfurl (mode 1)
- Top of hour: Comet Relay (mode 4)
- Interval animations: Enabled

---

## Error Responses

### `400 Bad Request`
**Causes**:
- Missing required parameters
- Invalid parameter format
- Out-of-range values

**Example**: `POST /set` without `hour` parameter returns:
```
400 Bad Request
missing hour/minute/second
```

### `503 Service Unavailable`
**Causes**:
- NTP sync requested but WiFi disconnected
- NTP server unreachable
- Time received from NTP is invalid

**Example**: `POST /syncNtp` when WiFi offline returns:
```
503 Service Unavailable
ntp unavailable
```

---

## CORS & Security

**CORS**: Not currently implemented. Web UI only accessible from same-origin.

**Authentication**: None. Anyone on local network can access all endpoints.

**Rate limiting**: None. Endpoints can be called unlimited times.

**Future considerations**:
- Basic authentication for `/settings` POST
- CORS headers for cross-origin access
- Rate limiting for NTP sync (max 1/minute)
- API key for external integrations

---

## WebSocket Support

**Status**: Not implemented

**Future**: Live time updates could use WebSocket instead of 1-second polling to reduce HTTP overhead.

---

## MQTT Topics (Future)

**When MQTT support added**:

> Note: topic prefix `chronobloom-clock` below is the planned firmware identifier. Final topic names will be confirmed when MQTT is implemented.

**Subscribe** (commands from Home Assistant):
- `chronobloom-clock/command/brightness` — Set brightness (0-255)
- `chronobloom-clock/command/mode` — Set display mode
- `chronobloom-clock/command/animation` — Trigger specific animation

**Publish** (state to Home Assistant):
- `chronobloom-clock/state` — Current time, brightness, mode (JSON)
- `chronobloom-clock/sensor/lux` — Ambient light level
- `chronobloom-clock/sensor/temperature` — Temperature (when sensor added)

---

## Client Libraries

### JavaScript (Web UI)

**Fetch time**:
```javascript
const response = await fetch('/time');
const data = await response.json();
console.log(`Current time: ${data.hour}:${data.minute}:${data.second}`);
```

**Save settings**:
```javascript
const params = new URLSearchParams();
params.set('dayBrightness', 200);
params.set('secondsColor', '#FF0000');

await fetch('/settings', {
  method: 'POST',
  headers: {'Content-Type': 'application/x-www-form-urlencoded'},
  body: params.toString()
});
```

### Python (Home Automation)

**Get current time**:
```python
import requests

response = requests.get('http://esp32c3-v3-8inch.local/time')
data = response.json()
print(f"Current time: {data['hour']}:{data['minute']}:{data['second']}")
```

**Set brightness**:
```python
requests.post('http://esp32c3-v3-8inch.local/settings', data={
  'dayBrightness': 200,
  'nightBrightness': 10
})
```

### Curl (Command Line)

**Sync to NTP**:
```bash
curl -X POST http://esp32c3-v3-8inch.local/syncNtp
```

**Get lux reading**:
```bash
curl http://esp32c3-v3-8inch.local/lux
```

**Set time**:
```bash
curl -X POST http://esp32c3-v3-8inch.local/set \
  -d "hour=14&minute=30&second=0"
```

---

## WiFi Provisioning

### First-Boot Portal

**Behavior**:
- On first boot, if no saved WiFi credentials exist in EEPROM, device opens a WiFi access point
- SSID: `esp32c3-clock-setup` (no password required)
- IP address: `192.168.4.1`
- Portal timeout: 120 seconds (default)

### WiFi AP Fallback

If all STA connection attempts fail (wrong password, network unavailable), the device starts a software AP and runs the full web server at `192.168.4.1`. Clock display, web UI, and settings are all functional in AP mode. NTP, mDNS, and OTA are skipped until a STA connection is established.

**Portal page**:
1. Displays list of available WiFi networks (SSIDs)
2. User selects their SSID and enters password
3. Device stores credentials in EEPROM (WiFiManager library manages storage)
4. Device connects to saved network
5. Portal closes automatically

**Credential Storage**:
- WiFi SSID and password stored in ESP32 EEPROM (separate from clock settings)
- Persists across reboots
- To reset: uncomment `wm.resetSettings();` in setupWiFi(), rebuild, flash once, then comment out and rebuild

**Error Handling**:
- If connection fails after 120 seconds, portal reappears on next boot
- Invalid credentials (password changed in router): portal reappears on next boot
- WiFi network unavailable: portal reappears on next boot

---

## OTA (Over-The-Air) Firmware Updates

### Overview

After initial USB flash, firmware updates can be deployed over WiFi using ArduinoOTA protocol on port 3232.

### OTA Protocol Details

**Port**: 3232 (TCP)
**Authentication**: Password (`iris_ota_2026` — current firmware identifier, not renamed with project)
**Protocol**: ArduinoOTA binary protocol (not HTTP/HTTPS)

### Update Command

```powershell
# Build new firmware
pio run -e esp32c3_v3_8inch

# Upload via OTA to device at mDNS hostname
pio run -e esp32c3_v3_8inch -t upload --upload-port esp32c3-v3-8inch.local:3232

# Or upload to device by IP address
pio run -e esp32c3_v3_8inch -t upload --upload-port 192.168.1.x:3232
```

### Update Flow

1. **Build phase**: PlatformIO compiles firmware (.bin file)
2. **Connection phase**: PlatformIO connects to device on port 3232, authenticates with password
3. **Upload phase**: Binary streamed to device (shows progress % in serial monitor if connected)
4. **Flash phase**: Device writes new firmware to flash memory
5. **Verify phase**: Device verifies flash integrity
6. **Reboot phase**: Device automatically reboots with new firmware
7. **Status animations**: 
   - During upload: inner ring shows blue
   - On success: inner ring shows green
   - On failure: inner ring shows red

### Status Indicators

**Serial output during OTA**:
```
[OTA] Update starting...
[OTA] Progress: 45000/703962 (6.4%)
[OTA] Progress: 90000/703962 (12.8%)
...
[OTA] Update complete, rebooting...
```

### Troubleshooting OTA

**Port 3232 not responding**:
- Device must be on WiFi (test with `ping esp32c3-v3-8inch.local`)
- Check firewall: port 3232 may be blocked
- Restart device and try again (OTA server initializes ~30-60s after boot)
- Verify mDNS is working: ping should resolve hostname to IP

**Network interruption during upload**:
- Blue animation stops, device may reboot
- Try again with stronger WiFi signal (move closer to router)
- Check router is not dropping connection on specific devices

**Authentication failure**:
- Verify OTA password is `iris_ota_2026` in setupOTA() function
- If changed, rebuild and flash via USB with new password
