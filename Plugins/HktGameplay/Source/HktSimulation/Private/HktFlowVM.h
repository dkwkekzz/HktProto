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
	Player,
	UnitList,  // 범위 쿼리 결과용
	Integer    // 루프 인덱스용
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
		int32 IntVal;          // 4 bytes (루프 인덱스)
	};

	// UnitList는 union에 넣지 않음 (동적 할당)
	TArray<FUnitHandle> UnitList;

	FFlowRegister()
	{
		Type = EFlowRegType::Empty;
		FMemory::Memzero(&VectorVal, sizeof(FVector));
	}

	void SetUnit(FUnitHandle InUnit) { Type = EFlowRegType::Unit; Unit = InUnit; }
	void SetVector(const FVector& InVec) { Type = EFlowRegType::Vector; VectorVal = InVec; }
	void SetScalar(float InScalar) { Type = EFlowRegType::Scalar; ScalarVal = InScalar; }
	void SetPlayer(FPlayerHandle InPlayer) { Type = EFlowRegType::Player; Player = InPlayer; }
	void SetInt(int32 InInt) { Type = EFlowRegType::Integer; IntVal = InInt; }
	void SetUnitList(const TArray<FUnitHandle>& InList) { Type = EFlowRegType::UnitList; UnitList = InList; }

	// Getters
	int32& GetInt() { return IntVal; }
	TArray<FUnitHandle>& GetUnitList() { return UnitList; }

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

	// 이동 관련 상태
	FVector MoveTarget = FVector::ZeroVector;
	float MoveSpeed = 0.0f;
	bool bIsMoving = false;

	// ForEach 루프 상태 (별도의 레지스터 대신 명시적 관리)
	int32 LoopIndex = 0;
	int32 LoopEndPC = 0;  // 루프 종료 후 점프할 PC

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

	// --- Movement OpHandlers ---
	static void Op_MoveTo(FHktFlowVM& VM, uint8* Data);
	static void Op_MoveForward(FHktFlowVM& VM, uint8* Data);
	static void Op_Stop(FHktFlowVM& VM, uint8* Data);
	static void Op_ApplyForce(FHktFlowVM& VM, uint8* Data);

	// --- Wait OpHandlers ---
	static void Op_WaitArrival(FHktFlowVM& VM, uint8* Data);
	static void Op_WaitCollision(FHktFlowVM& VM, uint8* Data);
	static void Op_WaitSignal(FHktFlowVM& VM, uint8* Data);

	// --- Flow Control OpHandlers ---
	static void Op_Jump(FHktFlowVM& VM, uint8* Data);
	static void Op_Halt(FHktFlowVM& VM, uint8* Data);

	// --- Query OpHandlers ---
	static void Op_QuerySphere(FHktFlowVM& VM, uint8* Data);
	static void Op_ForEachTarget(FHktFlowVM& VM, uint8* Data);
	static void Op_EndForEach(FHktFlowVM& VM, uint8* Data);

	// --- Entity OpHandlers ---
	static void Op_SpawnEntity(FHktFlowVM& VM, uint8* Data);
	static void Op_DestroyEntity(FHktFlowVM& VM, uint8* Data);

	// --- Damage OpHandlers ---
	static void Op_SetDamage(FHktFlowVM& VM, uint8* Data);
	static void Op_ApplyDot(FHktFlowVM& VM, uint8* Data);
};
