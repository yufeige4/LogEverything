@echo off
REM LogEverything 多轮性能测试 (Development 环境)
REM 用法: RunPerfTestMultiRound.bat [轮数] (默认 10)
REM
REM 本脚本在 Development 环境下运行多轮性能测试
REM 收集每轮数据并生成详细分析报告
REM
REM 注意: Shipping/Test 环境不包含 UE 自动化测试框架，无法进行自动化性能测试

setlocal enabledelayedexpansion

set ROUNDS=%1
if "%ROUNDS%"=="" set ROUNDS=10

set UE_ROOT=M:\UEAS\trunk\Engine\Windows\Engine
set UE_EDITOR="%UE_ROOT%\Binaries\Win64\UnrealEditor.exe"
set PROJECT_DIR=M:\GitHub\MyModularGameplay\ModularGameplayFramework\TestProject
set PROJECT="%PROJECT_DIR%\TestProject.uproject"
set REPORT_PATH=%PROJECT_DIR%\Saved\LogEverything\PerfReport
set CSV_BASE=%REPORT_PATH%\perf_results

echo ============================================================
echo [LogEverything] Multi-Round Performance Test
echo ============================================================
echo [Config] Environment: Development
echo [Config] Test Rounds: %ROUNDS%
echo [Config] Report Path: %REPORT_PATH%
echo [Config] Log Count: 1,000,000 per test case
echo ============================================================
echo.

REM 确保报告目录存在
if not exist "%REPORT_PATH%" mkdir "%REPORT_PATH%"

REM 清理旧的轮次文件
echo [Info] Cleaning old round files...
del /q "%CSV_BASE%_dev_round_*.csv" 2>nul
echo.

REM ============ Development 环境测试 ============
echo [Testing] Running %ROUNDS% rounds of performance tests...
echo ============================================================

for /L %%i in (1,1,%ROUNDS%) do (
    echo [Round %%i/%ROUNDS%] Starting...

    REM 删除临时 CSV
    if exist "%CSV_BASE%.csv" del "%CSV_BASE%.csv"

    REM 运行 Development 环境测试（使用 UE Editor）
    %UE_EDITOR% %PROJECT% ^
        -game ^
        -nullrhi ^
        -nosplash ^
        -NoVerifyGC ^
        -LOG=PerfTest_Dev_Round%%i.log ^
        -ReportExportPath="%REPORT_PATH%" ^
        -ExecCmds="LogEverything.Debug.LogCategory 0,Automation RunTests LogEverything.Performance;Quit"

    REM 重命名 CSV 文件
    if exist "%CSV_BASE%.csv" (
        move "%CSV_BASE%.csv" "%CSV_BASE%_dev_round_%%i.csv" >nul
        echo [Round %%i/%ROUNDS%] Completed - saved to perf_results_dev_round_%%i.csv
    ) else (
        echo [Round %%i/%ROUNDS%] Warning: CSV not generated
    )
)

echo.
echo ============================================================
echo [LogEverything] All %ROUNDS% rounds completed.
echo [LogEverything] Generating detailed analysis report...
echo ============================================================
echo.

REM 调用 Python 脚本生成详细报告
python "%~dp0PerfReport\GeneratePerfReportMultiRound.py" ^
    --input-dir "%REPORT_PATH%" ^
    --output "%REPORT_PATH%" ^
    --rounds %ROUNDS%

echo.
echo [LogEverything] Report generation complete.
echo.

REM 打开输出目录
start "" "%REPORT_PATH%"

pause
