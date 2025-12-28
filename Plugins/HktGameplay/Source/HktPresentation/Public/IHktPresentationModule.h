// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * HktPresentation Module Interface
 * 
 * This module handles pure rendering/visualization of simulation results.
 * 
 * Key principles:
 * - ZERO LOGIC: No game logic, no state changes
 * - ZERO REFERENCES: Does not reference HktIntent or HktSimulation modules
 * - DATA-DRIVEN: Only renders data from MassEntity fragments
 * 
 * Data is injected by HktSimulation through shared MassEntity fragments,
 * creating a clean separation without direct module dependencies.
 */
class HKTPRESENTATION_API IHktPresentationModule : public IModuleInterface
{
public:
	static inline IHktPresentationModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IHktPresentationModule>("HktPresentation");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("HktPresentation");
	}
};

