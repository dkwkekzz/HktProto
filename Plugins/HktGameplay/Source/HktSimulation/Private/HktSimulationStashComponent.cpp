// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktSimulationStashComponent.h"
#include "HktSimulationSubsystem.h"
#include "Processors/HktCleanupProcessor.h"
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
// UHktSimulationStashComponent
// ============================================================================

UHktSimulationStashComponent::UHktSimulationStashComponent()
{
    SetIsReplicatedByDefault(true);
}

void UHktSimulationStashComponent::BeginPlay()
{
    Super::BeginPlay();
   
    // Server는 이미 최신 상태를 가지고 있으므로 초기화된 것으로 간주
    if (HasAuthority())
    {
        bIsSimulationInitialized = true;
    }

    // FFastArraySerializer 콜백용 소유자 설정
    SimulationState.UnitStates.OwnerComponent = this;

    // SimulationSubsystem에 등록
    if (UHktSimulationSubsystem* SimulationSubsystem = UHktSimulationSubsystem::Get(GetWorld()))
    {
        SimulationSubsystem->RegisterSimulationStashComponent(this);
    }
}

void UHktSimulationStashComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // SimulationSubsystem에서 등록 해제
    if (UHktSimulationSubsystem* SimulationSubsystem = UHktSimulationSubsystem::Get(GetWorld()))
    {
        SimulationSubsystem->UnregisterSimulationStashComponent(this);
    }

    Super::EndPlay(EndPlayReason);
}

void UHktSimulationStashComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // COND_InitialOnly: 최초 접속 시에만 상태를 전송
    // 이후 변경은 RPC나 별도 시스템으로 처리
    DOREPLIFETIME_CONDITION(UHktSimulationStashComponent, SimulationState, COND_InitialOnly);
}

// ============================================================================
// 외부 API
// ============================================================================

void UHktSimulationStashComponent::UpdateCompletedState(const FHktSimulationState& CompletedState)
{
    if (!ensure(HasAuthority()))
    {
        return;
    }

    // 서버는 늦게 들어오는 클라이언트(Late Join)를 위해 SimulationState를 최신으로 갱신
    // COND_InitialOnly 설정 덕분에 기존 클라이언트에게는 복제되지 않아 부하가 없음
    SimulationState.CompletedFrameNumber = CompletedState.CompletedFrameNumber;
    SimulationState.UnitStates = CompletedState.UnitStates;
}

void UHktSimulationStashComponent::OnSimulationEventCompleted(const FHktCompletedEvent& CompletedEvent)
{
    // 완료 이벤트를 델리게이트로 전파
    bool bSuccess = (CompletedEvent.FinalState == EHktVMState::Finished);
    OnEventCompleted.Broadcast(CompletedEvent.EventID, bSuccess);
    
    UE_LOG(LogTemp, Verbose, TEXT("[SimulationStashComponent] Event %d completed, success=%d"),
        CompletedEvent.EventID, bSuccess);
}

// ============================================================================
// 복제 콜백
// ============================================================================

void UHktSimulationStashComponent::OnRep_SimulationState()
{
    // [Client Only]
    // 서버에서 최초 접속 시 받은 스냅샷으로 월드를 초기화
    UE_LOG(LogTemp, Log, TEXT("[SimulationStashComponent] Client Initial Snapshot Received. Frame: %d"),
        SimulationState.CompletedFrameNumber);

    // 시뮬레이션 초기화 완료 플래그 설정
    bIsSimulationInitialized = true;

    // TODO: SimulationSubsystem을 통해 월드 상태를 강제 동기화하는 로직
    // 예: UHktSimulationSubsystem::Get(GetWorld())->ForceSyncState(SimulationState);
}
