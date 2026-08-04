#pragma once
#include <Arduino.h>

// PROGMEM string constants extracted from WebUi::setupRoutes.
// Included by main.cpp; all symbols have internal linkage (static).

// ===== Index page (GET /) =====

static const char INDEX_P1[] PROGMEM =
  "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>\n"
  "<title>ChronoBloom</title>\n"
  "<style>\n"
  ":root{color-scheme:dark;--bg:#090b10;--panel:#151922;--panel2:#10141c;--line:#2c3442;--text:#eef3fb;--muted:#92a0b5;--accent:#6bd7ff}\n"
  "*{box-sizing:border-box}body{margin:0;background:radial-gradient(circle at 50% 12%,#17202f 0,#090b10 54%);color:var(--text);font:14px/1.42 system-ui,Segoe UI,sans-serif}\n"
  "main{display:grid;grid-template-columns:320px 1fr;gap:12px;max-width:1080px;margin:0 auto;padding:12px}\n"
  ".stage{position:sticky;top:12px;align-self:start;background:linear-gradient(180deg,var(--panel),var(--panel2));border:1px solid var(--line);border-radius:12px;padding:12px;text-align:center;box-shadow:0 18px 45px #0008}\n"
  ".clock-wrap{display:grid;place-items:center;min-height:150px}\n"
  "svg{width:min(100%,300px);height:auto;display:block;margin:0 auto}.led{opacity:.16;transition:fill .18s,opacity .18s,filter .18s}.on{opacity:1;filter:drop-shadow(0 0 5px currentColor)}.ghost{opacity:.4}.marker{opacity:.5}.center{filter:drop-shadow(0 0 12px currentColor)}\n"
  "h1{font-size:16px;margin:0 0 2px}.sub{color:var(--muted);font-size:12px;margin:0 0 8px}#now{font-size:30px;font-weight:750;margin:4px 0 1px;font-variant-numeric:tabular-nums}.state{color:var(--muted);font-size:12px;line-height:1.4;min-height:18px}\n"
  ".netline{display:flex;gap:8px;align-items:center;margin-top:8px;flex-wrap:wrap;text-align:left}.netline #net{flex:1 1 auto;min-width:0;font-size:11px}\n"
  ".savebar{display:flex;gap:6px;margin-top:10px}.savebar button{flex:1}\n"
  ".dash{min-width:0;display:flex;flex-direction:column;gap:9px}\n"
  ".quick{background:linear-gradient(180deg,#182236,#111826);border:1px solid #33465f;border-radius:12px;padding:10px 12px}\n"
  ".quick h3{margin:0 0 6px;font-size:11px;color:#bcd0ea;text-transform:uppercase;letter-spacing:.6px}\n"
  ".qlabel{font-size:11px;color:var(--muted);margin:8px 0 4px;display:block}\n"
  ".themes{display:grid;grid-template-columns:repeat(auto-fill,minmax(100px,1fr));gap:5px}\n"
  ".themechip{display:flex;align-items:center;gap:6px;border:1px solid var(--line);border-radius:9px;padding:4px 7px;cursor:pointer;background:#0c1017;font-size:12px}.themechip .dot{width:20px;height:20px;border-radius:6px;flex:none;border:1px solid #ffffff22}.themechip.sel{border-color:var(--accent);box-shadow:0 0 0 1px var(--accent) inset}\n"
  ".palettePick{display:flex;gap:8px;align-items:center}.swatch{display:inline-block;width:24px;height:24px;border-radius:6px;border:1px solid #374253;flex:none}\n"
  ".brightwrap{display:flex;align-items:center;gap:9px}.brightwrap>span{font-size:15px}.brightwrap input[type=range]{flex:1;min-width:0}.brightwrap output{font-size:11px;color:var(--muted);min-width:26px;text-align:right}\n"
  ".grid2{display:grid;grid-template-columns:1fr 1fr;gap:8px}\n"
  "details.drawer{background:linear-gradient(180deg,var(--panel),var(--panel2));border:1px solid var(--line);border-radius:12px;overflow:hidden}\n"
  "details.drawer>summary{cursor:pointer;list-style:none;display:flex;align-items:center;gap:10px;padding:10px 12px}details.drawer>summary::-webkit-details-marker{display:none}details.drawer>summary .ic{font-size:16px}details.drawer>summary .nm{font-weight:650;font-size:13px;color:#dbe7f7}details.drawer>summary .hint{margin-left:auto;font-size:11px;color:var(--muted);text-align:right}details.drawer>summary::after{content:'\\25B8';color:var(--muted);font-size:12px;margin-left:8px}details.drawer[open]>summary::after{content:'\\25BE'}\n"
  "details.drawer.primary{border-color:#2d6f9c;box-shadow:0 0 0 1px #2d6f9c44,0 6px 20px #0006}details.drawer.primary>summary{background:linear-gradient(180deg,#16324a,#101826)}details.drawer.primary>summary .nm{color:#eaf4ff}\n"
  ".drawerbody{padding:0 12px 12px}\n"
  ".subhead{font-size:11px;text-transform:uppercase;letter-spacing:.5px;color:#7f8ea3;margin:11px 0 5px}.hair{height:1px;background:var(--line);margin:9px 0}\n"
  "label{display:block;color:var(--muted);font-size:11px;margin:6px 0 3px}input,select,button{font:inherit;font-size:12px;border-radius:6px;border:1px solid #374253;background:#0c1017;color:var(--text);padding:6px 8px;min-height:30px}\n"
  "input[type=number]{width:74px}input[type=color]{width:44px;padding:2px;min-height:26px}button{cursor:pointer;background:#203146;border-color:#42546d}button.primary{background:#145875;border-color:#2d9ccb;color:#fff}\n"
  ".row{display:flex;gap:8px;align-items:end;flex-wrap:wrap}\n"
  ".ringrow{display:grid;grid-template-columns:64px 44px 1fr 30px;gap:8px;align-items:center;margin:3px 0}.ringrow span{color:#dbe7f7;font-size:12px}.ringrow input[type=range]{min-width:0}.ringrow output{font-size:11px;text-align:right}\n"
  ".toggle{display:flex;gap:6px;flex-wrap:wrap}.toggle label{display:flex;gap:6px;align-items:center;margin:0;color:#dce6f5;background:#0c1017;border:1px solid #303846;border-radius:6px;padding:6px 8px;font-size:12px}\n"
  "@media(max-width:760px){main{grid-template-columns:1fr}.stage{position:static}"
  "input,select,button{font-size:16px;min-height:42px}"  /* 16px stops iOS auto-zoom on focus; 42px = touch target */
  "input[type=color]{min-height:36px}"
  "input[type=checkbox]{min-height:0;width:18px;height:18px}"  /* 42px would stretch checkboxes to 13x42 slivers */
  ".savebar{flex-wrap:wrap}.savebar button{flex:1 1 40%}.themechip{padding:8px 10px}}\n"
  "</style></head>\n";

