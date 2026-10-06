param(
    [ValidateRange(1, 256)]
    [int]$Jobs = 6
)

$ErrorActionPreference = 'Stop'
$buildDir = Join-Path $PSScriptRoot 'out\build\win-amd64-release'
foreach ($file in @(
    (Join-Path $buildDir 'CMakeCache.txt'),
    (Join-Path $PSScriptRoot 'generated\default\sources.cmake'),
    (Join-Path $PSScriptRoot 'generated\default\dll_targets.cmake'),
    (Join-Path $PSScriptRoot 'generated\sw2xl_us\sources.cmake')
)) {
    if (!(Test-Path -LiteralPath $file -PathType Leaf)) {
        throw "Initial build is missing: $file. Run build.ps1 with the Title Update and DLC paths first."
    }
}

$vsRoot = 'C:\Program Files\Microsoft Visual Studio\18\Community'
& "$vsRoot\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
$env:PATH = "$vsRoot\VC\Tools\Llvm\x64\bin;" + $env:PATH
Push-Location $PSScriptRoot
try {
    # Keep the SDK, PGO mode and profile already stored in CMakeCache.txt.
    cmake --preset win-amd64-release -DSW2_PREPARE_GAME=OFF
    if ($LASTEXITCODE -ne 0) { throw 'Rebuild configuration failed.' }
    cmake --build --preset win-amd64-release -j $Jobs
    if ($LASTEXITCODE -ne 0) { throw 'Rebuild failed.' }
} finally {
    Pop-Location
}
