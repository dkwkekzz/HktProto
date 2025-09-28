// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HktProtoServer : ModuleRules
{
	public HktProtoServer(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicIncludePathModuleNames.Add("Launch");
        PrivateDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Projects", "HktGrpc" });
    }
}