static const char INDEX_P2[] PROGMEM =
  "<body><main>\n"
  "<section class='stage'>\n"
  "<h1>ChronoBloom <span id='fwVersion' style='font-size:11px;font-weight:400;color:var(--muted);vertical-align:middle'></span></h1>\n"
  "<p class='sub'>Outer ring = clock face + hands; middle/inner rings show the hours.</p>\n"
  "<div class='clock-wrap'><svg id='clockSvg' viewBox='0 0 420 420' role='img' aria-label='NeoPixel clock preview'></svg></div>\n"
  "<div id='now'>--:--:--</div><div id='state' class='state'>Connecting...</div>\n"
  "<div class='netline'><div id='net' class='state'>--</div><button onclick='loadNet()'>Refresh</button></div>\n"
  "<div class='savebar'><button onclick='previewOnClock()' title='Show the current unsaved settings on the clock for 10s, then revert.'>&#9654; Preview</button><button class='primary' onclick='saveSettings()'>Save</button><button onclick='saveAsDefault()' title='Save the current settings as the restore point for Reset'>Set default</button><button id='resetBtn' onclick='resetToDefaults()' style='color:#e88a8a'>Reset</button></div>\n"
  "<div class='sub' style='margin-top:4px'>Preview shows changes on the clock for 10s, then reverts. Save writes the settings on this page; the time zone has its own Save zone button.</div>\n"
  "<div id='defaultSavedMsg' style='display:none;color:#27ae60;font-size:11px;margin-top:4px'>&#10003; Saved as default</div>\n"
  "<div class='credit' style='margin-top:14px;font-size:10px;line-height:1.5;color:var(--muted);opacity:.7;text-align:center'>ChronoBloom. Built by a dad, for his kids. Spring 2026.<br><a href='https://github.com/Maestro8484/ChronoBloom' target='_blank' rel='noopener' style='color:var(--muted)'>source on GitHub</a></div>\n"
  "</section>\n"
  "<div class='dash'>\n"
  "<div id='tzBanner' style='display:none;background:linear-gradient(180deg,#3a2c12,#241c0c);border:1px solid #8a6d2f;border-radius:12px;padding:10px 12px'>\n"
  "<b>Set your time zone.</b> <span id='tzBannerTxt'></span>\n"
  "<div class='row' style='margin-top:6px'><button class='primary' id='tzBannerBtn' onclick='tzBannerApply()'>Use detected zone</button><button onclick='tzBannerOpen()'>Pick manually</button></div>\n"
  "</div>\n"
  "<div class='quick'>\n"
  "<h3>Quick controls</h3>\n"
  "<span class='qlabel'>Theme</span>\n"
  "<div id='themeChips' class='themes'></div>\n"
  "<div style='margin-top:6px'><button onclick='saveAsTheme()' title='Save the current clock settings as a named theme (stored in this browser).' style='font-size:11px;padding:4px 8px;min-height:26px'>&#43; Save as theme&hellip;</button></div>\n"
  "<span class='qlabel'>Brightness &middot; <select id='autoBrightnessMode' onchange='updateBrightnessMode()' style='min-height:24px;padding:2px 6px;font-size:11px;width:auto;display:inline-block'><option value='0'>Manual</option><option value='1'>Auto (light sensor)</option><option value='2'>Scheduled</option></select></span>\n"
  "<div id='brightManual' class='brightwrap'><span>&#127769;</span><input id='dayBrightness' type='range' min='5' max='255'><span>&#9728;&#65039;</span><output id='dayBrightnessOut'></output></div>\n"
  "<div id='brightAuto' style='display:none'>\n"
  "  <label style='margin-top:2px'>Clock-face brightness range. Pitch black rests on Darkest, full daylight on Brightest, and the sensor glides the face between them. (Animations &amp; nudges have their own brightness: the &ldquo;Animation brightness&rdquo; slider in Motion &amp; nudges.)</label>\n"
  "  <label style='margin:5px 0 2px'>&#127769; Darkest: how dim the face goes in a pitch-dark room</label>\n"
  "  <div class='brightwrap'><span>&#127769;</span><input id='minAutoBrightness' type='range' min='5' max='255'><output id='minAutoBrightnessOut'></output></div>\n"
  "  <label style='margin:5px 0 2px'>&#9728;&#65039; Brightest: how bright the face goes in full daylight</label>\n"
  "  <div class='brightwrap'><span>&#9728;&#65039;</span><input id='maxAutoBrightness' type='range' min='5' max='255'><output id='maxAutoBrightnessOut'></output></div>\n"
  "  <div style='font-size:11px;color:var(--muted);margin-top:2px'>Current light: <span id='luxValue'>--</span> lux</div>\n"
  "</div>\n"
  "<div class='row' style='margin-top:8px'><button class='primary' onclick='saveBrightness()' title='Save the brightness settings to the clock.'>Save brightness</button></div>\n"
  "<span class='qlabel'>Focus nudges</span>\n"
  "<div class='row' style='align-items:center'><label style='display:flex;gap:6px;align-items:center;margin:0;color:#dce6f5;font-size:12px'><input id='quickRemEnabled' type='checkbox'> On</label><span style='font-size:12px;color:var(--muted)'>every</span><input id='quickRemInterval' type='number' min='1' max='1440' style='width:64px'><span style='font-size:12px;color:var(--muted)'>min</span><button class='primary' onclick='quickRemSave()' style='margin-left:auto'>Save nudge</button></div>\n"
  "<div class='sub' id='quickRemMsg' style='margin:2px 0 0'>A gentle light swell that asks you to look up. Press either clock button during one to say &ldquo;seen it&rdquo;. More options in Motion &amp; nudges.</div>\n"
  "</div>\n"
  "<details class='drawer primary'>\n"
  "<summary><span class='ic'>&#10024;</span><span class='nm'>Motion &amp; nudges</span><span class='hint'>animations &middot; reminders &middot; effects</span></summary>\n"
  "<div class='drawerbody'>\n"
  "<div class='subhead'>Animation style</div>\n"
  "<div class='grid2'>\n"
  "  <div><label title='Colors for the interval chime animations (quarter / half-hour / hour). Nudges use the Reminder colors picker below.'>Animation colors (interval chimes)</label><div class='palettePick'><select id='animationPalette' onchange='updatePaletteSwatches()'><option value='7'>Clock colors (default)</option><option value='0'>Golden hour (warm)</option><option value='1'>Moonlight (cool)</option><option value='2'>Dawn (soft-warm)</option><option value='3'>Twilight (muted-cool)</option></select><span id='animationPaletteSwatch' class='swatch'></span></div></div>\n"
  "  <div><label>Animation speed</label><select id='animationSpeed'><option value='1'>1 - Dreamy</option><option value='2'>2 - Relaxed</option><option value='3'>3 - Normal</option><option value='4'>4 - Energetic</option><option value='5'>5 - Hyper</option></select></div>\n"
  "</div>\n"
  "<div class='grid2'>\n"
  "  <div><label>Animation brightness (interval animations &amp; nudges)</label><input id='animationBrightness' type='range' min='50' max='255'><output id='animationBrightnessOut'></output></div>\n"
  "  <div><label>Animation trail length</label><input id='trailLength' type='range' min='2' max='12'><output id='trailLengthOut'></output></div>\n"
  "</div>\n"
  "<div class='row'><div><label title='Colors for reminder/nudge animations. Same palette list as Animation colors - pick a different one so a nudge reads differently from an interval chime.'>Reminder colors (nudges)</label><div class='palettePick'><select id='reminderPalette' onchange='updatePaletteSwatches()'><option value='7'>Clock colors (default)</option><option value='0'>Golden hour (warm)</option><option value='1'>Moonlight (cool)</option><option value='2'>Dawn (soft-warm)</option><option value='3'>Twilight (muted-cool)</option></select><span id='reminderPaletteSwatch' class='swatch'></span></div></div></div>\n"
  "<div class='row' style='gap:6px'><button onclick='previewAnimationTheme()' title='Preview your configured interval chime (hour, else half-hour, else quarter) in the selected animation colors. Reverts on its own.'>&#9654; Preview</button><button class='primary' onclick='saveAnimStyle()' title='Save the animation style: colors, speed, brightness, trail length, and reminder colors.'>Save style</button></div>\n"
  "<div class='subhead'>Trigger animations</div>\n"
  "<label><input id='intervalAnimationsEnabled' type='checkbox'> Enable interval animations</label>\n"
  "<div class='grid2'>\n"
  "  <div><label>Quarter (:15/:30/:45)</label><select id='quarterAnimation'><option value='0'>Off</option><option value='1'>Slow comet</option><option value='2'>Dual orbit</option><option value='3'>Bloom ripple</option></select><button onclick=\"previewAnim('quarter','quarterAnimation')\">&#9654;</button></div>\n"
  "  <div><label>Half-hour (:30)</label><select id='halfHourAnimation'><option value='0'>Off</option><option value='1'>Unfurl</option><option value='2'>Three comets</option><option value='3'>Breathe</option></select><button onclick=\"previewAnim('halfhour','halfHourAnimation')\">&#9654;</button></div>\n"
  "</div>\n"
  "<label>Top of hour (:00)</label><select id='hourAnimation'><option value='0'>Off</option><option value='1'>Ceremony</option><option value='2'>Galaxy spin</option><option value='3'>Supernova</option><option value='4'>Comet relay</option><option value='5'>Deep breath</option></select><button onclick=\"previewAnim('hour','hourAnimation')\">&#9654;</button>\n"
  "<div class='subhead'>Focus reminders (nudges)</div>\n"
  "<p class='sub' style='margin:0 0 4px'>A gentle nudge to look up and shift focus, at set intervals. If one goes unseen, the next asks twice. Pressing either clock button during or just after a nudge acknowledges it and restarts the interval.</p>\n"
  "<div class='toggle'><label><input id='focusReminder_enabled' type='checkbox'> Enable focus reminders</label></div>\n"
  "<div class='row' style='gap:6px;margin-top:4px'><button onclick='remPreset(25)'>Pomodoro 25m</button><button onclick='remPreset(30)'>Check-in 30m</button><button onclick='remPreset(60)'>Hourly</button></div>\n"
  "<div class='row'>\n"
  "  <div><label>Start hour</label><input id='focusReminder_startHour' type='number' min='0' max='23' placeholder='HH'></div>\n"
  "  <div><label>End hour</label><input id='focusReminder_endHour' type='number' min='0' max='23' placeholder='HH'></div>\n"
  "  <div><label>Interval (min)</label><input id='focusReminder_intervalMinutes' type='number' min='1' max='1440' placeholder='60'></div>\n"
  "</div>\n"
  "<label>Days of week</label>\n"
  "<div class='toggle' id='daysToggle'></div>\n"
  "<div><label>Reminder animation</label><select id='focusReminder_animation'><option value='0'>Use quarter animation</option><option value='1'>Use half-hour animation</option><option value='2'>Use hour animation</option><option value='6'>Gentle pulse</option><option value='7'>Orbiting orb</option><option value='8'>Ripple in</option><option value='9'>Heartbeat</option><option value='10'>Slow bloom</option><option value='11'>Firefly</option></select><button onclick=\"previewAnim('reminder','focusReminder_animation')\">&#9654;</button></div>\n"
  "<div class='row'><button class='primary' onclick='saveFocusReminder()'>Save reminder</button><span class='sub' id='remStatus' style='align-self:center;margin:0'></span></div>\n"
  "<div class='subhead'>Second / minute effects</div>\n"
  "<div class='toggle'><label><input id='secondTrail' type='checkbox'>Second trail</label><label><input id='progressSeconds' type='checkbox'>Progress ring</label><label><input id='hourlyChime' type='checkbox'>Hour-top center bloom</label><label><input id='statusAnimations' type='checkbox'>Center status blips</label></div>\n"
  "<div class='grid2' style='margin-top:6px'>\n"
  "  <div><label>Second-hand trail length</label><input id='secondTrailLength' type='range' min='2' max='12'><output id='secondTrailLengthOut'></output></div>\n"
  "  <div><label>Trail fade</label><select id='secondTrailStyle'><option value='0'>Classic</option><option value='1'>Linear</option><option value='2'>Smooth comet</option></select></div>\n"
  "  <div><label>Progress intensity</label><input id='progressLevel' type='range' min='0' max='255'><output id='progressLevelOut'></output></div>\n"
  "  <div><label>Progress style</label><select id='progressStyle'><option value='0'>Uniform arc</option><option value='1'>Comet gradient</option></select></div>\n"
  "</div>\n"
  "<p class='sub' style='font-size:11px;color:#92a0b5;margin:4px 0 0'>Trail &amp; progress blend over the ring colors, not to black.</p>\n"
  "</div>\n"
  "</details>\n"
  "<details class='drawer'>\n"
  "<summary><span class='ic'>&#127912;</span><span class='nm'>Colors &amp; rings</span><span class='hint'>set once &middot; 9 colors &middot; contrast</span></summary>\n"
  "<div class='drawerbody'>\n"
  "<div class='subhead'>Contrast preset</div>\n"
  "<select id='contrastPreset' onchange='applyContrastPreset(this.value)' style='width:100%'><option value=''>Custom</option><option value='current'>Defaults</option><option value='natural'>Natural Contrast</option><option value='subtle'>Subtle Bloom</option><option value='bloom'>Deep Bloom</option><option value='crisp'>Crisp</option><option value='vivid'>Vivid</option><option value='solstice'>Solstice</option><option value='deepspace'>Deep Space</option><option value='lava'>Lava</option><option value='frostbite'>Frostbite</option><option value='neongarden'>Neon Garden</option></select>\n"
  "<div class='subhead'>Ring colors</div>\n"
  "<div class='ringrow'><span>Outer marks</span><input id='outerMarkerColor' type='color'><input id='outerMarkerLevel' type='range' min='0' max='255'><output id='outerMarkerLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Outer fill</span><input id='outerFillerColor' type='color'><input id='outerFillerLevel' type='range' min='0' max='255'><output id='outerFillerLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Second hand</span><input id='secondsColor' type='color'><input id='secondsLevel' type='range' min='0' max='255'><output id='secondsLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Minute hand</span><input id='minutesColor' type='color'><input id='minutesLevel' type='range' min='0' max='255'><output id='minutesLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Middle face</span><input id='middleFaceColor' type='color'><input id='middleFaceScale' type='range' min='0' max='255'><output id='middleFaceScaleOut'></output></div>\n"
  "<div class='ringrow'><span>Middle hour</span><input id='hoursColor' type='color'><input id='hoursLevel' type='range' min='0' max='255'><output id='hoursLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Inner face</span><input id='innerFaceColor' type='color'><input id='innerFaceScale' type='range' min='0' max='255'><output id='innerFaceScaleOut'></output></div>\n"
  "<div class='ringrow'><span>Inner hour</span><input id='innerHourColor' type='color'><input id='innerHourLevel' type='range' min='0' max='255'><output id='innerHourLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Center</span><input id='centerColor' type='color'><input id='centerLevel' type='range' min='0' max='255'><output id='centerLevelOut'></output></div>\n"
  "<div class='ringrow'><span title='Shades the fills between petals so each ring reads as a row of petals rather than one flat glow. 0 = flat fill. Hour marks are never shaded.'>Petal depth</span><input id='petalDepth' type='range' min='0' max='100' style='grid-column:2/4'><output id='petalDepthOut'></output></div>\n"
  "<div class='subhead'>Ring setup</div>\n"
  "<div class='row'><div><label>Outer ring brightness (%)</label><input id='outerRingBrightness' type='number' min='0' max='100'></div>\n"
  "<div><label>Center LED shows</label><select id='centerSource'><option value='0'>Wi-Fi/status + bloom pulse</option><option value='1'>Bloom pulse only</option><option value='2'>Wi-Fi/status only</option><option value='3'>Temperature (sensor pending)</option><option value='4'>Off</option></select></div>\n"
  "<div><label>Ring rotation offset (use if 12 o'clock isn't at the top; 0-59 LEDs)</label><input id='outerRingOffset' type='number' min='0' max='59'></div></div>\n"
  "</div>\n"
  "</details>\n"
  "<details class='drawer' id='timeDrawer'>\n"
  "<summary><span class='ic'>&#128336;</span><span class='nm'>Time &amp; light</span><span class='hint'>set clock &middot; night &middot; sleep</span></summary>\n"
  "<div class='drawerbody'>\n"
  "<div class='subhead'>Set the clock time</div>\n"
  "<form onsubmit='setTime();return false;' class='row'>\n"
  "<div><label>Hour (0-23)</label><input id='h' type='number' min='0' max='23' placeholder='HH'></div>\n"
  "<div><label>Minute</label><input id='m' type='number' min='0' max='59' placeholder='MM'></div>\n"
  "<div><label>Second</label><input id='s' type='number' min='0' max='59' placeholder='SS'></div>\n"
  "<button class='primary' type='submit'>Set time</button></form>\n"
  "<div class='row'><button onclick='post(\"/addMinute\").catch(()=>{})'>+1 min</button><button onclick='post(\"/subMinute\").catch(()=>{})'>-1 min</button><button onclick='syncBrowser()'>Sync to browser</button><button onclick='post(\"/syncNtp\").catch(()=>{})'>Sync to internet</button></div>\n"
  "<div class='subhead'>Time zone</div>\n"
  "<div class='row'><div style='flex:2'><label title='Sets the zone the clock displays. Internet time is kept in UTC, so changing this takes effect immediately without a reboot.'>Zone</label>"
  "<select id='tzPick' onchange='tzPicked()'>"
  "<option value='PST8PDT,M3.2.0,M11.1.0'>US Pacific</option>"
  "<option value='MST7MDT,M3.2.0,M11.1.0'>US Mountain</option>"
  "<option value='MST7'>US Arizona (no DST)</option>"
  "<option value='CST6CDT,M3.2.0,M11.1.0'>US Central</option>"
  "<option value='EST5EDT,M3.2.0,M11.1.0'>US Eastern</option>"
  "<option value='AST4ADT,M3.2.0,M11.1.0'>Atlantic</option>"
  "<option value='GMT0BST,M3.5.0/1,M10.5.0'>UK</option>"
  "<option value='CET-1CEST,M3.5.0,M10.5.0/3'>Central Europe</option>"
  "<option value='EET-2EEST,M3.5.0/3,M10.5.0/4'>Eastern Europe</option>"
  "<option value='MSK-3'>Moscow</option>"
  "<option value='IST-5:30'>India</option>"
  "<option value='CST-8'>China</option>"
  "<option value='JST-9'>Japan</option>"
  "<option value='AEST-10AEDT,M10.1.0,M4.1.0/3'>Australia Eastern</option>"
  "<option value='AEST-10'>Australia Brisbane (no DST)</option>"
  "<option value='AWST-8'>Australia Western</option>"
  "<option value='NZST-12NZDT,M9.5.0,M4.1.0/3'>New Zealand</option>"
  "<option value='BRT3'>Brazil</option>"
  "<option value='UTC0'>UTC</option>"
  "<option value='__custom'>Other (enter POSIX TZ)</option>"
  "</select></div>"
  "<div style='flex:3;display:none' id='tzCustomWrap'><label>POSIX TZ string</label><input id='timezone' type='text' spellcheck='false' placeholder='MST7MDT,M3.2.0,M11.1.0'></div>"
  "<button class='primary' onclick='saveTz()'>Save zone</button></div>\n"
  "<div class='sub' id='tzMsg'>Pick a zone, or choose Other and paste a POSIX TZ string. Takes effect immediately, no reboot.</div>\n"
  "<div class='subhead'>Night dimming (Scheduled mode)</div>\n"
  "<div class='row'><div><label>Night brightness</label><input id='nightBrightness' type='number' min='0' max='255'></div><div><label>Night starts (hour)</label><input id='nightStartHour' type='number' min='0' max='23'></div><div><label>Night ends (hour)</label><input id='nightEndHour' type='number' min='0' max='23'></div></div>\n"
  "<div class='subhead'>Sleep &amp; web preview</div>\n"
  "<div class='toggle'><label><input id='darkRoomOff' type='checkbox'> Lights out in pitch black (&lt;0.3 lux for 30s; wakes above 2 lux)</label></div>\n"
  "<label>Web preview effect</label><select id='previewMode'><option value='live'>Live clock</option><option value='trail'>All-ring trails</option><option value='spark'>Hourly sparkle</option></select>\n"
  "</div>\n"
  "</details>\n"
  "<details class='drawer'>\n"
  "<summary><span class='ic'>&#9881;</span><span class='nm'>System</span><span class='hint'>firmware &middot; Wi-Fi &middot; demo &middot; backup</span></summary>\n"
  "<div class='drawerbody'>\n"
  "<div class='row' style='gap:6px'>"
  "<a href='/update' title='Open the firmware-update page to flash a new .bin over WiFi (OTA).' style='display:inline-flex;align-items:center;padding:6px 10px;background:#203146;border:1px solid #42546d;color:var(--text);border-radius:6px;text-decoration:none;font-size:13px;min-height:30px'>Firmware</a>"
  "<a href='/wifi' title='Change which WiFi network the clock joins (SSID + password).' style='display:inline-flex;align-items:center;padding:6px 10px;background:#203146;border:1px solid #42546d;color:var(--text);border-radius:6px;text-decoration:none;font-size:13px;min-height:30px'>WiFi</a>"
  "<label for='demoDelay' style='font-size:12px;color:#92a0b5;align-self:center'>Pre-roll</label><input id='demoDelay' type='number' min='0' max='60' value='5' title='Seconds to hold the clock dark before the reel opens, so you can start recording first. 0 starts immediately.' style='width:56px;padding:6px;background:#203146;border:1px solid #42546d;color:var(--text);border-radius:6px;font-size:13px;min-height:30px'>"
  "<span style='font-size:12px;color:#92a0b5;align-self:center'>s</span>"
  "<button class='primary' onclick='startDemo()' id='demoStartBtn' title='Play the automated ~130s feature showcase (for video recording).'>&#9654; Demo</button>"
  "<button onclick='stopDemo()' id='demoStopBtn' title='Stop the running demo and return to the clock.' style='display:none;background:#8b3a3a;border-color:#c75555'>&#9209; Stop</button>"
  "</div>\n"
  "<div class='row' style='gap:12px;margin-top:6px;font-size:12px;color:#92a0b5'><span id='demoStatus'>Idle</span></div>\n"
  "<div class='subhead'>Backup &amp; themes</div>\n"
  "<div class='row' style='gap:6px'>"
  "<button onclick='exportTheme()' title='Save your ring colors + animation style to a small .json file.'>&#8681; Theme</button>"
  "<label title='Load a theme .json (ring colors + animation style only).' style='display:inline-flex;align-items:center;gap:6px;margin:0;cursor:pointer;background:#203146;border:1px solid #42546d;border-radius:6px;padding:6px 10px;color:var(--text);font-size:13px'>&#8679; Load theme<input type='file' id='themeImportFile' accept='.json' style='display:none' onchange='importTheme(this.files[0])'></label>"
  "<button onclick='exportBackup()' title='Save EVERY setting on this clock to a .json file (full backup).'>&#8681; Full backup</button>"
  "<label title='Restore every setting from a full-backup .json.' style='display:inline-flex;align-items:center;gap:6px;margin:0;cursor:pointer;background:#203146;border:1px solid #42546d;border-radius:6px;padding:6px 10px;color:var(--text);font-size:13px'>&#8679; Restore<input type='file' id='backupImportFile' accept='.json' style='display:none' onchange='importBackup(this.files[0])'></label>"
  "</div>\n"
  "<div class='row' style='gap:12px;margin-top:6px;font-size:12px;color:#92a0b5'><span id='themeStatus'></span><span id='backupStatus'></span></div>\n"
  "</div>\n"
  "</details>\n"
  "</div>\n"
  "</main>\n"
  "<div id='toast' style='display:none;position:fixed;bottom:16px;left:50%;transform:translateX(-50%);background:#145875;border:1px solid #2d9ccb;color:#fff;padding:8px 16px;border-radius:8px;font-size:13px;z-index:9;max-width:90vw'></div>\n";

