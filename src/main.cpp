#include <Arduino.h>
#include <EEPROM.h>
#if !defined(ESP32)
#error "ChronoBloom v3 requires an ESP32-family Arduino core."
#endif
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>
#include <esp_sntp.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include "esp_task_wdt.h"
#include <Preferences.h>
#include "web_html.h"
#if LUX_SENSOR_ENABLED
#include <Adafruit_VEML7700.h>
#endif

using ClockWebServer = WebServer;

// ===================== Build-time configuration =====================
#ifndef LED_DATA_PIN
#define LED_DATA_PIN 10
#endif

#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN -1
#endif

#ifndef CLOCK_PIXEL_COUNT
#define CLOCK_PIXEL_COUNT 98
#endif

#ifndef CENTER_PIXEL_ENABLED
#define CENTER_PIXEL_ENABLED 1
#endif

#ifndef CENTER_PIXEL_INDEX
#define CENTER_PIXEL_INDEX 1
#endif

#ifndef CENTER_PIXEL_SEPARATE_OUTPUT
#define CENTER_PIXEL_SEPARATE_OUTPUT 0
#endif

#ifndef CENTER_PIXEL_PIN
#define CENTER_PIXEL_PIN -1
#endif

#ifndef CENTER_PIXEL_STRIP_COUNT
#define CENTER_PIXEL_STRIP_COUNT 1
#endif

#ifndef RING_PIXEL_OFFSET
#define RING_PIXEL_OFFSET 2
#endif

#ifndef DEFAULT_OUTER_RING_OFFSET
#define DEFAULT_OUTER_RING_OFFSET 0
#endif

// Sacrificial pixel: an extra WS2812B ahead of the rings that is kept dark and
// only re-drives the 3.3V data line at 5V logic. Vestigial troubleshooting
// remnant present in the maintainer's original 8" build; replicable builds
// leave it disabled. See the [led_chain] note in platformio.ini.
#ifndef SACRIFICIAL_PIXEL_ENABLED
#define SACRIFICIAL_PIXEL_ENABLED 0
#endif

#ifndef SACRIFICIAL_PIXEL_INDEX
#define SACRIFICIAL_PIXEL_INDEX 0
#endif

#ifndef ENABLE_WIFI_UI
#define ENABLE_WIFI_UI 1
#endif

#ifndef ENABLE_NTP
#define ENABLE_NTP 1
#endif

#ifndef TEMP_SENSOR_ENABLED
#define TEMP_SENSOR_ENABLED 0
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "clock-ssid"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "clock-password"
#endif

#ifndef DEVICE_HOSTNAME
#define DEVICE_HOSTNAME "esp32c3-ringclock"
#endif

#ifndef DEVICE_TITLE
#define DEVICE_TITLE "ChronoBloom v3"
#endif

#ifndef WIFI_CONNECT_TIMEOUT_MS
#define WIFI_CONNECT_TIMEOUT_MS 20000
#endif

#ifndef NTP_TIMEZONE_TZ
// Mountain time with US daylight saving rules.
#define NTP_TIMEZONE_TZ "MST7MDT,M3.2.0,M11.1.0"
#endif

#ifndef BUTTON_UP_PIN
#define BUTTON_UP_PIN 5
#endif

#ifndef BUTTON_DOWN_PIN
#define BUTTON_DOWN_PIN 9
#endif

// Shared logical ring order. RING_PIXEL_OFFSET selects where logical outer LED 0
// lands in the physical chain for each clock variant.
struct RingConfig {
  uint8_t count;
  uint16_t offset;
  bool clockwise;
};

constexpr RingConfig RING_OUTER_60 = {60, RING_PIXEL_OFFSET, true};
constexpr RingConfig RING_MIDDLE_24 = {24, RING_PIXEL_OFFSET + 60, true};
constexpr RingConfig RING_INNER_12 = {12, RING_PIXEL_OFFSET + 84, true};

#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT && (CENTER_PIXEL_PIN < 0)
#error "CENTER_PIXEL_PIN must be set when CENTER_PIXEL_SEPARATE_OUTPUT is enabled."
#endif

#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT && (CENTER_PIXEL_INDEX >= CENTER_PIXEL_STRIP_COUNT)
#error "CENTER_PIXEL_INDEX must be inside CENTER_PIXEL_STRIP_COUNT when CENTER_PIXEL_SEPARATE_OUTPUT is enabled."
#endif

#if CENTER_PIXEL_ENABLED && !CENTER_PIXEL_SEPARATE_OUTPUT && (CENTER_PIXEL_INDEX >= CLOCK_PIXEL_COUNT)
#error "CENTER_PIXEL_INDEX must be inside CLOCK_PIXEL_COUNT when CENTER_PIXEL_ENABLED is set."
#endif

#if SACRIFICIAL_PIXEL_ENABLED && (SACRIFICIAL_PIXEL_INDEX >= CLOCK_PIXEL_COUNT)
#error "SACRIFICIAL_PIXEL_INDEX must be inside CLOCK_PIXEL_COUNT when SACRIFICIAL_PIXEL_ENABLED is set."
#endif

#if (RING_PIXEL_OFFSET + 96) > CLOCK_PIXEL_COUNT
#error "RING_PIXEL_OFFSET leaves too few pixels for the 60+24+12 ring chain."
#endif

#if (DEFAULT_OUTER_RING_OFFSET < 0) || (DEFAULT_OUTER_RING_OFFSET >= 60)
#error "DEFAULT_OUTER_RING_OFFSET must be in the 0-59 LED range."
#endif

#if CENTER_PIXEL_ENABLED && !CENTER_PIXEL_SEPARATE_OUTPUT && (CENTER_PIXEL_INDEX >= RING_PIXEL_OFFSET) && (CENTER_PIXEL_INDEX < (RING_PIXEL_OFFSET + 96))
#error "CENTER_PIXEL_INDEX overlaps the ring pixel range."
#endif

#if SACRIFICIAL_PIXEL_ENABLED && (SACRIFICIAL_PIXEL_INDEX >= RING_PIXEL_OFFSET) && (SACRIFICIAL_PIXEL_INDEX < (RING_PIXEL_OFFSET + 96))
#error "SACRIFICIAL_PIXEL_INDEX overlaps the ring pixel range."
#endif

#if CENTER_PIXEL_ENABLED && !CENTER_PIXEL_SEPARATE_OUTPUT && SACRIFICIAL_PIXEL_ENABLED && (CENTER_PIXEL_INDEX == SACRIFICIAL_PIXEL_INDEX)
#error "CENTER_PIXEL_INDEX and SACRIFICIAL_PIXEL_INDEX must be different physical pixels."
#endif

static void configureWiFiHostname() {
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  if (!WiFi.setHostname(DEVICE_HOSTNAME)) {
    Serial.println("Wi-Fi hostname set failed");
  }
}

static void writeStatusLed(bool on) {
#if STATUS_LED_PIN >= 0
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
#else
  (void)on;
#endif
}

// ===================== Ambient light sensor =====================
#if LUX_SENSOR_ENABLED
#include <Wire.h>
#endif

class LuxSensor {
 public:
  void begin() {
#if LUX_SENSOR_ENABLED
    if (!veml_.begin()) {
      Serial.println("VEML7700 not found on I2C");
      available_ = false;
      return;
    }
    veml_.setGain(VEML7700_GAIN_1);
    veml_.setIntegrationTime(VEML7700_IT_100MS);
    veml_.enable(true);
    available_ = true;
    Serial.println("VEML7700 initialized");
#endif
  }

  bool available() const { return available_; }

  float lux() {
#if LUX_SENSOR_ENABLED
    if (!available_) return -1.0f;
    uint32_t nowMs = millis();
    if (nowMs - lastReadMs_ < 120) return luxAvg_;
    lastReadMs_ = nowMs;

    // Wait 400ms after a gain change (2 full 100ms integration cycles + margin)
    // before trusting the reading. Short settling was the primary cause of
    // gain oscillation: reading was still invalid when next switch decision ran.
    if (nowMs - lastGainChangeMs_ < 400) return luxAvg_;

    // NOWAIT is critical for animation smoothness. The default readLux()
    // (VEML_LUX_NORMAL) calls readWait(), which does a blocking delay() of up
    // to 2× the integration time (2×100ms = 200ms) before reading the ALS
    // register. Because lux() is on the animation render path
    // (renderAnimFrame → effectiveBrightness → autoBrightnessCached → lux),
    // that delay stalled the whole render loop — loop() captures `now` once
    // at the top, so a blocked iteration drags every animation frame with it,
    // throttling ~40fps animations down to ~8fps (galaxy spin "flashed"
    // instead of spinning; comet relay skipped 8-10 LEDs between frames).
    // The sensor integrates continuously every 100ms and we only read here at
    // most every 120ms, so the register already holds a fresh value — there
    // is nothing to wait for. NOWAIT reads it directly (~1-3ms I2C).
    float reading = veml_.readLux(VEML_LUX_NORMAL_NOWAIT);
    if (reading < 0.0f) return luxAvg_;  // discard library error returns

    // 3-state gain machine with hysteresis bands to prevent oscillation.
    // Previous code had no hysteresis: GAIN_1_8 -> GAIN_2 direct jump
    // caused reading to spike -> GAIN_1_8 again -> flicker loop.
    // Gain thresholds empirically set to avoid oscillation at lux boundaries.
    // GAIN_2->GAIN_1: >200 lux. GAIN_1->GAIN_2: <50 lux.
    // GAIN_1->GAIN_1_8: >900 lux. GAIN_1_8->GAIN_1: <300 lux.
    // Settle lockout: 400ms after any gain change (applies to the NEXT reading only —
    // `reading` above was captured at the still-current gain, so it's valid and gets
    // folded in below instead of being thrown away for a full lockout window).
    uint8_t g = veml_.getGain();
    bool gainChanged = false;
    if (g == VEML7700_GAIN_2 && reading > 200.0f) {
      // High-sensitivity: exit when scene is bright enough (>200 hysteresis vs <50 entry)
      veml_.setGain(VEML7700_GAIN_1);
      gainChanged = true;
    } else if (g == VEML7700_GAIN_1 && reading > 0.0f && reading < 50.0f) {
      // Normal: enter high-sensitivity for dark scenes
      veml_.setGain(VEML7700_GAIN_2);
      gainChanged = true;
    } else if (g == VEML7700_GAIN_1 && reading > 900.0f) {
      // Normal: enter low-sensitivity for very bright light
      veml_.setGain(VEML7700_GAIN_1_8);
      gainChanged = true;
    } else if (g == VEML7700_GAIN_1_8 && reading < 300.0f) {
      // Low-sensitivity: return to normal (never jump directly to GAIN_2)
      veml_.setGain(VEML7700_GAIN_1);
      gainChanged = true;
    }

    // Smoothing alpha 0.35: tracks real (esp. large) lux swings within
    // roughly 1-2s instead of the old 0.15 (~4s to settle a full-scale jump).
    // VEML7700 read noise is small relative to real scene changes, so this
    // stays visually stable while responding far faster.
    luxAvg_ = luxAvg_ * 0.65f + reading * 0.35f;

    if (gainChanged) {
      lastGainChangeMs_ = nowMs;
    }
    return luxAvg_;
#else
    return -1.0f;
#endif
  }

  // Returns the current ramped brightness (what LEDs actually show).
  // Use this for web UI display so it matches hardware state.
  uint8_t autoBrightness() {
    return autoBrightnessCached(millis());
  }

  // Returns the brightness target (pre-ramp) for diagnostic display.
  uint8_t autoBrightnessTarget() const {
    return (uint8_t)cachedBrightness_;
  }

  // True while rampedBrightness_ is still catching up to cachedBrightness_.
  // Used to drive extra render frames so the ramp is actually visible instead
  // of being starved by the once-per-second idle render cadence.
  bool brightnessRampInProgress() const {
    return fabs(cachedBrightness_ - rampedBrightness_) >= 1.0f;
  }

  void setLuxOverride(float lux) { luxOverrideActive_ = true; luxOverrideValue_ = lux; }
  void clearLuxOverride()        { luxOverrideActive_ = false; }

  // Dark-room detector with dwell + hysteresis: 30 s below 0.3 lux enters
  // "dark"; rising above 2.0 lux exits immediately. The wide band prevents
  // flapping at the threshold (e.g. streetlight through curtains).
  bool roomDark(uint32_t nowMs) {
#if LUX_SENSOR_ENABLED
    if (!available_) return false;
    const float lx = luxOverrideActive_ ? luxOverrideValue_ : lux();
    if (lx >= 0.0f && lx < 0.3f) {
      if (darkSinceMs_ == 0) darkSinceMs_ = nowMs ? nowMs : 1;
      if (nowMs - darkSinceMs_ >= 30000UL) isDark_ = true;
    } else if (lx > 2.0f) {
      darkSinceMs_ = 0;
      isDark_ = false;
    } else if (!isDark_) {
      darkSinceMs_ = 0;  // inside hysteresis band: hold state, restart dwell
    }
    return isDark_;
#else
    (void)nowMs;
    return false;
#endif
  }

  bool displaySleeping() const { return isDark_; }

  uint8_t autoBrightnessCached(uint32_t nowMs) {
#if LUX_SENSOR_ENABLED
    if (!available_) return 128;

    // Refresh the lux-derived target every 150ms. This used to be 500ms,
    // which stacked with the ramp below to make big lux swings take many
    // seconds to even start moving toward the new target.
    if (nowMs - lastBrightnessMs_ >= 150) {
      float lx = luxOverrideActive_ ? luxOverrideValue_ : lux();
      if (lx >= 0.0f) {
        // Lux ceiling: VEML7700 max useful range ~10000 lux. log10(10001) normalises to [0,1].
        // Output range: 15 (minimum visible) to 255 (full). 240 = 255 - 15.
        static constexpr float LUX_CEILING      = 10000.0f;
        static constexpr float BRIGHTNESS_MIN   = 15.0f;
        static constexpr float BRIGHTNESS_RANGE = 240.0f; // 255 - 15
        float normalized = log10(max(0.1f, lx) + 1.0f) / log10(LUX_CEILING + 1.0f);
        normalized = powf(normalized, 1.5f);
        cachedBrightness_ = constrain(normalized * BRIGHTNESS_RANGE + BRIGHTNESS_MIN,
                                      BRIGHTNESS_MIN, 255.0f);
      }
      lastBrightnessMs_ = nowMs;
    }

    // On first call, snap rampedBrightness_ to target to avoid a ramp from 128
    if (lastRampMs_ == 0) {
      rampedBrightness_ = cachedBrightness_;
      lastRampMs_ = nowMs;
      return (uint8_t)rampedBrightness_;
    }

    // Exponential (time-constant) ramp toward target instead of a fixed
    // units/sec linear ramp. A linear ramp moves at the same speed whether
    // the gap is 5 units or 200 units, so large swings (e.g. bright->dark)
    // crawled for ~5s and, when only sampled once/sec, looked like discrete
    // steps. An exponential ramp moves fast when far from target and eases
    // in as it approaches, so big changes resolve quickly (~63% of the way
    // in one time constant) while staying visually smooth, no linear
    // "cap" artifact, and no per-frame stall-jump risk.
    static constexpr float kRampTauMs = 220.0f;  // ~63% converged per 220ms
    uint32_t dt = nowMs - lastRampMs_;
    if (dt > 1000) dt = 1000;  // guard against a long stall producing an overflow-y factor
    lastRampMs_ = nowMs;
    float alpha = 1.0f - expf(-(float)dt / kRampTauMs);
    rampedBrightness_ = constrain(rampedBrightness_ + (cachedBrightness_ - rampedBrightness_) * alpha,
                                   15.0f, 255.0f);

    return (uint8_t)rampedBrightness_;
#else
    return 128;
#endif
  }

 private:
#if LUX_SENSOR_ENABLED
  Adafruit_VEML7700 veml_;
#endif
  bool available_ = false;
  float luxAvg_ = 100.0f;
  uint32_t lastGainChangeMs_ = 0;
  uint32_t lastReadMs_ = 0;
  float cachedBrightness_ = 128.0f;  // target (lux-derived, unsmoothed)
  float rampedBrightness_ = 128.0f;  // actual output (smoothed toward target)
  uint32_t lastBrightnessMs_ = 0;
  uint32_t lastRampMs_ = 0;
  bool luxOverrideActive_ = false;
  float luxOverrideValue_ = 0.0f;
  uint32_t darkSinceMs_ = 0;
  bool isDark_ = false;
};

// ===================== Persistent settings =====================
struct ClockSettings {
  uint8_t magic;
  uint8_t version;
  uint8_t dayBrightness;
  uint8_t nightBrightness;
  uint8_t nightStartHour;
  uint8_t nightEndHour;
  uint8_t colorTheme;
  uint8_t secondTrail;
  uint8_t progressSeconds;
  uint8_t hourlyChime;
  uint8_t statusAnimations;
  uint8_t outerMarkerRed;
  uint8_t outerMarkerGreen;
  uint8_t outerMarkerBlue;
  uint8_t outerMarkerLevel;
  uint8_t outerFillerRed;
  uint8_t outerFillerGreen;
  uint8_t outerFillerBlue;
  uint8_t outerFillerLevel;
  uint8_t secondsRed;
  uint8_t secondsGreen;
  uint8_t secondsBlue;
  uint8_t secondsLevel;
  uint8_t minutesRed;
  uint8_t minutesGreen;
  uint8_t minutesBlue;
  uint8_t minutesLevel;
  uint8_t hoursRed;
  uint8_t hoursGreen;
  uint8_t hoursBlue;
  uint8_t hoursLevel;
  uint8_t middleFaceRed;
  uint8_t middleFaceGreen;
  uint8_t middleFaceBlue;
  uint8_t innerFaceRed;
  uint8_t innerFaceGreen;
  uint8_t innerFaceBlue;
  uint8_t innerHourRed;
  uint8_t innerHourGreen;
  uint8_t innerHourBlue;
  uint8_t innerHourLevel;
  uint8_t centerRed;
  uint8_t centerGreen;
  uint8_t centerBlue;
  uint8_t centerLevel;
  // Auto-brightness
  uint8_t autoBrightnessMode;
  uint8_t minAutoBrightness;
  uint8_t maxAutoBrightness;
  // Time-interval animations
  uint8_t quarterAnimation;
  uint8_t halfHourAnimation;
  uint8_t hourAnimation;
  uint8_t intervalAnimationsEnabled;
  // FOCUS REMINDERS MVP (v8) - 16 bytes total (13 used + 3 reserved)
  uint8_t focusReminder_enabled;        // Master enable/disable
  uint8_t focusReminder_startHour;      // Window start (0-23)
  uint8_t focusReminder_endHour;        // Window end (0-23)
  uint16_t focusReminder_intervalMinutes; // Repeat interval in minutes (1-1440)
  uint8_t focusReminder_daysMask;       // Bitmask Sun=0, Sat=6
  uint8_t focusReminder_animation;      // Animation type (0-5)
  uint8_t focusReminder_durationSeconds; // Duration (1-60) - reserved for v2
  uint32_t focusReminder_lastFireMs;    // Last fire timestamp (millis)
  uint8_t outerRingOffset;              // 0-59: clockwise LED rotation applied to all rings at render time
  // Animation customization (added v11)
  uint8_t animationPalette;     // 0=Rainbow,1=Fire,2=Ocean,3=Forest,4=Candy,5=Neon,6=Monochrome,7=Clock
  uint8_t animationSpeed;       // 1-5 (1=slow/dreamy, 3=normal, 5=fast)
  uint8_t animationBrightness;  // 50-255 peak brightness during animations
  uint8_t trailLength;          // 2-12 LEDs (chase/sweep trail length)
  uint8_t reminderPalette;      // 0=Amber,1=Red,2=Magenta,3=Cyan (reminder animations only)
  uint8_t outerRingBrightness;  // 0-100 percent multiplier applied to outer ring colors
  uint8_t middleFaceScale;      // 0-255, face ambient level for middle ring (24 LED)
  uint8_t innerFaceScale;       // 0-255, face ambient level for inner ring (12 LED)
  uint8_t darkRoomOff;          // 0/1 (added v15): blank all LEDs when room is pitch black (auto mode only)
};

