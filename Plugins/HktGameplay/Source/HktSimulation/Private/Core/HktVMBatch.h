#pragma once

#include "CoreMinimal.h"
#include "HktVMTypes.h"
#include "HktVMProgram.h"

// ============================================================================
// HktVM Batch - SoA 레이아웃 VM 배치
// 
// DOD 핵심:
// - 동일 필드를 연속 메모리에 배치 (캐시 라인 최적화)
// - 배치 단위 처리로 분기 예측 개선
// - alignas(64)로 캐시 라인 경계 정렬
// ============================================================================

// 개별 VM 컨텍스트 (복합 접근 시 사용)
struct FHktVMContext
{
    const FHktProgram* Program = nullptr;
    int32 PC = 0;                       // Program Counter
    EHktVMState State = EHktVMState::Ready;
    EHktYieldCondition YieldCondition = EHktYieldCondition::None;
    float WaitTimer = 0.0f;
    int32 OwnerEntityID = INDEX_NONE;
    int32 OwnerGeneration = 0;
    FHktRegister Registers[HKT_NUM_REGISTERS];
    
    // ForEach 루프 상태
    int32 LoopListRegister = INDEX_NONE;    // 순회 중인 리스트 레지스터
    int32 LoopIterator = 0;                 // 현재 인덱스
    int32 LoopReturnPC = INDEX_NONE;        // 루프 시작 PC
    
    void Reset()
    {
        Program = nullptr;
        PC = 0;
        State = EHktVMState::Ready;
        YieldCondition = EHktYieldCondition::None;
        WaitTimer = 0.0f;
        OwnerEntityID = INDEX_NONE;
        OwnerGeneration = 0;
        LoopListRegister = INDEX_NONE;
        LoopIterator = 0;
        LoopReturnPC = INDEX_NONE;
        FMemory::Memzero(Registers, sizeof(Registers));
    }
};

// ============================================================================
// SoA VM 배치
// 
// 필드별 배열로 캐시 적중률 극대화
// Hot data와 Cold data 분리
// ============================================================================

struct FHktVMBatch
{
    // ========================================================================
    // Hot Data - 매 틱 접근 (캐시 라인 정렬)
    // ========================================================================
    
    // Program Counter (매 명령어 접근)
    alignas(HKT_CACHE_LINE) int32 PCs[HKT_VM_BATCH_SIZE];
    
    // 상태 (조건 체크)
    alignas(HKT_CACHE_LINE) EHktVMState States[HKT_VM_BATCH_SIZE];
    
    // 대기 타이머 (시간 대기 중인 VM만)
    alignas(HKT_CACHE_LINE) float WaitTimers[HKT_VM_BATCH_SIZE];
    
    // ========================================================================
    // Warm Data - 명령어 실행 시 접근
    // ========================================================================
    
    // Yield 조건
    alignas(HKT_CACHE_LINE) EHktYieldCondition YieldConditions[HKT_VM_BATCH_SIZE];
    
    // 프로그램 포인터
    alignas(HKT_CACHE_LINE) const FHktProgram* Programs[HKT_VM_BATCH_SIZE];
    
    // ========================================================================
    // Cold Data - 초기화/완료 시 접근
    // ========================================================================
    
    // 소유자 엔티티
    int32 OwnerEntityIDs[HKT_VM_BATCH_SIZE];
    int32 OwnerGenerations[HKT_VM_BATCH_SIZE];
    
    // 루프 상태
    int32 LoopListRegisters[HKT_VM_BATCH_SIZE];
    int32 LoopIterators[HKT_VM_BATCH_SIZE];
    int32 LoopReturnPCs[HKT_VM_BATCH_SIZE];
    
    // ========================================================================
    // 레지스터 뱅크 (2D: VM × Register)
    // 
    // 레이아웃: Registers[VMIndex][RegisterIndex]
    // 단일 VM의 레지스터들이 연속 -> 단일 VM 연산에 최적
    // ========================================================================
    
    alignas(HKT_CACHE_LINE) FHktRegister Registers[HKT_VM_BATCH_SIZE][HKT_NUM_REGISTERS];
    
