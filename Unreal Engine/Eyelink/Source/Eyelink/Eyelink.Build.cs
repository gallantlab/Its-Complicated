// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Eyelink : ModuleRules
{
	public Eyelink(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"Eyelink/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"Eyelink/Private",
				// ... add other private include paths required here ...
			}
			);

		// Eyelink
		if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
		{
			PublicIncludePaths.Add("D:\\Program Files (x86)\\SR Research\\EyeLink\\Includes\\eyelink");

			string[] DllFiles = Directory.GetFiles("D:\\Program Files (x86)\\SR Research\\EyeLink\\libs\\x64", "*.dll");
			foreach (string Dll in DllFiles)
			{
				RuntimeDependencies.Add(new RuntimeDependency(Dll));
			}
			string[] LibraryFiles = Directory.GetFiles("D:\\Program Files (x86)\\SR Research\\EyeLink\\libs\\x64", "*.lib");
			PublicAdditionalLibraries.AddRange(LibraryFiles);
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicIncludePaths.Add("/usr/include/EyeLink/");

			RuntimeDependencies.Add(new RuntimeDependency("/usr/lib/x86_64-linux-gnu/libeyelink_core.so"));
			PublicAdditionalLibraries.Add("/usr/lib/x86_64-linux-gnu/libeyelink_core.so");
		}


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UMG",
				"Slate",
				"SlateCore",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG"
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
