// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/LELogSubsystem.h"
#include "System/LELogTypes.h"
#include "Utils/LogEverythingUtils.h"
#include "Macros/LELogMacros.h"
#include "Category/LECategoryDefine.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "Generated/LogEverythingLogger.h"

// 静态成员变量定义
bool ULELogSubsystem::bStaticInitialized = false;

ULELogSubsystem::ULELogSubsystem()
	: CategoryTree(nullptr)
	, bIsInitialized(false)
	, GlobalLogLevel(ELELogVerbosity::Info)  // 默认级别为Info
{
}

void ULELogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LE_SYSTEM_LOG(TEXT("Initializing LogEverything Subsystem..."));

	// 防止在编辑器中重复初始化
	if (bStaticInitialized)
	{
		LE_SYSTEM_WARNING(TEXT("LogEverything Subsystem already initialized, skipping."));
		return;
	}

	// 创建默认配置
	FLELogSettings DefaultSettings;
	DefaultSettings.GlobalLogLevel = ELELogVerbosity::Info;
	DefaultSettings.bEnableAsyncLogging = true;
	DefaultSettings.BufferSize = 1048576; // 1MB
	DefaultSettings.LogFilePath = TEXT("Logs/LogEverything.log");

	// 初始化 BqLog 桥接
	if (FLEBqLogBridge::Get().Initialize(DefaultSettings, this))
	{
		LE_SYSTEM_LOG(TEXT("LogEverything Bridge initialized successfully"));
		
	}
	else
	{
		LE_SYSTEM_ERROR(TEXT("Failed to initialize LogEverything Bridge"));
	}

	// 初始化分类树
	if (!InitializeCategoryTree())
	{
		LE_SYSTEM_ERROR(TEXT("Failed to initialize category tree!"));
		return;
	}

	// 应用配置中的分类设置
	ApplyDefaultCategoryConfigurations();
	
	// 加载日志设置
	if (!LoadLogSettings())
	{
		LE_SYSTEM_WARNING(TEXT("Failed to load log settings, using defaults."));
	}

	bIsInitialized = true;
	bStaticInitialized = true;

	LE_SYSTEM_LOG(TEXT("LogEverything Subsystem initialized successfully."));
	
}

void ULELogSubsystem::Deinitialize()
{
	LE_SYSTEM_LOG(TEXT("Deinitializing LogEverything Subsystem..."));

	Cleanup();
	bIsInitialized = false;
	bStaticInitialized = false;
	FLEBqLogBridge::Get().Shutdown();
	Super::Deinitialize();
}

ULELogSubsystem* ULELogSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = nullptr;
	if (!IsValid(WorldContextObject))
	{
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		for (int i = 0; i < WorldContexts.Num(); ++i)
		{
			const FWorldContext& WorldContext = WorldContexts[i];
			if (WorldContext.WorldType != EWorldType::Editor && WorldContext.WorldType != EWorldType::EditorPreview)
			{
				World = WorldContext.World();
				break;
			}
		}
	}else
	{
		World = WorldContextObject->GetWorld();
	}
	
	if (!IsValid(World))
	{
		LE_SYSTEM_WARNING(TEXT("Cannot get World from WorldContext"));
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		LE_SYSTEM_WARNING(TEXT("Cannot get GameInstance from World"));
		return nullptr;
	}

	return GameInstance->GetSubsystem<ULELogSubsystem>();
}

bool ULELogSubsystem::SetCategoryLevel(const FName& CategoryPath, ELELogVerbosity Level, bool bPropagate)
{
	if (!IsValid(CategoryTree))
	{
		LE_SYSTEM_WARNING(TEXT("CategoryTree is null, cannot set category level"));
		return false;
	}

	bool bResult = CategoryTree->SetCategoryLevel(CategoryPath.ToString(), Level, bPropagate);
	
	if (bResult)
	{
		if (LogEverything::ConsoleVariable::DebugLogCategory.GetValueOnGameThread())
		{
			LE_SYSTEM_LOG(TEXT("Set category level: %s = %s (propagate: %s)"),
				*CategoryPath.ToString(), *UEnum::GetValueAsString(Level), bPropagate ? TEXT("true") : TEXT("false"));
		}
	}
	else
	{
		LE_SYSTEM_WARNING(TEXT("Failed to set category level for: %s"), *CategoryPath.ToString());
	}

	return bResult;
}

ELELogVerbosity ULELogSubsystem::GetEffectiveLevel(const FName& CategoryPath) const
{
	if (!IsValid(CategoryTree))
	{
		return ELELogVerbosity::Info; // 默认级别
	}

	return CategoryTree->GetEffectiveLevel(CategoryPath.ToString());
}