static const char INDEX_P3[] PROGMEM =
  "<script>\n"
  "const counts={outer:60,middle:24,inner:12}, radii={outer:182,middle:134,inner:88};\n"
  "const leds={outer:[],middle:[],inner:[]}; let settings={}, current={hour:12,minute:0,second:0}, netTimer=0;\n"
  "function qs(id){return document.getElementById(id)} function pad(n){return String(n).padStart(2,'0')}\n"
  "function makeClock(){const svg=qs('clockSvg'); for(const ring of ['outer','middle','inner']){for(let i=0;i<counts[ring];i++){const a=(i/counts[ring])*Math.PI*2-Math.PI/2,x=210+Math.cos(a)*radii[ring],y=210+Math.sin(a)*radii[ring],c=document.createElementNS('http://www.w3.org/2000/svg','circle');c.setAttribute('cx',x);c.setAttribute('cy',y);c.setAttribute('r',ring==='outer'?4.4:ring==='middle'?5.8:7.2);c.classList.add('led');svg.appendChild(c);leds[ring].push(c)}} const center=document.createElementNS('http://www.w3.org/2000/svg','circle');center.id='centerLed';center.setAttribute('cx',210);center.setAttribute('cy',210);center.setAttribute('r',17);center.classList.add('led','center');svg.appendChild(center)}\n"
  "function ledDisp(c,lvl){if(!c||c[0]!=='#'||c.length<7)return c;const q=s=>{const v=parseInt(s,16);const g=Math.min(255,Math.floor(Math.pow(v/255,2.2)*255+0.5));const e=Math.floor(g*lvl/255);return e>0?Math.round(255*Math.pow(e/255,1/2.2)):0};const h=n=>n.toString(16).padStart(2,'0');return '#'+h(q(c.substr(1,2)))+h(q(c.substr(3,2)))+h(q(c.substr(5,2)))}\n"
  "function setLed(el,color,level,cls='on'){const d=ledDisp(color,level);el.style.color=d;el.setAttribute('fill',d);el.style.opacity=1;el.className.baseVal='led '+cls}\n"
  "function clearRing(r){for(const el of leds[r]){el.setAttribute('fill','#243044');el.style.opacity=.22;el.className.baseVal='led'}}\n"
  "function level(id){return Number(qs(id+'Level')?.value||180)} function color(id){return qs(id+'Color')?.value||'#ffffff'}\n"
  "function gam(x){return Math.pow(Math.max(0,x)/255,2.2)*255}\n"
  "function trailAlphaJS(i,tl,st){const P=150;if(i>=tl)return 0;if(st===0)return P>>i;if(st===1)return P*(tl-i)/tl;return P*gam(255*(tl-i)/(tl+1))/255}\n"
  "const MOOD_SWATCH=['#FF9838','#A99EDC','#F08C78','#6074A8'];\n"
  "function paletteSwatch(v){return v===7?`linear-gradient(90deg,${color('outerMarker')},${color('hours')},${color('center')})`:(MOOD_SWATCH[v]||'#888')}\n"
  "function validPalette(v){v=Number(v);return (v===0||v===1||v===2||v===3||v===7)?v:7}\n"
  "function updatePaletteSwatches(){const asw=qs('animationPaletteSwatch');if(asw)asw.style.background=paletteSwatch(Number(qs('animationPalette')?.value));const rsw=qs('reminderPaletteSwatch');if(rsw)rsw.style.background=paletteSwatch(Number(qs('reminderPalette')?.value))}\n"
  "const PETAL_S5=[0,140,255,255,140],PETAL_S3=[255,0,255];\n"
  "function petalTintJS(c,sf){if(sf>=255||!c||c[0]!=='#')return c;const h=n=>n.toString(16).padStart(2,'0');const q=s=>Math.floor(parseInt(s,16)*sf/255);return '#'+h(q(c.substr(1,2)))+h(q(c.substr(3,2)))+h(q(c.substr(5,2)))}\n"
  "function petalFactorJS(shade,depth){if(!depth||!shade)return 255;const cut=Math.floor(170*shade/255);return 255-Math.floor(cut*depth/100)}\n"
  "function draw(){clearRing('middle');clearRing('inner');const orbS=Math.floor(Number(qs('outerRingBrightness')?.value??90)*255/100);const pd=Number(qs('petalDepth')?.value??0);const pD=petalFactorJS(255,pd);const pLift=l=>Math.min(255,Math.floor(l*255/pD));for(let i=0;i<60;i++){const mark=i%5===0;const lv=Math.floor((mark?level('outerMarker'):level('outerFiller'))*orbS/255);setLed(leds.outer[i],mark?color('outerMarker'):petalTintJS(color('outerFiller'),petalFactorJS(PETAL_S5[i%5],pd)),mark?lv:pLift(lv),mark?'marker':'ghost')}let s=current.second,m=current.minute,h=current.hour%12,hoff=current.minute>=30?1:0,h24=(h*2+hoff)%24;const mode=qs('previewMode')?.value||'live',tick=Math.floor(Date.now()/90);if(settings.progressSeconds){const pl=Number(qs('progressLevel')?.value)||90;const pst=Number(qs('progressStyle')?.value)||0;for(let i=0;i<=s;i++){let a=pl;if(pst===1&&s>0)a=pl*(i+1)/(s+1);setLed(leds.outer[i],color('seconds'),Math.max(6,a),'ghost')}}if(settings.secondTrail||mode==='trail'){const tl=Number(qs('secondTrailLength')?.value)||4;const tst=Number(qs('secondTrailStyle')?.value)||0;for(let i=0;i<tl;i++)setLed(leds.outer[(s+60-i-1)%60],color('seconds'),Math.max(8,trailAlphaJS(i,tl,tst)),'ghost')}if(mode==='trail'){for(let i=1;i<5;i++){setLed(leds.outer[(m+60-i)%60],color('minutes'),Math.max(25,level('minutes')-(i*42)),'ghost');setLed(leds.middle[(h24+24-i)%24],color('hours'),Math.max(25,level('hours')-(i*42)),'ghost');setLed(leds.inner[(h+12-i)%12],color('innerHour'),Math.max(25,level('innerHour')-(i*52)),'ghost')}}if(mode==='spark'){for(let i=0;i<10;i++){setLed(leds.outer[(tick+i*6)%60],i%2?color('innerHour'):color('minutes'),90+(i*10),'ghost')}}const mfs=Number(qs('middleFaceScale')?.value??50);const ifs=Number(qs('innerFaceScale')?.value??50);for(let i=0;i<24;i++)setLed(leds.middle[i],petalTintJS(color('middleFace'),petalFactorJS(PETAL_S3[i%3],pd)),pLift(mfs),'marker');for(let i=0;i<12;i++)setLed(leds.inner[i],petalTintJS(color('innerFace'),petalFactorJS(PETAL_S3[(i+1)%3],pd)),pLift(ifs),'marker');setLed(leds.outer[s],color('seconds'),level('seconds'));setLed(leds.outer[m],color('minutes'),level('minutes'));setLed(leds.middle[h24],color('hours'),level('hours'));setLed(leds.inner[h],color('innerHour'),level('innerHour'));setLed(leds.inner[(h+hoff)%12],color('innerHour'),level('innerHour'));const pulse=45+Math.floor((Math.sin(Date.now()/450)+1)*85);setLed(qs('centerLed'),color('center'),Math.min(level('center'),pulse),'on');updatePaletteSwatches()}\n"
  "async function refresh(){const r=await fetch('/time');const t=await r.json();current=t;const h12=t.hour%12||12;const ampm=t.hour<12?'AM':'PM';qs('now').textContent=`${pad(h12)}:${pad(t.minute)}:${pad(t.second)} ${ampm}`;qs('state').textContent=`IP ${t.ip||'-'} | Wi-Fi ${t.wifi?'on':'off'} | NTP ${t.ntpSynced?'synced':'waiting'}`;draw()}\n"
  "async function loadNet(){const r=await fetch('/net');const n=await r.json();qs('net').textContent=`${n.hostname} | ${n.ssid} | IP ${n.ip} | gateway ${n.gateway} | signal ${n.rssi} dBm`}\n"
  "async function loadSettings(){const r=await fetch('/settings');settings=await r.json();for(const k of ['dayBrightness','nightBrightness','nightStartHour','nightEndHour','centerSource','outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','hoursLevel','middleFaceScale','innerFaceScale','innerHourLevel','centerLevel'])qs(k).value=settings[k];for(const k of ['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','hoursColor','middleFaceColor','innerFaceColor','innerHourColor','centerColor'])qs(k).value=settings[k];for(const k of ['secondTrail','progressSeconds','hourlyChime','statusAnimations'])qs(k).checked=!!settings[k];qs('autoBrightnessMode').value=settings.autoBrightnessMode;qs('minAutoBrightness').value=settings.minAutoBrightness;qs('maxAutoBrightness').value=settings.maxAutoBrightness;qs('darkRoomOff').checked=!!settings.darkRoomOff;qs('petalDepth').value=settings.petalDepth??0;qs('quarterAnimation').value=settings.quarterAnimation;qs('halfHourAnimation').value=settings.halfHourAnimation;qs('hourAnimation').value=settings.hourAnimation;qs('intervalAnimationsEnabled').checked=!!settings.intervalAnimationsEnabled;qs('focusReminder_enabled').checked=!!settings.focusReminder_enabled;qs('focusReminder_startHour').value=settings.focusReminder_startHour||8;qs('focusReminder_endHour').value=settings.focusReminder_endHour||22;qs('focusReminder_intervalMinutes').value=settings.focusReminder_intervalMinutes||60;qs('focusReminder_animation').value=(m=>m>=3&&m<=5?m-3:m)(settings.focusReminder_animation||0);const daysToggle=qs('daysToggle');daysToggle.innerHTML='';const daysNames=['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];for(let i=0;i<7;i++){const label=document.createElement('label');const checkbox=document.createElement('input');checkbox.type='checkbox';checkbox.checked=!!(settings.focusReminder_daysMask&(1<<i));checkbox.id='focusReminder_day'+i;label.appendChild(checkbox);label.appendChild(document.createTextNode(daysNames[i]));daysToggle.appendChild(label)}qs('outerRingOffset').value=settings.outerRingOffset||0;qs('animationPalette').value=validPalette(settings.animationPalette??7);qs('animationSpeed').value=settings.animationSpeed??3;qs('animationBrightness').value=settings.animationBrightness??200;qs('trailLength').value=settings.trailLength??4;qs('reminderPalette').value=validPalette(settings.reminderPalette??0);qs('timezone').value=settings.timezone||'';tzSync();qs('outerRingBrightness').value=settings.outerRingBrightness??90;qs('secondTrailLength').value=settings.secondTrailLength??4;qs('secondTrailStyle').value=settings.secondTrailStyle??2;qs('progressLevel').value=settings.progressLevel??90;qs('progressStyle').value=settings.progressStyle??0;qs('quickRemEnabled').checked=!!settings.focusReminder_enabled;qs('quickRemInterval').value=settings.focusReminder_intervalMinutes||30;settings._loaded=true;updateContrastPreset();updateBrightnessMode();bindLive();draw();refreshLux();setInterval(refreshLux,2000);updateDefaultLabel();updateThemePreset();tzBannerCheck()}\n"
  "async function refreshLux(){const r=await fetch('/lux');const data=await r.json();if(data.available){qs('luxValue').textContent=data.lux.toFixed(1)}else{const o=qs('autoBrightnessMode').querySelector(\"option[value='1']\");if(o&&!o.disabled){o.disabled=true;o.textContent='Auto (no sensor)';if(Number(qs('autoBrightnessMode').value)===1){qs('autoBrightnessMode').value='0';updateBrightnessMode()}}}}\n"
  "function updateBrightnessMode(){const m=Number(qs('autoBrightnessMode').value);qs('brightManual').style.display=m===1?'none':'flex';qs('brightAuto').style.display=m===1?'block':'none'}\n"
  "async function loadVersion(){try{const r=await fetch('/diag');const d=await r.json();qs('fwVersion').textContent='v'+d.firmware_version}catch(e){}}\n"
  "function bindLive(){for(const k of ['outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel']){const out=qs(k+'Out');const upd=()=>{out.value=qs(k).value;if(['middleFaceScale','innerFaceScale','hoursLevel','innerHourLevel'].includes(k))qs('contrastPreset').value='';draw()};qs(k).oninput=upd;upd()}for(const k of ['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','centerColor','previewMode'])qs(k).oninput=draw;for(const k of ['middleFaceColor','hoursColor','innerFaceColor','innerHourColor'])qs(k).oninput=()=>{qs('contrastPreset').value='';draw()};for(const k of ['secondTrail','progressSeconds'])qs(k).oninput=()=>{settings[k]=qs(k).checked;draw()};for(const k of ['animationBrightness','trailLength']){const out=qs(k+'Out');const upd=()=>{out.value=qs(k).value};qs(k).oninput=upd;upd()}for(const k of ['secondTrailLength','progressLevel','petalDepth']){const out=qs(k+'Out');const upd=()=>{out.value=qs(k).value;draw()};qs(k).oninput=upd;upd()}for(const k of ['secondTrailStyle','progressStyle'])qs(k).oninput=draw;qs('outerRingBrightness').oninput=()=>{qs('contrastPreset').value='';draw()};for(const k of ['dayBrightness','minAutoBrightness','maxAutoBrightness']){const o=qs(k+'Out');if(o){const u=()=>{o.value=qs(k).value};qs(k).oninput=u;u()}}}\n"
  "let toastT=0;function toast(msg,err){const t=qs('toast');if(!t)return;t.textContent=msg;t.style.background=err?'#8b2e2e':'#145875';t.style.borderColor=err?'#c04040':'#2d9ccb';t.style.display='block';clearTimeout(toastT);toastT=setTimeout(()=>t.style.display='none',2600)}\n"
  // Distinguish "clock rejected the input" (surface its message) from "clock
  // unreachable" (generic toast). e.server marks errors we raised from a
  // received response.
  "async function post(url,body){try{const r=await fetch(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});if(!r.ok){const err=new Error(await r.text());err.server=true;throw err}await refresh()}catch(e){toast(e&&e.server&&e.message?e.message:'Could not reach the clock',true);throw e}}\n"
  "function setTime(){post('/set',`hour=${h.value}&minute=${m.value}&second=${s.value}`).catch(()=>{})}\n"
  "function tzPicked(){const v=qs('tzPick').value;const w=qs('tzCustomWrap');if(v!=='__custom'){qs('timezone').value=v;w.style.display='none'}else{w.style.display='';qs('timezone').focus()}}\n"
  "function tzSync(){const v=qs('timezone').value.trim();const opt=[...qs('tzPick').options].find(o=>o.value===v);qs('tzPick').value=opt?v:'__custom';qs('tzCustomWrap').style.display=opt?'none':''}\n"
  "const TZ_GUESS={'America/Los_Angeles':'PST8PDT,M3.2.0,M11.1.0','America/Vancouver':'PST8PDT,M3.2.0,M11.1.0','America/Denver':'MST7MDT,M3.2.0,M11.1.0','America/Edmonton':'MST7MDT,M3.2.0,M11.1.0','America/Phoenix':'MST7','America/Chicago':'CST6CDT,M3.2.0,M11.1.0','America/Winnipeg':'CST6CDT,M3.2.0,M11.1.0','America/New_York':'EST5EDT,M3.2.0,M11.1.0','America/Toronto':'EST5EDT,M3.2.0,M11.1.0','America/Halifax':'AST4ADT,M3.2.0,M11.1.0','Europe/London':'GMT0BST,M3.5.0/1,M10.5.0','Europe/Dublin':'GMT0BST,M3.5.0/1,M10.5.0','Europe/Paris':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Berlin':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Madrid':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Rome':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Amsterdam':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Stockholm':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Warsaw':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Helsinki':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Athens':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Kyiv':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Moscow':'MSK-3','Asia/Kolkata':'IST-5:30','Asia/Shanghai':'CST-8','Asia/Hong_Kong':'CST-8','Asia/Singapore':'CST-8','Asia/Tokyo':'JST-9','Asia/Seoul':'JST-9','Australia/Sydney':'AEST-10AEDT,M10.1.0,M4.1.0/3','Australia/Melbourne':'AEST-10AEDT,M10.1.0,M4.1.0/3','Australia/Brisbane':'AEST-10','Australia/Perth':'AWST-8','Pacific/Auckland':'NZST-12NZDT,M9.5.0,M4.1.0/3','America/Sao_Paulo':'BRT3','UTC':'UTC0'};\n"
  "let tzGuessVal='';\n"
  "function tzBannerCheck(){try{if(settings.tzConfigured||localStorage.getItem('cb_tzBannerDone'))return;const cz=settings.timezone||'its factory zone';const iana=Intl.DateTimeFormat().resolvedOptions().timeZone||'';tzGuessVal=TZ_GUESS[iana]||'';if(tzGuessVal&&tzGuessVal!==cz){qs('tzBannerTxt').textContent='The clock is still on its compiled-in zone ('+cz+'). This device says '+iana+'.';qs('tzBannerBtn').style.display=''}else if(tzGuessVal){qs('tzBannerTxt').textContent='The clock ships on '+cz+', which matches this device. One tap makes it stick.';qs('tzBannerBtn').style.display=''}else{qs('tzBannerTxt').textContent='The clock is still on its compiled-in zone ('+cz+'). Pick yours in Time & light.';qs('tzBannerBtn').style.display='none'}qs('tzBanner').style.display='block'}catch(e){}}\n"
  // Only dismiss the banner when the save actually landed; a rejected save
  // (e.g. settings locked during OTA) keeps the banner and says why.
  "async function tzBannerApply(){if(!tzGuessVal){tzBannerOpen();return}qs('timezone').value=tzGuessVal;let ok=false;try{ok=await saveTz()}catch(e){}if(!ok){toast('Could not save the zone - try again in a minute',true)}}\n"
  "function tzBannerOpen(){qs('tzBanner').style.display='none';try{localStorage.setItem('cb_tzBannerDone','1')}catch(e){}const d=qs('timeDrawer');if(d)d.open=true;qs('tzPick').scrollIntoView({behavior:'smooth',block:'center'})}\n"
  // Save posts ONLY the timezone: POST /settings applies each field it is given
  // (hasArg guards), so a lone timezone cannot clobber unsaved edits elsewhere
  // on the page. It is also the one field the firmware can reject, so surface
  // the 400 instead of pretending the save worked.
  "async function saveTz(){const v=qs('timezone').value.trim();const msg=qs('tzMsg');let r;"
  "try{r=await fetch('/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'timezone='+encodeURIComponent(v)+'&silent=1'})}catch(e){msg.textContent='Could not reach the clock.';toast('Could not reach the clock',true);return false}"
  "if(r.ok){msg.textContent='Saved. Clock is now on '+v;tzSync();settings.tzConfigured=true;const b=qs('tzBanner');if(b)b.style.display='none';try{localStorage.setItem('cb_tzBannerDone','1')}catch(e){}toast('Time zone saved');await refresh();return true}"
  "else{msg.textContent=await r.text();return false}}\n"
  "function syncBrowser(){const d=new Date();post('/syncBrowser',`hour=${d.getHours()}&minute=${d.getMinutes()}&second=${d.getSeconds()}`).catch(()=>{})}\n"
  "function collectFaceParams(){const p=new URLSearchParams();for(const k of ['dayBrightness','nightBrightness','nightStartHour','nightEndHour','centerSource','outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel','outerRingBrightness','secondTrailLength','secondTrailStyle','progressLevel','progressStyle'])p.set(k,qs(k).value);for(const k of ['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','middleFaceColor','hoursColor','innerFaceColor','innerHourColor','centerColor'])p.set(k,qs(k).value);for(const k of ['secondTrail','progressSeconds','hourlyChime','statusAnimations'])p.set(k,qs(k).checked?1:0);p.set('autoBrightnessMode',qs('autoBrightnessMode').value);p.set('minAutoBrightness',qs('minAutoBrightness').value);p.set('maxAutoBrightness',qs('maxAutoBrightness').value);p.set('darkRoomOff',qs('darkRoomOff').checked?1:0);p.set('petalDepth',qs('petalDepth').value);p.set('quarterAnimation',qs('quarterAnimation').value);p.set('halfHourAnimation',qs('halfHourAnimation').value);p.set('hourAnimation',qs('hourAnimation').value);p.set('intervalAnimationsEnabled',qs('intervalAnimationsEnabled').checked?1:0);p.set('outerRingOffset',qs('outerRingOffset').value);return p}\n"
  // Save collects EVERYTHING on the page (v2.31.0). It used to post only the
  // face params, then loadSettings() re-read device state and visibly reverted
  // unsaved reminder/style edits: the most prominent button silently
  // discarding work. The sectional buttons remain as conveniences.
  "function collectAllParams(){const p=collectFaceParams();for(const k of ['animationPalette','animationSpeed','animationBrightness','trailLength','reminderPalette'])p.set(k,qs(k).value);p.set('focusReminder_enabled',qs('focusReminder_enabled').checked?1:0);for(const k of ['focusReminder_startHour','focusReminder_endHour','focusReminder_intervalMinutes','focusReminder_animation'])p.set(k,qs(k).value);const d0=qs('focusReminder_day0');if(d0){let dm=0;for(let i=0;i<7;i++){const d=qs('focusReminder_day'+i);if(d&&d.checked)dm|=(1<<i)}"
  "if(qs('focusReminder_enabled').checked&&dm===0){for(let i=0;i<7;i++)qs('focusReminder_day'+i).checked=true;dm=127;const st=qs('remStatus');if(st)st.textContent='No nudge days were picked, so all seven are on.';toast('Nudge days: none picked, so all seven are on')}"  // never rewrite the mask silently
  "p.set('focusReminder_daysMask',dm)}return p}\n"
  // Refuse to save from a page that never finished loading: posting browser
  // defaults would overwrite the deployed clock's entire config.
  "function saveSettings(){if(!settings._loaded){toast('Settings never loaded - reload the page before saving',true);return}post('/settings',collectAllParams().toString()).then(loadSettings).then(()=>toast('Saved')).catch(()=>{})}\n"
  "function previewOnClock(){const p=collectFaceParams();p.set('ttl','10000');fetch('/settings/preview',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()})}\n"
  "function saveFocusReminder(){const p=new URLSearchParams();const en=qs('focusReminder_enabled').checked;p.set('focusReminder_enabled',en?1:0);p.set('focusReminder_startHour',qs('focusReminder_startHour').value);p.set('focusReminder_endHour',qs('focusReminder_endHour').value);p.set('focusReminder_intervalMinutes',qs('focusReminder_intervalMinutes').value);p.set('focusReminder_animation',qs('focusReminder_animation').value);let daysMask=0;for(let i=0;i<7;i++){if(qs('focusReminder_day'+i).checked)daysMask|=(1<<i)}"
  "if(en&&daysMask===0){for(let i=0;i<7;i++)qs('focusReminder_day'+i).checked=true;daysMask=127;const st=qs('remStatus');if(st)st.textContent='No days were picked, so all seven are on.'}"  // a saved-but-never-fires reminder was the audit's top story-killer
  "p.set('focusReminder_daysMask',daysMask);post('/settings',p.toString()).then(loadSettings).then(()=>toast(en?'Reminder saved':'Reminders off')).catch(()=>{})}\n"
  "function remPreset(iv){qs('focusReminder_enabled').checked=true;qs('focusReminder_intervalMinutes').value=iv;saveFocusReminder()}\n"
  // Fresh /settings read before deciding on the daysMask rescue - the cached
  // settings object can be stale when another tab/device saved in between.
  // The confirmation names the active hour window so an evening enable outside
  // it doesn't read as "nudges broken" while they silently wait for morning.
  "async function quickRemSave(){const en=qs('quickRemEnabled').checked;const iv=Math.min(1440,Math.max(1,Number(qs('quickRemInterval').value)||30));try{const cur=await(await fetch('/settings')).json();const p=new URLSearchParams();p.set('focusReminder_enabled',en?1:0);p.set('focusReminder_intervalMinutes',iv);if(en&&!(Number(cur.focusReminder_daysMask)&127))p.set('focusReminder_daysMask',127);p.set('silent','1');const r=await fetch('/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()});if(!r.ok)throw 0;const sh=cur.focusReminder_startHour,eh=cur.focusReminder_endHour;const win=(sh===eh)?'all day':(sh+':00-'+eh+':00');qs('quickRemMsg').textContent=en?('Nudging every '+iv+' min, '+win+'. Press either clock button during a nudge to say \\u201cseen it\\u201d. Hours are in Motion & nudges.'):'Nudges off.';toast(en?'Nudges on, every '+iv+' min ('+win+')':'Nudges off');await loadSettings()}catch(e){toast('Could not reach the clock',true)}}\n"
  "function updateDefaultLabel(){const hasUD=settings&&settings.hasUserDefaults;const btn=qs('resetBtn');if(btn)btn.textContent=hasUD?'Reset to my defaults':'Reset to factory defaults'}\n"
  "async function saveAsDefault(){await fetch('/settings/saveDefault',{method:'POST'});const m=qs('defaultSavedMsg');m.style.display='inline';setTimeout(()=>m.style.display='none',3000);await loadSettings()}\n"
  "async function resetToDefaults(){const hasUD=settings&&settings.hasUserDefaults;if(!confirm(hasUD?'Reset ALL settings to your saved defaults?':'Reset ALL settings to factory defaults?'))return;await fetch('/settings/reset',{method:'POST'});loadSettings()}\n"
  "function styleParams(p){p.set('palette',qs('animationPalette').value);p.set('speed',qs('animationSpeed').value);p.set('brightness',qs('animationBrightness').value);p.set('trail',qs('trailLength').value);p.set('reminderPalette',qs('reminderPalette').value);return p}\n"
  // Reminder mode 0-2 are valid ("use quarter/half/hour animation"), so only the
  // interval selects use 0 as Off, so the 0-guard applies to those alone.
  "function previewAnim(type,modeId){const mode=qs(modeId).value;if(type!=='reminder'&&(!mode||mode==='0'))return;"
  "if(type==='reminder'&&Number(mode)<=2){const del=['quarterAnimation','halfHourAnimation','hourAnimation'][Number(mode)];if(qs(del).value==='0'){toast('That chime is set to Off, so this nudge has nothing to play - pick a chime style or a dedicated nudge animation');return}}"  // delegated modes silently no-op when the chime is Off
  "const p=styleParams(new URLSearchParams());p.set('type',type);p.set('mode',mode);fetch('/previewAnimation',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()}).catch(()=>{})}\n"
  "async function saveAnimStyle(){const p=new URLSearchParams();p.set('animationPalette',qs('animationPalette').value);p.set('animationSpeed',qs('animationSpeed').value);p.set('animationBrightness',qs('animationBrightness').value);p.set('trailLength',qs('trailLength').value);p.set('reminderPalette',qs('reminderPalette').value);p.set('silent','1');try{const r=await fetch('/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()});if(!r.ok)throw 0;toast('Style saved')}catch(e){toast('Could not reach the clock',true)}loadSettings()}\n"
  "function previewAnimationTheme(){const p=styleParams(new URLSearchParams());let t='hour',m=qs('hourAnimation').value;if(!m||m==='0'){t='halfhour';m=qs('halfHourAnimation').value}if(!m||m==='0'){t='quarter';m=qs('quarterAnimation').value}if(!m||m==='0'){t='hour';m='2'}p.set('type',t);p.set('mode',m);fetch('/previewAnimation',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()})}\n"
  "function saveBrightness(){const p=new URLSearchParams();for(const k of ['autoBrightnessMode','dayBrightness','minAutoBrightness','maxAutoBrightness'])p.set(k,qs(k).value);post('/settings',p.toString()).then(loadSettings).then(()=>toast('Brightness saved')).catch(()=>{})}\n"
  "async function startDemo(){const d=Math.min(60,Math.max(0,Number(qs('demoDelay')?.value)||0));try{localStorage.setItem('cb_demoDelay',d)}catch(e){}await fetch('/demo/start?delay='+d,{method:'POST'});qs('demoStartBtn').style.display='none';qs('demoStopBtn').style.display='inline-block';updateDemoStatus();demoStatusInterval=setInterval(updateDemoStatus,500)}\n"
  "async function stopDemo(){await fetch('/demo/stop',{method:'POST'});qs('demoStartBtn').style.display='inline-block';qs('demoStopBtn').style.display='none';qs('demoStatus').textContent='Idle';clearInterval(demoStatusInterval)}\n"
  "async function updateDemoStatus(){try{const r=await fetch('/demo/status');const data=await r.json();if(data.active&&data.preroll){qs('demoStatus').textContent=`Recording pre-roll: clock dark, reel opens in ${Math.ceil(data.preroll_ms/1000)}s`}else if(data.active){const elapsed=data.elapsed_ms,dur=data.step_duration_ms,pct=Math.round(100*elapsed/dur);qs('demoStatus').textContent=`Step ${data.step+1}/${data.steps||9}: ${data.subtitle} (${pct}%)`}else{qs('demoStatus').textContent='Idle'}}catch(e){}}\n"
  "let demoStatusInterval=0;\n"
  "try{const d=localStorage.getItem('cb_demoDelay');if(d!==null&&qs('demoDelay'))qs('demoDelay').value=d}catch(e){}\n"
  "const CONTRAST_PRESETS={current:{outerRingBrightness:90,middleFaceScale:55,innerFaceScale:55,middleFaceColor:'#DC00B4',hoursColor:'#DC00B4',innerFaceColor:'#FF3C00',innerHourColor:'#DC00B4',hoursLevel:255,innerHourLevel:255},natural:{outerRingBrightness:82,middleFaceScale:64,innerFaceScale:72,middleFaceColor:'#123A48',hoursColor:'#FFD166',innerFaceColor:'#382B50',innerHourColor:'#FFD166',hoursLevel:238,innerHourLevel:238},subtle:{outerRingBrightness:65,middleFaceScale:38,innerFaceScale:58,middleFaceColor:'#183242',hoursColor:'#E8C77B',innerFaceColor:'#352B4C',innerHourColor:'#E8C77B',hoursLevel:215,innerHourLevel:215},bloom:{outerRingBrightness:35,middleFaceScale:75,innerFaceScale:115,middleFaceColor:'#174454',hoursColor:'#FFD166',innerFaceColor:'#40305D',innerHourColor:'#FFD166',hoursLevel:235,innerHourLevel:235},crisp:{outerRingBrightness:80,middleFaceScale:10,innerFaceScale:15,middleFaceColor:'#0F2F3E',hoursColor:'#FFD166',innerFaceColor:'#2F2448',innerHourColor:'#FFD166',hoursLevel:255,innerHourLevel:255},vivid:{outerRingBrightness:95,middleFaceScale:88,innerFaceScale:110,middleFaceColor:'#006D77',hoursColor:'#FFE66D',innerFaceColor:'#573280',innerHourColor:'#FFE66D',hoursLevel:245,innerHourLevel:245},solstice:{outerRingBrightness:82,middleFaceScale:58,innerFaceScale:65,middleFaceColor:'#0A2B10',hoursColor:'#FFD44A',innerFaceColor:'#071A0A',innerHourColor:'#FFD44A',hoursLevel:240,innerHourLevel:240},deepspace:{outerRingBrightness:72,middleFaceScale:52,innerFaceScale:58,middleFaceColor:'#0A1840',hoursColor:'#2080FF',innerFaceColor:'#081030',innerHourColor:'#2080FF',hoursLevel:250,innerHourLevel:250},lava:{outerRingBrightness:80,middleFaceScale:60,innerFaceScale:68,middleFaceColor:'#200800',hoursColor:'#FF5800',innerFaceColor:'#180600',innerHourColor:'#FF5800',hoursLevel:245,innerHourLevel:245},frostbite:{outerRingBrightness:80,middleFaceScale:50,innerFaceScale:56,middleFaceColor:'#0C2040',hoursColor:'#C0F0FF',innerFaceColor:'#081428',innerHourColor:'#C0F0FF',hoursLevel:248,innerHourLevel:248},neongarden:{outerRingBrightness:78,middleFaceScale:55,innerFaceScale:62,middleFaceColor:'#150A28',hoursColor:'#80FF00',innerFaceColor:'#0E0618',innerHourColor:'#80FF00',hoursLevel:242,innerHourLevel:242}};\n"
  "function applyContrastPreset(val){if(!val)return;const p=CONTRAST_PRESETS[val];if(!p)return;for(const[k,v]of Object.entries(p))qs(k).value=v;saveSettings()}\n"
  "function updateContrastPreset(){let matched='';for(const[k,p]of Object.entries(CONTRAST_PRESETS)){let ok=true;for(const[f,v]of Object.entries(p)){const el=qs(f);if(!el)continue;ok=ok&&(el.type==='color'?el.value.toUpperCase()===String(v).toUpperCase():Number(el.value)===Number(v))}if(ok){matched=k;break}}qs('contrastPreset').value=matched}\n"
  "const THEMES={\"chronobloom\":{\"name\":\"ChronoBloom (clock default)\",\"version\":3,\"outerRingBrightness\":77,\"outerMarkerColor\":\"#6eb9ff\",\"outerFillerColor\":\"#0008c8\",\"secondsColor\":\"#64ffb4\",\"minutesColor\":\"#dc00b4\",\"middleFaceColor\":\"#dc00b4\",\"hoursColor\":\"#dc00b4\",\"innerFaceColor\":\"#ff3c00\",\"innerHourColor\":\"#dc00b4\",\"centerColor\":\"#ff3c00\",\"petalDepth\":45,\"outerMarkerLevel\":225,\"outerFillerLevel\":145,\"secondsLevel\":230,\"minutesLevel\":255,\"middleFaceScale\":39,\"hoursLevel\":255,\"innerFaceScale\":39,\"innerHourLevel\":255,\"centerLevel\":180,\"animationPalette\":7,\"animationSpeed\":3,\"animationBrightness\":157,\"trailLength\":6,\"reminderPalette\":0},\"moonflower\":{\"name\":\"Moonflower (night bloom)\",\"version\":3,\"outerRingBrightness\":70,\"outerMarkerColor\":\"#c8d8ff\",\"outerFillerColor\":\"#002974\",\"secondsColor\":\"#c8fff0\",\"minutesColor\":\"#f0f0ff\",\"middleFaceColor\":\"#863cd9\",\"hoursColor\":\"#f0f0ff\",\"innerFaceColor\":\"#bfa8e0\",\"innerHourColor\":\"#f0f0ff\",\"centerColor\":\"#ffc864\",\"petalDepth\":60,\"outerMarkerLevel\":210,\"outerFillerLevel\":120,\"secondsLevel\":215,\"minutesLevel\":255,\"middleFaceScale\":50,\"hoursLevel\":255,\"innerFaceScale\":55,\"innerHourLevel\":255,\"centerLevel\":170,\"animationPalette\":7,\"animationSpeed\":2,\"animationBrightness\":150,\"trailLength\":6,\"reminderPalette\":1},\"cherryblossom\":{\"name\":\"Cherry Blossom (spring bloom)\",\"version\":3,\"outerRingBrightness\":75,\"outerMarkerColor\":\"#ffb4c8\",\"outerFillerColor\":\"#662d3f\",\"secondsColor\":\"#64ff8c\",\"minutesColor\":\"#ff3c78\",\"middleFaceColor\":\"#ff3c78\",\"hoursColor\":\"#ff3c78\",\"innerFaceColor\":\"#ff7850\",\"innerHourColor\":\"#ff3c78\",\"centerColor\":\"#ffb428\",\"petalDepth\":60,\"outerMarkerLevel\":220,\"outerFillerLevel\":130,\"secondsLevel\":220,\"minutesLevel\":255,\"middleFaceScale\":50,\"hoursLevel\":255,\"innerFaceScale\":55,\"innerHourLevel\":255,\"centerLevel\":180,\"animationPalette\":7,\"animationSpeed\":3,\"animationBrightness\":157,\"trailLength\":6,\"reminderPalette\":2},\"emberdahlia\":{\"name\":\"Ember Dahlia (fire bloom)\",\"version\":3,\"outerRingBrightness\":78,\"outerMarkerColor\":\"#ffc83c\",\"outerFillerColor\":\"#970000\",\"secondsColor\":\"#fff0c8\",\"minutesColor\":\"#ff1e00\",\"middleFaceColor\":\"#ff7000\",\"hoursColor\":\"#ff1e00\",\"innerFaceColor\":\"#ffe25a\",\"innerHourColor\":\"#ff1e00\",\"centerColor\":\"#ffe07a\",\"petalDepth\":70,\"outerMarkerLevel\":225,\"outerFillerLevel\":135,\"secondsLevel\":225,\"minutesLevel\":255,\"middleFaceScale\":29,\"hoursLevel\":255,\"innerFaceScale\":32,\"innerHourLevel\":255,\"centerLevel\":190,\"animationPalette\":7,\"animationSpeed\":3,\"animationBrightness\":160,\"trailLength\":5,\"reminderPalette\":0},\"lotuspond\":{\"name\":\"Lotus Pond (water lily)\",\"version\":3,\"outerRingBrightness\":76,\"outerMarkerColor\":\"#a7e0d8\",\"outerFillerColor\":\"#2d7a7f\",\"secondsColor\":\"#3cff96\",\"minutesColor\":\"#ff50b4\",\"middleFaceColor\":\"#ffd1df\",\"hoursColor\":\"#ff50b4\",\"innerFaceColor\":\"#f36464\",\"innerHourColor\":\"#ff50b4\",\"centerColor\":\"#ffc850\",\"petalDepth\":65,\"outerMarkerLevel\":220,\"outerFillerLevel\":140,\"secondsLevel\":225,\"minutesLevel\":255,\"middleFaceScale\":28,\"hoursLevel\":255,\"innerFaceScale\":55,\"innerHourLevel\":255,\"centerLevel\":180,\"animationPalette\":7,\"animationSpeed\":3,\"animationBrightness\":157,\"trailLength\":6,\"reminderPalette\":2},\"sunflower\":{\"name\":\"Sunflower (gold & rust)\",\"version\":3,\"outerRingBrightness\":82,\"outerMarkerColor\":\"#ff9500\",\"outerFillerColor\":\"#ffa200\",\"secondsColor\":\"#af0000\",\"minutesColor\":\"#b30000\",\"middleFaceColor\":\"#ff7300\",\"hoursColor\":\"#b30000\",\"innerFaceColor\":\"#ffa000\",\"innerHourColor\":\"#b30000\",\"centerColor\":\"#660014\",\"petalDepth\":0,\"outerMarkerLevel\":28,\"outerFillerLevel\":248,\"secondsLevel\":163,\"minutesLevel\":255,\"middleFaceScale\":181,\"hoursLevel\":255,\"innerFaceScale\":244,\"innerHourLevel\":255,\"centerLevel\":195,\"animationPalette\":7,\"animationSpeed\":3,\"animationBrightness\":165,\"trailLength\":6,\"reminderPalette\":0},\"birdofparadise\":{\"name\":\"Bird of Paradise (orange & cobalt)\",\"version\":3,\"outerRingBrightness\":82,\"outerMarkerColor\":\"#ff5a0a\",\"outerFillerColor\":\"#0e28dc\",\"secondsColor\":\"#fff2dc\",\"minutesColor\":\"#ff7014\",\"middleFaceColor\":\"#446b81\",\"hoursColor\":\"#ff7014\",\"innerFaceColor\":\"#206637\",\"innerHourColor\":\"#ff7014\",\"centerColor\":\"#ff5a00\",\"petalDepth\":50,\"outerMarkerLevel\":225,\"outerFillerLevel\":135,\"secondsLevel\":210,\"minutesLevel\":255,\"middleFaceScale\":95,\"hoursLevel\":255,\"innerFaceScale\":90,\"innerHourLevel\":255,\"centerLevel\":205,\"animationPalette\":7,\"animationSpeed\":3,\"animationBrightness\":165,\"trailLength\":6,\"reminderPalette\":0}};\n"
  "function customThemes(){try{return JSON.parse(localStorage.getItem('cb_customThemes')||'{}')}catch(e){return{}}}\n"
  "function setCustomThemes(o){try{localStorage.setItem('cb_customThemes',JSON.stringify(o))}catch(e){}}\n"
  "async function saveAsTheme(){const name=(prompt('Save current clock as a theme - name it:')||'').trim();if(!name)return;const r=await fetch('/settings');const s=await r.json();delete s.hasUserDefaults;delete s.tzConfigured;const o=customThemes();o[name]=s;setCustomThemes(o);renderThemeChips();qs('themeStatus').textContent='Saved theme: '+name}\n"
  "async function applyCustomTheme(name){const s=customThemes()[name];if(!s)return;const p=new URLSearchParams();for(const[k,v]of Object.entries(s)){if(k==='hasUserDefaults')continue;p.set(k,v)}await fetch('/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()});await loadSettings();qs('themeStatus').textContent='Applied theme: '+name}\n"
  "function deleteCustomTheme(name){if(!confirm('Delete saved theme: '+name+'?'))return;const o=customThemes();delete o[name];setCustomThemes(o);renderThemeChips();qs('themeStatus').textContent='Deleted theme: '+name}\n"
  "function renderThemeChips(){const box=qs('themeChips');box.innerHTML='';for(const[key,t]of Object.entries(THEMES)){const c=document.createElement('div');c.className='themechip';c.dataset.key=key;c.onclick=()=>applyTheme(key);c.innerHTML=`<span class='dot' style='background:linear-gradient(135deg,${t.outerMarkerColor},${t.middleFaceColor},${t.centerColor})'></span>`+t.name.split(' (')[0];box.appendChild(c)}const ct=customThemes();for(const name of Object.keys(ct)){const s=ct[name];const c=document.createElement('div');c.className='themechip';c.dataset.custom=name;c.onclick=()=>applyCustomTheme(name);c.innerHTML=`<span class='dot' style='background:linear-gradient(135deg,${s.outerMarkerColor||'#888'},${s.middleFaceColor||'#888'},${s.centerColor||'#888'})'></span>`+name+`<span class='cbx' title='Delete' style='margin-left:6px;color:#e88a8a'>&#10005;</span>`;c.querySelector('.cbx').onclick=(e)=>{e.stopPropagation();deleteCustomTheme(name)};box.appendChild(c)}}\n"
  "function applyTheme(key){if(!key)return;const t=THEMES[key];if(!t)return;const fields=['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','middleFaceColor','hoursColor','innerFaceColor','innerHourColor','centerColor'];const levels=['outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel'];const anim=['animationPalette','animationSpeed','animationBrightness','trailLength','reminderPalette'];for(const k of fields)if(t[k])qs(k).value=t[k];for(const k of levels)if(t[k]!=null)qs(k).value=t[k];for(const k of anim)if(t[k]!=null)qs(k).value=t[k];if(t.outerRingBrightness!=null)qs('outerRingBrightness').value=t.outerRingBrightness;qs('petalDepth').value=t.petalDepth ?? (t.petalMode?45:0);draw();saveSettings();saveAnimStyle();qs('themeStatus').textContent='Theme applied: '+t.name}\n"
  "function updateThemePreset(){let matched='';for(const[key,t]of Object.entries(THEMES)){let ok=true;for(const[f,v]of Object.entries(t)){if(f==='name'||f==='version')continue;const el=qs(f);if(!el)continue;ok=ok&&(el.type==='color'?el.value.toUpperCase()===String(v).toUpperCase():Number(el.value)===Number(v))}if(ok){matched=key;break}}document.querySelectorAll('#themeChips .themechip').forEach(c=>c.classList.toggle('sel',c.dataset.key===matched))}\n"
  "makeClock();renderThemeChips();loadSettings();refresh();loadNet();loadVersion();setInterval(refresh,1000);setInterval(draw,90);\n"
  "function exportTheme(){const fields=['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','middleFaceColor','hoursColor','innerFaceColor','innerHourColor','centerColor'];const levels=['outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel'];const anim=['animationPalette','animationSpeed','animationBrightness','trailLength','reminderPalette'];const theme={version:3,outerRingBrightness:Number(qs('outerRingBrightness').value)};for(const k of fields)theme[k]=qs(k).value;for(const k of levels)theme[k]=Number(qs(k).value);for(const k of anim)theme[k]=Number(qs(k).value);const blob=new Blob([JSON.stringify(theme,null,2)],{type:'application/json'});const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download='chronobloom-theme.json';a.click();URL.revokeObjectURL(a.href);qs('themeStatus').textContent='Theme exported.'}\n"
  "function importTheme(file){if(!file)return;const reader=new FileReader();reader.onload=e=>{try{const theme=JSON.parse(e.target.result);const fields=['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','middleFaceColor','hoursColor','innerFaceColor','innerHourColor','centerColor'];const levels=['outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel'];const anim=['animationPalette','animationSpeed','animationBrightness','trailLength','reminderPalette'];for(const k of fields)if(theme[k])qs(k).value=theme[k];for(const k of levels)if(theme[k]!=null)qs(k).value=theme[k];for(const k of anim){if(theme[k]==null)continue;qs(k).value=(k==='animationPalette'||k==='reminderPalette')?validPalette(theme[k]):theme[k]}if(theme.outerRingBrightness!=null)qs('outerRingBrightness').value=theme.outerRingBrightness;draw();saveSettings();saveAnimStyle();qs('themeStatus').textContent='Theme imported and saved.'}catch(err){qs('themeStatus').textContent='Import failed: invalid JSON.'}};reader.readAsText(file)}\n"
  "async function exportBackup(){const r=await fetch('/settings');const s=await r.json();delete s.hasUserDefaults;delete s.tzConfigured;const blob=new Blob([JSON.stringify(s,null,2)],{type:'application/json'});const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download='chronobloom-backup.json';a.click();URL.revokeObjectURL(a.href);qs('backupStatus').textContent='Full backup exported.'}\n"
  "function importBackup(file){if(!file)return;const reader=new FileReader();reader.onload=async e=>{try{const s=JSON.parse(e.target.result);const p=new URLSearchParams();for(const[k,v]of Object.entries(s)){if(k==='hasUserDefaults')continue;p.set(k,v)}await fetch('/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()});await loadSettings();qs('backupStatus').textContent='Full backup imported and saved.'}catch(err){qs('backupStatus').textContent='Import failed: invalid JSON.'}};reader.readAsText(file)}\n"
  "</script></body></html>";

