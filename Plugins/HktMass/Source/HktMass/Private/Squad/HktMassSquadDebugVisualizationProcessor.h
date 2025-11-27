// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassSquadDebugVisualizationProcessor.generated.h"

/**
 * Processor for visualizing HktMass squad debug data
 */
UCLASS()
class UHktMassSquadDebugVisualizationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassSquadDebugVisualizationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};


