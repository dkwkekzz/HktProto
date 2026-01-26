// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktVMRuntime.h"

// ============================================================================
// HktJobQueue - AOS 구조의 작업 큐
// 
// 설계 철학:
// - VM 런타임을 AOS로 관리 (직관적 접근)
// - 실행 대기 중인 VM 인덱스만 별도 관리
// - WaitQueue와의 연동으로 효율적인 상태 전이
// ============================================================================

// 최대 동시 VM 수
constexpr int32 HKT_JOB_QUEUE_SIZE = 1024;

/**
 * 작업 큐 (AOS 구조)
 * 
 * VM 런타임의 전체 상태를 관리하고, 실행 대기/완료 상태 추적
 */
struct FHktJobQueue
{
    // ========================================================================
    // VM 런타임 풀 (AOS)
    // ========================================================================
    
    FHktVMRuntime Runtimes[HKT_JOB_QUEUE_SIZE];
    
    // ========================================================================
    // 상태별 인덱스 목록
    // ========================================================================
    
    // 실행 대기 중인 VM (Running/Yielded 상태)
    TArray<int32> PendingSlots;
    
    // 완료된 VM (Finished/Error 상태, 정리 대기)
    TArray<int32> FinishedSlots;
    
    // ========================================================================
    // 관리 데이터
    // ========================================================================
    
    int32 ActiveCount = 0;               // 활성 VM 수 (Ready 제외)
    int32 HighWaterMark = 0;             // 최대 사용 슬롯 + 1
    TArray<int32> FreeSlots;             // 재사용 가능 슬롯
    
    // 이벤트 ID → 슬롯 매핑 (빠른 조회용)
    TMap<int32, int32> EventToSlotMap;
    
    // ========================================================================
    // API
    // ========================================================================
    
    FHktJobQueue()
    {
        Clear();
    }
    
    void Clear()
    {
        for (int32 i = 0; i < HKT_JOB_QUEUE_SIZE; ++i)
        {
            Runtimes[i].Reset();
            Runtimes[i].SlotIndex = i;
        }
        PendingSlots.Empty();
        FinishedSlots.Empty();
        ActiveCount = 0;
        HighWaterMark = 0;
        FreeSlots.Empty();
        EventToSlotMap.Empty();
    }
    
    /**
     * 새 VM 런타임 할당
     * @return 할당된 슬롯 인덱스, 실패 시 INDEX_NONE
     */
    int32 Allocate()
    {
        int32 Slot = INDEX_NONE;
        
        if (FreeSlots.Num() > 0)
        {
            Slot = FreeSlots.Pop(EAllowShrinking::No);
        }
        else if (HighWaterMark < HKT_JOB_QUEUE_SIZE)
        {
            Slot = HighWaterMark++;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[HktJobQueue] Queue overflow!"));
            return INDEX_NONE;
        }
        
        // 슬롯 초기화 (Generation 증가)
        FHktVMRuntime& Runtime = Runtimes[Slot];
        Runtime.Reset();
        Runtime.SlotIndex = Slot;
        Runtime.Generation++;
        
        ActiveCount++;
        return Slot;
    }
    
    /**
     * VM 런타임 해제
     */
    void Free(int32 Slot)
    {
        if (Slot < 0 || Slot >= HKT_JOB_QUEUE_SIZE) return;
        
        FHktVMRuntime& Runtime = Runtimes[Slot];
        if (Runtime.State == EHktVMState::Ready) return; // 이미 해제됨
        
        // 이벤트 매핑 제거
        if (Runtime.EventID != INDEX_NONE)
        {
            EventToSlotMap.Remove(Runtime.EventID);
        }
        
        // PendingSlots에서 제거
        PendingSlots.RemoveSingle(Slot);
        FinishedSlots.RemoveSingle(Slot);
        
        Runtime.Reset();
        FreeSlots.Add(Slot);
        ActiveCount--;
    }
    
    /**
     * VM 초기화 및 시작
     */
    FHktVMHandle StartVM(int32 EventID, const FHktProgram* Program, int32 OwnerEntityID, int32 OwnerGeneration)
    {
        int32 Slot = Allocate();
        if (Slot == INDEX_NONE)
        {
            return FHktVMHandle::Invalid();
        }
        
        FHktVMRuntime& Runtime = Runtimes[Slot];
        Runtime.Initialize(EventID, Program, OwnerEntityID, OwnerGeneration);
        
        // 이벤트 매핑 등록
        if (EventID != INDEX_NONE)
        {
            EventToSlotMap.Add(EventID, Slot);
        }
        
        // 실행 대기 목록에 추가
        PendingSlots.Add(Slot);
        
        return FHktVMHandle{ Slot, Runtime.Generation };
    }
    
