// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

class FHktCodexModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_MODULE(FHktCodexModule, HktCodex)

