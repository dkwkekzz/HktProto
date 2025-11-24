#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "HktMassSquadCommandProcessor.generated.h"

/**
 * 분대원의 명령을 분대장에게 전달하는 프로세서
 */
UCLASS(MinimalAPI)
class UHktMassSquadCommandProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassSquadCommandProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
