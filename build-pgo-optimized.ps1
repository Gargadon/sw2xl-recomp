$ErrorActionPreference = 'Stop'
$sdkRoot = Split-Path -Parent $PSScriptRoot
$vsRoot = 'C:\Program Files\Microsoft Visual Studio\18\Community'
$profileDir = Join-Path $PSScriptRoot 'pgo\raw'
$mergedProfile = Join-Path $PSScriptRoot 'pgo\sw2.profdata'

& "$vsRoot\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
$env:PATH = "$vsRoot\VC\Tools\Llvm\x64\bin;" + $env:PATH
$rawProfiles = @(Get-ChildItem -LiteralPath $profileDir -Filter '*.profraw' -File -ErrorAction SilentlyContinue)
if (!$rawProfiles.Count) {
    throw 'No training profiles found. Run build-pgo-instrumented.ps1 and run-pgo-training.ps1 first.'
}
New-Item -ItemType Directory -Path (Split-Path -Parent $mergedProfile) -Force | Out-Null
& llvm-profdata merge -o $mergedProfile @($rawProfiles.FullName)
if ($LASTEXITCODE -ne 0) { throw 'llvm-profdata could not merge the training profiles.' }
if (!(Test-Path -LiteralPath $mergedProfile)) {
    throw "llvm-profdata did not create the merged profile: $mergedProfile"
}

Push-Location $PSScriptRoot
try {
    cmake --preset win-amd64-release "-DREXSDK_DIR=$sdkRoot" -DSW2_PGO=USE -DSW2_PREPARE_GAME=ON "-DSW2_PGO_PROFILE=$mergedProfile"
    if ($LASTEXITCODE -ne 0) { throw 'PGO optimized configuration failed.' }
    cmake --build --preset win-amd64-release --target samurai_warriors_2_codegen -j 6
    if ($LASTEXITCODE -ne 0) { throw 'Code generation failed.' }
    cmake --preset win-amd64-release
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration after codegen failed.' }
    cmake --build --preset win-amd64-release -j 6
    if ($LASTEXITCODE -ne 0) { throw 'PGO optimized build failed.' }
} finally {
    Pop-Location
}
Write-Host 'PGO optimized build ready. Launch it with .\run.ps1'
