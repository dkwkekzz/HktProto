#include "HktIntentSubsystem.h"
#include "HktIntentComponent.h"
#include "HktServiceSubsystem.h"

void UHktIntentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ChannelMap.Reset();
    CurrentServerFrame = 0;
    
    if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
    {
        Service->RegisterIntentEventProvider(this);
    }
}

void UHktIntentSubsystem::Deinitialize()
{
    ChannelMap.Reset();

    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        Service->UnregisterIntentEventProvider(this);
    }

    Super::Deinitialize();
}

UHktIntentSubsystem* UHktIntentSubsystem::Get(UWorld* World)
{
    return World->GetSubsystem<UHktIntentSubsystem>();
}

FHktChannelIntentData& UHktIntentSubsystem::GetChannelData(int32 ChannelId)
{
    return ChannelMap.FindOrAdd(ChannelId);
}

void UHktIntentSubsystem::SetCurrentServerFrame(int32 FrameNumber)
{
    // 프레임은 보통 증가하는 방향이어야 하지만, 롤백 상황 등을 고려해 단순 대입
    CurrentServerFrame = FrameNumber;
}

void UHktIntentSubsystem::AddIntentEvent(int32 ChannelId, const FHktIntentEvent& InEvent)
{
    FHktChannelIntentData& Data = GetChannelData(ChannelId);

    // 1. 활성 리스트에 추가 (State)
    Data.ActiveIntents.Add(InEvent);

    // 2. 히스토리 버퍼에 추가 이벤트로 기록
    Data.HistoryBuffer.Emplace(InEvent, false);
}

void UHktIntentSubsystem::RemoveIntentEvent(int32 ChannelId, const FHktIntentEvent& InEvent)
{
    FHktChannelIntentData& Data = GetChannelData(ChannelId);

    // 1. 활성 리스트에서 제거 (State)
    Data.ActiveIntents.Remove(InEvent);
    
    // 2. 히스토리 버퍼에 제거 이벤트로 기록
    Data.HistoryBuffer.Emplace(InEvent, true);
}

void UHktIntentSubsystem::UpdateIntentEvent(int32 ChannelId, const FHktIntentEvent& InNewEvent)
{
    FHktChannelIntentData& Data = GetChannelData(ChannelId);
    
    // 1. 활성 리스트에서 기존 이벤트 찾아서 업데이트 (State)
    FHktIntentEvent* ExistingEvent = Data.ActiveIntents.FindByPredicate([&](const FHktIntentEvent& Event)
    {
        return Event.EventId == InNewEvent.EventId;
    });
    if (ExistingEvent)
    {
        *ExistingEvent = InNewEvent;
    }
    
    FHktIntentHistoryEntry* ExistingHistoryEntry = Data.HistoryBuffer.FindByPredicate([&](const FHktIntentHistoryEntry& Entry)
    {
        return Entry.Event.EventId == InNewEvent.EventId;
    });
    if (ExistingHistoryEntry)
    {
        ExistingHistoryEntry->Event = InNewEvent;
    }
}

bool UHktIntentSubsystem::FlushEvents(int32 ChannelId, int32& OutSyncedFrame, TArray<FHktIntentHistoryEntry>& OutHistory)
{
    // 맵에 없으면 처리할 게 없음
    if (!ChannelMap.Contains(ChannelId))
    {
        return false;
    }

    FHktChannelIntentData& Data = ChannelMap[ChannelId];

    // 버퍼에 데이터가 없으면 false 반환 (최적화)
    if (Data.HistoryBuffer.Num() == 0)
    {
        // 데이터는 없지만 프레임 동기화 정보는 갱신해주어야 할 수도 있음.
        OutSyncedFrame = CurrentServerFrame;
        Data.LastSyncedFrame = CurrentServerFrame;
        return false; 
    }

    // 1. 현재 시스템의 서버 프레임을 기록
    OutSyncedFrame = CurrentServerFrame;
    Data.LastSyncedFrame = CurrentServerFrame;

    // 2. 히스토리 버퍼 이동 (MoveTemp로 복사 비용 절감)
    OutHistory = MoveTemp(Data.HistoryBuffer);

    return true;
}

int32 UHktIntentSubsystem::GetCurrentServerFrame() const
{
    return CurrentServerFrame;
}