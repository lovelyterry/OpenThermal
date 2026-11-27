param(
    [Parameter(Position = 0)]
    [string]$Action = "",
    [switch]$Flash,
    [switch]$FlashOnly
)

if ($Action -ieq "flash") { $Flash = $true }
elseif ($Action -ieq "flashonly") { $FlashOnly = $true }

$CurrentDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$BuildRoot = Join-Path $env:TEMP "openthermal_example_pio"
$PioExe = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\platformio.exe"
$PioPy = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\python.exe"
$EsptoolPy = Join-Path $env:USERPROFILE ".platformio\packages\tool-esptoolpy\esptool.py"

function Select-SerialPort {
    $ports = @()
    try {
        $pnp = Get-CimInstance Win32_SerialPort -ErrorAction SilentlyContinue
        foreach ($item in $pnp) {
            if ($item.DeviceID -and $item.DeviceID -ne "COM1") {
                $desc = "$($item.Description)"
                if ($desc -match "Bluetooth|蓝牙|Virtual|Eltima|com0com") { continue }
                $score = 0
                if ($item.PNPDeviceID -match "VID_303A") { $score += 100 }
                if ($item.PNPDeviceID -match "PID_1001|PID_1002") { $score += 20 }
                if ($desc -match "USB|Serial|JTAG|UART|CH34|CP210|FTDI|Espressif") { $score += 10 }
                $ports += [PSCustomObject]@{ Port = $item.DeviceID; Name = "$($item.DeviceID) ($desc)"; Score = $score }
            }
        }
    } catch {}

    if ($ports.Count -eq 0) {
        $rawPorts = [System.IO.Ports.SerialPort]::GetPortNames() | Where-Object { $_ -ne "COM1" } | Sort-Object -Unique
        foreach ($p in $rawPorts) {
            $ports += [PSCustomObject]@{ Port = $p; Name = $p; Score = 0 }
        }
    }

    $ports = @($ports | Sort-Object -Property Score -Descending)
    if ($ports.Count -eq 0) {
        throw "未检测到可用串口，请先连接设备 USB 数据线。"
    }
    Write-Host "[PORT] 自动识别串口: $($ports[0].Name)" -ForegroundColor Cyan
    return $ports[0].Port
}

Push-Location $CurrentDir
try {
    if (-not $FlashOnly) {
        $env:PLATFORMIO_BUILD_DIR = $BuildRoot
        & $PioExe run --target buildfs --environment espirc
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        & $PioExe run --environment espirc
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

        if (!(Test-Path "./firmware" -PathType Container)) {
            New-Item -ItemType Directory -Force -Path "./firmware" | Out-Null
        }

        $OutDir = Join-Path $BuildRoot "espirc"
        Copy-Item -Force (Join-Path $OutDir "firmware.bin") -Destination "./firmware/firmware.bin"
        Copy-Item -Force (Join-Path $OutDir "bootloader.bin") -Destination "./firmware/bootloader.bin"
        Copy-Item -Force (Join-Path $OutDir "littlefs.bin") -Destination "./firmware/littlefs.bin"
        Copy-Item -Force (Join-Path $OutDir "partitions.bin") -Destination "./firmware/partitions.bin"
        Copy-Item -Force "~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin" -Destination "./firmware/boot_app0.bin"

        if (-not $Flash) {
            $ans = Read-Host "`n[BUILD] 编译完成！是否立即自动识别串口并烧录？(Y/n)"
            if ([string]::IsNullOrWhiteSpace($ans) -or $ans -match "^[yY](es)?$") {
                $Flash = $true
            }
        }
    }

    if ($Flash -or $FlashOnly) {
        if (!(Test-Path $EsptoolPy)) {
            throw "未找到 esptool.py: $EsptoolPy"
        }
        $fwDir = Join-Path $CurrentDir "firmware"
        $required = @("bootloader.bin", "partitions.bin", "boot_app0.bin", "firmware.bin", "littlefs.bin")
        foreach ($f in $required) {
            if (!(Test-Path (Join-Path $fwDir $f))) {
                throw "缺少固件文件 $(Join-Path $fwDir $f)，请先运行 .\make.ps1 完成编译。"
            }
        }

        $targetPort = Select-SerialPort
        Write-Host "[FLASH] 正在烧录全套固件到 $targetPort (波特率: 921600)..." -ForegroundColor Green
        & $PioPy $EsptoolPy `
            --chip esp32s3 `
            --port $targetPort `
            --baud 921600 `
            --before default_reset `
            --after hard_reset `
            write_flash -z `
            --flash_mode qio `
            --flash_freq 80m `
            --flash_size 4MB `
            0x0     (Join-Path $fwDir "bootloader.bin") `
            0x8000  (Join-Path $fwDir "partitions.bin") `
            0x1E000 (Join-Path $fwDir "boot_app0.bin") `
            0x20000 (Join-Path $fwDir "firmware.bin") `
            0x2FB000 (Join-Path $fwDir "littlefs.bin")
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        Write-Host "[DONE] 固件烧录完成！" -ForegroundColor Green
    }
} finally {
    Remove-Item Env:\PLATFORMIO_BUILD_DIR -ErrorAction SilentlyContinue
    Pop-Location
}