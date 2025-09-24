// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LELogTypes.h"
#include "Engine/Engine.h"
#include "Generated/LogEverythingLogger.h"

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
	bool Initialize(const FLELogSettings& Settings);

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

	void RegisterCategory(const FName& CategoryName);


	/** 设置分类的日志级别（基于 FName）*/
	void SetCategoryLevel(const FName& CategoryName, ELogVerbosity::Type Level);

	/** 设置全局日志级别 */
	void SetGlobalLevel(ELogVerbosity::Type Level);

	/** 启用/禁用特定分类（基于 FName）*/
	void EnableCategory(const FName& CategoryName, bool bEnable);

	/** 强制刷新日志缓冲区 */
	void FlushLogs();


	/** 高性能模板日志函数 - 直接调用 BqLog 模板接口，避免字符串预格式化
	 * 使用 LE Category 对象，支持级别检查和后续的 CheckCanLog 功能
	 * @param Category  声明的分类对象 (如 LogGameCombatSkill)
	 * @param Level     UE 日志级别
	 * @param Format    格式化字符串
	 * @param Args      格式化参数
	 */
	template<typename CategoryType, typename FormatType, typename... Args>
	void LogWithTemplate(const CategoryType& Category, ELogVerbosity::Type Level, const FormatType& Format, const Args&... Arguments);

public:
	/** UTF-8 与 UTF-16 转换函数 */
	static FString UTF8ToFString(const char* UTF8String);
	static TArray<uint8> FStringToUTF8(const FString& InString);

	/** UE 日志级别转换为 BqLog 级别 */
	static bq::log_level ConvertVerbosityToBqLog(ELogVerbosity::Type Verbosity);

	/** BqLog 级别转换为 UE 日志级别 */
	static ELogVerbosity::Type ConvertBqLogToVerbosity(bq::log_level BqLogLevel);

private:
	/** BqLog Category Log 实例 */
	bq::LogEverythingLogger* CategoryLogInstance;

	/** 初始化状态 */
	bool bIsInitialized;

	/** 当前配置 */
	FLELogSettings CurrentSettings;

	/** 分类级别映射 - 基于 FName */
	TMap<FName, ELogVerbosity::Type> CategoryLevels;


	/** 全局日志级别 */
	ELogVerbosity::Type GlobalLogLevel;

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

	/** 创建日志文件路径 */
	FString CreateLogFilePath(const FString& RelativePath);

	/** 检查分类是否应该输出日志 - 基于 FName */
	bool ShouldLogCategory(const FName& CategoryName, ELogVerbosity::Type Verbosity) const;


};

// 模板函数实现

template<typename CategoryType, typename FormatType, typename... Args>
void FLEBqLogBridge::LogWithTemplate(const CategoryType& Category, ELogVerbosity::Type Level, const FormatType& Format, const Args&... Arguments)
{
	// 检查 BqLog 实例是否有效
	if (!bIsInitialized || !CategoryLogInstance)
	{
		return;
	}

	// 检查分类级别是否允许输出日志（保留现有的级别检查机制）
	// 性能太差, 暂时先注掉, 后续进行详细设计后再打开
	// if (!ShouldLogCategory(Category.GetCategoryName(), Level))
	// {
	// 	return;
	// }

	// 获取分类句柄
	auto CategoryHandle = Category.GetCategoryHandle(*CategoryLogInstance);

	// 根据日志级别调用相应的 BqLog 模板方法，直接传递格式和参数
	switch (Level)
	{
	case ELogVerbosity::Fatal:
		(void)CategoryLogInstance->fatal(CategoryHandle, Format, Arguments...);
		break;
	case ELogVerbosity::Error:
		(void)CategoryLogInstance->error(CategoryHandle, Format, Arguments...);
		break;
	case ELogVerbosity::Warning:
		(void)CategoryLogInstance->warning(CategoryHandle, Format, Arguments...);
		break;
	case ELogVerbosity::Display:
	case ELogVerbosity::Log:
		(void)CategoryLogInstance->info(CategoryHandle, Format, Arguments...);
		break;
	case ELogVerbosity::Verbose:
		(void)CategoryLogInstance->debug(CategoryHandle, Format, Arguments...);
		break;
	case ELogVerbosity::VeryVerbose:
		(void)CategoryLogInstance->verbose(CategoryHandle, Format, Arguments...);
		break;
	default:
		(void)CategoryLogInstance->info(CategoryHandle, Format, Arguments...);
		break;
	}
}