// ===== WiFi settings page (GET /wifi) =====

static const char WIFI_P1[] PROGMEM =
  "<!doctype html><html><head>\n"
  "<meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>\n"
  "<title>WiFi Settings</title>\n"
  "<style>\n"
  ":root{color-scheme:dark;--bg:#090b10;--panel:#151922;--panel2:#10141c;--line:#2c3442;--text:#eef3fb;--muted:#92a0b5;--accent:#6bd7ff}\n"
  "*{box-sizing:border-box}body{margin:0;background:radial-gradient(circle at 50% 18%,#17202f 0,#090b10 54%);color:var(--text);font-family:system-ui,Segoe UI,sans-serif}\n"
  "main{max-width:480px;margin:40px auto;padding:20px}\n"
  ".panel{background:linear-gradient(180deg,var(--panel),var(--panel2));border:1px solid var(--line);border-radius:8px;padding:24px;box-shadow:0 18px 45px #0008}\n"
  "h1{font-size:18px;margin:0 0 16px}\n"
  ".info{background:#0c1017;border:1px solid var(--line);border-radius:6px;padding:12px;margin-bottom:20px;font-size:13px;color:var(--muted);line-height:1.8}\n"
  "label{display:block;color:var(--muted);font-size:12px;margin:12px 0 4px}\n"
  "input{width:100%;font:inherit;border-radius:6px;border:1px solid #374253;background:#0c1017;color:var(--text);padding:8px 10px;min-height:38px}\n"
  ".row{display:flex;gap:10px;margin-top:20px}\n"
  "button{font:inherit;border-radius:6px;cursor:pointer;padding:10px 20px;min-height:40px;border:1px solid #42546d;background:#203146;color:var(--text)}\n"
  "button.primary{background:#145875;border-color:#2d9ccb;color:white}\n"
  "#status{margin-top:14px;min-height:18px;font-size:13px;color:var(--accent)}\n"
  "a{color:var(--accent);text-decoration:none;font-size:13px}a:hover{text-decoration:underline}\n"
  "</style></head><body><main>\n"
  "<div class='panel'>\n"
  "<h1>&#128246; WiFi Settings</h1>\n"
  "<div class='info'><strong>Saved SSID:</strong> ";

