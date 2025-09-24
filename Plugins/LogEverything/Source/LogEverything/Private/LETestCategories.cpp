// Copyright Epic Games, Inc. All Rights Reserved.

#include "LETestCategories.h"

/**
 * LogEverything 测试分类实现
 * Test category implementations for LogEverything system
 *
 * 注意：这里必须与 Config/LogEverythingCategories.txt 中的分类路径保持一致
 */

// =============================================================================
// 游戏核心系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogGameCoreMemory);
DEFINE_LE_CATEGORY(LogGameCoreThreading);
DEFINE_LE_CATEGORY(LogGameCorePerformance);

// =============================================================================
// 游戏战斗系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogGameCombatDamage);
DEFINE_LE_CATEGORY(LogGameCombatSkill);
DEFINE_LE_CATEGORY(LogGameCombatInput);

// =============================================================================
// 游戏 AI 系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogGameAIBehaviorTree);
DEFINE_LE_CATEGORY(LogGameAIPathfinding);
DEFINE_LE_CATEGORY(LogGameAIDecision);

// =============================================================================
// 游戏物理系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogGamePhysicsCollision);
DEFINE_LE_CATEGORY(LogGamePhysicsSimulation);

// =============================================================================
// 游戏渲染系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogGameRenderingMesh);
DEFINE_LE_CATEGORY(LogGameRenderingMaterial);
DEFINE_LE_CATEGORY(LogGameRenderingLighting);

// =============================================================================
// 游戏网络系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogGameNetworkReplication);
DEFINE_LE_CATEGORY(LogGameNetworkConnection);
DEFINE_LE_CATEGORY(LogGameNetworkSecurity);

// =============================================================================
// 游戏音频系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogGameAudioSFX);
DEFINE_LE_CATEGORY(LogGameAudioMusic);
DEFINE_LE_CATEGORY(LogGameAudioVoice);

// =============================================================================
// 游戏 UI 系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogGameUIHUD);
DEFINE_LE_CATEGORY(LogGameUIMenu);
DEFINE_LE_CATEGORY(LogGameUIInteraction);

// =============================================================================
// 编辑器工具分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogEditorToolsBlueprint);
DEFINE_LE_CATEGORY(LogEditorToolsAnimation);
DEFINE_LE_CATEGORY(LogEditorToolsAsset);

// =============================================================================
// 编辑器 UI 分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogEditorUIInspector);
DEFINE_LE_CATEGORY(LogEditorUIViewport);

// =============================================================================
// 编辑器资产分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogEditorAssetImport);
DEFINE_LE_CATEGORY(LogEditorAssetExport);

// =============================================================================
// 引擎核心分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogEngineCoreMemory);
DEFINE_LE_CATEGORY(LogEngineCoreThreading);

// =============================================================================
// 引擎 IO 分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogEngineIOFileSystem);
DEFINE_LE_CATEGORY(LogEngineIONetwork);

// =============================================================================
// 测试分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LogTestUnit);
DEFINE_LE_CATEGORY(LogTestIntegration);
DEFINE_LE_CATEGORY(LogTestPerformance);
DEFINE_LE_CATEGORY(LogTestBenchmark);