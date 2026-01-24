// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "HktService/Public/HktIntentInterface.h"
#include "HktService/Public/HktSimulationInterface.h"
#include "HktIntentEventComponent.generated.h"

class UHktIntentEventComponent;

// ... (FHktUnitStateItem, FHktUnitStateArray, FHktSimulationModel 등은 기존과 동일) ...

/**
 * 개별 유닛의 상태 데이터 (FFastArraySerializerItem 기반)
 */
USTRUCT()
struct FHktUnitStateItem : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FHktUnitStateItem() = default;

    explicit FHktUnitStateItem(const FHktUnitHandle& InHandle)
        : UnitHandle(InHandle)
    {
        Attributes.SetNumZeroed(static_cast<int32>(EHktAttribute::Count));
    }

    UPROPERTY()
    FHktUnitHandle UnitHandle;

    UPROPERTY()
    TArray<int32> Attributes;

    void SetAttribute(EHktAttribute Attr, int32 Value)
    {
        const int32 Index = static_cast<int32>(Attr);
        if (Attributes.IsValidIndex(Index)) Attributes[Index] = Value;
    }

    int32 GetAttribute(EHktAttribute Attr) const
    {
        const int32 Index = static_cast<int32>(Attr);
        return Attributes.IsValidIndex(Index) ? Attributes[Index] : 0;
    }

    void ModifyAttribute(EHktAttribute Attr, int32 Delta)
    {
        const int32 Index = static_cast<int32>(Attr);
        if (Attributes.IsValidIndex(Index)) Attributes[Index] += Delta;
    }

    void PostReplicatedAdd(const struct FHktUnitStateArray& InArray) { }
    void PostReplicatedChange(const struct FHktUnitStateArray& InArray) { }
    void PreReplicatedRemove(const struct FHktUnitStateArray& InArray) { }
};

USTRUCT()
struct FHktUnitStateArray : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FHktUnitStateItem> Items;

    UPROPERTY(NotReplicated)
    TObjectPtr<UHktIntentEventComponent> OwnerComponent = nullptr;

    void AddOrUpdateUnit(const FHktUnitHandle& Handle, const TArray<int32>& Attributes);
    void RemoveUnit(const FHktUnitHandle& Handle);
    FHktUnitStateItem* FindByHandle(const FHktUnitHandle& Handle);
    void Clear();

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FHktUnitStateItem, FHktUnitStateArray>(Items, DeltaParms, *this);
    }
};

template<>
struct TStructOpsTypeTraits<FHktUnitStateArray> : public TStructOpsTypeTraitsBase2<FHktUnitStateArray>
{
    enum { WithNetDeltaSerializer = true };
};

USTRUCT()
struct FHktSimulationModel
{
    GENERATED_BODY()
    FHktSimulationModel() = default;

    UPROPERTY()
    int32 ProcessedFrameNumber = 0;

    UPROPERTY()
    FHktUnitStateArray UnitStates;
};

/**
 * 시뮬레이션 상태 (이제 스냅샷 데이터만 보유)
 * ProcessingBatches는 RPC로 직접 전송하므로 여기서 제거되었습니다.
 */
USTRUCT()
struct FHktSimulationState
{
    GENERATED_BODY()

    FHktSimulationState() = default;

    /** * 완료된 시뮬레이션 결과 (기준점).
     * 최초 1회만 동기화되어 클라이언트 시뮬레이션의 시작점이 됩니다.
     */
    UPROPERTY()
    FHktSimulationModel CompletedModel;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HKTINTENT_API UHktIntentEventComponent : public UActorComponent, public IHktIntentEventProvider
{
    GENERATED_BODY()

public:
    UHktIntentEventComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** [Client] 의도 전송 */
    UFUNCTION(BlueprintCallable, Category = "Hkt|Intent")
    void NotifyIntent(const FHktIntentEvent& IntentEvent);

    /** [Server] 배치 처리 시작 (RPC 호출 트리거) */
    void NotifyIntentBatch(int32 FrameNumber);

    // --- IHktIntentEventProvider 구현 ---
    virtual TArray<FHktIntentEventBatch> GetPendingBatches() const override;
    virtual void NotifyCompletedSimulation(const FHktSimulationModel& SimulationResult) override;
    virtual bool HasAuthority() const override { return GetOwner() && GetOwner()->HasAuthority(); }

protected:
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_NotifyIntent(FHktIntentEvent IntentEvent);
   
    /**
     * [Client] 서버로부터 배치를 수신하는 RPC (Reliable Multicast)
     * 리플리케이션 대신 이 함수를 통해 입력을 동기화합니다.
     */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SyncBatch(const FHktIntentEventBatch& Batch);

    /** RepNotify: 초기 스냅샷 수신 시 호출 */
    UFUNCTION()
    void OnRep_SimulationState();

private:
    /**
     * [Safety] 배치 정합성 보정 함수 (Gatekeeper)
     *
     * 역할 1: SimulationState(스냅샷)보다 과거이거나 같은 프레임의 배치가 LocalProcessingBatches에 있다면 제거 (중복 방지)
     * 역할 2: LocalProcessingBatches를 프레임 순서대로 정렬 (패킷 순서 뒤섞임 방지)
     *
     * 호출 시점:
     * 1. OnRep_SimulationState 직후: 스냅샷이 늦게 도착했을 때, 이미 쌓여있던 미래의 배치들 중 중복된 것 제거
     * 2. Multicast_SyncBatch 수신 시: 초기화 이후라면 즉시 검사하여 유효한 배치만 큐에 추가
     */
    void PruneInvalidBatches();

    /** [Server-Only] 수집 중인 의도들 */
    TArray<FHktIntentEvent> PendingIntentEvents;

    /** * [Client/Server] 로컬에서 처리해야 할 배치 큐
     * Client: 초기화 전에는 버퍼(Jitter Buffer) 역할을 하며, 초기화 후에는 실행 대기열이 됩니다.
     */
    TArray<FHktIntentEventBatch> LocalProcessingBatches;

    /**
     * [Replicated] 초기화용 스냅샷.
     * COND_InitialOnly로 설정되어 최초 접속 시에만 전송됩니다.
     *
     * [Safety Note]
     * 이 데이터는 '절대적 기준점(Baseline)'입니다.
     * RPC로 도착한 배치가 이 스냅샷의 ProcessedFrameNumber보다 작거나 같다면,
     * 그 배치는 이미 이 스냅샷에 반영된 것이므로 반드시 폐기되어야 합니다. (PruneInvalidBatches 참조)
     */
    UPROPERTY(ReplicatedUsing = OnRep_SimulationState)
    FHktSimulationState SimulationState;

    /** [Client-Only] 시뮬레이션 초기화 여부 플래그 */
    bool bIsSimulationInitialized = false;
};