static const char WIFI_MID[] PROGMEM =
  "<br><strong>Status:</strong> ";

static const char WIFI_P2[] PROGMEM =
  "</div>\n"
  "<form onsubmit='save();return false;'>\n"
  "<label>SSID</label><input id='ssid' type='text' maxlength='32' placeholder='Network name' required>\n"
  "<label>Password</label><input id='pass' type='password' maxlength='64' placeholder='Leave blank for open network'>\n"
  "<div class='row'><button type='submit' class='primary'>Save &amp; Connect</button><button type='button' onclick='history.back()'>Cancel</button></div>\n"
  "</form>\n"
  "<div id='status'></div>\n"
  "<p style='margin-top:20px'><a href='/'>&#8592; Back to clock</a></p>\n"
  "</div></main>\n"
  "<script>\n"
  "async function save(){\n"
  "  const ssid=document.getElementById('ssid').value.trim();\n"
  "  const pass=document.getElementById('pass').value;\n"
  "  if(!ssid){alert('SSID required');return}\n"
  "  document.getElementById('status').textContent='Saving...';\n"
  "  try{\n"
  "    const r=await fetch('/wifi',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass)});\n"
  "    document.getElementById('status').textContent=await r.text();\n"
  "  }catch(e){document.getElementById('status').textContent='Saved. Device reconnecting \xe2\x80\x94 you may need to rejoin the network.';}\n"
  "}\n"
  "</script></body></html>";

