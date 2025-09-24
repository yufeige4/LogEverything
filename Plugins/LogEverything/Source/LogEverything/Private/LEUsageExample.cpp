// Copyright Epic Games, Inc. All Rights Reserved.

#include "LETestCategories.h"
#include "LogEverythingUtils.h"

/**
 * LogEverything 使用示例
 * Usage examples for the LogEverything system
 *
 * 这个文件演示了如何使用 BqLog 现代化格式化和 UE 风格的分类声明系统
 * 重要：现在使用 {} 占位符而不是传统的 %s、%d 格式
 * 可以直接传递 FString、FName、FText 等 UE 类型，无需解引用
 */

void ExampleUsage()
{
	// =============================================================================
	// 基础日志记录示例
	// =============================================================================

	// 基本日志记录
	LE_LOG(LogGameCombatSkill, Log, TEXT("Player activated skill system"));

	// 带参数的日志记录 - 使用 BqLog 现代化 {} 占位符格式
	FString PlayerName = TEXT("TestPlayer");
	FString SkillName = TEXT("FireBall");
	int32 Damage = 150;
	float CriticalChance = 0.75f;
	LE_LOG(LogGameCombatDamage, Warning, TEXT("Player {} dealt {} damage with {} (crit chance: {:.2f})"), *PlayerName, Damage, *SkillName, CriticalChance);

	// 条件日志记录 - 展示 BqLog 对 Unreal 类型的处理
	int32 Health = 15;
	FName PlayerState = FName(TEXT("CriticalHealth"));
	FText StatusMessage = FText::FromString(TEXT("需要治疗"));
	LE_CLOG(Health < 20, LogGameCombatDamage, Error, TEXT("Player health critical: {}, status: {}, message: {}"), Health, *PlayerState.ToString(), *StatusMessage.ToString());

	// =============================================================================
	// 便利宏使用示例
	// =============================================================================

	// 使用便利宏 - 展示各种类型的 BqLog 格式化
	FString ThreatLevel = TEXT("Critical");
	LE_LOG_FATAL(LogGameNetworkSecurity, TEXT("Security breach detected! Threat level: {}"), *ThreatLevel);

	FString TargetLocation = TEXT("NavMesh_Point_142");
	LE_LOG_ERROR(LogGameAIPathfinding, TEXT("Failed to find path to target: {}"), *TargetLocation);

	float CollisionThreshold = 0.85f;
	bool bThresholdExceeded = true;
	LE_LOG_WARNING(LogGamePhysicsCollision, TEXT("Collision detection threshold exceeded: {:.3f}, critical: {}"), CollisionThreshold, bThresholdExceeded);

	FString MeshName = TEXT("SM_Character_Helmet");
	int32 VertexCount = 1024;
	LE_LOG_INFO(LogGameRenderingMesh, TEXT("Mesh loaded successfully: {}, vertices: {}"), *MeshName, VertexCount);

	FName ButtonName = FName(TEXT("btn_StartGame"));
	double ClickTime = FPlatformTime::Seconds();
	LE_LOG_VERBOSE(LogGameUIInteraction, TEXT("Button clicked: {}, timestamp: {:.6f}"), *ButtonName.ToString(), ClickTime);

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

		// 性能日志 - 展示高精度浮点数格式化
		FString OperationName = TEXT("PathfindingAlgorithm");
		double StartTime = FPlatformTime::Seconds();
		// ... 执行计算 ...
		double Duration = FPlatformTime::Seconds() - StartTime;
		int32 NodesProcessed = 1500;
		LE_LOG(LogGameCorePerformance, Log, TEXT("{} completed in {:.4f} seconds, processed {} nodes"), *OperationName, Duration, NodesProcessed);

		// 更多工作...
	} // 这里会自动记录退出作用域的日志

	// =============================================================================
	// 断言和检查示例
	// =============================================================================

	UObject* SomeObject = nullptr;

	// 断言检查 - 展示 nullptr 和对象信息的格式化
	FString ObjectTypeName = TEXT("UGameplayEffect");
	void* ObjectPtr = SomeObject;
	LE_CHECK(IsValid(SomeObject), LogGameCoreMemory, Error, TEXT("Object validation failed: {} at address {}"), *ObjectTypeName, ObjectPtr);

	// 确保检查 - 展示布尔值和条件信息的格式化
	bool bShouldBeValid = true;
	FText ErrorContext = FText::FromString(TEXT("初始化阶段"));
	LE_ENSURE(SomeObject != nullptr, LogGameCoreMemory, Warning, TEXT("Object is null but continuing, expected valid: {}, context: {}"), bShouldBeValid, *ErrorContext.ToString());

	// =============================================================================
	// 调试专用日志示例
	// =============================================================================

#if UE_BUILD_DEBUG
	// 这些日志只在 Debug 构建中编译 - 展示调试信息的现代化格式
	FString DebugData = TEXT("MemoryPool_Status");
	int32 AllocatedBytes = 2048;
	double MemoryUsagePercent = 0.67;
	LE_LOG_DEBUG(LogTestUnit, TEXT("Debug information: {}, allocated: {} bytes, usage: {:.1f}%"), DebugData, AllocatedBytes, MemoryUsagePercent * 100.0);

	bool bConditionMet = true;
	FName TestSuiteName = FName(TEXT("IntegrationTestSuite_v2"));
	LE_CLOG_DEBUG(bConditionMet, LogTestIntegration, TEXT("Debug condition met: {}, suite: {}"), bConditionMet, TestSuiteName);
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
		FString SystemName = TEXT("GameplaySubsystem");
		FString Version = TEXT("v1.2.3");
		LE_LOG(LogGameCoreMemory, Log, TEXT("Initializing game system: {} {}"), *SystemName, *Version);

		// 模拟系统初始化
		bool bInitSuccess = true;

		if (bInitSuccess)
		{
			double InitTime = 1.234;
			int32 ModulesLoaded = 15;
			LE_LOG(LogGameCoreMemory, Log, TEXT("Game system initialized successfully in {:.3f}s, loaded {} modules"), InitTime, ModulesLoaded);
		}
		else
		{
			FString ErrorCode = TEXT("INIT_ERR_001");
			FText ErrorMessage = FText::FromString(TEXT("模块依赖失败"));
			LE_LOG(LogGameCoreMemory, Error, TEXT("Failed to initialize game system: {} - {}"), *ErrorCode, *ErrorMessage.ToString());
		}
	}

	void ProcessGameplay()
	{
		// AI 系统处理 - 展示 AI 相关参数
		FName AICharacterName = FName(TEXT("EnemyGuard_01"));
		int32 ActiveNodes = 7;
		LE_LOG(LogGameAIBehaviorTree, Verbose, TEXT("Processing AI behavior tree for {}, active nodes: {}"), *AICharacterName.ToString(), ActiveNodes);

		// 物理系统处理 - 展示物理参数
		float DeltaTime = 0.0166f;
		int32 ActiveBodies = 234;
		LE_LOG(LogGamePhysicsSimulation, Verbose, TEXT("Running physics simulation, deltaTime: {:.4f}, bodies: {}"), DeltaTime, ActiveBodies);

		// 渲染系统处理 - 展示渲染统计
		int32 MeshCount = 156;
		int32 TriangleCount = 45678;
		float FrameTime = 16.67f;
		LE_LOG(LogGameRenderingMesh, Verbose, TEXT("Rendering frame: {} meshes, {} triangles, frame time: {:.2f}ms"), MeshCount, TriangleCount, FrameTime);
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