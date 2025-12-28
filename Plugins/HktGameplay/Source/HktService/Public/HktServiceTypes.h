#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "HktServiceTypes.generated.h"

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
 * Defines how an action behaves regarding targeting.
 */
UENUM(BlueprintType)
enum class EHktActionTargetType : uint8
{
	None,       // Immediate execution (Instant self buff, Stop, etc.)
	Location,   // Requires a location (Move, AOE)
	Actor,      // Requires an entity/actor (Attack, Targeted Spell)
	Direction   // Requires a direction (Dash, Skillshot)
};

// ---------------------------------------------------
// Payloads
// ---------------------------------------------------

USTRUCT(BlueprintType)
struct FHktMoveEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bQueued = false;
};

USTRUCT(BlueprintType)
struct FHktCombatEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FMassEntityHandle TargetEntity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag AbilityTag;
};

USTRUCT(BlueprintType)
struct FHktSpawnEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EntityType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Count = 1;
};

// ---------------------------------------------------
// Event
// ---------------------------------------------------

/**
 * [Intent Event]
 * Represents an incident or event in the world.
 * Can be an input action, a state change, or an entity existence.
 */
USTRUCT(BlueprintType)
struct FHktIntentEvent
{
	GENERATED_BODY()

	FHktIntentEvent()
		: EventId(0)
		, FrameNumber(0)
		, Magnitude(0.0f)
	{}

	// Unique ID of the event
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 EventId;

	// The Subject of this event (Owner)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FHktUnitHandle> Subjects;

	// Classification of the event (What happened)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EventTag;

	// The Targets involved (e.g. Selection)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktUnitHandle Target;

	// Location data (if applicable)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Location = FVector::ZeroVector;

	// Execution Frame (Server Absolute Frame)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 FrameNumber;
};

USTRUCT(BlueprintType)
struct FHktIntentEffect
{
	GENERATED_BODY()

	FHktIntentEffect()
		: EffectId(0)
		, FrameNumber(0)
	{}

	// Unique ID of the effect
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 EffectId;

	// Execution Frame (Server Absolute Frame)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 FrameNumber;

	// The Owner of this effect
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktUnitHandle Owner;

	// Classification of the effect (What happened)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EffectTag;
};