bool ULELogSubsystem::ShouldLogCategory(const FName& CategoryName, ELELogVerbosity Level) const
{

	bool bShouldLog = false;
	if (!IsValid(CategoryTree))
	{
		// 默认规则：Info 及以上级别显示
		bShouldLog = static_cast<uint8>(Level) >= static_cast<uint8>(ELELogVerbosity::Info);
	}
	else
	{
		bShouldLog = CategoryTree->ShouldLogCategory(CategoryName, Level);
	}

	// 调试日志输出
	if (LogEverything::ConsoleVariable::DebugLogCategory.GetValueOnGameThread())
	{
		LE_LOG_DEBUG(LELogTestLogSystem, TEXT("[ShouldLogCategory] ShouldLogCategory: {}, Level={}, Result={}"),
			CategoryName.ToString(), UEnum::GetValueAsString(Level), bShouldLog ? TEXT("true") : TEXT("false"));
	}

	return bShouldLog;
}

bool ULELogSubsystem::SetCategoryEnabled(const FName& CategoryPath, bool bEnabled, bool bPropagate)
{
	if (!IsValid(CategoryTree))
	{
		LE_SYSTEM_WARNING(TEXT("CategoryTree is null, cannot set category enabled state"));
		return false;
	}

	bool bResult = CategoryTree->SetCategoryEnabled(CategoryPath.ToString(), bEnabled, bPropagate);

	if (bResult)
	{
		LE_SYSTEM_LOG(TEXT("Set category enabled: %s = %s (propagate: %s)"),
			*CategoryPath.ToString(), bEnabled ? TEXT("true") : TEXT("false"), bPropagate ? TEXT("true") : TEXT("false"));
	}
	else
	{
		LE_SYSTEM_WARNING(TEXT("Failed to set category enabled state for: %s"), *CategoryPath.ToString());
	}

	return bResult;
}

bool ULELogSubsystem::IsCategoryEnabled(const FName& CategoryPath) const
{
	if (!IsValid(CategoryTree))
	{
		return true; // 默认启用
	}

	return CategoryTree->IsCategoryEnabled(CategoryPath.ToString());
}

TArray<FString> ULELogSubsystem::GetAllCategoryPaths() const
{
	if (!IsValid(CategoryTree))
	{
		return TArray<FString>();
	}

	return CategoryTree->GetAllCategoryPaths();
}

TArray<FString> ULELogSubsystem::GetChildCategories(const FName& CategoryPath) const
{
	if (!IsValid(CategoryTree))
	{
		return TArray<FString>();
	}

	return CategoryTree->GetChildCategories(CategoryPath.ToString());
}

void ULELogSubsystem::ResetToDefault()
{
	if (!IsValid(CategoryTree))
	{
		LE_SYSTEM_WARNING(TEXT("CategoryTree is null, cannot reset to default"));
		return;
	}

	CategoryTree->ResetToDefault();
	LE_SYSTEM_LOG(TEXT("Reset category tree to default state"));
}

void ULELogSubsystem::GetTreeStatistics(int32& OutTotalNodes, int32& OutMaxDepth, int32& OutExplicitNodes) const
{
	if (!IsValid(CategoryTree))
	{
		OutTotalNodes = 0;
		OutMaxDepth = 0;
		OutExplicitNodes = 0;
		return;
	}

	CategoryTree->GetTreeStatistics(OutTotalNodes, OutMaxDepth, OutExplicitNodes);
}

FString ULELogSubsystem::ExportTreeDebugString() const
{
	if (!IsValid(CategoryTree))
	{
		return TEXT("CategoryTree is null");
	}

	return CategoryTree->ExportTreeDebugString();
}

bool ULELogSubsystem::ReinitializeCategoryTree()
{
	LE_SYSTEM_LOG(TEXT("Reinitializing category tree..."));

	// 清理现有树
	if (IsValid(CategoryTree))
	{
		CategoryTree->ConditionalBeginDestroy();
		CategoryTree = nullptr;
	}

	// 重新初始化
	bool bResult = InitializeCategoryTree();
	if (bResult)
	{
		ApplyDefaultCategoryConfigurations();
		LE_SYSTEM_LOG(TEXT("Category tree reinitialized successfully"));
	}
	else
	{
		LE_SYSTEM_ERROR(TEXT("Failed to reinitialize category tree"));
	}

	return bResult;
}

bool ULELogSubsystem::ReloadLogSettings()
{
	LE_SYSTEM_LOG(TEXT("Reloading log settings..."));
	
	bool bResult = LoadLogSettings();
	if (bResult)
	{
		LE_SYSTEM_LOG(TEXT("Log settings reloaded successfully"));
	}
	else
	{
		LE_SYSTEM_WARNING(TEXT("Failed to reload log settings"));
	}

	return bResult;
}

bool ULELogSubsystem::InitializeCategoryTree()
{
	// 创建分类树实例
	CategoryTree = NewObject<ULECategoryTree>(this);
	if (!IsValid(CategoryTree))
	{
		LE_SYSTEM_ERROR(TEXT("Failed to create CategoryTree object"));
		return false;
	}

	// 获取所有预定义分类路径
	TArray<FString> CategoryPaths;
	if (!GetPredefinedCategoryPaths(CategoryPaths))
	{
		LE_SYSTEM_ERROR(TEXT("Failed to get predefined category paths"));
		return false;
	}

	// 初始化树结构
	bool bResult = CategoryTree->InitializeTree(CategoryPaths);
	if (!bResult)
	{
		LE_SYSTEM_ERROR(TEXT("Failed to initialize category tree with predefined paths"));
		CategoryTree = nullptr;
		return false;
	}

	LE_SYSTEM_LOG(TEXT("Category tree initialized with %d categories"), CategoryPaths.Num());
	return true;
}

