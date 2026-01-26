// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktSimulationProcessor.h"
#include "Core/HktVMTypes.h"
#include "Core/HktVMProgram.h"
#include "Core/HktStateTypes.h"
#include "Core/HktVMRuntime.h"

// ============================================================================
// HktExecuteProcessor - 명령어 실행 Processor
// 
// 책임:
// - JobQueue의 PendingSlots에 있는 VM 실행
// - 명령어 Yield 시 WaitQueue로 이동
// - 완료 시 FinishedSlots로 이동
// ============================================================================

// 전방 선언
struct FHktVMRuntime;

/**
 * 명령어 실행 결과
 */
enum class EHktExecuteResult : uint8
{
    Continue,       // 계속 실행
    Yield,          // 다음 틱에 계속
    WaitTime,       // 시간 대기로 전환
    WaitArrival,    // 도착 대기로 전환
    WaitCollision,  // 충돌 대기로 전환
    WaitCustom,     // 커스텀 조건 대기로 전환
    Finished,       // 실행 완료
    Error,          // 에러 발생
};

/**
 * 실행 컨텍스트 (단일 VM용)
 */
struct FHktExecuteContext
{
    FHktSimulationContext* SimContext = nullptr;
    FHktVMRuntime* Runtime = nullptr;
    int32 RuntimeSlot = INDEX_NONE;
    
    // 결과
    EHktExecuteResult Result = EHktExecuteResult::Continue;
    float WaitDuration = 0.0f;       // WaitTime용
    int32 WaitEntityID = INDEX_NONE; // WaitArrival/Collision용
    int32 WaitEntityGen = 0;
    
    bool IsValid() const { return SimContext && Runtime && RuntimeSlot != INDEX_NONE; }
};

/**
 * 명령어 실행 Processor
 */
class FHktExecuteProcessor
{
public:
    /**
     * 실행 대기 중인 VM들 처리
     */
    static void Process(FHktSimulationContext& Context);
    
private:
    /**
     * 단일 VM 실행 (Yield/End까지)
     */
    static void ExecuteVM(FHktExecuteContext& ExecContext);
    
    /**
     * 단일 명령어 실행
     * @return 다음 PC (점프 시), -1이면 PC+1
     */
    static int32 ExecuteInstruction(FHktExecuteContext& ExecContext, const FHktInstruction& Inst);
    
    /**
     * 실행 결과에 따른 후처리
     */
    static void HandleExecuteResult(FHktExecuteContext& ExecContext);
};

// ============================================================================
// 명령어 핸들러 (새로운 시그니처)
// ============================================================================

namespace HktOpsNew
{
    // 핸들러 타입: 실행 컨텍스트를 받고, 다음 PC 반환 (-1이면 PC+1)
    using FHktOpHandlerNew = int32(*)(FHktExecuteContext& Ctx, const FHktInstruction& Inst);
    
    // === 기본 명령어 ===
    
    inline int32 Op_Nop(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        return -1;
    }
    
    inline int32 Op_End(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        Ctx.Result = EHktExecuteResult::Finished;
        return -1;
    }
    
    inline int32 Op_Yield(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        Ctx.Result = EHktExecuteResult::Yield;
        return -1;
    }
    
    // === 대기 명령어 ===
    
    inline int32 Op_WaitTime(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        const FHktProgram* Prog = Ctx.Runtime->Program;
        Ctx.WaitDuration = Prog->GetConst(Inst.Offset).FloatVal;
        Ctx.Result = EHktExecuteResult::WaitTime;
        return -1;
    }
    
    inline int32 Op_WaitArrival(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        // Owner 엔티티의 도착 대기
        Ctx.WaitEntityID = Ctx.Runtime->OwnerEntityID;
        Ctx.WaitEntityGen = Ctx.Runtime->OwnerGeneration;
        Ctx.Result = EHktExecuteResult::WaitArrival;
        return -1;
    }
    
    inline int32 Op_WaitCollision(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        // 외부 충돌 이벤트가 이미 있으면 즉시 진행
        if (Ctx.SimContext->LastCollisionEntityID != INDEX_NONE)
        {
            // 충돌 엔티티를 레지스터에 저장
            Ctx.Runtime->GetRegister(Inst.A).SetEntity(
                Ctx.SimContext->LastCollisionEntityID,
                Ctx.SimContext->LastCollisionGeneration
            );
            Ctx.SimContext->LastCollisionEntityID = INDEX_NONE;
            Ctx.SimContext->LastCollisionGeneration = 0;
            return -1;
        }
        
        Ctx.Result = EHktExecuteResult::WaitCollision;
        return -1;
    }
    
