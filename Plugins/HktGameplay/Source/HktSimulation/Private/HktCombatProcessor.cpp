// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktCombatProcessor.h"
#include "HktAbilityTypes.h"
#include "HktSimulationTypes.h"
#include "HktFrameSyncSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"

UHktCombatProcessor::UHktCombatProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	ExecutionOrder.ExecuteAfter.Add(TEXT("UHktMovementProcessor"));
	bAutoRegisterWithProcessingPhases = true;
}

void UHktCombatProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void UHktCombatProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// Query for entities that can attack
	CombatQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	CombatQuery.AddRequirement<FHktCombatFragment>(EMassFragmentAccess::ReadWrite);
	CombatQuery.AddRequirement<FHktVisualFragment>(EMassFragmentAccess::ReadWrite);
	CombatQuery.AddRequirement<FHktMovementFragment>(EMassFragmentAccess::ReadWrite);

	// Query for entities that can be damaged
	TargetQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	TargetQuery.AddRequirement<FHktHealthFragment>(EMassFragmentAccess::ReadWrite);
	TargetQuery.AddRequirement<FHktCombatFragment>(EMassFragmentAccess::ReadOnly);
}

void UHktCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const uint32 CurrentFrame = GetCurrentFrame();

	ProcessMeleeAttacks(EntityManager, Context, CurrentFrame);
}

void UHktCombatProcessor::ProcessMeleeAttacks(FMassEntityManager& EntityManager, FMassExecutionContext& Context, uint32 CurrentFrame)
{
	// Melee attack configuration (would come from data asset in full impl)
	constexpr float MeleeRange = 150.0f;
	constexpr float MeleeArc = 90.0f;
	constexpr float MeleeDamage = 10.0f;
	constexpr int32 DamageFrameOffset = 20;  // Damage applies 20 frames after attack start
	constexpr int32 AttackDuration = 40;      // Total attack animation duration

	// Collect attack info for damage application
	struct FPendingDamage
	{
		FVector AttackerPos;
		FVector AttackerForward;
		int32 AttackerTeam;
		float Damage;
	};
	TArray<FPendingDamage> PendingDamages;

	CombatQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
	{
		const int32 NumEntities = ChunkContext.GetNumEntities();
		TConstArrayView<FTransformFragment> TransformList = ChunkContext.GetFragmentView<FTransformFragment>();
		TArrayView<FHktCombatFragment> CombatList = ChunkContext.GetMutableFragmentView<FHktCombatFragment>();
		TArrayView<FHktVisualFragment> VisualList = ChunkContext.GetMutableFragmentView<FHktVisualFragment>();
		TArrayView<FHktMovementFragment> MovementList = ChunkContext.GetMutableFragmentView<FHktMovementFragment>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			const FTransformFragment& Transform = TransformList[i];
			FHktCombatFragment& Combat = CombatList[i];
			FHktVisualFragment& Visual = VisualList[i];
			FHktMovementFragment& Movement = MovementList[i];

			// Skip dead entities
			if (Combat.CombatState == EHktCombatState::Dead)
			{
				continue;
			}

			// Handle attacking state
			if (Combat.CombatState == EHktCombatState::Attacking)
			{
				const int32 FramesSinceStart = static_cast<int32>(CurrentFrame - Combat.AbilityStartFrame);

				// Check if we're at the damage frame
				if (FramesSinceStart == DamageFrameOffset)
				{
					FPendingDamage Damage;
					Damage.AttackerPos = Transform.GetTransform().GetLocation();
					Damage.AttackerForward = Transform.GetTransform().GetRotation().GetForwardVector();
					Damage.AttackerTeam = Combat.TeamId;
					Damage.Damage = MeleeDamage * Combat.AttackPower;
					PendingDamages.Add(Damage);

					// Set hit VFX
					Visual.VisualEffect = FGameplayTag::RequestGameplayTag(FName("Hkt.VFX.MeleeHit"));
				}

				// Check if attack is complete
				if (FramesSinceStart >= AttackDuration)
				{
					Combat.CombatState = EHktCombatState::Idle;
					Combat.CurrentAbilityTag = FGameplayTag();
					Visual.AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Idle"));
				}
				else
				{
					// Still attacking - update animation
					Visual.AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Attack"));
					Visual.AnimationProgress = static_cast<float>(FramesSinceStart) / static_cast<float>(AttackDuration);
				}
			}
		}
	});

	// Apply all pending damage
	for (const FPendingDamage& Pending : PendingDamages)
	{
		ApplyMeleeDamage(
			EntityManager,
			Pending.AttackerPos,
			Pending.AttackerForward,
			MeleeRange,
			MeleeArc,
			Pending.Damage,
			Pending.AttackerTeam
		);
	}
}

