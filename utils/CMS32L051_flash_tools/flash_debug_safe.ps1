param(
    [switch]$ValidateOnly
)

$ErrorActionPreference = "Stop"

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$openOcd = Join-Path $env:USERPROFILE "Downloads\openocd\xpack-openocd-0.12.0-7\bin\openocd.exe"
$firmware = Join-Path $repositoryRoot "build-debug-cms32\src\hardware\cms32-CMS32L051\targets\imaxB6-80W-cms32L051\imaxB6-80W-cms32L051_cms32-CMS32L051.bin"
$backupDirectory = Join-Path $repositoryRoot "build-debug-cms32\eeprom-backups"

if (-not (Test-Path -LiteralPath $openOcd -PathType Leaf)) {
    throw "OpenOCD not found: $openOcd"
}
if (-not (Test-Path -LiteralPath $firmware -PathType Leaf)) {
    throw "Debug firmware not found. Run the VS Code build task first: $firmware"
}

$firmwareBytes = [IO.File]::ReadAllBytes($firmware)
if ($firmwareBytes.Length -gt 0xFC00) {
    throw "Firmware is too large ($($firmwareBytes.Length) bytes); it would overlap EEPROM at 0xFC00"
}
if ($firmwareBytes.Length -lt 0xC3) {
    throw "Firmware is too small to contain the expected option bytes"
}

$optionBytes = "{0:X2} {1:X2} {2:X2}" -f $firmwareBytes[0xC0], $firmwareBytes[0xC1], $firmwareBytes[0xC2]
if ($optionBytes -ne "EE 36 E0") {
    throw "Unexpected option bytes at 0xC0..0xC2: $optionBytes (expected EE 36 E0)"
}

New-Item -ItemType Directory -Force -Path $backupDirectory | Out-Null
$timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"
$backup = Join-Path $backupDirectory "eeprom-before-debug-$timestamp.bin"
$firmwareHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $firmware).Hash

Write-Host "Firmware: $firmware"
Write-Host "Size: $($firmwareBytes.Length) bytes (limit 64512)"
Write-Host "Option bytes: $optionBytes"
Write-Host "SHA-256: $firmwareHash"
Write-Host "EEPROM backup: $backup"

if ($ValidateOnly) {
    Write-Host "Validation completed; OpenOCD was not started and flash was not modified"
    exit 0
}

$firmwareTcl = $firmware.Replace("\", "/")
$backupTcl = $backup.Replace("\", "/")
$flashCommand = "cms32_flash_safe_word {$firmwareTcl} {$backupTcl}"

Push-Location $PSScriptRoot
try {
    & $openOcd `
        -f "tcl/target_CMS32L051_win.cfg" `
        -c "adapter speed 4000" `
        -c "script tcl/CMS32L051_flash.tcl" `
        -c "init" `
        -c $flashCommand `
        -c "shutdown"
    if ($LASTEXITCODE -ne 0) {
        throw "OpenOCD failed with exit code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
}

$backupBytes = [IO.File]::ReadAllBytes($backup)
if ($backupBytes.Length -ne 1024) {
    throw "Invalid EEPROM backup length after flash: $($backupBytes.Length)"
}
if ([Text.Encoding]::ASCII.GetString($backupBytes, 0, 4) -ne "chli") {
    throw "EEPROM backup does not contain the expected chli signature"
}

$backupHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $backup).Hash
Write-Host "Flash and verification completed successfully"
Write-Host "EEPROM backup SHA-256: $backupHash"
