// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktPresentationTypes.h"
#include "HktPresentationProcessor.generated.h"

/**
 * Mass Processor that handles pure rendering of simulation results.
 * 
 * This processor:
 * - Reads visual fragments (written by HktSimulation)
 * - Updates ISM transforms
 * - Applies animation states
 * - Triggers visual effects
 * 
 * Design principles:
 * - NO GAME LOGIC - only rendering
 * - NO MODULE REFERENCES - no dependency on HktIntent or HktSimulation
 * - DATA-DRIVEN - reads from MassEntity fragments only
 */
UCLASS()
class HKTPRESENTATION_API UHktPresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktPresentationProcessor();

protected:
	//~ Begin UMassProcessor Interface
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void Initialize(UObject& Owner) override;
	//~ End UMassProcessor Interface

private:
	/**
	 * Update ISM transforms from visual fragment data.
	 * @param Context The execution context
	 */
	void UpdateInstancedStaticMeshes(FMassExecutionContext& Context);

	/**
	 * Apply animation states from visual fragments.
	 * @param Context The execution context
	 */
	void ApplyAnimationStates(FMassExecutionContext& Context);

	/**
	 * Trigger visual effects based on visual fragment data.
	 * @param Context The execution context
	 */
	void TriggerVisualEffects(FMassExecutionContext& Context);

	/**
	 * Calculate LOD level based on distance from camera.
	 * @param EntityLocation The entity's world location
	 * @return LOD level (0 = highest detail)
	 */
	uint8 CalculateLODLevel(const FVector& EntityLocation) const;

	/** Query for entities with visual data */
	FMassEntityQuery VisualDataQuery;

	/** Presentation configuration */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	FHktPresentationConfig Config;

	/** Cached camera location for LOD calculations */
	FVector CachedCameraLocation = FVector::ZeroVector;

	/** Active VFX handles for cleanup */
	TMap<FMassEntityHandle, TWeakObjectPtr<class UNiagaraComponent>> ActiveVFXComponents;
};

/**
 * Processor specifically for ISM (Instanced Static Mesh) updates.
 * Runs after the main presentation processor.
 */
UCLASS()
class HKTPRESENTATION_API UHktISMUpdateProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktISMUpdateProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** Query for ISM-rendered entities */
	FMassEntityQuery ISMQuery;
};

/**
 * Processor for animation bone updates (skeletal mesh entities).
 */
UCLASS()
class HKTPRESENTATION_API UHktAnimationUpdateProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktAnimationUpdateProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** Query for animated entities */
	FMassEntityQuery AnimationQuery;
};

