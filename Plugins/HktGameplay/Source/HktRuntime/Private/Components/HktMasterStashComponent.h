#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HktRuntimeTypes.h"
#include "HktRuntimeInterfaces.h"
#include "HktMasterStashComponent.generated.h"

/**
 * UMasterStashComponent - 서버 전용 전체 데이터 저장소
 * 
 * 모든 엔티티의 "진실의 원천(Source of Truth)"
 * 클라이언트에는 IntentEvent + AttachedSnapshots로 필요한 것만 전송
 * 
 * IStashInterface를 구현하여 HktCore의 VM과 직접 연결 가능
 */
UCLASS(ClassGroup=(HktSimulation), meta=(BlueprintSpawnableComponent))
class HKTRUNTIME_API UHktMasterStashComponent : public UActorComponent, public IStashInterface
{
    GENERATED_BODY()

public:
    UHktMasterStashComponent();
    
    // ========== IStashInterface Implementation ==========
    virtual FHktEntityId AllocateEntity() override;
    virtual void FreeEntity(FHktEntityId Entity) override;
    virtual bool IsValidEntity(FHktEntityId Entity) const override;
    virtual int32 GetProperty(FHktEntityId Entity, uint16 PropertyId) const override;
    virtual void SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value) override;
    
    // ========== Entity Management (추가 기능) ==========
    int32 GetEntityCount() const;
    
    // ========== Batch Operations ==========
    struct FPendingWrite
    {
        FHktEntityId Entity;
        uint16 PropertyId;
        int32 Value;
    };
    void ApplyWrites(const TArray<FPendingWrite>& Writes);
    
    // ========== Frame Management ==========
    int32 GetCompletedFrameNumber() const { return CompletedFrameNumber; }
    void MarkFrameCompleted(int32 FrameNumber);
    bool ValidateEntityFrame(FHktEntityId Entity, int32 FrameNumber) const;
    
    // ========== Snapshot & Delta ==========
    
    /** 특정 엔티티의 스냅샷 생성 */
    FHktEntitySnapshot CreateEntitySnapshot(FHktEntityId Entity) const;
    
    /** 여러 엔티티의 스냅샷 생성 */
    TArray<FHktEntitySnapshot> CreateSnapshots(const TArray<FHktEntityId>& Entities) const;
    
    /** 전체 상태 직렬화 (디버그/저장용) */
    TArray<uint8> SerializeFullState() const;
    void DeserializeFullState(const TArray<uint8>& Data);
    
    // ========== Position Access ==========
    
    /** 엔티티 위치 조회 (Relevancy 계산용) */
    bool TryGetPosition(FHktEntityId Entity, FVector& OutPosition) const;
    
    /** 엔티티 위치 설정 */
    void SetPosition(FHktEntityId Entity, const FVector& Position);
    
    // ========== Checksum ==========
    uint32 CalculateChecksum() const;
    
    // ========== Change Tracking ==========
    
    /** 이번 프레임에 변경된 엔티티 목록 */
    const TSet<FHktEntityId>& GetDirtyEntities() const { return DirtyEntities; }
    
    /** 변경 추적 초기화 (프레임 끝에 호출) */
    void ClearDirtyFlags();
    
    // ========== Iteration ==========
    template<typename Func>
    void ForEachEntity(Func&& Callback) const;
    
    template<typename Func>
    void ForEachEntityInRadius(FHktEntityId Center, int32 RadiusCm, Func&& Callback) const;

private:
    void MarkDirty(FHktEntityId Entity);
    
    static constexpr int32 MaxEntities = 1024;
    static constexpr int32 MaxProperties = 256;
    
    /** SOA 레이아웃: Properties[PropertyId][FHktEntityId] */
    TArray<TArray<int32>> Properties;
    TBitArray<> ValidEntities;
    TArray<FHktEntityId> FreeList;
    FHktEntityId NextFHktEntityId = 0;
    int32 CompletedFrameNumber = 0;
    TArray<int32> EntityCreationFrame;
    
    /** 변경 추적 */
    TSet<FHktEntityId> DirtyEntities;
};

// ============================================================================
// Template Implementation
// ============================================================================

template<typename Func>
void UHktMasterStashComponent::ForEachEntity(Func&& Callback) const
{
    for (FHktEntityId E = 0; E < MaxEntities; ++E)
    {
        if (ValidEntities[E])
        {
            Callback(E);
        }
    }
}

template<typename Func>
void UHktMasterStashComponent::ForEachEntityInRadius(FHktEntityId Center, int32 RadiusCm, Func&& Callback) const
{
    if (!IsValidEntity(Center))
        return;
    
    int32 CX = GetProperty(Center, PropertyId::PosX);
    int32 CY = GetProperty(Center, PropertyId::PosY);
    int32 CZ = GetProperty(Center, PropertyId::PosZ);
    int64 RadiusSq = static_cast<int64>(RadiusCm) * RadiusCm;
    
    ForEachEntity([&](FHktEntityId E)
    {
        if (E == Center) return;
        
        int32 EX = GetProperty(E, PropertyId::PosX);
        int32 EY = GetProperty(E, PropertyId::PosY);
        int32 EZ = GetProperty(E, PropertyId::PosZ);
        
        int64 DX = EX - CX;
        int64 DY = EY - CY;
        int64 DZ = EZ - CZ;
        
        if (DX*DX + DY*DY + DZ*DZ <= RadiusSq)
        {
            Callback(E);
        }
    });
}