constexpr uint8_t SETTINGS_MAGIC = 0xC1;
constexpr uint8_t SETTINGS_VERSION = 15;
constexpr uint8_t USER_DEFAULTS_MAGIC = 0xD2;       // marks a user-saved defaults block
constexpr uint16_t USER_DEFAULTS_EEPROM_OFFSET = 128; // second EEPROM slot; must be > sizeof(ClockSettings)
constexpr size_t EEPROM_BYTES = 256;

// Layout guards: growing ClockSettings past either boundary would silently
// corrupt the user-defaults slot (or run off the end of the EEPROM window).
static_assert(sizeof(ClockSettings) <= USER_DEFAULTS_EEPROM_OFFSET,
              "ClockSettings overlaps the user-defaults EEPROM slot");
static_assert(USER_DEFAULTS_EEPROM_OFFSET + sizeof(ClockSettings) <= EEPROM_BYTES,
              "user-defaults slot runs past EEPROM_BYTES");

class SettingsStore {
 public:
  void begin() {
    EEPROM.begin(EEPROM_BYTES);
    EEPROM.get(0, settings_);
    if (valid(settings_)) {
      // Magic+version match: repair individual out-of-range fields instead of
      // wiping every user setting because one byte went stale or corrupt.
      ClockSettings repaired = sanitize(settings_);
      if (memcmp(&repaired, &settings_, sizeof(ClockSettings)) != 0) {
        settings_ = repaired;
        save();
      }
    } else {
      settings_ = defaults();
      save();
    }
  }

  const ClockSettings &get() const { return settings_; }

  void update(const ClockSettings &settings) {
    settings_ = sanitize(settings);
    save();
    settingsSaveCount_++;
  }

  uint16_t saveCount() const { return settingsSaveCount_; }

  bool hasUserDefaults() const {
    return EEPROM.read(USER_DEFAULTS_EEPROM_OFFSET) == USER_DEFAULTS_MAGIC &&
           EEPROM.read(USER_DEFAULTS_EEPROM_OFFSET + 1) == SETTINGS_VERSION;
  }

  void saveAsUserDefaults() {
    ClockSettings ud = settings_;
    ud.magic = USER_DEFAULTS_MAGIC;
    EEPROM.put(USER_DEFAULTS_EEPROM_OFFSET, ud);
    EEPROM.commit();
  }

  void resetToDefaults() {
    if (hasUserDefaults()) {
      ClockSettings ud;
      EEPROM.get(USER_DEFAULTS_EEPROM_OFFSET, ud);
      if (ud.version == SETTINGS_VERSION) {
        ud.magic = SETTINGS_MAGIC;
        ud.focusReminder_lastFireMs = 0;  // runtime timestamp — reset so reminder doesn't fire immediately
        settings_ = sanitize(ud);
      } else {
        settings_ = defaults();
      }
    } else {
      settings_ = defaults();
    }
    save();
    settingsSaveCount_++;
  }

 private:
  static ClockSettings defaults() {
    return {SETTINGS_MAGIC, SETTINGS_VERSION, 44, 5, 22, 7, 0, 0, 0, 1, 1,
            110, 185, 255, 210,   // outerMarker: periwinkle blue
            0,   8,   200, 145,   // outerFiller: deep royal blue
            100, 255, 180, 230,   // seconds:     mint green
            255, 100, 0,   220,   // minutes:     orange
            220, 0,   180, 255,   // hours:       hot pink/magenta (middle 24h hand; restores v<2.4.7 default)
            220, 0,   180,        // middleFace:  hot pink — face glow matches hand (v<2.4.7: face used hoursColor)
            255, 60,  0,          // innerFace:   warm orange — matches v<2.4.7 inner face glow (was centerColor)
            220, 0,   180, 255,   // innerHour:   hot pink — unified with middle hand for visual continuity
            255, 60,  0,   180,   // center:      warm orange-red
            1,   10,  255,        // autoBrightness: mode=auto, min=10, max=255
            3,   1,   4,   1,     // animations: shimmer, sweep, spiral, enabled
            0,   8,   22,   60,   0, 0, 60, 60,  // focusReminder: disabled, 08-22h, 60min, no days, quarter anim
            DEFAULT_OUTER_RING_OFFSET,   // outerRingOffset: build-time default rotation
            7, 3, 200, 4, 0,      // animPalette (7 = clock colors), animSpeed, animBrightness, trailLength, reminderPalette
            90, 55, 55,  // outerRingBrightness, middleFaceScale, innerFaceScale (restores v<2.4.7 values)
            0};          // darkRoomOff: disabled by default
  }

  // Layout identity only. Field-level range repair is sanitize()'s job —
  // begin() runs every load through sanitize(), so a single stale byte no
  // longer wipes the whole settings block.
  static bool valid(const ClockSettings &settings) {
    return settings.magic == SETTINGS_MAGIC && settings.version == SETTINGS_VERSION;
  }

  static ClockSettings sanitize(ClockSettings settings) {
    settings.magic = SETTINGS_MAGIC;
    settings.version = SETTINGS_VERSION;
    settings.nightStartHour %= 24;
    settings.nightEndHour %= 24;
    if (settings.colorTheme > 2) settings.colorTheme = 0;
    settings.secondTrail = settings.secondTrail ? 1 : 0;
    settings.progressSeconds = settings.progressSeconds ? 1 : 0;
    settings.hourlyChime = settings.hourlyChime ? 1 : 0;
    settings.statusAnimations = settings.statusAnimations ? 1 : 0;
    if (settings.autoBrightnessMode > 2) settings.autoBrightnessMode = 1;
    if (settings.minAutoBrightness > settings.maxAutoBrightness) {
      settings.minAutoBrightness = 10;
      settings.maxAutoBrightness = 255;
    }
    // Mode maxima match the v2.4.0 animation set: quarter 1-3, half 1-3,
    // hour 1-5. Values beyond these were legacy modes whose implementations
    // were removed; the old loose bounds made trigger*Animation() index past
    // the end of each block (quarter 4 played half-hour 1, hour 6-10 nothing).
    if (settings.quarterAnimation > 3) settings.quarterAnimation = 0;
    if (settings.halfHourAnimation > 3) settings.halfHourAnimation = 0;
    if (settings.hourAnimation > 5) settings.hourAnimation = 0;
    settings.intervalAnimationsEnabled = settings.intervalAnimationsEnabled ? 1 : 0;
    settings.focusReminder_enabled = settings.focusReminder_enabled ? 1 : 0;
    if (settings.focusReminder_startHour >= 24) settings.focusReminder_startHour = 8;
    if (settings.focusReminder_endHour >= 24) settings.focusReminder_endHour = 22;
    if (settings.focusReminder_intervalMinutes < 1) settings.focusReminder_intervalMinutes = 60;
    if (settings.focusReminder_intervalMinutes > 1440) settings.focusReminder_intervalMinutes = 1440;
    settings.focusReminder_daysMask &= 127;  // Mask to 7 bits (Sun-Sat)
    if (settings.focusReminder_animation > 10) settings.focusReminder_animation = 0;
    if (settings.focusReminder_durationSeconds < 1) settings.focusReminder_durationSeconds = 60;
    if (settings.focusReminder_durationSeconds > 60) settings.focusReminder_durationSeconds = 60;
    if (settings.outerRingOffset >= 60) settings.outerRingOffset = DEFAULT_OUTER_RING_OFFSET;
    if (settings.animationPalette > 7) settings.animationPalette = 0;
    if (settings.animationSpeed < 1 || settings.animationSpeed > 5) settings.animationSpeed = 3;
    if (settings.animationBrightness < 50) settings.animationBrightness = 200;
    if (settings.trailLength < 2 || settings.trailLength > 12) settings.trailLength = 4;
    if (settings.reminderPalette > 3) settings.reminderPalette = 0;
    if (settings.outerRingBrightness > 100) settings.outerRingBrightness = 90;
    settings.darkRoomOff = settings.darkRoomOff ? 1 : 0;
    if (settings.dayBrightness < 5) settings.dayBrightness = 44;
    if (settings.nightBrightness < 1) settings.nightBrightness = 5;
    return settings;
  }

  void save() {
    EEPROM.put(0, settings_);
    EEPROM.commit();
  }

  ClockSettings settings_ = defaults();
  uint16_t settingsSaveCount_ = 0;
};

// ===================== Time model =====================
struct ClockTime {
  uint8_t hour;   // 0-23
  uint8_t minute; // 0-59
  uint8_t second; // 0-59
};

class TimeModel {
 public:
  ClockTime get() {
    noInterrupts();
    ClockTime copy = time_;
    interrupts();
    return copy;
  }

  void set(uint8_t hour, uint8_t minute, uint8_t second) {
    if (hour > 23 || minute > 59 || second > 59) return;
    noInterrupts();
    time_.hour = hour;
    time_.minute = minute;
    time_.second = second;
    dirty_ = true;
    interrupts();
  }

  void tickOneSecond() {
    noInterrupts();
    incrementSecondNoLock();
    dirty_ = true;
    interrupts();
  }

  void addMinutes(int delta) {
    noInterrupts();
    int total = static_cast<int>(time_.hour) * 60 + time_.minute + delta;
    const int dayMinutes = 24 * 60;
    total %= dayMinutes;
    if (total < 0) total += dayMinutes;
    time_.hour = total / 60;
    time_.minute = total % 60;
    dirty_ = true;
    interrupts();
  }

  void addHours(int delta) {
    noInterrupts();
    int h = (static_cast<int>(time_.hour) + delta) % 24;
    if (h < 0) h += 24;
    time_.hour = static_cast<uint8_t>(h);
    dirty_ = true;
    interrupts();
  }

  void markDirty() {
    noInterrupts();
    dirty_ = true;
    interrupts();
  }

  bool consumeDirty() {
    noInterrupts();
    bool changed = dirty_;
    dirty_ = false;
    interrupts();
    return changed;
  }

 private:
  void incrementSecondNoLock() {
    time_.second++;
    if (time_.second > 59) {
      time_.second = 0;
      time_.minute++;
      if (time_.minute > 59) {
        time_.minute = 0;
        time_.hour = (time_.hour + 1) % 24;
      }
    }
  }

  ClockTime time_ = {12, 0, 0};
  volatile bool dirty_ = true;
};

// ===================== LED renderer =====================
enum AnimPhase : uint8_t {
  ANIM_IDLE,
  ANIM_Q1,   // Quarter: sparkle burst
  ANIM_Q2,   // Quarter: pulse markers
  ANIM_Q3,   // Quarter: ring shimmer
  ANIM_H1,   // Half: rainbow sweep
  ANIM_H2,   // Half: dual flash
  ANIM_H3,   // Half: tidal pulse
  ANIM_HR1,  // Hour: chime sweep
  ANIM_HR2,  // Hour: firework burst
  ANIM_HR3,  // Hour: zenith cascade
  ANIM_HR4,  // Hour: rainbow spiral
  ANIM_HR5,  // Hour: breathing mandala
  // Reminder animations
  ANIM_REM1, // Reminder: amber pulse
  ANIM_REM2, // Reminder: attention ring
  ANIM_REM3, // Reminder: heartbeat
  ANIM_REM4, // Reminder: sunrise wake
  ANIM_REM5  // Reminder: campfire flicker
};

enum StatusMode : uint8_t {
  STATUS_NONE,
  STATUS_WIFI_CONNECTING,
  STATUS_WIFI_OK,
  STATUS_WIFI_FAIL,
  STATUS_BUTTON,
  STATUS_TIME_SYNC,
  STATUS_SETTINGS_SAVED,
  STATUS_OTA_UPDATE,
  STATUS_OTA_SUCCESS,
  STATUS_OTA_FAILED
};

class ClockRenderer {
 public:
  ClockRenderer(Adafruit_NeoPixel &strip, SettingsStore &settings,
                Adafruit_NeoPixel *centerStrip = nullptr)
      : strip_(strip), settings_(settings), centerStrip_(centerStrip),
        lux_(nullptr) {}

  void begin() {
    strip_.begin();
    strip_.setBrightness(settings_.get().dayBrightness);
    strip_.clear();
    strip_.show();
#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT
    if (centerStrip_) {
      centerStrip_->begin();
      centerStrip_->setBrightness(settings_.get().dayBrightness);
      centerStrip_->clear();
      centerStrip_->show();
    }
#endif
  }

  void setLuxSensor(LuxSensor *lux) { lux_ = lux; }

  void setStatus(StatusMode mode, uint16_t durationMs) {
    if (!settings_.get().statusAnimations) return;
    statusMode_ = mode;
    statusUntilMs_ = millis() + durationMs;
    lastAnimationMs_ = 0;
  }

  void triggerQuarterAnimation(uint32_t now) {
    const uint8_t mode = settings_.get().quarterAnimation;
    if (mode == 0) return;
    animPhase_ = static_cast<AnimPhase>(ANIM_Q1 + mode - 1);
    animStartMs_ = now;
    animStep_ = 255;  // force first-frame update
    animHue_ = 0;
    lastAnimSource_ = "quarter"; lastAnimMode_ = mode;
  }

  void triggerHalfHourAnimation(uint32_t now) {
    const uint8_t mode = settings_.get().halfHourAnimation;
    if (mode == 0) return;
    animPhase_ = static_cast<AnimPhase>(ANIM_H1 + mode - 1);
    animStartMs_ = now;
    animStep_ = 255;
    animHue_ = 0;
    lastAnimSource_ = "halfhour"; lastAnimMode_ = mode;
  }

  void triggerHourAnimation(uint32_t now) {
    const uint8_t mode = settings_.get().hourAnimation;
    if (mode == 0) return;
    animPhase_ = static_cast<AnimPhase>(ANIM_HR1 + mode - 1);
    animStartMs_ = now;
    animStep_ = 255;
    animHue_ = 0;
    lastAnimSource_ = "hour"; lastAnimMode_ = mode;
  }

  void triggerAnimDirect(AnimPhase phase, uint32_t now) {
    animPhase_   = phase;
    animStartMs_ = now;
    animStep_    = 255;
    animHue_     = 0;
    lastAnimSource_ = "preview"; lastAnimMode_ = (uint8_t)phase;
  }

  void triggerReminderDirectAnimation(uint8_t mode, uint32_t now) {
    if (mode == 0) { triggerQuarterAnimation(now); return; }
    if (mode == 1) { triggerHalfHourAnimation(now); return; }
    if (mode == 2) { triggerHourAnimation(now); return; }
    if (mode == 3) { triggerQuarterAnimation(now); return; }
    if (mode == 4) { triggerHalfHourAnimation(now); return; }
    if (mode == 5) { triggerHourAnimation(now); return; }
    static const AnimPhase remPhases[] = {
      ANIM_REM1, ANIM_REM2, ANIM_REM3, ANIM_REM4, ANIM_REM5
    };
    const uint8_t idx = mode - 6;
    if (idx >= 5) return;
    animPhase_   = remPhases[idx];
    animStartMs_ = now;
    animStep_    = 0;
    animHue_     = 0;
    lastAnimSource_ = "reminder"; lastAnimMode_ = mode;
  }

  bool animating() const { return animPhase_ != ANIM_IDLE; }
  const char* lastAnimSource() const { return lastAnimSource_; }
  uint8_t lastAnimMode() const { return lastAnimMode_; }

  // Non-persistent style preview for demo mode and web-UI previews — never
  // touches SettingsStore/EEPROM. Previously every preview click POSTed
  // /settings first, which committed to flash (a blocking, interrupt-masking
  // write) per click; rapid preview use visibly stalled rendering and wore
  // the flash. Overrides auto-clear when the running animation finishes.
  void setPaletteOverride(uint8_t palette) { paletteOverride_ = (int8_t)palette; }
  void clearPaletteOverride() { paletteOverride_ = -1; }
  void setStyleOverride(int16_t palette, int16_t speed, int16_t brightness,
                        int16_t trail, int16_t reminderPal) {
    if (palette >= 0)     paletteOverride_ = (int8_t)palette;
    if (speed >= 1)       speedOverride_ = (int8_t)speed;
    if (brightness >= 0)  brightnessOverride_ = brightness;
    if (trail >= 0)       trailOverride_ = (int8_t)trail;
    if (reminderPal >= 0) reminderPaletteOverride_ = (int8_t)reminderPal;
  }
  void clearStyleOverrides() {
    paletteOverride_ = -1;
    speedOverride_ = -1;
    brightnessOverride_ = -1;
    trailOverride_ = -1;
    reminderPaletteOverride_ = -1;
  }
  const char* animPhaseName() const {
    switch (animPhase_) {
      case ANIM_IDLE: return "idle";
      case ANIM_Q1:  return "Q1";  case ANIM_Q2:  return "Q2";  case ANIM_Q3:  return "Q3";
      case ANIM_H1:  return "H1";  case ANIM_H2:  return "H2";  case ANIM_H3:  return "H3";
      case ANIM_HR1: return "Hr1"; case ANIM_HR2: return "Hr2"; case ANIM_HR3: return "Hr3";
      case ANIM_HR4: return "Hr4"; case ANIM_HR5: return "Hr5";
      case ANIM_REM1: return "Rem1"; case ANIM_REM2: return "Rem2"; case ANIM_REM3: return "Rem3";
      case ANIM_REM4: return "Rem4"; case ANIM_REM5: return "Rem5";
      default:        return "?";
    }
  }

  void renderAnimFrame(uint32_t now) {
    const ClockSettings &settings = settings_.get();
    const uint8_t br = effectiveBrightness(lastTime_, settings, now);
    strip_.setBrightness(br);
#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT
    if (centerStrip_) {
      centerStrip_->setBrightness(br);
      centerStrip_->clear();
    }
#endif
    strip_.clear();
    tickAnimation(now);
    setSacrificialPixelDark();
    logShow(now, "anim");
    strip_.show();
#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT
    if (centerStrip_) centerStrip_->show();
#endif
  }

  bool needsFullAnimationFrame(uint32_t now) const {
    return statusActive(now) || (settings_.get().hourlyChime && chimeActive(lastTime_));
  }

  bool needsCenterAnimationFrame() const {
    return centerIdleActive();
  }

  // Auto-brightness (mode 1) ramps toward its lux-derived target at a fixed
  // rate, but that ramp is only ever applied to the strip when render() runs.
  // Idle render only happens once per second (on the time tick), so without
  // this the ramp was invisible in ~10 units/sec steps instead of the
  // intended 50 units/sec — this drives extra frames while it's catching up.
  bool needsBrightnessRampFrame(uint32_t now) const {
    return settings_.get().autoBrightnessMode == 1 && lux_ &&
           lux_->brightnessRampInProgress();
  }

  void render(const ClockTime &time) {
    lastTime_ = time;
    const ClockSettings &settings = settings_.get();
    const uint32_t now = millis();
    const uint8_t br = effectiveBrightness(time, settings, now);
    strip_.setBrightness(br);
#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT
    if (centerStrip_) {
      centerStrip_->setBrightness(br);
      centerStrip_->clear();
    }
#endif
    strip_.clear();

    renderFace(settings);
    renderSeconds(time, settings);
    renderMinutes(time, settings);
    renderHours(time, settings);

    const bool hourlyChimeNow = settings.hourlyChime && chimeActive(time);
    if (hourlyChimeNow) {
      renderHourlyChime(now);
    }
    if (statusActive(now)) {
      renderStatus(now);
    } else if (!hourlyChimeNow) {
      renderCenterIdle(now);
    }

    setSacrificialPixelDark();
    logShow(now, "face");
    strip_.show();
#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT
    if (centerStrip_) {
      centerStrip_->show();
    }
#endif
  }

 private:
  bool nightActive(const ClockTime &time, const ClockSettings &settings) const {
    if (settings.nightStartHour == settings.nightEndHour) return false;
    if (settings.nightStartHour < settings.nightEndHour) {
      return time.hour >= settings.nightStartHour && time.hour < settings.nightEndHour;
    }
    return time.hour >= settings.nightStartHour || time.hour < settings.nightEndHour;
  }

  uint8_t effectiveBrightness(const ClockTime &time, const ClockSettings &settings, uint32_t now) const {
    switch (settings.autoBrightnessMode) {
      case 0:
        return settings.dayBrightness;
      case 1:
        if (lux_) {
          // Dark-room sleep: in pitch black, blank the display entirely.
          if (settings.darkRoomOff && lux_->roomDark(now)) return 0;
          uint8_t auto_val = lux_->autoBrightnessCached(now);
          return constrain(auto_val, settings.minAutoBrightness, settings.maxAutoBrightness);
        }
        return settings.dayBrightness;
      case 2:
        return nightActive(time, settings) ? settings.nightBrightness : settings.dayBrightness;
      default:
        return settings.dayBrightness;
    }
  }

