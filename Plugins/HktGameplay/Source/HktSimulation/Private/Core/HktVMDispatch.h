#pragma once

#include "CoreMinimal.h"
#include "HktVMTypes.h"
#include "HktVMBatch.h"
#include "HktStateTypes.h"
#include "HktStateStore.h"
#include "HktStateCache.h"

// ============================================================================
// HktVM Dispatch - 명령어 디스패처
// 
// 핵심: switch 대신 함수 포인터 테이블로 직접 점프
// - 분기 예측 개선
// - 명령어 추가 시 테이블에만 등록
// - 각 명령어는 독립적인 함수로 구현
// ============================================================================

// 전방 선언
struct FHktVMWorld;

// ============================================================================
// 명령어 핸들러 타입
// 
// 파라미터:
// - Batch: VM 배치 (SoA)
// - VMIdx: 현재 실행 중인 VM 인덱스
// - Inst: 현재 명령어
// - World: 월드 컨텍스트 (상태, 리스트 등)
// 
// 반환: 다음 PC (점프 시), -1이면 현재 PC+1
// ============================================================================

using FHktOpHandler = int32(*)(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World);

// ============================================================================
// VM 월드 컨텍스트
// 
// VM 실행에 필요한 외부 의존성
// 새로운 상태 시스템 사용
// ============================================================================

struct FHktVMWorld
{
    // === 상태 시스템 ===
    FHktStateStore* StateStore = nullptr;       // 중앙 상태 저장소
    FHktStateCache* StateCache = nullptr;       // VM별 로컬 캐시 (실행 중 사용)
    FHktListStorage* Lists = nullptr;
    
    UWorld* UnrealWorld = nullptr;
    float DeltaTime = 0.0f;
    
    // 콜리전 결과 (WaitCollision용)
    int32 LastCollisionEntityID = INDEX_NONE;
    int32 LastCollisionGeneration = 0;
    
    // 디버그
    bool bDebugLog = false;
    
    // === 캐시 헬퍼 ===
    
    // 엔티티 캐시 가져오기 (StateCache 우선, 없으면 StateStore에서 로드)
    FHktEntityCache* GetEntityCache(int32 EntityID, int32 Generation)
    {
        if (StateCache)
        {
            return StateCache->GetOrLoadEntity(EntityID, Generation);
        }
        return nullptr;
    }
    
    // Owner 엔티티 캐시
    FHktEntityCache* GetOwnerCache()
    {
        if (StateCache)
        {
            return &StateCache->OwnerCache;
        }
        return nullptr;
    }
    
    // 프로세스 캐시
    FHktProcessCache* GetProcessCache()
    {
        if (StateCache)
        {
            return &StateCache->ProcessCache;
        }
        return nullptr;
    }
};

// ============================================================================
// 명령어 구현
// 
// 규칙:
// - 반환 -1: PC를 1 증가 (다음 명령어로)
// - 반환 N: PC를 N으로 설정 (점프)
// - Yield 시: State를 Yielded로 설정하고 -1 반환
// ============================================================================

namespace HktOps
{
    // === Nop ===
    inline int32 Op_Nop(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        return -1;
    }
    
    // === End ===
    inline int32 Op_End(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        Batch.States[VMIdx] = EHktVMState::Finished;
        return -1;
    }
    
    // === Yield ===
    inline int32 Op_Yield(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        Batch.States[VMIdx] = EHktVMState::Yielded;
        return -1;
    }
    
    // === WaitTime ===
    // A: 상수 풀 인덱스 (float 시간)
    inline int32 Op_WaitTime(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        float Duration = Prog->GetConst(Inst.Offset).FloatVal;
        
        Batch.WaitTimers[VMIdx] = Duration;
        Batch.States[VMIdx] = EHktVMState::WaitingTime;
        Batch.YieldConditions[VMIdx] = EHktYieldCondition::Time;
        
        return -1;
    }
    
    // === WaitCollision ===
    // A: 결과 저장 레지스터 (충돌 엔티티)
    // Offset: 타임아웃 상수 인덱스
    inline int32 Op_WaitCollision(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        // 콜리전 체크 결과가 있으면 진행
        if (World.LastCollisionEntityID != INDEX_NONE)
        {
            Batch.GetRegister(VMIdx, Inst.A).SetEntity(
                World.LastCollisionEntityID, 
                World.LastCollisionGeneration);
            World.LastCollisionEntityID = INDEX_NONE;
            return -1;
        }
        
        // 없으면 대기
        Batch.States[VMIdx] = EHktVMState::WaitingCondition;
        Batch.YieldConditions[VMIdx] = EHktYieldCondition::Collision;
        return -1;
    }
    
