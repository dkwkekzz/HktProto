#include "HktIntentSubsystem.h"
#include "HktIntentEventComponent.h"
#include "HktAttributeComponent.h"
#include "HktIntentPlayerState.h"
#include "HktServiceSubsystem.h"

DECLARE_STATS_GROUP(TEXT("HktIntent"), STATGROUP_HktIntent, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("IntentSubsystem Tick"), STAT_IntentSubsystemTick, STATGROUP_HktIntent);
DECLARE_CYCLE_STAT(TEXT("SyncAttributesFromProvider"), STAT_SyncAttributesFromProvider, STATGROUP_HktIntent);

void UHktIntentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    EventHistory.Reset();
    EventHistory.Reserve(MaxBufferSize);
    
    if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
    {
        Service->RegisterIntentEventProvider(this);
        
        // Provider 델리게이트 바인딩 (Provider가 등록되면 바인딩)
        // Note: Provider는 HktSimulation이 등록하므로 나중에 바인딩됨
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

void UHktIntentSubsystem::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_IntentSubsystemTick);
    
    // 서버에서만 속성 동기화 (Simulation → Component)
    if (GetWorld() && GetWorld()->GetNetMode() != NM_Client)
    {
        SyncAttributesFromProvider();
    }
}

TStatId UHktIntentSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UHktIntentSubsystem, STATGROUP_HktIntent);
}

UHktIntentSubsystem* UHktIntentSubsystem::Get(UWorld* World)
{
    return World->GetSubsystem<UHktIntentSubsystem>();
}

bool UHktIntentSubsystem::AddEvent(const FHktIntentEvent& InEvent)
{
    EventHistory.Add(InEvent);
    
    // 오래된 이벤트 정리
    CleanupOldEvents(InEvent.FrameNumber);
    
    UE_LOG(LogTemp, Verbose, TEXT("[HktIntentSubsystem] Added Event Frame: %d, EventId: %d, Tag: %s"), 
        InEvent.FrameNumber, InEvent.EventId, *InEvent.EventTag.ToString());
    
    return true;
}

bool UHktIntentSubsystem::RemoveEvent(const FHktIntentEvent& InEvent)
{
    // 이벤트 ID로 찾아서 제거
    int32 RemovedCount = EventHistory.RemoveAll([&](const FHktIntentEvent& Event) {
        return Event.EventId == InEvent.EventId;
    });
    return RemovedCount > 0;
}

bool UHktIntentSubsystem::UpdateEvent(const FHktIntentEvent& InNewEvent)
{
    // 기존 이벤트를 찾아서 업데이트
    for (FHktIntentEvent& Event : EventHistory)
    {
        if (Event.EventId == InNewEvent.EventId)
        {
            Event = InNewEvent;
            return true;
        }
    }
    return false;
}

bool UHktIntentSubsystem::FetchNewEvents(int32 InLastProcessedFrame, TArray<FHktIntentEvent>& OutEvents)
{
    OutEvents.Reset();
    
    for (const FHktIntentEvent& Event : EventHistory)
    {
        // 마지막으로 처리한 프레임보다 큰 것만 가져옴 (중복 처리 방지)
        if (Event.FrameNumber > InLastProcessedFrame)
        {
            OutEvents.Add(Event);
        }
    }
    
    return OutEvents.Num() > 0;
}

int32 UHktIntentSubsystem::GetLatestFrameNumber() const
{
    if (EventHistory.Num() == 0)
    {
        return 0;
    }
    return EventHistory.Last().FrameNumber;
}

int32 UHktIntentSubsystem::GetOldestFrameNumber() const
{
    if (EventHistory.Num() == 0)
    {
        return 0;
    }
    return EventHistory[0].FrameNumber;
}

void UHktIntentSubsystem::CleanupOldEvents(int32 CurrentFrame)
{
    // 1. 프레임 기반 만료 처리
    const int32 ExpireFrame = CurrentFrame - RetentionFrames;
    EventHistory.RemoveAll([&](const FHktIntentEvent& Event) {
        return Event.FrameNumber < ExpireFrame;
    });
    
    // 2. 버퍼 크기 기반 만료 처리 (오래된 것부터 제거)
    while (EventHistory.Num() > MaxBufferSize)
    {
        EventHistory.RemoveAt(0);
    }
}

// ============================================================================
// Player Attribute Synchronization
// ============================================================================

void UHktIntentSubsystem::RegisterPlayerState(AHktIntentPlayerState* PlayerState)
{
    if (!PlayerState || !PlayerState->GetAttributeComponent())
    {
        return;
    }

    UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
    if (!Service)
    {
        return;
    }

    // Simulation의 Provider를 통해 플레이어 등록
    IHktPlayerAttributeProvider* Provider = Service->GetPlayerAttributeProvider().GetInterface();
    if (!Provider)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HktIntentSubsystem] PlayerAttributeProvider not available. Player registration deferred."));
        return;
    }

    // Simulation에 플레이어 등록
    FHktPlayerHandle Handle = Provider->RegisterPlayer();
    
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
    if (!Handle.IsValid())
    {
        return;
    }

    // 매핑 제거
    PlayerHandleToComponent.Remove(Handle.Value);

    // Simulation에서 등록 해제
    UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
    if (Service)
    {
        if (IHktPlayerAttributeProvider* Provider = Service->GetPlayerAttributeProvider().GetInterface())
        {
            Provider->UnregisterPlayer(Handle);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[HktIntentSubsystem] Unregistered PlayerState with Handle: %d"), Handle.Value);
}

void UHktIntentSubsystem::SyncAttributesFromProvider()
{
    SCOPE_CYCLE_COUNTER(STAT_SyncAttributesFromProvider);

    UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
    if (!Service)
    {
        return;
    }

    IHktPlayerAttributeProvider* Provider = Service->GetPlayerAttributeProvider().GetInterface();
    if (!Provider)
    {
        return;
    }

    // Dirty 플레이어 스냅샷 수신
    TArray<FHktPlayerAttributeSnapshot> ChangedSnapshots;
    if (Provider->ConsumeChangedPlayers(ChangedSnapshots))
    {
        for (const FHktPlayerAttributeSnapshot& Snapshot : ChangedSnapshots)
        {
            // 해당 핸들의 Component 찾기
            if (UHktAttributeComponent* Component = FindComponentByHandle(Snapshot.PlayerHandle))
            {
                // 스냅샷 적용 (FFastArraySerializer가 자동으로 델타 리플리케이션)
                Component->ApplyAttributeSnapshot(Snapshot);
            }
        }
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