    // ========================================================================
    // 관리 데이터
    // ========================================================================
    
    int32 ActiveCount = 0;              // 활성 VM 수
    int32 HighWaterMark = 0;            // 최대 사용 인덱스
    
    // Free list (완료된 VM 슬롯 재사용)
    TArray<int32> FreeSlots;
    
    // ========================================================================
    // API
    // ========================================================================
    
    FHktVMBatch()
    {
        Clear();
    }
    
    void Clear()
    {
        FMemory::Memzero(PCs, sizeof(PCs));
        FMemory::Memzero(States, sizeof(States));
        FMemory::Memzero(WaitTimers, sizeof(WaitTimers));
        FMemory::Memzero(YieldConditions, sizeof(YieldConditions));
        FMemory::Memzero(Programs, sizeof(Programs));
        FMemory::Memzero(OwnerEntityIDs, sizeof(OwnerEntityIDs));
        FMemory::Memzero(OwnerGenerations, sizeof(OwnerGenerations));
        FMemory::Memzero(LoopListRegisters, sizeof(LoopListRegisters));
        FMemory::Memzero(LoopIterators, sizeof(LoopIterators));
        FMemory::Memzero(LoopReturnPCs, sizeof(LoopReturnPCs));
        FMemory::Memzero(Registers, sizeof(Registers));
        
        // LoopListRegisters 초기화
        for (int32 i = 0; i < HKT_VM_BATCH_SIZE; ++i)
        {
            LoopListRegisters[i] = INDEX_NONE;
            LoopReturnPCs[i] = INDEX_NONE;
        }
        
        ActiveCount = 0;
        HighWaterMark = 0;
        FreeSlots.Empty();
    }
    
    // 새 VM 슬롯 할당
    int32 Allocate()
    {
        int32 Slot = INDEX_NONE;
        
        if (FreeSlots.Num() > 0)
        {
            Slot = FreeSlots.Pop(EAllowShrinking::No);
        }
        else if (HighWaterMark < HKT_VM_BATCH_SIZE)
        {
            Slot = HighWaterMark++;
        }
        else
        {
            // 배치 가득 참
            checkf(false, TEXT("VM batch overflow"));
            return INDEX_NONE;
        }
        
        // 슬롯 초기화
        ResetSlot(Slot);
        ActiveCount++;
        return Slot;
    }
    
    // VM 슬롯 해제
    void Free(int32 Slot)
    {
        checkf(Slot >= 0 && Slot < HKT_VM_BATCH_SIZE, TEXT("Invalid slot: %d"), Slot);
        States[Slot] = EHktVMState::Ready;
        Programs[Slot] = nullptr;
        FreeSlots.Add(Slot);
        ActiveCount--;
    }
    
    // 슬롯 초기화
    void ResetSlot(int32 Slot)
    {
        PCs[Slot] = 0;
        States[Slot] = EHktVMState::Ready;
        WaitTimers[Slot] = 0.0f;
        YieldConditions[Slot] = EHktYieldCondition::None;
        Programs[Slot] = nullptr;
        OwnerEntityIDs[Slot] = INDEX_NONE;
        OwnerGenerations[Slot] = 0;
        LoopListRegisters[Slot] = INDEX_NONE;
        LoopIterators[Slot] = 0;
        LoopReturnPCs[Slot] = INDEX_NONE;
        FMemory::Memzero(Registers[Slot], sizeof(Registers[Slot]));
    }
    
    // VM 초기화
    void InitVM(int32 Slot, const FHktProgram* Program, int32 OwnerID, int32 Generation)
    {
        Programs[Slot] = Program;
        OwnerEntityIDs[Slot] = OwnerID;
        OwnerGenerations[Slot] = Generation;
        States[Slot] = EHktVMState::Running;
        
        // Owner를 R0에 저장
        Registers[Slot][0].SetEntity(OwnerID, Generation);
    }
    
