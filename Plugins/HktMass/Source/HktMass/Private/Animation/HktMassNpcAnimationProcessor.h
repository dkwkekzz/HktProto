// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "HktMassNpcAnimationProcessor.generated.h"

class UAnimToTextureDataAsset;
struct FHktMassNpcAnimationFragment;

/**
 * Fragment initializer for HktMassNpcAnimationFragment
 * Initializes animation timing when fragment is added to an entity
 */
UCLASS()
class HKTMASS_API UHktMassNpcAnimationFragmentInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UHktMassNpcAnimationFragmentInitializer();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	UPROPERTY()
	UWorld* World = nullptr;

	FMassEntityQuery EntityQuery;
};

/**
 * Processor for updating NPC animation states
 * Updates animation state index based on movement and AI state
 */
UCLASS()
class HKTMASS_API UHktMassNpcAnimationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassNpcAnimationProcessor();

	/** Threshold for determining if NPC is moving (squared velocity) */
	UPROPERTY(EditAnywhere, Category="Animation", meta=(ClampMin=0.0, UIMin=0.0))
	float MoveThresholdSq = 100.0f;

protected:
	/** Configure the owned FMassEntityQuery instances to express processor's requirements */
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	UPROPERTY(Transient)
	UWorld* World = nullptr;

	FMassEntityQuery AnimationEntityQuery;
};
