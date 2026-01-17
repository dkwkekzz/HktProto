// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
        return EventId == Other.EventId;
    }

    bool operator!=(const FHktIntentEvent& Other) const
    {
        return !(*this == Other);
    }
};

/**
 * [Intent Event Entry]
 * 이벤트에 시퀀스 ID와 타임스탬프를 추가한 Sliding Window 엔트리
 * - SequenceId: 서버에서 발급하는 고유 번호 (1, 2, 3...)
 * - Timestamp: 생성 시간 (오래된 이벤트 만료 처리용)
 */
USTRUCT(BlueprintType)
struct FHktIntentEventEntry
{
	GENERATED_BODY()

	FHktIntentEventEntry()
		: SequenceId(0)
		, Timestamp(0.0)
	{}

	FHktIntentEventEntry(const FHktIntentEvent& InEvent, int64 InSequenceId)
		: EventData(InEvent)
		, SequenceId(InSequenceId)
		, Timestamp(FPlatformTime::Seconds())
	{}

	// 이벤트 고유 데이터
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHktIntentEvent EventData;

	// 이 이벤트의 고유 번호 (서버에서 발급 1, 2, 3...)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int64 SequenceId;

	// 생성된 시간 (오래된 이벤트 만료 처리용)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Timestamp;

	bool operator==(const FHktIntentEventEntry& Other) const
	{
		return SequenceId == Other.SequenceId;
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

UINTERFACE(MinimalAPI)
class UHktIntentChannel : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for a system that provides Intent Events to consumers (Simulation).
 * Implemented by HktIntent module.
 *
 * Channel: 함께 동기화가 필요한 이벤트 그룹
 * 
 * Sliding Window 방식:
 * - 이벤트는 바로 삭제되지 않고 일정 시간/개수 동안 유지됨
 * - 소비자는 자신의 커서(LastProcessedSeqId) 이후의 이벤트만 가져감
 * - Late Join 유저도 히스토리에서 이벤트를 가져와 따라잡을 수 있음
 */
class IHktIntentChannel
{
	GENERATED_BODY()

public:
	/** 인텐트 추가: Subject와 Tag가 겹치지 않는다고 가정하거나, 중복 허용 정책에 따름 */
	virtual bool AddEvent(const FHktIntentEvent& InEvent) = 0;

	/** 인텐트 제거: Subject와 Tag가 일치하는 이벤트를 찾아 제거 */
	virtual bool RemoveEvent(const FHktIntentEvent& InEvent) = 0;

	/** 인텐트 갱신: 기존 인텐트를 찾아 Target이나 Frame 등을 변경 (Remove -> Add 로직으로 처리하여 변경 사항 전파) */
	virtual bool UpdateEvent(const FHktIntentEvent& InNewEvent) = 0;

	/**
	 * [Sliding Window] 커서 기반 이벤트 조회
	 * @param InLastProcessedSeqId 마지막으로 처리한 시퀀스 ID (이후의 이벤트만 반환)
	 * @param OutEntries 새로 처리해야 할 이벤트 목록
	 * @return 새 이벤트가 있으면 true
	 */
	virtual bool FetchNewEvents(int64 InLastProcessedSeqId, TArray<FHktIntentEventEntry>& OutEntries) = 0;

	/** 현재 히스토리의 가장 최신 시퀀스 ID 반환 */
	virtual int64 GetLatestSequenceId() const = 0;

	/** 현재 히스토리의 가장 오래된 시퀀스 ID 반환 (Late Join 시 시작점) */
	virtual int64 GetOldestSequenceId() const = 0;

	// --- Legacy (하위 호환용, 사용 자제) ---
	
	/** [DEPRECATED] 외부 시스템에서 호출하여 변경된 이벤트 히스토리를 가져가고 내부 버퍼를 비웁니다. */
	UE_DEPRECATED(5.0, "Use FetchNewEvents instead for Sliding Window pattern")
	virtual bool FlushEvents(TArray<FHktIntentEvent>& OutEvents) = 0;
};
