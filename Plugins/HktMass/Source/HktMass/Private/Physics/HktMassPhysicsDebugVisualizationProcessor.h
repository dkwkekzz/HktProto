// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassPhysicsDebugVisualizationProcessor.generated.h"

/**
 * Processor for visualizing HktMass physics/movement debug data
 */
UCLASS()
class UHktMassPhysicsDebugVisualizationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassPhysicsDebugVisualizationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery VelocityQuery;
	FMassEntityQuery ForceQuery;
};


