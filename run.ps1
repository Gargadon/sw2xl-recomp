$ErrorActionPreference = 'Stop'
$sdkRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $PSScriptRoot 'out\build\win-amd64-release\samurai_warriors_2.exe'
if (!(Test-Path -LiteralPath $exe)) { throw 'Run build.ps1 first.' }
$env:PATH = "$sdkRoot\bin;" + $env:PATH
$launchArgs = @($args)
$localUserData = Join-Path $PSScriptRoot 'userdata'
$localCache = Join-Path $PSScriptRoot 'cache'
# Boolean flags use --no-fullscreen, not the two tokens --fullscreen false.
if (!($launchArgs | Where-Object { $_ -eq '--fullscreen' -or $_ -like '--fullscreen=*' -or $_ -eq '--no-fullscreen' })) {
    $launchArgs += '--no-fullscreen'
}
# ReXGlue's MnK driver is a synthetic Xbox 360 controller. It is merged with
# the physical XInput pad for player one, so either device can be used.
if (!($launchArgs | Where-Object { $_ -eq '--mnk_mode' -or $_ -like '--mnk_mode=*' -or $_ -eq '--no-mnk_mode' })) {
    $launchArgs += '--mnk_mode'
}
# Explicit command-line options take precedence over these play defaults.
$defaults = [ordered]@{
    input_backend = 'xinput'
    log_level = 'warn'
    render_target_path_d3d12 = 'rtv'
    user_data_root = $localUserData
    cache_root = $localCache
}
foreach ($option in $defaults.Keys) {
    if (!($launchArgs | Where-Object { $_ -eq "--$option" -or $_ -like "--$option=*" })) {
        $launchArgs += @("--$option", $defaults[$option])
    }
}
Push-Location (Split-Path -Parent $exe)
try {
    & $exe --game_data_root "$sdkRoot\Samurai Warriors 2 (USA, Europe)" --gpu_plugin xenos @launchArgs
    $gameExitCode = $LASTEXITCODE
} finally {
    Pop-Location
}
if ($gameExitCode -ne 0) {
    $logDir = Join-Path (Split-Path -Parent $exe) 'logs'
    $latestLog = Get-ChildItem -LiteralPath $logDir -Filter 'samurai_warriors_2_*.log' -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    $logPath = if ($latestLog) { $latestLog.FullName } else { $logDir }
    throw "Game exited with code $gameExitCode. See log: $logPath"
}
