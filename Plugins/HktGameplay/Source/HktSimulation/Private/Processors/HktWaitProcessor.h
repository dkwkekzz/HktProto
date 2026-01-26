// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktSimulationProcessor.h"
#include "Core/HktStateTypes.h"
#include "Core/HktVMRuntime.h"

// ============================================================================
// HktWaitProcessor - 대기 조건 체크 (SOA 최적화)
// 
// 책임:
// - WaitQueue의 대기 조건 체크 (SOA로 캐시 효율적)
// - 조건 충족 시 완료 목록 반환
// - 완료된 VM을 JobQueue의 PendingSlots로 복귀
// ============================================================================

class FHktWaitProcessor
{
public:
    /**
     * 대기 중인 VM 조건 체크
     * 
     * @param Context 시뮬레이션 컨텍스트
     * @param OutCompletions 완료된 대기 항목들
     */
    static void Process(FHktSimulationContext& Context, TArray<FHktWaitCompletion>& OutCompletions);
    
private:
    // ========================================================================
    // 타입별 조건 체크 (SOA 최적화된 배치 처리)
    // ========================================================================
    
    /**
     * 시간 대기 체크 (WaitTime)
     * WaitTimers 배열만 순회하여 캐시 효율적
     */
    static void ProcessTimeWaits(FHktSimulationContext& Context, TArray<FHktWaitCompletion>& OutCompletions);
    
    /**
     * 도착 대기 체크 (WaitArrival)
     * 엔티티 위치와 타겟 비교
     */
    static void ProcessArrivalWaits(FHktSimulationContext& Context, TArray<FHktWaitCompletion>& OutCompletions);
    
    /**
     * 충돌 대기 체크 (WaitCollision)
     * 외부 충돌 이벤트 확인
     */
    static void ProcessCollisionWaits(FHktSimulationContext& Context, TArray<FHktWaitCompletion>& OutCompletions);
};

// ============================================================================
// 구현
// ============================================================================

inline void FHktWaitProcessor::Process(FHktSimulationContext& Context, TArray<FHktWaitCompletion>& OutCompletions)
{
    if (!Context.IsValid()) return;
    
    OutCompletions.Reset();
    
    // 타입별 배치 처리 (SOA 최적화)
    ProcessTimeWaits(Context, OutCompletions);
    ProcessArrivalWaits(Context, OutCompletions);
    ProcessCollisionWaits(Context, OutCompletions);
    
    // 완료된 항목들을 WaitQueue에서 제거하고 JobQueue로 복귀
    for (const FHktWaitCompletion& Completion : OutCompletions)
    {
        // WaitQueue에서 제거
        Context.WaitQueue->Remove(Completion.WaitQueueSlot);
        
        // Runtime 상태 업데이트
        FHktVMRuntime& Runtime = Context.JobQueue->GetBySlot(Completion.RuntimeSlot);
        if (Runtime.Generation == Completion.RuntimeGeneration && Runtime.IsWaiting())
        {
            // WaitQueue 참조 해제
            Runtime.WaitQueueIndex = INDEX_NONE;
            
            // Running 상태로 전환
            Runtime.State = EHktVMState::Running;
            
            // JobQueue의 PendingSlots에 추가
            Context.JobQueue->AddToPending(Completion.RuntimeSlot);
            
            if (Context.bDebugLog)
            {
                UE_LOG(LogTemp, Log, TEXT("[WaitProcessor] VM[%d] wait completed, condition=%d"),
                    Completion.RuntimeSlot, (int32)Completion.Condition);
            }
        }
    }
}

inline void FHktWaitProcessor::ProcessTimeWaits(FHktSimulationContext& Context, TArray<FHktWaitCompletion>& OutCompletions)
{
    FHktWaitQueue* WQ = Context.WaitQueue;
    const float DeltaTime = Context.DeltaTime;
    
    // SOA 순회: WaitTimers 배열만 접근 (캐시 라인 최적화)
    for (int32 i = 0; i < WQ->HighWaterMark; ++i)
    {
        if (WQ->WaitTypes[i] != EHktWaitType::Time)
        {
            continue;
        }
        
        // 타이머 감소
        WQ->WaitTimers[i] -= DeltaTime;
        
        if (WQ->WaitTimers[i] <= 0.0f)
        {
            // 완료
            FHktWaitCompletion Completion;
            Completion.RuntimeSlot = WQ->RuntimeSlots[i];
            Completion.RuntimeGeneration = WQ->RuntimeGenerations[i];
            Completion.WaitQueueSlot = i;
            Completion.Condition = EHktYieldCondition::Time;
            OutCompletions.Add(Completion);
        }
    }
}

