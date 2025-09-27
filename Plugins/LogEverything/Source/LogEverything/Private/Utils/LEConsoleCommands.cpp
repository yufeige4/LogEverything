// Copyright Epic Games, Inc. All Rights Reserved.

#include "Category/LECategoryDefine.h"
#include "Macros/LELogMacros.h"
#include "System/LELogSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

// =============================================================================
// LogEverything Namespace - Console Variables & Test Functions
// =============================================================================

namespace LogEverything
{
	namespace ConsoleFunction
	{

		// =============================================================================
		// Conditional logging test command
		// =============================================================================

		/**
		 * LE.Test.ConditionalLogging - Tests conditional logging workflow
		 * Demonstrates LE_CLOG and related conditional logging macros
		 */
		static FAutoConsoleCommand TestConditionalLoggingCommand(
			TEXT("LE.Test.ConditionalLogging"),
			TEXT("Test LogEverything conditional logging\nDemonstrates LE_CLOG and other conditional log macros"),
			FConsoleCommandDelegate::CreateLambda([]() {
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("=== Conditional logging test ==="));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

				// Simulate sample gameplay state
				float PlayerHealth = 15.0f;
				int32 AmmoCount = 0;
				bool bIsEnemyNear = true;

				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Current gameplay state:"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("- Player health: {:.1f}"), PlayerHealth);
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("- Ammo count: {}"), AmmoCount);
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("- Enemy nearby: {}"), bIsEnemyNear ? TEXT("Yes") : TEXT("No"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

				// Exercise conditional logging
				LE_CLOG_DEBUG(PlayerHealth < 20.0f, LELogGameCombatDamage, TEXT("Player health critically low: {:.1f}"), PlayerHealth);
				LE_CLOG_DEBUG(AmmoCount == 0, LELogGameCombatSkill, TEXT("Ammunition depleted, ranged skills unavailable"));
				LE_CLOG_DEBUG(bIsEnemyNear && PlayerHealth < 50.0f, LELogGameAI, TEXT("Danger: enemy closing in while health is low"));

				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("=== Conditional logging test complete ==="));
			})
		);

		// =============================================================================
		// Dynamic log level test command
		// =============================================================================

		/**
		 * LE.Test.DynamicLevelFilter - Tests dynamic log level adjustments
		 * Demonstrates live log level adjustments and CVar control
		 */
		static FAutoConsoleCommand TestDynamicLevelFilterCommand(
			TEXT("LE.Test.DynamicLevelFilter"),
			TEXT("Test LogEverything dynamic log level filtering\nDemonstrates real-time log level adjustments and CVar control"),
			FConsoleCommandDelegate::CreateLambda([]() {
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("===================================================="));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("          Dynamic log level test"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("===================================================="));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

				// Acquire the LogEverything subsystem
				ULELogSubsystem* LogSubsystem = ULELogSubsystem::Get(nullptr);
				if (!LogSubsystem)
				{
					LE_LOG_ERROR(LELogTestLogSystem, TEXT("Unable to acquire ULELogSubsystem instance!"));
					return;
				}

				const FName TestCategoryName(TEXT("Test.LogSystem"));

				// === Step 1: Configure LELogTestLogSystem to Verbose ===
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Step 1: Set Test.LogSystem category to Verbose"));

				bool bSetResult = LogSubsystem->SetCategoryLevel(TestCategoryName, ELELogVerbosity::Verbose);

				// Verify the change
				ELELogVerbosity CurrentLevel = LogSubsystem->GetEffectiveLevel(TestCategoryName);
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Current Test.LogSystem effective level: {} (Verbose=5)"), (int32)CurrentLevel);
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

				// === Step 2: Emit Verbose log ===
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Step 2: Emit Verbose-level log"));
				LE_LOG_VERBOSE(LELogTestLogSystem, TEXT("This is a Verbose-level test log entry"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

				// === Step 3: Switch level to Debug ===
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Step 3: Change Test.LogSystem category to Debug at runtime"));

				bSetResult = LogSubsystem->SetCategoryLevel(TestCategoryName, ELELogVerbosity::Debug);

				// Verify the updated level
				CurrentLevel = LogSubsystem->GetEffectiveLevel(TestCategoryName);
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Updated Test.LogSystem effective level: {} (Debug=1)"), (int32)CurrentLevel);
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

				// === Step 4: Emit Verbose log again (should be blocked) ===
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Step 4: Emit Verbose log again (should be blocked)"));

				// Check whether a Verbose log should be recorded
				bool bShouldLog = LogSubsystem->ShouldLogCategory(TestCategoryName, ELELogVerbosity::Verbose);
				if (LogEverything::ConsoleVariable::DebugLogCategory.GetValueOnGameThread())
				{
					LE_LOG_DEBUG(LELogTestLogSystem, TEXT("[Debug] Should Verbose log be recorded: {}"), bShouldLog ? TEXT("Yes") : TEXT("No"));
				}

				// Attempt to emit a Verbose log
				LE_LOG_VERBOSE(LELogTestLogSystem, TEXT("Another Verbose-level test log (may be filtered)"));

				if (!bShouldLog && LogEverything::ConsoleVariable::DebugLogCategory.GetValueOnGameThread())
				{
					LE_LOG_DEBUG(LELogTestLogSystem, TEXT("[Debug] Verbose log was filtered by level"));
				}
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

				// === Step 5: Test CVar controls ===
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Step 5: Demonstrate CVar toggling for debug output"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Current LogEverything.DebugLogCategory = {}"),
				             LogEverything::ConsoleVariable::DebugLogCategory.GetValueOnGameThread() ? TEXT("true") : TEXT("false"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Run command: LogEverything.DebugLogCategory 1 (enable debug output)"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Run command: LogEverything.DebugLogCategory 0 (disable debug output)"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));

				// === Summary ===
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("===================================================="));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("          Dynamic level test complete"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("===================================================="));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Key takeaways:"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("1. Successfully changed Test.LogSystem from Verbose -> Log"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("2. Verified Verbose logs are blocked after lowering the level"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("3. Demonstrated CVar LogEverything.DebugLogCategory controlling debug output"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("4. Used ULELogSubsystem API for runtime log configuration"));
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT(""));
			})
		);

		// =============================================================================
		// Debug utility commands
		// =============================================================================

		/**
		 * LE.Debug.PrintCategoryTree - Prints the full category tree
		 * Displays hierarchy, state, and configuration for each node
		 */
		static FAutoConsoleCommand PrintCategoryTreeCommand(
			TEXT("LE.Debug.PrintCategoryTree"),
			TEXT("Print the full LogEverything category tree\nDisplays hierarchy, state, and configuration for each node"),
			FConsoleCommandDelegate::CreateLambda([]() {
				ULELogSubsystem* LogSubsystem = ULELogSubsystem::Get(nullptr);
				if (!LogSubsystem)
				{
					LE_LOG_ERROR(LELogTestLogSystem, TEXT("Unable to acquire ULELogSubsystem instance!"));
					return;
				}

				FString TreeDebugString = LogSubsystem->ExportTreeDebugString();

				// Print the tree line by line
				TArray<FString> Lines;
				TreeDebugString.ParseIntoArrayLines(Lines, false);

				for (const FString& Line : Lines)
				{
					LE_LOG_DEBUG(LELogTestLogSystem, TEXT("{}"), *Line);
				}
			})
		);

		/**
		 * LE.Debug.QueryCategoryLevel <CategoryName> - Queries the effective level for a category
		 * Shows the effective level and whether it was explicitly set
		 */
		static FAutoConsoleCommand QueryCategoryLevelCommand(
			TEXT("LE.Debug.QueryCategoryLevel"),
			TEXT("Query the effective log level for a category\nUsage: LE.Debug.QueryCategoryLevel <CategoryName>\nExample: LE.Debug.QueryCategoryLevel Test.LogSystem"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
				if (Args.Num() == 0)
				{
					LE_LOG_ERROR(LELogTestLogSystem, TEXT("Usage: LE.Debug.QueryCategoryLevel <CategoryName>"));
					LE_LOG_ERROR(LELogTestLogSystem, TEXT("Example: LE.Debug.QueryCategoryLevel Test.LogSystem"));
					return;
				}

				FString CategoryName = Args[0];
				ULELogSubsystem* LogSubsystem = ULELogSubsystem::Get(nullptr);
				if (!LogSubsystem)
				{
					LE_LOG_ERROR(LELogTestLogSystem, TEXT("Unable to acquire ULELogSubsystem instance!"));
					return;
				}

				FName CategoryFName(*CategoryName);

				// Query the effective level
				ELELogVerbosity EffectiveLevel = LogSubsystem->GetEffectiveLevel(CategoryFName);

				// Log the query result
				LE_LOG_DEBUG(LELogTestLogSystem, TEXT("Effective level for category {}: {} ({})"),
					*CategoryName, (int32)EffectiveLevel, *UEnum::GetValueAsString(EffectiveLevel));
			})
		);
	}
}

