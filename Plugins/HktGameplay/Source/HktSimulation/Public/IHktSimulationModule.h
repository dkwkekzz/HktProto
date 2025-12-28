// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * HktSimulation Module Interface
 * 
 * This module handles deterministic world simulation based on events from HktIntent.
 * It produces visual fragments that are consumed by HktPresentation.
 * 
 * Key principles:
 * - Read-only access to HktIntent (never modifies events)
 * - Deterministic: same input always produces same output
 * - Frame-based: processes fixed timestep simulation
 */
class HKTSIMULATION_API IHktSimulationModule : public IModuleInterface
{
public:
	static inline IHktSimulationModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IHktSimulationModule>("HktSimulation");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("HktSimulation");
	}
};