    inline int32 Op_WaitCondition(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        Ctx.Result = EHktExecuteResult::WaitCustom;
        return -1;
    }
    
    // === 이동 명령어 ===
    
    inline int32 Op_MoveTo(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        FHktStateStore* StateStore = Ctx.SimContext->StateStore;
        if (!StateStore) return -1;
        
        FHktVMRuntime* Runtime = Ctx.Runtime;
        FHktEntityState* State = StateStore->Entities.Get(Runtime->OwnerEntityID);
        if (!State) return -1;
        
        const FHktProgram* Prog = Runtime->Program;
        const FHktConstant& TargetConst = Prog->GetConst(Inst.Offset);
        
        State->TargetX = HktFloatToFixed(TargetConst.VecVal[0]);
        State->TargetY = HktFloatToFixed(TargetConst.VecVal[1]);
        State->TargetZ = HktFloatToFixed(TargetConst.VecVal[2]);
        
        // 목표 방향으로 속도 계산
        float DX = TargetConst.VecVal[0] - HktFixedToFloat(State->PosX);
        float DY = TargetConst.VecVal[1] - HktFixedToFloat(State->PosY);
        float DZ = TargetConst.VecVal[2] - HktFixedToFloat(State->PosZ);
        float Dist = FMath::Sqrt(DX*DX + DY*DY + DZ*DZ);
        
        if (Dist > KINDA_SMALL_NUMBER)
        {
            float Speed = State->Attributes.GetFloat(HktAttrKeys::Speed, 300.0f);
            float InvDist = 1.0f / Dist;
            State->VelX = HktFloatToFixed(DX * InvDist * Speed);
            State->VelY = HktFloatToFixed(DY * InvDist * Speed);
            State->VelZ = HktFloatToFixed(DZ * InvDist * Speed);
            State->SetFlag(FHktEntityState::FLAG_MOVING);
        }
        
        return -1;
    }
    
    inline int32 Op_Stop(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        FHktStateStore* StateStore = Ctx.SimContext->StateStore;
        if (!StateStore) return -1;
        
        FHktEntityState* State = StateStore->Entities.Get(Ctx.Runtime->OwnerEntityID);
        if (!State) return -1;
        
        State->VelX = 0;
        State->VelY = 0;
        State->VelZ = 0;
        State->ClearFlag(FHktEntityState::FLAG_MOVING);
        
        return -1;
    }
    
    // === 점프 명령어 ===
    
    inline int32 Op_Jump(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        return Inst.Offset;
    }
    