// ===== Firmware update page (GET /update) =====

static const char UPDATE_P1[] PROGMEM =
  "<!doctype html><html><head>\n"
  "<meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>\n"
  "<title>Firmware Update - ChronoBloom</title>\n"
  "<style>\n"
  ":root{color-scheme:dark;--bg:#090b10;--panel:#151922;--panel2:#10141c;--line:#2c3442;--text:#eef3fb;--muted:#92a0b5;--accent:#6bd7ff}\n"
  "*{box-sizing:border-box}body{margin:0;background:radial-gradient(circle at 50% 18%,#17202f 0,#090b10 54%);color:var(--text);font-family:system-ui,Segoe UI,sans-serif}\n"
  "main{max-width:600px;margin:40px auto;padding:20px}\n"
  ".panel{background:linear-gradient(180deg,var(--panel),var(--panel2));border:1px solid var(--line);border-radius:8px;box-shadow:0 18px 45px #0008;padding:24px;margin-bottom:16px}\n"
  "h1{margin:0 0 8px;font-size:24px}p{color:var(--muted);margin:0 0 16px}\n"
  "input[type=file]{display:block;margin:16px 0;padding:8px;background:#0c1017;border:1px solid #374253;border-radius:6px;color:var(--text)}\n"
  "button{background:#145875;border:1px solid #2d9ccb;color:white;padding:12px 24px;border-radius:6px;font-size:16px;cursor:pointer;margin:8px 8px 8px 0}\n"
  "button:hover{background:#1a6a8f}button.danger{background:#8b2e2e;border-color:#c04040}\n"
  ".progress{width:100%;height:24px;background:#0c1017;border:1px solid #374253;border-radius:4px;overflow:hidden;margin:16px 0}\n"
  ".progress-bar{height:100%;background:#6bd7ff;width:0%;transition:width 0.3s;display:flex;align-items:center;justify-content:center;font-size:12px;color:#090b10}\n"
  ".status{margin:16px 0;padding:12px;border-radius:6px;display:none}\n"
  ".status.success{background:#0a3a2a;border:1px solid #10a060;color:#90ff90}\n"
  ".status.error{background:#3a0a0a;border:1px solid #a01010;color:#ff9090}\n"
  ".status.info{background:#0a1a3a;border:1px solid #1060a0;color:#90d0ff}\n"
  "#updateForm{margin:16px 0}\n"
  ".info-box{background:#0c1017;border-left:3px solid #6bd7ff;padding:12px;margin:16px 0;border-radius:4px}\n"
  "a{color:var(--accent);text-decoration:none}a:hover{text-decoration:underline}\n"
  "</style></head>\n"
  "<body><main>\n"
  "<div class='panel'>\n"
  "<h1>&#128295; Firmware Update</h1>\n"
  "<p>Upload a new .bin firmware file to update the clock.</p>\n"
  "<div class='info-box'>\n"
  "<strong>Current Firmware:</strong> ";

