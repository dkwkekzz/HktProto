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
                "UMG",
                "HktBase",
                "HktCustomNet"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "HktCustomNet"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}



