// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Logging/LogVerbosity.h"

// BqLog 配置相关类型（FLEBqLogConfig, ELELogOutput, ELELogReliableLevel）
#include "LEBqLogConfig.h"

#include "LELogTypes.generated.h"

#ifndef LOGEVERYTHING_API
#define LOGEVERYTHING_API
#endif

/**
 * 蓝图友好的日志级别枚举 - 与 BqLog 的 log_level 一一对应
 * Blueprint-friendly log verbosity enumeration that maps directly to BqLog log_level
 */
UENUM(BlueprintType)
enum class ELELogVerbosity : uint8
{
	Verbose     = 0		UMETA(DisplayName = "Verbose"),     // 对应 BqLog::verbose
	Debug       = 1		UMETA(DisplayName = "Debug"),       // 对应 BqLog::debug
	Info        = 2		UMETA(DisplayName = "Info"),        // 对应 BqLog::info
	Warning     = 3		UMETA(DisplayName = "Warning"),     // 对应 BqLog::warning
	Error       = 4		UMETA(DisplayName = "Error"),       // 对应 BqLog::error
	Fatal       = 5		UMETA(DisplayName = "Fatal"),       // 对应 BqLog::fatal

	NotSet      = 254	UMETA(DisplayName = "Not Set"),		// 特殊值，表示未设置
	NoLogging   = 255	UMETA(DisplayName = "No Logging"),  // 特殊值，表示完全关闭日志
};

/**
 * 构建环境类型枚举
 * 用于区分不同构建配置下的日志行为
 *
 * - Development: 开发环境，详细日志输出，适合开发调试
 * - Debug: 调试环境，最详细的日志输出，包含调试符号和额外检查
 * - Test: 测试环境，针对QA和自动化测试优化的日志配置，关注关键业务逻辑
 * - Shipping: 发布环境，最精简的日志配置，仅输出警告和错误
 */
UENUM(BlueprintType)
enum class ELEBuildEnvironment : uint8
{
	/** 开发环境 - 详细日志输出，适合日常开发调试 */
	Development		UMETA(DisplayName = "Development"),

	/** 调试环境 - 最详细的日志输出，包含所有调试信息 */
	Debug			UMETA(DisplayName = "Debug"),

	/** 测试环境 - QA和自动化测试优化的日志配置 */
	Test			UMETA(DisplayName = "Test"),

	/** 发布环境 - 仅输出警告和错误，性能优先 */
	Shipping		UMETA(DisplayName = "Shipping")
};

/**
 * 网络环境类型枚举
 * 用于区分客户端和专用服务器的日志配置
 *
 * - Client: 客户端环境，可能包含UI相关的详细日志
 * - DedicatedServer: 专用服务器环境，关注网络和游戏逻辑，减少渲染相关日志
 */
UENUM(BlueprintType)
enum class ELENetEnvironment : uint8
{
	/** 客户端 - 包含UI和渲染相关的日志 */
	Client				UMETA(DisplayName = "Client"),

	/** 专用服务器 - 关注网络和游戏逻辑 */
	DedicatedServer		UMETA(DisplayName = "Dedicated Server")
};

/**
 * 分类启用状态枚举
 * 用一个变量同时表达"是否有显式设置"和"设置的值"
 *
 * 这是解决"启用状态继承缺陷"的核心机制：
 * - NotSet=0: 默认值，表示未显式设置启用状态，应该继承父节点的启用状态
 *   例如：父节点 "Game" 启用，子节点 "Game.Combat" 为 NotSet，则 "Game.Combat" 继承启用
 * - Disabled=1: 显式禁用，即使父节点启用也保持禁用状态
 *   例如：父节点 "Game" 启用，子节点 "Game.Combat" 为 Disabled，则 "Game.Combat" 禁用
 * - Enabled=2: 显式启用，但父节点禁用时会被强制禁用（继承父节点的禁用状态）
 *   例如：父节点 "Game" 禁用，子节点 "Game.Combat" 为 Enabled，则 "Game.Combat" 仍然禁用
 *
 * 启用状态继承规则：
 * 1. 如果父节点禁用，所有子节点强制禁用（无论子节点设置为什么）
 * 2. 如果父节点启用，子节点的实际状态取决于自身设置：
 *    - NotSet: 继承父节点启用状态
 *    - Disabled: 禁用
 *    - Enabled: 启用
 */
