// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktStash.h"
#include "HktCoreInterfaces.h"

/**
 * FHktMasterStash - 서버 전용 Stash 구현
 * 
 * 모든 엔티티의 "진실의 원천(Source of Truth)"
 * 스냅샷 생성, 변경 추적, 위치 관리 기능 제공
 */
class HKTCORE_API FHktMasterStash : public FHktStashBase, public IHktMasterStashInterface
{
public:
    FHktMasterStash();
    virtual ~FHktMasterStash() override = default;

    // ========== IHktStashInterface Implementation ==========
    virtual FHktEntityId AllocateEntity() override { return FHktStashBase::AllocateEntity(); }
    virtual void FreeEntity(FHktEntityId Entity) override { FHktStashBase::FreeEntity(Entity); }
    virtual bool IsValidEntity(FHktEntityId Entity) const override { return FHktStashBase::IsValidEntity(Entity); }
    virtual int32 GetProperty(FHktEntityId Entity, uint16 PropertyId) const override { return FHktStashBase::GetProperty(Entity, PropertyId); }
    virtual void SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value) override { FHktStashBase::SetProperty(Entity, PropertyId, Value); }
    virtual int32 GetEntityCount() const override { return FHktStashBase::GetEntityCount(); }
    virtual int32 GetCompletedFrameNumber() const override { return FHktStashBase::GetCompletedFrameNumber(); }
    virtual void MarkFrameCompleted(int32 FrameNumber) override { FHktStashBase::MarkFrameCompleted(FrameNumber); }
    virtual void ForEachEntity(TFunctionRef<void(FHktEntityId)> Callback) const override { FHktStashBase::ForEachEntity(Callback); }
    virtual uint32 CalculateChecksum() const override { return FHktStashBase::CalculateChecksum(); }

    // ========== IHktMasterStashInterface Implementation ==========
    virtual void ApplyWrites(const TArray<FPendingWrite>& Writes) override;
    virtual bool ValidateEntityFrame(FHktEntityId Entity, int32 FrameNumber) const override;
    virtual FHktEntitySnapshot CreateEntitySnapshot(FHktEntityId Entity) const override;
    virtual TArray<FHktEntitySnapshot> CreateSnapshots(const TArray<FHktEntityId>& Entities) const override;
    virtual TArray<uint8> SerializeFullState() const override;
    virtual void DeserializeFullState(const TArray<uint8>& Data) override;
    virtual bool TryGetPosition(FHktEntityId Entity, FVector& OutPosition) const override;
    virtual void SetPosition(FHktEntityId Entity, const FVector& Position) override;
    virtual uint32 CalculatePartialChecksum(const TArray<FHktEntityId>& Entities) const override;
    virtual const TSet<FHktEntityId>& GetDirtyEntities() const override { return DirtyEntities; }
    virtual void ClearDirtyFlags() override;
    virtual void ForEachEntityInRadius(FHktEntityId Center, int32 RadiusCm, TFunctionRef<void(FHktEntityId)> Callback) const override;

protected:
    virtual void OnEntityDirty(FHktEntityId Entity) override;

private:
    /** 엔티티 생성 프레임 (Validation용) */
    TArray<int32> EntityCreationFrame;
    
    /** 변경 추적 */
    TSet<FHktEntityId> DirtyEntities;
};
