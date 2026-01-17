// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktFlowOpcodes.h"
#include "HktFlowVM.h"

void FHktOpRegistry::Register(HktOpCode Op, FHktOpHandler Handler)
{
	Handlers[Op] = Handler;
}

void FHktOpRegistry::EnsureInitialized()
{
	if (bInitialized) return;
	bInitialized = true;
	
	for (int i = 0; i < 256; ++i) 
	{
		Handlers[i] = NoOp;
	}

	Register(ECoreOp::WaitTime, FHktFlowVM::Op_WaitTime);
	Register(ECoreOp::WaitUntilDestroyed, FHktFlowVM::Op_WaitUntilDestroyed);
	Register(ECoreOp::PlayAnim, FHktFlowVM::Op_PlayAnim);
	Register(ECoreOp::SpawnProjectile, FHktFlowVM::Op_SpawnProjectile);
	Register(ECoreOp::ExplodeAndDamage, FHktFlowVM::Op_ExplodeAndDamage);
	Register(ECoreOp::ModifyAttribute, FHktFlowVM::Op_ModifyAttribute);
}
