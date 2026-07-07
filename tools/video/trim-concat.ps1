<#
.SYNOPSIS
    Trim multiple source clips to in/out ranges and concatenate them into a
    single 1080p30 master file for the ChronoBloom demo video.

.DESCRIPTION
    Takes a list of input clip paths, each paired with a start (-in) and end
    (-out) timestamp, trims each clip to that range, and concatenates the
    results in the order given. Uses stream copy (-c copy) when the cut
    points and codecs allow lossless trimming; falls back to a single
    re-encode pass (libx264, yuv420p, 1080p30) when -Reencode is passed or
    when mixed input formats/resolutions make stream copy concat unsafe.

    NOTE ON FRAME RATE: this project's OBS capture / firmware timing note
    (docs/publish/DEMO_MODE.md) flags that a 12.5 Hz LED animation beat can
    alias visibly against a 30fps capture unless OBS is set to 25fps or the
    firmware step timer is set to a 33.3ms tick. This script does not assume
    which mitigation was used upstream — it only standardizes the OUTPUT to
    1080p30 for final delivery. If your source footage still shows beating,
    fix it at the capture/firmware stage, not here.

.PARAMETER Clips
    Array of input clip file paths, in the order they should appear in the
    output. Must be the same length as -InPoints and -OutPoints.

.PARAMETER InPoints
    Array of start timestamps (HH:MM:SS or HH:MM:SS.ms), one per clip, same
    order as -Clips.

.PARAMETER OutPoints
    Array of end timestamps (HH:MM:SS or HH:MM:SS.ms), one per clip, same
    order as -Clips.

.PARAMETER OutputPath
    Path to the concatenated master output file (.mp4).

.PARAMETER Reencode
    Force a single re-encode pass instead of attempting -c copy trimming.
    Use this if stream-copy concat produces glitches (common when clips have
    different codecs, resolutions, or frame rates).

.PARAMETER Captions
    Path to an .srt file to burn in as hardcoded subtitles on the final
    output (e.g. docs/publish/chronobloom_demo_reel.srt from the Demo Reel
    Designer export). Use this if your editor doesn't support SRT import
    (Clipchamp does not) — this does the burn-in with ffmpeg instead, so no
    GUI caption tool is needed at all. Forces a re-encode of the final
    output regardless of -Reencode, since subtitle burn-in requires it.

.PARAMETER WhatIf
    Print the ffmpeg commands that would run without executing them.

.EXAMPLE
    .\trim-concat.ps1 -Clips "clip1.mp4","clip2.mp4" -InPoints "00:00:02","00:00:00" -OutPoints "00:01:38","00:00:45" -OutputPath "master.mp4"

.EXAMPLE
    .\trim-concat.ps1 -Clips "a.mov","b.mov","c.mov" -InPoints "0:00:05","0:00:00","0:00:10" -OutPoints "0:01:00","0:00:30","0:00:50" -OutputPath "D:\demo\master.mp4" -Reencode

.EXAMPLE
    .\trim-concat.ps1 -Clips "a.mp4" -InPoints "0:00:00" -OutPoints "0:00:10" -OutputPath "master.mp4" -WhatIf

.EXAMPLE
    .\trim-concat.ps1 -Clips "a.mp4" -InPoints "0:00:00" -OutPoints "0:01:50" -OutputPath "master.mp4" -Captions "chronobloom_demo_reel.srt"
#>

