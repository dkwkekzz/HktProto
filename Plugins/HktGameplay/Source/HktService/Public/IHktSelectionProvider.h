#pragma once

#include "HktServiceInterfaces.h"
#include "IHktSelectionProvider.generated.h"

struct FHitResult;

UINTERFACE(MinimalAPI, BlueprintType)
class UHktSelectionProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for a system that provides selection information.
 * Implemented by HktPresentation module.
 */
class HKTSERVICE_API IHktSelectionProvider
{
	GENERATED_BODY()

public:
	virtual bool QuerySelectUnit(const FHitResult& InHitResult, FHktUnitHandle& OutUnitHandle) const = 0;
	virtual bool QuerySelectUnits(const FHitResult& InBeginHitResult, const FHitResult& InEndHitResult, TArray<FHktUnitHandle>& OutUnitHandles) const = 0;
};

