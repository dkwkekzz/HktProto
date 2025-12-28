#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HktServiceTypes.h"
#include "HktActionDataAsset.generated.h"

class UTexture2D;

/**
 * Defines an actionable command available to units.
 * Contains UI information and execution rules.
 */
UCLASS(BlueprintType)
class HKTCODEX_API UHktActionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Display name of the action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	FText ActionName;

	/** Icon for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UTexture2D> Icon;

	/** The event tag to trigger when executed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	FGameplayTag EventTag;

	/** Targeting requirement for this action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	EHktActionTargetType TargetType = EHktActionTargetType::None;

	/** Max range for the action (0 for infinite/irrelevant) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	float Range = 0.0f;
};

