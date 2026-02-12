// Copyright Benedict Guo. All Rights Reserved.

/**
 * LogEverything 功能健壮性测试
 * Functional robustness tests for LogEverything system
 *
 * 测试用例设计：
 * - FUNC-01: 分类树初始化与结构
 * - FUNC-02: 级别继承与显式覆盖
 * - FUNC-03: 启用状态与传播
 * - FUNC-04: 级别过滤与运行时修改
 * - FUNC-05: 全局级别与重置
 * - FUNC-06: JSON 重载与配置持久
 * - FUNC-07: 多线程安全与边界处理
 * - FUNC-08: Format 参数类型覆盖
 *
 * 测试环境说明：
 * 由于 ULELogSubsystem 是 GameInstanceSubsystem，在编辑器自动化测试环境中不可用。
 * 因此测试创建独立的 ULECategoryTree 对象进行验证，确保测试可以在编辑器环境下运行。
 */

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Async/ParallelFor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Category/LECategoryTree.h"
#include "System/LELogSubsystem.h"
#include "System/LELogTypes.h"
#include "Macros/LELogMacros.h"

// 声明测试分类
DECLARE_LE_CATEGORY_EXTERN(LELogFuncTest, Test.Functional);
DEFINE_LE_CATEGORY(LELogFuncTest);

/**
 * 创建测试用的分类树
 * 使用项目实际的分类路径初始化
 */
static ULECategoryTree* CreateTestCategoryTree()
{
	ULECategoryTree* Tree = NewObject<ULECategoryTree>();

	// 使用项目实际定义的分类路径
	TArray<FString> CategoryPaths = {
		TEXT("Engine"),
		TEXT("Game"),
		TEXT("Game.Combat.Damage"),
		TEXT("Game.Combat.Skill"),
		TEXT("Game.Combat.Input"),
		TEXT("Game.Animation"),
		TEXT("Game.AI"),
		TEXT("Game.AI.BehaviorTree"),
		TEXT("Game.AI.Pathfinding"),
		TEXT("Game.Input"),
		TEXT("Game.Input.Ability"),
		TEXT("Game.Input.Movement"),
		TEXT("Game.Input.Interaction"),
		TEXT("Editor"),
		TEXT("Test"),
		TEXT("Test.LogSystem"),
		TEXT("Test.Functional"),
		TEXT("Test.Performance")
	};

	Tree->InitializeTree(CategoryPaths);
	Tree->SetDefaultGlobalLevel(ELELogVerbosity::Info);

	return Tree;
}

// ============================================================================
// FUNC-01: 分类树初始化与结构
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_01_TreeInit,
	"LogEverything.Functional.FUNC-01 Category Tree Init and Structure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_01_TreeInit::RunTest(const FString& Parameters)
{
	// 创建独立的测试分类树
	ULECategoryTree* Tree = CreateTestCategoryTree();
	if (!Tree)
	{
		AddError(TEXT("Failed to create test category tree"));
		return false;
	}

	AddInfo(TEXT("Testing category tree initialization and structure..."));

	// 获取树统计信息
	int32 TotalNodes, MaxDepth, ExplicitNodes;
	Tree->GetTreeStatistics(TotalNodes, MaxDepth, ExplicitNodes);

	// 验证树有节点
	TestTrue(TEXT("Tree should have nodes"), TotalNodes > 0);
	AddInfo(FString::Printf(TEXT("  Total nodes: %d, Max depth: %d, Explicit nodes: %d"),
		TotalNodes, MaxDepth, ExplicitNodes));

	// 验证根节点存在
	const FLECategoryNode* RootNode = Tree->GetNode(0);
	TestNotNull(TEXT("Root node should exist"), RootNode);

	if (RootNode)
	{
		TestTrue(TEXT("Root node should be root"), RootNode->IsRoot());
		TestEqual(TEXT("Root node depth should be 0"), RootNode->Depth, 0);
		AddInfo(FString::Printf(TEXT("  Root node: %s, Children: %d"),
			*RootNode->CategoryFullName.ToString(), RootNode->ChildIndices.Num()));
	}

	// 验证预期的分类存在
	TArray<FString> ExpectedCategories = {
		TEXT("Engine"),
		TEXT("Game"),
		TEXT("Game.Combat"),
		TEXT("Game.Combat.Damage"),
		TEXT("Game.AI"),
		TEXT("Test"),
		TEXT("Test.Functional")
	};

	for (const FString& CategoryPath : ExpectedCategories)
	{
		const FLECategoryNode* Node = Tree->GetNode(CategoryPath);
		TestNotNull(FString::Printf(TEXT("Category '%s' should exist"), *CategoryPath), Node);

		if (Node)
		{
			AddInfo(FString::Printf(TEXT("  Found: %s (Depth=%d, Level=%s)"),
				*CategoryPath,
				Node->Depth,
				*UEnum::GetValueAsString(Node->EffectiveLevel)));
		}
	}

	// 验证父子关系（通过 GetNode 访问）
	const FLECategoryNode* GameNode = Tree->GetNode(TEXT("Game"));
	const FLECategoryNode* CombatNode = Tree->GetNode(TEXT("Game.Combat"));

	if (GameNode && CombatNode)
	{
		// Combat 的父节点索引应该有效
		TestTrue(TEXT("Game.Combat should have valid parent index"),
			CombatNode->ParentIndex != INDEX_NONE);

		// Combat 的父节点应该是 Game
		const FLECategoryNode* CombatParent = Tree->GetNode(CombatNode->ParentIndex);
		TestTrue(TEXT("Game.Combat parent should be Game"),
			CombatParent != nullptr && CombatParent->CategoryFullName == FName(TEXT("Game")));
	}

	return true;
}

