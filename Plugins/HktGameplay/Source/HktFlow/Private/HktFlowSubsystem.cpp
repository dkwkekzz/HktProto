// Copyright (c) 2026 Hkt Games. All Rights Reserved.

#include "HktFlowSubsystem.h"
#include "HktFlowBuilder.h"
#include "HktIntentInterfaces.h"
#include "HktServiceSubsystem.h"

// ============================================================================
// FHktFlowRegistry Implementation
// ============================================================================
void FHktFlowRegistry::RegisterFlow(FGameplayTag Tag, TSharedPtr<IHktFlow> FlowInstance)
{
    UHktFlowSubsystem::RegisterFlowType(Tag, FlowInstance);
}

// ============================================================================
// UHktFlowSubsystem Implementation
// ============================================================================

DEFINE_LOG_CATEGORY(LogHktFlow);

TMap<FGameplayTag, TSharedPtr<IHktFlow>>& UHktFlowSubsystem::GetGlobalFlowRegistry()
{
    static TMap<FGameplayTag, TSharedPtr<IHktFlow>> Registry;
    return Registry;
}

void UHktFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
    {
        Service->RegisterJobProvider(this);
    }
}

void UHktFlowSubsystem::Deinitialize()
{
    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        Service->UnregisterJobProvider(this);
    }

    Super::Deinitialize();
}

void UHktFlowSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateFlows(DeltaTime);
}

TStatId UHktFlowSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UHktFlowSubsystem, STATGROUP_Tickables);
}

void UHktFlowSubsystem::RegisterFlowInstance(FGameplayTag Tag, TSharedPtr<IHktFlow> FlowInstance)
{
    if (Tag.IsValid() && FlowInstance.IsValid())
    {
        GetGlobalFlowRegistry().Add(Tag, FlowInstance);
    }
}

TSharedPtr<IHktFlow> UHktFlowSubsystem::GetFlow(FGameplayTag Tag)
{
    if (TSharedPtr<IHktFlow>* FoundFlow = GetGlobalFlowRegistry().Find(Tag))
    {
        return *FoundFlow;
    }
    return nullptr;
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

    const int32 DefaultChannelId = 0;
    TSharedPtr<IHktIntentChannel> Channel = Provider->GetChannel(DefaultChannelId);
    if (!Channel)
    {
        return;
    }

	// FlushEvents 호출
    TArray<FHktIntentEvent> Buffer;
	if (!Channel->FlushEvents(Buffer))
	{
        return;
	}

    // 2. 이벤트를 순회하며 Flow 정의(Define) 수행
    for (const FHktIntentEvent& Event : Buffer)
    {
        // 전역 레지스트리에서 Flow 인스턴스 가져오기 (CDO 개념 재활용)
        if (TSharedPtr<IHktFlow> Flow = GetFlow(Event.EventTag))
        {
            FHktJobBuilder Builder;
            
            // Flow 정의: Builder에 작업(Job) 목록을 기록
            // Flow 인스턴스는 상태가 없는 메타데이터이므로 안전하게 재사용 가능
            Flow->DefineFlow(Builder, Event);

            // TODO: Builder 내용을 JobSystem으로 전달
        }
    }
}
