// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "HktCoreTypes.generated.h"

/** 엔티티 식별자 (Stash 내 엔티티 인덱스) */
using FHktEntityId = uint32;
constexpr FHktEntityId InvalidEntityId = INDEX_NONE;

/**
 * Handle to uniquely identify a player.
 */
USTRUCT(BlueprintType)
struct HKTCORE_API FHktPlayerHandle
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
 * 엔티티 스냅샷 - 클라이언트가 모르는 엔티티 정보를 전달할 때 사용
 */
USTRUCT(BlueprintType)
struct HKTCORE_API FHktEntitySnapshot
{
    GENERATED_BODY()

    UPROPERTY()
    FHktEntityId EntityId = InvalidEntityId;

    // SOA 데이터 직렬화
    UPROPERTY()
    TArray<int32> Properties;

    bool IsValid() const { return EntityId != InvalidEntityId; }
	FHktEntityId GetEntityId() const { return EntityId; }
};

/**
 * [Intent Event]
 * Represents an incident or event in the world.
 * Can be an input action, a state change, or an entity existence.
 */
USTRUCT(BlueprintType)
struct HKTCORE_API FHktIntentEvent
{
	GENERATED_BODY()

	FHktIntentEvent()
		: EventId(0)
		, SubjectEntityId(InvalidEntityId)
		, TargetEntityId(InvalidEntityId)
		, FrameNumber(0)
	{}

	// Unique ID of the event
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint32 EventId;

	// The Subject of this event (Owner)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktEntityId SubjectEntityId;

	// Classification of the event (What happened)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EventTag;

	// The Targets involved (e.g. Selection)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktEntityId TargetEntityId;

	// Location data (if applicable)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Location = FVector::ZeroVector;

    // 추가 파라미터
    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> Payload;

    // 클라이언트가 모르는 엔티티의 스냅샷 (S2C 전송 시 첨부)
    UPROPERTY()
    TArray<FHktEntitySnapshot> AttachedSnapshots;

    // 서버 틱 (동기화용)
    UPROPERTY()
    int64 FrameNumber = 0;

    bool operator==(const FHktIntentEvent& Other) const
    {
        return EventId == Other.EventId;
    }

    bool operator!=(const FHktIntentEvent& Other) const
    {
        return !(*this == Other);
    }

	bool IsValid() const 
	{ 
		return EventId != 0; 
	}
};

/**
 * [Intent Event Batch]
 * 프레임 번호와 해당 프레임의 이벤트들을 묶은 배치 구조체
 * GameMode → IntentEventComponent → IHktSimulationProvider로 전달됨
 */
USTRUCT(BlueprintType)
struct HKTCORE_API FHktIntentEventBatch
{
	GENERATED_BODY()

	FHktIntentEventBatch()
		: FrameNumber(0)
	{}

	FHktIntentEventBatch(int64 InFrameNumber)
		: FrameNumber(InFrameNumber)
	{}

	// 이 배치의 프레임 번호
	UPROPERTY(BlueprintReadOnly)
	int64 FrameNumber;

	// 이 프레임에 포함된 이벤트들
	UPROPERTY(BlueprintReadOnly)
	TArray<FHktIntentEvent> Events;

	bool IsEmpty() const { return Events.Num() == 0; }
};

/**
 * Relevancy 정보 - 클라이언트가 어떤 엔티티를 알고 있는지 추적
 */
USTRUCT()
struct HKTCORE_API FHktClientRelevancy
{
    GENERATED_BODY()

    // 이 클라이언트가 알고 있는 엔티티 목록
    UPROPERTY()
    TSet<FHktEntityId> KnownEntities;

    bool KnowsEntity(FHktEntityId EntityId) const
    {
        return KnownEntities.Contains(EntityId);
    }

    void MarkEntityKnown(FHktEntityId EntityId)
    {
        KnownEntities.Add(EntityId);
    }

    void MarkEntityUnknown(FHktEntityId EntityId)
    {
        KnownEntities.Remove(EntityId);
    }

    void Reset()
    {
        KnownEntities.Empty();
    }
};