// Copyright Benedict Guo. All Rights Reserved.

#include "Config/LELogConfigAsset.h"
#include "Category/LECategoryTree.h"
#include "Utils/LogEverythingUtils.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IPluginManager.h"

// ============================================================================
// ULELogConfigAsset Implementation
// ============================================================================

FString ULELogConfigAsset::GetJsonConfigFullPath() const
{
	// 构建完整路径：插件目录/Config/CategoryConfigJsonPath
	// Build full path: Plugin directory/Config/CategoryConfigJsonPath
	FString PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("LogEverything"))->GetBaseDir();
	return FPaths::Combine(PluginBaseDir, TEXT("Config"), CategoryConfigJsonPath);
}

bool ULELogConfigAsset::ValidateConfiguration(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();

	// 验证 JSON 文件路径非空
	// Validate JSON file path is not empty
	if (CategoryConfigJsonPath.IsEmpty())
	{
		OutErrors.Add(TEXT("CategoryConfigJsonPath is empty"));
		return false;
	}

	// 验证 JSON 文件存在
	// Validate JSON file exists
	FString FullPath = GetJsonConfigFullPath();
	if (!FPaths::FileExists(FullPath))
	{
		// JSON 文件不存在不算错误，只是没有分类配置
		// JSON file not existing is not an error, just no category config
		LE_SYSTEM_LOG(TEXT("ValidateConfiguration: JSON config file not found at '%s', will use defaults"), *FullPath);
	}

	// BqLogConfiguration 的基本验证
	// Basic validation of BqLogConfiguration
	if (BqLogConfiguration.BufferSize < 1024)
	{
		OutErrors.Add(FString::Printf(TEXT("BufferSize (%d) is too small, minimum is 1024"), BqLogConfiguration.BufferSize));
	}

	return OutErrors.Num() == 0;
}

void ULELogConfigAsset::ApplyToCategoryTree(ULECategoryTree* InCategoryTree) const
{
	if (!InCategoryTree)
	{
		LE_SYSTEM_WARNING(TEXT("ApplyToCategoryTree: InCategoryTree is null, cannot apply configuration"));
		return;
	}

	// 同步全局日志级别
	// Sync global log level
	InCategoryTree->SetDefaultGlobalLevel(BqLogConfiguration.GlobalLogLevel);
	LE_SYSTEM_LOG(TEXT("ApplyToCategoryTree: Set global log level to %s"),
		*UEnum::GetValueAsString(BqLogConfiguration.GlobalLogLevel));

	// 从 JSON 文件加载并应用分类配置
	// Load and apply category config from JSON file
	LoadAndApplyCategoryConfigFromJson(InCategoryTree);
}

