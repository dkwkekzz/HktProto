// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktAbilityTypes.h"
#include "HktProjectileProcessor.generated.h"

class UHktFrameSyncSubsystem;

/**
 * Processor that handles projectile movement and collision.
 * 
 * Updates projectile positions, checks for impacts, and applies
 * explosion damage for projectiles like fireballs.
 */
UCLASS()
class HKTSIMULATION_API UHktProjectileProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktProjectileProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void Initialize(UObject& Owner) override;

private:
	/** Query for projectile entities */
	FMassEntityQuery ProjectileQuery;

	/** Query for entities that can be damaged */
	FMassEntityQuery DamageableQuery;

	/** Move projectiles and check for impacts */
	void UpdateProjectiles(FMassEntityManager& EntityManager, FMassExecutionContext& Context, uint32 CurrentFrame, float DeltaTime);

	/** Apply explosion damage at location */
	void ApplyExplosionDamage(
		FMassEntityManager& EntityManager,
		const FVector& ExplosionCenter,
		float Radius,
		float Damage,
		int32 OwnerTeam
	);

	/** Get current frame */
	uint32 GetCurrentFrame() const;

	/** Get fixed delta time */
	float GetFixedDeltaTime() const;

	/** Get frame sync subsystem */
	UHktFrameSyncSubsystem* GetFrameSyncSubsystem() const;

	UPROPERTY()
	mutable TWeakObjectPtr<UHktFrameSyncSubsystem> CachedFrameSync;

	/** Entities to destroy after processing */
	TArray<FMassEntityHandle> EntitiesToDestroy;
};

