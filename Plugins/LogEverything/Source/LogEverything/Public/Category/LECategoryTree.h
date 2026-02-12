// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Logging/LogVerbosity.h"
#include "System/LELogTypes.h"
#include "LECategoryTree.generated.h"

/**
 * 日志分类节点结构体 - 轻量级USTRUCT实现
 * Log category node structure - lightweight USTRUCT implementation
 *
 * 用于构建树状的分类级别管理系统，支持智能继承和高效查询
 * Used to build tree-based category level management system with smart inheritance and efficient queries
 */
USTRUCT(BlueprintType)
struct LOGEVERYTHING_API FLECategoryNode
{
	GENERATED_BODY()

public:
	
	/** 分类子名称（相对名称，如"Skill"而不是"Game.Combat.Skill"） */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	FName CategorySubName;

	/** 分类完整名称（如"Game.Combat.Skill"）- 使用FName优化内存 */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	FName CategoryFullName;

	/** 显式设置的日志级别 */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	ELELogVerbosity ExplicitLevel;

	/** 有效的日志级别（考虑继承后的最终级别） */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	ELELogVerbosity EffectiveLevel;

	/** 是否有显式设置的级别 */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	bool bHasExplicitLevel;

	/**
	 * 显式设置的启用状态
	 * Explicitly set enabled state
	 *
	 * 控制当前节点的启用状态，支持三种模式：
	 * - NotSet (默认): 未显式设置，继承父节点的启用状态
	 *   例如：父节点启用时子节点也启用，父节点禁用时子节点也禁用
	 * - Disabled: 显式禁用，即使父节点启用，当前节点也保持禁用
	 *   例如：临时禁用某个子分类的日志输出
	 * - Enabled: 显式启用，但父节点禁用时仍会被强制禁用（继承父节点的禁用状态）
	 *   例如：在父节点启用的前提下，确保当前节点也启用
	 *
	 * 启用状态继承规则：
	 * 1. 如果父节点禁用，所有子节点强制禁用（无论子节点 ExplicitEnabled 设置为什么）
	 * 2. 如果父节点启用，子节点的实际状态（bIsEnabled）取决于 ExplicitEnabled：
	 *    - NotSet: 继承父节点的启用状态（bIsEnabled = 父节点的 bIsEnabled）
	 *    - Disabled: 禁用（bIsEnabled = false）
	 *    - Enabled: 启用（bIsEnabled = true）
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	ELEEnabledState ExplicitEnabled;

	/**
	 * 是否启用该分类（有效的启用状态）
	 * Effective enabled state after inheritance
	 *
	 * 这是考虑继承后的最终启用状态，由 ExplicitEnabled 和父节点状态共同决定
	 * 注意：这个值不应该直接设置，而是通过 ExplicitEnabled 和继承逻辑自动计算
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	bool bIsEnabled;

	/** 父节点在数组中的索引 */
	UPROPERTY(BlueprintReadOnly, Category = "Structure")
	int32 ParentIndex;

	/** 子节点在数组中的索引列表 */
	UPROPERTY(BlueprintReadOnly, Category = "Structure")
	TArray<int32> ChildIndices;

	/** 深度级别（根节点为0） */
	UPROPERTY(BlueprintReadOnly, Category = "Structure")
	int32 Depth;

public:

	FLECategoryNode()
	: CategorySubName(NAME_None)
	, ExplicitLevel(ELELogVerbosity::NoLogging)
	, EffectiveLevel(ELELogVerbosity::Info)
	, bHasExplicitLevel(false)
	, ExplicitEnabled(ELEEnabledState::NotSet)
	, bIsEnabled(true)
	, ParentIndex(INDEX_NONE)
	, Depth(0)
	{
	}
	/**
	 * 设置显式日志级别
	 * @param Level 要设置的级别
	 * @param bUpdateEffective 是否同时更新有效级别
	 */
	void SetExplicitLevel(ELELogVerbosity Level, bool bUpdateEffective = true)
	{
		ExplicitLevel = Level;
		bHasExplicitLevel = true;
		if (bUpdateEffective)
		{
			EffectiveLevel = Level;
		}
	}

	/**
	 * 清除显式级别设置
	 * @param InheritedLevel 从父节点继承的级别
	 */
	void ClearExplicitLevel(ELELogVerbosity InheritedLevel = ELELogVerbosity::Info)
	{
		bHasExplicitLevel = false;
		ExplicitLevel = ELELogVerbosity::NoLogging;
		EffectiveLevel = InheritedLevel;
	}