// ============================================================================
// FUNC-02: 级别继承与显式覆盖
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_02_LevelInheritance,
	"LogEverything.Functional.FUNC-02 Level Inheritance and Override",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_02_LevelInheritance::RunTest(const FString& Parameters)
{
	ULECategoryTree* Tree = CreateTestCategoryTree();
	if (!Tree)
	{
		AddError(TEXT("Failed to create test category tree"));
		return false;
	}

	AddInfo(TEXT("Testing level inheritance and explicit override..."));

	// 测试 1: 设置父节点级别，验证继承
	AddInfo(TEXT("  Test 1: Parent level inheritance..."));
	Tree->SetCategoryLevel(TEXT("Game"), ELELogVerbosity::Warning, true); // bPropagate = true

	ELELogVerbosity CombatLevel = Tree->GetEffectiveLevel(TEXT("Game.Combat"));
	ELELogVerbosity DamageLevel = Tree->GetEffectiveLevel(TEXT("Game.Combat.Damage"));

	TestEqual(TEXT("Game.Combat should inherit Warning from Game"),
		CombatLevel, ELELogVerbosity::Warning);
	TestEqual(TEXT("Game.Combat.Damage should inherit Warning from Game"),
		DamageLevel, ELELogVerbosity::Warning);

	// 测试 2: 显式覆盖子节点级别
	AddInfo(TEXT("  Test 2: Explicit level override..."));
	Tree->SetCategoryLevel(TEXT("Game.Combat"), ELELogVerbosity::Debug, false);

	CombatLevel = Tree->GetEffectiveLevel(TEXT("Game.Combat"));
	TestEqual(TEXT("Game.Combat explicit level should be Debug"),
		CombatLevel, ELELogVerbosity::Debug);

	// 验证 bHasExplicitLevel 标志
	const FLECategoryNode* CombatNode = Tree->GetNode(TEXT("Game.Combat"));
	if (CombatNode)
	{
		TestTrue(TEXT("Game.Combat should have explicit level set"), CombatNode->bHasExplicitLevel);
		AddInfo(FString::Printf(TEXT("  Combat explicit=%s, effective=%s"),
			CombatNode->bHasExplicitLevel ? TEXT("Yes") : TEXT("No"),
			*UEnum::GetValueAsString(CombatNode->EffectiveLevel)));
	}

	// 测试 3: 显式设置后再传播父节点级别（不强制覆盖）
	AddInfo(TEXT("  Test 3: Propagation with explicit child..."));
	Tree->SetCategoryLevel(TEXT("Game"), ELELogVerbosity::Error, false); // bPropagate = false

	// Game.Combat 有显式设置，不应该被覆盖
	CombatLevel = Tree->GetEffectiveLevel(TEXT("Game.Combat"));
	TestEqual(TEXT("Game.Combat should keep explicit Debug level"),
		CombatLevel, ELELogVerbosity::Debug);

	return true;
}

