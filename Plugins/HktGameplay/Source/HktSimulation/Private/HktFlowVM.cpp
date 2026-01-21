// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktFlowVM.h"
#include "Core/HktSimulationStats.h"
#include "GameFramework/Actor.h"

FVector FFlowRegister::ResolveLocation(const FHktEntityManager* Mgr) const
{
	if (Type == EFlowRegType::Vector) return VectorVal;
	if (Type == EFlowRegType::Unit && Mgr) return Mgr->GetUnitLocation(Unit);
	return FVector::ZeroVector;
}

FHktFlowVM::FHktFlowVM(UWorld* InWorld, FHktEntityManager* InMgr, FUnitHandle InOwnerUnit)
	: World(InWorld), Mgr(InMgr)
{
	Regs.OwnerUnit = InOwnerUnit;
	if (Mgr)
	{
		Regs.OwnerPlayer = Mgr->GetUnitOwner(InOwnerUnit);
		Regs.Get(0).SetVector(Mgr->GetUnitLocation(InOwnerUnit));
	}
	
	FHktOpRegistry::EnsureInitialized();
}

void FHktFlowVM::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_VMExecution);

	if (!Mgr || Regs.ProgramCounter >= Bytecode.Num()) return;

	// 1. 블로킹 상태 처리
	if (Regs.bBlocked)
	{
		if (Regs.WaitTimer > 0.0f)
		{
			Regs.WaitTimer -= DeltaTime;
			if (Regs.WaitTimer <= 0.0f) 
			{
				Regs.bBlocked = false;
			}
			else 
			{
				return;
			}
		}
		else
		{
			// WaitUntilDestroyed 처리
			FFlowRegister& TargetReg = Regs.Get(Regs.BlockingRegIndex);
			
			if (TargetReg.Type == EFlowRegType::Unit)
			{
				if (!Mgr->IsUnitValid(TargetReg.Unit))
				{
					// [Dead] 마지막 위치 복원
					FVector LastPos = Mgr->Entities.Locations[TargetReg.Unit.Index];
					TargetReg.SetVector(LastPos);
					Regs.bBlocked = false;
				}
				else
				{
					// [Alive] 위치 동기화 (Visual -> Logic)
					Mgr->SyncLocationFromVisual(TargetReg.Unit);
					return;
				}
			}
			else
			{
				Regs.bBlocked = false;
			}
		}
	}

	// 2. 명령어 실행
	while (Regs.ProgramCounter < Bytecode.Num() && !Regs.bBlocked)
	{
		FInstructionHeader* Header = reinterpret_cast<FInstructionHeader*>(&Bytecode[Regs.ProgramCounter]);
		uint8* DataPtr = &Bytecode[Regs.ProgramCounter + sizeof(FInstructionHeader)];
		int32 NextPC = Regs.ProgramCounter + sizeof(FInstructionHeader) + Header->DataSize;
		
		// Use new handler lookup
		FHktOpHandler Handler = FHktOpRegistry::GetHandler(Header->Op);
		Handler(*this, DataPtr);

		Regs.ProgramCounter = NextPC;
	}
}

void FHktFlowVM::Op_WaitTime(FHktFlowVM& VM, uint8* Data)
{
	SCOPE_CYCLE_COUNTER(STAT_Op_WaitTime);

	float Duration = *reinterpret_cast<float*>(Data);
	VM.Regs.WaitTimer = Duration;
	VM.Regs.bBlocked = true;
}

void FHktFlowVM::Op_WaitUntilDestroyed(FHktFlowVM& VM, uint8* Data)
{
	struct FParam { uint8 RegIndex; };
	FParam* P = reinterpret_cast<FParam*>(Data);
	FFlowRegister& Target = VM.Regs.Get(P->RegIndex);

	if (Target.Type == EFlowRegType::Unit && VM.Mgr->IsUnitValid(Target.Unit))
	{
		VM.Regs.WaitTimer = 0.0f;
		VM.Regs.BlockingRegIndex = P->RegIndex;
		VM.Regs.bBlocked = true;
	}
}

void FHktFlowVM::Op_PlayAnim(FHktFlowVM& VM, uint8* Data)
{
	struct FParam { FName AnimName; };
	FParam* P = reinterpret_cast<FParam*>(Data);
	
	if (AActor* Actor = VM.Mgr->GetVisualActor(VM.Regs.OwnerUnit))
	{
		// Play Anim Logic...
		// UAnimInstance* AnimInstance = ...;
		// AnimInstance->Montage_Play(...);
	}
}

