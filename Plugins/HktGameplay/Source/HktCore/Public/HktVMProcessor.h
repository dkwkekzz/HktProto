#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "VMTypes.h"
#include "VMProgram.h"
#include "VMStore.h"
#include "VMRuntime.h"

/**
 * FVMProcessor - 3단계 파이프라인으로 VM들을 처리
 * 
 * Build:   IntentEvent → VM 생성
 * Execute: 모든 VM yield까지 실행
 * Cleanup: 결과 적용, 완료된 VM 정리
 */
class HKTCORE_API FHktVMProcessor
{
public:
    FHktVMProcessor() = default;
    
    void Initialize(UWorld* InWorld, FStashBase* InStash);
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
    TOptional<FVMHandle> TryCreateVM(const FHktIntentEvent& Event, int32 CurrentFrame);
    bool ValidateStoreFrame(FHktEntityId Entity, int32 CurrentFrame) const;

    // Phase 2
    void Execute(float DeltaSeconds);
    EVMStatus ExecuteUntilYield(FVMHandle Handle, float DeltaSeconds);

    // Phase 3
    void Cleanup(int32 CurrentFrame);
    void ApplyStoreChanges(FVMHandle Handle);
    void FinalizeVM(FVMHandle Handle);

private:
    UWorld* World = nullptr;
    FStashBase* Stash = nullptr;
    
    FVMRuntimePool RuntimePool;
    TArray<FVMStore> StorePool;
    
    TArray<FHktIntentEvent> PendingEvents;
    TArray<FVMHandle> PendingVMs;
    TArray<FVMHandle> ActiveVMs;
    TArray<FVMHandle> CompletedVMs;
    
    class FVMInterpreter* Interpreter = nullptr;
};

