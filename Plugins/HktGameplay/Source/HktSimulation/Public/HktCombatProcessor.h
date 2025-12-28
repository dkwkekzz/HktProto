// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktAbilityTypes.h"
#include "HktCombatProcessor.generated.h"

class UHktFrameSyncSubsystem;
class UHktAbilitySubsystem;

/**
 * Processor that handles combat actions (melee attacks, abilities).
 * 
 * Processes attack events, calculates damage, and updates combat states
 * deterministically.
 */
UCLASS()
class HKTSIMULATION_API UHktCombatProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktCombatProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void Initialize(UObject& Owner) override;

private:
	/** Query for entities with combat capability */
	FMassEntityQuery CombatQuery;

	/** Query for all potential targets */
	FMassEntityQuery TargetQuery;

	/** Process melee attacks */
	void ProcessMeleeAttacks(FMassEntityManager& EntityManager, FMassExecutionContext& Context, uint32 CurrentFrame);

	/** Apply damage to entities in range */
	void ApplyMeleeDamage(
		FMassEntityManager& EntityManager,
		const FVector& AttackerPos,
		const FVector& AttackerForward,
		float Range,
		float Arc,
		float Damage,
		int32 AttackerTeam
	);

	/** Find targets in cone */
	TArray<FMassEntityHandle> FindTargetsInCone(
		FMassEntityManager& EntityManager,
		const FVector& Origin,
		const FVector& Direction,
		float Range,
		float ArcDegrees,
		int32 ExcludeTeam
	);

	/** Get current server frame */
	uint32 GetCurrentFrame() const;

	/** Get fixed delta time */
	float GetFixedDeltaTime() const;

	/** Get frame sync subsystem */
	UHktFrameSyncSubsystem* GetFrameSyncSubsystem() const;

	/** Cached subsystem */
	UPROPERTY()
	mutable TWeakObjectPtr<UHktFrameSyncSubsystem> CachedFrameSync;
};