bool ULELogConfigAsset::LoadAndApplyCategoryConfigFromJson(ULECategoryTree* InCategoryTree) const
{
	if (!InCategoryTree)
	{
		LE_SYSTEM_WARNING(TEXT("LoadAndApplyCategoryConfigFromJson: InCategoryTree is null"));
		return false;
	}

	// ========================================================================
	// 步骤 1: 获取文件路径并读取文件
	// Step 1: Get file path and read file
	// ========================================================================
	FString FullPath = GetJsonConfigFullPath();

	if (!FPaths::FileExists(FullPath))
	{
		LE_SYSTEM_LOG(TEXT("LoadAndApplyCategoryConfigFromJson: JSON config file not found at '%s', using defaults"), *FullPath);
		return false;
	}

	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *FullPath))
	{
		LE_SYSTEM_ERROR(TEXT("LoadAndApplyCategoryConfigFromJson: Failed to read JSON file: '%s'"), *FullPath);
		return false;
	}

	LE_SYSTEM_LOG(TEXT("LoadAndApplyCategoryConfigFromJson: Loaded JSON file (%d bytes): '%s'"), JsonContent.Len(), *FullPath);

	// ========================================================================
	// 步骤 2: 解析 JSON
	// Step 2: Parse JSON
	// ========================================================================
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		LE_SYSTEM_ERROR(TEXT("LoadAndApplyCategoryConfigFromJson: Failed to parse JSON file: '%s'"), *FullPath);
		return false;
	}

	// ========================================================================
	// 步骤 3: 读取 defaultLevel 并设置全局级别
	// Step 3: Read defaultLevel and set global level
	// ========================================================================
	FString DefaultLevelStr;
	if (RootObject->TryGetStringField(TEXT("defaultLevel"), DefaultLevelStr))
	{
		ELELogVerbosity DefaultLevel;
		if (StringToLogVerbosity(DefaultLevelStr, DefaultLevel))
		{
			InCategoryTree->SetDefaultGlobalLevel(DefaultLevel);
			LE_SYSTEM_LOG(TEXT("  JSON defaultLevel: %s"), *DefaultLevelStr);
		}
		else
		{
			LE_SYSTEM_WARNING(TEXT("  JSON defaultLevel '%s' is invalid, ignoring"), *DefaultLevelStr);
		}
	}

	// ========================================================================
	// 步骤 4: 递归解析 categories 数组并应用
	// Step 4: Recursively parse categories array and apply
	// ========================================================================
	const TArray<TSharedPtr<FJsonValue>>* CategoriesArray;
	if (!RootObject->TryGetArrayField(TEXT("categories"), CategoriesArray))
	{
		LE_SYSTEM_LOG(TEXT("  JSON has no 'categories' array, skipping category config"));
		return true;
	}

	int32 AppliedCount = 0;
	for (const TSharedPtr<FJsonValue>& CategoryValue : *CategoriesArray)
	{
		const TSharedPtr<FJsonObject>* CategoryObject;
		if (CategoryValue.IsValid() && CategoryValue->TryGetObject(CategoryObject))
		{
			ParseAndApplyJsonNodeRecursive(*CategoryObject, FString(), InCategoryTree, AppliedCount);
		}
	}

	LE_SYSTEM_LOG(TEXT("  Applied JSON config to %d category nodes"), AppliedCount);

	// ========================================================================
	// 步骤 5: 传播 EffectiveLevel 和 EnabledState
	// Step 5: Propagate EffectiveLevel and EnabledState
	// ========================================================================
	const int32 RootNodeIndex = InCategoryTree->RootNodeIndex;
	if (InCategoryTree->IsValidNodeIndex(RootNodeIndex))
	{
		const FLECategoryNode& RootNode = InCategoryTree->Nodes[RootNodeIndex];

		InCategoryTree->InternalUpdateChildrenEffectiveLevels(
			RootNodeIndex, RootNode.EffectiveLevel, false);

		InCategoryTree->InternalUpdateChildrenEnabledState(
			RootNodeIndex, RootNode.bIsEnabled);

		LE_SYSTEM_LOG(TEXT("  Propagated EffectiveLevel and EnabledState from root"));
	}

	LE_SYSTEM_LOG(TEXT("LoadAndApplyCategoryConfigFromJson: Configuration applied successfully"));
	return true;
}

