// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktJobProcessSubsystem.h"
#include "HktServiceSubsystem.h"
#include "IHktJobProvider.h"
#include "HktFlowInterfaces.h"
#include "HktJobContainer.h"
#include "HktFlowSubsystem.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogHktJobProcess, Log, All);

void UHktJobProcessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Ensure HktService subsystem is initialized first
	if (UHktServiceSubsystem* Service = Collection.InitializeDependency<UHktServiceSubsystem>())
	{
		// JobProvider가 등록될 때까지 대기할 수 있도록 지연 구독 처리
		// 현재는 Initialize 시점에 바로 구독 시도
		if (TScriptInterface<IHktJobProvider> JobProvider = Service->GetJobProvider())
		{
			FlowChangedHandle = JobProvider->GetOnFlowChangedDelegate().AddUObject(
				this, &UHktJobProcessSubsystem::HandleFlowChanged);
			
			UE_LOG(LogHktJobProcess, Log, TEXT("Subscribed to JobProvider's FlowChanged delegate"));
		}
	}
}

void UHktJobProcessSubsystem::Deinitialize()
{
	// 델리게이트 구독 해제
	if (FlowChangedHandle.IsValid())
	{
		if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
		{
			if (TScriptInterface<IHktJobProvider> JobProvider = Service->GetJobProvider())
			{
				JobProvider->GetOnFlowChangedDelegate().Remove(FlowChangedHandle);
			}
		}
		FlowChangedHandle.Reset();
	}

	JobHandlers.Empty();
	ExecutingJobIds.Empty();
	Super::Deinitialize();
}

UHktJobProcessSubsystem* UHktJobProcessSubsystem::Get(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UHktJobProcessSubsystem>();
	}
	return nullptr;
}

void UHktJobProcessSubsystem::HandleFlowChanged(const FHktFlowChangedData& ChangedData)
{
	UE_LOG(LogHktJobProcess, Verbose, TEXT("FlowChanged - Frame: %d, Added: %d, Removed: %d"),
		ChangedData.SyncedFrame,
		ChangedData.AddedFlowEventIds.Num(),
		ChangedData.RemovedFlowEventIds.Num());

	// 추가된 Flow 처리
	for (int32 i = 0; i < ChangedData.AddedFlowEventIds.Num(); ++i)
	{
		const int32 EventId = ChangedData.AddedFlowEventIds[i];
		const FGameplayTag& Tag = ChangedData.AddedFlowTags[i];
		
		ExecutingJobIds.Add(EventId);
		UE_LOG(LogHktJobProcess, Verbose, TEXT("  Added Flow - EventId: %d, Tag: %s"), EventId, *Tag.ToString());
	}

	// 제거된 Flow 처리
	for (int32 i = 0; i < ChangedData.RemovedFlowEventIds.Num(); ++i)
	{
		const int32 EventId = ChangedData.RemovedFlowEventIds[i];
		const FGameplayTag& Tag = ChangedData.RemovedFlowTags[i];
		
		ExecutingJobIds.Remove(EventId);
		UE_LOG(LogHktJobProcess, Verbose, TEXT("  Removed Flow - EventId: %d, Tag: %s"), EventId, *Tag.ToString());
	}
}