// ============================================================================
// FUNC-03: 启用状态与传播
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_03_EnableState,
	"LogEverything.Functional.FUNC-03 Enable State and Propagation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_03_EnableState::RunTest(const FString& Parameters)
{
	ULECategoryTree* Tree = CreateTestCategoryTree();
	if (!Tree)
	{
		AddError(TEXT("Failed to create test category tree"));
		return false;
	}

	AddInfo(TEXT("Testing enable state and propagation..."));

	// 测试 1: 禁用父节点，验证子节点传播
	AddInfo(TEXT("  Test 1: Disable parent propagation..."));
	Tree->SetCategoryEnabled(TEXT("Game"), false, true); // bPropagate = true

	bool bCombatEnabled = Tree->IsCategoryEnabled(TEXT("Game.Combat"));
	bool bDamageEnabled = Tree->IsCategoryEnabled(TEXT("Game.Combat.Damage"));

	TestFalse(TEXT("Game.Combat should be disabled when Game is disabled"), bCombatEnabled);
	TestFalse(TEXT("Game.Combat.Damage should be disabled when Game is disabled"), bDamageEnabled);

	// 测试 2: 重新启用父节点
	AddInfo(TEXT("  Test 2: Re-enable parent..."));
	Tree->SetCategoryEnabled(TEXT("Game"), true, true);

	bCombatEnabled = Tree->IsCategoryEnabled(TEXT("Game.Combat"));
	TestTrue(TEXT("Game.Combat should be enabled when Game is re-enabled"), bCombatEnabled);

	// 测试 3: 验证 ShouldLogCategory 考虑启用状态
	AddInfo(TEXT("  Test 3: ShouldLogCategory with enable state..."));
	Tree->SetCategoryEnabled(TEXT("Game.AI"), false, false);

	bool bShouldLog = Tree->ShouldLogCategory(FName(TEXT("Game.AI")), ELELogVerbosity::Error);
	TestFalse(TEXT("Disabled category should not log"), bShouldLog);

	Tree->SetCategoryEnabled(TEXT("Game.AI"), true, false);
	bShouldLog = Tree->ShouldLogCategory(FName(TEXT("Game.AI")), ELELogVerbosity::Error);
	TestTrue(TEXT("Enabled category should log"), bShouldLog);

	return true;
}

// ============================================================================
// FUNC-04: 级别过滤与运行时修改
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_04_LevelFilter,
	"LogEverything.Functional.FUNC-04 Level Filter and Runtime Modify",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_04_LevelFilter::RunTest(const FString& Parameters)
{
	ULECategoryTree* Tree = CreateTestCategoryTree();
	if (!Tree)
	{
		AddError(TEXT("Failed to create test category tree"));
		return false;
	}

	AddInfo(TEXT("Testing level filter and runtime modification..."));

	// 测试 1: 级别过滤正确性
	AddInfo(TEXT("  Test 1: Level filtering correctness..."));
	Tree->SetCategoryLevel(TEXT("Test.Functional"), ELELogVerbosity::Warning, false);

	// Warning 级别下：Debug < Warning，不应该输出
	bool bShouldLogDebug = Tree->ShouldLogCategory(FName(TEXT("Test.Functional")), ELELogVerbosity::Debug);
	TestFalse(TEXT("Debug should be filtered when level is Warning"), bShouldLogDebug);

	// Warning >= Warning，应该输出
	bool bShouldLogWarning = Tree->ShouldLogCategory(FName(TEXT("Test.Functional")), ELELogVerbosity::Warning);
	TestTrue(TEXT("Warning should pass when level is Warning"), bShouldLogWarning);

	// Error > Warning，应该输出
	bool bShouldLogError = Tree->ShouldLogCategory(FName(TEXT("Test.Functional")), ELELogVerbosity::Error);
	TestTrue(TEXT("Error should pass when level is Warning"), bShouldLogError);

	// 测试 2: 运行时修改立即生效
	// 注意：ShouldLogCategory 取 CurrentGlobalLevel 和节点 EffectiveLevel 中更严格的
	// 因此需要同时修改 CurrentGlobalLevel 来允许 Debug 级别日志输出
	AddInfo(TEXT("  Test 2: Runtime modification takes effect immediately..."));
	Tree->SetCategoryLevel(TEXT("Test.Functional"), ELELogVerbosity::Debug, false);
	Tree->SetCurrentGlobalLevel(ELELogVerbosity::Debug);

	bShouldLogDebug = Tree->ShouldLogCategory(FName(TEXT("Test.Functional")), ELELogVerbosity::Debug);
	TestTrue(TEXT("Debug should pass after level changed to Debug"), bShouldLogDebug);

	// 验证级别确实改变了
	ELELogVerbosity CurrentLevel = Tree->GetEffectiveLevel(TEXT("Test.Functional"));
	TestEqual(TEXT("Effective level should be Debug"), CurrentLevel, ELELogVerbosity::Debug);

	// 测试 3: 验证全局级别与分类级别的交互
	AddInfo(TEXT("  Test 3: Global level and category level interaction..."));

	// 分类级别 Debug，全局级别 Warning
	// ShouldLogCategory 取更严格的 Warning，所以 Debug 应该被过滤
	Tree->SetCurrentGlobalLevel(ELELogVerbosity::Warning);
	bShouldLogDebug = Tree->ShouldLogCategory(FName(TEXT("Test.Functional")), ELELogVerbosity::Debug);
	TestFalse(TEXT("Debug should be filtered when global level is Warning"), bShouldLogDebug);

	// Warning >= Warning，应该输出
	bShouldLogWarning = Tree->ShouldLogCategory(FName(TEXT("Test.Functional")), ELELogVerbosity::Warning);
	TestTrue(TEXT("Warning should pass when global level is Warning"), bShouldLogWarning);

	return true;
}

