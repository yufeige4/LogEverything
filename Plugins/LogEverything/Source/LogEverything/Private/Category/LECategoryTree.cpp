// Copyright Epic Games, Inc. All Rights Reserved.

#include "Category/LECategoryTree.h"
#include "Utils/LogEverythingUtils.h"
#include "Engine/Engine.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

ULECategoryTree::ULECategoryTree()
	: RootNodeIndex(INDEX_NONE)
	, DefaultGlobalLevel(ELELogVerbosity::NotSet)
	, CurrentGlobalLevel(ELELogVerbosity::NotSet)
{
}

bool ULECategoryTree::InitializeTree(const TArray<FString>& CategoryPaths)
{
	// 清空现有数据
	Nodes.Empty();
	PathToIndexMap.Empty();

	// 重置全局级别
	DefaultGlobalLevel = ELELogVerbosity::NotSet;
	CurrentGlobalLevel = ELELogVerbosity::NotSet;

	// 创建根节点
	CreateRootNode();

	// 添加所有分类路径
	bool bSuccess = true;
	for (const FString& Path : CategoryPaths)
	{
		if (!Path.IsEmpty())
		{
			int32 NodeIndex = FindOrCreateNode(Path);
			if (NodeIndex == INDEX_NONE)
			{
				LE_SYSTEM_WARNING(TEXT("Failed to create node for path: %s"), *Path);
				bSuccess = false;
			}
		}
	}

	LE_SYSTEM_LOG(TEXT("Category tree initialized with %d nodes, success: %s"),
		Nodes.Num(), bSuccess ? TEXT("true") : TEXT("false"));

	return bSuccess;
}

bool ULECategoryTree::SetCategoryLevel(const FString& CategoryPath, ELELogVerbosity Level, bool bPropagate)
{
	int32 NodeIndex = FindNodeIndex(CategoryPath);

	if (!ensure(IsValidNodeIndex(NodeIndex)))
	{
		LE_SYSTEM_WARNING(TEXT("[SetCategoryLevel] Cannot find node for path: %s"), *CategoryPath);
		return false;
	}

	// 设置节点的显式级别
	Nodes[NodeIndex].SetExplicitLevel(Level, true);

	// 根据传播选项更新子节点
	if (bPropagate)
	{
		// 强制传播：覆盖所有子节点
		InternalUpdateChildrenEffectiveLevels(NodeIndex, Level, true);
	}
	else
	{
		// 智能继承：仅更新没有显式设置的子节点
		InternalUpdateChildrenEffectiveLevels(NodeIndex, Level, false);
	}

	LE_SYSTEM_LOG(TEXT("Set category level: %s = %s (propagate: %s)"),
		*CategoryPath, *UEnum::GetValueAsString(Level), bPropagate ? TEXT("true") : TEXT("false"));

	return true;
}

ELELogVerbosity ULECategoryTree::GetEffectiveLevel(const FString& CategoryPath) const
{
	int32 NodeIndex = FindNodeIndex(CategoryPath);
	if (IsValidNodeIndex(NodeIndex))
	{
		// 直接返回枚举类型
		return Nodes[NodeIndex].EffectiveLevel;
	}

	// 如果找不到节点，返回默认级别
	return ELELogVerbosity::Info;
}

bool ULECategoryTree::ShouldLogCategory(const FName& CategoryName, ELELogVerbosity Level) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(LE_Tree_ShouldLogCategory);

	int32 NodeIndex;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(LE_Tree_FindCategory);
		const int32* FoundIndex = PathToIndexMap.Find(CategoryName);
		NodeIndex = FoundIndex ? *FoundIndex : INDEX_NONE;
	}

	if (IsValidNodeIndex(NodeIndex))
	{
		const FLECategoryNode& Node = Nodes[NodeIndex];

		// 检查分类是否启用
		if (!Node.bIsEnabled)
		{
			return false;
		}

		// 取 CurrentGlobalLevel 和节点 EffectiveLevel 中更严格的那个
		// 级别越高（数值越大）越严格，只显示更重要的日志
		// Take the stricter of CurrentGlobalLevel and node EffectiveLevel
		// Higher level (larger value) is stricter, showing only more important logs
		ELELogVerbosity EffectiveThreshold = Node.EffectiveLevel;
		if (CurrentGlobalLevel != ELELogVerbosity::NotSet &&
			static_cast<uint8>(CurrentGlobalLevel) > static_cast<uint8>(EffectiveThreshold))
		{
			EffectiveThreshold = CurrentGlobalLevel;
		}

		// 日志级别必须 >= 有效阈值才输出
		return static_cast<uint8>(Level) >= static_cast<uint8>(EffectiveThreshold);
	}

	// 如果找不到节点，使用 CurrentGlobalLevel 或默认 Warning
	ELELogVerbosity FallbackLevel = (CurrentGlobalLevel != ELELogVerbosity::NotSet)
		? CurrentGlobalLevel
		: ELELogVerbosity::Warning;
	return static_cast<uint8>(Level) >= static_cast<uint8>(FallbackLevel);
}

