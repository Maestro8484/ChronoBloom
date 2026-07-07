@echo off
REM Launch the standalone Demo Reel Designer (docs/publish/demo_reel_designer.html)
REM via a local static server, and open it in the default browser.
REM Usage: double-click, or run from any directory. Ctrl+C in the console window
REM (or just close it) to stop the server when done.

setlocal
set REPO_ROOT=%~dp0..
set PORT=8420
set DIR=%REPO_ROOT%\docs\publish
set URL=http://localhost:%PORT%/demo_reel_designer.html

echo ==========================================================
echo  ChronoBloom - Demo Reel Designer
echo ==========================================================
echo.
echo Serving %DIR%
echo   %URL%
echo.
echo Leave this window open while you use the tool.
echo Close this window (or Ctrl+C) to stop the server.
echo.

start "ChronoBloom Demo Reel Designer server" /min cmd /c python -m http.server %PORT% --directory "%DIR%"

timeout /t 2 /nobreak >nul
start "" "%URL%"

echo Server running in a separate minimized window titled
echo   "ChronoBloom Demo Reel Designer server"
echo Close that window when you're done to stop it.
echo.
pause