void FHktFlowVM::Op_SpawnProjectile(FHktFlowVM& VM, uint8* Data)
{
	struct FParam { UClass* Class; float Offset; float Speed; uint8 OutputReg; };
	FParam* P = reinterpret_cast<FParam*>(Data);

	if (VM.Mgr->IsUnitValid(VM.Regs.OwnerUnit))
	{
		FVector OwnerLoc = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
		FRotator OwnerRot = VM.Mgr->GetUnitRotation(VM.Regs.OwnerUnit);
		FVector SpawnLoc = OwnerLoc + (OwnerRot.Vector() * P->Offset);

		// 1. Manager를 통해 할당 (소유주는 시전자와 동일하게 설정)
		FUnitHandle NewHandle = VM.Mgr->AllocUnit(VM.Regs.OwnerPlayer, SpawnLoc, OwnerRot);

		// 2. Visual Actor 생성
		if (VM.World && P->Class)
		{
			AActor* SpawnedActor = VM.World->SpawnActor(P->Class, &SpawnLoc, &OwnerRot);
			VM.Mgr->SetVisualActor(NewHandle, SpawnedActor);
		}
		 
		// 3. 레지스터 저장
		FFlowRegister& OutReg = VM.Regs.Get(P->OutputReg);
		OutReg.SetUnit(NewHandle);
	}
}

void FHktFlowVM::Op_ModifyAttribute(FHktFlowVM& VM, uint8* Data)
{
	struct FParam { uint8 TargetReg; EHktAttribute AttrId; float Value; };
	FParam* P = reinterpret_cast<FParam*>(Data);

	FFlowRegister& Reg = VM.Regs.Get(P->TargetReg);
	
	if (Reg.Type == EFlowRegType::Unit)
	{
		if (FHktAttributeSet* Attrs = VM.Mgr->GetUnitAttrs(Reg.Unit))
		{
			Attrs->Modify(P->AttrId, P->Value);
		}
	}
	else if (Reg.Type == EFlowRegType::Player)
	{
		if (FHktAttributeSet* Attrs = VM.Mgr->GetPlayerAttrs(Reg.Player))
		{
			Attrs->Modify(P->AttrId, P->Value);
		}
	}
}

void FHktFlowVM::Op_ExplodeAndDamage(FHktFlowVM& VM, uint8* Data)
{
	SCOPE_CYCLE_COUNTER(STAT_Op_ExplodeAndDamage);

	struct FParam { float Radius; float DirectDmg; float DotDmg; float DotDuration; uint8 LocReg; };
	FParam* P = reinterpret_cast<FParam*>(Data);

	FVector CenterLoc = VM.Regs.Get(P->LocReg).ResolveLocation(VM.Mgr);
	
	// [Optimized] Use spatial index for range query instead of linear search
	TArray<FUnitHandle> AffectedUnits;
	VM.Mgr->QueryUnitsInSphere(CenterLoc, P->Radius, AffectedUnits, true);

	// Apply damage to all units in range
	for (const FUnitHandle& TargetHandle : AffectedUnits)
	{
		// Don't damage self
		if (TargetHandle == VM.Regs.OwnerUnit)
		{
			continue;
		}

		// Apply direct damage
		if (FHktAttributeSet* Attrs = VM.Mgr->GetUnitAttrs(TargetHandle))
		{
			Attrs->Modify(EHktAttribute::Health, -P->DirectDmg);
		}

		// TODO: Apply DoT (would require a separate system for ongoing effects)
	}
}

// ============================================================
// === Movement OpHandlers ===
// ============================================================

void FHktFlowVM::Op_MoveTo(FHktFlowVM& VM, uint8* Data)
{
  FVector* Target = reinterpret_cast<FVector*>(Data);

  if (VM.Mgr->IsUnitValid(VM.Regs.OwnerUnit))
  {
    VM.Regs.MoveTarget = *Target;
    VM.Regs.bIsMoving = true;
    // 실제 이동은 Tick에서 처리하거나 외부 시스템에 위임
  }
}

