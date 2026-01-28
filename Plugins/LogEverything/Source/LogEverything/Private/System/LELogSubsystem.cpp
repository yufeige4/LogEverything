// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/LELogSubsystem.h"
#include "System/LELogTypes.h"
#include "Utils/LogEverythingUtils.h"
#include "Macros/LELogMacros.h"
#include "Category/LECategoryDefine.h"
#include "Config/LELogEverythingSettings.h"
#include "Config/LELogConfigAsset.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "Generated/LogEverythingLogger.h"

// ============================================================================
// 环境检测辅助函数
// Environment Detection Helper Functions
// ============================================================================

/**
 * 获取当前构建环境
 * 根据 UE 预处理器宏检测当前构建环境类型
 *
 * Get Current Build Environment
 * Detect current build environment type based on UE preprocessor macros
 *
 * @return 当前构建环境枚举 / Current build environment enum
 */
static ELEBuildEnvironment GetCurrentBuildEnvironment()
{
	// UE 的构建环境宏是互斥的,同一时间只有一个为 1
	// UE build environment macros are mutually exclusive, only one is 1 at a time

#if UE_BUILD_SHIPPING
	// Shipping 构建：发布版本，最高性能，最少日志
	// Shipping build: Release version, highest performance, minimal logging
	return ELEBuildEnvironment::Shipping;

#elif UE_BUILD_TEST
	// Test 构建：测试版本，用于 QA 测试
	// Test build: Test version, used for QA testing
	return ELEBuildEnvironment::Test;

#elif UE_BUILD_DEBUG
	// Debug 构建：调试版本，包含完整调试信息
	// Debug build: Debug version, includes full debug information
	return ELEBuildEnvironment::Debug;

#elif UE_BUILD_DEVELOPMENT
	// Development 构建：开发版本，平衡性能和调试能力
	// Development build: Development version, balances performance and debuggability
	return ELEBuildEnvironment::Development;

#else
	// 默认情况：如果没有任何宏定义，回退到 Development
	// Default case: If no macro is defined, fallback to Development
	return ELEBuildEnvironment::Development;
#endif
}

/**
 * 获取当前网络环境（服务器类型）
 * 根据运行时检测当前是 Client 还是 DedicatedServer
 *
 * Get Current Network Environment (Server Type)
 * Detect at runtime whether current instance is Client or DedicatedServer
 *
 * @return 当前网络环境枚举 / Current network environment enum
 */
static ELENetEnvironment GetCurrentNetEnvironment()
{
	// 使用 UE 运行时 API 检测是否为专用服务器
	// Use UE runtime API to detect if this is a dedicated server
	if (IsRunningDedicatedServer())
	{
		// 专用服务器：无渲染、无 UI、仅处理游戏逻辑和网络
		// Dedicated server: No rendering, no UI, only game logic and networking
		return ELENetEnvironment::DedicatedServer;
	}
	else
	{
		// 客户端：包括独立游戏、监听服务器、纯客户端
		// Client: Includes standalone game, listen server, pure client
		return ELENetEnvironment::Client;
	}
}

