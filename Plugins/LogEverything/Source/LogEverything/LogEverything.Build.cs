// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class LogEverything : ModuleRules
{
	public LogEverything(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// 定义导出宏
		PublicDefinitions.AddRange(new string[]
		{
			"LOGEVERYTHING_API=DLLEXPORT",
			"BQ_BUILD_STATIC_LIB=1",  // BqLog静态库编译选项
			"_CRT_SECURE_NO_WARNINGS"
		});

		// BqLog路径配置
		string ThirdPartyPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty");
		string BqLogPath = Path.Combine(ThirdPartyPath, "BqLog");
		string BqLogIncludePath = Path.Combine(BqLogPath, "include");

		// 添加BqLog头文件路径
		PublicIncludePaths.AddRange(new string[]
		{
			BqLogIncludePath,
			Path.Combine(ModuleDirectory, "..", "..", "Source", "Generated")
		});

		// 根据UE配置映射到BqLog配置
		string GetBqLogConfig(UnrealTargetConfiguration Config)
		{
			if (Config == UnrealTargetConfiguration.Debug)
			{
				return "Debug";
			}
			else if (Config == UnrealTargetConfiguration.DebugGame ||
			         Config == UnrealTargetConfiguration.Development)
			{
				return "RelWithDebInfo"; // 使用RelWithDebInfo以确保运行时库兼容性
			}
			else
			{
				return "Release"; // Shipping, Test等其他配置
			}
		}

		string BqLogConfig = GetBqLogConfig(Target.Configuration);

		// 平台特定的库链接配置
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			LinkWin64StaticLib(BqLogPath, BqLogConfig);
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			LinkMacStaticLib(BqLogPath, BqLogConfig);
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			LinkLinux64StaticLib(BqLogPath, BqLogConfig);
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			LinkAndroidDynamicLib(BqLogPath, BqLogConfig, Target);
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			LinkiOSFramework(BqLogPath, BqLogConfig);
		}
		else
		{
			System.Console.WriteLine($"Warning: BqLog library may not be available for platform {Target.Platform}");
		}

		// 平台特定的编译定义
		AddPlatformDefinitions(Target.Platform);

		// 模块依赖
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Engine"
		});
	}

	private void LinkWin64StaticLib(string BqLogPath, string BqLogConfig)
	{
		string LibPath = Path.Combine(BqLogPath, "static_lib", "win64", BqLogConfig, "BqLog.lib");

		if (System.IO.File.Exists(LibPath))
		{
			PublicAdditionalLibraries.Add(LibPath);
			System.Console.WriteLine($"LogEverything: Using Win64 BqLog library ({BqLogConfig}): {LibPath}");
		}
		else
		{
			// 回退到Release版本
			string FallbackPath = Path.Combine(BqLogPath, "static_lib", "win64", "Release", "BqLog.lib");
			if (System.IO.File.Exists(FallbackPath))
			{
				PublicAdditionalLibraries.Add(FallbackPath);
				System.Console.WriteLine($"LogEverything: Using Win64 BqLog library (fallback to Release): {FallbackPath}");
			}
			else
			{
				System.Console.WriteLine($"Error: Win64 BqLog library not found: {LibPath}");
			}
		}
	}

	private void LinkMacStaticLib(string BqLogPath, string BqLogConfig)
	{
		// Mac平台优先使用Framework，回退到静态库
		string FrameworkPath = Path.Combine(BqLogPath, "static_lib", "mac", BqLogConfig, "BqLog.framework");
		string StaticLibPath = Path.Combine(BqLogPath, "static_lib", "mac", BqLogConfig, "libBqLog.a");

		if (System.IO.Directory.Exists(FrameworkPath))
		{
			PublicFrameworks.Add("BqLog");
			PublicAdditionalFrameworks.Add(new Framework("BqLog", FrameworkPath, ""));
			System.Console.WriteLine($"LogEverything: Using Mac BqLog Framework ({BqLogConfig}): {FrameworkPath}");
		}
		else if (System.IO.File.Exists(StaticLibPath))
		{
			PublicAdditionalLibraries.Add(StaticLibPath);
			System.Console.WriteLine($"LogEverything: Using Mac BqLog static library ({BqLogConfig}): {StaticLibPath}");
		}
		else
		{
			// 回退到Release版本
			string FallbackStaticPath = Path.Combine(BqLogPath, "static_lib", "mac", "Release", "libBqLog.a");
			if (System.IO.File.Exists(FallbackStaticPath))
			{
				PublicAdditionalLibraries.Add(FallbackStaticPath);
				System.Console.WriteLine($"LogEverything: Using Mac BqLog static library (fallback to Release): {FallbackStaticPath}");
			}
			else
			{
				System.Console.WriteLine($"Error: Mac BqLog library not found: {StaticLibPath}");
			}
		}
	}

	private void LinkLinux64StaticLib(string BqLogPath, string BqLogConfig)
	{
		string LibPath = Path.Combine(BqLogPath, "static_lib", "linux64", BqLogConfig, "libBqLog.a");

		if (System.IO.File.Exists(LibPath))
		{
			PublicAdditionalLibraries.Add(LibPath);
			System.Console.WriteLine($"LogEverything: Using Linux64 BqLog library ({BqLogConfig}): {LibPath}");
		}
		else
		{
			// 回退到Release版本
			string FallbackPath = Path.Combine(BqLogPath, "static_lib", "linux64", "Release", "libBqLog.a");
			if (System.IO.File.Exists(FallbackPath))
			{
				PublicAdditionalLibraries.Add(FallbackPath);
				System.Console.WriteLine($"LogEverything: Using Linux64 BqLog library (fallback to Release): {FallbackPath}");
			}
			else
			{
				System.Console.WriteLine($"Error: Linux64 BqLog library not found: {LibPath}");
			}
		}
	}

	private void LinkAndroidDynamicLib(string BqLogPath, string BqLogConfig, ReadOnlyTargetRules Target)
	{
		// Android架构映射
		string GetAndroidArch(string UEArch)
		{
			if (UEArch == "Arm64")
				return "arm64-v8a";
			else if (UEArch == "Armv7")
				return "armeabi-v7a";
			else if (UEArch == "x64")
				return "x86_64";
			else
				return UEArch.ToLower();
		}

		string AndroidArch = GetAndroidArch(Target.Architecture.ToString());

		// Android动态库配置映射（只有Debug和MinSizeRel）
		string AndroidConfig = (BqLogConfig == "Debug") ? "Debug" : "MinSizeRel";

		string LibPath = Path.Combine(BqLogPath, "dynamic_lib", "android", AndroidArch, AndroidConfig, "libBqLog.so");

		if (System.IO.File.Exists(LibPath))
		{
			PublicAdditionalLibraries.Add(LibPath);
			System.Console.WriteLine($"LogEverything: Using Android BqLog library ({AndroidArch}, {AndroidConfig}): {LibPath}");
		}
		else
		{
			// 回退到MinSizeRel版本
			string FallbackPath = Path.Combine(BqLogPath, "dynamic_lib", "android", AndroidArch, "MinSizeRel", "libBqLog.so");
			if (System.IO.File.Exists(FallbackPath))
			{
				PublicAdditionalLibraries.Add(FallbackPath);
				System.Console.WriteLine($"LogEverything: Using Android BqLog library ({AndroidArch}, fallback to MinSizeRel): {FallbackPath}");
			}
			else
			{
				System.Console.WriteLine($"Error: Android BqLog library not found for architecture {AndroidArch}: {LibPath}");
			}
		}
	}

	private void LinkiOSFramework(string BqLogPath, string BqLogConfig)
	{
		string FrameworkPath = Path.Combine(BqLogPath, "dynamic_lib", "ios", BqLogConfig, "BqLog.framework");

		if (System.IO.Directory.Exists(FrameworkPath))
		{
			PublicFrameworks.Add("BqLog");
			PublicAdditionalFrameworks.Add(new Framework("BqLog", FrameworkPath, ""));
			System.Console.WriteLine($"LogEverything: Using iOS BqLog Framework ({BqLogConfig}): {FrameworkPath}");
		}
		else
		{
			// 回退到Release版本
			string FallbackPath = Path.Combine(BqLogPath, "dynamic_lib", "ios", "Release", "BqLog.framework");
			if (System.IO.Directory.Exists(FallbackPath))
			{
				PublicFrameworks.Add("BqLog");
				PublicAdditionalFrameworks.Add(new Framework("BqLog", FallbackPath, ""));
				System.Console.WriteLine($"LogEverything: Using iOS BqLog Framework (fallback to Release): {FallbackPath}");
			}
			else
			{
				System.Console.WriteLine($"Error: iOS BqLog Framework not found: {FrameworkPath}");
			}
		}
	}

	private void AddPlatformDefinitions(UnrealTargetPlatform Platform)
	{
		if (Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.Add("BQ_WIN=1");
		}
		else if (Platform == UnrealTargetPlatform.Mac)
		{
			PublicDefinitions.Add("BQ_MAC=1");
			PublicDefinitions.Add("BQ_APPLE=1");
		}
		else if (Platform == UnrealTargetPlatform.Linux)
		{
			PublicDefinitions.Add("BQ_LINUX=1");
		}
		else if (Platform == UnrealTargetPlatform.Android)
		{
			PublicDefinitions.Add("BQ_ANDROID=1");
		}
		else if (Platform == UnrealTargetPlatform.IOS)
		{
			PublicDefinitions.Add("BQ_IOS=1");
			PublicDefinitions.Add("BQ_APPLE=1");
		}
	}
}