  void renderFace(const ClockSettings &settings) {
    const uint32_t outerMarker = ringColor(settings.outerMarkerRed, settings.outerMarkerGreen,
                                           settings.outerMarkerBlue, settings.outerMarkerLevel);
    const uint32_t outerFiller = ringColor(settings.outerFillerRed, settings.outerFillerGreen,
                                           settings.outerFillerBlue, settings.outerFillerLevel);
    const uint8_t orbScale = (uint8_t)((uint16_t)settings.outerRingBrightness * 255 / 100);
    const uint32_t outerMarkerScaled = scale(outerMarker, orbScale);
    const uint32_t outerFillerScaled = scale(outerFiller, orbScale);
    const uint32_t middleAmbient = scale(ringColor(settings.middleFaceRed, settings.middleFaceGreen,
                                                  settings.middleFaceBlue, 255), settings.middleFaceScale);
    const uint32_t innerAmbient = scale(ringColor(settings.innerFaceRed, settings.innerFaceGreen,
                                                 settings.innerFaceBlue, 255), settings.innerFaceScale);
    for (uint8_t i = 0; i < RING_OUTER_60.count; ++i) {
      setRingPixel(RING_OUTER_60, i, (i % 5 == 0) ? outerMarkerScaled : outerFillerScaled);
    }
    for (uint8_t i = 0; i < RING_MIDDLE_24.count; ++i) {
      setRingPixel(RING_MIDDLE_24, i, middleAmbient);
    }
    for (uint8_t i = 0; i < RING_INNER_12.count; ++i) {
      setRingPixel(RING_INNER_12, i, innerAmbient);
    }
    (void)settings.colorTheme;
  }

  void renderSeconds(const ClockTime &time, const ClockSettings &settings) {
    const uint32_t color = ringColor(settings.secondsRed, settings.secondsGreen, settings.secondsBlue,
                                     settings.secondsLevel);
    if (settings.progressSeconds) {
      for (uint8_t i = 0; i <= time.second; ++i) {
        setRingPixel(RING_OUTER_60, i, scale(color, 20));
      }
    }

    if (settings.secondTrail) {
      // Geometric decay: each step ~half the previous (52->28->14->7).
      // Fixed 4-LED trail; the trailLength setting applies to interval
      // animations only, not this clock-face trail.
      const uint8_t trail[] = {52, 28, 14, 7};
      for (uint8_t i = 0; i < sizeof(trail); ++i) {
        const uint8_t idx = (time.second + RING_OUTER_60.count - i - 1) % RING_OUTER_60.count;
        setRingPixel(RING_OUTER_60, idx, scale(color, trail[i]));
      }
    }

    setRingPixel(RING_OUTER_60, time.second, color);
  }

  void renderMinutes(const ClockTime &time, const ClockSettings &settings) {
    setRingPixel(RING_OUTER_60, time.minute,
                 ringColor(settings.minutesRed, settings.minutesGreen, settings.minutesBlue,
                           settings.minutesLevel));
  }

  void renderHours(const ClockTime &time, const ClockSettings &settings) {
    const uint8_t hour12 = time.hour % 12;
    const uint32_t middleHour = ringColor(settings.hoursRed, settings.hoursGreen, settings.hoursBlue,
                                          settings.hoursLevel);
    const uint32_t innerHour = ringColor(settings.innerHourRed, settings.innerHourGreen,
                                         settings.innerHourBlue, settings.innerHourLevel);

    // Middle ring (24 LED): 1→2→1 LED by thirds, base shifted +1 CW from hour position.
    // :00–:19 → 1 LED; :20–:39 → 2 LEDs (straddle); :40–:59 → 1 LED advanced.
    const uint8_t midBase = (hour12 * 2 + 1) % RING_MIDDLE_24.count;
    if (time.minute < 20) {
      setRingPixel(RING_MIDDLE_24, midBase, middleHour);
    } else if (time.minute < 40) {
      setRingPixel(RING_MIDDLE_24, midBase, middleHour);
      setRingPixel(RING_MIDDLE_24, (midBase + 1) % RING_MIDDLE_24.count, middleHour);
    } else {
      setRingPixel(RING_MIDDLE_24, (midBase + 1) % RING_MIDDLE_24.count, middleHour);
    }

    // Inner ring (12 LED): fixed 1 LED per hour. Middle ring carries sub-hour progression.
    setRingPixel(RING_INNER_12, hour12, innerHour);
  }

  uint32_t secondColor(uint8_t intensity) {
    switch (settings_.get().colorTheme) {
      case 1:
        return strip_.Color(0, intensity, 65);
      case 2:
        return strip_.Color(intensity, 0, 90);
      default:
        return strip_.Color(0, 0, intensity);
    }
  }

  uint32_t ringColor(uint8_t r, uint8_t g, uint8_t b, uint8_t level) {
    return strip_.Color((static_cast<uint16_t>(r) * level) / 255,
                        (static_cast<uint16_t>(g) * level) / 255,
                        (static_cast<uint16_t>(b) * level) / 255);
  }

  void renderStatus(uint32_t now) {
    uint8_t head = (now / 90) % RING_INNER_12.count;
    uint32_t color = strip_.Color(40, 40, 40);
    switch (statusMode_) {
      case STATUS_WIFI_CONNECTING:
        color = strip_.Color(0, 0, 140);
        break;
      case STATUS_WIFI_OK:
        color = strip_.Color(0, 120, 20);
        break;
      case STATUS_WIFI_FAIL:
        color = strip_.Color(140, 0, 0);
        break;
      case STATUS_BUTTON:
        color = strip_.Color(120, 50, 0);
        break;
      case STATUS_TIME_SYNC:
        color = strip_.Color(0, 100, 90);
        break;
      case STATUS_SETTINGS_SAVED:
        color = strip_.Color(70, 0, 120);
        break;
      case STATUS_OTA_UPDATE:
        color = strip_.Color(0, 0, 200);  // Blue for OTA update in progress
        break;
      case STATUS_OTA_SUCCESS:
        color = strip_.Color(0, 200, 0);  // Green for OTA success
        break;
      case STATUS_OTA_FAILED:
        color = strip_.Color(200, 0, 0);  // Red for OTA failure
        break;
      default:
        break;
    }
    setRingPixel(RING_INNER_12, head, color);
    setRingPixel(RING_INNER_12, (head + RING_INNER_12.count - 1) % RING_INNER_12.count, dim(color, 3));
    setCenterPixel(pulse(color, now, 220, 6, 180));
  }

  void renderHourlyChime(uint32_t now) {
    uint8_t sweep = (now / 65) % RING_OUTER_60.count;
    setRingPixel(RING_OUTER_60, sweep, strip_.Color(80, 80, 80));
    setRingPixel(RING_MIDDLE_24, (sweep * RING_MIDDLE_24.count) / RING_OUTER_60.count, strip_.Color(120, 80, 0));
    setCenterPixel(pulse(strip_.Color(90, 65, 10), now, 180, 4, 130));
  }

  uint32_t paletteColor(uint8_t position, bool useReminderPalette = false) {
    const ClockSettings &s = settings_.get();
    const uint8_t br = animBr();
    const uint8_t pal = useReminderPalette
        ? (reminderPaletteOverride_ >= 0 ? (uint8_t)reminderPaletteOverride_ : s.reminderPalette)
        : (paletteOverride_ >= 0 ? (uint8_t)paletteOverride_ : s.animationPalette);
    if (useReminderPalette) {
      switch (pal) {
        case 0: {
          uint8_t g = (uint8_t)((uint16_t)position * br / 510 + br / 4);
          return strip_.Color(br, g, 0);
        }
        case 1:
          return strip_.Color(br, (uint8_t)(position / 8), 0);
        case 2:
          return strip_.Color(br, 0, (uint8_t)((uint16_t)position * br / 510));
        case 3:
          return strip_.Color((uint8_t)(position / 4), (uint8_t)(br * 3 / 4), (uint8_t)(br / 2));
        default:
          return strip_.Color(br, br / 4, 0);
      }
    }
    switch (pal) {
      case 0:
        return strip_.ColorHSV((uint16_t)((uint32_t)position * 65536 / 256), 255, br);
      case 1: {
        if (position < 85)  return strip_.Color((uint8_t)((uint32_t)position * 3 * br / 255), 0, 0);
        if (position < 170) return strip_.Color(br, (uint8_t)((uint32_t)(position - 85) * 3 * br / 510), 0);
        return strip_.Color(br, (uint8_t)((uint32_t)(position - 170) * 3 * br / 510 + br / 2), 0);
      }
      case 2: {
        if (position < 128) return strip_.Color(0, (uint8_t)((uint16_t)position * br / 128), br);
        return strip_.Color((uint8_t)((uint16_t)(position - 128) * br / 127),
                            (uint8_t)(br / 2 + (uint16_t)(position - 128) * br / 254), br);
      }
      case 3: {
        if (position < 128) return strip_.Color(0, (uint8_t)((uint16_t)position * br / 128), 0);
        return strip_.Color((uint8_t)((uint16_t)(position - 128) * br / 127),
                            br, (uint8_t)((uint16_t)(position - 128) * br / 127));
      }
      case 4: {
        uint16_t hue = (uint16_t)((uint32_t)position * 43690 / 256) + 54613;
        return strip_.ColorHSV(hue, 220, br);
      }
      case 5: {
        uint16_t hue = (uint16_t)((uint32_t)position * 32768 / 256) + 43690;
        return strip_.ColorHSV(hue, 255, br);
      }
      case 6: {
        uint8_t scaled = (uint8_t)((uint16_t)position * br / 255);
        return ringColor(s.hoursRed, s.hoursGreen, s.hoursBlue, scaled);
      }
      case 7: {
        if (position < 64)  return ringColor(s.outerMarkerRed, s.outerMarkerGreen, s.outerMarkerBlue, br);
        if (position < 128) return ringColor(s.minutesRed, s.minutesGreen, s.minutesBlue, br);
        if (position < 192) return ringColor(s.hoursRed, s.hoursGreen, s.hoursBlue, br);
        return ringColor(s.centerRed, s.centerGreen, s.centerBlue, br);
      }
      default:
        return strip_.ColorHSV((uint16_t)((uint32_t)position * 65536 / 256), 255, br);
    }
  }

  // Palette color for a ring band rather than a raw palette position.
  // band: 0=outer, 1=middle, 2=inner, 3=center. frac is the 0-255 position
  // around that ring for palettes that render a gradient. Ring-mapped
  // palette 7 ignores frac and returns the band's actual configured face
  // color — previously animations sampled palette 7 by LED position, which
  // striped every ring into four quadrant colors (position 0-63 = outer
  // marker color, 64-127 = minutes, ...) instead of showing each ring in
  // its own assigned color.
  uint32_t bandColor(uint8_t band, uint8_t frac, bool useReminderPalette = false) {
    const ClockSettings &s = settings_.get();
    const uint8_t pal = paletteOverride_ >= 0 ? (uint8_t)paletteOverride_ : s.animationPalette;
    if (!useReminderPalette && pal == 7) {
      const uint8_t br = animBr();
      switch (band) {
        case 0:  return ringColor(s.outerMarkerRed, s.outerMarkerGreen, s.outerMarkerBlue, br);
        case 1:  return ringColor(s.hoursRed, s.hoursGreen, s.hoursBlue, br);
        // Animations render each band as one solid-color ring, unlike the
        // idle face where innerHourColor is deliberately matched to the
        // middle hand's hoursColor for visual continuity between the two
        // hour indicators. That match makes the middle-24 and inner-12
        // rings visually fuse into one ring during animations. Use
        // secondsColor instead — already part of the clock's configured
        // palette, and distinct from outer/middle/center — so the inner
        // ring reads as its own ring against the two rings outside it.
        // Idle-face rendering (renderFace) is untouched; only animations.
        case 2:  return ringColor(s.secondsRed, s.secondsGreen, s.secondsBlue, br);
        default: return ringColor(s.centerRed, s.centerGreen, s.centerBlue, br);
      }
    }
    static const uint8_t base[] = {0, 85, 160, 192};
    return paletteColor((uint8_t)(base[band & 3] + frac), useReminderPalette);
  }

  bool animPaletteRingMapped() const {
    const uint8_t pal = paletteOverride_ >= 0 ? (uint8_t)paletteOverride_
                                              : settings_.get().animationPalette;
    return pal == 7;
  }

  // Style values with the non-persistent preview override applied.
  uint8_t animBr() const {
    return brightnessOverride_ >= 0 ? (uint8_t)brightnessOverride_
                                    : settings_.get().animationBrightness;
  }
  uint8_t animTl() const {
    return trailOverride_ >= 0 ? (uint8_t)trailOverride_
                               : settings_.get().trailLength;
  }

  // Quadratic ease-in-out on a 0-255 ramp. Linear brightness ramps read as
  // visible steps on WS2812s, especially at the dim end; easing both ends
  // of every envelope is most of what "smooth" looks like on these LEDs.
  static uint8_t ease8(uint8_t x) {
    if (x < 128) { const uint16_t t = x; return (uint8_t)((t * t) >> 7); }
    const uint16_t t = 255 - x;
    return (uint8_t)(255 - ((t * t) >> 7));
  }

  // Perceptual (approx gamma-2) scale for trail falloff and fade tails.
  static uint8_t gamma8(uint8_t v) { return (uint8_t)(((uint16_t)v * v) / 255); }

  // Two-arm spiral brightness wave with a true-black floor: two bright arms
  // per revolution separated by dark "space". Gamma-shaped so the arms punch
  // and the gaps read as genuinely dark, not merely dim — this is what gives
  // Galaxy Spin its light/dark contrast on every palette (gradient palettes
  // used to render flat full-brightness with no dark lanes at all).
  static uint8_t galaxyWave(uint8_t frac) {
    uint8_t f = (uint8_t)(frac * 2u);  // two peaks per revolution
    uint8_t tri = f < 128 ? (uint8_t)(f * 2u) : (uint8_t)((255u - f) * 2u);
    return gamma8(tri);
  }

  // Cheap 8-bit hash for star/twinkle placement — no PRNG state on the hot
  // render path. Same input always maps to the same pixel, so stars hold
  // still for a frame window instead of buzzing every tick.
  static uint8_t hash8(uint16_t x) {
    x ^= (uint16_t)(x >> 7); x = (uint16_t)(x * 0x2C9Bu); x ^= (uint16_t)(x >> 5);
    return (uint8_t)x;
  }

  // Focus-reminder brightness ceiling. A "nudge" for hyperfocus interruption
  // must read as a gentle swell, never a full-intensity alert flash. Every
  // reminder animation scales its peak to this fraction of animBr() (~80%).
  static constexpr uint8_t NUDGE_CEIL = 205;

  // Warm golden flower-core (stamen) for bloom animations, so the center +
  // inner ring read as a bright warm heart regardless of the selected
  // palette — the ChronoBloom flower metaphor, from the inside out:
  // stamen (center / inner-12) → stigma (middle-24) → petals (outer-60).
  uint32_t stamenColor(uint8_t br) {
    return strip_.Color(br,
                        (uint8_t)((uint16_t)br * 205u / 255u),
                        (uint8_t)((uint16_t)br * 55u / 255u));
  }

  // Eased attack/sustain/release brightness envelope for a whole animation:
  // 0 → animBr() over `attack` ms, hold, back to 0 over the final `release`
  // ms of `dur`. Caller guarantees se < dur.
  uint8_t animEnv(uint32_t se, uint32_t dur, uint32_t attack, uint32_t release) {
    const uint8_t br = animBr();
    uint8_t lin;
    if (se < attack)             lin = (uint8_t)(se * 255u / attack);
    else if (dur - se < release) lin = (uint8_t)((dur - se) * 255u / release);
    else                         return br;
    return (uint8_t)(((uint16_t)ease8(lin) * br) / 255u);
  }

  // Trail pixel brightness: linear falloff by tap index, gamma-weighted so
  // the tail fades out perceptually instead of in four visible chunks.
  static uint8_t trailLevel(uint8_t tap, uint8_t tl) {
    return gamma8((uint8_t)(255u * (tl - tap + 1u) / (tl + 1u)));
  }

  uint32_t scaledElapsed(uint32_t elapsed) {
    const uint8_t spd = speedOverride_ >= 1 ? (uint8_t)speedOverride_
                                            : settings_.get().animationSpeed;
    static const uint8_t num[] = {1, 3, 4, 6, 8};
    static const uint8_t den[] = {2, 4, 4, 4, 4};
    const uint8_t idx = (spd < 1) ? 0u : (spd > 5) ? 4u : (uint8_t)(spd - 1);
    return elapsed * num[idx] / den[idx];
  }

  void tickAnimation(uint32_t now) {
    if (animPhase_ == ANIM_IDLE) return;
    switch (animPhase_) {
      case ANIM_Q1:   animQ1(now);   break;
      case ANIM_Q2:   animQ2(now);   break;
      case ANIM_Q3:   animQ3(now);   break;
      case ANIM_H1:   animH1(now);   break;
      case ANIM_H2:   animH2(now);   break;
      case ANIM_H3:   animH3(now);   break;
      case ANIM_HR1:  animHr1(now);  break;
      case ANIM_HR2:  animHr2(now);  break;
      case ANIM_HR3:  animHr3(now);  break;
      case ANIM_HR4:  animHr4(now);  break;
      case ANIM_HR5:  animHr5(now);  break;
      case ANIM_REM1: animRem1(now); break;
      case ANIM_REM2: animRem2(now); break;
      case ANIM_REM3: animRem3(now); break;
      case ANIM_REM4: animRem4(now); break;
      case ANIM_REM5: animRem5(now); break;
      default: animPhase_ = ANIM_IDLE; break;
    }
    // Preview style overrides live only as long as the animation they were
    // set for; scheduled chimes afterwards must use the saved settings.
    if (animPhase_ == ANIM_IDLE) clearStyleOverrides();
  }

  // ── Quarter animations (modes 1-3, 2000-2800ms) ──────────────────────────

  void animQ1(uint32_t now) {  // Slow Comet
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 2500;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    const uint8_t tl = animTl();
    strip_.setBrightness(animEnv(se, dur, 300, 300));
    uint8_t pos = (uint8_t)(se * 60u / dur);
    uint32_t c = bandColor(0, (uint8_t)(pos * 4u));
    setRingPixel(RING_OUTER_60, pos, c);
    for (uint8_t t = 1; t <= tl; t++)
      setRingPixel(RING_OUTER_60, (pos + 60u - t) % 60u, scale(c, trailLevel(t, tl)));
  }

  void animQ2(uint32_t now) {  // Dual Orbit
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 2800;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    const uint8_t tl = animTl();
    strip_.setBrightness(animEnv(se, dur, 300, 300));
    uint8_t posA = (uint8_t)(se * 120u / dur % 60u);
    uint8_t posB = (posA + 30u) % 60u;
    // Second orbiter uses the center band: for gradient palettes 192+192
    // wraps to position 128 (same contrast as before); for ring-mapped
    // palette 7 it gives the configured center accent color instead of a
    // duplicate of the first orbiter.
    uint32_t cA = bandColor(0, 0);
    uint32_t cB = bandColor(3, 192);
    setRingPixel(RING_OUTER_60, posA, cA);
    setRingPixel(RING_OUTER_60, posB, cB);
    for (uint8_t t = 1; t <= tl; t++) {
      uint8_t tbr = trailLevel(t, tl);
      setRingPixel(RING_OUTER_60, (posA + 60u - t) % 60u, scale(cA, tbr));
      setRingPixel(RING_OUTER_60, (posB + 60u - t) % 60u, scale(cB, tbr));
    }
  }

