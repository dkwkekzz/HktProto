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
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "HktCustomNet",
            }
        );
    }
}



