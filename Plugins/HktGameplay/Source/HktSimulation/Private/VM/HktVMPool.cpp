// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktVMPool.h"

DEFINE_LOG_CATEGORY_STATIC(LogVMPool, Log, All);

FHktVMPool::FHktVMPool()
{
}

FHktVMPool::~FHktVMPool()
{
	Clear();
}

FHktFlowVM* FHktVMPool::Acquire(UWorld* World, FHktEntityManager* Mgr, FUnitHandle Owner)
{
	FHktFlowVM* VM = nullptr;

	if (AvailableIndices.Num() > 0)
	{
		// Reuse from pool
		int32 Index = AvailableIndices.Pop();
		VM = Pool[Index].Get();
		
		// Reinitialize the VM
		VM->World = World;
		VM->Mgr = Mgr;
		VM->Regs.OwnerUnit = Owner;
		
		if (Mgr)
		{
			VM->Regs.OwnerPlayer = Mgr->GetUnitOwner(Owner);
			VM->Regs.Get(0).SetVector(Mgr->GetUnitLocation(Owner));
		}

		UE_LOG(LogVMPool, Verbose, TEXT("Reused VM from pool (Index: %d)"), Index);
	}
	else
	{
		// Create new VM
		TUniquePtr<FHktFlowVM> NewVM = MakeUnique<FHktFlowVM>(World, Mgr, Owner);
		VM = NewVM.Get();
		Pool.Add(MoveTemp(NewVM));

		UE_LOG(LogVMPool, Verbose, TEXT("Created new VM (Total: %d)"), Pool.Num());
	}

	ActiveVMs.Add(VM);
	return VM;
}

void FHktVMPool::Release(FHktFlowVM* VM)
{
	if (!VM)
	{
		return;
	}

	// Remove from active set
	if (!ActiveVMs.Remove(VM))
	{
		UE_LOG(LogVMPool, Warning, TEXT("Attempted to release VM that wasn't tracked as active"));
		return;
	}

	// Find the VM in our pool
	int32 Index = INDEX_NONE;
	for (int32 i = 0; i < Pool.Num(); ++i)
	{
		if (Pool[i].Get() == VM)
		{
			Index = i;
			break;
		}
	}

	if (Index == INDEX_NONE)
	{
		UE_LOG(LogVMPool, Error, TEXT("Attempted to release VM not owned by this pool"));
		return;
	}

	// Check pool size limit
	if (AvailableIndices.Num() >= MaxPoolSize)
	{
		// Pool is full, destroy this VM
		Pool.RemoveAt(Index);
		
		// Update available indices to account for removed element
		for (int32& AvailIdx : AvailableIndices)
		{
			if (AvailIdx > Index)
			{
				AvailIdx--;
			}
		}

		UE_LOG(LogVMPool, Verbose, TEXT("Destroyed VM (pool at max size: %d)"), MaxPoolSize);
	}
	else
	{
		// Reset and return to pool
		ResetVM(VM);
		AvailableIndices.Add(Index);

		UE_LOG(LogVMPool, Verbose, TEXT("Released VM to pool (Available: %d)"), AvailableIndices.Num());
	}
}

void FHktVMPool::Prewarm(int32 Count)
{
	for (int32 i = 0; i < Count; ++i)
	{
		TUniquePtr<FHktFlowVM> NewVM = MakeUnique<FHktFlowVM>(nullptr, nullptr, FUnitHandle());
		int32 Index = Pool.Add(MoveTemp(NewVM));
		AvailableIndices.Add(Index);
	}

	UE_LOG(LogVMPool, Log, TEXT("Prewarmed pool with %d VMs"), Count);
}

void FHktVMPool::Clear()
{
	Pool.Empty();
	AvailableIndices.Empty();
	ActiveVMs.Empty();

	UE_LOG(LogVMPool, Log, TEXT("Cleared VM pool"));
}

void FHktVMPool::GetStats(int32& OutTotalVMs, int32& OutAvailableVMs, int32& OutActiveVMs) const
{
	OutTotalVMs = Pool.Num();
	OutAvailableVMs = AvailableIndices.Num();
	OutActiveVMs = ActiveVMs.Num();
}

void FHktVMPool::ResetVM(FHktFlowVM* VM)
{
	if (!VM)
	{
		return;
	}

	// Clear bytecode
	VM->Bytecode.Reset();

	// Reset registers
	VM->Regs = FFlowRegisters();

	// Clear world/manager references
	VM->World = nullptr;
	VM->Mgr = nullptr;
}
