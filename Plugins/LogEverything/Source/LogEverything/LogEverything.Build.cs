// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LogEverything : ModuleRules
{
	public LogEverything(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// BqLog 要求禁用异常和 RTTI
		bEnableExceptions = false;
		bUseRTTI = false;

		// 禁用将警告视为错误，因为 BqLog 源码有一些警告
		bTreatAsEngineModule = false;

		// BqLog CMake 配置映射
		PublicDefinitions.AddRange(new string[]
		{
			"BQ_BUILD_STATIC_LIB=1",
			"LOGEVERYTHING_API=DLLEXPORT" // 定义导出宏
		});

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.AddRange(new string[]
			{
				"_CRT_SECURE_NO_WARNINGS"
			});

			// 禁用 BqLog 中的警告
			bEnableUndefinedIdentifierWarnings = false;

			// MSVC 特定编译选项 (映射自 CMake: /EHsc /D_HAS_EXCEPTIONS=0 /GR-)
			// bEnableExceptions=false 和 bUseRTTI=false 已经处理了 /D_HAS_EXCEPTIONS=0 和 /GR-
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Mac)
		{
			// GCC/Clang 特定编译选项 (映射自 CMake: -fno-exceptions -fno-rtti)
			// bEnableExceptions=false 和 bUseRTTI=false 已经处理了这些选项
		}
		
		PublicIncludePaths.AddRange(
			new string[] {
				// BqLog 包含路径
				System.IO.Path.Combine(ModuleDirectory, "..", "BQLog"),
				System.IO.Path.Combine(ModuleDirectory, "..", "BQLog", "bq_common"),
				System.IO.Path.Combine(ModuleDirectory, "..", "BQLog", "bq_log"),
				System.IO.Path.Combine(ModuleDirectory, "..", "BQLog", "include"),
				// 生成的文件路径
				System.IO.Path.Combine(ModuleDirectory, "..", "..", "Source", "Generated")
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		// 添加平台特定的编译定义
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.AddRange(new string[] {
				"BQ_WIN=1",
				"BQ_WIN64=1"
			});
		}
	}
}
