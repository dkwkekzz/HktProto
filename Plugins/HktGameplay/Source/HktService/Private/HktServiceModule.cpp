#include "IHktServiceModule.h"
#include "HktServiceSubsystem.h"

#define LOCTEXT_NAMESPACE "FHktServiceModule"

class FHktServiceModule : public IHktServiceModule
{
	virtual void StartupModule() override
	{
		// HktService module startup
	}

	virtual void ShutdownModule() override
	{
		// HktService module shutdown
	}
};

IMPLEMENT_MODULE(FHktServiceModule, HktService)

#undef LOCTEXT_NAMESPACE

