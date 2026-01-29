// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HktRuntimeTypes.h"
#include "HktVMProcessorComponent.generated.h"

class FHktVMProcessor;
class IStashInterface;
class UHktMasterStashComponent;
class UHktVisibleStashComponent;

/**
 * UHktVMProcessorComponent - FHktVMProcessor를 랩핑하는 컴포넌트
 * 
 * 서버(GameMode)와 클라이언트(PlayerController) 모두에서 사용 가능
 * - 서버: MasterStash와 연결하여 권위 있는 시뮬레이션 실행
 * - 클라: VisibleStash와 연결하여 로컬 시뮬레이션 실행
 * 
 * 사용법:
 *   1. InitializeWithMasterStash() 또는 InitializeWithVisibleStash() 호출
 *   2. QueueIntentEvent()로 이벤트 주입
 *   3. ProcessFrame()으로 프레임 처리
 */
UCLASS(ClassGroup=(HktSimulation), meta=(BlueprintSpawnableComponent))
class HKTRUNTIME_API UHktVMProcessorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHktVMProcessorComponent();

    // ========== Initialization ==========
    
    /** 서버용: MasterStash로 초기화 */
    void InitializeWithMasterStash(UHktMasterStashComponent* InMasterStash);
    
    /** 클라용: VisibleStash로 초기화 */
    void InitializeWithVisibleStash(UHktVisibleStashComponent* InVisibleStash);
    
    /** 초기화 여부 */
    bool IsInitialized() const { return bIsInitialized; }

    // ========== Event Queue ==========
    
    /** 단일 Intent 이벤트 큐잉 */
    void QueueIntentEvent(const FHktIntentEvent& Event);
    
    /** 여러 Intent 이벤트 일괄 큐잉 */
    void QueueIntentEvents(const TArray<FHktIntentEvent>& Events);

    // ========== Processing ==========
    
    /** 프레임 처리 (Build → Execute → Cleanup 파이프라인 실행) */
    void ProcessFrame(int32 CurrentFrame, float DeltaSeconds);

    // ========== Notifications ==========
    
    /** 충돌 알림 */
    void NotifyCollision(FHktEntityId WatchedEntity, FHktEntityId HitEntity);
    
    /** 애니메이션 종료 알림 */
    void NotifyAnimEnd(FHktEntityId Entity);
    
    /** 이동 종료 알림 */
    void NotifyMoveEnd(FHktEntityId Entity);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    /** 내부 VMProcessor 인스턴스 */
    TUniquePtr<FHktVMProcessor> VMProcessor;
    
    /** 초기화 완료 여부 */
    bool bIsInitialized = false;
    
    /** 연결된 MasterStash (서버용) - IStashInterface 구현 */
    UPROPERTY()
    TObjectPtr<UHktMasterStashComponent> MasterStash;
    
    /** 연결된 VisibleStash (클라용) - IStashInterface 구현 */
    UPROPERTY()
    TObjectPtr<UHktVisibleStashComponent> VisibleStash;
};
