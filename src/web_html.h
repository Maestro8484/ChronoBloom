#pragma once
#include <Arduino.h>

// PROGMEM string constants extracted from WebUi::setupRoutes.
// Included by main.cpp; all symbols have internal linkage (static).

// ===== Index page (GET /) =====

static const char INDEX_P1[] PROGMEM =
  "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>\n"
  "<title>ESP32 Ring Clock</title>\n"
  "<style>\n"
  ":root{color-scheme:dark;--bg:#090b10;--panel:#151922;--panel2:#10141c;--line:#2c3442;--text:#eef3fb;--muted:#92a0b5;--accent:#6bd7ff}\n"
  "*{box-sizing:border-box}body{margin:0;background:radial-gradient(circle at 50% 18%,#17202f 0,#090b10 54%);color:var(--text);font-family:system-ui,Segoe UI,sans-serif}\n"
  "main{display:grid;grid-template-columns:minmax(280px,430px) minmax(310px,1fr);gap:18px;max-width:1120px;margin:0 auto;padding:18px}\n"
  ".stage,.panel{background:linear-gradient(180deg,var(--panel),var(--panel2));border:1px solid var(--line);border-radius:8px;box-shadow:0 18px 45px #0008}\n"
  ".stage{position:sticky;top:14px;align-self:start;padding:16px}.clock-wrap{display:grid;place-items:center;min-height:380px}\n"
  "h1{font-size:18px;margin:0 0 4px}.sub{color:var(--muted);font-size:13px;margin:0 0 12px}#now{font-size:42px;font-weight:750;letter-spacing:0;margin:8px 0 2px}.state{color:var(--muted);font-size:13px;line-height:1.4;min-height:20px}\n"
  ".grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}.panel{padding:14px;margin-bottom:12px}.panel h2{font-size:15px;margin:0 0 12px;color:#dbe7f7}\n"
  "label{display:block;color:var(--muted);font-size:12px;margin:10px 0 4px}input,select,button{font:inherit;border-radius:6px;border:1px solid #374253;background:#0c1017;color:var(--text);padding:8px;min-height:38px}\n"
  "input[type=number]{width:82px}input[type=color]{width:58px;padding:3px}.row{display:flex;gap:8px;align-items:end;flex-wrap:wrap}.ringrow{display:grid;grid-template-columns:82px 66px 1fr 54px;gap:8px;align-items:center;margin:8px 0}.ringrow span{color:#dbe7f7}\n"
  "button{cursor:pointer;background:#203146;border-color:#42546d}button.primary{background:#145875;border-color:#2d9ccb;color:white}.toggle{display:flex;gap:8px;flex-wrap:wrap}.toggle label{display:flex;gap:6px;align-items:center;margin:0;color:#dce6f5;background:#0c1017;border:1px solid #303846;border-radius:6px;padding:8px}\n"
  "svg{width:min(86vw,380px);height:auto;display:block}.led{opacity:.18;transition:fill .18s,opacity .18s,filter .18s}.on{opacity:1;filter:drop-shadow(0 0 5px currentColor)}.ghost{opacity:.4}.marker{opacity:.5}.center{filter:drop-shadow(0 0 12px currentColor)}\n"
  "@media(max-width:820px){main{grid-template-columns:1fr}.stage{position:static}.grid{grid-template-columns:1fr}.clock-wrap{min-height:300px}}\n"
  "</style></head>\n";

