@echo off
REM LogEverything Performance Test - 直接运行测试（不依赖 Session Frontend）

set UE_EDITOR="M:\UEAS\trunk\Engine\Windows\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJECT="M:\GitHub\MyModularGameplay\ModularGameplayFramework\TestProject\TestProject.uproject"
set REPORT_PATH=M:\GitHub\MyModularGameplay\ModularGameplayFramework\TestProject\Saved\LogEverything\PerfReport
set LOG_FILE=M:\GitHub\MyModularGameplay\ModularGameplayFramework\TestProject\Saved\Logs\PerfTestDirect.log

echo [LogEverything] Performance Test - Direct Execution Mode
echo ============================================================
echo [Config] Project: %PROJECT%
echo [Config] Report Path: %REPORT_PATH%
echo [Config] Log File: %LOG_FILE%
echo ============================================================
echo.

REM 确保报告目录存在
if not exist "%REPORT_PATH%" mkdir "%REPORT_PATH%"

REM 删除旧日志文件
if exist "%LOG_FILE%" del "%LOG_FILE%"

echo [Info] Starting log monitor in new window...
echo [Info] Watch the log window for test progress.
echo [Info] Look for "PERF-01", "PERF-02", etc. to track progress.
echo.

REM 启动日志监控窗口（使用 PowerShell 实时显示日志）
start "LogEverything Test Monitor" powershell -NoExit -Command "Write-Host 'Waiting for log file...' -ForegroundColor Yellow; while (-not (Test-Path '%LOG_FILE%')) { Start-Sleep -Milliseconds 500 }; Write-Host 'Log file found, monitoring...' -ForegroundColor Green; Get-Content '%LOG_FILE%' -Wait | Select-String -Pattern 'PERF-|LogAutomation|Error|Warning|Success|Fail|Complete'"

REM 等待一秒让监控窗口启动
timeout /t 2 /nobreak > nul

echo [Info] Running tests... Check the monitor window for progress.
echo.

REM 直接运行性能测试
%UE_EDITOR% %PROJECT% ^
    -game ^
    -nullrhi ^
    -nosplash ^
    -NoVerifyGC ^
    -LOG=PerfTestDirect.log ^
    -ReportExportPath="%REPORT_PATH%" ^
    -ExecCmds="LogEverything.Debug.LogCategory 0,Automation RunTests LogEverything.Performance;Quit"

echo.
echo [LogEverything] Test execution completed.
echo [LogEverything] Check report at: %REPORT_PATH%
echo [LogEverything] Full log at: %LOG_FILE%
pause
