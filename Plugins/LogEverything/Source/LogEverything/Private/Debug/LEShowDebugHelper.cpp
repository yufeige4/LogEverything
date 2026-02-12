// Copyright Epic Games, Inc. All Rights Reserved.

#include "Debug/LEShowDebugHelper.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "DisplayDebugHelpers.h"
#include "Category/LECategoryTree.h"
#include "System/LELogSubsystem.h"
#include "System/LELogTypes.h"

// 静态成员初始化
FDelegateHandle FLEShowDebugHelper::ShowDebugDelegateHandle;

void FLEShowDebugHelper::Register()
{
	if (!ShowDebugDelegateHandle.IsValid())
	{
		// 注册到 AHUD::OnShowDebugInfo 静态多播委托
		// 这是 Module 级别的注册，整个引擎生命周期只执行一次
		ShowDebugDelegateHandle = AHUD::OnShowDebugInfo.AddStatic(&FLEShowDebugHelper::OnShowDebugInfo);
	}
}

void FLEShowDebugHelper::Unregister()
{
	if (ShowDebugDelegateHandle.IsValid())
	{
		AHUD::OnShowDebugInfo.Remove(ShowDebugDelegateHandle);
		ShowDebugDelegateHandle.Reset();
	}
}

void FLEShowDebugHelper::OnShowDebugInfo(
	AHUD* HUD,
	UCanvas* Canvas,
	const FDebugDisplayInfo& DisplayInfo,
	float& YL,
	float& YPos)
{
	// 检查参数有效性
	if (!Canvas || !HUD)
	{
		return;
	}

	// 检查 LogEverything 是否被启用（玩家输入了 ShowDebug LogEverything）
	static const FName LogEverythingName(TEXT("LogEverything"));
	if (!DisplayInfo.IsDisplayOn(LogEverythingName))
	{
		return;
	}

	// 获取 HUD 所属的 World
	UWorld* World = HUD->GetWorld();
	if (!World)
	{
		return;
	}

	// 获取日志子系统
	ULELogSubsystem* LogSubsystem = ULELogSubsystem::Get(World);
	if (!LogSubsystem)
	{
		return;
	}

	// 获取分类树
	ULECategoryTree* CategoryTree = LogSubsystem->GetCategoryTree();
	if (!CategoryTree)
	{
		return;
	}

	// 获取 DisplayDebugManager 进行绘制
	FDisplayDebugManager& DisplayManager = Canvas->DisplayDebugManager;

	// 获取统计信息
	int32 TotalNodes = 0;
	int32 MaxDepth = 0;
	int32 ExplicitNodes = 0;
	CategoryTree->GetTreeStatistics(TotalNodes, MaxDepth, ExplicitNodes);

	// 绘制标题（显示当前全局级别，运行时可动态修改）
	DrawHeader(DisplayManager, TotalNodes, ExplicitNodes, CategoryTree->GetCurrentGlobalLevel());

	// 绘制分类树（从根节点开始，根节点索引为 0）
	DrawCategoryNode(DisplayManager, CategoryTree, 0, 0);

	// 绘制图例（直接设置 Y 位置而不是调用 ShiftYDrawPosition，避免链接未导出的 AddColumnIfNeeded）
	DisplayManager.SetYPos(DisplayManager.GetYPos() + 5.0f);
	DisplayManager.SetDrawColor(FColor(150, 150, 150));
	DisplayManager.DrawString(TEXT("Format: Level = inherited | Parent(Explicit)* = explicit set"));
}

void FLEShowDebugHelper::DrawHeader(
	FDisplayDebugManager& DisplayManager,
	int32 TotalNodes,
	int32 ExplicitNodes,
	ELELogVerbosity GlobalLevel)
{
	// 标题
	DisplayManager.SetDrawColor(FColor(0, 200, 0));
	DisplayManager.DrawString(TEXT("=== LogEverything Category Tree ==="));

	// 统计摘要
	DisplayManager.SetDrawColor(FColor(200, 200, 200));
	DisplayManager.DrawString(FString::Printf(
		TEXT("Nodes: %d | Explicit: %d | Global: %s"),
		TotalNodes, ExplicitNodes, GetLevelString(GlobalLevel)));

	// 添加一点间距（直接设置 Y 位置而不是调用 ShiftYDrawPosition，避免链接未导出的 AddColumnIfNeeded）
	DisplayManager.SetYPos(DisplayManager.GetYPos() + 5.0f);
}

