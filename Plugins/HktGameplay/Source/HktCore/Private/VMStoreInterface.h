#pragma once

#include "CoreMinimal.h"
#include "VMTypes.h"

class UMasterStashComponent;
class UVisibleStashComponent;

/**
 * IStashInterface - Stash 공통 인터페이스
 * 
 * MasterStash(서버)와 VisibleStash(클라) 모두 동일한 인터페이스
 */
class HKTSIMULATION_API IStashInterface
{
public:
    virtual ~IStashInterface() = default;
    
    virtual bool IsValidEntity(EntityId Entity) const = 0;
    virtual int32 GetProperty(EntityId Entity, uint16 PropertyId) const = 0;
    virtual void SetProperty(EntityId Entity, uint16 PropertyId, int32 Value) = 0;
    virtual EntityId AllocateEntity() = 0;
    virtual void FreeEntity(EntityId Entity) = 0;
};

/**
 * FMasterStashWrapper - MasterStash용 래퍼
 */
class HKTSIMULATION_API FMasterStashWrapper : public IStashInterface
{
public:
    explicit FMasterStashWrapper(UMasterStashComponent* InStash) : Stash(InStash) {}
    
    virtual bool IsValidEntity(EntityId Entity) const override;
    virtual int32 GetProperty(EntityId Entity, uint16 PropertyId) const override;
    virtual void SetProperty(EntityId Entity, uint16 PropertyId, int32 Value) override;
    virtual EntityId AllocateEntity() override;
    virtual void FreeEntity(EntityId Entity) override;
    
    UMasterStashComponent* GetStash() const { return Stash; }
    
private:
    UMasterStashComponent* Stash;
};

/**
 * FVisibleStashWrapper - VisibleStash용 래퍼
 */
class HKTSIMULATION_API FVisibleStashWrapper : public IStashInterface
{
public:
    explicit FVisibleStashWrapper(UVisibleStashComponent* InStash) : Stash(InStash) {}
    
    virtual bool IsValidEntity(EntityId Entity) const override;
    virtual int32 GetProperty(EntityId Entity, uint16 PropertyId) const override;
    virtual void SetProperty(EntityId Entity, uint16 PropertyId, int32 Value) override;
    virtual EntityId AllocateEntity() override;
    virtual void FreeEntity(EntityId Entity) override;
    
    UVisibleStashComponent* GetStash() const { return Stash; }
    
private:
    UVisibleStashComponent* Stash;
};

/**
 * FVMStore - VM의 로컬 데이터 뷰
 * 
 * 읽기: 로컬 캐시 → Stash 순으로 조회
 * 쓰기: 로컬 캐시 + PendingWrites에 기록
 * VM 완료 시 PendingWrites가 Stash에 일괄 적용
 */
struct HKTSIMULATION_API FVMStore
{
    EntityId SourceEntity = InvalidEntityId;
    EntityId TargetEntity = InvalidEntityId;
    
    int32 Read(uint16 PropertyId) const;
    int32 ReadEntity(EntityId Entity, uint16 PropertyId) const;
    
    void Write(uint16 PropertyId, int32 Value);
    void WriteEntity(EntityId Entity, uint16 PropertyId, int32 Value);
    
    struct FPendingWrite
    {
        EntityId Entity;
        uint16 PropertyId;
        int32 Value;
    };
    TArray<FPendingWrite> PendingWrites;
    
    /** 로컬 캐시 (VM 내 읽기/쓰기 일관성) */
    TMap<uint64, int32> LocalCache;
    
    void ClearPendingWrites();
    void Reset();
    
    IStashInterface* Stash = nullptr;

private:
    static uint64 MakeCacheKey(EntityId Entity, uint16 PropertyId)
    {
        return (static_cast<uint64>(Entity) << 16) | PropertyId;
    }
};