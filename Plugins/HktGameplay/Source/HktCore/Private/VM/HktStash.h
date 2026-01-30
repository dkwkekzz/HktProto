// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktCoreInterfaces.h"

/**
 * FHktStashBase - Stash 공통 기능 구현
 * 
 * SOA 레이아웃으로 엔티티 데이터 저장
 * FHktMasterStash, FHktVisibleStash의 기본 클래스
 */
class FHktStashBase
{
public:
    FHktStashBase();
    virtual ~FHktStashBase() = default;

    // ========== IHktStashInterface 공통 구현 ==========
    FHktEntityId AllocateEntity();
    void FreeEntity(FHktEntityId Entity);
    bool IsValidEntity(FHktEntityId Entity) const;
    int32 GetProperty(FHktEntityId Entity, uint16 PropertyId) const;
    void SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value);
    int32 GetEntityCount() const;
    int32 GetCompletedFrameNumber() const { return CompletedFrameNumber; }
    void MarkFrameCompleted(int32 FrameNumber);
    void ForEachEntity(TFunctionRef<void(FHktEntityId)> Callback) const;
    uint32 CalculateChecksum() const;

protected:
    /** SetProperty 시 자동 엔티티 생성 여부 (VisibleStash에서 사용) */
    bool bAutoCreateOnSet = false;
    
    /** 변경 추적 (파생 클래스에서 오버라이드) */
    virtual void OnEntityDirty(FHktEntityId Entity) {}

    static constexpr int32 MaxEntities = 1024;
    static constexpr int32 MaxProperties = 256;

    /** SOA 레이아웃: Properties[PropertyId][EntityId] */
    TArray<TArray<int32>> Properties;
    TBitArray<> ValidEntities;
    TArray<FHktEntityId> FreeList;
    int32 NextEntityId = 0;
    int32 CompletedFrameNumber = 0;
};