  void animQ3(uint32_t now) {  // Bloom Ripple
    const uint32_t se = scaledElapsed(now - animStartMs_);
    // The outer ring (last band, onset 800) reaches zero brightness at local
    // t=1500, i.e. se=2300 — dur used to run to 2500, leaving 200ms of pure
    // black before the animation officially ended and the next one chained
    // in. That baked-in dead-black hold is what read as "halting" on camera:
    // ripple fades to black, holds black, then a new ripple pops on abruptly.
    // Trimmed to end exactly when the last visible ring goes dark.
    const uint32_t dur = 2300;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    strip_.setBrightness(animBr());
    // Ripple outward: center → inner → middle → outer
    static const uint32_t onsets[] = {0, 250, 500, 800};
    for (uint8_t r = 0; r < 4; r++) {
      if (se < onsets[r]) continue;
      uint32_t t = se - onsets[r];
      uint8_t pct;
      if      (t < 450u)  pct = ease8((uint8_t)(t * 255u / 450u));
      else if (t < 650u)  pct = 255u;
      else if (t < 1500u) pct = ease8((uint8_t)(255u - (t - 650u) * 255u / 850u));
      else                pct = 0u;
      if (pct == 0u) continue;
      // Flower-part coloring, center outward: stamen (warm gold core + inner
      // filaments) → stigma (middle, palette) → petals (outer, palette). The
      // warm core is palette-independent so every palette still blooms from a
      // bright warm heart, the strongest ChronoBloom cue.
      switch (r) {
        case 0: setCenterPixel(scale(stamenColor(animBr()), pct)); break;
        case 1: {
          uint32_t c = scale(scale(stamenColor(animBr()), 200), pct);
          for (uint8_t i = 0; i < RING_INNER_12.count; i++)  setRingPixel(RING_INNER_12, i, c);
        } break;
        case 2: {
          uint32_t c = scale(bandColor(1, 0), pct);
          for (uint8_t i = 0; i < RING_MIDDLE_24.count; i++) setRingPixel(RING_MIDDLE_24, i, c);
        } break;
        case 3: {
          uint32_t c = scale(bandColor(0, 0), pct);
          for (uint8_t i = 0; i < RING_OUTER_60.count; i++)  setRingPixel(RING_OUTER_60, i, c);
        } break;
      }
    }
  }

  // ── Half-hour animations (modes 1-3, 4500-5500ms) ────────────────────────

  void animH1(uint32_t now) {  // Unfurl
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 5000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    strip_.setBrightness(animEnv(se, dur, 300, 800));
    setCenterPixel(stamenColor(255));  // warm stamen heart, not flat white
    if (se >= 300u) {
      uint32_t t = se - 300u;
      uint8_t n = (t < 1000u) ? (uint8_t)(t * RING_INNER_12.count / 1000u) : RING_INNER_12.count;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_INNER_12, i, bandColor(2, (uint8_t)(i * 21u)));
    }
    if (se >= 1100u) {
      uint32_t t = se - 1100u;
      uint8_t n = (t < 1400u) ? (uint8_t)(t * RING_MIDDLE_24.count / 1400u) : RING_MIDDLE_24.count;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_MIDDLE_24, i, bandColor(1, (uint8_t)(i * 10u)));
    }
    if (se >= 2200u) {
      uint32_t t = se - 2200u;
      uint8_t n = (t < 1800u) ? (uint8_t)(t * RING_OUTER_60.count / 1800u) : RING_OUTER_60.count;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_OUTER_60, i, bandColor(0, (uint8_t)(i * 4u)));
    }
  }

  void animH2(uint32_t now) {  // Three Comets
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 5000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    const uint8_t tl = animTl();
    strip_.setBrightness(animEnv(se, dur, 400, 400));
    // All three comets travel the same direction. The middle one used to be
    // reversed for a counter-rotation effect, which read as chunks of LEDs
    // moving against each other rather than as a coordinated relay.
    {
      uint8_t pos = (uint8_t)(se * 180u / 5000u % 60u);
      uint32_t c = bandColor(0, 0);
      setRingPixel(RING_OUTER_60, pos, c);
      for (uint8_t t = 1; t <= tl; t++)
        setRingPixel(RING_OUTER_60, (pos + 60u - t) % 60u, scale(c, trailLevel(t, tl)));
    }
    {
      uint8_t pos = (uint8_t)(se * 48u / 5000u % 24u);
      uint32_t c = bandColor(1, 0);
      setRingPixel(RING_MIDDLE_24, pos, c);
      for (uint8_t t = 1; t <= tl; t++)
        setRingPixel(RING_MIDDLE_24, (pos + 24u - t) % 24u, scale(c, trailLevel(t, tl)));
    }
    {
      uint8_t pos = (uint8_t)(se * 48u / 5000u % 12u);
      uint32_t c = bandColor(2, 10);
      setRingPixel(RING_INNER_12, pos, c);
      for (uint8_t t = 1; t <= tl; t++)
        setRingPixel(RING_INNER_12, (pos + 12u - t) % 12u, scale(c, trailLevel(t, tl)));
    }
  }

  void animH3(uint32_t now) {  // Breathe
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 5000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    const uint8_t env = animEnv(se, dur, 400, 400);
    const float ft = (float)se * 6.2832f / 3200.0f;
    // Gamma-weight each ring's sine phase so the troughs settle gently into
    // dark instead of visibly stepping through the last few PWM levels.
    uint8_t brI = gamma8((uint8_t)((sinf(ft + 1.5708f) + 1.0f) * 127.5f));
    uint8_t brM = gamma8((uint8_t)((sinf(ft)            + 1.0f) * 127.5f));
    uint8_t brO = gamma8((uint8_t)((sinf(ft - 1.5708f)  + 1.0f) * 127.5f));
    brI = (uint8_t)(((uint16_t)brI * env) / 255u);
    brM = (uint8_t)(((uint16_t)brM * env) / 255u);
    brO = (uint8_t)(((uint16_t)brO * env) / 255u);
    strip_.setBrightness(255);
    for (uint8_t i = 0; i < RING_INNER_12.count; i++)
      setRingPixel(RING_INNER_12, i, scale(bandColor(2, (uint8_t)(i * 21u)), brI));
    for (uint8_t i = 0; i < RING_MIDDLE_24.count; i++)
      setRingPixel(RING_MIDDLE_24, i, scale(bandColor(1, (uint8_t)(i * 10u)), brM));
    for (uint8_t i = 0; i < RING_OUTER_60.count; i++)
      setRingPixel(RING_OUTER_60, i, scale(bandColor(0, (uint8_t)(i * 4u)), brO));
  }

  // ── Hour animations (modes 1-5, 6500-9000ms) ─────────────────────────────

  void animHr1(uint32_t now) {  // Ceremony
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 9000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    strip_.setBrightness(animEnv(se, dur, 400, 1000));
    if (se >= 400u) {
      uint32_t t = se - 400u;
      uint8_t n = (t < 2000u) ? (uint8_t)(t * 60u / 2000u) : 60u;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_OUTER_60, i, bandColor(0, (uint8_t)(i * 4u)));
    }
    if (se >= 2200u) {
      uint32_t t = se - 2200u;
      uint8_t n = (t < 1500u) ? (uint8_t)(t * 24u / 1500u) : 24u;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_MIDDLE_24, (uint8_t)((24u - i) % 24u), bandColor(1, (uint8_t)(i * 10u)));
    }
    if (se >= 3500u) {
      uint32_t t = se - 3500u;
      uint8_t n = (t < 1200u) ? (uint8_t)(t * 12u / 1200u) : 12u;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_INNER_12, i, bandColor(2, (uint8_t)(i * 21u)));
    }
    if (se >= 4500u) {
      uint8_t pct;
      if (se < 5200u) {
        pct = ease8((uint8_t)((se - 4500u) * 255u / 700u));
      } else {
        float ft = (float)(se - 5200u) * 6.2832f / 1800.0f;
        pct = (uint8_t)(200u + (uint8_t)(27.5f + 27.5f * sinf(ft)));
      }
      setCenterPixel(scale(strip_.Color(255, 255, 255), pct));
    }
  }

  void animHr2(uint32_t now) {  // Galaxy Spin — spiral arms, dark space, stars
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 9000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    strip_.setBrightness(animEnv(se, dur, 500, 1000));
    uint8_t outerOff  = (uint8_t)(se * 256u / 6000u);
    uint8_t middleOff = (uint8_t)(86u - (uint8_t)(se * 256u / 8000u));
    uint8_t innerOff  = (uint8_t)(171u + (uint8_t)(se * 256u / 5000u));
    // Slow global hue drift: the arms cycle through the palette across the
    // 9s run instead of holding one flat color band — far more color
    // variation on camera than the old single-hue spin.
    uint8_t hueDrift = (uint8_t)(se * 96u / dur);
    // Brightness now rides galaxyWave() for EVERY palette (not just the
    // ring-mapped one), carving dark lanes between two bright spiral arms so
    // the "space vs. stars" contrast is always present. Rare white pixels in
    // the dark lanes twinkle like stars.
    for (uint8_t i = 0; i < RING_OUTER_60.count; i++) {
      uint8_t frac = (uint8_t)(outerOff + i * 256u / RING_OUTER_60.count);
      uint8_t w = galaxyWave(frac);
      if (w < 70u && hash8((uint16_t)(i * 31u + se / 130u)) < 10u)
        setRingPixel(RING_OUTER_60, i, strip_.Color(255, 255, 255));
      else
        setRingPixel(RING_OUTER_60, i, scale(bandColor(0, (uint8_t)(frac + hueDrift)), w));
    }
    for (uint8_t i = 0; i < RING_MIDDLE_24.count; i++) {
      uint8_t frac = (uint8_t)(middleOff + i * 256u / RING_MIDDLE_24.count);
      uint8_t w = galaxyWave(frac);
      if (w < 70u && hash8((uint16_t)(i * 53u + se / 150u + 7u)) < 8u)
        setRingPixel(RING_MIDDLE_24, i, strip_.Color(255, 255, 255));
      else
        setRingPixel(RING_MIDDLE_24, i, scale(bandColor(1, (uint8_t)(frac + hueDrift)), w));
    }
    for (uint8_t i = 0; i < RING_INNER_12.count; i++) {
      uint8_t frac = (uint8_t)(innerOff + i * 256u / RING_INNER_12.count);
      setRingPixel(RING_INNER_12, i,
                   scale(bandColor(2, (uint8_t)(frac + hueDrift)), galaxyWave(frac)));
    }
    // Bright galactic core that gently twinkles.
    uint8_t coreTw = (uint8_t)(200u + (hash8((uint16_t)(se / 90u)) % 56u));
    setCenterPixel(scale(bandColor(3, (uint8_t)(outerOff + 128u + hueDrift)), coreTw));
  }

  void animHr3(uint32_t now) {  // Supernova
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 8000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    const uint8_t br = animBr();
    uint8_t globalFade = (se > 6000u) ? ease8((uint8_t)((dur - se) * 255u / 2000u)) : 255u;
    strip_.setBrightness((uint8_t)((uint32_t)br * globalFade / 255u));
    {
      uint8_t pct = (se < 300u) ? ease8((uint8_t)(se * 255u / 300u)) : 255u;
      setCenterPixel(scale(strip_.Color(255, 255, 255), pct));
    }
    if (se >= 200u) {
      uint32_t t = se - 200u;
      uint8_t pct = (t < 400u) ? ease8((uint8_t)(t * 255u / 400u)) : 255u;
      uint32_t c = scale(bandColor(2, 0), pct);
      for (uint8_t i = 0; i < RING_INNER_12.count; i++) setRingPixel(RING_INNER_12, i, c);
    }
    if (se >= 600u) {
      uint32_t t = se - 600u;
      uint8_t pct = (t < 500u) ? ease8((uint8_t)(t * 255u / 500u)) : 255u;
      uint32_t c = scale(bandColor(1, 0), pct);
      for (uint8_t i = 0; i < RING_MIDDLE_24.count; i++) setRingPixel(RING_MIDDLE_24, i, c);
    }
    if (se >= 1200u) {
      uint32_t t = se - 1200u;
      uint8_t pct = (t < 800u) ? ease8((uint8_t)(t * 255u / 800u)) : 255u;
      uint32_t c = scale(bandColor(0, 0), pct);
      for (uint8_t i = 0; i < RING_OUTER_60.count; i++) setRingPixel(RING_OUTER_60, i, c);
    }
  }

  void animHr4(uint32_t now) {  // Comet Relay — rainbow spiral
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 8000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    const uint8_t tl = animTl();
    strip_.setBrightness(animEnv(se, dur, 300, 500));
    // Each comet's head sweeps the palette as it travels and its trail lags a
    // few hue-steps behind, so the tail is a rainbow gradient rather than a
    // single flat color — this is what makes the relay read as a spiral.
    if (se < 3200u) {
      uint8_t pos = (uint8_t)(se * 60u / 3000u % 60u);
      uint8_t hf  = (uint8_t)(se * 200u / 3000u);
      setRingPixel(RING_OUTER_60, pos, bandColor(0, hf));
      for (uint8_t t = 1; t <= tl; t++)
        setRingPixel(RING_OUTER_60, (pos + 60u - t) % 60u,
                     scale(bandColor(0, (uint8_t)(hf - t * 14u)), trailLevel(t, tl)));
    }
    if (se >= 2800u && se < 5400u) {
      uint32_t t = se - 2800u;
      uint8_t pos = (uint8_t)(t * 24u / 2400u % 24u);
      uint8_t hf  = (uint8_t)(t * 200u / 2400u + 40u);
      setRingPixel(RING_MIDDLE_24, pos, bandColor(1, hf));
      for (uint8_t tt = 1; tt <= tl; tt++)
        setRingPixel(RING_MIDDLE_24, (pos + 24u - tt) % 24u,
                     scale(bandColor(1, (uint8_t)(hf - tt * 14u)), trailLevel(tt, tl)));
    }
    if (se >= 5000u && se < 7200u) {
      uint32_t t = se - 5000u;
      uint8_t pos = (uint8_t)(t * 24u / 2000u % 12u);
      uint8_t hf  = (uint8_t)(t * 200u / 2000u + 80u);
      setRingPixel(RING_INNER_12, pos, bandColor(2, hf));
      for (uint8_t tt = 1; tt <= tl; tt++)
        setRingPixel(RING_INNER_12, (pos + 12u - tt) % 12u,
                     scale(bandColor(2, (uint8_t)(hf - tt * 16u)), trailLevel(tt, tl)));
    }
    if (se >= 6800u) {
      uint8_t pct = (se < 7300u) ? ease8((uint8_t)((se - 6800u) * 255u / 500u)) : 255u;
      setCenterPixel(scale(stamenColor(animBr()), pct));
    }
  }

  void animHr5(uint32_t now) {  // Deep Breath
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 9000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    const uint8_t env = animEnv(se, dur, 500, 1000);
    const float ft = (float)se * 6.2832f / 3000.0f;
    // Gamma-weight the breath so it settles into darkness smoothly instead
    // of stepping through the lowest PWM levels.
    uint8_t breath = gamma8((uint8_t)((sinf(ft) + 1.0f) * 127.5f));
    strip_.setBrightness((uint8_t)(((uint16_t)breath * env) / 255u));
    for (uint8_t i = 0; i < RING_OUTER_60.count; i++)
      setRingPixel(RING_OUTER_60, i, bandColor(0, (uint8_t)(i * 4u)));
    for (uint8_t i = 0; i < RING_MIDDLE_24.count; i++)
      setRingPixel(RING_MIDDLE_24, i, bandColor(1, (uint8_t)(i * 10u)));
    for (uint8_t i = 0; i < RING_INNER_12.count; i++)
      setRingPixel(RING_INNER_12, i, bandColor(2, (uint8_t)(i * 21u)));
    setCenterPixel(bandColor(3, 0));
  }


  // ── Reminder animations (modes 6-10, 2500-4000ms) ────────────────────────

  void animRem1(uint32_t now) {  // Gentle Pulse — slow breathing nudge
    const uint32_t se = scaledElapsed(now - animStartMs_);
    // Longer, softer envelope than before (was 800/400/1800): a slow 1.2s
    // rise, brief crest, 1.9s fall — a breath, not a blink.
    const uint32_t dur = 3600;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    strip_.setBrightness(animBr());
    uint8_t pct;
    if      (se < 1200u) pct = ease8((uint8_t)(se * 255u / 1200u));
    else if (se < 1700u) pct = 255u;
    else                 pct = ease8((uint8_t)(255u - (se - 1700u) * 255u / 1900u));
    pct = (uint8_t)((uint16_t)pct * NUDGE_CEIL / 255u);  // cap: nudge, not alert
    // Subtle warm hue drift across the swell so the color shifts noticeably
    // but gently instead of holding one flat tone.
    uint8_t drift = (uint8_t)(se * 40u / dur);
    for (uint8_t i = 0; i < RING_OUTER_60.count; i++)
      setRingPixel(RING_OUTER_60, i, scale(bandColor(0, drift, true), pct));
    for (uint8_t i = 0; i < RING_MIDDLE_24.count; i++)
      setRingPixel(RING_MIDDLE_24, i, scale(bandColor(1, (uint8_t)(drift + 20u), true), pct));
    for (uint8_t i = 0; i < RING_INNER_12.count; i++)
      setRingPixel(RING_INNER_12, i, scale(bandColor(2, (uint8_t)(drift + 40u), true), pct));
    setCenterPixel(scale(bandColor(3, (uint8_t)(drift + 8u), true), pct));
  }

  void animRem2(uint32_t now) {  // Orbiting Orb
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 3000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    const uint8_t tl = animTl();
    strip_.setBrightness(animEnv(se, dur, 300, 300));
    uint32_t dimC = scale(bandColor(0, 0, true), 40);
    for (uint8_t i = 0; i < RING_OUTER_60.count; i++) setRingPixel(RING_OUTER_60, i, dimC);
    uint8_t pos = (uint8_t)(se * 30u / dur % 12u);
    uint32_t c = bandColor(0, 0, true);
    setRingPixel(RING_INNER_12, pos, c);
    for (uint8_t t = 1; t <= tl && t < 6u; t++)
      setRingPixel(RING_INNER_12, (pos + 12u - t) % 12u, scale(c, trailLevel(t, tl)));
  }

  void animRem3(uint32_t now) {  // Ripple In
    const uint32_t se = scaledElapsed(now - animStartMs_);
    // Last band (center, onset 900) reaches zero at local t=1600, i.e.
    // se=2500 — dur used to run to 3500, a full second of dead black before
    // the animation ended and the next one chained in. Trimmed to match.
    // Slower onsets and a softer 600ms rise / 1100ms fall (was 400/900) so
    // each ring swells in gently rather than snapping on. Last band onset
    // 1050 + 1750 tail = 2800.
    const uint32_t dur = 2800;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    strip_.setBrightness(animBr());
    static const uint32_t onsets[] = {0, 350, 700, 1050};
    static const uint8_t bands[]  = {0, 1, 2, 3};
    for (uint8_t r = 0; r < 4; r++) {
      if (se < onsets[r]) continue;
      uint32_t t = se - onsets[r];
      uint8_t pct;
      if      (t < 600u)  pct = ease8((uint8_t)(t * 255u / 600u));
      else if (t < 850u)  pct = 255u;
      else if (t < 1750u) pct = ease8((uint8_t)(255u - (t - 850u) * 255u / 900u));
      else                pct = 0u;
      if (pct == 0u) continue;
      pct = (uint8_t)((uint16_t)pct * NUDGE_CEIL / 255u);  // cap: nudge, not alert
      uint32_t c = scale(bandColor(bands[r], 0, true), pct);
      switch (r) {
        case 0: for (uint8_t i = 0; i < RING_OUTER_60.count; i++)  setRingPixel(RING_OUTER_60, i, c);  break;
        case 1: for (uint8_t i = 0; i < RING_MIDDLE_24.count; i++) setRingPixel(RING_MIDDLE_24, i, c); break;
        case 2: for (uint8_t i = 0; i < RING_INNER_12.count; i++)  setRingPixel(RING_INNER_12, i, c);  break;
        case 3: setCenterPixel(c); break;
      }
    }
  }

  void animRem4(uint32_t now) {  // Heartbeat — soft double swell
    const uint32_t se = scaledElapsed(now - animStartMs_);
    // Formerly a hard strobe (140ms rise / 80ms fall) that read as a warning
    // flash. Rebuilt as two eased swells — a 350ms rise / 550ms fall per beat
    // with a short rest between — so it still has a "two-thump" cadence but
    // breathes rather than blinks. Second beat is quieter. Beat 2 at 1200 +
    // 900ms envelope = 2100.
    const uint32_t dur = 2100;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    strip_.setBrightness(animBr());
    static const uint32_t beatTimes[] = {0, 1200};
    uint8_t pct = 0;
    for (uint8_t b = 0; b < 2; b++) {
      if (se < beatTimes[b]) continue;
      uint32_t t = se - beatTimes[b];
      uint8_t amp = (b == 0) ? NUDGE_CEIL : (uint8_t)((uint16_t)NUDGE_CEIL * 180u / 255u);
      uint8_t env;
      if      (t < 350u) env = ease8((uint8_t)(t * 255u / 350u));
      else if (t < 900u) env = ease8((uint8_t)(255u - (t - 350u) * 255u / 550u));
      else               env = 0u;
      pct = (uint8_t)((uint16_t)env * amp / 255u);
    }
    if (pct > 0u) {
      // Whole flower thumps together, with a warm hue and a brighter stamen
      // core so the pulse still centers on the bloom.
      uint32_t c = scale(bandColor(0, 12, true), pct);
      for (uint8_t i = 0; i < RING_OUTER_60.count; i++)  setRingPixel(RING_OUTER_60, i, c);
      for (uint8_t i = 0; i < RING_MIDDLE_24.count; i++) setRingPixel(RING_MIDDLE_24, i, scale(bandColor(1, 24, true), pct));
      for (uint8_t i = 0; i < RING_INNER_12.count; i++)  setRingPixel(RING_INNER_12, i, scale(bandColor(2, 40, true), pct));
      setCenterPixel(scale(bandColor(3, 8, true), pct));
    }
  }

  void animRem5(uint32_t now) {  // Slow Bloom
    const uint32_t se = scaledElapsed(now - animStartMs_);
    const uint32_t dur = 4000;
    if (se >= dur) { animPhase_ = ANIM_IDLE; return; }
    strip_.setBrightness(animEnv(se, dur, 500, 800));
    {
      uint8_t n = (se < 2500u) ? (uint8_t)(se * 60u / 2500u) : 60u;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_OUTER_60, i, bandColor(0, (uint8_t)(i * 4u), true));
    }
    if (se >= 800u) {
      uint32_t t = se - 800u;
      uint8_t n = (t < 2000u) ? (uint8_t)(t * 24u / 2000u) : 24u;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_MIDDLE_24, i, bandColor(1, (uint8_t)(i * 10u), true));
    }
    // Stamen filaments (inner-12) unfurl just before the core lights, so the
    // bloom completes from petals inward instead of skipping the inner ring.
    if (se >= 1400u) {
      uint32_t t = se - 1400u;
      uint8_t n = (t < 1400u) ? (uint8_t)(t * 12u / 1400u) : 12u;
      for (uint8_t i = 0; i < n; i++)
        setRingPixel(RING_INNER_12, i, bandColor(2, (uint8_t)(i * 21u + 20u), true));
    }
    if (se >= 1500u) {
      uint8_t pct = (se < 2000u) ? ease8((uint8_t)((se - 1500u) * 255u / 500u)) : 255u;
      setCenterPixel(scale(bandColor(3, 8, true), pct));
    }
  }

  bool chimeActive(const ClockTime &time) const {
    return time.minute == 0 && time.second < 6;
  }

  bool statusActive(uint32_t now) const {
    return statusMode_ != STATUS_NONE && static_cast<int32_t>(statusUntilMs_ - now) > 0;
  }

  void logShow(uint32_t now, const char *src) {
    if (lastShowMs_ != 0) {
      uint32_t gap = now - lastShowMs_;
      if (gap < minFrameMs_) minFrameMs_ = gap;
      if (gap > maxFrameMs_) maxFrameMs_ = gap;
      if (gap < 15) {
        Serial.printf("[RENDER] RAPID %s gap=%lums\n", src, (unsigned long)gap);
      }
    }
    ++renderCount_;
    lastShowMs_ = now;
    if (now - lastDiagMs_ >= 30000) {
      if (lastDiagMs_ != 0) {
        uint32_t dt = now - lastDiagMs_;
        Serial.printf("[RENDER] src=%s fps=%.1f min=%lu max=%lu ms/frame\n",
                      src, renderCount_ * 1000.0f / dt,
                      minFrameMs_ == 0xFFFFFFFFu ? 0UL : (unsigned long)minFrameMs_,
                      (unsigned long)maxFrameMs_);
      }
      renderCount_ = 0;
      minFrameMs_ = 0xFFFFFFFFu;
      maxFrameMs_ = 0;
      lastDiagMs_ = now;
    }
  }

  uint32_t dim(uint32_t color, uint8_t divisor) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    return strip_.Color(r / divisor, g / divisor, b / divisor);
  }

  uint32_t scale(uint32_t color, uint8_t amount) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    return strip_.Color((static_cast<uint16_t>(r) * amount) / 255,
                        (static_cast<uint16_t>(g) * amount) / 255,
                        (static_cast<uint16_t>(b) * amount) / 255);
  }

  uint32_t pulse(uint32_t color, uint32_t now, uint16_t periodMs, uint8_t floor, uint8_t ceiling) {
    const uint16_t phase = now % periodMs;
    const uint16_t half = periodMs / 2;
    const uint16_t ramp = phase < half ? phase : periodMs - phase;
    const uint8_t span = ceiling - floor;
    const uint8_t amount = floor + (static_cast<uint32_t>(span) * ramp) / half;
    return scale(color, amount);
  }

  bool centerIdleActive() const {
    return CENTER_PIXEL_ENABLED != 0 && settings_.get().statusAnimations;
  }

  void renderCenterIdle(uint32_t now) {
    if (!centerIdleActive()) return;
    const ClockSettings &settings = settings_.get();
    static constexpr uint16_t PERIOD_MS = 1800;
    const uint16_t phase = now % PERIOD_MS;
    const uint16_t half  = PERIOD_MS / 2;
    const uint16_t ramp  = phase < half ? phase : PERIOD_MS - phase;
    // Single multiply (not double-scale) to avoid quantization at low brightness.
    // Sweeps amount from 12% to 100% of centerLevel directly in raw-pixel space.
    const uint8_t lo     = (uint16_t)settings.centerLevel * 12 / 100;
    const uint8_t amount = lo + (uint32_t)(settings.centerLevel - lo) * ramp / half;
    setCenterPixel(strip_.Color(
      (uint16_t)settings.centerRed   * amount / 255,
      (uint16_t)settings.centerGreen * amount / 255,
      (uint16_t)settings.centerBlue  * amount / 255
    ));
  }

  void setCenterPixel(uint32_t color) {
#if CENTER_PIXEL_ENABLED
#if CENTER_PIXEL_SEPARATE_OUTPUT
    if (centerStrip_ && CENTER_PIXEL_INDEX < CENTER_PIXEL_STRIP_COUNT) {
      centerStrip_->setPixelColor(CENTER_PIXEL_INDEX, color);
    }
#else
    if (CENTER_PIXEL_INDEX < CLOCK_PIXEL_COUNT) {
      strip_.setPixelColor(CENTER_PIXEL_INDEX, color);
    }
#endif
#else
    (void)color;
#endif
  }

  void setSacrificialPixelDark() {
#if SACRIFICIAL_PIXEL_ENABLED
    if (SACRIFICIAL_PIXEL_INDEX < CLOCK_PIXEL_COUNT) {
      strip_.setPixelColor(SACRIFICIAL_PIXEL_INDEX, 0);
    }
#endif
  }

  void setRingPixel(const RingConfig &ring, uint8_t logicalIndex, uint32_t color) {
    if (logicalIndex >= ring.count) return;
    uint8_t rot = settings_.get().outerRingOffset;
    // +30 before dividing by 60 rounds to nearest LED rather than truncating.
    uint8_t rotated = rot
        ? static_cast<uint8_t>((static_cast<uint32_t>(logicalIndex) +
                                (static_cast<uint32_t>(rot) * ring.count + 30) / 60) % ring.count)
        : logicalIndex;
    uint8_t physical = ring.clockwise ? rotated : (ring.count - 1 - rotated);
    strip_.setPixelColor(ring.offset + physical, color);
  }

  Adafruit_NeoPixel &strip_;
  SettingsStore &settings_;
  Adafruit_NeoPixel *centerStrip_;
  LuxSensor *lux_;
  StatusMode statusMode_ = STATUS_NONE;
  uint32_t statusUntilMs_ = 0;
  uint32_t lastAnimationMs_ = 0;
  ClockTime lastTime_ = {12, 0, 0};
  AnimPhase animPhase_ = ANIM_IDLE;
  uint32_t animStartMs_ = 0;
  uint8_t animStep_ = 0;
  uint32_t animHue_ = 0;
  const char* lastAnimSource_ = "none";
  uint8_t lastAnimMode_ = 0;
  uint32_t lastShowMs_ = 0;
  uint32_t renderCount_ = 0;
  uint32_t minFrameMs_ = 0xFFFFFFFFu;
  uint32_t maxFrameMs_ = 0;
  uint32_t lastDiagMs_ = 0;
  int8_t paletteOverride_ = -1;
  int8_t speedOverride_ = -1;
  int16_t brightnessOverride_ = -1;
  int8_t trailOverride_ = -1;
  int8_t reminderPaletteOverride_ = -1;
};