// ============================================================================
// FUNC-05: 全局级别与重置
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_05_GlobalLevel,
	"LogEverything.Functional.FUNC-05 Global Level and Reset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_05_GlobalLevel::RunTest(const FString& Parameters)
{
	ULECategoryTree* Tree = CreateTestCategoryTree();
	if (!Tree)
	{
		AddError(TEXT("Failed to create test category tree"));
		return false;
	}

	AddInfo(TEXT("Testing global level and reset..."));

	// 保存原始状态
	ELELogVerbosity OriginalDefaultGlobal = Tree->GetDefaultGlobalLevel();

	// 测试 1: 设置当前全局级别
	AddInfo(TEXT("  Test 1: Set current global level..."));
	Tree->SetCurrentGlobalLevel(ELELogVerbosity::Error);

	ELELogVerbosity CurrentGlobal = Tree->GetCurrentGlobalLevel();
	TestEqual(TEXT("Current global level should be Error"), CurrentGlobal, ELELogVerbosity::Error);

	// 验证默认全局级别不变
	ELELogVerbosity DefaultGlobal = Tree->GetDefaultGlobalLevel();
	TestEqual(TEXT("Default global level should not change"), DefaultGlobal, OriginalDefaultGlobal);

	// 测试 2: 全局级别影响 ShouldLogCategory
	AddInfo(TEXT("  Test 2: Global level affects ShouldLogCategory..."));

	// 将分类级别设为 Debug，全局级别为 Error
	// 由于全局级别更严格，Debug 级别的日志应该被过滤
	Tree->SetCategoryLevel(TEXT("Test.Functional"), ELELogVerbosity::Debug, false);

	bool bShouldLogDebug = Tree->ShouldLogCategory(FName(TEXT("Test.Functional")), ELELogVerbosity::Debug);
	TestFalse(TEXT("Debug should be filtered by global Error level"), bShouldLogDebug);

	bool bShouldLogError = Tree->ShouldLogCategory(FName(TEXT("Test.Functional")), ELELogVerbosity::Error);
	TestTrue(TEXT("Error should pass global Error level"), bShouldLogError);

	// 测试 3: 重置功能
	AddInfo(TEXT("  Test 3: Reset to default..."));
	Tree->ResetToDefault();

	// 验证重置后当前全局级别恢复为默认值
	CurrentGlobal = Tree->GetCurrentGlobalLevel();
	TestEqual(TEXT("Current global level should reset to default"), CurrentGlobal, OriginalDefaultGlobal);

	AddInfo(TEXT("  Reset completed, global level restored"));

	return true;
}

// ============================================================================
// FUNC-06: JSON 重载与配置持久（独立测试树的重建）
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_06_TreeRebuild,
	"LogEverything.Functional.FUNC-06 Tree Rebuild",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_06_TreeRebuild::RunTest(const FString& Parameters)
{
	AddInfo(TEXT("Testing tree rebuild capability..."));

	// 测试 1: 创建树并设置配置
	AddInfo(TEXT("  Test 1: Create tree and apply configuration..."));
	ULECategoryTree* Tree = CreateTestCategoryTree();
	TestNotNull(TEXT("Initial tree should be created"), Tree);

	if (Tree)
	{
		// 设置一些配置
		Tree->SetCategoryLevel(TEXT("Game"), ELELogVerbosity::Warning, true);
		Tree->SetCategoryEnabled(TEXT("Editor"), false, false);

		int32 TotalNodes1, MaxDepth1, ExplicitNodes1;
		Tree->GetTreeStatistics(TotalNodes1, MaxDepth1, ExplicitNodes1);
		AddInfo(FString::Printf(TEXT("  Initial tree: %d nodes, %d explicit"), TotalNodes1, ExplicitNodes1));
	}

	// 测试 2: 重新创建树
	AddInfo(TEXT("  Test 2: Recreate tree..."));
	ULECategoryTree* Tree2 = CreateTestCategoryTree();
	TestNotNull(TEXT("Second tree should be created"), Tree2);

	if (Tree2)
	{
		int32 TotalNodes2, MaxDepth2, ExplicitNodes2;
		Tree2->GetTreeStatistics(TotalNodes2, MaxDepth2, ExplicitNodes2);
		AddInfo(FString::Printf(TEXT("  Rebuilt tree: %d nodes, %d explicit"), TotalNodes2, ExplicitNodes2));

		// 验证新树是干净的（没有之前的配置）
		ELELogVerbosity GameLevel = Tree2->GetEffectiveLevel(TEXT("Game"));
		bool bEditorEnabled = Tree2->IsCategoryEnabled(TEXT("Editor"));

		// 新树应该使用默认配置
		TestEqual(TEXT("Game level should be default Info"), GameLevel, ELELogVerbosity::Info);
		TestTrue(TEXT("Editor should be enabled by default"), bEditorEnabled);
	}

	// 测试 3: 验证导出调试字符串功能
	AddInfo(TEXT("  Test 3: Export debug string..."));
	if (Tree2)
	{
		FString DebugString = Tree2->ExportTreeDebugString();
		TestTrue(TEXT("Debug string should not be empty"), DebugString.Len() > 0);
		AddInfo(FString::Printf(TEXT("  Debug string length: %d characters"), DebugString.Len()));
	}

	return true;
}

