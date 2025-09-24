// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LELogMacros.h"

/**
 * LogEverything 测试分类定义
 * Test category definitions for LogEverything system
 *
 * 使用示例：
 * 1. 在头文件中声明分类
 * 2. 在源文件中定义分类
 * 3. 在代码中使用分类记录日志
 */

// =============================================================================
// 游戏核心系统分类
// =============================================================================

/** 游戏核心内存管理 */
DECLARE_LE_CATEGORY_EXTERN(LogGameCoreMemory, Game.Core.Memory);

/** 游戏核心线程管理 */
DECLARE_LE_CATEGORY_EXTERN(LogGameCoreThreading, Game.Core.Threading);

/** 游戏核心性能监控 */
DECLARE_LE_CATEGORY_EXTERN(LogGameCorePerformance, Game.Core.Performance);

// =============================================================================
// 游戏战斗系统分类
// =============================================================================

/** 战斗系统伤害计算 */
DECLARE_LE_CATEGORY_EXTERN(LogGameCombatDamage, Game.Combat.Damage);

/** 战斗系统技能系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameCombatSkill, Game.Combat.Skill);

/** 战斗系统输入处理 */
DECLARE_LE_CATEGORY_EXTERN(LogGameCombatInput, Game.Combat.Input);

// =============================================================================
// 游戏 AI 系统分类
// =============================================================================

/** AI 行为树系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameAIBehaviorTree, Game.AI.BehaviorTree);

/** AI 路径查找系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameAIPathfinding, Game.AI.Pathfinding);

/** AI 决策系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameAIDecision, Game.AI.Decision);

// =============================================================================
// 游戏物理系统分类
// =============================================================================

/** 物理碰撞检测 */
DECLARE_LE_CATEGORY_EXTERN(LogGamePhysicsCollision, Game.Physics.Collision);

/** 物理仿真系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGamePhysicsSimulation, Game.Physics.Simulation);

// =============================================================================
// 游戏渲染系统分类
// =============================================================================

/** 渲染网格系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameRenderingMesh, Game.Rendering.Mesh);

/** 渲染材质系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameRenderingMaterial, Game.Rendering.Material);

/** 渲染光照系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameRenderingLighting, Game.Rendering.Lighting);

// =============================================================================
// 游戏网络系统分类
// =============================================================================

/** 网络复制系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameNetworkReplication, Game.Network.Replication);

/** 网络连接管理 */
DECLARE_LE_CATEGORY_EXTERN(LogGameNetworkConnection, Game.Network.Connection);

/** 网络安全系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameNetworkSecurity, Game.Network.Security);

// =============================================================================
// 游戏音频系统分类
// =============================================================================

/** 音频音效系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameAudioSFX, Game.Audio.SFX);

/** 音频音乐系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameAudioMusic, Game.Audio.Music);

/** 音频语音系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameAudioVoice, Game.Audio.Voice);

// =============================================================================
// 游戏 UI 系统分类
// =============================================================================

/** UI HUD 系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameUIHUD, Game.UI.HUD);

/** UI 菜单系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameUIMenu, Game.UI.Menu);

/** UI 交互系统 */
DECLARE_LE_CATEGORY_EXTERN(LogGameUIInteraction, Game.UI.Interaction);

// =============================================================================
// 编辑器工具分类
// =============================================================================

/** 编辑器蓝图工具 */
DECLARE_LE_CATEGORY_EXTERN(LogEditorToolsBlueprint, Editor.Tools.Blueprint);

/** 编辑器动画工具 */
DECLARE_LE_CATEGORY_EXTERN(LogEditorToolsAnimation, Editor.Tools.Animation);

/** 编辑器资产工具 */
DECLARE_LE_CATEGORY_EXTERN(LogEditorToolsAsset, Editor.Tools.Asset);

// =============================================================================
// 编辑器 UI 分类
// =============================================================================

/** 编辑器检视器 */
DECLARE_LE_CATEGORY_EXTERN(LogEditorUIInspector, Editor.UI.Inspector);

/** 编辑器视口 */
DECLARE_LE_CATEGORY_EXTERN(LogEditorUIViewport, Editor.UI.Viewport);

// =============================================================================
// 编辑器资产分类
// =============================================================================

/** 编辑器资产导入 */
DECLARE_LE_CATEGORY_EXTERN(LogEditorAssetImport, Editor.Asset.Import);

/** 编辑器资产导出 */
DECLARE_LE_CATEGORY_EXTERN(LogEditorAssetExport, Editor.Asset.Export);

// =============================================================================
// 引擎核心分类
// =============================================================================

/** 引擎核心内存管理 */
DECLARE_LE_CATEGORY_EXTERN(LogEngineCoreMemory, Engine.Core.Memory);

/** 引擎核心线程管理 */
DECLARE_LE_CATEGORY_EXTERN(LogEngineCoreThreading, Engine.Core.Threading);

// =============================================================================
// 引擎 IO 分类
// =============================================================================

/** 引擎文件系统 */
DECLARE_LE_CATEGORY_EXTERN(LogEngineIOFileSystem, Engine.IO.FileSystem);

/** 引擎网络 IO */
DECLARE_LE_CATEGORY_EXTERN(LogEngineIONetwork, Engine.IO.Network);

// =============================================================================
// 测试分类
// =============================================================================

/** 单元测试 */
DECLARE_LE_CATEGORY_EXTERN(LogTestUnit, Test.Unit);

/** 集成测试 */
DECLARE_LE_CATEGORY_EXTERN(LogTestIntegration, Test.Integration);

/** 性能测试 */
DECLARE_LE_CATEGORY_EXTERN(LogTestPerformance, Test.Performance);

/** 基准测试 */
DECLARE_LE_CATEGORY_EXTERN(LogTestBenchmark, Test.Benchmark);