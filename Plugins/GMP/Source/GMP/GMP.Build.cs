//  Copyright GenericMessagePlugin, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class GMP : ModuleRules
{
	public GMP(ReadOnlyTargetRules Target)
		: base(Target)
	{
		PublicIncludePaths.AddRange(new string[] {
			ModuleDirectory,
			ModuleDirectory + "/Shared",
			ModuleDirectory + "/ThirdParty",
			// ... add public include paths required here ...
		});

		PrivateIncludePaths.AddRange(new string[] {
			ModuleDirectory + "/Private",
			ModuleDirectory + "/GMP",
			// ... add other private include paths required here ...
		});

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",  // UBlueprintFunctionLibrary
					   // "GenericStorages",
		});

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
			PrivateDependencyModuleNames.Add("BlueprintGraph");
		}
		PrivateDefinitions.Add("SUPPRESS_MONOLITHIC_HEADER_WARNINGS=1");
		PublicDefinitions.Add("UPB_PUBLIC_API=GMP_API");
		if (Target.Type == TargetType.Editor) 
			PrivateDefinitions.Add("UPB_EXPORT_API=WITH_EDITOR");
		PrivateDefinitions.Add("UPB_BUILD_API=1");

		if (Target.Configuration == UnrealTargetConfiguration.DebugGame || Target.Configuration == UnrealTargetConfiguration.Debug)
		{
			// Tools.DotNETCommon.Log.TraceInformation("GMP_DEBUGGAME=1");
			PublicDefinitions.Add("GMP_DEBUGGAME=1");
			if (Target.Type == TargetType.Editor)
				PublicDefinitions.Add("GMP_DEBUGGAME_EDITOR=1");
			else
				PublicDefinitions.Add("GMP_DEBUGGAME_EDITOR=0");
		}
		else
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
			SharedPCHHeaderFile = ModuleDirectory + "/Shared/GMPCore.h";

			PublicDefinitions.Add("GMP_DEBUGGAME=0");
			PublicDefinitions.Add("GMP_DEBUGGAME_EDITOR=0");
		}
		DynamicallyLoadedModuleNames.AddRange(new string[] {
			// ... add any modules that your module loads dynamically here ...
		});

		BuildVersion Version;
		if (BuildVersion.TryRead(BuildVersion.GetDefaultFileName(), out Version))
		{
			if (Version.MajorVersion > 4 || (Version.MajorVersion == 4 && Version.MinorVersion > 23))
			{
				PublicDependencyModuleNames.Add("NetCore");
			}
			bool bUE_USE_FPROPERTY = (Version.MajorVersion > 4 || (Version.MajorVersion == 4 && Version.MinorVersion >= 25));
			string IncFile = Path.Combine(ModuleDirectory, "GMP/PropertyCompatibility.include");
			if (bUE_USE_FPROPERTY)
			{
				PublicDefinitions.Add("UE_USE_UPROPERTY=0");
				File.Delete(IncFile);
			}
			else
			{
				PublicDefinitions.Add("UE_USE_UPROPERTY=1");
				if (!File.Exists(IncFile))
					File.Copy(Path.Combine(ModuleDirectory, "..", "ThirdParty/PropertyCompatibility.include"), IncFile);
			}
		}
		bool bEnableScriptExtensions = true;
		if (bEnableScriptExtensions)
		{
			PublicDefinitions.Add("GMP_EXTEND_CONSOLE=1");
			PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Source/Runtime/Online/HTTP/Public"));
			if (Target.Platform.IsInGroup(UnrealPlatformGroup.Desktop) || Target.Configuration != UnrealTargetConfiguration.Shipping)
			{
				PrivateDependencyModuleNames.AddRange(new string[] {
					"HTTPServer",
				});
				PrivateDefinitions.Add("GMP_HTTPSERVER=1");
			}

			if (Target.bBuildEditor)
			{
				PrivateDependencyModuleNames.AddRange(new string[] {
					"PythonScriptPlugin",
				});
			}
		}
	}
}