    /**
     * 핸들로 런타임 가져오기 (유효성 검사 포함)
     */
    FHktVMRuntime* Get(const FHktVMHandle& Handle)
    {
        if (!Handle.IsValid()) return nullptr;
        if (Handle.SlotIndex >= HKT_JOB_QUEUE_SIZE) return nullptr;
        
        FHktVMRuntime& Runtime = Runtimes[Handle.SlotIndex];
        if (Runtime.Generation != Handle.Generation) return nullptr;
        if (!Runtime.IsValid()) return nullptr;
        
        return &Runtime;
    }
    
    const FHktVMRuntime* Get(const FHktVMHandle& Handle) const
    {
        return const_cast<FHktJobQueue*>(this)->Get(Handle);
    }
    
    /**
     * 슬롯으로 런타임 가져오기 (직접 접근, 내부용)
     */
    FORCEINLINE FHktVMRuntime& GetBySlot(int32 Slot)
    {
        checkf(Slot >= 0 && Slot < HKT_JOB_QUEUE_SIZE, TEXT("Invalid slot: %d"), Slot);
        return Runtimes[Slot];
    }
    
    FORCEINLINE const FHktVMRuntime& GetBySlot(int32 Slot) const
    {
        checkf(Slot >= 0 && Slot < HKT_JOB_QUEUE_SIZE, TEXT("Invalid slot: %d"), Slot);
        return Runtimes[Slot];
    }
    
    /**
     * 이벤트 ID로 런타임 가져오기
     */
    FHktVMRuntime* GetByEventID(int32 EventID)
    {
        const int32* SlotPtr = EventToSlotMap.Find(EventID);
        if (!SlotPtr) return nullptr;
        
        FHktVMRuntime& Runtime = Runtimes[*SlotPtr];
        if (!Runtime.IsValid()) return nullptr;
        
        return &Runtime;
    }
    
    /**
     * 실행 대기 목록에 추가 (WaitQueue에서 복귀 시)
     */
    void AddToPending(int32 Slot)
    {
        if (Slot < 0 || Slot >= HKT_JOB_QUEUE_SIZE) return;
        
        FHktVMRuntime& Runtime = Runtimes[Slot];
        if (!Runtime.IsValid()) return;
        
        // 중복 방지
        if (!PendingSlots.Contains(Slot))
        {
            PendingSlots.Add(Slot);
        }
    }
    
    /**
     * 실행 대기 목록에서 제거 (WaitQueue로 이동 시)
     */
    void RemoveFromPending(int32 Slot)
    {
        PendingSlots.RemoveSingle(Slot);
    }
    
    /**
     * 완료 목록에 추가
     */
    void AddToFinished(int32 Slot)
    {
        if (Slot < 0 || Slot >= HKT_JOB_QUEUE_SIZE) return;
        
        // 실행 대기에서 제거
        PendingSlots.RemoveSingle(Slot);
        
        // 중복 방지
        if (!FinishedSlots.Contains(Slot))
        {
            FinishedSlots.Add(Slot);
        }
    }
    
    /**
     * 활성 VM 순회
     */
    template<typename Func>
    void ForEachActive(Func&& Callback)
    {
        for (int32 i = 0; i < HighWaterMark; ++i)
        {
            if (Runtimes[i].IsActive())
            {
                Callback(i);
            }
        }
    }
    
    /**
     * 실행 대기 VM 순회 (틱에서 실행할 것들)
     */
    template<typename Func>
    void ForEachPending(Func&& Callback)
    {
        // 복사본으로 순회 (Callback 내에서 목록 변경 가능)
        TArray<int32> CurrentPending = PendingSlots;
        for (int32 Slot : CurrentPending)
        {
            if (Runtimes[Slot].IsRunning())
            {
                Callback(Slot);
            }
        }
    }
    
    /**
     * 핸들 생성
     */
    FHktVMHandle MakeHandle(int32 Slot) const
    {
        if (Slot < 0 || Slot >= HKT_JOB_QUEUE_SIZE)
        {
            return FHktVMHandle::Invalid();
        }
        return FHktVMHandle{ Slot, Runtimes[Slot].Generation };
    }
    
    /**
     * 디버그 정보
     */
    void DebugDump() const
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Log, TEXT("=== HktJobQueue Debug ==="));
        UE_LOG(LogTemp, Log, TEXT("Active: %d, HighWaterMark: %d, Pending: %d, Finished: %d"),
            ActiveCount, HighWaterMark, PendingSlots.Num(), FinishedSlots.Num());
        
        for (int32 i = 0; i < HighWaterMark; ++i)
        {
            const FHktVMRuntime& Runtime = Runtimes[i];
            if (Runtime.IsValid())
            {
                UE_LOG(LogTemp, Log, TEXT("[%d] Event=%d State=%d PC=%d Owner=%d"),
                    i, Runtime.EventID, (int32)Runtime.State, Runtime.PC, Runtime.OwnerEntityID);
            }
        }
#endif
    }
};
