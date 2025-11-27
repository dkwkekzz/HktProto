// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "HktMassSquadFragmentInitializer.generated.h"

/**
 * 분대(Squad) Entity가 생성될 때(SquadFragment가 추가될 때) 실행되는 Observer.
 * 1. 분대 서브시스템에 분대 Entity 등록
 * 2. 설정에 따라 분대원(Member) Entity들 스폰
 */
UCLASS()
class UHktMassSquadFragmentInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UHktMassSquadFragmentInitializer();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
