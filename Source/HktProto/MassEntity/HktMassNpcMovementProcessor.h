// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassObserverProcessor.h"
#include "HktMassNpcMovementProcessor.generated.h"

// NPC의 이동을 처리하는 Processor
UCLASS()
class HKTPROTO_API UHktMassNpcMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassNpcMovementProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