bool ULELogSubsystem::LoadLogSettings()
{
	// TODO: 这里将来会从 DataAsset 或配置文件加载设置
	// 目前使用默认设置，暂时不创建 ULELogSettings 对象

	// LogSettings = NewObject<ULELogSettings>(this);
	// if (!IsValid(LogSettings))
	// {
	// 	LE_SYSTEM_WARNING(TEXT("Failed to create LogSettings object"));
	// 	return false;
	// }

	// 设置默认配置
	// 这里可以从文件或其他配置源加载

	LE_SYSTEM_LOG(TEXT("Using default log settings (ULELogSettings not implemented yet)"));
	return true;
}

bool ULELogSubsystem::ParseBqLogCategories(TArray<FString>& OutCategoryPaths) const
{
	OutCategoryPaths.Empty();

	// 从 BqLogBridge 获取 LogEverythingLogger 实例
	const FLEBqLogBridge& Bridge = FLEBqLogBridge::Get();
	if (!Bridge.IsInitialized())
	{
		LE_SYSTEM_WARNING(TEXT("BqLogBridge is not initialized, cannot get categories"));
		return false;
	}

	const bq::LogEverythingLogger* CategoryLogInstance = Bridge.GetCategoryLogInstance();
	if (!CategoryLogInstance)
	{
		LE_SYSTEM_WARNING(TEXT("CategoryLogInstance is null, cannot get categories"));
		return false;
	}

	// 使用 BqLog 接口获取分类信息
	uint32_t CategoryCount = static_cast<uint32_t>(CategoryLogInstance->get_categories_count());
	const bq::array<bq::string> CategoryNames = CategoryLogInstance->get_categories_name_array();
	
	// 将 BqLog 分类名称转换为 UE 字符串数组
	for (uint32_t i = 0; i < CategoryCount; ++i)
	{
		FString CategoryName = FLEBqLogBridge::UTF8ToFString(CategoryNames[i].c_str());
		// 跳过空字符串（根分类）
		if (!CategoryName.IsEmpty())
		{
			OutCategoryPaths.Add(CategoryName);
		}
	}

	LE_SYSTEM_LOG(TEXT("Successfully retrieved %d categories from BqLog interface"), OutCategoryPaths.Num());
	return true;
}

bool ULELogSubsystem::GetPredefinedCategoryPaths(TArray<FString>& OutCategoryPaths) const
{
	OutCategoryPaths.Empty();

	// 尝试动态解析BqLog生成文件
	if (ParseBqLogCategories(OutCategoryPaths))
	{
		LE_SYSTEM_LOG(TEXT("Successfully loaded %d category paths from BqLog generated file"), OutCategoryPaths.Num());
		return true;
	}

	// 解析失败，报告错误并返回失败
	LE_SYSTEM_ERROR(TEXT("Failed to parse BqLog generated categories - this may indicate:"));
	LE_SYSTEM_ERROR(TEXT("1. BqLog generated file not found or corrupted"));
	LE_SYSTEM_ERROR(TEXT("2. File format changed in BqLog tool"));
	LE_SYSTEM_ERROR(TEXT("3. File path resolution issue"));
	LE_SYSTEM_ERROR(TEXT("Please regenerate BqLog categories or check the generated file"));

	return false;
}

void ULELogSubsystem::ApplyDefaultCategoryConfigurations()
{
	if (!IsValid(CategoryTree))
	{
		LE_SYSTEM_WARNING(TEXT("CategoryTree is null, skipping configuration"));
		return;
	}

	CategoryTree->SetCategoryLevel(LELogEngine.GetCategoryName().ToString(), ELELogVerbosity::Info, false);
	CategoryTree->SetCategoryLevel(LELogGame.GetCategoryName().ToString(), ELELogVerbosity::Verbose, false);
	CategoryTree->SetCategoryLevel(LELogEditor.GetCategoryName().ToString(), ELELogVerbosity::Info, false);
	CategoryTree->SetCategoryLevel(LELogTest.GetCategoryName().ToString(), ELELogVerbosity::Verbose, false);

	LE_SYSTEM_LOG(TEXT("Applied default category configurations"));
}

void ULELogSubsystem::Cleanup()
{
	if (IsValid(CategoryTree))
	{
		CategoryTree->ConditionalBeginDestroy();
		CategoryTree = nullptr;
	}

	LE_SYSTEM_LOG(TEXT("LogEverything Subsystem cleanup completed"));
}

void ULELogSubsystem::SetGlobalLogLevel(ELELogVerbosity Level)
{
	GlobalLogLevel = Level;
	LE_SYSTEM_LOG(TEXT("Global log level set to: %s"),
		*UEnum::GetValueAsString(Level));
}

ELELogVerbosity ULELogSubsystem::GetGlobalLogLevel() const
{
	return GlobalLogLevel;
}