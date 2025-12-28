// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * HktIntent Module Interface
 * 
 * This module handles player input capture and event creation.
 * It receives input from PlayerController and creates frame-based events
 * that are stored and replicated for deterministic simulation.
 */
class HKTINTENT_API IHktIntentModule : public IModuleInterface
{
public:
	static inline IHktIntentModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IHktIntentModule>("HktIntent");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("HktIntent");
	}
};