	/**
	 * 更新有效级别
	 * @param NewEffectiveLevel 新的有效级别
	 */
	void UpdateEffectiveLevel(ELELogVerbosity NewEffectiveLevel)
	{
		EffectiveLevel = NewEffectiveLevel;
	}

	/**
	 * 设置分类启用状态
	 * @param bEnabled 是否启用
	 */
	void SetEnabled(bool bEnabled)
	{
		bIsEnabled = bEnabled;
	}

	/**
	 * 添加子节点索引
	 * @param ChildIndex 子节点在数组中的索引
	 */
	void AddChild(int32 ChildIndex)
	{
		if (!ChildIndices.Contains(ChildIndex))
		{
			ChildIndices.Add(ChildIndex);
		}
	}

	/**
	 * 移除子节点索引
	 * @param ChildIndex 要移除的子节点索引
	 */
	void RemoveChild(int32 ChildIndex)
	{
		ChildIndices.Remove(ChildIndex);
	}

	/**
	 * 检查是否为根节点
	 */
	bool IsRoot() const
	{
		return ParentIndex == INDEX_NONE;
	}

	/**
	 * 检查是否为叶子节点
	 */
	bool IsLeaf() const
	{
		return ChildIndices.Num() == 0;
	}

	/**
	 * 获取节点的调试信息字符串
	 */
	FString GetDebugString() const
	{
		return FString::Printf(TEXT("Node[%s]: Explicit=%s(%s), Effective=%s, Enabled=%s, Parent=%d, Children=%d"),
			*CategoryFullName.ToString(),
			bHasExplicitLevel ? TEXT("Yes") : TEXT("No"),
			bHasExplicitLevel ? *UEnum::GetValueAsString(ExplicitLevel) : TEXT("None"),
			*UEnum::GetValueAsString(EffectiveLevel),
			bIsEnabled ? TEXT("Yes") : TEXT("No"),
			ParentIndex,
			ChildIndices.Num());
	}
};

/**
 * 日志分类树管理器 - UObject实现，支持UE反射系统
 * Log category tree manager - UObject implementation with UE reflection system support
 *
 * 负责管理整个分类树结构，提供高效的级别查询和修改功能
 * Manages the entire category tree structure, providing efficient level query and modification features
 */
UCLASS(BlueprintType, Category = "LogEverything")
class LOGEVERYTHING_API ULECategoryTree : public UObject
{
	GENERATED_BODY()

public:
	ULECategoryTree();

protected:
	// ========================================================================
	// 友元声明 - 允许配置类直接访问 Nodes 进行高效配置应用
	// Friend declarations - Allow config class to directly access Nodes for efficient configuration application
	// ========================================================================
	friend class ULELogConfigAsset;

	/** 所有节点的数组存储（索引即为节点ID） */
	UPROPERTY(BlueprintReadOnly, Category = "Tree Structure")
	TArray<FLECategoryNode> Nodes;

	/** 路径到节点索引的快速映射（使用FName提升性能） */
	UPROPERTY()
	TMap<FName, int32> PathToIndexMap;

	/** 根节点索引 */
	UPROPERTY(BlueprintReadOnly, Category = "Tree Structure")
	int32 RootNodeIndex;