// ===================== Temperature placeholder =====================
class TemperatureInput {
 public:
  void begin() {}
  bool available() const { return TEMP_SENSOR_ENABLED != 0; }
  float celsius() const { return NAN; }
};

// ===================== Time sync =====================
class TimeSync {
 public:
  explicit TimeSync(TimeModel &model) : model_(model) {}

  void begin() {
#if ENABLE_NTP
    configTzTime(NTP_TIMEZONE_TZ, "pool.ntp.org", "time.nist.gov", "time.google.com");
    // Fire syncNow() from the Arduino loop task each time SNTP updates the system
    // clock, rather than polling blindly. This closes the race where the system
    // clock is set to UTC but localtime_r hasn't yet re-applied the TZ string.
    sntp_set_time_sync_notification_cb([](struct timeval *) {
      TimeSync::s_sntpPending_ = true;
    });
#endif
  }

  bool syncNow() {
#if ENABLE_NTP
    time_t rawUtc = time(nullptr);
    if (rawUtc < 1700000000) return false;
    struct tm localTime;
    if (!localtime_r(&rawUtc, &localTime)) return false;
    {
      ClockTime old = model_.get();
      int32_t oldSec = old.hour * 3600 + old.minute * 60 + old.second;
      int32_t newSec = localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec;
      lastDeltaSec_ = newSec - oldSec;
      if (lastDeltaSec_ > 43200)  lastDeltaSec_ -= 86400;
      if (lastDeltaSec_ < -43200) lastDeltaSec_ += 86400;
    }
    model_.set(localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
    lastSyncMs_ = millis();
    synced_ = true;
    Serial.printf("[NTP] Time synced: %02d:%02d:%02d local  (UTC epoch %lu)\n",
                  localTime.tm_hour, localTime.tm_min, localTime.tm_sec,
                  (unsigned long)rawUtc);
    return true;
#else
    return false;
#endif
  }

  void loop() {
    if (WiFi.status() != WL_CONNECTED) return;
    const uint32_t now = millis();
    // Consume SNTP callback flag (set from SNTP task, read here in Arduino task).
    // On single-core ESP32-C3, volatile bool read/write is effectively atomic.
    const bool sntpFired = s_sntpPending_;
    if (sntpFired) s_sntpPending_ = false;
    if (sntpFired || !synced_ || now - lastSyncMs_ > syncIntervalMs_) {
      syncNow();
    }
  }

  bool synced() const { return synced_; }

  int32_t lastDeltaSec() const { return lastDeltaSec_; }

  static volatile bool s_sntpPending_;

 private:
  TimeModel &model_;
  bool synced_ = false;
  uint32_t lastSyncMs_ = 0;
  int32_t lastDeltaSec_ = 0;
  static constexpr uint32_t syncIntervalMs_ = 6UL * 60UL * 60UL * 1000UL;
};

volatile bool TimeSync::s_sntpPending_ = false;

// Forward declaration — defined in the globals section below
extern Adafruit_NeoPixel ledStrip;

// ===================== WiFi Setup (WiFiManager) =====================
bool setupWiFi() {
  Preferences prefs;
  prefs.begin("factory", false);
  const bool forcePortal = prefs.getBool("portal", false);
  if (forcePortal) prefs.putBool("portal", false);
  prefs.end();

  WiFi.setAutoReconnect(true);
  WiFiManager wm;
  wm.setConnectRetries(1);

  // Blue LEDs while portal is open — visible from across the room
  wm.setAPCallback([](WiFiManager *) {
    ledStrip.setBrightness(150);
    ledStrip.fill(ledStrip.Color(0, 60, 255));
    ledStrip.show();
    Serial.println("[WiFi] Portal open — connect to esp32c3-clock-setup");
  });

  if (forcePortal) {
    Serial.println("[WiFi] Factory-reset flag set — launching provisioning portal.");
    WiFi.disconnect(false, true);  // erase saved AP from NVS
    delay(200);
    { Preferences wprefs; wprefs.begin("wifi", false); wprefs.clear(); wprefs.end(); }
    wm.resetSettings();
    wm.setConfigPortalTimeout(0);  // stay open until user configures
    return wm.startConfigPortal("esp32c3-clock-setup", "");
  }

  // Priority 1: credentials saved via /wifi web page
  {
    Preferences wprefs;
    wprefs.begin("wifi", true);
    String savedSsid = wprefs.getString("ssid", "");
    String savedPass = wprefs.getString("pass", "");
    wprefs.end();
    if (savedSsid.length() > 0) {
      Serial.printf("[WiFi] Trying saved SSID: %s\n", savedSsid.c_str());
      WiFi.begin(savedSsid.c_str(), savedPass.c_str());
      const uint32_t start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        Serial.print(".");
        delay(500);
      }
      Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] Connected with saved credentials");
        return true;
      }
      Serial.printf("[WiFi] Saved credentials failed, status=%d.\n", WiFi.status());
      WiFi.disconnect(false);
      delay(250);
    }
  }

  wm.setConfigPortalTimeout(120);  // 2-minute portal window for normal fallback

  if (strcmp(WIFI_SSID, "clock-ssid") != 0) {
    Serial.printf("[WiFi] Trying build-time SSID: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[WiFi] Connected with build-time credentials");
      return true;
    }

    Serial.printf("[WiFi] Build-time credentials failed, status=%d. Starting setup portal.\n",
                  WiFi.status());
    WiFi.disconnect(false);
    delay(250);
  }

  return wm.autoConnect("esp32c3-clock-setup", "");
}

// ===================== OTA Setup =====================
void setupOTA() {
  ArduinoOTA.setHostname(DEVICE_HOSTNAME);

  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Update starting...");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("[OTA] Update complete, rebooting...");
    delay(2000);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    esp_task_wdt_reset();
    static uint32_t lastLog = 0;
    if (millis() - lastLog > 500) {
      Serial.printf("[OTA] Progress: %u/%u (%.1f%%)\n", progress, total,
                   (float)progress * 100.0 / total);
      lastLog = millis();
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error %u: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    else Serial.println("Unknown");
    ESP.restart();
  });

  ArduinoOTA.begin();
  Serial.printf("[OTA] Ready. Upload: pio run -e esp32c3_v3_8inch -t upload --upload-port %s.local:3232\n", DEVICE_HOSTNAME);
}

// Incremented by ButtonInput on each consume. Readable from WebUi /diag handler.
static uint32_t g_buttonEventCount = 0;

// ===================== Demo Mode =====================
class DemoMode {
 public:
  struct Step {
    uint32_t duration_ms;
    const char *subtitle;
  };

  explicit DemoMode(ClockRenderer &renderer, LuxSensor &luxSensor, SettingsStore &settings)
      : renderer_(renderer), luxSensor_(luxSensor), settings_(settings) {}

  void start() {
    active_ = true;
    step_ = 0;
    stepStartMs_ = millis();
    subStep_ = 0;
    renderer_.clearStyleOverrides();
  }

  void stop() {
    active_ = false;
    luxSensor_.clearLuxOverride();
    renderer_.clearStyleOverrides();
  }

  void loop(uint32_t now) {
    if (!active_) return;
    // Re-dispatch immediately after a step transition so the new step's
    // first trigger fires in this same tick — otherwise stepTick() would
    // only flip step_/subStep_ and return, leaving one full loop() iteration
    // with nothing animating. The main loop's render dispatch checks
    // renderer.animating() right after demoMode.loop() returns; if nothing
    // got triggered yet, it falls into the idle-render branch and flashes
    // the live, fully-lit real-time clock face into that gap before the new
    // step's first frame appears next tick. Bounded to 8 (one per step) as a
    // safety guard — normally this loops at most twice.
    for (uint8_t guard = 0; guard < 8 && active_; guard++) {
      if (!stepTick(now)) break;
    }
  }

  // Runs the current step; returns true if it just transitioned to a new
  // step (caller should re-invoke immediately so the new step's first
  // trigger fires in the same tick), false otherwise.
  bool stepTick(uint32_t now) {
    uint32_t elapsed = now - stepStartMs_;

    // Step 0: Idle intro (8s)
    if (step_ == 0) {
      if (elapsed >= 8000) { advanceStep(now); return true; }
      return false;
    }

    // Steps 1-3: the clock's real chimes, exactly as configured — each slot
    // fires its *configured* trigger (not a hardcoded phase list) twice, so
    // the demo always mirrors what this unit actually does at :15/:30/:00
    // in its own palette, speed, brightness, and trail settings. Fewer
    // styles with more dwell time reads better than a rapid-fire catalog
    // of every mode.
    if (step_ == 1) {
      if (advanceConfiguredSeq(now, 0, 2)) { advanceStep(now); return true; }
      return false;
    }
    if (step_ == 2) {
      if (advanceConfiguredSeq(now, 1, 2)) { advanceStep(now); return true; }
      return false;
    }
    if (step_ == 3) {
      if (advanceConfiguredSeq(now, 2, 2)) { advanceStep(now); return true; }
      return false;
    }

    // Step 4: showcase — the big all-ring hour animations (Ceremony, Galaxy
    // Spin, Supernova, Comet Relay) played once each, full duration, in the
    // clock's own colors. Galaxy Spin and Comet Relay are the spiral/galaxy
    // pieces the enhancement pass targeted, so both are on camera here.
    if (step_ == 4) {
      static const AnimPhase showPhases[] = {ANIM_HR1, ANIM_HR2, ANIM_HR3, ANIM_HR4};
      if (advanceAnimPhaseSeq(now, showPhases, 4)) { advanceStep(now); return true; }
      return false;
    }

    // Step 5: focus reminder nudges — a varied but uniformly gentle set
    // (Gentle Pulse → Ripple In → Heartbeat → Slow Bloom) so the demo shows
    // that every nudge now reads as a soft swell, not an alert flash.
    if (step_ == 5) {
      static const uint8_t remModes[] = {6, 8, 9, 10};  // REM1, REM3, REM4, REM5
      if (advanceReminderSeq(now, remModes, 4)) { advanceStep(now); return true; }
      return false;
    }

    // Step 6: Auto-brightness (14s) - two quick dim/bright cycles, showcasing
    // the faster 150ms lux-poll response (was 500ms, ~3-4x slower to react).
    if (step_ == 6) {
      if (elapsed >= 14000) { luxSensor_.clearLuxOverride(); advanceStep(now); return true; }

      float luxValue = 220.0f;
      if (elapsed < 1500) {
        float t = elapsed / 1500.0f;
        luxValue = 220.0f - (220.0f - 1.0f) * t;
      } else if (elapsed < 3000) {
        luxValue = 1.0f;
      } else if (elapsed < 4500) {
        float t = (elapsed - 3000) / 1500.0f;
        luxValue = 1.0f + (220.0f - 1.0f) * t;
      } else if (elapsed < 6000) {
        luxValue = 220.0f;
      } else if (elapsed < 7500) {
        float t = (elapsed - 6000) / 1500.0f;
        luxValue = 220.0f - (220.0f - 1.0f) * t;
      } else if (elapsed < 9000) {
        luxValue = 1.0f;
      } else if (elapsed < 10500) {
        float t = (elapsed - 9000) / 1500.0f;
        luxValue = 1.0f + (220.0f - 1.0f) * t;
      } else {
        luxValue = 220.0f;
      }
      luxSensor_.setLuxOverride(luxValue);
      return false;
    }

    // Step 7: End card (10s) - idle, subtitle shown
    if (step_ == 7) {
      if (elapsed >= 10000) {
        active_ = false;
        luxSensor_.clearLuxOverride();
      }
      return false;
    }
    return false;
  }

