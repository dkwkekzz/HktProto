// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "HktFlowInterfaces.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * HktBehavior Module Interface
 * 
 * This module handles behavior definition and job construction based on events.
 * Behaviors are created per EventTag and define jobs that can be executed by the simulation.
 * 
 * Key concepts:
 * - IHktFlow: Interface for defining jobs per event type
 * - FHktJobBuilder: Constructs job graphs from behavior definitions
 * - FHktJobHandle: Abstracted job handle with Op and values
 * - FHktJobContainer: Cache-friendly storage for job handles
 */
class HKTFLOW_API IHktFlowModule : public IModuleInterface
{
public:
	static inline IHktFlowModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IHktFlowModule>("HktFlow");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("HktFlow");
	}
};

