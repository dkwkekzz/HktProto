// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktSimulationProcessor.h"
#include "Core/HktVMTypes.h"

// ============================================================================
// HktCleanupProcessor - 완료된 이벤트 정리 Processor
// 
// 책임:
// - JobQueue의 FinishedSlots 정리
// - 관련 리소스 해제
// - 완료 이벤트 SimulationStashComponent에 전파
// ============================================================================

// 전방 선언
class UHktSimulationStashComponent;

/**
 * 완료된 이벤트 정보
 */
struct FHktCompletedEvent
{
    int32 EventID = INDEX_NONE;
    int32 OwnerEntityID = INDEX_NONE;
    EHktVMState FinalState = EHktVMState::Finished;
    
    bool IsValid() const { return EventID != INDEX_NONE; }
};

/**
 * 정리 Processor
 */
class FHktCleanupProcessor
{
public:
    /**
     * 완료된 VM 정리 및 이벤트 전파
     * 
     * @param Context 시뮬레이션 컨텍스트
     * @param OutCompletedEvents 완료된 이벤트 정보 (전파용)
     */
    static void Process(FHktSimulationContext& Context, TArray<FHktCompletedEvent>& OutCompletedEvents);
    
    /**
     * 완료된 이벤트를 SimulationStashComponent에 전파
     * 
     * @param CompletedEvents 완료된 이벤트 목록
     * @param StashComponents 알림 받을 컴포넌트 목록
     */
    static void NotifyCompletions(
        const TArray<FHktCompletedEvent>& CompletedEvents,
        const TArray<UHktSimulationStashComponent*>& StashComponents);
};

// ============================================================================
// 구현
// ============================================================================

inline void FHktCleanupProcessor::Process(FHktSimulationContext& Context, TArray<FHktCompletedEvent>& OutCompletedEvents)
{
    if (!Context.IsValid()) return;
    
    FHktJobQueue* JQ = Context.JobQueue;
    OutCompletedEvents.Reset();
    
    // FinishedSlots 처리
    for (int32 Slot : JQ->FinishedSlots)
    {
        FHktVMRuntime& Runtime = JQ->GetBySlot(Slot);
        
        if (!Runtime.IsFinished())
        {
            continue;
        }
        
        // 완료 이벤트 정보 수집
        FHktCompletedEvent CompletedEvent;
        CompletedEvent.EventID = Runtime.EventID;
        CompletedEvent.OwnerEntityID = Runtime.OwnerEntityID;
        CompletedEvent.FinalState = Runtime.State;
        OutCompletedEvents.Add(CompletedEvent);
        
        // WaitQueue에 남아있는 참조 정리 (혹시 모를 누수 방지)
        if (Runtime.WaitQueueIndex != INDEX_NONE)
        {
            Context.WaitQueue->Remove(Runtime.WaitQueueIndex);
            Runtime.WaitQueueIndex = INDEX_NONE;
        }
        
        if (Context.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[CleanupProcessor] Cleaning up VM[%d] EventID=%d State=%d"),
                Slot, Runtime.EventID, (int32)Runtime.State);
        }
    }
    
    // FinishedSlots의 모든 슬롯 해제
    TArray<int32> SlotsToFree = JQ->FinishedSlots;
    for (int32 Slot : SlotsToFree)
    {
        JQ->Free(Slot);
    }
    JQ->FinishedSlots.Empty();
    
    // Context의 CompletedEventIDs도 정리
    Context.CompletedEventIDs.Empty();
}

inline void FHktCleanupProcessor::NotifyCompletions(
    const TArray<FHktCompletedEvent>& CompletedEvents,
    const TArray<UHktSimulationStashComponent*>& StashComponents)
{
    if (CompletedEvents.Num() == 0 || StashComponents.Num() == 0)
    {
        return;
    }
    
    // TODO: 각 컴포넌트에 관련 이벤트만 전달하도록 필터링
    // 현재는 모든 컴포넌트에 모든 이벤트 전파
    for (const FHktCompletedEvent& Event : CompletedEvents)
    {
        for (UHktSimulationStashComponent* Component : StashComponents)
        {
            if (Component && IsValid(Component))
            {
                // TODO: Component->OnSimulationEventCompleted(Event);
            }
        }
    }
}
