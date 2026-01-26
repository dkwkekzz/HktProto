// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktVMTypes.h"

// ============================================================================
// HktWaitQueue - SOA 구조의 대기 큐
// 
// 설계 철학:
// - 대기 조건 체크에 필요한 최소 데이터만 SOA로 배치
// - 캐시 효율적인 조건 체크 (같은 필드끼리 연속 메모리)
// - Time/Condition 대기를 분리하여 불필요한 체크 최소화
// ============================================================================

// 대기 큐 최대 크기
constexpr int32 HKT_WAIT_QUEUE_SIZE = 1024;

/**
 * 대기 엔트리 타입
 */
enum class EHktWaitType : uint8
{
    None = 0,
    Time,           // 시간 대기 (WaitTimer > 0)
    Arrival,        // 도착 대기 (Entity가 Target에 도착)
    Collision,      // 충돌 대기 (외부 이벤트)
    Custom,         // 커스텀 조건
};

/**
 * 대기 큐 (SOA 구조)
 * 
 * 조건 체크 시 같은 타입의 데이터만 순회하여 캐시 효율 극대화
 */
struct FHktWaitQueue
{
    // ========================================================================
    // SOA 데이터 - 캐시 라인 정렬
    // ========================================================================
    
    // 런타임 인덱스 (JobQueue 참조)
    alignas(HKT_CACHE_LINE) int32 RuntimeSlots[HKT_WAIT_QUEUE_SIZE];
    alignas(HKT_CACHE_LINE) int32 RuntimeGenerations[HKT_WAIT_QUEUE_SIZE];
    
    // 대기 타입
    alignas(HKT_CACHE_LINE) EHktWaitType WaitTypes[HKT_WAIT_QUEUE_SIZE];
    
    // 시간 대기용 (WaitType::Time)
    alignas(HKT_CACHE_LINE) float WaitTimers[HKT_WAIT_QUEUE_SIZE];
    
    // 엔티티 기반 조건용 (Arrival, Collision)
    alignas(HKT_CACHE_LINE) int32 TargetEntityIDs[HKT_WAIT_QUEUE_SIZE];
    alignas(HKT_CACHE_LINE) int32 TargetGenerations[HKT_WAIT_QUEUE_SIZE];
    
    // Yield 조건 (원래 조건 저장용)
    alignas(HKT_CACHE_LINE) EHktYieldCondition YieldConditions[HKT_WAIT_QUEUE_SIZE];
    
    // ========================================================================
    // 관리 데이터
    // ========================================================================
    
    int32 Count = 0;                     // 현재 대기 중인 항목 수
    int32 HighWaterMark = 0;             // 최대 사용 인덱스 + 1
    TArray<int32> FreeSlots;             // 재사용 가능 슬롯
    
    // ========================================================================
    // API
    // ========================================================================
    
    FHktWaitQueue()
    {
        Clear();
    }
    
    void Clear()
    {
        FMemory::Memzero(RuntimeSlots, sizeof(RuntimeSlots));
        FMemory::Memzero(RuntimeGenerations, sizeof(RuntimeGenerations));
        FMemory::Memzero(WaitTypes, sizeof(WaitTypes));
        FMemory::Memzero(WaitTimers, sizeof(WaitTimers));
        FMemory::Memzero(TargetEntityIDs, sizeof(TargetEntityIDs));
        FMemory::Memzero(TargetGenerations, sizeof(TargetGenerations));
        FMemory::Memzero(YieldConditions, sizeof(YieldConditions));
        Count = 0;
        HighWaterMark = 0;
        FreeSlots.Empty();
    }
    
    /**
     * 시간 대기 추가
     * @return WaitQueue 내 인덱스
     */
    int32 AddTimeWait(int32 RuntimeSlot, int32 RuntimeGen, float Duration)
    {
        int32 Slot = AllocateSlot();
        if (Slot == INDEX_NONE) return INDEX_NONE;
        
        RuntimeSlots[Slot] = RuntimeSlot;
        RuntimeGenerations[Slot] = RuntimeGen;
        WaitTypes[Slot] = EHktWaitType::Time;
        WaitTimers[Slot] = Duration;
        YieldConditions[Slot] = EHktYieldCondition::Time;
        
        return Slot;
    }
    
