# Build and send new firmware to an 8 inch ChronoBloom, over WiFi.
# Launched by upload_8inch.bat (double-click that, not this).
#
# It finds your clock on the network by itself. If you would rather say where it
# is, pass an address or a name:
#   upload_8inch.bat <address>
#   upload_8inch.bat myclock.local
#
# WHY IT DOES NOT JUST USE esp32c3-v3-8inch.local:
#   Every board built from the 8 inch recipe claims that same name, so as soon as
#   you own two, the name belongs to whichever board answered first. Sending
#   firmware to a name you do not control means flashing a board you did not mean
#   to flash. This script asks each board what it is and shows you, before
#   anything is written.

param(
    [string]$Target,
    [switch]$CheckOnly          # inspect and stop; build nothing, write nothing
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$envName = 'esp32c3_v3_8inch'
$bin = Join-Path $repo ".pio\build\$envName\firmware.bin"

function Get-Diag([string]$addr, [int]$timeoutSec = 6) {
    try { return Invoke-RestMethod -Uri "http://$addr/diag" -TimeoutSec $timeoutSec } catch { return $null }
}

function Find-Clocks {
    # One process, 254 concurrent requests. Do NOT use Start-Job per address:
    # that spawns 254 PowerShell processes and effectively never finishes.
    $local = (Get-NetIPAddress -AddressFamily IPv4 |
              Where-Object { $_.PrefixOrigin -in 'Dhcp','Manual' -and $_.IPAddress -notlike '169.254.*' -and $_.IPAddress -ne '127.0.0.1' } |
              Sort-Object -Property SkipAsSource, InterfaceMetric |
              Select-Object -First 1).IPAddress
    if (-not $local) { return @() }
    $prefix = ($local -split '\.')[0..2] -join '.'
    Write-Host ("      looking for clocks on {0}.1 - {0}.254 ..." -f $prefix) -ForegroundColor DarkGray

    Add-Type -AssemblyName System.Net.Http
    $client = New-Object System.Net.Http.HttpClient
    $client.Timeout = [TimeSpan]::FromSeconds(4)
    $tasks = @{}
    foreach ($n in 1..254) { $tasks["$prefix.$n"] = $client.GetStringAsync("http://$prefix.$n/diag") }
    try { [Threading.Tasks.Task]::WaitAll([Threading.Tasks.Task[]]@($tasks.Values), 12000) } catch { }

    $out = @()
    foreach ($ip in $tasks.Keys) {
        if ($tasks[$ip].Status -ne 'RanToCompletion') { continue }
        try {
            $o = $tasks[$ip].Result | ConvertFrom-Json
            if ($o.firmware_version) { $o | Add-Member -NotePropertyName ip -NotePropertyValue $ip -Force; $out += $o }
        } catch { }
    }
    $client.Dispose()
    return $out
}

function Describe($d) {
    switch ($d.clock_pixel_count) {
        96      { '15 inch clock' }
        97      { '8 inch clock' }
        default { "8 inch clock, $($d.clock_pixel_count)-LED chain" }
    }
}

Write-Host ''
Write-Host '==========================================================' -ForegroundColor Cyan
Write-Host '  ChronoBloom - 8 inch firmware update'                     -ForegroundColor Cyan
Write-Host '==========================================================' -ForegroundColor Cyan
Write-Host ''
Write-Host '[1/3] Finding the clock ...'

$d = $null
if ($Target) {
    $d = Get-Diag $Target
    if (-not $d) {
        Write-Host ''
        Write-Host ("  Nothing answered at {0}." -f $Target) -ForegroundColor Red
        Write-Host '  Is the clock plugged in and on your WiFi?' -ForegroundColor Red
        Write-Host '  Run upload_8inch.bat with no address and it will search for you.' -ForegroundColor Red
        exit 1
    }
} else {
    $all = @(Find-Clocks)
    $eights = @($all | Where-Object { $_.clock_pixel_count -ne 96 })
    if ($eights.Count -eq 0) {
        Write-Host ''
        Write-Host '  No 8 inch clock found on this network.' -ForegroundColor Red
        if ($all.Count) { Write-Host ('  (found {0} other ChronoBloom board(s), but no 8 inch)' -f $all.Count) -ForegroundColor DarkGray }
        Write-Host '  Check it is powered and on the same WiFi as this PC, or pass its' -ForegroundColor Red
        Write-Host '  address:  upload_8inch.bat <address>' -ForegroundColor Red
        exit 1
    }
    if ($eights.Count -eq 1) {
        $d = $eights[0]
        $Target = $d.ip
    } else {
        Write-Host ''
        Write-Host ('  Found {0} eight-inch boards. Which one?' -f $eights.Count) -ForegroundColor Yellow
        for ($i = 0; $i -lt $eights.Count; $i++) {
            $e = $eights[$i]
            $note = if ($e.lux -lt 0) { '  (no light sensor - looks like a bare board)' } else { '' }
            Write-Host ('    [{0}]  {1}   firmware {2}{3}' -f ($i+1), $e.ip, $e.firmware_version, $note)
        }
        Write-Host ''
        $pick = Read-Host '  Number'
        $idx = 0
        if (-not [int]::TryParse($pick, [ref]$idx) -or $idx -lt 1 -or $idx -gt $eights.Count) {
            Write-Host '  Not a valid choice. Nothing written.' -ForegroundColor DarkGray; exit 0
        }
        $d = $eights[$idx-1]
        $Target = $d.ip
    }
}

Write-Host ''
Write-Host ('      address    {0}' -f $Target) -ForegroundColor White
Write-Host ('      board      {0}' -f (Describe $d)) -ForegroundColor Cyan
Write-Host ('      firmware   {0}' -f $d.firmware_version)
Write-Host ('      wifi       {0}' -f $d.wifi_ssid)
Write-Host ('      awake for  {0}' -f ([TimeSpan]::FromSeconds($d.uptime_sec).ToString('d\d\ hh\h\ mm\m')))
if ($d.lux -lt 0) { Write-Host '      no light sensor fitted - this looks like a bare test board' -ForegroundColor DarkGray }
Write-Host ''

if ($d.clock_pixel_count -eq 96) {
    Write-Host '  REFUSING: that is the 15 inch. Use upload_15inch.bat instead.' -ForegroundColor Red
    exit 1
}
if ($d.clock_pixel_count -ne 97) {
    Write-Host ('  WARNING: this board reports a {0}-LED chain, but this PC builds 97.' -f $d.clock_pixel_count) -ForegroundColor Yellow
    Write-Host '  Firmware geometry and real wiring must match, or the whole clock face' -ForegroundColor Yellow
    Write-Host '  rotates. Only continue if you have already changed the wiring to suit.' -ForegroundColor Yellow
    Write-Host ''
}

if ($CheckOnly) { Write-Host '  (check only - nothing built, nothing written)' -ForegroundColor DarkGray; exit 0 }

$ok = Read-Host '  Send new firmware to this board? (y/N)'
if ($ok -notmatch '^[Yy]') { Write-Host '  Cancelled. Nothing written.' -ForegroundColor DarkGray; exit 0 }

Write-Host ''
Write-Host '[2/3] Building ...'
Push-Location $repo
try { & pio run -e $envName } finally { Pop-Location }
if ($LASTEXITCODE -ne 0) { Write-Host '  BUILD FAILED - nothing sent.' -ForegroundColor Red; exit 1 }
if (-not (Test-Path $bin)) { Write-Host "  Built, but $bin is missing." -ForegroundColor Red; exit 1 }

Write-Host ''
Write-Host ('[3/3] Sending to http://{0}/update ...' -f $Target)
& curl.exe -f -F "image=@$bin" "http://$Target/update"
if ($LASTEXITCODE -ne 0) {
    Write-Host ''
    Write-Host '  UPLOAD FAILED. The clock may have dropped off WiFi mid-send.' -ForegroundColor Red
    Write-Host '  Re-run this and it will search again. If it never appears, flash over' -ForegroundColor Red
    Write-Host '  USB instead:  pio run -e esp32c3_v3_8inch -t upload' -ForegroundColor Red
    exit 1
}

Write-Host ''
Write-Host '  Sent. The clock reboots on its own - give it about 20 seconds.' -ForegroundColor Green
Write-Host '  Re-run this to confirm it is on the new version.' -ForegroundColor Green
