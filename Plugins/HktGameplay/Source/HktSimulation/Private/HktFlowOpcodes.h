// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * OpCode Definition & Registry
 * VM이 실행할 명령어 정의 및 핸들러 레지스트리
 */

class FHktFlowVM;

using HktOpCode = uint8;
using FHktOpHandler = void (*)(FHktFlowVM& VM, uint8* Payload);

namespace ECoreOp
{
	// === 기본 (0-9) ===
	static constexpr HktOpCode None = 0;
	static constexpr HktOpCode WaitTime = 1;
	static constexpr HktOpCode WaitUntilDestroyed = 2;

	// === 애니메이션 (10-19) ===
	static constexpr HktOpCode PlayAnim = 10;
	static constexpr HktOpCode SpawnProjectile = 11;
	static constexpr HktOpCode DestroyUnit = 12;

	// === 속성 (20-29) ===
	static constexpr HktOpCode ModifyAttribute = 20;
	static constexpr HktOpCode ExplodeAndDamage = 21;

	// === 이동 (30-39) ===
	static constexpr HktOpCode MoveTo = 30;           // 목표 위치로 이동 시작
	static constexpr HktOpCode MoveForward = 31;      // 전방으로 이동
	static constexpr HktOpCode Stop = 32;             // 이동 중지
	static constexpr HktOpCode ApplyForce = 33;       // 힘 적용

	// === 이벤트 대기 (40-49) ===
	static constexpr HktOpCode WaitArrival = 40;      // 도착까지 대기
	static constexpr HktOpCode WaitCollision = 41;    // 충돌까지 대기 (충돌 상대 Reg에 저장)
	static constexpr HktOpCode WaitSignal = 42;       // 시그널 대기 (애니메이션 종료 등)
	static constexpr HktOpCode WaitAny = 43;          // 복수 이벤트 중 하나 대기

	// === 분기 (50-59) ===
	static constexpr HktOpCode Branch = 50;           // 조건 분기 (이벤트 타입별)
	static constexpr HktOpCode Jump = 51;             // 무조건 점프
	static constexpr HktOpCode Halt = 52;             // 실행 종료

	// === 범위 쿼리 (60-69) ===
	static constexpr HktOpCode QuerySphere = 60;      // 구 범위 쿼리
	static constexpr HktOpCode ForEachTarget = 61;    // 쿼리 결과 순회
	static constexpr HktOpCode EndForEach = 62;       // 순회 종료

	// === 엔티티 (70-79) ===
	static constexpr HktOpCode SpawnEntity = 70;      // 엔티티 생성 (GameplayTag 기반)
	static constexpr HktOpCode DestroyEntity = 71;    // 엔티티 파괴

	// === 데미지 (80-89) ===
	static constexpr HktOpCode SetDamage = 80;        // 직접 데미지
	static constexpr HktOpCode ApplyDot = 81;         // DoT 적용

	static constexpr HktOpCode EndOfStream = 255;
}

/**
 * Enhanced Opcode Registry with dynamic registration and module-based ranges
 */
struct HKTSIMULATION_API FHktOpRegistry
{
	// No-op handler for unregistered opcodes
	static void NoOp(FHktFlowVM&, uint8*) {}

	/**
	 * Register an opcode handler
	 * @param Op - The opcode to register
	 * @param Handler - The handler function
	 * @param Category - Optional category name for organization
	 * @return true if registration was successful
	 */
	static bool RegisterOpcode(HktOpCode Op, FHktOpHandler Handler, FName Category = NAME_None);

	/**
	 * Unregister an opcode handler
	 * @param Op - The opcode to unregister
	 */
	static void UnregisterOpcode(HktOpCode Op);

	/**
	 * Check if an opcode is registered
	 */
	static bool IsRegistered(HktOpCode Op);

	/**
	 * Allocate a range of opcodes for a module
	 * @param ModuleName - Name of the module requesting opcodes
	 * @param Count - Number of opcodes requested
	 * @return The first opcode in the allocated range, or 255 if allocation failed
	 */
	static HktOpCode AllocateOpcodeRange(FName ModuleName, int32 Count);

	/**
	 * Get the opcode range allocated to a module
	 * @param ModuleName - Name of the module
	 * @param OutStart - Output parameter for the start of the range
	 * @param OutEnd - Output parameter for the end of the range (exclusive)
	 * @return true if the module has an allocated range
	 */
	static bool GetModuleRange(FName ModuleName, HktOpCode& OutStart, HktOpCode& OutEnd);

	/**
	 * Get statistics about opcode usage
	 */
	static int32 GetRegisteredCount();

	/**
	 * Get the handler for an opcode
	 */
	static FHktOpHandler GetHandler(HktOpCode Op);

	/**
	 * Legacy registration for backward compatibility
	 */
	static void Register(HktOpCode Op, FHktOpHandler Handler);

	/**
	 * Initialize core opcodes (called once)
	 */
	static void EnsureInitialized();

private:
	/**
	 * Handler map (Meyers Singleton to avoid static initialization order issues)
	 */
	static TMap<HktOpCode, FHktOpHandler>& GetHandlers();

	/**
	 * Module range allocations
	 */
	static TMap<FName, TPair<HktOpCode, HktOpCode>>& GetModuleRanges();

	/**
	 * Category tracking for debugging/profiling
	 */
	static TMap<HktOpCode, FName>& GetOpcodeCategories();

	/**
	 * Next available opcode for dynamic allocation
	 * Core opcodes use 0-99, dynamic allocation starts at 100
	 */
	static HktOpCode& GetNextDynamicOpcode();

	inline static bool bInitialized = false;
};

#pragma pack(push, 1)
struct FInstructionHeader
{
	HktOpCode Op;
	uint8 Flags;    
	uint16 DataSize;
};
#pragma pack(pop)

/**
 * Helper macro for auto-registering opcode handlers
 * Usage: REGISTER_OPCODE_HANDLER(MyOp, MyHandler, "MyCategory")
 */
#define REGISTER_OPCODE_HANDLER(Opcode, Handler, Category) \
	namespace { \
		struct F##Opcode##AutoRegister { \
			F##Opcode##AutoRegister() { \
				FHktOpRegistry::RegisterOpcode(Opcode, Handler, FName(Category)); \
			} \
		}; \
		static F##Opcode##AutoRegister G##Opcode##AutoRegister; \
	}