    // === WaitArrival ===
    // Offset: Tolerance 상수 인덱스
    inline int32 Op_WaitArrival(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        int32 OwnerID = Batch.OwnerEntityIDs[VMIdx];
        int32 OwnerGen = Batch.OwnerGenerations[VMIdx];
        
        if (!World.StateStore->Entities.IsValid(OwnerID, OwnerGen))
        {
            Batch.States[VMIdx] = EHktVMState::Finished;
            return -1;
        }
        
        float Tolerance = 10.0f;
        if (Inst.Offset >= 0)
        {
            const FHktProgram* Prog = Batch.Programs[VMIdx];
            Tolerance = Prog->GetConst(Inst.Offset).FloatVal;
        }
        
        FHktEntityState* State = World.StateStore->Entities.Get(OwnerID);
        if (State && State->HasArrivedAtTarget(Tolerance))
        {
            State->ClearFlag(FHktEntityState::FLAG_MOVING);
            return -1;
        }
        
        Batch.States[VMIdx] = EHktVMState::WaitingCondition;
        Batch.YieldConditions[VMIdx] = EHktYieldCondition::Arrival;
        return -1;
    }
    
    // === Spawn ===
    // A: 결과 레지스터 (새 엔티티)
    // D: 태그 테이블 인덱스 (엔티티 타입)
    inline int32 Op_Spawn(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& EntityTag = Prog->GetTag(Inst.D);
        
        int32 NewID = World.StateStore->Entities.Allocate(EntityTag);
        if (NewID != INDEX_NONE)
        {
            FHktEntityState* State = World.StateStore->Entities.Get(NewID);
            int32 NewGen = State ? State->Generation : 0;
            Batch.GetRegister(VMIdx, Inst.A).SetEntity(NewID, NewGen);
            
            if (World.bDebugLog)
            {
                UE_LOG(LogTemp, Log, TEXT("[VM %d] Spawn: %s -> ID=%d"), VMIdx, *EntityTag.ToString(), NewID);
            }
        }
        
        return -1;
    }
    