bool ULECategoryTree::SetCategoryEnabled(const FString& CategoryPath, bool bEnabled, bool bPropagate)
{
	int32 NodeIndex = FindNodeIndex(CategoryPath);

	if (!ensure(IsValidNodeIndex(NodeIndex)))
	{
		LE_SYSTEM_WARNING(TEXT("[SetCategoryEnabled] Cannot find node for path: %s"), *CategoryPath);
		return false;
	}

	// 设置节点启用状态
	Nodes[NodeIndex].SetEnabled(bEnabled);

	// 如果需要传播到子节点
	if (bPropagate)
	{
		InternalUpdateChildrenEnabledState(NodeIndex, bEnabled);
	}

	LE_SYSTEM_LOG(TEXT("Set category enabled: %s = %s (propagate: %s)"),
		*CategoryPath, bEnabled ? TEXT("true") : TEXT("false"), bPropagate ? TEXT("true") : TEXT("false"));

	return true;
}

bool ULECategoryTree::IsCategoryEnabled(const FString& CategoryPath) const
{
	int32 NodeIndex = FindNodeIndex(CategoryPath);
	if (IsValidNodeIndex(NodeIndex))
	{
		return Nodes[NodeIndex].bIsEnabled;
	}

	// 默认启用
	return true;
}

TArray<FString> ULECategoryTree::GetAllCategoryPaths() const
{
	TArray<FString> Paths;
	Paths.Reserve(Nodes.Num());

	for (const FLECategoryNode& Node : Nodes)
	{
		if (!Node.IsRoot()) // 跳过根节点
		{
			Paths.Add(Node.CategoryFullName.ToString());
		}
	}

	return Paths;
}

TArray<FString> ULECategoryTree::GetChildCategories(const FString& CategoryPath) const
{
	TArray<FString> ChildPaths;
	int32 NodeIndex = FindNodeIndex(CategoryPath);

	if (IsValidNodeIndex(NodeIndex))
	{
		const FLECategoryNode& Node = Nodes[NodeIndex];
		ChildPaths.Reserve(Node.ChildIndices.Num());

		for (int32 ChildIndex : Node.ChildIndices)
		{
			if (IsValidNodeIndex(ChildIndex))
			{
				ChildPaths.Add(Nodes[ChildIndex].CategoryFullName.ToString());
			}
		}
	}

	return ChildPaths;
}

void ULECategoryTree::ResetToDefault()
{
	// 重置全局级别到默认状态
	// DefaultGlobalLevel 保持原值（这是配置加载时设置的初始值）
	// CurrentGlobalLevel 重置为 DefaultGlobalLevel（用户运行时可能修改过）
	CurrentGlobalLevel = DefaultGlobalLevel;

	// 重置所有节点到默认状态
	for (FLECategoryNode& Node : Nodes)
	{
		Node.ClearExplicitLevel(ELELogVerbosity::Info);
		Node.SetEnabled(true);
	}

	// 重新计算所有有效级别，使用 DefaultGlobalLevel 作为基础级别
	ELELogVerbosity BaseLevel = (DefaultGlobalLevel != ELELogVerbosity::NotSet)
		? DefaultGlobalLevel
		: ELELogVerbosity::Info;

	if (IsValidNodeIndex(RootNodeIndex))
	{
		InternalUpdateChildrenEffectiveLevels(RootNodeIndex, BaseLevel, true);
	}

	LE_SYSTEM_LOG(TEXT("Category tree reset to default"));
}

