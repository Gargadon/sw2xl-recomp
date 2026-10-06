param(
    [string]$DlcRoot = 'D:\Samurai Warriors 2 XL',
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$RemainingArguments
)

$ErrorActionPreference = 'Stop'

# Also accept the GNU-style spelling used by the Bash launcher and common in
# the project documentation: .\install-dlc.ps1 --dlc-root 'D:\...'.
if ($DlcRoot -eq '--dlc-root') {
    if ($RemainingArguments.Count -ne 1) {
        throw 'Usage: .\install-dlc.ps1 [-DlcRoot <path>] or --dlc-root <path>'
    }
    $dlcRoot = $RemainingArguments[0]
} elseif ($RemainingArguments.Count -ne 0) {
    throw 'Usage: .\install-dlc.ps1 [-DlcRoot <path>] or --dlc-root <path>'
} else {
    $dlcRoot = $DlcRoot
}

if (!(Test-Path -LiteralPath $dlcRoot -PathType Container)) {
    throw "DLC directory not found: $dlcRoot"
}
$installer = Join-Path $PSScriptRoot 'out\build\win-amd64-release\sw2_install_dlc.exe'
if (!(Test-Path -LiteralPath $installer -PathType Leaf)) {
    throw 'DLC installer is missing. Run build.ps1 first.'
}
& $installer (Join-Path (Split-Path -Parent $PSScriptRoot) 'Samurai Warriors 2 (USA, Europe)') ([IO.Path]::GetFullPath($dlcRoot)) (Join-Path $PSScriptRoot 'userdata')
if ($LASTEXITCODE -ne 0) { throw "DLC installation failed with exit code $LASTEXITCODE." }
