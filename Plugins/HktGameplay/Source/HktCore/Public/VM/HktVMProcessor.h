#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HktCoreTypes.h"
#include "HktVMTypes.h"

// Forward declarations
struct FHktVMStore;
class FHktVMRuntimePool;
class IStashInterface;
enum class EVMStatus : uint8;

/**
 * FHktVMProcessor - 3단계 파이프라인으로 VM들을 처리 (Pure C++)
 * 
 * Build:   IntentEvent → VM 생성
 * Execute: 모든 VM yield까지 실행
 * Cleanup: 결과 적용, 완료된 VM 정리
 * 
 * UObject/UWorld 참조 없음 - HktCore의 순수성 유지
 */
class HKTCORE_API FHktVMProcessor
{
public:
    FHktVMProcessor() = default;
    ~FHktVMProcessor();
    
    void Initialize(IStashInterface* InStash);
    void Tick(int32 CurrentFrame, float DeltaSeconds);
    
    /** 외부에서 이벤트 주입 */
    void QueueIntentEvent(const FHktIntentEvent& Event);
    
    /** 이벤트 알림 (충돌, 애니메이션 등) */
    void NotifyCollision(FHktEntityId WatchedEntity, FHktEntityId HitEntity);
    void NotifyAnimEnd(FHktEntityId Entity);
    void NotifyMoveEnd(FHktEntityId Entity);

private:
    // Phase 1
    void Build(int32 CurrentFrame);
    TArray<FHktIntentEvent> PullIntentEvents();
    TOptional<FHktVMHandle> TryCreateVM(const FHktIntentEvent& Event, int32 CurrentFrame);
    bool ValidateStoreFrame(FHktEntityId Entity, int32 CurrentFrame) const;

    // Phase 2
    void Execute(float DeltaSeconds);
    EVMStatus ExecuteUntilYield(FHktVMHandle Handle, float DeltaSeconds);

    // Phase 3
    void Cleanup(int32 CurrentFrame);
    void ApplyStoreChanges(FHktVMHandle Handle);
    void FinalizeVM(FHktVMHandle Handle);

private:
    void ApplyAttachedSnapshots(const FHktIntentEvent& Event);

private:
    IStashInterface* Stash = nullptr;
    
    TUniquePtr<FHktVMRuntimePool> RuntimePool;
    TArray<FHktVMStore> StorePool;
    
    TArray<FHktIntentEvent> PendingEvents;
    TArray<FHktVMHandle> PendingVMs;
    TArray<FHktVMHandle> ActiveVMs;
    TArray<FHktVMHandle> CompletedVMs;
    
    class FHktVMInterpreter* Interpreter = nullptr;
};