void FLEShowDebugHelper::DrawCategoryNode(
	FDisplayDebugManager& DisplayManager,
	const ULECategoryTree* Tree,
	int32 NodeIndex,
	int32 Depth)
{
	// 获取节点
	const FLECategoryNode* Node = Tree->GetNode(NodeIndex);
	if (!Node)
	{
		return;
	}

	// 计算缩进（每级 15 像素）
	const float IndentPerLevel = 15.0f;
	const float Indent = Depth * IndentPerLevel;

	// 构建显示字符串
	// 节点名称（使用子名称，更简洁）
	FString NodeName = Node->CategorySubName.ToString();

	// 日志级别字符串
	// 格式：有显式设置时显示 InheritedLevel(ExplicitLevel)*，否则只显示 EffectiveLevel
	FString LevelStr;
	if (Node->bHasExplicitLevel)
	{
		// 获取继承的级别（父节点的有效级别或全局级别）
		ELELogVerbosity InheritedLevel = Tree->GetDefaultGlobalLevel();
		if (Node->ParentIndex != INDEX_NONE)
		{
			const FLECategoryNode* ParentNode = Tree->GetNode(Node->ParentIndex);
			if (ParentNode)
			{
				InheritedLevel = ParentNode->EffectiveLevel;
			}
		}

		// 格式：InheritedLevel(ExplicitLevel)*
		LevelStr = FString::Printf(TEXT("%s(%s)*"),
			GetLevelString(InheritedLevel),
			GetLevelString(Node->ExplicitLevel));
	}
	else
	{
		// 没有显式设置，只显示有效级别（继承的）
		LevelStr = GetLevelString(Node->EffectiveLevel);
	}

	// 启用状态
	FString EnabledStr = Node->bIsEnabled ? TEXT("[ON]") : TEXT("[OFF]");

	// 设置颜色（根据实际生效的级别）
	if (Node->bIsEnabled)
	{
		// 根据日志级别设置颜色
		DisplayManager.SetDrawColor(GetLevelColor(Node->EffectiveLevel));
	}
	else
	{
		// 禁用的节点使用灰色
		DisplayManager.SetDrawColor(FColor(100, 100, 100));
	}

	// 绘制节点信息（格式：名称 + 级别 + 状态）
	FString Line = FString::Printf(TEXT("%-20s %-20s %s"), *NodeName, *LevelStr, *EnabledStr);
	DisplayManager.DrawString(Line, Indent);

	// 递归绘制子节点
	for (int32 ChildIndex : Node->ChildIndices)
	{
		DrawCategoryNode(DisplayManager, Tree, ChildIndex, Depth + 1);
	}
}

FColor FLEShowDebugHelper::GetLevelColor(ELELogVerbosity Level)
{
	switch (Level)
	{
		case ELELogVerbosity::Verbose: return FColor(128, 128, 128); // 灰色
		case ELELogVerbosity::Debug:   return FColor(0, 200, 200);   // 青色
		case ELELogVerbosity::Info:    return FColor(255, 255, 255); // 白色
		case ELELogVerbosity::Warning: return FColor(255, 255, 0);   // 黄色
		case ELELogVerbosity::Error:   return FColor(255, 100, 100); // 红色
		case ELELogVerbosity::Fatal:   return FColor(200, 0, 200);   // 紫色
		default:                       return FColor(255, 255, 255); // 默认白色
	}
}

const TCHAR* FLEShowDebugHelper::GetLevelString(ELELogVerbosity Level)
{
	switch (Level)
	{
		case ELELogVerbosity::Verbose:   return TEXT("Verbose");
		case ELELogVerbosity::Debug:     return TEXT("Debug");
		case ELELogVerbosity::Info:      return TEXT("Info");
		case ELELogVerbosity::Warning:   return TEXT("Warning");
		case ELELogVerbosity::Error:     return TEXT("Error");
		case ELELogVerbosity::Fatal:     return TEXT("Fatal");
		case ELELogVerbosity::NotSet:    return TEXT("NotSet");
		case ELELogVerbosity::NoLogging: return TEXT("NoLog");
		default:                         return TEXT("Unknown");
	}
}
