// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ISReverb : ModuleRules
{
	public ISReverb(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDefinitions.Add("RHI_RAYTRACING=1");
		PublicDefinitions.Add("RHI_RAYTRACING_ALLOWED=1");
		
		PublicIncludePaths.AddRange(
			new string[] {
				"ISReverb/Public",
				"../Shaders/Shared"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"ISReverb/Private",
				Path.Combine(GetModuleDirectory("Renderer"), "Internal"),
				Path.Combine(GetModuleDirectory("Renderer"), "Private"),
				Path.Combine(GetModuleDirectory("RenderCore"), "Public")
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"RenderCore",
				"Renderer",
				"RHI",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects",
				"RHI",
				"Renderer"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
