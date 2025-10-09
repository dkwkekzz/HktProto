// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktClientTests : ModuleRules
{
    public HktClientTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "HktClient",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}



