// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktService/Public/HktIntentEventComponent.h"
#include "HktIntentGameMode.h"
#include "HktSimulation/Private/HktSimulationSubsystem.h"
#include "Net/UnrealNetwork.h"

// ============================================================================
// FHktUnitStateArray - FFastArraySerializer 구현
// ============================================================================

void FHktUnitStateArray::AddOrUpdateUnit(const FHktUnitHandle& Handle, const TArray<int32>& Attributes)
{
    if (FHktUnitStateItem* Existing = FindByHandle(Handle))
    {
        Existing->Attributes = Attributes;
        MarkItemDirty(*Existing);
    }
    else
    {
        FHktUnitStateItem& NewItem = Items.AddDefaulted_GetRef();
        NewItem.UnitHandle = Handle;
        NewItem.Attributes = Attributes;
        MarkItemDirty(NewItem);
    }
}

void FHktUnitStateArray::RemoveUnit(const FHktUnitHandle& Handle)
{
    for (int32 i = Items.Num() - 1; i >= 0; --i)
    {
        if (Items[i].UnitHandle == Handle)
        {
            Items.RemoveAt(i);
            MarkArrayDirty();
            break;
        }
    }
}

FHktUnitStateItem* FHktUnitStateArray::FindByHandle(const FHktUnitHandle& Handle)
{
    return Items.FindByPredicate([&Handle](const FHktUnitStateItem& Item)
    {
        return Item.UnitHandle == Handle;
    });
}

void FHktUnitStateArray::Clear()
{
    Items.Reset();
    MarkArrayDirty();
}

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
   
    // Server는 이미 최신 상태를 가지고 있으므로 초기화된 것으로 간주
    if (HasAuthority())
    {
        bIsSimulationInitialized = true;
    }

    // FFastArraySerializer 콜백용 소유자 설정
    SimulationState.CompletedModel.UnitStates.OwnerComponent = this;

    // GameMode에 등록 (서버 전용, 프레임 동기화용)
    if (AHktIntentGameMode* GameMode = AHktIntentGameMode::Get(GetWorld()))
    {
        GameMode->RegisterIntentEventComponent(this);
    }

    // SimulationSubsystem에 Provider로 등록
    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        Service->RegisterIntentEventProvider(this);
    }
}

void UHktIntentEventComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // SimulationSubsystem에서 등록 해제
    if (UHktServiceSubsystem* Service = UHktServiceSubsystem::Get(GetWorld()))
    {
        Service->UnregisterIntentEventProvider(this);
    }

    // GameMode에서 등록 해제
    if (AHktIntentGameMode* GameMode = AHktIntentGameMode::Get(GetWorld()))
    {
        GameMode->UnregisterIntentEventComponent(this);
    }

    Super::EndPlay(EndPlayReason);
}

void UHktIntentEventComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // COND_InitialOnly: 최초 접속 시에만 상태를 전송하고, 이후에는 RPC로만 동기화하여 대역폭 절약
    DOREPLIFETIME_CONDITION(UHktIntentEventComponent, SimulationState, COND_InitialOnly);
}

// ============================================================================
// Step 1: Client -> Server (Intent Submission)
// ============================================================================

void UHktIntentEventComponent::NotifyIntent(const FHktIntentEvent& IntentEvent)
{
    Server_NotifyIntent(IntentEvent);
}

void UHktIntentEventComponent::NotifyIntentBatch(int32 FrameNumber)
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
    LocalProcessingBatches.Add(Batch);

    // 3. [RPC] 클라이언트들에게 배치 전송 (동기화 핵심)
    // Reliable Multicast로 모든 클라이언트가 놓치지 않고 받도록 함
    Multicast_SyncBatch(Batch);

    UE_LOG(LogTemp, Log, TEXT("[HktIntentEventComponent] Server Broadcasted Batch Frame: %d"), FrameNumber);
}

