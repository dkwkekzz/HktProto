#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterface.h"
#include "HktIntentSubsystem.generated.h"

class AHktIntentPlayerState;
class UHktAttributeComponent;

// ------------------------------------------------------------------------------------------------
// [Subsystem Implementation]
// ------------------------------------------------------------------------------------------------

/**
 * HktIntent 모듈의 핵심 서브시스템
 * 
 * 역할:
 * 1. IntentEvent 히스토리 관리 (Lockstep)
 * 2. PlayerState → AttributeComponent 매핑 관리
 * 3. Fetch-Commit 패턴으로 Simulation과 상호작용
 * 
 * 데이터 흐름:
 * Simulation → Fetch() → Process → Commit() → AttributeComponent → FFastArraySerializer → Client
 */
UCLASS()
class UHktIntentSubsystem : public UWorldSubsystem, public IHktIntentEventProvider
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Helper to get the subsystem from a world context
    static UHktIntentSubsystem* Get(UWorld* World);

    bool AddEvent(const FHktIntentEvent& InEvent);

    // --- IHktIntentEventProvider 인터페이스 구현 ---
    virtual bool Fetch(TArray<FHktIntentEvent>& OutEvents) override;
    virtual void Commit(int32 LastProcessedEventId, const FHktSimulationResult& Result) override;
    virtual int32 GetLatestFrameNumber() const override;
    virtual int32 GetOldestFrameNumber() const override;

    // --- Player Registration ---
    
    /**
     * PlayerState 등록 (GameMode에서 호출)
     * Component를 매핑에 추가
     */
    void RegisterPlayerState(AHktIntentPlayerState* PlayerState, const FHktPlayerHandle& Handle);
    
    /**
     * PlayerState 등록 해제
     */
    void UnregisterPlayerState(AHktIntentPlayerState* PlayerState);

private:
    /** PlayerHandle → AttributeComponent 조회 */
    UHktAttributeComponent* FindComponentByHandle(const FHktPlayerHandle& Handle) const;
    
    /** 처리된 이벤트 제거 (EventBuffer 정리) */
    void CleanupProcessedEvents(int32 LastProcessedEventId);

private:
    // 이벤트 히스토리 (추가만, Fetch 시 Flush)
    TArray<FHktIntentEvent> EventHistory;
    
    // 최신/최오래된 프레임 추적 (Late Join 지원)
    int32 LatestFrameNumber = 0;
    int32 OldestFrameNumber = 0;
    
    // PlayerHandle → AttributeComponent 매핑
    UPROPERTY()
    TMap<int32, TWeakObjectPtr<UHktAttributeComponent>> PlayerHandleToComponent;
};
