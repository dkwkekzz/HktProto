// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktProto : ModuleRules
{
	public HktProto(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "EnhancedInput", // Enhanced Input ����� ���� �߰�
            "NavigationSystem", 
            "AIModule", 
            "Niagara",
            "UMG",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            // MassEntity 모듈
            "MassEntity",
            "MassCommon",
            "MassMovement",
            "MassSpawner",
            "MassActors",
            "MassRepresentation",
            "MassLOD",
            "MassSimulation",
            "StructUtils",
            "ZoneGraph"
        });
    }
}
