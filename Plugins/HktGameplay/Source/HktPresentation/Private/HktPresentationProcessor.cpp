// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktPresentationProcessor.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassRepresentationFragments.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

//-----------------------------------------------------------------------------
// UHktPresentationProcessor
//-----------------------------------------------------------------------------

UHktPresentationProcessor::UHktPresentationProcessor()
{
	// Set execution order - presentation runs after simulation
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	
	bAutoRegisterWithProcessingPhases = true;
}

void UHktPresentationProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void UHktPresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// Query for entities with visual data
	// Note: We use a generic approach that doesn't require HktSimulation types
	VisualDataQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktPresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Update camera location for LOD calculations
	UWorld* World = GetWorld();
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (PC->PlayerCameraManager)
			{
				CachedCameraLocation = PC->PlayerCameraManager->GetCameraLocation();
			}
		}
	}

	// Update ISM transforms
	UpdateInstancedStaticMeshes(Context);

	// Apply animation states
	if (Config.bEnableAnimations)
	{
		ApplyAnimationStates(Context);
	}

	// Trigger VFX
	if (Config.bEnableVFX)
	{
		TriggerVisualEffects(Context);
	}
}

void UHktPresentationProcessor::UpdateInstancedStaticMeshes(FMassExecutionContext& Context)
{
	if (!Config.bUseInstancedRendering)
	{
		return;
	}

	VisualDataQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; ++EntityIdx)
		{
			const FTransformFragment& TransformFrag = TransformList[EntityIdx];
			const FTransform& EntityTransform = TransformFrag.GetTransform();
			
			// Calculate LOD level
			const uint8 LODLevel = CalculateLODLevel(EntityTransform.GetLocation());
			
			// ISM updates are handled by MassRepresentation system
			// This processor prepares the data; actual ISM updates happen in MassRepresentation processors
			
			// For direct ISM manipulation, you would access the ISM component here
			// and update instance transforms. The MassRepresentation plugin handles this
			// automatically for entities configured with ISM visualization.
		}
	});
}

void UHktPresentationProcessor::ApplyAnimationStates(FMassExecutionContext& Context)
{
	// Animation state application
	// In a full implementation, this would:
	// 1. Read animation state from visual fragments
	// 2. Update skeletal mesh animation blend weights
	// 3. Trigger animation transitions
	
	VisualDataQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; ++EntityIdx)
		{
			// Animation logic would go here
			// For ISM-based animations (AnimToTexture), update texture offsets
			// For actor-based animations, update animation blueprint parameters
		}
	});
}

void UHktPresentationProcessor::TriggerVisualEffects(FMassExecutionContext& Context)
{
	// VFX triggering logic
	// In a full implementation, this would:
	// 1. Read VFX tags from visual fragments
	// 2. Spawn/update Niagara systems
	// 3. Manage VFX lifecycle
	
	// Cleanup expired VFX
	TArray<FMassEntityHandle> ExpiredHandles;
	for (auto& Pair : ActiveVFXComponents)
	{
		if (!Pair.Value.IsValid() || !Pair.Value->IsActive())
		{
			ExpiredHandles.Add(Pair.Key);
		}
	}
	for (const FMassEntityHandle& Handle : ExpiredHandles)
	{
		ActiveVFXComponents.Remove(Handle);
	}
}

uint8 UHktPresentationProcessor::CalculateLODLevel(const FVector& EntityLocation) const
{
	const float DistanceSquared = FVector::DistSquared(EntityLocation, CachedCameraLocation);
	
	for (int32 i = 0; i < Config.LODDistances.Num(); ++i)
	{
		const float ThresholdSquared = FMath::Square(Config.LODDistances[i]);
		if (DistanceSquared < ThresholdSquared)
		{
			return static_cast<uint8>(i);
		}
	}
	
	return static_cast<uint8>(Config.LODDistances.Num());
}

//-----------------------------------------------------------------------------
// UHktISMUpdateProcessor
//-----------------------------------------------------------------------------

UHktISMUpdateProcessor::UHktISMUpdateProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
	ExecutionOrder.ExecuteAfter.Add(TEXT("UHktPresentationProcessor"));
	
	bAutoRegisterWithProcessingPhases = true;
}

void UHktISMUpdateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ISMQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	// Would also add ISM-specific fragment requirements
}

void UHktISMUpdateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// ISM batch update logic
	// This would batch transform updates to ISM components for efficiency
	
	ISMQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();

		// Batch update ISM instances
		// In practice, this aggregates transforms and does bulk ISM updates
		for (int32 EntityIdx = 0; EntityIdx < NumEntities; ++EntityIdx)
		{
			const FTransformFragment& TransformFrag = TransformList[EntityIdx];
			// ISM update logic
		}
	});
}

//-----------------------------------------------------------------------------
// UHktAnimationUpdateProcessor
//-----------------------------------------------------------------------------

UHktAnimationUpdateProcessor::UHktAnimationUpdateProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Representation;
	ExecutionOrder.ExecuteAfter.Add(TEXT("UHktPresentationProcessor"));
	
	bAutoRegisterWithProcessingPhases = true;
}

void UHktAnimationUpdateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AnimationQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	// Would also add animation-specific fragment requirements
}

void UHktAnimationUpdateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Animation update logic
	// This updates skeletal mesh animations or AnimToTexture parameters
	
	AnimationQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; ++EntityIdx)
		{
			const FTransformFragment& TransformFrag = TransformList[EntityIdx];
			// Animation update logic
			// - Update animation blend weights
			// - Advance animation timers
			// - Apply bone transforms
		}
	});
}