	/**
	 * 默认全局日志级别（构造时的初始值）
	 * Default global log level (initial value at construction)
	 *
	 * 树构造时使用此值初始化根节点和子节点的有效级别
	 * Used to initialize root node and children's effective level when tree is constructed
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Tree Structure")
	ELELogVerbosity DefaultGlobalLevel;

	/**
	 * 当前全局日志级别（运行时动态调整）
	 * Current global log level (dynamically adjusted at runtime)
	 *
	 * 运行时通过 SetCurrentGlobalLevel 修改，ShouldLogCategory 会同时考虑此值和节点级别
	 * Modified at runtime via SetCurrentGlobalLevel, ShouldLogCategory considers both this and node level
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Tree Structure")
	ELELogVerbosity CurrentGlobalLevel;

public:
	/**
	 * 初始化分类树
	 * @param CategoryPaths 所有分类路径列表
	 * @return 初始化是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool InitializeTree(const TArray<FString>& CategoryPaths);

	/**
	 * 设置分类日志级别
	 * @param CategoryPath 分类路径
	 * @param Level 日志级别
	 * @param bPropagate 是否强制传播到所有子节点
	 * @return 设置是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool SetCategoryLevel(const FString& CategoryPath, ELELogVerbosity Level, bool bPropagate = false);

	/**
	 * 获取分类的有效日志级别
	 * @param CategoryPath 分类路径
	 * @return 有效的日志级别
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	ELELogVerbosity GetEffectiveLevel(const FString& CategoryPath) const;

	/**
	 * 检查分类是否应该输出指定级别的日志
	 * @param CategoryName 分类名称
	 * @param Level 要检查的日志级别
	 * @return 是否应该输出日志
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	bool ShouldLogCategory(const FName& CategoryName, ELELogVerbosity Level) const;

	/**
	 * 启用或禁用分类
	 * @param CategoryPath 分类路径
	 * @param bEnabled 是否启用
	 * @param bPropagate 是否传播到子节点
	 * @return 设置是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	bool SetCategoryEnabled(const FString& CategoryPath, bool bEnabled, bool bPropagate = false);

	/**
	 * 检查分类是否启用
	 * @param CategoryPath 分类路径
	 * @return 是否启用
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	bool IsCategoryEnabled(const FString& CategoryPath) const;

	/**
	 * 获取所有分类路径
	 * @return 分类路径列表
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	TArray<FString> GetAllCategoryPaths() const;

	/**
	 * 获取指定分类的子分类
	 * @param CategoryPath 父分类路径
	 * @return 子分类路径列表
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	TArray<FString> GetChildCategories(const FString& CategoryPath) const;

	/**
	 * 重置所有分类到默认状态
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	void ResetToDefault();

	/**
	 * 设置默认全局日志级别（仅在初始化时使用）
	 * Set default global log level (only used during initialization)
	 *
	 * 此方法在树构造时调用，设置根节点和子节点的初始有效级别
	 * 同时也会初始化 CurrentGlobalLevel
	 * Called during tree construction, sets initial effective level for root and children
	 * Also initializes CurrentGlobalLevel
	 *
	 * @param NewLevel 新的默认全局日志级别 / New default global log level
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	void SetDefaultGlobalLevel(ELELogVerbosity NewLevel);

	/**
	 * 获取默认全局日志级别
	 * Get default global log level
	 *
	 * @return 默认全局日志级别 / Default global log level
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	ELELogVerbosity GetDefaultGlobalLevel() const;

	/**
	 * 设置当前全局日志级别（运行时动态调整）
	 * Set current global log level (runtime dynamic adjustment)
	 *
	 * 运行时调用此方法修改全局日志级别，不会影响树结构
	 * ShouldLogCategory 会取 CurrentGlobalLevel 和节点级别中更严格的那个
	 * Call this at runtime to modify global log level without affecting tree structure
	 * ShouldLogCategory will use the stricter of CurrentGlobalLevel and node level
	 *
	 * @param NewLevel 新的当前全局日志级别 / New current global log level
	 */
	UFUNCTION(BlueprintCallable, Category = "LogEverything")
	void SetCurrentGlobalLevel(ELELogVerbosity NewLevel);

	/**
	 * 获取当前全局日志级别
	 * Get current global log level
	 *
	 * @return 当前全局日志级别 / Current global log level
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	ELELogVerbosity GetCurrentGlobalLevel() const;

	/**
	 * 获取树的统计信息
	 * @param OutTotalNodes 总节点数
	 * @param OutMaxDepth 最大深度
	 * @param OutExplicitNodes 有显式设置的节点数
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	void GetTreeStatistics(int32& OutTotalNodes, int32& OutMaxDepth, int32& OutExplicitNodes) const;

	/**
	 * 导出树结构为调试字符串
	 * @return 调试字符串
	 */
	UFUNCTION(BlueprintPure, Category = "LogEverything")
	FString ExportTreeDebugString() const;

	/**
	 * 获取节点访问器 (const版本, 按路径)
	 * Get node accessor (const version, by path)
	 *
	 * 根据分类路径获取对应的节点指针，返回 const 指针以防止外部修改
	 * Returns a const pointer to the node matching the category path to prevent external modification
	 *
	 * @param CategoryPath 分类路径 / Category path
	 * @return 节点的 const 指针，如果未找到则返回 nullptr / Const pointer to the node, or nullptr if not found
	 */
	const FLECategoryNode* GetNode(const FString& CategoryPath) const;

