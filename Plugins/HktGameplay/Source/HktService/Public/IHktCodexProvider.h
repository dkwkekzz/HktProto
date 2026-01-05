#pragma once

#include "HktServiceInterfaces.h"
#include "IHktCodexProvider.generated.h"

class UHktActionDataAsset;

UINTERFACE(MinimalAPI, BlueprintType)
class UHktCodexProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for a system that provides game data/codex information.
 * Implemented by HktCodex module (Subsystem).
 */
class HKTSERVICE_API IHktCodexProvider
{
	GENERATED_BODY()

public:
	virtual void QueryActionData(const FHktUnitHandle& InUnitHandle, TFunction<void(const TArray<UHktActionDataAsset*>&)> Callback) const = 0;
};

