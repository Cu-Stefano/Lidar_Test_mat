// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PoseDetectionPlugin : ModuleRules
{
	public PoseDetectionPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
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
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"AugmentedReality"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				//"CoreUObject",
				//"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
   
		if (Target.Platform == UnrealTargetPlatform.IOS || Target.Platform == UnrealTargetPlatform.Mac)
		{
			//string ExternalPath = Path.Combine(ModuleDirectory, "External", "MLKitPoseDetection");
			//string FrameworkPath = Path.Combine(ExternalPath, "MLKitPoseDetection.framework");

			// PublicAdditionalFrameworks.Add(new Framework(
			// 	"MLKitPoseDetection",
			// 	FrameworkPath
			// ));

			PublicFrameworks.AddRange(
        new string[] {
            "Vision",
            "CoreVideo",
            "CoreMedia"
        }
    	);

			// PublicFrameworks.AddRange(new string[]
			// {
			// 	"UIKit",
			// 	"Foundation",
			// 	"AVFoundation",
			// 	"Vision"
			// });

			//string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			//	AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "PoseDetectionPlugin_IOS_UPL.xml"));
		}
	}
}
