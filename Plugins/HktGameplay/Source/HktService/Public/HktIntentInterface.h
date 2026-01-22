// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktServiceInterface.h"
#include "HktIntentInterface.generated.h"

/**
 * [Intent Event]
 * Represents an incident or event in the world.
 * Can be an input action, a state change, or an entity existence.
 */
USTRUCT(BlueprintType)
struct HKTSERVICE_API FHktIntentEvent
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
        return EventId == Other.EventId;
    }

    bool operator!=(const FHktIntentEvent& Other) const
    {
        return !(*this == Other);
    }
};

/**
 * [Intent Event Batch]
 * 프레임 번호와 해당 프레임의 이벤트들을 묶은 배치 구조체
 * GameMode → IntentEventComponent → IHktSimulationProvider로 전달됨
 */
USTRUCT(BlueprintType)
struct HKTSERVICE_API FHktIntentEventBatch
{
	GENERATED_BODY()

	FHktIntentEventBatch()
		: FrameNumber(0)
	{}

	FHktIntentEventBatch(int32 InFrameNumber)
		: FrameNumber(InFrameNumber)
	{}

	// 이 배치의 프레임 번호
	UPROPERTY(BlueprintReadOnly)
	int32 FrameNumber;

	// 이 프레임에 포함된 이벤트들
	UPROPERTY(BlueprintReadOnly)
	TArray<FHktIntentEvent> Events;

	bool IsEmpty() const { return Events.Num() == 0; }
};

// --- Lockstep 시뮬레이션 결과 구조 ---

/**
 * 플레이어별 속성 변경 내역
 * 직렬 처리이므로 TMap 대신 간단한 배열 사용
 */
USTRUCT(BlueprintType)
struct FHktAttributeChanges
{
    GENERATED_BODY()
    
    // 변경된 속성과 새 값
    UPROPERTY()
    TArray<TPair<EHktAttributeType, float>> ChangedAttributes;
};

/**
 * 시뮬레이션 처리 결과
 * IHktSimulationProvider에서 처리 후 IntentEventComponent에 저장
 */
USTRUCT(BlueprintType)
struct HKTSERVICE_API FHktSimulationResult
{
    GENERATED_BODY()
    
    FHktSimulationResult()
        : ProcessedFrameNumber(0)
    {}
    
    // 처리된 프레임 번호
    UPROPERTY(BlueprintReadOnly)
    int32 ProcessedFrameNumber;
    
    // 플레이어별 속성 변경 내역
    UPROPERTY()
    TMap<int32, FHktAttributeChanges> PlayerAttributeChanges; // Key = PlayerHandle.Value
};
