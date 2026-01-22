// Copyright Hkt Studios, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktSimulation : ModuleRules
{
	public HktSimulation(ReadOnlyTargetRules Target) : base(Target)
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
				"NetCore",  // StashComponent 네트워크 복제 지원
				"MassEntity",
				"MassCommon",
				"MassSpawner",
				"GameplayTags",
				"StructUtils",
				"HktService",  // Access to service providers (includes HktIntentInterface)
				"HktFlow"  // Access to job types and provider
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

