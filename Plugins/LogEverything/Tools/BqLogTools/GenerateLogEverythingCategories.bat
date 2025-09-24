@echo off
REM ================================================================================
REM LogEverything Category Code Generator
REM
REM This script automatically generates LogEverything Category log class structure
REM from Config/LogEverythingCategories.txt configuration file
REM
REM Usage:
REM   GenerateLogEverythingCategories.bat [clean]
REM
REM Parameters:
REM   clean - Optional parameter to clean existing generated files before generation
REM
REM Author: LogEverything Plugin
REM Version: 1.0
REM ================================================================================

REM Check if we're in a new window, if not, open one
if not "%GENERATOR_NEW_WINDOW%"=="1" (
    set GENERATOR_NEW_WINDOW=1
    start "LogEverything Generator" cmd /k "%~f0 %*"
    exit /b 0
)

setlocal enabledelayedexpansion

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "PLUGIN_ROOT=%SCRIPT_DIR%.."
set "CONFIG_FILE=%PLUGIN_ROOT%\Config\LogEverythingCategories.txt"
set "GENERATED_DIR=%PLUGIN_ROOT%\Source\Generated"
set "TOOL_PATH=%SCRIPT_DIR%BqLogTools\BqLog_CategoryLogGenerator.exe"

echo === LogEverything Category Code Generator ===
echo.

REM Check if tool exists
if not exist "%TOOL_PATH%" (
    echo ERROR: Cannot find BqLog_CategoryLogGenerator.exe
    echo Path: %TOOL_PATH%
    echo Please ensure the tool file exists
    echo.
    goto :finish
)

REM Check if config file exists
if not exist "%CONFIG_FILE%" (
    echo ERROR: Cannot find config file LogEverythingCategories.txt
    echo Path: %CONFIG_FILE%
    echo Please create the config file and define log categories
    echo.
    goto :finish
)

REM Handle clean parameter
if /i "%1"=="clean" (
    echo Cleaning existing generated files...
    if exist "%GENERATED_DIR%" (
        rmdir /s /q "%GENERATED_DIR%"
        echo Generated directory cleaned
    )
    echo.
)

REM Ensure generated directory exists
if not exist "%GENERATED_DIR%" (
    echo Creating generated directory: %GENERATED_DIR%
    mkdir "%GENERATED_DIR%"
)

REM Display configuration info
echo Configuration Info:
echo   Tool Path: %TOOL_PATH%
echo   Config File: %CONFIG_FILE%
echo   Generated Dir: %GENERATED_DIR%
echo   Generated Class: LogEverythingLogger
echo.

REM Display config file content
echo Config File Content:
type "%CONFIG_FILE%"
echo.

echo Generating LogEverythingLogger...

REM Switch to generated directory
pushd "%GENERATED_DIR%"

REM Call BqLog code generation tool
REM Syntax: BqLog_CategoryLogGenerator.exe [ClassName] [ConfigFilePath] [OutputDir]
"%TOOL_PATH%" LogEverythingLogger "%CONFIG_FILE%" "."

REM Check generation result
if %ERRORLEVEL% neq 0 (
    echo ERROR: Code generation failed, error code: %ERRORLEVEL%
    popd
    echo.
    goto :finish
)

REM Return to original directory
popd

REM Verify generated files
set "GENERATED_HEADER=%GENERATED_DIR%\LogEverythingLogger.h"
if exist "%GENERATED_HEADER%" (
    echo SUCCESS: LogEverythingLogger.h generated successfully

    REM Display file size and modification time
    for %%F in ("%GENERATED_HEADER%") do (
        echo   File Size: %%~zF bytes
        echo   Modified: %%~tF
    )

    echo.
    echo File generated successfully at:
    echo %GENERATED_HEADER%
    echo.

    echo SUCCESS: Category code generation completed!
    echo.
    echo Next Steps:
    echo   1. Recompile LogEverything plugin
    echo   2. Use LE_CATEGORY(Game.Combat.Skill) syntax in your code
    echo   3. Refer to LECategoryMacros.h for usage examples
    echo.
    echo ===================================================
    echo    Operation completed successfully!
    echo ===================================================

) else (
    echo ERROR: Generated header file not found
    echo Path: %GENERATED_HEADER%
    echo Code generation may have failed
    echo.
)

:finish
echo.
echo Press any key to close this window...
pause >nul
exit /b 0