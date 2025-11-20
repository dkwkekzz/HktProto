// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassCollisionProcessor.generated.h"

UCLASS()
class UHktMassCollisionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassCollisionProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	// 쿼리: 위치, 속도, 충돌 정보가 있는 엔티티만 필터링
	FMassEntityQuery EntityQuery;
};