static const char UPDATE_P1B[] PROGMEM =
  "<br>\n"
  "<strong>Flash Size:</strong> ~700 KB<br>\n"
  "<strong>Device:</strong> XIAO ESP32-C3\n"
  "</div>\n"
  "<div id='updateForm'>\n"
  "<input type='file' id='binFile' accept='.bin' required>\n"
  "<button onclick='uploadFirmware()'>Upload &amp; Update</button>\n"
  "<button onclick='history.back()'>Cancel</button>\n"
  "</div>\n"
  "<div class='progress' id='progress' style='display:none'>\n"
  "<div class='progress-bar' id='progressBar'>0%</div>\n"
  "</div>\n"
  "<div class='status' id='status'></div>\n"
  "</main>\n";

static const char UPDATE_P2[] PROGMEM =
  "<script>\n"
  "function uploadFirmware(){\n"
  "  const file=document.getElementById('binFile').files[0];\n"
  "  if(!file){alert('Please select a .bin file');return}\n"
  "  if(!file.name.endsWith('.bin')){alert('File must be .bin format');return}\n"
  "  if(file.size>800000){alert('File too large (max ~700KB)');return}\n"
  "  const xhr=new XMLHttpRequest();\n"
  "  xhr.upload.onprogress=(e)=>{\n"
  "    if(e.lengthComputable){\n"
  "      const pct=Math.round(e.loaded/e.total*100);\n"
  "      document.getElementById('progressBar').style.width=pct+'%';\n"
  "      document.getElementById('progressBar').textContent=pct+'%';\n"
  "    }\n"
  "  };\n"
  "  xhr.onload=()=>{\n"
  "    const msg=document.getElementById('status');\n"
  "    if(xhr.status===200){\n"
  "      msg.innerHTML='&#10003; Upload successful! Device will reboot with new firmware...';\n"
  "      msg.className='status success';\n"
  "      document.getElementById('updateForm').style.display='none';\n"
  "      setTimeout(()=>{window.location.href='/'},5000);\n"
  "    }else{\n"
  "      msg.innerHTML='&#10007; Update failed: '+xhr.responseText;\n"
  "      msg.className='status error';\n"
  "    }\n"
  "    msg.style.display='block';\n"
  "  };\n"
  "  xhr.onerror=()=>{\n"
  "    const msg=document.getElementById('status');\n"
  "    msg.innerHTML='&#10007; Upload error (connection lost?)';\n"
  "    msg.className='status error';\n"
  "    msg.style.display='block';\n"
  "  };\n"
  "  document.getElementById('progress').style.display='block';\n"
  "  const fd=new FormData();\n"
  "  fd.append('firmware',file,file.name);\n"
  "  xhr.open('POST','/update');\n"
  "  xhr.send(fd);\n"
  "}\n"
  "</script></body></html>";

