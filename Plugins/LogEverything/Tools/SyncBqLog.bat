@echo off
REM BqLog 版本同步脚本
REM 用于将最新的 BqLog 源码同步到 LogEverything 插件

setlocal EnableDelayedExpansion

REM 设置默认的 BqLog 源路径
if "%BQLOG_SOURCE_PATH%"=="" (
    set "BQLOG_SOURCE_PATH=M:\GitHub\BqLog"
)

echo ========================================
echo BqLog 版本同步脚本
echo ========================================
echo 源路径: %BQLOG_SOURCE_PATH%
echo 目标路径: %~dp0..\Source\BQLog\
echo ========================================

REM 检查源路径是否存在
if not exist "%BQLOG_SOURCE_PATH%" (
    echo 错误: BqLog 源路径不存在: %BQLOG_SOURCE_PATH%
    echo 请设置环境变量 BQLOG_SOURCE_PATH 或确保路径正确
    exit /b 1
)

REM 检查源码目录结构
if not exist "%BQLOG_SOURCE_PATH%\src\bq_common" (
    echo 错误: 未找到 bq_common 目录
    exit /b 1
)

if not exist "%BQLOG_SOURCE_PATH%\src\bq_log" (
    echo 错误: 未找到 bq_log 目录
    exit /b 1
)

REM 检查许可证文件
if not exist "%BQLOG_SOURCE_PATH%\LICENSE" (
    echo 警告: 未找到 LICENSE 文件
)

REM 获取目标目录
set "TARGET_DIR=%~dp0..\Source\BQLog"

REM 创建目标目录（如果不存在）
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)

echo 开始同步源码文件...

REM 同步 bq_common
echo 同步 bq_common...
xcopy "%BQLOG_SOURCE_PATH%\src\bq_common" "%TARGET_DIR%\bq_common\" /E /Y /I
if errorlevel 1 (
    echo 错误: bq_common 同步失败
    exit /b 1
)

REM 同步 bq_log
echo 同步 bq_log...
xcopy "%BQLOG_SOURCE_PATH%\src\bq_log" "%TARGET_DIR%\bq_log\" /E /Y /I
if errorlevel 1 (
    echo 错误: bq_log 同步失败
    exit /b 1
)

REM 同步 CMakeLists.txt
echo 同步 CMakeLists.txt...
copy "%BQLOG_SOURCE_PATH%\src\CMakeLists.txt" "%TARGET_DIR%\CMakeLists.txt" /Y
if errorlevel 1 (
    echo 错误: CMakeLists.txt 同步失败
    exit /b 1
)

REM 同步编译后的头文件（如果存在）
if exist "%BQLOG_SOURCE_PATH%\dist\dynamic_lib\include\bq_log\bq_log.h" (
    echo 同步主API头文件...
    if not exist "%TARGET_DIR%\include" mkdir "%TARGET_DIR%\include"
    if not exist "%TARGET_DIR%\include\bq_log" mkdir "%TARGET_DIR%\include\bq_log"
    copy "%BQLOG_SOURCE_PATH%\dist\dynamic_lib\include\bq_log\bq_log.h" "%TARGET_DIR%\include\bq_log\bq_log.h" /Y

    REM 同步其他API头文件
    if exist "%BQLOG_SOURCE_PATH%\dist\dynamic_lib\include\bq_log\misc" (
        xcopy "%BQLOG_SOURCE_PATH%\dist\dynamic_lib\include\bq_log\misc" "%TARGET_DIR%\include\bq_log\misc\" /E /Y /I
    )

    if exist "%BQLOG_SOURCE_PATH%\dist\dynamic_lib\include\bq_log\adapters" (
        xcopy "%BQLOG_SOURCE_PATH%\dist\dynamic_lib\include\bq_log\adapters" "%TARGET_DIR%\include\bq_log\adapters\" /E /Y /I
    )
)

REM 同步许可证文件
if exist "%BQLOG_SOURCE_PATH%\LICENSE" (
    echo 同步许可证文件...
    copy "%BQLOG_SOURCE_PATH%\LICENSE" "%TARGET_DIR%\LICENSE.txt" /Y
)

if exist "%BQLOG_SOURCE_PATH%\NOTICE" (
    copy "%BQLOG_SOURCE_PATH%\NOTICE" "%TARGET_DIR%\NOTICE.txt" /Y
)

echo ========================================
echo 同步完成!
echo ========================================

REM 显示版本信息（如果有的话）
if exist "%BQLOG_SOURCE_PATH%\VERSION" (
    echo BqLog 版本信息:
    type "%BQLOG_SOURCE_PATH%\VERSION"
)

echo.
echo 注意: 同步完成后请重新编译项目以确保更改生效
echo.

pause