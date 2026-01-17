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
	static constexpr HktOpCode None = 0;
	static constexpr HktOpCode WaitTime = 1;
	static constexpr HktOpCode WaitUntilDestroyed = 2;
	static constexpr HktOpCode PlayAnim = 10;
	static constexpr HktOpCode SpawnProjectile = 11;
	static constexpr HktOpCode DestroyUnit = 12;
	static constexpr HktOpCode ModifyAttribute = 20;
	static constexpr HktOpCode ExplodeAndDamage = 21;
	static constexpr HktOpCode EndOfStream = 255;
}

struct HKTSIMULATION_API FHktOpRegistry
{
	inline static FHktOpHandler Handlers[256] = { nullptr };
	inline static bool bInitialized = false;

	static void NoOp(FHktFlowVM&, uint8*) {}
	static void Register(HktOpCode Op, FHktOpHandler Handler);
	static void EnsureInitialized();
};

#pragma pack(push, 1)
struct FInstructionHeader
{
	HktOpCode Op;
	uint8 Flags;    
	uint16 DataSize;
};
#pragma pack(pop)
