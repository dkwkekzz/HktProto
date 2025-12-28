// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktProjectileProcessor.h"
#include "HktAbilityTypes.h"
#include "HktSimulationTypes.h"
#include "HktFrameSyncSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"

UHktProjectileProcessor::UHktProjectileProcessor()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	ExecutionOrder.ExecuteAfter.Add(TEXT("UHktCombatProcessor"));
	bAutoRegisterWithProcessingPhases = true;
}

void UHktProjectileProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

void UHktProjectileProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// Query for projectiles
	ProjectileQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	ProjectileQuery.AddRequirement<FHktProjectileFragment>(EMassFragmentAccess::ReadWrite);
	ProjectileQuery.AddRequirement<FHktVisualFragment>(EMassFragmentAccess::ReadWrite);
	ProjectileQuery.AddTagRequirement<FHktProjectileTag>(EMassFragmentPresence::All);

	// Query for damageable targets
	DamageableQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	DamageableQuery.AddRequirement<FHktHealthFragment>(EMassFragmentAccess::ReadWrite);
	DamageableQuery.AddRequirement<FHktCombatFragment>(EMassFragmentAccess::ReadWrite);
	DamageableQuery.AddRequirement<FHktVisualFragment>(EMassFragmentAccess::ReadWrite);
}

void UHktProjectileProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const uint32 CurrentFrame = GetCurrentFrame();
	const float DeltaTime = GetFixedDeltaTime();

	EntitiesToDestroy.Reset();

	UpdateProjectiles(EntityManager, Context, CurrentFrame, DeltaTime);

	// Destroy expired/hit projectiles
	for (const FMassEntityHandle& Entity : EntitiesToDestroy)
	{
		if (EntityManager.IsEntityValid(Entity))
		{
			EntityManager.DestroyEntity(Entity);
		}
	}
}

void UHktProjectileProcessor::UpdateProjectiles(
	FMassEntityManager& EntityManager,
	FMassExecutionContext& Context,
	uint32 CurrentFrame,
	float DeltaTime)
{
	// Collect explosions to apply
	struct FPendingExplosion
	{
		FVector Location;
		float Radius;
		float Damage;
		int32 OwnerTeam;
		FMassEntityHandle ProjectileEntity;
	};
	TArray<FPendingExplosion> PendingExplosions;

	ProjectileQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
	{
		const int32 NumEntities = ChunkContext.GetNumEntities();
		TArrayView<FTransformFragment> TransformList = ChunkContext.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FHktProjectileFragment> ProjectileList = ChunkContext.GetMutableFragmentView<FHktProjectileFragment>();
		TArrayView<FHktVisualFragment> VisualList = ChunkContext.GetMutableFragmentView<FHktVisualFragment>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			FTransformFragment& Transform = TransformList[i];
			FHktProjectileFragment& Projectile = ProjectileList[i];
			FHktVisualFragment& Visual = VisualList[i];
			FMassEntityHandle Entity = ChunkContext.GetEntity(i);

			// Check lifetime
			const int32 Age = static_cast<int32>(CurrentFrame - Projectile.SpawnFrame);
			if (Age > Projectile.LifetimeFrames)
			{
				EntitiesToDestroy.Add(Entity);
				continue;
			}

			// Already hit something
			if (Projectile.bHasHit)
			{
				EntitiesToDestroy.Add(Entity);
				continue;
			}

			// Get current position
			FVector CurrentPos = Transform.GetTransform().GetLocation();
			
			// Calculate movement
			FVector ToTarget = Projectile.TargetPosition - CurrentPos;
			float DistanceToTarget = ToTarget.Size();

			// Check if reached target
			if (DistanceToTarget <= Projectile.Speed * DeltaTime * 1.5f)
			{
				// Reached destination - explode!
				Projectile.bHasHit = true;

				if (Projectile.bExplodesOnImpact)
				{
					FPendingExplosion Explosion;
					Explosion.Location = Projectile.TargetPosition;
					Explosion.Radius = Projectile.ExplosionRadius;
					Explosion.Damage = Projectile.Damage;
					Explosion.OwnerTeam = Projectile.OwnerTeamId;
					Explosion.ProjectileEntity = Entity;
					PendingExplosions.Add(Explosion);

					// Set explosion VFX
					Visual.VisualEffect = FGameplayTag::RequestGameplayTag(FName("Hkt.VFX.Explosion"));
					Visual.EffectIntensity = 1.0f;
				}

				EntitiesToDestroy.Add(Entity);
				continue;
			}

			// Move toward target
			FVector Direction = ToTarget.GetSafeNormal();
			Projectile.Velocity = Direction * Projectile.Speed;
			FVector NewPos = CurrentPos + Projectile.Velocity * DeltaTime;

			// Update transform
			FTransform NewTransform = Transform.GetTransform();
			NewTransform.SetLocation(NewPos);
			NewTransform.SetRotation(Direction.Rotation().Quaternion());
			Transform.SetTransform(NewTransform);

			// Update visual
			Visual.Transform = NewTransform;
			Visual.Velocity = Projectile.Velocity;
			Visual.VisualEffect = FGameplayTag::RequestGameplayTag(FName("Hkt.VFX.Projectile.Fireball"));
		}
	});

	// Apply all explosions
	for (const FPendingExplosion& Explosion : PendingExplosions)
	{
		ApplyExplosionDamage(
			EntityManager,
			Explosion.Location,
			Explosion.Radius,
			Explosion.Damage,
			Explosion.OwnerTeam
		);
	}
}

