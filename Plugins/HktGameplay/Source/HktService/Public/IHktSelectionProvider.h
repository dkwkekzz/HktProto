#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HktServiceTypes.h"
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
	virtual void QuerySelectUnits(const FHitResult& InHitResult, TFunction<void(const TArray<FHktUnitHandle>&)> Callback) const = 0;
};

