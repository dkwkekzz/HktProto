// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "IHktIntentModule.h"
#include "HktIntentSubsystem.h"

#define LOCTEXT_NAMESPACE "FHktIntentModule"

class FHktIntentModule : public IHktIntentModule
{
	virtual void StartupModule() override
	{
		// HktIntent module startup
	}

	virtual void ShutdownModule() override
	{
		// HktIntent module shutdown
	}
};

IMPLEMENT_MODULE(FHktIntentModule, HktIntent)

#undef LOCTEXT_NAMESPACE

