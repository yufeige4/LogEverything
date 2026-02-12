// Copyright Benedict Guo. All Rights Reserved.

/**
 * LogEverything 性能基准测试
 * Performance benchmark tests for LogEverything system
 *
 * 测试用例设计：
 * - PERF-01: UE_LOG 基准（100W 条日志）
 * - PERF-02: LE_LOG 基准（100W 条日志）
 * - PERF-03: UE_LOG 格式化（带变量参数）
 * - PERF-04: LE_LOG 格式化（带变量参数）
 * - PERF-05: LE_LOG 级别过滤（被过滤不输出）
 * - PERF-06: LE_LOG 分类禁用（被禁用不输出）
 * - PERF-07: 多线程 UE_LOG（4 线程并行）
 * - PERF-08: 多线程 LE_LOG（4 线程并行）
 *
 * 注意：LE_LOG 测试需要在游戏实例中运行（PIE 模式），
 * 因为 ULELogSubsystem 是 GameInstanceSubsystem，需要 GameInstance 才能正常初始化。
 *
 * 输出：CSV 文件到 Saved/LogEverything/PerfReport/perf_results.csv
 */

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Async/ParallelFor.h"
#include "Misc/Paths.h"
#include "Category/LECategoryTree.h"
#include "Macros/LELogMacros.h"
#include "System/LELogTypes.h"
#include "System/LELogSubsystem.h"
#include "Bridge/LEBqLogBridge.h"

// 声明性能测试分类
DECLARE_LE_CATEGORY_EXTERN(LELogPerfTest, Test.Performance);
DEFINE_LE_CATEGORY(LELogPerfTest);

// 测试结果数据结构
struct FLEPerfTestResult
{
	FString TestName;
	int32 LogCount;
	double TotalTimeMs;
	double AvgTimeNs;
	double Throughput;
	FString Platform;
	FString BuildConfig;
};

// 全局结果存储（用于跨测试收集数据）
static TArray<FLEPerfTestResult> GPerfResults;
static FCriticalSection GPerfResultsLock;

/**
 * 添加测试结果到全局数组
 */
static void AddPerfResult(const FLEPerfTestResult& Result)
{
	FScopeLock Lock(&GPerfResultsLock);
	GPerfResults.Add(Result);
}

/**
 * 获取平台和构建配置信息
 */
static FString GetPlatformString()
{
#if PLATFORM_WINDOWS
	return TEXT("Win64");
#elif PLATFORM_MAC
	return TEXT("Mac");
#elif PLATFORM_LINUX
	return TEXT("Linux");
#else
	return TEXT("Unknown");
#endif
}

static FString GetBuildConfigString()
{
#if UE_BUILD_DEBUG
	return TEXT("Debug");
#elif UE_BUILD_DEVELOPMENT
	return TEXT("Development");
#elif UE_BUILD_SHIPPING
	return TEXT("Shipping");
#else
	return TEXT("Unknown");
#endif
}

/**
 * 检查 LogEverything 系统是否已初始化
 * 需要在游戏实例中运行才能正常初始化
 */
static bool IsLogEverythingInitialized()
{
	FLEBqLogBridge& Bridge = FLEBqLogBridge::Get();
	return Bridge.IsInitialized();
}

/**
 * 创建测试用分类树
 */
static ULECategoryTree* CreatePerfTestCategoryTree()
{
	ULECategoryTree* Tree = NewObject<ULECategoryTree>();

	TArray<FString> CategoryPaths = {
		TEXT("Engine"),
		TEXT("Game"),
		TEXT("Game.Combat.Damage"),
		TEXT("Game.Combat.Skill"),
		TEXT("Game.AI"),
		TEXT("Editor"),
		TEXT("Test"),
		TEXT("Test.Performance")
	};

	Tree->InitializeTree(CategoryPaths);
	Tree->SetDefaultGlobalLevel(ELELogVerbosity::Info);

	return Tree;
}

/**
 * 导出结果到 CSV 文件
 */
