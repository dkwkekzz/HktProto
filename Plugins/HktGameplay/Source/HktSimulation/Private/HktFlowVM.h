// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktEntityManager.h"
#include "HktFlowOpcodes.h"

/**
 * Data Layout & VM Implementation
 * 바이트코드 실행을 위한 가상 머신
 */

enum class EFlowRegType : uint8 
{ 
	Empty, 
	Unit, 
	Vector, 
	Scalar, 
	Player 
};

struct FFlowRegister
{
	EFlowRegType Type = EFlowRegType::Empty;

	union
	{
		FUnitHandle Unit;      // 8 bytes
		FPlayerHandle Player;  // 4 bytes
		FVector VectorVal;     // 24 bytes
		float ScalarVal;       // 4 bytes
	};
	
	FFlowRegister()
	{
		Type = EFlowRegType::Empty;
		FMemory::Memzero(&VectorVal, sizeof(FVector));
	}

	void SetUnit(FUnitHandle InUnit) { Type = EFlowRegType::Unit; Unit = InUnit; }
	void SetVector(const FVector& InVec) { Type = EFlowRegType::Vector; VectorVal = InVec; }
	void SetScalar(float InScalar) { Type = EFlowRegType::Scalar; ScalarVal = InScalar; }
	void SetPlayer(FPlayerHandle InPlayer) { Type = EFlowRegType::Player; Player = InPlayer; }
	
	// Helper Getter
	FVector ResolveLocation(const FHktEntityManager* Mgr) const;
};

struct FFlowRegisters
{
	FUnitHandle OwnerUnit;          
	FPlayerHandle OwnerPlayer; // 시전한 플레이어 정보 캐싱
	
	static constexpr int32 NumGPR = 8;
	FFlowRegister GPR[NumGPR];
	
	float WaitTimer = 0.0f;
	int32 ProgramCounter = 0;
	bool bBlocked = false;
	uint8 BlockingRegIndex = 0;

	FFlowRegister& Get(uint8 Index) { return GPR[Index % NumGPR]; }
};

class HKTSIMULATION_API FHktFlowVM
{
public:
	TArray<uint8> Bytecode;
	FFlowRegisters Regs;
	
	UWorld* World = nullptr;
	FHktEntityManager* Mgr = nullptr;

	FHktFlowVM(UWorld* InWorld, FHktEntityManager* InMgr, FUnitHandle InOwnerUnit);

	void Tick(float DeltaTime);

	// --- Core OpHandlers ---
	static void Op_WaitTime(FHktFlowVM& VM, uint8* Data);
	static void Op_WaitUntilDestroyed(FHktFlowVM& VM, uint8* Data);
	static void Op_PlayAnim(FHktFlowVM& VM, uint8* Data);
	static void Op_SpawnProjectile(FHktFlowVM& VM, uint8* Data);
	static void Op_ModifyAttribute(FHktFlowVM& VM, uint8* Data);
	static void Op_ExplodeAndDamage(FHktFlowVM& VM, uint8* Data);
};
