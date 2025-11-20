#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "HktMassSquadMovementProcessor.generated.h"

/**
 * 분대원의 목표 위치(MoveToLocation)를 분대장 위치로 업데이트하는 프로세서
 */
UCLASS(MinimalAPI)
class UHktMassSquadMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassSquadMovementProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
