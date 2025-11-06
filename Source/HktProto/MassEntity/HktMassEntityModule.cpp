// Copyright Epic Games, Inc. All Rights Reserved.

#include "HktMassEntityModule.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"

void FHktMassEntityModule::Initialize()
{
	// 모듈 초기화 시 필요한 설정
	UE_LOG(LogTemp, Log, TEXT("HktMassEntity Module Initialized"));
}

void FHktMassEntityModule::Shutdown()
{
	// 모듈 종료 시 정리 작업
	UE_LOG(LogTemp, Log, TEXT("HktMassEntity Module Shutdown"));
}

bool FHktMassEntityModule::IsMassEntitySystemValid(UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is null"));
		return false;
	}

	UMassEntitySubsystem* MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (!MassEntitySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem is not available. Make sure MassEntity plugin is enabled."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Mass Entity System is valid"));
	return true;
}

