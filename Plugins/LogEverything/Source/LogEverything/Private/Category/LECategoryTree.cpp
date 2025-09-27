// Copyright Epic Games, Inc. All Rights Reserved.

#include "Category/LECategoryTree.h"
#include "Utils/LogEverythingUtils.h"
#include "Engine/Engine.h"

ULECategoryTree::ULECategoryTree()
	: RootNodeIndex(INDEX_NONE)
	, TreeVersion(0)
{
}

bool ULECategoryTree::InitializeTree(const TArray<FString>& CategoryPaths)
{
	// 清空现有数据
	Nodes.Empty();
	PathToIndexMap.Empty();
	TreeVersion = 0;

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

	IncrementVersion();
	LE_SYSTEM_LOG(TEXT("Category tree initialized with %d nodes, success: %s"),
		Nodes.Num(), bSuccess ? TEXT("true") : TEXT("false"));

	return bSuccess;
}

bool ULECategoryTree::SetCategoryLevel(const FString& CategoryPath, ELELogVerbosity Level, bool bPropagate)
{
	int32 NodeIndex = FindNodeIndex(CategoryPath);
	if (NodeIndex == INDEX_NONE)
	{
		// 如果节点不存在，尝试创建
		NodeIndex = FindOrCreateNode(CategoryPath);
		if (NodeIndex == INDEX_NONE)
		{
			LE_SYSTEM_WARNING(TEXT("Cannot find or create node for path: %s"), *CategoryPath);
			return false;
		}
	}

	if (!IsValidNodeIndex(NodeIndex))
	{
		return false;
	}

	// 设置节点的显式级别
	Nodes[NodeIndex].SetExplicitLevel(Level, true);

	// 根据传播选项更新子节点
	if (bPropagate)
	{
		// 强制传播：覆盖所有子节点
		UpdateChildrenEffectiveLevels(NodeIndex, Level, true);
	}
	else
	{
		// 智能继承：仅更新没有显式设置的子节点
		UpdateChildrenEffectiveLevels(NodeIndex, Level, false);
	}

	IncrementVersion();
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
	const int32* FoundIndex = PathToIndexMap.Find(CategoryName);
	int32 NodeIndex = FoundIndex ? *FoundIndex : INDEX_NONE;

	if (IsValidNodeIndex(NodeIndex))
	{
		const FLECategoryNode& Node = Nodes[NodeIndex];

		// 检查分类是否启用
		if (!Node.bIsEnabled)
		{
			return false;
		}

		// 直接比较枚举值（值越小级别越低）
		return static_cast<uint8>(Level) >= static_cast<uint8>(Node.EffectiveLevel);
	}

	// 如果找不到节点，使用默认规则：Info 及以上级别显示
	return static_cast<uint8>(Level) >= static_cast<uint8>(ELELogVerbosity::Warning);
}

bool ULECategoryTree::SetCategoryEnabled(const FString& CategoryPath, bool bEnabled, bool bPropagate)
{
	int32 NodeIndex = FindNodeIndex(CategoryPath);
	if (NodeIndex == INDEX_NONE)
	{
		NodeIndex = FindOrCreateNode(CategoryPath);
		if (NodeIndex == INDEX_NONE)
		{
			return false;
		}
	}

	if (!IsValidNodeIndex(NodeIndex))
	{
		return false;
	}

	// 设置节点启用状态
	Nodes[NodeIndex].SetEnabled(bEnabled);

	// 如果需要传播到子节点
	if (bPropagate)
	{
		UpdateChildrenEnabledState(NodeIndex, bEnabled);
	}

	IncrementVersion();
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
	// 重置所有节点到默认状态
	for (FLECategoryNode& Node : Nodes)
	{
		Node.ClearExplicitLevel(ELELogVerbosity::Info);
		Node.SetEnabled(true);
	}

	// 重新计算所有有效级别
	if (IsValidNodeIndex(RootNodeIndex))
	{
		UpdateChildrenEffectiveLevels(RootNodeIndex, ELELogVerbosity::Info, true);
	}

	IncrementVersion();
	LE_SYSTEM_LOG(TEXT("Category tree reset to default"));
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
	FString DebugString = FString::Printf(TEXT("=== Category Tree Debug Info (Version: %d) ===\n"), TreeVersion);
	DebugString += FString::Printf(TEXT("Total Nodes: %d, Root Index: %d\n\n"), Nodes.Num(), RootNodeIndex);

	if (IsValidNodeIndex(RootNodeIndex))
	{
		CollectDebugInfo(RootNodeIndex, 0, DebugString);
	}

	return DebugString;
}

void ULECategoryTree::CreateRootNode()
{
	FLECategoryNode RootNode;
	RootNode.CategorySubName = FName(TEXT("LogRoot"));
	RootNode.CategoryFullName = FName(TEXT("LogRoot"));
	RootNode.ExplicitLevel = ELELogVerbosity::Info;
	RootNode.EffectiveLevel = ELELogVerbosity::Info;
	RootNode.bHasExplicitLevel = true;
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
			NewNode.ExplicitLevel = ELELogVerbosity::NoLogging;
			NewNode.EffectiveLevel = IsValidNodeIndex(CurrentParentIndex) ? Nodes[CurrentParentIndex].EffectiveLevel : ELELogVerbosity::Info;
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

void ULECategoryTree::UpdateChildrenEffectiveLevels(int32 NodeIndex, ELELogVerbosity NewLevel, bool bForceOverride)
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
			UpdateChildrenEffectiveLevels(ChildIndex, NewLevel, bForceOverride);
		}
	}
}

void ULECategoryTree::UpdateChildrenEnabledState(int32 NodeIndex, bool bEnabled)
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

		Nodes[ChildIndex].SetEnabled(bEnabled);
		UpdateChildrenEnabledState(ChildIndex, bEnabled);
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