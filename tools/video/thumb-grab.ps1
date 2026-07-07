<#
.SYNOPSIS
    Extract candidate thumbnail frames from a video at given timestamps as
    PNG stills.

.DESCRIPTION
    Given an input video and a list of timestamps, extracts one PNG frame
    per timestamp for thumbnail selection (YouTube/Reddit/etc.). Each frame
    is saved as <OutputPrefix>_NN.png in -OutputDir, in the order the
    timestamps were given, decoded at source resolution.

    NOTE ON FRAME RATE: LED animation frames can visibly beat at 12.5 Hz
    against 30fps capture unless OBS was set to 25fps or the firmware step
    timer to a 33.3ms tick (see docs/publish/DEMO_MODE.md). If a grabbed
    still looks mid-beat/blurred on the ring, nudge the timestamp by a
    fraction of a second and re-grab — this script has no opinion on which
    upstream mitigation was used.

.PARAMETER InputPath
    Path to the source video to grab frames from.

.PARAMETER Timestamps
    Array of timestamps (HH:MM:SS or HH:MM:SS.ms) to extract, one PNG per
    entry.

.PARAMETER OutputDir
    Directory to write the extracted PNG frames into. Created if missing.

.PARAMETER OutputPrefix
    Filename prefix for extracted frames. Defaults to "thumb".

.PARAMETER WhatIf
    Print the ffmpeg commands that would run without executing them.

.EXAMPLE
    .\thumb-grab.ps1 -InputPath "master.mp4" -Timestamps "00:00:05","00:00:42","00:01:10" -OutputDir "thumbs"

.EXAMPLE
    .\thumb-grab.ps1 -InputPath "D:\demo\master.mp4" -Timestamps "0:00:20" -OutputDir "D:\demo\thumbs" -OutputPrefix "candidate" -WhatIf
#>

[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,

    [Parameter(Mandatory = $true)]
    [string[]]$Timestamps,

    [Parameter(Mandatory = $true)]
    [string]$OutputDir,

    [string]$OutputPrefix = "thumb"
)

$ErrorActionPreference = "Stop"

function Assert-Ffmpeg {
    $ffmpeg = Get-Command ffmpeg -ErrorAction SilentlyContinue
    if (-not $ffmpeg) {
        throw "ffmpeg not found on PATH. Install ffmpeg and ensure it's callable as 'ffmpeg' before running this script."
    }
}

function Assert-TimestampFormat {
    param([string]$Value, [string]$Label)
    if ($Value -notmatch '^\d{1,2}:\d{2}:\d{2}(\.\d+)?$') {
        throw "Invalid timestamp for $Label`: '$Value'. Expected HH:MM:SS or HH:MM:SS.ms."
    }
}

Assert-Ffmpeg

if (-not (Test-Path -LiteralPath $InputPath)) {
    throw "Input video not found: $InputPath"
}

if ($Timestamps.Count -eq 0) {
    throw "-Timestamps must contain at least one timestamp."
}

for ($i = 0; $i -lt $Timestamps.Count; $i++) {
    Assert-TimestampFormat -Value $Timestamps[$i] -Label "Timestamps[$i]"
}

if (-not (Test-Path -LiteralPath $OutputDir)) {
    if ($PSCmdlet.ShouldProcess($OutputDir, "Create output directory")) {
        New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    } else {
        Write-Host "WhatIf: would create output directory $OutputDir"
    }
}

$padWidth = [Math]::Max(2, ($Timestamps.Count - 1).ToString().Length)

for ($i = 0; $i -lt $Timestamps.Count; $i++) {
    $ts = $Timestamps[$i]
    $index = $i.ToString().PadLeft($padWidth, '0')
    $outFile = Join-Path $OutputDir ("{0}_{1}.png" -f $OutputPrefix, $index)

    $ffmpegArgs = @(
        "-y", "-hide_banner", "-loglevel", "error",
        "-ss", $ts,
        "-i", $InputPath,
        "-frames:v", "1",
        $outFile
    )

    Write-Host "Grabbing frame $($i + 1)/$($Timestamps.Count) at $ts -> $outFile"

    if ($PSCmdlet.ShouldProcess($outFile, "ffmpeg $($ffmpegArgs -join ' ')")) {
        & ffmpeg @ffmpegArgs
        if ($LASTEXITCODE -ne 0) {
            throw "ffmpeg failed grabbing frame at $ts."
        }
    } else {
        Write-Host "WhatIf: ffmpeg $($ffmpegArgs -join ' ')"
    }
}

Write-Host "Done. $($Timestamps.Count) candidate frame(s) written to $OutputDir"
