// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "IHktPresentationModule.h"

#define LOCTEXT_NAMESPACE "FHktPresentationModule"

class FHktPresentationModule : public IHktPresentationModule
{
	virtual void StartupModule() override
	{
		// HktPresentation module startup
	}

	virtual void ShutdownModule() override
	{
		// HktPresentation module shutdown
	}
};

IMPLEMENT_MODULE(FHktPresentationModule, HktPresentation)

#undef LOCTEXT_NAMESPACE

