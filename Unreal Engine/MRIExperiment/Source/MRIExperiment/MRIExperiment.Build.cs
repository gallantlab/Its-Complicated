using UnrealBuildTool;

public class MRIExperiment : ModuleRules
{
	public MRIExperiment(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"Slate",
				"SlateCore",
				"RenderCore",
				"RHI",
				"ImageWrapper",
				"NullNetworkReplayStreaming"
			}
		);
	}
}
