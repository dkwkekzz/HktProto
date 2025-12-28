// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktSimulationTypes.h"
#include "HktSimulationProcessor.generated.h"

class UHktIntentComponent;
class UHktFrameSyncSubsystem;

/**
 * Mass Processor that performs deterministic simulation based on intent events.
 * 
 * This processor:
 * - Reads events from HktIntentComponent (READ-ONLY)
 * - Computes entity states deterministically
 * - Writes visual fragments for HktPresentation to consume
 * 
 * Design principles:
 * - Never modifies HktIntent data
 * - Same input always produces same output (deterministic)
 * - Fixed timestep simulation
 */
UCLASS()
class HKTSIMULATION_API UHktSimulationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktSimulationProcessor();

protected:
	//~ Begin UMassProcessor Interface
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void Initialize(UObject& Owner) override;
	//~ End UMassProcessor Interface

	/**
	 * Gather all active events for the current frame.
	 * @param Frame The frame to gather events for
	 * @return Array of active events (read-only)
	 */
	TArray<FHktFrameEvent> GatherActiveEvents(uint32 Frame) const;

	/**
	 * Process movement events and update entity transforms.
	 * @param Context The execution context
	 * @param Events Active events to process
	 * @param DeltaTime Fixed delta time
	 */
	void ProcessMovementEvents(FMassExecutionContext& Context, const TArray<FHktFrameEvent>& Events, float DeltaTime);

	/**
	 * Process combat events and update entity states.
	 * @param Context The execution context
	 * @param Events Active events to process
	 * @param DeltaTime Fixed delta time
	 */
	void ProcessCombatEvents(FMassExecutionContext& Context, const TArray<FHktFrameEvent>& Events, float DeltaTime);

	/**
	 * Generate visual fragments from simulation state.
	 * @param Context The execution context
	 */
	void GenerateVisualFragments(FMassExecutionContext& Context);

	/**
	 * Update entity transforms based on velocity.
	 * @param Context The execution context
	 * @param DeltaTime Fixed delta time
	 */
	void UpdateEntityTransforms(FMassExecutionContext& Context, float DeltaTime);

	/**
	 * Get the frame sync subsystem.
	 */
	UHktFrameSyncSubsystem* GetFrameSyncSubsystem() const;

private:
	/** Query for entities with visual fragments */
	FMassEntityQuery EntityQuery;

	/** Query for entities needing movement simulation */
	FMassEntityQuery MovementQuery;

	/** Cached intent components (one per player) */
	UPROPERTY()
	TArray<TWeakObjectPtr<UHktIntentComponent>> CachedIntentComponents;

	/** Cached frame sync subsystem */
	UPROPERTY()
	mutable TWeakObjectPtr<UHktFrameSyncSubsystem> CachedFrameSyncSubsystem;

	/** Simulation configuration */
	UPROPERTY(EditAnywhere, Category = "Simulation")
	FHktSimulationConfig Config;

	/** Current simulation frame */
	uint32 CurrentSimFrame = 0;

	/** Refresh cached intent components */
	void RefreshIntentComponents();
};

/**
 * Subsystem that manages simulation state and provides access to simulation data.
 */
UCLASS()
class HKTSIMULATION_API UHktSimulationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin UWorldSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UWorldSubsystem Interface

	/**
	 * Get the current simulation frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	uint32 GetCurrentFrame() const { return CurrentFrame; }

	/**
	 * Get a snapshot of the simulation at a given frame.
	 * @param Frame The frame to get
	 * @return Snapshot if available, empty snapshot otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	FHktSimulationSnapshot GetSnapshot(uint32 Frame) const;

	/**
	 * Record a snapshot for the current frame.
	 * @param Snapshot The snapshot to record
	 */
	void RecordSnapshot(const FHktSimulationSnapshot& Snapshot);

	/**
	 * Validate simulation determinism by comparing snapshots.
	 * @param Frame The frame to validate
	 * @param ExpectedChecksum The expected checksum
	 * @return True if checksums match
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	bool ValidateDeterminism(uint32 Frame, uint32 ExpectedChecksum) const;

private:
	/** Current simulation frame */
	uint32 CurrentFrame = 0;

	/** Recorded snapshots for replay/debug */
	TMap<uint32, FHktSimulationSnapshot> Snapshots;

	/** Maximum snapshots to keep */
	int32 MaxSnapshots = 300;
};