    inline int32 Op_JumpIfZero(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        if (Ctx.Runtime->GetRegister(Inst.A).I64 == 0)
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    inline int32 Op_JumpIfNotZero(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        if (Ctx.Runtime->GetRegister(Inst.A).I64 != 0)
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // === 레지스터 명령어 ===
    
    inline int32 Op_LoadConst(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        const FHktProgram* Prog = Ctx.Runtime->Program;
        const FHktConstant& Const = Prog->GetConst(Inst.Offset);
        
        switch (Const.Type)
        {
        case EHktConstType::Int32:
            Ctx.Runtime->GetRegister(Inst.A).SetInt(Const.IntVal);
            break;
        case EHktConstType::Float:
            Ctx.Runtime->GetRegister(Inst.A).SetFloat(Const.FloatVal);
            break;
        default:
            break;
        }
        
        return -1;
    }
    
    inline int32 Op_Copy(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        Ctx.Runtime->GetRegister(Inst.B).Raw = Ctx.Runtime->GetRegister(Inst.A).Raw;
        return -1;
    }
    
    // === 산술 명령어 ===
    
    inline int32 Op_Add(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        Ctx.Runtime->GetRegister(Inst.C).F64 = 
            Ctx.Runtime->GetRegister(Inst.A).F64 + Ctx.Runtime->GetRegister(Inst.B).F64;
        return -1;
    }
    
    inline int32 Op_Sub(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        Ctx.Runtime->GetRegister(Inst.C).F64 = 
            Ctx.Runtime->GetRegister(Inst.A).F64 - Ctx.Runtime->GetRegister(Inst.B).F64;
        return -1;
    }
    
    inline int32 Op_Mul(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        Ctx.Runtime->GetRegister(Inst.C).F64 = 
            Ctx.Runtime->GetRegister(Inst.A).F64 * Ctx.Runtime->GetRegister(Inst.B).F64;
        return -1;
    }
    
    inline int32 Op_Div(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        double Divisor = Ctx.Runtime->GetRegister(Inst.B).F64;
        if (FMath::Abs(Divisor) > KINDA_SMALL_NUMBER)
        {
            Ctx.Runtime->GetRegister(Inst.C).F64 = 
                Ctx.Runtime->GetRegister(Inst.A).F64 / Divisor;
        }
        return -1;
    }
    
    // === 엔티티 명령어 ===
    
    inline int32 Op_Spawn(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        FHktStateStore* StateStore = Ctx.SimContext->StateStore;
        if (!StateStore) return -1;
        
        const FHktProgram* Prog = Ctx.Runtime->Program;
        const FGameplayTag& EntityTag = Prog->GetTag(Inst.D);
        
        int32 NewID = StateStore->Entities.Allocate(EntityTag);
        if (NewID != INDEX_NONE)
        {
            FHktEntityState* State = StateStore->Entities.Get(NewID);
            int32 NewGen = State ? State->Generation : 0;
            Ctx.Runtime->GetRegister(Inst.A).SetEntity(NewID, NewGen);
            
            if (Ctx.SimContext->bDebugLog)
            {
                UE_LOG(LogTemp, Log, TEXT("[ExecuteProcessor] Spawn: %s -> ID=%d"), 
                    *EntityTag.ToString(), NewID);
            }
        }
        
        return -1;
    }
    
    inline int32 Op_Destroy(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        FHktStateStore* StateStore = Ctx.SimContext->StateStore;
        if (!StateStore) return -1;
        
        FHktRegister& Reg = Ctx.Runtime->GetRegister(Inst.A);
        StateStore->Entities.Free(Reg.GetEntityID());
        
        return -1;
    }
    
    inline int32 Op_Damage(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        FHktStateStore* StateStore = Ctx.SimContext->StateStore;
        if (!StateStore) return -1;
        
        FHktRegister& TargetReg = Ctx.Runtime->GetRegister(Inst.A);
        int32 TargetID = TargetReg.GetEntityID();
        int32 TargetGen = TargetReg.GetGeneration();
        
        if (!StateStore->Entities.IsValid(TargetID, TargetGen))
        {
            return -1;
        }
        
        FHktEntityState* State = StateStore->Entities.Get(TargetID);
        if (!State) return -1;
        
        const FHktProgram* Prog = Ctx.Runtime->Program;
        float Damage = Prog->GetConst(Inst.Offset).FloatVal;
        
        float Health = State->Attributes.GetFloat(HktAttrKeys::Health, 0.0f);
        Health -= Damage;
        
        if (Health <= 0.0f)
        {
            Health = 0.0f;
            State->ClearFlag(FHktEntityState::FLAG_ALIVE);
        }
        State->Attributes.SetFloat(HktAttrKeys::Health, Health);
        
        return -1;
    }
    
    // TODO: 나머지 명령어들도 이 패턴으로 구현
    
} // namespace HktOpsNew

// ============================================================================
// 명령어 테이블 (새로운 버전)
// ============================================================================

class FHktExecuteDispatch
{
public:
    static HktOpsNew::FHktOpHandlerNew OpTable[static_cast<int32>(EHktOp::MAX)];
    
    static void Initialize()
    {
        // 기본값: Nop
        for (int32 i = 0; i < static_cast<int32>(EHktOp::MAX); ++i)
        {
            OpTable[i] = &HktOpsNew::Op_Nop;
        }
        
        // 명령어 등록
        OpTable[static_cast<int32>(EHktOp::Nop)]              = &HktOpsNew::Op_Nop;
        OpTable[static_cast<int32>(EHktOp::End)]              = &HktOpsNew::Op_End;
        OpTable[static_cast<int32>(EHktOp::Yield)]            = &HktOpsNew::Op_Yield;
        OpTable[static_cast<int32>(EHktOp::WaitTime)]         = &HktOpsNew::Op_WaitTime;
        OpTable[static_cast<int32>(EHktOp::WaitArrival)]      = &HktOpsNew::Op_WaitArrival;
        OpTable[static_cast<int32>(EHktOp::WaitCollision)]    = &HktOpsNew::Op_WaitCollision;
        OpTable[static_cast<int32>(EHktOp::WaitCondition)]    = &HktOpsNew::Op_WaitCondition;
        OpTable[static_cast<int32>(EHktOp::Spawn)]            = &HktOpsNew::Op_Spawn;
        OpTable[static_cast<int32>(EHktOp::Destroy)]          = &HktOpsNew::Op_Destroy;
        OpTable[static_cast<int32>(EHktOp::MoveTo)]           = &HktOpsNew::Op_MoveTo;
        OpTable[static_cast<int32>(EHktOp::Stop)]             = &HktOpsNew::Op_Stop;
        OpTable[static_cast<int32>(EHktOp::Damage)]           = &HktOpsNew::Op_Damage;
        OpTable[static_cast<int32>(EHktOp::Jump)]             = &HktOpsNew::Op_Jump;
        OpTable[static_cast<int32>(EHktOp::JumpIfZero)]       = &HktOpsNew::Op_JumpIfZero;
        OpTable[static_cast<int32>(EHktOp::JumpIfNotZero)]    = &HktOpsNew::Op_JumpIfNotZero;
        OpTable[static_cast<int32>(EHktOp::LoadConst)]        = &HktOpsNew::Op_LoadConst;
        OpTable[static_cast<int32>(EHktOp::Copy)]             = &HktOpsNew::Op_Copy;
        OpTable[static_cast<int32>(EHktOp::Add)]              = &HktOpsNew::Op_Add;
        OpTable[static_cast<int32>(EHktOp::Sub)]              = &HktOpsNew::Op_Sub;
        OpTable[static_cast<int32>(EHktOp::Mul)]              = &HktOpsNew::Op_Mul;
        OpTable[static_cast<int32>(EHktOp::Div)]              = &HktOpsNew::Op_Div;
    }
    
    FORCEINLINE static int32 Execute(FHktExecuteContext& Ctx, const FHktInstruction& Inst)
    {
        return OpTable[Inst.Op](Ctx, Inst);
    }
};

// ============================================================================
// Processor 구현
// ============================================================================

inline void FHktExecuteProcessor::Process(FHktSimulationContext& Context)
{
    if (!Context.IsValid()) return;
    
    FHktJobQueue* JQ = Context.JobQueue;
    
    // PendingSlots의 복사본으로 순회 (순회 중 수정 가능)
    TArray<int32> CurrentPending = JQ->PendingSlots;
    
    for (int32 Slot : CurrentPending)
    {
        FHktVMRuntime& Runtime = JQ->GetBySlot(Slot);
        
        // 실행 가능 상태인지 확인
        if (!Runtime.IsRunning())
        {
            continue;
        }
        
        // Yielded 상태였다면 Running으로 전환
        if (Runtime.State == EHktVMState::Yielded)
        {
            Runtime.State = EHktVMState::Running;
        }
        
        // 실행 컨텍스트 생성
        FHktExecuteContext ExecContext;
        ExecContext.SimContext = &Context;
        ExecContext.Runtime = &Runtime;
        ExecContext.RuntimeSlot = Slot;
        
        // VM 실행
        ExecuteVM(ExecContext);
        
        // 결과 처리
        HandleExecuteResult(ExecContext);
    }
}

inline void FHktExecuteProcessor::ExecuteVM(FHktExecuteContext& ExecContext)
{
    FHktVMRuntime* Runtime = ExecContext.Runtime;
    const FHktProgram* Prog = Runtime->Program;
    
    if (!Prog || !Prog->IsValid())
    {
        ExecContext.Result = EHktExecuteResult::Error;
        return;
    }
    
    const int32 MaxInstructions = 1000;  // 무한 루프 방지
    int32 InstructionCount = 0;
    
    while (InstructionCount++ < MaxInstructions)
    {
        // 결과 체크 (이전 명령어에서 중단 요청)
        if (ExecContext.Result != EHktExecuteResult::Continue)
        {
            break;
        }
        
        // PC 체크
        if (Runtime->PC < 0 || Runtime->PC >= Prog->GetInstructionCount())
        {
            ExecContext.Result = EHktExecuteResult::Finished;
            break;
        }
        
        // 명령어 페치 및 실행
        const FHktInstruction& Inst = Prog->GetInstruction(Runtime->PC);
        int32 NextPC = ExecuteInstruction(ExecContext, Inst);
        
        // PC 업데이트
        if (NextPC >= 0)
        {
            Runtime->PC = NextPC;
        }
        else
        {
            Runtime->PC++;
        }
    }
}

inline int32 FHktExecuteProcessor::ExecuteInstruction(FHktExecuteContext& ExecContext, const FHktInstruction& Inst)
{
    return FHktExecuteDispatch::Execute(ExecContext, Inst);
}

inline void FHktExecuteProcessor::HandleExecuteResult(FHktExecuteContext& ExecContext)
{
    FHktSimulationContext* Context = ExecContext.SimContext;
    FHktVMRuntime* Runtime = ExecContext.Runtime;
    int32 Slot = ExecContext.RuntimeSlot;
    
    switch (ExecContext.Result)
    {
    case EHktExecuteResult::Continue:
    case EHktExecuteResult::Yield:
        // 다음 틱에 계속 (PendingSlots에 유지)
        Runtime->State = EHktVMState::Yielded;
        break;
        
    case EHktExecuteResult::WaitTime:
        {
            // WaitQueue로 이동
            Runtime->State = EHktVMState::WaitingTime;
            Context->JobQueue->RemoveFromPending(Slot);
            
            int32 WaitSlot = Context->WaitQueue->AddTimeWait(
                Slot, Runtime->Generation, ExecContext.WaitDuration);
            Runtime->WaitQueueIndex = WaitSlot;
            
            if (Context->bDebugLog)
            {
                UE_LOG(LogTemp, Log, TEXT("[ExecuteProcessor] VM[%d] waiting time: %.2fs"),
                    Slot, ExecContext.WaitDuration);
            }
        }
        break;
        
    case EHktExecuteResult::WaitArrival:
        {
            Runtime->State = EHktVMState::WaitingCondition;
            Context->JobQueue->RemoveFromPending(Slot);
            
            int32 WaitSlot = Context->WaitQueue->AddArrivalWait(
                Slot, Runtime->Generation,
                ExecContext.WaitEntityID, ExecContext.WaitEntityGen);
            Runtime->WaitQueueIndex = WaitSlot;
            
            if (Context->bDebugLog)
            {
                UE_LOG(LogTemp, Log, TEXT("[ExecuteProcessor] VM[%d] waiting arrival"),
                    Slot);
            }
        }
        break;
        
    case EHktExecuteResult::WaitCollision:
        {
            Runtime->State = EHktVMState::WaitingCondition;
            Context->JobQueue->RemoveFromPending(Slot);
            
            int32 WaitSlot = Context->WaitQueue->AddCollisionWait(
                Slot, Runtime->Generation);
            Runtime->WaitQueueIndex = WaitSlot;
        }
        break;
        
    case EHktExecuteResult::WaitCustom:
        {
            Runtime->State = EHktVMState::WaitingCondition;
            Context->JobQueue->RemoveFromPending(Slot);
            
            int32 WaitSlot = Context->WaitQueue->AddCustomWait(
                Slot, Runtime->Generation);
            Runtime->WaitQueueIndex = WaitSlot;
        }
        break;
        
    case EHktExecuteResult::Finished:
        {
            Runtime->State = EHktVMState::Finished;
            Context->JobQueue->AddToFinished(Slot);
            
            // 완료 이벤트 수집
            if (Runtime->EventID != INDEX_NONE)
            {
                Context->CompletedEventIDs.Add(Runtime->EventID);
            }
            
            if (Context->bDebugLog)
            {
                UE_LOG(LogTemp, Log, TEXT("[ExecuteProcessor] VM[%d] finished, EventID=%d"),
                    Slot, Runtime->EventID);
            }
        }
        break;
        
    case EHktExecuteResult::Error:
        {
            Runtime->State = EHktVMState::Error;
            Context->JobQueue->AddToFinished(Slot);
            
            UE_LOG(LogTemp, Warning, TEXT("[ExecuteProcessor] VM[%d] error, EventID=%d"),
                Slot, Runtime->EventID);
        }
        break;
    }
}
