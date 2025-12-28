// Copyright Hkt Studios, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktPresentation : ModuleRules
{
	public HktPresentation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
		);
				
		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);

		// IMPORTANT: HktPresentation has NO dependency on HktIntent or HktSimulation
		// It only reads data from MassEntity fragments (injected by HktSimulation)
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"MassEntity",
				"MassCommon",
				"MassRepresentation",
				"MassActors",
				"GameplayTags",
				"Niagara"  // For VFX
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}

