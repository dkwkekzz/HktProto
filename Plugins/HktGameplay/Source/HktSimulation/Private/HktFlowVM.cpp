// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktFlowVM.h"
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
		
		FHktOpRegistry::Handlers[Header->Op](*this, DataPtr);

		Regs.ProgramCounter = NextPC;
	}
}

void FHktFlowVM::Op_WaitTime(FHktFlowVM& VM, uint8* Data)
{
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
	struct FParam { float Radius; float DirectDmg; float DotDmg; float DotDuration; uint8 LocReg; };
	FParam* P = reinterpret_cast<FParam*>(Data);

	FVector CenterLoc = VM.Regs.Get(P->LocReg).ResolveLocation(VM.Mgr);
	
	// 범위 검사: Manager의 DB 순회
	int32 Num = VM.Mgr->Entities.Attributes.Num();
	for (int32 i = 0; i < Num; ++i)
	{
		if (!VM.Mgr->Entities.IsActive[i]) continue;
		
		FUnitHandle TargetHandle(i, VM.Mgr->Entities.Generations[i]);
		if (TargetHandle == VM.Regs.OwnerUnit) continue;

		FVector TargetLoc = VM.Mgr->Entities.Locations[i];
		if (FVector::DistSquared(CenterLoc, TargetLoc) <= (P->Radius * P->Radius))
		{
			VM.Mgr->Entities.Attributes[i].Modify(EHktAttribute::Health, -P->DirectDmg);
		}
	}
}