    // === Destroy ===
    // A: 대상 레지스터
    inline int32 Op_Destroy(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        FHktRegister& Reg = Batch.GetRegister(VMIdx, Inst.A);
        World.StateStore->Entities.Free(Reg.GetEntityID());
        
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] Destroy: ID=%d"), VMIdx, Reg.GetEntityID());
        }
        
        return -1;
    }
    
    // === MoveForward ===
    // A: 속도 상수 인덱스
    inline int32 Op_MoveForward(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        int32 OwnerID = Batch.OwnerEntityIDs[VMIdx];
        int32 OwnerGen = Batch.OwnerGenerations[VMIdx];
        
        if (!World.StateStore->Entities.IsValid(OwnerID, OwnerGen))
        {
            return -1;
        }
        
        FHktEntityState* State = World.StateStore->Entities.Get(OwnerID);
        if (!State) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        float Speed = Prog->GetConst(Inst.Offset).FloatVal;
        
        // 현재 방향으로 속도 설정 (Yaw 기준)
        float Yaw = HktFixedToFloat(State->RotationYaw);
        float Rad = FMath::DegreesToRadians(Yaw);
        
        State->VelX = HktFloatToFixed(FMath::Cos(Rad) * Speed);
        State->VelY = HktFloatToFixed(FMath::Sin(Rad) * Speed);
        State->VelZ = 0;
        State->SetFlag(FHktEntityState::FLAG_MOVING);
        
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] MoveForward: Speed=%.1f"), VMIdx, Speed);
        }
        
        return -1;
    }
    
    // === MoveTo ===
    // Offset: 위치 상수 인덱스 (Vector)
    inline int32 Op_MoveTo(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        int32 OwnerID = Batch.OwnerEntityIDs[VMIdx];
        int32 OwnerGen = Batch.OwnerGenerations[VMIdx];
        
        if (!World.StateStore->Entities.IsValid(OwnerID, OwnerGen))
        {
            return -1;
        }
        
        FHktEntityState* State = World.StateStore->Entities.Get(OwnerID);
        if (!State) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
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
    
    // === Stop ===
    inline int32 Op_Stop(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        int32 OwnerID = Batch.OwnerEntityIDs[VMIdx];
        FHktEntityState* State = World.StateStore->Entities.Get(OwnerID);
        if (!State) return -1;
        
        State->VelX = 0;
        State->VelY = 0;
        State->VelZ = 0;
        State->ClearFlag(FHktEntityState::FLAG_MOVING);
        
        return -1;
    }
    
    // === PlayAnim ===
    // D: 태그 인덱스 (애니메이션)
    inline int32 Op_PlayAnim(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& AnimTag = Prog->GetTag(Inst.D);
        
        // TODO: 실제 애니메이션 시스템 연결
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] PlayAnim: %s"), VMIdx, *AnimTag.ToString());
        }
        
        return -1;
    }
    
    // === PlayEffect ===
    // D: 태그 인덱스 (이펙트)
    inline int32 Op_PlayEffect(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& EffectTag = Prog->GetTag(Inst.D);
        
        // TODO: 실제 이펙트 시스템 연결
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] PlayEffect: %s"), VMIdx, *EffectTag.ToString());
        }
        
        return -1;
    }
    
    // === Damage ===
    // A: 대상 레지스터
    // Offset: 데미지 상수 인덱스
    inline int32 Op_Damage(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        FHktRegister& TargetReg = Batch.GetRegister(VMIdx, Inst.A);
        int32 TargetID = TargetReg.GetEntityID();
        int32 TargetGen = TargetReg.GetGeneration();
        
        if (!World.StateStore->Entities.IsValid(TargetID, TargetGen))
        {
            return -1;
        }
        
        FHktEntityState* State = World.StateStore->Entities.Get(TargetID);
        if (!State) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        float Damage = Prog->GetConst(Inst.Offset).FloatVal;
        
        float Health = State->Attributes.GetFloat(HktAttrKeys::Health, 0.0f);
        Health -= Damage;
        
        if (Health <= 0.0f)
        {
            Health = 0.0f;
            State->ClearFlag(FHktEntityState::FLAG_ALIVE);
        }
        State->Attributes.SetFloat(HktAttrKeys::Health, Health);
        
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] Damage: %.1f to ID=%d"), VMIdx, Damage, TargetID);
        }
        
        return -1;
    }
    
    // === ApplyStatus ===
    // A: 대상 레지스터
    // B: 상태 플래그 (FLAG_BURNING 등)
    // Offset: 지속시간 상수 인덱스 (현재 미구현, 플래그만 설정)
    inline int32 Op_ApplyStatus(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        FHktRegister& TargetReg = Batch.GetRegister(VMIdx, Inst.A);
        int32 TargetID = TargetReg.GetEntityID();
        int32 TargetGen = TargetReg.GetGeneration();
        
        if (!World.StateStore->Entities.IsValid(TargetID, TargetGen))
        {
            return -1;
        }
        
        FHktEntityState* State = World.StateStore->Entities.Get(TargetID);
        if (!State) return -1;
        
        uint32 StatusFlag = static_cast<uint32>(1 << Inst.B);
        State->SetFlag(StatusFlag);
        
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] ApplyStatus: Flag=%d to ID=%d"), VMIdx, Inst.B, TargetID);
        }
        
        return -1;
    }
    
    // === QueryRadius ===
    // A: 결과 리스트 레지스터 (리스트 핸들)
    // B: 중심 엔티티 레지스터 (또는 Owner면 0xFF)
    // Offset: 반경 상수 인덱스
    inline int32 Op_QueryRadius(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore || !World.Lists) return -1;
        
        // 중심 위치 결정
        FVector Origin;
        if (Inst.B == 0xFF)
        {
            // Owner 위치
            int32 OwnerID = Batch.OwnerEntityIDs[VMIdx];
            FHktEntityState* State = World.StateStore->Entities.Get(OwnerID);
            if (State)
            {
                Origin = FVector(HktFixedToFloat(State->PosX), HktFixedToFloat(State->PosY), HktFixedToFloat(State->PosZ));
            }
        }
        else
        {
            FHktRegister& OriginReg = Batch.GetRegister(VMIdx, Inst.B);
            int32 OriginID = OriginReg.GetEntityID();
            FHktEntityState* State = World.StateStore->Entities.Get(OriginID);
            if (State)
            {
                Origin = FVector(HktFixedToFloat(State->PosX), HktFixedToFloat(State->PosY), HktFixedToFloat(State->PosZ));
            }
        }
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        float Radius = Prog->GetConst(Inst.Offset).FloatVal;
        
        // 리스트 할당
        int32 ListHandle = World.Lists->AllocateList();
        if (ListHandle == INDEX_NONE)
        {
            return -1;
        }
        
        // 쿼리 실행
        TArray<int32> Results;
        World.StateStore->Entities.QueryRadius(Origin, Radius, Results);
        
        // 결과를 리스트에 복사
        FHktListStorage::FList* List = World.Lists->GetList(ListHandle);
        for (int32 ID : Results)
        {
            List->Add(ID);
        }
        
        // 핸들을 레지스터에 저장
        Batch.GetRegister(VMIdx, Inst.A).SetInt(ListHandle);
        
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] QueryRadius: R=%.1f, Found=%d"), VMIdx, Radius, Results.Num());
        }
        
        return -1;
    }
    
    // === Jump ===
    // Offset: 점프 대상 PC
    inline int32 Op_Jump(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        return Inst.Offset;
    }
    
    // === JumpIfZero ===
    // A: 조건 레지스터
    // Offset: 점프 대상 PC
    inline int32 Op_JumpIfZero(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (Batch.GetRegister(VMIdx, Inst.A).I64 == 0)
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // === JumpIfNotZero ===
    inline int32 Op_JumpIfNotZero(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (Batch.GetRegister(VMIdx, Inst.A).I64 != 0)
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // === JumpIfLess ===
    // A: 좌측 레지스터, B: 우측 레지스터
    inline int32 Op_JumpIfLess(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (Batch.GetRegister(VMIdx, Inst.A).F64 < Batch.GetRegister(VMIdx, Inst.B).F64)
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // === JumpIfGreater ===
    inline int32 Op_JumpIfGreater(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (Batch.GetRegister(VMIdx, Inst.A).F64 > Batch.GetRegister(VMIdx, Inst.B).F64)
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // === ForEach ===
    // A: 리스트 레지스터 (핸들)
    // B: 아이템 출력 레지스터
    // Offset: 루프 종료 PC
    inline int32 Op_ForEach(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.Lists) return Inst.Offset;  // 리스트 없으면 루프 스킵
        
        int32 ListHandle = Batch.GetRegister(VMIdx, Inst.A).AsInt();
        FHktListStorage::FList* List = World.Lists->GetList(ListHandle);
        
        if (!List || List->Count == 0)
        {
            return Inst.Offset;  // 빈 리스트면 루프 스킵
        }
        
        // 루프 상태 초기화
        Batch.LoopListRegisters[VMIdx] = Inst.A;
        Batch.LoopIterators[VMIdx] = 0;
        Batch.LoopReturnPCs[VMIdx] = Batch.PCs[VMIdx];  // ForEach 명령어 위치
        
        // 첫 번째 아이템
        int32 ItemID = List->Items[0];
        int32 ItemGen = 0;
        if (World.StateStore)
        {
            FHktEntityState* ItemState = World.StateStore->Entities.Get(ItemID);
            if (ItemState) ItemGen = ItemState->Generation;
        }
        Batch.GetRegister(VMIdx, Inst.B).SetEntity(ItemID, ItemGen);
        Batch.LoopIterators[VMIdx] = 1;
        
        return -1;  // 다음 명령어 (루프 바디)
    }
    
    // === EndForEach ===
    // Offset: 루프 시작 PC (ForEach 다음 명령어)
    inline int32 Op_EndForEach(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.Lists) return -1;
        
        int32 ListReg = Batch.LoopListRegisters[VMIdx];
        if (ListReg == INDEX_NONE) return -1;
        
        int32 ListHandle = Batch.GetRegister(VMIdx, ListReg).AsInt();
        FHktListStorage::FList* List = World.Lists->GetList(ListHandle);
        
        if (!List) return -1;
        
        int32 Iterator = Batch.LoopIterators[VMIdx];
        
        if (Iterator < List->Count)
        {
            // 다음 아이템
            // ForEach 명령어의 B 피연산자를 알아야 하는데, 여기서는 저장해두지 않음
            // 간단히 ForEach로 돌아가서 다시 처리
            return Batch.LoopReturnPCs[VMIdx];
        }
        else
        {
            // 루프 종료
            Batch.LoopListRegisters[VMIdx] = INDEX_NONE;
            World.Lists->FreeList(ListHandle);
            return -1;
        }
    }
    
    // === LoadConst ===
    // A: 대상 레지스터
    // Offset: 상수 인덱스
    inline int32 Op_LoadConst(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FHktConstant& Const = Prog->GetConst(Inst.Offset);
        
        switch (Const.Type)
        {
        case EHktConstType::Int32:
            Batch.GetRegister(VMIdx, Inst.A).SetInt(Const.IntVal);
            break;
        case EHktConstType::Float:
            Batch.GetRegister(VMIdx, Inst.A).SetFloat(Const.FloatVal);
            break;
        default:
            // Vector는 단일 레지스터에 맞지 않음 - 무시
            break;
        }
        
        return -1;
    }
    
    // === LoadAttr ===
    // A: 대상 레지스터
    // B: 엔티티 레지스터 (0xFF면 Owner)
    // C: 속성 타입 (EHktAttrType)
    inline int32 Op_LoadAttr(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        int32 EntityID;
        if (Inst.B == 0xFF)
        {
            EntityID = Batch.OwnerEntityIDs[VMIdx];
        }
        else
        {
            EntityID = Batch.GetRegister(VMIdx, Inst.B).GetEntityID();
        }
        
        FHktEntityState* State = World.StateStore->Entities.Get(EntityID);
        if (!State) return -1;
        
        EHktAttrType AttrType = static_cast<EHktAttrType>(Inst.C);
        
        switch (AttrType)
        {
        case EHktAttrType::Health:
            Batch.GetRegister(VMIdx, Inst.A).SetFloat(State->Attributes.GetFloat(HktAttrKeys::Health, 0.0f));
            break;
        case EHktAttrType::MaxHealth:
            Batch.GetRegister(VMIdx, Inst.A).SetFloat(State->Attributes.GetFloat(HktAttrKeys::MaxHealth, 0.0f));
            break;
        case EHktAttrType::Mana:
            Batch.GetRegister(VMIdx, Inst.A).SetFloat(State->Attributes.GetFloat(HktAttrKeys::Mana, 0.0f));
            break;
        case EHktAttrType::Speed:
            Batch.GetRegister(VMIdx, Inst.A).SetFloat(State->Attributes.GetFloat(HktAttrKeys::Speed, 0.0f));
            break;
        default:
            break;
        }
        
        return -1;
    }
    
    // === StoreAttr ===
    // A: 소스 레지스터
    // B: 엔티티 레지스터 (0xFF면 Owner)
    // C: 속성 타입 (EHktAttrType)
    inline int32 Op_StoreAttr(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        if (!World.StateStore) return -1;
        
        int32 EntityID;
        if (Inst.B == 0xFF)
        {
            EntityID = Batch.OwnerEntityIDs[VMIdx];
        }
        else
        {
            EntityID = Batch.GetRegister(VMIdx, Inst.B).GetEntityID();
        }
        
        FHktEntityState* State = World.StateStore->Entities.Get(EntityID);
        if (!State) return -1;
        
        EHktAttrType AttrType = static_cast<EHktAttrType>(Inst.C);
        float Value = Batch.GetRegister(VMIdx, Inst.A).AsFloat();
        
        switch (AttrType)
        {
        case EHktAttrType::Health:
            State->Attributes.SetFloat(HktAttrKeys::Health, Value);
            break;
        case EHktAttrType::MaxHealth:
            State->Attributes.SetFloat(HktAttrKeys::MaxHealth, Value);
            break;
        case EHktAttrType::Mana:
            State->Attributes.SetFloat(HktAttrKeys::Mana, Value);
            break;
        case EHktAttrType::Speed:
            State->Attributes.SetFloat(HktAttrKeys::Speed, Value);
            break;
        default:
            break;
        }
        
        return -1;
    }
    
    // === Copy ===
    // A: 소스, B: 대상
    inline int32 Op_Copy(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        Batch.GetRegister(VMIdx, Inst.B).Raw = Batch.GetRegister(VMIdx, Inst.A).Raw;
        return -1;
    }
    
    // === 산술 연산 ===
    // A, B: 소스, C: 대상
    inline int32 Op_Add(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        Batch.GetRegister(VMIdx, Inst.C).F64 = 
            Batch.GetRegister(VMIdx, Inst.A).F64 + Batch.GetRegister(VMIdx, Inst.B).F64;
        return -1;
    }
    
    inline int32 Op_Sub(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        Batch.GetRegister(VMIdx, Inst.C).F64 = 
            Batch.GetRegister(VMIdx, Inst.A).F64 - Batch.GetRegister(VMIdx, Inst.B).F64;
        return -1;
    }
    
    inline int32 Op_Mul(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        Batch.GetRegister(VMIdx, Inst.C).F64 = 
            Batch.GetRegister(VMIdx, Inst.A).F64 * Batch.GetRegister(VMIdx, Inst.B).F64;
        return -1;
    }
    
    inline int32 Op_Div(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        double Divisor = Batch.GetRegister(VMIdx, Inst.B).F64;
        if (FMath::Abs(Divisor) > KINDA_SMALL_NUMBER)
        {
            Batch.GetRegister(VMIdx, Inst.C).F64 = 
                Batch.GetRegister(VMIdx, Inst.A).F64 / Divisor;
        }
        return -1;
    }
    
    // === WaitCondition (커스텀 조건) ===
    inline int32 Op_WaitCondition(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        Batch.States[VMIdx] = EHktVMState::WaitingCondition;
        Batch.YieldConditions[VMIdx] = EHktYieldCondition::Custom;
        return -1;
    }
    
    // === QueryRay (미구현 플레이스홀더) ===
    inline int32 Op_QueryRay(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        // TODO: 레이캐스트 구현
        return -1;
    }
    
    // ========================================================================
    // 태그 기반 분기 (NEW)
    // ========================================================================
    
    // === JumpIfHasTag ===
    // A: 엔티티 레지스터 (0xFF면 Owner)
    // D: 태그 테이블 인덱스
    // Offset: 점프 대상 PC
    inline int32 Op_JumpIfHasTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktEntityCache* Cache = nullptr;
        if (Inst.A == 0xFF)
        {
            Cache = World.GetOwnerCache();
        }
        else
        {
            FHktRegister& Reg = Batch.GetRegister(VMIdx, Inst.A);
            Cache = World.GetEntityCache(Reg.GetEntityID(), Reg.GetGeneration());
        }
        
        if (!Cache || !Cache->IsValid()) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        if (Cache->StatusTags.HasTag(Tag) || Cache->BuffTags.HasTag(Tag))
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // === JumpIfNotHasTag ===
    inline int32 Op_JumpIfNotHasTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktEntityCache* Cache = nullptr;
        if (Inst.A == 0xFF)
        {
            Cache = World.GetOwnerCache();
        }
        else
        {
            FHktRegister& Reg = Batch.GetRegister(VMIdx, Inst.A);
            Cache = World.GetEntityCache(Reg.GetEntityID(), Reg.GetGeneration());
        }
        
        if (!Cache || !Cache->IsValid()) return Inst.Offset;  // 없으면 점프
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        if (!Cache->StatusTags.HasTag(Tag) && !Cache->BuffTags.HasTag(Tag))
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // === JumpIfProcessDoing ===
    // D: 태그 테이블 인덱스
    // Offset: 점프 대상 PC
    inline int32 Op_JumpIfProcessDoing(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktProcessCache* ProcCache = World.GetProcessCache();
        if (!ProcCache) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        if (ProcCache->DoingTags.HasTag(Tag))
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // === JumpIfProcessDone ===
    inline int32 Op_JumpIfProcessDone(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktProcessCache* ProcCache = World.GetProcessCache();
        if (!ProcCache) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        if (ProcCache->CompletedTags.HasTag(Tag))
        {
            return Inst.Offset;
        }
        return -1;
    }
    
    // ========================================================================
    // 태그 기반 속성 (NEW)
    // ========================================================================
    
    // === LoadAttrByTag ===
    // A: 대상 레지스터
    // B: 엔티티 레지스터 (0xFF면 Owner)
    // D: 속성 태그 테이블 인덱스
    inline int32 Op_LoadAttrByTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktEntityCache* Cache = nullptr;
        if (Inst.B == 0xFF)
        {
            Cache = World.GetOwnerCache();
        }
        else
        {
            FHktRegister& Reg = Batch.GetRegister(VMIdx, Inst.B);
            Cache = World.GetEntityCache(Reg.GetEntityID(), Reg.GetGeneration());
        }
        
        if (!Cache || !Cache->IsValid()) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& AttrTag = Prog->GetTag(Inst.D);
        
        int32 Value = Cache->Attributes.Get(AttrTag, 0);
        Batch.GetRegister(VMIdx, Inst.A).SetInt(Value);
        
        return -1;
    }
    
    // === StoreAttrByTag ===
    // A: 소스 레지스터
    // B: 엔티티 레지스터 (0xFF면 Owner)
    // D: 속성 태그 테이블 인덱스
    inline int32 Op_StoreAttrByTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktEntityCache* Cache = nullptr;
        if (Inst.B == 0xFF)
        {
            Cache = World.GetOwnerCache();
        }
        else
        {
            FHktRegister& Reg = Batch.GetRegister(VMIdx, Inst.B);
            Cache = World.GetEntityCache(Reg.GetEntityID(), Reg.GetGeneration());
        }
        
        if (!Cache || !Cache->IsValid()) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& AttrTag = Prog->GetTag(Inst.D);
        
        int32 Value = Batch.GetRegister(VMIdx, Inst.A).AsInt();
        Cache->Attributes.Set(AttrTag, Value);
        Cache->MarkModified();
        
        return -1;
    }
    
    // === AddAttrByTag ===
    // A: 델타 레지스터
    // B: 엔티티 레지스터 (0xFF면 Owner)
    // D: 속성 태그 테이블 인덱스
    inline int32 Op_AddAttrByTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktEntityCache* Cache = nullptr;
        if (Inst.B == 0xFF)
        {
            Cache = World.GetOwnerCache();
        }
        else
        {
            FHktRegister& Reg = Batch.GetRegister(VMIdx, Inst.B);
            Cache = World.GetEntityCache(Reg.GetEntityID(), Reg.GetGeneration());
        }
        
        if (!Cache || !Cache->IsValid()) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& AttrTag = Prog->GetTag(Inst.D);
        
        int32 Delta = Batch.GetRegister(VMIdx, Inst.A).AsInt();
        Cache->Attributes.Add(AttrTag, Delta);
        Cache->MarkModified();
        
        return -1;
    }
    
    // ========================================================================
    // 태그 조작 (NEW)
    // ========================================================================
    
    // === AddStatusTag ===
    // A: 엔티티 레지스터 (0xFF면 Owner)
    // D: 태그 테이블 인덱스
    inline int32 Op_AddStatusTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktEntityCache* Cache = nullptr;
        if (Inst.A == 0xFF)
        {
            Cache = World.GetOwnerCache();
        }
        else
        {
            FHktRegister& Reg = Batch.GetRegister(VMIdx, Inst.A);
            Cache = World.GetEntityCache(Reg.GetEntityID(), Reg.GetGeneration());
        }
        
        if (!Cache || !Cache->IsValid()) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        Cache->StatusTags.AddTag(Tag);
        Cache->MarkModified();
        
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] AddStatusTag: %s to Entity %d"), 
                VMIdx, *Tag.ToString(), Cache->EntityID);
        }
        
        return -1;
    }
    
    // === RemoveStatusTag ===
    inline int32 Op_RemoveStatusTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktEntityCache* Cache = nullptr;
        if (Inst.A == 0xFF)
        {
            Cache = World.GetOwnerCache();
        }
        else
        {
            FHktRegister& Reg = Batch.GetRegister(VMIdx, Inst.A);
            Cache = World.GetEntityCache(Reg.GetEntityID(), Reg.GetGeneration());
        }
        
        if (!Cache || !Cache->IsValid()) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        Cache->StatusTags.RemoveTag(Tag);
        Cache->MarkModified();
        
        return -1;
    }
    
    // === MarkProcessDoing ===
    // D: 태그 테이블 인덱스
    inline int32 Op_MarkProcessDoing(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktProcessCache* ProcCache = World.GetProcessCache();
        if (!ProcCache) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        ProcCache->DoingTags.AddTag(Tag);
        ProcCache->CompletedTags.RemoveTag(Tag);  // Doing으로 이동
        ProcCache->MarkModified();
        
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] MarkProcessDoing: %s"), VMIdx, *Tag.ToString());
        }
        
        return -1;
    }
    
    // === MarkProcessDone ===
    // D: 태그 테이블 인덱스
    inline int32 Op_MarkProcessDone(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktProcessCache* ProcCache = World.GetProcessCache();
        if (!ProcCache) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        ProcCache->DoingTags.RemoveTag(Tag);
        ProcCache->CompletedTags.AddTag(Tag);  // Done으로 이동
        ProcCache->MarkModified();
        
        if (World.bDebugLog)
        {
            UE_LOG(LogTemp, Log, TEXT("[VM %d] MarkProcessDone: %s"), VMIdx, *Tag.ToString());
        }
        
        return -1;
    }
    
    // === AddProcessTag / RemoveProcessTag ===
    inline int32 Op_AddProcessTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktProcessCache* ProcCache = World.GetProcessCache();
        if (!ProcCache) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        ProcCache->ContextTags.AddTag(Tag);
        ProcCache->MarkModified();
        
        return -1;
    }
    
    inline int32 Op_RemoveProcessTag(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        FHktProcessCache* ProcCache = World.GetProcessCache();
        if (!ProcCache) return -1;
        
        const FHktProgram* Prog = Batch.Programs[VMIdx];
        const FGameplayTag& Tag = Prog->GetTag(Inst.D);
        
        ProcCache->ContextTags.RemoveTag(Tag);
        ProcCache->MarkModified();
        
        return -1;
    }
    
} // namespace HktOps

