#pragma once

#include "CoreMinimal.h"
#include "HktVMTypes.h"
#include "HktVMProgram.h"

// ============================================================================
// HktVM Builder - Flow 정의를 위한 Fluent API
// 
// Flow 정의는 자연어처럼 읽혀야 한다:
// 
//   Builder.PlayAnimation(AnimTag)       // "애니메이션을 재생하고"
//          .WaitSeconds(0.5f)            // "0.5초 기다린 뒤"
//          .SpawnEntity(EntityTag, REG)  // "엔티티를 생성하여"
//          .MoveForward(1500.0f)         // "앞으로 이동시킨다"
//          .WaitUntilCollision(REG_HIT)  // "충돌할 때까지 기다리고"
//          .Damage(REG_HIT, 100.0f)      // "피해를 입힌다"
//          .End();
//
// ============================================================================

class FHktVMBuilder
{
public:
    // 빌드 중인 프로그램
    TSharedPtr<FHktProgram> Program;
    
    // 루프 패치 스택
    TArray<int32> LoopPatchStack;   // ForEach의 점프 오프셋 패치 위치
    TArray<int32> LoopStartStack;   // ForEach 바디 시작 PC
    TArray<uint8> LoopItemRegStack; // ForEach 아이템 레지스터
    
    FHktVMBuilder()
    {
        Program = MakeShared<FHktProgram>();
    }
    
    // 현재 명령어 인덱스
    int32 CurrentPC() const
    {
        return Program->Instructions.Num();
    }
    
    // ========================================================================
    // 명령어 추가 헬퍼
    // ========================================================================
    
    FHktInstruction& EmitInstruction(EHktOp Op)
    {
        FHktInstruction Inst;
        FMemory::Memzero(Inst);
        Inst.SetOp(Op);
        return Program->Instructions.Add_GetRef(Inst);
    }
    
    // ========================================================================
    // 시간 제어 (자연어의 시간 흐름)
    // ========================================================================
    
    // "다음 틱까지 기다린다"
    FHktVMBuilder& Yield()
    {
        EmitInstruction(EHktOp::Yield);
        return *this;
    }
    
