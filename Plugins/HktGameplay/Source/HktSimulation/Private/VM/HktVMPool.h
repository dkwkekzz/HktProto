// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HktFlowVM.h"

/**
 * Object pool for FHktFlowVM instances
 * Reduces allocation overhead by reusing VM objects
 */
class HKTSIMULATION_API FHktVMPool
{
public:
	FHktVMPool();
	~FHktVMPool();

	/**
	 * Acquire a VM from the pool
	 * @param World - World context for the VM
	 * @param Mgr - Entity manager for the VM
	 * @param Owner - Owner unit handle
	 * @return A VM instance (either from pool or newly created)
	 */
	FHktFlowVM* Acquire(UWorld* World, FHktEntityManager* Mgr, FUnitHandle Owner);

	/**
	 * Release a VM back to the pool
	 * @param VM - The VM to release (will be reset for reuse)
	 */
	void Release(FHktFlowVM* VM);

	/**
	 * Prewarm the pool with a number of VM instances
	 * @param Count - Number of VMs to create
	 */
	void Prewarm(int32 Count);

	/**
	 * Clear all pooled VMs
	 */
	void Clear();

	/**
	 * Get pool statistics
	 */
	void GetStats(int32& OutTotalVMs, int32& OutAvailableVMs, int32& OutActiveVMs) const;

	/**
	 * Set the maximum pool size (default: 100)
	 * VMs beyond this limit will be destroyed instead of pooled
	 */
	void SetMaxPoolSize(int32 MaxSize) { MaxPoolSize = MaxSize; }

private:
	/**
	 * Reset a VM for reuse
	 */
	void ResetVM(FHktFlowVM* VM);

	// All VMs owned by this pool
	TArray<TUniquePtr<FHktFlowVM>> Pool;

	// Available VMs (indices into Pool array)
	TArray<int32> AvailableIndices;

	// Active VMs (for tracking)
	TSet<FHktFlowVM*> ActiveVMs;

	// Maximum pool size
	int32 MaxPoolSize = 100;
};
