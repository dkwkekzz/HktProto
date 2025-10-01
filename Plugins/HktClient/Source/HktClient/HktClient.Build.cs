// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktClient : ModuleRules
{
    public HktClient(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "NavigationSystem",
                "AIModule",
                "Niagara",
                "EnhancedInput",
                "HktBase"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}



