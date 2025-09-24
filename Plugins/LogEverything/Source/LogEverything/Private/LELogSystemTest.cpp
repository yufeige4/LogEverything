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

	// 4. 测试 BqLog 现代化格式的消息（使用 {} 占位符）
	int32 TestValue = 42;
	FString TestString = TEXT("测试字符串");
	float FloatValue = 3.14159f;
	bool BoolValue = true;
	LE_LOG(LogTestSystemBasic, Log, TEXT("测试现代化参数化消息: 数值={}, 字符串={}, 浮点={:.2f}, 布尔={}"), TestValue, *TestString, FloatValue, BoolValue);

	// 5. 测试条件日志和便利宏
	int32 HealthValue = 15;
	FString PlayerName = TEXT("TestPlayer");
	LE_CLOG(HealthValue < 20, LogTestSystemBasic, Warning, TEXT("玩家 {} 生命值过低: {}"), *PlayerName, HealthValue);

	// 6. 测试便利宏
	LE_LOG_INFO(LogTestSystemBasic, TEXT("便利宏测试: 信息级别"));
	LE_LOG_WARNING(LogTestSystemBasic, TEXT("便利宏测试: 警告级别"));
	LE_LOG_ERROR(LogTestSystemBasic, TEXT("便利宏测试: 错误级别"));

	// 7. 测试不同精度的浮点数格式化
	double PrecisionValue = 123.456789;
	LE_LOG(LogTestSystemBasic, Log, TEXT("精度测试: {:.1f}, {:.3f}, {:.6f}"), PrecisionValue, PrecisionValue, PrecisionValue);

	// 8. 测试 FName 和字符串转换
	FName TestName = FName(TEXT("TestCategory"));
	LE_LOG(LogTestSystemBasic, Log, TEXT("FName 测试: {}"), *TestName.ToString());

	// 9. 强制刷新日志缓冲区
	FLEBqLogBridge::Get().FlushLogs();

	LE_SYSTEM_LOG(TEXT("LogEverything 现代化格式和重构功能测试完成"));
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