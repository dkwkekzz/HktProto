// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "HktServiceInterfaces.h"
#include "HktIntentInterfaces.generated.h"

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
	FHktUnitHandle Subject;

	// Classification of the event (What happened)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EventTag;

	// The Targets involved (e.g. Selection)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktUnitHandle Target;

	// Location data (if applicable)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Location = FVector::ZeroVector;

	// Magnitude of the event (intensity, strength, etc.)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Magnitude;

	// Execution Frame (Server Absolute Frame)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 FrameNumber;
	
    bool operator==(const FHktIntentEvent& Other) const
    {
        return EventId == Other.EventId && Subject == Other.Subject && Target == Other.Target && Location == Other.Location && FrameNumber == Other.FrameNumber;
    }

    bool operator!=(const FHktIntentEvent& Other) const
    {
        return !(*this == Other);
    }
};

/**
 * [Intent Event History Entry]
 * Represents a historical change to an intent event (add/remove).
 * Used for event replay and simulation.
 */
USTRUCT(BlueprintType)
struct FHktIntentHistoryEntry
{
	GENERATED_BODY()

	FHktIntentHistoryEntry()
		: bIsRemoved(false)
	{}

	FHktIntentHistoryEntry(const FHktIntentEvent& InEvent, bool bInIsRemoved)
		: Event(InEvent)
		, bIsRemoved(bInIsRemoved)
	{}

	// The event data
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktIntentEvent Event;

	// Whether this history entry represents a removal (true) or addition (false)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsRemoved;
};

// --- 이벤트 그룹 (외부 제공용 컨테이너) ---
USTRUCT(BlueprintType)
struct FHktIntentEventGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 GroupID = -1;

    // 이 그룹에 속한 유닛들
    UPROPERTY(BlueprintReadOnly)
    TSet<FHktUnitHandle> MemberUnits;

    // 이 그룹을 구성하는 이벤트들 (EventContainer)
    UPROPERTY(BlueprintReadOnly)
    TArray<FHktIntentEvent> Events;
};