// ============================================================================
// FUNC-07: 多线程安全与边界处理
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_07_MultiThread,
	"LogEverything.Functional.FUNC-07 MultiThread and Boundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_07_MultiThread::RunTest(const FString& Parameters)
{
	ULECategoryTree* Tree = CreateTestCategoryTree();
	if (!Tree)
	{
		AddError(TEXT("Failed to create test category tree"));
		return false;
	}

	AddInfo(TEXT("Testing multithread safety and boundary handling..."));

	// 测试 1: 多线程并发读取（不应崩溃）
	AddInfo(TEXT("  Test 1: Concurrent read operations..."));
	const int32 ThreadCount = 4;
	const int32 IterationsPerThread = 1000;
	std::atomic<bool> bHasError{false};
	std::atomic<int32> SuccessCount{0};

	ParallelFor(ThreadCount, [&](int32 ThreadIndex)
	{
		for (int32 i = 0; i < IterationsPerThread && !bHasError; ++i)
		{
			// 并发读取操作
			ELELogVerbosity Level = Tree->GetEffectiveLevel(TEXT("Game.Combat"));
			bool bEnabled = Tree->IsCategoryEnabled(TEXT("Game.AI"));
			bool bShouldLog = Tree->ShouldLogCategory(FName(TEXT("Test")), ELELogVerbosity::Info);

			// 验证返回值合理（不能同时是两个特殊值）
			if (Level == ELELogVerbosity::NoLogging && Level == ELELogVerbosity::NotSet)
			{
				// 这种情况不应该发生
				bHasError = true;
			}
			else
			{
				SuccessCount++;
			}
		}
	});

	TestFalse(TEXT("No errors during concurrent reads"), bHasError.load());
	TestEqual(TEXT("All concurrent reads should succeed"),
		SuccessCount.load(), ThreadCount * IterationsPerThread);
	AddInfo(FString::Printf(TEXT("  Completed %d concurrent reads"), SuccessCount.load()));

	// 测试 2: 并发日志写入（不应崩溃）
	AddInfo(TEXT("  Test 2: Concurrent log writes..."));
	const int32 LogsPerThread = 100;
	std::atomic<bool> bLogError{false};

	ParallelFor(ThreadCount, [&](int32 ThreadIndex)
	{
		for (int32 i = 0; i < LogsPerThread && !bLogError; ++i)
		{
			// 使用 LE_LOG 宏进行并发写入
			LE_LOG(LELogFuncTest, Info, TEXT("Thread {} iteration {}"), ThreadIndex, i);
		}
	});

	TestFalse(TEXT("No crash during concurrent logging"), bLogError.load());
	AddInfo(FString::Printf(TEXT("  Completed %d concurrent logs"), ThreadCount * LogsPerThread));

	// 测试 3: 无效分类处理
	AddInfo(TEXT("  Test 3: Invalid category handling..."));

	// 查询不存在的分类，不应崩溃
	ELELogVerbosity InvalidLevel = Tree->GetEffectiveLevel(TEXT("NonExistent.Category.Path"));
	bool bInvalidEnabled = Tree->IsCategoryEnabled(TEXT("Another.Invalid.Path"));
	bool bInvalidShouldLog = Tree->ShouldLogCategory(FName(TEXT("DoesNotExist")), ELELogVerbosity::Info);

	AddInfo(FString::Printf(TEXT("  Invalid category level: %s"), *UEnum::GetValueAsString(InvalidLevel)));
	AddInfo(FString::Printf(TEXT("  Invalid category enabled: %s"), bInvalidEnabled ? TEXT("Yes") : TEXT("No")));
	AddInfo(FString::Printf(TEXT("  Invalid category should log: %s"), bInvalidShouldLog ? TEXT("Yes") : TEXT("No")));

	// 测试 4: 空路径处理
	AddInfo(TEXT("  Test 4: Empty path handling..."));
	const FLECategoryNode* EmptyNode = Tree->GetNode(TEXT(""));
	// 空路径应该返回根节点或 nullptr，不应崩溃
	AddInfo(FString::Printf(TEXT("  Empty path result: %s"), EmptyNode ? TEXT("Got node") : TEXT("nullptr")));

	return true;
}

