// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/VMStore.h"
#include "HktSimulationStashComponent.generated.h"

/**
 * UHktSimulationStashComponent
 * 
 * Actor에 부착되어 시뮬레이션 상태를 관리하고 복제하는 컴포넌트
 * 
 * FStashBase를 소유하고 네트워크 동기화 기능을 추가함
 * 
 * 책임:
 * - FStashBase 데이터 소유 및 관리
 * - 시뮬레이션 상태 스냅샷 저장/복제
 * - 완료된 이벤트 알림 수신
 * - 클라이언트 Late-Join 지원
 * 
 * [클라이언트 접속]
 *     ↓
 * BeginPlay() → ServerRequestSnapshot()
 *     ↓
 * [서버] 스냅샷 생성 → ClientReceiveSnapshot()
 *     ↓
 * [클라이언트] 스냅샷 복원 → bIsSimulationInitialized = true
 *     ↓
 * [이후 주기적으로]
 * 서버: MulticastValidateChecksum()
 *     ↓
 * 클라이언트: 체크섬 비교 (일치하면 OK)
 *     ↓
 * [불일치 시]
 * ServerReportDesync() → 자동으로 스냅샷 재전송
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HKTSIMULATION_API UHktSimulationStashComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHktSimulationStashComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ========== Stash Access ==========
    
    /** Core에서 사용하는 FStashBase 접근자 */
    FStashBase* GetStash() { return &StashData; }
    const FStashBase* GetStash() const { return &StashData; }

    // ========== Snapshot Sync (초기 동기화) ==========
    
    /** 클라이언트 → 서버: 스냅샷 요청 (최초 접속 또는 재동기화 시) */
    UFUNCTION(Server, Reliable)
    void ServerRequestSnapshot();
    
    /** 서버 → 클라이언트: 스냅샷 전송 */
    UFUNCTION(Client, Reliable)
    void ClientReceiveSnapshot(const TArray<uint8>& SnapshotData);
    
    /** 초기화 완료 여부 (스냅샷 수신 완료) */
    bool IsInitialized() const { return bIsSimulationInitialized; }
    
    // ========== Checksum Validation (디싱크 감지) ==========
    
    /** 현재 상태의 체크섬 계산 */
    uint32 CalculateChecksum() const;
    
    /** 서버: 클라이언트들에게 체크섬 검증 요청 */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastValidateChecksum(int32 FrameNumber, uint32 ExpectedChecksum);
    
    /** 클라이언트: 체크섬 불일치 시 서버에 보고 */
    UFUNCTION(Server, Reliable)
    void ServerReportDesync(int32 FrameNumber, uint32 ClientChecksum);
    
    /** 디싱크 감지 시 호출되는 델리게이트 */
    DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDesyncDetected, int32 /*FrameNumber*/, uint32 /*Expected*/, uint32 /*Actual*/);
    FOnDesyncDetected OnDesyncDetected;
    
private:
    TArray<uint8> CreateSnapshot() const;
    bool RestoreFromSnapshot(const TArray<uint8>& SnapshotData);

    /** 핵심 데이터 저장소 */
    FStashBase StashData;
    
    /** 초기화 완료 여부 (클라이언트: 스냅샷 수신 완료, 서버: 항상 true) */
    bool bIsSimulationInitialized = false;
};