[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [Parameter(Mandatory = $true)]
    [string[]]$Clips,

    [Parameter(Mandatory = $true)]
    [string[]]$InPoints,

    [Parameter(Mandatory = $true)]
    [string[]]$OutPoints,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath,

    [switch]$Reencode,

    [string]$Captions
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

function ConvertTo-FfmpegSubtitlesPath {
    # ffmpeg's subtitles filter parses ':' and '\' specially even inside
    # quotes, so a bare Windows path like C:\foo\bar.srt has to be escaped
    # (forward slashes, drive-letter colon escaped) before use in -vf.
    param([string]$Path)
    $resolved = (Resolve-Path -LiteralPath $Path).Path
    $escaped = $resolved -replace '\\', '/'
    $escaped = $escaped -replace ':', '\:'
    return $escaped
}

Assert-Ffmpeg

if ($Clips.Count -ne $InPoints.Count -or $Clips.Count -ne $OutPoints.Count) {
    throw "Clips, InPoints, and OutPoints must all have the same number of entries. Got $($Clips.Count) clips, $($InPoints.Count) in-points, $($OutPoints.Count) out-points."
}

if ($Captions -and -not (Test-Path -LiteralPath $Captions)) {
    throw "Captions file not found: $Captions"
}

for ($i = 0; $i -lt $Clips.Count; $i++) {
    if (-not (Test-Path -LiteralPath $Clips[$i])) {
        throw "Input clip not found: $($Clips[$i])"
    }
    Assert-TimestampFormat -Value $InPoints[$i] -Label "InPoints[$i]"
    Assert-TimestampFormat -Value $OutPoints[$i] -Label "OutPoints[$i]"
}

$outDir = Split-Path -Path $OutputPath -Parent
if ($outDir -and -not (Test-Path -LiteralPath $outDir)) {
    throw "Output directory does not exist: $outDir"
}

$workDir = Join-Path ([System.IO.Path]::GetTempPath()) ("trim-concat-" + [guid]::NewGuid().ToString("N"))

if (-not $PSCmdlet.ShouldProcess($OutputPath, "Trim $($Clips.Count) clip(s) and concatenate")) {
    Write-Host "WhatIf: would create working dir $workDir"
}

New-Item -ItemType Directory -Path $workDir -Force | Out-Null

try {
    $trimmedClips = @()

    for ($i = 0; $i -lt $Clips.Count; $i++) {
        $srcClip   = $Clips[$i]
        $inPoint   = $InPoints[$i]
        $outPoint  = $OutPoints[$i]
        $trimmedFile = Join-Path $workDir ("clip{0:D3}.mp4" -f $i)

        if ($Reencode) {
            $trimArgs = @(
                "-y", "-hide_banner", "-loglevel", "error",
                "-i", $srcClip,
                "-ss", $inPoint, "-to", $outPoint,
                "-vf", "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2",
                "-r", "30",
                "-c:v", "libx264", "-pix_fmt", "yuv420p", "-preset", "medium", "-crf", "18",
                "-c:a", "aac", "-b:a", "192k",
                $trimmedFile
            )
        } else {
            $trimArgs = @(
                "-y", "-hide_banner", "-loglevel", "error",
                "-ss", $inPoint, "-to", $outPoint,
                "-i", $srcClip,
                "-c", "copy",
                $trimmedFile
            )
        }

        Write-Host "Trimming clip $($i + 1)/$($Clips.Count): $srcClip [$inPoint - $outPoint]"

        if ($PSCmdlet.ShouldProcess($trimmedFile, "ffmpeg $($trimArgs -join ' ')")) {
            & ffmpeg @trimArgs
            if ($LASTEXITCODE -ne 0) {
                throw "ffmpeg failed trimming clip $($i + 1): $srcClip"
            }
        } else {
            Write-Host "WhatIf: ffmpeg $($trimArgs -join ' ')"
        }

        $trimmedClips += $trimmedFile
    }

    $concatListPath = Join-Path $workDir "concat_list.txt"
    $concatLines = $trimmedClips | ForEach-Object {
        "file '$($_ -replace "'", "'\''")'"
    }

    if ($PSCmdlet.ShouldProcess($concatListPath, "Write concat list")) {
        Set-Content -Path $concatListPath -Value $concatLines -Encoding UTF8
    } else {
        Write-Host "WhatIf: would write concat list to $concatListPath with $($concatLines.Count) entries"
    }

    $concatTarget = if ($Captions) { Join-Path $workDir "concat_master.mp4" } else { $OutputPath }

    $concatArgs = @(
        "-y", "-hide_banner", "-loglevel", "error",
        "-f", "concat", "-safe", "0",
        "-i", $concatListPath,
        "-c", "copy",
        $concatTarget
    )

    Write-Host "Concatenating $($trimmedClips.Count) trimmed clip(s) into $concatTarget"

    if ($PSCmdlet.ShouldProcess($concatTarget, "ffmpeg concat $($concatArgs -join ' ')")) {
        & ffmpeg @concatArgs
        if ($LASTEXITCODE -ne 0) {
            Write-Warning "Stream-copy concat failed (likely mismatched codecs/params across trimmed clips). Retry with -Reencode."
            throw "ffmpeg concat failed."
        }
        Write-Host "Concatenated: $concatTarget"
    } else {
        Write-Host "WhatIf: ffmpeg $($concatArgs -join ' ')"
    }

    if ($Captions) {
        $subtitlesPath = ConvertTo-FfmpegSubtitlesPath -Path $Captions
        $burnArgs = @(
            "-y", "-hide_banner", "-loglevel", "error",
            "-i", $concatTarget,
            "-vf", "subtitles='$subtitlesPath'",
            "-c:v", "libx264", "-pix_fmt", "yuv420p", "-preset", "medium", "-crf", "18",
            "-c:a", "copy",
            $OutputPath
        )

        Write-Host "Burning in captions from $Captions -> $OutputPath"

        if ($PSCmdlet.ShouldProcess($OutputPath, "ffmpeg subtitles burn-in $($burnArgs -join ' ')")) {
            & ffmpeg @burnArgs
            if ($LASTEXITCODE -ne 0) {
                throw "ffmpeg failed burning in captions."
            }
            Write-Host "Master written (captioned): $OutputPath"
        } else {
            Write-Host "WhatIf: ffmpeg $($burnArgs -join ' ')"
        }
    } else {
        Write-Host "Master written: $OutputPath"
    }
}
finally {
    if (Test-Path -LiteralPath $workDir) {
        Remove-Item -LiteralPath $workDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}