void FHktFlowVM::Op_MoveForward(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { float Speed; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  if (VM.Mgr->IsUnitValid(VM.Regs.OwnerUnit))
  {
    VM.Regs.MoveSpeed = P->Speed;
    VM.Regs.bIsMoving = true;

    // 전방 방향으로 이동 목표 설정 (먼 거리)
    FVector CurrentLoc = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
    FRotator CurrentRot = VM.Mgr->GetUnitRotation(VM.Regs.OwnerUnit);
    VM.Regs.MoveTarget = CurrentLoc + CurrentRot.Vector() * 100000.0f; // 매우 먼 거리
  }
}

void FHktFlowVM::Op_Stop(FHktFlowVM& VM, uint8* Data)
{
  VM.Regs.bIsMoving = false;
  VM.Regs.MoveSpeed = 0.0f;
}

void FHktFlowVM::Op_ApplyForce(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { FVector Dir; float Mag; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  if (VM.Mgr->IsUnitValid(VM.Regs.OwnerUnit))
  {
    // 힘 적용은 물리 시스템에 위임
    // 여기서는 위치를 즉시 이동시키는 간단한 구현
    FVector CurrentLoc = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
    FVector NewLoc = CurrentLoc + P->Dir.GetSafeNormal() * P->Mag;

    // EntityDatabase 직접 수정 (간소화된 구현)
    if (VM.Mgr->Entities.Locations.IsValidIndex(VM.Regs.OwnerUnit.Index))
    {
      VM.Mgr->Entities.Locations[VM.Regs.OwnerUnit.Index] = NewLoc;
    }
  }
}

// ============================================================
// === Wait OpHandlers ===
// ============================================================

void FHktFlowVM::Op_WaitArrival(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { float Tolerance; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  if (!VM.Regs.bIsMoving)
  {
    // 이동 중이 아니면 바로 통과
    return;
  }

  FVector CurrentPos = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
  float Dist = FVector::Dist(CurrentPos, VM.Regs.MoveTarget);

  if (Dist > P->Tolerance)
  {
    VM.Regs.bBlocked = true;
    // PC를 되돌려서 다음 Tick에 다시 체크
    VM.Regs.ProgramCounter -= (sizeof(FInstructionHeader) + sizeof(FParam));
  }
  else
  {
    // 도착
    VM.Regs.bIsMoving = false;
  }
}

void FHktFlowVM::Op_WaitCollision(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { uint8 OutReg; float Radius; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  // 범위 내 유닛 검색
  FVector CurrentPos = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
  TArray<FUnitHandle> NearbyUnits;
  VM.Mgr->QueryUnitsInSphere(CurrentPos, P->Radius, NearbyUnits, true);

  // 자기 자신 제외
  FUnitHandle Collided;
  for (const FUnitHandle& Handle : NearbyUnits)
  {
    if (Handle != VM.Regs.OwnerUnit)
    {
      Collided = Handle;
      break;
    }
  }

  if (Collided.IsValid())
  {
    // 충돌 발생, 결과 저장
    VM.Regs.Get(P->OutReg).SetUnit(Collided);
  }
  else
  {
    // 충돌 없음, 블로킹
    VM.Regs.bBlocked = true;
    VM.Regs.ProgramCounter -= (sizeof(FInstructionHeader) + sizeof(FParam));
  }
}

void FHktFlowVM::Op_WaitSignal(FHktFlowVM& VM, uint8* Data)
{
  FName* SignalName = reinterpret_cast<FName*>(Data);

  // Signal 시스템은 별도 구현 필요
  // 현재는 즉시 통과 (Placeholder)
  // TODO: Signal 시스템과 연동
}

// ============================================================
// === Flow Control OpHandlers ===
// ============================================================

void FHktFlowVM::Op_Jump(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { int32 Offset; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  // 상대 점프: 현재 PC + Offset
  // 주의: Tick에서 이미 NextPC로 증가시키므로, 여기서 조정
  VM.Regs.ProgramCounter += P->Offset - (sizeof(FInstructionHeader) + sizeof(FParam));
}

void FHktFlowVM::Op_Halt(FHktFlowVM& VM, uint8* Data)
{
  // 실행 종료: PC를 버퍼 끝으로 설정
  VM.Regs.ProgramCounter = VM.Bytecode.Num();
}

// ============================================================
// === Query OpHandlers ===
// ============================================================

void FHktFlowVM::Op_QuerySphere(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { float Radius; uint8 CenterReg; uint8 OutListReg; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  FVector CenterLoc = VM.Regs.Get(P->CenterReg).ResolveLocation(VM.Mgr);

  TArray<FUnitHandle> Results;
  VM.Mgr->QueryUnitsInSphere(CenterLoc, P->Radius, Results, true);

  // 결과를 레지스터에 저장
  VM.Regs.Get(P->OutListReg).SetUnitList(Results);
}

void FHktFlowVM::Op_ForEachTarget(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { uint8 ListReg; uint8 IterReg; int32 EndOffset; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  TArray<FUnitHandle>& List = VM.Regs.Get(P->ListReg).GetUnitList();
  int32& Index = VM.Regs.LoopIndex;

  if (Index < List.Num())
  {
    // 현재 항목을 Iterator 레지스터에 설정
    VM.Regs.Get(P->IterReg).SetUnit(List[Index]);
    Index++;
  }
  else
  {
    // 루프 종료: EndForEach로 점프
    Index = 0; // 리셋
    VM.Regs.ProgramCounter += P->EndOffset - (sizeof(FInstructionHeader) + sizeof(FParam));
  }
}

void FHktFlowVM::Op_EndForEach(FHktFlowVM& VM, uint8* Data)
{
  // ForEachTarget으로 돌아가기
  // Builder에서 기록한 ForEachStartOffset을 사용해야 함
  // 간소화된 구현: ForEachTarget 명령어를 다시 찾아서 점프

  // Bytecode를 역방향 탐색하여 ForEachTarget 찾기
  int32 SearchPC = VM.Regs.ProgramCounter - sizeof(FInstructionHeader);

  while (SearchPC > 0)
  {
    FInstructionHeader* Header = reinterpret_cast<FInstructionHeader*>(&VM.Bytecode[SearchPC]);

    if (Header->Op == ECoreOp::ForEachTarget)
    {
      // ForEachTarget으로 점프 (NextPC 계산을 고려)
      VM.Regs.ProgramCounter = SearchPC - (sizeof(FInstructionHeader));
      return;
    }

    SearchPC -= sizeof(FInstructionHeader);
    // 간단한 역방향 탐색 (실제로는 더 정교한 방법 필요)
    if (SearchPC > 0)
    {
      // 이전 명령어의 크기를 알 수 없으므로 단순화
      SearchPC -= 1;
    }
  }

  // ForEachTarget을 찾지 못함 - 그냥 진행
}

// ============================================================
// === Entity OpHandlers ===
// ============================================================

void FHktFlowVM::Op_SpawnEntity(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { FName Tag; uint8 OutReg; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  if (VM.Mgr)
  {
    // Owner의 위치에서 엔티티 생성
    FVector SpawnLoc = VM.Mgr->GetUnitLocation(VM.Regs.OwnerUnit);
    FRotator SpawnRot = VM.Mgr->GetUnitRotation(VM.Regs.OwnerUnit);

    // 새 유닛 할당
    FUnitHandle NewHandle = VM.Mgr->AllocUnit(VM.Regs.OwnerPlayer, SpawnLoc, SpawnRot);

    // 레지스터에 저장
    VM.Regs.Get(P->OutReg).SetUnit(NewHandle);

    // TODO: Tag 기반 비주얼 액터 생성 (EntityFactory 시스템 필요)
  }
}

void FHktFlowVM::Op_DestroyEntity(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { uint8 Reg; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  FFlowRegister& Reg = VM.Regs.Get(P->Reg);

  if (Reg.Type == EFlowRegType::Unit && VM.Mgr)
  {
    VM.Mgr->FreeUnit(Reg.Unit);
    Reg.Type = EFlowRegType::Empty;
  }
}

// ============================================================
// === Damage OpHandlers ===
// ============================================================

void FHktFlowVM::Op_SetDamage(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { uint8 TargetReg; float Damage; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  FFlowRegister& Reg = VM.Regs.Get(P->TargetReg);

  if (Reg.Type == EFlowRegType::Unit)
  {
    if (FHktAttributeSet* Attrs = VM.Mgr->GetUnitAttrs(Reg.Unit))
    {
      Attrs->Modify(EHktAttribute::Health, -P->Damage);
    }
  }
}

void FHktFlowVM::Op_ApplyDot(FHktFlowVM& VM, uint8* Data)
{
  struct FParam { uint8 TargetReg; float DmgPerSec; float Duration; };
  FParam* P = reinterpret_cast<FParam*>(Data);

  // DoT(Damage over Time) 시스템은 별도 구현 필요
  // 현재는 즉시 총 데미지 적용 (간소화)
  FFlowRegister& Reg = VM.Regs.Get(P->TargetReg);

  if (Reg.Type == EFlowRegType::Unit)
  {
    if (FHktAttributeSet* Attrs = VM.Mgr->GetUnitAttrs(Reg.Unit))
    {
      float TotalDamage = P->DmgPerSec * P->Duration;
      Attrs->Modify(EHktAttribute::Health, -TotalDamage);
    }
  }

  // TODO: DoT 효과를 별도 시스템에 등록하여 시간에 따라 적용
}