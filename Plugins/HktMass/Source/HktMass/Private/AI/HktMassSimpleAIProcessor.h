#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "HktMassSimpleAIProcessor.generated.h"

/**
 * Fragment initializer for HktMassNpcAnimationFragment
 * Initializes animation timing when fragment is added to an entity
 */
UCLASS(MinimalAPI)
class UHktMassSimpleAIFragmentInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UHktMassSimpleAIFragmentInitializer();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

/**
 * 순찰 로직을 담당하는 프로세서
 * 목표 지점 도착 여부를 판단하고 다음 목표를 설정합니다.
 */
UCLASS(MinimalAPI)
class UHktMassSimpleAIProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassSimpleAIProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};