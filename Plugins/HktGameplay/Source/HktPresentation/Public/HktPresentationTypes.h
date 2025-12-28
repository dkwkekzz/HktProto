// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MassEntityTypes.h"
#include "HktPresentationTypes.generated.h"

/**
 * Visual fragment containing all rendering data for an entity.
 * This struct is READ by HktPresentation and WRITTEN by HktSimulation.
 * 
 * Note: This is the same as FHktVisualFragment in HktSimulation.
 * We re-declare it here to avoid module dependency.
 * In practice, you might put this in a shared types module.
 */
USTRUCT(BlueprintType)
struct HKTPRESENTATION_API FHktPresentationVisualData : public FMassFragment
{
	GENERATED_BODY()

	FHktPresentationVisualData() = default;

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
 * Highlight states for entity rendering.
 */
UENUM(BlueprintType)
enum class EHktHighlightState : uint8
{
	None		= 0 UMETA(DisplayName = "None"),
	Selected	= 1 UMETA(DisplayName = "Selected"),
	Hovered		= 2 UMETA(DisplayName = "Hovered"),
	Targeted	= 3 UMETA(DisplayName = "Targeted"),
	Friendly	= 4 UMETA(DisplayName = "Friendly"),
	Hostile		= 5 UMETA(DisplayName = "Hostile"),
};

/**
 * Visibility flags for entity rendering.
 */
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EHktVisibilityFlags : uint8
{
	None			= 0,
	Visible			= 1 << 0,
	CastShadow		= 1 << 1,
	ReceiveShadow	= 1 << 2,
	Selectable		= 1 << 3,
	ShowInMinimap	= 1 << 4,
	ShowHealthBar	= 1 << 5,
	All				= 0xFF,
};
ENUM_CLASS_FLAGS(EHktVisibilityFlags);

/**
 * Configuration for presentation rendering.
 */
USTRUCT(BlueprintType)
struct HKTPRESENTATION_API FHktPresentationConfig
{
	GENERATED_BODY()

	/** Enable ISM (Instanced Static Mesh) rendering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	bool bUseInstancedRendering = true;

	/** Maximum instances per ISM batch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	int32 MaxInstancesPerBatch = 1000;

	/** LOD distance thresholds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	TArray<float> LODDistances = { 1000.0f, 2500.0f, 5000.0f, 10000.0f };

	/** Enable VFX spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	bool bEnableVFX = true;

	/** Maximum concurrent VFX per entity type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	int32 MaxConcurrentVFX = 50;

	/** Enable animation updates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	bool bEnableAnimations = true;
};

