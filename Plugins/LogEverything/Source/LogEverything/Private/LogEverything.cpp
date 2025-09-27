// Copyright Epic Games, Inc. All Rights Reserved.

#include "LogEverything.h"
#include "Bridge/LEBqLogBridge.h"
#include "System/LELogTypes.h"
#include "Utils/LogEverythingUtils.h"

#define LOCTEXT_NAMESPACE "FLogEverythingModule"

// 定义日志系统自身的日志分类
DEFINE_LOG_CATEGORY(LogEverythingPlugin);

void FLogEverythingModule::StartupModule()
{
	// 初始化 LogEverything 系统
	LE_SYSTEM_LOG(TEXT("LogEverything module starting up..."));
	
}

void FLogEverythingModule::ShutdownModule()
{
	// 关闭 LogEverything 系统
	LE_SYSTEM_LOG(TEXT("LogEverything module shutting down..."));

	LE_SYSTEM_LOG(TEXT("LogEverything module shutdown complete"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLogEverythingModule, LogEverything)