// ============================================================================
// 함수 포인터 테이블
// ============================================================================

class FHktVMDispatch
{
public:
    // 전역 Op 테이블
    static FHktOpHandler OpTable[static_cast<int32>(EHktOp::MAX)];
    
    // 초기화 (static initializer에서 호출)
    static void Initialize()
    {
        // 기본값: Nop
        for (int32 i = 0; i < static_cast<int32>(EHktOp::MAX); ++i)
        {
            OpTable[i] = &HktOps::Op_Nop;
        }
        
        // 각 Op 등록
        OpTable[static_cast<int32>(EHktOp::Nop)]              = &HktOps::Op_Nop;
        OpTable[static_cast<int32>(EHktOp::End)]              = &HktOps::Op_End;
        OpTable[static_cast<int32>(EHktOp::Yield)]            = &HktOps::Op_Yield;
        OpTable[static_cast<int32>(EHktOp::WaitTime)]         = &HktOps::Op_WaitTime;
        OpTable[static_cast<int32>(EHktOp::WaitCollision)]    = &HktOps::Op_WaitCollision;
        OpTable[static_cast<int32>(EHktOp::WaitArrival)]      = &HktOps::Op_WaitArrival;
        OpTable[static_cast<int32>(EHktOp::WaitCondition)]    = &HktOps::Op_WaitCondition;
        OpTable[static_cast<int32>(EHktOp::Spawn)]            = &HktOps::Op_Spawn;
        OpTable[static_cast<int32>(EHktOp::Destroy)]          = &HktOps::Op_Destroy;
        OpTable[static_cast<int32>(EHktOp::MoveForward)]      = &HktOps::Op_MoveForward;
        OpTable[static_cast<int32>(EHktOp::MoveTo)]           = &HktOps::Op_MoveTo;
        OpTable[static_cast<int32>(EHktOp::Stop)]             = &HktOps::Op_Stop;
        OpTable[static_cast<int32>(EHktOp::PlayAnim)]         = &HktOps::Op_PlayAnim;
        OpTable[static_cast<int32>(EHktOp::PlayEffect)]       = &HktOps::Op_PlayEffect;
        OpTable[static_cast<int32>(EHktOp::Damage)]           = &HktOps::Op_Damage;
        OpTable[static_cast<int32>(EHktOp::ApplyStatus)]      = &HktOps::Op_ApplyStatus;
        OpTable[static_cast<int32>(EHktOp::QueryRadius)]      = &HktOps::Op_QueryRadius;
        OpTable[static_cast<int32>(EHktOp::QueryRay)]         = &HktOps::Op_QueryRay;
        OpTable[static_cast<int32>(EHktOp::Jump)]             = &HktOps::Op_Jump;
        OpTable[static_cast<int32>(EHktOp::JumpIfZero)]       = &HktOps::Op_JumpIfZero;
        OpTable[static_cast<int32>(EHktOp::JumpIfNotZero)]    = &HktOps::Op_JumpIfNotZero;
        OpTable[static_cast<int32>(EHktOp::JumpIfLess)]       = &HktOps::Op_JumpIfLess;
        OpTable[static_cast<int32>(EHktOp::JumpIfGreater)]    = &HktOps::Op_JumpIfGreater;
        OpTable[static_cast<int32>(EHktOp::ForEach)]          = &HktOps::Op_ForEach;
        OpTable[static_cast<int32>(EHktOp::EndForEach)]       = &HktOps::Op_EndForEach;
        OpTable[static_cast<int32>(EHktOp::LoadConst)]        = &HktOps::Op_LoadConst;
        OpTable[static_cast<int32>(EHktOp::LoadAttr)]         = &HktOps::Op_LoadAttr;
        OpTable[static_cast<int32>(EHktOp::StoreAttr)]        = &HktOps::Op_StoreAttr;
        OpTable[static_cast<int32>(EHktOp::Copy)]             = &HktOps::Op_Copy;
        OpTable[static_cast<int32>(EHktOp::Add)]              = &HktOps::Op_Add;
        OpTable[static_cast<int32>(EHktOp::Sub)]              = &HktOps::Op_Sub;
        OpTable[static_cast<int32>(EHktOp::Mul)]              = &HktOps::Op_Mul;
        OpTable[static_cast<int32>(EHktOp::Div)]              = &HktOps::Op_Div;
        
        // === 태그 기반 분기 (NEW) ===
        OpTable[static_cast<int32>(EHktOp::JumpIfHasTag)]        = &HktOps::Op_JumpIfHasTag;
        OpTable[static_cast<int32>(EHktOp::JumpIfNotHasTag)]     = &HktOps::Op_JumpIfNotHasTag;
        OpTable[static_cast<int32>(EHktOp::JumpIfProcessDoing)]  = &HktOps::Op_JumpIfProcessDoing;
        OpTable[static_cast<int32>(EHktOp::JumpIfProcessDone)]   = &HktOps::Op_JumpIfProcessDone;
        
        // === 태그 기반 속성 (NEW) ===
        OpTable[static_cast<int32>(EHktOp::LoadAttrByTag)]       = &HktOps::Op_LoadAttrByTag;
        OpTable[static_cast<int32>(EHktOp::StoreAttrByTag)]      = &HktOps::Op_StoreAttrByTag;
        OpTable[static_cast<int32>(EHktOp::AddAttrByTag)]        = &HktOps::Op_AddAttrByTag;
        
        // === 태그 조작 (NEW) ===
        OpTable[static_cast<int32>(EHktOp::AddStatusTag)]        = &HktOps::Op_AddStatusTag;
        OpTable[static_cast<int32>(EHktOp::RemoveStatusTag)]     = &HktOps::Op_RemoveStatusTag;
        OpTable[static_cast<int32>(EHktOp::AddProcessTag)]       = &HktOps::Op_AddProcessTag;
        OpTable[static_cast<int32>(EHktOp::RemoveProcessTag)]    = &HktOps::Op_RemoveProcessTag;
        OpTable[static_cast<int32>(EHktOp::MarkProcessDoing)]    = &HktOps::Op_MarkProcessDoing;
        OpTable[static_cast<int32>(EHktOp::MarkProcessDone)]     = &HktOps::Op_MarkProcessDone;
    }
    
    // 단일 명령어 실행
    FORCEINLINE static int32 Execute(FHktVMBatch& Batch, int32 VMIdx, const FHktInstruction& Inst, FHktVMWorld& World)
    {
        return OpTable[Inst.Op](Batch, VMIdx, Inst, World);
    }
};

// 테이블 정의 (cpp에서)
// FHktOpHandler FHktVMDispatch::OpTable[static_cast<int32>(EHktOp::MAX)] = {};
