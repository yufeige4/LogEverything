// Copyright Epic Games, Inc. All Rights Reserved.

#include "Category/LECategoryDefine.h"

/**
 * LogEverything 测试分类实现
 * Test category implementations for LogEverything system
 *
 * 注意：这里必须与 Config/LogEverythingCategories.txt 中的分类路径保持一致
 */


DEFINE_LE_CATEGORY(LELogGame);
DEFINE_LE_CATEGORY(LELogEngine);
DEFINE_LE_CATEGORY(LELogEditor);
DEFINE_LE_CATEGORY(LELogTest);

// =============================================================================
// 游戏动画分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LELogGameAnimation);

// =============================================================================
// 游戏战斗系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LELogGameCombatDamage);
DEFINE_LE_CATEGORY(LELogGameCombatSkill);
DEFINE_LE_CATEGORY(LELogGameCombatInput);

// =============================================================================
// 游戏 AI 系统分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LELogGameAI);
DEFINE_LE_CATEGORY(LELogGameAIBehaviorTree);
DEFINE_LE_CATEGORY(LELogGameAIPathfinding);

// =============================================================================
// 游戏输入系统分类定义 - 用于演示层级化分类创建多个节点
// =============================================================================

DEFINE_LE_CATEGORY(LELogGameInput);
DEFINE_LE_CATEGORY(LELogGameInputAbility);
DEFINE_LE_CATEGORY(LELogGameInputMovement);
DEFINE_LE_CATEGORY(LELogGameInputInteraction);


// =============================================================================
// 测试分类定义
// =============================================================================

DEFINE_LE_CATEGORY(LELogTestLogSystem);