// ===== Demo overlay page (GET /demo/overlay) =====

static const char OVERLAY_HTML[] = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width,initial-scale=1'>
  <title>Demo Overlay</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      background: #000;
      width: 100vw;
      height: 100vh;
      overflow: hidden;
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    }
    #overlay {
      position: fixed;
      bottom: 33vh;
      left: 50%;
      transform: translateX(-50%);
      text-align: center;
      opacity: 0;
      transition: opacity 300ms ease;
    }
    #overlay.active {
      opacity: 1;
    }
    #subtitle {
      font-size: 48px;
      color: white;
      background: rgba(0, 0, 0, 0.6);
      padding: 20px 40px;
      border-radius: 50px;
      display: inline-block;
      max-width: 80vw;
      word-wrap: break-word;
    }
  </style>
</head>
<body>
  <div id='overlay'>
    <div id='subtitle'></div>
  </div>

  <script>
    const overlay = document.getElementById('overlay');
    const subtitle = document.getElementById('subtitle');
    let lastSubtitle = '';

    async function updateStatus() {
      try {
        const response = await fetch('/demo/status');
        const data = await response.json();

        // Pre-roll counts as "not yet running" here: this overlay is composited
        // onto the recording, so it must stay blank while the clock is dark and
        // only fade the first subtitle in as the reel actually opens.
        if (data.active && !data.preroll) {
          if (data.subtitle !== lastSubtitle) {
            overlay.classList.remove('active');
            setTimeout(() => {
              subtitle.textContent = data.subtitle;
              overlay.classList.add('active');
              lastSubtitle = data.subtitle;
            }, 50);
          } else if (!overlay.classList.contains('active')) {
            overlay.classList.add('active');
          }
        } else {
          overlay.classList.remove('active');
          lastSubtitle = '';
        }
      } catch (e) {
        console.error('Status fetch failed:', e);
      }
    }

    updateStatus();
    setInterval(updateStatus, 500);
  </script>
</body>
</html>
      )";
