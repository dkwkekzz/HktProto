#pragma once

#include "CoreMinimal.h"
#include "VMTypes.h"
#include "VMRuntime.h"

// Forward declarations
class UStashComponent;

/**
 * FVMInterpreter - 바이트코드 인터프리터
 * 
 * 단일 VM을 yield 또는 종료까지 실행합니다.
 */
class HKTSIMULATION_API FVMInterpreter
{
public:
    void Initialize();
    
    /** VM을 yield/완료/실패까지 실행 */
    EVMStatus Execute(FVMRuntime& Runtime);
    
    /** 이벤트 완료 알림 (외부에서 호출) */
    void NotifyCollision(FVMRuntime& Runtime, EntityId HitEntity);
    void NotifyAnimEnd(FVMRuntime& Runtime);
    void NotifyMoveEnd(FVMRuntime& Runtime);
    void UpdateTimer(FVMRuntime& Runtime, float DeltaSeconds);

private:
    EVMStatus ExecuteInstruction(FVMRuntime& Runtime, const FInstruction& Inst);
    
    // ===== Control Flow =====
    void Op_Nop(FVMRuntime& Runtime);
    EVMStatus Op_Halt(FVMRuntime& Runtime);
    EVMStatus Op_Yield(FVMRuntime& Runtime, int32 Frames);
    EVMStatus Op_YieldSeconds(FVMRuntime& Runtime, int32 DeciMillis);
    void Op_Jump(FVMRuntime& Runtime, int32 Target);
    void Op_JumpIf(FVMRuntime& Runtime, RegisterIndex Cond, int32 Target);
    void Op_JumpIfNot(FVMRuntime& Runtime, RegisterIndex Cond, int32 Target);
    
    // ===== Event Wait =====
    EVMStatus Op_WaitCollision(FVMRuntime& Runtime, RegisterIndex WatchEntity);
    EVMStatus Op_WaitAnimEnd(FVMRuntime& Runtime, RegisterIndex Entity);
    EVMStatus Op_WaitMoveEnd(FVMRuntime& Runtime, RegisterIndex Entity);
    
    // ===== Data Operations =====
    void Op_LoadConst(FVMRuntime& Runtime, RegisterIndex Dst, int32 Value);
    void Op_LoadConstHigh(FVMRuntime& Runtime, RegisterIndex Dst, int32 HighBits);
    void Op_LoadStore(FVMRuntime& Runtime, RegisterIndex Dst, uint16 PropertyId);
    void Op_LoadStoreEntity(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Entity, uint16 PropertyId);
    void Op_SaveStore(FVMRuntime& Runtime, uint16 PropertyId, RegisterIndex Src);
    void Op_SaveStoreEntity(FVMRuntime& Runtime, RegisterIndex Entity, uint16 PropertyId, RegisterIndex Src);
    void Op_Move(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src);
    
    // ===== Arithmetic =====
    void Op_Add(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_Sub(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_Mul(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_Div(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_Mod(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_AddImm(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src, int32 Imm);
    
    // ===== Comparison =====
    void Op_CmpEq(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_CmpNe(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_CmpLt(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_CmpLe(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_CmpGt(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    void Op_CmpGe(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Src1, RegisterIndex Src2);
    
    // ===== Entity Management =====
    void Op_SpawnEntity(FVMRuntime& Runtime, int32 StringIndex);
    void Op_DestroyEntity(FVMRuntime& Runtime, RegisterIndex Entity);
    
    // ===== Position & Movement =====
    void Op_GetPosition(FVMRuntime& Runtime, RegisterIndex DstBase, RegisterIndex Entity);
    void Op_SetPosition(FVMRuntime& Runtime, RegisterIndex Entity, RegisterIndex SrcBase);
    void Op_GetDistance(FVMRuntime& Runtime, RegisterIndex Dst, RegisterIndex Entity1, RegisterIndex Entity2);
    void Op_MoveToward(FVMRuntime& Runtime, RegisterIndex Entity, RegisterIndex TargetBase, int32 Speed);
    void Op_MoveForward(FVMRuntime& Runtime, RegisterIndex Entity, int32 Speed);
    void Op_StopMovement(FVMRuntime& Runtime, RegisterIndex Entity);
    
    // ===== Spatial Query =====
    void Op_FindInRadius(FVMRuntime& Runtime, RegisterIndex CenterEntity, int32 RadiusCm);
    void Op_NextFound(FVMRuntime& Runtime);
    
    // ===== Combat =====
    void Op_ApplyDamage(FVMRuntime& Runtime, RegisterIndex Target, RegisterIndex Amount);
    void Op_ApplyEffect(FVMRuntime& Runtime, RegisterIndex Target, int32 StringIndex);
    void Op_RemoveEffect(FVMRuntime& Runtime, RegisterIndex Target, int32 StringIndex);
    
    // ===== Animation & VFX =====
    void Op_PlayAnim(FVMRuntime& Runtime, RegisterIndex Entity, int32 StringIndex);
    void Op_PlayAnimMontage(FVMRuntime& Runtime, RegisterIndex Entity, int32 StringIndex);
    void Op_StopAnim(FVMRuntime& Runtime, RegisterIndex Entity);
    void Op_PlayVFX(FVMRuntime& Runtime, RegisterIndex PosBase, int32 StringIndex);
    void Op_PlayVFXAttached(FVMRuntime& Runtime, RegisterIndex Entity, int32 StringIndex);
    
    // ===== Audio =====
    void Op_PlaySound(FVMRuntime& Runtime, int32 StringIndex);
    void Op_PlaySoundAtLocation(FVMRuntime& Runtime, RegisterIndex PosBase, int32 StringIndex);
    
    // ===== Equipment =====
    void Op_SpawnEquipment(FVMRuntime& Runtime, RegisterIndex Owner, int32 Slot, int32 StringIndex);
    
    // ===== Utility =====
    void Op_Log(FVMRuntime& Runtime, int32 StringIndex);
    
    // ===== Helper =====
    const FString& GetString(FVMRuntime& Runtime, int32 Index);

private:
    static constexpr int32 MaxInstructionsPerTick = 10000;
};