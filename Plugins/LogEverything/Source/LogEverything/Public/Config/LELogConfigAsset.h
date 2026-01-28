// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "System/LELogTypes.h"
#include "LELogConfigAsset.generated.h"

/**
 * 日志系统配置数据资产
 * Log System Configuration Data Asset
 *
 * 用于在编辑器中配置日志系统的全局参数（BqLog 配置）
 * 分类配置通过 JSON 文件管理（Config/LogEverythingCategoryConfig.json）
 *
 * Used to configure log system global parameters (BqLog config) in the editor
 * Category configuration is managed via JSON file (Config/LogEverythingCategoryConfig.json)
 *
 * 配置流程：
 * 1. 在编辑器中创建 DataAsset，配置 BqLog 全局参数
 * 2. 设置 JSON 配置文件路径（默认 LogEverythingCategoryConfig.json）
 * 3. 运行时系统读取 JSON 文件，应用分类配置到 CategoryTree
 *
 * Configuration Flow:
 * 1. Create DataAsset in editor, configure BqLog global parameters
 * 2. Set JSON config file path (default: LogEverythingCategoryConfig.json)
 * 3. Runtime system reads JSON file and applies category config to CategoryTree
 */
UCLASS(BlueprintType)
class LOGEVERYTHING_API ULELogConfigAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * BqLog 全局配置
	 * 包含日志系统的全局参数：全局日志级别、缓冲区大小、日志文件路径、输出目标等
	 *
	 * BqLog Global Configuration
	 * Contains global parameters: global log level, buffer size, log file path, output targets, etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BqLog Configuration",
		meta = (DisplayName = "BqLog 全局配置", ToolTip = "BqLog 日志系统的全局配置参数"))
	FLEBqLogConfig BqLogConfiguration;

	/**
	 * 分类配置 JSON 文件路径
	 * 相对于插件 Config/ 目录的路径
	 *
	 * Category config JSON file path
	 * Relative to plugin Config/ directory
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category Configuration",
		meta = (DisplayName = "分类配置 JSON 路径",
			ToolTip = "分类配置 JSON 文件路径（相对于插件 Config/ 目录）\n默认值: LogEverythingCategoryConfig.json"))
	FString CategoryConfigJsonPath;

	/**
	 * 默认构造函数
	 * Default Constructor
	 */
	ULELogConfigAsset()
		: CategoryConfigJsonPath(TEXT("LogEverythingCategoryConfig.json"))
	{
	}

	/**
	 * 从 JSON 文件加载分类配置并应用到 CategoryTree
	 * Load category configuration from JSON file and apply to CategoryTree
	 *
	 * @param InCategoryTree 目标运行时分类树 / Target runtime category tree
	 * @return 加载和应用是否成功 / Whether loading and applying succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything|Configuration")
	bool LoadAndApplyCategoryConfigFromJson(class ULECategoryTree* InCategoryTree) const;

	/**
	 * 验证配置的合法性（检查 JSON 文件存在性和格式）
	 * Validate configuration (check JSON file existence and format)
	 *
	 * @param OutErrors 输出参数，存储所有验证错误信息
	 * @return 验证是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything|Configuration")
	bool ValidateConfiguration(TArray<FString>& OutErrors) const;

	/**
	 * 应用配置到运行时分类树
	 * Apply configuration to runtime category tree
	 *
	 * @param InCategoryTree 目标运行时分类树
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything|Configuration")
	void ApplyToCategoryTree(class ULECategoryTree* InCategoryTree) const;

private:
	/**
	 * 获取 JSON 配置文件的完整路径
	 * Get full path to the JSON config file
	 *
	 * @return JSON 文件的完整路径 / Full path to JSON file
	 */
	FString GetJsonConfigFullPath() const;

	/**
	 * 递归解析 JSON 分类节点并应用到 CategoryTree
	 * Recursively parse JSON category node and apply to CategoryTree
	 *
	 * @param JsonNode JSON 节点对象
	 * @param ParentPath 父路径
	 * @param InCategoryTree 目标分类树
	 * @param OutAppliedCount 已应用的节点计数
	 */
	void ParseAndApplyJsonNodeRecursive(
		const TSharedPtr<class FJsonObject>& JsonNode,
		const FString& ParentPath,
		class ULECategoryTree* InCategoryTree,
		int32& OutAppliedCount) const;

	/**
	 * 将字符串转换为 ELELogVerbosity
	 * Convert string to ELELogVerbosity
	 *
	 * @param LevelStr 级别字符串
	 * @param OutLevel 输出级别
	 * @return 转换是否成功
	 */
	static bool StringToLogVerbosity(const FString& LevelStr, ELELogVerbosity& OutLevel);

	/**
	 * 将字符串转换为 ELEEnabledState
	 * Convert string to ELEEnabledState
	 *
	 * @param StateStr 状态字符串
	 * @param OutState 输出状态
	 * @return 转换是否成功
	 */
	static bool StringToEnabledState(const FString& StateStr, ELEEnabledState& OutState);
};
