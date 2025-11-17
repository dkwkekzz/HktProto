// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassNpcAIProcessor.generated.h"

// NPC??AI 로직??처리?�는 Processor
UCLASS()
class HKTMASS_API UHktMassNpcAIProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassNpcAIProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;

	// Helper functions
	void ProcessIdleState(FMassExecutionContext& Context, int32 EntityIndex);
	void ProcessPatrolState(FMassExecutionContext& Context, int32 EntityIndex);
	void ProcessChaseState(FMassExecutionContext& Context, int32 EntityIndex);
	void ProcessAttackState(FMassExecutionContext& Context, int32 EntityIndex);
};

// NPC???�찰 로직??처리?�는 Processor
UCLASS()
class HKTMASS_API UHktMassNpcPatrolProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassNpcPatrolProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

