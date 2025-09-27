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
 * 蓝图友好的日志级别枚举 - 与 BqLog 的 log_level 一一对应
 * Blueprint-friendly log verbosity enumeration that maps directly to BqLog log_level
 */
UENUM(BlueprintType)
enum class ELELogVerbosity : uint8
{
	Verbose     = 0		UMETA(DisplayName = "Verbose"),     // 对应 BqLog::verbose
	Debug       = 1		UMETA(DisplayName = "Debug"),       // 对应 BqLog::debug
	Info        = 2		UMETA(DisplayName = "Info"),        // 对应 BqLog::info
	Warning     = 3		UMETA(DisplayName = "Warning"),     // 对应 BqLog::warning
	Error       = 4		UMETA(DisplayName = "Error"),       // 对应 BqLog::error
	Fatal       = 5		UMETA(DisplayName = "Fatal"),       // 对应 BqLog::fatal

	NoLogging   = 255	UMETA(DisplayName = "No Logging"),  // 特殊值，表示完全关闭日志
};

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
	ELELogVerbosity LogLevel;

	FLECategoryLevel()
		: CategoryName(NAME_None)
		, LogLevel(ELELogVerbosity::Info)
	{
	}

	FLECategoryLevel(const FName& InCategoryName, ELELogVerbosity InLogLevel)
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
	ELELogVerbosity GlobalLogLevel;

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
		: GlobalLogLevel(ELELogVerbosity::Info)
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

/**
 * 类型转换工具函数
 * Type conversion utility functions
 */
namespace LELogVerbosityUtils
{
	/** 将蓝图友好的枚举转换为 UE 内部枚举 */
	inline ELogVerbosity::Type ToUELogVerbosity(ELELogVerbosity Verbosity)
	{
		// 将 BqLog 对应的级别映射到 UE 的日志级别
		switch (Verbosity)
		{
		case ELELogVerbosity::NoLogging:    return ELogVerbosity::NoLogging;
		case ELELogVerbosity::Verbose:      return ELogVerbosity::VeryVerbose;  // BqLog verbose -> UE VeryVerbose
		case ELELogVerbosity::Debug:        return ELogVerbosity::Verbose;      // BqLog debug -> UE Verbose
		case ELELogVerbosity::Info:         return ELogVerbosity::Log;          // BqLog info -> UE Log
		case ELELogVerbosity::Warning:      return ELogVerbosity::Warning;      // BqLog warning -> UE Warning
		case ELELogVerbosity::Error:        return ELogVerbosity::Error;        // BqLog error -> UE Error
		case ELELogVerbosity::Fatal:        return ELogVerbosity::Fatal;        // BqLog fatal -> UE Fatal
		default:                            return ELogVerbosity::Log;          // 默认级别
		}
	}

	/** 将 UE 内部枚举转换为蓝图友好的枚举 */
	inline ELELogVerbosity FromUELogVerbosity(ELogVerbosity::Type Verbosity)
	{
		// 将 UE 的日志级别映射回 BqLog 对应的级别
		switch (Verbosity)
		{
		case ELogVerbosity::NoLogging:      return ELELogVerbosity::NoLogging;
		case ELogVerbosity::Fatal:          return ELELogVerbosity::Fatal;
		case ELogVerbosity::Error:          return ELELogVerbosity::Error;
		case ELogVerbosity::Warning:        return ELELogVerbosity::Warning;
		case ELogVerbosity::Display:        return ELELogVerbosity::Info;       // UE Display -> BqLog info
		case ELogVerbosity::Log:            return ELELogVerbosity::Info;       // UE Log -> BqLog info
		case ELogVerbosity::Verbose:        return ELELogVerbosity::Debug;      // UE Verbose -> BqLog debug
		case ELogVerbosity::VeryVerbose:    return ELELogVerbosity::Verbose;    // UE VeryVerbose -> BqLog verbose
		default:                            return ELELogVerbosity::Info;       // 默认级别
		}
	}

	/** 将 BqLog 级别直接转换为我们的枚举（零开销转换） */
	inline ELELogVerbosity FromBqLogLevel(uint8 BqLogLevel)
	{
		// BqLog 级别值可以直接转换（除了 NoLogging 特殊情况）
		if (BqLogLevel <= 5) // BqLog 有效级别范围 0-5
		{
			return static_cast<ELELogVerbosity>(BqLogLevel);
		}
		return ELELogVerbosity::Info; // 默认级别
	}

	/** 将我们的枚举转换为 BqLog 级别（零开销转换） */
	inline uint8 ToBqLogLevel(ELELogVerbosity Verbosity)
	{
		uint8 Level = static_cast<uint8>(Verbosity);
		// 如果是 NoLogging，返回一个 BqLog 不支持的值，由调用者处理
		if (Level == 255) // NoLogging
		{
			return 255; // 调用者需要特殊处理这种情况
		}
		return Level; // 其他情况直接返回数值（0-5）
	}
};