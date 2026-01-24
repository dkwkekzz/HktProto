#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "HktServiceInterfaces.generated.h"

// ---------------------------------------------------
// Common Types
// ---------------------------------------------------

/**
 * Handle to uniquely identify a unit (abstracted from Mass/Actor).
 */
USTRUCT(BlueprintType)
struct FHktUnitHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Value = -1;

	bool IsValid() const { return Value != -1; }

	bool operator==(const FHktUnitHandle& Other) const
	{
		return Value == Other.Value;
	}
};

/**
 * Handle to uniquely identify a player.
 */
USTRUCT(BlueprintType)
struct FHktPlayerHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Value = -1;

	bool IsValid() const { return Value != -1; }

	bool operator==(const FHktUnitHandle& Other) const
	{
		return Value == Other.Value;
	}
};