static void ExportResultsToCSV(FAutomationTestBase* Test)
{
	FScopeLock Lock(&GPerfResultsLock);

	if (GPerfResults.Num() == 0)
	{
		Test->AddWarning(TEXT("No performance results to export"));
		return;
	}

	TArray<FString> Lines;
	Lines.Add(TEXT("TestCase,LogCount,TotalTimeMs,AvgTimeNs,Throughput,Platform,BuildConfig"));

	for (const FLEPerfTestResult& R : GPerfResults)
	{
		Lines.Add(FString::Printf(TEXT("%s,%d,%.2f,%.2f,%.0f,%s,%s"),
			*R.TestName, R.LogCount, R.TotalTimeMs, R.AvgTimeNs, R.Throughput,
			*R.Platform, *R.BuildConfig));
	}

	FString OutputDir = FPaths::ProjectSavedDir() / TEXT("LogEverything/PerfReport");
	IFileManager::Get().MakeDirectory(*OutputDir, true);

	FString OutputPath = OutputDir / TEXT("perf_results.csv");
	if (FFileHelper::SaveStringArrayToFile(Lines, *OutputPath))
	{
		Test->AddInfo(FString::Printf(TEXT("CSV exported to: %s"), *OutputPath));
	}
	else
	{
		Test->AddError(FString::Printf(TEXT("Failed to export CSV to: %s"), *OutputPath));
	}
}

// ============================================================================
// PERF-01: UE_LOG 基准测试
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_01_UE_LOG_Baseline,
	"LogEverything.Performance.PERF-01 UE_LOG Baseline",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_01_UE_LOG_Baseline::RunTest(const FString& Parameters)
{
	// 清空之前的结果（作为第一个测试）
	{
		FScopeLock Lock(&GPerfResultsLock);
		GPerfResults.Empty();
	}

	const int32 LogCount = 1000000;
	const int32 WarmupCount = 10000;

	const int32 TestInt = 12345;
	const float TestFloat = 3.14159f;
	const FString TestString = TEXT("PerfTest");

	AddInfo(TEXT("PERF-01: UE_LOG Baseline - 1M logs with 3 parameters"));
	AddInfo(FString::Printf(TEXT("  Warmup: %d logs"), WarmupCount));

	// 预热
	for (int32 i = 0; i < WarmupCount; ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("Warmup %d %.2f %s"), TestInt, TestFloat, *TestString);
	}

	// 正式测试
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < LogCount; ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("Test %d %.2f %s"), TestInt, TestFloat, *TestString);
	}
	double EndTime = FPlatformTime::Seconds();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-01_UE_LOG_Baseline");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/log, Throughput: %.0f logs/sec"),
		TotalMs, AvgNs, Throughput));

	return true;
}

// ============================================================================
// PERF-02: LE_LOG 基准测试（使用标准 LE_LOG 宏）
// 需要在 PIE 模式下运行，因为 ULELogSubsystem 是 GameInstanceSubsystem
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_02_LE_LOG_Baseline,
	"LogEverything.Performance.PERF-02 LE_LOG Baseline",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_02_LE_LOG_Baseline::RunTest(const FString& Parameters)
{
	// 检查 LogEverything 是否已初始化
	if (!IsLogEverythingInitialized())
	{
		AddError(TEXT("LogEverything is not initialized. Please run this test in PIE mode (Play In Editor) so that ULELogSubsystem can initialize properly."));
		AddInfo(TEXT("Steps: 1. Start PIE  2. Open Session Frontend  3. Run Performance tests"));
		return false;
	}

	const int32 LogCount = 1000000;
	const int32 WarmupCount = 10000;

	const int32 TestInt = 12345;
	const float TestFloat = 3.14159f;
	const FString TestString = TEXT("PerfTest");

	AddInfo(TEXT("PERF-02: LE_LOG Baseline - 1M logs with 3 parameters"));
	AddInfo(FString::Printf(TEXT("  Warmup: %d logs"), WarmupCount));

	// 预热
	for (int32 i = 0; i < WarmupCount; ++i)
	{
		LE_LOG(LELogPerfTest, Info, TEXT("Warmup {} {:.2f} {}"), TestInt, TestFloat, TestString);
	}

	// 正式测试
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < LogCount; ++i)
	{
		LE_LOG(LELogPerfTest, Info, TEXT("Test {} {:.2f} {}"), TestInt, TestFloat, TestString);
	}
	double EndTime = FPlatformTime::Seconds();

	// 刷新日志确保写入完成
	LE_FLUSH_LOGS();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-02_LE_LOG_Baseline");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/log, Throughput: %.0f logs/sec"),
		TotalMs, AvgNs, Throughput));

	return true;
}

