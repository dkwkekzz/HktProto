// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassPhysicsDebugVisualizationProcessor.h"
#include "HktMassMovementFragments.h"
#include "HktMassPhysicsFragments.h"
#include "HktMassDefines.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "DrawDebugHelpers.h"

UHktMassPhysicsDebugVisualizationProcessor::UHktMassPhysicsDebugVisualizationProcessor()
	: VelocityQuery(*this)
    , ForceQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = HktMass::ExecuteGroupNames::Physics_DebugVisualization;
    
    // Execute after forces are calculated
    ExecutionOrder.ExecuteAfter.Add(HktMass::ExecuteGroupNames::Physics_ApplyVelocity);
    ExecutionOrder.ExecuteAfter.Add(HktMass::ExecuteGroupNames::Physics_ApplyTransform);
}

void UHktMassPhysicsDebugVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	VelocityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	VelocityQuery.AddRequirement<FHktMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	VelocityQuery.AddTagRequirement<FHktMassPhysicsDebugVisualizationTag>(EMassFragmentPresence::All);

	ForceQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	ForceQuery.AddRequirement<FHktMassForceFragment>(EMassFragmentAccess::ReadOnly);
	ForceQuery.AddTagRequirement<FHktMassPhysicsDebugVisualizationTag>(EMassFragmentPresence::All);
}

void UHktMassPhysicsDebugVisualizationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Draw Velocity (Green)
	VelocityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHktMassVelocityFragment> Velocities = Context.GetFragmentView<FHktMassVelocityFragment>();
        
        UWorld* World = Context.GetWorld();
        if (!World) return;

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FVector Location = Transforms[EntityIndex].GetTransform().GetLocation();
			const FVector Velocity = Velocities[EntityIndex].Value;

            if (!Velocity.IsNearlyZero())
            {
				const FVector EndLocation = Location + Velocity * 0.01f;
                DrawDebugDirectionalArrow(World, Location, EndLocation, 10.0f, FColor::Green, false, -1.0f, 0, 2.0f);
            }
		}
	});

    // Draw Force (Red)
	ForceQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHktMassForceFragment> Forces = Context.GetFragmentView<FHktMassForceFragment>();

        UWorld* World = Context.GetWorld();
        if (!World) return;

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FVector Location = Transforms[EntityIndex].GetTransform().GetLocation();
			const FVector Force = Forces[EntityIndex].Value;
            
            if (!Force.IsNearlyZero())
            {
                // Offset Force slightly in Z to separate from Velocity
                const FVector DrawLocation = Location + FVector(0.0f, 0.0f, 5.0f);
				const FVector DrawEndLocation = DrawLocation + Force * 0.01f;
			    DrawDebugDirectionalArrow(World, DrawLocation, DrawEndLocation, 10.0f, FColor::Red, false, -1.0f, 0, 2.0f);
            }
		}
	});
}

