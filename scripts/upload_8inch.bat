@echo off
REM Build and OTA-upload ChronoBloom firmware to the 8" clock.
REM Usage: double-click, or run from any directory.

setlocal
set REPO_ROOT=%~dp0..
set ENV_NAME=esp32c3_v3_8inch
set HOSTNAME=esp32c3-v3-8inch.local
set FIRMWARE_BIN=%REPO_ROOT%\.pio\build\%ENV_NAME%\firmware.bin

echo ==========================================================
echo  ChronoBloom - 8inch build + OTA upload
echo ==========================================================

cd /d "%REPO_ROOT%"

echo.
echo [1/2] Building %ENV_NAME% ...
pio run -e %ENV_NAME%
if errorlevel 1 (
    echo.
    echo BUILD FAILED - aborting upload.
    pause
    exit /b 1
)

echo.
echo [2/2] Uploading to http://%HOSTNAME%/update ...
curl -f -F "image=@%FIRMWARE_BIN%" http://%HOSTNAME%/update
if errorlevel 1 (
    echo.
    echo OTA UPLOAD FAILED.
    echo   - Is the clock powered on and connected to WiFi?
    echo   - Try browsing to http://%HOSTNAME%/update manually.
    echo   - USB fallback: pio run -e %ENV_NAME% -t upload --upload-protocol esptool --upload-port COM6
    pause
    exit /b 1
)

echo.
echo Done. 8inch clock updated.
pause