void ULECategoryTree::SetDefaultGlobalLevel(ELELogVerbosity NewLevel)
{
	// 记录旧的默认全局级别用于日志输出
	ELELogVerbosity OldLevel = DefaultGlobalLevel;

	// 更新默认全局级别
	DefaultGlobalLevel = NewLevel;

	// 初始化时同时设置 CurrentGlobalLevel
	CurrentGlobalLevel = NewLevel;

	// 如果根节点存在且未设置显式级别，需要更新根节点的有效级别并传播到子节点
	if (IsValidNodeIndex(RootNodeIndex))
	{
		FLECategoryNode& RootNode = Nodes[RootNodeIndex];

		// 只有在根节点没有显式设置级别时才更新
		if (!RootNode.bHasExplicitLevel)
		{
			// 更新根节点的有效级别
			RootNode.UpdateEffectiveLevel(NewLevel);

			// 传播更改到所有子节点（仅更新没有显式设置的子节点）
			InternalUpdateChildrenEffectiveLevels(RootNodeIndex, NewLevel, false);

			LE_SYSTEM_LOG(TEXT("SetDefaultGlobalLevel: Updated from %s to %s, propagated to tree"),
				*UEnum::GetValueAsString(OldLevel), *UEnum::GetValueAsString(NewLevel));
		}
		else
		{
			LE_SYSTEM_LOG(TEXT("SetDefaultGlobalLevel: Updated from %s to %s, root has explicit level"),
				*UEnum::GetValueAsString(OldLevel), *UEnum::GetValueAsString(NewLevel));
		}
	}
	else
	{
		LE_SYSTEM_WARNING(TEXT("SetDefaultGlobalLevel: %s -> %s, tree not initialized"),
			*UEnum::GetValueAsString(OldLevel), *UEnum::GetValueAsString(NewLevel));
	}
}

ELELogVerbosity ULECategoryTree::GetDefaultGlobalLevel() const
{
	return DefaultGlobalLevel;
}

void ULECategoryTree::SetCurrentGlobalLevel(ELELogVerbosity NewLevel)
{
	ELELogVerbosity OldLevel = CurrentGlobalLevel;
	CurrentGlobalLevel = NewLevel;

	LE_SYSTEM_LOG(TEXT("SetCurrentGlobalLevel: %s -> %s"),
		*UEnum::GetValueAsString(OldLevel), *UEnum::GetValueAsString(NewLevel));
}

ELELogVerbosity ULECategoryTree::GetCurrentGlobalLevel() const
{
	return CurrentGlobalLevel;
}

void ULECategoryTree::GetTreeStatistics(int32& OutTotalNodes, int32& OutMaxDepth, int32& OutExplicitNodes) const
{
	OutTotalNodes = Nodes.Num();
	OutMaxDepth = 0;
	OutExplicitNodes = 0;

	for (const FLECategoryNode& Node : Nodes)
	{
		if (Node.Depth > OutMaxDepth)
		{
			OutMaxDepth = Node.Depth;
		}

		if (Node.bHasExplicitLevel)
		{
			OutExplicitNodes++;
		}
	}
}

FString ULECategoryTree::ExportTreeDebugString() const
{
	FString DebugString = FString::Printf(TEXT("=== Category Tree Debug Info ===\n"));
	DebugString += FString::Printf(TEXT("Total Nodes: %d, Root Index: %d\n"), Nodes.Num(), RootNodeIndex);
	DebugString += FString::Printf(TEXT("Default Global Level: %s\n\n"), *UEnum::GetValueAsString(DefaultGlobalLevel));

	if (IsValidNodeIndex(RootNodeIndex))
	{
		CollectDebugInfo(RootNodeIndex, 0, DebugString);
	}

	return DebugString;
}

