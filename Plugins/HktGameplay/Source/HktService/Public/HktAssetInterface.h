#pragma once

#include "HktServiceInterface.h"
#include "IHktAssetProvider.generated.h"

// 단일 UDataAsset을 반환하도록 변경 (1 Tag : 1 Asset)
DECLARE_DELEGATE_OneParam(FOnQueryDataComplete, UDataAsset*);

class UHktActionDataAsset;

UINTERFACE(MinimalAPI, BlueprintType)
class UHktAssetProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for a system that provides game data/asset information.
 * Implemented by HktAsset module (Subsystem).
 */
class HKTSERVICE_API IHktAssetProvider
{
	GENERATED_BODY()

public:
	virtual void QueryActionData(const FHktUnitHandle& InUnitHandle, FOnQueryDataComplete Callback) const = 0;
};
