// Copyright Hkt Studios, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktFlow : ModuleRules
{
	public HktBehavior(ReadOnlyTargetRules Target) : base(Target)
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
			
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"HktService",
				"HktIntent"
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}

