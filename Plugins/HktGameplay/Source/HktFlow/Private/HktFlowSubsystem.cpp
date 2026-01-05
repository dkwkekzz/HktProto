// Copyright (c) 2026 Hkt Games. All Rights Reserved.

#include "HktFlowSubsystem.h"
#include "HktFlowBuilder.h"
#include "HktServiceSubsystem.h"

DEFINE_LOG_CATEGORY(LogHktFlow);

void UHktFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 초기화 시 필요한 기본 팩토리 등록 등을 수행할 수 있습니다.
    if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
    {
        Service->RegisterJobProvider(this);
    }
}

void UHktFlowSubsystem::Deinitialize()
{
	FlowFactories.Empty();
    ActiveFlows.Empty();

    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        Service->UnregisterJobProvider(this);
    }

    Super::Deinitialize();
}

void UHktFlowSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 매 틱마다 이벤트 동기화 및 로직 수행
    UpdateFlows(DeltaTime);
}

TStatId UHktFlowSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UHktFlowSubsystem, STATGROUP_Tickables);
}

void UHktFlowSubsystem::RegisterFlowFactory(FGameplayTag Tag, FFlowFactoryFunc FactoryFunc)
{
    if (Tag.IsValid() && FactoryFunc)
    {
        FlowFactories.Add(Tag, FactoryFunc);
    }
}

void UHktFlowSubsystem::UpdateFlows(float DeltaTime)
{
    // 1. 모든 채널의 프로바이더로부터 변경된 이벤트를 가져옵니다.
	UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld());
    if (!Service)
	{
		return;
	}

	TScriptInterface<IHktIntentEventProvider> Provider = Service->GetIntentEventProvider();
	if (!Provider)
	{
		return;
	}

	// 버퍼 초기화
	TempHistory.Reset();
	int32 SyncedFrame = 0;

	// FlushEvents 호출 (Provider 내부 버퍼 비우기 및 이벤트 히스토리 가져오기)
	const int32 DefaultChannelId = 0;
	const bool bSuccess = Provider->FlushEvents(DefaultChannelId, SyncedFrame, TempHistory);
	if (bSuccess)
	{
		// 변경점 데이터 준비
		FHktFlowChangedData ChangedData;
		ChangedData.SyncedFrame = SyncedFrame;

		// 2. 이벤트 히스토리를 순차적으로 처리 (시뮬레이션을 위한 순서 유지)
		for (const FHktIntentHistoryEntry& HistoryEntry : TempHistory)
		{
			if (HistoryEntry.bIsRemoved)
			{
				// 제거된 이벤트 처리 (Stop -> Remove)
				HandleFlowRemoved(HistoryEntry.Event);
				ChangedData.RemovedFlowEventIds.Add(HistoryEntry.Event.EventId);
				ChangedData.RemovedFlowTags.Add(HistoryEntry.Event.EventTag);
			}
			else
			{
				// 추가된 이벤트 처리 (Create -> Start -> Add)
				HandleFlowAdded(HistoryEntry.Event);
				ChangedData.AddedFlowEventIds.Add(HistoryEntry.Event.EventId);
				ChangedData.AddedFlowTags.Add(HistoryEntry.Event.EventTag);
			}
		}

		// 3. 변경점이 있을 경우 델리게이트 브로드캐스트
		if (ChangedData.HasChanges())
		{
			OnFlowChanged.Broadcast(ChangedData);
		}
	}
}

void UHktFlowSubsystem::HandleFlowAdded(const FHktIntentEvent& NewEvent)
{
    // 이미 존재하는 Flow인지 방어 코드 (중복 생성 방지)
    if (ActiveFlows.Contains(NewEvent.EventId))
    {
        UE_LOG(LogHktFlow, Warning, TEXT("Flow already exists for EventID: %s"), *NewEvent.EventId.ToString());
        return;
    }

    // EventTag에 매칭되는 팩토리 찾기
    // GameplayTag는 계층 구조이므로 Exact Match가 아니더라도 부모 태그로 찾는 로직을 추가할 수 있습니다.
    // 여기서는 단순화를 위해 Exact Match를 사용합니다.
    const FFlowFactoryFunc* Factory = FlowFactories.Find(NewEvent.EventTag);
    if (Factory && *Factory)
    {
        // 팩토리 함수 실행하여 Flow 인스턴스 생성
        TSharedPtr<IHktFlow> NewFlow = (*Factory)();
        if (NewFlow.IsValid())
        {
			FHktFlowBuilder FlowBuilder;
            // Flow 시작 및 관리 컨테이너에 등록
            NewFlow->DefineFlow(FlowBuilder, NewEvent);
            ActiveFlows.Add(NewEvent.EventId, NewFlow);
            
            UE_LOG(LogHktFlow, Verbose, TEXT("Started Flow for Event: %s (Tag: %s)"), *NewEvent.EventId.ToString(), *NewEvent.EventTag.ToString());
        }
    }
    else
    {
        UE_LOG(LogHktFlow, Warning, TEXT("No Flow Factory found for Tag: %s"), *NewEvent.EventTag.ToString());
    }
}

void UHktFlowSubsystem::HandleFlowRemoved(const FHktIntentEvent& RemovedEvent)
{
    TSharedPtr<IHktFlow> RemovedFlow;
    
    // 맵에서 제거하고 소유권을 가져옴
    if (ActiveFlows.RemoveAndCopyValue(RemovedEvent.EventId, RemovedFlow))
    {
        if (RemovedFlow.IsValid())
        {
            // Flow 종료 로직 수행
            RemovedFlow->StopFlow();
            UE_LOG(LogHktFlow, Verbose, TEXT("Stopped Flow for Event: %s"), *RemovedEvent.EventId.ToString());
        }
    }
}