// ============================================================================
// PERF-03: UE_LOG 格式化测试
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_03_UE_LOG_Format,
	"LogEverything.Performance.PERF-03 UE_LOG Format",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_03_UE_LOG_Format::RunTest(const FString& Parameters)
{
	const int32 LogCount = 1000000;
	const float TestFloat = 3.14159f;
	const FString TestString = TEXT("FormatTest");

	AddInfo(TEXT("PERF-03: UE_LOG Format - 1M logs with dynamic index"));

	// 正式测试（带动态索引）
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < LogCount; ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("Index=%d Value=%.2f Name=%s"), i, TestFloat + i, *TestString);
	}
	double EndTime = FPlatformTime::Seconds();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-03_UE_LOG_Format");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/log, Throughput: %.0f logs/sec"),
		TotalMs, AvgNs, Throughput));

	return true;
}

// ============================================================================
// PERF-04: LE_LOG 格式化测试（使用标准 LE_LOG 宏）
// 需要在 PIE 模式下运行
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_04_LE_LOG_Format,
	"LogEverything.Performance.PERF-04 LE_LOG Format",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_04_LE_LOG_Format::RunTest(const FString& Parameters)
{
	if (!IsLogEverythingInitialized())
	{
		AddError(TEXT("LogEverything is not initialized. Please run this test in PIE mode."));
		return false;
	}

	const int32 LogCount = 1000000;
	const float TestFloat = 3.14159f;
	const FString TestString = TEXT("FormatTest");

	AddInfo(TEXT("PERF-04: LE_LOG Format - 1M logs with dynamic index"));

	// 正式测试（带动态索引）
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < LogCount; ++i)
	{
		LE_LOG(LELogPerfTest, Info, TEXT("Index={} Value={:.2f} Name={}"), i, TestFloat + i, TestString);
	}
	double EndTime = FPlatformTime::Seconds();

	LE_FLUSH_LOGS();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-04_LE_LOG_Format");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/log, Throughput: %.0f logs/sec"),
		TotalMs, AvgNs, Throughput));

	return true;
}

// ============================================================================
// PERF-05: LE_LOG 级别过滤测试
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_05_LE_LOG_LevelFilter,
	"LogEverything.Performance.PERF-05 LE_LOG Level Filter",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_05_LE_LOG_LevelFilter::RunTest(const FString& Parameters)
{
	const int32 LogCount = 1000000;

	AddInfo(TEXT("PERF-05: LE_LOG Level Filter - 1M logs filtered by level"));
	AddInfo(TEXT("  Testing ShouldLogCategory overhead when logs are filtered"));

	// 创建独立的分类树进行测试
	ULECategoryTree* Tree = CreatePerfTestCategoryTree();

	// 设置 Test.Performance 级别为 Warning，使 Debug 级别被过滤
	Tree->SetCategoryLevel(TEXT("Test.Performance"), ELELogVerbosity::Warning, false);

	// 测试 ShouldLogCategory 的过滤开销
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < LogCount; ++i)
	{
		// 模拟 LE_LOG 的过滤检查（Debug < Warning，会被过滤）
		if (Tree->ShouldLogCategory(FName(TEXT("Test.Performance")), ELELogVerbosity::Debug))
		{
			// 这里不会执行，因为被过滤了
		}
	}
	double EndTime = FPlatformTime::Seconds();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-05_LE_LOG_LevelFilter");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/check, Throughput: %.0f checks/sec"),
		TotalMs, AvgNs, Throughput));
	AddInfo(TEXT("  (Measures filter overhead only, no actual log output)"));

	return true;
}

// ============================================================================
// PERF-06: LE_LOG 分类禁用测试
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_06_LE_LOG_CategoryDisabled,
	"LogEverything.Performance.PERF-06 LE_LOG Category Disabled",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_06_LE_LOG_CategoryDisabled::RunTest(const FString& Parameters)
{
	const int32 LogCount = 1000000;

	AddInfo(TEXT("PERF-06: LE_LOG Category Disabled - 1M logs with disabled category"));
	AddInfo(TEXT("  Testing ShouldLogCategory overhead when category is disabled"));

	// 创建独立的分类树进行测试
	ULECategoryTree* Tree = CreatePerfTestCategoryTree();

	// 禁用 Test.Performance 分类
	Tree->SetCategoryEnabled(TEXT("Test.Performance"), false, false);

	// 测试禁用分类的检查开销
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < LogCount; ++i)
	{
		// 模拟 LE_LOG 的禁用检查
		if (Tree->ShouldLogCategory(FName(TEXT("Test.Performance")), ELELogVerbosity::Info))
		{
			// 这里不会执行，因为分类被禁用
		}
	}
	double EndTime = FPlatformTime::Seconds();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-06_LE_LOG_CategoryDisabled");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/check, Throughput: %.0f checks/sec"),
		TotalMs, AvgNs, Throughput));
	AddInfo(TEXT("  (Measures disable check overhead only, no actual log output)"));

	return true;
}

