#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HktVisibleStashComponent.generated.h"


/**
 * UVisibleStashComponent - 클라이언트 로컬 저장소
 * 
 * IntentEvent를 받아 결정론적으로 실행한 결과를 저장
 * 서버와 동일한 입력 → 동일한 결과 보장
 * 
 * 서버의 MasterStash와 동일한 인터페이스 제공
 */
UCLASS(ClassGroup=(HktSimulation), meta=(BlueprintSpawnableComponent))
class HKTRUNTIME_API UHktVisibleStashComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHktVisibleStashComponent();
    
    // ========== Entity Management ==========
    FHktEntityId AllocateEntity();
    void FreeEntity(FHktEntityId Entity);
    bool IsValidEntity(FHktEntityId Entity) const;
    int32 GetEntityCount() const;
    
    // ========== Property Access ==========
    int32 GetProperty(FHktEntityId Entity, uint16 PropertyId) const;
    void SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value);
    
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
    
    // ========== Snapshot Sync ==========
    
    /** 단일 엔티티 스냅샷 적용 (IntentEvent에 첨부된 것) */
    void ApplySnapshot(const FHktEntitySnapshot& Snapshot);
    
    /** 여러 스냅샷 적용 */
    void ApplySnapshots(const TArray<FHktEntitySnapshot>& Snapshots);
    
    /** 전체 클리어 */
    void Clear();
    
    // ========== Checksum ==========
    uint32 CalculateChecksum() const;
    
    // ========== Iteration ==========
    template<typename Func>
    void ForEachEntity(Func&& Callback) const;

private:
    static constexpr int32 MaxEntities = 1024;
    static constexpr int32 MaxProperties = 256;
    
    /** SOA 레이아웃 (MasterStash와 동일) */
    TArray<TArray<int32>> Properties;
    TBitArray<> ValidEntities;
    TArray<FHktEntityId> FreeList;
    FHktEntityId NextEntityId = 0;
    int32 CompletedFrameNumber = 0;
};

// ============================================================================
// Template Implementation
// ============================================================================

template<typename Func>
void UHktVisibleStashComponent::ForEachEntity(Func&& Callback) const
{
    for (FHktEntityId E = 0; E < MaxEntities; ++E)
    {
        if (ValidEntities[E])
        {
            Callback(E);
        }
    }
}