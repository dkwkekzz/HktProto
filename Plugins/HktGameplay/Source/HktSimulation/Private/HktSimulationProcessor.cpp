// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktSimulationProcessor.h"
#include "HktIntentComponent.h"
#include "HktFrameSyncSubsystem.h"
#include "HktIntentTypes.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

#include "HktIntentTags.h"

// Helper to find intent component by player ID
static UHktIntentComponent* FindIntentComponentByPlayerId(const TArray<TWeakObjectPtr<UHktIntentComponent>>& Components, int32 PlayerId)
{
	for (const auto& WeakComp : Components)
	{
		if (UHktIntentComponent* Comp = WeakComp.Get())
		{
			if (APlayerController* PC = Comp->GetOwner<APlayerController>())
			{
				// Check PlayerState for ID
				if (PC->PlayerState && PC->PlayerState->GetPlayerId() == PlayerId)
				{
					return Comp;
				}
				// Fallback: Check if PC ID matches (for local/server mapping)
				// Note: PlayerId in Event might be from UNetConnection or PlayerState.
				// Assuming PlayerState.PlayerId for now.
			}
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
// UHktSimulationProcessor
//-----------------------------------------------------------------------------

UHktSimulationProcessor::UHktSimulationProcessor()
{
	// Set execution order - simulation runs before visualization
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Tasks);
	
	bAutoRegisterWithProcessingPhases = true;
}

void UHktSimulationProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void UHktSimulationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// Query for entities with visual fragments
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHktVisualFragment>(EMassFragmentAccess::ReadWrite);
	
	// Movement query
	MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	MovementQuery.AddRequirement<FHktVisualFragment>(EMassFragmentAccess::ReadWrite);
}

void UHktSimulationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Get current frame
	UHktFrameSyncSubsystem* FrameSync = GetFrameSyncSubsystem();
	if (!FrameSync)
	{
		return;
	}

	const uint32 CurrentFrame = FrameSync->GetServerFrame();
	const float DeltaTime = FrameSync->GetFixedDeltaTime();

	// Skip if we've already processed this frame
	if (CurrentFrame == CurrentSimFrame)
	{
		return;
	}
	CurrentSimFrame = CurrentFrame;

	// Refresh intent component cache periodically
	if (CachedIntentComponents.Num() == 0 || (CurrentFrame % 60) == 0)
	{
		RefreshIntentComponents();
	}

	// Gather all active events for this frame (READ-ONLY)
	TArray<FHktFrameEvent> ActiveEvents = GatherActiveEvents(CurrentFrame);

	// Process events by category
	ProcessMovementEvents(Context, ActiveEvents, DeltaTime);
	ProcessCombatEvents(Context, ActiveEvents, DeltaTime);

	// Update transforms based on current state
	UpdateEntityTransforms(Context, DeltaTime);

	// Generate visual fragments for presentation
	GenerateVisualFragments(Context);
}

TArray<FHktFrameEvent> UHktSimulationProcessor::GatherActiveEvents(uint32 Frame) const
{
	TArray<FHktFrameEvent> AllEvents;

	// Gather events from all intent components (READ-ONLY)
	for (const TWeakObjectPtr<UHktIntentComponent>& IntentComp : CachedIntentComponents)
	{
		if (UHktIntentComponent* Comp = IntentComp.Get())
		{
			TArray<FHktFrameEvent> Events = Comp->GetActiveEvents(static_cast<int32>(Frame));
			AllEvents.Append(Events);
		}
	}

	// Sort by priority (lower priority value = higher priority)
	AllEvents.Sort([](const FHktFrameEvent& A, const FHktFrameEvent& B)
	{
		if (A.Priority != B.Priority)
		{
			return A.Priority < B.Priority;
		}
		return A.StartFrame < B.StartFrame;
	});

	return AllEvents;
}