    // "N초 동안 기다린다"
    FHktVMBuilder& WaitSeconds(float Duration)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::WaitTime);
        Inst.Offset = Program->AddConstFloat(Duration);
        return *this;
    }
    
    // "충돌할 때까지 기다린다"
    FHktVMBuilder& WaitUntilCollision(uint8 OutHitReg, float Timeout = 10.0f)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::WaitCollision);
        Inst.A = OutHitReg;
        Inst.Offset = Program->AddConstFloat(Timeout);
        return *this;
    }
    
    // "도착할 때까지 기다린다"
    FHktVMBuilder& WaitUntilArrival(float Tolerance = 10.0f)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::WaitArrival);
        Inst.Offset = Program->AddConstFloat(Tolerance);
        return *this;
    }
    
    // ========================================================================
    // 엔티티 동작
    // ========================================================================
    
    // "엔티티를 생성한다"
    FHktVMBuilder& SpawnEntity(const FGameplayTag& EntityTag, uint8 OutReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Spawn);
        Inst.A = OutReg;
        Inst.D = Program->AddTag(EntityTag);
        return *this;
    }
    
    // "엔티티를 제거한다"
    FHktVMBuilder& DestroyEntity(uint8 EntityReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Destroy);
        Inst.A = EntityReg;
        return *this;
    }
    
    // ========================================================================
    // 이동
    // ========================================================================
    
    // "앞으로 이동한다"
    FHktVMBuilder& MoveForward(float Speed)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::MoveForward);
        Inst.Offset = Program->AddConstFloat(Speed);
        return *this;
    }
    
    // "위치로 이동한다"
    FHktVMBuilder& MoveTo(const FVector& Location)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::MoveTo);
        Inst.Offset = Program->AddConstVector(Location);
        return *this;
    }
    
    // "정지한다"
    FHktVMBuilder& Stop()
    {
        EmitInstruction(EHktOp::Stop);
        return *this;
    }
    
    // ========================================================================
    // 애니메이션/이펙트
    // ========================================================================
    
    // "애니메이션을 재생한다"
    FHktVMBuilder& PlayAnimation(const FGameplayTag& AnimTag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::PlayAnim);
        Inst.D = Program->AddTag(AnimTag);
        return *this;
    }
    
    // "이펙트를 재생한다"
    FHktVMBuilder& PlayEffect(const FGameplayTag& EffectTag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::PlayEffect);
        Inst.D = Program->AddTag(EffectTag);
        return *this;
    }
    
    // ========================================================================
    // 전투
    // ========================================================================
    
    // "피해를 입힌다"
    FHktVMBuilder& SetDamage(uint8 TargetReg, float Amount)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Damage);
        Inst.A = TargetReg;
        Inst.Offset = Program->AddConstFloat(Amount);
        return *this;
    }
    
    // "상태이상을 적용한다"
    FHktVMBuilder& ApplyStatus(uint8 TargetReg, uint8 StatusBit, float Duration = 0.0f)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::ApplyStatus);
        Inst.A = TargetReg;
        Inst.B = StatusBit;
        Inst.Offset = Program->AddConstFloat(Duration);
        return *this;
    }
    
    // 기존 API 호환: ApplyBurning
    FHktVMBuilder& ApplyBurning(uint8 TargetReg, float Damage, float Duration)
    {
        // FLAG_BURNING = 4 (1 << 4 = 16, but index is 4)
        return ApplyStatus(TargetReg, 4, Duration);
    }
    
    // ========================================================================
    // 공간 쿼리
    // ========================================================================
    
    // "반경 내 대상을 찾는다"
    FHktVMBuilder& QueryNearby(float Radius, uint8 OriginReg, uint8 OutListReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::QueryRadius);
        Inst.A = OutListReg;
        Inst.B = OriginReg;
        Inst.Offset = Program->AddConstFloat(Radius);
        return *this;
    }
    
    // Owner 중심 쿼리
    FHktVMBuilder& QueryNearbyOwner(float Radius, uint8 OutListReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::QueryRadius);
        Inst.A = OutListReg;
        Inst.B = 0xFF;  // Owner 플래그
        Inst.Offset = Program->AddConstFloat(Radius);
        return *this;
    }
    
    // ========================================================================
    // 분기
    // ========================================================================
    
    // "조건이 0이면 점프"
    FHktVMBuilder& JumpIfZero(uint8 CondReg, int32 TargetPC)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::JumpIfZero);
        Inst.A = CondReg;
        Inst.Offset = static_cast<int16>(TargetPC);
        return *this;
    }
    
    // "조건이 0이 아니면 점프"
    FHktVMBuilder& JumpIfNotZero(uint8 CondReg, int32 TargetPC)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::JumpIfNotZero);
        Inst.A = CondReg;
        Inst.Offset = static_cast<int16>(TargetPC);
        return *this;
    }
    
    // "무조건 점프"
    FHktVMBuilder& Jump(int32 TargetPC)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Jump);
        Inst.Offset = static_cast<int16>(TargetPC);
        return *this;
    }
    
    // ========================================================================
    // 루프
    // ========================================================================
    
    // "각 대상에 대해..."
    FHktVMBuilder& ForEachTarget(uint8 ListReg, uint8 OutItemReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::ForEach);
        Inst.A = ListReg;
        Inst.B = OutItemReg;
        // Inst.Offset은 EndForEach에서 패치
        
        // 스택에 저장
        LoopPatchStack.Add(CurrentPC() - 1);  // ForEach 명령어 위치
        LoopStartStack.Add(CurrentPC());       // 바디 시작 위치
        LoopItemRegStack.Add(OutItemReg);
        
        return *this;
    }
    
    // "...반복 끝"
    FHktVMBuilder& EndForEach()
    {
        if (LoopPatchStack.Num() == 0)
        {
            checkf(false, TEXT("EndForEach without matching ForEachTarget"));
            return *this;
        }
        
        int32 ForEachPC = LoopPatchStack.Pop();
        int32 BodyStartPC = LoopStartStack.Pop();
        LoopItemRegStack.Pop();
        
        // EndForEach 명령어
        FHktInstruction& EndInst = EmitInstruction(EHktOp::EndForEach);
        EndInst.Offset = static_cast<int16>(ForEachPC);  // ForEach로 돌아감
        
        // ForEach의 Offset을 루프 종료 위치로 패치
        Program->Instructions[ForEachPC].Offset = static_cast<int16>(CurrentPC());
        
        return *this;
    }
    
    // ========================================================================
    // 레지스터 조작
    // ========================================================================
    
    // "상수를 레지스터에 로드"
    FHktVMBuilder& LoadConstInt(uint8 DestReg, int32 Value)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::LoadConst);
        Inst.A = DestReg;
        Inst.Offset = Program->AddConstInt(Value);
        return *this;
    }
    
    FHktVMBuilder& LoadConstFloat(uint8 DestReg, float Value)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::LoadConst);
        Inst.A = DestReg;
        Inst.Offset = Program->AddConstFloat(Value);
        return *this;
    }
    
    // "속성을 레지스터에 로드"
    FHktVMBuilder& LoadAttribute(uint8 DestReg, uint8 EntityReg, EHktAttrType AttrType)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::LoadAttr);
        Inst.A = DestReg;
        Inst.B = EntityReg;
        Inst.C = static_cast<uint8>(AttrType);
        return *this;
    }
    
    // Owner 속성 로드
    FHktVMBuilder& LoadOwnerAttribute(uint8 DestReg, EHktAttrType AttrType)
    {
        return LoadAttribute(DestReg, 0xFF, AttrType);
    }
    
    // "레지스터를 속성에 저장"
    FHktVMBuilder& StoreAttribute(uint8 SrcReg, uint8 EntityReg, EHktAttrType AttrType)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::StoreAttr);
        Inst.A = SrcReg;
        Inst.B = EntityReg;
        Inst.C = static_cast<uint8>(AttrType);
        return *this;
    }
    
    // "레지스터 복사"
    FHktVMBuilder& Copy(uint8 SrcReg, uint8 DestReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Copy);
        Inst.A = SrcReg;
        Inst.B = DestReg;
        return *this;
    }
    
    // ========================================================================
    // 산술 연산
    // ========================================================================
    
    FHktVMBuilder& Add(uint8 RegA, uint8 RegB, uint8 DestReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Add);
        Inst.A = RegA;
        Inst.B = RegB;
        Inst.C = DestReg;
        return *this;
    }
    
    FHktVMBuilder& Sub(uint8 RegA, uint8 RegB, uint8 DestReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Sub);
        Inst.A = RegA;
        Inst.B = RegB;
        Inst.C = DestReg;
        return *this;
    }
    
    FHktVMBuilder& Mul(uint8 RegA, uint8 RegB, uint8 DestReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Mul);
        Inst.A = RegA;
        Inst.B = RegB;
        Inst.C = DestReg;
        return *this;
    }
    
    FHktVMBuilder& Div(uint8 RegA, uint8 RegB, uint8 DestReg)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::Div);
        Inst.A = RegA;
        Inst.B = RegB;
        Inst.C = DestReg;
        return *this;
    }
    
    // ========================================================================
    // 태그 기반 분기 (NEW)
    // ========================================================================
    
    // "엔티티가 태그를 가지면 점프"
    FHktVMBuilder& JumpIfHasTag(uint8 EntityReg, const FGameplayTag& Tag, int32 TargetPC)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::JumpIfHasTag);
        Inst.A = EntityReg;
        Inst.D = Program->AddTag(Tag);
        Inst.Offset = static_cast<int16>(TargetPC);
        return *this;
    }
    
    // "엔티티가 태그를 안가지면 점프"
    FHktVMBuilder& JumpIfNotHasTag(uint8 EntityReg, const FGameplayTag& Tag, int32 TargetPC)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::JumpIfNotHasTag);
        Inst.A = EntityReg;
        Inst.D = Program->AddTag(Tag);
        Inst.Offset = static_cast<int16>(TargetPC);
        return *this;
    }
    
    // "Owner가 태그를 가지면 점프"
    FHktVMBuilder& JumpIfOwnerHasTag(const FGameplayTag& Tag, int32 TargetPC)
    {
        return JumpIfHasTag(0xFF, Tag, TargetPC);
    }
    
    // "프로세스가 작업 중이면 점프"
    FHktVMBuilder& JumpIfProcessDoing(const FGameplayTag& Tag, int32 TargetPC)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::JumpIfProcessDoing);
        Inst.D = Program->AddTag(Tag);
        Inst.Offset = static_cast<int16>(TargetPC);
        return *this;
    }
    
    // "프로세스가 작업 완료면 점프"
    FHktVMBuilder& JumpIfProcessDone(const FGameplayTag& Tag, int32 TargetPC)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::JumpIfProcessDone);
        Inst.D = Program->AddTag(Tag);
        Inst.Offset = static_cast<int16>(TargetPC);
        return *this;
    }
    
    // ========================================================================
    // 태그 기반 속성 (NEW)
    // ========================================================================
    
    // "태그로 속성 로드"
    FHktVMBuilder& LoadAttrByTag(uint8 DestReg, uint8 EntityReg, const FGameplayTag& AttrTag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::LoadAttrByTag);
        Inst.A = DestReg;
        Inst.B = EntityReg;
        Inst.D = Program->AddTag(AttrTag);
        return *this;
    }
    
    FHktVMBuilder& LoadOwnerAttrByTag(uint8 DestReg, const FGameplayTag& AttrTag)
    {
        return LoadAttrByTag(DestReg, 0xFF, AttrTag);
    }
    
    // "태그로 속성 저장"
    FHktVMBuilder& StoreAttrByTag(uint8 SrcReg, uint8 EntityReg, const FGameplayTag& AttrTag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::StoreAttrByTag);
        Inst.A = SrcReg;
        Inst.B = EntityReg;
        Inst.D = Program->AddTag(AttrTag);
        return *this;
    }
    
    // "태그 속성에 값 더하기"
    FHktVMBuilder& AddAttrByTag(uint8 DeltaReg, uint8 EntityReg, const FGameplayTag& AttrTag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::AddAttrByTag);
        Inst.A = DeltaReg;
        Inst.B = EntityReg;
        Inst.D = Program->AddTag(AttrTag);
        return *this;
    }
    
    // ========================================================================
    // 태그 조작 (NEW)
    // ========================================================================
    
    // "엔티티에 상태 태그 추가"
    FHktVMBuilder& AddStatusTag(uint8 EntityReg, const FGameplayTag& Tag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::AddStatusTag);
        Inst.A = EntityReg;
        Inst.D = Program->AddTag(Tag);
        return *this;
    }
    
    // "Owner에 상태 태그 추가"
    FHktVMBuilder& AddOwnerStatusTag(const FGameplayTag& Tag)
    {
        return AddStatusTag(0xFF, Tag);
    }
    
    // "엔티티에서 상태 태그 제거"
    FHktVMBuilder& RemoveStatusTag(uint8 EntityReg, const FGameplayTag& Tag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::RemoveStatusTag);
        Inst.A = EntityReg;
        Inst.D = Program->AddTag(Tag);
        return *this;
    }
    
    // "프로세스 Doing 태그 설정"
    FHktVMBuilder& MarkDoing(const FGameplayTag& Tag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::MarkProcessDoing);
        Inst.D = Program->AddTag(Tag);
        return *this;
    }
    
    // "프로세스 Done 태그 설정"
    FHktVMBuilder& MarkDone(const FGameplayTag& Tag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::MarkProcessDone);
        Inst.D = Program->AddTag(Tag);
        return *this;
    }
    
    // "프로세스에 컨텍스트 태그 추가"
    FHktVMBuilder& AddProcessTag(const FGameplayTag& Tag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::AddProcessTag);
        Inst.D = Program->AddTag(Tag);
        return *this;
    }
    
    // "프로세스에서 컨텍스트 태그 제거"
    FHktVMBuilder& RemoveProcessTag(const FGameplayTag& Tag)
    {
        FHktInstruction& Inst = EmitInstruction(EHktOp::RemoveProcessTag);
        Inst.D = Program->AddTag(Tag);
        return *this;
    }
    
    // ========================================================================
    // 종료
    // ========================================================================
    
    // "끝"
    FHktVMBuilder& End()
    {
        EmitInstruction(EHktOp::End);
        return *this;
    }
    
    // ========================================================================
    // 빌드 완료
    // ========================================================================
    
    TSharedPtr<FHktProgram> Build()
    {
        checkf(LoopPatchStack.Num() == 0, TEXT("Unclosed ForEach loops"));
        return Program;
    }
    
    TSharedPtr<FHktProgram> Build(const FGameplayTag& FlowTag)
    {
        Program->FlowTag = FlowTag;
        return Build();
    }
};