inline void FHktWaitProcessor::ProcessArrivalWaits(FHktSimulationContext& Context, TArray<FHktWaitCompletion>& OutCompletions)
{
    FHktWaitQueue* WQ = Context.WaitQueue;
    FHktJobQueue* JQ = Context.JobQueue;
    FHktStateStore* StateStore = Context.StateStore;
    
    if (!StateStore) return;
    
    // SOA 순회: Arrival 타입만
    for (int32 i = 0; i < WQ->HighWaterMark; ++i)
    {
        if (WQ->WaitTypes[i] != EHktWaitType::Arrival)
        {
            continue;
        }
        
        // Runtime에서 Owner 엔티티 가져오기
        int32 RuntimeSlot = WQ->RuntimeSlots[i];
        const FHktVMRuntime& Runtime = JQ->GetBySlot(RuntimeSlot);
        
        if (Runtime.Generation != WQ->RuntimeGenerations[i])
        {
            // 이미 해제된 Runtime, 제거 처리
            FHktWaitCompletion Completion;
            Completion.RuntimeSlot = RuntimeSlot;
            Completion.RuntimeGeneration = WQ->RuntimeGenerations[i];
            Completion.WaitQueueSlot = i;
            Completion.Condition = EHktYieldCondition::Arrival;
            OutCompletions.Add(Completion);
            continue;
        }
        
        // 엔티티 도착 체크
        FHktEntityState* EntityState = StateStore->Entities.Get(Runtime.OwnerEntityID);
        if (!EntityState || !StateStore->Entities.IsValid(Runtime.OwnerEntityID, Runtime.OwnerGeneration))
        {
            // 엔티티 무효화됨, 완료 처리
            FHktWaitCompletion Completion;
            Completion.RuntimeSlot = RuntimeSlot;
            Completion.RuntimeGeneration = WQ->RuntimeGenerations[i];
            Completion.WaitQueueSlot = i;
            Completion.Condition = EHktYieldCondition::Arrival;
            OutCompletions.Add(Completion);
            continue;
        }
        
        // 도착 체크
        if (EntityState->HasArrivedAtTarget())
        {
            EntityState->ClearFlag(FHktEntityState::FLAG_MOVING);
            
            FHktWaitCompletion Completion;
            Completion.RuntimeSlot = RuntimeSlot;
            Completion.RuntimeGeneration = WQ->RuntimeGenerations[i];
            Completion.WaitQueueSlot = i;
            Completion.Condition = EHktYieldCondition::Arrival;
            OutCompletions.Add(Completion);
        }
    }
}

inline void FHktWaitProcessor::ProcessCollisionWaits(FHktSimulationContext& Context, TArray<FHktWaitCompletion>& OutCompletions)
{
    FHktWaitQueue* WQ = Context.WaitQueue;
    
    // 외부 충돌 이벤트가 없으면 스킵
    if (Context.LastCollisionEntityID == INDEX_NONE)
    {
        return;
    }
    
    // 현재 구현: 첫 번째 Collision 대기자에게 전달
    // TODO: 충돌 대상 매칭 로직 추가
    for (int32 i = 0; i < WQ->HighWaterMark; ++i)
    {
        if (WQ->WaitTypes[i] != EHktWaitType::Collision)
        {
            continue;
        }
        
        FHktWaitCompletion Completion;
        Completion.RuntimeSlot = WQ->RuntimeSlots[i];
        Completion.RuntimeGeneration = WQ->RuntimeGenerations[i];
        Completion.WaitQueueSlot = i;
        Completion.Condition = EHktYieldCondition::Collision;
        Completion.CollisionEntityID = Context.LastCollisionEntityID;
        Completion.CollisionGeneration = Context.LastCollisionGeneration;
        OutCompletions.Add(Completion);
        
        // 충돌 이벤트 소비
        Context.LastCollisionEntityID = INDEX_NONE;
        Context.LastCollisionGeneration = 0;
        break;
    }
}