const FLECategoryNode* ULECategoryTree::GetNode(const FString& CategoryPath) const
{
	// 使用 FindNodeIndex 查找节点索引
	int32 NodeIndex = FindNodeIndex(CategoryPath);

	// 验证索引有效性
	if (!IsValidNodeIndex(NodeIndex))
	{
#if !UE_BUILD_SHIPPING
		// 仅在调试模式下输出日志
		LE_SYSTEM_LOG(TEXT("[GetNode] Node not found for path: %s"), *CategoryPath);
#endif
		return nullptr;
	}

#if !UE_BUILD_SHIPPING
	// 仅在调试模式下输出详细日志
	LE_SYSTEM_LOG(TEXT("[GetNode] Found node for path: %s (Index: %d)"), *CategoryPath, NodeIndex);
#endif

	// 返回节点的 const 指针
	return &Nodes[NodeIndex];
}

const FLECategoryNode* ULECategoryTree::GetNode(int32 NodeIndex) const
{
	// 使用 IsValidNodeIndex 验证索引有效性
	if (!IsValidNodeIndex(NodeIndex))
	{
#if !UE_BUILD_SHIPPING
		// 仅在调试模式下输出日志
		LE_SYSTEM_LOG(TEXT("[GetNode] Invalid node index: %d (valid range: 0-%d)"), NodeIndex, Nodes.Num() - 1);
#endif
		return nullptr;
	}

	// 返回节点的 const 指针（直接索引访问，高效）
	return &Nodes[NodeIndex];
}

FLECategoryNode* ULECategoryTree::GetMutableNode(const FString& CategoryPath)
{
	// 使用 FindNodeIndex 查找节点索引
	int32 NodeIndex = FindNodeIndex(CategoryPath);

	// 验证索引有效性
	if (!IsValidNodeIndex(NodeIndex))
	{
#if !UE_BUILD_SHIPPING
		// 仅在调试模式下输出日志
		LE_SYSTEM_LOG(TEXT("[GetMutableNode] Node not found for path: %s"), *CategoryPath);
#endif
		return nullptr;
	}

#if !UE_BUILD_SHIPPING
	// 仅在调试模式下输出详细日志
	LE_SYSTEM_LOG(TEXT("[GetMutableNode] Found mutable node for path: %s (Index: %d)"), *CategoryPath, NodeIndex);
#endif

	// 返回节点的非 const 指针
	return &Nodes[NodeIndex];
}

FLECategoryNode* ULECategoryTree::GetMutableNode(int32 NodeIndex)
{
	// 使用 IsValidNodeIndex 验证索引有效性
	if (!IsValidNodeIndex(NodeIndex))
	{
#if !UE_BUILD_SHIPPING
		// 仅在调试模式下输出日志
		LE_SYSTEM_LOG(TEXT("[GetMutableNode] Invalid node index: %d (valid range: 0-%d)"), NodeIndex, Nodes.Num() - 1);
#endif
		return nullptr;
	}

	// 返回节点的非 const 指针（直接索引访问，高效）
	return &Nodes[NodeIndex];
}

void ULECategoryTree::CreateRootNode()
{
	FLECategoryNode RootNode;
	RootNode.CategorySubName = FName(TEXT("LogRoot"));
	RootNode.CategoryFullName = FName(TEXT("LogRoot"));
	RootNode.ExplicitLevel = ELELogVerbosity::NoLogging;
	RootNode.EffectiveLevel = ELELogVerbosity::NoLogging;
	RootNode.bHasExplicitLevel = false;
	RootNode.bIsEnabled = true;
	RootNode.ParentIndex = INDEX_NONE;
	RootNode.Depth = 0;

	RootNodeIndex = Nodes.Add(RootNode);
	PathToIndexMap.Add(FName(TEXT("LogRoot")), RootNodeIndex);

	LE_SYSTEM_LOG(TEXT("Created root node at index: %d"), RootNodeIndex);
}