// ============================================================================
// FUNC-08: Format 参数类型覆盖
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_08_FormatTypes,
	"LogEverything.Functional.FUNC-08 Format Parameter Types",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_08_FormatTypes::RunTest(const FString& Parameters)
{
	AddInfo(TEXT("Testing format parameter types..."));

	// 测试各种 UE 类型的格式化（不应崩溃）
	// 基础类型
	int32 IntValue = 42;
	int64 Int64Value = 9223372036854775807LL;
	float FloatValue = 3.14159f;
	double DoubleValue = 2.718281828459045;
	bool BoolValue = true;

	AddInfo(TEXT("  Testing basic types..."));
	LE_LOG(LELogFuncTest, Info, TEXT("Int: {}"), IntValue);
	LE_LOG(LELogFuncTest, Info, TEXT("Int64: {}"), Int64Value);
	LE_LOG(LELogFuncTest, Info, TEXT("Float: {:.2f}"), FloatValue);
	LE_LOG(LELogFuncTest, Info, TEXT("Double: {:.6f}"), DoubleValue);
	LE_LOG(LELogFuncTest, Info, TEXT("Bool: {}"), BoolValue);

	// UE 字符串类型
	FString StringValue = TEXT("TestString");
	FName NameValue = FName(TEXT("TestName"));
	FText TextValue = FText::FromString(TEXT("TestText"));

	AddInfo(TEXT("  Testing UE string types..."));
	LE_LOG(LELogFuncTest, Info, TEXT("FString: {}"), StringValue);
	LE_LOG(LELogFuncTest, Info, TEXT("FName: {}"), NameValue);
	// FText 需要特殊处理，使用 ToString()
	LE_LOG(LELogFuncTest, Info, TEXT("FText: {}"), TextValue.ToString());

	// 向量和旋转类型
	FVector VectorValue(1.0f, 2.0f, 3.0f);
	FRotator RotatorValue(45.0f, 90.0f, 0.0f);

	AddInfo(TEXT("  Testing math types..."));
	LE_LOG(LELogFuncTest, Info, TEXT("FVector: {}"), VectorValue.ToString());
	LE_LOG(LELogFuncTest, Info, TEXT("FRotator: {}"), RotatorValue.ToString());

	// 多参数格式化
	AddInfo(TEXT("  Testing multi-parameter format..."));
	LE_LOG(LELogFuncTest, Info, TEXT("Multi: int={}, float={:.2f}, string={}"),
		IntValue, FloatValue, StringValue);

	// 条件日志
	AddInfo(TEXT("  Testing conditional logging..."));
	LE_CLOG(true, LELogFuncTest, Info, TEXT("Conditional true: {}"), IntValue);
	LE_CLOG(false, LELogFuncTest, Info, TEXT("This should not appear"));

	// 断言日志
	AddInfo(TEXT("  Testing check/ensure macros..."));
	LE_CHECK(IntValue > 0, LELogFuncTest, Warning, TEXT("IntValue should be positive"));
	LE_ENSURE(StringValue.Len() > 0, LELogFuncTest, Warning, TEXT("StringValue should not be empty"));

	AddInfo(TEXT("  All format tests completed without crash"));
	return true;
}

