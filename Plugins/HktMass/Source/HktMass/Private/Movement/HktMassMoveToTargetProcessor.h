// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassMoveToTargetProcessor.generated.h"

/**
 * 목표 위치(TargetLocation)를 향해 엔티티를 이동시키는 프로세서
 */
UCLASS(MinimalAPI)
class UHktMassMoveToTargetProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UHktMassMoveToTargetProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
