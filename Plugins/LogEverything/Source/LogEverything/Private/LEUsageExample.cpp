// Copyright Epic Games, Inc. All Rights Reserved.

#include "LETestCategories.h"
#include "LogEverythingUtils.h"

/**
 * LogEverything 使用示例
 * Usage examples for the LogEverything system
 *
 * 这个文件演示了如何使用新的 UE 风格的分类声明和日志记录系统
 */

void ExampleUsage()
{
	// =============================================================================
	// 基础日志记录示例
	// =============================================================================

	// 基本日志记录
	LE_LOG(LogGameCombatSkill, Log, TEXT("Player activated skill system"));

	// 带参数的日志记录
	FString PlayerName = TEXT("TestPlayer");
	FString SkillName = TEXT("FireBall");
	int32 Damage = 150;
	LE_LOG(LogGameCombatDamage, Warning, TEXT("Player %s dealt %d damage with %s"), *PlayerName, Damage, *SkillName);

	// 条件日志记录
	int32 Health = 15;
	LE_CLOG(Health < 20, LogGameCombatDamage, Error, TEXT("Player health critical: %d"), Health);

	// =============================================================================
	// 便利宏使用示例
	// =============================================================================

	// 使用便利宏
	LE_LOG_FATAL(LogGameNetworkSecurity, TEXT("Security breach detected!"));
	LE_LOG_ERROR(LogGameAIPathfinding, TEXT("Failed to find path to target"));
	LE_LOG_WARNING(LogGamePhysicsCollision, TEXT("Collision detection threshold exceeded"));
	LE_LOG_INFO(LogGameRenderingMesh, TEXT("Mesh loaded successfully: %s"), *FString("TestMesh"));
	LE_LOG_VERBOSE(LogGameUIInteraction, TEXT("Button clicked: %s"), *FString("StartButton"));

	// =============================================================================
	// 运行时配置示例
	// =============================================================================

	// 设置特定分类的日志级别
	LE_SET_CATEGORY_LEVEL(LogGameCombatSkill, Warning); // 只显示 Warning 及以上级别
	LE_SET_CATEGORY_LEVEL(LogGameAIBehaviorTree, Verbose); // 显示所有级别

	// 启用/禁用特定分类
	LE_ENABLE_CATEGORY(LogGameNetworkReplication); // 启用网络复制日志
	LE_DISABLE_CATEGORY(LogGameUIInteraction); // 禁用 UI 交互日志

	// 设置全局日志级别
	LE_SET_GLOBAL_LEVEL(Warning); // 全局只显示 Warning 及以上级别

	// =============================================================================
	// 作用域和性能日志示例
	// =============================================================================

	// 作用域日志（自动记录进入和退出）
	{
		// 简单的作用域标记日志
		LE_LOG(LogGameCorePerformance, Verbose, TEXT("ENTER: CriticalSection"));

		// 模拟一些工作
		FPlatformProcess::Sleep(0.1f);

		// 性能日志（自动记录执行时间）
		// 简单的性能记录
		double StartTime = FPlatformTime::Seconds();
		// ... 执行计算 ...
		double Duration = FPlatformTime::Seconds() - StartTime;
		LE_LOG(LogGameCorePerformance, Log, TEXT("ExpensiveCalculation took %.4f seconds"), Duration);

		// 更多工作...
	} // 这里会自动记录退出作用域的日志

	// =============================================================================
	// 断言和检查示例
	// =============================================================================

	UObject* SomeObject = nullptr;

	// 断言检查
	LE_CHECK(IsValid(SomeObject), LogGameCoreMemory, Error, TEXT("Object validation failed"));

	// 确保检查（继续执行，但记录错误）
	LE_ENSURE(SomeObject != nullptr, LogGameCoreMemory, Warning, TEXT("Object is null but continuing"));

	// =============================================================================
	// 调试专用日志示例
	// =============================================================================

#if UE_BUILD_DEBUG
	// 这些日志只在 Debug 构建中编译
	LE_LOG_DEBUG(LogTestUnit, TEXT("Debug information: %s"), *FString("DebugData"));
	LE_CLOG_DEBUG(true, LogTestIntegration, TEXT("Debug condition met"));
#endif

	// =============================================================================
	// 强制刷新日志示例
	// =============================================================================

	// 在关键时刻强制刷新日志缓冲区
	LE_LOG(LogGameNetworkSecurity, Fatal, TEXT("Critical system failure!"));
	LE_FLUSH_LOGS(); // 确保日志立即写入文件
}

/**
 * 演示不同系统中的日志使用
 */
class FExampleGameSystem
{
public:
	void InitializeSystem()
	{
		LE_LOG(LogGameCoreMemory, Log, TEXT("Initializing game system..."));

		// 模拟系统初始化
		bool bInitSuccess = true;

		if (bInitSuccess)
		{
			LE_LOG(LogGameCoreMemory, Log, TEXT("Game system initialized successfully"));
		}
		else
		{
			LE_LOG(LogGameCoreMemory, Error, TEXT("Failed to initialize game system"));
		}
	}