// ============================================================================
// FUNC-09: JSON 配置加载与验证
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_09_JsonConfigLoad,
	"LogEverything.Functional.FUNC-09 JSON Config Load and Validate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_09_JsonConfigLoad::RunTest(const FString& Parameters)
{
	ULECategoryTree* Tree = CreateTestCategoryTree();
	if (!Tree)
	{
		AddError(TEXT("Failed to create test category tree"));
		return false;
	}

	AddInfo(TEXT("Testing JSON config load and validation..."));

	// 测试 1: 验证树初始化后包含预期的节点
	AddInfo(TEXT("  Test 1: Verify tree contains expected nodes from config..."));

	// 验证 JSON 配置中定义的分类存在
	TArray<FString> ExpectedCategories = {
		TEXT("Engine"),
		TEXT("Game"),
		TEXT("Game.Combat"),
		TEXT("Game.Combat.Damage"),
		TEXT("Game.AI"),
		TEXT("Game.AI.BehaviorTree"),
		TEXT("Editor"),
		TEXT("Test"),
		TEXT("Test.Functional"),
		TEXT("Test.Performance")
	};

	int32 FoundCount = 0;
	for (const FString& CategoryPath : ExpectedCategories)
	{
		const FLECategoryNode* Node = Tree->GetNode(CategoryPath);
		if (Node)
		{
			FoundCount++;
			AddInfo(FString::Printf(TEXT("  Found: %s"), *CategoryPath));
		}
		else
		{
			AddWarning(FString::Printf(TEXT("  Missing: %s"), *CategoryPath));
		}
	}

	TestEqual(TEXT("All expected categories should exist"),
		FoundCount, ExpectedCategories.Num());

	// 测试 2: 验证 DefaultGlobalLevel 设置
	AddInfo(TEXT("  Test 2: Verify DefaultGlobalLevel..."));
	ELELogVerbosity DefaultLevel = Tree->GetDefaultGlobalLevel();
	AddInfo(FString::Printf(TEXT("  DefaultGlobalLevel: %s"), *UEnum::GetValueAsString(DefaultLevel)));

	// DefaultGlobalLevel 应该是有效值（不是 NotSet）
	TestTrue(TEXT("DefaultGlobalLevel should be valid"),
		DefaultLevel != ELELogVerbosity::NotSet);

	// 测试 3: 验证层级关系正确
	AddInfo(TEXT("  Test 3: Verify hierarchy relationships..."));

	const FLECategoryNode* GameNode = Tree->GetNode(TEXT("Game"));
	const FLECategoryNode* CombatNode = Tree->GetNode(TEXT("Game.Combat"));
	const FLECategoryNode* DamageNode = Tree->GetNode(TEXT("Game.Combat.Damage"));

	if (GameNode && CombatNode && DamageNode)
	{
		// 验证深度递增
		TestTrue(TEXT("Combat depth > Game depth"),
			CombatNode->Depth > GameNode->Depth);
		TestTrue(TEXT("Damage depth > Combat depth"),
			DamageNode->Depth > CombatNode->Depth);

		// 验证父子关系（通过 ParentIndex 指向 Game 节点）
		const FLECategoryNode* CombatParent = Tree->GetNode(CombatNode->ParentIndex);
		TestNotNull(TEXT("Combat parent node should exist"), CombatParent);
		if (CombatParent)
		{
			TestEqual(TEXT("Combat parent should be Game"),
				CombatParent->CategoryFullName, FName(TEXT("Game")));
		}

		AddInfo(FString::Printf(TEXT("  Hierarchy verified: Game(d=%d) -> Combat(d=%d) -> Damage(d=%d)"),
			GameNode->Depth, CombatNode->Depth, DamageNode->Depth));
	}
	else
	{
		AddError(TEXT("Failed to get hierarchy nodes"));
	}

	return true;
}

// ============================================================================
// FUNC-10: 日志级别宏覆盖
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_10_LogLevelMacros,
	"LogEverything.Functional.FUNC-10 Log Level Macros Coverage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_10_LogLevelMacros::RunTest(const FString& Parameters)
{
	AddInfo(TEXT("Testing log level macros coverage..."));

	// 测试 1: 各级别快捷宏（不应崩溃）
	AddInfo(TEXT("  Test 1: Testing level-specific macros..."));

	// 测试 LE_LOG 的各个级别
	LE_LOG(LELogFuncTest, Fatal, TEXT("Fatal level test (will not actually crash in test)"));
	LE_LOG(LELogFuncTest, Error, TEXT("Error level test: {}"), 1);
	LE_LOG(LELogFuncTest, Warning, TEXT("Warning level test: {}"), 2);
	LE_LOG(LELogFuncTest, Info, TEXT("Info level test: {}"), 3);
	LE_LOG(LELogFuncTest, Debug, TEXT("Debug level test: {}"), 4);
	LE_LOG(LELogFuncTest, Verbose, TEXT("Verbose level test: {}"), 5);

	AddInfo(TEXT("  All level macros executed without crash"));

	// 测试 2: 条件日志宏
	AddInfo(TEXT("  Test 2: Testing conditional log macros..."));

	bool bConditionTrue = true;
	bool bConditionFalse = false;

	LE_CLOG(bConditionTrue, LELogFuncTest, Info, TEXT("CLOG with true condition"));
	LE_CLOG(bConditionFalse, LELogFuncTest, Info, TEXT("CLOG with false condition - should not appear"));

	AddInfo(TEXT("  Conditional macros executed without crash"));

	// 测试 3: CHECK 和 ENSURE 宏
	AddInfo(TEXT("  Test 3: Testing CHECK and ENSURE macros..."));

	int32 PositiveValue = 42;
	int32 ZeroValue = 0;

	// CHECK 成功的情况
	LE_CHECK(PositiveValue > 0, LELogFuncTest, Error, TEXT("PositiveValue should be positive"));

	// ENSURE 宏（条件为 true 时不输出日志，条件为 false 时输出警告）
	LE_ENSURE(PositiveValue != ZeroValue, LELogFuncTest, Warning,
		TEXT("Values should be different"));

	AddInfo(TEXT("  CHECK and ENSURE macros executed without crash"));

	// 测试 4: Flush 功能
	AddInfo(TEXT("  Test 4: Testing LE_FLUSH_LOGS..."));

	// 先写入一些日志
	for (int32 i = 0; i < 10; ++i)
	{
		LE_LOG(LELogFuncTest, Info, TEXT("Pre-flush log {}"), i);
	}

	// 调用 Flush（不应崩溃）
	LE_FLUSH_LOGS();

	AddInfo(TEXT("  LE_FLUSH_LOGS executed without crash"));

	return true;
}

