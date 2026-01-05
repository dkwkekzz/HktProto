// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "IHktFlowModule.h"

#define LOCTEXT_NAMESPACE "FHktFlowModule"

class FHktFlowModule : public IHktFlowModule
{
	virtual void StartupModule() override
	{
		// HktBehavior module startup
	}

	virtual void ShutdownModule() override
	{
		// HktBehavior module shutdown
	}
};

IMPLEMENT_MODULE(FHktFlowModule, HktBehavior)

#undef LOCTEXT_NAMESPACE

