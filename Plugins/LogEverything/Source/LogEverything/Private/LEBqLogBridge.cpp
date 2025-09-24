// Copyright Epic Games, Inc. All Rights Reserved.

#include "LEBqLogBridge.h"
#include "LogEverythingUtils.h"
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
	, GlobalLogLevel(ELogVerbosity::Log)
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


void FLEBqLogBridge::RegisterCategory(const FName& CategoryName)
{
	FScopeLock Lock(&CriticalSection);

	// 简化版注册，只记录分类名称
	LE_SYSTEM_LOG(TEXT("Registered LE category '%s' (simplified)"), *CategoryName.ToString());
}

void FLEBqLogBridge::SetCategoryLevel(const FName& CategoryName, ELogVerbosity::Type Level)
{
	FScopeLock Lock(&CriticalSection);

	CategoryLevels.Add(CategoryName, Level);

	LE_SYSTEM_LOG(TEXT("Set category level for '%s' to %d"),
		*CategoryName.ToString(), static_cast<int32>(Level));
}

void FLEBqLogBridge::SetGlobalLevel(ELogVerbosity::Type Level)
{
	FScopeLock Lock(&CriticalSection);
	GlobalLogLevel = Level;
	LE_SYSTEM_LOG(TEXT("Set global log level: %d"), (int32)Level);
}

void FLEBqLogBridge::EnableCategory(const FName& CategoryName, bool bEnable)
{
	FScopeLock Lock(&CriticalSection);

	if (bEnable)
	{
		// 启用分类：设置为 Verbose 级别
		CategoryLevels.Add(CategoryName, ELogVerbosity::Verbose);
	}
	else
	{
		// 禁用分类：设置为 NoLogging 级别
		CategoryLevels.Add(CategoryName, ELogVerbosity::NoLogging);
	}

	LE_SYSTEM_LOG(TEXT("%s category '%s'"),
		bEnable ? TEXT("Enabled") : TEXT("Disabled"), *CategoryName.ToString());
}

void FLEBqLogBridge::FlushLogs()
{
	if (bIsInitialized && CategoryLogInstance)
	{
		CategoryLogInstance->force_flush();
	}
}

bool FLEBqLogBridge::Initialize(const FLELogSettings& Settings)
{
	FScopeLock Lock(&CriticalSection);

	if (bIsInitialized)
	{
		LE_SYSTEM_LOG(TEXT("FLEBqLogBridge already initialized"));
		return true;
	}

	// 保存配置
	CurrentSettings = Settings;
	GlobalLogLevel = static_cast<ELogVerbosity::Type>(Settings.GlobalLogLevel);

	// 清空分类级别映射
	CategoryLevels.Empty();
	for (const FLECategoryLevel& CategoryLevel : Settings.CategoryLevels)
	{
		CategoryLevels.Add(CategoryLevel.CategoryName, static_cast<ELogVerbosity::Type>(CategoryLevel.LogLevel));
	}

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
	CategoryLevels.Empty();

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

bq::log_level FLEBqLogBridge::ConvertVerbosityToBqLog(ELogVerbosity::Type Verbosity)
{
	// 将 UE 日志级别映射到 BqLog 级别
	// BqLog 级别：verbose, debug, info, warning, error, fatal
	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:
		return bq::log_level::fatal;
	case ELogVerbosity::Error:
		return bq::log_level::error;
	case ELogVerbosity::Warning:
		return bq::log_level::warning;
	case ELogVerbosity::Display:
	case ELogVerbosity::Log:
		return bq::log_level::info;
	case ELogVerbosity::Verbose:
		return bq::log_level::debug;
	case ELogVerbosity::VeryVerbose:
		return bq::log_level::verbose;
	default:
		return bq::log_level::info;
	}
}

ELogVerbosity::Type FLEBqLogBridge::ConvertBqLogToVerbosity(bq::log_level BqLogLevel)
{
	// 将 BqLog 级别映射到 UE 日志级别
	switch (BqLogLevel)
	{
	case bq::log_level::fatal:
		return ELogVerbosity::Fatal;
	case bq::log_level::error:
		return ELogVerbosity::Error;
	case bq::log_level::warning:
		return ELogVerbosity::Warning;
	case bq::log_level::info:
		return ELogVerbosity::Log;
	case bq::log_level::debug:
		return ELogVerbosity::Verbose;
	case bq::log_level::verbose:
		return ELogVerbosity::VeryVerbose;
	default:
		return ELogVerbosity::Log;
	}
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

bool FLEBqLogBridge::ShouldLogCategory(const FName& CategoryName, ELogVerbosity::Type Verbosity) const
{
	// 检查分类是否被禁用
	if (const ELogVerbosity::Type* CategoryLevel = CategoryLevels.Find(CategoryName))
	{
		return Verbosity <= *CategoryLevel;
	}

	// 检查层级化分类（支持 Game.Combat.Skill 形式）
	// 使用 FName 的字符串表示进行解析
	FString CategoryString = CategoryName.ToString();
	TArray<FString> CategoryParts;
	CategoryString.ParseIntoArray(CategoryParts, TEXT("."));

	// 从最具体到最一般检查分类级别
	for (int32 i = CategoryParts.Num() - 1; i >= 0; --i)
	{
		FString PartialCategory;
		for (int32 j = 0; j <= i; ++j)
		{
			if (!PartialCategory.IsEmpty())
			{
				PartialCategory += TEXT(".");
			}
			PartialCategory += CategoryParts[j];
		}

		// 转换为 FName 进行查找，利用 FName 的快速比较
		FName PartialCategoryName(*PartialCategory);
		if (const ELogVerbosity::Type* PartialLevel = CategoryLevels.Find(PartialCategoryName))
		{
			return Verbosity <= *PartialLevel;
		}
	}

	// 最后检查全局级别
	return Verbosity <= GlobalLogLevel;
}