#include "HktIntentSubsystem.h"
#include "HktIntentEventComponent.h"
#include "HktServiceSubsystem.h"

namespace Internal
{
    /**
     * Sliding Window 기반 Intent Channel 구현
     * - 이벤트를 즉시 삭제하지 않고 일정 시간/개수 동안 히스토리에 유지
     * - 소비자는 커서(LastProcessedSeqId)를 기반으로 새 이벤트만 조회
     * - Late Join 유저도 히스토리에서 이벤트를 가져와 따라잡을 수 있음
     */
    class FSlidingWindowChannel : public IHktIntentChannel
    {
    public:
        FSlidingWindowChannel(int32 InMaxBufferSize = 64, double InRetentionTime = 5.0)
            : MaxBufferSize(InMaxBufferSize)
            , RetentionTime(InRetentionTime)
            , GlobalSequenceGenerator(0)
        {
            EventHistory.Reserve(MaxBufferSize);
        }

        virtual ~FSlidingWindowChannel() {}

        // --- IHktIntentChannel 구현 ---

        virtual bool AddEvent(const FHktIntentEvent& InEvent) override
        {
            // 시퀀스 ID 발급
            FHktIntentEventEntry Entry(InEvent, ++GlobalSequenceGenerator);
            
            EventHistory.Add(Entry);
            
            // 오래된 이벤트 정리
            CleanupOldEvents();
            
            UE_LOG(LogTemp, Verbose, TEXT("[SlidingWindowChannel] Added Event SeqId: %lld, EventId: %d, Tag: %s"), 
                Entry.SequenceId, InEvent.EventId, *InEvent.EventTag.ToString());
            
            return true;
        }

        virtual bool RemoveEvent(const FHktIntentEvent& InEvent) override
        {
            // 이벤트 ID로 찾아서 제거 (논리적 제거 - 실제로는 Cleanup에서 처리)
            int32 RemovedCount = EventHistory.RemoveAll([&](const FHktIntentEventEntry& Entry) {
                return Entry.EventData.EventId == InEvent.EventId;
            });
            return RemovedCount > 0;
        }

        virtual bool UpdateEvent(const FHktIntentEvent& InNewEvent) override
        {
            // 기존 이벤트를 찾아서 업데이트
            for (FHktIntentEventEntry& Entry : EventHistory)
            {
                if (Entry.EventData.EventId == InNewEvent.EventId)
                {
                    Entry.EventData = InNewEvent;
                    Entry.Timestamp = FPlatformTime::Seconds(); // 타임스탬프 갱신
                    return true;
                }
            }
            return false;
        }

        virtual bool FetchNewEvents(int64 InLastProcessedSeqId, TArray<FHktIntentEventEntry>& OutEntries) override
        {
            OutEntries.Reset();
            
            for (const FHktIntentEventEntry& Entry : EventHistory)
            {
                // 마지막으로 처리한 것보다 큰 ID만 가져옴 (중복 처리 방지)
                if (Entry.SequenceId > InLastProcessedSeqId)
                {
                    OutEntries.Add(Entry);
                }
            }
            
            return OutEntries.Num() > 0;
        }

        virtual int64 GetLatestSequenceId() const override
        {
            return GlobalSequenceGenerator;
        }

        virtual int64 GetOldestSequenceId() const override
        {
            if (EventHistory.Num() == 0)
            {
                return 0;
            }
            return EventHistory[0].SequenceId;
        }

        // --- Legacy (하위 호환용) ---
        
        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        virtual bool FlushEvents(TArray<FHktIntentEvent>& OutEvents) override
        {
            if (EventHistory.Num() == 0)
            {
                return false;
            }

            OutEvents.Reset();
            for (const FHktIntentEventEntry& Entry : EventHistory)
            {
                OutEvents.Add(Entry.EventData);
            }
            
            // FlushEvents는 하위 호환을 위해 유지하지만, 이벤트를 지우지 않음
            // 대신 경고 로그 출력
            UE_LOG(LogTemp, Warning, TEXT("[SlidingWindowChannel] FlushEvents is deprecated. Use FetchNewEvents instead."));
            
            return true;
        }
        PRAGMA_ENABLE_DEPRECATION_WARNINGS

    private:
        void CleanupOldEvents()
        {
            const double CurrentTime = FPlatformTime::Seconds();
            
            // 1. 시간 기반 만료 처리
            EventHistory.RemoveAll([&](const FHktIntentEventEntry& Entry) {
                return (CurrentTime - Entry.Timestamp) > RetentionTime;
            });
            
            // 2. 버퍼 크기 기반 만료 처리 (오래된 것부터 제거)
            while (EventHistory.Num() > MaxBufferSize)
            {
                EventHistory.RemoveAt(0);
            }
        }

    private:
        TArray<FHktIntentEventEntry> EventHistory; // 절대 바로 지우지 않음!
        int64 GlobalSequenceGenerator;
        int32 MaxBufferSize;
        double RetentionTime; // 초 단위
    };
}

void UHktIntentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ChannelMap.Reset();
    
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

TSharedRef<IHktIntentChannel> UHktIntentSubsystem::CreateOrGetChannel(int32 InChannelId)
{
    return ChannelMap.FindOrAdd(InChannelId, MakeShared<Internal::FSlidingWindowChannel>());
}

TSharedPtr<IHktIntentChannel> UHktIntentSubsystem::GetChannel(int32 InChannelId)
{
    if (TSharedRef<IHktIntentChannel>* ChannelPtr = ChannelMap.Find(InChannelId))
    {
        return *ChannelPtr;
    }

    return nullptr;
}
