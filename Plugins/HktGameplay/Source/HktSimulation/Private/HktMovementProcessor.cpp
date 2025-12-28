// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktMovementProcessor.h"
#include "HktAbilityTypes.h"
#include "HktSimulationTypes.h"
#include "HktFrameSyncSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"

UHktMovementProcessor::UHktMovementProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Tasks);
	bAutoRegisterWithProcessingPhases = true;
}

void UHktMovementProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void UHktMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	MovementQuery.AddRequirement<FHktMovementFragment>(EMassFragmentAccess::ReadWrite);
	MovementQuery.AddRequirement<FHktVisualFragment>(EMassFragmentAccess::ReadWrite);
	MovementQuery.AddRequirement<FHktCombatFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = GetFixedDeltaTime();

	MovementQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FHktMovementFragment> MovementList = Context.GetMutableFragmentView<FHktMovementFragment>();
		TArrayView<FHktVisualFragment> VisualList = Context.GetMutableFragmentView<FHktVisualFragment>();
		TConstArrayView<FHktCombatFragment> CombatList = Context.GetFragmentView<FHktCombatFragment>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			FTransformFragment& Transform = TransformList[i];
			FHktMovementFragment& Movement = MovementList[i];
			FHktVisualFragment& Visual = VisualList[i];
			const FHktCombatFragment& Combat = CombatList[i];

			// Don't move if in combat action or dead
			if (Combat.IsInCombatAction() || Combat.CombatState == EHktCombatState::Dead)
			{
				Movement.bIsMoving = false;
				Movement.Velocity = FVector::ZeroVector;
				continue;
			}

			if (!Movement.bIsMoving)
			{
				Movement.Velocity = FVector::ZeroVector;
				Visual.AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Idle"));
				continue;
			}

			// Calculate direction and distance to target
			FVector CurrentPos = Transform.GetTransform().GetLocation();
			FVector ToTarget = Movement.TargetPosition - CurrentPos;
			ToTarget.Z = 0; // Keep movement horizontal
			float Distance = ToTarget.Size();

			// Check if arrived
			if (Distance <= ArrivalThreshold)
			{
				Movement.bIsMoving = false;
				Movement.bHasReachedTarget = true;
				Movement.Velocity = FVector::ZeroVector;
				Visual.AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Idle"));
				continue;
			}

			// Calculate velocity
			FVector Direction = ToTarget.GetSafeNormal();
			Movement.Velocity = Direction * Movement.MoveSpeed;

			// Apply movement
			FVector NewPos = CurrentPos + Movement.Velocity * DeltaTime;
			
			// Update transform
			FTransform NewTransform = Transform.GetTransform();
			NewTransform.SetLocation(NewPos);
			
			// Face movement direction
			if (!Direction.IsNearlyZero())
			{
				FRotator NewRotation = Direction.Rotation();
				NewRotation.Pitch = 0;
				NewRotation.Roll = 0;
				NewTransform.SetRotation(NewRotation.Quaternion());
			}
			
			Transform.SetTransform(NewTransform);

			// Update visual fragment
			Visual.Transform = NewTransform;
			Visual.Velocity = Movement.Velocity;
			Visual.AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Moving"));
		}
	});
}

float UHktMovementProcessor::GetFixedDeltaTime() const
{
	if (UHktFrameSyncSubsystem* FrameSync = GetFrameSyncSubsystem())
	{
		return FrameSync->GetFixedDeltaTime();
	}
	return 1.0f / 60.0f;
}

UHktFrameSyncSubsystem* UHktMovementProcessor::GetFrameSyncSubsystem() const
{
	if (CachedFrameSync.IsValid())
	{
		return CachedFrameSync.Get();
	}

	UWorld* World = GetWorld();
	if (World)
	{
		CachedFrameSync = World->GetSubsystem<UHktFrameSyncSubsystem>();
		return CachedFrameSync.Get();
	}
	return nullptr;
}

