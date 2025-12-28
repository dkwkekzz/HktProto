// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktAbilitySubsystem.h"
#include "HktAbilityTypes.h"
#include "HktIntentComponent.h"
#include "HktIntentTypes.h"
#include "HktIntentTags.h"
#include "HktFrameSyncSubsystem.h"
#include "HktSimulationTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

void UHktAbilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UMassEntitySubsystem>();
}

void UHktAbilitySubsystem::Deinitialize()
{
	IntentComponents.Empty();
	Super::Deinitialize();
}

bool UHktAbilitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UHktAbilitySubsystem::Tick(float DeltaTime)
{
	ProcessIntentEvents();

	// Refresh components periodically
	if (++ComponentRefreshCounter > 60)
	{
		GatherIntentComponents();
		ComponentRefreshCounter = 0;
	}
}

TStatId UHktAbilitySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHktAbilitySubsystem, STATGROUP_Tickables);
}

void UHktAbilitySubsystem::ProcessIntentEvents()
{
	const uint32 CurrentFrame = GetCurrentFrame();
	if (CurrentFrame == LastProcessedFrame)
	{
		return;
	}

	UMassEntitySubsystem* MassSubsystem = GetMassEntitySubsystem();
	if (!MassSubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();

	// Process events from all intent components
	for (const TWeakObjectPtr<UHktIntentComponent>& IntentPtr : IntentComponents)
	{
		UHktIntentComponent* Intent = IntentPtr.Get();
		if (!Intent)
		{
			continue;
		}

		// Get events for current frame
		TArray<FHktFrameEvent> Events = Intent->GetActiveEvents(static_cast<int32>(CurrentFrame));
		
		for (const FHktFrameEvent& Event : Events)
		{
			// Get selected entities from the intent component
			const TArray<FMassEntityHandle>& SelectedEntities = Intent->GetSelection();

			// Process movement events
			if (Event.EventTag.MatchesTag(HktIntentTags::Input_Movement_Move))
			{
				if (const FHktMoveEventPayload* Payload = Event.GetPayload<FHktMoveEventPayload>())
				{
					IssueMoveCommand(SelectedEntities, Payload->TargetLocation, Event.SourcePlayerId);
				}
			}
			// Process stop events
			else if (Event.EventTag.MatchesTag(HktIntentTags::Input_Movement_Stop))
			{
				IssueStopCommand(SelectedEntities);
			}
			// Process melee attack events
			else if (Event.EventTag.MatchesTag(HktIntentTags::Input_Combat_Attack))
			{
				if (const FHktCombatEventPayload* Payload = Event.GetPayload<FHktCombatEventPayload>())
				{
					IssueMeleeAttack(SelectedEntities, Payload->TargetLocation, Event.SourcePlayerId);
				}
			}
			// Process skill events (fireball)
			else if (Event.EventTag.MatchesTag(HktIntentTags::Input_Combat_Skill))
			{
				if (const FHktCombatEventPayload* Payload = Event.GetPayload<FHktCombatEventPayload>())
				{
					// First selected entity casts the fireball
					if (SelectedEntities.Num() > 0)
					{
						IssueFireball(SelectedEntities[0], Payload->TargetLocation, Event.SourcePlayerId);
					}
				}
			}
			// Process summon events
			else if (Event.EventTag.MatchesTag(HktIntentTags::Input_Spawn_Unit))
			{
				if (const FHktSpawnEventPayload* Payload = Event.GetPayload<FHktSpawnEventPayload>())
				{
					if (SelectedEntities.Num() > 0)
					{
						IssueSummon(SelectedEntities[0], Payload->SpawnLocation, Event.SourcePlayerId);
					}
				}
			}
		}
	}

	LastProcessedFrame = CurrentFrame;
}

void UHktAbilitySubsystem::IssueMoveCommand(
	const TArray<FMassEntityHandle>& SelectedEntities,
	const FVector& TargetLocation,
	int32 PlayerId)
{
	UMassEntitySubsystem* MassSubsystem = GetMassEntitySubsystem();
	if (!MassSubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();

	for (const FMassEntityHandle& Entity : SelectedEntities)
	{
		if (!EntityManager.IsEntityValid(Entity))
		{
			continue;
		}

		FHktMovementFragment* Movement = EntityManager.GetFragmentDataPtr<FHktMovementFragment>(Entity);
		FHktCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHktCombatFragment>(Entity);

		if (Movement)
		{
			Movement->TargetPosition = TargetLocation;
			Movement->bIsMoving = true;
			Movement->bHasReachedTarget = false;
		}

		if (Combat)
		{
			// Cancel any ongoing attack
			if (Combat->CombatState == EHktCombatState::Attacking)
			{
				Combat->CombatState = EHktCombatState::Moving;
			}
			else
			{
				Combat->CombatState = EHktCombatState::Moving;
			}
		}
	}
}

void UHktAbilitySubsystem::IssueStopCommand(const TArray<FMassEntityHandle>& Entities)
{
	UMassEntitySubsystem* MassSubsystem = GetMassEntitySubsystem();
	if (!MassSubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();

	for (const FMassEntityHandle& Entity : Entities)
	{
		if (!EntityManager.IsEntityValid(Entity))
		{
			continue;
		}

		FHktMovementFragment* Movement = EntityManager.GetFragmentDataPtr<FHktMovementFragment>(Entity);
		FHktCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHktCombatFragment>(Entity);

		if (Movement)
		{
			Movement->bIsMoving = false;
			Movement->Velocity = FVector::ZeroVector;
		}

		if (Combat && Combat->CombatState != EHktCombatState::Dead)
		{
			Combat->CombatState = EHktCombatState::Idle;
		}
	}
}

void UHktAbilitySubsystem::IssueMeleeAttack(
	const TArray<FMassEntityHandle>& AttackerEntities,
	const FVector& TargetLocation,
	int32 PlayerId)
{
	UMassEntitySubsystem* MassSubsystem = GetMassEntitySubsystem();
	if (!MassSubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
	const uint32 CurrentFrame = GetCurrentFrame();

	for (const FMassEntityHandle& Entity : AttackerEntities)
	{
		if (!EntityManager.IsEntityValid(Entity))
		{
			continue;
		}

		FHktCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHktCombatFragment>(Entity);
		FHktMovementFragment* Movement = EntityManager.GetFragmentDataPtr<FHktMovementFragment>(Entity);
		FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);

		if (Combat && Combat->CombatState != EHktCombatState::Dead)
		{
			// Face the target
			if (Transform)
			{
				FVector CurrentPos = Transform->GetTransform().GetLocation();
				FVector ToTarget = TargetLocation - CurrentPos;
				ToTarget.Z = 0;
				if (!ToTarget.IsNearlyZero())
				{
					FRotator NewRotation = ToTarget.Rotation();
					FTransform NewTransform = Transform->GetTransform();
					NewTransform.SetRotation(NewRotation.Quaternion());
					Transform->SetTransform(NewTransform);
				}
			}

			// Stop moving
			if (Movement)
			{
				Movement->bIsMoving = false;
				Movement->Velocity = FVector::ZeroVector;
			}

			// Start attack
			Combat->CombatState = EHktCombatState::Attacking;
			Combat->AbilityStartFrame = CurrentFrame;
			Combat->AbilityTargetLocation = TargetLocation;
			Combat->CurrentAbilityTag = FGameplayTag::RequestGameplayTag(FName("Hkt.Ability.MeleeAttack"));
		}
	}
}

void UHktAbilitySubsystem::IssueFireball(
	FMassEntityHandle CasterEntity,
	const FVector& TargetLocation,
	int32 PlayerId)
{
	UMassEntitySubsystem* MassSubsystem = GetMassEntitySubsystem();
	if (!MassSubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();

	if (!EntityManager.IsEntityValid(CasterEntity))
	{
		return;
	}

	FTransformFragment* CasterTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(CasterEntity);
	FHktCombatFragment* CasterCombat = EntityManager.GetFragmentDataPtr<FHktCombatFragment>(CasterEntity);

	if (!CasterTransform || !CasterCombat)
	{
		return;
	}

	// Get spawn position (in front of caster)
	FVector CasterPos = CasterTransform->GetTransform().GetLocation();
	FVector ToTarget = TargetLocation - CasterPos;
	ToTarget.Z = 0;
	FVector SpawnOffset = ToTarget.GetSafeNormal() * 50.0f; // Offset from caster
	FVector SpawnPos = CasterPos + SpawnOffset + FVector(0, 0, 100); // Raise a bit

	// Spawn the projectile
	SpawnProjectile(
		SpawnPos,
		TargetLocation,
		CasterEntity,
		CasterCombat->TeamId,
		2000.0f,  // Speed
		25.0f,    // Damage
		300.0f    // Explosion radius
	);

	// Set caster to casting state briefly
	CasterCombat->CombatState = EHktCombatState::CastingAbility;
	CasterCombat->AbilityStartFrame = GetCurrentFrame();
	CasterCombat->CurrentAbilityTag = FGameplayTag::RequestGameplayTag(FName("Hkt.Ability.Fireball"));
}

void UHktAbilitySubsystem::IssueSummon(
	FMassEntityHandle SummonerEntity,
	const FVector& SpawnLocation,
	int32 PlayerId)
{
	UMassEntitySubsystem* MassSubsystem = GetMassEntitySubsystem();
	if (!MassSubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();

	if (!EntityManager.IsEntityValid(SummonerEntity))
	{
		return;
	}

	FHktCombatFragment* SummonerCombat = EntityManager.GetFragmentDataPtr<FHktCombatFragment>(SummonerEntity);
	int32 TeamId = SummonerCombat ? SummonerCombat->TeamId : 0;

	// Spawn the summoned unit
	SpawnSummonedUnit(
		SpawnLocation,
		SummonerEntity,
		TeamId,
		0  // Permanent summon
	);

	// Set summoner to casting state
	if (SummonerCombat)
	{
		SummonerCombat->CombatState = EHktCombatState::CastingAbility;
		SummonerCombat->AbilityStartFrame = GetCurrentFrame();
		SummonerCombat->CurrentAbilityTag = FGameplayTag::RequestGameplayTag(FName("Hkt.Ability.Summon"));
	}
}

FMassEntityHandle UHktAbilitySubsystem::SpawnProjectile(
	const FVector& StartLocation,
	const FVector& TargetLocation,
	FMassEntityHandle OwnerEntity,
	int32 OwnerTeam,
	float Speed,
	float Damage,
	float ExplosionRadius)
{
	UMassEntitySubsystem* MassSubsystem = GetMassEntitySubsystem();
	if (!MassSubsystem)
	{
		return FMassEntityHandle();
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();

	// Create archetype for projectile
	FMassArchetypeHandle Archetype = EntityManager.CreateArchetype(
		TArray<const UScriptStruct*>{
			FTransformFragment::StaticStruct(),
			FHktProjectileFragment::StaticStruct(),
			FHktVisualFragment::StaticStruct()
		},
		TArray<const UScriptStruct*>{
			FHktProjectileTag::StaticStruct()
		},
		FMassArchetypeSharedFragmentValues()
	);

	// Spawn entity
	TArray<FMassEntityHandle> Entities;
	EntityManager.BatchCreateEntities(Archetype, 1, Entities);

	if (Entities.Num() == 0)
	{
		return FMassEntityHandle();
	}

	FMassEntityHandle ProjectileEntity = Entities[0];

	// Initialize fragments
	FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(ProjectileEntity);
	FHktProjectileFragment* Projectile = EntityManager.GetFragmentDataPtr<FHktProjectileFragment>(ProjectileEntity);
	FHktVisualFragment* Visual = EntityManager.GetFragmentDataPtr<FHktVisualFragment>(ProjectileEntity);

	if (Transform)
	{
		FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(StartLocation);
		SpawnTransform.SetRotation(Direction.Rotation().Quaternion());
		SpawnTransform.SetScale3D(FVector(1.0f));
		Transform->SetTransform(SpawnTransform);
	}

	if (Projectile)
	{
		Projectile->OwnerEntity = OwnerEntity;
		Projectile->OwnerTeamId = OwnerTeam;
		Projectile->TargetPosition = TargetLocation;
		Projectile->Speed = Speed;
		Projectile->Damage = Damage;
		Projectile->ExplosionRadius = ExplosionRadius;
		Projectile->SpawnFrame = GetCurrentFrame();
		Projectile->LifetimeFrames = 300; // 5 seconds
		Projectile->bExplodesOnImpact = true;
		Projectile->AbilityTag = FGameplayTag::RequestGameplayTag(FName("Hkt.Ability.Fireball"));
	}

	if (Visual)
	{
		Visual->Transform = Transform ? Transform->GetTransform() : FTransform::Identity;
		Visual->VisualEffect = FGameplayTag::RequestGameplayTag(FName("Hkt.VFX.Projectile.Fireball"));
	}

	return ProjectileEntity;
}

FMassEntityHandle UHktAbilitySubsystem::SpawnSummonedUnit(
	const FVector& SpawnLocation,
	FMassEntityHandle OwnerEntity,
	int32 OwnerTeam,
	int32 LifetimeFrames)
{
	UMassEntitySubsystem* MassSubsystem = GetMassEntitySubsystem();
	if (!MassSubsystem)
	{
		return FMassEntityHandle();
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();

	// Create archetype for summoned unit
	FMassArchetypeHandle Archetype = EntityManager.CreateArchetype(
		TArray<const UScriptStruct*>{
			FTransformFragment::StaticStruct(),
			FHktMovementFragment::StaticStruct(),
			FHktCombatFragment::StaticStruct(),
			FHktHealthFragment::StaticStruct(),
			FHktVisualFragment::StaticStruct(),
			FHktSummonFragment::StaticStruct()
		},
		TArray<const UScriptStruct*>{
			FHktSelectableTag::StaticStruct()
		},
		FMassArchetypeSharedFragmentValues()
	);

	// Spawn entity
	TArray<FMassEntityHandle> Entities;
	EntityManager.BatchCreateEntities(Archetype, 1, Entities);

	if (Entities.Num() == 0)
	{
		return FMassEntityHandle();
	}

	FMassEntityHandle SummonEntity = Entities[0];

	// Initialize fragments
	FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(SummonEntity);
	FHktMovementFragment* Movement = EntityManager.GetFragmentDataPtr<FHktMovementFragment>(SummonEntity);
	FHktCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHktCombatFragment>(SummonEntity);
	FHktHealthFragment* Health = EntityManager.GetFragmentDataPtr<FHktHealthFragment>(SummonEntity);
	FHktVisualFragment* Visual = EntityManager.GetFragmentDataPtr<FHktVisualFragment>(SummonEntity);
	FHktSummonFragment* Summon = EntityManager.GetFragmentDataPtr<FHktSummonFragment>(SummonEntity);

	if (Transform)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLocation);
		SpawnTransform.SetScale3D(FVector(1.0f));
		Transform->SetTransform(SpawnTransform);
	}

	if (Movement)
	{
		Movement->MoveSpeed = 350.0f; // Mercenary speed
	}

	if (Combat)
	{
		Combat->TeamId = OwnerTeam;
		Combat->AttackPower = 1.2f; // Mercenary has 20% more attack
		Combat->Defense = 5.0f;
	}

	if (Health)
	{
		Health->MaxHealth = 80.0f; // Mercenary has less health
		Health->CurrentHealth = 80.0f;
	}

	if (Visual)
	{
		Visual->Transform = Transform ? Transform->GetTransform() : FTransform::Identity;
		Visual->AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Idle"));
		Visual->VisualEffect = FGameplayTag::RequestGameplayTag(FName("Hkt.VFX.Summon"));
		Visual->TeamColorIndex = static_cast<uint8>(OwnerTeam);
	}

	if (Summon)
	{
		Summon->OwnerEntity = OwnerEntity;
		Summon->SummonFrame = GetCurrentFrame();
		Summon->LifetimeFrames = LifetimeFrames;
		Summon->bIsSummoned = true;
	}

	return SummonEntity;
}

void UHktAbilitySubsystem::GatherIntentComponents()
{
	IntentComponents.Empty();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<APlayerController> It(World); It; ++It)
	{
		if (UHktIntentComponent* Intent = It->FindComponentByClass<UHktIntentComponent>())
		{
			IntentComponents.Add(Intent);
		}
	}
}

UMassEntitySubsystem* UHktAbilitySubsystem::GetMassEntitySubsystem() const
{
	if (CachedMassSubsystem.IsValid())
	{
		return CachedMassSubsystem.Get();
	}

	UWorld* World = GetWorld();
	if (World)
	{
		CachedMassSubsystem = World->GetSubsystem<UMassEntitySubsystem>();
		return CachedMassSubsystem.Get();
	}
	return nullptr;
}

UHktFrameSyncSubsystem* UHktAbilitySubsystem::GetFrameSyncSubsystem() const
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

uint32 UHktAbilitySubsystem::GetCurrentFrame() const
{
	if (UHktFrameSyncSubsystem* FrameSync = GetFrameSyncSubsystem())
	{
		return FrameSync->GetServerFrame();
	}
	return 0;
}

