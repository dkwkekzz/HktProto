// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktFlowOpcodes.h"
#include "HktAttributeSet.h"

/**
 * Builder (Compiler)
 * 바이트코드를 편리하게 작성하기 위한 빌더 클래스
 */
struct HKTSIMULATION_API FHktFlowBuilder
{
	TArray<uint8>& Buffer;

	FHktFlowBuilder(TArray<uint8>& InBuffer) : Buffer(InBuffer) {}

	template<typename T>
	void PushData(const T& Val)
	{
		int32 Idx = Buffer.AddUninitialized(sizeof(T));
		FMemory::Memcpy(&Buffer[Idx], &Val, sizeof(T));
	}

	void PushHeader(HktOpCode Op, uint16 Size)
	{
		FInstructionHeader H = { Op, 0, Size };
		int32 Idx = Buffer.AddUninitialized(sizeof(FInstructionHeader));
		FMemory::Memcpy(&Buffer[Idx], &H, sizeof(FInstructionHeader));
	}

	// ============================================================
	// === 애니메이션 (자연어: "애니메이션 재생") ===
	// ============================================================

	FHktFlowBuilder& PlayAnim(FName AnimName)
	{
		PushHeader(ECoreOp::PlayAnim, sizeof(FName));
		PushData(AnimName);
		return *this;
	}

	FHktFlowBuilder& PlayAnimation(FGameplayTag AnimTag)
	{
		// GameplayTag를 FName으로 변환하여 저장
		FName AnimName = AnimTag.GetTagName();
		return PlayAnim(AnimName);
	}

	// ============================================================
	// === 대기 (자연어: "~까지 기다림") ===
	// ============================================================

	FHktFlowBuilder& WaitSeconds(float Duration)
	{
		PushHeader(ECoreOp::WaitTime, sizeof(float));
		PushData(Duration);
		return *this;
	}

