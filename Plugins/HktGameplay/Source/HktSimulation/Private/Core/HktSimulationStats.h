// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"

/**
 * Stats group for HktSimulation module
 * Use stat HktSimulation in console to view these stats
 */

DECLARE_STATS_GROUP(TEXT("HktSimulation"), STATGROUP_HktSimulation, STATCAT_Advanced);

// Timing stats
DECLARE_CYCLE_STAT_EXTERN(TEXT("Process Intent Events"), STAT_ProcessIntentEvents, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Tick VMs"), STAT_TickVMs, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Cleanup Finished VMs"), STAT_CleanupFinishedVMs, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Build Bytecode"), STAT_BuildBytecode, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("VM Execution"), STAT_VMExecution, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Spatial Queries"), STAT_SpatialQueries, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Handle Mapping"), STAT_HandleMapping, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Flow Registry Lookup"), STAT_FlowRegistryLookup, STATGROUP_HktSimulation, HKTSIMULATION_API);

// Counter stats
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active VMs"), STAT_ActiveVMs, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Total Entities"), STAT_TotalEntities, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active Entities"), STAT_ActiveEntities, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Events Processed"), STAT_EventsProcessed, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("VMs Pooled"), STAT_VMsPooled, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Bytecode Buffers Pooled"), STAT_BytecodeBuffersPooled, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Spatial Index Cells"), STAT_SpatialIndexCells, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Registered Flow Definitions"), STAT_RegisteredFlowDefinitions, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Registered Opcodes"), STAT_RegisteredOpcodes, STATGROUP_HktSimulation, HKTSIMULATION_API);

// Memory stats
DECLARE_MEMORY_STAT_EXTERN(TEXT("VM Pool Memory"), STAT_VMPoolMemory, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Bytecode Pool Memory"), STAT_BytecodePoolMemory, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Entity Database Memory"), STAT_EntityDatabaseMemory, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Spatial Index Memory"), STAT_SpatialIndexMemory, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Flow Registry Memory"), STAT_FlowRegistryMemory, STATGROUP_HktSimulation, HKTSIMULATION_API);

// Opcodes individual stats
DECLARE_CYCLE_STAT_EXTERN(TEXT("Op: WaitTime"), STAT_Op_WaitTime, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Op: WaitUntilDestroyed"), STAT_Op_WaitUntilDestroyed, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Op: PlayAnim"), STAT_Op_PlayAnim, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Op: SpawnProjectile"), STAT_Op_SpawnProjectile, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Op: ModifyAttribute"), STAT_Op_ModifyAttribute, STATGROUP_HktSimulation, HKTSIMULATION_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Op: ExplodeAndDamage"), STAT_Op_ExplodeAndDamage, STATGROUP_HktSimulation, HKTSIMULATION_API);