void UHktSimulationProcessor::ProcessMovementEvents(FMassExecutionContext& Context, const TArray<FHktFrameEvent>& Events, float DeltaTime)
{
	// Filter movement events
	TArray<const FHktFrameEvent*> InputMoveEvents;
	TArray<const FHktFrameEvent*> ActionMoveEvents;

	for (const FHktFrameEvent& Event : Events)
	{
		if (Event.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Hkt.Input.Movement"))))
		{
			InputMoveEvents.Add(&Event);
		}
		else if (Event.EventTag.MatchesTag(HktIntentTags::Event_MoveToLocation))
		{
			ActionMoveEvents.Add(&Event);
		}
	}

	if (InputMoveEvents.Num() == 0 && ActionMoveEvents.Num() == 0)
	{
		return;
	}

	// Process movement for relevant entities
	MovementQuery.ForEachEntityChunk(Context, [this, &InputMoveEvents, &ActionMoveEvents, DeltaTime](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FHktVisualFragment> VisualList = Context.GetMutableFragmentView<FHktVisualFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; ++EntityIdx)
		{
			FTransformFragment& TransformFrag = TransformList[EntityIdx];
			FHktVisualFragment& VisualFrag = VisualList[EntityIdx];
			FMassEntityHandle Entity = Context.GetEntity(EntityIdx);

			// Process each relevant input movement event (Legacy/Input)
			for (const FHktFrameEvent* Event : InputMoveEvents)
			{
				if (const FHktMoveEventPayload* Payload = Event->GetPayload<FHktMoveEventPayload>())
				{
					// Simple movement toward target
					FVector CurrentPos = TransformFrag.GetTransform().GetLocation();
					FVector TargetPos = Payload->TargetLocation;
					
					FVector Direction = TargetPos - CurrentPos;
					float Distance = Direction.Size();
					
					if (Distance > 1.0f) // Threshold for movement
					{
						Direction.Normalize();
						
						// Movement speed (could be from entity data)
						const float MoveSpeed = 400.0f;
						FVector NewVelocity = Direction * MoveSpeed;
						
						VisualFrag.Velocity = NewVelocity;
						
						// Set animation state
						VisualFrag.AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Moving"));
					}
					else
					{
						VisualFrag.Velocity = FVector::ZeroVector;
						VisualFrag.AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Idle"));
					}
				}
			}

			// Process Action Movement (MoveToLocation)
			for (const FHktFrameEvent* Event : ActionMoveEvents)
			{
				// Check Subject
				if (Event->Subject == Entity)
				{
					if (const FHktMoveEventPayload* Payload = Event->GetPayload<FHktMoveEventPayload>())
					{
						FVector CurrentPos = TransformFrag.GetTransform().GetLocation();
						FVector TargetPos = Payload->TargetLocation;

						FVector Direction = TargetPos - CurrentPos;
						float Distance = Direction.Size();

						if (Distance > 5.0f) // Threshold
						{
							Direction.Normalize();
							const float MoveSpeed = 400.0f;
							VisualFrag.Velocity = Direction * MoveSpeed;
							VisualFrag.AnimationState = HktIntentTags::Event_Animation_Move;
						}
						else
						{
							// Arrived
							VisualFrag.Velocity = FVector::ZeroVector;
							VisualFrag.AnimationState = HktIntentTags::Anim_Idle;

							// Remove Event
							if (UHktIntentComponent* Comp = FindIntentComponentByPlayerId(CachedIntentComponents, Event->SourcePlayerId))
							{
								Comp->RemoveEvent(Event->EventId);
							}
						}
					}
				}
			}
		}
	});
}

void UHktSimulationProcessor::ProcessCombatEvents(FMassExecutionContext& Context, const TArray<FHktFrameEvent>& Events, float DeltaTime)
{
	// Filter combat events
	TArray<const FHktFrameEvent*> CombatEvents;
	for (const FHktFrameEvent& Event : Events)
	{
		if (Event.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Hkt.Input.Combat"))))
		{
			CombatEvents.Add(&Event);
		}
	}

	if (CombatEvents.Num() == 0)
	{
		return;
	}

	// Process combat for relevant entities
	EntityQuery.ForEachEntityChunk(Context, [this, &CombatEvents, DeltaTime](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TArrayView<FHktVisualFragment> VisualList = Context.GetMutableFragmentView<FHktVisualFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; ++EntityIdx)
		{
			FHktVisualFragment& VisualFrag = VisualList[EntityIdx];

			for (const FHktFrameEvent* Event : CombatEvents)
			{
				if (const FHktCombatEventPayload* Payload = Event->GetPayload<FHktCombatEventPayload>())
				{
					// Set combat animation
					VisualFrag.AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Attack"));
					
					// Set visual effect
					VisualFrag.VisualEffect = Payload->AbilityTag.IsValid() 
						? Payload->AbilityTag 
						: FGameplayTag::RequestGameplayTag(FName("Hkt.VFX.Attack"));
				}
			}
		}
	});
}

void UHktSimulationProcessor::UpdateEntityTransforms(FMassExecutionContext& Context, float DeltaTime)
{
	MovementQuery.ForEachEntityChunk(Context, [DeltaTime](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FHktVisualFragment> VisualList = Context.GetMutableFragmentView<FHktVisualFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; ++EntityIdx)
		{
			FTransformFragment& TransformFrag = TransformList[EntityIdx];
			const FHktVisualFragment& VisualFrag = VisualList[EntityIdx];

			// Apply velocity to position
			if (!VisualFrag.Velocity.IsNearlyZero())
			{
				FTransform CurrentTransform = TransformFrag.GetTransform();
				FVector NewLocation = CurrentTransform.GetLocation() + VisualFrag.Velocity * DeltaTime;
				CurrentTransform.SetLocation(NewLocation);
				
				// Face movement direction
				if (VisualFrag.Velocity.SizeSquared() > 1.0f)
				{
					FRotator NewRotation = VisualFrag.Velocity.Rotation();
					CurrentTransform.SetRotation(NewRotation.Quaternion());
				}
				
				TransformFrag.SetTransform(CurrentTransform);
			}
		}
	});
}

