#include "HktIntentEventComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"

UHktIntentEventComponent::UHktIntentEventComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UHktIntentEventComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Relevancy는 서버에서만 관리하므로 복제하지 않음
}

void UHktIntentEventComponent::BeginPlay()
{
    Super::BeginPlay();
}

// === C2S ===

void UHktIntentEventComponent::SendIntentToServer(const FHktIntentEvent& Event)
{
    if (!GetOwner()->HasAuthority())
    {
        // 클라이언트: 서버 RPC 호출
        Server_ReceiveIntent(Event);
    }
    else
    {
        // 서버(리슨 서버의 호스트): 직접 처리
        OnServerReceivedIntent.Broadcast(Event);
    }
}

bool UHktIntentEventComponent::Server_ReceiveIntent_Validate(const FHktIntentEvent& Event)
{
    // 기본 검증
    if (!Event.IsValid())
    {
        return false;
    }

    // TODO: 추가 검증 (치트 방지 등)
    // - SourceEntity가 이 클라이언트의 소유인가?
    // - 쿨다운 체크
    // - 범위 체크 등

    return true;
}

void UHktIntentEventComponent::Server_ReceiveIntent_Implementation(const FHktIntentEvent& Event)
{
    UE_LOG(LogTemp, Verbose, TEXT("Server received intent: %s from Entity %d"), 
        *Event.EventTag.ToString(), Event.SourceEntity);

    // 델리게이트로 GameMode에 알림
    OnServerReceivedIntent.Broadcast(Event);
}

// === S2C ===

void UHktIntentEventComponent::SendBatchToClient(const FHktIntentEventBatch& Batch)
{
    if (GetOwner()->HasAuthority())
    {
        // 서버: 클라이언트 RPC 호출
        Client_ReceiveBatch(Batch);
    }
}

void UHktIntentEventComponent::Client_ReceiveBatch_Implementation(const FHktIntentEventBatch& Batch)
{
    UE_LOG(LogTemp, Verbose, TEXT("Client received batch: Tick=%d, Events=%d"), 
        Batch.ServerTick, Batch.Num());

    // 델리게이트로 VMProcessor에 알림
    OnClientReceivedBatch.Broadcast(Batch);
}

void UHktIntentEventComponent::Client_ReceiveIntent_Implementation(const FHktIntentEvent& Event)
{
    UE_LOG(LogTemp, Verbose, TEXT("Client received intent: %s"), *Event.EventTag.ToString());

    // 델리게이트로 VMProcessor에 알림
    OnClientReceivedIntent.Broadcast(Event);
}