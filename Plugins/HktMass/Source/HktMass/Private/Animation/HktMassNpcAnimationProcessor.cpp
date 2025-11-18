// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMassNpcAnimationProcessor.h"
#include "HktMassNpcAnimationTypes.h"
#include "HktMassCommonFragments.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassExecutionContext.h"
#include "MassRepresentationFragments.h"
#include "AnimToTextureDataAsset.h"

// ===================================
// UHktMassNpcAnimationFragmentInitializer
// ===================================

UHktMassNpcAnimationFragmentInitializer::UHktMassNpcAnimationFragmentInitializer()
	: EntityQuery(*this)
{
	ObservedType = FHktMassNpcAnimationFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
}

void UHktMassNpcAnimationFragmentInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHktMassNpcAnimationFragment>(EMassFragmentAccess::ReadWrite);
}

void UHktMassNpcAnimationFragmentInitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);
	World = Owner.GetWorld();
}

void UHktMassNpcAnimationFragmentInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& ExecutionContext)
{
	check(World);
	const float GlobalTime = World->GetTimeSeconds();
	
	EntityQuery.ForEachEntityChunk(ExecutionContext, [GlobalTime](FMassExecutionContext& Context)
	{
		TArrayView<FHktMassNpcAnimationFragment> AnimationDataList = Context.GetMutableFragmentView<FHktMassNpcAnimationFragment>();
		
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FHktMassNpcAnimationFragment& AnimationData = AnimationDataList[EntityIt];
			
			// Initialize with random time offset for variety
			const float StartTimeOffset = FMath::FRandRange(0.0f, 5.0f);
			AnimationData.GlobalStartTime = GlobalTime - StartTimeOffset;
			AnimationData.PlayRate = 1.0f;
			AnimationData.AnimationStateIndex = 0; // Start with idle animation
			AnimationData.bSwappedThisFrame = false;
		}
	});
}

// ===================================
// UHktMassNpcAnimationProcessor
// ===================================

UHktMassNpcAnimationProcessor::UHktMassNpcAnimationProcessor()
	: AnimationEntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::SyncWorldToMass);
}

void UHktMassNpcAnimationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AnimationEntityQuery.AddRequirement<FHktMassNpcAnimationFragment>(EMassFragmentAccess::ReadWrite);
	AnimationEntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	AnimationEntityQuery.AddRequirement<FHktNpcStateFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	AnimationEntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
}

void UHktMassNpcAnimationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);
	World = Owner.GetWorld();
	check(World);
}

void UHktMassNpcAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	check(World);
	
	const float GlobalTime = World->GetTimeSeconds();
	
	AnimationEntityQuery.ForEachEntityChunk(Context, [this, GlobalTime](FMassExecutionContext& Context)
	{
		TArrayView<FHktMassNpcAnimationFragment> AnimationDataList = Context.GetMutableFragmentView<FHktMassNpcAnimationFragment>();
		TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
		
		// Optional fragments - just get the view and check IsEmpty()
		TConstArrayView<FHktNpcStateFragment> StateList = Context.GetFragmentView<FHktNpcStateFragment>();
		TConstArrayView<FMassRepresentationFragment> RepresentationList = Context.GetFragmentView<FMassRepresentationFragment>();
		
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FHktMassNpcAnimationFragment& AnimationData = AnimationDataList[EntityIt];
			const FMassVelocityFragment& Velocity = VelocityList[EntityIt];
			
			// Skip if not visible (optional optimization)
			if (!RepresentationList.IsEmpty())
			{
				const FMassRepresentationFragment& Representation = RepresentationList[EntityIt];
				if (Representation.CurrentRepresentation == EMassRepresentationType::None)
				{
					continue;
				}
			}
			
			// Validate AnimToTexture data
			const UAnimToTextureDataAsset* AnimToTextureData = AnimationData.AnimToTextureData.Get();
			if (!AnimToTextureData || AnimToTextureData->Animations.Num() == 0)
			{
				continue;
			}
			
			// Determine animation state based on velocity and AI state
			int32 NewStateIndex = 0; // Default: Idle
			
			const float VelocitySizeSq = Velocity.Value.SizeSquared();
			//const bool bIsMoving = VelocitySizeSq > MoveThresholdSq;
			const bool bIsMoving = true;
			
			if (bIsMoving)
			{
				// State 1: Walking/Running
				NewStateIndex = FMath::Min(1, AnimToTextureData->Animations.Num() - 1);
				
				// Adjust playback rate based on velocity
				const float AuthoredAnimSpeed = 200.0f; // Base speed the animation was authored at
				const float PrevPlayRate = AnimationData.PlayRate;
				AnimationData.PlayRate = FMath::Clamp(
					FMath::Sqrt(VelocitySizeSq / (AuthoredAnimSpeed * AuthoredAnimSpeed)), 
					0.5f, 2.0f
				);
				
				// Adjust global start time to maintain current animation frame when playrate changes
				// Formula: (GlobalTime - Offset1) * PlayRate1 == (GlobalTime - Offset2) * PlayRate2
				if (FMath::Abs(PrevPlayRate - AnimationData.PlayRate) > 0.01f)
				{
					AnimationData.GlobalStartTime = GlobalTime - 
						PrevPlayRate * (GlobalTime - AnimationData.GlobalStartTime) / AnimationData.PlayRate;
				}
			}
			else
			{
				// State 0: Idle
				NewStateIndex = 0;
				AnimationData.PlayRate = 1.0f;
			}
			
			// Check AI state for special animations (if available)
			if (!StateList.IsEmpty() && AnimToTextureData->Animations.Num() > 2)
			{
				const FHktNpcStateFragment& State = StateList[EntityIt];
				
				switch (State.CurrentState)
				{
					case 3: // Attack state
						NewStateIndex = FMath::Min(2, AnimToTextureData->Animations.Num() - 1);
						AnimationData.PlayRate = 1.0f;
						break;
					case 4: // Dead state
						NewStateIndex = FMath::Min(3, AnimToTextureData->Animations.Num() - 1);
						AnimationData.PlayRate = 1.0f;
						break;
					default:
						break;
				}
			}
			
			// Update animation state index
			if (AnimationData.AnimationStateIndex != NewStateIndex)
			{
				AnimationData.AnimationStateIndex = NewStateIndex;
				AnimationData.GlobalStartTime = GlobalTime; // Reset timing on state change
			}

			// Reset swapped flag
			AnimationData.bSwappedThisFrame = false;
		}
	});
}

