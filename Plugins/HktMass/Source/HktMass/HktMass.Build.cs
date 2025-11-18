// Copyright Hkt Studios, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktMass : ModuleRules
{
	public HktMass(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// 공용 인클루드 경로
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// 비공개 인클루드 경로
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"NetCore",
				// Mass Entity 관련 모듈
				"MassEntity",
				"MassCommon",
				"MassMovement",
				"MassSpawner",
				"MassActors",
				"MassRepresentation",
				"MassLOD",
				"MassSimulation",
				"MassReplication",
				"StructUtils",
				"ZoneGraph",
				"AnimToTexture",
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// 비공개 의존성 모듈
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// 동적으로 로드되는 모듈
			}
		);
	}
}


