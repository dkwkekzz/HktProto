// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktFlowOpcodes.h"
#include "HktFlowVM.h"

DEFINE_LOG_CATEGORY_STATIC(LogOpRegistry, Log, All);

TMap<HktOpCode, FHktOpHandler>& FHktOpRegistry::GetHandlers()
{
	static TMap<HktOpCode, FHktOpHandler> Handlers;
	return Handlers;
}

TMap<FName, TPair<HktOpCode, HktOpCode>>& FHktOpRegistry::GetModuleRanges()
{
	static TMap<FName, TPair<HktOpCode, HktOpCode>> Ranges;
	return Ranges;
}

TMap<HktOpCode, FName>& FHktOpRegistry::GetOpcodeCategories()
{
	static TMap<HktOpCode, FName> Categories;
	return Categories;
}

HktOpCode& FHktOpRegistry::GetNextDynamicOpcode()
{
	// Core opcodes use 0-99, dynamic allocation starts at 100
	static HktOpCode NextOpcode = 100;
	return NextOpcode;
}

bool FHktOpRegistry::RegisterOpcode(HktOpCode Op, FHktOpHandler Handler, FName Category)
{
	if (!Handler)
	{
		UE_LOG(LogOpRegistry, Error, TEXT("Attempted to register null handler for opcode %d"), Op);
		return false;
	}

	TMap<HktOpCode, FHktOpHandler>& Handlers = GetHandlers();
	
	if (Handlers.Contains(Op))
	{
		UE_LOG(LogOpRegistry, Warning, TEXT("Opcode %d already registered, replacing handler"), Op);
	}

	Handlers.Add(Op, Handler);
	
	if (Category != NAME_None)
	{
		GetOpcodeCategories().Add(Op, Category);
	}

	UE_LOG(LogOpRegistry, Verbose, TEXT("Registered opcode %d (Category: %s)"), Op, *Category.ToString());
	return true;
}

void FHktOpRegistry::UnregisterOpcode(HktOpCode Op)
{
	TMap<HktOpCode, FHktOpHandler>& Handlers = GetHandlers();
	
	if (Handlers.Remove(Op) > 0)
	{
		GetOpcodeCategories().Remove(Op);
		UE_LOG(LogOpRegistry, Verbose, TEXT("Unregistered opcode %d"), Op);
	}
}

bool FHktOpRegistry::IsRegistered(HktOpCode Op)
{
	return GetHandlers().Contains(Op);
}

HktOpCode FHktOpRegistry::AllocateOpcodeRange(FName ModuleName, int32 Count)
{
	if (ModuleName == NAME_None || Count <= 0)
	{
		UE_LOG(LogOpRegistry, Error, TEXT("Invalid module name or count for opcode range allocation"));
		return 255;
	}

	TMap<FName, TPair<HktOpCode, HktOpCode>>& Ranges = GetModuleRanges();
	
	if (Ranges.Contains(ModuleName))
	{
		UE_LOG(LogOpRegistry, Warning, TEXT("Module '%s' already has an allocated range"), *ModuleName.ToString());
		TPair<HktOpCode, HktOpCode> ExistingRange = Ranges[ModuleName];
		return ExistingRange.Key;
	}

	HktOpCode& NextOpcode = GetNextDynamicOpcode();
	
	if (NextOpcode + Count > 255)
	{
		UE_LOG(LogOpRegistry, Error, TEXT("Not enough opcodes available for module '%s' (requested %d)"), 
			*ModuleName.ToString(), Count);
		return 255;
	}

	HktOpCode StartOpcode = NextOpcode;
	HktOpCode EndOpcode = NextOpcode + Count;
	
	Ranges.Add(ModuleName, TPair<HktOpCode, HktOpCode>(StartOpcode, EndOpcode));
	NextOpcode = EndOpcode;

	UE_LOG(LogOpRegistry, Log, TEXT("Allocated opcode range [%d, %d) for module '%s'"), 
		StartOpcode, EndOpcode, *ModuleName.ToString());
	
	return StartOpcode;
}

bool FHktOpRegistry::GetModuleRange(FName ModuleName, HktOpCode& OutStart, HktOpCode& OutEnd)
{
	TMap<FName, TPair<HktOpCode, HktOpCode>>& Ranges = GetModuleRanges();
	
	if (TPair<HktOpCode, HktOpCode>* Range = Ranges.Find(ModuleName))
	{
		OutStart = Range->Key;
		OutEnd = Range->Value;
		return true;
	}
	
	return false;
}

