
#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/LogVerbosity.h"
#include "System/LELogTypes.h"
#include "Bridge/LEBqLogBridge.h"
#include "System/LELogSubsystem.h"
#include "LogEverythingUtils.generated.h"

#pragma region Log

DECLARE_LOG_CATEGORY_EXTERN(LogEverythingPlugin, Log, All)

#ifndef LE_SYSTEM_LOG
#define LE_SYSTEM_LOG(Format,...)\
{\
UE_LOG(LogEverythingPlugin, Log, TEXT("[LogEverything] " Format), ##__VA_ARGS__)\
}
#endif

#ifndef LE_SYSTEM_WARNING
#define LE_SYSTEM_WARNING(Format,...)\
{\
UE_LOG(LogEverythingPlugin, Warning, TEXT("[LogEverything] " Format), ##__VA_ARGS__)\
}
#endif

#ifndef LE_SYSTEM_ERROR
#define LE_SYSTEM_ERROR(Format,...)\
{\
UE_LOG(LogEverythingPlugin, Error, TEXT("[LogEverything] " Format), ##__VA_ARGS__)\
}
#endif

#pragma endregion

/**
 * LogEverything 静态函数库
 * LogEverything static function library
 *
 * 提供蓝图友好的静态接口来管理日志分类级别和设置
 * Provides Blueprint-friendly static interface for managing log category levels and settings
 */
UCLASS(BlueprintType, Category = "LogEverything")
class LOGEVERYTHING_API ULogEverythingUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 设置分类日志级别
	 * Set log category level
	 *
	 * @param WorldContext 世界上下文对象
	 * @param CategoryPath 分类路径（如 "LogRoot.Game.Combat.Skill"）
	 * @param Level 日志级别
	 * @param bPropagate 是否强制传播到所有子节点（默认为false，使用智能继承）
	 * @return 设置是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static bool SetLogCategoryLevel(const UObject* WorldContext, const FName& CategoryPath,
		ELELogVerbosity Level, bool bPropagate = false);

	/**
	 * 获取分类的有效日志级别
	 * Get effective log level for category
	 *
	 * @param WorldContext 世界上下文对象
	 * @param CategoryPath 分类路径
	 * @return 有效的日志级别
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static ELELogVerbosity GetEffectiveLogLevel(const UObject* WorldContext, const FName& CategoryPath);

	/**
	 * 检查分类是否应该输出指定级别的日志
	 * Check if category should log at specified level
	 *
	 * @param WorldContext 世界上下文对象
	 * @param CategoryName 分类名称
	 * @param Level 要检查的日志级别
	 * @return 是否应该输出日志
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static bool ShouldLogCategory(const UObject* WorldContext, const FName& CategoryName, ELELogVerbosity Level);

	/**
	 * 启用或禁用分类
	 * Enable or disable log category
	 *
	 * @param WorldContext 世界上下文对象
	 * @param CategoryPath 分类路径
	 * @param bEnabled 是否启用
	 * @param bPropagate 是否传播到子节点
	 * @return 设置是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static bool SetCategoryEnabled(const UObject* WorldContext, const FName& CategoryPath,
		bool bEnabled, bool bPropagate = false);

	/**
	 * 检查分类是否启用
	 * Check if category is enabled
	 *
	 * @param WorldContext 世界上下文对象
	 * @param CategoryPath 分类路径
	 * @return 是否启用
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static bool IsCategoryEnabled(const UObject* WorldContext, const FName& CategoryPath);

	/**
	 * 获取所有分类路径
	 * Get all category paths
	 *
	 * @param WorldContext 世界上下文对象
	 * @return 分类路径列表
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static TArray<FString> GetAllCategoryPaths(const UObject* WorldContext);

	/**
	 * 获取指定分类的子分类
	 * Get child categories of specified category
	 *
	 * @param WorldContext 世界上下文对象
	 * @param CategoryPath 父分类路径
	 * @return 子分类路径列表
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static TArray<FString> GetChildCategories(const UObject* WorldContext, const FName& CategoryPath);

	/**
	 * 重置所有分类到默认状态
	 * Reset all categories to default state
	 *
	 * @param WorldContext 世界上下文对象
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static void ResetToDefault(const UObject* WorldContext);

	/**
	 * 获取树的统计信息
	 * Get tree statistics
	 *
	 * @param WorldContext 世界上下文对象
	 * @param OutTotalNodes 总节点数
	 * @param OutMaxDepth 最大深度
	 * @param OutExplicitNodes 有显式设置的节点数
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static void GetTreeStatistics(const UObject* WorldContext, int32& OutTotalNodes, int32& OutMaxDepth, int32& OutExplicitNodes);

	/**
	 * 导出树结构为调试字符串
	 * Export tree structure as debug string
	 *
	 * @param WorldContext 世界上下文对象
	 * @return 调试字符串
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static FString ExportTreeDebugString(const UObject* WorldContext);

	/**
	 * 强制重新初始化分类树
	 * Force reinitialize category tree
	 *
	 * @param WorldContext 世界上下文对象
	 * @return 重新初始化是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static bool ReinitializeCategoryTree(const UObject* WorldContext);

	/**
	 * 从配置文件重新加载设置
	 * Reload settings from configuration file
	 *
	 * @param WorldContext 世界上下文对象
	 * @return 重新加载是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static bool ReloadLogSettings(const UObject* WorldContext);

	/**
	 * 检查LogEverything子系统是否已初始化
	 * Check if LogEverything subsystem is initialized
	 *
	 * @param WorldContext 世界上下文对象
	 * @return 是否已初始化
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything",
		meta = (DefaultToSelf = "WorldContext"))
	static bool IsLogSystemInitialized(const UObject* WorldContext);

	/**
	 * 便利函数：设置游戏相关分类的日志级别
	 * Convenience function: Set log level for game-related categories
	 *
	 * @param WorldContext 世界上下文对象
	 * @param Level 日志级别
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything|Convenience",
		meta = (DefaultToSelf = "WorldContext"))
	static void SetGameCategoriesLevel(const UObject* WorldContext, ELELogVerbosity Level);

	/**
	 * 便利函数：设置引擎相关分类的日志级别
	 * Convenience function: Set log level for engine-related categories
	 *
	 * @param WorldContext 世界上下文对象
	 * @param Level 日志级别
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything|Convenience",
		meta = (DefaultToSelf = "WorldContext"))
	static void SetEngineCategoriesLevel(const UObject* WorldContext, ELELogVerbosity Level);

	/**
	 * 便利函数：设置编辑器相关分类的日志级别
	 * Convenience function: Set log level for editor-related categories
	 *
	 * @param WorldContext 世界上下文对象
	 * @param Level 日志级别
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything|Convenience",
		meta = (DefaultToSelf = "WorldContext"))
	static void SetEditorCategoriesLevel(const UObject* WorldContext, ELELogVerbosity Level);

	/**
	 * 内部日志实现函数 - 统一的日志处理入口
	 * Internal logging implementation function - unified log processing entry point
	 *
	 * 负责完整的日志流程：级别判断 -> 实际打印
	 * Responsible for complete logging flow: level checking -> actual printing
	 *
	 * @param Category 分类对象
	 * @param Level 日志级别
	 * @param Format 格式化字符串
	 * @param Arguments 格式化参数
	 */
	template<typename CategoryType, typename FormatType, typename... Args>
	static void InternalLogImp(const CategoryType& Category, ELELogVerbosity Level,
		const FormatType& Format, const Args&... Arguments);

public:
	/**
	 * 获取LogEverything子系统实例
	 * Get LogEverything subsystem instance
	 *
	 * @param WorldContext 世界上下文对象
	 * @return 子系统实例，失败返回nullptr
	 */
	static ULELogSubsystem* GetLogSubsystem(const UObject* WorldContext);
};

// 前向声明
class FLEBqLogBridge;

// 模板函数实现 - 统一的日志处理入口
template<typename CategoryType, typename FormatType, typename... Args>
void ULogEverythingUtils::InternalLogImp(const CategoryType& Category, ELELogVerbosity Level,
	const FormatType& Format, const Args&... Arguments)
{
	// 第一步：通过Subsystem进行级别判断
	ULELogSubsystem* LogSubsystem = FLEBqLogBridge::Get().GetLogSubsystem();
	// 如果Subsystem未初始化，使用默认级别判断规则
	if (LogSubsystem && LogSubsystem->IsInitialized())
	{
		// 使用Subsystem进行级别判断
		if (!LogSubsystem->ShouldLogCategory(Category.GetCategoryName(), Level))
		{
			return; // 级别不匹配，直接返回，避免后续的字符串格式化
		}
	}
	else
	{
		// 后备方案：使用默认级别判断规则
		if (static_cast<uint8>(Level) < static_cast<uint8>(ELELogVerbosity::Info))
		{
			return;
		}
	}

	// 第二步：级别判断通过，直接调用Bridge进行实际的日志打印
	// 需要包含LEBqLogBridge.h才能调用LogWithTemplate
	FLEBqLogBridge::Get().LogWithTemplate(Category, Level, Format, Arguments...);
}