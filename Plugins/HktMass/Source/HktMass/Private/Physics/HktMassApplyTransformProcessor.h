// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassApplyTransformProcessor.generated.h"

/**
 * 속도(Velocity)를 기반으로 엔티티의 위치(Transform)를 업데이트하는 프로세서 (Integration)
 * 다른 모든 이동/충돌 계산이 끝난 후 실행되어야 합니다.
 */
UCLASS(MinimalAPI)
class UHktMassApplyTransformProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
UHktMassApplyTransformProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};