void ULELogConfigAsset::ParseAndApplyJsonNodeRecursive(
	const TSharedPtr<FJsonObject>& JsonNode,
	const FString& ParentPath,
	ULECategoryTree* InCategoryTree,
	int32& OutAppliedCount) const
{
	if (!JsonNode.IsValid() || !InCategoryTree)
	{
		return;
	}

	// 读取 name 字段（必填）
	// Read name field (required)
	FString NodeName;
	if (!JsonNode->TryGetStringField(TEXT("name"), NodeName) || NodeName.IsEmpty())
	{
		LE_SYSTEM_WARNING(TEXT("JSON category node missing 'name' field, skipping"));
		return;
	}

	// 构建完整路径
	// Build full path
	FString FullPath = ParentPath.IsEmpty() ? NodeName : (ParentPath + TEXT(".") + NodeName);

	// 查找运行时节点
	// Find runtime node
	int32 NodeIndex = InCategoryTree->FindNodeIndex(FullPath);
	if (NodeIndex == INDEX_NONE)
	{
		LE_SYSTEM_WARNING(TEXT("JSON category '%s' not found in CategoryTree, skipping"), *FullPath);
	}
	else
	{
		FLECategoryNode& Node = InCategoryTree->Nodes[NodeIndex];

		// 应用 level（可选）
		// Apply level (optional)
		FString LevelStr;
		if (JsonNode->TryGetStringField(TEXT("level"), LevelStr))
		{
			ELELogVerbosity Level;
			if (StringToLogVerbosity(LevelStr, Level))
			{
				Node.bHasExplicitLevel = true;
				Node.ExplicitLevel = Level;
				Node.EffectiveLevel = Level;
			}
			else
			{
				LE_SYSTEM_WARNING(TEXT("JSON category '%s' has invalid level '%s', ignoring"), *FullPath, *LevelStr);
			}
		}

		// 应用 enabled（可选）
		// Apply enabled (optional)
		FString EnabledStr;
		if (JsonNode->TryGetStringField(TEXT("enabled"), EnabledStr))
		{
			ELEEnabledState State;
			if (StringToEnabledState(EnabledStr, State))
			{
				ELEEnabledState OldState = Node.ExplicitEnabled;
				Node.ExplicitEnabled = State;

				if (State == ELEEnabledState::Enabled)
				{
					Node.bIsEnabled = true;
				}
				else if (State == ELEEnabledState::Disabled)
				{
					Node.bIsEnabled = false;
				}
				else // NotSet
				{
					// 继承父节点状态
					// Inherit parent state
					if (InCategoryTree->IsValidNodeIndex(Node.ParentIndex))
					{
						Node.bIsEnabled = InCategoryTree->Nodes[Node.ParentIndex].bIsEnabled;
					}
					else
					{
						Node.bIsEnabled = true;
					}
				}
			}
			else
			{
				LE_SYSTEM_WARNING(TEXT("JSON category '%s' has invalid enabled '%s', ignoring"), *FullPath, *EnabledStr);
			}
		}

		OutAppliedCount++;
	}

	// 递归处理子节点
	// Recursively process children
	const TArray<TSharedPtr<FJsonValue>>* ChildrenArray;
	if (JsonNode->TryGetArrayField(TEXT("children"), ChildrenArray))
	{
		for (const TSharedPtr<FJsonValue>& ChildValue : *ChildrenArray)
		{
			const TSharedPtr<FJsonObject>* ChildObject;
			if (ChildValue.IsValid() && ChildValue->TryGetObject(ChildObject))
			{
				ParseAndApplyJsonNodeRecursive(*ChildObject, FullPath, InCategoryTree, OutAppliedCount);
			}
		}
	}
}

bool ULELogConfigAsset::StringToLogVerbosity(const FString& LevelStr, ELELogVerbosity& OutLevel)
{
	if (LevelStr.Equals(TEXT("Verbose"), ESearchCase::IgnoreCase))
	{
		OutLevel = ELELogVerbosity::Verbose;
		return true;
	}
	if (LevelStr.Equals(TEXT("Debug"), ESearchCase::IgnoreCase))
	{
		OutLevel = ELELogVerbosity::Debug;
		return true;
	}
	if (LevelStr.Equals(TEXT("Info"), ESearchCase::IgnoreCase))
	{
		OutLevel = ELELogVerbosity::Info;
		return true;
	}
	if (LevelStr.Equals(TEXT("Warning"), ESearchCase::IgnoreCase))
	{
		OutLevel = ELELogVerbosity::Warning;
		return true;
	}
	if (LevelStr.Equals(TEXT("Error"), ESearchCase::IgnoreCase))
	{
		OutLevel = ELELogVerbosity::Error;
		return true;
	}
	if (LevelStr.Equals(TEXT("Fatal"), ESearchCase::IgnoreCase))
	{
		OutLevel = ELELogVerbosity::Fatal;
		return true;
	}
	return false;
}

bool ULELogConfigAsset::StringToEnabledState(const FString& StateStr, ELEEnabledState& OutState)
{
	if (StateStr.Equals(TEXT("NotSet"), ESearchCase::IgnoreCase))
	{
		OutState = ELEEnabledState::NotSet;
		return true;
	}
	if (StateStr.Equals(TEXT("Disabled"), ESearchCase::IgnoreCase))
	{
		OutState = ELEEnabledState::Disabled;
		return true;
	}
	if (StateStr.Equals(TEXT("Enabled"), ESearchCase::IgnoreCase))
	{
		OutState = ELEEnabledState::Enabled;
		return true;
	}
	return false;
}
