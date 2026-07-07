<#
.SYNOPSIS
    Produce a 1080x1920 center-cropped vertical cut from the master demo
    video, capped at 60 seconds, for Reddit / YouTube Shorts / TikTok.

.DESCRIPTION
    Takes the horizontal 1080p master, extracts a start/end range, and
    center-crops it to a 9:16 1080x1920 frame (crop width matched to source
    height, centered horizontally, no letterboxing). Re-encodes with libx264
    since crop cannot be done with -c copy. Enforces a 60-second max
    duration to match Shorts/Reddit limits — if -End minus -Start exceeds
    60s, the script errors instead of silently truncating.

    NOTE ON FRAME RATE: as with trim-concat.ps1, this project's capture
    pipeline may have used OBS at 25fps or a firmware 33.3ms step tick to
    avoid a 12.5 Hz LED animation beat aliasing against 30fps. This script
    does not assume which was used — it outputs at 30fps for platform
    compatibility. If the source master already shows beating, that must be
    fixed upstream (re-capture or re-encode the master), not here.

.PARAMETER InputPath
    Path to the master video (e.g. output of trim-concat.ps1).

.PARAMETER StartSeconds
    Start offset within the master, in seconds (e.g. 12 or 12.5).

.PARAMETER DurationSeconds
    Clip length in seconds, starting at -StartSeconds. Must be 60 or less.

.PARAMETER OutputPath
    Path to the output vertical .mp4 file.

.PARAMETER WhatIf
    Print the ffmpeg command that would run without executing it.

.EXAMPLE
    .\vertical-cut.ps1 -InputPath "master.mp4" -StartSeconds 10 -DurationSeconds 45 -OutputPath "vertical-clip.mp4"

.EXAMPLE
    .\vertical-cut.ps1 -InputPath "D:\demo\master.mp4" -StartSeconds 80 -DurationSeconds 30 -OutputPath "D:\demo\shorts\clip1.mp4" -WhatIf
#>

[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,

    [Parameter(Mandatory = $true)]
    [double]$StartSeconds,

    [Parameter(Mandatory = $true)]
    [double]$DurationSeconds,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$ErrorActionPreference = "Stop"

function Assert-Ffmpeg {
    $ffmpeg = Get-Command ffmpeg -ErrorAction SilentlyContinue
    if (-not $ffmpeg) {
        throw "ffmpeg not found on PATH. Install ffmpeg and ensure it's callable as 'ffmpeg' before running this script."
    }
}

Assert-Ffmpeg

if (-not (Test-Path -LiteralPath $InputPath)) {
    throw "Input video not found: $InputPath"
}

if ($StartSeconds -lt 0) {
    throw "-StartSeconds must be 0 or greater."
}

if ($DurationSeconds -le 0) {
    throw "-DurationSeconds must be greater than 0."
}

if ($DurationSeconds -gt 60) {
    throw "Requested clip is $([math]::Round($DurationSeconds, 2))s, which exceeds the 60s max for Reddit/Shorts. Lower -DurationSeconds."
}

$endSeconds = $StartSeconds + $DurationSeconds

$outDir = Split-Path -Path $OutputPath -Parent
if ($outDir -and -not (Test-Path -LiteralPath $outDir)) {
    throw "Output directory does not exist: $outDir"
}

# Center-crop to 9:16: crop width = source height * (9/16), centered horizontally,
# then scale to the canonical 1080x1920 vertical frame.
$vf = "crop=ih*9/16:ih:(iw-ih*9/16)/2:0,scale=1080:1920,setsar=1"

$ffmpegArgs = @(
    "-y", "-hide_banner", "-loglevel", "error",
    "-ss", "$StartSeconds", "-to", "$endSeconds",
    "-i", $InputPath,
    "-vf", $vf,
    "-r", "30",
    "-c:v", "libx264", "-pix_fmt", "yuv420p", "-preset", "medium", "-crf", "18",
    "-c:a", "aac", "-b:a", "192k",
    $OutputPath
)

Write-Host "Vertical cut: $InputPath [${StartSeconds}s - ${endSeconds}s] ($([math]::Round($DurationSeconds, 2))s) -> $OutputPath"

if ($PSCmdlet.ShouldProcess($OutputPath, "ffmpeg $($ffmpegArgs -join ' ')")) {
    & ffmpeg @ffmpegArgs
    if ($LASTEXITCODE -ne 0) {
        throw "ffmpeg failed producing vertical cut."
    }
    Write-Host "Vertical clip written: $OutputPath"
} else {
    Write-Host "WhatIf: ffmpeg $($ffmpegArgs -join ' ')"
}
