// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "HktCoreTypes.generated.h"

/** 엔티티 식별자 (Stash 내 엔티티 인덱스) */
USTRUCT(BlueprintType)
struct FHktEntityId
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    int32 RawValue;

    // 기본 생성자
    constexpr FHktEntityId() : RawValue(0) {}

    // 명시적 생성자
    constexpr FHktEntityId(int32 InValue) : RawValue(InValue) {}

    // 유효성 검사 (0은 보통 Null Entity로 간주)
    bool IsValid() const { return RawValue != 0; }

    int32 GetValue() const { return RawValue; }

    /** 1. 비교 연산자 (Comparison Operators) */
    // 동일성 검사
    FORCEINLINE bool operator==(const FHktEntityId& Other) const { return RawValue == Other.RawValue; }
    FORCEINLINE bool operator!=(const FHktEntityId& Other) const { return RawValue != Other.RawValue; }

    // 정수형(int32)과 직접 비교 (편의성)
    FORCEINLINE bool operator==(int32 InRawValue) const { return RawValue == InRawValue; }
    FORCEINLINE bool operator!=(int32 InRawValue) const { return RawValue != InRawValue; }

	operator int32() const { return RawValue; }

    // 언리얼 엔진 TMap/TSet 지원을 위한 해시 함수
    friend uint32 GetTypeHash(const FHktEntityId& EntityId)
    {
        // uint32 값 자체가 이미 고유한 비트 패턴이므로 RawValue를 그대로 반환하거나
        // 언리얼의 기본 HashCombine을 사용할 수 있습니다.
        return GetTypeHash(EntityId.RawValue);
    }

    friend FArchive& operator<<(FArchive& Ar, FHktEntityId& Value)
    {
		return Ar << Value.RawValue;
    }
};

constexpr FHktEntityId InvalidEntityId = FHktEntityId(INDEX_NONE);

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
		, SourceEntity(InvalidEntityId)
		, TargetEntity(InvalidEntityId)
		, bIsGlobal(false)
	{}

	// Unique ID of the event
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 EventId;

	// The Source/Subject of this event (Relevancy 계산 기준)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktEntityId SourceEntity;

	// Classification of the event (What happened)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EventTag;

	// The Target entity involved
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktEntityId TargetEntity;

	// Location data (if applicable)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Location = FVector::ZeroVector;

    // 추가 파라미터
    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> Payload;

    // 글로벌 이벤트 여부 (true면 모든 클라이언트에게 전송)
    UPROPERTY(BlueprintReadWrite)
    bool bIsGlobal = false;

    bool operator==(const FHktIntentEvent& Other) const
    {
        return EventId == Other.EventId;
    }

    bool operator!=(const FHktIntentEvent& Other) const
    {
        return !(*this == Other);
    }

    bool operator<(const FHktIntentEvent& Other) const
    {
        return EventId < Other.EventId;
	}

	bool IsValid() const 
	{ 
		return EventId != 0; 
	}
};

/**
 * FHktFrameBatch - 서버 → 클라이언트 프레임 배치
 * 
 * 스냅샷과 이벤트를 분리하여 전송
 * - Snapshots: Relevancy에 새로 진입한 엔티티들
 * - Events: 이번 프레임의 Intent들
 */
USTRUCT()
struct HKTCORE_API FHktFrameBatch
{
    GENERATED_BODY()

    UPROPERTY()
    int32 FrameNumber = 0;

    // Relevancy에 새로 진입한 엔티티 스냅샷
    UPROPERTY()
    TArray<FHktEntitySnapshot> Snapshots;

    // Relevancy를 벗어난 엔티티 ID (클라가 제거해야 함)
    UPROPERTY()
    TArray<FHktEntityId> RemovedEntities;

    // 이번 프레임의 이벤트들 (스냅샷 분리됨)
    UPROPERTY()
    TArray<FHktIntentEvent> Events;

    int32 NumEvents() const { return Events.Num(); }
    int32 NumSnapshots() const { return Snapshots.Num(); }
    bool IsEmpty() const { return Events.IsEmpty() && Snapshots.IsEmpty() && RemovedEntities.IsEmpty(); }
    
    void Reset()
    {
        FrameNumber = 0;
        Snapshots.Empty();
        RemovedEntities.Empty();
        Events.Empty();
    }
};
