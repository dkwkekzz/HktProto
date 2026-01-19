// Copyright Hkt Studios, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktIntent : ModuleRules
{
	public HktIntent(ReadOnlyTargetRules Target) : base(Target)
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
				"Engine",
				"NetCore",
				"EnhancedInput",
				"InputCore",
				"Niagara"
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"HktService",
				"HktAsset"
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}