void UHktIntentEventComponent::Multicast_SyncBatch_Implementation(const FHktIntentEventBatch& Batch)
{
    // Server는 이미 NotifyIntentBatch에서 로컬 큐에 넣었으므로 패스
    if (HasAuthority())
    {
        return;
    }

    // [Client]
    // 배치를 일단 큐에 넣습니다. (초기화 전이라면 지터 버퍼 역할을 함)
    LocalProcessingBatches.Add(Batch);
   
    // 초기화가 완료된 상태라면 즉시 유효성 검증 및 정렬 수행
    if (bIsSimulationInitialized)
    {
        PruneInvalidBatches();
    }

    UE_LOG(LogTemp, Verbose, TEXT("[HktIntentEventComponent] Client Received RPC Batch Frame: %d. Queue Size: %d"),
        Batch.FrameNumber, LocalProcessingBatches.Num());
}

TArray<FHktIntentEventBatch> UHktIntentEventComponent::GetPendingBatches() const
{
    return LocalProcessingBatches;
}

void UHktIntentEventComponent::NotifyCompletedSimulation(const FHktSimulationModel& SimulationResult)
{
    // 처리된 프레임 이하의 배치들 제거 (서버/클라이언트 공통 로직)
    LocalProcessingBatches.RemoveAll([&SimulationResult](const FHktIntentEventBatch& Batch) {
        return Batch.FrameNumber <= SimulationResult.ProcessedFrameNumber;
    });

    // [Server Only]
    // 서버는 늦게 들어오는 클라이언트(Late Join)를 위해 SimulationState를 최신으로 갱신해 둡니다.
    // COND_InitialOnly 설정 덕분에 기존 클라이언트에게는 복제되지 않아 부하가 없습니다.
    if (HasAuthority())
    {
        SimulationState.CompletedModel = SimulationResult;
    }
}

void UHktIntentEventComponent::Server_NotifyIntent_Implementation(FHktIntentEvent IntentEvent)
{
    PendingIntentEvents.Add(MoveTemp(IntentEvent));
}

bool UHktIntentEventComponent::Server_NotifyIntent_Validate(FHktIntentEvent IntentEvent)
{
    return true;
}

void UHktIntentEventComponent::OnRep_SimulationState()
{
    // [Client Only]
    // 서버에서 최초 접속 시 받은 스냅샷으로 월드를 초기화합니다.
    UE_LOG(LogTemp, Log, TEXT("[HktIntentEventComponent] Client Initial Snapshot Received. Frame: %d"),
        SimulationState.CompletedModel.ProcessedFrameNumber);

    // 시뮬레이션 초기화 완료 플래그 설정
    bIsSimulationInitialized = true;

    // 중요: 스냅샷이 도착했으므로, 그동안 쌓여있던 RPC 배치들을 검증합니다.
    // 스냅샷보다 과거의 배치는 제거하고, 순서가 섞였다면 정렬합니다.
    PruneInvalidBatches();
   
    // TODO: 여기서 SimulationSubsystem 등을 통해 월드 상태를 강제 동기화(Set) 하는 로직이 필요할 수 있습니다.
    // 예: UHktSimulationProcessSubsystem::Get(GetWorld())->ForceSyncState(SimulationState.CompletedModel);
}

void UHktIntentEventComponent::PruneInvalidBatches()
{
    // 기준점: 스냅샷이 보장하는 프레임
    const int32 SnapshotFrame = SimulationState.CompletedModel.ProcessedFrameNumber;

    // 1. 유효하지 않은(이미 스냅샷에 포함된) 배치 제거
    int32 RemovedCount = LocalProcessingBatches.RemoveAll([SnapshotFrame](const FHktIntentEventBatch& Batch)
    {
        return Batch.FrameNumber <= SnapshotFrame;
    });

    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HktIntentEventComponent] Pruned %d outdated batches (Snapshot Frame: %d)"),
            RemovedCount, SnapshotFrame);
    }

    // 2. 프레임 순서대로 정렬 (패킷 순서 뒤바뀜 보정)
    LocalProcessingBatches.Sort([](const FHktIntentEventBatch& A, const FHktIntentEventBatch& B)
    {
        return A.FrameNumber < B.FrameNumber;
    });
}

