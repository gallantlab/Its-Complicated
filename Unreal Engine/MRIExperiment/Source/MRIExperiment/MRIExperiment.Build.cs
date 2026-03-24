using UnrealBuildTool;

public class MRIExperiment : ModuleRules
{
	public MRIExperiment(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"RenderCore",
				"RHI",
				"Json"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"UMG",
				"MoviePlayer",
				"Landscape"
			}
		);
	}
}
