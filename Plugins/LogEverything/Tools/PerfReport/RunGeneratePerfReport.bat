@echo off
REM LogEverything 性能报告生成器
REM 从 perf_results.csv 生成性能对比图表和 Markdown 报告

echo [LogEverything] Performance Report Generator
echo ============================================================

REM 使用绝对路径
set PROJECT_ROOT=M:\GitHub\MyModularGameplay\ModularGameplayFramework\TestProject
set CSV_PATH=%PROJECT_ROOT%\Saved\LogEverything\PerfReport\perf_results.csv
set OUTPUT_DIR=%PROJECT_ROOT%\Saved\LogEverything\PerfReport
set SCRIPT_DIR=%~dp0

echo [Config] CSV Path: %CSV_PATH%
echo [Config] Output Dir: %OUTPUT_DIR%
echo.

REM 检查 CSV 文件是否存在
if not exist "%CSV_PATH%" (
    echo [Error] CSV file not found: %CSV_PATH%
    echo [Info] Please run performance tests first to generate perf_results.csv
    echo [Info] Use: Automation RunTests LogEverything.Performance
    pause
    exit /b 1
)

REM 检查 Python 是否可用
python --version >nul 2>&1
if errorlevel 1 (
    echo [Error] Python not found in PATH
    echo [Info] Please install Python 3.x and add it to PATH
    pause
    exit /b 1
)

REM 检查依赖库
echo [Info] Checking Python dependencies...
python -c "import matplotlib" >nul 2>&1
if errorlevel 1 (
    echo [Info] Installing matplotlib...
    pip install matplotlib
)

echo.
echo [Info] Generating performance report...
echo.

cd /d "%SCRIPT_DIR%"
python GeneratePerfReport.py --input "%CSV_PATH%" --output "%OUTPUT_DIR%"

echo.
echo [Info] Report generation complete.
echo.

REM 打开输出目录
start "" "%OUTPUT_DIR%"

pause
