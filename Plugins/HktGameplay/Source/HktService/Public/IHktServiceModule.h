#pragma once

#include "HktServiceInterfaces.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class HKTSERVICE_API IHktServiceModule : public IModuleInterface
{
public:
	static inline IHktServiceModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IHktServiceModule>("HktService");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("HktService");
	}
};