  String statusJson() const {
    if (!active_) return "{\"active\":false}";

    uint32_t now = millis();
    uint32_t elapsed = now - stepStartMs_;
    uint32_t duration = steps[step_].duration_ms;

    char buf[256];
    snprintf(buf, sizeof(buf),
             "{\"active\":true,\"step\":%u,\"subtitle\":\"%s\","
             "\"elapsed_ms\":%lu,\"step_duration_ms\":%lu}",
             (unsigned)step_, steps[step_].subtitle,
             (unsigned long)elapsed, (unsigned long)duration);
    return String(buf);
  }

 private:
  void advanceStep(uint32_t now) {
    step_++;
    stepStartMs_ = now;
    subStep_ = 0;
  }

  // Plays phases[0..count) back to back via triggerAnimDirect, chaining
  // straight into the next one the instant ClockRenderer::animating() goes
  // false — same tick, so the main loop's idle-render branch never gets a
  // window to flash the live clock face between beats. Each animation
  // already fades to near-black in its own last frames, so the hard cut
  // reads as a clean beat, not a jump. Returns true once every phase has
  // been triggered and the last one has finished.
  bool advanceAnimPhaseSeq(uint32_t now, const AnimPhase *phases, uint8_t count) {
    if (renderer_.animating()) return false;
    if (subStep_ < count) {
      renderer_.triggerAnimDirect(phases[subStep_], now);
      subStep_++;
      return false;
    }
    return true;
  }

  // Same immediate-chain pacing as advanceAnimPhaseSeq, but fires the
  // clock's *configured* animation for a chime slot (0=quarter, 1=half-hour,
  // 2=hour) `count` times. If that slot's configured mode is 0 (off), the
  // trigger no-ops and the sequence falls through without stalling.
  bool advanceConfiguredSeq(uint32_t now, uint8_t slot, uint8_t count) {
    if (renderer_.animating()) return false;
    if (subStep_ < count) {
      if (slot == 0)      renderer_.triggerQuarterAnimation(now);
      else if (slot == 1) renderer_.triggerHalfHourAnimation(now);
      else                renderer_.triggerHourAnimation(now);
      subStep_++;
      return false;
    }
    return true;
  }

  // Same pacing, but plays an explicit list of reminder modes (6..10 map to
  // ANIM_REM1..5), allowing repeats.
  bool advanceReminderSeq(uint32_t now, const uint8_t *modes, uint8_t count) {
    if (renderer_.animating()) return false;
    if (subStep_ < count) {
      renderer_.triggerReminderDirectAnimation(modes[subStep_], now);
      subStep_++;
      return false;
    }
    return true;
  }

  // Approximate wall-clock durations for the web UI progress bar. The
  // animation-driven steps are event-paced (each animation runs to its
  // natural end before the next is triggered) and steps 1-3 depend on which
  // styles are configured, so these are estimates at animationSpeed 3 with
  // the maintainer's configuration (Bloom Ripple / Unfurl / Comet Relay),
  // not exact bounds.
  static constexpr Step steps[] = {
    {8000,  "ChronoBloom — ESP32-C3 NeoPixel clock"},
    {4600,  "Quarter-hour chime — twice, exactly as configured"},
    {10000, "Half-hour chime — twice, exactly as configured"},
    {16000, "Top-of-hour chime — twice, exactly as configured"},
    {34000, "Hour animation showcase — Ceremony, Galaxy Spin, Supernova, Comet Relay"},
    {12500, "Focus reminder nudges — gentle, subtle, never a flash"},
    {14000, "Auto-brightness — VEML7700 sensor, now 3-4x faster response"},
    {10000, "Open source — github / printables / hackaday.io"}
  };

  bool active_ = false;
  uint8_t step_ = 0;
  uint8_t subStep_ = 0;
  uint32_t stepStartMs_ = 0;
  ClockRenderer &renderer_;
  LuxSensor &luxSensor_;
  SettingsStore &settings_;
};

// Static array definition for DemoMode::steps
constexpr DemoMode::Step DemoMode::steps[];

// ===================== Web UI =====================
class WebUi {
 public:
  WebUi(TimeModel &model, SettingsStore &settings, ClockRenderer &renderer,
        TimeSync &timeSync, TemperatureInput &temperature)
      : model_(model),
        settings_(settings),
        renderer_(renderer),
        timeSync_(timeSync),
        temperature_(temperature),
        lux_(nullptr),
        server_(80) {}

  void setLuxSensor(LuxSensor *lux) { lux_ = lux; }