// ============================================================================
// ULELogSubsystem Implementation
// ============================================================================

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

	// ========================================================================
	// 重要：初始化顺序调整
	// Important: Initialization order adjustment
	//
	// 由于 BqLog 使用 static 变量创建 Logger 实例，一旦创建就无法更改配置
	// 因此必须在初始化 BqLog 之前先加载 ConfigAsset，获取正确的配置
	//
	// Since BqLog uses static variable to create Logger instance, once created
	// the config cannot be changed. Therefore we must load ConfigAsset first
	// to get the correct configuration before initializing BqLog.
	//
	// 新的初始化顺序：
	// New initialization order:
	// 1. 加载 ConfigAsset 获取正确配置 / Load ConfigAsset to get correct config
	// 2. 用正确配置初始化 BqLog / Initialize BqLog with correct config
	// 3. 初始化分类树 / Initialize category tree
	// 4. 应用分类配置 / Apply category configurations
	// ========================================================================

	// Step 1: 尝试加载当前环境的配置
	// Try to load configuration for current environment
	LE_SYSTEM_LOG(TEXT("Step 1: Loading configuration for current environment..."));
	ULELogConfigAsset* LoadedAsset = LoadConfigAssetForCurrentEnvironment();

	// 确定要使用的 BqLog 配置
	// Determine which BqLog config to use
	FLEBqLogConfig BqLogConfig;
	bool bHasValidConfigAsset = false;

	if (IsValid(LoadedAsset))
	{
		// 验证配置
		TArray<FString> ValidationErrors;
		if (LoadedAsset->ValidateConfiguration(ValidationErrors))
		{
			// 使用 DataAsset 中的配置
			// Use configuration from DataAsset
			BqLogConfig = LoadedAsset->BqLogConfiguration;
			bHasValidConfigAsset = true;
			LE_SYSTEM_LOG(TEXT("  Using configuration from DataAsset"));
		}
		else
		{
			// 配置验证失败，记录错误
			LE_SYSTEM_WARNING(TEXT("  Configuration validation failed with %d error(s):"), ValidationErrors.Num());
			for (int32 i = 0; i < ValidationErrors.Num(); ++i)
			{
				LE_SYSTEM_WARNING(TEXT("    [Error %d]: %s"), i + 1, *ValidationErrors[i]);
			}
			LE_SYSTEM_WARNING(TEXT("  Falling back to default configuration"));
		}
	}
	else
	{
		LE_SYSTEM_WARNING(TEXT("  No config asset found for current environment, using defaults"));
	}

	// 如果没有有效配置，使用默认配置
	// If no valid config, use default configuration
	if (!bHasValidConfigAsset)
	{
		BqLogConfig.GlobalLogLevel = ELELogVerbosity::Info;
		BqLogConfig.bEnableAsyncLogging = true;
		BqLogConfig.BufferSize = 1048576; // 1MB
		BqLogConfig.LogFilePath = TEXT("LogEverything/Game.log");
		BqLogConfig.MaxLogFileSizeMB = 100;
		BqLogConfig.OutputTargets.Add(ELELogOutput::Console);
		BqLogConfig.OutputTargets.Add(ELELogOutput::File);
	}

	// Step 2: 用确定的配置初始化 BqLog
	// Initialize BqLog with determined configuration
	LE_SYSTEM_LOG(TEXT("Step 2: Initializing BqLog with %s configuration..."),
		bHasValidConfigAsset ? TEXT("DataAsset") : TEXT("default"));

	if (!InitializeBqLogBridge(BqLogConfig))
	{
		LE_SYSTEM_ERROR(TEXT("Failed to initialize LogEverything Bridge"));
		// 注意：即使 BqLog 初始化失败也继续执行，分类树仍可工作（使用备选方案）
		// Note: Continue execution even if BqLog initialization fails
	}

	// Step 3: 初始化分类树
	// Initialize category tree
	LE_SYSTEM_LOG(TEXT("Step 3: Initializing category tree..."));
	if (!InitializeCategoryTree())
	{
		LE_SYSTEM_ERROR(TEXT("Failed to initialize category tree!"));
		return;
	}

	// Step 4: 应用分类配置
	// Apply category configurations
	LE_SYSTEM_LOG(TEXT("Step 4: Applying category configurations..."));

	// 如果有有效的 ConfigAsset，应用其分类配置（包括 JSON 分类配置）
	// If valid ConfigAsset exists, apply its config (including JSON category config)
	if (bHasValidConfigAsset && IsValid(LoadedAsset) && IsValid(CategoryTree))
	{
		LoadedAsset->ApplyToCategoryTree(CategoryTree);
		LE_SYSTEM_LOG(TEXT("  Applied category configurations from DataAsset and JSON"));
	}
	else
	{
		LE_SYSTEM_LOG(TEXT("  No valid ConfigAsset, using default category levels"));
	}

	bIsInitialized = true;
	bStaticInitialized = true;

	LE_SYSTEM_LOG(TEXT("LogEverything Subsystem initialized successfully."));

	// Step 5: 打印完整分类树（受 CVar 控制）
	// Print full category tree (controlled by CVar)
	if (LogEverything::ConsoleVariable::DebugLogCategory.GetValueOnGameThread())
	{
		FString TreeDebug = ExportTreeDebugString();
		UE_LOG(LogTemp, Log, TEXT("=== LogEverything Category Tree ==="));
		TArray<FString> Lines;
		TreeDebug.ParseIntoArrayLines(Lines);
		for (const FString& Line : Lines)
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *Line);
		}
		UE_LOG(LogTemp, Log, TEXT("=== End Category Tree ==="));
	}
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
		// 重新加载配置
		// Reload configuration
		ULELogConfigAsset* LoadedAsset = LoadConfigAssetForCurrentEnvironment();
		if (IsValid(LoadedAsset) && IsValid(CategoryTree))
		{
			LoadedAsset->ApplyToCategoryTree(CategoryTree);
		}
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