void UHktProjectileProcessor::ApplyExplosionDamage(
	FMassEntityManager& EntityManager,
	const FVector& ExplosionCenter,
	float Radius,
	float Damage,
	int32 OwnerTeam)
{
	const float RadiusSq = FMath::Square(Radius);

	FMassExecutionContext Context(EntityManager, 0.0f);
	
	DamageableQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
	{
		const int32 NumEntities = ChunkContext.GetNumEntities();
		TConstArrayView<FTransformFragment> TransformList = ChunkContext.GetFragmentView<FTransformFragment>();
		TArrayView<FHktHealthFragment> HealthList = ChunkContext.GetMutableFragmentView<FHktHealthFragment>();
		TArrayView<FHktCombatFragment> CombatList = ChunkContext.GetMutableFragmentView<FHktCombatFragment>();
		TArrayView<FHktVisualFragment> VisualList = ChunkContext.GetMutableFragmentView<FHktVisualFragment>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			// Skip same team (friendly fire off)
			if (CombatList[i].TeamId == OwnerTeam)
			{
				continue;
			}

			// Skip dead
			if (HealthList[i].bIsDead)
			{
				continue;
			}

			const FVector TargetPos = TransformList[i].GetTransform().GetLocation();
			const float DistSq = FVector::DistSquared(TargetPos, ExplosionCenter);

			// Check range
			if (DistSq > RadiusSq)
			{
				continue;
			}

			// Calculate damage falloff (linear)
			const float Distance = FMath::Sqrt(DistSq);
			const float DamageMult = 1.0f - (Distance / Radius);
			float FinalDamage = Damage * FMath::Max(0.3f, DamageMult); // Min 30% damage

			// Apply defense
			FinalDamage = FMath::Max(1.0f, FinalDamage - CombatList[i].Defense);

			// Apply damage
			HealthList[i].TakeDamage(FinalDamage);

			// Visual feedback
			VisualList[i].VisualEffect = FGameplayTag::RequestGameplayTag(FName("Hkt.VFX.Hit.Fire"));

			// Check death
			if (HealthList[i].bIsDead)
			{
				CombatList[i].CombatState = EHktCombatState::Dead;
				VisualList[i].AnimationState = FGameplayTag::RequestGameplayTag(FName("Hkt.Anim.Death"));
			}
		}
	});
}

uint32 UHktProjectileProcessor::GetCurrentFrame() const
{
	if (UHktFrameSyncSubsystem* FrameSync = GetFrameSyncSubsystem())
	{
		return FrameSync->GetServerFrame();
	}
	return 0;
}

float UHktProjectileProcessor::GetFixedDeltaTime() const
{
	if (UHktFrameSyncSubsystem* FrameSync = GetFrameSyncSubsystem())
	{
		return FrameSync->GetFixedDeltaTime();
	}
	return 1.0f / 60.0f;
}

UHktFrameSyncSubsystem* UHktProjectileProcessor::GetFrameSyncSubsystem() const
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

