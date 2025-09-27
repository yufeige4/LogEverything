// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "System/LELogTypes.h"
#include "Category//LECategoryTree.h"
#include "Bridge/LEBqLogBridge.h"
#include "LELogSubsystem.generated.h"

namespace LogEverything
{
	namespace ConsoleVariable
	{
		/** Controls whether category debug information is printed */
		static TAutoConsoleVariable<bool> DebugLogCategory(
			TEXT("LogEverything.Debug.LogCategory"),
			false,
			TEXT("Controls whether LogEverything prints category debug details\n")
			TEXT("0: Disable category debug information (default)\n")
			TEXT("1: Enable category debug information"),
			ECVF_Default
		);
	}
}

/**
 * LogEverything 子系统
 * 管理整个日志系统的生命周期和配置
 */
UCLASS()
class LOGEVERYTHING_API ULELogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	ULELogSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 获取日志子系统实例 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	static ULELogSubsystem* Get(const UObject* WorldContextObject);

	/** 设置特定分类的日志级别 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool SetCategoryLevel(const FName& CategoryPath, ELELogVerbosity Level, bool bPropagate = false);

	/** 获取特定分类的有效日志级别 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	ELELogVerbosity GetEffectiveLevel(const FName& CategoryPath) const;

	/** 判断是否应该记录特定分类的日志 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool ShouldLogCategory(const FName& CategoryName, ELELogVerbosity Level) const;

	/** 启用或禁用特定分类 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool SetCategoryEnabled(const FName& CategoryPath, bool bEnabled, bool bPropagate = false);

	/** 检查特定分类是否已启用 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool IsCategoryEnabled(const FName& CategoryPath) const;

	/** 获取所有分类路径 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	TArray<FString> GetAllCategoryPaths() const;

	/** 获取指定分类的子分类 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	TArray<FString> GetChildCategories(const FName& CategoryPath) const;

	/** 重置所有分类到默认状态 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	void ResetToDefault();

	/** 获取分类树统计信息 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	void GetTreeStatistics(int32& OutTotalNodes, int32& OutMaxDepth, int32& OutExplicitNodes) const;

	/** 导出分类树调试字符串 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	FString ExportTreeDebugString() const;

	/** 重新初始化分类树 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool ReinitializeCategoryTree();

	/** 重新加载日志设置 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool ReloadLogSettings();

	/** 设置全局日志级别 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	void SetGlobalLogLevel(ELELogVerbosity Level);

	/** 获取全局日志级别 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	ELELogVerbosity GetGlobalLogLevel() const;

	/** 获取分类树实例 */
	FORCEINLINE ULECategoryTree* GetCategoryTree() const { return CategoryTree; }

	/** 检查是否已初始化 */
	FORCEINLINE bool IsInitialized() const { return bIsInitialized; }

private:
	/** 初始化分类树 */
	bool InitializeCategoryTree();

	/** 加载日志设置 */
	bool LoadLogSettings();

	/**
	 * 从BqLog接口获取分类路径
	 * @param OutCategoryPaths 输出的分类路径数组
	 * @return 获取是否成功
	 */
	bool ParseBqLogCategories(TArray<FString>& OutCategoryPaths) const;

	/**
	 * 获取预定义的分类路径（优化：使用out reference避免数组拷贝）
	 * @param OutCategoryPaths 输出的分类路径数组
	 * @return 获取是否成功
	 */
	bool GetPredefinedCategoryPaths(TArray<FString>& OutCategoryPaths) const;

	/** 应用分类配置 */
	void ApplyDefaultCategoryConfigurations();

	/** 清理资源 */
	void Cleanup();

private:
	/** 分类树实例 */
	UPROPERTY()
	TObjectPtr<ULECategoryTree> CategoryTree;

	/** 初始化状态 */
	bool bIsInitialized;

	/** 全局日志级别 */
	ELELogVerbosity GlobalLogLevel;

	/** 静态初始化标志（防止编辑器中重复初始化） */
	static bool bStaticInitialized;
};