    // 단일 VM 컨텍스트로 추출 (디버깅/복합 연산용)
    FHktVMContext ExtractContext(int32 Slot) const
    {
        FHktVMContext Ctx;
        Ctx.Program = Programs[Slot];
        Ctx.PC = PCs[Slot];
        Ctx.State = States[Slot];
        Ctx.YieldCondition = YieldConditions[Slot];
        Ctx.WaitTimer = WaitTimers[Slot];
        Ctx.OwnerEntityID = OwnerEntityIDs[Slot];
        Ctx.OwnerGeneration = OwnerGenerations[Slot];
        Ctx.LoopListRegister = LoopListRegisters[Slot];
        Ctx.LoopIterator = LoopIterators[Slot];
        Ctx.LoopReturnPC = LoopReturnPCs[Slot];
        FMemory::Memcpy(Ctx.Registers, Registers[Slot], sizeof(Ctx.Registers));
        return Ctx;
    }
    
    // 활성 VM 인덱스 순회 (완료되지 않은 것만)
    template<typename Func>
    void ForEachActive(Func&& Callback)
    {
        for (int32 i = 0; i < HighWaterMark; ++i)
        {
            if (States[i] != EHktVMState::Ready && 
                States[i] != EHktVMState::Finished &&
                Programs[i] != nullptr)
            {
                Callback(i);
            }
        }
    }
    
    // 완료된 VM 정리
    void CleanupFinished()
    {
        for (int32 i = HighWaterMark - 1; i >= 0; --i)
        {
            if (States[i] == EHktVMState::Finished && Programs[i] != nullptr)
            {
                Free(i);
            }
        }
    }
    
    // ========================================================================
    // 레지스터 접근 헬퍼
    // ========================================================================
    
    FORCEINLINE FHktRegister& GetRegister(int32 Slot, int32 RegIdx)
    {
        checkf(RegIdx >= 0 && RegIdx < HKT_NUM_REGISTERS, TEXT("Register out of bounds: %d"), RegIdx);
        return Registers[Slot][RegIdx];
    }
    
    FORCEINLINE const FHktRegister& GetRegister(int32 Slot, int32 RegIdx) const
    {
        checkf(RegIdx >= 0 && RegIdx < HKT_NUM_REGISTERS, TEXT("Register out of bounds: %d"), RegIdx);
        return Registers[Slot][RegIdx];
    }
};

// ============================================================================
// 임시 리스트 저장소 (QueryRadius 결과 등)
// 
// VM 레지스터는 64비트이므로 리스트를 직접 저장할 수 없음
// 별도 저장소에 저장하고 레지스터에는 핸들(인덱스) 저장
// ============================================================================

struct FHktListStorage
{
    static constexpr int32 MaxLists = 256;
    static constexpr int32 MaxItemsPerList = 64;
    
    // 고정 크기 리스트 풀 (동적 할당 회피)
    struct FList
    {
        int32 Items[MaxItemsPerList];
        int32 Count = 0;
        bool bInUse = false;
        
        void Clear() { Count = 0; bInUse = false; }
        void Add(int32 Item) { if (Count < MaxItemsPerList) Items[Count++] = Item; }
    };
    
    FList Lists[MaxLists];
    int32 NextFreeIndex = 0;
    
    // 리스트 할당 (핸들 반환)
    int32 AllocateList()
    {
        for (int32 i = 0; i < MaxLists; ++i)
        {
            int32 Idx = (NextFreeIndex + i) % MaxLists;
            if (!Lists[Idx].bInUse)
            {
                Lists[Idx].Clear();
                Lists[Idx].bInUse = true;
                NextFreeIndex = (Idx + 1) % MaxLists;
                return Idx;
            }
        }
        return INDEX_NONE; // 풀 소진
    }
    
    // 리스트 해제
    void FreeList(int32 Handle)
    {
        if (Handle >= 0 && Handle < MaxLists)
        {
            Lists[Handle].Clear();
        }
    }
    
    // 리스트 접근
    FList* GetList(int32 Handle)
    {
        if (Handle >= 0 && Handle < MaxLists && Lists[Handle].bInUse)
        {
            return &Lists[Handle];
        }
        return nullptr;
    }
    
    // 전체 클리어
    void Clear()
    {
        for (int32 i = 0; i < MaxLists; ++i)
        {
            Lists[i].Clear();
        }
        NextFreeIndex = 0;
    }
};
