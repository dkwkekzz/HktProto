// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "HktService/Public/HktIntentInterface.h"
#include "HktService/Public/HktSimulationInterface.h"
#include "Core/HktVMTypes.h"
#include "HktSimulationStashComponent.generated.h"

// 전방 선언
class UHktSimulationStashComponent;
struct FHktCompletedEvent;

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
    TObjectPtr<UHktSimulationStashComponent> OwnerComponent = nullptr;

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

/**
 * 시뮬레이션 상태 (스냅샷 데이터)
 */
USTRUCT()
struct FHktSimulationState
{
    GENERATED_BODY()

    FHktSimulationState() = default;

    /** 완료된 시뮬레이션 프레임 번호 */
    UPROPERTY()
    int32 CompletedFrameNumber = 0;

    /** 유닛 상태 배열 */
    UPROPERTY()
    FHktUnitStateArray UnitStates;
};

/**
 * 시뮬레이션 이벤트 완료 델리게이트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSimulationEventCompleted, int32, EventID, bool, bSuccess);

/**
 * UHktSimulationStashComponent
 * 
 * Actor에 부착되어 시뮬레이션 상태를 관리하고 복제하는 컴포넌트
 * 
 * 책임:
 * - 시뮬레이션 상태 스냅샷 저장/복제
 * - 완료된 이벤트 알림 수신
 * - 클라이언트 Late-Join 지원
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

    // ========================================================================
    // 외부 API
    // ========================================================================
    
    /**
     * 완료된 시뮬레이션 결과 업데이트 (서버에서 호출)
     */
    void UpdateCompletedState(const FHktSimulationState& CompletedState);
    
    /**
     * 시뮬레이션 이벤트 완료 알림 (CleanupProcessor에서 호출)
     */
    void OnSimulationEventCompleted(const FHktCompletedEvent& CompletedEvent);
    
    /**
     * 현재 시뮬레이션 상태 가져오기
     */
    const FHktSimulationState& GetSimulationState() const { return SimulationState; }
    
    // ========================================================================
    // 이벤트
    // ========================================================================
    
    /** 시뮬레이션 이벤트 완료 시 호출되는 델리게이트 */
    UPROPERTY(BlueprintAssignable, Category = "Simulation")
    FOnSimulationEventCompleted OnEventCompleted;

protected:
    UFUNCTION()
    void OnRep_SimulationState();

protected:
    /** 복제되는 시뮬레이션 상태 */
    UPROPERTY(ReplicatedUsing = OnRep_SimulationState)
    FHktSimulationState SimulationState;

    /** 시뮬레이션 초기화 여부 (클라이언트용) */
    bool bIsSimulationInitialized = false;
};
