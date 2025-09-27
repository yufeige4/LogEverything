// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "System/LELogTypes.h"
#include "Engine/Engine.h"
#include "Generated/LogEverythingLogger.h"

class ULELogSubsystem;

// BqLog 前向声明
namespace bq {
	class LogEverythingLogger;
	enum class log_level : int;
}

/**
 * BqLog 与 Unreal Engine 的桥接类
 * Bridge class between BqLog and Unreal Engine logging system
 */
class LOGEVERYTHING_API FLEBqLogBridge
{
public:
	/** 构造函数 */
	FLEBqLogBridge();

	/** 析构函数 */
	~FLEBqLogBridge();

	/** 初始化 BqLog 系统 */
	bool Initialize(const FLELogSettings& Settings, ULELogSubsystem* InLogSystem);

	/** 关闭 BqLog 系统 */
	void Shutdown();

	/** 是否已初始化 */
	bool IsInitialized() const { return bIsInitialized; }

	/** 获取单例实例 */
	static FLEBqLogBridge& Get();

	/** 获取 BqLog Category Log 实例（用于 LE_CATEGORY 宏）
	 * @return 返回有效的 LogEverythingLogger 指针，如果未初始化或失败则返回 nullptr
	 */
	const bq::LogEverythingLogger* GetCategoryLogInstance() const;

	/** 强制刷新日志缓冲区 */
	void FlushLogs();

	ULELogSubsystem* GetLogSubsystem() const { return LogSystemPtr.Get(); }

	/** 高性能模板日志函数 - 直接调用 BqLog 模板接口，避免字符串预格式化
	 * 使用 LE Category 对象，纯粹的BqLog交互桥梁
	 * @param Category  声明的分类对象 (如 LogGameCombatSkill)
	 * @param Level     LogEverything 日志级别
	 * @param Format    格式化字符串
	 * @param Args      格式化参数
	 */
	template<typename CategoryType, typename FormatType, typename... Args>
	void LogWithTemplate(const CategoryType& Category, ELELogVerbosity Level, const FormatType& Format, const Args&... Arguments);

public:
	/** UTF-8 与 UTF-16 转换函数 */
	static FString UTF8ToFString(const char* UTF8String);
	static TArray<uint8> FStringToUTF8(const FString& InString);

private:
	/** BqLog Category Log 实例 */
	bq::LogEverythingLogger* CategoryLogInstance;

	/** 初始化状态 */
	bool bIsInitialized;

	TWeakObjectPtr<ULELogSubsystem> LogSystemPtr;

	/** 当前配置 */
	FLELogSettings CurrentSettings;

	/** 线程安全锁 */
	mutable FCriticalSection CriticalSection;

	/** 单例实例 */
	static FLEBqLogBridge* Instance;

private:
	/** 不允许拷贝 */
	FLEBqLogBridge(const FLEBqLogBridge&) = delete;
	FLEBqLogBridge& operator=(const FLEBqLogBridge&) = delete;

	/** 初始化 BqLog 配置 */
	bool SetupBqLogConfig(const FLELogSettings& Settings);
	
};

// 模板函数实现

template<typename CategoryType, typename FormatType, typename... Args>
void FLEBqLogBridge::LogWithTemplate(const CategoryType& Category, ELELogVerbosity Level, const FormatType& Format, const Args&... Arguments)
{
	// 检查 BqLog 实例是否有效
	if (!bIsInitialized || !CategoryLogInstance)
	{
		return;
	}

	// 获取分类句柄
	auto CategoryHandle = Category.GetCategoryHandle(*CategoryLogInstance);

	// 根据LogEverything日志级别调用相应的 BqLog 模板方法
	switch (Level)
	{
	case ELELogVerbosity::Fatal:
		(void)CategoryLogInstance->fatal(CategoryHandle, Format, Arguments...);
		break;
	case ELELogVerbosity::Error:
		(void)CategoryLogInstance->error(CategoryHandle, Format, Arguments...);
		break;
	case ELELogVerbosity::Warning:
		(void)CategoryLogInstance->warning(CategoryHandle, Format, Arguments...);
		break;
	case ELELogVerbosity::Info:
		(void)CategoryLogInstance->info(CategoryHandle, Format, Arguments...);
		break;
	case ELELogVerbosity::Debug:
		(void)CategoryLogInstance->debug(CategoryHandle, Format, Arguments...);
		break;
	case ELELogVerbosity::Verbose:
		(void)CategoryLogInstance->verbose(CategoryHandle, Format, Arguments...);
		break;
	case ELELogVerbosity::NoLogging:
	default:
		// NoLogging级别不输出任何内容，其他未知级别默认使用info
		if (Level != ELELogVerbosity::NoLogging)
		{
			(void)CategoryLogInstance->info(CategoryHandle, Format, Arguments...);
		}
		break;
	}
}