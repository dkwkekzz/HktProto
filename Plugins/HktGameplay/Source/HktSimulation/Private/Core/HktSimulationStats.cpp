// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "HktSimulationStats.h"

// Define stats
DEFINE_STAT(STAT_ProcessIntentEvents);
DEFINE_STAT(STAT_TickVMs);
DEFINE_STAT(STAT_CleanupFinishedVMs);
DEFINE_STAT(STAT_BuildBytecode);
DEFINE_STAT(STAT_VMExecution);
DEFINE_STAT(STAT_SpatialQueries);
DEFINE_STAT(STAT_HandleMapping);
DEFINE_STAT(STAT_FlowRegistryLookup);

DEFINE_STAT(STAT_ActiveVMs);
DEFINE_STAT(STAT_TotalEntities);
DEFINE_STAT(STAT_ActiveEntities);
DEFINE_STAT(STAT_EventsProcessed);
DEFINE_STAT(STAT_VMsPooled);
DEFINE_STAT(STAT_BytecodeBuffersPooled);
DEFINE_STAT(STAT_SpatialIndexCells);
DEFINE_STAT(STAT_RegisteredFlowDefinitions);
DEFINE_STAT(STAT_RegisteredOpcodes);

DEFINE_STAT(STAT_VMPoolMemory);
DEFINE_STAT(STAT_BytecodePoolMemory);
DEFINE_STAT(STAT_EntityDatabaseMemory);
DEFINE_STAT(STAT_SpatialIndexMemory);
DEFINE_STAT(STAT_FlowRegistryMemory);

DEFINE_STAT(STAT_Op_WaitTime);
DEFINE_STAT(STAT_Op_WaitUntilDestroyed);
DEFINE_STAT(STAT_Op_PlayAnim);
DEFINE_STAT(STAT_Op_SpawnProjectile);
DEFINE_STAT(STAT_Op_ModifyAttribute);
DEFINE_STAT(STAT_Op_ExplodeAndDamage);