void UHktSimulationProcessor::GenerateVisualFragments(FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		TArrayView<FHktVisualFragment> VisualList = Context.GetMutableFragmentView<FHktVisualFragment>();

		for (int32 EntityIdx = 0; EntityIdx < NumEntities; ++EntityIdx)
		{
			const FTransformFragment& TransformFrag = TransformList[EntityIdx];
			FHktVisualFragment& VisualFrag = VisualList[EntityIdx];

			// Copy transform to visual fragment
			VisualFrag.Transform = TransformFrag.GetTransform();
			
			// Update animation progress
			// (In a real implementation, this would be based on animation length)
			VisualFrag.AnimationProgress = FMath::Fmod(VisualFrag.AnimationProgress + 0.016f, 1.0f);
		}
	});
}

UHktFrameSyncSubsystem* UHktSimulationProcessor::GetFrameSyncSubsystem() const
{
	if (CachedFrameSyncSubsystem.IsValid())
	{
		return CachedFrameSyncSubsystem.Get();
	}

	UWorld* World = GetWorld();
	if (World)
	{
		CachedFrameSyncSubsystem = World->GetSubsystem<UHktFrameSyncSubsystem>();
		return CachedFrameSyncSubsystem.Get();
	}

	return nullptr;
}

void UHktSimulationProcessor::RefreshIntentComponents()
{
	CachedIntentComponents.Empty();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Find all player controllers with intent components
	for (TActorIterator<APlayerController> It(World); It; ++It)
	{
		if (UHktIntentComponent* IntentComp = It->FindComponentByClass<UHktIntentComponent>())
		{
			CachedIntentComponents.Add(IntentComp);
		}
	}
}

//-----------------------------------------------------------------------------
// UHktSimulationSubsystem
//-----------------------------------------------------------------------------

void UHktSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentFrame = 0;
}

void UHktSimulationSubsystem::Deinitialize()
{
	Snapshots.Empty();
	Super::Deinitialize();
}

bool UHktSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	if (World)
	{
		return World->IsGameWorld();
	}
	return false;
}

FHktSimulationSnapshot UHktSimulationSubsystem::GetSnapshot(uint32 Frame) const
{
	if (const FHktSimulationSnapshot* Found = Snapshots.Find(Frame))
	{
		return *Found;
	}
	return FHktSimulationSnapshot();
}

void UHktSimulationSubsystem::RecordSnapshot(const FHktSimulationSnapshot& Snapshot)
{
	// Add snapshot
	Snapshots.Add(Snapshot.FrameNumber, Snapshot);
	CurrentFrame = FMath::Max(CurrentFrame, Snapshot.FrameNumber);

	// Cleanup old snapshots
	if (Snapshots.Num() > MaxSnapshots)
	{
		uint32 OldestToKeep = CurrentFrame > static_cast<uint32>(MaxSnapshots) 
			? CurrentFrame - static_cast<uint32>(MaxSnapshots) 
			: 0;
		
		TArray<uint32> FramesToRemove;
		for (const auto& Pair : Snapshots)
		{
			if (Pair.Key < OldestToKeep)
			{
				FramesToRemove.Add(Pair.Key);
			}
		}
		
		for (uint32 Frame : FramesToRemove)
		{
			Snapshots.Remove(Frame);
		}
	}
}

bool UHktSimulationSubsystem::ValidateDeterminism(uint32 Frame, uint32 ExpectedChecksum) const
{
	if (const FHktSimulationSnapshot* Found = Snapshots.Find(Frame))
	{
		return Found->Checksum == ExpectedChecksum;
	}
	return false;
}

//-----------------------------------------------------------------------------
// FHktSimulationSnapshot
//-----------------------------------------------------------------------------

void FHktSimulationSnapshot::CalculateChecksum()
{
	Checksum = 0;
	
	for (const FHktSimulationEntityState& State : EntityStates)
	{
		// Simple checksum combining position and state
		Checksum = HashCombine(Checksum, GetTypeHash(State.Position.X));
		Checksum = HashCombine(Checksum, GetTypeHash(State.Position.Y));
		Checksum = HashCombine(Checksum, GetTypeHash(State.Position.Z));
		Checksum = HashCombine(Checksum, GetTypeHash(State.StateTag));
	}
}

bool FHktSimulationSnapshot::ValidateAgainst(const FHktSimulationSnapshot& Other) const
{
	return FrameNumber == Other.FrameNumber && Checksum == Other.Checksum;
}