// ============================================================================
// PERF-07: 多线程 UE_LOG 测试
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_07_UE_LOG_MultiThread,
	"LogEverything.Performance.PERF-07 UE_LOG MultiThread",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_07_UE_LOG_MultiThread::RunTest(const FString& Parameters)
{
	const int32 LogCount = 1000000;
	const int32 ThreadCount = 4;
	const int32 LogsPerThread = LogCount / ThreadCount;
	const float TestFloat = 3.14159f;
	const FString TestString = TEXT("MTTest");

	AddInfo(TEXT("PERF-07: UE_LOG MultiThread - 1M logs across 4 threads"));
	AddInfo(FString::Printf(TEXT("  %d logs per thread"), LogsPerThread));

	// 多线程测试
	double StartTime = FPlatformTime::Seconds();
	ParallelFor(ThreadCount, [&](int32 ThreadIndex)
	{
		for (int32 i = 0; i < LogsPerThread; ++i)
		{
			UE_LOG(LogTemp, Log, TEXT("Thread %d Index %d %.2f %s"),
				ThreadIndex, i, TestFloat, *TestString);
		}
	});
	double EndTime = FPlatformTime::Seconds();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-07_UE_LOG_MultiThread");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/log, Throughput: %.0f logs/sec"),
		TotalMs, AvgNs, Throughput));

	return true;
}

// ============================================================================
// PERF-08: 多线程 LE_LOG 测试（使用标准 LE_LOG 宏）
// 需要在 PIE 模式下运行
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_08_LE_LOG_MultiThread,
	"LogEverything.Performance.PERF-08 LE_LOG MultiThread",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_08_LE_LOG_MultiThread::RunTest(const FString& Parameters)
{
	if (!IsLogEverythingInitialized())
	{
		AddError(TEXT("LogEverything is not initialized. Please run this test in PIE mode."));
		return false;
	}

	const int32 LogCount = 1000000;
	const int32 ThreadCount = 4;
	const int32 LogsPerThread = LogCount / ThreadCount;
	const float TestFloat = 3.14159f;
	const FString TestString = TEXT("MTTest");

	AddInfo(TEXT("PERF-08: LE_LOG MultiThread - 1M logs across 4 threads"));
	AddInfo(FString::Printf(TEXT("  %d logs per thread"), LogsPerThread));

	// 多线程测试
	double StartTime = FPlatformTime::Seconds();
	ParallelFor(ThreadCount, [&](int32 ThreadIndex)
	{
		for (int32 i = 0; i < LogsPerThread; ++i)
		{
			LE_LOG(LELogPerfTest, Info, TEXT("Thread {} Index {} {:.2f} {}"),
				ThreadIndex, i, TestFloat, TestString);
		}
	});
	double EndTime = FPlatformTime::Seconds();

	LE_FLUSH_LOGS();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-08_LE_LOG_MultiThread");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/log, Throughput: %.0f logs/sec"),
		TotalMs, AvgNs, Throughput));

	// 作为最后一个测试，导出 CSV
	ExportResultsToCSV(this);

	return true;
}

