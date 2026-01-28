#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HktIntentTypes.h"
#include "HktIntentEventComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntentEventReceived, const FHktIntentEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntentBatchReceived, const FHktIntentEventBatch&, Batch);

/**
 * UHktIntentEventComponent
 * 
 * IntentEvent의 C2S/S2C 전송을 담당하는 컴포넌트
 * PlayerController에 부착 (Replicated)
 */
UCLASS(ClassGroup=(HktSimulation), meta=(BlueprintSpawnableComponent))
class HKTRUNTIME_API UHktIntentEventComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHktIntentEventComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // === C2S: 클라이언트 → 서버 ===
    
    // 클라이언트에서 호출: 서버로 IntentEvent 전송
    UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
    void SendIntentToServer(const FHktIntentEvent& Event);

    // 서버 RPC
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ReceiveIntent(const FHktIntentEvent& Event);

    // === S2C: 서버 → 클라이언트 ===
    
    // 서버에서 호출: 클라이언트로 배치 전송
    UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
    void SendBatchToClient(const FHktIntentEventBatch& Batch);

    // 클라이언트 RPC
    UFUNCTION(Client, Reliable)
    void Client_ReceiveBatch(const FHktIntentEventBatch& Batch);

    // 단일 이벤트 전송 (배치 없이)
    UFUNCTION(Client, Reliable)
    void Client_ReceiveIntent(const FHktIntentEvent& Event);

    // === 이벤트 델리게이트 ===
    
    // 서버에서 Intent를 수신했을 때 (GameMode가 구독)
    UPROPERTY(BlueprintAssignable, Category = "Hkt|Intent")
    FOnIntentEventReceived OnServerReceivedIntent;

    // 클라이언트에서 배치를 수신했을 때 (VMProcessor가 구독)
    UPROPERTY(BlueprintAssignable, Category = "Hkt|Intent")
    FOnIntentBatchReceived OnClientReceivedBatch;

    // 클라이언트에서 단일 이벤트를 수신했을 때
    UPROPERTY(BlueprintAssignable, Category = "Hkt|Intent")
    FOnIntentEventReceived OnClientReceivedIntent;

    // === Relevancy 관리 (서버 전용) ===
    
    // 이 클라이언트의 Relevancy 정보
    FHktClientRelevancy& GetRelevancy() { return Relevancy; }
    const FHktClientRelevancy& GetRelevancy() const { return Relevancy; }

    // 엔티티 인지 여부
    UFUNCTION(BlueprintPure, Category = "Hkt|Intent")
    bool KnowsEntity(FHktEntityId EntityId) const { return Relevancy.KnowsEntity(EntityId); }

    // 엔티티 인지 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
    void MarkEntityKnown(FHktEntityId EntityId) { Relevancy.MarkEntityKnown(EntityId); }

    UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
    void MarkEntityUnknown(FHktEntityId EntityId) { Relevancy.MarkEntityUnknown(EntityId); }

protected:
    virtual void BeginPlay() override;

private:
    // 이 클라이언트의 Relevancy 정보 (서버에서만 유효)
    FHktClientRelevancy Relevancy;
};