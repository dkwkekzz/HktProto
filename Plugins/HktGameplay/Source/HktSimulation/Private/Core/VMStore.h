#pragma once

#include "CoreMinimal.h"
#include "VMTypes.h"

// Forward declaration
struct FStashBase;

/**
 * FVMStore - 단일 VM의 로컬 속성 뷰
 * 
 * 읽기: 로컬 캐시 → Stash 순으로 조회
 * 쓰기: 로컬 캐시 + PendingWrites에 기록
 * 
 * VM 완료 시 PendingWrites가 Stash에 일괄 적용됨
 * 로컬 캐시는 VM 내에서의 읽기/쓰기 일관성을 보장
 */
struct HKTSIMULATION_API FVMStore
{
    EntityId SourceEntity = InvalidEntityId;
    EntityId TargetEntity = InvalidEntityId;
    
    /** 속성 읽기 - 로컬 캐시 우선, 없으면 Stash에서 */
    int32 Read(uint16 PropertyId) const;
    int32 ReadEntity(EntityId Entity, uint16 PropertyId) const;
    float ReadFloat(uint16 PropertyId) const;
    
    /** 속성 쓰기 - 로컬 캐시 + PendingWrites에 기록 */
    void Write(uint16 PropertyId, int32 Value);
    void WriteEntity(EntityId Entity, uint16 PropertyId, int32 Value);
    void WriteFloat(uint16 PropertyId, float Value);
    
    /** Stash에 적용될 변경사항 */
    struct FPendingWrite
    {
        EntityId Entity;
        uint16 PropertyId;
        int32 Value;
    };
    TArray<FPendingWrite> PendingWrites;
    
    /** 로컬 캐시 (VM 내 읽기/쓰기 일관성용) */
    TMap<uint64, int32> LocalCache;  // Key = (Entity << 16) | PropertyId
    
    void ClearPendingWrites();
    void Reset();
    
    FStashBase* Stash = nullptr;

private:
    static uint64 MakeCacheKey(EntityId Entity, uint16 PropertyId)
    {
        return (static_cast<uint64>(Entity) << 16) | PropertyId;
    }
};

/**
 * FStashBase - SOA 레이아웃의 중앙 데이터 저장소
 * 
 * 결정론적 시뮬레이션의 "진실의 원천(Source of Truth)"
 * VM 완료 시에만 변경사항이 적용됨
 * 
 * Component가 아닌 일반 struct로, UHktSimulationStashComponent에서
 * 멤버로 소유하여 네트워크 동기화 기능을 추가함
 */
struct HKTSIMULATION_API FStashBase
{
    static constexpr int32 MaxEntities = 1024;
    
    FStashBase() = default;
    
    /** 초기화 (생성자 대신 명시적 호출) */
    void Initialize();
    
    // ========== Entity Management ==========
    EntityId AllocateEntity();
    void FreeEntity(EntityId Entity);
    bool IsValidEntity(EntityId Entity) const;
    int32 GetEntityCount() const;
    
    // ========== Property Access ==========
    int32 GetProperty(EntityId Entity, uint16 PropertyId) const;
    void SetProperty(EntityId Entity, uint16 PropertyId, int32 Value);
    void ApplyWrites(const TArray<FVMStore::FPendingWrite>& Writes);
    
    // ========== Frame Management ==========
    int32 GetCompletedFrameNumber() const { return CompletedFrameNumber; }
    void MarkFrameCompleted(int32 FrameNumber);
    bool ValidateEntityFrame(EntityId Entity, int32 FrameNumber) const;
    
    // ========== Serialization ==========
    void SerializeState(FArchive& Ar);
    uint32 CalculateChecksum() const;

private:
    /** SOA 레이아웃: Properties[PropertyId][EntityId] */
    TArray<TArray<int32>> Properties;
    TBitArray<> ValidEntities;
    TArray<EntityId> FreeList;
    EntityId NextEntityId = 0;
    int32 CompletedFrameNumber = 0;
    TArray<int32> EntityCreationFrame;
};