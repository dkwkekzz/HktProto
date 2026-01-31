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
            "EnhancedInput", // Enhanced Input 시스템 사용 추가
            "NavigationSystem", 
            "AIModule", 
            "Niagara",
            "UMG",
            "GameplayTags",
            "GameplayTasks",
            // HktMass 플러그인 (Mass Entity 시스템)
            "HktMass",
            "HktRuntime"
        });
    }
}
