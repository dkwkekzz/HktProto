// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassRepresentationProcessor.h"
#include "HktMassVisualizationProcessor.generated.h"

/**
 * Visualization Processor
 * Visualizes Mass entities in the world
 */
UCLASS()
class HKTMASS_API UHktMassVisualizationProcessor : public UMassVisualizationProcessor
{
	GENERATED_BODY()

public:
	UHktMassVisualizationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};