bool ULELogSubsystem::InitializeBqLogBridge(const FLEBqLogConfig& Config)
{
	// ========================================================================
	// 从 FLEBqLogConfig 初始化 BqLog 桥接
	// Initialize BqLog bridge from FLEBqLogConfig
	// ========================================================================

	LE_SYSTEM_LOG(TEXT("InitializeBqLogBridge: Starting BqLog bridge initialization..."));
	LE_SYSTEM_LOG(TEXT("  GlobalLogLevel: %s"), *UEnum::GetValueAsString(Config.GlobalLogLevel));
	LE_SYSTEM_LOG(TEXT("  BufferSize: %d bytes"), Config.BufferSize);
	LE_SYSTEM_LOG(TEXT("  LogFilePath: %s"), *Config.LogFilePath);
	LE_SYSTEM_LOG(TEXT("  MaxLogFileSizeMB: %d MB"), Config.MaxLogFileSizeMB);
	LE_SYSTEM_LOG(TEXT("  bEnableAsyncLogging: %s"), Config.bEnableAsyncLogging ? TEXT("true") : TEXT("false"));

	// 输出目标配置日志
	// Log output target configuration
	FString OutputTargetsStr;
	if (Config.OutputTargets.Contains(ELELogOutput::Console))
	{
		OutputTargetsStr += TEXT("Console ");
	}
	if (Config.OutputTargets.Contains(ELELogOutput::File))
	{
		OutputTargetsStr += TEXT("File ");
	}
	if (Config.OutputTargets.Contains(ELELogOutput::Compressed))
	{
		OutputTargetsStr += TEXT("Compressed ");
	}
	LE_SYSTEM_LOG(TEXT("  OutputTargets: %s"), *OutputTargetsStr);

	// ========================================================================
	// 调用 BqLog 桥接初始化
	// Call BqLog bridge initialization
	// ========================================================================
	if (FLEBqLogBridge::Get().Initialize(Config, this))
	{
		LE_SYSTEM_LOG(TEXT("InitializeBqLogBridge: BqLog bridge initialized successfully"));
		return true;
	}
	else
	{
		LE_SYSTEM_ERROR(TEXT("InitializeBqLogBridge: Failed to initialize BqLog bridge"));
		return false;
	}
}