	/**
	 * 获取节点访问器 (const版本, 按索引)
	 * Get node accessor (const version, by index)
	 *
	 * 根据节点索引获取对应的节点指针，返回 const 指针以防止外部修改
	 * 这个方法比按路径查找更高效，因为是直接索引访问
	 * Returns a const pointer to the node at the specified index to prevent external modification
	 * This method is more efficient than path-based lookup as it uses direct index access
	 *
	 * @param NodeIndex 节点索引 / Node index
	 * @return 节点的 const 指针，如果索引无效则返回 nullptr / Const pointer to the node, or nullptr if index is invalid
	 */
	const FLECategoryNode* GetNode(int32 NodeIndex) const;

	/**
	 * 获取可修改的节点访问器 (按路径)
	 * Get mutable node accessor (by path)
	 *
	 * 根据分类路径获取对应的节点指针，返回非 const 指针允许修改节点
	 * 注意：此方法仅供 ULECategoryTree 内部使用，外部调用者应该使用 const 版本的 GetNode
	 * Returns a non-const pointer to the node matching the category path to allow modification
	 * Note: This method is for internal use by ULECategoryTree only, external callers should use the const GetNode version
	 *
	 * @param CategoryPath 分类路径 / Category path
	 * @return 节点的非 const 指针，如果未找到则返回 nullptr / Non-const pointer to the node, or nullptr if not found
	 */
	FLECategoryNode* GetMutableNode(const FString& CategoryPath);

	/**
	 * 获取可修改的节点访问器 (按索引)
	 * Get mutable node accessor (by index)
	 *
	 * 根据节点索引获取对应的节点指针，返回非 const 指针允许修改节点
	 * 这个方法比按路径查找更高效，因为是直接索引访问
	 * 注意：此方法仅供 ULECategoryTree 内部使用，外部调用者应该使用 const 版本的 GetNode
	 * Returns a non-const pointer to the node at the specified index to allow modification
	 * This method is more efficient than path-based lookup as it uses direct index access
	 * Note: This method is for internal use by ULECategoryTree only, external callers should use the const GetNode version
	 *
	 * @param NodeIndex 节点索引 / Node index
	 * @return 节点的非 const 指针，如果索引无效则返回 nullptr / Non-const pointer to the node, or nullptr if index is invalid
	 */
	FLECategoryNode* GetMutableNode(int32 NodeIndex);

protected:
	/**
	 * 创建根节点
	 */
	void CreateRootNode();

	/**
	 * 查找或创建节点, 仅在构建树时使用!!!
	 * @param CategoryPath 分类路径
	 * @return 节点索引，失败返回INDEX_NONE
	 */
	int32 FindOrCreateNode(const FString& CategoryPath);

	/**
	 * 查找节点索引
	 * @param CategoryPath 分类路径
	 * @return 节点索引，未找到返回INDEX_NONE
	 */
	int32 FindNodeIndex(const FString& CategoryPath) const;

	/**
	 * 递归更新子节点的有效级别
	 * @param NodeIndex 父节点索引
	 * @param NewLevel 新的级别
	 * @param bForceOverride 是否强制覆盖显式设置
	 */
	void InternalUpdateChildrenEffectiveLevels(int32 NodeIndex, ELELogVerbosity NewLevel, bool bForceOverride);

	/**
	 * 递归更新子节点的启用状态
	 * @param NodeIndex 父节点索引
	 * @param bEnabled 启用状态
	 */
	void InternalUpdateChildrenEnabledState(int32 NodeIndex, bool bEnabled);

	/**
	 * 分割路径为组件
	 * @param Path 完整路径
	 * @return 路径组件数组
	 */
	TArray<FString> SplitPath(const FString& Path) const;

	/**
	 * 构建路径字符串
	 * @param Components 路径组件
	 * @param EndIndex 结束索引（不包含）
	 * @return 构建的路径
	 */
	FString BuildPath(const TArray<FString>& Components, int32 EndIndex) const;

	/**
	 * 验证节点索引的有效性
	 * @param NodeIndex 要验证的索引
	 * @return 是否有效
	 */
	bool IsValidNodeIndex(int32 NodeIndex) const;

	/**
	 * 递归收集调试信息
	 * @param NodeIndex 节点索引
	 * @param Depth 当前深度
	 * @param OutString 输出字符串
	 */
	void CollectDebugInfo(int32 NodeIndex, int32 Depth, FString& OutString) const;
};