// ============================================================================
// PERF-09: LE_LOG Trace 分析测试（20 万条，用于 Unreal Insights 分析）
// 此测试专门用于在 Unreal Insights 中分析 LE_LOG 的性能瓶颈
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_09_LE_LOG_TraceAnalysis,
	"LogEverything.Performance.PERF-09 LE_LOG Trace Analysis (200K)",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_09_LE_LOG_TraceAnalysis::RunTest(const FString& Parameters)
{
	if (!IsLogEverythingInitialized())
	{
		AddError(TEXT("LogEverything is not initialized. Please run this test in PIE mode."));
		return false;
	}

	const int32 LogCount = 200000; // 20 万条，足够分析但不会太久
	const int32 TestInt = 12345;
	const float TestFloat = 3.14159f;
	const FString TestString = TEXT("TraceTest");

	AddInfo(TEXT("PERF-09: LE_LOG Trace Analysis - 200K logs for Unreal Insights profiling"));
	AddInfo(TEXT("  Use Unreal Insights to analyze: LE_InternalLogImp, LE_ShouldLogCategory, LE_BqLogWrite"));

	// 正式测试
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < LogCount; ++i)
	{
		LE_LOG(LELogPerfTest, Info, TEXT("Trace {} {:.2f} {}"), TestInt, TestFloat, TestString);
	}
	double EndTime = FPlatformTime::Seconds();

	LE_FLUSH_LOGS();

	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / LogCount;
	double Throughput = LogCount / (TotalMs / 1000.0);

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-09_LE_LOG_TraceAnalysis");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgNs;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	AddInfo(FString::Printf(TEXT("  Total: %.2f ms, Avg: %.0f ns/log, Throughput: %.0f logs/sec"),
		TotalMs, AvgNs, Throughput));
	AddInfo(TEXT("  Open the .utrace file in Unreal Insights to analyze performance breakdown."));

	return true;
}

