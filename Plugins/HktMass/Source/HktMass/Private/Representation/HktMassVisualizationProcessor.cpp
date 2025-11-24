// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassVisualizationProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassExecutionContext.h"
#include "MassSimulationLOD.h"

UHktMassVisualizationProcessor::UHktMassVisualizationProcessor()
	: UMassVisualizationProcessor()
{
    // ⭐ 핵심: 자동 등록 활성화!
    bAutoRegisterWithProcessingPhases = false;
   
    // Client와 Standalone에서만 실행
    ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
   
    // LOD 계산 후에 실행
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::LOD);
}

void UHktMassVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager);
}

void UHktMassVisualizationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	Super::Execute(EntityManager, Context);
}

