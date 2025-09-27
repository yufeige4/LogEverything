// Copyright Epic Games, Inc. All Rights Reserved.

#include "Utils/LogEverythingUtils.h"
#include "System/LELogSubsystem.h"
#include "System/LELogTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

bool ULogEverythingUtils::SetLogCategoryLevel(const UObject* WorldContext, const FName& CategoryPath,
	ELELogVerbosity Level, bool bPropagate)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for SetLogCategoryLevel"));
		return false;
	}

	return LogSubsystem->SetCategoryLevel(CategoryPath, Level, bPropagate);
}

ELELogVerbosity ULogEverythingUtils::GetEffectiveLogLevel(const UObject* WorldContext, const FName& CategoryPath)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for GetEffectiveLogLevel"));
		return ELELogVerbosity::Info;
	}

	return LogSubsystem->GetEffectiveLevel(CategoryPath);
}

bool ULogEverythingUtils::ShouldLogCategory(const UObject* WorldContext, const FName& CategoryName, ELELogVerbosity Level)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		// 使用默认规则：Info 及以上级别显示
		return static_cast<uint8>(Level) >= static_cast<uint8>(ELELogVerbosity::Info);
	}

	return LogSubsystem->ShouldLogCategory(CategoryName, Level);
}

bool ULogEverythingUtils::SetCategoryEnabled(const UObject* WorldContext, const FName& CategoryPath,
	bool bEnabled, bool bPropagate)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for SetCategoryEnabled"));
		return false;
	}

	return LogSubsystem->SetCategoryEnabled(CategoryPath, bEnabled, bPropagate);
}

bool ULogEverythingUtils::IsCategoryEnabled(const UObject* WorldContext, const FName& CategoryPath)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		return true; // 默认启用
	}

	return LogSubsystem->IsCategoryEnabled(CategoryPath);
}

TArray<FString> ULogEverythingUtils::GetAllCategoryPaths(const UObject* WorldContext)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		return TArray<FString>();
	}

	return LogSubsystem->GetAllCategoryPaths();
}

TArray<FString> ULogEverythingUtils::GetChildCategories(const UObject* WorldContext, const FName& CategoryPath)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		return TArray<FString>();
	}

	return LogSubsystem->GetChildCategories(CategoryPath);
}

void ULogEverythingUtils::ResetToDefault(const UObject* WorldContext)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for ResetToDefault"));
		return;
	}

	LogSubsystem->ResetToDefault();
}

void ULogEverythingUtils::GetTreeStatistics(const UObject* WorldContext, int32& OutTotalNodes, int32& OutMaxDepth, int32& OutExplicitNodes)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		OutTotalNodes = 0;
		OutMaxDepth = 0;
		OutExplicitNodes = 0;
		return;
	}

	LogSubsystem->GetTreeStatistics(OutTotalNodes, OutMaxDepth, OutExplicitNodes);
}

FString ULogEverythingUtils::ExportTreeDebugString(const UObject* WorldContext)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		return TEXT("LogSubsystem not available");
	}

	return LogSubsystem->ExportTreeDebugString();
}

bool ULogEverythingUtils::ReinitializeCategoryTree(const UObject* WorldContext)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for ReinitializeCategoryTree"));
		return false;
	}

	return LogSubsystem->ReinitializeCategoryTree();
}

bool ULogEverythingUtils::ReloadLogSettings(const UObject* WorldContext)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for ReloadLogSettings"));
		return false;
	}

	return LogSubsystem->ReloadLogSettings();
}

bool ULogEverythingUtils::IsLogSystemInitialized(const UObject* WorldContext)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	return LogSubsystem && LogSubsystem->IsInitialized();
}

void ULogEverythingUtils::SetGameCategoriesLevel(const UObject* WorldContext, ELELogVerbosity Level)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for SetGameCategoriesLevel"));
		return;
	}

	LogSubsystem->SetCategoryLevel(TEXT("LogRoot.Game"), Level, false);
}

void ULogEverythingUtils::SetEngineCategoriesLevel(const UObject* WorldContext, ELELogVerbosity Level)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for SetEngineCategoriesLevel"));
		return;
	}

	LogSubsystem->SetCategoryLevel(TEXT("LogRoot.Engine"), Level, false);
}

void ULogEverythingUtils::SetEditorCategoriesLevel(const UObject* WorldContext, ELELogVerbosity Level)
{
	ULELogSubsystem* LogSubsystem = GetLogSubsystem(WorldContext);
	if (!LogSubsystem)
	{
		LE_SYSTEM_WARNING(TEXT("LogSubsystem not available for SetEditorCategoriesLevel"));
		return;
	}

	LogSubsystem->SetCategoryLevel(TEXT("LogRoot.Editor"), Level, false);
}

ULELogSubsystem* ULogEverythingUtils::GetLogSubsystem(const UObject* WorldContext)
{
	if (!IsValid(WorldContext))
	{
		return nullptr;
	}

	return ULELogSubsystem::Get(WorldContext);
}