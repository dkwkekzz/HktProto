// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktIntentSubsystem.h"
#include "HktIntentEventComponent.h"
#include "HktAttributeComponent.h"
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
    PlayerHandleToComponent.Reset();

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
    // 1. 속성 변경 내역을 AttributeComponent에 반영
    for (const auto& Pair : Result.PlayerAttributeChanges)
    {
        FHktPlayerHandle Handle;
        Handle.Value = Pair.Key;
        
        if (UHktAttributeComponent* Component = FindComponentByHandle(Handle))
        {
            for (const TPair<EHktAttributeType, float>& AttrChange : Pair.Value.ChangedAttributes)
            {
                Component->SetAttribute(AttrChange.Key, AttrChange.Value);
            }
        }
    }
    
    // 2. EventBuffer 정리 (처리된 이벤트 제거)
    CleanupProcessedEvents(LastProcessedEventId);
    
    UE_LOG(LogTemp, Verbose, TEXT("[HktIntentSubsystem] Committed LastEventId: %d, Applied %d player attribute changes"),
        LastProcessedEventId, Result.PlayerAttributeChanges.Num());
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
    // PlayerHandleToComponent의 Owner가 PlayerState라고 가정
    
    TSet<UHktIntentEventComponent*> ProcessedComponents;
    
    for (const auto& Pair : PlayerHandleToComponent)
    {
        if (UHktAttributeComponent* Component = Pair.Value.Get())
        {
            if (AActor* Owner = Component->GetOwner())
            {
                // PlayerState는 여러 Component를 가질 수 있으므로 중복 처리 방지
                if (UHktIntentEventComponent* EventComp = Owner->FindComponentByClass<UHktIntentEventComponent>())
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
}

// ============================================================================
// Player Registration
// ============================================================================

void UHktIntentSubsystem::RegisterPlayerState(AHktIntentPlayerState* PlayerState, const FHktPlayerHandle& Handle)
{
    if (!PlayerState || !PlayerState->GetAttributeComponent() || !Handle.IsValid())
    {
        return;
    }

    // PlayerState에 핸들 설정
    PlayerState->SetPlayerHandle(Handle);

    // Component 매핑 추가
    PlayerHandleToComponent.Add(Handle.Value, PlayerState->GetAttributeComponent());

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
        PlayerHandleToComponent.Remove(Handle.Value);
        UE_LOG(LogTemp, Log, TEXT("[HktIntentSubsystem] Unregistered PlayerState with Handle: %d"), Handle.Value);
    }
}

UHktAttributeComponent* UHktIntentSubsystem::FindComponentByHandle(const FHktPlayerHandle& Handle) const
{
    if (const TWeakObjectPtr<UHktAttributeComponent>* Found = PlayerHandleToComponent.Find(Handle.Value))
    {
        return Found->Get();
    }
    return nullptr;
}