// ============================================================================
// PERF-10: LE_LOG 延迟分布测试（检测阻塞和延迟尖峰）
// 此测试记录每条日志的耗时，统计延迟分布和最大延迟
// 用于验证 ReliableLevel + BufferSize 配置是否合理
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEPerfTest_10_LE_LOG_LatencyDistribution,
	"LogEverything.Performance.PERF-10 LE_LOG Latency Distribution",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FLEPerfTest_10_LE_LOG_LatencyDistribution::RunTest(const FString& Parameters)
{
	if (!IsLogEverythingInitialized())
	{
		AddError(TEXT("LogEverything is not initialized. Please run this test in PIE mode."));
		return false;
	}

	const int32 LogCount = 100000; // 10 万条
	const int32 TestInt = 12345;
	const float TestFloat = 3.14159f;
	const FString TestString = TEXT("LatencyTest");

	// 获取当前配置信息（通过 LogSubsystem）
	ULELogSubsystem* LogSubsystem = FLEBqLogBridge::Get().GetLogSubsystem();
	FString ConfigInfo = TEXT("Unknown");
	if (LogSubsystem)
	{
		// 配置信息会在日志中显示，这里只是提示
		ConfigInfo = TEXT("See log output for current BqLog configuration");
	}

	AddInfo(TEXT("PERF-10: LE_LOG Latency Distribution - 100K logs with per-log timing"));
	AddInfo(FString::Printf(TEXT("  Config: %s"), *ConfigInfo));
	AddInfo(TEXT("  This test measures individual log latency to detect blocking spikes."));
	AddInfo(TEXT("  Recommended configs:"));
	AddInfo(TEXT("    - Low + 1MB: High performance, may discard logs when buffer full"));
	AddInfo(TEXT("    - Normal + 16MB: No log loss, may block if buffer full"));

	// 存储每条日志的耗时（纳秒）
	TArray<double> Latencies;
	Latencies.SetNumUninitialized(LogCount);

	// 正式测试 - 记录每条日志的耗时
	double TotalStartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < LogCount; ++i)
	{
		double LogStartTime = FPlatformTime::Seconds();
		LE_LOG(LELogPerfTest, Info, TEXT("Latency {} {:.2f} {}"), TestInt, TestFloat, TestString);
		double LogEndTime = FPlatformTime::Seconds();
		Latencies[i] = (LogEndTime - LogStartTime) * 1000000000.0; // 转换为纳秒
	}
	double TotalEndTime = FPlatformTime::Seconds();

	LE_FLUSH_LOGS();

	// 统计延迟分布
	double MinLatency = Latencies[0];
	double MaxLatency = Latencies[0];
	double SumLatency = 0.0;
	int32 Over1usCount = 0;   // > 1 微秒
	int32 Over10usCount = 0;  // > 10 微秒
	int32 Over100usCount = 0; // > 100 微秒
	int32 Over1msCount = 0;   // > 1 毫秒
	int32 Over10msCount = 0;  // > 10 毫秒

	for (int32 i = 0; i < LogCount; ++i)
	{
		double Latency = Latencies[i];
		SumLatency += Latency;
		if (Latency < MinLatency) MinLatency = Latency;
		if (Latency > MaxLatency) MaxLatency = Latency;

		if (Latency > 1000.0) Over1usCount++;      // > 1us
		if (Latency > 10000.0) Over10usCount++;    // > 10us
		if (Latency > 100000.0) Over100usCount++;  // > 100us
		if (Latency > 1000000.0) Over1msCount++;   // > 1ms
		if (Latency > 10000000.0) Over10msCount++; // > 10ms
	}

	double AvgLatency = SumLatency / LogCount;
	double TotalMs = (TotalEndTime - TotalStartTime) * 1000.0;
	double Throughput = LogCount / (TotalMs / 1000.0);

	// 计算百分位数（需要排序）
	Latencies.Sort();
	double P50 = Latencies[LogCount * 50 / 100];
	double P90 = Latencies[LogCount * 90 / 100];
	double P95 = Latencies[LogCount * 95 / 100];
	double P99 = Latencies[LogCount * 99 / 100];
	double P999 = Latencies[LogCount * 999 / 1000];

	// 输出结果
	AddInfo(TEXT(""));
	AddInfo(TEXT("=== Latency Distribution Results ==="));
	AddInfo(FString::Printf(TEXT("  Total Time: %.2f ms"), TotalMs));
	AddInfo(FString::Printf(TEXT("  Throughput: %.0f logs/sec"), Throughput));
	AddInfo(TEXT(""));
	AddInfo(TEXT("  Latency Stats (nanoseconds):"));
	AddInfo(FString::Printf(TEXT("    Min: %.0f ns"), MinLatency));
	AddInfo(FString::Printf(TEXT("    Max: %.0f ns (%.3f ms)"), MaxLatency, MaxLatency / 1000000.0));
	AddInfo(FString::Printf(TEXT("    Avg: %.0f ns"), AvgLatency));
	AddInfo(TEXT(""));
	AddInfo(TEXT("  Percentiles:"));
	AddInfo(FString::Printf(TEXT("    P50: %.0f ns"), P50));
	AddInfo(FString::Printf(TEXT("    P90: %.0f ns"), P90));
	AddInfo(FString::Printf(TEXT("    P95: %.0f ns"), P95));
	AddInfo(FString::Printf(TEXT("    P99: %.0f ns (%.3f us)"), P99, P99 / 1000.0));
	AddInfo(FString::Printf(TEXT("    P99.9: %.0f ns (%.3f us)"), P999, P999 / 1000.0));
	AddInfo(TEXT(""));
	AddInfo(TEXT("  Spike Analysis:"));
	AddInfo(FString::Printf(TEXT("    > 1us:   %d (%.2f%%)"), Over1usCount, 100.0 * Over1usCount / LogCount));
	AddInfo(FString::Printf(TEXT("    > 10us:  %d (%.2f%%)"), Over10usCount, 100.0 * Over10usCount / LogCount));
	AddInfo(FString::Printf(TEXT("    > 100us: %d (%.4f%%)"), Over100usCount, 100.0 * Over100usCount / LogCount));
	AddInfo(FString::Printf(TEXT("    > 1ms:   %d (%.4f%%)"), Over1msCount, 100.0 * Over1msCount / LogCount));
	AddInfo(FString::Printf(TEXT("    > 10ms:  %d (%.4f%%) << BLOCKING DETECTED"), Over10msCount, 100.0 * Over10msCount / LogCount));

	// 判断测试结果
	if (Over10msCount > 0)
	{
		AddWarning(FString::Printf(TEXT("BLOCKING DETECTED: %d logs took >10ms. Consider using Low reliable_level or larger buffer."), Over10msCount));
	}
	else if (Over1msCount > 0)
	{
		AddWarning(FString::Printf(TEXT("High latency spikes detected: %d logs took >1ms."), Over1msCount));
	}
	else
	{
		AddInfo(TEXT("  No significant blocking detected. Configuration looks good!"));
	}

	FLEPerfTestResult Result;
	Result.TestName = TEXT("PERF-10_LE_LOG_LatencyDistribution");
	Result.LogCount = LogCount;
	Result.TotalTimeMs = TotalMs;
	Result.AvgTimeNs = AvgLatency;
	Result.Throughput = Throughput;
	Result.Platform = GetPlatformString();
	Result.BuildConfig = GetBuildConfigString();

	AddPerfResult(Result);

	return true;
}