	void ProcessGameplay()
	{
		// AI 系统处理
		LE_LOG(LogGameAIBehaviorTree, Verbose, TEXT("Processing AI behavior tree"));

		// 物理系统处理
		LE_LOG(LogGamePhysicsSimulation, Verbose, TEXT("Running physics simulation"));

		// 渲染系统处理
		LE_LOG(LogGameRenderingMesh, Verbose, TEXT("Rendering frame meshes"));
	}

	void HandleNetworking()
	{
		// 网络连接处理
		LE_LOG(LogGameNetworkConnection, Log, TEXT("Handling network connections"));

		// 网络复制处理
		LE_LOG(LogGameNetworkReplication, Verbose, TEXT("Replicating game state"));
	}

	void Shutdown()
	{
		LE_LOG(LogGameCoreMemory, Log, TEXT("Shutting down game system..."));

		// 确保所有日志都被写入
		LE_FLUSH_LOGS();

		LE_LOG(LogGameCoreMemory, Log, TEXT("Game system shutdown complete"));
	}
};

/**
 * 演示编辑器工具中的日志使用
 */
#if WITH_EDITOR
class FExampleEditorTool
{
public:
	void ImportAsset(const FString& AssetPath)
	{
		LE_LOG(LogEditorAssetImport, Log, TEXT("Starting asset import: %s"), *AssetPath);

		// 模拟资产导入过程
		bool bImportSuccess = true;

		if (bImportSuccess)
		{
			LE_LOG(LogEditorAssetImport, Log, TEXT("Asset imported successfully: %s"), *AssetPath);
		}
		else
		{
			LE_LOG(LogEditorAssetImport, Error, TEXT("Failed to import asset: %s"), *AssetPath);
		}
	}

	void CompileBlueprint(const FString& BlueprintPath)
	{
		// 简单的作用域标记日志
		LE_LOG(LogEditorToolsBlueprint, Verbose, TEXT("ENTER: BlueprintCompilation"));

		LE_LOG(LogEditorToolsBlueprint, Log, TEXT("Compiling blueprint: %s"), *BlueprintPath);

		// 性能监控
		// 简单的性能记录
		double CompileStartTime = FPlatformTime::Seconds();
		// ... 编译过程 ...
		double CompileDuration = FPlatformTime::Seconds() - CompileStartTime;
		LE_LOG(LogEditorToolsBlueprint, Log, TEXT("CompilationTime took %.4f seconds"), CompileDuration);

		// 模拟编译过程
		FPlatformProcess::Sleep(0.05f);

		LE_LOG(LogEditorToolsBlueprint, Log, TEXT("Blueprint compilation completed: %s"), *BlueprintPath);
	}
};
#endif

/**
 * 演示测试系统中的日志使用
 */
class FExampleTestSystem
{
public:
	void RunUnitTests()
	{
		LE_LOG(LogTestUnit, Log, TEXT("Starting unit test suite"));

		int32 TestsRun = 0;
		int32 TestsPassed = 0;

		// 模拟运行多个测试
		for (int32 i = 0; i < 10; ++i)
		{
			FString TestName = FString::Printf(TEXT("UnitTest_%d"), i);
			LE_LOG(LogTestUnit, Verbose, TEXT("Running test: %s"), *TestName);

			bool bTestPassed = (i % 3 != 0); // 模拟一些测试失败
			TestsRun++;

			if (bTestPassed)
			{
				TestsPassed++;
				LE_LOG(LogTestUnit, Verbose, TEXT("Test passed: %s"), *TestName);
			}
			else
			{
				LE_LOG(LogTestUnit, Warning, TEXT("Test failed: %s"), *TestName);
			}
		}

		LE_LOG(LogTestUnit, Log, TEXT("Unit test suite completed: %d/%d tests passed"), TestsPassed, TestsRun);
	}

	void RunPerformanceBenchmark()
	{
		LE_LOG(LogTestPerformance, Log, TEXT("Starting performance benchmark"));

		{
			// 简单的性能记录
			double BenchStartTime = FPlatformTime::Seconds();
			// ... 基准测试 ...
			double BenchDuration = FPlatformTime::Seconds() - BenchStartTime;
			LE_LOG(LogTestBenchmark, Log, TEXT("BenchmarkExecution took %.4f seconds"), BenchDuration);

			// 模拟性能密集型操作
			for (int32 i = 0; i < 1000000; ++i)
			{
				volatile float Dummy = FMath::Sin(i * 0.001f);
			}
		}

		LE_LOG(LogTestPerformance, Log, TEXT("Performance benchmark completed"));
	}
};