int32 ULECategoryTree::FindOrCreateNode(const FString& CategoryPath)
{
	// 检查节点是否已存在
	int32 ExistingIndex = FindNodeIndex(CategoryPath);
	if (ExistingIndex != INDEX_NONE)
	{
		return ExistingIndex;
	}

	// 分割路径
	TArray<FString> Components = SplitPath(CategoryPath);
	if (Components.Num() == 0)
	{
		return INDEX_NONE;
	}

	// 确保根节点存在
	if (!IsValidNodeIndex(RootNodeIndex))
	{
		CreateRootNode();
	}

	int32 CurrentParentIndex = RootNodeIndex;

	// 逐级创建或查找节点
	for (int32 i = 0; i < Components.Num(); ++i)
	{
		FString PartialPath = BuildPath(Components, i + 1);
		int32 NodeIndex = FindNodeIndex(PartialPath);

		if (NodeIndex == INDEX_NONE)
		{
			// 创建新节点
			FLECategoryNode NewNode;
			NewNode.CategorySubName = FName(*Components[i]);
			NewNode.CategoryFullName = FName(*PartialPath);
			NewNode.ExplicitLevel = ELELogVerbosity::NotSet;
			NewNode.EffectiveLevel = IsValidNodeIndex(CurrentParentIndex) ? Nodes[CurrentParentIndex].EffectiveLevel : ELELogVerbosity::NotSet;
			NewNode.bHasExplicitLevel = false;
			NewNode.bIsEnabled = true;
			NewNode.ParentIndex = CurrentParentIndex;
			NewNode.Depth = i + 1; // 根节点深度为0，所以这里是i+1

			NodeIndex = Nodes.Add(NewNode);
			PathToIndexMap.Add(FName(*PartialPath), NodeIndex);

			// 更新父节点的子节点列表
			if (IsValidNodeIndex(CurrentParentIndex))
			{
				Nodes[CurrentParentIndex].AddChild(NodeIndex);
			}

			LE_SYSTEM_LOG(TEXT("Created node: %s (Index: %d, Parent: %d)"),
				*PartialPath, NodeIndex, CurrentParentIndex);
		}

		CurrentParentIndex = NodeIndex;
	}

	return CurrentParentIndex;
}

int32 ULECategoryTree::FindNodeIndex(const FString& CategoryPath) const
{
	const int32* FoundIndex = PathToIndexMap.Find(FName(*CategoryPath));
	return FoundIndex ? *FoundIndex : INDEX_NONE;
}

void ULECategoryTree::InternalUpdateChildrenEffectiveLevels(int32 NodeIndex, ELELogVerbosity NewLevel, bool bForceOverride)
{
	if (!IsValidNodeIndex(NodeIndex))
	{
		return;
	}

	const FLECategoryNode& ParentNode = Nodes[NodeIndex];

	// 递归更新所有子节点
	for (int32 ChildIndex : ParentNode.ChildIndices)
	{
		if (!IsValidNodeIndex(ChildIndex))
		{
			continue;
		}

		FLECategoryNode& ChildNode = Nodes[ChildIndex];

		// 根据传播模式决定是否更新子节点
		bool bShouldUpdate = false;
		if (bForceOverride)
		{
			// 强制覆盖模式：无论子节点是否有显式设置都要更新
			bShouldUpdate = true;
			if (bForceOverride)
			{
				// 强制模式下，清除子节点的显式设置并设置新的显式级别
				ChildNode.SetExplicitLevel(NewLevel, true);
			}
		}
		else
		{
			// 智能继承模式：仅更新没有显式设置的子节点
			bShouldUpdate = !ChildNode.bHasExplicitLevel;
			if (bShouldUpdate)
			{
				ChildNode.UpdateEffectiveLevel(NewLevel);
			}
		}

		// 递归更新子节点的子节点
		if (bShouldUpdate)
		{
			InternalUpdateChildrenEffectiveLevels(ChildIndex, NewLevel, bForceOverride);
		}
	}
}