ULELogConfigAsset* ULELogSubsystem::LoadConfigAssetForCurrentEnvironment() const
{
	// ========================================================================
	// 为当前环境加载对应的日志配置 DataAsset
	// Load corresponding log config DataAsset for current environment
	// ========================================================================

	// Step 1: 检测当前构建环境和服务器类型
	// Detect current build environment and server type
	const ELEBuildEnvironment BuildEnv = GetCurrentBuildEnvironment();
	const ELENetEnvironment NetEnv = GetCurrentNetEnvironment();

	LE_SYSTEM_LOG(TEXT("LoadConfigAssetForCurrentEnvironment: Detecting environment..."));
	LE_SYSTEM_LOG(TEXT("  Build Environment: %s"), *UEnum::GetValueAsString(BuildEnv));
	LE_SYSTEM_LOG(TEXT("  Net Environment: %s"), *UEnum::GetValueAsString(NetEnv));

	// Step 2: 获取 ULELogEverythingSettings 单例
	// Get ULELogEverythingSettings singleton
	const ULELogEverythingSettings* Settings = GetDefault<ULELogEverythingSettings>();
	if (!IsValid(Settings))
	{
		LE_SYSTEM_ERROR(TEXT("LoadConfigAssetForCurrentEnvironment: Failed to get ULELogEverythingSettings"));
		return nullptr;
	}

	// Step 3: 根据环境类型选择对应的 TSoftObjectPtr<ULELogConfigAsset>
	// Select corresponding TSoftObjectPtr<ULELogConfigAsset> based on environment type
	TSoftObjectPtr<ULELogConfigAsset> ConfigAssetPtr;

	// 根据构建环境和服务器类型选择配置
	// Select config based on build environment and server type
	if (BuildEnv == ELEBuildEnvironment::Development)
	{
		// Development 环境
		// Development environment
		if (NetEnv == ELENetEnvironment::Client)
		{
			ConfigAssetPtr = Settings->DevelopmentClient;
			LE_SYSTEM_LOG(TEXT("  Selected Config: DevelopmentClient"));
		}
		else // DedicatedServer
		{
			ConfigAssetPtr = Settings->DevelopmentServer;
			LE_SYSTEM_LOG(TEXT("  Selected Config: DevelopmentServer"));
		}
	}
	else if (BuildEnv == ELEBuildEnvironment::Debug)
	{
		// Debug 环境
		// Debug environment
		if (NetEnv == ELENetEnvironment::Client)
		{
			ConfigAssetPtr = Settings->DebugClient;
			LE_SYSTEM_LOG(TEXT("  Selected Config: DebugClient"));
		}
		else // DedicatedServer
		{
			ConfigAssetPtr = Settings->DebugServer;
			LE_SYSTEM_LOG(TEXT("  Selected Config: DebugServer"));
		}
	}
	else if (BuildEnv == ELEBuildEnvironment::Test)
	{
		// Test 环境
		// Test environment
		if (NetEnv == ELENetEnvironment::Client)
		{
			ConfigAssetPtr = Settings->TestClient;
			LE_SYSTEM_LOG(TEXT("  Selected Config: TestClient"));
		}
		else // DedicatedServer
		{
			ConfigAssetPtr = Settings->TestServer;
			LE_SYSTEM_LOG(TEXT("  Selected Config: TestServer"));
		}
	}
	else if (BuildEnv == ELEBuildEnvironment::Shipping)
	{
		// Shipping 环境
		// Shipping environment
		if (NetEnv == ELENetEnvironment::Client)
		{
			ConfigAssetPtr = Settings->ShippingClient;
			LE_SYSTEM_LOG(TEXT("  Selected Config: ShippingClient"));
		}
		else // DedicatedServer
		{
			ConfigAssetPtr = Settings->ShippingServer;
			LE_SYSTEM_LOG(TEXT("  Selected Config: ShippingServer"));
		}
	}

	// Step 4: 检查是否有配置引用
	// Check if config reference exists
	if (ConfigAssetPtr.IsNull())
	{
		LE_SYSTEM_WARNING(TEXT("LoadConfigAssetForCurrentEnvironment: No config asset configured for current environment"));
		LE_SYSTEM_WARNING(TEXT("  Please configure corresponding DataAsset in Project Settings -> LogEverything Settings"));
		return nullptr;
	}

	// Step 5: 同步加载 DataAsset
	// Synchronously load DataAsset
	LE_SYSTEM_LOG(TEXT("  Config Asset Path: %s"), *ConfigAssetPtr.ToSoftObjectPath().ToString());
	ULELogConfigAsset* LoadedAsset = ConfigAssetPtr.LoadSynchronous();

	if (!IsValid(LoadedAsset))
	{
		LE_SYSTEM_ERROR(TEXT("LoadConfigAssetForCurrentEnvironment: Failed to load config asset"));
		LE_SYSTEM_ERROR(TEXT("  Asset Path: %s"), *ConfigAssetPtr.ToSoftObjectPath().ToString());
		return nullptr;
	}

	LE_SYSTEM_LOG(TEXT("LoadConfigAssetForCurrentEnvironment: Successfully loaded config asset"));
	return LoadedAsset;
}

