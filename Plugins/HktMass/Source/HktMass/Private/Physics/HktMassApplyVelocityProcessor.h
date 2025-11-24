// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassApplyVelocityProcessor.generated.h"

/**
 * 누적된 힘(Force)을 속도(Velocity)에 적용하는 프로세서 (F=ma)
 * MoveToTarget, Collision 프로세서 다음에 실행되어야 합니다.
 */
UCLASS(MinimalAPI)
class UHktMassApplyVelocityProcessor : public UMassProcessor
{
    GENERATED_BODY()
   
public:
    UHktMassApplyVelocityProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};

