// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HktCoreTypes.h"

//=============================================================================
// IHktStashInterface - 순수 C++ Stash 인터페이스
//=============================================================================

/**
 * IHktStashInterface - Stash 공통 인터페이스 (Pure C++)
 * 
 * HktCore에서 구현, HktRuntime에서 래퍼 컴포넌트로 사용
 */
class HKTCORE_API IHktStashInterface
{
public:
    virtual ~IHktStashInterface() = default;
    
    // ========== Entity Management ==========
    virtual bool IsValidEntity(FHktEntityId Entity) const = 0;
    virtual int32 GetProperty(FHktEntityId Entity, uint16 PropertyId) const = 0;
    virtual void SetProperty(FHktEntityId Entity, uint16 PropertyId, int32 Value) = 0;
    virtual FHktEntityId AllocateEntity() = 0;
    virtual void FreeEntity(FHktEntityId Entity) = 0;
    
    // ========== Entity Count ==========
    virtual int32 GetEntityCount() const = 0;
    
    // ========== Frame Management ==========
    virtual int32 GetCompletedFrameNumber() const = 0;
    virtual void MarkFrameCompleted(int32 FrameNumber) = 0;
    
    // ========== Iteration ==========
    virtual void ForEachEntity(TFunctionRef<void(FHktEntityId)> Callback) const = 0;
    
    // ========== Checksum ==========
    virtual uint32 CalculateChecksum() const = 0;
};

//=============================================================================
// IHktMasterStashInterface - 서버 전용 확장 인터페이스
//=============================================================================

/**
 * IHktMasterStashInterface - MasterStash 전용 인터페이스 (서버)
 * 
 * 스냅샷 생성, 위치 관리, 변경 추적 등 서버 전용 기능 제공
 */
class HKTCORE_API IHktMasterStashInterface : public IHktStashInterface
{
public:
    // ========== Batch Operations ==========
    struct FPendingWrite
    {
        FHktEntityId Entity;
        uint16 PropertyId;
        int32 Value;
    };
    virtual void ApplyWrites(const TArray<FPendingWrite>& Writes) = 0;
    
    // ========== Frame Validation ==========
    virtual bool ValidateEntityFrame(FHktEntityId Entity, int32 FrameNumber) const = 0;
    
    // ========== Snapshot & Delta ==========
    virtual FHktEntitySnapshot CreateEntitySnapshot(FHktEntityId Entity) const = 0;
    virtual TArray<FHktEntitySnapshot> CreateSnapshots(const TArray<FHktEntityId>& Entities) const = 0;
    virtual TArray<uint8> SerializeFullState() const = 0;
    virtual void DeserializeFullState(const TArray<uint8>& Data) = 0;
    
    // ========== Position Access ==========
    virtual bool TryGetPosition(FHktEntityId Entity, FVector& OutPosition) const = 0;
    virtual void SetPosition(FHktEntityId Entity, const FVector& Position) = 0;
    
    // ========== Partial Checksum ==========
    virtual uint32 CalculatePartialChecksum(const TArray<FHktEntityId>& Entities) const = 0;
    
    // ========== Change Tracking ==========
    virtual const TSet<FHktEntityId>& GetDirtyEntities() const = 0;
    virtual void ClearDirtyFlags() = 0;
    
    // ========== Radius Query ==========
    virtual void ForEachEntityInRadius(FHktEntityId Center, int32 RadiusCm, TFunctionRef<void(FHktEntityId)> Callback) const = 0;
};

//=============================================================================
// IHktVisibleStashInterface - 클라이언트 전용 확장 인터페이스
//=============================================================================

/**
 * IHktVisibleStashInterface - VisibleStash 전용 인터페이스 (클라이언트)
 * 
 * 스냅샷 적용, 클리어 등 클라이언트 전용 기능 제공
 */
class HKTCORE_API IHktVisibleStashInterface : public IHktStashInterface
{
public:
    // ========== Batch Operations ==========
    struct FPendingWrite
    {
        FHktEntityId Entity;
        uint16 PropertyId;
        int32 Value;
    };
    virtual void ApplyWrites(const TArray<FPendingWrite>& Writes) = 0;
    
    // ========== Snapshot Sync ==========
    virtual void ApplyEntitySnapshot(const FHktEntitySnapshot& Snapshot) = 0;
    virtual void ApplySnapshots(const TArray<FHktEntitySnapshot>& Snapshots) = 0;
    
    // ========== Clear ==========
    virtual void Clear() = 0;
};

//=============================================================================
// IHktVMProcessorInterface - VM 프로세서 인터페이스
//=============================================================================

/**
 * IHktVMProcessorInterface - VMProcessor 외부 사용 인터페이스 (Pure C++)
 * 
 * 외부 모듈(HktRuntime 등)에서 VMProcessor를 사용하기 위한 인터페이스
 * 실제 구현은 HktCore의 FHktVMProcessor에서 제공
 */
class HKTCORE_API IHktVMProcessorInterface
{
public:
    virtual ~IHktVMProcessorInterface() = default;
    
    /** 프레임 처리 (Build → Execute → Cleanup 파이프라인) */
    virtual void Tick(int32 CurrentFrame, float DeltaSeconds) = 0;
    
    /** Intent 이벤트 알림 */
    virtual void NotifyIntentEvent(const FHktIntentEvent& Event) = 0;
    
    /** 충돌 알림 */
    virtual void NotifyCollision(FHktEntityId WatchedEntity, FHktEntityId HitEntity) = 0;
    
    /** 애니메이션 종료 알림 */
    virtual void NotifyAnimEnd(FHktEntityId Entity) = 0;
    
    /** 이동 종료 알림 */
    virtual void NotifyMoveEnd(FHktEntityId Entity) = 0;
};

//=============================================================================
// 팩토리 함수 선언
//=============================================================================

/**
 * VMProcessor 인스턴스 생성
 * 
 * 호출자가 반환된 포인터의 수명을 관리해야 함
 * TUniquePtr로 래핑하여 사용 권장
 */
HKTCORE_API TUniquePtr<IHktVMProcessorInterface> CreateVMProcessor(IHktStashInterface* InStash);

/**
 * MasterStash 인스턴스 생성 (서버 전용)
 */
HKTCORE_API TUniquePtr<IHktMasterStashInterface> CreateMasterStash();

/**
 * VisibleStash 인스턴스 생성 (클라이언트 전용)
 */
HKTCORE_API TUniquePtr<IHktVisibleStashInterface> CreateVisibleStash();
