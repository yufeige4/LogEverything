// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Logging/LogVerbosity.h"
#include "LELogTypes.generated.h"

#ifndef LOGEVERYTHING_API
#define LOGEVERYTHING_API
#endif

/**
 * 日志输出目标枚举
 * Defines where log messages should be output
 */
UENUM(BlueprintType)
enum class ELELogOutput : uint8
{
	/** 输出到控制台 */
	Console		UMETA(DisplayName = "Console"),

	/** 输出到文件 */
	File		UMETA(DisplayName = "File"),

	/** 输出到压缩文件 */
	Compressed	UMETA(DisplayName = "Compressed"),

	/** 输出到网络 */
	Network		UMETA(DisplayName = "Network")
};

/**
 * 日志级别映射结构
 * Maps category names to their log verbosity levels
 * 使用 FName 优化性能：O(1) 比较和哈希查找，减少内存分配
 */
USTRUCT(BlueprintType)
struct LOGEVERYTHING_API FLECategoryLevel
{
	GENERATED_BODY()

	/** 日志分类名称 (使用 FName 优化性能) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Settings")
	FName CategoryName;

	/** 日志级别 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Settings")
	uint8 LogLevel;

	FLECategoryLevel()
		: CategoryName(NAME_None)
		, LogLevel(ELogVerbosity::Log)
	{
	}

	FLECategoryLevel(const FName& InCategoryName, uint8 InLogLevel)
		: CategoryName(InCategoryName)
		, LogLevel(InLogLevel)
	{
	}
	
};

/**
 * LogEverything 系统配置结构
 * Configuration structure for the LogEverything system
 */
USTRUCT(BlueprintType)
struct LOGEVERYTHING_API FLELogSettings
{
	GENERATED_BODY()

	/** 各分类的日志级别配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Settings")
	TArray<FLECategoryLevel> CategoryLevels;

	/** 全局默认日志级别 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Settings")
	uint8 GlobalLogLevel;

	/** 日志输出目标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Settings")
	TArray<ELELogOutput> OutputTargets;

	/** 缓冲区大小（字节） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1024", ClampMax = "67108864"))
	int32 BufferSize;

	/** 是否启用异步日志 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bEnableAsyncLogging;

	/** 是否启用压缩 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	bool bEnableCompression;

	/** 日志文件路径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage")
	FString LogFilePath;

	/** 最大日志文件大小（MB） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage", meta = (ClampMin = "1", ClampMax = "1024"))
	int32 MaxLogFileSizeMB;

	FLELogSettings()
		: GlobalLogLevel(ELogVerbosity::Log)
		, BufferSize(1048576) // 1MB default
		, bEnableAsyncLogging(true)
		, bEnableCompression(false)
		, LogFilePath(TEXT("Logs/Game.log"))
		, MaxLogFileSizeMB(100)
	{
		// 默认输出到控制台和文件
		OutputTargets.Add(ELELogOutput::Console);
		OutputTargets.Add(ELELogOutput::File);
	}
};