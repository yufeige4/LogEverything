// Copyright Epic Games, Inc. All Rights Reserved.

#include "Bridge/LEBqLogBridge.h"
#include "Utils/LogEverythingUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

// BqLog 包含文件
#include "bq_log/bq_log.h"

// 静态成员初始化
FLEBqLogBridge* FLEBqLogBridge::Instance = nullptr;

FLEBqLogBridge::FLEBqLogBridge()
	: CategoryLogInstance(nullptr)
	, bIsInitialized(false)
{
}

FLEBqLogBridge::~FLEBqLogBridge()
{
	Shutdown();
}

FLEBqLogBridge& FLEBqLogBridge::Get()
{
	if (!Instance)
	{
		Instance = new FLEBqLogBridge();
	}
	return *Instance;
}

const bq::LogEverythingLogger* FLEBqLogBridge::GetCategoryLogInstance() const
{
	if (!bIsInitialized || !CategoryLogInstance)
	{
		LE_SYSTEM_ERROR(TEXT("BqLogBridge not initialized or CategoryLogInstance is null"));
		return nullptr;
	}

	return CategoryLogInstance;
}



void FLEBqLogBridge::FlushLogs()
{
	if (bIsInitialized && CategoryLogInstance)
	{
		CategoryLogInstance->force_flush();
	}
}

bool FLEBqLogBridge::Initialize(const FLELogSettings& Settings, ULELogSubsystem* InLogSystem)
{
	FScopeLock Lock(&CriticalSection);

	if (bIsInitialized)
	{
		LE_SYSTEM_LOG(TEXT("FLEBqLogBridge already initialized"));
		return true;
	}

	LogSystemPtr = InLogSystem;
	// 保存配置
	CurrentSettings = Settings;

	// 初始化 BqLog 实例
	bIsInitialized = SetupBqLogConfig(Settings);

	if (bIsInitialized)
	{
		LE_SYSTEM_LOG(TEXT("FLEBqLogBridge initialized successfully"));
	}
	else
	{
		LE_SYSTEM_ERROR(TEXT("Failed to initialize FLEBqLogBridge"));
	}

	return bIsInitialized;
}

void FLEBqLogBridge::Shutdown()
{
	FScopeLock Lock(&CriticalSection);

	if (!bIsInitialized)
	{
		return;
	}

	// BqLog 实例会在静态对象析构时自动清理
	// 这里只需要重置指针
	if (CategoryLogInstance)
	{
		CategoryLogInstance->force_flush();
		CategoryLogInstance = nullptr;
	}

	bIsInitialized = false;

	LE_SYSTEM_LOG(TEXT("FLEBqLogBridge shutdown"));
}

FString FLEBqLogBridge::UTF8ToFString(const char* UTF8String)
{
	if (!UTF8String)
	{
		return FString();
	}

	// 使用 UE 的 UTF8_TO_TCHAR 宏进行转换
	return FString(UTF8_TO_TCHAR(UTF8String));
}

TArray<uint8> FLEBqLogBridge::FStringToUTF8(const FString& InString)
{
	TArray<uint8> Result;

	if (InString.IsEmpty())
	{
		return Result;
	}

	// 将 FString 转换为 UTF-8 字节数组
	FTCHARToUTF8 Converter(*InString);
	const int32 UTF8Length = Converter.Length();

	if (UTF8Length > 0)
	{
		Result.SetNumUninitialized(UTF8Length + 1); // +1 for null terminator
		FMemory::Memcpy(Result.GetData(), Converter.Get(), UTF8Length);
		Result[UTF8Length] = 0; // null terminator
		Result.SetNum(UTF8Length); // 移除 null terminator，让调用者决定是否需要
	}

	return Result;
}


bool FLEBqLogBridge::SetupBqLogConfig(const FLELogSettings& Settings)
{
	// 创建日志目录路径（使用 Saved/LogEverything 目录）
	FString SavedDir = FPaths::ProjectSavedDir();
	FString LogDirectory = FPaths::Combine(SavedDir, TEXT("LogEverything"));

	// 确保日志目录存在
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*LogDirectory))
	{
		PlatformFile.CreateDirectoryTree(*LogDirectory);
	}

	// 生成带时间戳的日志文件名（格式：LE_[进程ID]，不带扩展名）
	// BqLog会自动添加扩展名和时间戳
	FDateTime Now = FDateTime::Now();
	uint32 ProcessId = FPlatformProcess::GetCurrentProcessId();
	FString LogFileName = FString::Printf(TEXT("LE_%u"), ProcessId);

	// 使用绝对路径（BqLog 使用）
	FString AbsoluteLogPath = FPaths::Combine(LogDirectory, LogFileName);
	// 将路径转换为正斜杠格式，BqLog 可能需要这种格式
	AbsoluteLogPath = AbsoluteLogPath.Replace(TEXT("\\"), TEXT("/"));

	LE_SYSTEM_LOG(TEXT("Setting up BqLog config:"));
	LE_SYSTEM_LOG(TEXT("  LogDirectory: %s"), *LogDirectory);
	LE_SYSTEM_LOG(TEXT("  LogFileName: %s"), *LogFileName);
	LE_SYSTEM_LOG(TEXT("  AbsoluteLogPath: %s"), *AbsoluteLogPath);
	LE_SYSTEM_LOG(TEXT("  BufferSize: %d"), Settings.BufferSize);
	LE_SYSTEM_LOG(TEXT("  AsyncLogging: %s"), Settings.bEnableAsyncLogging ? TEXT("true") : TEXT("false"));
	LE_SYSTEM_LOG(TEXT("  Compression: %s"), Settings.bEnableCompression ? TEXT("true") : TEXT("false"));

	// 构建 BqLog 配置字符串（简化版本）
	// 先尝试最简单的配置，避免复杂参数导致解析失败
	FString ConfigString = FString::Printf(TEXT(
		"appenders_config.appender_0.type=text_file\n"
		"appenders_config.appender_0.file_name=%s\n"
		"appenders_config.appender_0.levels=[all]\n"
		"log.categories_mask=all"
	),
		*AbsoluteLogPath
	);

	// 输出配置字符串用于调试
	LE_SYSTEM_LOG(TEXT("BqLog Config String:"));
	LE_SYSTEM_LOG(TEXT("%s"), *ConfigString);

	// 转换配置字符串为 UTF-8
	TArray<uint8> ConfigUTF8 = FStringToUTF8(ConfigString);
	// 确保有null终止符
	if (ConfigUTF8.Num() == 0 || ConfigUTF8.Last() != 0)
	{
		ConfigUTF8.Add(0);
	}

	LE_SYSTEM_LOG(TEXT("ConfigUTF8 length: %d"), ConfigUTF8.Num());
	bq::string BqLogConfig((const char*)ConfigUTF8.GetData());

	// 使用配置字符串创建 LogEverythingLogger 实例
	static bq::LogEverythingLogger CategoryLogInstanceStatic = bq::LogEverythingLogger::create_log(
		bq::string("LogEverythingLogger"),
		BqLogConfig
	);

	// 保存实例指针
	CategoryLogInstance = &CategoryLogInstanceStatic;

	if (!CategoryLogInstance || !CategoryLogInstance->is_valid())
	{
		LE_SYSTEM_ERROR(TEXT("Failed to create BqLog Category Log instance"));
		CategoryLogInstance = nullptr;
		return false;
	}

	LE_SYSTEM_LOG(TEXT("BqLog Category Log instance created successfully with %d categories"), CategoryLogInstance->get_categories_count());
	return true;
}