void ULECategoryTree::InternalUpdateChildrenEnabledState(int32 NodeIndex, bool bParentEnabled)
{
	if (!IsValidNodeIndex(NodeIndex))
	{
		return;
	}

	const FLECategoryNode& ParentNode = Nodes[NodeIndex];

	// 递归更新所有子节点
	for (int32 ChildIndex : ParentNode.ChildIndices)
	{
		if (!IsValidNodeIndex(ChildIndex))
		{
			continue;
		}

		FLECategoryNode& ChildNode = Nodes[ChildIndex];

		// 根据父节点状态和子节点的显式设置计算子节点的有效启用状态
		// 继承逻辑：
		// 1. 如果父节点禁用，所有子节点强制禁用（无论子节点的显式设置如何）
		// 2. 如果父节点启用，子节点的状态取决于其 ExplicitEnabled 设置：
		//    - NotSet: 继承父节点状态（启用）
		//    - Enabled: 显式启用
		//    - Disabled: 显式禁用（保持禁用状态）
		if (!bParentEnabled)
		{
			// 父节点禁用：强制所有子节点禁用
			ChildNode.SetEnabled(false);

#if !UE_BUILD_SHIPPING
			LE_SYSTEM_LOG(TEXT("[InternalUpdateChildrenEnabledState] Parent disabled, force child disabled: %s (ExplicitEnabled=%d)"),
				*ChildNode.CategoryFullName.ToString(), static_cast<int32>(ChildNode.ExplicitEnabled));
#endif
		}
		else
		{
			// 父节点启用：根据子节点的显式设置决定状态
			if (ChildNode.ExplicitEnabled == ELEEnabledState::NotSet)
			{
				// 子节点未显式设置：继承父节点状态（启用）
				ChildNode.SetEnabled(true);

#if !UE_BUILD_SHIPPING
				LE_SYSTEM_LOG(TEXT("[InternalUpdateChildrenEnabledState] Parent enabled, child inherits (NotSet): %s -> Enabled"),
					*ChildNode.CategoryFullName.ToString());
#endif
			}
			else if (ChildNode.ExplicitEnabled == ELEEnabledState::Enabled)
			{
				// 子节点显式启用：设为启用
				ChildNode.SetEnabled(true);

#if !UE_BUILD_SHIPPING
				LE_SYSTEM_LOG(TEXT("[InternalUpdateChildrenEnabledState] Parent enabled, child explicitly enabled: %s"),
					*ChildNode.CategoryFullName.ToString());
#endif
			}
			else // ELEEnabledState::Disabled
			{
				// 子节点显式禁用：保持禁用状态
				ChildNode.SetEnabled(false);

#if !UE_BUILD_SHIPPING
				LE_SYSTEM_LOG(TEXT("[InternalUpdateChildrenEnabledState] Parent enabled, child explicitly disabled: %s"),
					*ChildNode.CategoryFullName.ToString());
#endif
			}
		}

		// 递归更新子节点的子树，使用子节点的有效启用状态
		InternalUpdateChildrenEnabledState(ChildIndex, ChildNode.bIsEnabled);
	}
}

TArray<FString> ULECategoryTree::SplitPath(const FString& Path) const
{
	TArray<FString> Components;

	// 移除 LogRoot 前缀（如果存在）
	FString CleanPath = Path;
	if (CleanPath.StartsWith(TEXT("LogRoot.")))
	{
		CleanPath = CleanPath.RightChop(8); // 移除 "LogRoot."
	}
	else if (CleanPath == TEXT("LogRoot"))
	{
		return Components; // 根节点路径，返回空数组
	}

	// 按点分割路径
	CleanPath.ParseIntoArray(Components, TEXT("."), true);

	return Components;
}

FString ULECategoryTree::BuildPath(const TArray<FString>& Components, int32 EndIndex) const
{
	if (Components.Num() == 0 || EndIndex <= 0)
	{
		return TEXT("");
	}

	FString Path;
	for (int32 i = 0; i < EndIndex && i < Components.Num(); ++i)
	{
		if (i > 0)
		{
			Path += TEXT(".");
		}
		Path += Components[i];
	}

	return Path;
}

bool ULECategoryTree::IsValidNodeIndex(int32 NodeIndex) const
{
	return NodeIndex >= 0 && NodeIndex < Nodes.Num();
}

void ULECategoryTree::CollectDebugInfo(int32 NodeIndex, int32 Depth, FString& OutString) const
{
	if (!IsValidNodeIndex(NodeIndex))
	{
		return;
	}

	const FLECategoryNode& Node = Nodes[NodeIndex];

	// 添加缩进
	FString Indent = TEXT("");
	for (int32 i = 0; i < Depth; ++i)
	{
		Indent += TEXT("  ");
	}

	// 添加节点信息
	OutString += FString::Printf(TEXT("%s[%d] %s\n"), *Indent, NodeIndex, *Node.GetDebugString());

	// 递归添加子节点信息
	for (int32 ChildIndex : Node.ChildIndices)
	{
		CollectDebugInfo(ChildIndex, Depth + 1, OutString);
	}
}