static const char INDEX_P2[] PROGMEM =
  "<body><main>\n"
  "<section class='stage'>\n"
  "<h1>ESP32 Ring Clock <span id='fwVersion' style='font-size:11px;font-weight:400;color:var(--muted);vertical-align:middle'></span></h1><p class='sub'>Outer 60 LEDs are the clock face, minute hand, and second hand; middle/inner rings show hours.</p>\n"
  "<div class='clock-wrap'><svg id='clockSvg' viewBox='0 0 420 420' role='img' aria-label='NeoPixel clock preview'></svg></div>\n"
  "<div id='now'>--:--:--</div><div id='state' class='state'>Connecting...</div>\n"
  "</section>\n"
  "<section>\n"
  "<div class='grid'>\n"
  "<div class='panel'><h2>Time</h2>\n"
  "<form onsubmit='setTime();return false;' class='row'>\n"
  "<div><label>Hour</label><input id='h' type='number' min='0' max='23' placeholder='HH'></div>\n"
  "<div><label>Minute</label><input id='m' type='number' min='0' max='59' placeholder='MM'></div>\n"
  "<div><label>Second</label><input id='s' type='number' min='0' max='59' placeholder='SS'></div>\n"
  "<button class='primary' type='submit'>Set</button></form>\n"
  "<div class='row'><button onclick='post(\"/addMinute\")'>+1 min</button><button onclick='post(\"/subMinute\")'>-1 min</button><button onclick='syncBrowser()'>Browser sync</button><button onclick='post(\"/syncNtp\")'>NTP sync</button></div>\n"
  "</div>\n"
  "<div class='panel'><h2>Display</h2>\n"
  "<div class='row'><div><label>Day</label><input id='dayBrightness' type='number' min='0' max='255'></div><div><label>Night</label><input id='nightBrightness' type='number' min='0' max='255'></div><div><label>Night start</label><input id='nightStartHour' type='number' min='0' max='23'></div><div><label>Night end</label><input id='nightEndHour' type='number' min='0' max='23'></div></div>\n"
  "<label>Preview effect</label><select id='previewMode'><option value='live'>Live clock</option><option value='trail'>All-ring trails</option><option value='spark'>Hourly sparkle</option></select>\n"
  "</div></div>\n"
  "<div class='panel'><h2>Rings</h2>\n"
  "<div class='ringrow'><span>Outer marks</span><input id='outerMarkerColor' type='color'><input id='outerMarkerLevel' type='range' min='0' max='255'><output id='outerMarkerLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Outer fill</span><input id='outerFillerColor' type='color'><input id='outerFillerLevel' type='range' min='0' max='255'><output id='outerFillerLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Outer sec</span><input id='secondsColor' type='color'><input id='secondsLevel' type='range' min='0' max='255'><output id='secondsLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Outer min</span><input id='minutesColor' type='color'><input id='minutesLevel' type='range' min='0' max='255'><output id='minutesLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Middle face</span><input id='middleFaceColor' type='color'><input id='middleFaceScale' type='range' min='0' max='255'><output id='middleFaceScaleOut'></output></div>\n"
  "<div class='ringrow'><span>Middle hour</span><input id='hoursColor' type='color'><input id='hoursLevel' type='range' min='0' max='255'><output id='hoursLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Inner face</span><input id='innerFaceColor' type='color'><input id='innerFaceScale' type='range' min='0' max='255'><output id='innerFaceScaleOut'></output></div>\n"
  "<div class='ringrow'><span>Inner hour</span><input id='innerHourColor' type='color'><input id='innerHourLevel' type='range' min='0' max='255'><output id='innerHourLevelOut'></output></div>\n"
  "<div class='ringrow'><span>Center</span><input id='centerColor' type='color'><input id='centerLevel' type='range' min='0' max='255'><output id='centerLevelOut'></output></div>\n"
  "<div class='row'><div><label>Outer ring brightness (%)</label><input id='outerRingBrightness' type='number' min='0' max='100' style='width:70px'></div></div>\n"
  "<div class='row'><div><label>Contrast preset</label><select id='contrastPreset' onchange='applyContrastPreset(this.value)'><option value=''>Custom</option><option value='current'>Defaults</option><option value='natural'>Natural Contrast</option><option value='subtle'>Subtle Bloom</option><option value='bloom'>Deep Bloom</option><option value='crisp'>Crisp</option><option value='vivid'>Vivid</option><option value='solstice'>Solstice</option><option value='deepspace'>Deep Space</option><option value='lava'>Lava</option><option value='frostbite'>Frostbite</option><option value='neongarden'>Neon Garden</option></select></div></div>\n"
  "<div class='row'><div><label>Theme</label><select id='colorTheme'><option value='0'>Classic</option><option value='1'>Aqua</option><option value='2'>Magenta</option></select></div></div>\n"
  "<div class='toggle'><label><input id='secondTrail' type='checkbox'>Second trail</label><label><input id='progressSeconds' type='checkbox'>Progress ring</label><label><input id='hourlyChime' type='checkbox'>Hourly chime</label><label><input id='statusAnimations' type='checkbox'>Status</label></div>\n"
  "<div class='row'><div><label>Ring rotation offset (0-59 LEDs)</label><input id='outerRingOffset' type='number' min='0' max='59' style='width:70px'></div></div>\n"
  "<div class='row'><button class='primary' onclick='saveSettings()'>Save display</button><button onclick='saveAsDefault()' title='Save current settings as the restore point for Reset'>Save as default</button><span id='defaultSavedMsg' style='display:none;color:#27ae60;margin-left:8px;font-size:0.9em'>&#10003; Saved as default</span></div>\n"
  "<div class='row'><button id='resetBtn' onclick='resetToDefaults()' style='color:#c0392b'>Reset to defaults</button></div>\n"
  "</div>\n"
  "<div class='panel'><h2>Auto-Brightness</h2>\n"
  "<select id='autoBrightnessMode'>\n"
  "  <option value='0'>Manual</option>\n"
  "  <option value='1'>Auto (light sensor)</option>\n"
  "  <option value='2'>Scheduled (day/night)</option>\n"
  "</select>\n"
  "<div id='autoPanel' style='display:none'>\n"
  "  <label>Current light: <span id='luxValue'>--</span> lux</label>\n"
  "  <div class='row'>\n"
  "    <div><label>Min bright</label><input id='minAutoBrightness' type='number' min='5' max='255'></div>\n"
  "    <div><label>Max bright</label><input id='maxAutoBrightness' type='number' min='5' max='255'></div>\n"
  "  </div>\n"
  "  <div class='toggle'><label><input id='darkRoomOff' type='checkbox'> Lights out in pitch black (&lt;0.3 lux for 30s; wakes above 2 lux)</label></div>\n"
  "</div>\n"
  "<div class='row'><button class='primary' onclick='saveSettings()'>Save auto-brightness</button></div>\n"
  "</div>\n"
  "<div class='panel'><h2>Time Animations</h2>\n"
  "<label><input id='intervalAnimationsEnabled' type='checkbox'> Enable interval animations</label>\n"
  "<div class='row'>\n"
  "  <div><label>Quarter (:15/:30/:45)</label><select id='quarterAnimation'>\n"
  "    <option value='0'>Off</option>\n"
  "    <option value='1'>Slow comet</option>\n"
  "    <option value='2'>Dual orbit</option>\n"
  "    <option value='3'>Bloom ripple</option>\n"
  "  </select><button onclick=\"previewAnim('quarter','quarterAnimation')\">&#9654; Preview</button></div>\n"
  "  <div><label>Half-hour (:30)</label><select id='halfHourAnimation'>\n"
  "    <option value='0'>Off</option>\n"
  "    <option value='1'>Unfurl</option>\n"
  "    <option value='2'>Three comets</option>\n"
  "    <option value='3'>Breathe</option>\n"
  "  </select><button onclick=\"previewAnim('halfhour','halfHourAnimation')\">&#9654; Preview</button></div>\n"
  "</div>\n"
  "<label>Top of hour (:00)</label><select id='hourAnimation'>\n"
  "  <option value='0'>Off</option>\n"
  "  <option value='1'>Ceremony</option>\n"
  "  <option value='2'>Galaxy spin</option>\n"
  "  <option value='3'>Supernova</option>\n"
  "  <option value='4'>Comet relay</option>\n"
  "  <option value='5'>Deep breath</option>\n"
  "</select><button onclick=\"previewAnim('hour','hourAnimation')\">&#9654; Preview</button>\n"
  "<div class='row'><button class='primary' onclick='saveSettings()'>Save animations</button></div>\n"
  "</div>\n"
  "<div class='panel'><h2>&#127775; Animation Style</h2>\n"
  "<div class='row'>\n"
  "  <div><label>Color palette</label><select id='animationPalette'>\n"
  "    <option value='7'>Clock colors (default)</option>\n"
  "    <option value='0'>Rainbow</option>\n"
  "    <option value='1'>Fire</option>\n"
  "    <option value='2'>Ocean</option>\n"
  "    <option value='3'>Forest</option>\n"
  "    <option value='4'>Candy</option>\n"
  "    <option value='5'>Neon</option>\n"
  "    <option value='6'>Monochrome (hour color)</option>\n"
  "  </select></div>\n"
  "  <div><label>Reminder palette</label><select id='reminderPalette'>\n"
  "    <option value='0'>Amber (warm)</option>\n"
  "    <option value='1'>Red (urgent)</option>\n"
  "    <option value='2'>Magenta (bold)</option>\n"
  "    <option value='3'>Cyan-warm (unusual)</option>\n"
  "  </select></div>\n"
  "</div>\n"
  "<div class='row'>\n"
  "  <div><label>Speed</label><select id='animationSpeed'>\n"
  "    <option value='1'>1 - Dreamy slow</option>\n"
  "    <option value='2'>2 - Relaxed</option>\n"
  "    <option value='3'>3 - Normal</option>\n"
  "    <option value='4'>4 - Energetic</option>\n"
  "    <option value='5'>5 - Hyperactive</option>\n"
  "  </select></div>\n"
  "  <div><label>Peak brightness</label>\n"
  "    <input id='animationBrightness' type='range' min='50' max='255'>\n"
  "    <output id='animationBrightnessOut'></output>\n"
  "  </div>\n"
  "  <div><label>Trail length</label>\n"
  "    <input id='trailLength' type='range' min='2' max='12'>\n"
  "    <output id='trailLengthOut'></output>\n"
  "  </div>\n"
  "</div>\n"
  "<div class='row'>\n"
  "  <div><label>Preview slot</label><select id='stylePreviewType'>\n"
  "    <option value='quarter'>Quarter (current selection)</option>\n"
  "    <option value='halfhour'>Half-hour (current selection)</option>\n"
  "    <option value='hour'>Hour (current selection)</option>\n"
  "    <option value='reminder'>Reminder (current selection)</option>\n"
  "  </select></div>\n"
  "  <button onclick='previewStyleAnim()'>&#9654; Preview style</button>\n"
  "  <button class='primary' onclick='saveAnimStyle()'>Save style</button>\n"
  "</div>\n"
  "</div>\n"
  "<div class='panel'><h2>Focus Reminders (ADHD)</h2>\n"
  "<p class='sub' style='font-size:12px;color:#92a0b5'>Visual nudge system for hyperfocus interruption. Fires animations at set intervals on selected days/times.</p>\n"
  "<div class='toggle'><label><input id='focusReminder_enabled' type='checkbox'> Enable focus reminders</label></div>\n"
  "<div class='row'>\n"
  "  <div><label>Start hour</label><input id='focusReminder_startHour' type='number' min='0' max='23' placeholder='HH'></div>\n"
  "  <div><label>End hour</label><input id='focusReminder_endHour' type='number' min='0' max='23' placeholder='HH'></div>\n"
  "  <div><label>Interval (min)</label><input id='focusReminder_intervalMinutes' type='number' min='1' max='1440' placeholder='60'></div>\n"
  "</div>\n"
  "<label>Days of week</label>\n"
  "<div class='toggle' id='daysToggle' style='display:flex;gap:4px;flex-wrap:wrap'></div>\n"
  "<div><label>Animation</label><select id='focusReminder_animation'>\n"
  "  <option value='0'>Use quarter animation</option>\n"
  "  <option value='1'>Use half-hour animation</option>\n"
  "  <option value='2'>Use hour animation</option>\n"
  "  <option value='3'>Use quarter animation (alt)</option>\n"
  "  <option value='4'>Use half-hour animation (alt)</option>\n"
  "  <option value='5'>Use hour animation (alt)</option>\n"
  "  <option value='6'>Gentle pulse</option>\n"
  "  <option value='7'>Orbiting orb</option>\n"
  "  <option value='8'>Ripple in</option>\n"
  "  <option value='9'>Heartbeat</option>\n"
  "  <option value='10'>Slow bloom</option>\n"
  "</select><button onclick=\"previewAnim('reminder','focusReminder_animation')\">&#9654; Preview</button></div>\n"
  "<div class='row'><button class='primary' onclick='saveFocusReminder()'>Save reminder</button></div>\n"
  "</div>\n"
  "<div class='panel'><h2>&#127912; Color Theme</h2>\n"
  "<p class='sub' style='font-size:12px;color:#92a0b5'>Export or import ring colors, levels, and animation style (palette, speed, peak brightness, trail length) as a portable JSON file.</p>\n"
  "<div class='row'>\n"
  "  <button onclick='exportTheme()'>&#8681; Export theme</button>\n"
  "  <label style='display:inline-flex;align-items:center;gap:8px;margin:0;cursor:pointer;background:#203146;border:1px solid #42546d;border-radius:6px;padding:8px 14px;color:var(--text)'>\n"
  "    &#8679; Import theme\n"
  "    <input type='file' id='themeImportFile' accept='.json' style='display:none' onchange='importTheme(this.files[0])'>\n"
  "  </label>\n"
  "</div>\n"
  "<div id='themeStatus' style='font-size:12px;color:#92a0b5;margin-top:8px;min-height:16px'></div>\n"
  "</div>\n"
  "<div class='panel'><h2>&#128190; Full Settings Backup</h2>\n"
  "<p class='sub' style='font-size:12px;color:#92a0b5'>Export or import every setting on this clock — ring colors/levels, animation palette, speed, peak brightness, trail length, auto-brightness, focus reminders, everything.</p>\n"
  "<div class='row'>\n"
  "  <button onclick='exportBackup()'>&#8681; Export full backup</button>\n"
  "  <label style='display:inline-flex;align-items:center;gap:8px;margin:0;cursor:pointer;background:#203146;border:1px solid #42546d;border-radius:6px;padding:8px 14px;color:var(--text)'>\n"
  "    &#8679; Import full backup\n"
  "    <input type='file' id='backupImportFile' accept='.json' style='display:none' onchange='importBackup(this.files[0])'>\n"
  "  </label>\n"
  "</div>\n"
  "<div id='backupStatus' style='font-size:12px;color:#92a0b5;margin-top:8px;min-height:16px'></div>\n"
  "</div>\n"
  "<div class='panel'><h2>Network</h2><div id='net' class='state'>--</div><div class='row'><button onclick='loadNet()'>Refresh network</button></div></div>\n"
  "<div class='panel'><h2>&#9881; Admin</h2><div class='row'>"
  "<a href='/update' style='display:inline-block;padding:10px 16px;background:#145875;border:1px solid #2d9ccb;color:white;border-radius:6px;text-decoration:none;font-size:14px'>Firmware Update</a>"
  "<a href='/wifi' style='display:inline-block;padding:10px 16px;background:#145875;border:1px solid #2d9ccb;color:white;border-radius:6px;text-decoration:none;font-size:14px'>WiFi Settings</a>"
  "</div></div>\n"
  "<div class='panel'><h2>Demo Mode (Video Recording)</h2>\n"
  "<p class='sub' style='font-size:12px;color:#92a0b5'>Automated ~110-second feature showcase — chimes, palettes, ADHD nudges, auto-dimming. Use /demo/overlay in OBS for live subtitles.</p>\n"
  "<div class='row'>\n"
  "  <button class='primary' onclick='startDemo()' id='demoStartBtn'>▶ Start Demo</button>\n"
  "  <button onclick='stopDemo()' id='demoStopBtn' style='display:none;background:#8b3a3a;border-color:#c75555'>⏹ Stop</button>\n"
  "</div>\n"
  "<div id='demoStatus' class='state' style='margin-top:12px;padding:8px;background:#0c1017;border:1px solid #2c3442;border-radius:4px'>Idle</div>\n"
  "</div>\n"
  "</section></main>\n";

