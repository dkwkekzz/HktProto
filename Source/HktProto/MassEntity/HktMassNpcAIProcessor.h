// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassNpcAIProcessor.generated.h"

// NPC의 AI 로직을 처리하는 Processor
UCLASS()
class HKTPROTO_API UHktMassNpcAIProcessor : public UMassProcessor
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

// NPC의 순찰 로직을 처리하는 Processor
UCLASS()
class HKTPROTO_API UHktMassNpcPatrolProcessor : public UMassProcessor
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

