// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMovementProcessor.generated.h"

class UHktFrameSyncSubsystem;
class UHktIntentComponent;

/**
 * Processor that handles entity movement based on Intent events.
 * 
 * Reads move events from HktIntentComponent and updates entity positions
 * deterministically based on fixed timestep.
 */
UCLASS()
class HKTSIMULATION_API UHktMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMovementProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void Initialize(UObject& Owner) override;

private:
	/** Query for entities with movement capability */
	FMassEntityQuery MovementQuery;

	/** Get delta time for fixed timestep */
	float GetFixedDeltaTime() const;

	/** Get frame sync subsystem */
	UHktFrameSyncSubsystem* GetFrameSyncSubsystem() const;

	/** Cached subsystem */
	UPROPERTY()
	mutable TWeakObjectPtr<UHktFrameSyncSubsystem> CachedFrameSync;

	/** Arrival threshold distance */
	float ArrivalThreshold = 50.0f;
};