int32 FHktOpRegistry::GetRegisteredCount()
{
	return GetHandlers().Num();
}

FHktOpHandler FHktOpRegistry::GetHandler(HktOpCode Op)
{
	TMap<HktOpCode, FHktOpHandler>& Handlers = GetHandlers();
	
	if (FHktOpHandler* Handler = Handlers.Find(Op))
	{
		return *Handler;
	}
	
	return NoOp;
}

void FHktOpRegistry::Register(HktOpCode Op, FHktOpHandler Handler)
{
	// Legacy interface - delegates to new method
	RegisterOpcode(Op, Handler, NAME_None);
}

void FHktOpRegistry::EnsureInitialized()
{
	if (bInitialized) return;
	bInitialized = true;

	// === 기존 Core Opcodes ===
	RegisterOpcode(ECoreOp::WaitTime, FHktFlowVM::Op_WaitTime, TEXT("Core.Wait"));
	RegisterOpcode(ECoreOp::WaitUntilDestroyed, FHktFlowVM::Op_WaitUntilDestroyed, TEXT("Core.Wait"));
	RegisterOpcode(ECoreOp::PlayAnim, FHktFlowVM::Op_PlayAnim, TEXT("Core.Animation"));
	RegisterOpcode(ECoreOp::SpawnProjectile, FHktFlowVM::Op_SpawnProjectile, TEXT("Core.Spawn"));
	RegisterOpcode(ECoreOp::ExplodeAndDamage, FHktFlowVM::Op_ExplodeAndDamage, TEXT("Core.Damage"));
	RegisterOpcode(ECoreOp.::ModifyAttribute, FHktFlowVM::Op_ModifyAttribute, TEXT("Core.Attribute"));

	// === Movement Opcodes ===
	RegisterOpcode(ECoreOp::MoveTo, FHktFlowVM::Op_MoveTo, TEXT("Movement"));
	RegisterOpcode(ECoreOp::MoveForward, FHktFlowVM::Op_MoveForward, TEXT("Movement"));
	RegisterOpcode(ECoreOp::Stop, FHktFlowVM::Op_Stop, TEXT("Movement"));
	RegisterOpcode(ECoreOp::ApplyForce, FHktFlowVM::Op_ApplyForce, TEXT("Movement"));

	// === Wait Opcodes ===
	RegisterOpcode(ECoreOp::WaitArrival, FHktFlowVM::Op_WaitArrival, TEXT("Wait"));
	RegisterOpcode(ECoreOp::WaitCollision, FHktFlowVM::Op_WaitCollision, TEXT("Wait"));
	RegisterOpcode(ECoreOp::WaitSignal, FHktFlowVM::Op_WaitSignal, TEXT("Wait"));

	// === Flow Control Opcodes ===
	RegisterOpcode(ECoreOp::Jump, FHktFlowVM::Op_Jump, TEXT("FlowControl"));
	RegisterOpcode(ECoreOp::Halt, FHktFlowVM::Op_Halt, TEXT("FlowControl"));

	// === Query Opcodes ===
	RegisterOpcode(ECoreOp::QuerySphere, FHktFlowVM::Op_QuerySphere, TEXT("Query"));
	RegisterOpcode(ECoreOp::ForEachTarget, FHktFlowVM::Op_ForEachTarget, TEXT("Query"));
	RegisterOpcode(ECoreOp::EndForEach, FHktFlowVM::Op_EndForEach, TEXT("Query"));

	// === Entity Opcodes ===
	RegisterOpcode(ECoreOp::SpawnEntity, FHktFlowVM::Op_SpawnEntity, TEXT("Entity"));
	RegisterOpcode(ECoreOp::DestroyEntity, FHktFlowVM::Op_DestroyEntity, TEXT("Entity"));

	// === Damage Opcodes ===
	RegisterOpcode(ECoreOp::SetDamage, FHktFlowVM::Op_SetDamage, TEXT("Damage"));
	RegisterOpcode(ECoreOp::ApplyDot, FHktFlowVM::Op_ApplyDot, TEXT("Damage"));

	UE_LOG(LogOpRegistry, Log, TEXT("Opcode registry initialized with %d core opcodes"), GetRegisteredCount());
}
