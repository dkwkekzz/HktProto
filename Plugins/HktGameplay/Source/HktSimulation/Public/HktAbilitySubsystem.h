// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktAbilityTypes.h"
#include "MassEntityTypes.h"
#include "HktAbilitySubsystem.generated.h"

class UMassEntitySubsystem;
class UHktFrameSyncSubsystem;
class UHktIntentComponent;

/**
 * Subsystem that processes intent events and applies ability effects.
 * 
 * This is the bridge between HktIntent events and HktSimulation state changes.
 * It reads events and updates entity fragments accordingly.
 */
UCLASS()
class HKTSIMULATION_API UHktAbilitySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin UWorldSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ End UWorldSubsystem Interface

	/**
	 * Issue a move command to selected entities.
	 * @param SelectedEntities Entities to move
	 * @param TargetLocation Destination
	 * @param PlayerId Player issuing the command
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void IssueMoveCommand(const TArray<FMassEntityHandle>& SelectedEntities, const FVector& TargetLocation, int32 PlayerId);

	/**
	 * Issue a melee attack command.
	 * @param AttackerEntities Entities to attack with
	 * @param TargetLocation Direction to attack
	 * @param PlayerId Player issuing the command
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void IssueMeleeAttack(const TArray<FMassEntityHandle>& AttackerEntities, const FVector& TargetLocation, int32 PlayerId);

	/**
	 * Issue a fireball ability.
	 * @param CasterEntity Entity casting the fireball
	 * @param TargetLocation Where to shoot the fireball
	 * @param PlayerId Player issuing the command
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void IssueFireball(FMassEntityHandle CasterEntity, const FVector& TargetLocation, int32 PlayerId);

	/**
	 * Issue a summon ability.
	 * @param SummonerEntity Entity doing the summoning
	 * @param SpawnLocation Where to summon
	 * @param PlayerId Player issuing the command
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void IssueSummon(FMassEntityHandle SummonerEntity, const FVector& SpawnLocation, int32 PlayerId);

	/**
	 * Issue a stop command.
	 * @param Entities Entities to stop
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void IssueStopCommand(const TArray<FMassEntityHandle>& Entities);

	/**
	 * Spawn a projectile entity.
	 */
	FMassEntityHandle SpawnProjectile(
		const FVector& StartLocation,
		const FVector& TargetLocation,
		FMassEntityHandle OwnerEntity,
		int32 OwnerTeam,
		float Speed,
		float Damage,
		float ExplosionRadius
	);

	/**
	 * Spawn a summoned unit.
	 */
	FMassEntityHandle SpawnSummonedUnit(
		const FVector& SpawnLocation,
		FMassEntityHandle OwnerEntity,
		int32 OwnerTeam,
		int32 LifetimeFrames = 0
	);

private:
	/** Process pending events from intent components */
	void ProcessIntentEvents();

	/** Find all intent components in the world */
	void GatherIntentComponents();

	/** Get subsystems */
	UMassEntitySubsystem* GetMassEntitySubsystem() const;
	UHktFrameSyncSubsystem* GetFrameSyncSubsystem() const;
	uint32 GetCurrentFrame() const;

	/** Cached subsystems */
	UPROPERTY()
	mutable TWeakObjectPtr<UMassEntitySubsystem> CachedMassSubsystem;

	UPROPERTY()
	mutable TWeakObjectPtr<UHktFrameSyncSubsystem> CachedFrameSync;

	/** Cached intent components */
	UPROPERTY()
	TArray<TWeakObjectPtr<UHktIntentComponent>> IntentComponents;

	/** Last frame we processed events */
	uint32 LastProcessedFrame = 0;

	/** Frame counter for component refresh */
	int32 ComponentRefreshCounter = 0;
};

