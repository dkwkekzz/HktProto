// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HktIntentEventDataAsset.generated.h"

/**
 * Defines a single effect to be applied.
 * Can be extended with Duration, Magnitude, etc.
 */
USTRUCT(BlueprintType)
struct HKTCODEX_API FHktIntentEffectDefinition
{
	GENERATED_BODY()

	/** The tag identifying this effect type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FGameplayTag EffectTag;

	/** Optional: Additional metadata for future extension */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	// float Duration = 0.0f;
	// float Magnitude = 1.0f;
};

/**
 * Asset that defines how an Intent Event maps to Effects.
 * Each asset handles one EventTag and specifies what effects to apply to Subjects and Targets.
 */
UCLASS(BlueprintType)
class HKTCODEX_API UHktIntentventDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The EventTag this asset handles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping")
	FGameplayTag EventTag;

	/** Effects to apply to the Subject(s) of the event */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping")
	TArray<FHktIntentEffectDefinition> SubjectEffects;

	/** Effects to apply to the Target of the event */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mapping")
	TArray<FHktIntentEffectDefinition> TargetEffects;
};

