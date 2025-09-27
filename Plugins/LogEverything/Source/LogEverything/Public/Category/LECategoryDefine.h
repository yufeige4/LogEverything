// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Macros/LELogMacros.h"

/**
 * LogEverything 测试分类定义
 * Test category definitions for LogEverything system
 *
 * 使用示例：
 * 1. 在头文件中声明分类
 * 2. 在源文件中定义分类
 * 3. 在代码中使用分类记录日志
 */

DECLARE_LE_CATEGORY_EXTERN(LELogGame, Game);
DECLARE_LE_CATEGORY_EXTERN(LELogEngine, Engine);
DECLARE_LE_CATEGORY_EXTERN(LELogEditor, Editor);
DECLARE_LE_CATEGORY_EXTERN(LELogTest, Test);



// =============================================================================
// 游戏动画分类
// =============================================================================

/** 游戏动画系统 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameAnimation, Game.Animation);

// =============================================================================
// 游戏战斗系统分类
// =============================================================================

/** 战斗系统 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameCombat, Game.Combat);

/** 战斗系统伤害计算 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameCombatDamage, Game.Combat.Damage);

/** 战斗系统技能系统 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameCombatSkill, Game.Combat.Skill);

/** 战斗系统输入处理 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameCombatInput, Game.Combat.Input);

// =============================================================================
// 游戏 AI 系统分类
// =============================================================================

/** AI */
DECLARE_LE_CATEGORY_EXTERN(LELogGameAI, Game.AI);

/** AI 行为树系统 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameAIBehaviorTree, Game.AI.BehaviorTree);

/** AI 路径查找系统 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameAIPathfinding, Game.AI.Pathfinding);

// =============================================================================
// 游戏输入系统分类 - 演示层级化分类创建多个节点
// =============================================================================

/** 游戏输入系统根分类 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameInput, Game.Input);

/** 输入系统 - 技能输入 (演示：此声明创建3个节点: Game, Game.Input, Game.Input.Ability) */
DECLARE_LE_CATEGORY_EXTERN(LELogGameInputAbility, Game.Input.Ability);

/** 输入系统 - 移动输入 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameInputMovement, Game.Input.Movement);

/** 输入系统 - 交互输入 */
DECLARE_LE_CATEGORY_EXTERN(LELogGameInputInteraction, Game.Input.Interaction);

// =============================================================================
// 演示层级化分类创建多个节点 - 使用现有Game.AI体系
// Game.AI.BehaviorTree演示：此声明创建3个节点: Game, Game.AI, Game.AI.BehaviorTree
// =============================================================================

// =============================================================================
// 测试分类
// =============================================================================

DECLARE_LE_CATEGORY_EXTERN(LELogTestLogSystem, Test.LogSystem);
