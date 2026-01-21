// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktIntentSubsystem.h"
#include "HktIntentEventComponent.h"
#include "HktIntentPlayerState.h"
#include "HktServiceSubsystem.h"

void UHktIntentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    EventHistory.Reset();
    LatestFrameNumber = 0;
    OldestFrameNumber = 0;
    
    if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
    {
        Service->RegisterIntentEventProvider(this);
    }
}

void UHktIntentSubsystem::Deinitialize()
{
    EventHistory.Reset();
    PlayerHandleToPlayerState.Reset();

    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        Service->UnregisterIntentEventProvider(this);
    }

    Super::Deinitialize();
}

UHktIntentSubsystem* UHktIntentSubsystem::Get(UWorld* World)
{
    return World ? World->GetSubsystem<UHktIntentSubsystem>() : nullptr;
}

bool UHktIntentSubsystem::AddEvent(const FHktIntentEvent& InEvent)
{
    EventHistory.Add(InEvent);
    
    // 프레임 번호 추적 (Late Join 지원)
    if (EventHistory.Num() == 1)
    {
        OldestFrameNumber = InEvent.FrameNumber;
        LatestFrameNumber = InEvent.FrameNumber;
    }
    else
    {
        LatestFrameNumber = FMath::Max(LatestFrameNumber, InEvent.FrameNumber);
    }

    UE_LOG(LogTemp, Verbose, TEXT("[HktIntentSubsystem] Added Event Frame: %d, EventId: %d, Tag: %s"),
        InEvent.FrameNumber, InEvent.EventId, *InEvent.EventTag.ToString());

    return true;
}

// ============================================================================
// IHktIntentEventProvider 구현
// ============================================================================

bool UHktIntentSubsystem::Fetch(TArray<FHktIntentEvent>& OutEvents)
{
    if (EventHistory.Num() == 0)
    {
        return false;
    }
    
    // History 전체를 반환하고 Flush
    OutEvents = MoveTemp(EventHistory);
    EventHistory.Reset();
    
    UE_LOG(LogTemp, Verbose, TEXT("[HktIntentSubsystem] Fetched %d events and flushed history"), OutEvents.Num());
    
    return true;
}

void UHktIntentSubsystem::Commit(int32 LastProcessedEventId, const FHktSimulationResult& Result)
{
    // Lockstep 방식: EventBuffer 정리만 수행
    // Late Join은 GameMode.PostLogin에서 RPC로 처리
    CleanupProcessedEvents(LastProcessedEventId);
    
    UE_LOG(LogTemp, Verbose, TEXT("[HktIntentSubsystem] Committed LastEventId: %d"), LastProcessedEventId);
}

int32 UHktIntentSubsystem::GetLatestFrameNumber() const
{
    return LatestFrameNumber;
}

int32 UHktIntentSubsystem::GetOldestFrameNumber() const
{
    return OldestFrameNumber;
}

void UHktIntentSubsystem::CleanupProcessedEvents(int32 LastProcessedEventId)
{
    // 모든 등록된 PlayerState의 EventBuffer에서 처리된 이벤트 제거
    TSet<UHktIntentEventComponent*> ProcessedComponents;
    
    for (const auto& Pair : PlayerHandleToPlayerState)
    {
        if (AHktIntentPlayerState* PlayerState = Pair.Value.Get())
        {
            if (UHktIntentEventComponent* EventComp = PlayerState->FindComponentByClass<UHktIntentEventComponent>())
            {
                if (!ProcessedComponents.Contains(EventComp))
                {
                    EventComp->RemoveProcessedEvents(LastProcessedEventId);
                    ProcessedComponents.Add(EventComp);
                }
            }
        }
    }
}

// ============================================================================
// Player Registration
// ============================================================================

void UHktIntentSubsystem::RegisterPlayerState(AHktIntentPlayerState* PlayerState, const FHktPlayerHandle& Handle)
{
    if (!PlayerState || !Handle.IsValid())
    {
        return;
    }

    // PlayerState에 핸들 설정
    PlayerState->SetPlayerHandle(Handle);

    // 매핑 추가
    PlayerHandleToPlayerState.Add(Handle.Value, PlayerState);

    UE_LOG(LogTemp, Log, TEXT("[HktIntentSubsystem] Registered PlayerState with Handle: %d"), Handle.Value);
}

void UHktIntentSubsystem::UnregisterPlayerState(AHktIntentPlayerState* PlayerState)
{
    if (!PlayerState)
    {
        return;
    }

    FHktPlayerHandle Handle = PlayerState->GetPlayerHandle();
    if (Handle.IsValid())
    {
        PlayerHandleToPlayerState.Remove(Handle.Value);
        UE_LOG(LogTemp, Log, TEXT("[HktIntentSubsystem] Unregistered PlayerState with Handle: %d"), Handle.Value);
    }
}

AHktIntentPlayerState* UHktIntentSubsystem::FindPlayerStateByHandle(const FHktPlayerHandle& Handle) const
{
    if (const TWeakObjectPtr<AHktIntentPlayerState>* Found = PlayerHandleToPlayerState.Find(Handle.Value))
    {
        return Found->Get();
    }
    return nullptr;
}