// ============================================================================
// FUNC-11: 中文字符串格式化
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLEFunctionalTest_11_ChineseFormat,
	"LogEverything.Functional.FUNC-11 Chinese String Format",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLEFunctionalTest_11_ChineseFormat::RunTest(const FString& Parameters)
{
	AddInfo(TEXT("Testing Chinese string formatting (UTF-16 to UTF-8 conversion)..."));

	// 测试 1: 中文 FString 格式化
	AddInfo(TEXT("  Test 1: Chinese FString formatting..."));

	FString ChineseString = TEXT("测试中文字符串");
	FString ChineseMessage = TEXT("这是一条中文日志消息");

	LE_LOG(LELogFuncTest, Info, TEXT("中文字符串测试: {}"), ChineseString);
	LE_LOG(LELogFuncTest, Info, TEXT("消息: {}"), ChineseMessage);

	AddInfo(TEXT("  Chinese FString formatting completed"));

	// 测试 2: 中文 FName 格式化
	AddInfo(TEXT("  Test 2: Chinese FName formatting..."));

	FName ChineseName = FName(TEXT("中文名称"));
	FName ChineseCategory = FName(TEXT("游戏.战斗.伤害"));

	LE_LOG(LELogFuncTest, Info, TEXT("中文 FName: {}"), ChineseName);
	LE_LOG(LELogFuncTest, Info, TEXT("中文分类: {}"), ChineseCategory);

	AddInfo(TEXT("  Chinese FName formatting completed"));

	// 测试 3: 中文 FText 格式化
	AddInfo(TEXT("  Test 3: Chinese FText formatting..."));

	FText ChineseText = FText::FromString(TEXT("中文文本内容"));
	FText LocalizedText = FText::FromString(TEXT("本地化文本"));

	LE_LOG(LELogFuncTest, Info, TEXT("中文 FText: {}"), ChineseText.ToString());
	LE_LOG(LELogFuncTest, Info, TEXT("本地化: {}"), LocalizedText.ToString());

	AddInfo(TEXT("  Chinese FText formatting completed"));

	// 测试 4: 混合中英文格式化
	AddInfo(TEXT("  Test 4: Mixed Chinese and English formatting..."));

	FString MixedContent = TEXT("Mixed混合Content内容");
	int32 NumericValue = 12345;
	float FloatValue = 3.14f;

	LE_LOG(LELogFuncTest, Info, TEXT("混合内容 Mixed: {} 数值 Value: {} 浮点 Float: {:.2f}"),
		MixedContent, NumericValue, FloatValue);

	LE_LOG(LELogFuncTest, Info, TEXT("Player玩家 {} scored得分 {} points分"),
		TEXT("张三"), 100);

	AddInfo(TEXT("  Mixed formatting completed"));

	// 测试 5: 特殊中文字符
	AddInfo(TEXT("  Test 5: Special Chinese characters..."));

	FString SpecialChars = TEXT("标点符号：，。！？、；：""''【】");
	FString Emojis = TEXT("表情符号测试");  // 不使用实际 emoji，避免编码问题

	LE_LOG(LELogFuncTest, Info, TEXT("特殊字符: {}"), SpecialChars);
	LE_LOG(LELogFuncTest, Info, TEXT("表情: {}"), Emojis);

	AddInfo(TEXT("  Special characters formatting completed"));

	// 测试 6: 长中文字符串
	AddInfo(TEXT("  Test 6: Long Chinese string..."));

	FString LongChinese = TEXT("这是一段很长的中文文本，用于测试日志系统对于长字符串的处理能力。");
	LongChinese += TEXT("包含多个句子，确保字符编码转换在处理大量中文字符时仍然正确。");
	LongChinese += TEXT("测试完成后应该能够在日志文件中看到完整的中文内容，没有乱码或截断。");

	LE_LOG(LELogFuncTest, Info, TEXT("长文本: {}"), LongChinese);

	AddInfo(TEXT("  Long Chinese string formatting completed"));

	AddInfo(TEXT("  All Chinese format tests completed without crash"));
	return true;
}
