// Copyright Hkt Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HktMassNpcClientInterpolationProcessor.generated.h"

/**
 * 클라이언트 전용 보간 Processor
 * 서버에서 받은 Position + Velocity로 부드러운 이동 구현
 * 네트워크 패킷 간 끊김 현상 해결
 */
UCLASS()
class HKTMASS_API UHktMassNpcClientInterpolationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHktMassNpcClientInterpolationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

