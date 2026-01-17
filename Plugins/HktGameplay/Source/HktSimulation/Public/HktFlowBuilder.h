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

	FHktFlowBuilder& PlayAnim(FName AnimName) 
	{
		PushHeader(ECoreOp::PlayAnim, sizeof(FName));
		PushData(AnimName);
		return *this;
	}

	FHktFlowBuilder& WaitSeconds(float Duration) 
	{
		PushHeader(ECoreOp::WaitTime, sizeof(float));
		PushData(Duration);
		return *this;
	}

	FHktFlowBuilder& SpawnFireball(UClass* ProjectileClass, uint8 OutRegisterIndex = 0) 
	{
		struct Param { UClass* C; float Off; float Spd; uint8 OutReg; } P = { ProjectileClass, 100.0f, 1500.0f, OutRegisterIndex };
		PushHeader(ECoreOp::SpawnProjectile, sizeof(Param));
		PushData(P);
		return *this;
	}

	FHktFlowBuilder& WaitForImpact(uint8 TargetRegisterIndex = 0) 
	{
		struct Param { uint8 RegIdx; } P = { TargetRegisterIndex };
		PushHeader(ECoreOp::WaitUntilDestroyed, sizeof(Param));
		PushData(P);
		return *this;
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
};
