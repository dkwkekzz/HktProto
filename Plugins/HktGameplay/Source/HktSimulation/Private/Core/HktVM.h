#pragma once

#include "CoreMinimal.h"
#include "HktVMTypes.h"
#include "HktVMProgram.h"
#include "HktVMBatch.h"
#include "HktVMDispatch.h"
#include "HktStateStore.h"

// ============================================================================
// HktVM - VM 실행 엔진
// 
// 핵심 루프:
// 1. 시간 대기 중인 VM 타이머 업데이트
// 2. 조건 대기 중인 VM 조건 체크
// 3. 실행 가능한 VM 명령어 실행
// 4. Yield 또는 End까지 반복
//
// 자연어 시간-공간:
// - WaitTime: "N초 기다린다" - VM이 시간을 추상화
// - WaitCollision: "닿을 때까지 기다린다" - VM이 이벤트를 추상화
// - 콜백 없음, 선형 흐름만
// ============================================================================

class FHktVM
{
public:
    // ========================================================================
    // 단일 VM 실행 (한 틱)
    // 
    // Yield 또는 End까지 명령어 실행
    // ========================================================================
    
    static void ExecuteVM(FHktVMBatch& Batch, int32 VMIdx, FHktVMWorld& World)
    {
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        if (!Prog || !Prog->IsValid())
        {
            Batch.States[VMIdx] = EHktVMState::Finished;
            return;
        }
        
        // 명령어 실행 루프
        const int32 MaxInstructions = 1000;  // 무한 루프 방지
        int32 InstructionCount = 0;
        
        while (InstructionCount++ < MaxInstructions)
        {
            // 상태 체크
            EHktVMState State = Batch.States[VMIdx];
            if (State == EHktVMState::Finished ||
                State == EHktVMState::Error ||
                State == EHktVMState::Yielded ||
                State == EHktVMState::WaitingTime ||
                State == EHktVMState::WaitingCondition)
            {
                break;
            }
            
            // PC 체크
            int32 PC = Batch.PCs[VMIdx];
            if (PC < 0 || PC >= Prog->GetInstructionCount())
            {
                Batch.States[VMIdx] = EHktVMState::Finished;
                break;
            }
            
            // 명령어 페치
            const FHktInstruction& Inst = Prog->GetInstruction(PC);
            
            // 명령어 실행 (함수 포인터 테이블)
            int32 NextPC = FHktVMDispatch::Execute(Batch, VMIdx, Inst, World);
            
            // PC 업데이트
            if (NextPC >= 0)
            {
                // 점프
                Batch.PCs[VMIdx] = NextPC;
            }
            else
            {
                // 다음 명령어
                Batch.PCs[VMIdx] = PC + 1;
            }
        }
    }
    
    // ========================================================================
    // 배치 전체 실행 (한 틱)
    // ========================================================================
    
    static void TickBatch(FHktVMBatch& Batch, FHktVMWorld& World)
    {
        // 1단계: 시간 대기 중인 VM 타이머 업데이트
        TickWaitingVMs(Batch, World);
        
        // 2단계: 조건 대기 중인 VM 조건 체크
        CheckWaitConditions(Batch, World);
        
        // 3단계: 실행 가능한 VM 실행
        ExecuteRunningVMs(Batch, World);
    }
    
private:
    // ========================================================================
    // 시간 대기 업데이트
    // ========================================================================
    
    static void TickWaitingVMs(FHktVMBatch& Batch, FHktVMWorld& World)
    {
        const float DeltaTime = World.DeltaTime;
        
        for (int32 i = 0; i < Batch.HighWaterMark; ++i)
        {
            if (Batch.States[i] == EHktVMState::WaitingTime)
            {
                Batch.WaitTimers[i] -= DeltaTime;
                
                if (Batch.WaitTimers[i] <= 0.0f)
                {
                    Batch.States[i] = EHktVMState::Running;
                    Batch.YieldConditions[i] = EHktYieldCondition::None;
                }
            }
        }
    }
    
    // ========================================================================
    // 조건 대기 체크
    // ========================================================================
    
    static void CheckWaitConditions(FHktVMBatch& Batch, FHktVMWorld& World)
    {
        for (int32 i = 0; i < Batch.HighWaterMark; ++i)
        {
            if (Batch.States[i] != EHktVMState::WaitingCondition)
            {
                continue;
            }
            
            switch (Batch.YieldConditions[i])
            {
            case EHktYieldCondition::Collision:
                // 외부에서 LastCollisionEntityID 설정 시 진행
                if (World.LastCollisionEntityID != INDEX_NONE)
                {
                    Batch.States[i] = EHktVMState::Running;
                }
                break;
                
            case EHktYieldCondition::Arrival:
                if (World.StateStore)
                {
                    int32 OwnerID = Batch.OwnerEntityIDs[i];
                    FHktEntityState* State = World.StateStore->Entities.Get(OwnerID);
                    if (State && State->HasArrivedAtTarget())
                    {
                        Batch.States[i] = EHktVMState::Running;
                    }
                }
                break;
                
            case EHktYieldCondition::Custom:
                // 커스텀 조건은 외부에서 상태 변경
                break;
                
            default:
                break;
            }
        }
    }
    
    // ========================================================================
    // 실행 가능한 VM 실행
    // ========================================================================
    
    static void ExecuteRunningVMs(FHktVMBatch& Batch, FHktVMWorld& World)
    {
        for (int32 i = 0; i < Batch.HighWaterMark; ++i)
        {
            if (Batch.States[i] == EHktVMState::Running ||
                Batch.States[i] == EHktVMState::Yielded)
            {
                // Yielded는 다음 틱에 재개
                if (Batch.States[i] == EHktVMState::Yielded)
                {
                    Batch.States[i] = EHktVMState::Running;
                }
                
                ExecuteVM(Batch, i, World);
            }
        }
    }
};

// ============================================================================
// 편의 함수
// ============================================================================

namespace HktVMUtils
{
    // 새 VM 시작
    inline int32 StartVM(
        FHktVMBatch& Batch,
        const FHktProgram* Program,
        int32 OwnerEntityID,
        int32 OwnerGeneration)
    {
        int32 Slot = Batch.Allocate();
        if (Slot != INDEX_NONE)
        {
            Batch.InitVM(Slot, Program, OwnerEntityID, OwnerGeneration);
        }
        return Slot;
    }
    
    // VM 취소
    inline void CancelVM(FHktVMBatch& Batch, int32 Slot)
    {
        if (Slot >= 0 && Slot < HKT_VM_BATCH_SIZE)
        {
            Batch.States[Slot] = EHktVMState::Finished;
        }
    }
    
    // 디버그 출력
    inline void DebugDumpBatch(const FHktVMBatch& Batch)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Log, TEXT("=== HktVMBatch Debug ==="));
        UE_LOG(LogTemp, Log, TEXT("Active: %d, HighWaterMark: %d"), Batch.ActiveCount, Batch.HighWaterMark);
        
        for (int32 i = 0; i < Batch.HighWaterMark; ++i)
        {
            if (Batch.Programs[i])
            {
                UE_LOG(LogTemp, Log, TEXT("[%d] State=%d PC=%d Owner=%d Wait=%.2f"),
                    i, (int32)Batch.States[i], Batch.PCs[i], 
                    Batch.OwnerEntityIDs[i], Batch.WaitTimers[i]);
            }
        }
#endif
    }
}
