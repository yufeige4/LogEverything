// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Bridge/LEBqLogBridge.h"
#include "Utils/LogEverythingUtils.h"
#include "Engine/Engine.h"

// 前向声明
class ULELogSubsystem;

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
 * @param Category     C++ 标识符名称 (如 LogGameCombatSkill)
 * @param BqCategoryPath   BqLog 分类路径 (如 Game.Combat.Skill)
 *
 * 使用示例：
 * DECLARE_LE_CATEGORY_EXTERN(LogGameCombatSkill, Game.Combat.Skill);
 */
#define DECLARE_LE_CATEGORY_EXTERN(Category, BqCategoryPath) \
	class FLECategory##Category \
	{ \
	public: \
		static const FName GetCategoryName() { return FName(TEXT(#BqCategoryPath)); } \
		static auto GetCategoryHandle(const bq::LogEverythingLogger& Logger) -> decltype((Logger.cat.BqCategoryPath)) \
		{ \
			return Logger.cat.BqCategoryPath; \
		} \
	}; \
	extern FLECategory##Category Category;

/**
 * LE 分类定义宏 - 在源文件中定义日志分类（仅负责创建类实例）
 * LE Category definition macro - define log category in source files (class instance creation only)
 *
 * 类似于 UE 的 DEFINE_LOG_CATEGORY，只负责创建实例和方法实现，不包含声明
 *
 * @param Category  C++ 标识符名称 (如 LogGameCombatSkill)
 *
 * 使用示例：
 * DEFINE_LE_CATEGORY(LogGameCombatSkill);
 *
 * 注意：必须先在头文件中使用 DECLARE_LE_CATEGORY_EXTERN 声明该分类
 */
#define DEFINE_LE_CATEGORY(Category) \
	FLECategory##Category Category; \


/**
 * 核心日志宏 - 使用声明的分类，通过统一的日志处理流程
 * Core logging macro - uses declared categories with unified log processing flow
 *
 * 使用方式：
 * LE_LOG(LogGameCombatSkill, Warning, TEXT("Player {} cast skill {}"), PlayerName, SkillName);
 *
 * 架构优势：
 * - 通过 ULELogUtils::InternalLogImp 统一处理日志流程
 * - 先进行级别判断（LELogSubsystem），再进行实际打印（Bridge）
 * - 避免不必要的字符串格式化，提升性能
 * - 职责分离：Subsystem负责分类管理，Bridge负责BqLog交互
 *
 * @param Category   已声明的分类 (如 LogGameCombatSkill)
 * @param Verbosity  日志级别 (Fatal, Error, Warning, Log, Verbose, VeryVerbose)
 * @param Format     格式化字符串
 * @param ...        格式化参数
 */
#define LE_LOG(Category, Verbosity, Format, ...) \
	ULogEverythingUtils::InternalLogImp(Category, ELELogVerbosity::Verbosity, Format, ##__VA_ARGS__) \
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
 */
#define LE_LOG_FATAL(CategoryHandle, Format, ...)    LE_LOG(CategoryHandle, Fatal, Format, ##__VA_ARGS__)
#define LE_LOG_ERROR(CategoryHandle, Format, ...)   LE_LOG(CategoryHandle, Error, Format, ##__VA_ARGS__)
#define LE_LOG_WARNING(CategoryHandle, Format, ...) LE_LOG(CategoryHandle, Warning, Format, ##__VA_ARGS__)
#define LE_LOG_INFO(CategoryHandle, Format, ...)     LE_LOG(CategoryHandle, Info, Format, ##__VA_ARGS__)

/**
 * 运行时配置宏 - 通过 ULELogUtils 调用 LELogSubsystem
 * Runtime configuration macros - call LELogSubsystem through ULELogUtils
 *
 * 使用方式：
 * LE_SET_CATEGORY_LEVEL(LogGameCombatSkill, Warning);
 * LE_ENABLE_CATEGORY(LogGameCombatSkill);
 * LE_DISABLE_CATEGORY(LogGameCombatSkill);
 */
#define LE_SET_CATEGORY_LEVEL(Category, Level) \
	ULogEverythingUtils::SetLogCategoryLevel(GEngine, Category.GetCategoryName(), ELELogVerbosity::Level)

#define LE_SET_GLOBAL_LEVEL(Level) \
	{ \
		if (ULELogSubsystem* LogSubsystem = ULogEverythingUtils::GetLogSubsystem(GEngine)) \
		{ \
			LogSubsystem->SetGlobalLogLevel(ELELogVerbosity::Level); \
		} \
	} \

#define LE_ENABLE_CATEGORY(Category) \
	ULogEverythingUtils::SetCategoryEnabled(GEngine, Category.GetCategoryName(), true)

#define LE_DISABLE_CATEGORY(Category) \
	ULogEverythingUtils::SetCategoryEnabled(GEngine, Category.GetCategoryName(), false)

#define LE_FLUSH_LOGS() \
	FLEBqLogBridge::Get().FlushLogs()

/**
 * 调试专用宏 - 仅在非正式构建中编译
 * Debug-only macros - compiled only in non-official builds
 */
#if (!UE_BUILD_SHIPPING && !UE_BUILD_TEST)
	#define LE_CLOG_DEBUG(Condition, CategoryHandle, Format, ...)  LE_CLOG(Condition, CategoryHandle, Debug, Format, ##__VA_ARGS__)
	#define LE_LOG_DEBUG(CategoryHandle, Format, ...)  LE_LOG(CategoryHandle, Debug, Format, ##__VA_ARGS__)
	#define LE_LOG_VERBOSE(CategoryHandle, Format, ...)  LE_LOG(CategoryHandle, Verbose, Format, ##__VA_ARGS__)
#else
	#define LE_LOG_DEBUG(CategoryHandle, Format, ...)  ((void)0)
	#define LE_LOG_VERBOSE(CategoryHandle, Format, ...)  ((void)0)
	#define LE_CLOG_DEBUG(Condition, CategoryHandle, Format, ...)  ((void)0)
#endif