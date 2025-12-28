// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MassEntityTypes.h"
#include "HktSimulationTypes.generated.h"

/**
 * Visual fragment produced by simulation, consumed by presentation.
 * Contains all data needed to render an entity for a single frame.
 * 
 * This is the OUTPUT of simulation - purely visual data with no game logic.
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktVisualFragment : public FMassFragment
{
	GENERATED_BODY()

	FHktVisualFragment() = default;

	//~ Transform Data

	/** World transform (position, rotation, scale) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FTransform Transform = FTransform::Identity;

	/** Velocity for motion blur / prediction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FVector Velocity = FVector::ZeroVector;

	//~ Animation Data

	/** Current animation state tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FGameplayTag AnimationState;

	/** Animation blend alpha (for transitions) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float AnimationBlend = 1.0f;

	/** Animation time/progress (0-1 normalized) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float AnimationProgress = 0.0f;

	//~ Visual Effect Data

	/** Active visual effect tag (VFX to play) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FGameplayTag VisualEffect;

	/** VFX intensity/scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float EffectIntensity = 1.0f;

	//~ Rendering Control

	/** LOD level (0 = highest detail) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	uint8 LODLevel = 0;

	/** Visibility flags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	uint8 VisibilityFlags = 0xFF;

	/** Rendering flags (custom per-project) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	uint8 RenderFlags = 0;

	//~ Team/Color Data

	/** Team color index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	uint8 TeamColorIndex = 0;

	/** Highlight state (selection, hover, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	uint8 HighlightState = 0;
};

/**
 * Simulation output state for an entity.
 * Used internally by simulation before writing to visual fragments.
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktSimulationEntityState
{
	GENERATED_BODY()

	/** The entity this state belongs to */
	UPROPERTY()
	FMassEntityHandle Entity;

	/** Position in world space */
	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	/** Rotation */
	UPROPERTY()
	FQuat Rotation = FQuat::Identity;

	/** Current velocity */
	UPROPERTY()
	FVector Velocity = FVector::ZeroVector;

	/** Movement target (if moving) */
	UPROPERTY()
	FVector TargetPosition = FVector::ZeroVector;

	/** Current state tag (idle, moving, attacking, etc.) */
	UPROPERTY()
	FGameplayTag StateTag;

	/** State progress (0-1) */
	UPROPERTY()
	float StateProgress = 0.0f;

	/** Is the entity alive/active? */
	UPROPERTY()
	bool bIsActive = true;
};

/**
 * Snapshot of the entire simulation state at a given frame.
 * Used for deterministic replay and debugging.
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktSimulationSnapshot
{
	GENERATED_BODY()

	/** The frame this snapshot represents */
	UPROPERTY()
	uint32 FrameNumber = 0;

	/** All entity states at this frame */
	UPROPERTY()
	TArray<FHktSimulationEntityState> EntityStates;

	/** Checksum for validation (determinism check) */
	UPROPERTY()
	uint32 Checksum = 0;

	/** Calculate checksum from entity states */
	void CalculateChecksum();

	/** Validate against another snapshot */
	bool ValidateAgainst(const FHktSimulationSnapshot& Other) const;
};

/**
 * Shared fragment for passing data from Simulation to Presentation.
 * This is the bridge between the two modules without direct dependency.
 */
USTRUCT()
struct HKTSIMULATION_API FHktPresentationDataFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	/** Last simulation frame processed */
	UPROPERTY()
	uint32 LastProcessedFrame = 0;

	/** Whether data has been updated and needs rendering */
	UPROPERTY()
	bool bDataDirty = false;
};

/**
 * Configuration for simulation processor.
 */
USTRUCT(BlueprintType)
struct HKTSIMULATION_API FHktSimulationConfig
{
	GENERATED_BODY()

	/** Fixed timestep (should match HktFrameSyncSubsystem) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	float FixedDeltaTime = 1.0f / 60.0f;

	/** Maximum simulation steps per frame (prevents spiral of death) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	int32 MaxStepsPerFrame = 4;

	/** Enable determinism checks (performance cost) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bEnableDeterminismChecks = false;

	/** Enable snapshot recording for replay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bRecordSnapshots = false;

	/** How many frames to keep in snapshot history */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	int32 SnapshotHistorySize = 300;
};