void UHktCombatProcessor::ApplyMeleeDamage(
	FMassEntityManager& EntityManager,
	const FVector& AttackerPos,
	const FVector& AttackerForward,
	float Range,
	float Arc,
	float Damage,
	int32 AttackerTeam)
{
	// Find targets in cone
	TArray<FMassEntityHandle> Targets = FindTargetsInCone(
		EntityManager,
		AttackerPos,
		AttackerForward,
		Range,
		Arc,
		AttackerTeam
	);

	// Apply damage to each target
	for (const FMassEntityHandle& Target : Targets)
	{
		if (!EntityManager.IsEntityValid(Target))
		{
			continue;
		}

		FHktHealthFragment* Health = EntityManager.GetFragmentDataPtr<FHktHealthFragment>(Target);
		FHktCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHktCombatFragment>(Target);
		FHktVisualFragment* Visual = EntityManager.GetFragmentDataPtr<FHktVisualFragment>(Target);

		if (Health && !Health->bIsDead)
		{
			// Apply defense reduction
			float FinalDamage = Damage;
			if (Combat)
			{
				FinalDamage = FMath::Max(1.0f, Damage - Combat->Defense);
			}

			Health->TakeDamage(FinalDamage);

			// Update visual for hit reaction
			if (Visual)
			{
				Visual->VisualEffect = FGameplayTag::RequestGameplayTag(FName("Hkt.VFX.Hit"));
			}

			// Check for death
			if (Health->bIsDead && Combat)
			{
				Combat->CombatState = EHktCombatState::Dead;
				if (Visual)
				{
					Visual->AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Death"));
				}
			}
		}
	}
}

TArray<FMassEntityHandle> UHktCombatProcessor::FindTargetsInCone(
	FMassEntityManager& EntityManager,
	const FVector& Origin,
	const FVector& Direction,
	float Range,
	float ArcDegrees,
	int32 ExcludeTeam)
{
	TArray<FMassEntityHandle> Result;
	const float RangeSq = FMath::Square(Range);
	const float HalfArcCos = FMath::Cos(FMath::DegreesToRadians(ArcDegrees * 0.5f));

	FMassExecutionContext Context(EntityManager, 0.0f);
	
	TargetQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
	{
		const int32 NumEntities = ChunkContext.GetNumEntities();
		TConstArrayView<FTransformFragment> TransformList = ChunkContext.GetFragmentView<FTransformFragment>();
		TConstArrayView<FHktCombatFragment> CombatList = ChunkContext.GetFragmentView<FHktCombatFragment>();
		TConstArrayView<FHktHealthFragment> HealthList = ChunkContext.GetFragmentView<FHktHealthFragment>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			// Skip same team
			if (CombatList[i].TeamId == ExcludeTeam)
			{
				continue;
			}

			// Skip dead
			if (HealthList[i].bIsDead)
			{
				continue;
			}

			const FVector TargetPos = TransformList[i].GetTransform().GetLocation();
			const FVector ToTarget = TargetPos - Origin;
			const float DistSq = ToTarget.SizeSquared();

			// Check range
			if (DistSq > RangeSq)
			{
				continue;
			}

			// Check cone angle
			const FVector ToTargetNorm = ToTarget.GetSafeNormal();
			const float DotProduct = FVector::DotProduct(Direction, ToTargetNorm);
			
			if (DotProduct >= HalfArcCos)
			{
				Result.Add(ChunkContext.GetEntity(i));
			}
		}
	});

	return Result;
}

uint32 UHktCombatProcessor::GetCurrentFrame() const
{
	if (UHktFrameSyncSubsystem* FrameSync = GetFrameSyncSubsystem())
	{
		return FrameSync->GetServerFrame();
	}
	return 0;
}

float UHktCombatProcessor::GetFixedDeltaTime() const
{
	if (UHktFrameSyncSubsystem* FrameSync = GetFrameSyncSubsystem())
	{
		return FrameSync->GetFixedDeltaTime();
	}
	return 1.0f / 60.0f;
}

UHktFrameSyncSubsystem* UHktCombatProcessor::GetFrameSyncSubsystem() const
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