bool ULELogSubsystem::LoadLogSettings()
{
	// ========================================================================
	// 加载日志设置（仅用于运行时重载分类树配置）
	// Load Log Settings (only for runtime category tree config reload)
	// ========================================================================
	// 注意：此函数现在主要用于运行时重新加载分类树配置
	// BqLog 配置只能在初始化时设置一次（使用 static 变量），无法在运行时更改
	//
	// Note: This function is now mainly used for runtime category tree config reload
	// BqLog config can only be set once at initialization (uses static variable),
	// cannot be changed at runtime.
	// ========================================================================

	LE_SYSTEM_LOG(TEXT("LoadLogSettings: Starting log settings loading process..."));

	// Step 1: 加载当前环境的日志配置 DataAsset
	// Load log config DataAsset for current environment
	ULELogConfigAsset* LoadedAsset = LoadConfigAssetForCurrentEnvironment();

	if (!IsValid(LoadedAsset))
	{
		LE_SYSTEM_WARNING(TEXT("LoadLogSettings: Failed to load config asset for current environment"));
		LE_SYSTEM_WARNING(TEXT("  Category tree will keep current configuration"));
		return false;
	}

	// Step 2: 验证配置的合法性
	// Validate configuration
	TArray<FString> ValidationErrors;
	bool bValidationPassed = LoadedAsset->ValidateConfiguration(ValidationErrors);

	if (!bValidationPassed)
	{
		LE_SYSTEM_ERROR(TEXT("LoadLogSettings: Configuration validation failed with %d error(s):"), ValidationErrors.Num());
		for (int32 i = 0; i < ValidationErrors.Num(); ++i)
		{
			LE_SYSTEM_ERROR(TEXT("  [Error %d]: %s"), i + 1, *ValidationErrors[i]);
		}
		LE_SYSTEM_WARNING(TEXT("  Category tree will keep current configuration"));
		return false;
	}

	LE_SYSTEM_LOG(TEXT("LoadLogSettings: Configuration validation passed successfully"));

	// Step 3: 应用配置到运行时分类树
	// Apply configuration to runtime category tree
	// 注意：BqLog 配置无法在运行时更改，只能更新分类树配置
	// Note: BqLog config cannot be changed at runtime, only category tree config can be updated
	if (!IsValid(CategoryTree))
	{
		LE_SYSTEM_ERROR(TEXT("LoadLogSettings: CategoryTree is null, cannot apply configuration"));
		return false;
	}

	// 调用 ULELogConfigAsset::ApplyToCategoryTree 应用配置
	// Call ULELogConfigAsset::ApplyToCategoryTree to apply configuration
	LoadedAsset->ApplyToCategoryTree(CategoryTree);

	LE_SYSTEM_LOG(TEXT("LoadLogSettings: Configuration applied to category tree successfully"));
	LE_SYSTEM_LOG(TEXT("LoadLogSettings: Log settings loaded and applied successfully"));
	LE_SYSTEM_LOG(TEXT("  Note: BqLog configuration can only be set at initialization, not at runtime"));

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