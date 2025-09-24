// Copyright Epic Games, Inc. All Rights Reserved.

#include "LETestCategories.h"
#include "LEBqLogBridge.h"
#include "LELogTypes.h"
#include "LogEverythingUtils.h"
#include "CoreMinimal.h"

// 声明测试专用分类
DECLARE_LE_CATEGORY_EXTERN(LogTestSystemBasic, Test.System.Basic);

/**
 * 基础测试函数，验证 LogEverything 和 BqLog 集成
 * Basic test function to verify LogEverything and BqLog integration
 */
void TestLogEverythingBasicSystem()
{
	LE_SYSTEM_LOG(TEXT("开始测试 LogEverything 基础功能"));

	// 1. 测试 BqLog 桥接初始化
	FLELogSettings TestSettings;
	TestSettings.LogFilePath = TEXT("Logs/Test");
	TestSettings.GlobalLogLevel = static_cast<uint8>(ELogVerbosity::Verbose);
	TestSettings.BufferSize = 1024 * 1024; // 1MB
	TestSettings.bEnableAsyncLogging = true;
	TestSettings.bEnableCompression = false;

	bool bInitSuccess = FLEBqLogBridge::Get().Initialize(TestSettings);
	if (bInitSuccess)
	{
		LE_SYSTEM_LOG(TEXT("BqLog 桥接初始化成功"));
	}
	else
	{
		LE_SYSTEM_ERROR(TEXT("BqLog 桥接初始化失败"));
		return;
	}

	// 2. 测试基础日志输出
	LE_LOG(LogTestSystemBasic, Log, TEXT("LogEverything 系统测试开始"));
	LE_LOG(LogTestSystemBasic, Verbose, TEXT("详细级别测试消息"));
	LE_LOG(LogTestSystemBasic, Warning, TEXT("警告级别测试消息"));
	LE_LOG(LogTestSystemBasic, Error, TEXT("错误级别测试消息"));

	// 3. 测试层级化分类
	LE_LOG(LogGameCombatSkill, Log, TEXT("技能系统日志测试"));
	LE_LOG(LogGameAIBehaviorTree, Verbose, TEXT("AI 行为树调试信息"));
	LE_LOG(LogEngineCoreMemory, Log, TEXT("内存分配跟踪"));

	// 4. 测试不同格式的消息
	int32 TestValue = 42;
	FString TestString = TEXT("测试字符串");
	LE_LOG(LogTestSystemBasic, Log, TEXT("测试参数化消息: 数值=%d, 字符串=%s"), TestValue, *TestString);

	// 5. 强制刷新日志缓冲区
	FLEBqLogBridge::Get().FlushLogs();

	LE_SYSTEM_LOG(TEXT("LogEverything 基础功能测试完成"));
}

/**
 * 在模块加载时自动运行的测试
 */
void RunLogEverythingSystemTests()
{
	// 延迟执行测试，确保引擎完全初始化
	FTimerHandle TestTimerHandle;
	if (GEngine && GEngine->GetWorld())
	{
		GEngine->GetWorld()->GetTimerManager().SetTimer(
			TestTimerHandle,
			FTimerDelegate::CreateStatic(&TestLogEverythingBasicSystem),
			2.0f, // 延迟 2 秒
			false
		);
	}
	else
	{
		// 如果没有 World，直接运行测试
		TestLogEverythingBasicSystem();
	}
}

// 定义测试专用分类
DEFINE_LE_CATEGORY(LogTestSystemBasic);