  void begin() {
#if ENABLE_WIFI_UI
    Serial.println("[WiFi] Starting WiFi provisioning...");
    WiFi.mode(WIFI_STA);
    WiFi.persistent(true);
    WiFi.setAutoReconnect(true);
    renderer_.setStatus(STATUS_WIFI_CONNECTING, WIFI_CONNECT_TIMEOUT_MS);

    if (!setupWiFi() || WiFi.status() != WL_CONNECTED) {
      // STA failed — broadcast own AP so device is always reachable
      WiFi.mode(WIFI_AP);
      WiFi.softAP(DEVICE_HOSTNAME);
      apMode_ = true;
      Serial.printf("[WiFi] AP mode active. SSID=%s  IP=192.168.4.1\n", DEVICE_HOSTNAME);
      Serial.println("[WiFi] Connect to that SSID then visit http://192.168.4.1/");
      renderer_.setStatus(STATUS_WIFI_FAIL, 3000);
    } else {
      // Reapply hostname after STA connection stabilises
      delay(2000);
      WiFi.setHostname(DEVICE_HOSTNAME);
      WiFi.mode(WIFI_STA);
      WiFi.setSleep(false);  // prevent modem-sleep RMT interference with WS2812B
      Serial.printf("[WiFi] Hostname: %s\n", WiFi.getHostname());
      Serial.printf("[WiFi] SSID: %s\n", WiFi.SSID().c_str());
      Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    }

    enabled_ = true;
    setupRoutes();
    server_.begin();

    if (!apMode_) {
      mdnsEnabled_ = MDNS.begin(DEVICE_HOSTNAME);
      if (mdnsEnabled_) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("[mDNS] Registered: %s.local\n", DEVICE_HOSTNAME);
      } else {
        Serial.println("[mDNS] Start failed");
      }

      // Re-register mDNS after WiFi reconnects. end() first so begin() does a
      // clean re-init, and re-add the HTTP service record — begin() alone does
      // not restore services, so the hostname resolved but the service browse
      // entry was lost after every reconnect.
      WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (mdnsEnabled_) {
          MDNS.end();
          if (MDNS.begin(DEVICE_HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
            Serial.printf("[mDNS] Re-registered after reconnect: %s.local\n", DEVICE_HOSTNAME);
          } else {
            Serial.println("[mDNS] Re-registration failed");
          }
        }
      }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

      setupOTA();

      timeSync_.begin();
      if (timeSync_.syncNow()) {
        renderer_.setStatus(STATUS_TIME_SYNC, 1500);
      } else {
        renderer_.setStatus(STATUS_WIFI_OK, 1500);
      }
    }
#endif
  }

  void loop() {
    if (!enabled_) return;
    server_.handleClient();
    ArduinoOTA.handle();

    if (!apMode_) {
      static uint32_t lastReconnectMs = 0;
      const uint32_t now = millis();
      if (WiFi.status() != WL_CONNECTED && now - lastReconnectMs >= 30000) {
        lastReconnectMs = now;
        Serial.println("[WiFi] Not connected — attempting reconnect...");
        WiFi.reconnect();
      }
    } else {
      // AP-fallback recovery. Without this, a power blip where the clock boots
      // faster than the router strands the device in AP mode until someone
      // power-cycles it. While nobody is connected to the setup AP, retry the
      // known STA credentials every 5 minutes (AP+STA, non-blocking); on
      // success reboot into a clean STA boot (NTP restores the time).
      const uint32_t now = millis();
      if (apRetryMs_ == 0) apRetryMs_ = now;
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] STA reconnected from AP fallback — rebooting into normal mode.");
        delay(200);
        ESP.restart();
      }
      if (WiFi.softAPgetStationNum() == 0 && now - apRetryMs_ >= 300000UL) {
        apRetryMs_ = now;
        String ssid, pass;
        {
          Preferences p;
          p.begin("wifi", true);
          ssid = p.getString("ssid", "");
          pass = p.getString("pass", "");
          p.end();
        }
        if (ssid.isEmpty() && strcmp(WIFI_SSID, "clock-ssid") != 0) {
          ssid = WIFI_SSID;
          pass = WIFI_PASSWORD;
        }
        if (!ssid.isEmpty()) {
          Serial.printf("[WiFi] AP fallback: retrying STA \"%s\"\n", ssid.c_str());
          WiFi.mode(WIFI_AP_STA);
          WiFi.begin(ssid.c_str(), pass.c_str());
        }
      }
    }
  }

  bool enabled() const { return enabled_; }
  bool apMode() const { return apMode_; }

  ClockWebServer& getServer() { return server_; }

 private:
  void setupRoutes() {
    server_.on("/", HTTP_GET, [&]() {
      server_.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      server_.setContentLength(CONTENT_LENGTH_UNKNOWN);
      sendClose(200, "text/html", "");
      server_.sendContent_P(INDEX_P1);
      server_.sendContent_P(INDEX_P2);
      server_.sendContent_P(INDEX_P3);
      server_.client().flush();
    });

    server_.on("/time", HTTP_GET, [&]() {
      ClockTime t = model_.get();
      char buf[128];
      snprintf(buf, sizeof(buf),
        "{\"hour\":%u,\"minute\":%u,\"second\":%u"
        ",\"ntpSynced\":%s,\"wifi\":%s,\"ip\":\"%s\"}",
        t.hour, t.minute, t.second,
        timeSync_.synced() ? "true" : "false",
        (WiFi.status() == WL_CONNECTED) ? "true" : "false",
        WiFi.localIP().toString().c_str());
      sendClose(200, "application/json", buf);
    });

    server_.on("/temperature", HTTP_GET, [&]() {
      const float c = temperature_.celsius();
      if (!temperature_.available() || isnan(c)) {
        sendClose(200, "application/json", "{\"available\":false}");
        return;
      }
      char buf[64];
      snprintf(buf, sizeof(buf), "{\"available\":true,\"celsius\":%.1f}", c);
      sendClose(200, "application/json", buf);
    });

    server_.on("/lux", HTTP_GET, [&]() {
      if (!lux_ || !lux_->available()) {
        sendClose(200, "application/json", "{\"available\":false}");
        return;
      }
      char buf[96];
      snprintf(buf, sizeof(buf), "{\"available\":true,\"lux\":%.1f,\"autoBrightness\":%u}",
               lux_->lux(), (unsigned)lux_->autoBrightness());
      sendClose(200, "application/json", buf);
    });

    server_.on("/net", HTTP_GET, [&]() {
      char buf[256];
      snprintf(buf, sizeof(buf),
        "{\"hostname\":\"%s\",\"ssid\":\"%s\",\"ip\":\"%s\""
        ",\"gateway\":\"%s\",\"subnet\":\"%s\",\"dns\":\"%s\""
        ",\"rssi\":%d,\"status\":%d}",
        DEVICE_HOSTNAME,
        WiFi.SSID().c_str(),
        WiFi.localIP().toString().c_str(),
        WiFi.gatewayIP().toString().c_str(),
        WiFi.subnetMask().toString().c_str(),
        WiFi.dnsIP().toString().c_str(),
        (int)WiFi.RSSI(),
        (int)WiFi.status());
      sendClose(200, "application/json", buf);
    });

    server_.on("/diag", HTTP_GET, [&]() {
      uint32_t uptimeSec = millis() / 1000;
      ClockTime t = model_.get();
      float lux_val = lux_ ? lux_->lux() : -1.0f;
      uint8_t br_target = lux_ ? lux_->autoBrightnessTarget() : 0;
      uint8_t br_ramped = lux_ ? lux_->autoBrightnessCached(millis()) : 0;
      const ClockSettings &ds = settings_.get();
      uint8_t br_effective = (ds.autoBrightnessMode == 1 && lux_)
          ? (uint8_t)constrain((int)br_ramped, ds.minAutoBrightness, ds.maxAutoBrightness)
          : ds.dayBrightness;
      uint8_t mid_amb_scale = ds.middleFaceScale;
      uint8_t inn_amb_scale = ds.innerFaceScale;
      char buf[1800];
      int n = snprintf(buf, sizeof(buf),
        "{\"uptime_sec\":%lu,\"firmware_version\":\"%s\",\"settings_version\":%u"
        ",\"time\":\"%02u:%02u:%02u\""
        ",\"ntp_synced\":%s,\"ntp_last_delta_sec\":%d"
        ",\"wifi_status\":%d,\"wifi_ssid\":\"%s\",\"wifi_rssi\":%d,\"wifi_ip\":\"%s\""
        ",\"lux\":%.1f,\"brightness_target\":%u,\"brightness_ramped\":%u,\"effective_brightness\":%u"
        ",\"outer_marker_level\":%u,\"outer_filler_level\":%u"
        ",\"hours_level\":%u,\"middle_hour_level\":%u,\"inner_hour_level\":%u,\"center_level\":%u"
        ",\"middle_ambient_scale\":%u,\"inner_ambient_scale\":%u"
        ",\"button_event_count\":%lu,\"free_heap\":%lu"
        ",\"clock_pixel_count\":%u,\"ring_pixel_offset\":%u"
        ",\"default_outer_ring_offset\":%u,\"outer_ring_offset\":%u,\"sacrificial_enabled\":%s"
        ",\"anim_phase\":\"%s\",\"last_anim_source\":\"%s\",\"last_anim_mode\":%u"
        ",\"display_sleep\":%s"
        ",\"settings_save_count\":%u}",
        (unsigned long)uptimeSec, FIRMWARE_VERSION, (unsigned)SETTINGS_VERSION,
        (unsigned)t.hour, (unsigned)t.minute, (unsigned)t.second,
        timeSync_.synced() ? "true" : "false", (int)timeSync_.lastDeltaSec(),
        (int)WiFi.status(), WiFi.SSID().c_str(), (int)WiFi.RSSI(),
        WiFi.localIP().toString().c_str(),
        lux_val, (unsigned)br_target, (unsigned)br_ramped, (unsigned)br_effective,
        (unsigned)ds.outerMarkerLevel, (unsigned)ds.outerFillerLevel,
        (unsigned)ds.hoursLevel, (unsigned)ds.hoursLevel, (unsigned)ds.innerHourLevel, (unsigned)ds.centerLevel,
        (unsigned)mid_amb_scale, (unsigned)inn_amb_scale,
        (unsigned long)g_buttonEventCount, (unsigned long)ESP.getFreeHeap(),
        (unsigned)CLOCK_PIXEL_COUNT, (unsigned)RING_PIXEL_OFFSET,
        (unsigned)DEFAULT_OUTER_RING_OFFSET,
        (unsigned)ds.outerRingOffset,
        SACRIFICIAL_PIXEL_ENABLED ? "true" : "false",
        renderer_.animPhaseName(), renderer_.lastAnimSource(), (unsigned)renderer_.lastAnimMode(),
        (lux_ && lux_->displaySleeping()) ? "true" : "false",
        (unsigned)settings_.saveCount());
      if (n < 0 || n >= (int)sizeof(buf)) {
        Serial.printf("[WEB] /diag JSON truncated (%d/%u) — grow buf\n", n, (unsigned)sizeof(buf));
      }
      sendClose(200, "application/json", buf);
    });

    server_.on("/settings", HTTP_GET, [&]() {
      sendClose(200, "application/json", settingsJson());
    });

    server_.on("/settings", HTTP_POST, [&]() {
      ClockSettings settings = settings_.get();
      if (server_.hasArg("dayBrightness")) settings.dayBrightness = clampByte(server_.arg("dayBrightness").toInt(), 0, 255);
      if (server_.hasArg("nightBrightness")) settings.nightBrightness = clampByte(server_.arg("nightBrightness").toInt(), 0, 255);
      if (server_.hasArg("nightStartHour")) settings.nightStartHour = clampByte(server_.arg("nightStartHour").toInt(), 0, 23);
      if (server_.hasArg("nightEndHour")) settings.nightEndHour = clampByte(server_.arg("nightEndHour").toInt(), 0, 23);
      if (server_.hasArg("colorTheme")) settings.colorTheme = clampByte(server_.arg("colorTheme").toInt(), 0, 2);
      if (server_.hasArg("secondTrail")) settings.secondTrail = server_.arg("secondTrail").toInt() ? 1 : 0;
      if (server_.hasArg("progressSeconds")) settings.progressSeconds = server_.arg("progressSeconds").toInt() ? 1 : 0;
      if (server_.hasArg("hourlyChime")) settings.hourlyChime = server_.arg("hourlyChime").toInt() ? 1 : 0;
      if (server_.hasArg("statusAnimations")) settings.statusAnimations = server_.arg("statusAnimations").toInt() ? 1 : 0;
      if (server_.hasArg("outerMarkerLevel")) settings.outerMarkerLevel = clampByte(server_.arg("outerMarkerLevel").toInt(), 0, 255);
      if (server_.hasArg("outerFillerLevel")) settings.outerFillerLevel = clampByte(server_.arg("outerFillerLevel").toInt(), 0, 255);
      if (server_.hasArg("secondsLevel")) settings.secondsLevel = clampByte(server_.arg("secondsLevel").toInt(), 0, 255);
      if (server_.hasArg("minutesLevel")) settings.minutesLevel = clampByte(server_.arg("minutesLevel").toInt(), 0, 255);
      if (server_.hasArg("hoursLevel")) settings.hoursLevel = clampByte(server_.arg("hoursLevel").toInt(), 0, 255);
      if (server_.hasArg("innerHourLevel")) settings.innerHourLevel = clampByte(server_.arg("innerHourLevel").toInt(), 0, 255);
      if (server_.hasArg("centerLevel")) settings.centerLevel = clampByte(server_.arg("centerLevel").toInt(), 0, 255);
      if (server_.hasArg("outerMarkerColor")) parseColor(server_.arg("outerMarkerColor"), settings.outerMarkerRed, settings.outerMarkerGreen, settings.outerMarkerBlue);
      if (server_.hasArg("outerFillerColor")) parseColor(server_.arg("outerFillerColor"), settings.outerFillerRed, settings.outerFillerGreen, settings.outerFillerBlue);
      if (server_.hasArg("secondsColor")) parseColor(server_.arg("secondsColor"), settings.secondsRed, settings.secondsGreen, settings.secondsBlue);
      if (server_.hasArg("minutesColor")) parseColor(server_.arg("minutesColor"), settings.minutesRed, settings.minutesGreen, settings.minutesBlue);
      if (server_.hasArg("hoursColor")) parseColor(server_.arg("hoursColor"), settings.hoursRed, settings.hoursGreen, settings.hoursBlue);
      if (server_.hasArg("middleFaceColor")) parseColor(server_.arg("middleFaceColor"), settings.middleFaceRed, settings.middleFaceGreen, settings.middleFaceBlue);
      if (server_.hasArg("innerFaceColor")) parseColor(server_.arg("innerFaceColor"), settings.innerFaceRed, settings.innerFaceGreen, settings.innerFaceBlue);
      if (server_.hasArg("innerHourColor")) parseColor(server_.arg("innerHourColor"), settings.innerHourRed, settings.innerHourGreen, settings.innerHourBlue);
      if (server_.hasArg("centerColor")) parseColor(server_.arg("centerColor"), settings.centerRed, settings.centerGreen, settings.centerBlue);
      if (server_.hasArg("autoBrightnessMode")) settings.autoBrightnessMode = clampByte(server_.arg("autoBrightnessMode").toInt(), 0, 2);
      if (server_.hasArg("minAutoBrightness")) settings.minAutoBrightness = clampByte(server_.arg("minAutoBrightness").toInt(), 5, 255);
      if (server_.hasArg("maxAutoBrightness")) settings.maxAutoBrightness = clampByte(server_.arg("maxAutoBrightness").toInt(), 5, 255);
      if (server_.hasArg("quarterAnimation")) settings.quarterAnimation = clampByte(server_.arg("quarterAnimation").toInt(), 0, 3);
      if (server_.hasArg("halfHourAnimation")) settings.halfHourAnimation = clampByte(server_.arg("halfHourAnimation").toInt(), 0, 3);
      if (server_.hasArg("hourAnimation")) settings.hourAnimation = clampByte(server_.arg("hourAnimation").toInt(), 0, 5);
      if (server_.hasArg("intervalAnimationsEnabled")) settings.intervalAnimationsEnabled = server_.arg("intervalAnimationsEnabled").toInt() ? 1 : 0;
      if (server_.hasArg("outerRingOffset")) settings.outerRingOffset = clampByte(server_.arg("outerRingOffset").toInt(), 0, 59);
      if (server_.hasArg("focusReminder_enabled")) settings.focusReminder_enabled = server_.arg("focusReminder_enabled").toInt() ? 1 : 0;
      if (server_.hasArg("focusReminder_startHour")) settings.focusReminder_startHour = clampByte(server_.arg("focusReminder_startHour").toInt(), 0, 23);
      if (server_.hasArg("focusReminder_endHour")) settings.focusReminder_endHour = clampByte(server_.arg("focusReminder_endHour").toInt(), 0, 23);
      if (server_.hasArg("focusReminder_intervalMinutes")) settings.focusReminder_intervalMinutes = clampWord(server_.arg("focusReminder_intervalMinutes").toInt(), 1, 1440);
      if (server_.hasArg("focusReminder_daysMask")) settings.focusReminder_daysMask = clampByte(server_.arg("focusReminder_daysMask").toInt(), 0, 127);
      if (server_.hasArg("focusReminder_animation")) settings.focusReminder_animation = clampByte(server_.arg("focusReminder_animation").toInt(), 0, 10);
      if (server_.hasArg("focusReminder_durationSeconds")) settings.focusReminder_durationSeconds = clampByte(server_.arg("focusReminder_durationSeconds").toInt(), 1, 60);
      if (server_.hasArg("animationPalette"))    settings.animationPalette    = clampByte(server_.arg("animationPalette").toInt(), 0, 7);
      if (server_.hasArg("animationSpeed"))      settings.animationSpeed      = clampByte(server_.arg("animationSpeed").toInt(), 1, 5);
      if (server_.hasArg("animationBrightness")) settings.animationBrightness = clampByte(server_.arg("animationBrightness").toInt(), 50, 255);
      if (server_.hasArg("trailLength"))         settings.trailLength         = clampByte(server_.arg("trailLength").toInt(), 2, 12);
      if (server_.hasArg("reminderPalette"))     settings.reminderPalette     = clampByte(server_.arg("reminderPalette").toInt(), 0, 3);
      if (server_.hasArg("outerRingBrightness")) settings.outerRingBrightness = clampByte(server_.arg("outerRingBrightness").toInt(), 0, 100);
      if (server_.hasArg("middleFaceScale"))     settings.middleFaceScale     = clampByte(server_.arg("middleFaceScale").toInt(), 0, 255);
      if (server_.hasArg("innerFaceScale"))      settings.innerFaceScale      = clampByte(server_.arg("innerFaceScale").toInt(), 0, 255);
      if (server_.hasArg("darkRoomOff"))         settings.darkRoomOff         = server_.arg("darkRoomOff").toInt() ? 1 : 0;
      settings_.update(settings);
      if (!server_.hasArg("silent")) renderer_.setStatus(STATUS_SETTINGS_SAVED, 1300);
      model_.markDirty();
      sendClose(200, "text/plain", "ok");
    });

    server_.on("/settings/reset", HTTP_POST, [&]() {
      settings_.resetToDefaults();
      renderer_.setStatus(STATUS_SETTINGS_SAVED, 1300);
      model_.markDirty();
      sendClose(200, "text/plain", "ok");
    });

    server_.on("/settings/saveDefault", HTTP_POST, [&]() {
      settings_.saveAsUserDefaults();
      renderer_.setStatus(STATUS_SETTINGS_SAVED, 1300);
      sendClose(200, "text/plain", "ok");
    });

    server_.on("/set", HTTP_POST, [&]() {
      if (!server_.hasArg("hour") || !server_.hasArg("minute") || !server_.hasArg("second")) {
        sendClose(400, "text/plain", "missing hour/minute/second");
        return;
      }
      model_.set(server_.arg("hour").toInt(), server_.arg("minute").toInt(), server_.arg("second").toInt());
      renderer_.setStatus(STATUS_TIME_SYNC, 1200);
      sendClose(200, "text/plain", "ok");
    });

    server_.on("/syncBrowser", HTTP_POST, [&]() {
      if (!server_.hasArg("hour") || !server_.hasArg("minute") || !server_.hasArg("second")) {
        sendClose(400, "text/plain", "missing hour/minute/second");
        return;
      }
      model_.set(server_.arg("hour").toInt(), server_.arg("minute").toInt(), server_.arg("second").toInt());
      renderer_.setStatus(STATUS_TIME_SYNC, 1500);
      sendClose(200, "text/plain", "ok");
    });

    server_.on("/syncNtp", HTTP_POST, [&]() {
      bool ok = timeSync_.syncNow();
      renderer_.setStatus(ok ? STATUS_TIME_SYNC : STATUS_WIFI_FAIL, 1500);
      sendClose(ok ? 200 : 503, "text/plain", ok ? "ok" : "ntp unavailable");
    });

    server_.on("/addMinute", HTTP_POST, [&]() {
      model_.addMinutes(1);
      renderer_.setStatus(STATUS_BUTTON, 700);
      sendClose(200, "text/plain", "ok");
    });

    server_.on("/subMinute", HTTP_POST, [&]() {
      model_.addMinutes(-1);
      renderer_.setStatus(STATUS_BUTTON, 700);
      sendClose(200, "text/plain", "ok");
    });

    server_.on("/previewAnimation", HTTP_POST, [&]() {
      const String type = server_.arg("type");
      const int    mode = server_.arg("mode").toInt();
      const uint32_t now = millis();
      // Optional style params apply as non-persistent renderer overrides for
      // this preview only (auto-cleared when the animation ends). The web UI
      // used to persist the style via POST /settings before every preview —
      // an EEPROM flash commit per click, which both stalled rendering and
      // wore the flash.
      renderer_.setStyleOverride(
          server_.hasArg("palette")         ? (int16_t)clampByte(server_.arg("palette").toInt(), 0, 7)             : -1,
          server_.hasArg("speed")           ? (int16_t)clampByte(server_.arg("speed").toInt(), 1, 5)               : -1,
          server_.hasArg("brightness")      ? (int16_t)clampByte(server_.arg("brightness").toInt(), 50, 255)       : -1,
          server_.hasArg("trail")           ? (int16_t)clampByte(server_.arg("trail").toInt(), 2, 12)              : -1,
          server_.hasArg("reminderPalette") ? (int16_t)clampByte(server_.arg("reminderPalette").toInt(), 0, 3)     : -1);
      static const AnimPhase qPhases[]  = {ANIM_Q1,  ANIM_Q2,  ANIM_Q3};
      static const AnimPhase hhPhases[] = {ANIM_H1,  ANIM_H2,  ANIM_H3};
      static const AnimPhase hrPhases[] = {ANIM_HR1, ANIM_HR2, ANIM_HR3, ANIM_HR4, ANIM_HR5};
      if (type == "quarter") {
        if (mode < 1 || mode > 3) { sendClose(400, "text/plain", "mode 1-3"); return; }
        renderer_.triggerAnimDirect(qPhases[mode - 1], now);
      } else if (type == "halfhour") {
        if (mode < 1 || mode > 3) { sendClose(400, "text/plain", "mode 1-3"); return; }
        renderer_.triggerAnimDirect(hhPhases[mode - 1], now);
      } else if (type == "hour") {
        if (mode < 1 || mode > 5) { sendClose(400, "text/plain", "mode 1-5"); return; }
        renderer_.triggerAnimDirect(hrPhases[mode - 1], now);
      } else if (type == "reminder") {
        if (mode < 0 || mode > 10) { sendClose(400, "text/plain", "mode 0-10"); return; }
        renderer_.triggerReminderDirectAnimation((uint8_t)mode, now);
      } else {
        sendClose(400, "text/plain", "type: quarter|halfhour|hour|reminder");
        return;
      }
      sendClose(200, "text/plain", "ok");
    });

    // ===== WiFi Settings Page (GET /wifi) =====
    server_.on("/wifi", HTTP_GET, [&]() {
      Preferences wprefs;
      wprefs.begin("wifi", true);
      String savedSsid = wprefs.getString("ssid", "");
      wprefs.end();
      const char *dispSsid = savedSsid.length() > 0 ? savedSsid.c_str() : "(none saved)";
      char wifiSt[64];
      snprintf(wifiSt, sizeof(wifiSt), "%s", wifiStatusText(WiFi.status()));
      if (WiFi.status() == WL_CONNECTED) {
        snprintf(wifiSt, sizeof(wifiSt), "%s (%s)", wifiStatusText(WiFi.status()), WiFi.SSID().c_str());
      }
      server_.setContentLength(CONTENT_LENGTH_UNKNOWN);
      sendClose(200, "text/html", "");
      server_.sendContent_P(WIFI_P1);
      server_.sendContent(dispSsid);
      server_.sendContent_P(WIFI_MID);
      server_.sendContent(wifiSt);
      server_.sendContent_P(WIFI_P2);
      server_.client().flush();
    });

    // ===== WiFi Save Handler (POST /wifi) =====
    server_.on("/wifi", HTTP_POST, [&]() {
      String ssid = server_.arg("ssid");
      String pass = server_.arg("pass");
      ssid.trim();
      if (ssid.length() < 1 || ssid.length() > 32) {
        sendClose(400, "text/plain", "SSID must be 1-32 characters");
        return;
      }
      Preferences wprefs;
      wprefs.begin("wifi", false);
      wprefs.putString("ssid", ssid);
      wprefs.putString("pass", pass);
      wprefs.end();
      Serial.printf("[WiFi] Credentials saved via web UI: SSID=%s\n", ssid.c_str());
      sendClose(200, "text/plain", "Saved. Reconnecting to " + ssid + "...");
      delay(300);
      WiFi.disconnect(false);
      delay(100);
      WiFi.begin(ssid.c_str(), pass.c_str());
    });

    // ===== Firmware Update Web Page (GET /update) =====
    server_.on("/update", HTTP_GET, [&]() {
      server_.setContentLength(CONTENT_LENGTH_UNKNOWN);
      sendClose(200, "text/html", "");
      server_.sendContent_P(UPDATE_P1);
      server_.sendContent(FIRMWARE_VERSION);
      server_.sendContent_P(UPDATE_P1B);
      server_.sendContent_P(UPDATE_P2);
      server_.client().flush();
    });

    // ===== Firmware Upload Handler (POST /update) =====
    server_.on("/update", HTTP_POST, [&]() {
      // Handler completion - called after all chunks received
      if (Update.isRunning()) {
        if (Update.end(true)) {
          renderer_.setStatus(STATUS_OTA_SUCCESS, 1500);
          sendClose(200, "text/plain", "Update successful, rebooting...");
          delay(1000);
          ESP.restart();
        } else {
          renderer_.setStatus(STATUS_OTA_FAILED, 2000);
          sendClose(500, "text/plain", String("Update failed: ") + Update.getError());
        }
      } else {
        sendClose(400, "text/plain", "No update in progress");
      }
    }, [&]() {
      // Handler upload chunks
      HTTPUpload &upload = server_.upload();

      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("[FW] Update starting: %s, size: %u\n", upload.filename.c_str(), upload.totalSize);
        if (upload.totalSize > 0 && upload.totalSize > (ESP.getFreeSketchSpace() - 0x1000)) {
          Serial.println("[FW] Not enough free space");
          sendClose(413, "text/plain", "File too large");
          return;
        }
        renderer_.setStatus(STATUS_OTA_UPDATE, 60000);  // 60s timeout
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
          Serial.printf("[FW] Update.begin() failed, error: %d\n", Update.getError());
          sendClose(500, "text/plain", "Update.begin failed");
          return;
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        esp_task_wdt_reset();
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Serial.printf("[FW] Write failed, error: %d\n", Update.getError());
          sendClose(500, "text/plain", "Update.write failed");
          Update.abort();
          return;
        }
        // Rate-limited: per-chunk logging at 115200 baud added ~5 ms per
        // ~1.4 KB chunk, stretching every web OTA by seconds and starving
        // handleClient(). (The old line also printed a bogus byte count.)
        static uint32_t lastFwLogMs = 0;
        if (millis() - lastFwLogMs >= 500) {
          lastFwLogMs = millis();
          Serial.printf("[FW] Received: %u bytes\n", upload.totalSize + upload.currentSize);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        Serial.println("[FW] Upload complete, finalizing...");
      }
    });

    server_.on("/demo/overlay", HTTP_GET, [&]() {
      sendClose(200, "text/html", OVERLAY_HTML);
    });
  }

  String settingsJson() {
    const ClockSettings &s = settings_.get();
    char oc[8], fc[8], sc[8], mc[8], hc[8], mfc[8], ifc[8], ihc[8], cc[8];
    snprintf(oc, sizeof(oc), "#%02X%02X%02X", s.outerMarkerRed, s.outerMarkerGreen, s.outerMarkerBlue);
    snprintf(fc, sizeof(fc), "#%02X%02X%02X", s.outerFillerRed, s.outerFillerGreen, s.outerFillerBlue);
    snprintf(sc, sizeof(sc), "#%02X%02X%02X", s.secondsRed, s.secondsGreen, s.secondsBlue);
    snprintf(mc, sizeof(mc), "#%02X%02X%02X", s.minutesRed, s.minutesGreen, s.minutesBlue);
    snprintf(hc, sizeof(hc), "#%02X%02X%02X", s.hoursRed, s.hoursGreen, s.hoursBlue);
    snprintf(mfc, sizeof(mfc), "#%02X%02X%02X", s.middleFaceRed, s.middleFaceGreen, s.middleFaceBlue);
    snprintf(ifc, sizeof(ifc), "#%02X%02X%02X", s.innerFaceRed, s.innerFaceGreen, s.innerFaceBlue);
    snprintf(ihc, sizeof(ihc), "#%02X%02X%02X", s.innerHourRed, s.innerHourGreen, s.innerHourBlue);
    snprintf(cc, sizeof(cc), "#%02X%02X%02X", s.centerRed, s.centerGreen, s.centerBlue);
    char buf[1600];
    int n = snprintf(buf, sizeof(buf),
      "{\"dayBrightness\":%u,\"nightBrightness\":%u"
      ",\"nightStartHour\":%u,\"nightEndHour\":%u"
      ",\"colorTheme\":%u,\"secondTrail\":%u,\"progressSeconds\":%u"
      ",\"hourlyChime\":%u,\"statusAnimations\":%u"
      ",\"outerMarkerColor\":\"%s\",\"outerMarkerLevel\":%u"
      ",\"outerFillerColor\":\"%s\",\"outerFillerLevel\":%u"
      ",\"secondsColor\":\"%s\",\"secondsLevel\":%u"
      ",\"minutesColor\":\"%s\",\"minutesLevel\":%u"
      ",\"hoursColor\":\"%s\",\"hoursLevel\":%u"
      ",\"middleFaceColor\":\"%s\""
      ",\"innerFaceColor\":\"%s\""
      ",\"innerHourColor\":\"%s\",\"innerHourLevel\":%u"
      ",\"centerColor\":\"%s\",\"centerLevel\":%u"
      ",\"autoBrightnessMode\":%u,\"minAutoBrightness\":%u,\"maxAutoBrightness\":%u"
      ",\"quarterAnimation\":%u,\"halfHourAnimation\":%u,\"hourAnimation\":%u"
      ",\"intervalAnimationsEnabled\":%u"
      ",\"focusReminder_enabled\":%u,\"focusReminder_startHour\":%u"
      ",\"focusReminder_endHour\":%u,\"focusReminder_intervalMinutes\":%u"
      ",\"focusReminder_daysMask\":%u,\"focusReminder_animation\":%u"
      ",\"focusReminder_durationSeconds\":%u,\"outerRingOffset\":%u"
      ",\"animationPalette\":%u,\"animationSpeed\":%u"
      ",\"animationBrightness\":%u,\"trailLength\":%u"
      ",\"reminderPalette\":%u"
      ",\"outerRingBrightness\":%u"
      ",\"middleFaceScale\":%u,\"innerFaceScale\":%u"
      ",\"darkRoomOff\":%u"
      ",\"hasUserDefaults\":%s}",
      s.dayBrightness, s.nightBrightness,
      s.nightStartHour, s.nightEndHour,
      s.colorTheme, s.secondTrail, s.progressSeconds,
      s.hourlyChime, s.statusAnimations,
      oc, s.outerMarkerLevel,
      fc, s.outerFillerLevel,
      sc, s.secondsLevel,
      mc, s.minutesLevel,
      hc, s.hoursLevel,
      mfc,
      ifc,
      ihc, s.innerHourLevel,
      cc, s.centerLevel,
      s.autoBrightnessMode, s.minAutoBrightness, s.maxAutoBrightness,
      s.quarterAnimation, s.halfHourAnimation, s.hourAnimation,
      s.intervalAnimationsEnabled,
      s.focusReminder_enabled, s.focusReminder_startHour,
      s.focusReminder_endHour, s.focusReminder_intervalMinutes,
      s.focusReminder_daysMask, s.focusReminder_animation,
      s.focusReminder_durationSeconds, s.outerRingOffset,
      s.animationPalette, s.animationSpeed,
      s.animationBrightness, s.trailLength,
      s.reminderPalette,
      s.outerRingBrightness,
      s.middleFaceScale, s.innerFaceScale,
      s.darkRoomOff,
      settings_.hasUserDefaults() ? "true" : "false");
    if (n < 0 || n >= (int)sizeof(buf)) {
      // Truncated JSON breaks the whole web UI (loadSettings JSON.parse fails).
      Serial.printf("[WEB] settingsJson truncated (%d/%u) — grow buf\n", n, (unsigned)sizeof(buf));
    }
    return String(buf);
  }

  static String boolJson(bool value) {
    return value ? "true" : "false";
  }

  static uint8_t clampByte(int value, int minValue, int maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return static_cast<uint8_t>(value);
  }

  static uint16_t clampWord(int value, int minValue, int maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return static_cast<uint16_t>(value);
  }

  static String colorHex(uint8_t r, uint8_t g, uint8_t b) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", r, g, b);
    return String(buffer);
  }

  static int hexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  }

  static void parseColor(const String &value, uint8_t &r, uint8_t &g, uint8_t &b) {
    uint8_t start = value.startsWith("#") ? 1 : 0;
    if (value.length() - start != 6) return;
    int digits[6];
    for (uint8_t i = 0; i < 6; ++i) {
      digits[i] = hexNibble(value[start + i]);
      if (digits[i] < 0) return;
    }
    r = static_cast<uint8_t>((digits[0] << 4) | digits[1]);
    g = static_cast<uint8_t>((digits[2] << 4) | digits[3]);
    b = static_cast<uint8_t>((digits[4] << 4) | digits[5]);
  }

  static const char *wifiStatusText(wl_status_t status) {
    switch (status) {
      case WL_IDLE_STATUS:
        return "idle";
      case WL_NO_SSID_AVAIL:
        return "ssid not found";
      case WL_SCAN_COMPLETED:
        return "scan completed";
      case WL_CONNECTED:
        return "connected";
      case WL_CONNECT_FAILED:
        return "connect failed";
      case WL_CONNECTION_LOST:
        return "connection lost";
      case WL_DISCONNECTED:
        return "disconnected";
      default:
        return "unknown";
    }
  }

  String htmlPage() { return ""; }

  // Adds Connection:close so the browser closes the TCP socket immediately after
  // each response. Without this the ESP32 WebServer sits in HC_WAIT_CLOSE for
  // HTTP_MAX_CLOSE_WAIT (2 s default), blocking all new connections during that
  // window and making the UI feel frozen under rapid saves / previews.
  void sendClose(int code, const char *contentType, const String &body) {
    server_.sendHeader("Connection", "close");
    server_.send(code, contentType, body);
  }

  TimeModel &model_;
  SettingsStore &settings_;
  ClockRenderer &renderer_;
  TimeSync &timeSync_;
  TemperatureInput &temperature_;
  LuxSensor *lux_;
  ClockWebServer server_;
  bool enabled_ = false;
  bool mdnsEnabled_ = false;
  bool apMode_ = false;
  uint32_t apRetryMs_ = 0;
};

