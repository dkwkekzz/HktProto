// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktVMTypes.h"
#include "HktVMProgram.h"

// ============================================================================
// HktVM Runtime - AOS 구조의 VM 런타임
// 
// 설계 철학:
// - 개별 VM의 전체 상태를 한 곳에 보관 (AOS)
// - 직관적인 접근과 디버깅
// - 대기 상태는 별도 WaitQueue로 분리하여 SOA 처리
// ============================================================================

/**
 * 개별 VM의 전체 런타임 상태 (AOS)
 * 
 * 이벤트 시작 시 생성되어 완료될 때까지 상태 유지
 */
struct FHktVMRuntime
{
    // ========================================================================
    // 식별 정보
    // ========================================================================
    
    int32 EventID = INDEX_NONE;          // 이벤트 고유 ID (외부에서 부여)
    int32 SlotIndex = INDEX_NONE;        // JobQueue 내 슬롯 인덱스
    int32 Generation = 0;                // 슬롯 재사용 시 증가 (dangling 참조 방지)
    
    // ========================================================================
    // 소유자 정보
    // ========================================================================
    
    int32 OwnerEntityID = INDEX_NONE;
    int32 OwnerGeneration = 0;
    
    // ========================================================================
    // 프로그램 실행 상태
    // ========================================================================
    
    const FHktProgram* Program = nullptr;
    int32 PC = 0;                        // Program Counter
    EHktVMState State = EHktVMState::Ready;
    
    // ========================================================================
    // 레지스터 뱅크
    // ========================================================================
    
    FHktRegister Registers[HKT_NUM_REGISTERS];
    
    // ========================================================================
    // 루프 상태
    // ========================================================================
    
    int32 LoopListRegister = INDEX_NONE;
    int32 LoopIterator = 0;
    int32 LoopReturnPC = INDEX_NONE;
    
    // ========================================================================
    // 대기 상태 참조 (WaitQueue에 있을 때만 유효)
    // ========================================================================
    
    int32 WaitQueueIndex = INDEX_NONE;   // WaitQueue 내 위치
    
    // ========================================================================
    // API
    // ========================================================================
    
    bool IsValid() const { return Program != nullptr && State != EHktVMState::Ready; }
    bool IsActive() const { return IsValid() && State != EHktVMState::Finished && State != EHktVMState::Error; }
    bool IsWaiting() const { return State == EHktVMState::WaitingTime || State == EHktVMState::WaitingCondition; }
    bool IsRunning() const { return State == EHktVMState::Running || State == EHktVMState::Yielded; }
    bool IsFinished() const { return State == EHktVMState::Finished || State == EHktVMState::Error; }
    
    void Reset()
    {
        EventID = INDEX_NONE;
        // SlotIndex와 Generation은 유지
        OwnerEntityID = INDEX_NONE;
        OwnerGeneration = 0;
        Program = nullptr;
        PC = 0;
        State = EHktVMState::Ready;
        LoopListRegister = INDEX_NONE;
        LoopIterator = 0;
        LoopReturnPC = INDEX_NONE;
        WaitQueueIndex = INDEX_NONE;
        FMemory::Memzero(Registers, sizeof(Registers));
    }
    
    void Initialize(int32 InEventID, const FHktProgram* InProgram, int32 InOwnerID, int32 InOwnerGen)
    {
        EventID = InEventID;
        Program = InProgram;
        OwnerEntityID = InOwnerID;
        OwnerGeneration = InOwnerGen;
        PC = 0;
        State = EHktVMState::Running;
        WaitQueueIndex = INDEX_NONE;
        
        // Owner를 R0에 저장
        Registers[0].SetEntity(InOwnerID, InOwnerGen);
    }
    
    // 레지스터 접근
    FORCEINLINE FHktRegister& GetRegister(int32 Index)
    {
        checkf(Index >= 0 && Index < HKT_NUM_REGISTERS, TEXT("Register index out of bounds: %d"), Index);
        return Registers[Index];
    }
    
    FORCEINLINE const FHktRegister& GetRegister(int32 Index) const
    {
        checkf(Index >= 0 && Index < HKT_NUM_REGISTERS, TEXT("Register index out of bounds: %d"), Index);
        return Registers[Index];
    }
};

/**
 * VM 런타임 핸들
 * 
 * 외부에서 VM을 안전하게 참조하기 위한 핸들
 * Generation 체크로 dangling 참조 방지
 */
struct FHktVMHandle
{
    int32 SlotIndex = INDEX_NONE;
    int32 Generation = 0;
    
    bool IsValid() const { return SlotIndex != INDEX_NONE; }
    
    bool operator==(const FHktVMHandle& Other) const
    {
        return SlotIndex == Other.SlotIndex && Generation == Other.Generation;
    }
    
    bool operator!=(const FHktVMHandle& Other) const
    {
        return !(*this == Other);
    }
    
    static FHktVMHandle Invalid() { return FHktVMHandle(); }
};

inline uint32 GetTypeHash(const FHktVMHandle& Handle)
{
    return HashCombine(GetTypeHash(Handle.SlotIndex), GetTypeHash(Handle.Generation));
}
