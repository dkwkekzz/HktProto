// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "IHktSimulationModule.h"

#define LOCTEXT_NAMESPACE "FHktSimulationModule"

class FHktSimulationModule : public IHktSimulationModule
{
	virtual void StartupModule() override
	{
		// HktSimulation module startup
	}

	virtual void ShutdownModule() override
	{
		// HktSimulation module shutdown
	}
};

IMPLEMENT_MODULE(FHktSimulationModule, HktSimulation)

#undef LOCTEXT_NAMESPACE