static const char INDEX_P3[] PROGMEM =
  "<script>\n"
  "const counts={outer:60,middle:24,inner:12}, radii={outer:182,middle:134,inner:88};\n"
  "const leds={outer:[],middle:[],inner:[]}; let settings={}, current={hour:12,minute:0,second:0}, netTimer=0;\n"
  "function qs(id){return document.getElementById(id)} function pad(n){return String(n).padStart(2,'0')}\n"
  "function makeClock(){const svg=qs('clockSvg'); for(const ring of ['outer','middle','inner']){for(let i=0;i<counts[ring];i++){const a=(i/counts[ring])*Math.PI*2-Math.PI/2,x=210+Math.cos(a)*radii[ring],y=210+Math.sin(a)*radii[ring],c=document.createElementNS('http://www.w3.org/2000/svg','circle');c.setAttribute('cx',x);c.setAttribute('cy',y);c.setAttribute('r',ring==='outer'?4.4:ring==='middle'?5.8:7.2);c.classList.add('led');svg.appendChild(c);leds[ring].push(c)}} const center=document.createElementNS('http://www.w3.org/2000/svg','circle');center.id='centerLed';center.setAttribute('cx',210);center.setAttribute('cy',210);center.setAttribute('r',17);center.classList.add('led','center');svg.appendChild(center)}\n"
  "function setLed(el,color,level,cls='on'){el.style.color=color;el.setAttribute('fill',color);el.style.opacity=Math.max(.04,level/255);el.className.baseVal='led '+cls}\n"
  "function clearRing(r){for(const el of leds[r]){el.setAttribute('fill','#243044');el.style.opacity=.22;el.className.baseVal='led'}}\n"
  "function level(id){return Number(qs(id+'Level')?.value||180)} function color(id){return qs(id+'Color')?.value||'#ffffff'}\n"
  "function draw(){clearRing('middle');clearRing('inner');for(let i=0;i<60;i++){const mark=i%5===0;setLed(leds.outer[i],mark?color('outerMarker'):color('outerFiller'),mark?level('outerMarker'):level('outerFiller'),mark?'marker':'ghost')}let s=current.second,m=current.minute,h=current.hour%12,hoff=current.minute>=30?1:0,h24=(h*2+hoff)%24;const mode=qs('previewMode')?.value||'live',tick=Math.floor(Date.now()/90);if(settings.progressSeconds){for(let i=0;i<=s;i++)setLed(leds.outer[i],color('seconds'),38,'ghost')}if(settings.secondTrail||mode==='trail'){for(let i=1;i<7;i++)setLed(leds.outer[(s+60-i)%60],color('seconds'),Math.max(20,level('seconds')-(i*32)),'ghost')}if(mode==='trail'){for(let i=1;i<5;i++){setLed(leds.outer[(m+60-i)%60],color('minutes'),Math.max(25,level('minutes')-(i*42)),'ghost');setLed(leds.middle[(h24+24-i)%24],color('hours'),Math.max(25,level('hours')-(i*42)),'ghost');setLed(leds.inner[(h+12-i)%12],color('innerHour'),Math.max(25,level('innerHour')-(i*52)),'ghost')}}if(mode==='spark'){for(let i=0;i<10;i++){setLed(leds.outer[(tick+i*6)%60],i%2?color('innerHour'):color('minutes'),90+(i*10),'ghost')}}const mfs=Number(qs('middleFaceScale')?.value??50);const ifs=Number(qs('innerFaceScale')?.value??50);for(let i=0;i<24;i++)setLed(leds.middle[i],color('middleFace'),mfs,'marker');for(let i=0;i<12;i++)setLed(leds.inner[i],color('innerFace'),ifs,'marker');setLed(leds.outer[s],color('seconds'),level('seconds'));setLed(leds.outer[m],color('minutes'),level('minutes'));setLed(leds.middle[h24],color('hours'),level('hours'));setLed(leds.inner[h],color('innerHour'),level('innerHour'));setLed(leds.inner[(h+hoff)%12],color('innerHour'),level('innerHour'));const pulse=45+Math.floor((Math.sin(Date.now()/450)+1)*85);setLed(qs('centerLed'),color('center'),Math.min(level('center'),pulse),'on')}\n"
  "async function refresh(){const r=await fetch('/time');const t=await r.json();current=t;const h12=t.hour%12||12;const ampm=t.hour<12?'AM':'PM';qs('now').textContent=`${pad(h12)}:${pad(t.minute)}:${pad(t.second)} ${ampm}`;qs('state').textContent=`IP ${t.ip||'-'} | Wi-Fi ${t.wifi?'on':'off'} | NTP ${t.ntpSynced?'synced':'waiting'}`;draw()}\n"
  "async function loadNet(){const r=await fetch('/net');const n=await r.json();qs('net').textContent=`${n.hostname} | ${n.ssid} | IP ${n.ip} | GW ${n.gateway} | RSSI ${n.rssi} dBm`}\n"
  "async function loadSettings(){const r=await fetch('/settings');settings=await r.json();for(const k of ['dayBrightness','nightBrightness','nightStartHour','nightEndHour','colorTheme','outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','hoursLevel','middleFaceScale','innerFaceScale','innerHourLevel','centerLevel'])qs(k).value=settings[k];for(const k of ['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','hoursColor','middleFaceColor','innerFaceColor','innerHourColor','centerColor'])qs(k).value=settings[k];for(const k of ['secondTrail','progressSeconds','hourlyChime','statusAnimations'])qs(k).checked=!!settings[k];qs('autoBrightnessMode').value=settings.autoBrightnessMode;qs('minAutoBrightness').value=settings.minAutoBrightness;qs('maxAutoBrightness').value=settings.maxAutoBrightness;qs('darkRoomOff').checked=!!settings.darkRoomOff;qs('quarterAnimation').value=settings.quarterAnimation;qs('halfHourAnimation').value=settings.halfHourAnimation;qs('hourAnimation').value=settings.hourAnimation;qs('intervalAnimationsEnabled').checked=!!settings.intervalAnimationsEnabled;qs('focusReminder_enabled').checked=!!settings.focusReminder_enabled;qs('focusReminder_startHour').value=settings.focusReminder_startHour||8;qs('focusReminder_endHour').value=settings.focusReminder_endHour||22;qs('focusReminder_intervalMinutes').value=settings.focusReminder_intervalMinutes||60;qs('focusReminder_animation').value=settings.focusReminder_animation||0;const daysToggle=qs('daysToggle');daysToggle.innerHTML='';const daysNames=['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];for(let i=0;i<7;i++){const label=document.createElement('label');const checkbox=document.createElement('input');checkbox.type='checkbox';checkbox.checked=!!(settings.focusReminder_daysMask&(1<<i));checkbox.id='focusReminder_day'+i;label.appendChild(checkbox);label.appendChild(document.createTextNode(daysNames[i]));daysToggle.appendChild(label)}qs('outerRingOffset').value=settings.outerRingOffset||0;qs('animationPalette').value=settings.animationPalette??0;qs('animationSpeed').value=settings.animationSpeed??3;qs('animationBrightness').value=settings.animationBrightness??200;qs('trailLength').value=settings.trailLength??4;qs('reminderPalette').value=settings.reminderPalette??0;qs('outerRingBrightness').value=settings.outerRingBrightness??90;updateContrastPreset();qs('autoBrightnessMode').onchange=()=>{qs('autoPanel').style.display=Number(qs('autoBrightnessMode').value)===1?'block':'none'};qs('autoBrightnessMode').onchange();bindLive();draw();refreshLux();setInterval(refreshLux,2000);updateDefaultLabel()}\n"
  "async function refreshLux(){const r=await fetch('/lux');const data=await r.json();if(data.available){qs('luxValue').textContent=data.lux.toFixed(1)}}\n"
  "async function loadVersion(){try{const r=await fetch('/diag');const d=await r.json();qs('fwVersion').textContent='v'+d.firmware_version}catch(e){}}\n"
  "function bindLive(){for(const k of ['outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel']){const out=qs(k+'Out');const upd=()=>{out.value=qs(k).value;if(['middleFaceScale','innerFaceScale','hoursLevel','innerHourLevel'].includes(k))qs('contrastPreset').value='';draw()};qs(k).oninput=upd;upd()}for(const k of ['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','centerColor','previewMode'])qs(k).oninput=draw;for(const k of ['middleFaceColor','hoursColor','innerFaceColor','innerHourColor'])qs(k).oninput=()=>{qs('contrastPreset').value='';draw()};for(const k of ['secondTrail','progressSeconds'])qs(k).oninput=()=>{settings[k]=qs(k).checked;draw()};for(const k of ['animationBrightness','trailLength']){const out=qs(k+'Out');const upd=()=>{out.value=qs(k).value};qs(k).oninput=upd;upd()}qs('outerRingBrightness').oninput=()=>{qs('contrastPreset').value='';draw()}}\n"
  "async function post(url,body){await fetch(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});await refresh()}\n"
  "function setTime(){post('/set',`hour=${h.value}&minute=${m.value}&second=${s.value}`)}\n"
  "function syncBrowser(){const d=new Date();post('/syncBrowser',`hour=${d.getHours()}&minute=${d.getMinutes()}&second=${d.getSeconds()}`)}\n"
  "function saveSettings(){const p=new URLSearchParams();for(const k of ['dayBrightness','nightBrightness','nightStartHour','nightEndHour','colorTheme','outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel','outerRingBrightness'])p.set(k,qs(k).value);for(const k of ['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','middleFaceColor','hoursColor','innerFaceColor','innerHourColor','centerColor'])p.set(k,qs(k).value);for(const k of ['secondTrail','progressSeconds','hourlyChime','statusAnimations'])p.set(k,qs(k).checked?1:0);p.set('autoBrightnessMode',qs('autoBrightnessMode').value);p.set('minAutoBrightness',qs('minAutoBrightness').value);p.set('maxAutoBrightness',qs('maxAutoBrightness').value);p.set('darkRoomOff',qs('darkRoomOff').checked?1:0);p.set('quarterAnimation',qs('quarterAnimation').value);p.set('halfHourAnimation',qs('halfHourAnimation').value);p.set('hourAnimation',qs('hourAnimation').value);p.set('intervalAnimationsEnabled',qs('intervalAnimationsEnabled').checked?1:0);p.set('outerRingOffset',qs('outerRingOffset').value);post('/settings',p.toString()).then(loadSettings)}\n"
  "function saveFocusReminder(){const p=new URLSearchParams();p.set('focusReminder_enabled',qs('focusReminder_enabled').checked?1:0);p.set('focusReminder_startHour',qs('focusReminder_startHour').value);p.set('focusReminder_endHour',qs('focusReminder_endHour').value);p.set('focusReminder_intervalMinutes',qs('focusReminder_intervalMinutes').value);p.set('focusReminder_animation',qs('focusReminder_animation').value);let daysMask=0;for(let i=0;i<7;i++){if(qs('focusReminder_day'+i).checked)daysMask|=(1<<i)}p.set('focusReminder_daysMask',daysMask);post('/settings',p.toString()).then(loadSettings)}\n"
  "function updateDefaultLabel(){const hasUD=settings&&settings.hasUserDefaults;const btn=qs('resetBtn');if(btn)btn.textContent=hasUD?'Reset to my defaults':'Reset to factory defaults'}\n"
  "async function saveAsDefault(){await fetch('/settings/saveDefault',{method:'POST'});const m=qs('defaultSavedMsg');m.style.display='inline';setTimeout(()=>m.style.display='none',3000);await loadSettings()}\n"
  "async function resetToDefaults(){const hasUD=settings&&settings.hasUserDefaults;if(!confirm(hasUD?'Reset ALL settings to your saved defaults?':'Reset ALL settings to factory defaults?'))return;await fetch('/settings/reset',{method:'POST'});loadSettings()}\n"
  "function styleParams(p){p.set('palette',qs('animationPalette').value);p.set('speed',qs('animationSpeed').value);p.set('brightness',qs('animationBrightness').value);p.set('trail',qs('trailLength').value);p.set('reminderPalette',qs('reminderPalette').value);return p}\n"
  "function previewAnim(type,modeId){const mode=qs(modeId).value;if(!mode||mode==='0')return;const p=styleParams(new URLSearchParams());p.set('type',type);p.set('mode',mode);fetch('/previewAnimation',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()})}\n"
  "function previewStyleAnim(){const slotType=qs('stylePreviewType').value;const modeIds={quarter:'quarterAnimation',halfhour:'halfHourAnimation',hour:'hourAnimation',reminder:'focusReminder_animation'};const modeId=modeIds[slotType];const mode=modeId?qs(modeId).value:'0';if(!mode||mode==='0')return;const p=styleParams(new URLSearchParams());p.set('type',slotType);p.set('mode',mode);fetch('/previewAnimation',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()})}\n"
  "async function saveAnimStyle(){const p=new URLSearchParams();p.set('animationPalette',qs('animationPalette').value);p.set('animationSpeed',qs('animationSpeed').value);p.set('animationBrightness',qs('animationBrightness').value);p.set('trailLength',qs('trailLength').value);p.set('reminderPalette',qs('reminderPalette').value);p.set('silent','1');await fetch('/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()});loadSettings()}\n"
  "async function startDemo(){await fetch('/demo/start',{method:'POST'});qs('demoStartBtn').style.display='none';qs('demoStopBtn').style.display='inline-block';updateDemoStatus();demoStatusInterval=setInterval(updateDemoStatus,500)}\n"
  "async function stopDemo(){await fetch('/demo/stop',{method:'POST'});qs('demoStartBtn').style.display='inline-block';qs('demoStopBtn').style.display='none';qs('demoStatus').textContent='Idle';clearInterval(demoStatusInterval)}\n"
  "async function updateDemoStatus(){try{const r=await fetch('/demo/status');const data=await r.json();if(data.active){const elapsed=data.elapsed_ms,dur=data.step_duration_ms,pct=Math.round(100*elapsed/dur);qs('demoStatus').textContent=`Step ${data.step+1}/8: ${data.subtitle} (${pct}%)`}else{qs('demoStatus').textContent='Idle'}}catch(e){}}\n"
  "let demoStatusInterval=0;\n"
  "const CONTRAST_PRESETS={current:{outerRingBrightness:90,middleFaceScale:55,innerFaceScale:55,middleFaceColor:'#DC00B4',hoursColor:'#DC00B4',innerFaceColor:'#FF3C00',innerHourColor:'#DC00B4',hoursLevel:255,innerHourLevel:255},natural:{outerRingBrightness:82,middleFaceScale:64,innerFaceScale:72,middleFaceColor:'#123A48',hoursColor:'#FFD166',innerFaceColor:'#382B50',innerHourColor:'#FFD166',hoursLevel:238,innerHourLevel:238},subtle:{outerRingBrightness:65,middleFaceScale:38,innerFaceScale:58,middleFaceColor:'#183242',hoursColor:'#E8C77B',innerFaceColor:'#352B4C',innerHourColor:'#E8C77B',hoursLevel:215,innerHourLevel:215},bloom:{outerRingBrightness:35,middleFaceScale:75,innerFaceScale:115,middleFaceColor:'#174454',hoursColor:'#FFD166',innerFaceColor:'#40305D',innerHourColor:'#FFD166',hoursLevel:235,innerHourLevel:235},crisp:{outerRingBrightness:80,middleFaceScale:10,innerFaceScale:15,middleFaceColor:'#0F2F3E',hoursColor:'#FFD166',innerFaceColor:'#2F2448',innerHourColor:'#FFD166',hoursLevel:255,innerHourLevel:255},vivid:{outerRingBrightness:95,middleFaceScale:88,innerFaceScale:110,middleFaceColor:'#006D77',hoursColor:'#FFE66D',innerFaceColor:'#573280',innerHourColor:'#FFE66D',hoursLevel:245,innerHourLevel:245},solstice:{outerRingBrightness:82,middleFaceScale:58,innerFaceScale:65,middleFaceColor:'#0A2B10',hoursColor:'#FFD44A',innerFaceColor:'#071A0A',innerHourColor:'#FFD44A',hoursLevel:240,innerHourLevel:240},deepspace:{outerRingBrightness:72,middleFaceScale:52,innerFaceScale:58,middleFaceColor:'#0A1840',hoursColor:'#2080FF',innerFaceColor:'#081030',innerHourColor:'#2080FF',hoursLevel:250,innerHourLevel:250},lava:{outerRingBrightness:80,middleFaceScale:60,innerFaceScale:68,middleFaceColor:'#200800',hoursColor:'#FF5800',innerFaceColor:'#180600',innerHourColor:'#FF5800',hoursLevel:245,innerHourLevel:245},frostbite:{outerRingBrightness:80,middleFaceScale:50,innerFaceScale:56,middleFaceColor:'#0C2040',hoursColor:'#C0F0FF',innerFaceColor:'#081428',innerHourColor:'#C0F0FF',hoursLevel:248,innerHourLevel:248},neongarden:{outerRingBrightness:78,middleFaceScale:55,innerFaceScale:62,middleFaceColor:'#150A28',hoursColor:'#80FF00',innerFaceColor:'#0E0618',innerHourColor:'#80FF00',hoursLevel:242,innerHourLevel:242}};\n"
  "function applyContrastPreset(val){if(!val)return;const p=CONTRAST_PRESETS[val];if(!p)return;for(const[k,v]of Object.entries(p))qs(k).value=v;saveSettings()}\n"
  "function updateContrastPreset(){let matched='';for(const[k,p]of Object.entries(CONTRAST_PRESETS)){let ok=true;for(const[f,v]of Object.entries(p)){const el=qs(f);if(!el)continue;ok=ok&&(el.type==='color'?el.value.toUpperCase()===String(v).toUpperCase():Number(el.value)===Number(v))}if(ok){matched=k;break}}qs('contrastPreset').value=matched}\n"
  "makeClock();loadSettings();refresh();loadNet();loadVersion();setInterval(refresh,1000);setInterval(draw,90);\n"
  "function exportTheme(){const fields=['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','middleFaceColor','hoursColor','innerFaceColor','innerHourColor','centerColor'];const levels=['outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel'];const anim=['animationPalette','animationSpeed','animationBrightness','trailLength','reminderPalette'];const theme={version:3,outerRingBrightness:Number(qs('outerRingBrightness').value)};for(const k of fields)theme[k]=qs(k).value;for(const k of levels)theme[k]=Number(qs(k).value);for(const k of anim)theme[k]=Number(qs(k).value);const blob=new Blob([JSON.stringify(theme,null,2)],{type:'application/json'});const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download='chronobloom-theme.json';a.click();URL.revokeObjectURL(a.href);qs('themeStatus').textContent='Theme exported.'}\n"
  "function importTheme(file){if(!file)return;const reader=new FileReader();reader.onload=e=>{try{const theme=JSON.parse(e.target.result);const fields=['outerMarkerColor','outerFillerColor','secondsColor','minutesColor','middleFaceColor','hoursColor','innerFaceColor','innerHourColor','centerColor'];const levels=['outerMarkerLevel','outerFillerLevel','secondsLevel','minutesLevel','middleFaceScale','hoursLevel','innerFaceScale','innerHourLevel','centerLevel'];const anim=['animationPalette','animationSpeed','animationBrightness','trailLength','reminderPalette'];for(const k of fields)if(theme[k])qs(k).value=theme[k];for(const k of levels)if(theme[k]!=null)qs(k).value=theme[k];for(const k of anim)if(theme[k]!=null)qs(k).value=theme[k];if(theme.outerRingBrightness!=null)qs('outerRingBrightness').value=theme.outerRingBrightness;draw();saveSettings();saveAnimStyle();qs('themeStatus').textContent='Theme imported and saved.'}catch(err){qs('themeStatus').textContent='Import failed: invalid JSON.'}};reader.readAsText(file)}\n"
  "async function exportBackup(){const r=await fetch('/settings');const s=await r.json();delete s.hasUserDefaults;const blob=new Blob([JSON.stringify(s,null,2)],{type:'application/json'});const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download='chronobloom-backup.json';a.click();URL.revokeObjectURL(a.href);qs('backupStatus').textContent='Full backup exported.'}\n"
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
  "<title>Firmware Update - ESP32 Ring Clock</title>\n"
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

        if (data.active) {
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
