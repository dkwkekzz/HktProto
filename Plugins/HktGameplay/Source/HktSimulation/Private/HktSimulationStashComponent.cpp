// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktSimulationStashComponent.h"
#include "HktSimulationSubsystem.h"
#include "Net/UnrealNetwork.h"

// ============================================================================
// UHktSimulationStashComponent
// ============================================================================

UHktSimulationStashComponent::UHktSimulationStashComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // FStashBase 초기화
    StashData.Initialize();
    
    SetIsReplicatedByDefault(true);
}

void UHktSimulationStashComponent::BeginPlay()
{
    Super::BeginPlay();
   
    if (HasAuthority())
    {
        // 서버: 이미 최신 상태를 가지고 있으므로 초기화 완료
        bIsSimulationInitialized = true;
    }
    else
    {
        // 클라이언트: 서버에 스냅샷 요청
        ServerRequestSnapshot();
    }

    // SimulationSubsystem에 등록
    if (UHktSimulationSubsystem* SimulationSubsystem = UHktSimulationSubsystem::Get(GetWorld()))
    {
        SimulationSubsystem->RegisterStashComponent(this);
    }
}

void UHktSimulationStashComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // SimulationSubsystem에서 등록 해제
    if (UHktSimulationSubsystem* SimulationSubsystem = UHktSimulationSubsystem::Get(GetWorld()))
    {
        SimulationSubsystem->UnregisterStashComponent(this);
    }

    Super::EndPlay(EndPlayReason);
}

void UHktSimulationStashComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 스냅샷/체크섬 기반 동기화 사용 - 별도 리플리케이션 프로퍼티 없음
}

// ============================================================================
// Snapshot Sync
// ============================================================================

TArray<uint8> UHktSimulationStashComponent::CreateSnapshot() const
{
    TArray<uint8> Data;
    FMemoryWriter Writer(Data);
    
    // const_cast 필요 (SerializeState가 non-const이므로)
    const_cast<FStashBase*>(&StashData)->SerializeState(Writer);
    
    return Data;
}

bool UHktSimulationStashComponent::RestoreFromSnapshot(const TArray<uint8>& SnapshotData)
{
    if (SnapshotData.Num() == 0)
        return false;
    
    FMemoryReader Reader(SnapshotData);
    StashData.SerializeState(Reader);
    
    UE_LOG(LogTemp, Log, TEXT("[Stash] Restored from snapshot: Frame=%d, Entities=%d"), 
        StashData.GetCompletedFrameNumber(), StashData.GetEntityCount());
    
    return true;
}

void UHktSimulationStashComponent::ServerRequestSnapshot_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("[Stash] Client requested snapshot"));
    
    // 요청한 클라이언트에게만 현재 스냅샷 전송
    TArray<uint8> Snapshot = CreateSnapshot();
    ClientReceiveSnapshot(Snapshot);
}

void UHktSimulationStashComponent::ClientReceiveSnapshot_Implementation(const TArray<uint8>& SnapshotData)
{
    UE_LOG(LogTemp, Log, TEXT("[Stash] Receiving snapshot (%d bytes)"), SnapshotData.Num());
    
    if (RestoreFromSnapshot(SnapshotData))
    {
        bIsSimulationInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("[Stash] Initialized from server snapshot"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Stash] Failed to apply snapshot"));
    }
}

// ============================================================================
// Checksum Validation
// ============================================================================

void UHktSimulationStashComponent::MulticastValidateChecksum_Implementation(int32 FrameNumber, uint32 ExpectedChecksum)
{
    // 서버는 검증 불필요
    if (GetOwnerRole() == ROLE_Authority)
        return;
    
    uint32 LocalChecksum = CalculateChecksum();
    
    if (LocalChecksum != ExpectedChecksum)
    {
        UE_LOG(LogTemp, Error, TEXT("[Stash] DESYNC DETECTED at Frame %d! Expected=%u, Local=%u"), 
            FrameNumber, ExpectedChecksum, LocalChecksum);
        
        // 서버에 보고
        ServerReportDesync(FrameNumber, LocalChecksum);
        
        // 델리게이트 호출
        OnDesyncDetected.Broadcast(FrameNumber, ExpectedChecksum, LocalChecksum);
    }
}

void UHktSimulationStashComponent::ServerReportDesync_Implementation(int32 FrameNumber, uint32 ClientChecksum)
{
    uint32 ServerChecksum = CalculateChecksum();
    
    UE_LOG(LogTemp, Error, TEXT("[Stash] Desync detected at Frame %d: Server=%u, Client=%u - Re-syncing..."),
        FrameNumber, ServerChecksum, ClientChecksum);
    
    // 해당 클라이언트에게 스냅샷 재전송
    TArray<uint8> Snapshot = CreateSnapshot();
    ClientReceiveSnapshot(Snapshot);
}

uint32 UHktSimulationStashComponent::CalculateChecksum() const
{
    return StashData.CalculateChecksum();
}