UENUM(BlueprintType)
enum class ELEEnabledState : uint8
{
	/** 未设置 - 继承父节点的启用状态 */
	NotSet   = 0	UMETA(DisplayName = "Not Set"),

	/** 显式禁用 - 即使父节点启用也保持禁用 */
	Disabled = 1	UMETA(DisplayName = "Disabled"),

	/** 显式启用 - 但父节点禁用时会被强制禁用 */
	Enabled  = 2	UMETA(DisplayName = "Enabled")
};


/**
 * 类型转换工具函数
 * Type conversion utility functions
 */
namespace LELogVerbosityUtils
{
	/** 将蓝图友好的枚举转换为 UE 内部枚举 */
	inline ELogVerbosity::Type ToUELogVerbosity(ELELogVerbosity Verbosity)
	{
		// 将 BqLog 对应的级别映射到 UE 的日志级别
		switch (Verbosity)
		{
		case ELELogVerbosity::NoLogging:    return ELogVerbosity::NoLogging;
		case ELELogVerbosity::Verbose:      return ELogVerbosity::VeryVerbose;  // BqLog verbose -> UE VeryVerbose
		case ELELogVerbosity::Debug:        return ELogVerbosity::Verbose;      // BqLog debug -> UE Verbose
		case ELELogVerbosity::Info:         return ELogVerbosity::Log;          // BqLog info -> UE Log
		case ELELogVerbosity::Warning:      return ELogVerbosity::Warning;      // BqLog warning -> UE Warning
		case ELELogVerbosity::Error:        return ELogVerbosity::Error;        // BqLog error -> UE Error
		case ELELogVerbosity::Fatal:        return ELogVerbosity::Fatal;        // BqLog fatal -> UE Fatal
		default:                            return ELogVerbosity::Log;          // 默认级别
		}
	}

	/** 将 UE 内部枚举转换为蓝图友好的枚举 */
	inline ELELogVerbosity FromUELogVerbosity(ELogVerbosity::Type Verbosity)
	{
		// 将 UE 的日志级别映射回 BqLog 对应的级别
		switch (Verbosity)
		{
		case ELogVerbosity::NoLogging:      return ELELogVerbosity::NoLogging;
		case ELogVerbosity::Fatal:          return ELELogVerbosity::Fatal;
		case ELogVerbosity::Error:          return ELELogVerbosity::Error;
		case ELogVerbosity::Warning:        return ELELogVerbosity::Warning;
		case ELogVerbosity::Display:        return ELELogVerbosity::Info;       // UE Display -> BqLog info
		case ELogVerbosity::Log:            return ELELogVerbosity::Info;       // UE Log -> BqLog info
		case ELogVerbosity::Verbose:        return ELELogVerbosity::Debug;      // UE Verbose -> BqLog debug
		case ELogVerbosity::VeryVerbose:    return ELELogVerbosity::Verbose;    // UE VeryVerbose -> BqLog verbose
		default:                            return ELELogVerbosity::Info;       // 默认级别
		}
	}

	/** 将 BqLog 级别直接转换为我们的枚举（零开销转换） */
	inline ELELogVerbosity FromBqLogLevel(uint8 BqLogLevel)
	{
		// BqLog 级别值可以直接转换（除了 NoLogging 特殊情况）
		if (BqLogLevel <= 5) // BqLog 有效级别范围 0-5
		{
			return static_cast<ELELogVerbosity>(BqLogLevel);
		}
		return ELELogVerbosity::Info; // 默认级别
	}

	/** 将我们的枚举转换为 BqLog 级别（零开销转换） */
	inline uint8 ToBqLogLevel(ELELogVerbosity Verbosity)
	{
		uint8 Level = static_cast<uint8>(Verbosity);
		// 如果是 NoLogging，返回一个 BqLog 不支持的值，由调用者处理
		if (Level == 255) // NoLogging
		{
			return 255; // 调用者需要特殊处理这种情况
		}
		return Level; // 其他情况直接返回数值（0-5）
	}
};
