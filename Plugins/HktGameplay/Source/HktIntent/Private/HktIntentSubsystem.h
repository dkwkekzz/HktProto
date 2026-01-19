#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HktIntentInterface.h"
#include "IHktPlayerAttributeProvider.h"
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
 * 1. IntentEvent 히스토리 관리 (Sliding Window)
 * 2. PlayerState → AttributeComponent 매핑 관리
 * 3. Simulation의 PlayerAttributeProvider 구독 및 Component 동기화
 */
UCLASS()
class UHktIntentSubsystem : public UTickableWorldSubsystem, public IHktIntentEventProvider
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // UTickableWorldSubsystem
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

    // Helper to get the subsystem from a world context
    static UHktIntentSubsystem* Get(UWorld* World);

    // --- IHktIntentEventProvider 인터페이스 구현 ---
    virtual bool AddEvent(const FHktIntentEvent& InEvent) override;
    virtual bool RemoveEvent(const FHktIntentEvent& InEvent) override;
    virtual bool UpdateEvent(const FHktIntentEvent& InNewEvent) override;
    virtual bool FetchNewEvents(int32 InLastProcessedFrame, TArray<FHktIntentEvent>& OutEvents) override;
    virtual int32 GetLatestFrameNumber() const override;
    virtual int32 GetOldestFrameNumber() const override;

    // --- Player Attribute Synchronization ---
    
    /**
     * PlayerState 등록 (GameMode에서 호출)
     * Simulation에 플레이어를 등록하고 AttributeComponent에 핸들 설정
     */
    void RegisterPlayerState(AHktIntentPlayerState* PlayerState);
    
    /**
     * PlayerState 등록 해제
     */
    void UnregisterPlayerState(AHktIntentPlayerState* PlayerState);

private:
    void CleanupOldEvents(int32 CurrentFrame);
    
    /** Simulation Provider로부터 변경된 속성을 수신하여 Component에 적용 */
    void SyncAttributesFromProvider();
    
    /** PlayerHandle → AttributeComponent 조회 */
    UHktAttributeComponent* FindComponentByHandle(const FHktPlayerHandle& Handle) const;

private:
    // Sliding Window 설정
    int32 MaxBufferSize = 64;
    int32 RetentionFrames = 300; // 약 5초 (60fps 기준)

    // 이벤트 히스토리 (Sliding Window)
    TArray<FHktIntentEvent> EventHistory;
    
    // PlayerHandle → AttributeComponent 매핑
    // PlayerState 등록 시 추가, 등록 해제 시 제거
    UPROPERTY()
    TMap<int32, TWeakObjectPtr<UHktAttributeComponent>> PlayerHandleToComponent;
    
    // Provider 델리게이트 핸들 (Deinitialize에서 Unbind)
    FDelegateHandle AttributeChangedDelegateHandle;
};
