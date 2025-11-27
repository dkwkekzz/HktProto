// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassSquadDebugVisualizationProcessor.h"
#include "HktMassSquadFragments.h"
#include "HktMassDefines.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "DrawDebugHelpers.h"

UHktMassSquadDebugVisualizationProcessor::UHktMassSquadDebugVisualizationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = HktMass::ExecuteGroupNames::Physics_DebugVisualization;
    
    // Execute after forces are calculated
    ExecutionOrder.ExecuteAfter.Add(HktMass::ExecuteGroupNames::Physics_ApplyVelocity);
    ExecutionOrder.ExecuteAfter.Add(HktMass::ExecuteGroupNames::Physics_ApplyTransform);
}

void UHktMassSquadDebugVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHktMassSquadFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FHktMassSquadDebugVisualizationTag>(EMassFragmentPresence::All);
}

void UHktMassSquadDebugVisualizationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Draw Velocity (Green)
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHktMassSquadFragment> SquadFragments = Context.GetFragmentView<FHktMassSquadFragment>();
        
        UWorld* World = Context.GetWorld();
        if (!World) return;

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FVector Location = Transforms[EntityIndex].GetTransform().GetLocation();
			DrawDebugSphere(World, Location, 100.0f, 10, FColor::Green, false, -1.0f, 0, 2.0f);
		}
	});
}

