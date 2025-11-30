#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "HktMassSquadMoveToLeaderProcessor.generated.h"

/**
 * 분대원이 분대장을 따라가도록 하는 프로세서
 */
UCLASS(MinimalAPI)
class UHktMassSquadMoveToLeaderProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassSquadMoveToLeaderProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
