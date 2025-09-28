// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)] // 서버 계열 플랫폼에서만 빌드되도록 제한
public class HktProtoServerTarget : TargetRules
{
    public HktProtoServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Program;
        LaunchModuleName = "HktProtoServer";
        LinkType = TargetLinkType.Monolithic;

        // UE 5.1
        // 이상 버전과 호환성을 위한 설정입니다.
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
        CppStandard = CppStandardVersion.Cpp20;

        // Lean and mean
        bBuildDeveloperTools = false;

        // Editor-only is enabled for desktop platforms to run unit tests that depend on editor-only data
        // It's disabled in test and shipping configs to make profiling similar to the game
        bool bDebugOrDevelopment = Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.Development;
        bBuildWithEditorOnlyData = Target.Platform.IsInGroup(UnrealPlatformGroup.Desktop) && bDebugOrDevelopment;

        // Currently this app is not linking against the engine, so we'll compile out references from Core to the rest of the engine
        bCompileAgainstEngine = false;
        bCompileAgainstCoreUObject = false;
        bCompileAgainstApplicationCore = false;
        bCompileICU = false;

        // to build with automation tests:
        // bForceCompileDevelopmentAutomationTests = true;

        // to enable tracing:
        // bEnableTrace = true;

        // This app is a console application, not a Windows app (sets entry point to main(), instead of WinMain())
        bIsBuildingConsoleApplication = true;
    }
}