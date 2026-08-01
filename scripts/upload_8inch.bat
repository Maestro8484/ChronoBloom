@echo off
REM Build and send new firmware to the 8 inch ChronoBloom, over WiFi.
REM Double-click this. No arguments needed - it finds the clock for you.
REM
REM It shows you WHICH board it found and asks before writing anything. If more
REM than one 8 inch board is on the network, it lists them and lets you pick.
REM
REM To skip the search and say where the clock is:
REM   upload_8inch.bat <address>
REM   upload_8inch.bat myclock.local
REM
REM It deliberately does not default to esp32c3-v3-8inch.local. Every board built
REM from the 8 inch recipe answers to that name, so once you own two, the name
REM belongs to whichever one replied first - and you can flash the wrong board.

setlocal
if "%~1"=="" (
  powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0upload_8inch.ps1"
) else (
  powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0upload_8inch.ps1" -Target %1
)
echo(
pause
