// Copyright Epic Games, Inc. All Rights Reserved.

#include "LogEverything.h"
#include "Bridge/LEBqLogBridge.h"
#include "System/LELogTypes.h"
#include "Utils/LogEverythingUtils.h"
#include "Debug/LEShowDebugHelper.h"

#define LOCTEXT_NAMESPACE "FLogEverythingModule"

// 定义日志系统自身的日志分类
DEFINE_LOG_CATEGORY(LogEverythingPlugin);

void FLogEverythingModule::StartupModule()
{
	// 初始化 LogEverything 系统
	LE_SYSTEM_LOG(TEXT("LogEverything module starting up..."));

	// 注册 ShowDebug 委托（Module 级别，整个引擎生命周期只执行一次）
	// 评审修正：委托注册必须在 Module 级别，而非 Subsystem 级别
	// 原因：PIE 多实例时，Subsystem 会创建多个实例，首个实例析构会错误注销委托
	FLEShowDebugHelper::Register();
	LE_SYSTEM_LOG(TEXT("ShowDebug LogEverything registered"));
}

void FLogEverythingModule::ShutdownModule()
{
	// 关闭 LogEverything 系统
	LE_SYSTEM_LOG(TEXT("LogEverything module shutting down..."));

	// 注销 ShowDebug 委托
	FLEShowDebugHelper::Unregister();
	LE_SYSTEM_LOG(TEXT("ShowDebug LogEverything unregistered"));

	LE_SYSTEM_LOG(TEXT("LogEverything module shutdown complete"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLogEverythingModule, LogEverything)