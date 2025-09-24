// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LEBqLogBridge.h"
#include "LogEverythingUtils.h"
#include "Engine/Engine.h"

/**
 * LogEverything 核心日志宏定义
 * Core logging macros for the LogEverything system
 */

/**
 * LE 分类声明宏 - 在头文件中声明日志分类（仅负责类声明）
 * LE Category declaration macro - declare log category in header files (class declaration only)
 *
 * 类似于 UE 的 DECLARE_LOG_CATEGORY_EXTERN，只负责声明，不包含任何实现
 *
 * @param CategoryName     C++ 标识符名称 (如 LogGameCombatSkill)
 * @param BqCategoryPath   BqLog 分类路径 (如 Game.Combat.Skill)
 *
 * 使用示例：
 * DECLARE_LE_CATEGORY_EXTERN(LogGameCombatSkill, Game.Combat.Skill);
 */
#define DECLARE_LE_CATEGORY_EXTERN(CategoryName, BqCategoryPath) \
	class FLECategory##CategoryName \
	{ \
	public: \
		static const FName GetCategoryName() { return FName(TEXT(#CategoryName)); } \
		static auto GetCategoryHandle(const bq::LogEverythingLogger& Logger) -> decltype((Logger.cat.BqCategoryPath)) \
		{ \
			return Logger.cat.BqCategoryPath; \
		} \
		static bool IsInitialized(); \
		static void Initialize(); \
	}; \
	extern FLECategory##CategoryName CategoryName;

/**
 * LE 分类定义宏 - 在源文件中定义日志分类（仅负责创建类实例）
 * LE Category definition macro - define log category in source files (class instance creation only)
 *
 * 类似于 UE 的 DEFINE_LOG_CATEGORY，只负责创建实例和方法实现，不包含声明
 *
 * @param CategoryName  C++ 标识符名称 (如 LogGameCombatSkill)
 *
 * 使用示例：
 * DEFINE_LE_CATEGORY(LogGameCombatSkill);
 *
 * 注意：必须先在头文件中使用 DECLARE_LE_CATEGORY_EXTERN 声明该分类
 */
#define DEFINE_LE_CATEGORY(CategoryName) \
	FLECategory##CategoryName CategoryName; \
	bool FLECategory##CategoryName::IsInitialized() \
	{ \
		static bool bInitialized = false; \
		if (!bInitialized) \
		{ \
			Initialize(); \
			bInitialized = true; \
		} \
		return bInitialized; \
	} \
	void FLECategory##CategoryName::Initialize() \
	{ \
		FLEBqLogBridge::Get().RegisterCategory(GetCategoryName()); \
	}

/**
 * 核心日志宏 - 使用声明的分类
 * Core logging macro - uses declared categories
 *
 * 使用方式：
 * LE_LOG(LogGameCombatSkill, Warning, TEXT("Player %s cast skill %s"), *PlayerName, *SkillName);
 *
 * @param Category   已声明的分类 (如 LogGameCombatSkill)
 * @param Verbosity  日志级别 (Fatal, Error, Warning, Log, Verbose, VeryVerbose)
 * @param Format     格式化字符串
 * @param ...        格式化参数
 */
#define LE_LOG(Category, Verbosity, Format, ...) \
	do \
	{ \
		const bq::LogEverythingLogger* LogInstance = FLEBqLogBridge::Get().GetCategoryLogInstance(); \
		if (LogInstance != nullptr) \
		{ \
			FString FormattedMessage = FString::Printf(Format, ##__VA_ARGS__); \
			TArray<uint8> UTF8Message = FLEBqLogBridge::Get().FStringToUTF8(FormattedMessage); \
			const char* MessageCStr = (const char*)UTF8Message.GetData(); \
			bq::log_level BqLogLevel = FLEBqLogBridge::Get().ConvertVerbosityToBqLog(ELogVerbosity::Verbosity); \
			auto CategoryHandle = Category.GetCategoryHandle(*LogInstance); \
			FLEBqLogBridge::Get().LogMessage(CategoryHandle, BqLogLevel, MessageCStr); \
		} \
		else \
		{ \
			LE_SYSTEM_LOG(TEXT("[LE-NotInit][%s] " Format), *Category.GetCategoryName().ToString(), ##__VA_ARGS__); \
		} \
	} while (0)

/**
 * 条件日志宏
 * Conditional logging macro
 *
 * @param Condition  条件表达式
 * @param Category   日志分类
 * @param Verbosity  日志级别
 * @param Format     格式化字符串
 * @param ...        格式化参数
 */
#define LE_CLOG(Condition, Category, Verbosity, Format, ...) \
	do \
	{ \
		if (Condition) \
		{ \
			LE_LOG(Category, Verbosity, Format, ##__VA_ARGS__); \
		} \
	} while (0)

/**
 * 断言日志宏 - 检查表达式，失败时记录日志
 * Assertion logging macro - checks expression and logs if it fails
 *
 * @param Expression 要检查的表达式
 * @param Category   日志分类
 * @param Verbosity  日志级别
 * @param Format     格式化字符串
 * @param ...        格式化参数
 */
#define LE_CHECK(Expression, Category, Verbosity, Format, ...) \
	do \
	{ \
		if (!(Expression)) \
		{ \
			LE_LOG(Category, Verbosity, TEXT("CHECK FAILED: (%s) - " Format), TEXT(#Expression), ##__VA_ARGS__); \
		} \
	} while (0)

/**
 * 确保宏 - 类似 LE_CHECK，但继续执行
 * Ensure macro - similar to LE_CHECK but continues execution
 *
 * @param Expression 要检查的表达式
 * @param Category   日志分类
 * @param Verbosity  日志级别
 * @param Format     格式化字符串
 * @param ...        格式化参数
 */
#define LE_ENSURE(Expression, Category, Verbosity, Format, ...) \
	do \
	{ \
		if (!(Expression)) \
		{ \
			LE_LOG(Category, Verbosity, TEXT("ENSURE FAILED: (%s) - " Format), TEXT(#Expression), ##__VA_ARGS__); \
		} \
	} while (0)


/**
 * 便利宏 - 快速访问常用日志级别
 * Convenience macros for quick access to common log levels
 *
 * 使用方式：
 * LE_LOG_FATAL(LogGameCombatSkill, TEXT("Critical error: %s"), *ErrorMsg);
 * LE_LOG_ERROR(LogGameAIPathfinding, TEXT("Path not found"));
 * LE_LOG_WARNING(LogGamePhysicsCollision, TEXT("Collision detected"));
 * LE_LOG_INFO(LogGameCoreMemory, TEXT("Memory allocated: %d bytes"), Size);
 * LE_LOG_VERBOSE(LogGameRenderingMesh, TEXT("Rendering mesh: %s"), *MeshName);
 */
#define LE_LOG_FATAL(CategoryHandle, Format, ...)    LE_LOG(CategoryHandle, Fatal, Format, ##__VA_ARGS__)
#define LE_LOG_ERROR(CategoryHandle, Format, ...)   LE_LOG(CategoryHandle, Error, Format, ##__VA_ARGS__)
#define LE_LOG_WARNING(CategoryHandle, Format, ...) LE_LOG(CategoryHandle, Warning, Format, ##__VA_ARGS__)
#define LE_LOG_INFO(CategoryHandle, Format, ...)     LE_LOG(CategoryHandle, Log, Format, ##__VA_ARGS__)
#define LE_LOG_VERBOSE(CategoryHandle, Format, ...)  LE_LOG(CategoryHandle, Verbose, Format, ##__VA_ARGS__)

/**
 * 运行时配置宏 - 基于 FName
 * Runtime configuration macros based on FName
 *
 * 使用方式：
 * LE_SET_CATEGORY_LEVEL(LogGameCombatSkill, Warning);
 * LE_ENABLE_CATEGORY(LogGameCombatSkill);
 * LE_DISABLE_CATEGORY(LogGameCombatSkill);
 */
#define LE_SET_CATEGORY_LEVEL(Category, Level) \
	FLEBqLogBridge::Get().SetCategoryLevel(Category.GetCategoryName(), ELogVerbosity::Level)

#define LE_SET_GLOBAL_LEVEL(Level) \
	FLEBqLogBridge::Get().SetGlobalLevel(ELogVerbosity::Level)

#define LE_ENABLE_CATEGORY(Category) \
	FLEBqLogBridge::Get().EnableCategory(Category.GetCategoryName(), true)

#define LE_DISABLE_CATEGORY(Category) \
	FLEBqLogBridge::Get().EnableCategory(Category.GetCategoryName(), false)

#define LE_FLUSH_LOGS() \
	FLEBqLogBridge::Get().FlushLogs()

/**
 * 调试专用宏 - 仅在调试模式下编译
 * Debug-only macros - compiled only in debug builds
 */
#if UE_BUILD_DEBUG
	#define LE_LOG_DEBUG(Category, Format, ...)  LE_LOG(Category, Verbose, Format, ##__VA_ARGS__)
	#define LE_CLOG_DEBUG(Condition, Category, Format, ...)  LE_CLOG(Condition, Category, Verbose, Format, ##__VA_ARGS__)
#else
	#define LE_LOG_DEBUG(Category, Format, ...)  ((void)0)
	#define LE_CLOG_DEBUG(Condition, Category, Format, ...)  ((void)0)
#endif