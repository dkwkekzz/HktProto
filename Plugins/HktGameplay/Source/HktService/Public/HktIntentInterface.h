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
        return EventId == Other.EventId;
    }

    bool operator!=(const FHktIntentEvent& Other) const
    {
        return !(*this == Other);
    }
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
    TSet<FHktUnitHandle> Members;

    // 이 그룹을 구성하는 이벤트들 (EventContainer)
    UPROPERTY(BlueprintReadOnly)
    TArray<FHktIntentEvent> Events;
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
 * Commit 시 IntentSubsystem에 전달됨
 */
USTRUCT(BlueprintType)
struct FHktSimulationResult
{
    GENERATED_BODY()
    
    // 플레이어별 속성 변경 내역
    UPROPERTY()
    TMap<int32, FHktAttributeChanges> PlayerAttributeChanges; // Key = PlayerHandle.Value
};

UINTERFACE(MinimalAPI, BlueprintType)
class UHktIntentEventProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for a system that provides Intent Events to consumers (Simulation).
 * Implemented by HktIntent module.
 * 
 * Lockstep 방식:
 * - Fetch: 모든 이벤트 History를 반환하고 Flush
 * - Process: Simulation이 이벤트를 처리하고 결과 누적
 * - Commit: 처리 결과를 반영하고 EventBuffer 정리
 */
class HKTSERVICE_API IHktIntentEventProvider
{
	GENERATED_BODY()

public:
	/**
	 * [Lockstep] 모든 이벤트 History를 반환하고 Flush
	 * @param OutEvents 처리해야 할 모든 이벤트 (반환 후 History는 비워짐)
	 * @return 이벤트가 있으면 true
	 */
	virtual bool Fetch(TArray<FHktIntentEvent>& OutEvents) = 0;
	
	/**
	 * [Lockstep] 시뮬레이션 처리 결과를 Commit
	 * @param LastProcessedEventId 마지막으로 처리한 이벤트 ID
	 * @param Result 시뮬레이션 결과 (속성 변경 내역)
	 */
	virtual void Commit(int32 LastProcessedEventId, const FHktSimulationResult& Result) = 0;

	/** 현재 히스토리의 가장 최신 프레임 번호 반환 */
	virtual int32 GetLatestFrameNumber() const = 0;

	/** 현재 히스토리의 가장 오래된 프레임 번호 반환 (Late Join 시 시작점) */
	virtual int32 GetOldestFrameNumber() const = 0;
};
