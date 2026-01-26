// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktIntentEventComponent.h"
#include "HktIntentEventSubsystem.h"
#include "Net/UnrealNetwork.h"

// ============================================================================
// UHktIntentEventComponent
// ============================================================================

UHktIntentEventComponent::UHktIntentEventComponent()
{
    SetIsReplicatedByDefault(true);
}

void UHktIntentEventComponent::BeginPlay()
{
    Super::BeginPlay();
   
    // IntentEventSubsystem에 등록
    if (UHktIntentEventSubsystem* IntentEventSubsystem = UHktIntentEventSubsystem::Get(GetWorld()))
    {
        IntentEventSubsystem->RegisterIntentEventComponent(this);
    }
}

void UHktIntentEventComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // IntentEventSubsystem에서 등록 해제
    if (UHktIntentEventSubsystem* IntentEventSubsystem = UHktIntentEventSubsystem::Get(GetWorld()))
    {
        IntentEventSubsystem->UnregisterIntentEventComponent(this);
    }

    Super::EndPlay(EndPlayReason);
}

void UHktIntentEventComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // COND_InitialOnly: 최초 접속 시에만 상태를 전송하고, 이후에는 RPC로만 동기화하여 대역폭 절약
    DOREPLIFETIME_CONDITION(UHktIntentEventComponent, ProcessingIntentEvents, COND_InitialOnly);
}

// ============================================================================
// Step 1: Client -> Server (Intent Submission)
// ============================================================================

void UHktIntentEventComponent::CommitIntent(const FHktIntentEvent& IntentEvent)
{
    Server_CommitIntent(IntentEvent);
}

void UHktIntentEventComponent::PushIntentBatch(int32 FrameNumber)
{
    if (!ensure(HasAuthority()))
    {
        return;
    }

    // 1. 배치를 생성합니다.
    FHktIntentEventBatch Batch(FrameNumber);
    Batch.Events = MoveTemp(PendingIntentEvents);
    PendingIntentEvents.Reset();

    // 2. 서버 로컬 큐에 추가 (서버 시뮬레이션용)
    ProcessingIntentEvents.Append(Batch.Events);

    // 3. [RPC] 클라이언트들에게 배치 전송 (동기화 핵심)
    // Reliable Multicast로 모든 클라이언트가 놓치지 않고 받도록 함
    Multicast_PushIntentBatch(Batch);

    UE_LOG(LogTemp, Log, TEXT("[HktIntentEventComponent] Server Broadcasted Batch Frame: %d"), FrameNumber);
}

void UHktIntentEventComponent::PullIntentEvents(int32 CompletedFrameNumber, TArray<FHktIntentEvent>& OutIntentEvents)
{
    // 기준점: 스냅샷이 보장하는 프레임
    // 1. 유효하지 않은(이미 스냅샷에 포함된) 배치 제거
    int32 RemovedCount = ProcessingIntentEvents.RemoveAll([CompletedFrameNumber](const FHktIntentEvent& IntentEvent)
    {
        return IntentEvent.FrameNumber <= CompletedFrameNumber;
    });

    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HktIntentEventComponent] Pruned %d outdated intent events (Snapshot Frame: %d)"),
            RemovedCount, SnapshotFrame);
    }

    OutIntentEvents.Append(ProcessingIntentEvents);
}

void UHktIntentEventComponent::Multicast_PushIntentBatch_Implementation(const FHktIntentEventBatch& Batch)
{
    // Server는 이미 PushIntentBatch에서 로컬 큐에 넣었으므로 패스
    if (HasAuthority())
    {
        return;
    }

    // [Client]
    // 배치를 일단 큐에 넣습니다. (초기화 전이라면 지터 버퍼 역할을 함)
    ProcessingIntentEvents.Append(Batch.Events);
   
    UE_LOG(LogTemp, Verbose, TEXT("[HktIntentEventComponent] Client Received RPC Batch Frame: %d. Queue Size: %d"),
        Batch.FrameNumber, ProcessingIntentEvents.Num());
}

void UHktIntentEventComponent::Server_CommitIntent_Implementation(FHktIntentEvent IntentEvent)
{
    PendingIntentEvents.Add(MoveTemp(IntentEvent));
}

bool UHktIntentEventComponent::Server_CommitIntent_Validate(FHktIntentEvent IntentEvent)
{
    return true;
}

void UHktIntentEventComponent::OnRep_ProcessingIntentEvents()
{
    // [Client Only]
    UE_LOG(LogTemp, Log, TEXT("[HktIntentEventComponent] Client Initial Intent Events Received. Queue Size: %d"),
        ProcessingIntentEvents.Num());
}

