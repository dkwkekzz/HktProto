// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassApplyTransformProcessor.h"
#include "HktMassPhysicsFragments.h"
#include "HktMassDefines.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassSimulationLOD.h"

UHktMassApplyTransformProcessor::UHktMassApplyTransformProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = HktMass::ExecuteGroupNames::Physics_ApplyTransform;
	ExecutionOrder.ExecuteAfter.Add(HktMass::ExecuteGroupNames::Physics_ApplyVelocity);
}

void UHktMassApplyTransformProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHktMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddRequirement<FMassSimulationVariableTickFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UHktMassApplyTransformProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float WorldDeltaTime = Context.GetDeltaTimeSeconds();

	EntityQuery.ForEachEntityChunk(Context, [WorldDeltaTime](FMassExecutionContext& LoopContext)
	{
		const int32 NumEntities = LoopContext.GetNumEntities();
		const auto& Velocities = LoopContext.GetFragmentView<FHktMassVelocityFragment>();
		const auto& Transforms = LoopContext.GetMutableFragmentView<FTransformFragment>();
        
        const auto& SimVariableTickList = LoopContext.GetFragmentView<FMassSimulationVariableTickFragment>();
		const bool bHasVariableTick = (SimVariableTickList.Num() > 0);

		for (int32 i = 0; i < NumEntities; ++i)
		{
            const float DeltaTime = bHasVariableTick ? SimVariableTickList[i].DeltaTime : WorldDeltaTime;
			const FVector Velocity = Velocities[i].Value;
            
            if (!Velocity.IsNearlyZero())
            {
			    FTransform& Transform = Transforms[i].GetMutableTransform();
                
                // 위치 업데이트
                Transform.AddToTranslation(Velocity * DeltaTime);
                
                // 회전 업데이트 (이동 방향 바라보기)
                const FQuat NewRotation = Velocity.ToOrientationQuat();
                Transform.SetRotation(NewRotation);
            }
		}
	});
}