    /**
     * 도착 대기 추가
     */
    int32 AddArrivalWait(int32 RuntimeSlot, int32 RuntimeGen, int32 EntityID, int32 EntityGen)
    {
        int32 Slot = AllocateSlot();
        if (Slot == INDEX_NONE) return INDEX_NONE;
        
        RuntimeSlots[Slot] = RuntimeSlot;
        RuntimeGenerations[Slot] = RuntimeGen;
        WaitTypes[Slot] = EHktWaitType::Arrival;
        TargetEntityIDs[Slot] = EntityID;
        TargetGenerations[Slot] = EntityGen;
        YieldConditions[Slot] = EHktYieldCondition::Arrival;
        
        return Slot;
    }
    
    /**
     * 충돌 대기 추가
     */
    int32 AddCollisionWait(int32 RuntimeSlot, int32 RuntimeGen)
    {
        int32 Slot = AllocateSlot();
        if (Slot == INDEX_NONE) return INDEX_NONE;
        
        RuntimeSlots[Slot] = RuntimeSlot;
        RuntimeGenerations[Slot] = RuntimeGen;
        WaitTypes[Slot] = EHktWaitType::Collision;
        YieldConditions[Slot] = EHktYieldCondition::Collision;
        
        return Slot;
    }
    
    /**
     * 커스텀 대기 추가
     */
    int32 AddCustomWait(int32 RuntimeSlot, int32 RuntimeGen)
    {
        int32 Slot = AllocateSlot();
        if (Slot == INDEX_NONE) return INDEX_NONE;
        
        RuntimeSlots[Slot] = RuntimeSlot;
        RuntimeGenerations[Slot] = RuntimeGen;
        WaitTypes[Slot] = EHktWaitType::Custom;
        YieldConditions[Slot] = EHktYieldCondition::Custom;
        
        return Slot;
    }
    
    /**
     * 대기 항목 제거
     */
    void Remove(int32 Slot)
    {
        if (Slot < 0 || Slot >= HKT_WAIT_QUEUE_SIZE) return;
        if (WaitTypes[Slot] == EHktWaitType::None) return;
        
        WaitTypes[Slot] = EHktWaitType::None;
        RuntimeSlots[Slot] = INDEX_NONE;
        FreeSlots.Add(Slot);
        Count--;
    }
    
    /**
     * 활성 대기 항목 순회
     */
    template<typename Func>
    void ForEachActive(Func&& Callback)
    {
        for (int32 i = 0; i < HighWaterMark; ++i)
        {
            if (WaitTypes[i] != EHktWaitType::None)
            {
                Callback(i);
            }
        }
    }
    
    /**
     * 특정 타입의 대기 항목만 순회 (SOA 최적화)
     */
    template<typename Func>
    void ForEachByType(EHktWaitType Type, Func&& Callback)
    {
        for (int32 i = 0; i < HighWaterMark; ++i)
        {
            if (WaitTypes[i] == Type)
            {
                Callback(i);
            }
        }
    }
    
private:
    int32 AllocateSlot()
    {
        int32 Slot = INDEX_NONE;
        
        if (FreeSlots.Num() > 0)
        {
            Slot = FreeSlots.Pop(EAllowShrinking::No);
        }
        else if (HighWaterMark < HKT_WAIT_QUEUE_SIZE)
        {
            Slot = HighWaterMark++;
        }
        else
        {
            // 큐 가득 참
            UE_LOG(LogTemp, Warning, TEXT("[HktWaitQueue] Queue overflow!"));
            return INDEX_NONE;
        }
        
        Count++;
        return Slot;
    }
};

/**
 * 대기 완료 결과
 */
struct FHktWaitCompleteResult
{
    int32 RuntimeSlot = INDEX_NONE;
    int32 RuntimeGeneration = 0;
    EHktYieldCondition CompletedCondition = EHktYieldCondition::None;
    
    // 충돌 대기 완료 시 충돌 대상
    int32 CollisionEntityID = INDEX_NONE;
    int32 CollisionGeneration = 0;
    
    bool IsValid() const { return RuntimeSlot != INDEX_NONE; }
};
