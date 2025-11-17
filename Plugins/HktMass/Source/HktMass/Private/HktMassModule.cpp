// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassModule.h"

#define LOCTEXT_NAMESPACE "FHktMassModule"

void FHktMassModule::StartupModule()
{
	// 모듈 ?�작 ??초기??코드
	UE_LOG(LogTemp, Log, TEXT("HktMass Plugin: Module Started"));
}

void FHktMassModule::ShutdownModule()
{
	// 모듈 종료 ???�리 코드
	UE_LOG(LogTemp, Log, TEXT("HktMass Plugin: Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHktMassModule, HktMass)

