param(
    [Parameter(Mandatory = $true)]
    [string]$TitleUpdatePackage,
    [Parameter(Mandatory = $true)]
    [string]$DlcRoot
)

$ErrorActionPreference = 'Stop'
if (!(Test-Path -LiteralPath $TitleUpdatePackage -PathType Leaf)) {
    throw "Title Update #3 package not found: $TitleUpdatePackage"
}
if (!(Test-Path -LiteralPath $DlcRoot -PathType Container)) {
    throw "XL DLC directory not found: $DlcRoot"
}
# Resolve relative input paths before changing to the project directory.
$TitleUpdatePackage = (Resolve-Path -LiteralPath $TitleUpdatePackage).ProviderPath
$DlcRoot = (Resolve-Path -LiteralPath $DlcRoot).ProviderPath
$sdkRoot = Split-Path -Parent $PSScriptRoot
$vsRoot = 'C:\Program Files\Microsoft Visual Studio\18\Community'
& "$vsRoot\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
$env:PATH = "$vsRoot\VC\Tools\Llvm\x64\bin;" + $env:PATH
Push-Location $PSScriptRoot
try {
    cmake --preset win-amd64-release "-DREXSDK_DIR=$sdkRoot" -DSW2_PGO=AUTO -DSW2_PREPARE_GAME=ON "-DSW2_DLC_ROOT=$DlcRoot" "-DSW2_TU_PACKAGE=$TitleUpdatePackage"
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
    # Codegen creates the source lists and XL target on a fresh checkout.
    cmake --build --preset win-amd64-release --target samurai_warriors_2_codegen -j 6
    if ($LASTEXITCODE -ne 0) { throw 'Code generation failed.' }
    cmake --preset win-amd64-release
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration after codegen failed.' }
    cmake --build --preset win-amd64-release -j 6
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
} finally {
    Pop-Location
}