// ===================== Focus Reminder Scheduler =====================
class FocusReminderScheduler {
 public:
  explicit FocusReminderScheduler(TimeModel &model, ClockRenderer &renderer, SettingsStore &settings)
      : model_(model), renderer_(renderer), settings_(settings) {}

  void checkAndFire(uint32_t now) {
    const ClockSettings &s = settings_.get();
    if (!s.focusReminder_enabled) return;

    ClockTime t = model_.get();

    // No days selected = never fire (matches pre-existing semantics).
    if (s.focusReminder_daysMask == 0) return;

    // Day-of-week comes from the system epoch (set only by NTP). If the epoch
    // was never synced (offline boot, time set via buttons/web), tm_wday is a
    // 1970 artifact — skip the day filter rather than silently never firing.
    // The hour window below still applies; it uses TimeModel, not the epoch.
    if (time(nullptr) >= 1700000000) {
      uint8_t dow = getDayOfWeek();
      if (!(s.focusReminder_daysMask & (1 << dow))) return;
    }

    // Time window check
    bool inWindow = false;
    if (s.focusReminder_startHour < s.focusReminder_endHour) {
      // Normal window (e.g., 08:00-22:00)
      inWindow = (t.hour >= s.focusReminder_startHour && t.hour < s.focusReminder_endHour);
    } else if (s.focusReminder_startHour > s.focusReminder_endHour) {
      // Wrapped window (e.g., 22:00-08:00, crosses midnight)
      inWindow = (t.hour >= s.focusReminder_startHour || t.hour < s.focusReminder_endHour);
    }
    if (!inWindow) return;

    // Interval check: enough time since last fire?
    uint32_t intervalMs = static_cast<uint32_t>(s.focusReminder_intervalMinutes) * 60000UL;
    if (lastFireMs_ != 0 && now - lastFireMs_ < intervalMs) return;

    // Fire animation (reuse existing renderer methods)
    triggerReminderAnimation(s.focusReminder_animation, now);

    // Record fire time in RAM only — no EEPROM write needed
    lastFireMs_ = now;

    Serial.printf("[FocusReminder] Fired at %02d:%02d (interval=%d min)\n",
                  t.hour, t.minute, s.focusReminder_intervalMinutes);
  }

 private:
  uint8_t getDayOfWeek() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    return static_cast<uint8_t>(t.tm_wday);  // 0=Sun … 6=Sat
  }

  void triggerReminderAnimation(uint8_t mode, uint32_t now) {
    renderer_.triggerReminderDirectAnimation(mode, now);
  }

  TimeModel &model_;
  ClockRenderer &renderer_;
  SettingsStore &settings_;
  uint32_t lastFireMs_ = 0;
};

class ButtonInput {
 public:
  enum class HoldPhase : uint8_t { IDLE, REPEAT_MIN, REPEAT_HOUR };

  void begin() {
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  }

  void poll(uint32_t now) {
    bool upNow   = (digitalRead(BUTTON_UP_PIN)   == LOW);
    bool downNow = (digitalRead(BUTTON_DOWN_PIN) == LOW);

    // --- UP button ---
    if (upNow && !lastUp_ && now - lastUpMs_ > kDebounceMs) {
      // Fresh press: debounce passed, leading edge
      upPressed_     = true;
      upPressedAt_   = now;
      upLastRepeat_  = now;
      upPhase_       = HoldPhase::IDLE;
      lastUpMs_      = now;
    } else if (upNow && lastUp_) {
      // Held: advance hold phase and fire repeats
      uint32_t held = now - upPressedAt_;
      if (held >= kHoldHourMs) {
        upPhase_ = HoldPhase::REPEAT_HOUR;
      } else if (held >= kHoldRepeatMs) {
        if (upPhase_ == HoldPhase::IDLE) upPhase_ = HoldPhase::REPEAT_MIN;
      }
      if (upPhase_ != HoldPhase::IDLE && now - upLastRepeat_ >= kRepeatIntervalMs) {
        upRepeat_     = true;
        upLastRepeat_ = now;
      }
    } else if (!upNow) {
      // Released
      upPhase_ = HoldPhase::IDLE;
    }

    // --- DOWN button ---
    if (downNow && !lastDown_ && now - lastDownMs_ > kDebounceMs) {
      downPressed_     = true;
      downPressedAt_   = now;
      downLastRepeat_  = now;
      downPhase_       = HoldPhase::IDLE;
      lastDownMs_      = now;
    } else if (downNow && lastDown_) {
      uint32_t held = now - downPressedAt_;
      if (held >= kHoldHourMs) {
        downPhase_ = HoldPhase::REPEAT_HOUR;
      } else if (held >= kHoldRepeatMs) {
        if (downPhase_ == HoldPhase::IDLE) downPhase_ = HoldPhase::REPEAT_MIN;
      }
      if (downPhase_ != HoldPhase::IDLE && now - downLastRepeat_ >= kRepeatIntervalMs) {
        downRepeat_     = true;
        downLastRepeat_ = now;
      }
    } else if (!downNow) {
      downPhase_ = HoldPhase::IDLE;
    }

    lastUp_   = upNow;
    lastDown_ = downNow;
  }

  // Returns non-zero delta if a press or repeat fired, 0 otherwise.
  // Positive = UP direction, negative = DOWN direction.
  int consumeUp() {
    if (upPressed_) { upPressed_ = false; ++g_buttonEventCount; return 1; }
    if (upRepeat_)  { upRepeat_  = false; ++g_buttonEventCount;
                      return (upPhase_ == HoldPhase::REPEAT_HOUR) ? 60 : 1; }
    return 0;
  }
  int consumeDown() {
    if (downPressed_) { downPressed_ = false; ++g_buttonEventCount; return -1; }
    if (downRepeat_)  { downRepeat_  = false; ++g_buttonEventCount;
                        return (downPhase_ == HoldPhase::REPEAT_HOUR) ? -60 : -1; }
    return 0;
  }

  // Legacy single-press API kept for any callers that only care about edge.
  bool consumeUpPress()   { return consumeUp()   != 0; }
  bool consumeDownPress() { return consumeDown() != 0; }

 private:
  static constexpr uint32_t kDebounceMs      =  50;
  static constexpr uint32_t kHoldRepeatMs    = 500;
  static constexpr uint32_t kRepeatIntervalMs= 150;
  static constexpr uint32_t kHoldHourMs      = 2000;

  bool upPressed_   = false;
  bool downPressed_ = false;
  bool upRepeat_    = false;
  bool downRepeat_  = false;
  bool lastUp_      = false;
  bool lastDown_    = false;

  uint32_t lastUpMs_      = 0;
  uint32_t lastDownMs_    = 0;
  uint32_t upPressedAt_   = 0;
  uint32_t downPressedAt_ = 0;
  uint32_t upLastRepeat_  = 0;
  uint32_t downLastRepeat_= 0;

  HoldPhase upPhase_   = HoldPhase::IDLE;
  HoldPhase downPhase_ = HoldPhase::IDLE;
};

// ===================== Globals =====================
Adafruit_NeoPixel ledStrip(CLOCK_PIXEL_COUNT, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);
#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT
Adafruit_NeoPixel centerLedStrip(CENTER_PIXEL_STRIP_COUNT, CENTER_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif
SettingsStore settingsStore;
TimeModel timeModel;
#if CENTER_PIXEL_ENABLED && CENTER_PIXEL_SEPARATE_OUTPUT
ClockRenderer renderer(ledStrip, settingsStore, &centerLedStrip);
#else
ClockRenderer renderer(ledStrip, settingsStore);
#endif
TemperatureInput temperature;
LuxSensor luxSensor;
TimeSync timeSync(timeModel);
WebUi webUi(timeModel, settingsStore, renderer, timeSync, temperature);
FocusReminderScheduler reminderScheduler(timeModel, renderer, settingsStore);
DemoMode demoMode(renderer, luxSensor, settingsStore);
ButtonInput buttons;

uint32_t lastTickMs = 0;
uint32_t lastAnimationRenderMs = 0;
uint32_t lastStatusLogMs = 0;
static uint8_t lastMinute = 255;

static void setupDemoModeRoutes() {
  webUi.getServer().on("/demo/start", HTTP_POST, [](){ demoMode.start(); webUi.getServer().sendHeader("Connection","close"); webUi.getServer().send(200, "application/json", "{\"status\":\"started\"}"); });
  webUi.getServer().on("/demo/stop", HTTP_POST, [](){ demoMode.stop(); webUi.getServer().sendHeader("Connection","close"); webUi.getServer().send(200, "application/json", "{\"status\":\"stopped\"}"); });
  webUi.getServer().on("/demo/status", HTTP_GET, [](){ webUi.getServer().sendHeader("Connection","close"); webUi.getServer().send(200, "application/json", demoMode.statusJson()); });
}

static void logRuntimeStatus(uint32_t now) {
  if (now - lastStatusLogMs < 10000) return;
  lastStatusLogMs = now;

  ClockTime t = timeModel.get();
  uint32_t uptimeSec = now / 1000;
  const ClockSettings &s = settingsStore.get();

  Serial.printf("\n[%02d:%02d:%02d] uptime=%02lu:%02lu:%02lu  heap=%lu B free\n",
                t.hour, t.minute, t.second,
                uptimeSec / 3600, (uptimeSec % 3600) / 60, uptimeSec % 60,
                static_cast<unsigned long>(ESP.getFreeHeap()));

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("  WiFi  : %s  IP=%s  RSSI=%d dBm\n",
                  WiFi.SSID().c_str(),
                  WiFi.localIP().toString().c_str(),
                  WiFi.RSSI());
  } else {
    Serial.printf("  WiFi  : OFFLINE (status=%d)\n", WiFi.status());
  }

#if LUX_SENSOR_ENABLED
  if (luxSensor.available()) {
    float lx = luxSensor.lux();
    uint8_t brRamped = luxSensor.autoBrightnessCached(now);
    uint8_t brTarget = luxSensor.autoBrightnessTarget();
    uint8_t brEffective = constrain((int)brRamped, s.minAutoBrightness, s.maxAutoBrightness);
    Serial.printf("  Lux   : %.1f lux  br=%d->%d(eff=%d)  mode=%d  min=%d  max=%d\n",
                  lx, brTarget, brRamped, brEffective,
                  s.autoBrightnessMode, s.minAutoBrightness, s.maxAutoBrightness);
  } else {
    Serial.println("  Lux   : VEML7700 not available");
  }
#endif

  Serial.printf("  LEDs  : count=%d  ringOffset(hw)=%d  defaultRot=%d  rotOffset(sw)=%d  center=%d  sac=%s\n",
                CLOCK_PIXEL_COUNT, RING_PIXEL_OFFSET, DEFAULT_OUTER_RING_OFFSET, s.outerRingOffset,
                CENTER_PIXEL_INDEX, SACRIFICIAL_PIXEL_ENABLED ? "yes" : "no");
  Serial.printf("  NTP   : %s\n", timeSync.synced() ? "synced" : "waiting");
}

void setup() {
#if STATUS_LED_PIN >= 0
  pinMode(STATUS_LED_PIN, OUTPUT);
#endif
  writeStatusLed(false);

  Serial.begin(115200);
  const uint32_t serialStartMs = millis();
  while (!Serial && millis() - serialStartMs < 2000) {
    delay(10);
  }
  Serial.println();
  Serial.println("ChronoBloom v3 booting...");
  {
    esp_reset_reason_t rr = esp_reset_reason();
    const char *rrStr = "unknown";
    switch (rr) {
      case ESP_RST_POWERON:  rrStr = "power-on";         break;
      case ESP_RST_SW:       rrStr = "software reset";   break;
      case ESP_RST_PANIC:    rrStr = "panic/exception";  break;
      case ESP_RST_INT_WDT:  rrStr = "interrupt WDT";   break;
      case ESP_RST_TASK_WDT: rrStr = "task WDT";        break;
      case ESP_RST_WDT:      rrStr = "watchdog";        break;
      case ESP_RST_BROWNOUT: rrStr = "brownout";        break;
      case ESP_RST_DEEPSLEEP:rrStr = "deep-sleep wake"; break;
      default: break;
    }
    Serial.printf("Reset reason: %s (%d)\n", rrStr, (int)rr);
  }
  Serial.print("Build target: ");
  Serial.println(DEVICE_TITLE);
  Serial.print("LED count: ");
  Serial.println(CLOCK_PIXEL_COUNT);
  Serial.print("Ring pixel offset: ");
  Serial.println(RING_PIXEL_OFFSET);
  Serial.print("Center pixel: ");
  if (CENTER_PIXEL_ENABLED) {
#if CENTER_PIXEL_SEPARATE_OUTPUT
    Serial.print("separate output GPIO ");
    Serial.print(CENTER_PIXEL_PIN);
    Serial.print(", pixel index ");
    Serial.println(CENTER_PIXEL_INDEX);
#else
    Serial.print("enabled at physical index ");
    Serial.println(CENTER_PIXEL_INDEX);
#endif
  } else {
    Serial.println("disabled");
  }
  Serial.print("Sacrificial pixel: ");
  if (SACRIFICIAL_PIXEL_ENABLED) {
    Serial.print("enabled at physical index ");
    Serial.println(SACRIFICIAL_PIXEL_INDEX);
  } else {
    Serial.println("disabled");
  }
#if LUX_SENSOR_ENABLED
  Wire.begin(LUX_SENSOR_I2C_SDA, LUX_SENSOR_I2C_SCL);
#endif
  buttons.begin();
  settingsStore.begin();
  temperature.begin();
  luxSensor.begin();
  renderer.setLuxSensor(&luxSensor);
  renderer.begin();

  // Factory reset: hold UP (GPIO5) at power-on to enter prompt, then hold DOWN (GPIO9)
  // for 3 continuous seconds to confirm.  GPIO9 is the XIAO BOOT pin — it must NOT be
  // held at the reset instant or the chip enters download mode.  Requiring UP-only at
  // power-on avoids this; DOWN can safely be added once firmware is running.
  if (digitalRead(BUTTON_UP_PIN) == LOW) {
    Serial.println("[FactoryReset] UP held at boot — add DOWN within 5s and hold both 3s to reset.");
    ledStrip.setBrightness(200);
    ledStrip.fill(ledStrip.Color(255, 0, 0));
    ledStrip.show();

    bool confirmed = false;
    const uint32_t windowStart = millis();
    uint32_t bothHeldSince = 0;
    bool bothHeld = false;

    while (millis() - windowStart < 5000) {
      const bool upLow  = (digitalRead(BUTTON_UP_PIN)   == LOW);
      const bool downLow = (digitalRead(BUTTON_DOWN_PIN) == LOW);

      if (!upLow) break;  // UP released: cancel immediately

      if (upLow && downLow) {
        if (!bothHeld) { bothHeld = true; bothHeldSince = millis(); }
        if (millis() - bothHeldSince >= 3000) { confirmed = true; break; }
      } else {
        bothHeld = false;  // DOWN released: reset hold timer
      }
      delay(50);
    }

    if (confirmed) {
      Serial.println("[FactoryReset] Confirmed — clearing settings, forcing WiFi portal, rebooting.");
      settingsStore.resetToDefaults();
      Preferences prefs;
      prefs.begin("factory", false);
      prefs.putBool("portal", true);
      prefs.end();
      for (int i = 0; i < 2; i++) {
        ledStrip.fill(ledStrip.Color(255, 255, 255));
        ledStrip.show();
        delay(400);
        ledStrip.clear();
        ledStrip.show();
        delay(200);
      }
      ESP.restart();
    } else {
      Serial.println("[FactoryReset] Cancelled — resuming normal boot.");
      ledStrip.setBrightness(settingsStore.get().dayBrightness);
      ledStrip.clear();
      ledStrip.show();
    }
  }

  webUi.setLuxSensor(&luxSensor);
  webUi.begin();
  if (webUi.enabled()) {
    setupDemoModeRoutes();
    writeStatusLed(true);
    Serial.print("Web UI available at IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Web UI hostname: http://");
    Serial.print(DEVICE_HOSTNAME);
    Serial.println(".local/");
  } else {
    Serial.println("Web UI disabled (Wi-Fi not connected). Clock still runs offline.");
  }

  lastTickMs = millis();

  // Configure task watchdog: 10-second window, reboot on timeout.
  esp_task_wdt_init(10, true);  // 10s timeout, panic (reboot) on trigger
  esp_task_wdt_add(NULL);       // subscribe the Arduino loop task
}

void loop() {
  esp_task_wdt_reset();
  const uint32_t now = millis();
  buttons.poll(now);

  while (now - lastTickMs >= 1000) {
    timeModel.tickOneSecond();
    lastTickMs += 1000;
  }

  timeSync.loop();
  webUi.loop();
  demoMode.loop(now);
  logRuntimeStatus(now);

  // Check and fire focus reminders
  reminderScheduler.checkAndFire(now);

  // Trigger interval animations (quarter, half-hour, hourly)
  const ClockTime time = timeModel.get();
  const ClockSettings &settings = settingsStore.get();

  if (settings.intervalAnimationsEnabled && !renderer.animating()) {
    if (time.minute != lastMinute && time.second == 0) {
      lastMinute = time.minute;
      if (time.minute == 0) {
        renderer.triggerHourAnimation(now);
      } else if (time.minute == 30) {
        renderer.triggerHalfHourAnimation(now);
      } else if (time.minute % 15 == 0) {
        renderer.triggerQuarterAnimation(now);
      }
    }
  }

  {
    int upDelta = buttons.consumeUp();
    if (upDelta != 0) { timeModel.addMinutes(upDelta); renderer.setStatus(STATUS_BUTTON, 700); }
  }
  {
    int downDelta = buttons.consumeDown();
    if (downDelta != 0) { timeModel.addMinutes(downDelta); renderer.setStatus(STATUS_BUTTON, 700); }
  }

  if (renderer.animating()) {
    if (now - lastAnimationRenderMs >= 25) {  // cap at ~40fps; keeps loop fast for handleClient()
      renderer.renderAnimFrame(millis());  // fresh millis(): animStartMs_ may be newer than loop's now
      lastAnimationRenderMs = now;
    }
    if (!renderer.animating()) {
      timeModel.markDirty();
    }
  } else if (timeModel.consumeDirty()) {
    renderer.render(timeModel.get());
    lastAnimationRenderMs = now;
  } else if (renderer.needsFullAnimationFrame(now) && now - lastAnimationRenderMs >= 80) {
    renderer.render(timeModel.get());
    lastAnimationRenderMs = now;
  } else if (renderer.needsBrightnessRampFrame(now) && now - lastAnimationRenderMs >= 60) {
    // Keep re-rendering at ~16fps while auto-brightness is ramping so the
    // lux->brightness ramp actually shows up instead of only advancing once
    // per second on the idle time-tick render.
    renderer.render(timeModel.get());
    lastAnimationRenderMs = now;
  } else if (renderer.needsCenterAnimationFrame() && now - lastAnimationRenderMs >= 50) {
    // Full render instead of center-only: renderCenterAnimationFrame() called
    // setBrightness() every 80ms which re-scales the whole pixel buffer via
    // lossy integer division. Over ~12 frames/sec, ring pixels drifted ~7 units
    // darker than their correct value. render() on tick snapped them back —
    // creating a 1Hz brightness pulse on the outer ring. Inner ring ambient
    // pixels (scale 22-24/255) were too dim to show the drift, making them
    // appear "not in sync" with the outer ring. Full render here is safe:
    // strip_.clear() only zeroes RAM — it never calls show() — so no blank frame.
    renderer.render(timeModel.get());
    lastAnimationRenderMs = now;
  }
}