	FHktFlowBuilder& WaitUntilDestroyed(uint8 TargetReg = 0)
	{
		struct FParam { uint8 RegIdx; } P = { TargetReg };
		PushHeader(ECoreOp::WaitUntilDestroyed, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& WaitUntilArrival(float Tolerance = 10.0f)
	{
		struct FParam { float Tolerance; } P = { Tolerance };
		PushHeader(ECoreOp::WaitArrival, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& WaitUntilCollision(uint8 OutCollidedReg = 0, float CollisionRadius = 50.0f)
	{
		struct FParam { uint8 OutReg; float Radius; } P = { OutCollidedReg, CollisionRadius };
		PushHeader(ECoreOp::WaitCollision, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& WaitUntilSignal(FGameplayTag SignalTag)
	{
		FName SignalName = SignalTag.GetTagName();
		PushHeader(ECoreOp::WaitSignal, sizeof(FName));
		PushData(SignalName);
		return *this;
	}

	// ============================================================
	// === 이동 (자연어: "목표로 이동") ===
	// ============================================================

	FHktFlowBuilder& MoveTo(FVector Target)
	{
		PushHeader(ECoreOp::MoveTo, sizeof(FVector));
		PushData(Target);
		return *this;
	}

	FHktFlowBuilder& MoveForward(float Speed)
	{
		struct FParam { float Speed; } P = { Speed };
		PushHeader(ECoreOp::MoveForward, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& Stop()
	{
		PushHeader(ECoreOp::Stop, 0);
		return *this;
	}

	FHktFlowBuilder& ApplyForce(FVector Direction, float Magnitude)
	{
		struct FParam { FVector Dir; float Mag; } P = { Direction, Magnitude };
		PushHeader(ECoreOp::ApplyForce, sizeof(FParam));
		PushData(P);
		return *this;
	}

	// ============================================================
	// === 엔티티 (자연어: "엔티티 생성/파괴") ===
	// ============================================================

	FHktFlowBuilder& SpawnEntity(FGameplayTag EntityTag, uint8 OutReg = 0)
	{
		FName EntityName = EntityTag.GetTagName();
		struct FParam { FName Tag; uint8 OutReg; } P = { EntityName, OutReg };
		PushHeader(ECoreOp::SpawnEntity, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& DestroyEntity(uint8 TargetReg)
	{
		struct FParam { uint8 Reg; } P = { TargetReg };
		PushHeader(ECoreOp::DestroyEntity, sizeof(FParam));
		PushData(P);
		return *this;
	}

	// ============================================================
	// === 데미지 (자연어: "데미지 적용") ===
	// ============================================================

	FHktFlowBuilder& SetDamage(uint8 TargetReg, float Damage)
	{
		struct FParam { uint8 TargetReg; float Damage; } P = { TargetReg, Damage };
		PushHeader(ECoreOp::SetDamage, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& ApplyBurning(uint8 TargetReg, float DmgPerSec, float Duration)
	{
		struct FParam { uint8 TargetReg; float DmgPerSec; float Duration; } P = { TargetReg, DmgPerSec, Duration };
		PushHeader(ECoreOp::ApplyDot, sizeof(FParam));
		PushData(P);
		return *this;
	}

	// ============================================================
	// === 범위 쿼리 (자연어: "주변 유닛 검색") ===
	// ============================================================

	FHktFlowBuilder& QueryNearby(float Radius, uint8 CenterReg, uint8 OutListReg)
	{
		struct FParam { float Radius; uint8 CenterReg; uint8 OutListReg; } P = { Radius, CenterReg, OutListReg };
		PushHeader(ECoreOp::QuerySphere, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& ForEachTarget(uint8 ListReg, uint8 IteratorReg)
	{
		// EndOffset은 EndForEach에서 패치됨 (빌드 타임에 알 수 없음)
		struct FParam { uint8 ListReg; uint8 IterReg; int32 EndOffset; } P = { ListReg, IteratorReg, 0 };
		ForEachStartOffset = Buffer.Num(); // 패치 위치 기록
		PushHeader(ECoreOp::ForEachTarget, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& EndForEach()
	{
		// ForEachTarget의 EndOffset 패치
		if (ForEachStartOffset > 0)
		{
			int32 CurrentOffset = Buffer.Num();
			int32 JumpOffset = CurrentOffset - ForEachStartOffset;

			// ForEachTarget의 EndOffset 필드 위치 계산
			int32 EndOffsetFieldPos = ForEachStartOffset + sizeof(FInstructionHeader) + sizeof(uint8) + sizeof(uint8);
			FMemory::Memcpy(&Buffer[EndOffsetFieldPos], &JumpOffset, sizeof(int32));
		}

		PushHeader(ECoreOp::EndForEach, 0);
		ForEachStartOffset = 0;
		return *this;
	}

	// ============================================================
	// === 흐름 제어 (자연어: "분기/종료") ===
	// ============================================================

	FHktFlowBuilder& Jump(int32 Offset)
	{
		struct FParam { int32 Offset; } P = { Offset };
		PushHeader(ECoreOp::Jump, sizeof(FParam));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& End()
	{
		PushHeader(ECoreOp::Halt, 0);
		return *this;
	}

	// ============================================================
	// === Legacy API (기존 호환성) ===
	// ============================================================

	FHktFlowBuilder& SpawnFireball(UClass* ProjectileClass, uint8 OutRegisterIndex = 0)
	{
		struct Param { UClass* C; float Off; float Spd; uint8 OutReg; } P = { ProjectileClass, 100.0f, 1500.0f, OutRegisterIndex };
		PushHeader(ECoreOp::SpawnProjectile, sizeof(Param));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& WaitForImpact(uint8 TargetRegisterIndex = 0)
	{
		return WaitUntilDestroyed(TargetRegisterIndex);
	}

	FHktFlowBuilder& Explode(float Radius, float Damage, float BurnDmg, float BurnTime, uint8 LocationRegisterIndex = 0)
	{
		struct Param { float R; float D; float BD; float BT; uint8 LocReg; } P = { Radius, Damage, BurnDmg, BurnTime, LocationRegisterIndex };
		PushHeader(ECoreOp::ExplodeAndDamage, sizeof(Param));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& ModifyHealth(uint8 TargetRegisterIndex, float DeltaValue)
	{
		struct Param { uint8 TargetReg; EHktAttribute AttrId; float Value; } P = { TargetRegisterIndex, EHktAttribute::Health, DeltaValue };
		PushHeader(ECoreOp::ModifyAttribute, sizeof(Param));
		PushData(P);
		return *this;
	}

private:
	// ForEach 패치용 오프셋 기록
	int32 ForEachStartOffset = 0;
};
