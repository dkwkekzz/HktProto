// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "HktServiceInterfaces.h"
#include "IHktJobProvider.generated.h"

/**
 * [Flow Changed Data]
 * Contains information about flow changes that occurred during an update.
 * Passed to subscribers via the OnFlowChanged delegate.
 */
USTRUCT(BlueprintType)
struct HKTSERVICE_API FHktFlowChangedData
{
	GENERATED_BODY()
};

/** Delegate broadcast when flows are changed (added or removed) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlowChanged, const FHktFlowChangedData&);

/**
 * [IHktJobProvider Interface]
 * Interface for providing job containers to execution systems.
 * Implemented by HktFlowSubsystem, accessed via HktService.
 * 
 * This interface allows the simulation module to access jobs
 * without directly depending on the behavior module.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UHktJobProvider : public UInterface
{
	GENERATED_BODY()
};

class HKTSERVICE_API IHktJobProvider
{
	GENERATED_BODY()

public:
};

