// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class FDebugDisplayInfo;
class ULECategoryTree;
struct FDisplayDebugManager;
enum class ELELogVerbosity : uint8;

/**
 * ShowDebug LogEverything 可视化辅助类
 * 订阅 AHUD::OnShowDebugInfo 委托，在 HUD 上绘制分类树
 *
 * 使用方式：在控制台输入 "ShowDebug LogEverything" 即可显示分类树
 * 再次输入相同命令可关闭显示
 *
 * 显示内容：
 * - 分类树结构（缩进表示层级）
 * - 每个节点的有效日志级别
 * - 启用/禁用状态
 * - 有显式设置的级别使用 * 标记
 */
class LOGEVERYTHING_API FLEShowDebugHelper
{
public:
	/**
	 * 注册到 ShowDebug 系统
	 * 应在 FLogEverythingModule::StartupModule 中调用
	 */
	static void Register();

	/**
	 * 从 ShowDebug 系统注销
	 * 应在 FLogEverythingModule::ShutdownModule 中调用
	 */
	static void Unregister();

	/**
	 * 检查是否已注册
	 * @return 是否已注册到 ShowDebug 系统
	 */
	static bool IsRegistered() { return ShowDebugDelegateHandle.IsValid(); }

private:
	/**
	 * OnShowDebugInfo 委托回调
	 * 当玩家输入 ShowDebug LogEverything 时被调用
	 *
	 * @param HUD 当前 HUD 实例
	 * @param Canvas 绘制画布
	 * @param DisplayInfo 显示信息，用于检查是否启用了 LogEverything
	 * @param YL 行高（输出参数）
	 * @param YPos 当前 Y 位置（输出参数）
	 */
	static void OnShowDebugInfo(
		AHUD* HUD,
		UCanvas* Canvas,
		const FDebugDisplayInfo& DisplayInfo,
		float& YL,
		float& YPos);

	/**
	 * 绘制标题和统计摘要
	 *
	 * @param DisplayManager Canvas 显示管理器
	 * @param TotalNodes 总节点数
	 * @param ExplicitNodes 有显式设置的节点数
	 * @param GlobalLevel 全局默认级别
	 */
	static void DrawHeader(
		FDisplayDebugManager& DisplayManager,
		int32 TotalNodes,
		int32 ExplicitNodes,
		ELELogVerbosity GlobalLevel);

	/**
	 * 递归绘制分类树节点
	 *
	 * @param DisplayManager Canvas 显示管理器
	 * @param Tree 分类树
	 * @param NodeIndex 当前节点索引
	 * @param Depth 当前深度（用于计算缩进）
	 */
	static void DrawCategoryNode(
		FDisplayDebugManager& DisplayManager,
		const ULECategoryTree* Tree,
		int32 NodeIndex,
		int32 Depth);

	/**
	 * 获取日志级别对应的显示颜色
	 *
	 * @param Level 日志级别
	 * @return 对应的颜色
	 */
	static FColor GetLevelColor(ELELogVerbosity Level);

	/**
	 * 获取日志级别的字符串表示
	 *
	 * @param Level 日志级别
	 * @return 级别名称字符串
	 */
	static const TCHAR* GetLevelString(ELELogVerbosity Level);

	/** 委托句柄，用于注销 */
	static FDelegateHandle ShowDebugDelegateHandle;
};
