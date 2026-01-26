// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "HktIntentEventComponent.generated.h"

class UHktIntentEventComponent;

/**
 * 시뮬레이션 상태 (이제 스냅샷 데이터만 보유)
 * ProcessingBatches는 RPC로 직접 전송하므로 여기서 제거되었습니다.
 */
USTRUCT()
struct FHktSimulationState
{
    GENERATED_BODY()

    FHktSimulationState() = default;

    /** * 완료된 시뮬레이션 결과 (기준점).
     * 최초 1회만 동기화되어 클라이언트 시뮬레이션의 시작점이 됩니다.
     */
    UPROPERTY()
    int32 CompletedFrameNumber = 0;

    UPROPERTY()
    FHktUnitStateArray UnitStates;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HKTINTENT_API UHktIntentEventComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHktIntentEventComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** [Client] 의도 전송 */
    void CommitIntent(const FHktIntentEvent& IntentEvent);

    /** [Server] 배치 처리 시작 (RPC 호출 트리거) */
    void PushIntentBatch(int32 FrameNumber);

    /** [Client & Server] 배치 제공 */
    void PullIntentEvents(int32 CompletedFrameNumber, TArray<FHktIntentEvent>& OutIntentEvents);

protected:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_CommitIntent(FHktIntentEvent IntentEvent);
   
    /**
     * [Client] 서버로부터 배치를 수신하는 RPC (Reliable Multicast)
     * 리플리케이션 대신 이 함수를 통해 입력을 동기화합니다.
     */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PushIntentBatch(const FHktIntentEventBatch& Batch);

    /** RepNotify: 초기 스냅샷 수신 시 호출 */
    UFUNCTION()
    void OnRep_ProcessingIntentEvents();

private:
    bool HasAuthority() const { return GetOwner() && GetOwner()->HasAuthority(); }

    /** [Server-Only] 수집 중인 의도들 */
    TArray<FHktIntentEvent> PendingIntentEvents;

    /** * [Client/Server] 로컬에서 처리해야 할 배치 큐
     * Client: 초기화 전에는 버퍼(Jitter Buffer) 역할을 하며, 초기화 후에는 실행 대기열이 됩니다.
     */
    UPROPERTY(ReplicatedUsing = OnRep_ProcessingIntentEvents)
    TArray<FHktIntentEvent> ProcessingIntentEvents;

    int32 LastPulledEventID = INDEX_NONE;
};

