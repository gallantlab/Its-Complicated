using System.IO;
using UnrealBuildTool;

public class Experiment : ModuleRules
{
	public Experiment(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				ModuleDirectory,
				Path.Combine(ModuleDirectory, "Agent"),
				Path.Combine(ModuleDirectory, "Controller"),
				Path.Combine(ModuleDirectory, "Demo"),
				Path.Combine(ModuleDirectory, "Logging"),
				Path.Combine(ModuleDirectory, "Camera"),
				Path.Combine(ModuleDirectory, "Settings"),
				Path.Combine(ModuleDirectory, "State")
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"NetworkReplayStreaming",
				"UMG"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EnhancedInput",
				"Slate",
				"SlateCore",
				"RenderCore",
				"RHI",
				"ImageWrapper",
				"Json",
				"NullNetworkReplayStreaming"
			}
		);
	}
}
