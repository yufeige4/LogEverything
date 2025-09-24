// Copyright Epic Games, Inc. All Rights Reserved.

#include "LogEverything.h"
#include "LEBqLogBridge.h"
#include "LELogTypes.h"
#include "LogEverythingUtils.h"

#define LOCTEXT_NAMESPACE "FLogEverythingModule"

// 定义日志系统自身的日志分类
DEFINE_LOG_CATEGORY(LogEverythingPlugin);

// 前向声明测试函数
extern void RunLogEverythingSystemTests();

void FLogEverythingModule::StartupModule()
{
	// 初始化 LogEverything 系统
	LE_SYSTEM_LOG(TEXT("LogEverything module starting up..."));

	// 创建默认配置
	FLELogSettings DefaultSettings;
	DefaultSettings.GlobalLogLevel = static_cast<uint8>(ELogVerbosity::Log);
	DefaultSettings.bEnableAsyncLogging = true;
	DefaultSettings.BufferSize = 1048576; // 1MB
	DefaultSettings.LogFilePath = TEXT("Logs/LogEverything.log");

	// 添加一些默认分类级别
	FLECategoryLevel GameCategory(TEXT("Game"), static_cast<uint8>(ELogVerbosity::Log));
	FLECategoryLevel EngineCategory(TEXT("Engine"), static_cast<uint8>(ELogVerbosity::Warning));
	FLECategoryLevel DebugCategory(TEXT("Debug"), static_cast<uint8>(ELogVerbosity::Verbose));

	DefaultSettings.CategoryLevels.Add(GameCategory);
	DefaultSettings.CategoryLevels.Add(EngineCategory);
	DefaultSettings.CategoryLevels.Add(DebugCategory);

	// 初始化 BqLog 桥接
	if (FLEBqLogBridge::Get().Initialize(DefaultSettings))
	{
		LE_SYSTEM_LOG(TEXT("LogEverything system initialized successfully"));

		// 运行基础功能测试
		RunLogEverythingSystemTests();
	}
	else
	{
		LE_SYSTEM_ERROR(TEXT("Failed to initialize LogEverything system"));
	}
}

void FLogEverythingModule::ShutdownModule()
{
	// 关闭 LogEverything 系统
	LE_SYSTEM_LOG(TEXT("LogEverything module shutting down..."));

	FLEBqLogBridge::Get().Shutdown();

	LE_SYSTEM_LOG(TEXT("LogEverything module shutdown complete"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLogEverythingModule, LogEverything)